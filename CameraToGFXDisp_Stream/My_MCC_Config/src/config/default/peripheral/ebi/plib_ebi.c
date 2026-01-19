/*******************************************************************
  Company:
    Microchip Technology Inc.
    Memory System Service EBI Initialization File

  File Name:
    plib_ebi.c

  Summary:
    External Bus Interface (EBI).
    This file contains the source code to initialize the EBI.

  Description:
    EBI-SMC configuration interface
    The EBI-SMC System Memory interface provides a simple interface to manage 8-bit and 16-bit
    parallel devices.

  *******************************************************************/

/*******************************************************************************
* Copyright (C) 2022 Microchip Technology Inc. and its subsidiaries.
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
#include "plib_ebi.h"
#include "device.h"

/* Function:
    void EBI_Initialize( void )

  Summary:
    Initializes hardware and data for the given instance of the EBI module.

  Description:
    This function initializes the SMC timings according to the external parralel device requirements.

  Returns:
  None.
 */

void EBI_Initialize( void )
{


    /* Chip Select CS0 Timings */
    /* Setup EBI SMC SETUP register */
    EBI_REGS->CS_X[0].EBI_SMC_SETUP = EBI_SMC_SETUP_NWE_SETUP(5) | EBI_SMC_SETUP_NCS_WR_SETUP(0) | EBI_SMC_SETUP_NRD_SETUP(16) | EBI_SMC_SETUP_NCS_RD_SETUP(16);

    /* Setup EBI SMC CYCLE register */
    EBI_REGS->CS_X[0].EBI_SMC_CYCLE = EBI_SMC_CYCLE_NWE_CYCLE(6) | EBI_SMC_CYCLE_NRD_CYCLE(3);

    /* Setup EBI SMC_PULSE register */
    EBI_REGS->CS_X[0].EBI_SMC_PULSE = EBI_SMC_PULSE_NWE_PULSE(2) | EBI_SMC_PULSE_NCS_WR_PULSE(6) | EBI_SMC_PULSE_NRD_PULSE(16) | EBI_SMC_PULSE_NCS_RD_PULSE(16);

    /* Setup EBI SMC MODE register */
    EBI_REGS->CS_X[0].EBI_SMC_MODE = EBI_SMC_MODE_EXNW_MODE(EBI_SMC_MODE_EXNW_MODE_DISABLED_Val)    \
                                          | EBI_SMC_MODE_WRITE_MODE(0) | EBI_SMC_MODE_READ_MODE(0) | EBI_SMC_MODE_DBW(EBI_SMC_MODE_DBW_16_BIT_Val) | EBI_SMC_MODE_BAT_BYTE_SELECT;




} /* EBI_Initialize */

/*******************************************************************************
 End of File
*/
