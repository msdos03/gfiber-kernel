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
**  FILE        : gponOnuIsr.c                                               **
**                                                                           **
**  DESCRIPTION : This file implements ONU GPON Interrupt handling           **
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
#define __FILE_DESC__ "mv_pon/core/gpon/gponOnuIsr.c"

#define MV_NO_XVR_RST (0xFF)

/* Global Variables
------------------------------------------------------------------------------*/
extern spinlock_t onuPonIrqLock;

/* Local Variables
------------------------------------------------------------------------------*/
MV_U32               g_onuGponPreviousInterrupt = ONU_GPON_INT_ALARMS;
MV_U32               g_onuGponCurrentInterrupt = 0;
S_OnuGponIsrCounters g_onuGponIsrCounters;
MV_BOOL              g_ponDebugMode = MV_FALSE;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
MV_STATUS onuGponIsrXvrReset(void);

/******************************************************************************/
/* ========================================================================== */
/*                         Operational Section                                */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuGponIsrInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function reset onu interrupt counters 
**               
**  PARAMETERS:  None 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK 
**                   
*******************************************************************************/
MV_STATUS onuGponIsrInit(void)
{
  MV_U32 i;

  g_onuGponIsrCounters.interruptReadError = 0;
  g_onuGponIsrCounters.ploamStatusNoInterrupt = 0;
  for (i = 0; i < ONU_GPON_ISR_TYPE_MAX ; i++)
  {
    g_onuGponIsrCounters.sendMessageError[i] = 0;
    g_onuGponIsrCounters.setEventError[i] = 0;
  }
  for (i = 0; i < ON_GPON_MAX_ALARMS ; i++)
  {
    g_onuGponIsrCounters.interruptSameStatus[i] = 0;
  }

  return(MV_OK);
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuGponIsrXvrResetTimerHndl
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function is called by the GPON Interrupt handler to execute 
**               XVR reset sequence in case of a problem with XVR signal detect
**               
**  PARAMETERS:  unsigned long data
**
**  OUTPUTS:     None
**
**  RETURNS:     None 
**                   
*******************************************************************************/
void onuGponIsrXvrResetTimerHndl(unsigned long data)
{
  MV_STATUS status;
  unsigned long flags;

  spin_lock_irqsave(&onuPonIrqLock, flags);

  onuPonResourceTbl_s.onuPonIsrXvrRstTimerId.onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;

  status = onuGponIsrXvrReset();
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
void onuGponDgIsrRoutine(void)
{
  onuGponPonMngIntrDgHandler();
}
#endif /* PON_FPGA */

/*******************************************************************************
**
**  onuGponIsrLowRoutine
**  onuGponIsrRoutine
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function implements interrupt polling mode, it polls the 
**               current status of an interrupt compare it to the previous 
**               status, and if an interrupt is active trigger the operation 
**               of the pon manager task.
**               
**  PARAMETERS:  None, void* param 
**
**  OUTPUTS:     None
**
**  RETURNS:     None, void* 
**                   
*******************************************************************************/

/*******************************************************************************
**
**  onuGponIsrLowRoutine
**                   
*******************************************************************************/
void onuGponIsrLowRoutine(MV_U32 *interruptEvent, MV_U32 *interruptStatus)
{
  MV_U32 interrupt;
  MV_U32 interruptRawEvent;
  MV_U32 bip8InterruptValue;
#ifdef MV_GPON_PERFORMANCE_CHECK
  S_GponPerformanceCheckNode *tmpPmCheckNode;
#endif /* MV_GPON_PERFORMANCE_CHECK */


#ifdef MV_GPON_PERFORMANCE_CHECK
  tmpPmCheckNode = &(g_GponPmCheck.pmCheckNode[PON_ISR_PERFORMANCE]);
  asicOntGlbRegReadNoCheck(mvAsicReg_GPON_GEN_MICRO_SEC_CNT, 
                             &(tmpPmCheckNode->uSecCntStart[tmpPmCheckNode->uSecCntIdx]), 0);
#endif /* MV_GPON_PERFORMANCE_CHECK */

  mvOnuGponMacPonInterruptGet(&interrupt);
  g_onuGponCurrentInterrupt &= 0xFFFF0000;
  g_onuGponCurrentInterrupt |= interrupt;

  interruptRawEvent = (g_onuGponCurrentInterrupt >> ONU_GPON_EVENT_SHIFT) & 
                                                 ONU_GPON_INTERRUPTS;
  
  if (interruptRawEvent & ONU_GPON_BIP_INERVAL_MASK)
  {  
    mvOnuGponMacBipInterruptStatusValueGet(&bip8InterruptValue);
    onuGponDbBipInterruptStatusValueSet(bip8InterruptValue);
  }

  *interruptEvent  = (g_onuGponCurrentInterrupt >> ONU_GPON_EVENT_SHIFT) & ONU_GPON_INTERRUPTS;
  *interruptStatus = (g_onuGponCurrentInterrupt                        ) & ONU_GPON_INTERRUPTS;

  onuGponSyncLog(ONU_GPON_LOG_INTERRUPT, g_onuGponCurrentInterrupt, 0, 0);

  g_onuGponCurrentInterrupt = 0;

#ifdef MV_GPON_PERFORMANCE_CHECK
  asicOntGlbRegReadNoCheck(mvAsicReg_GPON_GEN_MICRO_SEC_CNT, 
                           &(tmpPmCheckNode->uSecCntStop[tmpPmCheckNode->uSecCntIdx]), 0);
  if(tmpPmCheckNode->uSecCntIdx < 255) tmpPmCheckNode->uSecCntIdx++;
#endif /* MV_GPON_PERFORMANCE_CHECK */

}

/*******************************************************************************
**
**  onuGponIsrRoutine
**                   
*******************************************************************************/
void onuGponIsrRoutine(MV_U32 event, MV_U32 status)
{
  MV_BOOL state;
  MV_U32 interruptEvent;
  MV_U32 interruptStatus;
#ifdef MV_GPON_DEBUG_PRINT
  MV_U32 timer = 0;
#endif

#ifdef MV_GPON_HW_INTERRUPT
  onuGponIsrLowRoutine(&interruptEvent, &interruptStatus);
#else
  interruptEvent  = event; 
  interruptStatus = status;
#endif /* MV_GPON_HW_INTERRUPT */

  if (g_ponDebugMode != MV_TRUE)
  {
#ifdef MV_GPON_DEBUG_PRINT
    asicOntGlbRegReadNoCheck(mvAsicReg_GPON_GEN_MICRO_SEC_CNT, &timer, 0);
    mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE, 
               "DEBUG: (%s:%d) Event(0x%08x) Status(0x%08x) timestamp(%d)\n", 
               __FILE_DESC__, __LINE__, interruptEvent, interruptStatus, timer); 
#endif

    if (interruptEvent & ONU_GPON_XVR_SIGNAL_DETECT_STATUS_MASK)
    {
        state = ponXvrFunc(interruptStatus, ONU_GPON_XVR_SIGNAL_DETECT_STATUS_MASK);

	  if (state == MV_TRUE) {
		onuGponPonMngIntrAlarmHandler(ONU_PON_MNGR_LOS_ALARM, MV_FALSE);
		/* Set signal detect to ON */
		onuGponDbOnuSignalDetectSet(1);

#ifdef MV_GPON_DEBUG_PRINT
		mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE,
			   "DEBUG: (%s:%d) Set signal detect to ON = 1 \n", __FILE_DESC__, __LINE__);
#endif /* MV_GPON_DEBUG_PRINT */
#ifndef PON_FPGA
		onuGponIsrXvrReset();
		onuGponIsrXvrResetStateSet(MV_TRUE);
#endif /* PON_FPGA */
	  }
	  else if (state == MV_FALSE)
	  {
		onuGponPonMngIntrAlarmHandler(ONU_PON_MNGR_LOS_ALARM, MV_TRUE);
		/* Set signal detect to OFF */
		onuGponDbOnuSignalDetectSet(0);

#ifdef MV_GPON_DEBUG_PRINT
		mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE,
			   "DEBUG: (%s:%d) Set signal detect to OFF = 0\n", __FILE_DESC__, __LINE__);
#endif /* MV_GPON_DEBUG_PRINT */
	  }

	  onuGponSyncLog(ONU_GPON_LOG_INTERRUPT_XVR_SD, state, 0, 0);
    }
    
    if (interruptEvent & ONU_GPON_LOS_ALARM_MASK)
    {
     // state = (interruptStatus & ONU_GPON_LOS_ALARM_MASK) ? MV_TRUE : MV_FALSE;
     // onuGponPonMngIntrAlarmHandler(ONU_PON_MNGR_LOS_ALARM, state);
    }

    if (interruptEvent & ONU_GPON_LOF_ALARM_MASK)
    {
      state = (interruptStatus & ONU_GPON_LOF_ALARM_MASK) ? MV_TRUE : MV_FALSE;
	  if ((ONU_GPON_XVR_SD_LOW_ACTIVE == onuGponDbXvrSdPolarityGet()) &&
	  (state != MV_TRUE)) {
            onuGponPonMngIntrAlarmHandler(ONU_PON_MNGR_LOS_ALARM, MV_FALSE);
            /* Set signal detect to ON */
            onuGponDbOnuSignalDetectSet(1);

#ifdef MV_GPON_DEBUG_PRINT
            mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE,
                        "DEBUG: (%s:%d) Set signal detect to ON = 1 \n", __FILE_DESC__, __LINE__);
#endif /* MV_GPON_DEBUG_PRINT */
#ifndef PON_FPGA
            onuGponIsrXvrReset();
            onuGponIsrXvrResetStateSet(MV_TRUE);
            //onuGponPonMngIntrMessageHandler();
#endif /* PON_FPGA */
      }
      onuGponPonMngIntrAlarmHandler(ONU_PON_MNGR_LOF_ALARM, state);

      onuGponSyncLog(ONU_GPON_LOG_INTERRUPT_LOF, state, 0, 0);

	 // if (state == MV_TRUE) 
	 // {
  	 //   /* Set DS Sync Ind to OFF */
	 //   //onuGponDbOnuDsSyncOnSet(0);
	 // }
	 // else if (state == MV_FALSE)
	 // {
	 //   /* Set DS Sync Ind  to ON */
	 //   //onuGponDbOnuDsSyncOnSet(1);
	 // }
     if (state != MV_TRUE) 
        onuGponPonMngIntrMessageHandler();
    }

    if (interruptEvent & ONU_GPON_LCDG_ALARM_MASK)
    {
     // state = (interruptStatus & ONU_GPON_LCDG_ALARM_MASK) ? MV_TRUE : MV_FALSE;
     // onuGponPonMngIntrAlarmHandler(ONU_PON_MNGR_LCDG_ALARM, state);
    }

    if (interruptEvent & ONU_GPON_DS_PLOAM_MASK)
    {
      onuGponPonMngIntrMessageHandler();
    }

    if (interruptEvent & ONU_GPON_BIP_INERVAL_MASK)
    {
      onuGponPonMngIntrBeCounterHandler();
    }

    if (interruptEvent & ONU_GPON_RAM_TEST_GRX_PARITY_ERR_MASK)
    {
#ifdef MV_GPON_DEBUG_PRINT
      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE, 
                 "DEBUG: (%s:%d) Event 0x%x - Not supported\n", __FILE_DESC__, __LINE__, interruptEvent);
#endif /* MV_GPON_DEBUG_PRINT */
    }

    if (interruptEvent & ONU_GPON_RAM_TEST_GEM_PARITY_ERR_MASK)
    {
#ifdef MV_GPON_DEBUG_PRINT
      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE, 
                 "DEBUG: (%s:%d) Event 0x%x - Not supported\n", __FILE_DESC__, __LINE__, interruptEvent);
#endif /* MV_GPON_DEBUG_PRINT */
    }

    if (interruptEvent & ONU_GPON_RAM_TEST_GTX_PARITY_ERR_MASK)
    {
#ifdef MV_GPON_DEBUG_PRINT
      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE, 
                 "DEBUG: (%s:%d) Event 0x%x - Not supported\n", __FILE_DESC__, __LINE__, interruptEvent);
#endif /* MV_GPON_DEBUG_PRINT */
    }

    if (interruptEvent & ONU_GPON_RAM_TEST_UTM_PARITY_ERR_MASK)
    {
#ifdef MV_GPON_DEBUG_PRINT
      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE, 
                 "DEBUG: (%s:%d) Event 0x%x - Not supported\n", __FILE_DESC__, __LINE__, interruptEvent);
#endif /* MV_GPON_DEBUG_PRINT */
    }

    if (interruptEvent & ONU_GPON_PHY_READY_STATUS_MASK)
    {
#ifdef MV_GPON_DEBUG_PRINT
      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE, 
                 "DEBUG: (%s:%d) Event 0x%x - Not supported\n", __FILE_DESC__, __LINE__, interruptEvent);
#endif /* MV_GPON_DEBUG_PRINT */
    }

    if (interruptEvent & ONU_GPON_PHY_SIGNAL_DETECT_STATUS_MASK)
    {
#ifdef MV_GPON_DEBUG_PRINT
      mvPonPrint(PON_PRINT_DEBUG, PON_ISR_MODULE,
                 "DEBUG: (%s:%d) Event 0x%x - Not supported\n", __FILE_DESC__, __LINE__, interruptEvent);
#endif /* MV_GPON_DEBUG_PRINT */
    }
  }
}

/*******************************************************************************
**
**  onuGponPonMngDebugModeSet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu debug mode
**               
**  PARAMETERS:  MV_BOOL mode 
**
**  OUTPUTS:     None
**
**  RETURNS:     None  
**
*******************************************************************************/
void onuGponPonMngDebugModeSet(MV_BOOL mode)
{
  g_ponDebugMode = mode;
}

/*******************************************************************************
**
**  onuGponPonMngDebugModeGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function returns onu debug mode
**               
**  PARAMETERS:  None  
**
**  OUTPUTS:     None
**
**  RETURNS:     Debug mode   
**
*******************************************************************************/
MV_BOOL onuGponPonMngDebugModeGet(void)
{
  return(g_ponDebugMode);
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuGponIsrXvrReset
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
MV_STATUS onuGponIsrXvrReset(void)
{
  MV_STATUS status;
  MV_U32    onuState;
  MV_U32    initDone;
  MV_U32    durationStart = 0;
  MV_U32    durationEnd   = 0;


  onuState = onuGponDbOnuStateGet();
  if ((onuState == ONU_GPON_01_INIT) ||
     ((onuState == ONU_GPON_07_EMERGANCY_STOP) && (onuGponAsicAlarmStatusGet() == ONU_GPON_ALARM_ON)) ||
     ((onuState == ONU_GPON_06_POPUP)          && ((onuGponAlarmGet(ONU_GPON_ALARM_LOS) == ONU_GPON_ALARM_OFF) &&
                                                   (onuGponAlarmGet(ONU_GPON_ALARM_LOF) == ONU_GPON_ALARM_ON))))
  {
    asicOntGlbRegReadNoCheck(mvAsicReg_GPON_GEN_MICRO_SEC_CNT, &durationStart, 0);
    onuGponSyncLog(ONU_GPON_LOG_INTERRUPT_SERDES_START, 0, 0, 0);

    status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT, 0x1, 0);  
    if (status != MV_OK) 
      return(status);

    do 
    {
      status  = asicOntMiscRegRead(mvAsicReg_PON_SERDES_PHY_CTRL_0_INIT_DONE, &initDone, 0);  
      if (status != MV_OK) 
        return(status);

      asicOntGlbRegReadNoCheck(mvAsicReg_GPON_GEN_MICRO_SEC_CNT, &durationEnd, 0);
      if (abs((durationEnd - durationStart)) >= 6000)
	      return (MV_OK);

    } while (initDone == 0);

    status  = asicOntMiscRegWrite(mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT, 0x0, 0);  
    if (status != MV_OK) 
      return(status);

    onuGponSyncLog(ONU_GPON_LOG_INTERRUPT_SERDES_STOP, 0, 0, 0);
  }
  else
  {
	return(MV_NO_XVR_RST);
  }

  return (MV_OK);
}

/*******************************************************************************
**
**  onuGponIsrXvrResetStateSet
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
MV_STATUS onuGponIsrXvrResetStateSet(MV_BOOL mode)
{
  MV_STATUS rcode = MV_OK;

  if (mode == MV_TRUE)
  {
    onuPonTimerEnable(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId));
  }
  else
  {
    onuPonTimerDisable(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId));
  }

  return (rcode);
}
#endif /* PON_FPGA */


