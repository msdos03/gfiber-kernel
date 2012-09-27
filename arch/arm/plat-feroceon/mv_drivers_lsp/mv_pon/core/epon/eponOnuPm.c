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
/**  FILE        : eponOnuPm.c                                               **/
/**                                                                          **/
/**  DESCRIPTION : This file implements ONU EPON Alarm and Statistics        **/
/**                functionality                                             **/
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
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuPm.c"

#define POLYNOMIAL (0x04c11db7)

/* Macro based on "TXQn Number Of Bytes" register group offset formula (registers TXQ0Bytes, TXQ1Bytes,...TXQ7Bytes) */
#define MV_PON_GUNIT_TX_N_QUEUES(llid, queue)	(0xA5900 + (llid%2) * 0x400 + (llid/2) * 0x2000 + queue * 4)


/* Global Variables
------------------------------------------------------------------------------*/
S_EponPm g_OnuEponPm[EPON_MAX_MAC_NUM];
S_SwPm   g_OnuEponSwPm[EPON_MAX_MAC_NUM];
MV_U32   g_OnuEponSwPmAddCntPerLlidAndQueue[EPON_MAX_MAC_NUM][EPON_MAX_QUEUE];
MV_U32   g_OnuEponSwPmAddCntPerLlidFifo[EPON_MAX_MAC_NUM];

#define GPON_RANDOM_ARR_SZ	8
#define GPON_RANDOM_VAL_ARR_SZ	256
MV_U32   g_OnuEponRandomNum[GPON_RANDOM_ARR_SZ] = {0, 0, 0, 0, 0, 0, 0, 0};
MV_U32   g_OnuEponRandomCount = 0;
MV_U32   g_OnuEponRandomValues[GPON_RANDOM_VAL_ARR_SZ] = {0};

static MV_U32 crc_table[256];

extern spinlock_t onuPonIrqLock;

/* Global functions
------------------------------------------------------------------------------*/
MV_STATUS onuEponPmSwCountersUpdate(S_SwPm *swPm, MV_U32 macId);

/******************************************************************************/
/* ========================================================================== */
/*                         Statistics Section                                 */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPmInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function init the onu epon pm table
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**                   
*******************************************************************************/
void onuEponPmInit(void)
{
  MV_U32 index;
  
  /* reset all counters */
  memset(&g_OnuEponPm,   0, sizeof(S_EponPm) * EPON_MAX_MAC_NUM);
  memset(&g_OnuEponSwPm, 0, sizeof(S_SwPm)   * EPON_MAX_MAC_NUM);
  memset(g_OnuEponSwPmAddCntPerLlidAndQueue, 0, (sizeof(MV_U32) * EPON_MAX_MAC_NUM * EPON_MAX_QUEUE));

  for (index = 0; index < EPON_MAX_MAC_NUM; index++) 
    g_OnuEponSwPmAddCntPerLlidFifo[index] = 4000;  

  /* generate crc table */
  onuEponPmGenCrcTable();

  mvOnuEponMacGenLocalTimeStampGet(&g_OnuEponRandomNum[0]);
}

/*******************************************************************************
**
**  onuEponPmTimerPmHndl
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function is called by the EPON PM timer
**               
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**
*******************************************************************************/
void onuEponPmTimerPmHndl(unsigned long data)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);
  onuPonResourceTbl_s.onuPonPmTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;
  spin_unlock_irqrestore(&onuPonIrqLock, flags);

  /* Call PM handler */
  onuEponPmTimerExpireHndl();

  spin_lock_irqsave(&onuPonIrqLock, flags);
  if ((onuPonResourceTbl_s.onuPonPmTimerId.onuPonTimerPeriodic) != 0) 
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonPmTimerId));
  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponPmTimerExpireHndl
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
void onuEponPmTimerExpireHndl(void)
{
  onuEponPmCountersAdd();
}

/*******************************************************************************
**
**  onuEponPmCountersAdd
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
MV_STATUS onuEponPmCountersAdd(void)
{
  MV_U32 counter;
  MV_U32 llidTxFecState;
  MV_U32 macId;
  S_PcsPm statCounters;


  /* Rx Counters - One set of counters for all MACs */
  /* ============================================== */
  mvOnuEponMacRxStatsFcsErrorGet(&counter, 0);          /* Count number of received frames with FCS errors */
  g_OnuEponPm[0].rx.fcsErrorFramesCnt += counter;                                                             
  mvOnuEponMacRxStatsShortErrorGet(&counter, 0);        /* Count number of short frames received */
  g_OnuEponPm[0].rx.shortFramesCnt += counter;                                                     
  mvOnuEponMacRxStatsLongErrorGet(&counter, 0);         /* Count number of long frames received */
  g_OnuEponPm[0].rx.longFramesCnt += counter;                                                     
  mvOnuEponMacRxStatsDataFrameGet(&counter, 0);         /* Count number of data frames received */
  g_OnuEponPm[0].rx.dataFramesCnt += counter;                                                                
  mvOnuEponMacRxStatsCtrlFrameGet(&counter, 0);         /* Count number of control frames received */
  g_OnuEponPm[0].rx.ctrlFramesCnt += counter;                       
  mvOnuEponMacRxStatsReportFrameGet(&counter, 0);       /* Count number of report frames received */
  g_OnuEponPm[0].rx.reportFramesCnt += counter;                                                     
  mvOnuEponMacRxStatsGateFrameGet(&counter, 0);         /* Count number of gate frames received */
  g_OnuEponPm[0].rx.gateFramesCnt += counter;    

	/* PCS Stats counters - one for all MACs */
	mvOnuEponMacPcsStatsGet(&statCounters.legalFrameCnt,
				&statCounters.maxFrameSizeErrCnt,
				&statCounters.parityLenErrCnt,
				&statCounters.longGateErrCnt,
				&statCounters.protocolErrCnt,
				&statCounters.minFrameSizeErrCnt,
				&statCounters.legalFecFrameCnt,
				&statCounters.legalNonFecFrameCnt);

	g_OnuEponPm[0].pcs.legalFrameCnt += statCounters.legalFrameCnt;
	g_OnuEponPm[0].pcs.maxFrameSizeErrCnt += statCounters.maxFrameSizeErrCnt;
	g_OnuEponPm[0].pcs.parityLenErrCnt += statCounters.parityLenErrCnt;
	g_OnuEponPm[0].pcs.longGateErrCnt += statCounters.longGateErrCnt;
	g_OnuEponPm[0].pcs.protocolErrCnt += statCounters.protocolErrCnt;
	g_OnuEponPm[0].pcs.minFrameSizeErrCnt += statCounters.minFrameSizeErrCnt;
	g_OnuEponPm[0].pcs.legalFecFrameCnt += statCounters.legalFecFrameCnt;
	g_OnuEponPm[0].pcs.legalNonFecFrameCnt += statCounters.legalNonFecFrameCnt;

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) 
  {   
    /* Tx Counters */
    /* =========== */
    mvOnuEponMacTxStatsCtrlRegReqFrameGet(&counter, macId);   /* Count number of register request frames transmitted */    
    g_OnuEponPm[macId].tx.ctrlRegReqFramesCnt += counter;                                                                  
    mvOnuEponMacTxStatsCtrlRegAckFrameGet(&counter, macId);   /* Count number of register acknowledge frames transmitted */
    g_OnuEponPm[macId].tx.ctrlRegAckFramesCnt += counter;                                                                  
    mvOnuEponMacTxStatsCtrlReportFrameGet(&counter, macId);   /* Count number of report frames transmitted */              
    g_OnuEponPm[macId].tx.reportFramesCnt += counter;                                                                      
    mvOnuEponMacTxStatsTxAllowedByteCountGet(&counter, macId) /* Count number of Tx Byte Allow counter */                  ;
    g_OnuEponPm[macId].tx.txAllowedBytesCnt += counter;   

	/* update counters in case FEC is disabled */
	onuEponDbOnuTxFecCfgGet(&llidTxFecState, macId);
	if (llidTxFecState == 0) 
	{
	  mvOnuEponMacTxStatsDataFrameGet(&counter, macId);         /* Count number of data frames transmitted */                
	  g_OnuEponPm[macId].tx.dataFramesCnt += counter;                                                                        
	}

    /* Sw Counters */
    /* =========== */
    onuEponPmSwCountersUpdate(&g_OnuEponPm[macId].sw, macId); 

    /* Gpm Counters */
    /* ============ */
    mvOnuEponMacGpmGrantValidCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.grantValidCnt += counter;                 
    mvOnuEponMacGpmGrantMaxFutureTimeErrorCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.grantMaxFutureTimeErrorCnt += counter;    
    mvOnuEponMacGpmMinProcTimeErrorCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.minProcTimeErrorCnt += counter;          
    mvOnuEponMacGpmLengthErrorCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.lengthErrorCnt += counter;                
    mvOnuEponMacGpmDiscoveryAndRegisterCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.discoveryAndRegCnt += counter;            
    mvOnuEponMacGpmFifoFullErrorCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.fifoFullErrorCnt += counter;              
    mvOnuEponMacGpmOpcDiscoveryNotRegisterBcastCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.opcDiscoveryNotRegBcastCnt += counter;    
    mvOnuEponMacGpmOpcRegisterNotDiscoveryCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.opcRegisterNotDiscoveryCnt += counter;    
    mvOnuEponMacGpmOpcDiscoveryNotRegisterNotBcastCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.opcDiscoveryNotRegNotBcastCnt += counter; 
    mvOnuEponMacGpmOpcDropGrantCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.opcDropGrantCnt += counter;               
    mvOnuEponMacGpmOpcHiddenGrantCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.opcHiddenGrantCnt += counter;             
    mvOnuEponMacGpmOpcBackToBackCounterGet(&counter, macId);
    g_OnuEponPm[macId].gpm.opcBackToBackCnt += counter; 
  }

  return(MV_OK);
}

 /*******************************************************************************
**
**  onuEponPmSwCountersUpdate
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function updates software counters
**                
**  PARAMETERS:  S_SwPm *swPm
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmSwCountersUpdate(S_SwPm *swPm, MV_U32 macId)
{
  MV_U32 index;

  /* MAC 0 counts RX statitics for all modules */
  for (index = 0; index < MAX_EPON_RX_SW_CNT; index++)
  {
    swPm->swRxCnt[index] = g_OnuEponSwPm[macId].swRxCnt[index];
  }

  for (index = 0; index < MAX_EPON_TX_SW_CNT; index++)
  {
    swPm->swTxCnt[index] = g_OnuEponSwPm[macId].swTxCnt[index];
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmSwRxCountersAdd
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function updates SW Rx counters
**                
**  PARAMETERS:  MV_U32 cnt 
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmSwRxCountersAdd(MV_U32 cnt, MV_U32 macId)
{
  if (cnt >= MAX_EPON_RX_SW_CNT) 
    return(MV_ERROR);

  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  g_OnuEponSwPm[macId].swRxCnt[cnt]++;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmSwTxCountersAdd
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function updates SW counters
**                
**  PARAMETERS:  MV_U32 cnt 
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmSwTxCountersAdd(MV_U32 cnt, MV_U32 macId)
{
  if (cnt >= MAX_EPON_TX_SW_CNT) 
    return(MV_ERROR);

  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  g_OnuEponSwPm[macId].swTxCnt[cnt]++;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmRxPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu Rx counters
**                 
**  PARAMETERS:  S_RxPm *rxPm
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmRxPmGet(S_RxPm *rxPm, MV_U32 macId)
{
  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  rxPm->fcsErrorFramesCnt = g_OnuEponPm[macId].rx.fcsErrorFramesCnt;
  rxPm->shortFramesCnt    = g_OnuEponPm[macId].rx.shortFramesCnt;   
  rxPm->longFramesCnt     = g_OnuEponPm[macId].rx.longFramesCnt;    
  rxPm->dataFramesCnt     = g_OnuEponPm[macId].rx.dataFramesCnt;    
  rxPm->ctrlFramesCnt     = g_OnuEponPm[macId].rx.ctrlFramesCnt;    
  rxPm->reportFramesCnt   = g_OnuEponPm[macId].rx.reportFramesCnt;  
  rxPm->gateFramesCnt     = g_OnuEponPm[macId].rx.gateFramesCnt;    

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmCtrlCntGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu control packet counters
**                 
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_U32 onuEponPmCtrlCntGet(MV_U32 macId)
{
  return(g_OnuEponPm[macId].rx.ctrlFramesCnt);
}

/*******************************************************************************
**
**  onuEponPmTxPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu Tx counters
**                 
**  PARAMETERS:  S_TxPm *txPm
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmTxPmGet(S_EponTxPm *txPm, MV_U32 macId)
{
  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  txPm->ctrlRegReqFramesCnt = g_OnuEponPm[macId].tx.ctrlRegReqFramesCnt;
  txPm->ctrlRegAckFramesCnt = g_OnuEponPm[macId].tx.ctrlRegAckFramesCnt;
  txPm->reportFramesCnt     = g_OnuEponPm[macId].tx.reportFramesCnt;    
  txPm->dataFramesCnt       = g_OnuEponPm[macId].tx.dataFramesCnt;      
  txPm->txAllowedBytesCnt   = g_OnuEponPm[macId].tx.txAllowedBytesCnt;  

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmTxDataPmLastIntervalGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu Tx datacounters
**                 
**  PARAMETERS:  MV_U32 *txDataPm
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmTxDataPmLastIntervalGet(MV_U32 *txDataPm, MV_U32 macId)
{
	MV_U32 counter;
	MV_STATUS status = MV_OK;

	if (macId >= EPON_MAX_MAC_NUM)
		return(MV_ERROR);

	status = mvOnuEponMacTxStatsDataFrameGet(&counter, macId); /* Count number of data frames transmitted */
	if (status == MV_OK) {
		g_OnuEponPm[macId].tx.dataFramesCnt += counter;
		*txDataPm = counter;                                                                        
	}

	return(status);
}

/*******************************************************************************
**
**  onuEponPmTxByteCntGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu control tx byte allowed counter
**                 
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_U32 onuEponPmTxByteCntGet(MV_U32 macId)
{
  return(g_OnuEponPm[macId].tx.txAllowedBytesCnt);
}

/*******************************************************************************
**
**  onuEponPmSwPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu SW counters
**                 
**  PARAMETERS:  S_SwPm *swPm
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmSwPmGet(S_SwPm *swPm, MV_U32 macId)
{
  MV_U32 index;

  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  for (index = 0; index < MAX_EPON_RX_SW_CNT; index++)
  {
    swPm->swRxCnt[index] = g_OnuEponPm[macId].sw.swRxCnt[index];
  }

  for (index = 0; index < MAX_EPON_TX_SW_CNT; index++)
  {
    swPm->swTxCnt[index] = g_OnuEponPm[macId].sw.swTxCnt[index];
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmGpmPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu Gpm counters
**                 
**  PARAMETERS:  S_GpmPm *gpmPm
**               MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK  
**
*******************************************************************************/
MV_STATUS onuEponPmGpmPmGet(S_GpmPm *gpmPm, MV_U32 macId)
{
  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  gpmPm->grantValidCnt                 = g_OnuEponPm[macId].gpm.grantValidCnt;                
  gpmPm->grantMaxFutureTimeErrorCnt    = g_OnuEponPm[macId].gpm.grantMaxFutureTimeErrorCnt;   
  gpmPm->minProcTimeErrorCnt           = g_OnuEponPm[macId].gpm.minProcTimeErrorCnt;          
  gpmPm->lengthErrorCnt                = g_OnuEponPm[macId].gpm.lengthErrorCnt;               
  gpmPm->discoveryAndRegCnt            = g_OnuEponPm[macId].gpm.discoveryAndRegCnt;           
  gpmPm->fifoFullErrorCnt              = g_OnuEponPm[macId].gpm.fifoFullErrorCnt;             
  gpmPm->opcDiscoveryNotRegBcastCnt    = g_OnuEponPm[macId].gpm.opcDiscoveryNotRegBcastCnt;   
  gpmPm->opcRegisterNotDiscoveryCnt    = g_OnuEponPm[macId].gpm.opcRegisterNotDiscoveryCnt;   
  gpmPm->opcDiscoveryNotRegNotBcastCnt = g_OnuEponPm[macId].gpm.opcDiscoveryNotRegNotBcastCnt;
  gpmPm->opcDropGrantCnt               = g_OnuEponPm[macId].gpm.opcDropGrantCnt;              
  gpmPm->opcHiddenGrantCnt             = g_OnuEponPm[macId].gpm.opcHiddenGrantCnt;            
  gpmPm->opcBackToBackCnt              = g_OnuEponPm[macId].gpm.opcBackToBackCnt;             
                                         
  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmGpmValidGrantGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu Gpm valid grant counter
**                 
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None  
**
**  RETURNS:     onu Gpm valid grant counter  
**
*******************************************************************************/
MV_U32 onuEponPmGpmValidGrantGet(MV_U32 macId)
{
  if (macId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  return(g_OnuEponPm[macId].gpm.grantValidCnt);
}

/*******************************************************************************
**
**  onuEponPmPcsPmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The funtion return onu Pcs counters
**
**  PARAMETERS:  S_PcsPm *pcsPm
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuEponPmPcsPmGet(S_PcsPm *pcsPm)
{
	if (pcsPm == 0)
		return(MV_BAD_PARAM);

	pcsPm->legalFrameCnt       = g_OnuEponPm[0].pcs.legalFrameCnt;
	pcsPm->maxFrameSizeErrCnt  = g_OnuEponPm[0].pcs.maxFrameSizeErrCnt;
	pcsPm->parityLenErrCnt     = g_OnuEponPm[0].pcs.parityLenErrCnt;
	pcsPm->longGateErrCnt      = g_OnuEponPm[0].pcs.longGateErrCnt;
	pcsPm->protocolErrCnt      = g_OnuEponPm[0].pcs.protocolErrCnt;
	pcsPm->minFrameSizeErrCnt  = g_OnuEponPm[0].pcs.minFrameSizeErrCnt;
	pcsPm->legalFecFrameCnt    = g_OnuEponPm[0].pcs.legalFecFrameCnt;
	pcsPm->legalNonFecFrameCnt = g_OnuEponPm[0].pcs.legalNonFecFrameCnt;

	return(MV_OK);
}

/******************************************************************************/
/* ========================================================================== */
/*                         GUNIT LLID TX Queues Section                       */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponPmDbaAddBytePerLlidFifo
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion add additional bytes per LLID FIFO
**                 
**  PARAMETERS:  MV_U32 llId
**               MV_U32 counter
**
**  OUTPUTS:     None  
**
**  RETURNS:     OK or ERROR
**
*******************************************************************************/
MV_STATUS onuEponPmDbaAddBytePerLlidFifo(MV_U32 llId, MV_U32 counter)
{
	if (llId >= EPON_MAX_MAC_NUM)
    return(MV_ERROR);

  g_OnuEponSwPmAddCntPerLlidFifo[llId] = counter;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmDbaAddBytePerLlidFifoGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return additional bytes per LLID FIFO
**                 
**  PARAMETERS:  MV_U32 llId
**
**  OUTPUTS:     None  
**
**  RETURNS:     Additional bytes per LLID FIFO
**
*******************************************************************************/
MV_U32 onuEponPmDbaAddBytePerLlidFifoGet(MV_U32 llId)
{
	if (llId >= EPON_MAX_MAC_NUM)
		return(0);

	return(g_OnuEponSwPmAddCntPerLlidFifo[llId]);
}

/*******************************************************************************
**
**  onuEponPmDbaAddBytePerLlidAndQueue
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion add additional bytes per queue
**                 
**  PARAMETERS:  MV_U32 llId
**				 MV_U32 queueId
**               MV_U32 counter
**
**  OUTPUTS:     None  
**
**  RETURNS:     OK or ERROR
**
*******************************************************************************/
MV_STATUS onuEponPmDbaAddBytePerLlidAndQueue(MV_U32 llId, MV_U32 queueId, MV_U32 counter)
{
	if ((queueId >= EPON_MAX_QUEUE) || (llId >= EPON_MAX_MAC_NUM))
		return(MV_ERROR);

	g_OnuEponSwPmAddCntPerLlidAndQueue[llId][queueId] = counter;

	return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmDbaAddBytePerLlidAndQueueGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return additional bytes per queue
**                 
**  PARAMETERS:  MV_U32 llId
**				 MV_U32 queueId
**
**  OUTPUTS:     None  
**
**  RETURNS:     Additional bytes per queue
**
*******************************************************************************/
MV_U32 onuEponPmDbaAddBytePerLlidAndQueueGet(MV_U32 llId, MV_U32 queueId)
{
	if ((queueId >= EPON_MAX_QUEUE) || (llId >= EPON_MAX_MAC_NUM))
		return(0);

	return(g_OnuEponSwPmAddCntPerLlidAndQueue[llId][queueId]);
}

/*******************************************************************************
**
**  onuEponPmGunitLlidQueueCntGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu Gunit LLID TX queue counter
**                 
**  PARAMETERS:  MV_U32 llId
**               MV_U32 queueId
**               MV_U32 *counter
**
**  OUTPUTS:     None  
**
**  RETURNS:     nu Gunit LLID TX queue counter
**
*******************************************************************************/
MV_STATUS onuEponPmGunitLlidQueueCntGet(MV_U32 llid, MV_U32 queueId, MV_U32 *counter)
{
	/* This function is called from IRQ context, therefore the registers access should be as fast as possible
	For this purpose the standard call to asicOntMiscRegRead() API is not used and registers are indexed
	and accesses using macros only
	*/
	if ((llid >= EPON_MAX_MAC_NUM) || (queueId > onuEponDbOnuDbaHighestReportQueueGet(llid)) || (counter == 0))
		return(MV_ERROR);

	*counter = MV_REG_READ(MV_PON_GUNIT_TX_N_QUEUES(llid, queueId));
	*counter += onuEponPmDbaAddBytePerLlidAndQueueGet(llid, queueId);

	return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmGunitLlid0PktModMaxHeaderSizeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion configure LLID0 Packet Modification max header size to 2 bytes
**                 
**  PARAMETERS:  MV_U32 headerSize
**
**  OUTPUTS:     None  
**
**  RETURNS:     MV_OK or Error
**
*******************************************************************************/
MV_STATUS onuEponPmGunitLlid0PktModMaxHeaderSizeSet(MV_U32 headerSize)
{
	MV_STATUS status;

	status  = asicOntMiscRegWrite(mvAsicReg_GUNIT_TX_0_PKT_MOD_MAX_HEAD_SIZE_CFG, headerSize, 0); 

	return(status);
}

/*******************************************************************************
**
**  onuEponPmGunitLlid0PktModPktCountGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return LLID0 Packet Modification long header packet 
**               counter
**                 
**  PARAMETERS:  MV_U32 *packetCount
**
**  OUTPUTS:     None  
**
**  RETURNS:     Onu Gunit LLID0 Packet Modification long header packet count
**
*******************************************************************************/
MV_STATUS onuEponPmGunitLlid0PktModPktCountGet(MV_U32 *packetCount)
{
	MV_STATUS status;

	status  = asicOntMiscRegRead(mvAsicReg_GUNIT_TX_0_PKT_MOD_STATS_PKT_COUNT, packetCount, 0); 

	return(status);
}

/*******************************************************************************
**
**  onuEponPmRandomGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu random number
**                 
**  PARAMETERS:  None
**
**  OUTPUTS:     None  
**
**  RETURNS:     0nu random number
**
*******************************************************************************/
MV_STATUS onuEponPmRandomGet(MV_U32 *random)
{
	MV_U32 addressLow;
	MV_U32 addressHigh;
	MV_U32 thermalRegValue;
	MV_U32 thermalRegAddr = 0x000184D8;
	MV_STATUS status;
	S_PcsPm pcsPm;

	if (random == 0)
		return(MV_BAD_PARAM);

	status = onuEponDbOnuMacAddrGet(&addressLow, &addressHigh, 0);
	if (status == MV_OK)
		status = onuEponPmPcsPmGet(&pcsPm); /* take stat counters from DB */

	if (status != MV_OK)
		return status;

	thermalRegValue = ((MV_REG_READ(thermalRegAddr) >> 10)  & 0x1FF);
	g_OnuEponRandomNum[7] = thermalRegValue;
	mvOsDelay(addressLow & 0xFF);

	g_OnuEponRandomNum[1] = pcsPm.legalFrameCnt;
	g_OnuEponRandomNum[2] = pcsPm.parityLenErrCnt | (pcsPm.maxFrameSizeErrCnt << 16);

	thermalRegValue = ((MV_REG_READ(thermalRegAddr) >> 10)  & 0x1FF);
	g_OnuEponRandomNum[7] |= thermalRegValue << 9;    
	mvOsDelay(addressLow & 0xFF);

	g_OnuEponRandomNum[3] = pcsPm.protocolErrCnt | (pcsPm.longGateErrCnt << 16);

	thermalRegValue = ((MV_REG_READ(thermalRegAddr) >> 10)  & 0x1FF);
	g_OnuEponRandomNum[7] |= thermalRegValue << 18;   
	mvOsDelay(addressLow & 0xFF);

	g_OnuEponRandomNum[4] = pcsPm.minFrameSizeErrCnt;
	g_OnuEponRandomNum[5] = pcsPm.legalNonFecFrameCnt | (pcsPm.legalFecFrameCnt << 16);

	g_OnuEponRandomNum[6] = addressLow;

	thermalRegValue = ((MV_REG_READ(thermalRegAddr) >> 10)  & 0x1F);
	g_OnuEponRandomNum[7] |= thermalRegValue << 27;

	*random = onuEponPmGenCrcKey(g_OnuEponRandomNum, GPON_RANDOM_ARR_SZ);
	/* Count number of random value requests and put value into DB */
	g_OnuEponRandomValues[g_OnuEponRandomCount % GPON_RANDOM_VAL_ARR_SZ] = *random;
	g_OnuEponRandomCount++;

	return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmRandomReset
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion reset onu random number
**                 
**  PARAMETERS:  None
**
**  OUTPUTS:     None  
**
**  RETURNS:     0nu random number
**
*******************************************************************************/
MV_STATUS onuEponPmRandomReset(void)
{
	MV_U32	i;

	for (i = 0; i < GPON_RANDOM_ARR_SZ; i++)
		g_OnuEponRandomNum[i] = 0;

	return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmRandomThresholdCheck
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The funtion return onu random threshold check parameters
**                 
**  PARAMETERS:  MV_U32 *gateFrames
**               MV_U32 *discNotRegFrames
**               MV_U32 *regReqframes
**
**  OUTPUTS:     None  
**
**  RETURNS:     0nu random number
**
*******************************************************************************/
MV_STATUS onuEponPmRandomThresholdCheck(MV_U32 *gateFrames, MV_U32 *discNotRegFrames, 
										MV_U32 *regReqframes)
{
  *gateFrames       = g_OnuEponPm[0].rx.gateFramesCnt; 
  *discNotRegFrames = g_OnuEponPm[0].gpm.opcDiscoveryNotRegBcastCnt;
  *regReqframes     = g_OnuEponPm[0].tx.ctrlRegReqFramesCnt;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponPmGenCrcTable
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function generate table of CRC remainders 
**               
**  PARAMETERS:  None  
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**
*******************************************************************************/
MV_STATUS onuEponPmGenCrcTable(void)
{ 
  register int    i, j;  
  register MV_U32 crc_accum;

  for ( i = 0;  i < 256;  i++ )
  { 
    crc_accum = ( (MV_U32) i << 24 );
    for ( j = 0;  j < 8;  j++ )
    { 
      if ( crc_accum & 0x80000000 )
      {
        crc_accum = ( crc_accum << 1 ) ^ POLYNOMIAL;
      }
      else
      {
        crc_accum = ( crc_accum << 1 ); 
      }
    }
    crc_table[i] = crc_accum; 
  } 

  return(MV_OK); 
}

/*******************************************************************************
**
**  onuEponPmGenCrcKey
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function calculate CRC key 
**               
** 	PARAMETERS:  unsigned long msg[]
**				 int           size  
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**
*******************************************************************************/
MV_U32 onuEponPmGenCrcKey(MV_U32 msg[], MV_U32 size)
{ 
   MV_U8  data;
   MV_U32 idx;
   MV_U32 calcCrc = 0;
   
   for (idx = 0;  idx < size;  idx++)
   { 
     data    = (MV_U8)((msg[idx]) ^ (calcCrc >> 24));
     calcCrc = ((crc_table[data]) ^ (calcCrc << 8)); 
   }

   return (calcCrc); 
}
