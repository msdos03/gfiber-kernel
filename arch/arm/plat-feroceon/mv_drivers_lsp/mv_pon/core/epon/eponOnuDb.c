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
/**  FILE        : eponOnuDb.c                                               **/
/**                                                                          **/
/**  DESCRIPTION : This file implements ONU EPON database functionality      **/
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
#define __FILE_DESC__ "mv_pon/core/epon/eponOnuDb.c"

/* Global Variables
------------------------------------------------------------------------------*/
/* ONU GPON Database */
S_OnuEponDb onuEponDb_s;

#ifdef PON_FPGA
#define MV_MAC_ADDR_SIZE (6)
#endif /* PON_FPGA */

/* Local Variables
------------------------------------------------------------------------------*/
/* ONU EPON database init function */
MV_STATUS onuEponOnuMacTblInit(void);
MV_STATUS onuEponOnuGenTblInit(void);
MV_STATUS onuEponDbOnuDatapathTblInit(void);

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/
MV_U32 swRprtMacTimerInterval; /* in 16 nano sec units */
MV_U32 swRprtTimerType;

MV_U8 fixMacAddrs[8][6] = {{0x00,0x00,0x00,0x00,0x00,0x82},
                           {0x00,0x00,0x00,0x00,0x00,0x83},
                           {0x00,0x00,0x00,0x00,0x00,0x84},
                           {0x00,0x00,0x00,0x00,0x00,0x85},
                           {0x00,0x00,0x00,0x00,0x00,0x86},
                           {0x00,0x00,0x00,0x00,0x00,0x87},
                           {0x00,0x00,0x00,0x00,0x00,0x88},
                           {0x00,0x00,0x00,0x00,0x00,0x89}};

extern u8 mvMacAddr[CONFIG_MV_ETH_PORTS_NUM][MV_MAC_ADDR_SIZE];

/******************************************************************************/
/* ========================================================================== */
/*                         Initialization Section                             */
/* ========================================================================== */
/******************************************************************************/

/*******************************************************************************
**
**  onuEponDbInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function init onu database to default values
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponDbInit(void)
{
#ifndef PON_FPGA
  onuEponOnuMacTblInit();             
#endif /* PON_FPGA */
  onuEponOnuGenTblInit();             
  onuEponDbOnuDatapathTblInit();      

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbReInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function re-init onu database to default values
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponDbReInit(MV_U32 macId)
{
  /* set onu to state 01 */
  onuEponDbOnuStateSet(ONU_EPON_02_REGISTER_PENDING, macId);

  /* set onu sync time to 0 */
  onuEponDbOnuSyncTimeSet(0, macId);

  onuEponDbPktTxLlidSet(ONU_DEF_TX_LLID, macId);
  onuEponDbPktRxLlidSet(ONU_DEF_RX_LLID, macId);

  if (macId == 0)
    onuPonLedHandler(ONU_PON_SYNC_LED, ACTIVE_LED_BLINK_SLOW);

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponOnuGenTblInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function init onu database MAC table
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponOnuMacTblInit(void)
{
  MV_U32 macId;
  MV_U8  macAddrSkeleton[MV_MAC_ADDR_SIZE];

  memcpy(macAddrSkeleton, mvMacAddr[2], MV_MAC_ADDR_SIZE);
  printk("macAddrSkeleton = %x:%x:%x:%x:%x:%x\n\r", 
         macAddrSkeleton[0], macAddrSkeleton[1], macAddrSkeleton[2],
         macAddrSkeleton[3], macAddrSkeleton[4], macAddrSkeleton[5]);

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) 
  {
    memcpy(fixMacAddrs[macId], macAddrSkeleton, MV_MAC_ADDR_SIZE);

    macAddrSkeleton[5]++;
    if (macAddrSkeleton[5] == 0x10) 
        macAddrSkeleton[5] = 0;

    printk("mac -%d- macAddr = %x:%x:%x:%x:%x:%x\n\r", macId,
           fixMacAddrs[macId][0], fixMacAddrs[macId][1], fixMacAddrs[macId][2],
           fixMacAddrs[macId][3], fixMacAddrs[macId][4], fixMacAddrs[macId][5]);
  }


  return(MV_OK); 
}

/*******************************************************************************
**
**  onuEponOnuGenTblInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function init onu database general table
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK
**                   
*******************************************************************************/
MV_STATUS onuEponOnuGenTblInit(void)
{
	MV_U32 macAddrLow;
	MV_U32 macAddrHigh;
	MV_U32 macId;
	MV_U32 queueSet;
	MV_U32 queueId;

	/* clear onu database general table */
	memset ((&(onuEponDb_s.onuEponGenTbl_s)), 0, sizeof (S_OnuEponGenTbl));

	/* set onu control type to MPCP - 8808 */
	onuEponDbOnuCtrlTypeSet(ONU_MPCP_CTRL_TYPE);

	/* set onu broadcast address */
	onuEponDbOnuBcastAddrSet(ONU_BROADCAST_ADDR_LOW, ONU_BROADCAST_ADDR_HIGH);

	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		/* set onu to valid */
		onuEponDbOnuValidSet(MV_TRUE, macId);

		/* set onu to state 01 */
		onuEponDbOnuStateSet(ONU_EPON_02_REGISTER_PENDING, macId);

		/* set onu sync time to 0 */
		onuEponDbOnuSyncTimeSet(0, macId);

		/* set onu mac address */
		macAddrLow  = ((fixMacAddrs[macId][5] & 0xFF)      ) |
			      ((fixMacAddrs[macId][4] & 0xFF) <<  8) |
			      ((fixMacAddrs[macId][3] & 0xFF) << 16) |
			      ((fixMacAddrs[macId][2] & 0xFF) << 24);
		macAddrHigh = ((fixMacAddrs[macId][1] & 0xFF)      ) |
			      ((fixMacAddrs[macId][0] & 0xFF) << 8 );

		onuEponDbOnuMacAddrSet(macAddrLow, macAddrHigh, macId);
	}

	/* set led */
	onuPonLedHandler(ONU_PON_SYNC_LED, ACTIVE_LED_BLINK_SLOW);

	/* set holdover */
	onuEponDbOnuHoldoverStateSet(ONU_HOLDOVER_NOT_ACTIVE);

	/* set silence */
	onuEponDbOnuSilenceStateSet(ONU_SILENCE_NOT_ACTIVE);


	/* SW DBA params */
	onuEponDbOnuSwRprtTimerTypeSet(ONU_EPON_SW_DBA_RPRT_TIMER);
	onuEponDbOnuSwRprtMacTimerIntervalSet(62500); /* 1 msec in 16nano units */

	onuEponDbOnuDbaModeSet(ONU_DBA_SW_RPRT_MODE);
	onuEponDbOnuDbaSwModeQueuesetDirSet(ONU_DBA_SW_RPRT_NORMAL_DIR);
	onuEponDbOnuDbaSwModeQueueDirSet(ONU_DBA_SW_RPRT_NORMAL_DIR);

	for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) {
		if (macId == 0)
			onuEponDbOnuDbaLlidActiveStatusSet(ONU_DBA_ACTIVE_LLID, macId);
		else
			onuEponDbOnuDbaLlidActiveStatusSet(ONU_DBA_NON_ACTIVE_LLID, macId);

		onuEponDbOnuDbaHighestReportQueueSet(ONU_DBA_DEF_HIGHEST_QUEUE, macId);
		onuEponDbOnuDbaNumOfQueuesSet(ONU_DBA_DEF_NUM_OF_QUEUE, macId);
		onuEponDbOnuDbaNumOfQueuesetSet(ONU_DBA_DEF_NUM_OF_QUEUESET, macId);
		onuEponDbOnuDbaQueuesBitMapSet(ONU_DBA_DEF_QUEUES_BIT_MAP, macId);

		for (queueSet = 0; queueSet < ONU_DBA_DEF_NUM_OF_QUEUESET; queueSet++) {
			for (queueId = 0; queueId < ONU_DBA_DEF_NUM_OF_QUEUE; queueId++) {
				if (queueSet == 0)
					onuEponDbOnuDbaQueueThresholdSet(ONU_DBA_DEF_QUEUESET_DEF_THRESHOLD,
									 1, queueId, queueSet, macId);
				else if (queueSet == 1)
					onuEponDbOnuDbaQueueThresholdSet(ONU_DBA_DEF_QUEUESET_DEF_THRESHOLD,
									 1, queueId, queueSet, macId);
				else if (queueSet == 2)
					onuEponDbOnuDbaQueueThresholdSet(ONU_DBA_DEF_QUEUESET_DEF_THRESHOLD,
									 1, queueId, queueSet, macId);
				else
					onuEponDbOnuDbaQueueThresholdSet(ONU_DBA_DEF_QUEUESET_DEF_THRESHOLD,
									 1, queueId, queueSet, macId);
			}
		}
	}

	/* set onu configuration */
	onuEponDbOnuCfgSet(ONU_REG_REQ_AUTO_RES,
			   ONU_REG_ACK_AUTO_RES,
			   ONU_REPORT_AUTO_RES,
			   ONU_RX_PCS_FEC_DIS,
			   ONU_TX_PCS_FEC_DIS,
			   ONU_TX_FEC_DIS);

	onuEponDbOnuRxFecCfgSet(ONU_RX_PCS_FEC_DIS);

	onuEponDbStatusNotifySet(NULL);
	onuEponDbDgCallbackSet(NULL);
	onuEponDbModeSet(E_EPON_IOCTL_STD_MODE);

	return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDatapathTblInit
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function init onu database data path table
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDatapathTblInit(void)
{
  MV_U32 macId;

  /* clear onu database data path table */
  memset ((&(onuEponDb_s.onuEponDataPathTbl_s)), 0, sizeof (S_OnuEponDatapathTbl));

  for (macId = 0; macId < EPON_MAX_MAC_NUM; macId++) 
  {
    onuEponDb_s.onuEponDataPathTbl_s.onuEponRxLLID[macId] = ONU_UNUSED_LLID;
    onuEponDb_s.onuEponDataPathTbl_s.onuEponTxLLID[macId] = ONU_UNUSED_LLID;

    onuEponDbPktTxLlidSet(ONU_DEF_TX_LLID, macId);
    onuEponDbPktRxLlidSet(ONU_DEF_RX_LLID, macId);
  }

  onuEponDbPktForwardSet(ONU_SLOW_FRAME_TO_CTRL_QUEUE,  
                         ONU_SLOW_FRAME_TO_RPRT_QUEUE,  
                         ONU_RPRT_FRAME_TO_RPRT_QUEUE,  
                         ONU_RPRT_FRAME_TO_DATA_QUEUE,  
                         ONU_CTRL_FRAME_TO_CTRL_QUEUE,  
                         ONU_CTRL_FRAME_TO_DATA_QUEUE); 

  onuEponDbPktFilterPacketSet(ONU_FORWARD_LLID_ALL_PKT, 
                              ONU_FORWARD_LLID_XXXX_MODE_1_PKT, 
                              ONU_FORWARD_LLID_7FFF_MODE_1_PKT, 
                              ONU_FORWARD_LLID_7FFF_MODE_0_PKT, 
                              ONU_DROP_LLID_NNNN_MODE_1_PKT);

  onuEponDbPktFilterErrorSet(ONU_FORWARD_LLID_LEN_ERR_PKT, 
                             ONU_FORWARD_LLID_GMII_ERR_PKT, 
                             ONU_FORWARD_LLID_FCS_ERR_PKT, 
                             ONU_FORWARD_LLID_CRC_ERR_PKT);

  onuEponDbPktSizeSet(ONU_DEF_MIN_PKT_SIZE, ONU_DEF_MAX_PKT_SIZE);

  return(MV_OK);
}

/******************************************************************************/
/* ========================================================================== */
/*                         Interface Section                                  */
/* ========================================================================== */
/******************************************************************************/

/********************************************/
/* ======================================== */
/*   ONU EPON General Table API Functions   */
/* ======================================== */
/********************************************/
MV_STATUS matchDestAddressToMacId(MV_U8 *destAddr, MV_U32 *macId)
{
  MV_U32 addrIndex;

  for (addrIndex = 0; addrIndex < EPON_MAX_MAC_NUM; addrIndex++) 
  {
    if (memcmp(fixMacAddrs[addrIndex], destAddr, 6) == 0)
    {
      *macId = addrIndex;
      return(MV_OK);
    }
  }

  return(MV_ERROR);
}

/*******************************************************************************
**
**  onuEponDbOnuValidSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu valid in the database
**               
**  PARAMETERS:  MV_BOOL onuValid  
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuValidSet(MV_BOOL onuValid, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponValid[macId] = onuValid;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuValidGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu valid 
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     onu state
**                   
*******************************************************************************/
MV_BOOL onuEponDbOnuValidGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponValid[macId]);
}

/*******************************************************************************
**
**  onuEponDbOnuStateSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu state in the database
**               
**  PARAMETERS:  MV_U32 onuState - ONU_EPON_02_REGISTER_PENDING
**                                 ONU_EPON_03_OPERATION         
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuStateSet(MV_U32 onuState, MV_U32 macId)
{
  if ((onuState < ONU_EPON_02_REGISTER_PENDING) ||
      (onuState > ONU_EPON_03_OPERATION))
  {
    return(MV_ERROR);
  }

  onuEponDb_s.onuEponGenTbl_s.onuEponState[macId] = onuState;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuStateGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu state 
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     onu state
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuStateGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponState[macId]);
}

/*******************************************************************************
**
**  onuEponDbOnuSignalDetectSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu signal detect in the database
**               
**  PARAMETERS:  MV_U32 state       
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuSignalDetectSet(MV_U32 state)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponSignalDetect = state;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuSignalDetectGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu state signal detect
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     onu state
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuSignalDetectGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponSignalDetect);
}

/*******************************************************************************
**
**  onuEponDbOnuDsSyncOkPcsSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu ds sync OK PCS in the database
**               
**  PARAMETERS:  MV_U32 state       
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDsSyncOkPcsSet(MV_U32 state)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDsSyncOkPcs = state;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDsSyncOkPcsGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu ds sync OK PCS
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     onu state
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDsSyncOkPcsGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDsSyncOkPcs);
}

/*******************************************************************************
**
**  onuEponDbOnuCtrlTypeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu ether type in the database
**               
**  PARAMETERS:  MV_U32 ctrlType 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuCtrlTypeSet(MV_U32 ctrlType)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponCtrlType = ctrlType;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuCtrlTypeGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu ether type 
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuCtrlTypeGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponCtrlType);
}

/*******************************************************************************
**
**  onuEponDbOnuSyncTimeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu sync time in the database
**               
**  PARAMETERS:  MV_U32 syncTime
**               MV_U32 macId 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuSyncTimeSet(MV_U32 syncTime, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponSyncTime[macId] = syncTime;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuSyncTimeGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu sync time
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     onu sync time
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuSyncTimeGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponSyncTime[macId]);
}

/*******************************************************************************
**
**  onuEponDbOnuMacAddrSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu mac address in the database
**               
**  PARAMETERS:  MV_U32 lowAddr
**               MV_U32 highAddr
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuMacAddrSet(MV_U32 lowAddr, MV_U32 highAddr, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponMacAddr[macId].addressLow  = lowAddr;
  onuEponDb_s.onuEponGenTbl_s.onuEponMacAddr[macId].addressHigh = highAddr;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuMacAddrGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu mac address in the database
**               
**  PARAMETERS:  MV_U32 *lowAddr
**               MV_U32 *highAddr
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuMacAddrGet(MV_U32 *lowAddr, MV_U32 *highAddr, MV_U32 macId)
{
  *lowAddr  = onuEponDb_s.onuEponGenTbl_s.onuEponMacAddr[macId].addressLow; 
  *highAddr = onuEponDb_s.onuEponGenTbl_s.onuEponMacAddr[macId].addressHigh; 

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuBcastAddrSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu mac broadcast address in the database
**               
**  PARAMETERS:  MV_U32 lowAddr
**               MV_U32 highAddr
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuBcastAddrSet(MV_U32 lowAddr, MV_U32 highAddr)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponBcastAddr.addressLow  = lowAddr;
  onuEponDb_s.onuEponGenTbl_s.onuEponBcastAddr.addressHigh = highAddr;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuMacAddrGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu mac broadcast address in the database
**               
**  PARAMETERS:  MV_U32 *lowAddr
**               MV_U32 *highAddr
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuBcastAddrGet(MV_U32 *lowAddr, MV_U32 *highAddr)
{
  *lowAddr  = onuEponDb_s.onuEponGenTbl_s.onuEponBcastAddr.addressLow; 
  *highAddr = onuEponDb_s.onuEponGenTbl_s.onuEponBcastAddr.addressHigh; 

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuCfgAutoSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu mac auto configuration in the database
**               
**  PARAMETERS:  MV_U32 regReqAutoRes
**               MV_U32 regAckAutoRes
**               MV_U32 reportAutoRes
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuCfgAutoSet(MV_U32 regReqAutoRes, 
                                 MV_U32 regAckAutoRes, 
                                 MV_U32 reportAutoRes)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRegReqAutoRes = regReqAutoRes;
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRegAckAutoRes = regAckAutoRes;
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponReportAutoRes = reportAutoRes;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuCfgSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu mac configuration in the database
**               
**  PARAMETERS:  MV_U32 regReqAutoRes
**               MV_U32 regAckAutoRes
**               MV_U32 reportAutoRes
**               MV_U32 rxPcsFecEn        
**               MV_U32 txPcsFecEn   
**               MV_U32 txFecEn     
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuCfgSet(MV_U32 regReqAutoRes, 
                             MV_U32 regAckAutoRes, 
                             MV_U32 reportAutoRes, 
                             MV_U32 rxPcsFecEn, 
                             MV_U32 txPcsFecEn, 
                             MV_U32 txFecEn)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRegReqAutoRes = regReqAutoRes;
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRegAckAutoRes = regAckAutoRes;
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponReportAutoRes = reportAutoRes;
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRxPcsFecEn    = rxPcsFecEn;      
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponTxPcsFecEn    = txPcsFecEn;      
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponTxFecEn       = txFecEn;      

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuCfgGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu mac configuration in the database
**               
**  PARAMETERS:  MV_U32 regReqAutoRes
**               MV_U32 regAckAutoRes
**               MV_U32 reportAutoRes
**               MV_U32 rxFec        
**               MV_U32 txFec        
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuCfgGet(MV_U32 *regReqAutoRes, 
                             MV_U32 *regAckAutoRes, 
                             MV_U32 *reportAutoRes, 
                             MV_U32 *rxPcsFecEn, 
                             MV_U32 *txPcsFecEn,
                             MV_U32 *txFecEn)
{
  *regReqAutoRes = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRegReqAutoRes;
  *regAckAutoRes = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRegAckAutoRes;
  *reportAutoRes = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponReportAutoRes;
  *rxPcsFecEn    = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRxPcsFecEn;      
  *txPcsFecEn    = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponTxPcsFecEn;      
  *txFecEn       = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponTxFecEn;
                  
  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuRxFecCfgSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu mac Rx FEC configuration in the database
**               
**  PARAMETERS:  MV_U32 rxPcsFecEn        
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuRxFecCfgSet(MV_U32 rxPcsFecEn)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRxPcsFecEn = rxPcsFecEn;      

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuRxFecCfgGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu mac Rx FEC configuration in the database
**               
**  PARAMETERS:  MV_U32 rxFec        
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuRxFecCfgGet(MV_U32 *rxPcsFecEn)
{
  *rxPcsFecEn = onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponRxPcsFecEn;      
                  
  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuTxFecCfgGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu mac Tx FEC configuration in the database
**               
**  PARAMETERS:  MV_U32 txFec   
**               MV_U32 macId     
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuTxFecCfgGet(MV_U32 *txFecEn, MV_U32 macId)
{
  *txFecEn = ((onuEponDb_s.onuEponGenTbl_s.onuEponCfg.onuEponTxFecEn >> macId) & 0x1);      
                  
  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaModeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu DBA mode 
**               
**  PARAMETERS:  MV_U32 mode - HW(0) SW(1)
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaModeSet(MV_U32 mode)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDbaMode = mode;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaModeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu DBA mode 
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 mode - HW(0) SW(1)
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaModeGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDbaMode);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaSwModeQueuesetDirSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu SW mode queueset direction  
**               
**  PARAMETERS:  MV_U32 dir - Normal(0) Reverse(1)
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaSwModeQueuesetDirSet(MV_U32 dir)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDbaSwQueuesetDir = dir;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaSwModeQueuesetDirGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu SW mode queueset direction
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 dir - Normal(0) Reverse(1)
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaSwModeQueuesetDirGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDbaSwQueuesetDir);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaSwModeQueueDirSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu SW mode queues direction  
**               
**  PARAMETERS:  MV_U32 dir - Normal(0) Reverse(1)
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaSwModeQueueDirSet(MV_U32 dir)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDbaSwQueueDir = dir;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaSwModeQueueDirGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu SW mode queues direction
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 dir - Normal(0) Reverse(1)
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaSwModeQueueDirGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDbaSwQueueDir);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaLlidActiveStatusSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu DBA LLID active status 
**               
**  PARAMETERS:  MV_U32 status - Off(0) On(1)
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaLlidActiveStatusSet(MV_U32 status, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].active = status;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaLlidActiveStatusGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu DBA mode 
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 status - Off(0) On(1)
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaLlidActiveStatusGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].active);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaHighestReportQueueSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu highest DBA report queue
**               
**  PARAMETERS:  MV_U32 queueNum 
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaHighestReportQueueSet(MV_U32 queueNum, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].highestReportQueue = queueNum;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaHighestReportQueueGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu highest DBA report queue
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaHighestReportQueueGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].highestReportQueue);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaQueuesBitMapSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu queues bit map per LLID
**               
**  PARAMETERS:  MV_U32 queuesBitMap 
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaQueuesBitMapSet(MV_U32 queuesBitMap, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].queuesBitMap = queuesBitMap;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaQueuesBitMapGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu queues bit map per LLID
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     onu queues bit map
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaQueuesBitMapGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].queuesBitMap);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaNumOfQueuesSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu number of queues per LLID
**               
**  PARAMETERS:  MV_U32 numOfQueues 
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaNumOfQueuesSet(MV_U32 numOfQueues, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].numOfQueues = numOfQueues;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaNumOfQueuesGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu number of queues per LLID
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaNumOfQueuesGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].numOfQueues);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaNumOfQueuesetSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu number of queueset per LLID
**               
**  PARAMETERS:  MV_U32 numOfQueueset 
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaNumOfQueuesetSet(MV_U32 numOfQueueset, MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].numOfQueueSets = numOfQueueset;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaNumOfQueuesetGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu number of queueset per LLID
**               
**  PARAMETERS:  MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuDbaNumOfQueuesetGet(MV_U32 macId)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].numOfQueueSets);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaQueueThresholdSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu queue threshold per LLID
**               
**  PARAMETERS:  MV_U32 threshold  
**               MV_U32 state
**               MV_U32 queueId 
**               MV_U32 queueSet
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaQueueThresholdSet(MV_U32 threshold, 
										   MV_U32 state, 
										   MV_U32 queueId, 
										   MV_U32 queueSet, 
										   MV_U32 macId)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].queueThreshold[queueSet][queueId].threshold = threshold;
  onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].queueThreshold[queueSet][queueId].state     = state;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaQueueThresholdGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu queue threshold per LLID
**               
**  PARAMETERS:  MV_U32 *threshold  
**               MV_U32 *state
**               MV_U32 queueId 
**               MV_U32 queueSet
**               MV_U32 macId
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaQueueThresholdGet(MV_U32 *threshold, 
										   MV_U32 *state, 
										   MV_U32 queueId, 
										   MV_U32 queueSet,
										   MV_U32 macId)
{
  *threshold = onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].queueThreshold[queueSet][queueId].threshold; 
  *state     = onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId].queueThreshold[queueSet][queueId].state; 

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuDbaLlidThresholdGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu llid threshold per LLID
**               
**  PARAMETERS:  S_OnuEponDba *llidDba 
**               MV_U32        macId
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuDbaLlidThresholdGet(S_OnuEponDba *llidDba, MV_U32 macId)
{
  memcpy(llidDba, &(onuEponDb_s.onuEponGenTbl_s.onuEponDba[macId]), sizeof(S_OnuEponDba));

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuHoldoverStateSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu holdover state in the database
**               
**  PARAMETERS:  MV_U32 state 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuHoldoverStateSet(MV_U32 state)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponHoldoverState = state;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuHoldoverStateGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu holdover state
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuHoldoverStateGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponHoldoverState);
}
/*******************************************************************************
**
**  onuEponDbOnuHoldoverTimeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu holdover time in the database
**               
**  PARAMETERS:  MV_U32 time 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuHoldoverTimeSet(MV_U32 time)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponHoldoverTime = time;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuHoldoverTimeGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu holdover time
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuHoldoverTimeGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponHoldoverTime);
}

/*******************************************************************************
**
**  onuEponDbOnuHoldoverExecSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu holdover execute in the database
**               
**  PARAMETERS:  MV_U32 state 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuHoldoverExecSet(MV_U32 state)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponHoldoverExecute = state;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuHoldoverExecGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu holdover execute
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuHoldoverExecGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponHoldoverExecute);
}

/*******************************************************************************
**
**  onuEponDbOnuSilenceStateSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu silence state in the database
**               
**  PARAMETERS:  MV_U32 state 
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuSilenceStateSet(MV_U32 state)
{
  onuEponDb_s.onuEponGenTbl_s.onuEponSilenceState = state;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuSilenceStateGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu silence state
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     ctrl frame ether type
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuSilenceStateGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuEponSilenceState);
}

/********************************************/
/* ======================================== */
/*   ONU EPON DataPath Table API Functions  */
/* ======================================== */
/********************************************/

/*******************************************************************************
**
**  onuEponDbPktSizeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu mac RX packet size
**               
**  PARAMETERS:  MV_U32 minSize 
**               MV_U32 maxSize
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktSizeSet(MV_U32 minSize, MV_U32 maxSize)
{
  onuEponDb_s.onuEponDataPathTbl_s.onuEponRxPktMinSize = minSize;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponRxPktMaxSize = maxSize;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktSizeGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu mac RX packet size
**               
**  PARAMETERS:  MV_U32 *minSize 
**               MV_U32 *maxSize
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktSizeGet(MV_U32 *minSize, MV_U32 *maxSize)
{
 *minSize = onuEponDb_s.onuEponDataPathTbl_s.onuEponRxPktMinSize;  
 *maxSize = onuEponDb_s.onuEponDataPathTbl_s.onuEponRxPktMaxSize;  

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktFilterPacketSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu filter packet configuration
**               
**  PARAMETERS:  MV_U32 forwardLlidAll 
**               MV_U32 forwardLlid1XXX
**               MV_U32 forwardLlid1FFF
**               MV_U32 forwardLlid0FFF
**               MV_U32 dropLlid1NNN
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktFilterPacketSet(MV_U32 forwardLlidAll, 
                                      MV_U32 forwardLlid1XXX, 
                                      MV_U32 forwardLlid1FFF, 
                                      MV_U32 forwardLlid0FFF,
                                      MV_U32 dropLlid1NNN)
{
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlidAll  = forwardLlidAll;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlid1XXX = forwardLlid1XXX;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlid1FFF = forwardLlid1FFF;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlid0FFF = forwardLlid0FFF;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.dropLlid1NNN    = dropLlid1NNN;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktFilterPacketGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu filter packet configuration
**               
**  PARAMETERS:  MV_U32 *forwardLlidAll 
**               MV_U32 *forwardLlid1XXX
**               MV_U32 *forwardLlid1FFF
**               MV_U32 *forwardLlid0FFF
**               MV_U32 *dropLlid1NNN
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktFilterPacketGet(MV_U32 *forwardLlidAll, 
                                      MV_U32 *forwardLlid1XXX, 
                                      MV_U32 *forwardLlid1FFF, 
                                      MV_U32 *forwardLlid0FFF,
                                      MV_U32 *dropLlid1NNN)
{
  *forwardLlidAll  = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlidAll; 
  *forwardLlid1XXX = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlid1XXX;
  *forwardLlid1FFF = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlid1FFF;
  *forwardLlid0FFF = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.forwardLlid0FFF;
  *dropLlid1NNN    = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.dropLlid1NNN;


  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktFilterErrorSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu filter error configuration
**               
**  PARAMETERS:  MV_U32 ignoreLenErr   
**               MV_U32 ignoreGmiiErr  
**               MV_U32 ignoreFcsErr   
**               MV_U32 ignoreLlidCrcErr
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktFilterErrorSet(MV_U32 ignoreLenErr, 
                                     MV_U32 ignoreGmiiErr, 
                                     MV_U32 ignoreFcsErr, 
                                     MV_U32 ignoreLlidCrcErr)
{
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreLenErr     = ignoreLenErr;   
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreGmiiErr    = ignoreGmiiErr;  
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreFcsErr     = ignoreFcsErr;   
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreLlidCrcErr = ignoreLlidCrcErr;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktFilterErrorGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu filter error configuration
**               
**  PARAMETERS:  MV_U32 *ignoreLenErr   
**               MV_U32 *ignoreGmiiErr  
**               MV_U32 *ignoreFcsErr   
**               MV_U32 *ignoreLlidCrcErr
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktFilterErrorGet(MV_U32 *ignoreLenErr, 
                                     MV_U32 *ignoreGmiiErr, 
                                     MV_U32 *ignoreFcsErr, 
                                     MV_U32 *ignoreLlidCrcErr)
{
  *ignoreLenErr     = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreLenErr;  
  *ignoreGmiiErr    = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreGmiiErr; 
  *ignoreFcsErr     = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreFcsErr;  
  *ignoreLlidCrcErr = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktFilter.ignoreLlidCrcErr; 

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktForwardSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu forwarding configuration
**               
**  PARAMETERS:  MV_U32 slowFrameToCtrlQueue
**               MV_U32 slowFrameToRprtQueue
**               MV_U32 rprtFrameToRprtQueue
**               MV_U32 rprtFrameToDataQueue
**               MV_U32 ctrlFrameToCtrlQueue
**               MV_U32 ctrlFrameToDataQueue
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktForwardSet(MV_U32 slowFrameToCtrlQueue, 
                                 MV_U32 slowFrameToRprtQueue,
                                 MV_U32 rprtFrameToRprtQueue, 
                                 MV_U32 rprtFrameToDataQueue,
                                 MV_U32 ctrlFrameToCtrlQueue, 
                                 MV_U32 ctrlFrameToDataQueue)
{
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.slowFrameToCtrlQueue = slowFrameToCtrlQueue;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.slowFrameToRprtQueue = slowFrameToRprtQueue;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.rprtFrameToRprtQueue = rprtFrameToRprtQueue;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.rprtFrameToDataQueue = rprtFrameToDataQueue;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.ctrlFrameToCtrlQueue = ctrlFrameToCtrlQueue;
  onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.ctrlFrameToDataQueue = ctrlFrameToDataQueue;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktForwardGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu forwarding configuration
**               
**  PARAMETERS:  MV_U32 *slowFrameToCtrlQueue
**               MV_U32 *slowFrameToRprtQueue
**               MV_U32 *rprtFrameToRprtQueue
**               MV_U32 *rprtFrameToDataQueue
**               MV_U32 *ctrlFrameToCtrlQueue
**               MV_U32 *ctrlFrameToDataQueue
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktForwardGet(MV_U32 *slowFrameToCtrlQueue, 
                                 MV_U32 *slowFrameToRprtQueue,
                                 MV_U32 *rprtFrameToRprtQueue, 
                                 MV_U32 *rprtFrameToDataQueue,
                                 MV_U32 *ctrlFrameToCtrlQueue, 
                                 MV_U32 *ctrlFrameToDataQueue)
{
  *slowFrameToCtrlQueue = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.slowFrameToCtrlQueue;
  *slowFrameToRprtQueue = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.slowFrameToRprtQueue;
  *rprtFrameToRprtQueue = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.rprtFrameToRprtQueue;
  *rprtFrameToDataQueue = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.rprtFrameToDataQueue;
  *ctrlFrameToCtrlQueue = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.ctrlFrameToCtrlQueue;
  *ctrlFrameToDataQueue = onuEponDb_s.onuEponDataPathTbl_s.onuEponPktForward.ctrlFrameToDataQueue;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktRxLlidSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu Rx LLID
**               
**  PARAMETERS:  MV_U32 llid 
**               MV_U32 index
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktRxLlidSet(MV_U32 llid, MV_U32 index)
{
  if (index >= EPON_MAX_MAC_NUM) 
    return(MV_ERROR);

  onuEponDb_s.onuEponDataPathTbl_s.onuEponRxLLID[index] = llid;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktRxLlidGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu Rx LLID
**               
**  PARAMETERS:  MV_U32 *llid 
**               MV_U32 index
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktRxLlidGet(MV_U32 *llid, MV_U32 index)
{
  if (index >= EPON_MAX_MAC_NUM) 
    return(MV_ERROR);

  *llid = onuEponDb_s.onuEponDataPathTbl_s.onuEponRxLLID[index];

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktTxLlidSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set onu Tx LLID
**               
**  PARAMETERS:  MV_U32 llid 
**               MV_U32 index
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktTxLlidSet(MV_U32 llid, MV_U32 index)
{
  if (index >= EPON_MAX_MAC_NUM) 
    return(MV_ERROR);

  onuEponDb_s.onuEponDataPathTbl_s.onuEponTxLLID[index] = llid;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPktTxLlidGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu Tx LLID
**               
**  PARAMETERS:  MV_U32 *llid 
**               MV_U32 index
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPktTxLlidGet(MV_U32 *llid, MV_U32 index)
{
  if (index >= EPON_MAX_MAC_NUM) 
    return(MV_ERROR);

  *llid = onuEponDb_s.onuEponDataPathTbl_s.onuEponTxLLID[index];

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbStatusNotifySet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set Status Notify Callback in the database
**               
**  PARAMETERS:  STATUSNOTIFYFUNC statusCallback      
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbStatusNotifySet(STATUSNOTIFYFUNC statusCallback)
{
  onuEponDb_s.onuEponGenTbl_s.onuStatusCallback = statusCallback;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbStatusNotifyGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu Status Notify Callback
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     Status Notify Callback
**                   
*******************************************************************************/
STATUSNOTIFYFUNC onuEponDbStatusNotifyGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuStatusCallback);
}

/*******************************************************************************
**
**  onuEponDbOnuSwRprtTimerTypeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set SW report timer type MAC or OS
**               
**  PARAMETERS:  MV_U32 timer type      
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuSwRprtTimerTypeSet(MV_U32 timerType)
{
  swRprtTimerType = timerType;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuSwRprtTimerTypeGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return SW report timer type MAC or OS
**               
**  PARAMETERS:  None     
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 timer type 
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuSwRprtTimerTypeGet(void)
{
  return(swRprtTimerType);
}

/*******************************************************************************
**
**  onuEponDbOnuSwRprtTimerTypeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set SW report MAC timer interval
**               
**  PARAMETERS:  MV_U32 timer interval      
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOnuSwRprtMacTimerIntervalSet(MV_U32 timerInterval)
{
  swRprtMacTimerInterval = timerInterval;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOnuSwRprtMacTimerIntervalGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return SW report MAC timer interval
**               
**  PARAMETERS:  None     
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 timer interval 
**                   
*******************************************************************************/
MV_U32 onuEponDbOnuSwRprtMacTimerIntervalGet(void)
{
  return(swRprtMacTimerInterval);
}


/*******************************************************************************
**
**  onuEponDbModeSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function sets EPON mode in the database
**               
**  PARAMETERS:  MV_U32 mode
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbModeSet(MV_U32   mode)
{
    if (mode >= E_EPON_IOCTL_MAX_MODE_NUM) 
      return(MV_ERROR);

    onuEponDb_s.onuEponGenTbl_s.onuEponMode = mode;

    return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbModeGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function returns current EPON mode
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 mode
**                   
*******************************************************************************/
MV_U32 onuEponDbModeGet(void)
{
    return(onuEponDb_s.onuEponGenTbl_s.onuEponMode);
}

/*******************************************************************************
**
**  onuEponDbPcsCfgSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function sets EPON PCS configuration register value in the database
**               
**  PARAMETERS:  MV_U32 val
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbPcsCfgSet(MV_U32   val)
{
    onuEponDb_s.onuEponGenTbl_s.onuEponPcsCfg = val;

    return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbPcsCfgGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function returns EPON PCS configuration register value
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 mode
**                   
*******************************************************************************/
MV_U32 onuEponDbPcsCfgGet(void)
{
    return(onuEponDb_s.onuEponGenTbl_s.onuEponPcsCfg);
}

/*******************************************************************************
**
**  onuEponDbOverheadSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function sets EPON Overhead register value in the database
**               
**  PARAMETERS:  MV_U32 val
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbOverheadSet(MV_U32   val)
{
    onuEponDb_s.onuEponGenTbl_s.onuEponOverHead = val;

    return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbOverheadGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function returns EPON Overhead register value
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 mode
**                   
*******************************************************************************/
MV_U32 onuEponDbOverheadGet(void)
{
    return(onuEponDb_s.onuEponGenTbl_s.onuEponOverHead);
}

/*******************************************************************************
**
**  onuEponDbXvrPolaritySet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function sets EPON XVR polarity register value in the database
**               
**  PARAMETERS:  MV_U32 val
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbXvrPolaritySet(MV_U32   val)
{
    onuEponDb_s.onuEponGenTbl_s.onuEponXvrPolarity = val;

    return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbXvrPolarityGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function returns EPON XVR polarity register value
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_U32 mode
**                   
*******************************************************************************/
MV_U32 onuEponDbXvrPolarityGet(void)
{
    return(onuEponDb_s.onuEponGenTbl_s.onuEponXvrPolarity);
}

/*******************************************************************************
**
**  onuEponDbDgCallbackSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set Dying Gasp Callback in the database
**               
**  PARAMETERS:  DYINGGASPFUNC dgCallback      
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbDgCallbackSet(DYINGGASPFUNC dgCallback)
{
  onuEponDb_s.onuEponGenTbl_s.onuDgCallback = dgCallback;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbDgCallbackGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu Dying Gasp Callback
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     Dying Gasp Callback
**                   
*******************************************************************************/
DYINGGASPFUNC onuEponDbDgCallbackGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuDgCallback);
}

/*******************************************************************************
**
**  onuEponDbLinkStatusCallbackSet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function set Link Status Callback in the database
**               
**  PARAMETERS:  LINKSTATUSFUNC linkStatusCallback    
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or error
**                   
*******************************************************************************/
MV_STATUS onuEponDbLinkStatusCallbackSet(LINKSTATUSFUNC linkStatusCallback)
{
  onuEponDb_s.onuEponGenTbl_s.onuLinkStatusCallback = linkStatusCallback;

  return(MV_OK);
}

/*******************************************************************************
**
**  onuEponDbLinkStatusCallbackGet
**  ____________________________________________________________________________
** 
**  DESCRIPTION: The function return onu Link Status Callback
**               
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     Link Status Callback
**                   
*******************************************************************************/
LINKSTATUSFUNC onuEponDbLinkStatusCallbackGet(void)
{
  return(onuEponDb_s.onuEponGenTbl_s.onuLinkStatusCallback);
}

