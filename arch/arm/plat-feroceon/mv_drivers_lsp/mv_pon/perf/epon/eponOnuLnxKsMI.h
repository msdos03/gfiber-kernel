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
**  FILE        : eponOnuLnxKsMI.h                                           **
**                                                                           **
**  DESCRIPTION : This file contains ONU GPON Management Interface           **
*******************************************************************************
*                                                                             *                              
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *  
* =========================================================================== *      
******************************************************************************/
#ifndef _ONU_EPON_LINUX_KS_MNG_INTERFACE_H
#define _ONU_EPON_LINUX_KS_MNG_INTERFACE_H

/* Include Files
------------------------------------------------------------------------------*/
 
/* Definitions
------------------------------------------------------------------------------*/ 
#define MVEPON_IOCTL_INIT          _IOW(MVPON_IOCTL_MAGIC, 1, unsigned int)
#define MVEPON_IOCTL_FEC_CONFIG    _IOW(MVPON_IOCTL_MAGIC, 2, unsigned int)
#define MVEPON_IOCTL_ENC_CONFIG    _IOW(MVPON_IOCTL_MAGIC, 3, unsigned int)
#define MVEPON_IOCTL_ENC_KEY       _IOW(MVPON_IOCTL_MAGIC, 4, unsigned int)
#define MVEPON_IOCTL_INFO          _IOR(MVPON_IOCTL_MAGIC, 5, unsigned int)
#define MVEPON_IOCTL_PM            _IOR(MVPON_IOCTL_MAGIC, 6, unsigned int)
#define MVEPON_IOCTL_OAM_TX        _IOW(MVPON_IOCTL_MAGIC, 7, unsigned int)
#define MVEPON_IOCTL_DBA_CFG       _IOW(MVPON_IOCTL_MAGIC, 8, unsigned int)
#define MVEPON_IOCTL_DBA_RPRT      _IOR(MVPON_IOCTL_MAGIC, 8, unsigned int)
#define MVEPON_IOCTL_HOLDOVER_CFG  _IOW(MVPON_IOCTL_MAGIC, 9, unsigned int)
#define MVEPON_IOCTL_HOLDOVER_RPRT _IOW(MVPON_IOCTL_MAGIC, 10, unsigned int)
#define MVEPON_IOCTL_SILENCE       _IOW(MVPON_IOCTL_MAGIC, 11, unsigned int)
#define MVEPON_IOCTL_P2P_SET       _IOW(MVPON_IOCTL_MAGIC, 12, unsigned int)
#define MVEPON_IOCTL_TDM_QUE_CFG   _IOW(MVPON_IOCTL_MAGIC, 13, unsigned int)
#define MVEPON_IOCTL_ALARM_GET     _IOR(MVPON_IOCTL_MAGIC, 14, unsigned int)
#define MVEPON_IOCTL_ROGUE_ONU_SET _IOW(MVPON_IOCTL_MAGIC, 15, unsigned int)

#define EPON_MAX_NUM_OF_MAC          (8)
#define EPON_MAX_NUM_OF_QUEUE        (8)
#define EPON_MAC_LEN                 (6)

#define EPON_API_MAX_OAM_FRAME_SIZE      (512)
/*db threshold set*/
#define EPON_MIN_QUEUE_SET_NUMBER        (2)
#define EPON_MAX_QUEUE_SET_NUMBER        (4)
#define EPON_MAX_THRESHOLD_SET_RER_QUEUE (8)
#define EPON_MAX_QUEUE                   (8)

/* Enums                              
------------------------------------------------------------------------------*/ 
typedef enum
{
  E_EPON_IOCTL_PM_RX  = 1,
  E_EPON_IOCTL_PM_TX  = 2,
  E_EPON_IOCTL_PM_SW  = 3,
  E_EPON_IOCTL_PM_GPM = 4
}E_EponIoctlPmSection;

typedef enum
{
  E_EPON_IOCTL_MPCP_TX_FRAME_CNT             = 0,
  E_EPON_IOCTL_MPCP_TX_ERROR_FRAME_CNT       = 1,
  E_EPON_IOCTL_MAX_TX_SW_CNT
}E_EponIoctlTxSwCnt;

typedef enum
{
  E_EPON_IOCTL_MPCP_RX_FRAME_CNT             = 0,
  E_EPON_IOCTL_MPCP_RX_ERROR_FRAME_CNT       = 1,
  E_EPON_IOCTL_MPCP_REGISTER_ACK_CNT         = 2,
  E_EPON_IOCTL_MPCP_REGISTER_NACK_CNT        = 3,
  E_EPON_IOCTL_MPCP_REGISTER_DEREG_FRAME_CNT = 4,
  E_EPON_IOCTL_MPCP_REGISTER_REREG_FRAME_CNT = 5,
  E_EPON_IOCTL_MAX_RX_SW_CNT
}E_EponIoctlRxSwCnt;

typedef enum
{
  E_EPON_IOCTL_STD_MODE,
  E_EPON_IOCTL_P2P_MODE,
  E_EPON_IOCTL_MAX_MODE_NUM
}E_EponIoctlMode;

/* Typedefs
------------------------------------------------------------------------------*/
/* TX Counters */
typedef struct
{
  MV_U32 ctrlRegReqFramesCnt; /* Count number of register request frames transmitted     */  
  MV_U32 ctrlRegAckFramesCnt; /* Count number of register acknowledge frames transmitted */  
  MV_U32 reportFramesCnt;     /* Count number of report frames transmitted               */
  MV_U32 dataFramesCnt;       /* Count number of data frames transmitted                 */
  MV_U32 txAllowedBytesCnt;   /* Count number of Tx Byte Allow counter                   */
}S_EponIoctlTxPm;

/* RX Counters */
typedef struct
{
  MV_U32 fcsErrorFramesCnt;   /* Count number of received frames with FCS errors */
  MV_U32 shortFramesCnt;      /* Count number of short frames received           */
  MV_U32 longFramesCnt;       /* Count number of long frames received            */
  MV_U32 dataFramesCnt;       /* Count number of data frames received            */
  MV_U32 ctrlFramesCnt;       /* Count number of control frames received         */
  MV_U32 reportFramesCnt;     /* Count number of report frames received          */
  MV_U32 gateFramesCnt;       /* Count number of gate frames received            */
}S_EponIoctlRxPm;

/* SW Counters */
typedef struct
{
  MV_U32 swTxCnt[E_EPON_IOCTL_MAX_TX_SW_CNT];
  MV_U32 swRxCnt[E_EPON_IOCTL_MAX_RX_SW_CNT]; 
}S_EponIoctlSwPm;

/* GEM Counters */
typedef struct
{
  MV_U32 grantValidCnt;                 /* Count number of valid grant                          */
  MV_U32 grantMaxFutureTimeErrorCnt;    /* Count number of grant max future time error          */
  MV_U32 minProcTimeErrorCnt;           /* Count number of min proc time error                  */
  MV_U32 lengthErrorCnt;                /* Count number of length error                         */
  MV_U32 discoveryAndRegCnt;            /* Count number of discovery & register                 */
  MV_U32 fifoFullErrorCnt;              /* Count number of fifo full error                      */
  MV_U32 opcDiscoveryNotRegBcastCnt;    /* Count number of opc discoveryNotRegBcastCnt          */
  MV_U32 opcRegisterNotDiscoveryCnt;    /* Count number of opc register not discovery           */ 
  MV_U32 opcDiscoveryNotRegNotBcastCnt; /* Count number of opc discovery not register not bcast */ 
  MV_U32 opcDropGrantCnt;               /* Count number of opc drop grant                       */ 
  MV_U32 opcHiddenGrantCnt;             /* Count number of opc hidden grant                     */ 
  MV_U32 opcBackToBackCnt;              /* Count number of opc back to back                     */ 
}S_EponIoctlGpmPm;                               

/* PM Counters */
typedef struct
{
  MV_U32 macId;
  MV_U32 section;
  union
  {
    S_EponIoctlRxPm  rxCnt;
    S_EponIoctlTxPm  txCnt;
    S_EponIoctlSwPm  swCnt;
    S_EponIoctlGpmPm gpmCnt;
  };
}S_EponIoctlPm;

/* INFO */
typedef struct
{
  MV_U32 macId;
  MV_U32 onuEponState;                   /* ONU State                   */  
  MV_U32 onuEponSignalDetect;            /* ONU Signal Detect           */  
  MV_U32 onuEponDsSyncOkPcs;             /* ONU DS Sync OK - PCS        */  
  MV_U32 onuEponCtrlType;                /* ONU Control Type            */
  MV_U8  onuEponMacAddr[EPON_MAC_LEN];   /* ONU MAC Address             */
  MV_U8  onuEponBcastAddr[EPON_MAC_LEN]; /* ONU MAC Broadcast Address   */
  MV_U32 onuEponRxLLID;                  /* ONU Rx Packet Rx LLID Array */
  MV_U32 onuEponTxLLID;                  /* ONU Rx Packet Tx LLID Array */
  MV_U32 onuEponMode;                    /* ONU mode (EPON, P2P)        */  
}S_EponIoctlInfo;

/* Init */
typedef struct
{
  MV_U32 xvrPolarity;
  MV_U32 dgPolarity;
  MV_U8  macAddr[EPON_MAX_NUM_OF_MAC][EPON_MAC_LEN];
}S_EponIoctlInit;

/* FEC */
typedef struct
{
  MV_U32 rxGenFecEn;
  MV_U32 txGenFecEn;
  MV_U32 txMacFecEn[EPON_MAX_NUM_OF_MAC];
}S_EponIoctlFec;

/* Encryption */
typedef struct
{
  MV_U32 macId;
  MV_U32 encEnable;
  MV_U32 encKey;
  MV_U32 encKeyIndex;
}S_EponIoctlEnc;

/* OAM Transmission */
typedef struct
{
  MV_U32 length;
  MV_U8  data[EPON_API_MAX_OAM_FRAME_SIZE];
}S_EponOamFrame;

typedef struct
{
  MV_U32         macId;
  S_EponOamFrame oamFrame;
}S_EponIoctlOamTx;

/* DBA */
typedef struct  
{
    MV_U8  state;	    /* Is threshold configuration active for the queue */ 
    MV_U16 threshold;   /* Queue threshold: range 0 - 65535                */ 
}S_EponDbaThreshold; 

typedef struct
{
  MV_U32             numOfQueueSets;
  MV_U32             numOfQueues;
  MV_U32             aggrThreshold;
  S_EponDbaThreshold threshold[EPON_MAX_QUEUE_SET_NUMBER][EPON_MAX_THRESHOLD_SET_RER_QUEUE];
}S_EponIoctlDbaLlid;

typedef struct 
{
  MV_U8              validLlid[EPON_MAX_NUM_OF_MAC];
  S_EponIoctlDbaLlid dbaLlid[EPON_MAX_NUM_OF_MAC];
}S_EponIoctlDba; 

/* Holdover */
typedef struct
{
  MV_U32 holdoverState; /* 0 - Deactivate, 1 - Activate */ 
  MV_U32 holdoverTime;
}S_EponIoctlHoldOver;

/* Silence */
typedef struct
{
  MV_U32 silenceState; /* 0 - Deactivate, 1 - Activate */ 
}S_EponIoctlSilence;

typedef struct
{
  uint32_t tdmQueCnt[EPON_MAX_NUM_OF_MAC][EPON_MAX_NUM_OF_QUEUE];
}S_EponIoctlTdmQueue;

/* Rogue ONU */
typedef struct
{
  uint32_t macId;
  uint32_t enable;
}S_EponIoctlRogueOnu;

typedef struct 
{
  S_EponIoctlInit     init;
  S_EponIoctlInfo     e_info;
  S_EponIoctlPm       e_pm;
  S_EponIoctlFec      fec;
  S_EponIoctlEnc      enc;
  S_EponIoctlDba      dba;
  S_EponIoctlHoldOver holdover;
  S_EponIoctlSilence  silence;
  S_EponIoctlTdmQueue tdmQueue;
  S_EponIoctlRogueOnu rogueOnu;
  struct cdev         cdev;
}S_EponModuleCdev;

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

MV_STATUS onuEponMngInterfaceCreate(void);
MV_STATUS onuEponMngInterfaceRelease(void);
void      onuEponMiNotifyCallback(MV_U32 onuState);

MV_STATUS mvP2PStart(void);
MV_STATUS mvP2PStop(void);
/* Macros
------------------------------------------------------------------------------*/    

#endif /* _ONU_EPON_LINUX_KS_MNG_INTERFACE_H */

  

