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

/******************************************************************************/
/**                                                                          **/
/**  MODULE      : ONU GPON                                                  **/
/**                                                                          **/
/**  FILE        : eponOnuIsr.h                                              **/
/**                                                                          **/
/**  DESCRIPTION : This file contains ONU EPON Interrupt implementation      **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *
 * ========================================================================== *
 *
 ******************************************************************************/
#ifndef _ONU_EPON_ISR_H
#define _ONU_EPON_ISR_H

/* Include Files
------------------------------------------------------------------------------*/

/* Definitions
------------------------------------------------------------------------------*/
#define ONU_EPON_REGISTERED_LLID_0_MASK     (0x0001)
#define ONU_EPON_REGISTERED_LLID_0_7_MASK   (0x0002)
#define ONU_EPON_TIMESTAMP_DRIFT_MASK       (0x0004)
#define ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK (0x0008)
#define ONU_EPON_RX_CTRL_QUEUE_MASK         (0x0010)
#define ONU_EPON_RX_RPRT_QUEUE_MASK         (0x0020)
#define ONU_EPON_DS_SYNC_OK_PCS_MASK        (0x0080)
#define ONU_EPON_XVR_SD_MASK                (0x0100)
#define ONU_EPON_SERDES_SD_MASK             (0x0200)

#define ONU_EPON_EVENT_SHIFT                (16)

#ifdef PON_Z1

#define ONU_EPON_INTERRUPTS                 (ONU_EPON_RX_CTRL_QUEUE_MASK | \
                                             ONU_EPON_XVR_SD_MASK)

#else /* PON_Z2 */

#define ONU_EPON_INTERRUPTS                 (ONU_EPON_RX_CTRL_QUEUE_MASK  | \
                                             ONU_EPON_XVR_SD_MASK         | \
                                             ONU_EPON_DS_SYNC_OK_PCS_MASK | \
                                             ONU_EPON_REGISTERED_LLID_0_MASK | \
                                             ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK)
#endif /* PON_Z2 */

/* Alarms Mask */
#define ONU_EPON_ALARM_MASK                 (ONU_EPON_SERDES_SD_MASK | \
                                             ONU_EPON_XVR_SD_MASK    | \
                                             ONU_EPON_TIMESTAMP_DRIFT_MASK)
/* Message Types */
#define ONU_EPON_RX_CTRL_MSG                (1)
#define ONU_EPON_RX_RPRT_MSG                (2)

/* DBA Report Timer Types */
#define ONU_EPON_SW_DBA_RPRT_TIMER          (0)
#define ONU_EPON_HW_DBA_RPRT_TIMER          (1)


/* Enums
------------------------------------------------------------------------------*/

/* Typedefs
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/
void onuEponIsrLowRoutine(MV_U32 *interruptEvent, MV_U32 *interruptStatus);
void onuEponIsrRoutine(MV_U32 interruptEvent, MV_U32 interruptStatus);
#ifndef PON_FPGA
void onuEponDgIsrRoutine(void);
#endif /* PON_FPGA */
void onuEponIsrTimerMpcpHndl(unsigned long data);
void onuEponIsrTimerMpcpStateSet(MV_BOOL mode);
void onuEponIsrTimerMpcpIntervalSet(MV_U32 interval);
void onuEponIsrTimerMpcpRandomResetSet(MV_U32 checkThreshold, MV_U32 maskThreshold);
void onuEponIsrTimerHwReportHndl(unsigned long data);
void onuEponIsrTimerHwReportStateSet(MV_U32 enable, MV_U32 t0_val, MV_U32 t0_time, MV_U32 t1_val, MV_U32 t1_time);
void onuEponIsrTimerHoldoverHndl(unsigned long data);
void onuEponIsrTimerHoldoverIntervalSet(MV_U32 interval, MV_U32 state);
void onuEponDbaSwRprtMacTimerCfg(MV_U32 interval);
void onuEponIsrTimerEventMissHndl(unsigned long data);
void onuEponIsrTimerEventMissStateSet(MV_BOOL mode);
void onuEponIsrXvrResetTimerHndl(unsigned long data);
void onuEponIsrXvrResetStateSet(MV_BOOL mode);
void onuEponForceTxDownStateSet(MV_U32 txEnable, MV_U32 macId);
MV_BOOL onuEponForceTxDownStateGet(MV_U32 macId);

/* Macros
------------------------------------------------------------------------------*/

#endif /* _ONU_EPON_ISR_H */



