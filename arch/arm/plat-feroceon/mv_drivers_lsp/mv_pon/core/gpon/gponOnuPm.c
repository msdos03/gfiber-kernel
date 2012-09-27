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
**  FILE        : gponOnuPm.c                                                **
**                                                                           **
**  DESCRIPTION : This file implements ONU GPON Alarm and Statistics         **
**                functionality                                              **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *
* =========================================================================== *
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "gponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/core/gpon/gponOnuPm.c"

/* Global Variables
------------------------------------------------------------------------------*/
/* ONU GPON PM table */
S_GponPm    g_OnuGponPm;
S_PloamSwPm g_OnuGponSwPm;

#ifdef MV_GPON_PERFORMANCE_CHECK
S_GponPerformanceCheck g_GponPmCheck;
#endif /* MV_GPON_PERFORMANCE_CHECK */
extern spinlock_t onuPonIrqLock;

static S_apiGemPortPmConfigDb apiGemPortPmConfigDb;

/* Global functions
------------------------------------------------------------------------------*/
void       onuGponPmInPmInit(void);
MV_STATUS onuGponPmCountersAdd(void);
MV_STATUS onuGponRougeOnuCheck(void);

/******************************************************************************/
/* ========================================================================== */
/*                         Init Section                                       */
/* ========================================================================== */
/******************************************************************************/
#ifdef MV_GPON_PERFORMANCE_CHECK
MV_STATUS onuGponPmInit(void)
{
  MV_U32 index;

  for (index = 0; index < PON_MAX_PERFORMANCE; index++)
  {
    g_GponPmCheck.pmCheckNode[index].uSecCntStart = (MV_U32*)onuPonMemAlloc(256 * sizeof(MV_U32));
    if (g_GponPmCheck.pmCheckNode[index].uSecCntStart == NULL)
      return(MV_ERROR);

    g_GponPmCheck.pmCheckNode[index].uSecCntStop = (MV_U32*)onuPonMemAlloc(256 * sizeof(MV_U32));
    if (g_GponPmCheck.pmCheckNode[index].uSecCntStop == NULL)
      return(MV_ERROR);

    g_GponPmCheck.pmCheckNode[index].uSecCntIdx = 0;
  }

  return(MV_OK);
}
#endif /* MV_GPON_PERFORMANCE_CHECK */

/******************************************************************************/
/* ========================================================================== */
/*                         Statistics Section                                 */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuGponPmInPmInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function init the onu gpon alarm table
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuGponPmInPmInit(void)
{
  /* reset all counters */
  memset(&g_OnuGponPm,   0, sizeof(S_GponPm));
  memset(&g_OnuGponSwPm, 0, sizeof(S_PloamSwPm));
}

/*******************************************************************************
**
**  onuGponPmTimerPmHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is called by the GPON PM timer and trigger the
**               GPON handler to execute PM functionality
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuGponPmTimerPmHndl(unsigned long data)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);
  onuPonResourceTbl_s.onuPonPmTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;
  spin_unlock_irqrestore(&onuPonIrqLock, flags);

  /* Call PM handler */
  onuGponPmTimerExpireHndl();

  spin_lock_irqsave(&onuPonIrqLock, flags);
  if ((onuPonResourceTbl_s.onuPonPmTimerId.onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonPmTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuGponPmTimerExpireHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: This routine execute PM handler functionality
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuGponPmTimerExpireHndl(void)
{
  S_RxBip8Pm inBip8Pm;
  unsigned long flags;

  onuGponPmCountersAdd();
  onuGponPmRxBip8PmGet(&inBip8Pm);
  spin_lock_irqsave(&onuPonIrqLock, flags);
  onuGponBerPeriodPass(inBip8Pm.bip8);
  spin_unlock_irqrestore(&onuPonIrqLock, flags);
  onuGponRougeOnuCheck();
}

/*******************************************************************************
**
**  onuGponRougeOnuCheck
**  ____________________________________________________________________________
**
**  DESCRIPTION: This routine validate rouge ONU statistics values
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponRougeOnuCheck(void)
{
#ifndef PON_FPGA
  MV_STATUS status;
#endif /* PON_FPGA */
  MV_U32    txBurstEnCnt;

  onuGponPmTxBurstEnCntGet(&txBurstEnCnt);

  if (txBurstEnCnt >= GPON_BURST_THRESHOLD)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_PM_MODULE, "===========================\n");
    mvPonPrint(PON_PRINT_ERROR, PON_PM_MODULE, "== ROUGE ONU             ==\n");
    mvPonPrint(PON_PRINT_ERROR, PON_PM_MODULE, "== Shutting Down the XVR ==\n");
    mvPonPrint(PON_PRINT_ERROR, PON_PM_MODULE, "===========================\n");

#ifndef PON_FPGA
    /* Set the output enable of the GP_BEN IO to Input */
    status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_INPUT, 0);
    if (status != MV_OK)
      return(status);
#endif /* PON_FPGA */
  }

  return(MV_OK);
}


/*******************************************************************************
**
**  ponOltPmOnuCountersAdd
**  ____________________________________________________________________________
**
**  DESCRIPTION: This routine read statistics values from ASIC to the database
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmCountersAdd(void)
{
	MV_U32  tempCounter1;
	MV_U32  tempCounter2;
	MV_U32  tempCounter3;
	MV_U32  tempCounter4;
	MV_U32  tcont;
	MV_BOOL exist;
	MV_STATUS status;

	/* BIP8 Counter */
	/* ============ */
	mvOnuGponMacBipStatusValueGet(&tempCounter1);
	g_OnuGponPm.rxBip8.bip8 += tempCounter1;

	/* Ploam Counters */
	/* ============== */
	mvOnuGponMacRxPloamLostCrcCounterGet(&tempCounter1);				 /* Error CRC */
	g_OnuGponPm.rxPloam.crcErrorPloamCounter += tempCounter1;
	mvOnuGponMacRxPloamLostFullCounterGet(&tempCounter1);				 /* Downstream lost due to full FIFO */
	g_OnuGponPm.rxPloam.fifoOverErrorPloamCounter += tempCounter1;
	mvOnuGponMacRxPloamRcvdIdleCounterGet(&tempCounter1);				 /* Downstream received idle */
	g_OnuGponPm.rxPloam.idlePloamCounter += tempCounter1;
	mvOnuGponMacRxPloamRcvdBroadCounterGet(&tempCounter1);				 /* Downstream received broadcast */
	g_OnuGponPm.rxPloam.receivedBroadcastPloamCounter += tempCounter1;
	mvOnuGponMacRxPloamRcvdOnuIdCounterGet(&tempCounter1);				 /* Downstream received ONU ID */
	g_OnuGponPm.rxPloam.receivedOnuIdPloamCounter += tempCounter1;

	/* BW MAP counters  */
	/* =============== */
	mvOnuGponMacRxBwMapAllocatrionsCounterGet(&tempCounter1, &tempCounter2, &tempCounter3, &tempCounter4);
	g_OnuGponPm.rxBwMap.allocCorrec += tempCounter1;				   /* Received allocations errors free */
	g_OnuGponPm.rxBwMap.allocUnCorrectableCrcErr += tempCounter2;	   /* Received allocations with uncorrectable CRC errors */
	g_OnuGponPm.rxBwMap.allocCorrectableCrcErr += tempCounter3;		   /* Received allocations with correctable CRC errors or errors free */
	g_OnuGponPm.rxBwMap.allocCrcErr += tempCounter4;				   /* Received allocations with CRC error */
	mvOnuGponMacRxBwMapTotalBwGet(&tempCounter1);						 /* Total received byte allocations */
	g_OnuGponPm.rxBwMap.totalReceivedAllocBytes += tempCounter1;

	/* PLEND unusable counter */
	/* ====================== */
	mvOnuGponMacInComingPlendCntGet(&tempCounter1);
	g_OnuGponPm.rxPlend.plend += tempCounter1;

	/* FEC Counters */
	/* ============ */
	mvOnuGponMacRxFecBytesCounterGet(&tempCounter1);					 /* Received bytes */
	g_OnuGponPm.rxFec.receivedBytes += tempCounter1;
	mvOnuGponMacRxFecCorrectCounterGet(&tempCounter1, &tempCounter2);
	g_OnuGponPm.rxFec.correctedBits += tempCounter2;				   /* Received corrected bits */
	g_OnuGponPm.rxFec.correctedBytes += tempCounter1;				   /* Received corrected bytes */
	mvOnuGponMacRxFecInorrectCounterGet(&tempCounter1, &tempCounter2);
	g_OnuGponPm.rxFec.receivedCodeWords += tempCounter2;			   /* Received code words */
	g_OnuGponPm.rxFec.uncorrectedCodeWords += tempCounter1;			   /* Received corrected code words */

	/* GEM Counters */
	/* ============ */
	mvOnuGponMacGemRcvdIdleGemFramesCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedIdleGemFrames += tempCounter1;
	mvOnuGponMacGemRcvdValidGemFramesCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedValidGemFrames += tempCounter1;
	mvOnuGponMacGemRcvdUndefGemFramesCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedUndefinedGemFrames += tempCounter1;
	mvOnuGponMacGemRcvdOmciFramesCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedOmciFrames += tempCounter1;
	mvOnuGponMacGemDropGemFramesCounterGet(&tempCounter1);
	g_OnuGponPm.gem.droppedGemFrames += tempCounter1;
	mvOnuGponMacGemDropOmciFramesCounterGet(&tempCounter1);
	g_OnuGponPm.gem.droppedOmciFrames += tempCounter1;
	mvOnuGponMacGemRcvdGemFramesWithUncorrHecErrCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedGemFramesWithUncorrHecErr += tempCounter1;
	mvOnuGponMacGemRcvdGemFramesWithOneFixedHecErrCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedGemFramesWithOneFixedHecErr += tempCounter1;
	mvOnuGponMacGemRcvdGemFramesWithTwoFixedHecErrCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedGemFramesWithTwoFixedHecErr += tempCounter1;
	mvOnuGponMacGemRcvdValidGemFramesTotalByteCounterGet(&tempCounter1);
	g_OnuGponPm.gem.totalByteCountOfReceivedValidGemFrames += tempCounter1;
	mvOnuGponMacGemRcvdUndefGemFramesTotalByteCounterGet(&tempCounter1);
	g_OnuGponPm.gem.totalByteCountOfReceivedUndefinedGemFrames += tempCounter1;
	mvOnuGponMacGemReassembleMemoryFlushCounterGet(&tempCounter1);
	g_OnuGponPm.gem.gemReassembleMemoryFlush += tempCounter1;
	mvOnuGponMacGemSynchLostCounterGet(&tempCounter1);
	g_OnuGponPm.gem.gemSynchLost += tempCounter1;
	mvOnuGponMacGemRcvdEthFramesWithCorrFcsCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedEthFramesWithCorrFcs += tempCounter1;
	mvOnuGponMacGemRcvdEthFramesWithFcsErrCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedEthFramesWithFcsError += tempCounter1;
	mvOnuGponMacGemRcvdOmciFramesWithCorrCrcCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedOmciFramesWithCorrCrc += tempCounter1;
	mvOnuGponMacGemRcvdOmciFramesWithCrcErrCounterGet(&tempCounter1);
	g_OnuGponPm.gem.receivedOmciFramesWithCrcError += tempCounter1;

	/* Tx Counters */
	/* =========== */
	mvOnuGponMacTxGemPtiTypeOneFrameCounterGet(&tempCounter1);
	g_OnuGponPm.tx.transmittedGemPtiTypeOneFrames += tempCounter1;
	mvOnuGponMacTxGemPtiTypeZeroFrameCounterGet(&tempCounter1);
	g_OnuGponPm.tx.transmittedGemPtiTypeZeroFrames += tempCounter1;
	mvOnuGponMacTxIdleGemFramesCounterGet(&tempCounter1);
	g_OnuGponPm.tx.transmittedIdleGemFrames += tempCounter1;
	mvOnuGponMacTxTxEnableCounterGet(&tempCounter1);
	g_OnuGponPm.tx.transmittedTxEnableCount += tempCounter1;

	for (tcont = 0; tcont < ONU_GPON_MAX_NUM_OF_T_CONTS; tcont++) {
		status = onuGponDbBwTcontExist(tcont, &exist);
		if ((status == MV_OK) && (exist != MV_FALSE)) {
			mvOnuGponMacTxEthFramesTcontiCounterGet(tcont, &tempCounter1);
			g_OnuGponPm.tx.transmittedEthFramesViaTconti[tcont] += tempCounter1;
			mvOnuGponMacTxEthBytesTcontiCounterGet(tcont, &tempCounter1);
			g_OnuGponPm.tx.transmittedEthBytesViaTconti[tcont] += tempCounter1;
			mvOnuGponMacTxGemFramesTcontiCounterGet(tcont, &tempCounter1);
			g_OnuGponPm.tx.transmittedGemFramesViaTconti[tcont] += tempCounter1;
			mvOnuGponMacTxIdleGemFramesTcontiCounterGet(tcont, &tempCounter1);
			g_OnuGponPm.tx.transmittedIdleGemFramesViaTconti[tcont] += tempCounter1;
		}
	}

	/* Sw Counters */
	/* =========== */
	onuGponPmSwCountersUpdate(&g_OnuGponPm.rxPloam, &g_OnuGponPm.txPloam);

	return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmRxSwCountersAdd
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function updates Rx Ploam message counters
**
**  PARAMETERS:  MV_U8 a_msgId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmRxSwCountersAdd(MV_U8 a_msgId)
{
  g_OnuGponSwPm.rxMsgTotalPloamCounter++;
  g_OnuGponSwPm.rxMsgIdPloamCounter[a_msgId]++;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmSwCountersUpdate
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function updates Rx Ploam message counters
**
**  PARAMETERS:  MV_U8 a_msgId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmSwCountersUpdate(S_RxPloamPm *rxPloamPm, S_TxPloamPm *txPloamPm)
{
  MV_U32 numOfMsgIdIndex;

  /* Rx PLOAM */
  rxPloamPm->rxMsgTotalPloamCounter = g_OnuGponSwPm.rxMsgTotalPloamCounter;

  for (numOfMsgIdIndex = ONU_GPON_DS_MSG_OVERHEAD;
       numOfMsgIdIndex <= ONU_GPON_DS_MSG_EXT_BURST_LEN;
       numOfMsgIdIndex++)
  {
    rxPloamPm->rxMsgIdPloamCounter[numOfMsgIdIndex] = g_OnuGponSwPm.rxMsgIdPloamCounter[numOfMsgIdIndex];
  }

  mvOnuGponMacTxSwCountersGet((void*)txPloamPm);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmFecPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Rx FEC counters
**
**  PARAMETERS:  S_RxFecPm *fecPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmFecPmGet(S_RxFecPm *fecPm)
{
  fecPm->receivedBytes        = g_OnuGponPm.rxFec.receivedBytes;
  fecPm->correctedBytes       = g_OnuGponPm.rxFec.correctedBytes;
  fecPm->correctedBits        = g_OnuGponPm.rxFec.correctedBits ;
  fecPm->receivedCodeWords    = g_OnuGponPm.rxFec.receivedCodeWords;
  fecPm->uncorrectedCodeWords = g_OnuGponPm.rxFec.uncorrectedCodeWords;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmRxPloamPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Rx PLOAM counters
**
**  PARAMETERS:  S_RxPloamPm *rxPloamPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmRxPloamPmGet(S_RxPloamPm *rxPloamPm)
{
  MV_U32 numOfMsgIdIndex;

  rxPloamPm->crcErrorPloamCounter          = g_OnuGponPm.rxPloam.crcErrorPloamCounter;
  rxPloamPm->receivedBroadcastPloamCounter = g_OnuGponPm.rxPloam.receivedBroadcastPloamCounter;
  rxPloamPm->receivedOnuIdPloamCounter     = g_OnuGponPm.rxPloam.receivedOnuIdPloamCounter;
  rxPloamPm->fifoOverErrorPloamCounter     = g_OnuGponPm.rxPloam.fifoOverErrorPloamCounter;
  rxPloamPm->idlePloamCounter              = g_OnuGponPm.rxPloam.idlePloamCounter;
  rxPloamPm->rxMsgTotalPloamCounter        = g_OnuGponPm.rxPloam.rxMsgTotalPloamCounter;

  for (numOfMsgIdIndex = 0 ;numOfMsgIdIndex <= ONU_GPON_DS_MSG_LAST;numOfMsgIdIndex++)
  {
    rxPloamPm->rxMsgIdPloamCounter[numOfMsgIdIndex] = g_OnuGponPm.rxPloam.rxMsgIdPloamCounter[numOfMsgIdIndex];
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmTxPloamPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Tx PLOAM counters
**
**  PARAMETERS:  S_TxPloamPm *txPloamPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmTxPloamPmGet(S_TxPloamPm *txPloamPm)
{
  MV_U32 numOfMsgIdIndex;

  txPloamPm->txMsgTotalPloamCounter = g_OnuGponPm.txPloam.txMsgTotalPloamCounter;

  for (numOfMsgIdIndex = 1; numOfMsgIdIndex <= ONU_GPON_US_MSG_LAST; numOfMsgIdIndex++ )
  {
    txPloamPm->txMsgIdPloamCounter[numOfMsgIdIndex] = g_OnuGponPm.txPloam.txMsgIdPloamCounter[numOfMsgIdIndex];

    txPloamPm->txErrMsgIdPloamCounter[numOfMsgIdIndex] = g_OnuGponPm.txPloam.txErrMsgIdPloamCounter[numOfMsgIdIndex];
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmRxBwMapCountersAdd
**  ____________________________________________________________________________
**
**  DESCRIPTION: This routine read BW MAP counters from ASIC to the database
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmRxBwMapCountersAdd(void)
{
  MV_U32  tempCounter1;
  MV_U32  tempCounter2;
  MV_U32  tempCounter3;
  MV_U32  tempCounter4;

  mvOnuGponMacRxBwMapAllocatrionsCounterGet(&tempCounter1, &tempCounter2, &tempCounter3, &tempCounter4);
  g_OnuGponPm.rxBwMap.allocCorrec              += tempCounter1;      /* Received allocations errors free */
  g_OnuGponPm.rxBwMap.allocUnCorrectableCrcErr += tempCounter2;      /* Received allocations with uncorrectable CRC errors */
  g_OnuGponPm.rxBwMap.allocCorrectableCrcErr   += tempCounter3;      /* Received allocations with correctable CRC errors or errors free */
  g_OnuGponPm.rxBwMap.allocCrcErr              += tempCounter4;      /* Received allocations with CRC error */
  mvOnuGponMacRxBwMapTotalBwGet(&tempCounter1);                     /* Total received byte allocations */
  g_OnuGponPm.rxBwMap.totalReceivedAllocBytes  += tempCounter1;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmRxBwMapPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Rx BW MAP counters
**
**  PARAMETERS:  S_RxPloamPm *rxPloamPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmRxBwMapPmGet(S_RxBwMapPm *rxBwMapPm)
{
  rxBwMapPm->allocCorrec              = g_OnuGponPm.rxBwMap.allocCorrec;
  rxBwMapPm->allocCorrectableCrcErr   = g_OnuGponPm.rxBwMap.allocCorrectableCrcErr;
  rxBwMapPm->allocCrcErr              = g_OnuGponPm.rxBwMap.allocCrcErr;
  rxBwMapPm->allocUnCorrectableCrcErr = g_OnuGponPm.rxBwMap.allocUnCorrectableCrcErr;
  rxBwMapPm->totalReceivedAllocBytes  = g_OnuGponPm.rxBwMap.totalReceivedAllocBytes;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmRxPlendPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Rx Plend counter
**
**  PARAMETERS:  S_RxPlendPm *rxPlendPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmRxPlendPmGet(S_RxPlendPm *rxPlendPm)
{
  rxPlendPm->plend = g_OnuGponPm.rxPlend.plend;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmRxBip8PmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Rx Bip8 counter
**
**  PARAMETERS:  S_RxBip8Pm *rxBip8Pm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmRxBip8PmGet(S_RxBip8Pm *rxBip8Pm)
{
  rxBip8Pm->bip8 = g_OnuGponPm.rxBip8.bip8;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmGemPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Gem pm counters
**
**  PARAMETERS:  S_GemPm *gemPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmGemPmGet(S_GemPm *gemPm)
{
  gemPm->receivedIdleGemFrames                       = g_OnuGponPm.gem.receivedIdleGemFrames;
  gemPm->receivedValidGemFrames                      = g_OnuGponPm.gem.receivedValidGemFrames;
  gemPm->receivedUndefinedGemFrames                  = g_OnuGponPm.gem.receivedUndefinedGemFrames;
  gemPm->receivedOmciFrames                          = g_OnuGponPm.gem.receivedOmciFrames;
  gemPm->droppedGemFrames                            = g_OnuGponPm.gem.droppedGemFrames;
  gemPm->droppedOmciFrames                           = g_OnuGponPm.gem.droppedOmciFrames;
  gemPm->receivedGemFramesWithUncorrHecErr           = g_OnuGponPm.gem.receivedGemFramesWithUncorrHecErr;
  gemPm->receivedGemFramesWithOneFixedHecErr         = g_OnuGponPm.gem.receivedGemFramesWithOneFixedHecErr;
  gemPm->receivedGemFramesWithTwoFixedHecErr         = g_OnuGponPm.gem.receivedGemFramesWithTwoFixedHecErr;
  gemPm->totalByteCountOfReceivedValidGemFrames      = g_OnuGponPm.gem.totalByteCountOfReceivedValidGemFrames;
  gemPm->totalByteCountOfReceivedUndefinedGemFrames  = g_OnuGponPm.gem.totalByteCountOfReceivedUndefinedGemFrames;
  gemPm->gemReassembleMemoryFlush                    = g_OnuGponPm.gem.gemReassembleMemoryFlush;
  gemPm->gemSynchLost                                = g_OnuGponPm.gem.gemSynchLost;
  gemPm->receivedEthFramesWithCorrFcs                = g_OnuGponPm.gem.receivedEthFramesWithCorrFcs;
  gemPm->receivedEthFramesWithFcsError               = g_OnuGponPm.gem.receivedEthFramesWithFcsError;
  gemPm->receivedOmciFramesWithCorrCrc               = g_OnuGponPm.gem.receivedOmciFramesWithCorrCrc;
  gemPm->receivedOmciFramesWithCrcError              = g_OnuGponPm.gem.receivedOmciFramesWithCrcError;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmTxPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Tx pm counters
**
**  PARAMETERS:  S_TxPm *txPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmTxPmGet(S_GponTxPm *txPm)
{
	MV_U32 tcont;
	MV_BOOL   exist;
	MV_STATUS status;

	txPm->transmittedGemPtiTypeOneFrames    = g_OnuGponPm.tx.transmittedGemPtiTypeOneFrames;
	txPm->transmittedGemPtiTypeZeroFrames   = g_OnuGponPm.tx.transmittedGemPtiTypeZeroFrames;
	txPm->transmittedIdleGemFrames          = g_OnuGponPm.tx.transmittedIdleGemFrames;
	txPm->transmittedTxEnableCount          = g_OnuGponPm.tx.transmittedTxEnableCount;

	for (tcont = 0; tcont < ONU_GPON_MAX_NUM_OF_T_CONTS; tcont++) {
		status = onuGponDbBwTcontExist(tcont, &exist);
		if ((status == MV_OK) && (exist != MV_FALSE)) {
			txPm->transmittedEthFramesViaTconti[tcont]     = g_OnuGponPm.tx.transmittedEthFramesViaTconti[tcont];
			txPm->transmittedEthBytesViaTconti[tcont]      = g_OnuGponPm.tx.transmittedEthBytesViaTconti[tcont];
			txPm->transmittedGemFramesViaTconti[tcont]     = g_OnuGponPm.tx.transmittedGemFramesViaTconti[tcont];
			txPm->transmittedIdleGemFramesViaTconti[tcont] = g_OnuGponPm.tx.transmittedIdleGemFramesViaTconti[tcont];
		}
	}

	return(MV_OK);
}

/*******************************************************************************
**
**  onuGponPmTxBurstEnCntGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Tx burst enable counter
**
**  PARAMETERS:  unsigned int *txBurstEnCnt
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponPmTxBurstEnCntGet(unsigned int *txBurstEnCnt)
{
  *txBurstEnCnt = g_OnuGponPm.tx.transmittedTxEnableCount;

  g_OnuGponPm.tx.transmittedTxEnableCount = 0;

  return(MV_OK);
}



/******************************************************************************/
/* ========================================================================== */
/*                         GEM Port PM Section                                */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuGponPmGemPortPmInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function clears GEM Port PM database
**
**  PARAMETERS:  void
**
**  OUTPUTS:     None
**
**  RETURNS:     void
**
*******************************************************************************/
void onuGponPmGemPortPmInit(void)
{
  int       indx;

  for (indx = 0; indx < MAX_ENTRIES_GEMPORTPM_CONFIG_DB; indx++)
  {
    apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx].in_use = MV_FALSE;
  }

  if (asicOntMiscRegRead(mvAsicReg_RX_MIB_DEFAULT, &apiGemPortPmConfigDb.origRxMibDefaultReg, 0) != MV_OK)
  {
    printk("%s: asicOntMiscRegRead failed for mvAsicReg_RX_MIB_DEFAULT(%d)\n", __FUNCTION__, mvAsicReg_RX_MIB_DEFAULT);
  }
}


/*******************************************************************************
**
**  onuGponPmGemPortPmGetNumUsedEntries
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function counts the number of used GEM port entries in the DB
**
**  PARAMETERS:  void
**
**  OUTPUTS:     None
**
**  RETURNS:     int
**
*******************************************************************************/
int onuGponPmGemPortPmGetNumUsedEntries(void)
{
  int       indx;
  int       usedEntries = 0;

  for (indx = 0; indx < MAX_ENTRIES_GEMPORTPM_CONFIG_DB; indx++)
  {
    if (apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx].in_use == MV_TRUE)
      usedEntries++;
  }
  return usedEntries;
}


/*******************************************************************************
**
**  onuGponPmGemPortPmStart
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function activates MIB Counter PM for a GEM port
**
**  PARAMETERS:  S_GponIoctlGemPortPmConfig *
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponPmGemPortPmStart(MV_U16 gemPort)
{
  MV_STATUS            rcode = MV_OK;
  int                  indx;
  S_apiGemPortPmConfig *apiGemPortPmConfig;
  S_apiGemPortPmConfig *tgtApiGemPortPmConfig = 0;
  int                  freeIndx;
  MV_U32               mibCtrlValue;

  // Does the GEM Port already appear in table
  for (indx = 0; indx < MAX_SUPPORTED_GEMPORTS_FOR_PM; indx++)
  {
    apiGemPortPmConfig = &apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx];
    if (apiGemPortPmConfig->in_use == MV_TRUE && apiGemPortPmConfig->gem_port == gemPort)
    {
        printk("%s: gemPort %d already in table (start)\n", __FUNCTION__, gemPort);
        return MV_ALREADY_EXIST;
    }
  }

  for (indx = 0; indx < MAX_SUPPORTED_GEMPORTS_FOR_PM; indx++)
  {
    apiGemPortPmConfig = &apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx];
    if (apiGemPortPmConfig->in_use == MV_FALSE)
    {
      tgtApiGemPortPmConfig = apiGemPortPmConfig;
      freeIndx              = indx;
      break;
    }
  }

  // Did we find a free entry
  if (tgtApiGemPortPmConfig == 0)
  {
      printk("%s: Table full. gemPort = %d (start)\n", __FUNCTION__, gemPort);
      return MV_NO_MORE;
  }

  if (onuGponPmGemPortPmGetNumUsedEntries() == 0)
  {
    if ((rcode = asicOntMiscRegRead(mvAsicReg_RX_MIB_DEFAULT, &apiGemPortPmConfigDb.origRxMibDefaultReg, 0)) != MV_OK)
    {
      printk("%s: asicOntMiscRegRead failed for mvAsicReg_RX_MIB_DEFAULT(%d)\n", __FUNCTION__, mvAsicReg_RX_MIB_DEFAULT);
      return rcode;
    }
    mibCtrlValue = MAKE_MIB_COUNTER_CONTROL(MIB_COUNTER_CONTROL_INVALID_BIT,0,0);
    if ((rcode = asicOntMiscRegWrite(mvAsicReg_RX_MIB_DEFAULT, mibCtrlValue, 0)) != MV_OK)
    {
      printk("%s: asicOntMiscRegWrite failed for mvAsicReg_RX_MIB_DEFAULT(%d)\n", __FUNCTION__, mvAsicReg_RX_MIB_DEFAULT);
      return rcode;
    }
  }

  mibCtrlValue = MAKE_MIB_COUNTER_CONTROL(MIB_COUNTER_CONTROL_VALID_BIT,freeIndx,gemPort);
  if ((rcode = asicOntMiscRegWrite(mvAsicReg_RX_MIB_CTRL, mibCtrlValue, indx)) != MV_OK)
  {
      printk("%s: asicOntMiscRegWrite failed for indx = %d (start)\n", __FUNCTION__, indx);
  }
  else
  {
    tgtApiGemPortPmConfig->gem_port    = gemPort;
    tgtApiGemPortPmConfig->counter_set = freeIndx;
    tgtApiGemPortPmConfig->in_use      = MV_TRUE;
  }
  return rcode;
}


/*******************************************************************************
**
**  onuGponPmGemPortPmStop
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function activates MIB Counter PM for a GEM port
**
**  PARAMETERS:  S_GponIoctlGemPortPmConfig *
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS  onuGponPmGemPortPmStop(MV_U16 gemPort)
{
  MV_STATUS            rcode = MV_OK;
  int                  indx;
  S_apiGemPortPmConfig *apiGemPortPmConfig;
  S_apiGemPortPmConfig *tgtApiGemPortPmConfig = 0;
  MV_U32               mibCtrlValue;

  // Does the GEM Port already appear in table
  for (indx = 0; indx < MAX_SUPPORTED_GEMPORTS_FOR_PM; indx++)
  {
    apiGemPortPmConfig = &apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx];
    if (apiGemPortPmConfig->in_use == MV_TRUE && apiGemPortPmConfig->gem_port == gemPort)
    {
      tgtApiGemPortPmConfig = apiGemPortPmConfig;
      break;
    }
  }

  // Did we find a free entry
  if (tgtApiGemPortPmConfig == 0)
  {
      printk("%s: gemPort %d not in table (stop)\n", __FUNCTION__, gemPort);
      return MV_NOT_FOUND;
  }

  mibCtrlValue = MAKE_MIB_COUNTER_CONTROL(MIB_COUNTER_CONTROL_INVALID_BIT,0,0);
  if ((rcode = asicOntMiscRegWrite(mvAsicReg_RX_MIB_CTRL, mibCtrlValue, indx)) != MV_OK)
  {
    printk("%s: asicOntMiscRegWrite failed for indx = %d (stop)\n", __FUNCTION__, indx);
    return rcode;
  }
  else
  {
    tgtApiGemPortPmConfig->in_use = MV_FALSE;
  }

  // If no more GEM ports are being monitored, restore the original RX_MIB_DEFAULT register value
  if (onuGponPmGemPortPmGetNumUsedEntries() == 0)
  {
    if ((rcode = asicOntMiscRegWrite(mvAsicReg_RX_MIB_DEFAULT, apiGemPortPmConfigDb.origRxMibDefaultReg, 0)) != MV_OK)
    {
      printk("%s: asicOntMiscRegWrite failed for mvAsicReg_RX_MIB_DEFAULT(%d)\n", __FUNCTION__, mvAsicReg_RX_MIB_DEFAULT);
      return rcode;
    }
  }

  return rcode;
}

/*******************************************************************************
**
**  onuGponPmGemPortPmReset
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function clears HW register for PM collection for GEM Port
**
**  PARAMETERS:  void
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponPmGemPortPmReset(void)
{
  MV_STATUS            rcode;
  int                  indx;
  uint32_t             mibCtrlValue = MAKE_MIB_COUNTER_CONTROL(MIB_COUNTER_CONTROL_INVALID_BIT,0,0);

  for (indx = 0; indx < MAX_ENTRIES_GEMPORTPM_CONFIG_DB; indx++)
  {
    // Clear register (indx)
    apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx].in_use = MV_FALSE;
    if ((rcode = asicOntMiscRegWrite(mvAsicReg_RX_MIB_CTRL, mibCtrlValue, indx)) != MV_OK)
    {
      printk("%s: asicOntMiscRegWrite failure, rcode = %d, indx = %d\n", __FUNCTION__, rcode, indx);
      return rcode;
    }
  }

  if ((rcode = asicOntMiscRegWrite(mvAsicReg_RX_MIB_DEFAULT, apiGemPortPmConfigDb.origRxMibDefaultReg, 0)) != MV_OK)
  {
    printk("%s: asicOntMiscRegWrite failed for mvAsicReg_RX_MIB_DEFAULT(%d)\n", __FUNCTION__, mvAsicReg_RX_MIB_DEFAULT);
    return rcode;
  }

  return MV_OK;
}


/*******************************************************************************
**
**  onuGponPmGemPortPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function retrieves MIB Counter PM for GEM Port
**
**  PARAMETERS:  S_GponIoctlGemPortMibCounters *
**
**  OUTPUTS:     Reads MIB counters from HW
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponPmGemPortPmGet(MV_U16 gemPort, S_GponIoctlGemPortMibCounters *gemPortMibCounters)
{
  MV_STATUS            rcode = MV_OK;
  int                  indx;
  S_apiGemPortPmConfig *apiGemPortPmConfig;
  S_apiGemPortPmConfig *tgtApiGemPortPmConfig = 0;
  MV_U32               baseCounterReg;
  MV_U32               lowvalue;
  MV_U32               hivalue;
  MV_U32               registersAra[] =
  {
    mvAsicReg_PON_MAC_MIB_COUNTERS_0, mvAsicReg_PON_MAC_MIB_COUNTERS_1, mvAsicReg_PON_MAC_MIB_COUNTERS_2, mvAsicReg_PON_MAC_MIB_COUNTERS_3,
    mvAsicReg_PON_MAC_MIB_COUNTERS_4, mvAsicReg_PON_MAC_MIB_COUNTERS_5, mvAsicReg_PON_MAC_MIB_COUNTERS_6, mvAsicReg_PON_MAC_MIB_COUNTERS_7
  };

  for (indx = 0; indx < MAX_SUPPORTED_GEMPORTS_FOR_PM; indx++)
  {
    apiGemPortPmConfig = &apiGemPortPmConfigDb.apiGemPortPmConfigAra[indx];
    if (apiGemPortPmConfig->in_use == MV_TRUE && apiGemPortPmConfig->gem_port == gemPort)
    {
      baseCounterReg = registersAra[apiGemPortPmConfig->counter_set];
      tgtApiGemPortPmConfig = apiGemPortPmConfig;
      break;
    }
  }

  if (tgtApiGemPortPmConfig != 0)
  {
//    printk("%s: gemPort %d, counterSet %d, baseCounterReg %d\n",
//           __FUNCTION__, tgtApiGemPortPmConfig->gem_port, tgtApiGemPortPmConfig->counter_set, baseCounterReg);

    // Read PM MIB Counters from counter_set (indx)

    // 64 bit counter treatment: read the low part first, to lock the high part
    rcode |= asicOntMiscRegRead(baseCounterReg, &lowvalue, ONU_GPON_MIB_PM_GoodOctetsReceived_low );
    rcode |= asicOntMiscRegRead(baseCounterReg, &hivalue,  ONU_GPON_MIB_PM_GoodOctetsReceived_high);
    gemPortMibCounters->good_octets_received = (uint64_t) hivalue << 32 | lowvalue;

    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->bad_octets_received,       ONU_GPON_MIB_PM_BadOctetsReceived);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->mac_trans_error,           ONU_GPON_MIB_PM_MACTransError);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->good_frames_received,      ONU_GPON_MIB_PM_GoodFramesReceived);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->bad_frames_received,       ONU_GPON_MIB_PM_BadFramesReceived);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->broadcast_frames_received, ONU_GPON_MIB_PM_BroadcastFramesReceived);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->multicast_frames_received, ONU_GPON_MIB_PM_MulticastFramesReceived);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->frames_64_octets,          ONU_GPON_MIB_PM_Frames64Octets);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->frames_65_to_127_octets,   ONU_GPON_MIB_PM_Frames65to127Octets);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->frames_128_to_255octets,   ONU_GPON_MIB_PM_Frames128to255Octets);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->frames_256_to_511_octets,  ONU_GPON_MIB_PM_Frames256to511Octets);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->frames_512_to_1023_octets, ONU_GPON_MIB_PM_Frames512to1023Octets);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->frames_1024_to_max_octets, ONU_GPON_MIB_PM_Frames1024toMaxOctets);

    // 64 bit counter treatment: read the low part first, to lock the high part
    rcode |= asicOntMiscRegRead(baseCounterReg, &lowvalue, ONU_GPON_MIB_PM_GoodOctetsSent_low);
    rcode |= asicOntMiscRegRead(baseCounterReg, &hivalue,  ONU_GPON_MIB_PM_GoodOctetsSent_high);
    gemPortMibCounters->good_octets_sent = (uint64_t) hivalue << 32 | lowvalue;

    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->good_frames_sent, ONU_GPON_MIB_PM_GoodFramesSent);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->multicast_frames_sent, ONU_GPON_MIB_PM_MulticastFramesSent);
    rcode |= asicOntMiscRegRead(baseCounterReg, &gemPortMibCounters->broadcast_frames_sent, ONU_GPON_MIB_PM_BroadcastFramesSent);

    if (rcode != MV_OK)
    {
      printk("%s: asicOntMiscRegRead failure, rcode = %d, baseCounterReg = %d\n", __FUNCTION__, rcode, baseCounterReg);
    }
  }
  else
  {
    printk("%s: gemPort %d not in table\n", __FUNCTION__, gemPort);
    return MV_NOT_FOUND;
  }

  return rcode;
}

/*******************************************************************************
**
**  onuGponPmGemPortPmConfigGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function returns configured GEM port data
**
**  PARAMETERS:  Index in the GEM port DB
**
**  OUTPUTS:     S_apiGemPortPmConfig
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuGponPmGemPortPmConfigGet(MV_U16 dbIdx, S_apiGemPortPmConfig *gemPortPmConfig)
{
  memset (gemPortPmConfig, 0, sizeof(S_apiGemPortPmConfig));

  if ( dbIdx >= MAX_SUPPORTED_GEMPORTS_FOR_PM )
  {
      printk("%s: Invalid index = %d\n", __FUNCTION__, dbIdx);
      return MV_OUT_OF_RANGE;
  }

  if ( apiGemPortPmConfigDb.apiGemPortPmConfigAra[dbIdx].in_use == MV_FALSE )
      return MV_GET_ERROR;

  memcpy (gemPortPmConfig, &apiGemPortPmConfigDb.apiGemPortPmConfigAra[dbIdx], sizeof(S_apiGemPortPmConfig));

  return MV_OK;
}


