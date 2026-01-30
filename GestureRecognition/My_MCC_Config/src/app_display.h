/*******************************************************************************
  MPLAB Harmony Application Header File

  Company:
    Microchip Technology Inc.

  File Name:
    app_display.h

  Summary:
    This header file provides prototypes and definitions for the application.

  Description:
    This header file provides function prototypes and data type definitions for
    the application.  Some of these are required by the system (such as the
    "APP_DISPLAY_Initialize" and "APP_DISPLAY_Tasks" prototypes) and some of them are only used
    internally by the application (such as the "APP_DISPLAY_STATES" definition).  Both
    are defined here for convenience.
*******************************************************************************/

#ifndef _APP_DISPLAY_H
#define _APP_DISPLAY_H

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
#include "definitions.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Macros and Definitions
// *****************************************************************************
// *****************************************************************************
// Panel geometry
#define PANEL_WIDTH     64
#define PANEL_HEIGHT    64
#define ROW_PAIRS       32

// BCM settings
#define BCM_BITS        5
#define BCM_BASE_TIME   120


/* Base address for HUB75 RGB output port (GPIO group 3, upper byte) */
#define LED_RGB_PORT_ADDR    ((const void *)((uint8_t *)&PORT_REGS->GROUP[3].PORT_OUT + 3))

/* DMA buffer size: BCM_BITS bitplanes × ROW_PAIRS × PANEL_WIDTH pixels */
/* Use PANEL_WIDTH/PANEL_HEIGHT/ROW_PAIRS as defined in app_cam.h */
#define DMA_BUF_SIZE (BCM_BITS * ROW_PAIRS * PANEL_WIDTH)

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
    APP_DISPLAY_STATE_INIT=0,
    APP_DISPLAY_STATE_SERVICE_TASKS,
    /* TODO: Define states used by the application state machine. */

} APP_DISPLAY_STATES;


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
    APP_DISPLAY_STATES state;

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
    uint8_t hub75_dma_buf[DMA_BUF_SIZE];


} APP_DISPLAY_DATA;

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
    void APP_DISPLAY_Initialize ( void )

  Summary:
     MPLAB Harmony application initialization routine.

  Description:
    This function initializes the Harmony application.  It places the
    application in its initial state and prepares it to run so that its
    APP_DISPLAY_Tasks function can be called.

  Precondition:
    All other system initialization routines should be called before calling
    this routine (in "SYS_Initialize").

  Parameters:
    None.

  Returns:
    None.

  Example:
    <code>
    APP_DISPLAY_Initialize();
    </code>

  Remarks:
    This routine must be called from the SYS_Initialize function.
*/

void APP_DISPLAY_Initialize ( void );


/*******************************************************************************
  Function:
    void APP_DISPLAY_Tasks ( void )

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
    APP_DISPLAY_Tasks();
    </code>

  Remarks:
    This routine must be called from SYS_Tasks() routine.
 */

void APP_DISPLAY_Tasks( void );

//DOM-IGNORE-BEGIN
#ifdef __cplusplus
}
#endif
//DOM-IGNORE-END

#endif /* _APP_DISPLAY_H */

/*******************************************************************************
 End of File
 */

