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
**  FILE        : eponOnuLnxKsMI.c                                           **
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
#include "eponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/perf/epon/eponOnuLnxKsMI.c"

/* Global Variables
------------------------------------------------------------------------------*/
extern spinlock_t onuPonIrqLock;

/* Local Variables
------------------------------------------------------------------------------*/
static   S_EponIoctlTdmQueue ioctlTdmQueue;
static   S_EponIoctlDba      ioctlDba;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/

/*******************************************************************************
**
**  onuEponMiInfoGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function return onu info
**
**  PARAMETERS:  S_IoctlInfo *info
**               MV_U32      macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuEponMiInfoGet(S_EponIoctlInfo *info, MV_U32 macId)
{
  return(onuEponApiInformationGet(info, macId));
}

/*******************************************************************************
**
**  onuEponMiPm
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function return onu pm info
**
**  PARAMETERS:  S_IoctlPm *ioctlPm
**               MV_U32    macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuEponMiPm(S_EponIoctlPm *ioctlPm, MV_U32 macId)
{
  switch (ioctlPm->section)
  {
    case E_EPON_IOCTL_PM_RX:
      return(onuEponApiRxPmGet((&(ioctlPm->rxCnt)), MV_TRUE, 0));
      break;
    case E_EPON_IOCTL_PM_TX:
      return(onuEponApiTxPmGet((&(ioctlPm->txCnt)), MV_TRUE, macId));
      break;
    case E_EPON_IOCTL_PM_SW:
      return(onuEponApiSwPmGet((&(ioctlPm->swCnt)), MV_TRUE, macId));
      break;
    case E_EPON_IOCTL_PM_GPM:
      return(onuEponApiGpmPmGet((&(ioctlPm->gpmCnt)), MV_TRUE, macId));
      break;
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponMiFecConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu FEC
**
**  PARAMETERS:  MV_U32 rxGenFecEn
**               MV_U32 txGenFecEn
**               MV_U32 txMacFecEn[8]
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiFecConfig(MV_U32 rxGenFecEn, MV_U32 txGenFecEn, MV_U32 *txMacFecEn)
{
  return(onuEponApiFecConfig(rxGenFecEn, txGenFecEn, txMacFecEn, ONU_FEC_CFG_INIT));
}

/*******************************************************************************
**
**  onuEponMiEncConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu ENC
**
**  PARAMETERS:  MV_U32 onuEncryptCfg
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiEncConfig(MV_U32 onuEncryptCfg)
{
  MV_STATUS status;

  status  = onuEponApiEncryptionConfig(onuEncryptCfg);

  return(status);
}

/*******************************************************************************
**
**  onuEponMiEncConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu ENC
**
**  PARAMETERS:  MV_U32 encryptKey
**               MV_U32 encryptKeyIndex
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiEncKey(MV_U32 encryptKey, MV_U32 encryptKeyIndex, MV_U32 macId)
{
  MV_STATUS status;

  status = onuEponApiEncryptionKeyConfig(encryptKey, encryptKeyIndex, macId);

  return(status);
}

/*******************************************************************************
**
**  onuEponMiDbaConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu DBA report
**
**  PARAMETERS:  S_EponIoctlDba *ioctlDba
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiDbaConfig(S_EponIoctlDba *ioctlDba)
{
	MV_STATUS status = MV_OK;
	MV_U32    validReportQueueBitMap;
	MV_U32    highestReportQueue;
	MV_U32    llidIdx;
	MV_U32    queueSetIdx;
	MV_U32    queueIdx;
	MV_U32    numOfQueues;
	MV_U32    numOfQueueSets;

	MV_U32    reportLimit[4] = {/* Queueset, Queues */
					/* 1 */      8,
					/* 2 */      8,
					/* 3 */      5,
					/* 4 */      3};

	for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++) {
		validReportQueueBitMap = 0;
		highestReportQueue     = 0;

		/* LLID */
		/* ==== */
		if (!ioctlDba->validLlid[llidIdx]) {
			onuEponDbOnuDbaLlidActiveStatusSet(ONU_DBA_NON_ACTIVE_LLID, llidIdx);
			continue;
		}

		onuEponDbOnuDbaLlidActiveStatusSet(ONU_DBA_UPDATE_LLID, llidIdx);

		numOfQueues    = ioctlDba->dbaLlid[llidIdx].numOfQueues;
		numOfQueueSets = ioctlDba->dbaLlid[llidIdx].numOfQueueSets;

		/* Queue Set */
		/* ========= */
		if (ioctlDba->dbaLlid[llidIdx].numOfQueueSets == 0)
			continue;

		/* support up to 4 queuesets */
		if (numOfQueueSets > EPON_MAX_QUEUE_SET_NUMBER)
			numOfQueueSets = EPON_MAX_QUEUE_SET_NUMBER;

		for (queueSetIdx = 0; queueSetIdx < numOfQueueSets; queueSetIdx++) {
			/* Queue Id */
			/* ======== */
			for (queueIdx = 0; queueIdx < EPON_MAX_QUEUE; queueIdx++) {

				if (ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].state != 0) {

					validReportQueueBitMap |= 1 << queueIdx;
					highestReportQueue = queueIdx;

					onuEponDbOnuDbaQueueThresholdSet(
						ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].threshold,
						ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].state,
						 queueIdx,
						 queueSetIdx,
						 llidIdx);
					printk("onuEponMiDbaConfig Qset(%d) queue(%d) state(%d) thres(%d)\n",
					       queueSetIdx,
					       queueIdx,
					       ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].state,
					       ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].threshold);

				} else {

					onuEponDbOnuDbaQueueThresholdSet(0,	    /* threshold */
									 0,	    /* state     */
									 queueIdx,
									 queueSetIdx,
									 llidIdx);
					printk("onuEponMiDbaConfig Qset(%d) queue(%d) state(%d) thres(%d)\n",
					       queueSetIdx,
					       queueIdx,
					       ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].state,
					       ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].threshold);

				}
			} /* for each queueIdx */
		} /* for each queueIdx */

		/* Report Validation Check                                                  */
		/* =======================                                                  */
		/* Number of bytes for queue report = 39 bytes (64 - mac, type, fcs, etc')  */
		/* Formula - Number of Queueset X                                           */
		/*           Number of Queues   X                                           */
		/*           Queue bytes [2]    +                                           */
		/*           Number of reports ( == Number of Queueset)                     */
		/*                                                                          */
		/* SW REPORT via EPON REGS is limited to 34 bytes                           */
		/*                                       ========                           */
		/*                                                                          */
		/*  QueueSet    1         2        3        4                               */
		/*  Report      1b        1b       -        -                               */
		/*  Queue0      2b        2b                                                */
		/*  Queue1      2b        2b                                                */
		/*  Queue2      2b        2b                                                */
		/*  Queue3      2b        2b                                                */
		/*  Queue4      2b        2b                                                */
		/*  Queue5      2b        2b                                                */
		/*  Queue6      2b        2b                                                */
		/*  Queue7      2b        2b                      = 2 X 8 X 2 + 2 = 34bytes */
		/*                                                                          */
		/*  QueueSet    1         2        3        4                               */
		/*  Report      1b        1b       1b       -                               */
		/*  Queue0      2b        2b       2b                                       */
		/*  Queue1      2b        2b       2b                                       */
		/*  Queue2      2b        2b       2b                                       */
		/*  Queue3      2b        2b       2b                                       */
		/*  Queue4      2b        2b       2b                                       */
		/*  Queue5                                                                  */
		/*  Queue6                                                                  */
		/*  Queue7                                        = 3 X 5 X 2 + 3 = 33bytes */
		/*                                                                          */
		/*  QueueSet    1         2        3        4                               */
		/*  Report      1b        1b       1b       1b                              */
		/*  Queue0      2b        2b       2b       2b                              */
		/*  Queue1      2b        2b       2b       2b                              */
		/*  Queue2                                                                  */
		/*  Queue3                                                                  */
		/*  Queue4                                                                  */
		/*  Queue5                                                                  */
		/*  Queue6                                                                  */
		/*  Queue7                                        = 4 X 3 X 2 + 4 = 28bytes */
		/*                                                                          */
		/*  MV_U32 reportLimit[4] = {Queueset, Queues                               */
		/*                              1         8,                                */
		/*                              2         8,                                */
		/*                              3         5,                                */
		/*                              4         3};                               */

		printk("numOfQueues(%d), numOfQueueSets(%d), reportLimit[numOfQueueSets](%d)\n",
		       numOfQueues, numOfQueueSets, reportLimit[numOfQueueSets - 1]);

		while (numOfQueues > reportLimit[numOfQueueSets - 1])
			numOfQueueSets--;

		onuEponDbOnuDbaHighestReportQueueSet(highestReportQueue, llidIdx);
		onuEponDbOnuDbaNumOfQueuesSet(numOfQueues, llidIdx);
		onuEponDbOnuDbaNumOfQueuesetSet(numOfQueueSets, llidIdx);
		onuEponDbOnuDbaQueuesBitMapSet(validReportQueueBitMap, llidIdx);

		printk("highestReportQueue(%d), numOfQueues(%d, bit map 0x%08x), numOfQueueSets(%d)\n",
		       highestReportQueue, numOfQueues, validReportQueueBitMap, numOfQueueSets);

		onuEponDbaInfoUpdate(llidIdx);

		onuEponDbOnuDbaLlidActiveStatusSet(ONU_DBA_ACTIVE_LLID, llidIdx);

		/* ==================================== */
		/* ==         HW REPORT MODE         == */
		/* ==================================== */
		if (onuEponDbOnuDbaModeGet() == ONU_DBA_HW_RPRT_MODE) {
			status = mvOnuEponMacTxmCppReportUpdate(validReportQueueBitMap, highestReportQueue, llidIdx);
			if (status != MV_OK)
				return(status);
		}

	} /* for each llidIdx */

	return(status);
}

/*******************************************************************************
**
**  onuEponMiDbaRprt
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function return onu DBA report
**
**  PARAMETERS:  S_EponIoctlDba *ioctlDba
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiDbaRprt(S_EponIoctlDba *ioctlDba)
{
  MV_STATUS status = MV_OK;
  MV_U32    llidIdx;
  MV_U32    queueSetIdx;
  MV_U32    queueIdx;
  MV_U32    queueState;
  MV_U32    queueThreshold;

  for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++)
  {
	/* LLID */
	/* ==== */
	if (ioctlDba->validLlid[llidIdx])
	{
	  ioctlDba->dbaLlid[llidIdx].numOfQueues    = onuEponDbOnuDbaNumOfQueuesGet(llidIdx);
	  ioctlDba->dbaLlid[llidIdx].numOfQueueSets = onuEponDbOnuDbaNumOfQueuesetGet(llidIdx);

      /* Queue Set */
      /* ========= */
	  for (queueSetIdx = 0; queueSetIdx < ioctlDba->dbaLlid[llidIdx].numOfQueueSets; queueSetIdx++)
	  {
	    /* Queue Id */
	    /* ======== */
	    for (queueIdx = 0; queueIdx < ioctlDba->dbaLlid[llidIdx].numOfQueues; queueIdx++)
	    {
		  onuEponDbOnuDbaQueueThresholdGet(&(queueThreshold),
										   &(queueState),
										   queueIdx,
										   queueSetIdx,
										   llidIdx);

		  ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].threshold = (MV_U16)queueThreshold;
		  ioctlDba->dbaLlid[llidIdx].threshold[queueSetIdx][queueIdx].state     = (MV_U8)queueState;
	    }
	  }
	}
  }

  return(status);
}

/*******************************************************************************
**
**  onuEponMiHoldoverConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu Holdover time
**
**  PARAMETERS:  S_EponIoctlHoldOver *ioctlHoldover
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiHoldoverConfig(S_EponIoctlHoldOver *ioctlHoldover)
{
  MV_STATUS status = MV_OK;

  onuEponDbOnuHoldoverStateSet(ioctlHoldover->holdoverState);
  onuEponDbOnuHoldoverTimeSet(ioctlHoldover->holdoverTime);

  onuEponIsrTimerHoldoverIntervalSet(ioctlHoldover->holdoverTime, 0);

  mvPonPrint(PON_PRINT_INFO, PON_API_MODULE,
			 "DEBUG: Holdover config, state(%d), time(%d))\n",
			 ioctlHoldover->holdoverState, ioctlHoldover->holdoverTime);

  return(status);
}

/*******************************************************************************
**
**  onuEponMiHoldoverReport
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function return onu Holdover parameters
**
**  PARAMETERS:  S_EponIoctlHoldOver *ioctlHoldover
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiHoldoverReport(S_EponIoctlHoldOver *ioctlHoldover)
{
  MV_STATUS status = MV_OK;

  ioctlHoldover->holdoverState = onuEponDbOnuHoldoverStateGet();
  ioctlHoldover->holdoverTime  = onuEponDbOnuHoldoverTimeGet();

  return(status);
}

/*******************************************************************************
**
**  onuEponMiSilenceConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu silence state
**
**  PARAMETERS:  S_EponIoctlSilence *ioctlSilence
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiSilenceConfig(S_EponIoctlSilence *ioctlSilence)
{
  MV_STATUS status = MV_OK;

  onuEponDbOnuSilenceStateSet(ioctlSilence->silenceState);

  mvPonPrint(PON_PRINT_INFO, PON_API_MODULE,
			 "DEBUG: Silence config, state(%d)\n", ioctlSilence->silenceState);

  return(status);
}

/*******************************************************************************
**
**  onuEponMiAlarmGet
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function get EPON alarm(PON LOS)
**
**  PARAMETERS:  None
**
**  OUTPUTS:     alarm bit0:XVR LoS, alarm bit1:SERDES Los.
**               bit8: XVR ON, bit9: SERDES ON
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiAlarmGet(MV_U32 *alarm)
{
  MV_STATUS status = MV_OK;

  onuEponLosAlarmGet(alarm);

/*
  mvPonPrint(PON_PRINT_INFO, PON_API_MODULE,
             "DEBUG: EPON Alarm[%x], bit0: XVR LoS, bit1:SERDES LoS, bit8: XVR ON, bit9:SERDES ON\n", *alarm);
*/
  return(status);
}

/*******************************************************************************
**
**  onuEponMiTdmQueueConfig
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure onu tdm queue params
**
**  PARAMETERS:  S_EponIoctlTdmQueue *ioctlTdm
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
MV_STATUS onuEponMiTdmQueueConfig(S_EponIoctlTdmQueue *ioctlTdm)
{
  MV_STATUS status = MV_OK;
  MV_U32    llidIdx;
  MV_U32    queueIdx;
  MV_U32    numOfQueues;

  for (llidIdx = 0; llidIdx < EPON_MAX_MAC_NUM; llidIdx++)
  {
	if (onuEponDbOnuDbaLlidActiveStatusGet(llidIdx) == ONU_DBA_ACTIVE_LLID)
	{
	  numOfQueues = onuEponDbOnuDbaNumOfQueuesGet(llidIdx);

	  for (queueIdx = 0; queueIdx < numOfQueues; queueIdx++)
	  {
		onuEponPmDbaAddBytePerLlidAndQueue(llidIdx, queueIdx, ioctlTdm->tdmQueCnt[llidIdx][queueIdx]);
	  }
	}
  }

  return(status);
}

/*******************************************************************************
**
**  onuEponMiCfgMacTxEn
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function config onu mac TX enable
**
**  PARAMETERS:  enable - enable [1] or disable [0] TX
**
**  OUTPUTS:
**
**  RETURNS:     void
**
*******************************************************************************/
void onuEponMiCfgMacTxEn(MV_U32 enable, MV_U32 macId)
{
	onuPonTxPowerOn(enable == 1 ? MV_TRUE : MV_FALSE);
	onuEponForceTxDownStateSet(enable, macId);
	mvOnuEponMacOnuTxEnableSet(enable, macId);
}


/*******************************************************************************
**
**  mvEponCdevIoctl
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
int mvEponCdevIoctl(struct inode *inode, struct file *filp, unsigned int cmd,
                    unsigned long arg)
{
  MV_STATUS           status;
  S_EponIoctlInit     ioctlInit;
  S_EponIoctlInfo     ioctlInfo;
  S_EponIoctlPm       ioctlPm;
  S_EponIoctlFec      ioctlFec;
  S_EponIoctlEnc      ioctlEnc;
  S_EponIoctlOamTx    ioctlTxOam;
  S_EponIoctlHoldOver ioctlHoldover;
  S_EponIoctlSilence  ioctlSilence;
  MV_U32              ioctlState;
  MV_U32              ioctlAlarm;
  S_EponIoctlRogueOnu ioctlRogueOnu;
  unsigned long       flags;
  int                 ret = -EINVAL;



  switch(cmd)
  {
    /* ====== MVEPON_IOCTL_INIT ======= */
    case MVEPON_IOCTL_INIT:
      if(copy_from_user(&ioctlInit, (S_EponIoctlInit*)arg, sizeof(S_EponIoctlInit)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      onuEponInit(&ioctlInit);

      ret = 0;
      break;


    /* ====== MVEPON_IOCTL_FEC_CONFIG ======= */
    case MVEPON_IOCTL_FEC_CONFIG:
      if(copy_from_user(&ioctlFec, (S_EponIoctlFec*)arg, sizeof(S_EponIoctlFec)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiFecConfig(ioctlFec.rxGenFecEn,
                                  ioctlFec.txGenFecEn,
                                  ioctlFec.txMacFecEn);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_ENC_CONFIG ======== */
    case MVEPON_IOCTL_ENC_CONFIG:
      if(copy_from_user(&ioctlEnc, (S_EponIoctlEnc*)arg, sizeof(S_EponIoctlEnc)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiEncConfig(ioctlEnc.encEnable);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_ENC_KEY ======== */
    case MVEPON_IOCTL_ENC_KEY:
      if(copy_from_user(&ioctlEnc, (S_EponIoctlEnc*)arg, sizeof(S_EponIoctlEnc)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

	  spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiEncKey(ioctlEnc.encKey,
                               ioctlEnc.encKeyIndex & 0x1,
                               ioctlEnc.macId);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if(status != MV_OK)
        goto ioctlErr;
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_INFO ==================== */
    case MVEPON_IOCTL_INFO:
      if(copy_from_user(&ioctlInfo, (S_EponIoctlInfo*)arg, sizeof(S_EponIoctlInfo)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      status = onuEponMiInfoGet(&ioctlInfo, ioctlInfo.macId);
      if(status != MV_OK)
        goto ioctlErr;

      if(copy_to_user((S_EponIoctlInfo*)arg, &ioctlInfo, sizeof(S_EponIoctlInfo)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_PM ==================== */
    case MVEPON_IOCTL_PM:
      if(copy_from_user(&ioctlPm, (S_EponIoctlPm*)arg, sizeof(S_EponIoctlPm)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      status = onuEponMiPm(&ioctlPm, ioctlPm.macId);
      if(status != MV_OK)
        goto ioctlErr;

      if(copy_to_user((S_EponIoctlPm*)arg, &ioctlPm, sizeof(S_EponIoctlPm)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_OAM_TX ==================== */
    case MVEPON_IOCTL_OAM_TX:
      if(copy_from_user(&ioctlTxOam, (S_EponIoctlOamTx*)arg, sizeof(S_EponIoctlOamTx)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
      			 "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

	  spin_lock_irqsave(&onuPonIrqLock, flags);
	  status = onuEponApiTransmitOamFrame((S_OnuEponCtrlBuffer*)&(ioctlTxOam.oamFrame), ioctlTxOam.macId);
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);
	  if (status != MV_OK)
	  {
		mvPonPrint(PON_PRINT_DEBUG, PON_MNG_OAM_TX_MODULE,
				   "ERROR: (%s:%d) onuEponSendCtrlFrameData\n\r", __FILE_DESC__, __LINE__);
		goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_DBA_CFG ==================== */
    case MVEPON_IOCTL_DBA_CFG:
      if(copy_from_user(&ioctlDba, (S_EponIoctlDba*)arg, sizeof(S_EponIoctlDba)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
      		   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiDbaConfig((S_EponIoctlDba*)&(ioctlDba));
      spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if (status != MV_OK)
        goto ioctlErr;

      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_DBA_RPRT =================== */
    case MVEPON_IOCTL_DBA_RPRT:
      if(copy_from_user(&ioctlDba, (S_EponIoctlDba*)arg, sizeof(S_EponIoctlDba)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
      		 "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      status = onuEponMiDbaRprt((S_EponIoctlDba*)&(ioctlDba));
      if (status != MV_OK)
        goto ioctlErr;

      if(copy_to_user((S_EponIoctlDba*)arg, &ioctlDba, sizeof(S_EponIoctlDba)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_HOLDOVER_CFG ==================== */
    case MVEPON_IOCTL_HOLDOVER_CFG:
      if(copy_from_user(&ioctlHoldover, (S_EponIoctlHoldOver*)arg, sizeof(S_EponIoctlHoldOver)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
      		 "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiHoldoverConfig((S_EponIoctlHoldOver*)&(ioctlHoldover));
      spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if (status != MV_OK)
        goto ioctlErr;

      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_HOLDOVER_RPRT ==================== */
    case MVEPON_IOCTL_HOLDOVER_RPRT:
      status = onuEponMiHoldoverReport((S_EponIoctlHoldOver*)&(ioctlHoldover));
      if (status != MV_OK)
        goto ioctlErr;

      if(copy_to_user((S_EponIoctlHoldOver*)arg, &ioctlHoldover, sizeof(S_EponIoctlHoldOver)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_SILENCE ==================== */
    case MVEPON_IOCTL_SILENCE:
      if(copy_from_user(&ioctlSilence, (S_EponIoctlSilence*)arg, sizeof(S_EponIoctlSilence)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
      	   "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiSilenceConfig((S_EponIoctlSilence*)&(ioctlSilence));
      spin_unlock_irqrestore(&onuPonIrqLock, flags);
      if (status != MV_OK)
        goto ioctlErr;

      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_P2P_SET ==================== */
    case MVEPON_IOCTL_P2P_SET:
      if(copy_from_user(&ioctlState, (MV_U32*)arg, sizeof(MV_U32)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
           "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

	  spin_lock_irqsave(&onuPonIrqLock, flags);
      if (ioctlState)
      {
        status = mvP2PStart();
      }
      else
      {
        status = mvP2PStop();
      }
	  spin_unlock_irqrestore(&onuPonIrqLock, flags);

      if (status != MV_OK)
        goto ioctlErr;

      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_P2P_ FORCE_MODE_SET ==================== */
    case MVEPON_IOCTL_P2P_FORCE_MODE_SET:
        if (copy_from_user(&ioctlState, (MV_U32*)arg, sizeof(MV_U32)))
        {
            mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                       "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
            goto ioctlErr;
        }

        spin_lock_irqsave(&onuPonIrqLock, flags);
        
        status = onuEponDbP2PForceModeSet(ioctlState);
        
        spin_unlock_irqrestore(&onuPonIrqLock, flags);

        if (status != MV_OK)
        {
            goto ioctlErr;
        }

        ret = 0;
        break;
        
    /* ====== MVEPON_IOCTL_TDM_QUE_CFG ==================== */
    case MVEPON_IOCTL_TDM_QUE_CFG:
      if(copy_from_user(&ioctlTdmQueue,  (S_EponIoctlTdmQueue*)arg, sizeof(S_EponIoctlTdmQueue)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
      	 "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      spin_lock_irqsave(&onuPonIrqLock, flags);
      status = onuEponMiTdmQueueConfig((S_EponIoctlTdmQueue*)&(ioctlTdmQueue));
      spin_unlock_irqrestore(&onuPonIrqLock, flags);

      if (status != MV_OK)
        goto ioctlErr;

      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_ALARM_GET ==================== */
    case MVEPON_IOCTL_ALARM_GET:
      status = onuEponMiAlarmGet(&ioctlAlarm);
      if (status != MV_OK)
        goto ioctlErr;

      if(copy_to_user((MV_U32 *)arg, &ioctlAlarm, sizeof(MV_U32)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
                   "ERROR: (%s:%d) copy_to_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }
      ret = 0;
      break;

    /* ====== MVEPON_IOCTL_ROGUE_ONU_SET ==================== */
    case MVEPON_IOCTL_ROGUE_ONU_SET:
      if(copy_from_user(&ioctlRogueOnu, (S_EponIoctlRogueOnu*)arg, sizeof(S_EponIoctlRogueOnu)))
      {
        mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
           "ERROR: (%s:%d) copy_from_user failed\n", __FILE_DESC__, __LINE__);
        goto ioctlErr;
      }

      onuEponMiCfgMacTxEn(ioctlRogueOnu.enable, ioctlRogueOnu.macId);

      ret = 0;
      break;

     default:
      ret = -EINVAL;
  }

ioctlErr:


  return(ret);
}
