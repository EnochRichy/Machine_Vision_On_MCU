/*******************************************************************************
 * APP_DISPLAY.C - HUB75 LED Panel Display Implementation
 *
 * Implements HUB75 control, bitplane DMA, color conversion, and text overlay
 * rendering for the 64x64 RGB LED matrix.
 ******************************************************************************/

#include "app_display.h"
#include "definitions.h"
#include "peripheral/port/plib_port.h"
#include "app_cam.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================================
 * Hardware Definitions - GPIO and DMA Configuration
 * ============================================================================ */

/**
 * HUB75 GPIO Pin Masks for RGB Data Lines
 *
 * Standard HUB75 uses dual-bank color output:
 *   - R1/G1/B1: Top half of row pair (rows 0-31)
 *   - R2/G2/B2: Bottom half of row pair (rows 32-63)
 * Each pin is mapped to a specific GPIO bit position.
 */
#define PIN_R1   (1u << 0)
#define PIN_G1   (1u << 1)
#define PIN_B1   (1u << 2)
#define PIN_R2   (1u << 3)
#define PIN_G2   (1u << 4)
#define PIN_B2   (1u << 5)

/* Base address for HUB75 RGB output port (GPIO group 3, upper byte) */
#define LED_RGB_PORT_ADDR    ((const void *)((uint8_t *)&PORT_REGS->GROUP[3].PORT_OUT + 3))

/* DMA buffer size: BCM_BITS bitplanes × ROW_PAIRS × PANEL_WIDTH pixels */
/* Use PANEL_WIDTH/PANEL_HEIGHT/ROW_PAIRS as defined in app_cam.h */
#define DMA_BUF_SIZE (BCM_BITS * ROW_PAIRS * PANEL_WIDTH)

/* ============================================================================
 * Color Buffers (Source for LED Panel Rendering)
 * ============================================================================ */

/**
 * buffer_R, buffer_G, buffer_B - RGB color buffers (64x64 pixels)
 *
 * Intermediate representation after camera capture and color conversion.
 * Values are 5-bit color components (0-31 range) used for DMA bitplane
 * decomposition. Filled by camera module during rgb565_to_gray64().
 */
uint8_t buffer_R[PANEL_HEIGHT][PANEL_WIDTH];
uint8_t buffer_G[PANEL_HEIGHT][PANEL_WIDTH];
uint8_t buffer_B[PANEL_HEIGHT][PANEL_WIDTH];

/* HUB75 DMA ring buffer: stores bitplane-encoded data for DMA transfer */
static uint8_t hub75_dma_buf[DMA_BUF_SIZE];

/* ============================================================================
 * Gamma Correction Table (for LED brightness control)
 * ============================================================================ */

/**
 * gamma5 - Gamma correction lookup table for 5-bit color values
 *
 * Maps 5-bit input (0-31) to 8-bit output (0-255) with gamma curve.
 * Compensates for human eye's nonlinear perception of brightness.
 */
static const uint8_t gamma5[32] = {
    0,0,0,0,1,1,2,3,
    4,5,7,9,11,13,15,17,
    19,21,23,25,27,28,29,30,
    31,31,31,31,31,31,31,31
};

/* ============================================================================
 * BCM (Binary Code Modulation) Timing Configuration
 * ============================================================================ */

/**
 * bcm_time[] - LED on-time (in clock units) for each bitplane
 *
 * Implements BCM: each bitplane has exponentially longer display time,
 * allowing 32 brightness levels per channel (2^5 = 32).
 * Values: {1, 2, 4, 8, 12} control relative ON duration for bits 0-4.
 * Actual duration = bcm_time[bit] * BCM_BASE_TIME (set in app_cam.h).
 */
static const uint16_t bcm_time[BCM_BITS] = { 1, 2, 4, 8, 12 };

/* ============================================================================
 * Shared Buffers from Other Modules
 * ============================================================================ */

/* Camera module: raw RGB565 frame buffer (for cache coherency) */
extern uint8_t panda_scaled_data[FRAME_BYTES];

/* ============================================================================
 * Helper Functions - HUB75 Data Conversion
 * ============================================================================ */

/**
 * hub75_pixel_to_gpio_rgb - Convert two 5-bit RGB pixels to GPIO output byte
 *
 * Packs 6 color components (RGB × 2 rows) into a single GPIO byte for
 * simultaneous transfer to HUB75 panel. One bit per color line.
 *
 * Parameters:
 *   r_top, g_top, b_top - Top row pixel (5-bit values, one bit at position 'bit')
 *   r_bot, g_bot, b_bot - Bottom row pixel (5-bit values, one bit at position 'bit')
 *   bit - Which bitplane bit to extract (0-4)
 *
 * Returns:
 *   GPIO byte with R1/G1/B1/R2/G2/B2 bits set according to 'bit' position
 */
static inline uint8_t hub75_pixel_to_gpio_rgb(
    uint8_t r_top, uint8_t g_top, uint8_t b_top,
    uint8_t r_bot, uint8_t g_bot, uint8_t b_bot,
    uint8_t bit)
{
    uint8_t out = 0;
    if (r_top & (1u << bit)) out |= PIN_R1;
    if (g_top & (1u << bit)) out |= PIN_G1;
    if (b_top & (1u << bit)) out |= PIN_B1;
    if (r_bot & (1u << bit)) out |= PIN_R2;
    if (g_bot & (1u << bit)) out |= PIN_G2;
    if (b_bot & (1u << bit)) out |= PIN_B2;
    return out;
}

/**
 * hub75_pixel_to_gpio - Convert two grayscale pixels to GPIO output byte
 *
 * Simplified version for grayscale: duplicates value to all color channels
 * to produce white/gray pixel on the LED panel.
 *
 * Parameters:
 *   g_top, g_bot - Grayscale values for top and bottom row
 *   bit - Bitplane bit position (0-4)
 *
 * Returns:
 *   GPIO byte with R/G/B bits set identically for gray appearance
 */
static inline uint8_t hub75_pixel_to_gpio(uint8_t g_top, uint8_t g_bot, uint8_t bit)
{
    uint8_t out = 0;
    if (g_top & (1u << bit)) out |= (PIN_R1 | PIN_G1 | PIN_B1);
    if (g_bot & (1u << bit)) out |= (PIN_R2 | PIN_G2 | PIN_B2);
    return out;
}

/**
 * rgb_to_hub75_dma - Convert RGB buffer to HUB75 bitplane DMA data
 *
 * Decomposes 5-bit RGB color buffers into BCM bitplanes. Result is stored
 * in dma_buf in row-major bitplane order for efficient DMA transfer.
 *
 * Layout: for each bitplane (0-4), for each row pair (0-31), write all
 * 64 pixels with that bitplane bit extracted and mapped to GPIO pins.
 *
 * Parameters:
 *   dma_buf - Output buffer (size DMA_BUF_SIZE); bitplane-encoded pixel data
 */
void rgb_to_hub75_dma(uint8_t *dma_buf)
{
    uint32_t idx = 0;
    for (uint8_t bit = 0; bit < BCM_BITS; bit++) {
        for (uint8_t row = 0; row < ROW_PAIRS; row++) {
            uint8_t *r_top = buffer_R[row];
            uint8_t *g_top = buffer_G[row];
            uint8_t *b_top = buffer_B[row];
            uint8_t *r_bot = buffer_R[row + ROW_PAIRS];
            uint8_t *g_bot = buffer_G[row + ROW_PAIRS];
            uint8_t *b_bot = buffer_B[row + ROW_PAIRS];
            for (uint8_t x = 0; x < PANEL_WIDTH; x++) {
                dma_buf[idx++] = hub75_pixel_to_gpio_rgb(
                    r_top[x], g_top[x], b_top[x],
                    r_bot[x], g_bot[x], b_bot[x], bit);
            }
        }
    }
}

/**
 * greyscale_to_hub75_dma - Convert grayscale image to HUB75 bitplane DMA data
 *
 * Similar to rgb_to_hub75_dma but for grayscale input (single 8-bit value
 * per pixel). Converts to 5-bit by right-shifting and replicates across
 * R/G/B channels.
 *
 * Parameters:
 *   greyscale_img - Input grayscale image (64x64, 8-bit values)
 *   dma_buf - Output bitplane DMA buffer
 */
void greyscale_to_hub75_dma(const uint8_t *greyscale_img, uint8_t *dma_buf)
{
    uint32_t idx = 0;
    for (uint8_t bit = 0; bit < BCM_BITS; bit++) {
        for (uint8_t row = 0; row < ROW_PAIRS; row++) {
            uint32_t top_row = row * PANEL_WIDTH;
            uint32_t bot_row = (row + ROW_PAIRS) * PANEL_WIDTH;
            for (uint8_t x = 0; x < PANEL_WIDTH; x++) {
                uint8_t g_top = greyscale_img[top_row + x] >> 5;  /* 8-bit → 5-bit */
                uint8_t g_bot = greyscale_img[bot_row + x] >> 5;
                dma_buf[idx++] = hub75_pixel_to_gpio(g_top, g_bot, bit);
            }
        }
    }
}

/* ============================================================================
 * Helper Functions - Timing and Row Control
 * ============================================================================ */

/**
 * delay_nops - Busy-wait delay using NOP instructions
 *
 * Used for sub-microsecond timing adjustments in the DMA refresh cycle.
 * Approximately: 1 iteration ≈ 1-2 CPU cycles at typical 48-120 MHz.
 *
 * Parameters:
 *   n - Number of NOP iterations
 */
static inline void delay_nops(uint32_t n)
{
    while (n--) { __asm volatile("nop"); }
}

/**
 * set_row - Set HUB75 row address (0-31)
 *
 * Controls A/B/C/D/E address lines to select which row pair is currently
 * active on the LED panel. Each row pair displays simultaneously.
 *
 * Parameters:
 *   row - Row index (0-31); binary representation sets A-E pins
 */
static inline void set_row(uint8_t row)
{
    LED_PANEL_A_Clear(); LED_PANEL_B_Clear(); LED_PANEL_C_Clear();
    LED_PANEL_D_Clear(); LED_PANEL_E_Clear();
    if (row & 0x01) LED_PANEL_A_Set();
    if (row & 0x02) LED_PANEL_B_Set();
    if (row & 0x04) LED_PANEL_C_Set();
    if (row & 0x08) LED_PANEL_D_Set();
    if (row & 0x10) LED_PANEL_E_Set();
}

/* ============================================================================
 * DMA Interrupt Handler (BCM State Machine)
 * ============================================================================ */

/**
 * DMA_LED_ROW_Complete_cb - DMA completion callback for LED panel refresh
 *
 * Implements BCM state machine: advances through bitplanes (0-4) and rows (0-31).
 * For each bitplane × row combination:
 *   1. Set row address (A-E pins)
 *   2. Latch data into shift registers (LAT pulse)
 *   3. Enable output with BCM-controlled ON time (OE pulse)
 *   4. Queue next DMA transfer for following bitplane
 *
 * Static variables track current row/bit position across ISR invocations.
 * Full refresh cycle: 5 bitplanes × 32 rows × PANEL_WIDTH pixels.
 *
 * Context:
 *   event - DMA event type (block complete, error, etc.)
 *   context - Unused context pointer
 */
void DMA_LED_ROW_Complete_cb(DMA_TRANSFER_EVENT event, uintptr_t context)
{
    static uint8_t row = 0;         /* Current row pair (0-31) */
    static uint8_t bit = 0;         /* Current bitplane (0-4) */
    static uint32_t idx = 0;        /* Current index in DMA buffer */

    if (event == DMA_TRANSFER_EVENT_BLOCK_TRANSFER_COMPLETE) {
        /* Compute LED ON time for current bitplane (exponential BCM) */
        uint32_t on_time = bcm_time[bit] * BCM_BASE_TIME;

        /* Select row address (sets A-E pins) */
        set_row(row);

        /* Latch data into shift registers (LAT pulse: ~2 NOPs) */
        LED_PANEL_LAT_Set();
        delay_nops(2);
        LED_PANEL_LAT_Clear();

        /* Enable output for BCM duration, then disable for next bitplane */
        LED_PANEL_OE_Clear();
        delay_nops(on_time);
        LED_PANEL_OE_Set();

        /* Advance state machine: next row, wrap to next bitplane at row 31 */
        row++;
        if (row >= ROW_PAIRS) {
            row = 0;
            bit++;
            if (bit >= BCM_BITS) {
                bit = 0;  /* Full refresh cycle complete, start over */
            }
        }

        /* Compute buffer index for next DMA transfer */
            idx = (bit * ROW_PAIRS * PANEL_WIDTH) + (row * PANEL_WIDTH);

        /* Queue next row's data transfer */
        DMA_ChannelDisable(DMA_CHANNEL_2);
    DMA_ChannelTransfer(DMA_CHANNEL_2, &hub75_dma_buf[idx], LED_RGB_PORT_ADDR, PANEL_WIDTH);
    } else if (event == DMA_TRANSFER_EVENT_ERROR) {
        /* Error handling: log and optionally halt/restart */
    }
}

/* ============================================================================
 * Text Rendering - 5x7 Font and Helpers
 * ============================================================================ */

/**
 * font_5x7[][] - Minimal 5×7 bitmap font (8 glyphs)
 *
 * Subset containing: F(0) a(1) l(2) m(3) and P(4) a(5) l(6) m(7).
 * Each glyph is 7 bytes (rows), each byte is 5 bits (columns, MSB=leftmost).
 * Glyphs form labels "Fal m" (Palm) and "Palm" (Fist placeholder).
 *
 * Layout:
 *   Row 0 (top):    0b10000 = leftmost pixel set
 *   Row 1:          0b01000 = next pixel to right
 *   ...continuing for 7 rows total
 */
static const uint8_t font_5x7[][7] = {
    /* 0: 'F' */ {0b11110,0b10001,0b10001,0b11110,0b10000,0b10000,0b10000},
    /* 1: 'a' */ {0b01110,0b10001,0b10001,0b11111,0b10001,0b10001,0b10001},
    /* 2: 'l' */ {0b10000,0b10000,0b10000,0b10000,0b10000,0b10000,0b11111},
    /* 3: 'm' */ {0b10001,0b11011,0b10101,0b10001,0b10001,0b10001,0b10001},
    /* 4: 'P' */ {0b11111,0b10000,0b10000,0b11110,0b10000,0b10000,0b10000},
    /* 5: 'a' */ {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b11111},
    /* 6: 'l' */ {0b01111,0b10000,0b10000,0b01110,0b00001,0b00001,0b11110},
    /* 7: 'm' */ {0b11111,0b00100,0b00100,0b00100,0b00100,0b00100,0b00100}
};

/**
 * draw_char_5x7 - Render a single 5×7 character at (x0, y0)
 *
 * Draws character bitmap onto buffer_R/G/B with specified color.
 * Clamps to 64×64 panel bounds; ignores pixels outside.
 *
 * Parameters:
 *   x0, y0 - Top-left corner pixel position (0-63)
 *   glyph - Pointer to 7-byte character bitmap (from font_5x7[])
 *   r, g, b - 5-bit RGB color (0-31 each)
 */
static void draw_char_5x7(int x_offset, int y_offset, const uint8_t glyph[7], uint8_t r, uint8_t g, uint8_t b)
{
    for (int y = 0; y < 7; y++) {
        uint8_t row = glyph[y];
        for (int x = 0; x < 5; x++) {
            if (row & (1 << (4 - x))) {  /* Check bit from left (MSB=0) to right */
                int px = x_offset + x;
                int py = y_offset + y;
                if (px >= 0 && px < PANEL_WIDTH && py >= 0 && py < PANEL_HEIGHT) {
                    buffer_R[py][px] = r;
                    buffer_G[py][px] = g;
                    buffer_B[py][px] = b;
                }
            }
        }
    }
}

/**
 * draw_char_5x7_scaled - Render a 5×7 character at (x0, y0) with scaling
 *
 * Renders each pixel as a scale×scale block, allowing enlargement for visibility.
 * Identical to draw_char_5x7 but with per-pixel scaling.
 *
 * Parameters:
 *   x_offset, y_offset - Top-left corner (0-63)
 *   glyph - Character bitmap
 *   scale - Scale factor (1=1×1, 2=2×2, etc.)
 *   r, g, b - RGB color (5-bit each)
 */
static void draw_char_5x7_scaled(int x_offset, int y_offset, const uint8_t glyph[7], uint8_t scale, uint8_t r, uint8_t g, uint8_t b)
{
    for (int gy = 0; gy < 7; gy++) {
        uint8_t row = glyph[gy];
        for (int gx = 0; gx < 5; gx++) {
            if (row & (1 << (4 - gx))) {
                /* Draw scale×scale block for this glyph pixel */
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        int px = x_offset + gx * scale + sx;
                        int py = y_offset + gy * scale + sy;
                        if (px >= 0 && px < PANEL_WIDTH && py >= 0 && py < PANEL_HEIGHT) {
                            buffer_R[py][px] = r;
                            buffer_G[py][px] = g;
                            buffer_B[py][px] = b;
                        }
                    }
                }
            }
        }
    }
}

/**
 * draw_text_palm_overlay_scaled - Render "Palm" label with shadow effect
 *
 * Draws 4-character "Palm" text (glyphs P/a/l/m) at 2× scale with black
 * drop shadow (offset +1,+1) for visibility on colored backgrounds.
 * Color: red (r=31, g=0, b=0).
 */
void draw_text_palm_overlay_scaled(void)
{
    const uint8_t scale = 2;
    uint8_t r = 31, g = 0, b = 0;
    int x = 6, y = 8;

    /* Draw black shadow at (x+1, y+1) */
    draw_char_5x7_scaled(x + 1, y + 1, font_5x7[4], scale, 0, 0, 0);
    draw_char_5x7_scaled(x + 1 + 6 * scale, y + 1, font_5x7[5], scale, 0, 0, 0);
    draw_char_5x7_scaled(x + 1 + 12 * scale, y + 1, font_5x7[6], scale, 0, 0, 0);
    draw_char_5x7_scaled(x + 1 + 18 * scale, y + 1, font_5x7[7], scale, 0, 0, 0);

    /* Draw red text at (x, y) */
    draw_char_5x7_scaled(x + 0, y, font_5x7[4], scale, r, g, b);
    draw_char_5x7_scaled(x + 6 * scale, y, font_5x7[5], scale, r, g, b);
    draw_char_5x7_scaled(x + 12 * scale, y, font_5x7[6], scale, r, g, b);
    draw_char_5x7_scaled(x + 18 * scale, y, font_5x7[7], scale, r, g, b);
}

/**
 * draw_text_fist_overlay_scaled - Render "Fist" label with shadow effect
 *
 * Draws 4-character "Fist" text (glyphs F/a/l/m) at 2× scale with black
 * drop shadow (+1,+1) for contrast. Color: red (r=31, g=0, b=0).
 */
void draw_text_fist_overlay_scaled(void)
{
    const uint8_t scale = 2;
    uint8_t r = 31, g = 0, b = 0;
    int x = 6, y = 8;

    /* Draw black shadow at (x+1, y+1) */
    draw_char_5x7_scaled(x + 1, y + 1, font_5x7[0], scale, 0, 0, 0);
    draw_char_5x7_scaled(x + 1 + 6 * scale, y + 1, font_5x7[1], scale, 0, 0, 0);
    draw_char_5x7_scaled(x + 1 + 12 * scale, y + 1, font_5x7[2], scale, 0, 0, 0);
    draw_char_5x7_scaled(x + 1 + 18 * scale, y + 1, font_5x7[3], scale, 0, 0, 0);

    /* Draw red text at (x, y) */
    draw_char_5x7_scaled(x + 0, y, font_5x7[0], scale, r, g, b);
    draw_char_5x7_scaled(x + 6 * scale, y, font_5x7[1], scale, r, g, b);
    draw_char_5x7_scaled(x + 12 * scale, y, font_5x7[2], scale, r, g, b);
    draw_char_5x7_scaled(x + 18 * scale, y, font_5x7[3], scale, r, g, b);
}

/* ============================================================================
 * Public API - Initialization and Display Control
 * ============================================================================ */

/**
 * APP_Display_Initialize - Initialize HUB75 LED panel and DMA
 *
 * Registers DMA completion callback and primes the first DMA transfer
 * to start the BCM refresh state machine.
 */
void APP_Display_Initialize(void)
{
    /* Register DMA callback for LED panel BCM state machine */
    DMA_ChannelCallbackRegister(DMA_CHANNEL_2, DMA_LED_ROW_Complete_cb, 0);

    /* Prime the DMA state machine with first transfer */
    DMA_ChannelTransfer(DMA_CHANNEL_2, &hub75_dma_buf[0], LED_RGB_PORT_ADDR, PANEL_WIDTH);
}

/**
 * APP_Display_ShowOverlay - Render gesture label and refresh display
 *
 * Draws gesture classification text overlay ("Fist" or "Palm") and
 * converts RGB buffer to bitplane DMA data. Initiates panel refresh cycle.
 *
 * Parameters:
 *   ident - Gesture ID (0=Fist, 1=Palm, other=no label)
 */
void APP_Display_ShowOverlay(uint8_t ident)
{
    /* Draw gesture label overlay based on classification ID */
    if (ident == 0) {
        draw_text_fist_overlay_scaled();
    } else if (ident == 1) {
        draw_text_palm_overlay_scaled();
    }

    /* Convert RGB color buffers to HUB75 bitplane DMA buffer */
    rgb_to_hub75_dma(hub75_dma_buf);

    /* Initiate panel refresh (DMA state machine advances via ISR) */
    DMA_ChannelTransfer(DMA_CHANNEL_2, &hub75_dma_buf[0], LED_RGB_PORT_ADDR, PANEL_WIDTH);

    /* Ensure cache coherency for camera frame buffer (if applicable) */
    DCACHE_CLEAN_BY_ADDR((uint32_t *)panda_scaled_data, APP_Cam_GetFrameBytes());
}

/**
 * APP_Display_GetHub75Buf - Return pointer to HUB75 DMA ring buffer
 *
 * Exposes internal buffer for debug/inspection purposes.
 *
 * Returns:
 *   Pointer to hub75_dma_buf (bitplane-encoded DMA data)
 */
const uint8_t *APP_Display_GetHub75Buf(void)
{
    return hub75_dma_buf;
}
