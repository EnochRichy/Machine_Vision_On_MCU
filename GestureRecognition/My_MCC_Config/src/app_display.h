#ifndef APP_DISPLAY_H_
#define APP_DISPLAY_H_

#include <stdint.h>

// Initialize display subsystem (DMA channel registration, timers, etc.)
void APP_Display_Initialize(void);

// Draw overlay for identified gesture and start display DMA
// ident: 0 = fist, 1 = palm
void APP_Display_ShowOverlay(uint8_t ident);

// Optional: expose pointer to hub75 DMA buffer
const uint8_t *APP_Display_GetHub75Buf(void);

#endif // APP_DISPLAY_H_
