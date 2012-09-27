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
**  FILE        : eponOnuLnxKsOs.c                                           **
**                                                                           **
**  DESCRIPTION : This file implements ONU GPON Linux OS handling            **
*******************************************************************************
*                                                                             *                              
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *  
* =========================================================================== *      
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include "ponOnuHeader.h"
#include "eponOnuHeader.h"
#ifndef PON_FPGA
#include "mvSysPonConfig.h"
#endif /* PON_FPGA */

/* Local Constant
------------------------------------------------------------------------------*/     
#define __FILE_DESC__ "mv_pon/plat/epon/eponOnuLnxKsOs.c"

/* Global Variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Local Variables
------------------------------------------------------------------------------*/

/* ========================================================================== */
/* ===========================  ISR SECTION  ================================ */
/* ========================================================================== */
MV_U32 eponCurrentInterruptEvent  = 0;
MV_U32 eponCurrentInterruptStatus = 0;


/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
void        onuEponTaskletFunc(unsigned long dummy);
irqreturn_t onuEponIrqRoutine(int irq, void *arg);
#ifndef PON_FPGA
irqreturn_t onuEponDgIrqRoutine(int irq, void *arg);
#endif /* PON_FPGA */


/*******************************************************************************
**
**  onuEponRtosResourceInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function allocates onu RTOS resources
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error          
**                   
*******************************************************************************/
MV_STATUS onuEponRtosResourceInit(void)
{
  MV_STATUS retcode;
  MV_U32    idx;

  /* Timer */
  /* ===== */

  /* onu epon pm timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonPmTimerId),           /* timer Id */
                              "epon_Pm",                                        /* timer description */
                              (PTIMER_FUNCPTR)onuEponPmTimerPmHndl,             /* timer function */
                              0,                                                /* timer function param */
                              ONU_PON_TIMER_PM_INTERVAL,                        /* init value */ 
                              1);                                               /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
               "ERROR: (%s:%d) pon PM timer create\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu epon Pattern Burst timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonPatternBurstTimerId), /* timer Id */
                              "epon_PatternBurst",                              /* timer description */
                              (PTIMER_FUNCPTR)onuPonPatternBurstTimerHndl,      /* timer function */
                              0,                                                /* timer function param */
                              ONU_PON_TIMER_NOT_ACTIVE ,                        /* init value */ 
                              0);                                               /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
               "ERROR: (%s:%d) pon Pattern Burst timer create\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu epon mpcp timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonMpcpTimerId),         /* timer Id */
                              "epon_mpc",                                       /* timer description */
                              (PTIMER_FUNCPTR)onuEponIsrTimerMpcpHndl,          /* timer function */
                              ONU_PON_TIMER_NOT_ACTIVE,                         /* timer function param */
                              ONU_PON_TIMER_MPCP_INTERVAL,                      /* init value */ 
                              1);                                               /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
               "ERROR: (%s:%d) pon mpcp timer create\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu epon Isr miss timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonIsrMissTimerId),      /* timer Id */
							  "epon_tx",                                        /* timer description */
							  (PTIMER_FUNCPTR)onuEponIsrTimerEventMissHndl,     /* timer function */
							  ONU_PON_TIMER_NOT_ACTIVE,                         /* timer function param */
							  ONU_PON_TIMER_EVENT_MISS_INTERVAL,                /* init value */ 
							  1);                                               /* periodic value */ 
  if (retcode != MV_OK)
  {
	mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
			   "ERROR: (%s:%d) pon event miss timer create\n\r", __FILE_DESC__, __LINE__);
	return(MV_ERROR);
  }

#ifndef PON_FPGA
  /* onu epon xvr reset timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId),    /* timer Id */
                               "gpon_xvr",                                      /* timer description */
                               (PTIMER_FUNCPTR)onuEponIsrXvrResetTimerHndl,     /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                        /* timer active (run) state */
                               ONU_PON_TIMER_XVR_RST_INTERVAL,                  /* init value */ 
                               1);                                              /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon Cvr Reset timer create\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }
#endif /* PON_FPGA */

  
   /* onu epon report timer */
   retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonHwRprtTimerId),     /* timer Id */
                               "epon_rprt",                                    /* timer description */
                               (PTIMER_FUNCPTR)onuEponIsrTimerHwReportHndl,    /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                       /* timer function param */
                               ONU_PON_TIMER_RPRT_INTERVAL,                    /* init value */ 
                               0);                                             /* periodic value */ 
   if (retcode != MV_OK)
   {
     mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                "ERROR: (%s:%d) pon report timer create\n\r", __FILE_DESC__, __LINE__);
     return(MV_ERROR);
   }
 
   /* onu epon tx module timer */
   retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonHwRprtTxModTimerId),  /* timer Id */
                               "epon_tx",                                        /* timer description */
                               (PTIMER_FUNCPTR)onuEponPonMngTimerHwRprtTxModHndl,/* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                         /* timer function param */
                               ONU_PON_TIMER_TX_MOD_INTERVAL,                    /* init value */ 
                               1);                                               /* periodic value */ 
   if (retcode != MV_OK)
   {
     mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                "ERROR: (%s:%d) pon report timer create\n\r", __FILE_DESC__, __LINE__);
     return(MV_ERROR);
   }
  

  /* onu epon holdover timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonHoldoverTimerId),     /* timer Id */
                              "epon_hold",                                      /* timer description */
                              (PTIMER_FUNCPTR)onuEponIsrTimerHoldoverHndl,      /* timer function */
                              ONU_PON_TIMER_NOT_ACTIVE,                         /* timer function param */
                              ONU_PON_TIMER_HOLDOVER_INTERVAL,                  /* init value */ 
                              0);                                               /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
               "ERROR: (%s:%d) pon holdover timer create\n\r", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu epon silence timer */
  for (idx = 0; idx < EPON_MAX_MAC_NUM; idx++) 
  {
	retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonSilenceTimerId[idx]),/* timer Id */
	  						    "epon_Sile",                                       /* timer description */
	  						    (PTIMER_FUNCPTR)onuEponPonMngTimerSilenceHndl,     /* timer function */
	  						    ONU_PON_TIMER_NOT_ACTIVE,                          /* timer function param */
	  						    ONU_PON_TIMER_SILENCE_INTERVAL,                    /* init value */ 
	  						    0);                                                /* periodic value */ 
	if (retcode != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
	  		   "ERROR: (%s:%d) pon holdover timer create\n\r", __FILE_DESC__, __LINE__);
	  return(MV_ERROR);
	}
  }


  /* Interrupt */
  /* ========= */
  retcode = onuPonIrqInit();
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) epon interrupt init\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  retcode = onuEponIrqTaskletInit(&(onuPonResourceTbl_s.onuPonIrqId));
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) epon interrupt tasklet\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  retcode = onuEponIrqRegister(&(onuPonResourceTbl_s.onuPonIrqId));      
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) epon interrupt register\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponRtosResourceRelease
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function release onu RTOS resources
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error          
**                   
*******************************************************************************/
MV_STATUS onuEponRtosResourceRelease(void)
{
  del_timer(&(onuPonResourceTbl_s.onuPonMpcpTimerId.onuPonTimerId));
  del_timer(&(onuPonResourceTbl_s.onuPonPmTimerId.onuPonTimerId));

  free_irq(onuPonResourceTbl_s.onuPonIrqId.onuPonIrqNum, &(onuPonResourceTbl_s.onuPonIrqId));
#ifndef PON_FPGA
  printk("onuPonResourceTbl_s.onuPonIrqId.onuDgIrqNum, &(onuPonResourceTbl_s.onuPonIrqId\n\r");
  free_irq(onuPonResourceTbl_s.onuPonIrqId.onuDgIrqNum, &(onuPonResourceTbl_s.onuPonIrqId));
#endif /* PON_FPGA */

  tasklet_kill(&(onuPonResourceTbl_s.onuPonIrqId.onuPonTasklet));

  return(MV_OK);
}


/*******************************************************************************
**
**  onuEponIrqTaskletInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: EPON Interrupt tasklet init
**               
**  PARAMETERS:  S_onuPonIrq irqId
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuEponIrqTaskletInit(S_onuPonIrq *irqId)
{
  tasklet_init(&(irqId->onuPonTasklet), onuEponTaskletFunc, (unsigned int)0);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponIrqRegister
**  ____________________________________________________________________________
**
**  DESCRIPTION: EPON Interrupt register
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     EPON Interrupt number
**
*******************************************************************************/
MV_STATUS onuEponIrqRegister(S_onuPonIrq *irqId)
{
  int rcode;

  rcode = request_irq(irqId->onuPonIrqNum, onuEponIrqRoutine, IRQF_DISABLED, "mvEpon", (void*)irqId);
  if (rcode) 
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) epon interrupt register\n", __FILE_DESC__, __LINE__);
    free_irq(irqId->onuPonIrqNum, NULL);
    return(MV_ERROR);
  }

#ifndef PON_FPGA
  rcode = request_irq(irqId->onuDgIrqNum, onuEponDgIrqRoutine, IRQF_DISABLED, "mvDyGasp", (void*)irqId);
  if (rcode) 
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) EPON Dying Gasp interrupt register\n", __FILE_DESC__, __LINE__);
    free_irq(irqId->onuDgIrqNum, NULL);
    return(MV_ERROR);
  }
#endif /* PON_FPGA */

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponIrqRoutine
**  ____________________________________________________________________________
**
**  DESCRIPTION: EPON Interrupt routinr
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IRQ_HANDLED
**
*******************************************************************************/
irqreturn_t onuEponIrqRoutine(int irq, void *arg)
{
#ifdef MV_EPON_HW_INTERRUPT
  onuEponIsrRoutine(0, 0);
#else /* SW_INTERRUPT - TASKLET */
  MV_U32 interruptEvent;
  MV_U32 interruptStatus;

  S_onuPonIrq *irqId = (S_onuPonIrq*)arg;
  onuEponIsrLowRoutine(&interruptEvent, &interruptStatus);
  eponCurrentInterruptEvent  = interruptEvent;
  eponCurrentInterruptStatus = interruptStatus;
  tasklet_hi_schedule(&(irqId->onuPonTasklet));
#endif 

  return(IRQ_HANDLED);
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuEponDgIrqRoutine
**  ____________________________________________________________________________
**
**  DESCRIPTION: Dying Gasp Interrupt routinr
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IRQ_HANDLED
**
*******************************************************************************/
irqreturn_t onuEponDgIrqRoutine(int irq, void *arg)
{
  mvOnuPonMacDgInterruptDis();
  onuPonDyingGaspProcess();
  onuEponDgIsrRoutine();

  return(IRQ_HANDLED);
}
#endif /* PON_FPGA */

/*******************************************************************************
**
**  onuEponTaskletFunc
**  ____________________________________________________________________________
**
**  DESCRIPTION: EPON Interrupt tasklet function
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     GPON Interrupt number
**
*******************************************************************************/
void onuEponTaskletFunc(unsigned long dummy)
{
  MV_U32 interruptEvent  = eponCurrentInterruptEvent;  
  MV_U32 interruptStatus = eponCurrentInterruptStatus; 

  onuPonIrqDisable(&(onuPonResourceTbl_s.onuPonIrqId)); /* disable EPON interrupt */

  onuEponIsrRoutine(interruptEvent, interruptStatus);

  onuPonIrqEnable(&(onuPonResourceTbl_s.onuPonIrqId)); /* enable EPON interrupt */
}

/*******************************************************************************
**
**  onuEponGlbAddrInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: Allocate a memory space for EPON MAC register space
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IAMBA_OK
**
*******************************************************************************/
MV_STATUS onuEponGlbAddrInit(void)
{
#ifndef PON_FPGA
  /* set base address in the pon onu register table */
  ponOnuGlbAddrSet((MV_U32)(MV_EPON_MAC_REGS_BASE));
#else /* PON_FPGA */
  /* set base address in the pon onu register table */
  ponOnuGlbAddrSet((MV_U32)(MV_GPON_REG_BASE + 0xC000));
#endif /* PON_FPGA */

  return (MV_OK);
}             



