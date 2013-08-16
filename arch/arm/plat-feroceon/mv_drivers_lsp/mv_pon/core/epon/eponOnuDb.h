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
/**  FILE        : eponOnuDb.h                                               **/
/**                                                                          **/
/**  DESCRIPTION : This file contains ONU EPON database definitions          **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *                              
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *  
 * ========================================================================== *      
 *                                                                     
 ******************************************************************************/
#ifndef _ONU_EPON_DB_H
#define _ONU_EPON_DB_H

/* Include Files
------------------------------------------------------------------------------*/
#include "eponOnuLnxKsMI.h"
 
/* Definitions
------------------------------------------------------------------------------*/ 
/******************************************************************************/
/* ========================================================================== */
/*                               Database Definitions                         */
/* ========================================================================== */
/******************************************************************************/

/* EPON MAC State Definitions */
#define ONU_EPON_REGISTERED               (0x01) 
#define ONU_EPON_NOT_REGISTERD            (0x00)
 
/* EPON State Definitions */
#define ONU_EPON_02_REGISTER_PENDING      (0x01) 
#define ONU_EPON_03_OPERATION             (0x02) 
#define ONU_EPON_MAX_NUM_OF_STATE         (4) 

/* EPON Event Definitions */
#define ONU_EPON_REGISTER_MSG_FLAG_ACK    (0x01) 
#define ONU_EPON_REGISTER_MSG_FLAG_DEREG  (0x02) 
#define ONU_EPON_REGISTER_MSG_FLAG_REREG  (0x03) 
#define ONU_EPON_MAX_NUM_OF_EVENT         (8) 

/* EPON Configuration Definitions */
#define ONU_REG_REQ_AUTO_RES              (0x01) 
#define ONU_REG_REQ_SW_RES                (0x00) 
#define ONU_REG_ACK_AUTO_RES              (0x01) 
#define ONU_REG_ACK_SW_RES                (0x00) 
#define ONU_REPORT_AUTO_RES               (0x01) 
#define ONU_REPORT_SW_RES                 (0x00) 
#define ONU_RX_PCS_FEC_EN                 (0x01) 
#define ONU_RX_PCS_FEC_DIS                (0x00) 
#define ONU_TX_PCS_FEC_EN                 (0x01) 
#define ONU_TX_PCS_FEC_DIS                (0x00) 
#define ONU_TX_FEC_DIS                    (0x00) 
#define ONU_RX_DIS                        (0x00) 
#define ONU_RX_EN                         (0x01) 
#define ONU_TX_DIS                        (0x00) 
#define ONU_TX_EN                         (0x01) 

/* EPON Registration Definitions */
#define ONU_REGISTER                      (0x01) 
#define ONU_REGISTER_ACK                  (0x02) 
#define ONU_REGISTER_REQ                  (0x03) 

/* EPON ONU Sync Time Definitions */
#define ONU_DEF_SYNC_TIME_ADD             (0x00)
#define ONU_DEF_SYNC_TIME_DIS_DISCOVER_AUTO (0x01)
#define ONU_DEF_SYNC_TIME_DIS_GATE_AUTO   (0x00)
#define ONU_DEF_SYNC_TIME_FORCE_SW        (0x00)
#define ONU_DEF_SYNC_TIME                 (0x34)

/* EPON ONU DDM Definitions */
#define ONU_DEF_DDM_DELAY                 (0x08)

/* EPON Packet Filtering Definitions */
#define ONU_FORWARD_LLID_ALL_PKT          (0x00) 
#define ONU_FORWARD_LLID_ALL_PKT_FILTER   (0x01) 
#define ONU_FORWARD_LLID_7FFF_MODE_0_PKT  (0x01) 
#define ONU_FORWARD_LLID_7FFF_MODE_1_PKT  (0x01) 
#define ONU_FORWARD_LLID_XXXX_MODE_1_PKT  (0x00) 
#define ONU_DROP_LLID_NNNN_MODE_1_PKT     (0x00) 

/* EPON Error Filtering Definitions */
#define ONU_FORWARD_LLID_CRC_ERR_PKT      (0x00) 
#define ONU_FORWARD_LLID_CRC_ERR_PKT_IGNORE (0x01) 
#define ONU_FORWARD_LLID_FCS_ERR_PKT      (0x00) 
#define ONU_FORWARD_LLID_GMII_ERR_PKT     (0x00) 
#define ONU_FORWARD_LLID_LEN_ERR_PKT      (0x00) 

/* EPON Packet Forwarding Definitions */
#define ONU_SLOW_FRAME_TO_CTRL_QUEUE      (0x00) 
#define ONU_SLOW_FRAME_TO_RPRT_QUEUE      (0x00) 
#define ONU_RPRT_FRAME_TO_RPRT_QUEUE      (0x00) 
#define ONU_RPRT_FRAME_TO_DATA_QUEUE      (0x00) 
#define ONU_CTRL_FRAME_TO_CTRL_QUEUE      (0x01) 
#define ONU_CTRL_FRAME_TO_DATA_QUEUE      (0x00) 

/* EPON Ether Type Definitions */
#define ONU_MPCP_CTRL_TYPE                (0x8808) 
#define ONU_OAM_CTRL_TYPE                 (0x8809) 

/* EPON MAC Address Definitions */
#define ONU_BROADCAST_ADDR_LOW            (0xC2000001) 
#define ONU_BROADCAST_ADDR_HIGH           (0x00000180) 
#define ONU_MAC_ADDR_LOW                  (0x09b0302c) 
#define ONU_MAC_ADDR_HIGH                 (0x00000013) 

/* EPON LLID Definitions */
#define ONU_UNUSED_LLID                   (0xFFFF)
#define ONU_DEF_TX_LLID                   (0x07FF)
#define ONU_DEF_RX_LLID                   (0x0000)
#define ONU_LLID_VALUE_MASK               (0x7FFF)
#define ONU_LLID_INDEX_MASK               (0x000F)
#define ONU_LLID_VALID_MASK               (0x0001)
#define ONU_LLID_VALUE_SHIFT              (0)
#define ONU_LLID_INDEX_SHIFT              (15)
#define ONU_LLID_VALID_SHIFT              (19)

/* EPON Laser Definitions */
#define ONU_DEF_LASER_ON_TIME             (0x10)
#define ONU_DEF_LASER_OFF_TIME            (0x00)
#define ONU_DEF_LASER_ON_OFF_TIME         (0x30)

/* EPON TXM Definitions */
#ifdef PON_Z1
#define ONU_DEF_TXM_CFG_MODE              (0x0)
#define ONU_DEF_TXM_CFG_ALIGNMENT         (0x1)
#define ONU_DEF_TXM_CFG_PRIORITY          (0x0)
#else /* PON_Z2 */
#define ONU_DEF_TXM_CFG_TFEC_1              (0x2)
#define ONU_DEF_TXM_CFG_TFEC_2              (0x2)
#define ONU_DEF_TXM_CFG_CTRL_FIFO_DIS_FCS_ON  (0x1)
#define ONU_DEF_TXM_CFG_CTRL_FIFO_DIS_FCS_OFF (0x0)
#define ONU_DEF_TXM_CFG_MASK_GMII_ON        (0x1)
#define ONU_DEF_TXM_CFG_MASK_GMII_OFF       (0x0)
#define ONU_DEF_TXM_CFG_BLOCK_DATA_ON       (0x1) 
#define ONU_DEF_TXM_CFG_BLOCK_DATA_OFF      (0x0) 
#define ONU_DEF_TXM_CFG_IPG_ADD             (0x1)
#define ONU_DEF_TXM_CFG_IPG_DONT_ADD        (0x0)
#define ONU_DEF_TXM_CFG_MODE_EPON           (0x00)
#define ONU_DEF_TXM_CFG_MODE_GE             (0x01)
#define ONU_DEF_TXM_CFG_MODE_TX_DIS         (0x10)
#define ONU_DEF_TXM_CFG_ALIGNMENT_ODD       (0x10)
#define ONU_DEF_TXM_CFG_ALIGNMENT_EVEN      (0x01)
#define ONU_DEF_TXM_CFG_ALIGNMENT_DONT_CARE (0x00)
#define ONU_DEF_TXM_CFG_PRIORITY_CTRL_FIRST (0x1)
#define ONU_DEF_TXM_CFG_PRIORITY_RPRT_FIRST (0x0)
#define ONU_DEF_TXM_CFG_EPON_OVERHEAD       (0x0E01)
#define ONU_DEF_TXM_CFG_P2P_OVERHEAD        (0x0E06)
#endif /* PON_Z2 */

/* EPON PCS Definitions */
#define ONU_DEF_PCS_CFG_RX_EN               (0x01)
#define ONU_DEF_PCS_CFG_TX_EN               (0x10)

/* P2P PCS Definitions */
#define ONU_P2P_PCS_TX_RX_NEW               (0x610)
#define ONU_P2P_PCS_TX_NEW                  (0x611)
#define ONU_P2P_PCS_TX_RX_OLD               (0x613)

/* EPON DDM Definitions */
#define ONU_DEF_DDM_CFG_TX_EN_OR            (0x2)
#define ONU_DEF_DDM_CFG_POLARITY_HIGH       (0x0)
#define ONU_DEF_DDM_CFG_POLARITY_LOW        (0x1)

/* EPON Packet Size */
#define ONU_DEF_MIN_PKT_SIZE                (64)
#define ONU_DEF_MAX_PKT_SIZE                (2040)

/* EPON Holdover State */
#define ONU_HOLDOVER_NOT_ACTIVE             (0)
#define ONU_HOLDOVER_ACTIVE                 (1)

/* EPON Silence State */
#define ONU_SILENCE_NOT_ACTIVE              (0)
#define ONU_SILENCE_ACTIVE                  (1)
#define ONU_SILENCE_DISABLED                (0)
#define ONU_SILENCE_ENABLED                 (1)

/* EPON DBA */
#define ONU_DBA_HW_RPRT_MODE                (0)
#define ONU_DBA_SW_RPRT_MODE                (1)
#define ONU_DBA_DEF_QUEUES_BIT_MAP          (0xFF)
#define ONU_DBA_DEF_NUM_OF_QUEUE            (8)
#define ONU_DBA_DEF_NUM_OF_QUEUESET         (1)
#define ONU_DBA_MAX_NUM_OF_QUEUE            (8)
#define ONU_DBA_MAX_NUM_OF_QUEUESET         (4)
#define ONU_DBA_DEF_QUEUESET_0_THRESHOLD    (0x2000)
#define ONU_DBA_DEF_QUEUESET_1_THRESHOLD    (0x4000)
#define ONU_DBA_DEF_QUEUESET_2_THRESHOLD    (0x6000)
#define ONU_DBA_DEF_QUEUESET_3_THRESHOLD    (0xFFFF)
#define ONU_DBA_DEF_QUEUESET_DEF_THRESHOLD  (0xFFFF)
#define ONU_DBA_UPDATE_LLID                 (2)
#define ONU_DBA_ACTIVE_LLID                 (1)
#define ONU_DBA_NON_ACTIVE_LLID             (0)
#define ONU_DBA_SW_RPRT_NORMAL_DIR          (0)
#define ONU_DBA_SW_RPRT_REVERSE_DIR         (1)

#define ONU_OAM_CTRL_MSG                    (0)
#define ONU_OAM_RPRT_MSG                    (1)
#define ONU_OAM_DG_MSG                      (2)

/* EPON FEC */
#define ONU_FEC_CFG_INIT                    (0)
#define ONU_FEC_CFG_RE_INIT                 (1)

/* Enums                              
------------------------------------------------------------------------------*/ 

/* Typedefs
------------------------------------------------------------------------------*/
typedef void (*EPONFUNCPTR) (MV_U8, MV_U8, MV_U8*);

/******************************************************************************/
/* ========================================================================== */
/*                               Database Definitions                         */
/* ========================================================================== */
/******************************************************************************/

/* ONU EPON General Tables */
typedef struct
{
  MV_U32 onuEponRegReqAutoRes;                          /* Register Request Auto Response */  
  MV_U32 onuEponRegAckAutoRes;                          /* Register Ack Auto Response  */
  MV_U32 onuEponReportAutoRes;                          /* Report Auto Response  */
  MV_U32 onuEponRxPcsFecEn;                             /* Rx PCS FEC Enable */
  MV_U32 onuEponTxPcsFecEn;                             /* Tx PCS FEC Enable */
  MV_U32 onuEponTxFecEn;                                /* Tx FEC Enable */
}S_OnuEponConfig;                                      
                                                       
typedef struct                                         
{                                                      
  MV_U32 addressLow;                                    /* MAC Address Low */  
  MV_U32 addressHigh;                                   /* MAC Address High */
}S_OnuEponMacAddr;

typedef struct  
{
    MV_U8  state;	    /* Is threshold configuration active for the queue */ 
    MV_U16 threshold;   /* Queue threshold: range 0 - 65535                */ 
}S_OnuEponDbaQueue; 

typedef struct                                         
{
  MV_U32            active;                          
  MV_U32            highestReportQueue;                          
  MV_U32            queuesBitMap;                                 
  MV_U32            numOfQueues;                                 
  MV_U32            numOfQueueSets;                              
  S_OnuEponDbaQueue queueThreshold[EPON_MAX_QUEUE_SET_NUMBER][EPON_MAX_THRESHOLD_SET_RER_QUEUE];   
}S_OnuEponDba;

typedef struct                                         
{
  MV_U32 reg;                          
  MV_U32 shift;                          
  MV_U32 shadowIdx;                              
}S_OnuEponDbaSwRprtRegMap;

typedef struct                                         
{
  MV_U32 reg;                          
  MV_U32 shadow;                              
}S_OnuEponDbaShadowRegMap;

typedef enum
{
    E_EPON_SLEEP_MODE_CTRL_NOTHING = 0,
    E_EPON_SLEEP_MODE_CTRL_TX      = 1,
    E_EPON_SLEEP_MODE_CTRL_TX_RX   = 2, 
} E_EponSleepCtrlMode;

typedef enum
{
    E_EPON_NOT_POWER_SAVING_STATUS    = 0, 
    E_EPON_POWER_SAVING_SLEEP_STATUS  = 1, 
    E_EPON_POWER_SAVING_WAKEUP_STATUS = 2, 
} E_EponPowerSavingStatus;

typedef struct                                         
{                                                      
  MV_U32           onuEponValid[EPON_MAX_MAC_NUM];      /* ONU Valid for sync */  
  MV_U32           onuEponState[EPON_MAX_MAC_NUM];      /* ONU State */  
  MV_U32           onuEponSignalDetect;             
  MV_U32           onuEponDsSyncOkPcs;             
  MV_U32           onuEponCtrlType;                     /* ONU Control Type */
  MV_U32           onuEponSyncTime[EPON_MAX_MAC_NUM];   /* ONU Sync Time */
  MV_U32           onuEponHoldoverState;             
  MV_U32           onuEponHoldoverTime;             
  MV_U32           onuEponHoldoverExecute;             
  MV_U32           onuEponSilenceState;             
  MV_U32           onuEponDbaMode;                      /* HW / SW Modes */
  MV_U32           onuEponDbaSwQueuesetDir;             /* SW Report Queueset Direction */
  MV_U32           onuEponDbaSwQueueDir;                /* SW Report Queue Direction */
  S_OnuEponDba     onuEponDba[EPON_MAX_MAC_NUM];
  S_OnuEponConfig  onuEponCfg;                          /* ONU Configuration */
  S_OnuEponMacAddr onuEponMacAddr[EPON_MAX_MAC_NUM];    /* ONU MAC Address */
  S_OnuEponMacAddr onuEponBcastAddr;                    /* ONU MAC Broadcast Address */
  STATUSNOTIFYFUNC onuStatusCallback;
  DYINGGASPFUNC    onuDgCallback;
  LINKSTATUSFUNC   onuLinkStatusCallback;
  MV_U32           onuEponMode;
  MV_U32           onuEponP2PForceMode;
  MV_U32           onuEponPcsCfg;             
  MV_U32           onuEponOverHead;             
  MV_U32           onuEponPhyOutput;             
  MV_U32           onuEponXvrBurstEnPolarity;
  MV_U32           onuEponOpticalLosTime;
  MV_U32           onuEponMacLosTime;
  MV_U32           onuEponPowerSavingWakeup;
  MV_U32           onuEponPowerSavingMaxSleepDuration;
  MV_U32           onuEponSleepAction;
  MV_U32           onuEponSleepMode;
  MV_U32           onuEponSleepDuration;
  MV_U32           onuEponWakeupDuration;
  MV_U32           onuEponPowerSavingStatus;
}S_OnuEponGenTbl;

/* ONU EPON Data Path tables */
typedef struct
{
  MV_U32 slowFrameToCtrlQueue;                     
  MV_U32 slowFrameToRprtQueue;                     
  MV_U32 rprtFrameToRprtQueue;                     
  MV_U32 rprtFrameToDataQueue;                     
  MV_U32 ctrlFrameToCtrlQueue;                     
  MV_U32 ctrlFrameToDataQueue;                     
}S_OnuEponRxPktForward;

typedef struct
{
  MV_U32 dropLlid1NNN;
  MV_U32 forwardLlidAll;                     
  MV_U32 forwardLlid1XXX;                     
  MV_U32 forwardLlid1FFF;                     
  MV_U32 forwardLlid0FFF;                     
  MV_U32 ignoreLenErr;                   
  MV_U32 ignoreGmiiErr;                    
  MV_U32 ignoreFcsErr;                    
  MV_U32 ignoreLlidCrcErr;                    
}S_OnuEponRxPktFilter;

typedef struct
{
  MV_U32                onuEponRxPktMinSize;             /* ONU Rx Packet Min Size */
  MV_U32                onuEponRxPktMaxSize;             /* ONU Rx Packet Max Size */
  S_OnuEponRxPktForward onuEponPktForward;               /* ONU Rx Packet Forward */
  S_OnuEponRxPktFilter  onuEponPktFilter;                /* ONU Rx Packet Filter */ 
  MV_U32                onuEponRxLLID[EPON_MAX_MAC_NUM]; /* ONU Rx Packet Rx LLID Array */
  MV_U32                onuEponTxLLID[EPON_MAX_MAC_NUM]; /* ONU Rx Packet Tx LLID Array */
}S_OnuEponDatapathTbl;                                     

/* ONU EPON DataBase */
typedef struct
{
  S_OnuEponGenTbl      onuEponGenTbl_s;
  S_OnuEponDatapathTbl onuEponDataPathTbl_s;
}S_OnuEponDb;

/* Global variables
------------------------------------------------------------------------------*/
/* ONU EPON Database */
extern S_OnuEponDb onuEponDb_s;

/* Global functions
------------------------------------------------------------------------------*/
/* ONU EPON database init function */
MV_STATUS onuEponDbInit(void);
MV_STATUS onuEponDbReInit(MV_U32 macId);

/* ONU EPON general table API functions */
MV_STATUS onuEponDbOnuValidSet(MV_BOOL onuValid, MV_U32 macId);
MV_BOOL   onuEponDbOnuValidGet(MV_U32 macId);
MV_STATUS onuEponDbOnuStateSet(MV_U32 onuState, MV_U32 macId);
MV_U32    onuEponDbOnuStateGet(MV_U32 macId);
MV_STATUS onuEponDbOnuSignalDetectSet(MV_U32 onuState);
MV_U32    onuEponDbOnuSignalDetectGet(void);
MV_STATUS onuEponDbOnuDsSyncOkPcsSet(MV_U32 onuState);
MV_U32    onuEponDbOnuDsSyncOkPcsGet(void);
MV_STATUS onuEponDbOnuCtrlTypeSet(MV_U32 ctrlType);
MV_U32    onuEponDbOnuCtrlTypeGet(void);
MV_STATUS onuEponDbOnuSyncTimeSet(MV_U32 syncTime, MV_U32 macId);
MV_U32    onuEponDbOnuSyncTimeGet(MV_U32 macId);
MV_STATUS onuEponDbOnuMacAddrSet(MV_U32 lowAddr, MV_U32 highAddr, MV_U32 macId);
MV_STATUS onuEponDbOnuMacAddrGet(MV_U32 *lowAddr, MV_U32 *highAddr, MV_U32 macId);
MV_STATUS onuEponDbOnuBcastAddrSet(MV_U32 lowAddr, MV_U32 highAddr);
MV_STATUS onuEponDbOnuBcastAddrGet(MV_U32 *lowAddr, MV_U32 *highAddr);
MV_STATUS onuEponDbOnuCfgSet(MV_U32 regReqAutoRes, MV_U32 regAckAutoRes, MV_U32 reportAutoRes, 
                             MV_U32 rxPcsFecEn, MV_U32 txPcsFecEn, MV_U32 txFecEn);
MV_STATUS onuEponDbOnuCfgGet(MV_U32 *regReqAutoRes, MV_U32 *regAckAutoRes, MV_U32 *reportAutoRes, 
                             MV_U32 *rxPcsFecEn, MV_U32 *txPcsFecEn, MV_U32 *txFecEn);
MV_STATUS onuEponDbOnuCfgAutoSet(MV_U32 regReqAutoRes, MV_U32 regAckAutoRes, MV_U32 reportAutoRes);
MV_STATUS onuEponDbOnuRxFecCfgSet(MV_U32 rxPcsFecEn);
MV_STATUS onuEponDbOnuRxFecCfgGet(MV_U32 *rxPcsFecEn);
MV_STATUS onuEponDbOnuTxFecCfgGet(MV_U32 *txFecEn, MV_U32 macId);
MV_STATUS onuEponDbOnuDbaHighestReportQueueSet(MV_U32 queueNum, MV_U32 macId);
MV_U32    onuEponDbOnuDbaHighestReportQueueGet(MV_U32 macId);
MV_STATUS onuEponDbOnuDbaQueuesBitMapSet(MV_U32 queuesBitMap, MV_U32 macId);
MV_U32    onuEponDbOnuDbaQueuesBitMapGet(MV_U32 macId);
MV_STATUS onuEponDbOnuDbaNumOfQueuesSet(MV_U32 numOfQueues, MV_U32 macId);
MV_U32    onuEponDbOnuDbaNumOfQueuesGet(MV_U32 macId);
MV_STATUS onuEponDbOnuDbaNumOfQueuesetSet(MV_U32 numOfQueueset, MV_U32 macId);
MV_U32    onuEponDbOnuDbaNumOfQueuesetGet(MV_U32 macId);
MV_STATUS onuEponDbOnuDbaQueueThresholdSet(MV_U32 threshold, MV_U32 state, MV_U32 queueId, 
										   MV_U32 queueSet, MV_U32 macId);
MV_STATUS onuEponDbOnuDbaQueueThresholdGet(MV_U32 *threshold, MV_U32 *state, MV_U32 queueId, 
										   MV_U32 queueSet, MV_U32 macId);
MV_STATUS onuEponDbOnuDbaLlidThresholdGet(S_OnuEponDba *llidDba, MV_U32 macId);
MV_STATUS onuEponDbOnuDbaLlidActiveStatusSet(MV_U32 status, MV_U32 macId);
MV_U32    onuEponDbOnuDbaLlidActiveStatusGet(MV_U32 macId);
MV_STATUS onuEponDbOnuDbaModeSet(MV_U32 mode);
MV_U32    onuEponDbOnuDbaModeGet(void);
MV_STATUS onuEponDbOnuDbaSwModeQueuesetDirSet(MV_U32 dir);
MV_U32    onuEponDbOnuDbaSwModeQueuesetDirGet(void);
MV_STATUS onuEponDbOnuDbaSwModeQueueDirSet(MV_U32 dir);
MV_U32    onuEponDbOnuDbaSwModeQueueDirGet(void);
MV_STATUS onuEponDbOnuHoldoverStateSet(MV_U32 state);
MV_U32    onuEponDbOnuHoldoverStateGet(void);
MV_STATUS onuEponDbOnuHoldoverTimeSet(MV_U32 time);
MV_U32    onuEponDbOnuHoldoverTimeGet(void);
MV_STATUS onuEponDbOnuHoldoverExecSet(MV_U32 state);
MV_U32    onuEponDbOnuHoldoverExecGet(void);
MV_STATUS onuEponDbOnuOpticalLosTimeSet(MV_U16 losTime);
MV_U32    onuEponDbOnuOpticalLosTimeGet(void);
MV_STATUS onuEponDbOnuMacLosTimeSet(MV_U16 losTime);
MV_U32    onuEponDbOnuMacLosTimeGet(void);
MV_STATUS onuEponDbOnuPowerSavingWakeupSet(MV_U8 wakeUpEnable);
MV_U32    onuEponDbOnuPowerSavingWakeupGet(void);
MV_STATUS onuEponDbOnuPowerSavingMaxSleepDurationSet(MV_U64 maxSleepDuration);
MV_U32    onuEponDbOnuPowerSavingMaxSleepDurationGet(void);
MV_STATUS onuEponDbOnuSleepActionSet(MV_U8 action);
MV_U32    onuEponDbOnuSleepActionGet(void);
MV_STATUS onuEponDbOnuSleepModeSet(MV_U8 mode);
MV_U32    onuEponDbOnuSleepModeGet(void);
MV_STATUS onuEponDbOnuSleepDurationSet(MV_U32 sleepDuration);
MV_U32    onuEponDbOnuSleepDurationGet(void);
MV_STATUS onuEponDbOnuWakeupDurationSet(MV_U32 wakeupDuration);
MV_U32    onuEponDbOnuWakeupDurationGet(void);
MV_STATUS onuEponDbOnuSleepWakeupStatusSet(MV_U32 status);
MV_U32    onuEponDbOnuSleepWakeupStatusGet(void);
MV_STATUS onuEponDbOnuSilenceStateSet(MV_U32 state);
MV_U32    onuEponDbOnuSilenceStateGet(void);
MV_STATUS onuEponDbOnuSwRprtTimerTypeSet(MV_U32 timerType);
MV_U32    onuEponDbOnuSwRprtTimerTypeGet(void);
MV_STATUS onuEponDbOnuSwRprtMacTimerIntervalSet(MV_U32 timerInterval);
MV_U32    onuEponDbOnuSwRprtMacTimerIntervalGet(void);


/* ONU EPON Data Path table API functions */
MV_STATUS onuEponDbPktSizeSet(MV_U32 minSize, MV_U32 maxSize);
MV_STATUS onuEponDbPktSizeGet(MV_U32 *minSize, MV_U32 *maxSize);
MV_STATUS onuEponDbPktFilterPacketSet(MV_U32 forwardLlidAll, MV_U32 forwardLlid1XXX, 
                                      MV_U32 forwardLlid1FFF, MV_U32 forwardLlid0FFF, MV_U32 dropLlid1NNN);
MV_STATUS onuEponDbPktFilterPacketGet(MV_U32 *forwardLlidAll, MV_U32 *forwardLlid1XXX, 
                                      MV_U32 *forwardLlid1FFF, MV_U32 *forwardLlid0FFF, MV_U32 *dropLlid1NNN);
MV_STATUS onuEponDbPktFilterErrorSet(MV_U32 ignoreLenErr, MV_U32 ignoreGmiiErr, 
                                     MV_U32 ignoreFcsErr, MV_U32 ignoreLlidCrcErr);
MV_STATUS onuEponDbPktFilterErrorGet(MV_U32 *ignoreLenErr, MV_U32 *ignoreGmiiErr, 
                                     MV_U32 *ignoreFcsErr, MV_U32 *ignoreLlidCrcErr);
MV_STATUS onuEponDbPktForwardSet(MV_U32 slowFrameToCtrlQueue, MV_U32 slowFrameToRprtQueue,
                                 MV_U32 rprtFrameToRprtQueue, MV_U32 rprtFrameToDataQueue,
                                 MV_U32 ctrlFrameToCtrlQueue, MV_U32 ctrlFrameToDataQueue);
MV_STATUS onuEponDbPktForwardGet(MV_U32 *slowFrameToCtrlQueue, MV_U32 *slowFrameToRprtQueue,
                                 MV_U32 *rprtFrameToRprtQueue, MV_U32 *rprtFrameToDataQueue,
                                 MV_U32 *ctrlFrameToCtrlQueue, MV_U32 *ctrlFrameToDataQueue);
MV_STATUS onuEponDbPktRxLlidSet(MV_U32 llid, MV_U32 index);
MV_STATUS onuEponDbPktRxLlidGet(MV_U32 *llid, MV_U32 index);
MV_STATUS onuEponDbPktTxLlidSet(MV_U32 llid, MV_U32 index);
MV_STATUS onuEponDbPktTxLlidGet(MV_U32 *llid, MV_U32 index);

MV_STATUS        onuEponDbStatusNotifySet(STATUSNOTIFYFUNC statusCallback);
STATUSNOTIFYFUNC onuEponDbStatusNotifyGet(void);

MV_STATUS        onuEponDbDgCallbackSet(DYINGGASPFUNC dgCallback);
DYINGGASPFUNC    onuEponDbDgCallbackGet(void);

MV_STATUS        onuEponDbLinkStatusCallbackSet(LINKSTATUSFUNC linkStatusCallback);
LINKSTATUSFUNC   onuEponDbLinkStatusCallbackGet(void);

MV_STATUS matchDestAddressToMacId(MV_U8 *destAddr, MV_U32 *macId);

MV_STATUS   onuEponDbModeSet(MV_U32 mode);
MV_U32      onuEponDbModeGet(void);
MV_STATUS   onuEponDbP2PForceModeSet(MV_U32 mode);
MV_U32      onuEponDbP2PForceModeGet(void);
MV_STATUS   onuEponDbPcsCfgSet(MV_U32 val);
MV_U32      onuEponDbPcsCfgGet(void);
MV_STATUS   onuEponDbOverheadSet(MV_U32 val);
MV_U32      onuEponDbOverheadGet(void);
MV_STATUS   onuEponDbBurstEnablePolaritySet(MV_U32 val);
MV_U32      onuEponDbBurstEnablePolarityGet(void);

/* Macros
------------------------------------------------------------------------------*/    

#endif /* _ONU_EPON_DB_H */

 



