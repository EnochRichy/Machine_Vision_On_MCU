/*******************************************************************************
  DMA Peripheral Library Interface Header File

  Company:
    Microchip Technology Inc.

  File Name:
    plib_dma.h

  Summary:
    DMA peripheral library interface.

  Description:
    This file defines the interface to the DMA peripheral library. This
    library provides access to and control of the DMA controller.

  Remarks:
    None.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2021 Microchip Technology Inc. and its subsidiaries.
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

#ifndef PLIB_DMA_H    // Guards against multiple inclusion
#define PLIB_DMA_H

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

/*  This section lists the other files that are included in this file.
*/
#include <device.h>
#include <string.h>
#include <stdbool.h>

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

// *****************************************************************************

typedef enum
{
    /* DMA Channel 0 */
    DMA_CHANNEL_0 = 0,
    /* DMA Channel 1 */
    DMA_CHANNEL_1 = 1,
    /* DMA Channel 2 */
    DMA_CHANNEL_2 = 2,
} DMA_CHANNEL;

typedef uint32_t DMA_TRANSFER_EVENT;

/* Block was transferred successfully. */
#define DMA_TRANSFER_EVENT_BLOCK_TRANSFER_COMPLETE          (0x01U)

/* Error while processing the request */
#define DMA_TRANSFER_EVENT_ERROR                            (0x02U)

/* An start trigger event has been detected and the block transfer has started */
#define DMA_TRANSFER_EVENT_START_DETECTED                   (0x04U)

/* An abort trigger event has been detected and the DMA transfer has been aborted */
#define DMA_TRANSFER_EVENT_TRANSFER_ABORTED                 (0x08U)

/* A cell transfer has been completed (CSZ bytes has been transferred) */
#define DMA_TRANSFER_EVENT_CELL_TRANSFER_COMPLETE           (0x10U)

/* A half block transfer has been completed */
#define DMA_TRANSFER_EVENT_HALF_BLOCK_TRANSFER_COMPLETE     (0x20U)

/* A link list done event has been completed */
#define DMA_TRANSFER_EVENT_LINKED_LIST_TRANSFER_COMPLETE    (0x40U)

typedef uint32_t DMA_INT;

#define DMA_INT_START_DETECTED                              (DMA_CHINTF_SD_Msk)
#define DMA_INT_TRANSFER_ABORT                              (DMA_CHINTF_TA_Msk)
#define DMA_INT_CELL_TRANSFER_COMPLETE                      (DMA_CHINTF_CC_Msk)
#define DMA_INT_BLOCK_TRANSFER_COMPLETE                     (DMA_CHINTF_BC_Msk)
#define DMA_INT_BLOCK_HALF_TRANSFER_COMPLETE                (DMA_CHINTF_BH_Msk)
#define DMA_INT_LINKED_LIST_DONE                            (DMA_CHINTF_LL_Msk)
#define DMA_INT_WRITE_ERROR                                 (DMA_CHINTF_WRE_Msk)
#define DMA_INT_READ_ERROR                                  (DMA_CHINTF_RDE_Msk)

typedef enum
{
    /* Length of pattern match is 1 byte */
    DMA_PATTERN_MATCH_LEN_1BYTE,

    /* Length of pattern match is 2 bytes */
    DMA_PATTERN_MATCH_LEN_2BYTE,
}DMA_PATTERN_MATCH_LEN;

typedef enum
{
    /* Normal (0x8005) CRC-16/CRC-16-IBM/CRC-16-ANSI */
    DMA_CRC_MODE_CRC_16 = 0x0,

    /* Normal (0x1021) CRC-16-CCITT */
    DMA_CRC_MODE_CRC_16_CCITT = 0x1,

    /* CRC-16 based on polynomial provided in CRCPOLYA (POLYA[15:0]) register */
    DMA_CRC_MODE_16_BIT_CRCPOLYA = 0x2,

    /* CRC-16 based on polynomial provided in CRCPOLYB (POLYB[15:0]) register */
    DMA_CRC_MODE_16_BIT_CRCPOLYB = 0x3,

    /* Normal (0x04C11DB7) CRC-32 */
    DMA_CRC_MODE_CRC_32 = 0x4,

    /* CRC-32 based on polynomial provided in CRCPOLYA register */
    DMA_CRC_MODE_32_BIT_CRCPOLYA = 0x5,

    /* CRC-32 based on polynomial provided in CRCPOLYB register */
    DMA_CRC_MODE_32_BIT_CRCPOLYB = 0x6,

    /* Calculate IP header checksum */
    DMA_CRC_MODE_IP_HDR_CHECKSUM = 0x7

} DMA_CRC_MODE;

typedef struct
{
    /* CHCTRLCRC-CRCMD: CRC Mode */
    DMA_CRC_MODE crc_mode;

    /* CHCRCDAT: Initial Seed for calculating the CRC */
    uint32_t seed;

    /* Polynomial for CRCPOLYA or CRCPOLYB */
    uint32_t polynomial;

    /* CRC append mode */
    bool appendMode;

    /* CRC XOR mode */
    bool xorMode;

    /* CRC reflected output */
    bool reflectedOutput;

    /* CRC reflected input */
    bool reflectedInput;

} DMA_CRC_SETUP;

#if !(defined(__ASSEMBLER__) || defined(__IAR_SYSTEMS_ASM__))

typedef union
{
    struct
    {
        unsigned int CTRLB      :1;
        unsigned int EVCTRL     :1;
        unsigned int SSA        :1;
        unsigned int DSA        :1;
        unsigned int SSTRD      :1;
        unsigned int DSTRD      :1;
        unsigned int XSIZ       :1;
        unsigned int PDAT       :1;
        unsigned int CTRLCRC    :1;
        unsigned int CTRLDAT    :1;
        unsigned int            :14;
        unsigned int ENABLE     :1;
        unsigned int LLEN       :1;
        unsigned int SWFRC      :1;
        unsigned int RUNSTDBY   :1;
        unsigned int            :4;
    };
    struct
    {
        unsigned int w          :32;
    };
} DMA_BDCFGbits_t;

typedef union
{
  struct
    {
        unsigned int WAS        :3;
        unsigned int            :1;
        unsigned int RAS        :3;
        unsigned int            :1;
        unsigned int PRI        :2;
        unsigned int            :3;
        unsigned int WBOEN      :1;
        unsigned int BYTORD     :2;
        unsigned int TRIG       :8;
        unsigned int PIGNEN     :1;
        unsigned int PATLEN     :1;
        unsigned int PATEN      :1;
        unsigned int            :2;
        unsigned int CASTEN     :1;
        unsigned int            :1;
        unsigned int CRCEN      :1;
    };
    struct
    {
        unsigned int w          :32;
    };
} DMA_BDCTRLBbits_t;

/* MISRA C-2012 Rule 6.1 deviated:16 Deviation record ID -  H3_MISRAC_2012_R_6_1_DR_1 */

typedef union
{
    struct
    {
        uint16_t EVAUXACT   :2;
        uint16_t EVOMODE    :2;
        uint16_t            :1;
        uint16_t EVAUXIE    :1;
        uint16_t EVSTRIE    :1;
        uint16_t EVOE       :1;
        uint16_t            :8;
    };
    struct
    {
        uint16_t w          :16;
    };
} DMA_BDEVCTRLbits_t;

typedef union
{
    struct
    {
        uint16_t CRCMD      :3;
        uint16_t CRCAPP     :1;
        uint16_t            :1;
        uint16_t CRCXOR     :1;
        uint16_t CRCROUT    :1;
        uint16_t CRCRIN     :1;
        uint16_t            :8;
    };
    struct
    {
        uint16_t w          :16;
    };
} DMA_BDCTRLCRCbits_t;

/* MISRAC 2012 deviation block end */

typedef union
{
    struct
    {
        unsigned int CSZ        :10;
        unsigned int            :6;
        unsigned int BLKSZ      :16;
    };
    struct
    {
        unsigned int w          :32;
    };
} DMA_BDXSIZbits_t;

typedef union
{
    struct
    {
        unsigned int PDAT       :16;
        unsigned int            :8;
        unsigned int PIGN       :8;
    };
    struct
    {
        unsigned int w          :32;
    };
} DMA_BDPDATbits_t;

/** \brief DMA_DESCRIPTOR register API structure */
CACHE_ALIGN typedef struct
{  /* Direct Memory Access Controller */
    uint32_t                            DMA_BDNXT;
    DMA_BDCFGbits_t                     DMA_BDCFG;
    DMA_BDCTRLBbits_t                   DMA_BDCTRLB;
    DMA_BDEVCTRLbits_t                  DMA_BDEVCTRL;
    DMA_BDCTRLCRCbits_t                 DMA_BDCTRLCRC;
    uint32_t                            DMA_BDSSA;
    uint32_t                            DMA_BDDSA;
    uint16_t                            DMA_BDSSTRD;
    uint16_t                            DMA_BDDSTRD;
    DMA_BDXSIZbits_t                    DMA_BDXSIZ;
    DMA_BDPDATbits_t                    DMA_BDPDAT;
    uint32_t                            DMA_BDCRCDAT;
    uint8_t                             dummy_for_cache_align[CACHE_ALIGNED_SIZE_GET(40) - 40];
} DMA_DESCRIPTOR_REGS

#ifdef __GNUC__
  __attribute__ ((aligned (4)))
#endif
;
#endif

typedef uint32_t DMA_CHANNEL_CONFIG;

typedef void (*DMA_CHANNEL_CALLBACK) (DMA_TRANSFER_EVENT event, uintptr_t contextHandle);

void DMA_Initialize( void );
bool DMA_ChannelTransfer( DMA_CHANNEL channel, const void *srcAddr, const void *destAddr, size_t blockSize );
bool DMA_ChannelLinkedListTransfer (DMA_CHANNEL channel, DMA_DESCRIPTOR_REGS* channelDesc);
void DMA_ChannelCallbackRegister( DMA_CHANNEL channel, const DMA_CHANNEL_CALLBACK callback, const uintptr_t context );
bool DMA_ChannelIsBusy ( DMA_CHANNEL channel );
void DMA_ChannelDisable ( DMA_CHANNEL channel );
void DMA_ChannelEnable ( DMA_CHANNEL channel );
uint32_t DMA_ChannelGetTransferredCount( DMA_CHANNEL channel );
void DMA_ChannelInterruptEnable ( DMA_CHANNEL channel, DMA_INT intSources );
void DMA_ChannelInterruptDisable ( DMA_CHANNEL channel, DMA_INT intSources );
DMA_INT DMA_ChannelInterruptFlagsGet ( DMA_CHANNEL channel );
void DMA_ChannelPatternMatchSetup ( DMA_CHANNEL channel, DMA_PATTERN_MATCH_LEN patternLen, uint16_t matchData );
void DMA_ChannelPatternMatchEnable ( DMA_CHANNEL channel );
void DMA_ChannelPatternMatchDisable ( DMA_CHANNEL channel );
void DMA_ChannelPatternIgnoreByteEnable ( DMA_CHANNEL channel );
void DMA_ChannelPatternIgnoreByteDisable ( DMA_CHANNEL channel );
void DMA_ChannelPatternIgnoreValue ( DMA_CHANNEL channel, uint8_t ignoreValue );
DMA_CHANNEL_CONFIG DMA_ChannelSettingsGet (DMA_CHANNEL channel);
bool DMA_ChannelSettingsSet (DMA_CHANNEL channel, DMA_CHANNEL_CONFIG setting);
void DMA_ChannelCRCDisable(DMA_CHANNEL channel);
void DMA_ChannelCRCSetup(DMA_CHANNEL channel, DMA_CRC_SETUP *CRCSetup);
uint32_t DMA_ChannelCRCRead(DMA_CHANNEL channel);

// DOM-IGNORE-BEGIN
#ifdef __cplusplus  // Provide C++ Compatibility

    }

#endif
// DOM-IGNORE-END

#endif //PLIB_DMA_H
