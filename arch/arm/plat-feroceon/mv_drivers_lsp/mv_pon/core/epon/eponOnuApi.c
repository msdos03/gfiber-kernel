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
/**  FILE        : eponOnuApi.c                                              **/
/**                                                                          **/
/**  DESCRIPTION : This file implements ONU EPON API functionality           **/
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
#include "mv_netdev.h"

/* Local Constant
------------------------------------------------------------------------------*/                                               
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuApi.c"

/* Global Variables
------------------------------------------------------------------------------*/
S_EponPm g_OnuEponOutPm[EPON_MAX_MAC_NUM];

/* Local Variables
------------------------------------------------------------------------------*/

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions 
------------------------------------------------------------------------------*/

/* Typedefs
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/*******************************************************************************
**
**  onuEponApiStatusNotifyRegister
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function register status callback function
**               
**  PARAMETERS:  STATUSNOTIFYFUNC notifyCallBack 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponApiStatusNotifyRegister(STATUSNOTIFYFUNC notifyCallBack)
{
  MV_STATUS status;

  status = onuEponDbStatusNotifySet(notifyCallBack);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) ponOnuApiStatusNotifyRegister", __FILE_DESC__, __LINE__); 
    return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponStatusNotify
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function notify EPON status to user space
**               
**  PARAMETERS:  MV_U32 status 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponStatusNotify(MV_U32 status)
{
  STATUSNOTIFYFUNC statusFunc = NULL; 

  /* Send signal to management layer */
  statusFunc = onuEponDbStatusNotifyGet();
  if (statusFunc != NULL)
  {
    statusFunc(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiInformationGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu info
**               
**  PARAMETERS:  S_IoctlInfo *onuInfo 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiInformationGet(S_EponIoctlInfo *onuInfo, MV_U32 macId)
{
  MV_U32 addressLow;
  MV_U32 addressHigh;

  onuInfo->onuEponState        = onuEponDbOnuStateGet(macId);      
  onuInfo->onuEponCtrlType     = onuEponDbOnuCtrlTypeGet(); 
  onuInfo->onuEponSignalDetect = onuEponDbOnuSignalDetectGet();
  onuInfo->onuEponDsSyncOkPcs  = onuEponDbOnuDsSyncOkPcsGet();
  onuInfo->onuEponMode         = onuEponDbModeGet();

  onuEponDbOnuMacAddrGet(&addressLow, &addressHigh, macId);
  onuInfo->onuEponMacAddr[5] = ((addressLow  >>  0) & 0xFF);
  onuInfo->onuEponMacAddr[4] = ((addressLow  >>  8) & 0xFF); 
  onuInfo->onuEponMacAddr[3] = ((addressLow  >> 16) & 0xFF);
  onuInfo->onuEponMacAddr[2] = ((addressLow  >> 24) & 0xFF);
  onuInfo->onuEponMacAddr[1] = ((addressHigh >>  0) & 0xFF);
  onuInfo->onuEponMacAddr[0] = ((addressHigh >>  8) & 0xFF);

  onuEponDbOnuBcastAddrGet(&addressLow, &addressHigh);
  onuInfo->onuEponBcastAddr[5] = ((addressLow  >>  0) & 0xFF);
  onuInfo->onuEponBcastAddr[4] = ((addressLow  >>  8) & 0xFF); 
  onuInfo->onuEponBcastAddr[3] = ((addressLow  >> 16) & 0xFF);
  onuInfo->onuEponBcastAddr[2] = ((addressLow  >> 24) & 0xFF);
  onuInfo->onuEponBcastAddr[1] = ((addressHigh >>  0) & 0xFF);
  onuInfo->onuEponBcastAddr[0] = ((addressHigh >>  8) & 0xFF);

  onuEponDbPktRxLlidGet(&(onuInfo->onuEponRxLLID), macId);
  onuEponDbPktTxLlidGet(&(onuInfo->onuEponTxLLID), macId);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiLinkStateGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu link state
**               
**  PARAMETERS:  MV_U32 *linkState 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiLinkStateGet(MV_U32 *linkState, MV_U32 macId)
{
  if (onuEponDbOnuStateGet(macId) == ONU_EPON_03_OPERATION) *linkState = 1;
  else                                                      *linkState = 0;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiFecConfig
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
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiFecConfig(MV_U32 rxGenFecEn, MV_U32 txGenFecEn, MV_U32 *txMacFecEn, MV_U32 source)
{
  MV_STATUS status;
  MV_U32    regReqAutoRes;
  MV_U32    regAckAutoRes;
  MV_U32    reportAutoRes;
  MV_U32    rxGenFecEnTemp;
  MV_U32    txGenFecEnTemp;   
  MV_U32    txMacFecEnTemp;   

  mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE, 
			 "DEBUG: (%s:%d) onuEponApiFecConfig, input rxGenFecEn(%d), txGenFecEn(%d), txMacFecEn(%d,%d,%d,%d,%d,%d,%d,%d)", 
			 __FILE_DESC__, __LINE__, rxGenFecEn, txGenFecEn, txMacFecEn[0], txMacFecEn[1], txMacFecEn[2], txMacFecEn[3],
             txMacFecEn[4], txMacFecEn[5], txMacFecEn[6], txMacFecEn[7]);

  onuEponDbOnuCfgGet(&regReqAutoRes, &regAckAutoRes, &reportAutoRes, &rxGenFecEnTemp, &txGenFecEnTemp, &txMacFecEnTemp);
  rxGenFecEnTemp = rxGenFecEn;
  txGenFecEnTemp = txGenFecEn;
  txMacFecEnTemp = ((txMacFecEn[0] & 0x1) << 0) |
                   ((txMacFecEn[1] & 0x1) << 1) |
                   ((txMacFecEn[2] & 0x1) << 2) |
                   ((txMacFecEn[3] & 0x1) << 3) |
                   ((txMacFecEn[4] & 0x1) << 4) |
                   ((txMacFecEn[5] & 0x1) << 5) |
                   ((txMacFecEn[6] & 0x1) << 6) |
                   ((txMacFecEn[7] & 0x1) << 7);

  if (source == ONU_FEC_CFG_INIT) 
  {
	onuEponDbOnuCfgSet(regReqAutoRes, regAckAutoRes, reportAutoRes, rxGenFecEnTemp, txGenFecEnTemp, txMacFecEnTemp);
	onuEponDbOnuRxFecCfgSet(rxGenFecEn);
  }

  mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE, 
			 "DEBUG: (%s:%d) onuEponApiFecConfig, config regReqAutoRes(%d), regAckAutoRes(%d), reportAutoRes(%d), rxGenFecEnTemp(%d), txGenFecEnTemp(%d), txMacFecEnTemp(%d)\n", 
			 __FILE_DESC__, __LINE__, regReqAutoRes, regAckAutoRes, reportAutoRes, rxGenFecEnTemp, txGenFecEnTemp,txMacFecEnTemp);

  status = mvOnuEponMacGenOnuConfigSet(rxGenFecEnTemp, txGenFecEnTemp, reportAutoRes, regAckAutoRes, regReqAutoRes, txMacFecEnTemp);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiFecConfig", __FILE_DESC__, __LINE__); 
    return(status);
  }

  if (rxGenFecEn)
  {
	status = mvOnuEponMacPcsDelaySet(0x1058);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
	  		   "ERROR: (%s:%d) onuEponApiFecConfig", __FILE_DESC__, __LINE__); 
	  return(status);
	}
  }
  else
  {
	status = mvOnuEponMacPcsDelaySet(0x1C58);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
	  		 "ERROR: (%s:%d) onuEponApiFecConfig", __FILE_DESC__, __LINE__); 
	  return(status);
	}
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiFecReConfig
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function re-configure onu FEC
**               
**  PARAMETERS:  MV_U32 rxGenFecEn
**               MV_U32 txGenFecEn
**               MV_U32 txMacFecEn[8]
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiFecReConfig(void)
{
  MV_STATUS status;
  MV_U32    regReqAutoRes;
  MV_U32    regAckAutoRes;
  MV_U32    reportAutoRes;
  MV_U32    rxGenFecEn;
  MV_U32    txGenFecEn;   
  MV_U32    txMacFecEn;   

  onuEponDbOnuCfgGet(&regReqAutoRes, &regAckAutoRes, &reportAutoRes, &rxGenFecEn, &txGenFecEn, &txMacFecEn);

  mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE, 
			 "DEBUG: (%s:%d) onuEponApiFecReConfig, regReqAutoRes(%d), regAckAutoRes(%d), reportAutoRes(%d), rxGenFecEn(%d), txGenFecEn(%d), txMacFecEn(0x%x)\n", 
			 __FILE_DESC__, __LINE__, regReqAutoRes, regAckAutoRes, reportAutoRes, rxGenFecEn, txGenFecEn, txMacFecEn);

  status = mvOnuEponMacGenOnuConfigSet(rxGenFecEn, txGenFecEn, reportAutoRes, regAckAutoRes, regReqAutoRes, txMacFecEn);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiFecReConfig", __FILE_DESC__, __LINE__); 
    return(status);
  }

  if (rxGenFecEn)
  {
	status = mvOnuEponMacPcsDelaySet(0x1058);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
	  		   "ERROR: (%s:%d) onuEponApiFecReConfig", __FILE_DESC__, __LINE__); 
	  return(status);
	}
  }
  else
  {
	status = mvOnuEponMacPcsDelaySet(0x1C58);
	if (status != MV_OK)
	{
	  mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
	  		 "ERROR: (%s:%d) onuEponApiFecReConfig", __FILE_DESC__, __LINE__); 
	  return(status);
	}
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiEncryptionConfig
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function configure onu Encryption
**               
**  PARAMETERS:  MV_U32 onuEncryptCfg
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiEncryptionConfig(MV_U32 onuEncryptCfg)
{
  MV_STATUS status;

  status = mvOnuEponMacRxpEncConfigSet(onuEncryptCfg);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiEncryptionConfig", __FILE_DESC__, __LINE__); 
    return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiEncryptionKeyConfig
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function configure onu Encryption Key
**               
**  PARAMETERS:  MV_U32 encryptKey
**               MV_U32 encryptKeyIndex
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiEncryptionKeyConfig(MV_U32 encryptKey, MV_U32 encryptKeyIndex, MV_U32 macId)
{
  MV_STATUS status;

  status = mvOnuEponMacRxpEncKeySet(encryptKey, encryptKeyIndex, macId); 
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiEncryptionKeyConfig", __FILE_DESC__, __LINE__); 
    return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiRxPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function is used to retrieve and clear (if requested) ONU
**               Rx counters                           
**                
**  PARAMETERS:  S_EponIoctlRxPm *rxPm
**               MV_BOOL         clear
**               MV_U32          macId
**
**  OUTPUTS:     None 
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponApiRxPmGet(S_EponIoctlRxPm *rxPm,  MV_BOOL clear, MV_U32 macId)
{
  MV_STATUS status;
  S_RxPm    inCounters;

  status = onuEponPmRxPmGet(&inCounters, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiRxPmGet", __FILE_DESC__, __LINE__); 
    return(status);
  }

  rxPm->fcsErrorFramesCnt = inCounters.fcsErrorFramesCnt - g_OnuEponOutPm[macId].rx.fcsErrorFramesCnt;
  rxPm->shortFramesCnt    = inCounters.shortFramesCnt    - g_OnuEponOutPm[macId].rx.shortFramesCnt;
  rxPm->longFramesCnt     = inCounters.longFramesCnt     - g_OnuEponOutPm[macId].rx.longFramesCnt; 
  rxPm->dataFramesCnt     = inCounters.dataFramesCnt     - g_OnuEponOutPm[macId].rx.dataFramesCnt; 
  rxPm->ctrlFramesCnt     = inCounters.ctrlFramesCnt     - g_OnuEponOutPm[macId].rx.ctrlFramesCnt; 
  rxPm->reportFramesCnt   = inCounters.reportFramesCnt   - g_OnuEponOutPm[macId].rx.reportFramesCnt;  
  rxPm->gateFramesCnt     = inCounters.gateFramesCnt     - g_OnuEponOutPm[macId].rx.gateFramesCnt;    

  if (clear == MV_TRUE)
  {
    g_OnuEponOutPm[macId].rx.fcsErrorFramesCnt = inCounters.fcsErrorFramesCnt;
    g_OnuEponOutPm[macId].rx.shortFramesCnt    = inCounters.shortFramesCnt;   
    g_OnuEponOutPm[macId].rx.longFramesCnt     = inCounters.longFramesCnt;    
    g_OnuEponOutPm[macId].rx.dataFramesCnt     = inCounters.dataFramesCnt;    
    g_OnuEponOutPm[macId].rx.ctrlFramesCnt     = inCounters.ctrlFramesCnt;    
    g_OnuEponOutPm[macId].rx.reportFramesCnt   = inCounters.reportFramesCnt;   
    g_OnuEponOutPm[macId].rx.gateFramesCnt     = inCounters.gateFramesCnt;     
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiTxPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function is used to retrieve and clear (if requested) ONU
**               Tx counters                           
**                
**  PARAMETERS:  S_EponIoctlTxPm *rxPm
**               MV_BOOL         clear
**               MV_U32          macId
**
**  OUTPUTS:     None 
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponApiTxPmGet(S_EponIoctlTxPm *txPm, MV_BOOL clear, MV_U32 macId)
{
  MV_STATUS  status;
  S_EponTxPm inCounters;

  status = onuEponPmTxPmGet(&inCounters, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiTxPmGet", __FILE_DESC__, __LINE__); 
    return(status);
  }

  txPm->ctrlRegReqFramesCnt = inCounters.ctrlRegReqFramesCnt - g_OnuEponOutPm[macId].tx.ctrlRegReqFramesCnt;
  txPm->ctrlRegAckFramesCnt = inCounters.ctrlRegAckFramesCnt - g_OnuEponOutPm[macId].tx.ctrlRegAckFramesCnt;
  txPm->reportFramesCnt     = inCounters.reportFramesCnt     - g_OnuEponOutPm[macId].tx.reportFramesCnt;    
  txPm->dataFramesCnt       = inCounters.dataFramesCnt       - g_OnuEponOutPm[macId].tx.dataFramesCnt;      
  txPm->txAllowedBytesCnt   = inCounters.txAllowedBytesCnt   - g_OnuEponOutPm[macId].tx.txAllowedBytesCnt;  

  if (clear == MV_TRUE)
  {
    g_OnuEponOutPm[macId].tx.ctrlRegReqFramesCnt = inCounters.ctrlRegReqFramesCnt;
    g_OnuEponOutPm[macId].tx.ctrlRegAckFramesCnt = inCounters.ctrlRegAckFramesCnt;
    g_OnuEponOutPm[macId].tx.reportFramesCnt     = inCounters.reportFramesCnt;   
    g_OnuEponOutPm[macId].tx.dataFramesCnt       = inCounters.dataFramesCnt;      
    g_OnuEponOutPm[macId].tx.txAllowedBytesCnt   = inCounters.txAllowedBytesCnt;
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiSwPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function is used to retrieve and clear (if requested) ONU
**               SW counters                           
**                
**  PARAMETERS:  S_EponIoctlSwPm *swPm
**               MV_BOOL         clear
**               MV_U32          macId
**
**  OUTPUTS:     None 
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponApiSwPmGet(S_EponIoctlSwPm *swPm, MV_BOOL clear, MV_U32 macId)
{
  MV_STATUS status;
  S_SwPm    inCounters;
  MV_U32    index;

  status = onuEponPmSwPmGet(&inCounters, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiSwPmGet", __FILE_DESC__, __LINE__); 
    return(status);
  }

  for (index = 0; index < MAX_EPON_RX_SW_CNT; index++)
  {
    swPm->swRxCnt[index] = inCounters.swRxCnt[index] - g_OnuEponOutPm[macId].sw.swRxCnt[index];
  }

  for (index = 0; index < MAX_EPON_TX_SW_CNT; index++)
  {
    swPm->swTxCnt[index] = inCounters.swTxCnt[index] - g_OnuEponOutPm[macId].sw.swTxCnt[index];
  }

  if (clear == MV_TRUE)
  {
    for (index = 0; index < MAX_EPON_RX_SW_CNT; index++)
    {
      g_OnuEponOutPm[macId].sw.swRxCnt[index] = inCounters.swRxCnt[index];
    }

    for (index = 0; index < MAX_EPON_TX_SW_CNT; index++)
    {
      g_OnuEponOutPm[macId].sw.swTxCnt[index] = inCounters.swTxCnt[index];
    }
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiGpmPmGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function is used to retrieve and clear (if requested) ONU
**               Gpm counters                           
**                
**  PARAMETERS:  S_EponIoctlGpmPm *rxPm
**               MV_BOOL          clear
**               MV_U32           macId
**
**  OUTPUTS:     None 
**
**  RETURNS:     MV_OK or error
**
*******************************************************************************/
MV_STATUS onuEponApiGpmPmGet(S_EponIoctlGpmPm *gpmPm, MV_BOOL clear, MV_U32 macId)
{
  MV_STATUS status;
  S_GpmPm   inCounters;

  status = onuEponPmGpmPmGet(&inCounters, macId);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponApiTxPmGet", __FILE_DESC__, __LINE__); 
    return(status);
  }


  gpmPm->grantValidCnt                 = inCounters.grantValidCnt                 - g_OnuEponOutPm[macId].gpm.grantValidCnt;                
  gpmPm->grantMaxFutureTimeErrorCnt    = inCounters.grantMaxFutureTimeErrorCnt    - g_OnuEponOutPm[macId].gpm.grantMaxFutureTimeErrorCnt;   
  gpmPm->minProcTimeErrorCnt           = inCounters.minProcTimeErrorCnt           - g_OnuEponOutPm[macId].gpm.minProcTimeErrorCnt;          
  gpmPm->lengthErrorCnt                = inCounters.lengthErrorCnt                - g_OnuEponOutPm[macId].gpm.lengthErrorCnt;               
  gpmPm->discoveryAndRegCnt            = inCounters.discoveryAndRegCnt            - g_OnuEponOutPm[macId].gpm.discoveryAndRegCnt;           
  gpmPm->fifoFullErrorCnt              = inCounters.fifoFullErrorCnt              - g_OnuEponOutPm[macId].gpm.fifoFullErrorCnt;             
  gpmPm->opcDiscoveryNotRegBcastCnt    = inCounters.opcDiscoveryNotRegBcastCnt    - g_OnuEponOutPm[macId].gpm.opcDiscoveryNotRegBcastCnt;   
  gpmPm->opcRegisterNotDiscoveryCnt    = inCounters.opcRegisterNotDiscoveryCnt    - g_OnuEponOutPm[macId].gpm.opcRegisterNotDiscoveryCnt;   
  gpmPm->opcDiscoveryNotRegNotBcastCnt = inCounters.opcDiscoveryNotRegNotBcastCnt - g_OnuEponOutPm[macId].gpm.opcDiscoveryNotRegNotBcastCnt;
  gpmPm->opcDropGrantCnt               = inCounters.opcDropGrantCnt               - g_OnuEponOutPm[macId].gpm.opcDropGrantCnt;              
  gpmPm->opcHiddenGrantCnt             = inCounters.opcHiddenGrantCnt             - g_OnuEponOutPm[macId].gpm.opcHiddenGrantCnt;            
  gpmPm->opcBackToBackCnt              = inCounters.opcBackToBackCnt              - g_OnuEponOutPm[macId].gpm.opcBackToBackCnt;             

  if (clear == MV_TRUE)
  {
    g_OnuEponOutPm[macId].gpm.grantValidCnt                 = inCounters.grantValidCnt;                
    g_OnuEponOutPm[macId].gpm.grantMaxFutureTimeErrorCnt    = inCounters.grantMaxFutureTimeErrorCnt;   
    g_OnuEponOutPm[macId].gpm.minProcTimeErrorCnt           = inCounters.minProcTimeErrorCnt;          
    g_OnuEponOutPm[macId].gpm.lengthErrorCnt                = inCounters.lengthErrorCnt;               
    g_OnuEponOutPm[macId].gpm.discoveryAndRegCnt            = inCounters.discoveryAndRegCnt;           
    g_OnuEponOutPm[macId].gpm.fifoFullErrorCnt              = inCounters.fifoFullErrorCnt;             
    g_OnuEponOutPm[macId].gpm.opcDiscoveryNotRegBcastCnt    = inCounters.opcDiscoveryNotRegBcastCnt;   
    g_OnuEponOutPm[macId].gpm.opcRegisterNotDiscoveryCnt    = inCounters.opcRegisterNotDiscoveryCnt;   
    g_OnuEponOutPm[macId].gpm.opcDiscoveryNotRegNotBcastCnt = inCounters.opcDiscoveryNotRegNotBcastCnt;
    g_OnuEponOutPm[macId].gpm.opcDropGrantCnt               = inCounters.opcDropGrantCnt;              
    g_OnuEponOutPm[macId].gpm.opcHiddenGrantCnt             = inCounters.opcHiddenGrantCnt;            
    g_OnuEponOutPm[macId].gpm.opcBackToBackCnt              = inCounters.opcBackToBackCnt;             
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiTransmitOamFrame
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function transmit OAM frame via FIFO
**               
**  PARAMETERS:  S_OnuEponCtrlBuffer *frame
**               MV_U32              macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponApiTransmitOamFrame(S_OnuEponCtrlBuffer *frame, MV_U32 macId)
{
  MV_STATUS status;

  if (onuEponDbOnuStateGet(0) != ONU_EPON_03_OPERATION) 
  {
	mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE, 
	  		 "ERROR: (%s:%d) onuEponApiTransmitOamFrame, ONU not registered", __FILE_DESC__, __LINE__); 
	return(MV_OK);
  }

  status = onuEponOamAllocMsg(frame, macId);
  if (status != MV_OK)
  {
	mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE, 
			   "ERROR: (%s:%d) onuEponApiTransmitOamFrame", __FILE_DESC__, __LINE__); 
	return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiDgCallbackRegister
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function register Dying Gasp callback function
**               
**  PARAMETERS:  DYINGGASPFUNC dgCallBack 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponApiDgCallbackRegister(DYINGGASPFUNC dgCallBack)
{
  MV_STATUS status;

  status = onuEponDbDgCallbackSet(dgCallBack);
  if (status != MV_OK)
  {
    mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE, 
               "ERROR: (%s:%d) onuEponDbDgCallbackSet", __FILE_DESC__, __LINE__); 
    return(status);
  }

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponApiLinkStatusCallbackRegister
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function register Link Status callback function
**               
**  PARAMETERS:  LINKSTATUSFUNC linkStatusCallBack
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponApiLinkStatusCallbackRegister(void)
{
  mv_pon_link_state_register(onuEponLinkIsUp,
	 					     &onuEponDb_s.onuEponGenTbl_s.onuLinkStatusCallback);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponLinkIsUp
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function register Link Status callback function
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_TRUE or MV_FLASE
**
*******************************************************************************/
MV_BOOL onuEponLinkIsUp(void)
{
	MV_U32 mode;

	mode = onuEponDbModeGet();

	if (onuEponForceTxDownStateGet(0) == MV_TRUE)
	{
        return (MV_TRUE);
	}

	if (mode == E_EPON_IOCTL_STD_MODE)
	{
        if (onuEponDbOnuStateGet(0) == ONU_EPON_03_OPERATION)
        {
            return (MV_TRUE);
        }
        else
        {
            return (MV_FALSE);
        }
	}
	else /* P2P */
	{
        if (onuEponDbOnuSignalDetectGet() == 1)
        {
            mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE,
                       "DEBUG: (%s:%d) onuPonLinkIsUp, UP\n", __FILE_DESC__, __LINE__);
            return (MV_TRUE);
        }
        else
        {
            mvPonPrint(PON_PRINT_DEBUG, PON_API_MODULE,
                       "DEBUG: (%s:%d) onuPonLinkIsUp, DOWN\n", __FILE_DESC__, __LINE__);
            return (MV_FALSE);
        }
	}
}

MV_STATUS mvEponApi2kSupportedSet(MV_U32 pkt2kSupported)
{
    MV_STATUS status;
    MV_U32    devId;

    devId = mvCtrlModelGet();
	if (devId != MV_6601_DEV_ID)
	{
		return(MV_OK);
	}

    //status =  mvOnuEponMacPcsRxEnableSet(EPON_PCS_CONFIG_RX_DISABLE);

    if (pkt2kSupported == 1)  /* 2K packet supported */
    {
        status |= mvOnuEponMacPcsFrameSizeLimitsSet(EPON_MAC_PCS_FRAME_SIZE_LIMIT_SIZE_2K_SUPP, 
                                                    EPON_MAC_PCS_FRAME_SIZE_LIMIT_LATENCY_2K_SUPP);
        status |= mvOnuEponMacRxpDataFifoThresholdSet(EPON_MAC_RXP_DATA_FIFO_THRESHOLD_2K_SUPP);
    }
    else if (pkt2kSupported == 0)
    {
        status |= mvOnuEponMacPcsFrameSizeLimitsSet(EPON_MAC_PCS_FRAME_SIZE_LIMIT_SIZE_DEF, 
                                                    EPON_MAC_PCS_FRAME_SIZE_LIMIT_LATENCY_DEF);
        status |= mvOnuEponMacRxpDataFifoThresholdSet(EPON_MAC_RXP_DATA_FIFO_THRESHOLD_DEF);
    }

    return (status);
}


