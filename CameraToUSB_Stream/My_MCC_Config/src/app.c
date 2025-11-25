/*******************************************************************************
  MPLAB Harmony Application Source File
  
  Company:
    Microchip Technology Inc.
  
  File Name:
    app.c

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

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files 
// *****************************************************************************
// *****************************************************************************

#include "app.h"


// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************
#if defined (__PIC32MZ__) || defined (__PIC32MX__) || defined(__PIC32CX1025SG41128__) || defined (_SAMD21J18A_H_) || defined (_SAME54P20A_H_) ||  defined (__PIC32MM__) ||  defined (_PIC32CM5164LE00100_H_) || defined (DRV_USBHS_DEVICE_SUPPORT) || defined (DRV_USBFS_DEVICE_SUPPORT)|| defined (__PIC32CK2051GC01144__) || defined (_PIC32CM5112GC00100_H_)
#define APP_EP_BULK_OUT 1
#define APP_EP_BULK_IN 1
#else
#define APP_EP_BULK_OUT 1
#define APP_EP_BULK_IN 2
#endif 
// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.
    
    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

/* Receive data buffer */
uint8_t receivedDataBuffer[512] CACHE_ALIGN;

/* Transmit data buffer */
uint8_t  transmitDataBuffer[512] CACHE_ALIGN;

extern volatile bool frame_ready;

extern uint8_t panda_scaled_data[38400];

#define FRAME_WIDTH          160
#define FRAME_HEIGHT         120
#define FRAME_BPP            2
#define FRAME_PAYLOAD_SIZE   (FRAME_WIDTH * FRAME_HEIGHT * FRAME_BPP)   // 38400
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

/*********************************************
 * Application USB Device Layer Event Handler
 *********************************************/

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

            appData.deviceIsConfigured = false;

            break;

        case USB_DEVICE_EVENT_CONFIGURED:

            /* Check the configuration */
            configurationValue = (uint8_t *)eventData;
            if(*configurationValue == 1 )
            {
                /* The device is in configured state. Update LED indication */
                LED0_Clear();

                /* Reset endpoint data send & receive flag  */
                appData.deviceIsConfigured = true;
            }
            break;

        case USB_DEVICE_EVENT_SUSPENDED:

			      LED0_Set();
            /* Device is suspended. */ 
            break;


        case USB_DEVICE_EVENT_POWER_DETECTED:

            /* VBUS is detected. Attach the device */
            USB_DEVICE_Attach(appData.usbDevHandle);
            break;

        case USB_DEVICE_EVENT_POWER_REMOVED:

            /* VBUS is removed. Detach the device */
            USB_DEVICE_Detach (appData.usbDevHandle);
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
                USB_DEVICE_ControlStatus(appData.usbDevHandle,USB_DEVICE_CONTROL_STATUS_OK);
            }
            else if(setupPacket->bRequest == USB_REQUEST_GET_INTERFACE)
            {
                /* We have only one alternate setting and this setting 0. So
                 * we send this information to the host. */

                USB_DEVICE_ControlSend(appData.usbDevHandle, &appData.altSetting, 1);
            }
            else
            {
                /* We have received a request that we cannot handle. Stall it*/
                USB_DEVICE_ControlStatus(appData.usbDevHandle, USB_DEVICE_CONTROL_STATUS_ERROR);
            }
            break;

        case USB_DEVICE_EVENT_ENDPOINT_READ_COMPLETE:
           /* Endpoint read is complete */
            appData.epDataReadPending = false;
            break;

        case USB_DEVICE_EVENT_ENDPOINT_WRITE_COMPLETE:
            /* Endpoint write is complete */
            appData.epDataWritePending = false;
            if (txPacketIndex < FRAME_NUM_PACKETS)
            {
                // Choose flag: MORE_DATA for all but last, DATA_COMPLETE for last
                USB_DEVICE_TRANSFER_FLAGS flags =
                    (txPacketIndex == (FRAME_NUM_PACKETS - 1)) ?
                    USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE :
                    USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING;

                USB_DEVICE_EndpointWrite(
                    appData.usbDevHandle,
                    &appData.writeTranferHandle,
                    appData.endpointTx,
                    &panda_scaled_data[txPacketIndex * FRAME_PACKET_SIZE],
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
            if(appData.deviceIsConfigured == true)
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
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{
    /* Place the App state machine in its initial state. */
    appData.state = APP_STATE_INIT;
    appData.usbDevHandle = USB_DEVICE_HANDLE_INVALID;
    appData.deviceIsConfigured = false;
    appData.endpointRx = (APP_EP_BULK_OUT | USB_EP_DIRECTION_OUT);
    appData.endpointTx = (APP_EP_BULK_IN | USB_EP_DIRECTION_IN);
    appData.epDataReadPending = false;
    appData.epDataWritePending = false;
    appData.altSetting = 0;
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks (void )
{
    switch(appData.state)
    {
        case APP_STATE_INIT:
            /* Open the device layer */
            appData.usbDevHandle = USB_DEVICE_Open( USB_DEVICE_INDEX_0,
                    DRV_IO_INTENT_READWRITE );

            if(appData.usbDevHandle != USB_DEVICE_HANDLE_INVALID)
            {
                /* Register a callback with device layer to get event notification (for end point 0) */
                USB_DEVICE_EventHandlerSet(appData.usbDevHandle,  APP_USBDeviceEventHandler, 0);

                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;
            }
            else
            {
                /* The Device Layer is not ready to be opened. We should try
                 * again later. */
            }

            break;

        case APP_STATE_WAIT_FOR_CONFIGURATION:

            /* Check if the device is configured */
            if(appData.deviceIsConfigured == true)
            {
                if (USB_DEVICE_ActiveSpeedGet(appData.usbDevHandle) == USB_SPEED_FULL)
                {
                    appData.endpointMaxPktSize = 64;
                }
                else if (USB_DEVICE_ActiveSpeedGet(appData.usbDevHandle) == USB_SPEED_HIGH)
                {
                    appData.endpointMaxPktSize = 512;
                }
                if (USB_DEVICE_EndpointIsEnabled(appData.usbDevHandle, appData.endpointRx) == false )
                {
                    /* Enable Read Endpoint */
                    USB_DEVICE_EndpointEnable(appData.usbDevHandle, 0, appData.endpointRx,
                            USB_TRANSFER_TYPE_BULK, appData.endpointMaxPktSize);
                }
                if (USB_DEVICE_EndpointIsEnabled(appData.usbDevHandle, appData.endpointTx) == false )
                {
                    /* Enable Write Endpoint */
                    USB_DEVICE_EndpointEnable(appData.usbDevHandle, 0, appData.endpointTx,
                            USB_TRANSFER_TYPE_BULK, appData.endpointMaxPktSize);
                }
                /* Indicate that we are waiting for read */
                appData.epDataReadPending = true;

                /* Place a new read request. */
                USB_DEVICE_EndpointRead(appData.usbDevHandle, &appData.readTranferHandle,
                        appData.endpointRx, &receivedDataBuffer[0], sizeof(receivedDataBuffer) );

                /* Device is ready to run the main task */
                appData.state = APP_STATE_MAIN_TASK;
            }
            break;

        case APP_STATE_MAIN_TASK:

            if(!appData.deviceIsConfigured)
            {
                /* This means the device got deconfigured. Change the
                 * application state back to waiting for configuration. */
                appData.state = APP_STATE_WAIT_FOR_CONFIGURATION;

                /* Disable the endpoint*/
                USB_DEVICE_EndpointDisable(appData.usbDevHandle, appData.endpointRx);
                USB_DEVICE_EndpointDisable(appData.usbDevHandle, appData.endpointTx);
                appData.epDataReadPending = false;
                appData.epDataWritePending = false;
            }
            // else if (appData.epDataReadPending == false)
            // {
            //     /* Look at the data the host sent, to see what kind of
            //      * application specific command it sent. */

            //     switch(receivedDataBuffer[0])
            //     {
            //         case 0x80:

            //             /* This is the toggle LED command */
            //             LED_Toggle();
            //             break;

            //         case 0x81:

            //             /* This is a switch check command. Check if the TX is free
            //              * for us to send a status. */

            //             if(appData.epDataWritePending == false)
            //             {
            //                 /* Echo back to the host PC the command we are fulfilling
            //                  * in the first byte.  In this case, the Get Pushbutton
            //                  * State command. */

            //                 transmitDataBuffer[0] = 0x81;

            //                 if(SWITCH_Get() == 0)
            //                 {
            //                     transmitDataBuffer[1] = 0x00;
            //                 }
            //                 else
            //                 {
            //                     transmitDataBuffer[1] = 0x01;
            //                 }

            //                 /* Fill dummy data pattern (512 bytes) */
            //                 for (int i = 0; i < sizeof(transmitDataBuffer); i++)
            //                 {
            //                     transmitDataBuffer[i] = (uint8_t)i;
            //                 }

            //                 /* Send the data to the host */

            //                 appData.epDataWritePending = true;

            //                 USB_DEVICE_EndpointWrite ( appData.usbDevHandle, &appData.writeTranferHandle,
            //                         appData.endpointTx, &transmitDataBuffer[0],
            //                         sizeof(transmitDataBuffer),
            //                         USB_DEVICE_TRANSFER_FLAGS_DATA_COMPLETE);
            //             }
            //             break;
            //         default:
            //             break;
            //     }

            //     appData.epDataReadPending = true ;

            //     /* Place a new read request. */
            //     USB_DEVICE_EndpointRead ( appData.usbDevHandle, &appData.readTranferHandle,
            //             appData.endpointRx, &receivedDataBuffer[0], sizeof(receivedDataBuffer) );
            // }

            else if(appData.epDataWritePending == false)
            {  


              if (frame_ready) {

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
                appData.epDataWritePending = true;

                USB_DEVICE_EndpointWrite ( appData.usbDevHandle, &appData.writeTranferHandle,
                        appData.endpointTx, frameHeader,
                        FRAME_HEADER_SIZE,
                        USB_DEVICE_TRANSFER_FLAGS_MORE_DATA_PENDING);
                }
            }
            break;

        case APP_STATE_ERROR:
            break;

        default:
            break;
    }
}
 

/*******************************************************************************
 End of File
 */