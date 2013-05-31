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
**  FILE        : gponOnuLnxKsOs.c                                           **
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
#include "gponOnuHeader.h"
#ifndef PON_FPGA
#include "mvSysPonConfig.h"
#endif /* PON_FPGA */

/* Local Constant
------------------------------------------------------------------------------*/     
#define __FILE_DESC__ "mv_pon/plat/gpon/gponOnuLnxKsOs.c"

/* Global Variables
------------------------------------------------------------------------------*/
S_onuPonWork      gponTcontCleanWork[8];
S_onuPonWork      gponTcontCleanAllWork;
S_onuPonWork      gponTcontActiveWork[8];
S_onuPonWorkQueue gponTcontFlushWq;

/* Global functions
------------------------------------------------------------------------------*/

/* Local Variables
------------------------------------------------------------------------------*/

/* ========================================================================== */
/* ===========================  ISR SECTION  ================================ */
/* ========================================================================== */
MV_U32 gponCurrentInterruptEvent  = 0;
MV_U32 gponCurrentInterruptStatus = 0;


/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
void        onuGponTaskletFunc(unsigned long dummy);
irqreturn_t onuGponIrqRoutine(int irq, void *arg);
#ifndef PON_FPGA
irqreturn_t onuGponDgIrqRoutine(int irq, void *arg);
#endif /* PON_FPGA */


/*******************************************************************************
**
**  onuPonWqInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function allocates GPON work queue and init two work
**  		 structures for TCONT clean and activate
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuPonWqInit(void)
{
  MV_U32 tcontIndex;

  gponTcontFlushWq.ponWq = create_workqueue("gponTcontFlushWQ");
  if (gponTcontFlushWq.ponWq)
  {
    for (tcontIndex = 0; tcontIndex < 8; tcontIndex++)
    {
	    INIT_WORK((struct work_struct *) &gponTcontCleanWork[tcontIndex], onuGponWqTcontFunc);
	    gponTcontCleanWork[tcontIndex].action = TCONT_CLEAN_EVENT;
	    gponTcontCleanWork[tcontIndex].param  = tcontIndex;

	    INIT_WORK((struct work_struct *) &gponTcontActiveWork[tcontIndex], onuGponWqTcontFunc);
	    gponTcontActiveWork[tcontIndex].action = TCONT_ACTIVE_EVENT;
	    gponTcontActiveWork[tcontIndex].param  = tcontIndex;
    }

    INIT_WORK((struct work_struct *) &gponTcontCleanAllWork, onuGponWqTcontFunc);
    gponTcontCleanAllWork.action = TCONT_CLEAN_ALL_EVENT;

    printk("STARTING PON WORK QUEUE!!!!!\n");

    return(MV_OK);
  }

  return(MV_ERROR);
}

/*******************************************************************************
**
**  onuGponRtosResourceInit
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
MV_STATUS onuGponRtosResourceInit(void)
{
  MV_STATUS retcode;

  /* Timer */
  /* ===== */

  /* onu gpon T01 timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuGponT01_TimerId),   /* timer Id */
                               "gpon_T01",                                 /* timer description */
                               (PTIMER_FUNCPTR)onuGponPonMngTimerT01Hndl,  /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                   /* timer active (run) state */
                               ONU_PON_TIMER_T01_INTERVAL,                 /* init value */ 
                               0);                                         /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon T01 timer create\n", __FILE_DESC__, __LINE__);
    return(retcode);
  }

  /* onu gpon T02 timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuGponT02_TimerId),   /* timer Id */
                               "gpon_T02",                                 /* timer description */
                               (PTIMER_FUNCPTR)onuGponPonMngTimerT02Hndl,  /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                   /* timer active (run) state */
                               ONU_PON_TIMER_T02_INTERVAL,                 /* init value */ 
                               0);                                         /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon T02 timer create\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu gpon PEE timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuGponPeeTimerId),    /* timer Id */
                               "gpon_Pee",                                 /* timer description */
                               (PTIMER_FUNCPTR)onuGponPonMngTimerPeeHndl,  /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                   /* timer active (run) state */
                               ONU_PON_TIMER_PEE_INTERVAL,                 /* init value */ 
                               0);                                         /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon Pee timer create\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

#ifndef PON_FPGA
  /* onu gpon xvr reset timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonIsrXvrRstTimerId),    /* timer Id */
                               "gpon_xvr",                                      /* timer description */
                               (PTIMER_FUNCPTR)onuGponIsrXvrResetTimerHndl,     /* timer function */
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

  /* onu gpon pm timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonPmTimerId),      /* timer Id */
                               "gpon_Pm",                                  /* timer description */
                               (PTIMER_FUNCPTR)onuGponPmTimerPmHndl,       /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                   /* timer active (run) state */
                               ONU_PON_TIMER_PM_INTERVAL,                  /* init value */ 
                               1);                                         /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon PM timer create\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu gpon Pattern Burst timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonPatternBurstTimerId), /* timer Id */
                              "gpon_PatternBurst",                              /* timer description */
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

  /* onu gpon sw fifo timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonSwFIFOTimerId),  /* timer Id */
                               "gpon_Fifo",                                /* timer description */
                               (PTIMER_FUNCPTR)onuGponTimerFifoAuditHndl,  /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                   /* timer active (run) state */
                               ONU_PON_TIMER_FIFO_AUDIT_INTERVAL,          /* init value */ 
                               1);                                         /* periodic value */ 
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon FIFO Audit timer create\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* onu pon TX Power timer */
  retcode = onuPonTimerCreate(&(onuPonResourceTbl_s.onuPonTxPwrTimerId),   /* timer Id */
                               "pon_txPwr",                                /* timer description */
                               (PTIMER_FUNCPTR)onuGponTimerTxPwrHndl,      /* timer function */
                               ONU_PON_TIMER_NOT_ACTIVE,                   /* timer active (run) state */
                               ONU_PON_TIMER_TX_PWR_INTERVAL,              /* init value */
                               0);                                         /* periodic value */
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) pon Tx Power timer create\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  /* Work Queue */
  /* ========== */
  retcode = onuPonWqInit();
  if (retcode != MV_OK)
  {
     mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
                "ERROR: (%s:%d) pon work queue create\n", __FILE_DESC__, __LINE__);
     return(MV_ERROR);
  }

  /* Interrupt */
  /* ========= */
  retcode = onuPonIrqInit();
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) gpon interrupt init\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  retcode = onuGponIrqTaskletInit(&(onuPonResourceTbl_s.onuPonIrqId));
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) gpon interrupt tasklet\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  retcode = onuGponIrqRegister(&(onuPonResourceTbl_s.onuPonIrqId));      
  if (retcode != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) gpon interrupt register\n", __FILE_DESC__, __LINE__);
    return(MV_ERROR);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponRtosResourceRelease
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
MV_STATUS onuGponRtosResourceRelease(void)
{
  del_timer(&(onuPonResourceTbl_s.onuGponT01_TimerId.onuPonTimerId));
  del_timer(&(onuPonResourceTbl_s.onuGponT02_TimerId.onuPonTimerId));
  del_timer(&(onuPonResourceTbl_s.onuGponPeeTimerId.onuPonTimerId));
  del_timer(&(onuPonResourceTbl_s.onuPonSwFIFOTimerId.onuPonTimerId));
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
**  onuGponIrqTaskletInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt tasklet init
**               
**  PARAMETERS:  S_onuPonIrq irqId
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS onuGponIrqTaskletInit(S_onuPonIrq *irqId)
{
  tasklet_init(&(irqId->onuPonTasklet), onuGponTaskletFunc, (unsigned int)0);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponIrqRegister
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt register
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     GPON Interrupt number
**
*******************************************************************************/
MV_STATUS onuGponIrqRegister(S_onuPonIrq *irqId)
{
  int rcode;

  rcode = request_irq(irqId->onuPonIrqNum, onuGponIrqRoutine, IRQF_DISABLED, "mvGpon", (void*)irqId);
  if (rcode) 
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) gpon interrupt register\n", __FILE_DESC__, __LINE__);
    free_irq(irqId->onuPonIrqNum, NULL);
    return(MV_ERROR);
  }

#ifndef PON_FPGA
  rcode = request_irq(irqId->onuDgIrqNum, onuGponDgIrqRoutine, IRQF_DISABLED, "mvDyGasp", (void*)irqId);
  if (rcode) 
  {
    mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
               "ERROR: (%s:%d) GPON Dying Gasp interrupt register\n", __FILE_DESC__, __LINE__);
    free_irq(irqId->onuDgIrqNum, NULL);
    return(MV_ERROR);
  }
#endif /* PON_FPGA */

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponIrqRoutine
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt routinr
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IRQ_HANDLED
**
*******************************************************************************/
irqreturn_t onuGponIrqRoutine(int irq, void *arg)
{
#ifdef MV_GPON_HW_INTERRUPT
  onuGponIsrRoutine(0, 0);
#else /* SW_INTERRUPT - TASKLET */
  MV_U32 interruptEvent;
  MV_U32 interruptStatus;

  S_onuPonIrq *irqId = (S_onuPonIrq*)arg;
  onuGponIsrLowRoutine(&interruptEvent, &interruptStatus);
  gponCurrentInterruptEvent  = interruptEvent;
  gponCurrentInterruptStatus = interruptStatus;
  tasklet_hi_schedule(&(irqId->onuPonTasklet));
#endif /* MV_GPON_HW_INTERRUPT */

  return(IRQ_HANDLED);
}

#ifndef PON_FPGA
/*******************************************************************************
**
**  onuGponDgIrqRoutine
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
irqreturn_t onuGponDgIrqRoutine(int irq, void *arg)
{
  mvOnuPonMacDgInterruptDis();
  onuPonDyingGaspProcess();
  onuGponDgIsrRoutine();
  
  return(IRQ_HANDLED);
}
#endif /* PON_FPGA */

/*******************************************************************************
**
**  onuGponTaskletFunc
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt tasklet function
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     GPON Interrupt number
**
*******************************************************************************/
void onuGponTaskletFunc(unsigned long dummy)
{
  MV_U32 interruptEvent  = gponCurrentInterruptEvent;  
  MV_U32 interruptStatus = gponCurrentInterruptStatus; 

  onuPonIrqDisable(&(onuPonResourceTbl_s.onuPonIrqId)); /* disable GPON interrupt */

  onuGponIsrRoutine(interruptEvent, interruptStatus);

  onuPonIrqEnable(&(onuPonResourceTbl_s.onuPonIrqId)); /* enable GPON interrupt */
}


/*******************************************************************************
**
**  gponOnuGlbAddrInit
**  ____________________________________________________________________________
**
**  DESCRIPTION: Allocate a memory space for GPON MAC register space
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IAMBA_OK
**
*******************************************************************************/
MV_STATUS onuGponGlbAddrInit(void)
{
#ifndef PON_FPGA
  /* set base address in the pon onu register table */
  ponOnuGlbAddrSet((MV_U32)(MV_GPON_MAC_REGS_BASE));
#else /* PON_FPGA */
  /* set base address in the pon onu register table */
  ponOnuGlbAddrSet((MV_U32)(MV_GPON_REG_BASE + 0xC000));
#endif /* PON_FPGA */

  return (MV_OK);
}             

