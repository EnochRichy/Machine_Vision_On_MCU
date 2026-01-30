/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app_usb.c

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

#include "app_usb.h"
#include "app_cam.h"

// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#define APP_EP_BULK_OUT 1
#define APP_EP_BULK_IN 1
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_USB_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_USB_DATA app_usbData;

/* Receive data buffer */
uint8_t receivedDataBuffer[512] CACHE_ALIGN;

/* Transmit data buffer */
uint8_t  transmitDataBuffer[512] CACHE_ALIGN;

extern APP_CAM_DATA app_camData;

// greyscale image is provided by the camera module; use APP_Cam_GetGreyscaleImg()

#define FRAME_WIDTH          64
#define FRAME_HEIGHT         64
#define FRAME_BPP            1
#define FRAME_PAYLOAD_SIZE   (FRAME_WIDTH * FRAME_HEIGHT * FRAME_BPP)   // 4096
#define FRAME_HEADER_SIZE    8
#define FRAME_PACKET_SIZE    512
#define FRAME_NUM_PACKETS    (FRAME_PAYLOAD_SIZE / FRAME_PACKET_SIZE)   // 75

static uint8_t  frameHeader[FRAME_HEADER_SIZE];
static uint32_t frameCounter = 0;
static uint32_t txPacketIndex = 0;
static bool     streamingInProgress = false;


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

void APP_USBDeviceEventHandler(USB_DEVICE_EVENT event, void * eventData, uintptr_t context)
{
    uint8_t * configurationValue;
    USB_SETUP_PACKET * setupPacket;
    switch(event)
    {
        case USB_DEVICE_EVENT_RESET:
        case USB_DEVICE_EVENT_DECONFIGURED:

            /* Device is reset or deconfigured. Provide LED indication.*/
            LED0_Set();

            app_usbData.deviceIsConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration */
            configurationValue = (uint8_t *)eventData;
            if(*configurationValue == 1 )
            {
                /* The device is in configured state. Update LED indication */
                LED0_Clear();

                /* Reset endpoint data send & receive flag  */
                app_usbData.deviceIsConfigured = true;
            }
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

			      LED0_Set();
            /* Device is suspended. */ 
            break;


        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS is detected. Attach the device */
            USB_DEVICE_Attach(app_usbData.usbDevHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is removed. Detach the device */
            USB_DEVICE_Detach (app_usbData.usbDevHandle);
            LED0_Clear();
            break;

        case USB_DEVICE_EVENT_CONTROL_TRANSFER_SETUP_REQUEST:
            /* This means we have received a setup packet */
            setupPacket = (USB_SETUP_PACKET *)eventData;
            if(setupPacket->bRequest == USB_REQUEST_SET_INTERFACE)
            {
                /* If we have got the SET_INTERFACE request, we just acknowledge
                 for now. This demo has only one alternate setting which is already
                 active. */
                USB_DEVICE_ControlStatus(app_usbData.usbDevHandle,USB_DEVICE_CONTROL_STATUS_OK);
            }
            else if(setupPacket->bRequest == USB_REQUEST_GET_INTERFACE)
            {
                /* We have only one alternate setting and this setting 0. So
                 * we send this information to the host. */

                USB_DEVICE_ControlSend(app_usbData.usbDevHandle, &app_usbData.altSetting, 1);
            }
            else
            {
                /* We have received a request that we cannot handle. Stall it*/
                USB_DEVICE_ControlStatus(app_usbData.usbDevHandle, USB_DEVICE_CONTROL_STATUS_ERROR);
            }
            break;

        case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
           /* Endpoint read is complete */
            app_usbData.epDataReadPending = false;
            break;

        case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
            /* Endpoint write is complete */
            app_usbData.epDataWritePending = false;
            if (txPacketIndex < FRAME_NUM_PACKETS)
            {
                // Choose flag: MORE_DATA for all but last, DATA_COMPLETE for last
                USB_DEVICE_TRANSFER_FLAGS flags =
                    (txPacketIndex == (FRAME_NUM_PACKETS - 1)) ?
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE :
                    USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING;

                USB_DEVICE_EndpointWrite(
                    app_usbData.usbDevHandle,
                    &app_usbData.writeTranferHandle,
                    app_usbData.endpointTx,
                    (void *)(APP_Cam_GetGreyscaleImg() + (txPacketIndex * FRAME_PACKET_SIZE)),
                    FRAME_PACKET_SIZE,
                    flags
                );

                txPacketIndex++;
            }
            else
            {
                // Finished sending this frame
                streamingInProgress        = false;
                frameCounter++;          // for debug / PC sync
            }
                
            break;

        /* These events are not used in this demo. */
        case USB_DEVICE_EVENT_RESUMED:
            if(app_usbData.deviceIsConfigured == true)
            {
                LED0_Clear();
            }
            break;
        case USB_DEVICE_EVENT_ERROR:
        default:
            break;
    }
}


// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_USB_Initialize ( void )

  Remarks:
    See prototype in app_usb.h.
 */

void APP_USB_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    app_usbData.state = APP_USB_STATE_INIT;
    app_usbData.usbDevHandle = USB_DEVICE_HANDLE_INVALID;
    app_usbData.deviceIsConfigured = false;
    app_usbData.endpointRx = (APP_EP_BULK_OUT | USB_EP_DIRECTION_OUT);
    app_usbData.endpointTx = (APP_EP_BULK_IN | USB_EP_DIRECTION_IN);
    app_usbData.epDataReadPending = false;
    app_usbData.epDataWritePending = false;
    app_usbData.altSetting = 0;
    /* TODO: Initialize your application's state machine and other
     * parameters.
     */
}


/******************************************************************************
  Function:
    void APP_USB_Tasks ( void )

  Remarks:
    See prototype in app_usb.h.
 */

void APP_USB_Tasks (void )
{
    switch(app_usbData.state)
    {
        case APP_USB_STATE_INIT:
            /* Open the device layer */
            app_usbData.usbDevHandle = USB_DEVICE_Open( USB_DEVICE_INDEX_0,
                    DRV_IO_INTENT_READWRITE );

            if(app_usbData.usbDevHandle != USB_DEVICE_HANDLE_INVALID)
            {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(app_usbData.usbDevHandle,  APP_USBDeviceEventHandler, 0);

                app_usbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;
            }
            else
            {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_USB_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device is configured */
            if(app_usbData.deviceIsConfigured == true)
            {
                if (USB_DEVICE_ActiveSpeedGet(app_usbData.usbDevHandle) == USB_SPEED_FULL)
                {
                    app_usbData.endpointMaxPktSize = 64;
                }
                else if (USB_DEVICE_ActiveSpeedGet(app_usbData.usbDevHandle) == USB_SPEED_HIGH)
                {
                    app_usbData.endpointMaxPktSize = 512;
                }
                if (USB_DEVICE_EndpointIsEnabled(app_usbData.usbDevHandle, app_usbData.endpointRx) == false )
                {
                    /* Enable Read Endpoint */
                    USB_DEVICE_EndpointEnable(app_usbData.usbDevHandle, 0, app_usbData.endpointRx,
                            USB_TRANSFER_TYPE_BULK, app_usbData.endpointMaxPktSize);
                }
                if (USB_DEVICE_EndpointIsEnabled(app_usbData.usbDevHandle, app_usbData.endpointTx) == false )
                {
                    /* Enable Write Endpoint */
                    USB_DEVICE_EndpointEnable(app_usbData.usbDevHandle, 0, app_usbData.endpointTx,
                            USB_TRANSFER_TYPE_BULK, app_usbData.endpointMaxPktSize);
                }
                /* Indicate that we are waiting for read */
                app_usbData.epDataReadPending = true;

                /* Place a new read request. */
                USB_DEVICE_EndpointRead(app_usbData.usbDevHandle, &app_usbData.readTranferHandle,
                        app_usbData.endpointRx, &receivedDataBuffer[0], sizeof(receivedDataBuffer) );

                /* Device is ready to run the main task */
                app_usbData.state = APP_USB_STATE_MAIN_TASK;
            }
            break;

        case APP_USB_STATE_MAIN_TASK:

            if(!app_usbData.deviceIsConfigured)
            {
                /* This means the device got deconfigured. Change the
                 * application state back to waiting for configuration. */
                app_usbData.state = APP_USB_STATE_WAIT_FOR_CONFIGURATION;

                /* Disable the endpoint*/
                USB_DEVICE_EndpointDisable(app_usbData.usbDevHandle, app_usbData.endpointRx);
                USB_DEVICE_EndpointDisable(app_usbData.usbDevHandle, app_usbData.endpointTx);
                app_usbData.epDataReadPending = false;
                app_usbData.epDataWritePending = false;
            }

            else if(app_usbData.epDataWritePending == false)
            {  


              if (app_camData.frame_ready) {

                // Build header: marker + frame counter (little endian)
                frameHeader[0] = 0xAA;
                frameHeader[1] = 0x55;
                frameHeader[2] = 0xAA;
                frameHeader[3] = 0x55;

                uint32_t cnt = frameCounter;
                frameHeader[4] = (uint8_t)(cnt & 0xFF);
                frameHeader[5] = (uint8_t)((cnt >> 8) & 0xFF);
                frameHeader[6] = (uint8_t)((cnt >> 16) & 0xFF);
                frameHeader[7] = (uint8_t)((cnt >> 24) & 0xFF);

                streamingInProgress        = true;
                txPacketIndex              = 0;            // next: payload packet 0
                app_usbData.epDataWritePending = true;

                USB_DEVICE_EndpointWrite ( app_usbData.usbDevHandle, &app_usbData.writeTranferHandle,
                        app_usbData.endpointTx, frameHeader,
                        FRAME_HEADER_SIZE,
                        USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING);
                }
            }
            break;

        case APP_USB_STATE_ERROR:
            break;

        default:
            break;
    }
}


/*******************************************************************************
 End of File
 */
