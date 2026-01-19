/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdio.h>
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include "peripheral/port/plib_port.h"


// Color data
#define PIN_R1   (1u << 0)
#define PIN_G1   (1u << 1)
#define PIN_B1   (1u << 2)
#define PIN_R2   (1u << 3)
#define PIN_G2   (1u << 4)
#define PIN_B2   (1u << 5)

// Control
#define PIN_CLK  (1u << 6)
#define PIN_LAT  (1u << 7)
#define PIN_OE   (1u << 8)

// Row address
#define PIN_A    (1u << 9)
#define PIN_B    (1u << 10)
#define PIN_C    (1u << 11)
#define PIN_D    (1u << 12)
#define PIN_E    (1u << 13)

#define BCM_BITS        3       // 3-bit BCM = 8 grayscale levels
#define PANEL_WIDTH     64
#define PANEL_HEIGHT    64
#define ROW_PAIRS       32       // HUB75 scans row pairs

#define HUB75_DATA_MASK  0x3F   // bits 0..5

#define CLK_HIGH()   GPIO_PA13_Set()
#define CLK_LOW()    GPIO_PA13_Clear()

#define LAT_HIGH()   LED_PANEL_LAT_Set()
#define LAT_LOW()    LED_PANEL_LAT_Clear()

#define OE_HIGH()    LED_PANEL_OE_Set()
#define OE_LOW()     LED_PANEL_OE_Clear()


// Simple 1-bit color buffer (Red, Green, Blue)
uint8_t buffer_R[PANEL_HEIGHT][PANEL_WIDTH];
uint8_t buffer_G[PANEL_HEIGHT][PANEL_WIDTH];
uint8_t buffer_B[PANEL_HEIGHT][PANEL_WIDTH];

#define DMA_BUF_SIZE (BCM_BITS * ROW_PAIRS * PANEL_WIDTH)

uint8_t hub75_dma_buf[DMA_BUF_SIZE];


/* Macro definitions */

#define IMG_WIDTH       160
#define IMG_HEIGHT      120
#define LINE_BYTES      (IMG_WIDTH*2)    // 2 byte per pixel, rgb565
#define FRAME_BYTES     (IMG_WIDTH * IMG_HEIGHT *2) 

#define DST_W 64
#define DST_H 64


volatile bool frame_ready = false;

volatile bool inference_complete = true;

volatile bool ml_input_ready = false;

volatile uint32_t line_index = 0;

extern uint8_t panda_scaled_data[38400];

uint8_t greyscale_img[64*64];   // output grayscale downscaled image

#define SRC_PORT_ADDR ((const void *)((uint8_t *)&PORT_REGS->GROUP[2].PORT_IN + 1)) // keep +1 if that reads correct bits
#define LED_RGB_PORT_ADDR    ((const void *)((uint8_t *)&PORT_REGS->GROUP[3].PORT_OUT + 3))

void DMA_EventHandler(DMA_TRANSFER_EVENT event, uintptr_t context)
{

    if (event == DMA_TRANSFER_EVENT_BLOCK_TRANSFER_COMPLETE) {
        frame_ready = true;

    } else if (event == DMA_TRANSFER_EVENT_ERROR) {
      // Handle error
    }
}



void HSYNC_ISR(void)
{

  line_index++;

}



void VSYNC_ISR(void)
{

    if (line_index >= (IMG_HEIGHT-5))
    {
      DMA_ChannelDisable(DMA_CHANNEL_1);        
      DMA_ChannelTransfer(DMA_CHANNEL_1, SRC_PORT_ADDR, &panda_scaled_data[0], FRAME_BYTES);

      line_index = 0;
      frame_ready = false;
    }
}

void I2C_Write(uint8_t reg_ad,uint8_t reg_value)
{
    uint8_t read_reg;
    uint8_t writeData[2];
    SYS_TIME_HANDLE delayHandle;

    writeData[0] = reg_ad;
    writeData[1] = reg_value;

    while(SERCOM5_I2C_IsBusy());
    SERCOM5_I2C_Write((0x21),writeData, 2);

    while(SERCOM5_I2C_IsBusy());

    for(int i=0;i<100000;i++);  // delay
    for(int i=0;i<100000;i++);  // delay
    for(int i=0;i<100000;i++);  // delay 
                
}


void I2C_Read(uint8_t reg_ad)
{
    uint8_t read_reg;

    while(SERCOM5_I2C_IsBusy());
    
    SERCOM5_I2C_Write((0x43 >> 1),&reg_ad, 1);
    while(SERCOM5_I2C_IsBusy());
    SERCOM5_I2C_Read((0x43 >> 1),&read_reg, 1);
    while(SERCOM5_I2C_IsBusy());
    printf("Read success : add - %x, val - %x\n\r",reg_ad,read_reg);           
}


void ov7670_set_rgb565_QQVGA_Working(void)
{
   SYS_TIME_HANDLE delayHandle;
  
   // Reset OV7670
  RST_OV_Clear();

  for(int i=0;i<100000;i++);  // delay
  for(int i=0;i<100000;i++);  // delay
  for(int i=0;i<100000;i++);  // delay  

  RST_OV_Set();


  // Reset
   I2C_Write(0x12, 0x80);

   I2C_Write(0x00, 0x0B); // AGC

  // Color settings, recommended defaults
  I2C_Write(0x11, 0x80);   // CLKRC: Prescaler /1

  I2C_Write(0x12, 0x14);   // COM7: RGB + QVGA scaling enabled

   // Auto-exposure/white balance default
  I2C_Write(0x13, 0x81);   // COM8: AEC ON

  I2C_Write(0x8C, 0x00);   // RGB444: Off

  I2C_Write(0x3A, 0x08);   // TSLB: Set correct RGB order
  I2C_Write(0x3D, 0x88);   // TSLB: Set correct RGB order


  I2C_Write(0xA2, 0x02);   // PCLK automatic


  // RGB565 format
  I2C_Write(0x40, 0xD0);   // COM15: Full range, RGB565

  // ----- SYNC & SIGNAL POLARITY -----
  I2C_Write(0x15, 0x20);     // COM10: PCLK only at active href

  // No manual scaling
  I2C_Write(0x0C, 0x0C);   // COM3: No DCW scaling here
  I2C_Write(0x3E, 0x12);  // COM14: Normal PCLK

  I2C_Write(0x09, 0x01); // output drive capability


  // QQVGA SCALING (160x120)
  I2C_Write(0x70, 0x3F);   // SCALING_XSC
  I2C_Write(0x71, 0x35);   // SCALING_YSC
  I2C_Write(0x72, 0x21);   // SCALING_DCWCTR: Downsample 4x
  I2C_Write(0x73, 0xF0);   // SCALING_PCLK_DIV: Divide pixel clock

  // Windowing (centering the image)
  I2C_Write(0x17, 0x16);   // HSTART
  I2C_Write(0x18, 0x04);   // HSTOP
  I2C_Write(0x32, 0xA4);   // HREF
  I2C_Write(0x19, 0x02);   // VSTART
  I2C_Write(0x1A, 0x7A);   // VSTOP
  I2C_Write(0x03, 0x0A);   // VREF

  // COLOR MATRIX (CRITICAL)
  I2C_Write(0x4F, 0x83);
  I2C_Write(0x50, 0x30);
  I2C_Write(0x51, 0x83);
  //  I2C_Write(0x52, 0x22);
  //  I2C_Write(0x53, 0x50);
  //  I2C_Write(0x54, 0x40);
  //  I2C_Write(0x56, 0x80);

  I2C_Write(0xB0, 0x0F);   // undocumented “magic” – fixes bad colors on many OV7670 boards


  SYS_TIME_DelayMS(50, &delayHandle);  // delay 5ms

   while(!SYS_TIME_DelayIsComplete(delayHandle));


}


static inline uint8_t hub75_pixel_to_gpio(
    uint8_t g_top,
    uint8_t g_bot,
    uint8_t bit)
{
    uint8_t out = 0;
    uint8_t mask = (1u << bit);

    if (g_top & mask) {
        out |= PIN_R1 | PIN_G1 | PIN_B1;
    }

    if (g_bot & mask) {
        out |= PIN_R2 | PIN_G2 | PIN_B2;
    }

    return out;
}


void greyscale_to_hub75_dma(
    const uint8_t *greyscale_img,
    uint8_t *dma_buf)
{
    uint32_t idx = 0;

    for (uint8_t bit = 0; bit < BCM_BITS; bit++) {
        for (uint8_t row = 0; row < ROW_PAIRS; row++) {

            uint32_t top_row = row * PANEL_WIDTH;
            uint32_t bot_row = (row + 32) * PANEL_WIDTH;

            for (uint8_t x = 0; x < PANEL_WIDTH; x++) {

                /* Reduce 8-bit grayscale to 3-bit */
                uint8_t g_top = greyscale_img[top_row + x] >> 5;
                uint8_t g_bot = greyscale_img[bot_row + x] >> 5;

                dma_buf[idx++] =
                    hub75_pixel_to_gpio(g_top, g_bot, bit);
            }
        }
    }
}



static inline void delay_nops(uint32_t n)
{
    while (n--) {
        __asm volatile("nop");
    }
}


static inline void set_row(uint8_t row)
{
    LED_PANEL_A_Clear();
    LED_PANEL_B_Clear();
    LED_PANEL_C_Clear();
    LED_PANEL_D_Clear();
    LED_PANEL_E_Clear();         

    if (row & 0x01) LED_PANEL_A_Set();
    if (row & 0x02) LED_PANEL_B_Set();
    if (row & 0x04) LED_PANEL_C_Set();
    if (row & 0x08) LED_PANEL_D_Set();
    if (row & 0x10) LED_PANEL_E_Set();
}


void rgb565_to_gray64(void)
{
    // fixed-point scaling (1000x)
    const int x_scale = (IMG_WIDTH * 1000) / DST_W; // 160000/64 = 2500
    const int y_scale = (IMG_HEIGHT * 1000) / DST_H; // 120000/64 = 1875

    if(inference_complete == false)
    {
        return;
    }
    for (int dy = 0; dy < DST_H; dy++)
    {
        int sy = (dy * y_scale) / 1000;

        for (int dx = 0; dx < DST_W; dx++)
        {
            int sx = (dx * x_scale) / 1000;

            // pixel index
            int src_index = (sy * IMG_WIDTH + sx) * 2;

            uint16_t p = panda_scaled_data[src_index] | (panda_scaled_data[src_index + 1] << 8);

            // extract R,G,B
            uint8_t r = (p >> 11) & 0x1F;
            uint8_t g = (p >> 5)  & 0x3F;
            uint8_t b = (p      ) & 0x1F;

            // scale to 8-bit
            r <<= 3;
            g <<= 2;
            b <<= 3;

            // grayscale Y = 0.299R + 0.587G + 0.114B
            // MCU-optimized formula:
            uint8_t y = (r*30 + g*59 + b*11) / 100;

            greyscale_img[dy * DST_W + dx] = y;
            ml_input_ready = true;
        }
    }
}

void DMA_LED_ROW_Complete_cb(DMA_TRANSFER_EVENT event, uintptr_t context)
{
    static uint8_t row = 0;
    static uint8_t bit = 0;
    static uint32_t idx = 0;

    if (event == DMA_TRANSFER_EVENT_BLOCK_TRANSFER_COMPLETE) {

            uint32_t on_time = (1 << bit) * 200; 
            set_row(row);
            
            LED_PANEL_LAT_Set();
            delay_nops(2);
            LED_PANEL_LAT_Clear();

            // 3. Show for the calculated BCM duration
            LED_PANEL_OE_Clear();
            delay_nops(on_time);
            LED_PANEL_OE_Set();


            row++;            
            if (row >= ROW_PAIRS) {
                row = 0;
                bit++;
                
                if (bit >= BCM_BITS) {
                    bit = 0;
                }
            }

            idx = (bit * ROW_PAIRS * PANEL_WIDTH) + (row * PANEL_WIDTH);

            DMA_ChannelDisable(DMA_CHANNEL_2);        
            DMA_ChannelTransfer(DMA_CHANNEL_2, &hub75_dma_buf[idx], LED_RGB_PORT_ADDR, PANEL_WIDTH);
    } 
    
    
    
    
    else if (event == DMA_TRANSFER_EVENT_ERROR) {
      // Handle error
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );

   // printf("Starting application...\r\n");

   
    TCC7_PWMStart();

    TCC1_PWMStart();


    ov7670_set_rgb565_QQVGA_Working();

    DMA_ChannelCallbackRegister(DMA_CHANNEL_1, DMA_EventHandler, 0);
    DMA_ChannelCallbackRegister(DMA_CHANNEL_2, DMA_LED_ROW_Complete_cb, 0);

    DMA_ChannelTransfer(DMA_CHANNEL_2, &hub75_dma_buf[0], LED_RGB_PORT_ADDR, PANEL_WIDTH);

    EIC_CallbackRegister(EIC_PIN_1, (EIC_CALLBACK)VSYNC_ISR, 0);
    EIC_CallbackRegister(EIC_PIN_2, (EIC_CALLBACK)HSYNC_ISR, 0);

   

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        

        if (frame_ready) {
            frame_ready = false;
            rgb565_to_gray64();

            greyscale_to_hub75_dma(greyscale_img, hub75_dma_buf);
            legato_showScreen(screenID_Screen0);
            // If sending via other DMA (to LCD), ensure caches are cleaned
            DCACHE_CLEAN_BY_ADDR((uint32_t *)panda_scaled_data, FRAME_BYTES);
        }
      
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

