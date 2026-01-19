/*******************************************************************************
  Data Type definition of Timer/Counter(TCC) PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_tcc9.h

  Summary:
    Data Type definition of the TCC Peripheral Interface Plib.

  Description:
    This file defines the Data Types for the TCC Plib.

  Remarks:
    None.

*******************************************************************************/

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

#ifndef PLIB_TCC9_H
#define PLIB_TCC9_H

#include "device.h"
#include "plib_tcc_common.h"

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    extern "C" {

#endif
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Data Types
// *****************************************************************************
// *****************************************************************************
/*  The following data type definitions are used by the functions in this
    interface and should be considered part it.
*/

/* Total number of TCC channels in a module */
#define TCC9_NUM_CHANNELS    (6U)

/* TCC Channel numbers

   Summary:
    Identifies channel number within TCC module

   Description:
    This enumeration identifies TCC channel number.

   Remarks:
    None.
*/
typedef enum
{
    TCC9_CHANNEL0,
    TCC9_CHANNEL1,
    TCC9_CHANNEL2,
    TCC9_CHANNEL3,
    TCC9_CHANNEL4,
    TCC9_CHANNEL5,
}TCC9_CHANNEL_NUM;

// *****************************************************************************

/* TCC Channel interrupt status

   Summary:
    Identifies TCC PWM interrupt status flags

   Description:
    This enumeration identifies TCC PWM interrupt status falgs

   Remarks:
    None.
*/
typedef enum
{
    TCC9_PWM_STATUS_OVF = TCC_INTFLAG_OVF_Msk,
    TCC9_PWM_STATUS_FAULT_0 = TCC_INTFLAG_FAULT0_Msk,
    TCC9_PWM_STATUS_FAULT_1 = TCC_INTFLAG_FAULT1_Msk,
    TCC9_PWM_STATUS_MC_0 = TCC_INTFLAG_MC0_Msk,
    TCC9_PWM_STATUS_MC_1 = TCC_INTFLAG_MC1_Msk,
    TCC9_PWM_STATUS_MC_2 = TCC_INTFLAG_MC2_Msk,
    TCC9_PWM_STATUS_MC_3 = TCC_INTFLAG_MC3_Msk,
    TCC9_PWM_STATUS_MC_4 = TCC_INTFLAG_MC4_Msk,
    TCC9_PWM_STATUS_MC_5 = TCC_INTFLAG_MC5_Msk,
}TCC9_PWM_STATUS;

// *****************************************************************************
// *****************************************************************************
// Section: Interface Routines
// *****************************************************************************
// *****************************************************************************
/* The following functions make up the methods (set of possible operations) of
   this interface.
*/

// *****************************************************************************
void TCC9_PWMInitialize(void);

void TCC9_PWMStart(void);

void TCC9_PWMStop(void);

void TCC9_PWMDeadTimeSet(uint8_t deadtime_high, uint8_t deadtime_low);

void TCC9_PWMForceUpdate(void);

bool TCC9_PWMPatternSet(uint8_t pattern_enable, uint8_t pattern_output);


void TCC9_PWMPeriodInterruptEnable(void);

void TCC9_PWMPeriodInterruptDisable(void);

uint32_t TCC9_PWMInterruptStatusGet(void);

bool TCC9_PWM32bitPeriodSet(uint32_t period);

uint32_t TCC9_PWM32bitPeriodGet(void);

void TCC9_PWM32bitCounterSet(uint32_t countVal);

uint32_t TCC9_PWM32bitCounterGet(void);

__STATIC_INLINE bool TCC9_PWM32bitDutySet(TCC9_CHANNEL_NUM channel, uint32_t duty)
{
    bool status = false;
    if ((TCC9_REGS->TCC_STATUS & (1UL << (TCC_STATUS_CCBUFV0_Pos + (uint32_t)channel))) == 0U)
    {    
        TCC9_REGS->TCC_CCBUF[channel] = duty;
        status = true;
    }
    return status;
}
// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }
#endif
// DOM-IGNORE-END

#endif /* PLIB_TCC9_H */
