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
/**  FILE        : eponOnuPm.h                                               **/
/**                                                                          **/
/**  DESCRIPTION : This file contains ONU EPON Alarm and Statistics          **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *                              
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *  
 * ========================================================================== *  
 *                                                                            
 ******************************************************************************/
#ifndef _ONU_EPON_PM_H
#define _ONU_EPON_PM_H

/* Definitions
------------------------------------------------------------------------------*/ 

/* Enums                              
------------------------------------------------------------------------------*/ 
typedef enum
{
  TOTAL_MPCP_TX_FRAME_CNT             = 0,
  TOTAL_MPCP_TX_ERROR_FRAME_CNT       = 1,
  MAX_EPON_TX_SW_CNT
}E_EponTxSwCnt;

typedef enum
{
  TOTAL_MPCP_RX_FRAME_CNT             = 0,
  TOTAL_MPCP_RX_ERROR_FRAME_CNT       = 1,
  TOTAL_MPCP_REGISTER_ACK_CNT         = 2,
  TOTAL_MPCP_REGISTER_NACK_CNT        = 3,
  TOTAL_MPCP_REGISTER_DEREG_FRAME_CNT = 4,
  TOTAL_MPCP_REGISTER_REREG_FRAME_CNT = 5,
  MAX_EPON_RX_SW_CNT
}E_EponRxSwCnt;

/* Typedefs
------------------------------------------------------------------------------*/
typedef struct
{
  MV_U32 ctrlRegReqFramesCnt; /* Count number of register request frames transmitted     */  
  MV_U32 ctrlRegAckFramesCnt; /* Count number of register acknowledge frames transmitted */  
  MV_U32 reportFramesCnt;     /* Count number of report frames transmitted               */
  MV_U32 dataFramesCnt;       /* Count number of data frames transmitted                 */
  MV_U32 txAllowedBytesCnt;   /* Count number of Tx Byte Allow counter                   */
}S_EponTxPm;

typedef struct
{
  MV_U32 fcsErrorFramesCnt;   /* Count number of received frames with FCS errors */
  MV_U32 shortFramesCnt;      /* Count number of short frames received           */
  MV_U32 longFramesCnt;       /* Count number of long frames received            */
  MV_U32 dataFramesCnt;       /* Count number of data frames received            */
  MV_U32 ctrlFramesCnt;       /* Count number of control frames received         */
  MV_U32 reportFramesCnt;     /* Count number of report frames received          */
  MV_U32 gateFramesCnt;       /* Count number of gate frames received            */
}S_RxPm;

typedef struct
{
  MV_U32 grantValidCnt;                 /* Count number of valid grant                          */
  MV_U32 grantMaxFutureTimeErrorCnt;    /* Count number of grant max future time error          */
  MV_U32 minProcTimeErrorCnt;           /* Count number of min proc time error                  */
  MV_U32 lengthErrorCnt;                /* Count number of length error                         */
  MV_U32 discoveryAndRegCnt;            /* Count number of discovery & register                 */
  MV_U32 fifoFullErrorCnt;              /* Count number of fifo full error                      */
  MV_U32 opcDiscoveryNotRegBcastCnt;    /* Count number of opc discoveryNotRegBcastCnt          */
  MV_U32 opcRegisterNotDiscoveryCnt;    /* Count number of opc register not discovery           */ 
  MV_U32 opcDiscoveryNotRegNotBcastCnt; /* Count number of opc discovery not register not bcast */ 
  MV_U32 opcDropGrantCnt;               /* Count number of opc drop grant                       */ 
  MV_U32 opcHiddenGrantCnt;             /* Count number of opc hidden grant                     */ 
  MV_U32 opcBackToBackCnt;              /* Count number of opc back to back                     */ 
}S_GpmPm;                               

typedef struct {
	MV_U32 legalFrameCnt;		/* stat0[31:0]	*/
	MV_U32 maxFrameSizeErrCnt;	/* stat1[31:16]	*/
	MV_U32 parityLenErrCnt;		/* stat1[15:0]	*/
	MV_U32 longGateErrCnt;		/* stat2[31:16]	*/
	MV_U32 protocolErrCnt;		/* stat2[15:0]	*/
	MV_U32 minFrameSizeErrCnt;	/* stat3[15:0]	*/
	MV_U32 legalFecFrameCnt;	/* stat4[31:16]	*/
	MV_U32 legalNonFecFrameCnt;	/* stat4[15:0]	*/
} S_PcsPm;

typedef struct
{
  MV_U32 swTxCnt[MAX_EPON_TX_SW_CNT];
  MV_U32 swRxCnt[MAX_EPON_RX_SW_CNT];
}S_SwPm;

typedef struct {
	S_RxPm     rx;
	S_EponTxPm tx;
	S_SwPm     sw;
	S_GpmPm    gpm;
	S_PcsPm    pcs;
} S_EponPm;

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/
void      onuEponPmInit(void);
void      onuEponPmTimerPmHndl(unsigned long data);
void      onuEponPmTimerExpireHndl(void);
MV_STATUS onuEponPmCountersAdd(void);
MV_STATUS onuEponPmSwRxCountersAdd(MV_U32 cnt, MV_U32 macId);
MV_STATUS onuEponPmSwTxCountersAdd(MV_U32 cnt, MV_U32 macId);
MV_STATUS onuEponPmSwPmGet(S_SwPm *swPm, MV_U32 macId);
MV_STATUS onuEponPmRxPmGet(S_RxPm *rxPm, MV_U32 macId);
MV_STATUS onuEponPmTxPmGet(S_EponTxPm *txPm, MV_U32 macId);
MV_STATUS onuEponPmTxDataPmLastIntervalGet(MV_U32 *txDataPm, MV_U32 macId);
MV_STATUS onuEponPmGpmPmGet(S_GpmPm *gpmPm, MV_U32 macId);
MV_U32    onuEponPmGpmValidGrantGet(MV_U32 macId);
MV_STATUS onuEponPmPcsPmGet(S_PcsPm *pcsPm);
MV_U32    onuEponPmCtrlCntGet(MV_U32 macId);
MV_U32    onuEponPmTxByteCntGet(MV_U32 macId);

MV_STATUS onuEponPmDbaAddBytePerLlidAndQueue(MV_U32 llId, MV_U32 queueId, MV_U32 counter);
MV_U32    onuEponPmDbaAddBytePerLlidAndQueueGet(MV_U32 llId, MV_U32 queueId);
MV_STATUS onuEponPmDbaAddBytePerLlidFifo(MV_U32 llId, MV_U32 counter);
MV_U32    onuEponPmDbaAddBytePerLlidFifoGet(MV_U32 llId);

MV_STATUS onuEponPmGunitLlidQueueCntGet(MV_U32 llid, MV_U32 queueId, MV_U32 *counter);
MV_STATUS onuEponPmGunitLlid0PktModMaxHeaderSizeSet(MV_U32 headerSize);
MV_STATUS onuEponPmGunitLlid0PktModPktCountGet(MV_U32 *packetCount);

MV_STATUS onuEponAlarmSet(MV_U32 alarm, MV_BOOL state);
void      onuEponAlarmGet(MV_U32 *alarm);
void      onuEponLosAlarmGet(MV_U32 *alarm);


MV_STATUS onuEponPmGenCrcTable(void);
MV_U32    onuEponPmGenCrcKey(MV_U32 msg[], MV_U32 size);
MV_STATUS onuEponPmRandomGet(MV_U32 *random);
MV_STATUS onuEponPmRandomReset(void);
MV_STATUS onuEponPmRandomThresholdCheck(MV_U32 *gateFrames, MV_U32 *discNotRegFrames, 
										MV_U32 *regReqframes);

/* Macros
------------------------------------------------------------------------------*/    

#endif /* _ONU_EPON_PM_H */
     


              





