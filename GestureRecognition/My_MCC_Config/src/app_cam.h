/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_cam.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_CAM_Initialize" and "APP_CAM_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_CAM_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_CAM_H
#define _APP_CAM_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include "configuration.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Global Definitions and Macros
// *****************************************************************************
// *****************************************************************************

// Source frame dimensions (QQVGA RGB565)
#define IMG_WIDTH       160
#define IMG_HEIGHT      120
#define FRAME_BYTES     (IMG_WIDTH * IMG_HEIGHT * 2)

// *****************************************************************************
/* Application states

  Summary:
    Application states enumeration

  Description:
    This enumeration defines the valid application states.  These states
    determine the behavior of the application at various times.
*/

typedef enum
{
    /* Application's state machine's initial state. */
    APP_CAM_STATE_INIT=0,
    APP_CAM_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_CAM_STATES;


// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    Application strings and buffers are be defined outside this structure.
 */

typedef struct
{
    /* The application's current state */
    APP_CAM_STATES state;

    bool frame_ready;

    uint32_t line_index;

    bool processed_frame_data_ready;

} APP_CAM_DATA;

// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Routines
// *****************************************************************************
// *****************************************************************************
/* These routines are called by drivers when certain events occur.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_CAM_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_CAM_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CAM_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_CAM_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_CAM_Tasks ( void )

  Summary:
    MPLAB Harmony Demo application tasks function

  Description:
    This routine is the Harmony Demo application's tasks function.  It
    defines the application's state machine and core logic.

  Precondition:
    The system and application initialization ("SYS_Initialize") should be
    called before calling this.

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_CAM_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_CAM_Tasks( void );

// Accessors
const uint8_t *APP_Cam_GetGreyscaleImg(void);
uint8_t APP_Cam_GetIdentifiedGesture(void);
void APP_Cam_SetIdentifiedGesture(uint8_t id);

// Expose frame buffer size for other modules
static inline uint32_t APP_Cam_GetFrameBytes(void) { return FRAME_BYTES; }


//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_CAM_H */

/*******************************************************************************
 End of File
 */

