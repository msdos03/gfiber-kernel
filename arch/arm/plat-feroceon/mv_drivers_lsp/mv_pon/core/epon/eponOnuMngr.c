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
/**  MODULE      : ONU EPON                                                  **/
/**                                                                          **/
/**  FILE        : eponOnuMngr.c                                             **/
/**                                                                          **/
/**  DESCRIPTION : This file implements ONU EPON Manager functionality       **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *
 * ========================================================================== *
 *
 ******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "eponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuMngr.c"

#define OAM_MSG_NOT_TRANS   (0xFFFF)
#define OAM_MSG_FIFO_LENGTH (16)
#define OAM_MSG_REGS        (10)
#define OAM_MSG_DG_IND      (1)

#define DBA_RPRT_MAX_SIZE   (35) /* Report via EPON REGS */

#define DBA_RPRT_PERIODIC_CYCLE (6)
#define DBA_RPRT_TX_FIFO_SIZE (4*1024)

/* Global Variables
------------------------------------------------------------------------------*/
extern spinlock_t onuPonIrqLock;

extern MV_U8 fixMacAddrs[8][6];

/* Local Variables
------------------------------------------------------------------------------*/

/* HW DBA params */
MV_U32 dbaRprtState           = 0; /* TO */
MV_U32 dbaRprtT0StateVal      = 0x43FF;
MV_U32 dbaRprtT0StateInterval = ONU_PON_TIMER_HW_RPRT_T0_INTERVAL;
MV_U32 dbaRprtT1StateVal      = 0x07FF;
MV_U32 dbaRprtT1StateInterval = ONU_PON_TIMER_HW_RPRT_T1_INTERVAL;

/* SW DBA params */

MV_U32 llidDbaRprtCount = 0;
MV_U32 llidDbaRprtValue = 0;

MV_U32 llidDbaRprtForOamMsgInd = 0;

MV_U32 llidCounter[8]           = {0,0,0,0,0,0,0,0};

MV_U32 llidQueueState[8][4][8]  = {
	                               {{0,0,0,0,0,0,0,0},
                                    {0,0,0,0,0,0,0,0},
                                    {0,0,0,0,0,0,0,0},
                                    {0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}},
								   {{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0},
									{0,0,0,0,0,0,0,0}}
                                  };

MV_U32 llidQueueThreshold[8][4][8] = {
	                                  {{0,0,0,0,0,0,0,0},
                                       {0,0,0,0,0,0,0,0},
                                       {0,0,0,0,0,0,0,0},
                                       {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}},
									  {{0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0},
									   {0,0,0,0,0,0,0,0}}
                                     };

MV_U32 llidNumOfQueuesets[8] = {0,0,0,0,0,0,0,0};

MV_U32 llidNumOfQueues[8] = {0,0,0,0,0,0,0,0};

MV_U32 llidQueuesBitMap[8] = {0,0,0,0,0,0,0,0};

MV_U32 llidNumOfReportBytes[8] = {0,0,0,0,0,0,0,0};

/* SW DBA array for Mix mode SW/HW */
S_OnuEponDbaShadowRegMap dbaSwRprtRegShadow[] =
{
  /* Index 0 - queueset*/{mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_SET, 0},
  /* Index 1 - bitmap*/  {mvAsicReg_EPON_TXM_CPP_RPRT_BIT_MAP,   0},
  /* Index 2 - queue0*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0,   0},
  /* Index 3 - queue1*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1,   0},
  /* Index 4 - queue2*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2,   0},
  /* Index 5 - queue3*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3,   0},
  /* Index 6 - queue4*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4,   0},
  /* Index 7 - queue5*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5,   0},
  /* Index 8 - queue6*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6,   0},
  /* Index 9 - queue7*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7,   0}
};

S_OnuEponDbaSwRprtRegMap dbaSwRprtRegMap[] =
{
  /*  0 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_SET,  0,  0},
  /*  1 */ {mvAsicReg_EPON_TXM_CPP_RPRT_BIT_MAP,    0,  1},
  /*  2 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0,    8,  2},
  /*  3 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0,    0,  2},
  /*  4 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1,    8,  3},
  /*  5 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1,    0,  3},
  /*  6 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2,    8,  4},
  /*  7 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2,    0,  4},
  /*  8 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3,    8,  5},
  /*  9 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3,    0,  5},
  /* 10 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4,    8,  6},
  /* 11 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4,    0,  6},
  /* 12 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5,    8,  7},
  /* 13 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5,    0,  7},
  /* 14 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6,    8,  8},
  /* 15 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6,    0,  8},
  /* 16 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7,    8,  9},
  /* 17 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7,    0,  9},
  /* 18 */ {mvAsicReg_EPON_TXM_CPP_RPRT_BIT_MAP,    8,  1},
  /* 19 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0,   24,  2},
  /* 20 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0,   16,  2},
  /* 21 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1,   24,  3},
  /* 22 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1,   16,  3},
  /* 23 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2,   24,  4},
  /* 24 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2,   16,  4},
  /* 25 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3,   24,  5},
  /* 26 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3,   16,  5},
  /* 27 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4,   24,  6},
  /* 28 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4,   16,  6},
  /* 29 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5,   24,  7},
  /* 30 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5,   16,  7},
  /* 31 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6,   24,  8},
  /* 32 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6,   16,  8},
  /* 33 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7,   24,  9},
  /* 34 */ {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7,   16,  9}
};

/* SW DBA array for Mix mode SW/HW */
S_OnuEponDbaShadowRegMap dbaDgSwRprtRegShadow[] =
{
  /* Index 0 - queueset*/{mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_SET, 0x00000001},
  /* Index 1 - bitmap*/  {mvAsicReg_EPON_TXM_CPP_RPRT_BIT_MAP,   0x000000FF},
  /* Index 2 - queue7*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0,   0x00000000},
  /* Index 3 - queue6*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1,   0x00000000},
  /* Index 4 - queue5*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2,   0x00000000},
  /* Index 5 - queue4*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3,   0x00000000},
  /* Index 6 - queue3*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4,   0x00000000},
  /* Index 7 - queue2*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5,   0x00000000},
  /* Index 8 - queue1*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6,   0x00000000},
  /* Index 9 - queue0*/  {mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7,   0x0000FFFF}
};

MV_U32 llidPacketFcsCompensation[] =
{
  /*  0    0 add    0 */  0,
  /*  1   64 add  1/2 */  2,
  /*  2  128 add  1/4 */  4,
  /*  3  192 add  1/6 */  6,
  /*  4  256 add  1/5 */  5,
  /*  5  320 add  1/6 */  6,
  /*  6  384 add  1/8 */  8,
  /*  7  448 add  1/8 */  8,
  /*  8  512 add  1/8 */  8,
  /*  9  576 add  1/9 */  9,
  /* 10  640 add 1/10 */ 10,
  /* 11  704 add 1/10 */ 10,
  /* 12  768 add 1/10 */ 10,
  /* 13  832 add 1/11 */ 11,
  /* 14  896 add 1/11 */ 11,
  /* 15  960 add 1/11 */ 11,
  /* 16 1024 add 1/11 */ 11,
  /* 17 1088 add 1/12 */ 12,
  /* 18 1152 add 1/12 */ 12,
  /* 19 1216 add 1/12 */ 12,
  /* 20 1280 add 1/12 */ 12,
  /* 21 1344 add 1/12 */ 12,
  /* 22 1408 add 1/12 */ 12,
  /* 23 1472 add 1/12 */ 12,
  /* 24 1536 add 1/12 */ 12,
  /* 25 1600 add 1/12 */ 12,
  /* 26 1664 add 1/12 */ 12,
  /* 27 1728 add 1/12 */ 12,
  /* 28 1792 add 1/12 */ 12,
  /* 29 1856 add 1/12 */ 12,
  /* 30 1920 add 1/12 */ 12,
  /* 31 1984 add 1/12 */ 12,
  /* 32 2048 add 1/12 */ 12
};

/* SW DBA report additional bandwidth params */
MV_U32 rprtLowRateThreshold;
MV_U32 rprtLowRateValue;
MV_U32 rprtLowRateConstValue;
MV_U32 rprtHighRateThreshold;
MV_U32 rprtHighRateValue;

/* SW DBA report message */
MV_U8  reportMsg[40];
MV_U8  reportMsgIdx;

/* OAM message transmission handling */
//S_OnuEponOamMsg oamDgMsgQueue;
S_OnuEponOamMsg oamMsgData[OAM_MSG_FIFO_LENGTH];

/* Data array pointers definition */
MV_U8 oamMsgWriteIndex = 0;
MV_U8 oamMsgReadIndex  = 0;

/* Dying Gasp message parameters */
MV_U16 dgMsgEtype    = 0x8809; /* Slow Protocol */
MV_U16 dgMsgSubType  = 0x03;   /* OAM */
MV_U16 dgMsgFlags    = 0x52;   /* Dying Gasp */
MV_U8  dgMsgCode     = 0x0;    /* Information OAMPDU */
S_OnuEponCtrlBuffer dgOamMsg;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
/* Interrupt handler Functions */
void      onuEponPonMngIntrAlarmHandler(MV_U32 alarm, MV_BOOL alarmStatus);
void      onuEponPonMngIntrMessageHandler(MV_U32 msg);
void      onuEponPonMngIntrRxCtrlFifoCleanup(void);
MV_STATUS onuEponPonMngIntrCtrlMessageHandler(S_OnuEponCtrlBuffer *ctrlBuf);
MV_STATUS onuEponPonMngIntrRprtMessageHandler(S_OnuEponCtrlBuffer *ctrlBuf);
MV_STATUS onuEponPonMngTimerRprtMessageHandler(MV_U32 macId);

/* State Machine Functions */
MV_STATUS onuEponPonMngRegMsgFlagAck(S_OnuEponRegMpcFrame *mpcFrame);
MV_STATUS onuEponPonMngRegMsgFlagNack(S_OnuEponRegMpcFrame *mpcFrame);
MV_STATUS onuEponPonMngRegMsgFlagReReg(S_OnuEponRegMpcFrame *mpcFrame);
MV_STATUS onuEponPonMngRegMsgFlagDeReg(S_OnuEponRegMpcFrame *mpcFrame);

/* MPC Message Handling Functions */
MV_STATUS onuEponReadCtrlFrameData(S_OnuEponCtrlBuffer *ctrlBuf);
MV_STATUS onuEponRetrieveCtrlFrameData(S_OnuEponCtrlBuffer *ctrlBuf);
MV_STATUS onuEponReadReportFrameData(S_OnuEponCtrlBuffer *ctrlBuf);
MV_STATUS onuEponRetrieveReportFrameData(S_OnuEponCtrlBuffer *ctrlBuf);

void      onuEponConvertN2HShort(void *srcValPtr, void *destValPtr);
void      onuEponConvertN2HLong(void *srcValPtr, void *destValPtr);
void      onuEponConvertH2NShort(void *srcValPtr, void *destValPtr);
void      onuEponConvertH2NLong(void *srcValPtr, void *destValPtr);

MV_STATUS onuEponOamMsgFifoInit(void);
MV_STATUS onuEponPonMngDbaParamsInit(void);
MV_STATUS onuEponPonMngDbaHwRprtHandler(void);
MV_STATUS onuEponPonMngDbaSwRprtHandler(void);
MV_STATUS onuEponPonMngDbaDgSwRprtHandler(void);

MV_STATUS onuEponSendCtrlFrameData(S_OnuEponCtrlBuffer *ctrlBuf, MV_U32 macId);
MV_BOOL   onuEponSendCtrlFrameDataFreeStatus(MV_U32 macId);

void      onuEponDgMsgDefaultMessageInit(void);

/******************************************************************************/
/* ========================================================================== */
/*                         Init Section                                       */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngAlarmHandlerExecute
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init EPON App
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponAppInit(void)
{
  MV_U32 macId;

  onuEponOamMsgFifoInit();
  onuEponDgMsgDefaultMessageInit();
  onuEponPonMngDbaParamsInit();
  for (macId = 0; macId < 8; macId++)
    onuEponDbaInfoUpdate(macId);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponOamMsgFifoInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init OAM messages FIFO
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponOamMsgFifoInit(void)
{
  memset(oamMsgData, 0, sizeof(S_OnuEponOamMsg) * 8);
//  memset(&oamDgMsgQueue, 0, sizeof(S_OnuEponOamMsg));

  return(MV_OK);
}


/*******************************************************************************
**
**  onuEponPonMngDbaParamsInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init DBA Report parameters
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponPonMngDbaParamsInit(void)
{
  rprtLowRateThreshold  = ONU_EPON_DBA_LOW_RATE_THRESHOLD;
  rprtLowRateValue      = ONU_EPON_DBA_LOW_RATE_VALUE;
  rprtLowRateConstValue = ONU_EPON_DBA_LOW_RATE_CONST_VALUE;
  rprtHighRateThreshold = ONU_EPON_DBA_HIGH_RATE_THRESHOLD;
  rprtHighRateValue     = ONU_EPON_DBA_HIGH_RATE_VALUE;

  /* LLID0 configuration */
  onuEponPmGunitLlid0PktModMaxHeaderSizeSet(0);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbaInfoUpdate
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init DBA Report parameters
**
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponDbaInfoUpdate(MV_U32 macId)
{
  MV_U32 queueId;
  MV_U32 queueSetId;

  llidNumOfQueues[macId]    = onuEponDbOnuDbaNumOfQueuesGet(macId);
  llidQueuesBitMap[macId]   = onuEponDbOnuDbaQueuesBitMapGet(macId);
  llidNumOfQueuesets[macId] = onuEponDbOnuDbaNumOfQueuesetGet(macId);

  llidNumOfReportBytes[macId] =  (((llidNumOfQueuesets[macId] * llidNumOfQueues[macId]) * 2) + /* number of bytes for queues */
  				                  llidNumOfQueuesets[macId]                                + /* number of bytes for bit map */
  				                  1);                                                        /* number of bytes for #queuesets */
  if (llidNumOfReportBytes[macId] > DBA_RPRT_MAX_SIZE) /* hardware limit */
    llidNumOfReportBytes[macId] = DBA_RPRT_MAX_SIZE;


  mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MODULE,
  		   "MAC[%d] QSet(%d) Q(%d) Rbytes(%d)\n",
  		   macId, llidNumOfQueuesets[macId], llidNumOfQueues[macId], llidNumOfReportBytes[macId]);

  for (queueSetId = 0; queueSetId < llidNumOfQueuesets[macId]; queueSetId++)
  {
    mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MODULE,
  		     "QSet(%d) Q[thres:state]", queueSetId);

    for (queueId = 0; queueId <  EPON_MAX_QUEUE; queueId++)
    {
  	  onuEponDbOnuDbaQueueThresholdGet(&(llidQueueThreshold[macId][queueSetId][queueId]),
  	  							     &(llidQueueState[macId][queueSetId][queueId]),
  	    							     queueId, queueSetId, macId);
  	  mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MODULE,
  	  		     "[%d:%d]",
  	  		     llidQueueThreshold[macId][queueSetId][queueId],
  	  		     llidQueueState[macId][queueSetId][queueId]);
    }

    mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "\n");
  }

  return(MV_OK);
}

/******************************************************************************/
/* ========================================================================== */
/*                         Alarm Section                                      */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngAlarmHandlerExecute
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt alarm
**               functionality
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngAlarmHandlerExecute(MV_U32 macId)
{
  MV_STATUS status;
  MV_U32    startMacId;
  MV_U32    endMacId;
  MV_U32    interruptMask;
  LINKSTATUSFUNC linkStatusCallback;

  if (macId == ALL_MAC_INDICATION)
  {
	startMacId = 0;
	endMacId   = EPON_MAX_MAC_NUM;
  }
  else
  {
	startMacId = macId;
	endMacId   = macId + 1;
  }

  if ((macId == 0) || (macId == ALL_MAC_INDICATION))
  {
	if (onuEponDbModeGet() == E_EPON_IOCTL_STD_MODE)
	{
	  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "===================\n");
	  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "== MPCP Sync Off ==\n");
	  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "===================\n");
	}
  }

  if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_SW_DBA_RPRT_TIMER)
  {
	/* Stop SW DBA event miss timer */
	onuEponIsrTimerEventMissStateSet(MV_FALSE);

	/* Clear onu DBA Report interrupt */
	mvOnuEponMacPonInterruptMaskGet(&interruptMask);
	interruptMask &= ~(ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
	mvOnuEponMacPonInterruptMaskSet(interruptMask);
  }
  else
  {
	/* Clear onu DBA Report Timer */
	onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
  }

  /* handle silence mode */
  if (onuEponDbOnuSilenceStateGet() != ONU_SILENCE_NOT_ACTIVE)
  {

	/* Clear onu Silence Timers */
	for (macId = startMacId; macId < endMacId; macId++)
	{
		onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonSilenceTimerId[macId]));
	}

	status  = mvOnuEponMacGenOnuConfigAutoAckSet(ONU_REG_ACK_AUTO_RES);
	status |= mvOnuEponMacGenOnuConfigAutoRequestSet(ONU_REG_REQ_AUTO_RES);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
				 "ERROR: (%s:%d) Failed to enable auto request / ack\n\r", __FILE_DESC__, __LINE__);
	  return;
	}

	onuEponDbOnuCfgAutoSet(ONU_REG_REQ_AUTO_RES, ONU_REG_ACK_AUTO_RES, ONU_REPORT_AUTO_RES);
  }

  for (macId = startMacId; macId < endMacId; macId++)
  {
      /* re-init onu database */
      status = onuEponDbReInit(macId);
      if (status != MV_OK)
      {
        mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
                   "ERROR: (%s:%d) Failed to re-init onu database\n\r", __FILE_DESC__, __LINE__);
        return;
      }

      /* init onu Asic */
      status = onuEponAsicReInit(macId, ONU_SILENCE_DISABLED);
      if (status != MV_OK)
      {
        mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
                   "ERROR: (%s:%d) Failed to re-init onu EPON MAC\n\r", __FILE_DESC__, __LINE__);
        return;
      }
  }

  /* Call link status callback function */
  linkStatusCallback = onuEponDbLinkStatusCallbackGet();
  if (linkStatusCallback != NULL)
  {
    if (onuEponForceTxDownStateGet(0) != MV_TRUE)
	{ 
	  mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE, 
                 "DEBUG: (%s:%d) Notify link is DOWN\n", __FILE_DESC__, __LINE__); 
      linkStatusCallback(MV_FALSE); 
    } 
  }

  if (onuEponDbOnuHoldoverStateGet() != ONU_HOLDOVER_NOT_ACTIVE)
  {
	if (onuEponDbOnuHoldoverExecGet() != ONU_HOLDOVER_NOT_ACTIVE)
	{
	  onuEponDbOnuHoldoverExecSet(ONU_HOLDOVER_NOT_ACTIVE);
	}
  }
}

/*******************************************************************************
**
**  onuEponPonMngIntrAlarmHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt alarm
**               functionality
**
**  PARAMETERS:  MV_U32 alarm
**               MV_U32 alarmStatus
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngIntrAlarmHandler(MV_U32 alarm, MV_BOOL alarmStatus)
{
	if (alarmStatus != MV_FALSE) {

		/* alarm is ON */
		if (onuEponDbOnuHoldoverStateGet() != ONU_HOLDOVER_NOT_ACTIVE) {
			if (onuEponDbOnuHoldoverExecGet() != ONU_HOLDOVER_ACTIVE) {
				/* start onu epon pon holdover timer */
				onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHoldoverTimerId));
				onuEponDbOnuHoldoverExecSet(ONU_HOLDOVER_ACTIVE);
				mvPonPrint(PON_PRINT_DEBUG, PON_ISR_STATE_MODULE,
						   "DEBUG: (%s:%d) Holdover Timer Start(Intr)\n", __FILE_DESC__, __LINE__);
			}
		} else
			onuEponPonMngAlarmHandlerExecute(ALL_MAC_INDICATION);

		/* update alarm */
		onuEponAlarmSet(alarm, alarmStatus);

	} else {

		/* alarm is OFF */
		if (onuEponDbOnuHoldoverStateGet() != ONU_HOLDOVER_NOT_ACTIVE) {
			if (onuEponDbOnuHoldoverExecGet() != ONU_HOLDOVER_NOT_ACTIVE) {
				/* stop onu epon pon holdover timer */
				onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHoldoverTimerId));
				onuEponDbOnuHoldoverExecSet(ONU_HOLDOVER_NOT_ACTIVE);
				mvPonPrint(PON_PRINT_DEBUG, PON_ISR_STATE_MODULE,
						   "DEBUG: (%s:%d) Holdover Timer Stop\n", __FILE_DESC__, __LINE__);
			}
		}

		/* update alarm */
		onuEponAlarmSet(alarm, alarmStatus);
	}
}

/******************************************************************************/
/* ========================================================================== */
/*                         Register Section                                   */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngIntrRegHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt register
**               functionality
**
**  PARAMETERS:  MV_U32 reg
**               MV_U32 regStatus
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngIntrRegHandler(MV_U32 reg, MV_BOOL regStatus)
{
  MV_STATUS status;
  MV_U32    delay;
  MV_U32    laser;

  /* Register State Active */
  if (regStatus != MV_FALSE)
  {
    delay  = ((0x0 << 8) | (0x0 + ONU_DEF_DDM_DELAY));
    status = mvOnuEponMacDdmDelaySet(delay);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                 "ERROR: (%s:%d) mvAsicReg_EPON_DDM_1814_CONFIG\n\r", __FILE_DESC__, __LINE__);
    }

    laser  = (ONU_DEF_LASER_ON_OFF_TIME << 24) |
             (ONU_DEF_LASER_OFF_TIME << 8) |
             (ONU_DEF_LASER_ON_TIME);
    status = mvOnuEponMacGenLaserParamSet(ONU_DEF_LASER_ON_OFF_TIME,
                                          ONU_DEF_LASER_ON_TIME,
                                          ONU_DEF_LASER_OFF_TIME);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacGenLaserParamSet\n\r", __FILE_DESC__, __LINE__);
    }

    mvPonPrint(PON_PRINT_INFO, PON_API_MODULE,
               "Register State[ON]: DDM_Config(0x%x) LASER_PARAM(0x%x)\n\r", delay, laser);
  }
  /* Register State Not - Active */
  else
  {
    delay  = ONU_DEF_DDM_DELAY;
    status = mvOnuEponMacDdmDelaySet(delay);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                 "ERROR: (%s:%d) mvAsicReg_EPON_DDM_1814_CONFIG\n\r", __FILE_DESC__, __LINE__);
    }

    laser  = (ONU_DEF_LASER_ON_OFF_TIME << 24) |
             (ONU_DEF_LASER_OFF_TIME << 8) |
             (ONU_DEF_LASER_ON_TIME);
    status = mvOnuEponMacGenLaserParamSet(ONU_DEF_LASER_ON_OFF_TIME,
                                          ONU_DEF_LASER_ON_TIME,
                                          ONU_DEF_LASER_OFF_TIME);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacGenLaserParamSet\n\r", __FILE_DESC__, __LINE__);
    }

    mvPonPrint(PON_PRINT_INFO, PON_API_MODULE,
               "Register State[OFF]: DDM_Config(0x%x) LASER_PARAM(0x%x)\n\r", delay, laser);
  }


}

/******************************************************************************/
/* ========================================================================== */
/*                         Message Section                                    */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngTimerHwRprtTxModHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function transmit OAM frame handling
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngTimerHwRprtTxModHndl(unsigned long data)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonHwRprtTxModTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  onuEponPonMngTxCtrlMessageHandler();

  if ((onuPonResourceTbl_s.onuPonHwRprtTxModTimerId.onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTxModTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponPonMngTxCtrlMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt MPC message
**               functionality
**
**  PARAMETERS:  MV_U32 msg
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngTxCtrlMessageHandler(void)
{
	MV_STATUS status;

	status = onuEponOamHandleMsg();
	if (status != OAM_MSG_NOT_TRANS)
		llidDbaRprtForOamMsgInd = 1;

	if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_SW_DBA_RPRT_TIMER)
		onuEponPonMngSwRprtMessageHandler();
}

/*******************************************************************************
**
**  onuEponPonMngHwRprtMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager DBA HW report functionality
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngHwRprtMessageHandler(void)
{
  MV_STATUS status;

  status = onuEponPonMngDbaHwRprtHandler();
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_MODULE,
  			   "ERROR: (%s:%d) Handle HW DBA Report\n\r", __FILE_DESC__, __LINE__);
    return;
  }
}

/*******************************************************************************
**
**  onuEponPonMngSwRprtMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager DBA SW report functionality
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngSwRprtMessageHandler(void)
{
	MV_STATUS status;

	status = onuEponPonMngDbaSwRprtHandler();
	if (status != MV_OK) {
		mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_MODULE,
			   "ERROR: (%s:%d) Handle SW DBA Report\n\r", __FILE_DESC__, __LINE__);
		return;
	}
}

/*******************************************************************************
**
**  onuEponPonMngIntrMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager message handling functionality
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngIntrMessageHandler(MV_U32 msg)
{
	MV_STATUS           status;
	S_OnuEponCtrlBuffer ctrlBuf;

	memset((void*)&ctrlBuf, 0, sizeof (S_OnuEponCtrlBuffer));

	if (msg == ONU_EPON_RX_CTRL_MSG) {
		do {
			status = onuEponRetrieveCtrlFrameData(&ctrlBuf);
			if (status != MV_OK)
				return;

			status = onuEponPonMngIntrCtrlMessageHandler(&ctrlBuf);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
						   "ERROR: (%s:%d) Handle Rx Ctrl Frame\n\r", __FILE_DESC__, __LINE__);
				onuEponPmSwRxCountersAdd(TOTAL_MPCP_RX_ERROR_FRAME_CNT, 0);
                return;
			}
		} while (status == MV_OK);

	} else if (msg == ONU_EPON_RX_RPRT_MSG) {

		status = onuEponRetrieveReportFrameData(&ctrlBuf);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
					   "ERROR: (%s:%d) Read Rx Report Frame\n\r", __FILE_DESC__, __LINE__);
			return;
		}

		status = onuEponPonMngIntrRprtMessageHandler(&ctrlBuf);
		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
					   "ERROR: (%s:%d) Handle Rx Report Frame\n\r", __FILE_DESC__, __LINE__);
			return;
		}

	} else {

		mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
				   "ERROR: (%s:%d) Unsupported Frame\n\r", __FILE_DESC__, __LINE__);
        return;
	}
}

/*******************************************************************************
**
**  onuEponPonMngIntrRxCtrlFifoCleanup
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function clear the pon manager RX ctrl message FIFO
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngIntrRxCtrlFifoCleanup(void)
{
	MV_STATUS           status;
	S_OnuEponCtrlBuffer ctrlBuf;

	memset((void*)&ctrlBuf, 0, sizeof (S_OnuEponCtrlBuffer));

	do {
		status = onuEponRetrieveCtrlFrameData(&ctrlBuf);

	} while (status == MV_OK);
}

/*******************************************************************************
**
**  onuEponPonMngIntrCtrlMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt MPC Ctrl
**               message functionality
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponPonMngIntrCtrlMessageHandler(S_OnuEponCtrlBuffer *ctrlBuf)
{
  MV_STATUS            status;
  S_OnuEponGenMpcFrame *genMpcMsg;
  S_OnuEponRegMpcFrame *regMpcMsg;
  MV_U16               regOpcode;
  MV_U16               regFlags;

  genMpcMsg = (S_OnuEponGenMpcFrame *)ctrlBuf->data;
  onuEponConvertN2HShort(&genMpcMsg->genMpcPdu.opCode, &regOpcode);

	if (regOpcode == MPC_OPCODE_REGISTER) {

		regMpcMsg = (S_OnuEponRegMpcFrame *)ctrlBuf->data;
		regFlags  = regMpcMsg->regMpcPdu.flags;

		if (regFlags == REGISTER_FLAGS_ACK) {
			onuEponPmSwRxCountersAdd(TOTAL_MPCP_REGISTER_ACK_CNT, 0);

			status = onuEponPonMngRegMsgFlagAck(regMpcMsg);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
						   "ERROR: (%s:%d) Register with flag Ack\n\r", __FILE_DESC__, __LINE__);
				return(MV_ERROR);
			}

		} else if (regFlags == REGISTER_FLAGS_NACK) {

			onuEponPmSwRxCountersAdd(TOTAL_MPCP_REGISTER_NACK_CNT, 0);

			status = onuEponPonMngRegMsgFlagNack(regMpcMsg);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
						   "ERROR: (%s:%d) Register with flag Nack\n\r", __FILE_DESC__, __LINE__);
				return(MV_ERROR);
			}

		} else if (regFlags == REGISTER_FLAGS_REREG) {

			onuEponPmSwRxCountersAdd(TOTAL_MPCP_REGISTER_REREG_FRAME_CNT, 0);

			status = onuEponPonMngRegMsgFlagReReg(regMpcMsg);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
						   "ERROR: (%s:%d) Register with flag Re-Register\n\r", __FILE_DESC__, __LINE__);
				return(MV_ERROR);
			}

		} else if (regFlags == REGISTER_FLAGS_DEREG) {

			onuEponPmSwRxCountersAdd(TOTAL_MPCP_REGISTER_DEREG_FRAME_CNT, 0);

			status = onuEponPonMngRegMsgFlagDeReg(regMpcMsg);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
						   "ERROR: (%s:%d) Register with flag De-Register\n\r", __FILE_DESC__, __LINE__);
				return(MV_ERROR);
			}

		} else {

			mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
					   "ERROR: (%s:%d) Invalid Control message flag(%d)\n\r", __FILE_DESC__, __LINE__, regFlags);
			return(MV_ERROR);
		}

	}

	return(MV_OK);

} /* end of onuEponPonMngIntrCtrlMessageHandler */


/*******************************************************************************
**
**  onuEponPonMngIntrRprtMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt MPC Report
**               message functionality
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngIntrRprtMessageHandler(S_OnuEponCtrlBuffer *ctrlBuf)
{
  mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MODULE,
             "ERROR: (%s:%d) Report Messages are handled by the EPON MAC\n\r", __FILE_DESC__, __LINE__);
  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPonMngTimerRprtMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager interrupt MPC Report
**               message transmit function
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngTimerRprtMessageHandler(MV_U32 macId)
{
  MV_STATUS             status;
  S_OnuEponCtrlBuffer   ctrlBuf;
  S_OnuEponGenMpcFrame *genMpcMsg;
  MV_U16                msgEtype  = 0x8808;
  MV_U16                msgOpcode = 0x0003;
  MV_U16                reportIdx;
  MV_U16                frameIdx;

  /* clear message */
  memset(&ctrlBuf, 0, MIN_RPRT_FRAME_LEN);

  /* set message length */
  ctrlBuf.length = MIN_RPRT_FRAME_LEN;

  genMpcMsg = (S_OnuEponGenMpcFrame *)&(ctrlBuf.data);

  /* update message content */
  genMpcMsg->stdEthFrame.destAddr[0] = 0x01;
  genMpcMsg->stdEthFrame.destAddr[1] = 0x80;
  genMpcMsg->stdEthFrame.destAddr[2] = 0xC2;
  genMpcMsg->stdEthFrame.destAddr[3] = 0x00;
  genMpcMsg->stdEthFrame.destAddr[4] = 0x00;
  genMpcMsg->stdEthFrame.destAddr[5] = 0x01;

  onuEponConvertH2NShort(&msgEtype,  &genMpcMsg->stdEthFrame.etherType);

  onuEponConvertH2NShort(&msgOpcode, &genMpcMsg->genMpcPdu.opCode);

  genMpcMsg->genMpcPdu.timeStamp  = 0;

  genMpcMsg->stdEthFrame.srcAddr[0] = fixMacAddrs[macId][0];
  genMpcMsg->stdEthFrame.srcAddr[1] = fixMacAddrs[macId][1];
  genMpcMsg->stdEthFrame.srcAddr[2] = fixMacAddrs[macId][2];
  genMpcMsg->stdEthFrame.srcAddr[3] = fixMacAddrs[macId][3];
  genMpcMsg->stdEthFrame.srcAddr[4] = fixMacAddrs[macId][4];
  genMpcMsg->stdEthFrame.srcAddr[5] = fixMacAddrs[macId][5];

  for (reportIdx = 0, frameIdx = 20; reportIdx < 40; reportIdx++, frameIdx++)
  {
    ctrlBuf.data[frameIdx] = reportMsg[reportIdx];
  }

  status = onuEponSendCtrlFrameData(&ctrlBuf, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Handle OAM Message\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  return(MV_OK);
}

/*******************************************************************************
**  ========================================================================  **
**                            SW Report Section                               **
**  ========================================================================  **
*******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngDbaSwRprtAddQueueSetThreshold
**
*******************************************************************************/
//void onuEponPonMngDbaSwRprtAddQueueSetThreshold(MV_32 queueSetIdx, MV_32 queueIdx)
//{
//  if (llidQueueState[queueSetIdx][queueIdx] != 0)
//  {
//    if (llidCounter[queueIdx] > llidQueueThreshold[queueSetIdx][queueIdx])
//	{
//	  reportMsg[reportMsgIdx++] = ((llidQueueThreshold[queueSetIdx][queueIdx] >> 8) & 0xFF);
//	  reportMsg[reportMsgIdx++] =  (llidQueueThreshold[queueSetIdx][queueIdx]       & 0xFF);
//	}
//    else
//	{
//	  reportMsg[reportMsgIdx++] = ((llidCounter[queueIdx] >> 8) & 0xFF);
//	  reportMsg[reportMsgIdx++] =  (llidCounter[queueIdx]       & 0xFF);
//	}
//  }
//}

/*******************************************************************************
**
**  onuEponPonMngDbaSwRprtHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager DBA SW report handler
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngDbaSwRprtHandler(void)
{
	MV_STATUS status = MV_OK;
	MV_U32    idx;
	MV_U32    llidIdx;
	MV_32     queueSetIdx;
	MV_32     queueIdx;
	MV_U32    msgIdx;
	MV_U32    msgTmpData;
	MV_U32    llidQueuesByteCount;
	MV_U32    llidTotalByteCount;
	MV_U32    llidAvePacketLength;
	MV_U32    llidPacketCountCompensationIdx;
	MV_U32    llidPacketCountCompensation;
	MV_U32    llidPacketCount = 0;
	MV_U32    llidTransmittedPacketCount = 0;
	MV_U32    tmpLlidHighQueue;   /* highest queue according to mng configuration */
	MV_U32    llidTxFecState = 0;
	MV_U32    ts[10] = {0};
	MV_U32    *pTs = ts;
	MV_U32    interval[6] = {0};
#define COLLECT_DBA_TS		mvOnuEponMacGenLocalTimeStampGet(pTs++)

	for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++) {

		llidQueuesByteCount         = 0;
		llidTotalByteCount          = 0;
		llidPacketCount             = 0;
		llidPacketCountCompensation = 0;

		/* Non-active LLID */
		if (onuEponDbOnuDbaLlidActiveStatusGet(llidIdx) != ONU_DBA_ACTIVE_LLID)
			continue;

		COLLECT_DBA_TS;

		/* ======================= */
		/* Retrieve Queue Counters */
		/* ======================= */

		/* update counters in case FEC is enabled */
		onuEponDbOnuTxFecCfgGet(&llidTxFecState, llidIdx);

		queueSetIdx = 0;

		for (queueIdx = 0; queueIdx < EPON_MAX_QUEUE; queueIdx++) {
			/* for each queue */
			if (llidQueuesBitMap[llidIdx] & (1 << queueIdx)) {

				status = onuEponPmGunitLlidQueueCntGet(llidIdx, queueIdx, &(llidCounter[queueIdx]));
				if (status != MV_OK) {
					mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
							"ERROR: (%s:%d) Llid[%d] Queue[%d] Cnt Get Failed\n\r",
							__FILE_DESC__, __LINE__, llidIdx, queueIdx);
					return(MV_ERROR);
				}

			} else
				llidCounter[queueIdx] = 0;

			mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_MODULE,
					"SW DBA, queue[%d] count[%d]\n", queueIdx, llidCounter[queueIdx]);

			/* convert to TQ units */
			llidCounter[queueIdx] /= 2;

			/* sum queues TQ to verify data was received */
			llidQueuesByteCount += llidCounter[queueIdx];
		} /* for each queue */

		/* Count the number of transmitted words in the GPUnit without - checksum and header modified bytes */
		/* Counter is clear-on-read */
		onuEponPmGunitLlid0PktModPktCountGet(&llidPacketCount);

		tmpLlidHighQueue = onuEponDbOnuDbaHighestReportQueueGet(llidIdx);

		llidTotalByteCount = (llidQueuesByteCount * 2) + (llidPacketCount * 2);

		COLLECT_DBA_TS;

		/* =========================== */
		/* Low-Rate and Birsty Traffic */
		/* =========================== */
		/* Incase there is no data was found in the LLID queues, and */
		/* The LLID packet count != 0, meaning packets were transmitted towards the GPON MAC */
		/* Add the number of transmitted packets in words + threshold LLID to the highest queue report */
		/* The number of transmitted packets does include packets checksum and header modified bytes */
		/* The threshold is used to compensate checksum and header modified bytes */
		/* The report is sent maximum of DBA_RPRT_PERIODIC_CYCLE (6) intervals, it will be periodicly transmitted */
		/* until is identifes data in the LLID Queues or LLID FIFO. */
		/* In case of very low rate traffic, it send the report DBA_RPRT_PERIODIC_CYCLE (6) intervals. */
		/* This WA is to compensate the "breathing" scheme of OLT DBA GATE allocation */
		if (llidQueuesByteCount == 0) {

			if (llidPacketCount != 0) {

				llidDbaRprtCount = DBA_RPRT_PERIODIC_CYCLE;
				/* If the previous GATE allowed large traffic burst, so all packets from queues got TXed and
				*  packet counter indicates more packets than theoretically can be waiting in TX FIFO,
				*  limit the counter value to the FIFO size in order to avoid bandwidth oversubscribtion needed
				*  for FIFO release
				*/
				if (llidPacketCount > DBA_RPRT_TX_FIFO_SIZE)
					llidDbaRprtValue = DBA_RPRT_TX_FIFO_SIZE;
				else
					llidDbaRprtValue = llidPacketCount;
			}

			if ((llidDbaRprtCount > 0) && (llidDbaRprtCount <= DBA_RPRT_PERIODIC_CYCLE)) {

				llidCounter[tmpLlidHighQueue] += (llidDbaRprtValue + (rprtLowRateConstValue / 2));

				mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_WA_MODULE,
							"DBA WA[%d], PktCnt(%d), RprtCnt(%d)\n",
							llidDbaRprtCount, llidDbaRprtValue, llidCounter[tmpLlidHighQueue]);

				llidDbaRprtCount--;
			}

		} else {

			llidDbaRprtCount = 0;
			llidDbaRprtValue = 0;
		}

		COLLECT_DBA_TS;

		/* ================= */
		/* High-Rate Traffic */
		/* ================= */
		/* Incase data was found in the LLID queues, the 4K FIFO is full */
		/* If total number of bytes in the LLID queues is less the 8192 bytes */
		/* Add configurable number of TQ (default to 1000) to highest queue report */
		if ((llidQueuesByteCount > 0) && (llidQueuesByteCount < (rprtHighRateThreshold / 2)))
			llidCounter[tmpLlidHighQueue] += (rprtHighRateValue / 2);

		/* =========== */
		/* OAM Traffic */
		/* =========== */
		if (llidDbaRprtForOamMsgInd == 1) {
			mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_MODULE,
					"SW DBA, OAM Message addition to highQ[%d], \n", tmpLlidHighQueue);

			llidCounter[tmpLlidHighQueue] += (rprtLowRateValue / 2);

			llidDbaRprtForOamMsgInd = 0;
		}

		COLLECT_DBA_TS;

		/* =========== */
		/* FEC Section */
		/* =========== */
		if (llidTxFecState != 0) {

			status = onuEponPmTxDataPmLastIntervalGet(&llidTransmittedPacketCount, llidIdx);
			if (status != MV_OK) {
				for (queueIdx = 0; queueIdx < EPON_MAX_QUEUE; queueIdx++) {
					if (llidCounter[queueIdx] != 0)
						llidCounter[queueIdx] = (llidCounter[queueIdx] + (llidCounter[queueIdx] / 12));/* default */
				}

				mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_FEC_MODULE, "SW DBA FEC, default setting\n");

			} else {

				if (llidTransmittedPacketCount != 0) {

					llidAvePacketLength            = (llidTotalByteCount / llidTransmittedPacketCount);
					llidPacketCountCompensationIdx = ((llidAvePacketLength / 64) + 1);

					if (llidPacketCountCompensationIdx > 32/*limit*/)
						llidPacketCountCompensationIdx = 32;

					llidPacketCountCompensation = llidPacketFcsCompensation[llidPacketCountCompensationIdx];

					mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_FEC_MODULE,
							"SW DBA FEC, Total Bytes(%d), Total Pkt, Ave Pkt(%d), CompIdx(%d), CompVal(%d)\n",
							llidTotalByteCount, llidTransmittedPacketCount,
							llidAvePacketLength, llidPacketCountCompensationIdx, llidPacketCountCompensation);

					for (queueIdx = 0; queueIdx < EPON_MAX_QUEUE; queueIdx++) {
						if (llidCounter[queueIdx] != 0)
							llidCounter[queueIdx] =
									(llidCounter[queueIdx] + (llidCounter[queueIdx] / llidPacketCountCompensation));
					}

				} /* llidTransmittedPacketCount != 0 */
			} /* onuEponPmTxDataPmLastIntervalGet() == MV_OK */
		} /* llidTxFecState != 0 */

		COLLECT_DBA_TS;

		/* ==================== */
		/* Build Report Message */
		/* ==================== */
		/* Support for normal report
		** Queue Set: [Qset1, Qset2, .., QsetN, BC]
		** Queue: [Queue0, .., QueueN]
		** Report is sent via EPON REGISTERS
		** =================== */
		memset(reportMsg, 0, 40);

		reportMsgIdx = 0;
		reportMsg[reportMsgIdx++] = llidNumOfQueuesets[llidIdx];

		/* update queueset reports */
		for (queueSetIdx = 0; queueSetIdx < llidNumOfQueuesets[llidIdx]; queueSetIdx++) {

			reportMsg[reportMsgIdx++] = llidQueuesBitMap[llidIdx];

			for (queueIdx = (EPON_MAX_QUEUE - 1); queueIdx >= 0; queueIdx--) {
				if (llidQueueState[llidIdx][queueSetIdx][queueIdx] != 0) {
					if (llidCounter[queueIdx] > llidQueueThreshold[llidIdx][queueSetIdx][queueIdx]) {
						reportMsg[reportMsgIdx++] = ((llidQueueThreshold[llidIdx][queueSetIdx][queueIdx] >> 8) & 0xFF);
						reportMsg[reportMsgIdx++] = (llidQueueThreshold[llidIdx][queueSetIdx][queueIdx] & 0xFF);
					} else {
						reportMsg[reportMsgIdx++] = ((llidCounter[queueIdx] >> 8) & 0xFF);
						reportMsg[reportMsgIdx++] =  (llidCounter[queueIdx]       & 0xFF);
					}
				}
			} /* for each queue */
		} /* for each queueset */

		/* Build report in message format */
		for (idx = 0; idx < OAM_MSG_REGS; idx++)
			dbaSwRprtRegShadow[idx].shadow = 0;

		for (msgIdx = 0; msgIdx < llidNumOfReportBytes[llidIdx]; msgIdx++) {

			msgTmpData = ((((MV_U32)(reportMsg[msgIdx])) << dbaSwRprtRegMap[msgIdx].shift) |
										(dbaSwRprtRegShadow[dbaSwRprtRegMap[msgIdx].shadowIdx].shadow));
			dbaSwRprtRegShadow[dbaSwRprtRegMap[msgIdx].shadowIdx].shadow = msgTmpData;
		}

		/* =================== */
		/* Send Report Message */
		/* =================== */
		for (msgIdx = 0; msgIdx < OAM_MSG_REGS; msgIdx++) {

			status = asicOntGlbRegWrite(dbaSwRprtRegShadow[msgIdx].reg,
										dbaSwRprtRegShadow[msgIdx].shadow,
										llidIdx);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
						"ERROR: (%s:%d) Llid[%d] msgIdx[%d] Regiter write failed\n\r",
						__FILE_DESC__, __LINE__, llidIdx, msgIdx);
				return(MV_ERROR);
			}
		}

		COLLECT_DBA_TS;

		for (idx = 0; idx < 4; idx++) {
			if (ts[idx + 1] > ts[idx])
				interval[idx] = (MV_U32)(-1) - ts[idx] + ts[idx + 1];
			else
				interval[idx] = ts[idx + 1] - ts[idx];
			interval[5] += interval[idx];
		}
		mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_TS_MODULE,
				"llid[%d] ticks - QC[%d], LR[%d], HR[%d], FEC[%d], TX[%d], TOTAL[%d - %dnS]\n\r",
				llidIdx, interval[0], interval[1], interval[2], interval[3], interval[4], interval[5],
				interval[5] * 16);

	} /* For each LLID */

	return(status);
} /* enf of onuEponPonMngDbaSwRprtHandler */

/*******************************************************************************
**
**  onuEponPonMngDbaSwRprtHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager DBA SW report handler
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngDbaDgSwRprtHandler(void)
{
	MV_STATUS status = MV_OK;
	MV_U32    llidIdx;
	MV_U32    msgIdx;

	for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++) {
		/* Active LLID */
		/* =========== */
		if (onuEponDbOnuDbaLlidActiveStatusGet(llidIdx) == ONU_DBA_ACTIVE_LLID) {
			/* Send Report Message */
			/* =================== */
			for (msgIdx = 0; msgIdx < OAM_MSG_REGS; msgIdx++) {
				status = asicOntGlbRegWrite(dbaDgSwRprtRegShadow[msgIdx].reg,
							    dbaDgSwRprtRegShadow[msgIdx].shadow,
							    llidIdx);
				if (status != MV_OK)
					return(MV_ERROR);
			}
		}
	}

	return(status);
}

/*******************************************************************************
**
**  onuEponPonMngDbaDgSwRprtCfg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function build DG DBA SW report
**
**  PARAMETERS:  MV_U32 numOfQueue
**               MV_U32 queueReport
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponPonMngDbaDgSwRprtCfg(MV_U32 numOfQueue, MV_U32 queueReport)
{
  MV_U32 msgIdx;
  MV_U32 msgIdx_2;
  MV_U8  bitMapArray[] = { /* 0  */ 0x00,
	                       /* 1  */ 0x80,
	                       /* 2  */ 0xC0,
	                       /* 3  */ 0xE0,
	                       /* 4  */ 0xF0,
	                       /* 5  */ 0xF8,
	                       /* 6  */ 0xFC,
	                       /* 7  */ 0xFE,
	                       /* 8  */ 0xFF};

  if (numOfQueue > 8)       numOfQueue  = 8;
  if (queueReport > 0xFFFF) queueReport = 0xFFFF;

  /* bit map */
  dbaDgSwRprtRegShadow[1].shadow = bitMapArray[numOfQueue];

  /* queues */
  for (msgIdx = 2; msgIdx < (2 + numOfQueue); msgIdx++)
    dbaDgSwRprtRegShadow[msgIdx].shadow = queueReport;

  for (msgIdx_2 = msgIdx; msgIdx_2 < 10; msgIdx_2++)
    dbaDgSwRprtRegShadow[msgIdx_2].shadow = 0;

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "DG DBA Report\n");
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queueset - 0x%08X\n", dbaDgSwRprtRegShadow[0].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Bitmap   - 0x%08X\n", dbaDgSwRprtRegShadow[1].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-0  - 0x%08X\n", dbaDgSwRprtRegShadow[2].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-1  - 0x%08X\n", dbaDgSwRprtRegShadow[3].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-2  - 0x%08X\n", dbaDgSwRprtRegShadow[4].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-3  - 0x%08X\n", dbaDgSwRprtRegShadow[5].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-4  - 0x%08X\n", dbaDgSwRprtRegShadow[6].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-5  - 0x%08X\n", dbaDgSwRprtRegShadow[7].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-6  - 0x%08X\n", dbaDgSwRprtRegShadow[8].shadow);
  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "Queue-7  - 0x%08X\n", dbaDgSwRprtRegShadow[9].shadow);
}

/*******************************************************************************
**
**  onuEponPonMngDbaHwRprtHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager DBA HW report handler
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngDbaHwRprtHandler(void)
{
  MV_STATUS status;
  MV_U32    highRprtQueue;
  MV_U32    macId;

  if (dbaRprtState == 0)
  {
    for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
    {
      highRprtQueue = onuEponDbOnuDbaHighestReportQueueGet(macId);

      mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_MODULE,
     		     "HW DBA report[T0], mac(%d), highest(%d)\n", macId, highRprtQueue);

      status = mvOnuEponMacTxmCppReportQueueX(highRprtQueue, dbaRprtT0StateVal, macId);
      if (status != MV_OK)
      {
    	mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
    		   "ERROR: (%s:%d) onuEponSendCtrlFrameData\n\r", __FILE_DESC__, __LINE__);
    	return(MV_ERROR);
      }
    }

	onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonHwRprtTimerId), 0, dbaRprtT0StateInterval, 1);
    dbaRprtState = 1;
  }
  else if (dbaRprtState == 1)
  {
    for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++)
    {
      highRprtQueue = onuEponDbOnuDbaHighestReportQueueGet(macId);

      mvPonPrint(PON_PRINT_DEBUG, PON_MNG_DBA_MODULE,
     		     "HW DBA report[T1], mac(%d), highest(%d)\n", macId, highRprtQueue);

      status = mvOnuEponMacTxmCppReportQueueX(highRprtQueue, dbaRprtT1StateVal, macId);
      if (status != MV_OK)
      {
    	mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
    		   "ERROR: (%s:%d) onuEponSendCtrlFrameData\n\r", __FILE_DESC__, __LINE__);
    	return(MV_ERROR);
      }
    }

	onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonHwRprtTimerId), 0, dbaRprtT1StateInterval, 1);
    dbaRprtState = 0;
  }

  return (0);
}

/*******************************************************************************
**
**  onuEponPonMngTimerHwRprtCfg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager DBA timer report config
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngTimerHwRprtCfg(MV_U32 t0_val, MV_U32 t0_time, MV_U32 t1_val, MV_U32 t1_time)
{
  dbaRprtT0StateVal      = t0_val;
  dbaRprtT0StateInterval = t0_time;
  dbaRprtT1StateVal      = t1_val;
  dbaRprtT1StateInterval = t1_time;

  return (0);
}

/*******************************************************************************
**
**  onuEponPonMngTimerSilenceHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager silence timer handler
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
void onuEponPonMngTimerSilenceHndl(unsigned long macId)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonSilenceTimerId[macId].onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  mvPonPrint(PON_PRINT_DEBUG, PON_MNG_SILENCE_MODULE,
			 "DEBUG: (%s:%d) Silence Timer[%d] Expire\n", __FILE_DESC__, __LINE__, macId);
  onuEponPonMngAlarmHandlerExecute(macId);

  if ((onuPonResourceTbl_s.onuPonSilenceTimerId[macId].onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonSilenceTimerId[macId]));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

#ifndef PON_FPGA
/******************************************************************************/
/* ========================================================================== */
/*                         OAM - Dying Gasp Message Handling Section          */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngDgMsgPrint
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function print DG message
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
void onuEponPonMngDgMsgPrint(void)
{
  S_OnuEponOamFrame *oamMsg;

  oamMsg = ((S_OnuEponOamFrame*)(&(dgOamMsg.data[0])));

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE,
  		 "\n"
  		 "DG Message\n"
  		 "==========\n"
  		 "DestMac[%x:%x:%x:%x:%x:%x] SrcMac[%x:%x:%x:%x:%x:%x]\n"
  		 "Etype[0x%x] flags[%x] code[0x%x]\n",
  		 oamMsg->stdEthFrame.destAddr[0], oamMsg->stdEthFrame.destAddr[1],
  		 oamMsg->stdEthFrame.destAddr[2], oamMsg->stdEthFrame.destAddr[3],
  		 oamMsg->stdEthFrame.destAddr[4], oamMsg->stdEthFrame.destAddr[5],
  		 oamMsg->stdEthFrame.srcAddr[0],  oamMsg->stdEthFrame.srcAddr[1],
  		 oamMsg->stdEthFrame.srcAddr[2],  oamMsg->stdEthFrame.srcAddr[3],
  		 oamMsg->stdEthFrame.srcAddr[4],  oamMsg->stdEthFrame.srcAddr[5],
  		 dgMsgEtype, oamMsg->stdOamHeader.subType,
  		 dgMsgFlags, oamMsg->stdOamHeader.code);
}

/*******************************************************************************
**
**  onuEponPonMngDgMessageHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager dying gasp interrupt
**               message transmit function
**
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
MV_STATUS onuEponPonMngDgMessageHandler(MV_U32 macId)
{
  MV_STATUS status = MV_ERROR;
  MV_U32    transNum = 0;
  MV_U32    maxTransNum = 100;

  onuEponPonMngDbaDgSwRprtHandler();
  do
  {
	if (onuEponSendCtrlFrameDataFreeStatus(macId) != MV_TRUE)
	  continue;

	status = onuEponSendCtrlFrameData(&dgOamMsg, macId);
    transNum++;

  } while ((status != MV_OK) && (transNum <= maxTransNum));

//  status = onuEponDgAllocMsg(&dgOamMsg, macId);
//  if (status != MV_OK)
//  {
//	mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
//			   "ERROR: (%s:%d) onuEponPonMngDgMessageHandler", __FILE_DESC__, __LINE__);
//	return(status);
//  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDgMsgDefaultMessageInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: Create default dying gasp interrupt message
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponDgMsgDefaultMessageInit(void)
{
  MV_U32 macId = 0;
  S_OnuEponOamFrame *oamMsg;

  /* clear message */
  memset(&dgOamMsg, 0, sizeof(S_OnuEponCtrlBuffer));

  /* set message length */
  dgOamMsg.length = MIN_DG_FRAME_LEN;

  /* update message content */
  oamMsg = ((S_OnuEponOamFrame*)(&(dgOamMsg.data[0])));

  oamMsg->stdEthFrame.destAddr[0] = 0x01;
  oamMsg->stdEthFrame.destAddr[1] = 0x80;
  oamMsg->stdEthFrame.destAddr[2] = 0xC2;
  oamMsg->stdEthFrame.destAddr[3] = 0x00;
  oamMsg->stdEthFrame.destAddr[4] = 0x00;
  oamMsg->stdEthFrame.destAddr[5] = 0x02;

  oamMsg->stdEthFrame.srcAddr[0] = fixMacAddrs[macId][0];
  oamMsg->stdEthFrame.srcAddr[1] = fixMacAddrs[macId][1];
  oamMsg->stdEthFrame.srcAddr[2] = fixMacAddrs[macId][2];
  oamMsg->stdEthFrame.srcAddr[3] = fixMacAddrs[macId][3];
  oamMsg->stdEthFrame.srcAddr[4] = fixMacAddrs[macId][4];
  oamMsg->stdEthFrame.srcAddr[5] = fixMacAddrs[macId][5];

  onuEponConvertH2NShort(&dgMsgEtype,  &oamMsg->stdEthFrame.etherType);

  oamMsg->stdOamHeader.subType = dgMsgSubType;

  onuEponConvertH2NShort(&dgMsgFlags,  &oamMsg->stdOamHeader.flags);

  oamMsg->stdOamHeader.code = dgMsgCode;
}

/*******************************************************************************
**
**  onuEponPonMngIntrDgHandler
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements the pon manager dying gasp interrupt
**               handler function
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_STATUS
**
*******************************************************************************/
void onuEponPonMngIntrDgHandler(void)
{
  static MV_U32 first_time = 1;
  DYINGGASPFUNC dgCallback;
  MV_U32        macId = 0;

  onuEponPonMngDgMessageHandler(macId);

  if (first_time)
  {
    dgCallback = onuEponDbDgCallbackGet();

    if (dgCallback != NULL)
    {
        dgCallback();
        first_time = 0;
    }
  }

  onuPonDyingGaspExit();

  onuEponPonMngDgMsgPrint();
}

#endif /* PON_FPGA */

/******************************************************************************/
/* ========================================================================== */
/*                         MPC State Machine Handling Section                 */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPonMngRegMsgFlagAck
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is called when REGISTER message with flag Ack is
**               received
**
**  PARAMETERS:  S_OnuEponRegMpcFrame *mpcFrame
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponPonMngRegMsgFlagAck(S_OnuEponRegMpcFrame *mpcFrame)
{
  MV_STATUS status;
  MV_U32    macId;
  MV_U32    macState;
  MV_U16    msgEtype;
  MV_U16    msgSyncTime;
  MV_U16    msgLlid;
  MV_U16    msgOpcode;
  MV_U8     msgFlags;
  MV_U8     msgEchoedPendingGrants;
  MV_U32    interruptMask;
  LINKSTATUSFUNC linkStatusCallback;

  status = matchDestAddressToMacId(mpcFrame->stdEthFrame.destAddr, &macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MAC_ADDR_MODULE,
               "ERROR: (%s:%d) matchDestAddressToMacId, invalid dest addr [%02X:%02X:%02X:%02X:%02X:%02X]\n",
               __FILE_DESC__, __LINE__,
               mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
               mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
               mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5]);
    return(MV_OK);
  }

  onuEponConvertN2HShort(&mpcFrame->stdEthFrame.etherType, &msgEtype);
  onuEponConvertN2HShort(&mpcFrame->genMpcPdu.opCode, &msgOpcode);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.assignedLlidPort, &msgLlid);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.syncTime, &msgSyncTime);
  msgFlags               = mpcFrame->regMpcPdu.flags;
  msgEchoedPendingGrants = mpcFrame->regMpcPdu.echoedPendingGrants;

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE,
             "\n"
             "REGISTER [Flag=Ack] Message\n"
             "===========================\n"
             "DestMac[%x:%x:%x:%x:%x:%x] SrcMac[%x:%x:%x:%x:%x:%x]\n"
             "Etype[0x%x] Opcode[%x] Sync[0x%x], LLID[%x] Grant[%x] Flag[%x]\n",
             mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
             mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
             mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5],
             mpcFrame->stdEthFrame.srcAddr[0], mpcFrame->stdEthFrame.srcAddr[1],
             mpcFrame->stdEthFrame.srcAddr[2], mpcFrame->stdEthFrame.srcAddr[3],
             mpcFrame->stdEthFrame.srcAddr[4], mpcFrame->stdEthFrame.srcAddr[5],
             msgEtype,
             msgOpcode,
             msgSyncTime,
             msgLlid,
             msgEchoedPendingGrants,
             msgFlags);

  /* Validate mac state was updated by HW */
  status = mvOnuEponMacOnuStateGet(&macState, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuStateGet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  if (macState != ONU_EPON_REGISTERED)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Mac[%d] state is not Registered!!!!\n\r", __FILE_DESC__, __LINE__, macId);
    return(MV_ERROR);
  }

  onuEponApiFecReConfig();

  if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_SW_DBA_RPRT_TIMER)
  {
	/* Set onu DBA Report interrupt */
	mvOnuEponMacPonInterruptMaskGet(&interruptMask);
	interruptMask |= (ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
	mvOnuEponMacPonInterruptMaskSet(interruptMask);
	onuEponDbaSwRprtMacTimerCfg(onuEponDbOnuSwRprtMacTimerIntervalGet());

	/* Start SW DBA event miss timer */
	onuEponIsrTimerEventMissStateSet(MV_TRUE);

  }
  else
  {
	onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
  }

  onuEponDbPktTxLlidSet(msgLlid, macId);
  onuEponDbPktRxLlidSet(msgLlid, macId);
  onuEponDbOnuStateSet(ONU_EPON_03_OPERATION, macId);
  onuPonLedHandler(ONU_PON_SYNC_LED, ACTIVE_LED_ON);

#if 0 /* NOT USED */
  status = onuEponAsicQueueEnable(macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
               "ERROR: (%s:%d) onuEponAsicQueueEnable\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }
#endif /* NOT USED */

  mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
             "= ONU Mac[%d] Registered =\n\r", macId);
  if (macId == 0)
  {
     mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "==================\n");
     mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "== MPCP Sync On ==\n");
     mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE, "==================\n");
  }

  /* Call link status callback function */
  linkStatusCallback = onuEponDbLinkStatusCallbackGet();
  if (linkStatusCallback != NULL)
  {
  	 mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE, 
                "DEBUG: (%s:%d) Notify link is UP\n", __FILE_DESC__, __LINE__); 
    linkStatusCallback(MV_TRUE);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPonMngRegMsgFlagNack
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is called when REGISTER message with flag Nack
**               is received
**
**  PARAMETERS:  S_OnuEponRegMpcFrame *mpcFrame
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponPonMngRegMsgFlagNack(S_OnuEponRegMpcFrame *mpcFrame)
{
  MV_STATUS status;
  MV_U32    macId;
  MV_U16    msgEtype;
  MV_U16    msgSyncTime;
  MV_U16    msgLlid;
  MV_U16    msgOpcode;
  MV_U8     msgFlags;
  MV_U8     msgEchoedPendingGrants;

  status = matchDestAddressToMacId(mpcFrame->stdEthFrame.destAddr, &macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MAC_ADDR_MODULE,
               "ERROR: (%s:%d) matchDestAddressToMacId, invalid dest addr [%02X:%02X:%02X:%02X:%02X:%02X]\n",
               __FILE_DESC__, __LINE__,
               mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
               mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
               mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5]);
    return(MV_OK);
  }

  onuEponConvertN2HShort(&mpcFrame->stdEthFrame.etherType, &msgEtype);
  onuEponConvertN2HShort(&mpcFrame->genMpcPdu.opCode, &msgOpcode);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.assignedLlidPort, &msgLlid);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.syncTime, &msgSyncTime);
  msgFlags               = mpcFrame->regMpcPdu.flags;
  msgEchoedPendingGrants = mpcFrame->regMpcPdu.echoedPendingGrants;

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE,
             "\n"
             "REGISTER [Flag=Nack] Message\n"
             "============================\n"
             "DestMac[%x:%x:%x:%x:%x:%x] SrcMac[%x:%x:%x:%x:%x:%x]\n"
             "Etype[0x%x] Opcode[%x] Sync[0x%x], LLID[%x] Grant[%x] Flag[%x]\n",
             mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
             mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
             mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5],
             mpcFrame->stdEthFrame.srcAddr[0], mpcFrame->stdEthFrame.srcAddr[1],
             mpcFrame->stdEthFrame.srcAddr[2], mpcFrame->stdEthFrame.srcAddr[3],
             mpcFrame->stdEthFrame.srcAddr[4], mpcFrame->stdEthFrame.srcAddr[5],
             msgEtype,
             msgOpcode,
             msgSyncTime,
             msgLlid,
             msgEchoedPendingGrants,
             msgFlags);


  if (onuEponDbOnuSilenceStateGet() != ONU_SILENCE_NOT_ACTIVE)
  {
    status  = mvOnuEponMacGenOnuConfigAutoAckSet(0);
    status |= mvOnuEponMacGenOnuConfigAutoRequestSet(0);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
                 "ERROR: (%s:%d) Failed to disable auto request / ack\n\r", __FILE_DESC__, __LINE__);
	  return(MV_ERROR);
    }

	onuEponDbOnuCfgAutoSet(ONU_REG_REQ_SW_RES, ONU_REG_ACK_SW_RES, ONU_REPORT_AUTO_RES);

	onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonSilenceTimerId[macId]), macId, ONU_PON_TIMER_SILENCE_INTERVAL, 1);

	mvPonPrint(PON_PRINT_DEBUG, PON_MNG_SILENCE_MODULE,
			   "DEBUG: (%s:%d) Silence Timer Start\n", __FILE_DESC__, __LINE__);
  }
  else
  {
    onuEponPonMngAlarmHandlerExecute(macId);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPonMngRegMsgFlagReReg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is called when REGISTER message with flag re-register
**               is received
**
**  PARAMETERS:  S_OnuEponRegMpcFrame *mpcFrame
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponPonMngRegMsgFlagReReg(S_OnuEponRegMpcFrame *mpcFrame)
{
  MV_STATUS status;
  MV_U32    macId;
  MV_U32    macState;
  MV_U16    msgEtype;
  MV_U16    msgSyncTime;
  MV_U16    msgLlid;
  MV_U16    msgOpcode;
  MV_U8     msgFlags;
  MV_U8     msgEchoedPendingGrants;

  status = matchDestAddressToMacId(mpcFrame->stdEthFrame.destAddr, &macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MAC_ADDR_MODULE,
               "ERROR: (%s:%d) matchDestAddressToMacId, invalid dest addr [%02X:%02X:%02X:%02X:%02X:%02X]\n",
               __FILE_DESC__, __LINE__,
               mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
               mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
               mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5]);
    return(MV_OK);
  }

  onuEponConvertN2HShort(&mpcFrame->stdEthFrame.etherType, &msgEtype);
  onuEponConvertN2HShort(&mpcFrame->genMpcPdu.opCode, &msgOpcode);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.assignedLlidPort, &msgLlid);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.syncTime, &msgSyncTime);
  msgFlags               = mpcFrame->regMpcPdu.flags;
  msgEchoedPendingGrants = mpcFrame->regMpcPdu.echoedPendingGrants;

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE,
             "\n"
             "REGISTER [Flag=ReReg] Message\n"
             "=============================\n"
             "DestMac[%x:%x:%x:%x:%x:%x] SrcMac[%x:%x:%x:%x:%x:%x]\n"
             "Etype[0x%x] Opcode[%x] Sync[0x%x], LLID[%x] Grant[%x] Flag[%x]\n",
             mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
             mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
             mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5],
             mpcFrame->stdEthFrame.srcAddr[0], mpcFrame->stdEthFrame.srcAddr[1],
             mpcFrame->stdEthFrame.srcAddr[2], mpcFrame->stdEthFrame.srcAddr[3],
             mpcFrame->stdEthFrame.srcAddr[4], mpcFrame->stdEthFrame.srcAddr[5],
             msgEtype,
             msgOpcode,
             msgSyncTime,
             msgLlid,
             msgEchoedPendingGrants,
             msgFlags);

  /* Validate mac state was updated by HW */
  status = mvOnuEponMacOnuStateGet(&macState, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) mvOnuEponMacOnuStateGet\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  if (macState != ONU_EPON_REGISTERED)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Mac[%d] state is not Registered!!!!\n\r", __FILE_DESC__, __LINE__, macId);
    return(MV_ERROR);
  }

  onuEponDbPktTxLlidSet(msgLlid, macId);
  onuEponDbPktRxLlidSet(msgLlid, macId);
  onuEponDbOnuStateSet(ONU_EPON_03_OPERATION, macId);
  onuPonLedHandler(ONU_PON_SYNC_LED, ACTIVE_LED_ON);

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE,
             "= ONU Mac[%d] Registered =\n\r", macId);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPonMngRegMsgFlagDeReg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is called when REGISTER message with flag de-register
**               is received
**
**  PARAMETERS:  S_OnuEponRegMpcFrame *mpcFrame
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponPonMngRegMsgFlagDeReg(S_OnuEponRegMpcFrame *mpcFrame)
{
  MV_STATUS status;
  MV_U32    macId;
  MV_U32    interruptMask;
  MV_U16    msgEtype;
  MV_U16    msgSyncTime;
  MV_U16    msgLlid;
  MV_U16    msgOpcode;
  MV_U8     msgFlags;
  MV_U8     msgEchoedPendingGrants;
  LINKSTATUSFUNC linkStatusCallback;

  status = matchDestAddressToMacId(mpcFrame->stdEthFrame.destAddr, &macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_DEBUG, PON_MNG_MAC_ADDR_MODULE,
               "ERROR: (%s:%d) matchDestAddressToMacId, invalid dest addr [%02X:%02X:%02X:%02X:%02X:%02X]\n",
               __FILE_DESC__, __LINE__,
               mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
               mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
               mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5]);
    return(MV_OK);
  }

  onuEponConvertN2HShort(&mpcFrame->stdEthFrame.etherType, &msgEtype);
  onuEponConvertN2HShort(&mpcFrame->genMpcPdu.opCode, &msgOpcode);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.assignedLlidPort, &msgLlid);
  onuEponConvertN2HShort(&mpcFrame->regMpcPdu.syncTime, &msgSyncTime);
  msgFlags               = mpcFrame->regMpcPdu.flags;
  msgEchoedPendingGrants = mpcFrame->regMpcPdu.echoedPendingGrants;

  mvPonPrint(PON_PRINT_INFO, PON_MNG_MODULE,
             "\n"
             "REGISTER [Flag=DeReg] Message\n"
             "=============================\n"
             "DestMac[%x:%x:%x:%x:%x:%x] SrcMac[%x:%x:%x:%x:%x:%x]\n"
             "Etype[0x%x] Opcode[%x] Sync[0x%x], LLID[%x] Grant[%x] Flag[%x]\n",
             mpcFrame->stdEthFrame.destAddr[0], mpcFrame->stdEthFrame.destAddr[1],
             mpcFrame->stdEthFrame.destAddr[2], mpcFrame->stdEthFrame.destAddr[3],
             mpcFrame->stdEthFrame.destAddr[4], mpcFrame->stdEthFrame.destAddr[5],
             mpcFrame->stdEthFrame.srcAddr[0], mpcFrame->stdEthFrame.srcAddr[1],
             mpcFrame->stdEthFrame.srcAddr[2], mpcFrame->stdEthFrame.srcAddr[3],
             mpcFrame->stdEthFrame.srcAddr[4], mpcFrame->stdEthFrame.srcAddr[5],
             msgEtype,
             msgOpcode,
             msgSyncTime,
             msgLlid,
             msgEchoedPendingGrants,
             msgFlags);

  if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_SW_DBA_RPRT_TIMER)
  {
	/* Stop SW DBA event miss timer */
	onuEponIsrTimerEventMissStateSet(MV_FALSE);

	/* Clear onu DBA Report interrupt */
	mvOnuEponMacPonInterruptMaskGet(&interruptMask);
	interruptMask &= ~(ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
	mvOnuEponMacPonInterruptMaskSet(interruptMask);
  }
  else
  {
	/* Clear onu DBA Report Timer */
	onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
  }

  /* re-init onu database */
  status = onuEponDbReInit(macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Failed to re-init onu database\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* init onu Asic */
  status = onuEponAsicReInit(macId, ONU_SILENCE_ENABLED);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Failed to re-init onu EPON MAC\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* Call link status callback function */
  linkStatusCallback = onuEponDbLinkStatusCallbackGet();
  if (linkStatusCallback != NULL)
  {
    if (onuEponForceTxDownStateGet(0) != MV_TRUE)
      linkStatusCallback(MV_FALSE);
  }

  /* send signal to management layer */
  onuEponStatusNotify(0/*don't care*/);

  return(MV_OK);
}

/******************************************************************************/
/* ========================================================================== */
/*                         MPC Message Handling Section                       */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponReadCtrlFrameData
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function read control frame from the Rx Control queue
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponReadCtrlFrameData(S_OnuEponCtrlBuffer *ctrlBuf)
{
  MV_STATUS status;
  MV_U32    usedCount;
  MV_U32    data;
  MV_U32    index;

  status = mvOnuEponMacCpqRxCtrlQueueUsedCountGet(&usedCount);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Rx Ctrl Queue Used Count\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  if (usedCount < ctrlBuf->length)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Not enough data,  Frame Length = %d, usedCount = %d\n\r",
               __FILE_DESC__, __LINE__, ctrlBuf->length, usedCount);
    return(MV_ERROR);
  }

  for(index = 0; index < ctrlBuf->length; index++)
  {
    status = mvOnuEponMacCpqRxCtrlQueueReadData(&data);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
                 "ERROR: (%s:%d) Read Rx Ctrl Queue Data\n\r", __FILE_DESC__, __LINE__);
      return(status);
    }
    else
    {
      ctrlBuf->data[index] = (MV_U8)data;
    }
  }

  onuEponPmSwRxCountersAdd(TOTAL_MPCP_RX_FRAME_CNT, 0);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponRetrieveCtrlFrameData
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function retrieve control frame from the Rx Control queue
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponRetrieveCtrlFrameData(S_OnuEponCtrlBuffer *ctrlBuf)
{
  MV_STATUS status;
  MV_U32    usedHeaderCount;
  MV_U32    flagLow;
  MV_U32    flagHigh;
  MV_U32    frameLength;

  ctrlBuf->length = 0;

  status = mvOnuEponMacCpqRxCtrlHeaderQueueUsedCountGet(&usedHeaderCount);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Rx Ctrl Header Queue Used Count\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  if (usedHeaderCount <= 0)
  {
    return(MV_ERROR);
  }

  status = mvOnuEponMacCpqRxCtrlHeaderQueueReadData(&flagLow, &flagHigh);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Rx Ctrl Header Queue Data\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  frameLength = ((flagLow & MPC_FRAME_LENGTH_MASK) >> MPC_FRAME_LENGTH_SHIFT);
  if ((frameLength > 0) && (frameLength <= sizeof(ctrlBuf->data)))
  {
    ctrlBuf->length = frameLength;
    status = onuEponReadCtrlFrameData(ctrlBuf);
    if (status != MV_OK)
    {
      return(status);
    }
  }
  else
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Unexpected frame length %d\n\r", __FILE_DESC__, __LINE__, frameLength);
    return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponSendCtrlFrameData
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function check for free space for transmission of control frame
**
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_TRUE or MV_FALSE
**
*******************************************************************************/
MV_BOOL onuEponSendCtrlFrameDataFreeStatus(MV_U32 macId)
{
  MV_STATUS status;
  MV_U32    numFreeFrames;
  MV_U32    numFreeData;

  status = mvOnuEponMacCpqTxCtrlHeaderQueueFree(&numFreeFrames, macId);
  if (status != MV_OK)
    return(MV_FALSE);

  status = mvOnuEponMacCpqTxCtrlQueueFree(&numFreeData, macId);
  if (status != MV_OK)
    return(MV_FALSE);

  if ((numFreeFrames <= 0) || (numFreeData < 256))
    return(MV_FALSE);

  return(MV_TRUE);
}

/*******************************************************************************
**
**  onuEponSendCtrlFrameData
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function send control frame to the Tx Control queue
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponSendCtrlFrameData(S_OnuEponCtrlBuffer *ctrlBuf, MV_U32 macId)
{
  MV_STATUS status;
  MV_U32    numFreeFrames;
  MV_U32    numFreeData;
  MV_U32    index;

  status = mvOnuEponMacCpqTxCtrlHeaderQueueFree(&numFreeFrames, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Tx Ctrl Header Queue\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  status = mvOnuEponMacCpqTxCtrlQueueFree(&numFreeData, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Tx Ctrl Queue\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  if (numFreeFrames <= 0)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
    		   "ERROR: (%s:%d) No free place in Tx Ctrl Header Queue (%d), data(%d)\n\r",
    		   __FILE_DESC__, __LINE__, numFreeFrames, numFreeData);
    return(MV_ERROR);
  }

  if (numFreeData < ctrlBuf->length)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) No free place in Tx Ctrl Data Queue\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  for(index = 0; index < ctrlBuf->length; index++)
  {
    status = mvOnuEponMacCpqTxCtrlQueueWrite(ctrlBuf->data[index], macId);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
                 "ERROR: (%s:%d) Write Tx Ctrl Queue Data\n\r", __FILE_DESC__, __LINE__);
      return(status);
    }
  }

  status = mvOnuEponMacCpqTxCtrlHeaderQueueWrite(ctrlBuf->length, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Write Tx Ctrl Header Queue\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  onuEponPmSwTxCountersAdd(TOTAL_MPCP_TX_FRAME_CNT, macId);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponReadReportFrameData
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function read report frame from the Rx report queue
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponReadReportFrameData(S_OnuEponCtrlBuffer *ctrlBuf)
{
  MV_STATUS status;
  MV_U32    usedCount;
  MV_U32    data;
  MV_U32    index;

  status = mvOnuEponMacCpqRxRprtQueueUsedCountGet(&usedCount);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Rx Report Queue Used Count\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  if (usedCount < ctrlBuf->length)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Not enough data,  Frame Length = %i, usedCount = %i\n\r",
               __FILE_DESC__, __LINE__, ctrlBuf->length, usedCount);
    return(MV_ERROR);
  }

  for(index = 0; index < usedCount; index++)
  {
    status = mvOnuEponMacCpqRxRprtQueueReadData(&data);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
                 "ERROR: (%s:%d) Read Rx Report Queue Data\n\r", __FILE_DESC__, __LINE__);
      return(status);
    }
    else
    {
      ctrlBuf->data[index] = (MV_U8)data;
    }
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponRetrieveReportFrameData
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function retrieve report frame from the Rx report queue
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponRetrieveReportFrameData(S_OnuEponCtrlBuffer *ctrlBuf)
{
  MV_STATUS status;
  MV_U32    usedHeaderCount;
  MV_U32    flagLow;
  MV_U32    flagHigh;
  MV_U32    frameLength;

  ctrlBuf->length = 0;

  status = mvOnuEponMacCpqRxRprtHeaderQueueUsedCountGet(&usedHeaderCount);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Rx Report Header Queue Used Count\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  if (usedHeaderCount <= 0)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) No frames received,  Used Header Count = %i\n\r",
               __FILE_DESC__, __LINE__, usedHeaderCount);
    return(MV_ERROR);
  }

  status = mvOnuEponMacCpqRxRprtHeaderQueueReadData(&flagLow, &flagHigh);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Read Rx Report Header Queue Data\n\r", __FILE_DESC__, __LINE__);
    return(status);
  }

  frameLength = flagLow & MPC_FRAME_LENGTH_MASK;
  if (frameLength > 0 && frameLength <= sizeof(ctrlBuf->data))
  {
    ctrlBuf->length = frameLength;
    status = onuEponReadReportFrameData(ctrlBuf);
    if (status != MV_OK)
    {
      return(status);
    }
  }
  else
  {
    mvPonPrint(PON_PRINT_ERROR, PON_MNG_MODULE,
               "ERROR: (%s:%d) Unexpected frame length %d\n\r", __FILE_DESC__, __LINE__, frameLength);
    return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponConvertN2HShort
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function convert MV_U16 from Netwotk order to Host
**
**  PARAMETERS:  void *srcValPtr
**               void *destValPtr
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponConvertN2HShort(void *srcValPtr, void *destValPtr)
{
  MV_U16 src;
  MV_U16 dest;

  memcpy(&src, srcValPtr, sizeof(src));

  dest = ((src       & 0xFF) << 8) |
         ((src >> 8) & 0xFF);

  memcpy(destValPtr, &dest, sizeof(dest));
}

/*******************************************************************************
**
**  onuEponConvertH2NShort
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function convert MV_U16 from Host order to Network
**
**  PARAMETERS:  void *srcValPtr
**               void *destValPtr
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponConvertH2NShort(void *srcValPtr, void *destValPtr)
{
  MV_U16 src;
  MV_U16 dest;

  memcpy(&src, srcValPtr, sizeof(src));

  dest = ((src       & 0xFF) << 8) |
         ((src >> 8) & 0xFF);

  memcpy(destValPtr, &dest, sizeof(dest));
}

/*******************************************************************************
**
**  onuEponConvertN2HLong
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function convert MV_U32 from Netwotk order to Host
**
**  PARAMETERS:  void *srcValPtr
**               void *destValPtr
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponConvertN2HLong(void *srcValPtr, void *destValPtr)
{
  MV_U32 src;
  MV_U32 dest;

  memcpy(&src, srcValPtr, sizeof(src));

  dest =  ((src        & 0xFF) << 24) |
         (((src >>  8) & 0xFF) << 16) |
         (((src >> 16) & 0xFF) <<  8) |
          ((src >> 24) & 0xFF);

  memcpy(destValPtr, &dest, sizeof(dest));
}

/*******************************************************************************
**
**  onuEponConvertH2NLong
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function convert MV_U32 from Host order to Network
**
**  PARAMETERS:  void *srcValPtr
**               void *destValPtr
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponConvertH2NLong(void *srcValPtr, void *destValPtr)
{
  MV_U32 src;
  MV_U32 dest;

  memcpy(&src, srcValPtr, sizeof(src));

  dest =  ((src        & 0xFF) << 24) |
         (((src >>  8) & 0xFF) << 16) |
         (((src >> 16) & 0xFF) <<  8) |
          ((src >> 24) & 0xFF);

  memcpy(destValPtr, &dest, sizeof(dest));
}

/*******************************************************************************
**
**  onuEponOamHandleMsg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function transmit OAM frames from SW FIFO
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponOamHandleMsg(void)
{
  MV_STATUS        status;
  MV_U8  		   nextOamMsgReadIndex;
  S_OnuEponOamMsg *oamMsg;

  if (oamMsgReadIndex != oamMsgWriteIndex)
  {
    nextOamMsgReadIndex = oamMsgReadIndex + 1;
    /* roll over */
    if (nextOamMsgReadIndex == OAM_MSG_FIFO_LENGTH)
    {
      nextOamMsgReadIndex = 0;
    }
    oamMsg = (S_OnuEponOamMsg*)(&(oamMsgData[nextOamMsgReadIndex]));

	if(onuEponSendCtrlFrameDataFreeStatus(oamMsg->macId) != MV_TRUE)
	  return(MV_OK);

	/* Send OAM message to queue */
	status = onuEponSendCtrlFrameData(&(oamMsg->msg), oamMsg->macId);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
				 "ERROR: (%s:%d) onuEponSendCtrlFrameData OAM", __FILE_DESC__, __LINE__);
	  return(status);
	}

	oamMsgReadIndex = nextOamMsgReadIndex;
	mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE,
               "Handle read(%d) write(%d)\n", oamMsgReadIndex, oamMsgWriteIndex);

	return(MV_OK);
  }

  return(OAM_MSG_NOT_TRANS);
}

/*******************************************************************************
**
**  onuEponOamFlushAllMsg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function flush OAM frames from SW FIFO
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponOamFlushAllMsg(void)
{
  oamMsgReadIndex = oamMsgWriteIndex;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponOamAllocMsg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function allocate OAM frame for transmission
**
**  PARAMETERS:  S_OnuEponOamMsg *msg
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponOamAllocMsg(S_OnuEponCtrlBuffer *msg, MV_U32 macId)
{
  MV_U8 nextOamMsgWriteIndex;

  if ((oamMsgWriteIndex + 1) != oamMsgReadIndex)
  {
    nextOamMsgWriteIndex = oamMsgWriteIndex + 1;
    /* roll over */
    if(nextOamMsgWriteIndex == OAM_MSG_FIFO_LENGTH) nextOamMsgWriteIndex = 0;

	/* update OAM message to the FIFO */
	oamMsgData[nextOamMsgWriteIndex].macId = macId;
    memcpy (&(oamMsgData[nextOamMsgWriteIndex].msg), msg, sizeof (S_OnuEponCtrlBuffer));

	/* update index */
    oamMsgWriteIndex = nextOamMsgWriteIndex;

	mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE,
	           "Alloc read(%d) write(%d\n", oamMsgReadIndex, oamMsgWriteIndex);

	if (((oamMsgWriteIndex > oamMsgReadIndex)              &&
		((oamMsgWriteIndex - oamMsgReadIndex) > 2))        ||
		((oamMsgWriteIndex < oamMsgReadIndex)              &&
		((OAM_MSG_FIFO_LENGTH - (oamMsgReadIndex - oamMsgWriteIndex)) > 2)))
	{
	  onuEponOamHandleMsg();
	  onuEponDbaSwRprtMacTimerCfg(62500);
	}
  }
  else
  {
	return (MV_ERROR);
  }

  return (MV_OK);
}

/*******************************************************************************
**
**  onuEponDgHandleMsg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function transmit DG OAM frame from SW FIFO
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
//MV_STATUS onuEponDgHandleMsg(void)
//{
//  MV_STATUS status;
//
//  if (oamDgMsgQueue.valid == OAM_MSG_DG_IND)
//  {
//	do
//	{
//	  status = onuEponSendCtrlFrameData(&oamDgMsgQueue.msg, oamDgMsgQueue.macId);
//	}while (status != MV_OK);
//
//	return(status);
//  }
//
//  return(OAM_MSG_NOT_TRANS);
//}

/*******************************************************************************
**
**  onuEponDgAllocMsg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function allocate DG OAM frame for transmission
**
**  PARAMETERS:  S_OnuEponCtrlBuffer *ctrlBuf
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
//MV_STATUS onuEponDgAllocMsg(S_OnuEponCtrlBuffer *ctrlBuf, MV_U32 macId)
//{
//  memcpy(&(oamDgMsgQueue.msg), ctrlBuf, sizeof(S_OnuEponCtrlBuffer));
//  oamDgMsgQueue.macId = macId;
//  oamDgMsgQueue.valid = OAM_MSG_DG_IND;
//
//  return(MV_OK);
//}

/*******************************************************************************
**
**  onuEponDbaModeInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function returns EPON PHY output register value
**
**  PARAMETERS:  dbaMode either ONU_DBA_HW_RPRT_MODE or ONU_DBA_SW_RPRT_MODE
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 status
**
*******************************************************************************/
MV_STATUS onuEponDbaModeInit(MV_32 dbaMode)
{
	MV_U32    macId;
	MV_U32    queueId;
	MV_U32    currentDbaMode;
	MV_U32    interruptMask;
	MV_STATUS status = MV_OK;

	currentDbaMode = onuEponDbOnuDbaModeGet();
	if (currentDbaMode == dbaMode)
		return(MV_OK);

	switch (dbaMode) {

	case ONU_DBA_HW_RPRT_MODE:

		/* Stop SW DBA event miss timer */
		onuEponIsrTimerEventMissStateSet(MV_FALSE);

		/* Clear DBA Report interrupt */
		mvOnuEponMacPonInterruptMaskGet(&interruptMask);
		interruptMask &= ~(ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
		mvOnuEponMacPonInterruptMaskSet(interruptMask);

		/* Configure MAC DBA Report Registers */
		for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {

			status  = mvOnuEponMacTxmCppReportConfig(0xFF,	 /* Queueset2 not supported -
										set to 0xFF (queueReportQ2) */
								 0x1,	 /* Queueset2 not supported -
										set to 0x1, Software (reportBitmapQ2) */
								 0x0,	 /* Queueset1 supported -
										set to 0, means all 8 queues
										are active (queueReport) */
								 0x1,	 /* Number of queueSet -
										support for 1 queueset */
								 0x1,	 /* Queueset1 supported -
										set to 0x1, Software (reportBitmap) */
								 macId);

			status |= mvOnuEponMacTxmCppReportBitMap(0x00FF, macId); /* Queueset1 - 8 queues enabled(0xFF),
											8 queues disabled(0x00) */
			status |= mvOnuEponMacTxmCppReportQueueSet(0x1, macId);	 /* Number of queuesets -
											support for 1 queueset */

			for (queueId = 0; queueId < 7; queueId++)
				status |= mvOnuEponMacTxmCppReportQueueX(queueId, 0x03FF, macId);

			status |= mvOnuEponMacTxmCppReportQueueX(queueId, 0x53FF, macId);
			if (status != MV_OK) {
				mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
					   "ERROR: (%s:%d) mvOnuEponMacTxmCppReport...\n\r", __FILE_DESC__, __LINE__);
				break;
			}
		}

		/* Set HW DBA mode */
		onuEponDbOnuSwRprtTimerTypeSet(ONU_EPON_HW_DBA_RPRT_TIMER);

		/* Start HW DBA Tx module timer */
		onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTxModTimerId));

		/* Enable HW DBA Report message handling */
		onuEponIsrTimerHwReportStateSet(MV_TRUE, /* Enable */
						dbaRprtT0StateVal,
						dbaRprtT0StateInterval,
						dbaRprtT1StateVal,
						dbaRprtT1StateInterval);
		break;

	case ONU_DBA_SW_RPRT_MODE:

		/* Disable HW DBA Report message handling */
		onuEponIsrTimerHwReportStateSet(MV_FALSE,
						dbaRprtT0StateVal,
						dbaRprtT0StateInterval,
						dbaRprtT1StateVal,
						dbaRprtT1StateInterval);

		/* Disable HW DBA Tx module timer */
		onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHwRprtTxModTimerId));

		/* Set SW DBA mode */
		onuEponDbOnuSwRprtTimerTypeSet(ONU_EPON_SW_DBA_RPRT_TIMER);

		/* Set SW DBA report update interval - 1 msec */
		onuEponDbOnuSwRprtMacTimerIntervalSet(62500);
		onuEponDbaSwRprtMacTimerCfg(onuEponDbOnuSwRprtMacTimerIntervalGet());

		/* Set SW DBA Report interrupt */
		mvOnuEponMacPonInterruptMaskGet(&interruptMask);
		interruptMask |= (ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
		mvOnuEponMacPonInterruptMaskSet(interruptMask);

		/* Start SW DBA event miss timer */
		onuEponIsrTimerEventMissStateSet(MV_TRUE);

		for (status = MV_OK, macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
			status |= mvOnuEponMacTxmCppReportConfig(0xFF,	/* Queueset2 not supported -
									 * set to 0xFF (queueReportQ2) */
								 0x1,	/* Queueset2 not supported -
									 * set to 0x1, Software (reportBitmapQ2)
									 */
								 0xFF,	/* Queueset1 supported -
									 * set to 0, means all 8 queues
									 * are active (queueReport) */
								 0x1,	/* Number of queueSet -
									 * support for 1 queueset */
								 0x1,	/* Queueset1 supported -
									 * set to 0x1, Software (reportBitmap)
									 */
								 macId);
		}

		if (status != MV_OK) {
			mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) onuEponDbaModeInit...\n\r", __FILE_DESC__, __LINE__);
			return(MV_ERROR);
		}
		break;

	default:
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
			   "ERROR: (%s:%d) onuEponDbaModeInit: Wrong DBA mode!\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	onuEponDbOnuDbaModeSet(dbaMode);

	return(status);
}

/*******************************************************************************
**
**  onuEponTimerTxPwrHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function start / stop 1 sec timer that will disable Tx if
**               expired
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponTimerTxPwrHndl(unsigned long data)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonTxPwrTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  if (onuEponDbOnuStateGet(0) < ONU_EPON_03_OPERATION)
  {
     onuPonTxPowerOn(MV_FALSE);
  }

  onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonTxPwrTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}


