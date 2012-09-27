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
**  FILE        : ponOnuLnxKsOs.c                                            **
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
#include "ponOnuLnxKsOs.h"
#ifndef PON_FPGA
#include "mvSysPonConfig.h"
#endif /* PON_FPGA */

/* Local Constant
------------------------------------------------------------------------------*/     
#define __FILE_DESC__ "mv_pon/plat/ponOnuLnxKsOs.c"

/* Global Variables
------------------------------------------------------------------------------*/
S_OnuPonResourceTbl onuPonResourceTbl_s;

S_PonModulePrint gponModulePrint_s[PON_LAST_MODULE + 1] =      
{                                                      
                           {0,0},                                 
  /* PON_MAC_MODULE   */  {PON_PRINT_INFO, 0}, /* No Options */                                
  /* PON_INIT_MODULE  */  {PON_PRINT_INFO, 0}, /* No Options */                                
  /* PON_ISR_MODULE   */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_CLI_MODULE   */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_MNG_MODULE   */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_SM_MODULE    */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_PM_MODULE    */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_ALARM_MODULE */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_BER_MODULE   */  {PON_PRINT_INFO, 0}, /* No Options */                           
  /* PON_API_MODULE   */  {PON_PRINT_INFO, 0}, /* No Options */                               
  /* PON_ALLOC_MODULE */  {PON_PRINT_INFO, 0}  /* No Options */                                
};

/* Global functions
------------------------------------------------------------------------------*/
spinlock_t onuPonIrqLock;

/* Local Variables
------------------------------------------------------------------------------*/

/* ========================================================================== */
/* ===========================  MEMORY SECTION  ============================= */
/* ========================================================================== */
void*  onuPonMemoryAllocArray[128];
MV_U32 onuPonMemoryAllocIndex = 0;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/

/*******************************************************************************
**
**  onuPonIrqInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function inits PON interrupts
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error          
**                   
*******************************************************************************/
MV_STATUS onuPonIrqInit(void)
{

  /* Interrupt */
  /* ========= */
  onuPonIrqNumInit();

  spin_lock_init(&onuPonIrqLock);

  return(MV_OK);
}

/* ========================================================================== */
/* ===========================  TIMER SECTION  ============================== */
/* ========================================================================== */

/*******************************************************************************
**
**  onuPonTimerCreate
**  ____________________________________________________________________________
** 
**  DESCRIPTION:   The function  
**                 
**  PARAMETERS:       
**
**  OUTPUTS:       None    
**
**  RETURNS:       
**
*******************************************************************************/
MV_STATUS onuPonTimerCreate(S_OnuPonTimer  *timerId, 
                            MV_U8          *timerDesc, 
                            PTIMER_FUNCPTR  timerFunc, 
                            MV_U32          timerParam, 
                            MV_U32          timerInterval, 
                            MV_U32          timerPeriodic)
{
  memset(&(timerId->onuPonTimerId), 0, sizeof(struct timer_list));
  timerId->onuPonTimerId.function = timerFunc;
  timerId->onuPonTimerId.data     = (-1);

  timerId->onuPonTimerInterval    = timerInterval; 
  timerId->onuPonTimerPeriodic    = timerPeriodic;
  init_timer(&(timerId->onuPonTimerId));

  return(MV_OK);
}

/*******************************************************************************
**
**  onuPonTimerUpdate
**  ____________________________________________________________________________
** 
**  DESCRIPTION:   The function  
**                 
**  PARAMETERS:       
**
**  OUTPUTS:       None    
**
**  RETURNS:       
**
*******************************************************************************/
int onuPonTimerUpdate(S_OnuPonTimer *timerId, MV_U32 data, MV_U32 interval, MV_U32 state)
{
  onuPonTimerDisable(timerId);
  timerId->onuPonTimerId.data  = data;
  timerId->onuPonTimerInterval = interval;
  if (state != 0)
	onuPonTimerEnable(timerId);

  return (0);
}

/*******************************************************************************
**
**  onuPonTimerEnable
**  ____________________________________________________________________________
** 
**  DESCRIPTION:   The function  
**                 
**  PARAMETERS:       
**
**  OUTPUTS:       None    
**
**  RETURNS:       
**
*******************************************************************************/
int onuPonTimerEnable(S_OnuPonTimer *timerId)
{
  if (timerId->onuPonTimerActive != ONU_PON_TIMER_ACTIVE) 
  {
    del_timer(&(timerId->onuPonTimerId));
    timerId->onuPonTimerId.expires = jiffies + (((timerId->onuPonTimerInterval) * HZ) / 1000); /* ms */
    add_timer(&(timerId->onuPonTimerId));
    timerId->onuPonTimerActive = ONU_PON_TIMER_ACTIVE;
  }

  return (0);
}

/*******************************************************************************
**
**  onuPonTimerDisable
**  ____________________________________________________________________________
** 
**  DESCRIPTION:   The function  
**                 
**  PARAMETERS:       
**
**  OUTPUTS:       None    
**
**  RETURNS:       
**
*******************************************************************************/
int onuPonTimerDisable(S_OnuPonTimer *timerId)
{
  if (timerId->onuPonTimerActive != ONU_PON_TIMER_NOT_ACTIVE) 
  {
    del_timer(&(timerId->onuPonTimerId));
    timerId->onuPonTimerActive = ONU_PON_TIMER_NOT_ACTIVE;
  }

  return (0);
}


/* ========================================================================== */
/* ===========================  ISR SECTION  ================================ */
/* ========================================================================== */

/*******************************************************************************
**
**  onuPonIrqNumGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt number get
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     GPON Interrupt number
**
*******************************************************************************/
MV_STATUS onuPonIrqNumInit(void)
{
#ifdef PON_FPGA
   onuPonResourceTbl_s.onuPonIrqId.onuPonIrqNum = 76/*GPON_MAC_IRQ_NUM*/; 
#else
   onuPonResourceTbl_s.onuPonIrqId.onuPonIrqNum = GPON_MAC_IRQ_NUM; 
#endif /* PON_FPGA */
#ifndef PON_FPGA
   onuPonResourceTbl_s.onuPonIrqId.onuDgIrqNum  = DYNGASP_IRQ_NUM; 
#endif /* PON_FPGA */

   return(MV_OK);
}

/*******************************************************************************
**  onuPonIrqEnable
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt enable
**               
**  PARAMETERS:  S_onuPonIrq irqId
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK
**
*******************************************************************************/
int onuPonIrqEnable(S_onuPonIrq *irqId)
{
  enable_irq(irqId->onuPonIrqNum);

  return(0);
}

/*******************************************************************************
**
**  onuPonIrqDisable
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON Interrupt disable
**               
**  PARAMETERS:  S_onuPonIrq irqId
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK
**
*******************************************************************************/
int onuPonIrqDisable(S_onuPonIrq *irqId)
{
  disable_irq(irqId->onuPonIrqNum);

  return(0);
}

/* ========================================================================== */
/* ===========================  MEMORY SECTION  ============================= */
/* ========================================================================== */


/*******************************************************************************
**
**  onuPonMemAlloc
**  ____________________________________________________________________________
**
**  DESCRIPTION: Allocate a memory space for GPON MAC
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IAMBA_OK
**
*******************************************************************************/
void* onuPonMemAlloc(unsigned int size) 
{
  void* memoryAlloc = kmalloc(size, GFP_KERNEL);

  onuPonMemoryAllocArray[onuPonMemoryAllocIndex] = memoryAlloc;
  onuPonMemoryAllocIndex++;

  return(memoryAlloc);
}

/*******************************************************************************
**
**  onuPonMemRelease
**  ____________________________________________________________________________
**
**  DESCRIPTION: release all allocated memory for GPON module
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     IAMBA_OK
**
*******************************************************************************/
void onuPonMemRelease(void) 
{
  MV_U32 index;

  for (index = 0; index < onuPonMemoryAllocIndex; index++) 
    kfree(onuPonMemoryAllocArray[index]);
}

/* ========================================================================== */
/* ===========================  PRINT SECTION  ============================== */
/* ========================================================================== */

/*******************************************************************************
**
**  mvPonPrint
**  ____________________________________________________________________________
**
**  DESCRIPTION: GPON print function
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK
**
*******************************************************************************/
MV_STATUS mvPonPrint(MV_U32 level, MV_U32 bitMask, const char *format, ...)
{
  MV_STATUS rcode;
  char      buf[256];
  va_list   argptr;

  /* check module printing status */
  rcode = ponOnuCheckPrintStatus(level, bitMask);
  if (rcode != MV_OK) 
    return(MV_OK); 

  /* build message */
  va_start(argptr, format);
  vsnprintf(buf, sizeof(buf), format, argptr);
  va_end(argptr);     

  /* print message */
  switch (level) 
  {
    case PON_PRINT_ERROR:
      printk(KERN_ERR "%s", buf);
      break;
    case PON_PRINT_INFO:
      printk(KERN_INFO "%s", buf);
      break;
    case PON_PRINT_DEBUG:
    default:
      printk(KERN_DEBUG "%s", buf);
      break;
  }

  return(MV_OK); 
}

/*******************************************************************************
**
**  ponOnuCheckPrintStatus
**  ____________________________________________________________________________
**
**  DESCRIPTION: Check module printing status
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK / MV_ERROR
**
*******************************************************************************/
MV_STATUS ponOnuCheckPrintStatus(MV_U32 printLevel, 
                                  MV_U32 moduleOptions)
{
  MV_U32  module;
  MV_U32  options;
  MV_U32  modulePrintLevel;
  MV_U32  modulePrintOptions;
  MV_BOOL printLevelInd;
  MV_BOOL printOptionsInd;

  /* input params */
  module             =  moduleOptions        & MODULE_MASK;
  options            = (moduleOptions >> 16) & OPTIONS_MASK;

  /* current module params */
  modulePrintLevel   = gponModulePrint_s[module].modulePrintLevel;
  modulePrintOptions = gponModulePrint_s[module].moduleOptions;

  /* check print level */
  if (printLevel <= modulePrintLevel) printLevelInd = MV_TRUE;
  else                                printLevelInd = MV_FALSE;

  /* check module options */
  if (options == 0) printOptionsInd = MV_TRUE; /* no options */
  else
  {
    if (options & modulePrintOptions) printOptionsInd = MV_TRUE;
    else                              printOptionsInd = MV_FALSE; 
  }

  if((printLevelInd == MV_TRUE) && (printOptionsInd == MV_TRUE)) return(MV_OK);
  else                                                             return(MV_ERROR);
}

/*******************************************************************************
**
**  ponOnuChangePrintStatus
**  ____________________________________________________________________________
**
**  DESCRIPTION: Change module printing status
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK / MV_ERROR
**
*******************************************************************************/
MV_STATUS ponOnuChangePrintStatus(MV_U32 module, 
                                   MV_U32 printLevel, 
                                   MV_U32 moduleOptions)
{
  gponModulePrint_s[module].modulePrintLevel = printLevel;
  gponModulePrint_s[module].moduleOptions    = moduleOptions;

  return(MV_OK);
}

/*******************************************************************************
**
**  ponOnuGetPrintStatus
**  ____________________________________________________________________________
**
**  DESCRIPTION: Change module printing status
**               
**  PARAMETERS:  none
** 
**  OUTPUTS:     none
**                               
**  RETURNS:     MV_OK / MV_ERROR
**
*******************************************************************************/
MV_STATUS ponOnuGetPrintStatus(MV_U32 module, 
                                MV_U32 *printLevel, 
                                MV_U32 *moduleOptions)
{
  *printLevel    = gponModulePrint_s[module].modulePrintLevel;
  *moduleOptions = gponModulePrint_s[module].moduleOptions;

  return(MV_OK);
}

/*******************************************************************************
**
**  ponOnuPrintStatus
**  ____________________________________________________________________________
**
**  DESCRIPTION: print modules printing status
**               
**  PARAMETERS:  char* buf
** 
**  OUTPUTS:     char* buf
**                               
**  RETURNS:     message length
**
*******************************************************************************/
int ponOnuPrintStatus(char* buf)
{
  MV_U32 index;
  MV_U32 printLevel;   
  MV_U32 moduleOptions;

  char* moduleDesc[PON_LAST_MODULE + 1] = 
  {
    "", 
    "MAC   -  1            ", 
    "INIT  -  2            ", 
    "ISR   -  3            ", 
    "CLI   -  4            ", 
    "MNG   -  5            ", 
    "SM    -  6 (GPON Only)", 
    "PM    -  7 (GPON Only)", 
    "ALARM -  8            ", 
    "BER   -  9 (GPON Only)", 
    "API   - 10            ", 
    "ALLOC - 11 (GPON Only)"
  };

  char* printLevelDesc[4] = 
  {
    "", 
    "ERROR", 
    "INFO", 
    "DEBUG" 
  };

  int off = 0;

  off += mvOsSPrintf(buf+off, "Module                  Print Level  Options\n");
  for (index = 0; index < PON_LAST_MODULE; index++) 
  {
    ponOnuGetPrintStatus(index, &printLevel, &moduleOptions);
    off += mvOsSPrintf(buf+off, "%22s  %11s   0x%08x\n", moduleDesc[index], printLevelDesc[printLevel], moduleOptions);
  }

  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "GPON              | EPON\n");
  off += mvOsSPrintf(buf+off, "==================+====================\n");
  off += mvOsSPrintf(buf+off, "SM Module Options | ISR Module Options\n"); 
  off += mvOsSPrintf(buf+off, "DEBUG    - 0x0001 | INT      - 0x0001\n");  
  off += mvOsSPrintf(buf+off, "STATE    - 0x0002 | MISS     - 0x0002\n");  
  off += mvOsSPrintf(buf+off, "AES      - 0x0004 | STATE    - 0x0004\n");  
  off += mvOsSPrintf(buf+off, "ALLOC    - 0x0008 | RAND     - 0x0008\n");  
  off += mvOsSPrintf(buf+off, "OMCC     - 0x0010 | \n");                   
  off += mvOsSPrintf(buf+off, "TIMER    - 0x0020 | MNG Module Options\n"); 
  off += mvOsSPrintf(buf+off, "ALARM    - 0x0040 | DBA      - 0x0001\n");  
  off += mvOsSPrintf(buf+off, "                  | SILENCE  - 0x0002\n");  
  off += mvOsSPrintf(buf+off, "                  | MAC ADDR - 0x0004\n");  
  off += mvOsSPrintf(buf+off, "                  | OAM TX   - 0x0008\n");  
  off += mvOsSPrintf(buf+off, "                  | DBA WA   - 0x0010\n");  
  off += mvOsSPrintf(buf+off, "                  | DBA FEC  - 0x0020\n");  
  off += mvOsSPrintf(buf+off, "                  | DBA TS   - 0x0040\n");  
  off += mvOsSPrintf(buf+off, "==================+====================\n");
  off += mvOsSPrintf(buf+off, "GPON Print Levels\n");
  off += mvOsSPrintf(buf+off, "ERROR    - 1\n");
  off += mvOsSPrintf(buf+off, "INFO     - 2\n");
  off += mvOsSPrintf(buf+off, "DEBUG    - 3\n");
  off += mvOsSPrintf(buf+off, "\n");
  off += mvOsSPrintf(buf+off, "Kernel Print Levels\n");
  off += mvOsSPrintf(buf+off, "KERN_ERR   - 4\n");
  off += mvOsSPrintf(buf+off, "KERN_INFO  - 7\n");
  off += mvOsSPrintf(buf+off, "KERN_DEBUG - 8\n");
  off += mvOsSPrintf(buf+off, "Change Kernel Print Level >> dmesg -n[level]\n");
  off += mvOsSPrintf(buf+off, "\n");

  return(off);
}

