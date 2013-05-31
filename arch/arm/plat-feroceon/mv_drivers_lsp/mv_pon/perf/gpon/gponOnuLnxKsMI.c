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
**  FILE        : gponOnuLnxKsMI.c                                           **
**                                                                           **
**  DESCRIPTION : This file implements ONU GPON Management Interface         **
*******************************************************************************
*                                                                             *                              
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *  
* =========================================================================== *      
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include <linux/version.h>
#include "gponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/                                               
#define __FILE_DESC__ "mv_pon/perf/gpon/gponOnuLnxKsMI.c"

/* Global Variables
------------------------------------------------------------------------------*/
extern spinlock_t onuPonIrqLock;

/* Local Variables
------------------------------------------------------------------------------*/

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/

/*******************************************************************************
**
**  onuGponMiInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function init onu 
**               
**  PARAMETERS:  MV_U32 alloc
**               MV_U32 tcont 
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**                   
*******************************************************************************/
MV_STATUS onuGponMiInit(S_GponIoctlInfo *ioctlInfo)
{
  return(onuGponStart(ioctlInfo));
}

/*******************************************************************************
**
**  onuGponMiInfoGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu info
**               
**  PARAMETERS:  S_IoctlInfo *onuInfo 
**                        
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR 
**                   
*******************************************************************************/
MV_STATUS onuGponMiInfoGet(S_GponIoctlInfo *onuInfo)
{
  MV_U32  uponDebugState;

  uponDebugState = onuGponApiUponDebugGet();

  if (uponDebugState) onuInfo->onuState = ONU_GPON_DEBUG_STATE;
  else                onuInfo->onuState = onuGponDbOnuStateGet();

  onuInfo->onuId           = onuGponDbOnuIdGet();
  onuInfo->omccPort        = onuGponDbOmccPortGet();
  onuInfo->omccValid       = onuGponDbOmccValidGet();
  onuInfo->onuSignalDetect = onuGponDbOnuSignalDetectGet();
  onuInfo->onuDsSyncOn     = onuGponDbOnuDsSyncOnGet();

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponMiAlarm
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu alarms
**               
**  PARAMETERS:  S_IoctlAlarm *ioctlAlarm
**                        
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR 
**                   
*******************************************************************************/
MV_STATUS onuGponMiAlarm(S_GponIoctlAlarm *ioctlAlarm)
{
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_LOS]  = onuGponAlarmGet(ONU_GPON_ALARM_LOS);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_LOF]  = onuGponAlarmGet(ONU_GPON_ALARM_LOF);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_LCDA] = onuGponAlarmGet(ONU_GPON_ALARM_LCDA);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_LCDG] = onuGponAlarmGet(ONU_GPON_ALARM_LCDG);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_SF]   = onuGponAlarmGet(ONU_GPON_ALARM_SF);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_SD]   = onuGponAlarmGet(ONU_GPON_ALARM_SD);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_TF]   = onuGponAlarmGet(ONU_GPON_ALARM_TF);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_SUF]  = onuGponAlarmGet(ONU_GPON_ALARM_SUF);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_MEM]  = onuGponAlarmGet(ONU_GPON_ALARM_MEM);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_DACT] = onuGponAlarmGet(ONU_GPON_ALARM_DACT);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_DIS]  = onuGponAlarmGet(ONU_GPON_ALARM_DIS);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_MIS]  = onuGponAlarmGet(ONU_GPON_ALARM_MIS);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_PEE]  = onuGponAlarmGet(ONU_GPON_ALARM_PEE);
  ioctlAlarm->alarmTbl[ONU_GPON_ALARM_RDI]  = onuGponAlarmGet(ONU_GPON_ALARM_RDI);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponMiPm
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu pm info
**               
**  PARAMETERS:  S_IoctlPm *ioctlPm
**                        
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR 
**                   
*******************************************************************************/
MV_STATUS onuGponMiPm(S_GponIoctlPm *ioctlPm)
{
  MV_STATUS            status;
  S_GponIoctlPloamTxPm ploamTxPm;
  S_GponIoctlPloamRxPm ploamRxPm;

  switch (ioctlPm->section) 
  {
    case E_GPON_IOCTL_PM_PLOAM_RX:
      status  = onuGponApiPmRxPloamPmGet(&(ioctlPm->ploamRx), MV_TRUE);
      status |= onuGponApiAdvancedPloamsCounterGet(&ploamTxPm, &(ioctlPm->ploamRx), MV_TRUE);
      return(status);
      break;
    case E_GPON_IOCTL_PM_PLOAM_TX:
      status = onuGponApiAdvancedPloamsCounterGet(&(ioctlPm->ploamTx), &ploamRxPm, MV_TRUE);
      break;
    case E_GPON_IOCTL_PM_BW_MAP:  
      return(onuGponApiPmRxBwMapPmGet(&(ioctlPm->bwMap), MV_TRUE));
      break;
    case E_GPON_IOCTL_PM_FEC:     
      return(onuGponApiPmFecPmGet(&(ioctlPm->fec), MV_TRUE));
      break;
    case E_GPON_IOCTL_PM_GEM_RX:
      return(onuGponApiGemRxCounterGet(&(ioctlPm->gemRx), MV_TRUE));
      break;
    case E_GPON_IOCTL_PM_GEM_TX:
      return(onuGponApiGemTxCounterGet(&(ioctlPm->gemTx), MV_TRUE));
      break;
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuGponMiTcontConfig
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function configure onu tcont <--> alloc
**               
**  PARAMETERS:  MV_U32 alloc
**               MV_U32 tcont 
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**                   
*******************************************************************************/
MV_STATUS onuGponMiTcontConfig(MV_U32 alloc, MV_U32 tcont)
{
  return(onuGponApiTcontConfig(alloc, tcont));
}

/*******************************************************************************
**
**  onuGponMiResetTcontsConfig
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function reset onu bw map table
**               
**  PARAMETERS:  None 
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**                   
*******************************************************************************/
MV_STATUS onuGponMiResetTcontsConfig(void)
{
  return(onuGponApiTcontsReset());
}

/*******************************************************************************
**
**  onuGponMiClearTcontConfig
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function clears the specifid tcont configuration
**               
**  PARAMETERS:  MV_U32 tcont 
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**                   
*******************************************************************************/
MV_STATUS onuGponMiClearTcontConfig(MV_U32 tcont)
{   
    return (onuGponApiTcontClear(tcont));
}

/*******************************************************************************
**
**  onuGponMiGem
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu pm info
**               
**  PARAMETERS:  S_GponIoctlGem *ioctlGem
**                        
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR 
**                   
*******************************************************************************/
MV_STATUS onuGponMiGem(S_GponIoctlGem *ioctlGem)
{
  MV_STATUS            status = MV_OK;
 

  switch (ioctlGem->action) 
  {
    case E_GEPON_IOCTL_GEM_ADD:
      status = onuGponApiGemConfig(ioctlGem->gemmap, ioctlGem->action);
      break;
    case E_GEPON_IOCTL_GEM_REMOVE:
      status = onuGponApiGemConfig(ioctlGem->gemmap, ioctlGem->action);
      break;
    case E_GEPON_IOCTL_GEM_CLEARALL:
        status = onuGponApiGemClearAll(MV_FALSE);
        break;
      break;
  }

  return(status);
}


/*******************************************************************************
**
**  mvPonCdevIoctl
**  ___________________________________________________________________________
** 
**  DESCRIPTION: The function execute IO commands
**               
**  PARAMETERS:  struct inode *inode
**               struct file *filp   
**               unsigned int cmd
**               unsigned long arg   
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**                   
*******************************************************************************/
int mvGponCdevIoctl(struct inode *inode, 
                    struct file  *filp, 
                    unsigned int  cmd,
                    unsigned long arg)
{
  MV_STATUS                     status;
  S_GponIoctlData               ioctlData;
  S_GponIoctlInfo               ioctlInfo;
  S_GponIoctlAlarm              ioctlAlarm;
  S_GponIoctlPm                 ioctlPm;
  S_GponIoctlXvr                ioctlXvr;
  S_GponIoctlGem                ioctlGem;  
  S_GponIoctlGemPortPmConfig    ioctlGemPortPmConfig;
  S_GponIoctlGemPortMibCounters ioctlGemPortMibCounters;
  unsigned long                 flags;
  int                           ret = -EINVAL;

  switch(cmd)
  {
    /* =========================== */
    /* ====== Init Section ======= */
    /* =========================== */

    /* ====== MVGPON_IOCTL_INIT ==================== */
    case MVGPON_IOCTL_INIT:
      if(copy_from_user(&ioctlInfo, (S_GponIoctlInfo*)arg, sizeof(S_GponIoctlInfo)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuGponMiInit(&ioctlInfo);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

    /* ====== MVGPON_IOCTL_BEN_INIT ================ */
    case MVGPON_IOCTL_BEN_INIT:
      if(copy_from_user(&ioctlXvr, (S_GponIoctlXvr*)arg, sizeof(S_GponIoctlXvr)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = mvOnuGponMacTxBurstEnParamSet(ioctlXvr.mask, 
                                             ioctlXvr.polarity, 
                                             ioctlXvr.delay, 
                                             ioctlXvr.enStop, 
                                             ioctlXvr.enStart);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status != MV_OK)
        goto ioctlErr;

      ret = 0;
      break;

    /* =========================== */
    /* ====== Data Section ======= */
    /* =========================== */

    /* ====== MVGPON_IOCTL_DATA_TCONT_CONFIG ======= */
    case MVGPON_IOCTL_DATA_TCONT_CONFIG:
      if(copy_from_user(&ioctlData, (S_GponIoctlData*)arg, sizeof(S_GponIoctlData)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuGponMiTcontConfig(ioctlData.alloc, 
                                    ioctlData.tcont);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

    /* ====== MVGPON_IOCTL_DATA_TCONT_RESET ======== */
    case MVGPON_IOCTL_DATA_TCONT_RESET:
	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuGponMiResetTcontsConfig();
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status == MV_OK)
        ret = 0;
      break;

    /* ====== MVGPON_IOCTL_DATA_TCONT_CLEAR ======== */
    case MVGPON_IOCTL_DATA_TCONT_CLEAR:
        if(copy_from_user(&ioctlData, (S_GponIoctlData*)arg, sizeof(S_GponIoctlData)))
        {
            mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
            "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
            goto ioctlErr;
        }
        
        spin_lock_irqsave(&onuPonIrqLock, flags);
        status = onuGponMiClearTcontConfig(ioctlData.tcont);
        spin_unlock_irqrestore(&onuPonIrqLock, flags);
        if(status != MV_OK)
        {
            goto ioctlErr;
        }
        
        ret = 0;

        break;

    /* ====== MVGPON_IOCTL_GEMPORT_STATE_SET ======= */
    case MVGPON_IOCTL_GEMPORT_STATE_SET:
      if(copy_from_user(&ioctlData, (S_GponIoctlData*)arg, sizeof(S_GponIoctlData)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);                   
        goto ioctlErr;
      }
      spin_lock_irqsave(&onuPonIrqLock, flags);
      if(ioctlData.gemState != 0)
         status = onuGponApiGemPortIdConfig(ioctlData.gemPort);
      else
         status = onuGponApiGemPortIdClear(ioctlData.gemPort);
      spin_unlock_irqrestore(&onuPonIrqLock, flags);

      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

    /* ====== MVGPON_IOCTL_GEMPORT_STATE_GET ======= */
    case MVGPON_IOCTL_GEMPORT_STATE_GET:
        
      if(copy_from_user(&ioctlData, (S_GponIoctlData*)arg, sizeof(S_GponIoctlData)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
                   
        goto ioctlErr;
      }
      spin_lock_irqsave(&onuPonIrqLock, flags);
      ioctlData.gemState = mvOnuGponMacPortIdValidGet(ioctlData.gemPort);
      spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(copy_to_user((S_GponIoctlData *)arg, &ioctlData, sizeof(S_GponIoctlData)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);                  
        goto ioctlErr;
      }        
      ret = 0;
      break;      
      
   
    /* =========================== */
    /* ====== Info Section ======= */
    /* =========================== */

    /* ====== MVGPON_IOCTL_INFO ==================== */
    case MVGPON_IOCTL_INFO:
      status = onuGponMiInfoGet(&ioctlInfo);
      if(status != MV_OK)
        goto ioctlErr;
      if(copy_to_user((S_GponIoctlInfo*)arg, &ioctlInfo, sizeof(S_GponIoctlInfo)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVGPON_IOCTL_ALARM =================== */
    case MVGPON_IOCTL_ALARM:
      status = onuGponMiAlarm(&ioctlAlarm);
      if(status != MV_OK)
        goto ioctlErr;
      if(copy_to_user((S_GponIoctlAlarm*)arg, &ioctlAlarm, sizeof(S_GponIoctlAlarm)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;
   
    /* =========================== */
    /* ====== PM Section ======= */
    /* =========================== */

    /* ====== MVGPON_IOCTL_PM ==================== */
    case MVGPON_IOCTL_PM:
		if(copy_from_user(&ioctlPm, (S_GponIoctlPm*)arg, sizeof(S_GponIoctlPm)))
		{
			mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
					   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
			goto ioctlErr;
		}
      status = onuGponMiPm(&ioctlPm);
      if(status != MV_OK)
        goto ioctlErr;
      if(copy_to_user((S_GponIoctlPm*)arg, &ioctlPm, sizeof(S_GponIoctlPm)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ================================== */
    /* ====== GEM Port PM Section ======= */
    /* ================================== */

    /* ====== MVGPON_IOCTL_GEMPORT_PM_CONFIG =========== */
    case MVGPON_IOCTL_GEMPORT_PM_CONFIG:
      if(copy_from_user(&ioctlGemPortPmConfig, (S_GponIoctlGemPortPmConfig *)arg, sizeof(S_GponIoctlGemPortPmConfig)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
    status = onuGponApiGemPortPmConfig(&ioctlGemPortPmConfig);
    if(status != MV_OK)
      goto ioctlErr;
    ret = 0;
    break;

    /* ====== MVGPON_IOCTL_GEMPORT_PM_GET =========== */
    case MVGPON_IOCTL_GEMPORT_PM_GET:
      if(copy_from_user(&ioctlGemPortMibCounters, (S_GponIoctlGemPortMibCounters *)arg, sizeof(S_GponIoctlGemPortMibCounters)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      status = onuGponApiGemPortPmGet(&ioctlGemPortMibCounters);
      if(status != MV_OK)
        goto ioctlErr;
      if(copy_to_user((S_GponIoctlGemPortMibCounters *)arg, &ioctlGemPortMibCounters, sizeof(S_GponIoctlGemPortMibCounters)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVGPON_IOCTL_GEMPORT_PM_RESET =========== */
    case MVGPON_IOCTL_GEMPORT_PM_RESET:
      status = onuGponPmGemPortPmReset();
      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

	/* ====== MVGPON_IOCTL_GEM ==================== */
    case MVGPON_IOCTL_GEM:
        if(copy_from_user(&ioctlGem, (S_GponIoctlGem*)arg, sizeof(S_GponIoctlGem)))
            {
              printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
            }
	spin_lock_irqsave(&onuPonIrqLock, flags);
        status =  onuGponMiGem(&ioctlGem);
	spin_unlock_irqrestore(&onuPonIrqLock, flags);
        ret = 0;
        break;

    default:
      ret = -EINVAL;
  }

ioctlErr:




  return(ret);
}
