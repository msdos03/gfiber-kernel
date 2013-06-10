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
/**  FILE        : eponOnuIsr.c                                              **/
/**                                                                          **/
/**  DESCRIPTION : This file implements ONU EPON Interrupt handling          **/
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
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuIsr.c"

#define MV_NO_XVR_RST (0xFF)

/* Global Variables
------------------------------------------------------------------------------*/
extern spinlock_t onuPonIrqLock;

/* Local Variables
------------------------------------------------------------------------------*/
MV_U32 onuEponCurrentInterrupt = 0;
MV_U32 onuEponRxMacCtrlPackets = 0;
MV_U32 onuEponTxMacByteAllowed = 0;
MV_U32 recvGateFrameBase       = 0;
MV_U32 recvDiscNotRegFrameBase = 0;
MV_U32 sendCtrlRegReqFrameBase = 0;
MV_U32 regRandomInitInd        = 0;
MV_U32 regRandomCheckThreshold = 3/*10*/;
MV_U32 regRandomMaskThreshold  = 0x1FF; /* 0-512 msec */
MV_U32 regRandomResetState     = 0;     /* Off */

MV_U32 isrTimestampEventCnt    = 0;
MV_U32 isrTimestampEventMiss   = 0;
MV_U32 isrInvalidRegState      = 0;

MV_U32 isrP2pPreviousFecState  = 0;

MV_BOOL eponMgmtForceTxDisable = MV_FALSE;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
MV_STATUS onuEponIsrXvrReset(void);
void      onuEponIsrResetRandomStateMachine(void);

/*******************************************************************************
**
**  onuEponIsrLowRoutine
**
*******************************************************************************/
void onuEponIsrLowRoutine(MV_U32 *interruptEvent, MV_U32 *interruptStatus)
{
	MV_U32 interrupt;

	mvOnuEponMacPonInterruptGet(&interrupt);
	onuEponCurrentInterrupt &= 0xFFFF0000;
	onuEponCurrentInterrupt |= interrupt;

	*interruptEvent  = (onuEponCurrentInterrupt >> ONU_EPON_EVENT_SHIFT) & ONU_EPON_INTERRUPTS;
	*interruptStatus =  onuEponCurrentInterrupt                          & ONU_EPON_INTERRUPTS;

	mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
		   "DEBUG: (%s:%d) Event(0x%08X) Status(0x%08X)\n",
		   __FILE_DESC__, __LINE__, *interruptEvent, *interruptStatus);

	onuEponCurrentInterrupt = 0;
}

/*******************************************************************************
**
**  onuEponIsrRoutine
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements interrupt handler
**
**  PARAMETERS:  None, void* param
**
**  OUTPUTS:     None
**
**  RETURNS:     None, void*
**
*******************************************************************************/
void onuEponIsrRoutine(MV_U32 event, MV_U32 status)
{
	MV_BOOL state;
	MV_U32  rxGenFecEn      = 0;
	MV_U32  interruptEvent  = 0;
	MV_U32  interruptStatus = 0;
	LINKSTATUSFUNC linkStatusCallback;

#ifdef MV_EPON_HW_INTERRUPT
	onuEponIsrLowRoutine(&interruptEvent, &interruptStatus);
#else
	interruptEvent  = event;
	interruptStatus = status;
#endif /* MV_GPON_HW_INTERRUPT */

	/* Interrupt Protect */
	/* ================= */

	if (interruptEvent & ONU_EPON_XVR_SD_MASK) {
		state = ponXvrFunc(interruptStatus, ONU_EPON_XVR_SD_MASK);

		if (state == MV_FALSE)
		{
			onuEponDbOnuSignalDetectSet(1); /* alarm is OFF */
			mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
				   "DEBUG: (%s:%d) Signal Detect ON\n", __FILE_DESC__, __LINE__);
		}
		else if (state == MV_TRUE)
		{
			onuEponDbOnuSignalDetectSet(0);/* alarm is ON */
			mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
				   "DEBUG: (%s:%d) Signal Detect OFF\n", __FILE_DESC__, __LINE__);
		}

		onuEponPonMngIntrAlarmHandler(ONU_EPON_XVR_SD_MASK, state);

		mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
			   "DEBUG: (%s:%d) Link %s\n", __FILE_DESC__, __LINE__, (state == MV_FALSE) ? "On" : "Off");

		if (state == MV_FALSE) { /* alarm is OFF */

			onuEponDbOnuRxFecCfgGet(&rxGenFecEn);

			if (rxGenFecEn != ONU_RX_PCS_FEC_DIS)		/* config PCS synchronization */
				mvOnuEponMacPcsDelaySet(0x1058);	/*configuration - FEC enabled */
#ifndef PON_FPGA
			onuEponIsrXvrReset();
#endif /* PON_FPGA */
			mvPonPrint(PON_PRINT_DEBUG, PON_ISR_RAND_MODULE,
				   "DEBUG: (%s:%d) ResetRandomStateMachine %s\n", __FILE_DESC__, __LINE__);
			onuEponIsrResetRandomStateMachine();
			/* Call link status callback function */
			if (onuEponDbModeGet() == E_EPON_IOCTL_P2P_MODE)
			{
				linkStatusCallback = onuEponDbLinkStatusCallbackGet();
				if (linkStatusCallback != NULL)
				{
					linkStatusCallback(MV_TRUE);
					mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
						   "DEBUG: (%s:%d) Notify link is UP\n", __FILE_DESC__, __LINE__);
				}
			}
		}
		else if (state == MV_TRUE) /* alarm is ON */
		{
			if (onuEponDbP2PForceModeGet())
			{
				/* set P2P mode */
				onuEponDbModeSet(E_EPON_IOCTL_P2P_MODE);
			}
			else
			{
				/* set EPON mode */
				onuEponDbModeSet(E_EPON_IOCTL_STD_MODE);
			}
            

			/* config PCS synchronization configuration  - FEC disabled */
			mvOnuEponMacPcsDelaySet(0x1C58);
			/* clear Rx Ctrl message FIFO */
			onuEponPonMngIntrRxCtrlFifoCleanup();
			/* clear OAM SW FIFO - Tx */
			onuEponOamFlushAllMsg();
			/* start xvr reset timer */
			onuEponIsrXvrResetStateSet(MV_TRUE);
		}
	}

	if (interruptEvent & ONU_EPON_SERDES_SD_MASK) {
		state = (interruptStatus & ONU_EPON_SERDES_SD_MASK) ? MV_FALSE : MV_TRUE;
		onuEponPonMngIntrAlarmHandler(ONU_EPON_SERDES_SD_MASK, state);
	}

	if (interruptEvent & ONU_EPON_TIMESTAMP_DRIFT_MASK) {
		state = (interruptStatus & ONU_EPON_TIMESTAMP_DRIFT_MASK) ? MV_TRUE : MV_FALSE;
		onuEponPonMngIntrAlarmHandler(ONU_EPON_TIMESTAMP_DRIFT_MASK, state);
	}

	if (interruptEvent & ONU_EPON_DS_SYNC_OK_PCS_MASK) {
		state = (interruptStatus & ONU_EPON_DS_SYNC_OK_PCS_MASK) ? MV_TRUE : MV_FALSE;

		mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
			   "DEBUG: (%s:%d) PCS Sync %s\n", __FILE_DESC__, __LINE__, (state == MV_FALSE) ? "Off" : "On");

		if (state == MV_TRUE)
			onuEponDbOnuDsSyncOkPcsSet(1); /* set DS Sync OK PCS to ON */
		else if (state == MV_FALSE)
			onuEponDbOnuDsSyncOkPcsSet(0); /* set DS Sync OK PCS to OFF */
	}

	if (interruptEvent & ONU_EPON_REGISTERED_LLID_0_MASK) {
		/* state = (interruptStatus & ONU_EPON_REGISTERED_LLID_0_MASK) ? MV_TRUE : MV_FALSE;
		 onuEponPonMngIntrRegHandler(ONU_EPON_REGISTERED_LLID_0_MASK, state); */
	}

	/* Event is triggered for each change in the Status  */
	/* Status == 0, means there is something in the FIFO */
	/* Status == 1, means there is nothing in the FIFO   */
	/* ================================================= */
	if ((interruptEvent   & ONU_EPON_RX_CTRL_QUEUE_MASK) &&
	    ((interruptStatus & ONU_EPON_RX_CTRL_QUEUE_MASK) == 0))
		onuEponPonMngIntrMessageHandler(ONU_EPON_RX_CTRL_MSG);

	if ((interruptEvent   & ONU_EPON_RX_RPRT_QUEUE_MASK) &&
	    ((interruptStatus & ONU_EPON_RX_RPRT_QUEUE_MASK) == 0))
		onuEponPonMngIntrMessageHandler(ONU_EPON_RX_RPRT_MSG);

	if (interruptEvent & ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK) {
		/* Set onu DBA Report interrupt */
		onuEponPonMngTxCtrlMessageHandler();
		onuEponDbaSwRprtMacTimerCfg(onuEponDbOnuSwRprtMacTimerIntervalGet());
		isrTimestampEventCnt++;
	}
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuGponDgIsrRoutine
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function implements Dying Gasp
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponDgIsrRoutine(void)
{
  onuEponPonMngIntrDgHandler();
}
#endif /* PON_FPGA */

/*******************************************************************************
**
**  onuEponIsrResetRandomStateMachine
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function clear Random state machine
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponIsrResetRandomStateMachine(void)
{
  /* enable tx */
  if (onuEponForceTxDownStateGet(0) != MV_TRUE)
    mvOnuEponMacOnuTxEnableSet(ONU_TX_EN, 0);

  /* restore MPCP timer interval */
  onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonMpcpTimerId), 0, ONU_PON_TIMER_MPCP_INTERVAL, 1);

  /* clear all required counters for handling random delay for ONU transmission */
  recvGateFrameBase       = 0;
  recvDiscNotRegFrameBase = 0;
  sendCtrlRegReqFrameBase = 0;
  regRandomInitInd        = 0;
  regRandomResetState     = 0;
  onuEponPmRandomReset();
}

/*******************************************************************************
**
**  onuEponIsrTimerMpcpHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function validate mpcp frames arrival rate
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponIsrTimerMpcpHndl(unsigned long data)
{
  MV_U32 currentCtrlPackets      = 0;
  MV_U32 currentTxByteAllowed    = 0;
  MV_U32 recvGateFrame           = 0;
  MV_U32 recvDiscoverNotRegFrame = 0;
  MV_U32 sendCtrlRegReqFrame     = 0;
  MV_U32 randomTime              = 0;
  MV_U32 randomShift             = 0;
  MV_U32 randomRange             = 0;
  MV_U32 state                   = 0;
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonMpcpTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  if (regRandomResetState != 0)
  {
	  mvPonPrint(PON_PRINT_DEBUG, PON_ISR_RAND_MODULE,
				 "DEBUG: (%s:%d) Reset Random State Machine\n", __FILE_DESC__, __LINE__);
	  onuEponIsrResetRandomStateMachine();
  }
  else
  {
	if (onuEponDbOnuStateGet(0) == ONU_EPON_03_OPERATION)
	{
	  /* = Rx Ctrl Packet Check = */
	  /* ======================== */
	  currentCtrlPackets   = onuEponPmCtrlCntGet(0);
	  currentTxByteAllowed = onuEponPmTxByteCntGet(0);

	  if ((currentTxByteAllowed == onuEponTxMacByteAllowed) ||
	  	  (currentCtrlPackets   == onuEponRxMacCtrlPackets))
	  {
	    /* Holdover Active */
	    if (onuEponDbOnuHoldoverStateGet() != ONU_HOLDOVER_NOT_ACTIVE)
	    {
	  	  if (onuEponDbOnuHoldoverExecGet() != ONU_HOLDOVER_ACTIVE)
	  	  {
	  	    /* start onu epon pon holdover timer */
	  	    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHoldoverTimerId));
	  	    onuEponDbOnuHoldoverExecSet(ONU_HOLDOVER_ACTIVE);

	  	    mvPonPrint(PON_PRINT_DEBUG, PON_ISR_STATE_MODULE,
	  	  			   "DEBUG: (%s:%d) Holdover Timer Start(Tmr)\n", __FILE_DESC__, __LINE__);
	  	  }
	    }
	    /* Holdover Not Active */
	    else
	    {
	  	  onuEponPonMngAlarmHandlerExecute(ALL_MAC_INDICATION);
	    }
	  }

	  onuEponRxMacCtrlPackets = currentCtrlPackets;
	  onuEponTxMacByteAllowed = currentTxByteAllowed;

	  /* clear all required counters for handling random delay for ONU transmission */
	  onuEponIsrResetRandomStateMachine();

	  isrInvalidRegState = 0;
	}
	else /* (onuEponDbOnuStateGet(0) != ONU_EPON_03_OPERATION) */
	{
	  mvPonPrint(PON_PRINT_DEBUG, PON_ISR_STATE_MODULE,
	  		     "DEBUG: (%s:%d) MPCP state not registered\n", __FILE_DESC__, __LINE__);

	  /* handling of invalid ONU register state */
	  /* ====================================== */
	  mvOnuEponMacOnuStateGet(&state, 0);
	  if (state != 0)
	  {
	    isrInvalidRegState++;
	    if (isrInvalidRegState >= 2)
	    {
	      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_STATE_MODULE,
	      		     "DEBUG: (%s:%d) Force ONU Re-registration\n", __FILE_DESC__, __LINE__);

	      mvOnuEponMacOnuStateSet(ONU_EPON_NOT_REGISTERD, 0);
	      isrInvalidRegState = 0;
	    }
	  }

	  /* handling of random delay for ONU transmission to disable transmission collisions */
	  /* ================================================================================ */
	  /* get counters for random transmission check */
	  onuEponPmRandomThresholdCheck(&recvGateFrame,
								    &recvDiscoverNotRegFrame,
								    &sendCtrlRegReqFrame);

	  mvPonPrint(PON_PRINT_DEBUG, PON_ISR_RAND_MODULE,
				 "regRandomInitInd (%d)\n", regRandomInitInd);

	  if (regRandomInitInd == 0)
	  {
	    /* save current counter state for future comparison */
	    recvGateFrameBase       = recvGateFrame;
	    recvDiscNotRegFrameBase = recvDiscoverNotRegFrame;
	    sendCtrlRegReqFrameBase = sendCtrlRegReqFrame;

	    regRandomInitInd = 1;

	    mvPonPrint(PON_PRINT_DEBUG, PON_ISR_RAND_MODULE,
	  		       "DEBUG[0]: B-gate(%d)disc(%d)regReq(%d), regRandomInitInd(%d)\n",
	  		       recvGateFrameBase, recvDiscNotRegFrameBase, sendCtrlRegReqFrameBase, regRandomInitInd);
	  }

	  mvPonPrint(PON_PRINT_DEBUG, PON_ISR_RAND_MODULE,
				 "DEBUG[1]: C-gate(%d)disc(%d)regReq(%d)\n"
				 "DEBUG[1]: B-gate(%d)disc(%d)regReq(%d), regRandomInitInd(%d), regRandomResetState(%d)\n",
				 recvGateFrame, recvDiscoverNotRegFrame, sendCtrlRegReqFrame,
				 recvGateFrameBase, recvDiscNotRegFrameBase, sendCtrlRegReqFrameBase, regRandomInitInd, regRandomResetState);


	  /* start random timer section */
	  if (((recvGateFrame           - recvGateFrameBase)       > regRandomCheckThreshold) &&
	   	  ((recvDiscoverNotRegFrame - recvDiscNotRegFrameBase) > regRandomCheckThreshold) &&
	  	  ((sendCtrlRegReqFrame     - sendCtrlRegReqFrameBase) > regRandomCheckThreshold))
	  {
	    /* calc random value */
	    onuEponPmRandomGet(&randomTime);
	    randomShift = ((randomTime >> 16) & 0xF);
	    randomRange = ((randomTime >> randomShift) & regRandomMaskThreshold);

	    mvPonPrint(PON_PRINT_DEBUG, PON_ISR_RAND_MODULE,
	  			   "DEBUG[2]: C-gate(%d)disc(%d)regReq(%d)\n"
	  			   "DEBUG[2]: B-gate(%d)disc(%d)regReq(%d), thres(%d)randtime(%d)\n",
	  			   recvGateFrame, recvDiscoverNotRegFrame, sendCtrlRegReqFrame,
	  			   recvGateFrameBase, recvDiscNotRegFrameBase, sendCtrlRegReqFrameBase, regRandomCheckThreshold, randomRange);

	    /* wait random time */
	    onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonMpcpTimerId), 0, randomRange, 0);

	    regRandomResetState = 1;

	    /* disable tx */
	    mvOnuEponMacOnuTxEnableSet(ONU_TX_DIS, 0);
	  }
	}
  }

  if ((onuPonResourceTbl_s.onuPonMpcpTimerId.onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonMpcpTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponIsrTimerMpcpStateSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enable / disable epon mpcp timer
**
**  PARAMETERS:  MV_BOOL mode
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrTimerMpcpStateSet(MV_BOOL mode)
{
  if (mode == MV_TRUE)
  {
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonMpcpTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonMpcpTimerId));
  }
}

/*******************************************************************************
**
**  onuEponIsrTimerMpcpIntervalSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure epon mpcp timer interval
**
**  PARAMETERS:  MV_BOOL interval
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrTimerMpcpIntervalSet(MV_U32 interval)
{
  onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonMpcpTimerId), 0, interval, 1);
}

/*******************************************************************************
**
**  onuEponIsrTimerMpcpRandomResetSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure epon mpcp timer random reset params
**
**  PARAMETERS:  MV_U32 checkThreshold
**               MV_U32 maskThreshold
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrTimerMpcpRandomResetSet(MV_U32 checkThreshold, MV_U32 maskThreshold)
{
  regRandomCheckThreshold = checkThreshold;
  regRandomMaskThreshold  = maskThreshold;
}

/*******************************************************************************
**
**  onuEponIsrTimerHwReportHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function transmit report frame
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponIsrTimerHwReportHndl(unsigned long data)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonHwRprtTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  onuEponPonMngHwRprtMessageHandler();

  if ((onuPonResourceTbl_s.onuPonHwRprtTimerId.onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponIsrTimerHwReportStateSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enable / disable epon HW report message timer
**
**  PARAMETERS:  MV_BOOL mode
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrTimerHwReportStateSet(MV_U32 enable, MV_U32 t0_val, MV_U32 t0_time,
								     MV_U32 t1_val, MV_U32 t1_time)
{
  if (enable == MV_TRUE)
  {
	onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
	onuEponPonMngTimerHwRprtCfg(t0_val, t0_time, t1_val, t1_time);
	onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
	onuEponPonMngTimerHwRprtCfg(t0_val, t0_time, t1_val, t1_time);
  }
}

/*******************************************************************************
**
**  onuEponIsrTimerHoldoverHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function handle holdover for epon
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponIsrTimerHoldoverHndl(unsigned long data)
{
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonHoldoverTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  mvPonPrint(PON_PRINT_DEBUG, PON_ISR_STATE_MODULE,
			 "DEBUG: (%s:%d) Holdover Timer Expire\n", __FILE_DESC__, __LINE__);
  onuEponPonMngAlarmHandlerExecute(ALL_MAC_INDICATION);

  if ((onuPonResourceTbl_s.onuPonHoldoverTimerId.onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHoldoverTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponIsrTimerHoldoverIntervalSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure epon mpcp timer interval
**
**  PARAMETERS:  MV_BOOL interval
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrTimerHoldoverIntervalSet(MV_U32 interval, MV_U32 state)
{
  onuPonTimerUpdate(&(onuPonResourceTbl_s.onuPonHoldoverTimerId), 0, interval, state);
}

/*******************************************************************************
**
**  onuEponIsrTimerEventMissHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function validate interrupt event miss
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponIsrTimerEventMissHndl(unsigned long data)
{
  MV_U32 currHwTime;
  MV_U32 currHwThreshold;
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonIsrMissTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  if (isrTimestampEventCnt != 0)
  {
	isrTimestampEventCnt = 0;
  }
  else
  {
	mvOnuEponMacGenLocalTimeStampGet(&currHwTime);
    mvOnuEponMacGenTimeStampForIntrGet(&currHwThreshold);

	if (isrTimestampEventMiss > 0)
	{
		mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MISS_MODULE,
				   "DEBUG: (%s:%d) Event miss(%d) Currtime(%x) CurrThreshold(%x)\n",
				   __FILE_DESC__, __LINE__, isrTimestampEventMiss, currHwTime, currHwThreshold);

		onuEponOamHandleMsg();
		onuEponDbaSwRprtMacTimerCfg(62500);
		isrTimestampEventMiss = 0;
	}

	isrTimestampEventMiss++;
  }

  if ((onuPonResourceTbl_s.onuPonIsrMissTimerId.onuPonTimerPeriodic) != 0)
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonIsrMissTimerId));

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponIsrTimerEventMissStateSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enable / disable epon event miss timer
**
**  PARAMETERS:  MV_BOOL mode
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrTimerEventMissStateSet(MV_BOOL mode)
{
  if (mode == MV_TRUE)
  {
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonIsrMissTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonIsrMissTimerId));
  }
}

/*******************************************************************************
**
**  onuEponDbaSwRprtMacTimerCfg
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure abd trigger timestamp interupt
**
**  PARAMETERS:  MV_U32 interval - 16 nanaosec units
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponDbaSwRprtMacTimerCfg(MV_U32 interval)
{
  MV_U32 currentTimestamp;
  MV_U32 maxHwTimer = 0xFFFFFFFF;

  mvOnuEponMacGenLocalTimeStampGet(&currentTimestamp);

  if ((maxHwTimer - currentTimestamp) < interval)
  {
	mvOnuEponMacGenTimeStampForIntrSet(interval);
  }
  else
  {
	mvOnuEponMacGenTimeStampForIntrSet(currentTimestamp + interval);
  }
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuEponIsrXvrResetTimerHndl
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function is called by the EPON Interrupt handler to execute
**               XVR reset sequence in case of a problem with XVR signal detect
**
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void onuEponIsrXvrResetTimerHndl(unsigned long data)
{
  MV_STATUS status;
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonIsrXvrRstTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  status = onuEponIsrXvrReset();
  if (status != MV_NO_XVR_RST)
  {
	if ((onuPonResourceTbl_s.onuPonIsrXvrRstTimerId.onuPonTimerPeriodic) != 0)
	  onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId));
  }

  spin_unlock_irqrestore(&onuPonIrqLock, flags);
}

/*******************************************************************************
**
**  onuEponIsrXvrReset
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function reset xvr
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponIsrXvrReset(void)
{
  MV_STATUS status;
  MV_U32    onuState;
  MV_U32    initDone;

  onuState = onuEponDbOnuStateGet(0);
  if (onuState == ONU_EPON_02_REGISTER_PENDING)
  {
    status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT, 0x1, 0);
    if (status != MV_OK)
      return(status);

    do
    {
      status  = asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0_INIT_DONE, &initDone, 0);
      if (status != MV_OK)
        return(status);

    } while (initDone == 0);

    status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT, 0x0, 0);
    if (status != MV_OK)
      return(status);
  }
  else
  {
	return(MV_NO_XVR_RST);
  }

  return (MV_OK);
}

/*******************************************************************************
**
**  onuEponIsrXvrResetStateSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enable / disable onu xvr reset timer
**
**  PARAMETERS:  MV_BOOL mode
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
void onuEponIsrXvrResetStateSet(MV_BOOL mode)
{
  if (mode == MV_TRUE)
  {
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId));
  }
}
#endif /* PON_FPGA */

/*******************************************************************************
**
**  mvP2PStart
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function starts ONU P2P configuration
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS mvP2PStart(void)
{
#ifdef PON_Z2
    MV_STATUS status = MV_OK;
    MV_U32    pcsConfig;
    MV_U32    overHead;
    MV_U32    ignoreLlidCrcError;
    MV_U32    ignoreFcsError;
    MV_U32    ignoreGmiiError;
    MV_U32    ignoreLengthError;
    MV_U32    forwardAllLlid;
    MV_U32    forwardBc0FFF;
    MV_U32    forwardBc1FFF;
    MV_U32    forwardBc1xxx;
    MV_U32    dropBc1nnn;
    MV_U32    gpioGroup, gpioMask;
	MV_U32    interruptMask;
	MV_U32    polarity;
	MV_U32    interruptStatus;
    MV_U32    state;
    LINKSTATUSFUNC linkStatusCallback;
    /* Check for XVR SD interrupt status */
    mvOnuEponMacPonInterruptGet(&interruptStatus);
    interruptStatus &= 0xFFFF;

    state = ponXvrFunc(interruptStatus, ONU_EPON_XVR_SD_MASK);
    if (state == MV_FALSE)
    {
        onuEponDbOnuSignalDetectSet(1); /* alarm is OFF */
    }
    else if (state == MV_TRUE)
    {
        onuEponDbOnuSignalDetectSet(0);/* alarm is ON */
    }

	if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_SW_DBA_RPRT_TIMER)
	{
	  /* Stop SW DBA event miss timer */
	  onuEponIsrTimerEventMissStateSet(MV_FALSE);

	  /* Clear onu DBA Report interrupt */
	  status |= mvOnuEponMacPonInterruptMaskGet(&interruptMask);
	  interruptMask &= ~(ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
	  status |= mvOnuEponMacPonInterruptMaskSet(interruptMask);
	  if (status != MV_OK)
		return(status);
	}
	else
	{
	  /* Clear onu DBA Report Timer */
	  onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
	}

	/* stop xvr reset timer */
	onuEponIsrXvrResetStateSet(MV_FALSE);

    /* Save EPON PCS configuration */
    status = mvOnuEponMacPcsConfigGet(&pcsConfig);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacPcsConfigGet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    onuEponDbPcsCfgSet(pcsConfig);

    /* Set P2P PCS */
    status = asicOntGlbRegWrite(mvAsicReg_P2P_PCS_CONFIGURATION, ONU_P2P_PCS_TX_RX_NEW, 0);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) asicOntGlbRegWrite\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Set TXM configuration mode */
    status = asicOntGlbRegWrite(mvAsicReg_P2P_TXM_CFG_MODE, ONU_DEF_TXM_CFG_MODE_GE, 0);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) asicOntGlbRegWrite\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Set configuration mode in the GEN ONT register */
    status = asicOntGlbRegWrite(mvAsicReg_P2P_GEN_ONT_MODE, ONU_DEF_TXM_CFG_MODE_GE, 0);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) asicOntGlbRegWrite\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Save TXM overhead value */
    status = mvOnuEponMacTxmOverheadGet(&overHead);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacTxmOverheadGet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    onuEponDbOverheadSet(overHead);

    mvOnuEponMacTxmOverheadSet(ONU_DEF_TXM_CFG_P2P_OVERHEAD);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacTxmOverheadSet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Save current RXP packet filter */
    mvOnuEponMacRxpPacketFilterGet(&ignoreLlidCrcError,
                                   &ignoreFcsError,
                                   &ignoreGmiiError,
                                   &ignoreLengthError,
                                   &forwardAllLlid,
                                   &forwardBc0FFF,
                                   &forwardBc1FFF,
                                   &forwardBc1xxx,
                                   &dropBc1nnn);

    onuEponDbPktFilterPacketSet(forwardAllLlid, forwardBc1xxx, forwardBc1FFF, forwardBc0FFF, dropBc1nnn);
    onuEponDbPktFilterErrorSet(ignoreLengthError, ignoreGmiiError, ignoreFcsError, ignoreLlidCrcError);

    /* Set P2P RXP packet filter */
    status = mvOnuEponMacRxpPacketFilterSet(ONU_FORWARD_LLID_CRC_ERR_PKT_IGNORE,
                                            ONU_FORWARD_LLID_FCS_ERR_PKT,
                                            ONU_FORWARD_LLID_GMII_ERR_PKT,
                                            ONU_FORWARD_LLID_LEN_ERR_PKT,
                                            ONU_FORWARD_LLID_ALL_PKT_FILTER,
                                            ONU_FORWARD_LLID_7FFF_MODE_0_PKT,
                                            ONU_FORWARD_LLID_7FFF_MODE_1_PKT,
                                            ONU_FORWARD_LLID_XXXX_MODE_1_PKT,
                                            ONU_DROP_LLID_NNNN_MODE_1_PKT);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacRxpPacketFilterSet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

   /* MC ASIC     */
   /* =========== */
   if (MV_6601_DEV_ID == mvCtrlModelGet())
	{
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	  if (status != MV_OK)
	  {
	    mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			 "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
	    return(MV_ERROR);
	  }

	  /* PHY control register - force enable */
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_FORCE, 1, 0);
	  if (status != MV_OK)
	  {
		mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			"ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	  }

	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_VALUE, 1, 0);
	  if (status != MV_OK)
	  {
		mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			"ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	  }
	}
	/* KW2 ASIC Rev Z2 */
	/* =============== */
	else if (mvCtrlRevGet() == ONU_ASIC_REV_Z2)
	{
	  /* PHY control register - output status*/
	  PON_GPIO_GET(BOARD_GPP_PON_XVR_TX, gpioGroup, gpioMask);
	  if (gpioMask != PON_GPIO_NOT_USED)
	  {
	  	status = mvGppTypeSet(gpioGroup, gpioMask, ~gpioMask/*output*/);
	  	if (status != MV_OK)
	  	{
	  	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  			   "ERROR: (%s:%d) mvGppTypeSet\n\r", __FILE_DESC__, __LINE__);
	  	  return(MV_ERROR);
	  	}

	  	status = mvGppValueSet(gpioGroup, gpioMask, gpioMask /*enable*/);
	  	if (status != MV_OK)
	  	{
	  	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  			   "ERROR: (%s:%d) mvGppTypeSet\n\r", __FILE_DESC__, __LINE__);
	  	  return(MV_ERROR);
	  	}
	  }

	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_INPUT, 0);
	  if (status != MV_OK)
	  {
	    mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  		   "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
	    return(MV_ERROR);
	  }
	}

	/* KW2 ASIC Rev A0 */
	/* =============== */
	else if (mvCtrlRevGet() == ONU_ASIC_REV_A0)
	{
      status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	  if (status != MV_OK)
	  {
	    mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  		 "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
	    return(MV_ERROR);
	  }

	  /* PHY control register - force enable */
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_EN, 1, 0);
	  if (status != MV_OK)
	  {
		mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			       "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	  }

      polarity = onuP2PDbXvrBurstEnablePolarityGet();

	  /* XVR polarity */
	  /* XVR polarity == 0, Active High, transmit 1 to the line  */
	  /* XVR polarity == 1, Active Low, transmit 0 to the line  */

	  /* P2P mode */
	  /* Force Value == 0, transmit 0 to the line  */
	  /* Force Value == 1, transmit 1 to the line  */

	  /* Setting P2P should be reversed from XVR polarity */
	  /* XVR polarity == 0, Active High, write 1 for Force Value */
	  /* XVR polarity == 1, Active Low, write 0 for Force Value */

	  /* PHY control register - force enable value - according to polarity */
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_VAL, polarity, 0);
	  if (status != MV_OK)
	  {
		mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			       "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	  }
	}

	/* Read current FEC configuration */
    status = mvOnuEponMacRxpEncConfigGet(&isrP2pPreviousFecState);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
				 "ERROR: (%s:%d) mvOnuEponMacRxpEncConfigGet\n\r", __FILE_DESC__, __LINE__);
	  return(MV_ERROR);
	}

	/* Reset FEC configuration */
    status = mvOnuEponMacRxpEncConfigSet(0);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
				 "ERROR: (%s:%d) mvOnuEponMacRxpEncConfigSet\n\r", __FILE_DESC__, __LINE__);
	  return(MV_ERROR);
	}

    onuEponDbModeSet(E_EPON_IOCTL_P2P_MODE);

    /* Call link status callback function */
    linkStatusCallback = onuEponDbLinkStatusCallbackGet();
    if (linkStatusCallback != NULL)
    {
        linkStatusCallback(MV_TRUE);
        mvPonPrint(PON_PRINT_DEBUG, PON_ISR_INT_MODULE,
                   "DEBUG: (%s:%d) Notify link is UP\n", __FILE_DESC__, __LINE__);
    }
    /* TODO(kedong): Enable laser when the RX is ready. Need to be cleaned up
     * when wavelength selection code is ready.
     */
    onuPonTxLaserOn(MV_TRUE);

#endif

    return(MV_OK);
}

/*******************************************************************************
**
**  mvP2PStop
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function stops ONU P2P configuration
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS mvP2PStop(void)
{
#ifdef PON_Z2
    MV_STATUS status;
    MV_U32    pcsConfig;
    MV_U32    overHead;
    MV_U32    ignoreLlidCrcError;
    MV_U32    ignoreFcsError;
    MV_U32    ignoreGmiiError;
    MV_U32    ignoreLengthError;
    MV_U32    forwardAllLlid;
    MV_U32    forwardBc0FFF;
    MV_U32    forwardBc1FFF;
    MV_U32    forwardBc1xxx;
    MV_U32    dropBc1nnn;
    MV_U32    gpioGroup, gpioMask;
	MV_U32    interruptMask;

    /* Set EPON PCS */
    pcsConfig = onuEponDbPcsCfgGet();
    status = mvOnuEponMacPcsConfigSet(pcsConfig);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacPcsConfigSet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Set TXM configuration mode */
    status = asicOntGlbRegWrite(mvAsicReg_P2P_TXM_CFG_MODE, ONU_DEF_TXM_CFG_MODE_EPON, 0);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) asicOntGlbRegWrite\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Set configuration mode in the GEN ONT register */
    status = asicOntGlbRegWrite(mvAsicReg_P2P_GEN_ONT_MODE, ONU_DEF_TXM_CFG_MODE_EPON, 0);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) asicOntGlbRegWrite\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    overHead = onuEponDbOverheadGet();
    status = mvOnuEponMacTxmOverheadSet(overHead);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacTxmOverheadSet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

    /* Set EPON RXP packet filter */
    onuEponDbPktFilterPacketGet(&forwardAllLlid, &forwardBc1xxx, &forwardBc1FFF, &forwardBc0FFF, &dropBc1nnn);
    onuEponDbPktFilterErrorGet(&ignoreLengthError, &ignoreGmiiError, &ignoreFcsError, &ignoreLlidCrcError);

    status = mvOnuEponMacRxpPacketFilterSet(ignoreLlidCrcError,
                                            ignoreFcsError,
                                            ignoreGmiiError,
                                            ignoreLengthError,
                                            forwardAllLlid,
                                            forwardBc0FFF,
                                            forwardBc1FFF,
                                            forwardBc1xxx,
                                            dropBc1nnn);
    if (status != MV_OK)
    {
      mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
                 "ERROR: (%s:%d) mvOnuEponMacRxpPacketFilterSet\n\r", __FILE_DESC__, __LINE__);
      return(MV_ERROR);
    }

   /* MC ASIC     */
   /* =========== */
   if (MV_6601_DEV_ID == mvCtrlModelGet())
   {
	  /* PHY control register - output status set */
	   status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	   if (status != MV_OK)
	   {
	     mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
				 "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
	     return(MV_ERROR);
	   }

	   /* PHY control register - force disable */
	   status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_FORCE, 0, 0);
	   if (status != MV_OK)
	   {
		 mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			 "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
		 return(MV_ERROR);
	   }
   }
	/* KW2 ASIC Rev Z2 */
	/* =============== */
	else if (mvCtrlRevGet() == ONU_ASIC_REV_Z2)
	{
	  PON_GPIO_GET(BOARD_GPP_PON_XVR_TX, gpioGroup, gpioMask);
	  if (gpioMask != PON_GPIO_NOT_USED)
	  {
	  	status = mvGppTypeSet(gpioGroup, gpioMask, ~gpioMask/*output*/);
	  	if (status != MV_OK)
	  	{
	  	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  			     "ERROR: (%s:%d) mvGppTypeSet\n\r", __FILE_DESC__, __LINE__);
	  	  return(MV_ERROR);
	  	}

	  	status = mvGppValueSet(gpioGroup, gpioMask, ~gpioMask/*disable*/);
	  	if (status != MV_OK)
	  	{
	  	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  			     "ERROR: (%s:%d) mvGppValueSet\n\r", __FILE_DESC__, __LINE__);
	  	  return(MV_ERROR);
	  	}

	  	status = mvGppTypeSet(gpioGroup, gpioMask, gpioMask/*input*/);
	  	if (status != MV_OK)
	  	{
	  	  mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  			     "ERROR: (%s:%d) mvGppTypeSet\n\r", __FILE_DESC__, __LINE__);
	  	  return(MV_ERROR);
	  	}
	  }

	  /* PHY control register - output status set */
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	  if (status != MV_OK)
      {
	    mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  		       "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
	    return(MV_ERROR);
	  }
	}
	/* KW2 ASIC Rev A0 */
	/* =============== */
	else if (mvCtrlRevGet() == ONU_ASIC_REV_A0)
	{
	  /* PHY control register - output status set */
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN, ONU_PHY_OUTPUT, 0);
	  if (status != MV_OK)
	  {
	    mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  		        "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
	    return(MV_ERROR);
	  }

	  /* PHY control register - force disable */
	  status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_EN, 0, 0);
	  if (status != MV_OK)
	  {
		mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
			       "ERROR: (%s:%d) asicOntMiscRegWrite\n\r", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	  }
	}

		if (onuEponDbP2PForceModeGet())
		{
		  /* set P2P mode */
          onuEponDbModeSet(E_EPON_IOCTL_P2P_MODE);
		}
		else
		{
		  /* set EPON mode */
          onuEponDbModeSet(E_EPON_IOCTL_STD_MODE);
		}

	if (onuEponDbOnuSwRprtTimerTypeGet() == ONU_EPON_SW_DBA_RPRT_TIMER)
	{
	  /* Set onu DBA Report interrupt */
	  status |= mvOnuEponMacPonInterruptMaskGet(&interruptMask);
	  interruptMask |= (ONU_EPON_TIMESTAMP_VALUE_MATCH_MASK);
	  status |= mvOnuEponMacPonInterruptMaskSet(interruptMask);
	  if (status != MV_OK)
		return(status);

	  onuEponDbaSwRprtMacTimerCfg(onuEponDbOnuSwRprtMacTimerIntervalGet());

      /* Start SW DBA event miss timer */
	  onuEponIsrTimerEventMissStateSet(MV_TRUE);
    }
	else
	{
	  onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonHwRprtTimerId));
	}

	if (isrP2pPreviousFecState & 0x1)
	{
	  /* Set FEC configuration */
	  status = mvOnuEponMacRxpEncConfigSet(isrP2pPreviousFecState);
	  if (status != MV_OK)
	  {
	    mvPonPrint(PON_PRINT_ERROR, PON_ISR_MODULE,
	  			 "ERROR: (%s:%d) mvOnuEponMacRxpEncConfigSet\n\r", __FILE_DESC__, __LINE__);
	    return(MV_ERROR);
	  }
	}

	/* start xvr reset timer */
	onuEponIsrXvrResetStateSet(MV_TRUE);
#endif

    return(MV_OK);
}

void onuEponForceTxDownStateSet(MV_U32 txEnable, MV_U32 macId)
{
	if (txEnable == 0)
		eponMgmtForceTxDisable = MV_TRUE;
	else
		eponMgmtForceTxDisable = MV_FALSE;
}

MV_BOOL onuEponForceTxDownStateGet(MV_U32 macId)
{
	return(eponMgmtForceTxDisable);
}


