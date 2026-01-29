// app_cam.h — camera module API
#ifndef APP_CAM_H_
#define APP_CAM_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Source frame dimensions (QQVGA RGB565)
#define IMG_WIDTH       160
#define IMG_HEIGHT      120
#define FRAME_BYTES     (IMG_WIDTH * IMG_HEIGHT * 2)

// Panel geometry
#define PANEL_WIDTH     64
#define PANEL_HEIGHT    64
#define ROW_PAIRS       32

// BCM settings
#define BCM_BITS        5
#define BCM_BASE_TIME   70

void APP_Cam_Initialize(void);

// Call from main loop; returns true if a frame was processed and buffers updated
bool APP_Cam_HandleFrame(void);

// Accessors
const uint8_t *APP_Cam_GetGreyscaleImg(void);
uint8_t APP_Cam_GetIdentifiedGesture(void);
void APP_Cam_SetIdentifiedGesture(uint8_t id);

// Expose frame buffer size for other modules
static inline uint32_t APP_Cam_GetFrameBytes(void) { return FRAME_BYTES; }

#ifdef __cplusplus
}
#endif

#endif // APP_CAM_H_
