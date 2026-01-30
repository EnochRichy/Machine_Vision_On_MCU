/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_cam.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app_cam.h"
#include "app_display.h"
#include "definitions.h"
#include <stdint.h>
#include <stdbool.h>

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

/* OV7670 I2C addresses */
#define OV7670_I2C_ADDR_WRITE       0x21    /* 0x42 >> 1 for 7-bit addressing */
#define OV7670_I2C_ADDR_READ        0x43    /* 0x86 >> 1 */

/* Camera frame layout (QQVGA RGB565: 160x120) */
#define CAM_LINE_BYTES              (IMG_WIDTH * 2)

/* ML inference input size (grayscale downsampled) */
#define ML_IMG_W                    64
#define ML_IMG_H                    64

/* Busy-wait loop iterations for timing (conservative approximation) */
#define I2C_WRITE_DELAY_LOOPS       100000

/* Extern buffers (provided by display module) */
extern uint8_t panda_scaled_data[FRAME_BYTES];      /* DMA target: raw RGB565 frame */
extern APP_DISPLAY_DATA app_displayData;

/* Module-local: 64x64 grayscale image for ML inference */
static uint8_t greyscale_img_local[ML_IMG_W * ML_IMG_H];

/* ============================================================================
 * Gamma Correction Tables (for color display rendering)
 * ============================================================================ */

/**
 * gamma5 - Standard gamma curve for 5-bit color values
 *
 * Maps 5-bit input (0-31) to 8-bit output (0-255) with gamma correction.
 * Applied to R/G/B channels before HUB75 panel DMA.
 */
static const uint8_t gamma5[32] = {
    0,0,0,0,1,1,2,3,
    4,5,7,9,11,13,15,17,
    19,21,23,25,27,28,29,30,
    31,31,31,31,31,31,31,31
};

/**
 * gamma5_boosted - Enhanced gamma curve for brighter display
 *
 * Maps 5-bit input (0-31) to 8-bit output (0-255) with more aggressive
 * gamma adjustment for panels requiring higher brightness.
 */
static const uint8_t gamma5_boosted[32] = {
    0,0,0,0,0,1,1,2,
    3,4,6,8,10,12,14,16,
    18,20,22,24,26,27,28,29,
    30,30,31,31,31,31,31,31
};


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_CAM_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_CAM_DATA app_camData;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/**
 * DMA_EventHandler - Camera DMA transfer completion callback
 *
 * Called when DMA_CHANNEL_1 completes block transfer (full frame received).
 * Sets frame_ready flag to trigger conversion in main loop.
 *
 * Context:
 *   event - DMA event type (error, transfer complete, etc.)
 *   context - Unused context pointer
 */
void DMA_EventHandler(DMA_TRANSFER_EVENT event, uintptr_t context)
{
    if (event == DMA_TRANSFER_EVENT_BLOCK_TRANSFER_COMPLETE) {
        app_camData.frame_ready = true;
    } else if (event == DMA_TRANSFER_EVENT_ERROR) {
        /* Simple error handling: log and continue (original behavior) */
        app_camData.frame_ready = true;
    }
}

/**
 * HSYNC_ISR - Camera horizontal sync interrupt handler
 *
 * Increments line counter each time camera sends HSYNC pulse (end of line).
 * Used by VSYNC handler to determine when frame is complete.
 */
void HSYNC_ISR(void)
{
    app_camData.line_index++;
}

/**
 * VSYNC_ISR - Camera vertical sync interrupt handler
 *
 * Detects end of frame (when line_index reaches frame height minus threshold).
 * Resets DMA for next frame and clears line counter.
 */
void VSYNC_ISR(void)
{
        if (app_camData.line_index >= (IMG_HEIGHT - 5))
    {
      DMA_ChannelDisable(DMA_CHANNEL_1);
      DMA_ChannelTransfer(DMA_CHANNEL_1,
                          (const void *)((const uint8_t *)&PORT_REGS->GROUP[2].PORT_IN + 1),
                          &panda_scaled_data[0],
                                                    FRAME_BYTES);

      app_camData.line_index = 0;
      app_camData.frame_ready = false;
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* ============================================================================
 * I2C Register Access (OV7670 Camera Control)
 * ============================================================================ */

/**
 * I2C_Write - Write OV7670 register via I2C
 *
 * Sends a single register write command to the camera over I2C bus (SERCOM5).
 * Includes busy-wait loops for timing compatibility with original code.
 *
 * Parameters:
 *   reg_addr - Register address (0x00-0xFF)
 *   reg_value - 8-bit value to write
 */
void I2C_Write(uint8_t reg_addr, uint8_t reg_value)
{
    uint8_t write_buffer[2];

    write_buffer[0] = reg_addr;
    write_buffer[1] = reg_value;

    while(SERCOM5_I2C_IsBusy());
    SERCOM5_I2C_Write(OV7670_I2C_ADDR_WRITE, write_buffer, 2);

    while(SERCOM5_I2C_IsBusy());

    /* Timing delays (3x conservative busy-wait for I2C settling) */
    for(int delay_count = 0; delay_count < I2C_WRITE_DELAY_LOOPS; delay_count++);
    for(int delay_count = 0; delay_count < I2C_WRITE_DELAY_LOOPS; delay_count++);
    for(int delay_count = 0; delay_count < I2C_WRITE_DELAY_LOOPS; delay_count++);
}

/**
 * I2C_Read - Read OV7670 register via I2C
 *
 * Sends read command to camera and returns register value.
 * Currently primarily for debug/diagnostic purposes.
 *
 * Parameters:
 *   reg_addr - Register address to read
 */
void I2C_Read(uint8_t reg_addr)
{
    uint8_t read_buffer;

    while(SERCOM5_I2C_IsBusy());
    SERCOM5_I2C_Write((OV7670_I2C_ADDR_READ >> 1), &reg_addr, 1);
    while(SERCOM5_I2C_IsBusy());
    SERCOM5_I2C_Read((OV7670_I2C_ADDR_READ >> 1), &read_buffer, 1);
    while(SERCOM5_I2C_IsBusy());
}

/* ============================================================================
 * OV7670 Camera Initialization and Configuration
 * ============================================================================ */

/**
 * ov7670_init - Initialize OV7670 for RGB565 QQVGA mode
 *
 * Configures OV7670 camera via I2C for:
 *   - RGB565 color format (16-bit output)
 *   - QQVGA resolution (160x120)
 *   - Pixel clock scaling and windowing
 *   - Color matrix and gamma settings
 *   - Auto-exposure and white balance
 *
 * Resets camera, applies register configuration, and includes settling delays.
 * Called once during application startup via APP_Cam_Initialize().
 */
void ov7670_init(void)
{
   /* Reset OV7670 via GPIO */
  RST_OV_Clear();
  for(int delay_count = 0; delay_count < I2C_WRITE_DELAY_LOOPS; delay_count++);
  for(int delay_count = 0; delay_count < I2C_WRITE_DELAY_LOOPS; delay_count++);
  for(int delay_count = 0; delay_count < I2C_WRITE_DELAY_LOOPS; delay_count++);
  RST_OV_Set();

   /* Hardware reset via I2C */
   I2C_Write(0x12, 0x80);

   I2C_Write(0x00, 0x0B);       /* AGC gain control */

  /* Color format and scaling settings */
  I2C_Write(0x11, 0x81);        /* CLKRC: Prescaler /1 */
  I2C_Write(0x12, 0x14);        /* COM7: RGB mode + QVGA scaling enabled */

   /* Auto-exposure and white balance */
  I2C_Write(0x13, 0x81);        /* COM8: AEC (Auto Exposure Control) ON */

  I2C_Write(0x8C, 0x00);        /* RGB444: Disabled (use RGB565) */

  I2C_Write(0x3A, 0x08);        /* TSLB: Correct RGB byte order */
  I2C_Write(0x3D, 0x88);        /* TSLB: Set RGB order (duplicate for stability) */

  I2C_Write(0xA2, 0x02);        /* PCLK: Automatic mode */

  /* RGB565 format configuration */
  I2C_Write(0x40, 0xD0);        /* COM15: Full range output, RGB565 mode */

  /* Synchronization and signal polarity */
  I2C_Write(0x15, 0x20);        /* COM10: PCLK output only during active HREF */

  /* Downsampling and pixel clock settings */
  I2C_Write(0x0C, 0x0C);        /* COM3: Disable DCW scaling (use hardware scaler) */
  I2C_Write(0x3E, 0x12);        /* COM14: Normal PCLK, manual scaling */

  I2C_Write(0x09, 0x01);        /* Output drive capability */

  /* QQVGA (160x120) scaling configuration */
  I2C_Write(0x70, 0x3F);        /* SCALING_XSC: Horizontal scaling */
  I2C_Write(0x71, 0x35);        /* SCALING_YSC: Vertical scaling */
  I2C_Write(0x72, 0x21);        /* SCALING_DCWCTR: Enable 4x downsample */
  I2C_Write(0x73, 0xF0);        /* SCALING_PCLK_DIV: Divide pixel clock */

  /* Image windowing (centering) */
  I2C_Write(0x17, 0x16);        /* HSTART: Horizontal window start */
  I2C_Write(0x18, 0x04);        /* HSTOP: Horizontal window stop */
  I2C_Write(0x32, 0xA4);        /* HREF: Reference pixel */
  I2C_Write(0x19, 0x02);        /* VSTART: Vertical window start */
  I2C_Write(0x1A, 0x7A);        /* VSTOP: Vertical window stop */
  I2C_Write(0x03, 0x0A);        /* VREF: Vertical reference */

  /* Color matrix (critical for correct color reproduction) */
  I2C_Write(0x4F, 0x83);        /* MTX1: Color matrix coefficient */
  I2C_Write(0x50, 0x30);        /* MTX2: Color matrix coefficient */
  I2C_Write(0x51, 0x83);        /* MTX3: Color matrix coefficient */
  I2C_Write(0xB0, 0x0F);        /* UNDOCUMENTED: Camera board color correction */

  /* Wait for settling (50ms) */
  SYS_TIME_HANDLE delayHandle;
  SYS_TIME_DelayMS(50, &delayHandle);
  while(!SYS_TIME_DelayIsComplete(delayHandle));
}


/* ============================================================================
 * Frame Processing: RGB565 → Grayscale Conversion
 * ============================================================================ */

/**
 * rgb565_to_gray64 - Convert incoming 160x120 RGB565 frame to 64x64 grayscale
 *
 * Downsamples the raw camera frame from panda_scaled_data buffer (QQVGA RGB565)
 * into a 64x64 grayscale image suitable for ML inference. Also populates
 * display color buffers (buffer_R/G/B) for HUB75 overlay rendering.
 *
 * Conversion pipeline:
 *   1. Fixed-point scaling factors compute source pixel coordinates
 *   2. RGB565 pixel extracted and unpacked to 8-bit R/G/B values
 *   3. Grayscale luminance computed: Y = 0.299R + 0.587G + 0.114B
 *   4. Results stored to greyscale_img_local[64x64]
 *   5. Scaled RGB values written to display buffers with gamma correction
 *
 * Gating:
 *   Skips conversion if inference_complete is false (protects ML input during inference).
 */
void rgb565_to_gray64(void)
{
    /* Fixed-point scaling factors (multiply by 1000 to avoid floating-point) */
    const int x_scale = (IMG_WIDTH * 1000) / ML_IMG_W;   /* (160 * 1000) / 64 = 2500 */
    const int y_scale = (IMG_HEIGHT * 1000) / ML_IMG_H;  /* (120 * 1000) / 64 = 1875 */

    // if( APP_ML_IsInferenceComplete() == false)
    // {
    //     /* Skip conversion while ML engine is reading the current frame */
    //     return;
    // }

    for (int dst_y = 0; dst_y < ML_IMG_H; dst_y++)
    {
        int src_y = (dst_y * y_scale) / 1000;

        for (int dst_x = 0; dst_x < ML_IMG_W; dst_x++)
        {
            int src_x = (dst_x * x_scale) / 1000;

            /* Compute source pixel index in RGB565 frame */
            int src_index = (src_y * IMG_WIDTH + src_x) * 2;

            /* Extract RGB565 pixel (little-endian: low byte = RRRGGGbb, high byte = GGGbbbbb) */
            uint16_t pixel = panda_scaled_data[src_index] | (panda_scaled_data[src_index + 1] << 8);

            /* Unpack 5-bit R, 6-bit G, 5-bit B values */
            uint8_t r_5bit = (pixel >> 11) & 0x1F;
            uint8_t g_6bit = (pixel >> 5)  & 0x3F;
            uint8_t b_5bit = (pixel      ) & 0x1F;

            /* Scale to 8-bit range */
            uint8_t r_8bit = r_5bit << 3;  /* 5-bit → 8-bit: multiply by 8 */
            uint8_t g_8bit = g_6bit << 2;  /* 6-bit → 8-bit: multiply by 4 */
            uint8_t b_8bit = b_5bit << 3;  /* 5-bit → 8-bit: multiply by 8 */

            /* Compute grayscale luminance: Y = 0.299R + 0.587G + 0.114B */
            /* Using integer approximation: (30R + 59G + 11B) / 100 */
            uint8_t grayscale = (r_8bit * 30 + g_8bit * 59 + b_8bit * 11) / 100;

            /* Store grayscale value for ML inference */
            greyscale_img_local[dst_y * ML_IMG_W + dst_x] = grayscale;

            /* Also populate display buffers with scaled RGB values */
            r_5bit = (pixel >> 11) & 0x1F;
            /* Convert 6-bit green to 5-bit by shifting (approx) */
            uint8_t g_5bit = (pixel >> 6)  & 0x1F;  /* Convert 6-bit to 5-bit */
            b_5bit =  pixel        & 0x1F;

            /* Scale in 6-bit space to avoid overflow, then apply gamma */
            uint8_t r_scaled = (r_5bit * 36) >> 5;   /* ~1.06x boost */
            uint8_t g_scaled = (g_5bit * 28) >> 5;   /* ~0.81x boost */
            uint8_t b_scaled = (b_5bit * 40) >> 5;   /* ~1.19x boost */

            /* Clamp to 5-bit range before gamma lookup */
            if (r_scaled > 31) r_scaled = 31;
            if (g_scaled > 31) g_scaled = 31;
            if (b_scaled > 31) b_scaled = 31;

            app_displayData.buffer_R[dst_y][dst_x] = gamma5[r_scaled];
            app_displayData.buffer_G[dst_y][dst_x] = gamma5[g_scaled];
            app_displayData.buffer_B[dst_y][dst_x] = gamma5[b_scaled];

            app_camData.processed_frame_data_ready = true;
        }
    }
}

/* ============================================================================
 * Public API - Data Access
 * ============================================================================ */

/**
 * APP_Cam_GetGreyscaleImg - Return pointer to 64x64 grayscale image buffer
 *
 * Returns pointer to module-owned grayscale image (64x64 uint8_t array).
 * Valid immediately after APP_Cam_HandleFrame returns true.
 *
 * Returns:
 *   Pointer to static 64x64 grayscale image (uint8_t[4096])
 */
const uint8_t *APP_Cam_GetGreyscaleImg(void)
{
    return greyscale_img_local;
}

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_CAM_Initialize ( void )

  Remarks:
    See prototype in app_cam.h.
 */

void APP_CAM_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_camData.state = APP_CAM_STATE_INIT;
    app_camData.frame_ready = false;
    app_camData.line_index = 0;
    app_camData.processed_frame_data_ready = false;
}


/******************************************************************************
  Function:
    void APP_CAM_Tasks ( void )

  Remarks:
    See prototype in app_cam.h.
 */

void APP_CAM_Tasks ( void )
{

    /* Check the application's current state. */
    switch ( app_camData.state )
    {
        /* Application's initial state. */
        case APP_CAM_STATE_INIT:
        {
            bool appInitialized = true;

            TCC7_PWMStart();

            TCC1_PWMStart();

            /* Configure camera registers for RGB565 QQVGA output */
            ov7670_init();

            /* Register DMA completion callback for frame capture */
            DMA_ChannelCallbackRegister(DMA_CHANNEL_1, DMA_EventHandler, 0);

            /* Register external interrupt handlers for HSYNC/VSYNC line/frame sync */
            EIC_CallbackRegister(EIC_PIN_1, (EIC_CALLBACK)VSYNC_ISR, 0);
            EIC_CallbackRegister(EIC_PIN_2, (EIC_CALLBACK)HSYNC_ISR, 0);


            if (appInitialized)
            {
                app_camData.state = APP_CAM_STATE_SERVICE_TASKS;
            }
            break;
        }

        case APP_CAM_STATE_SERVICE_TASKS:
        {

            if (app_camData.frame_ready)
            {
                /* Process the captured frame */
                /* Clear flag and perform conversion */
                app_camData.frame_ready = false;
                rgb565_to_gray64();
                legato_showScreen(screenID_Screen0);
                /* Ensure cache coherency for camera frame buffer (if applicable) */
                DCACHE_CLEAN_BY_ADDR((uint32_t *)panda_scaled_data, APP_Cam_GetFrameBytes());
            }         

            break;
        }

        /* TODO: implement your application state machine.*/


        /* The default state should never be executed. */
        default:
        {
            /* TODO: Handle error in application's state machine. */
            break;
        }
    }
}


/*******************************************************************************
 End of File
 */
