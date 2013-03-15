/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
	    this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

******************************************************************************/

/******************************************************************************
**  FILE        : ponOnuBoard.h                                              **
**                                                                           **
**  DESCRIPTION : This file contains ONU GPON board specific definitions     **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *
* =========================================================================== *
******************************************************************************/
#ifndef _ONU_PON_BOARD_H
#define _ONU_PON_BOARD_H

/* Include Files
------------------------------------------------------------------------------*/

/* Definitions
------------------------------------------------------------------------------*/
#define PON_NUM_DEVICES     (1)
#define PON_DEV_NAME        ("pon")
#define MVPON_IOCTL_MAGIC   ('P')

#define DB_88F65xx_XVR_SD (0x6)
#define RD_88F65xx_XVR_SD (0x5)

#define DB_88F65xx_DG     (0x3)
#define RD_88F65xx_DG     (0x4)

#define ONU_PON_TX_PATTERN_TYPE_T1        (0x1)
#define ONU_PON_TX_PATTERN_TYPE_T2        (0x2)
#define ONU_PON_TX_PATTERN_TYPE_PRBS_9    (0x80)
#define ONU_PON_TX_PATTERN_TYPE_PRBS_15   (0x82)
#define ONU_PON_TX_PATTERN_TYPE_PRBS_23   (0x83)

#define ONU_PON_XVR_POLARITY_ACT_HIGH     (0x1)
#define ONU_PON_XVR_POLARITY_ACT_LOW      (0x0)

#define ONU_XVR_GPIO_OUTPUT               (0x0)
#define ONU_XVR_GPIO_INPUT                (0x1)

#define ONU_PHY_OUTPUT                    (0x0)
#define ONU_PHY_INPUT                     (0x1)

#define ONU_ASIC_REV_Z2                   (0x1)
#define ONU_ASIC_REV_A0                   (0x2)

/* Enums
------------------------------------------------------------------------------*/
typedef enum
{
  ONU_PON_SYS_LED       = 0,
  ONU_PON_SYNC_LED      = 1
}E_OnuPonLedTypes;

typedef enum
{
  ACTIVE_LED_OFF        = 0,
  ACTIVE_LED_BLINK_SLOW = 1,
  ACTIVE_LED_BLINK_FAST = 2,
  ACTIVE_LED_ON         = 3
}E_OnuPonLedActivation;

/* Typedefs
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

#ifndef PON_FPGA
MV_STATUS onuPonLedInit(void);
MV_STATUS onuPonLedHandler(MV_U32 led, MV_U32 action);
MV_STATUS onuPonDyingGaspProcess(void);
MV_STATUS onuPonDyingGaspExit(void);
MV_STATUS mvOnuPonMacBurstEnableInit(void);
MV_STATUS mvOnuPonMacBurstEnablePolarityInit(MV_U32 polarity);
MV_STATUS onuPonPatternBurstOn(MV_U32 pattern, MV_BOOL isPeriodic, MV_U32 period, MV_U32 duration);
MV_STATUS onuPonPatternBurstOff(void);
void      onuPonPatternBurstTimerHndl(void);
MV_STATUS onuPonTxPowerOn(MV_BOOL txOn);
MV_STATUS onuPonTxPowerControlInit(void);
MV_STATUS onuPonTxLaserOn(MV_BOOL txOn);

#endif /* PON_FPGA */

/* Macros
------------------------------------------------------------------------------*/

#endif /* _ONU_PON_BOARD_H */



