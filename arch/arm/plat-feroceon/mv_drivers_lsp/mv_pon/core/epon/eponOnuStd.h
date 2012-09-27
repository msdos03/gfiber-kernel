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
/**  FILE        : eponOnustd.h                                              **/
/**                                                                          **/
/**  DESCRIPTION : This file contains ONU EPON standard definitions          **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *                              
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *  
 * ========================================================================== *      
 *                                                                          
 ******************************************************************************/
#ifndef _ONU_EPON_STD_H
#define _ONU_EPON_STD_H

/* Include Files
------------------------------------------------------------------------------*/
 
/* Definitions
------------------------------------------------------------------------------*/ 
#define MIN_ETH_FRAME_LEN  (256)
#define MIN_RPRT_FRAME_LEN (64)
#define MIN_DG_FRAME_LEN   (64)
#define MAC_ADDR_LEN       (6)
/* Enums                              
------------------------------------------------------------------------------*/ 
typedef enum
{
  MPC_OPCODE_GATE          = 2,     
  MPC_OPCODE_REPORT        = 3,       
  MPC_OPCODE_REGISTER_REQ  = 4,
  MPC_OPCODE_REGISTER      = 5, 
  MPC_OPCODE_REGISTER_ACK  = 6
}E_EponMpcOpcode;

typedef enum
{
  REGISTER_FLAGS_REREG     = 1,     
  REGISTER_FLAGS_DEREG     = 2,
  REGISTER_FLAGS_ACK       = 3,            
  REGISTER_FLAGS_NACK      = 4
}E_EponMpcRegFlag;

typedef enum
{
  REGISTER_ACK_FLAGS_NACK  = 0,     
  REGISTER_ACK_FLAGS_ACK   = 1
}E_EponMpcRegAckFlag;

typedef enum
{
  REGISTER_REQ_FLAGS_REG   = 1,     
  REGISTER_REQ_FLAGS_DEREG = 3
}E_EponMpcRegReqFlag;

/* Typedefs
------------------------------------------------------------------------------*/
#pragma pack(1)
typedef struct
{
  MV_U32 length;
  MV_U8  data[MIN_ETH_FRAME_LEN];
}S_OnuEponCtrlBuffer;
#pragma pack(0)

/******************************************************************************/
/* ========================================================================== */
/*                         General MPC Frame Definitions                      */
/* ========================================================================== */
/******************************************************************************/
#pragma pack(1)
typedef struct
{
  MV_U8  destAddr[MAC_ADDR_LEN];                      
  MV_U8  srcAddr[MAC_ADDR_LEN];                       
  MV_U16 etherType;                        
}S_OnuEponStdEthFrame;                                      
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  MV_U16 opCode;
  MV_U32 timeStamp;
}S_OnuEponGenMpcPdu;                                      
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  S_OnuEponStdEthFrame stdEthFrame;
  S_OnuEponGenMpcPdu   genMpcPdu;  
}S_OnuEponGenMpcFrame;                                      
#pragma pack(0)

/******************************************************************************/
/* ========================================================================== */
/*                         Register MPC Frame Definitions                     */
/* ========================================================================== */
/******************************************************************************/
#pragma pack(1)
typedef struct 
{
  MV_U16 assignedLlidPort;
  MV_U8  flags;              
  MV_U16 syncTime;           
  MV_U8  echoedPendingGrants;
}S_OnuEponRegMpcPdu;                                      
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  S_OnuEponStdEthFrame stdEthFrame;
  S_OnuEponGenMpcPdu   genMpcPdu;
  S_OnuEponRegMpcPdu   regMpcPdu;
}S_OnuEponRegMpcFrame;                                      
#pragma pack(0)

/******************************************************************************/
/* ========================================================================== */
/*                         Register Ack MPC Frame Definitions                 */
/* ========================================================================== */
/******************************************************************************/
#pragma pack(1)
typedef struct 
{
  MV_U8  flags;              
  MV_U16 echoedAssignedport;  
  MV_U16 echoedSyncTime;      
}S_OnuEponRegAckMpcPdu;                                      
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  S_OnuEponStdEthFrame  stdEthFrame;
  S_OnuEponGenMpcPdu    genMpcPdu;
  S_OnuEponRegAckMpcPdu regAckMpcPdu;
}S_OnuEponRegAckMpcFrame;                                      
#pragma pack(0)

/******************************************************************************/
/* ========================================================================== */
/*                         Register Request MPC Frame Definitions             */
/* ========================================================================== */
/******************************************************************************/
#pragma pack(1)
typedef struct 
{
  MV_U8  flags;              
  MV_U8  pendingGrants;  
}S_OnuEponRegReqMpcPdu;                                      
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  S_OnuEponStdEthFrame  stdEthFrame;
  S_OnuEponGenMpcPdu    genMpcPdu;
  S_OnuEponRegReqMpcPdu regReqMpcPdu;
}S_OnuEponRegReqMpcFrame;     
#pragma pack(0)

/******************************************************************************/
/* ========================================================================== */
/*                         Report MPC Frame Definitions                       */
/* ========================================================================== */
/******************************************************************************/
#pragma pack(1)
typedef struct 
{
  MV_U8  numQueueSets;
}S_OnuEponReportMpcPduStart;
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  MV_U16 queueLength;
}S_OnuEponReportQueueReport;
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  MV_U8                      bitMap;
  S_OnuEponReportQueueReport queueReport;
}S_OnuEponReportQueueSetStart;
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  S_OnuEponStdEthFrame        stdEthFrame;
  S_OnuEponGenMpcPdu          genMpcPdu;
  S_OnuEponReportMpcPduStart  reportMpcPduStart;
  MV_U8                       next;
}S_OnuEponReportMpcFrame;
#pragma pack(0)

/******************************************************************************/
/* ========================================================================== */
/*                                OAM Frame Definitions                       */
/* ========================================================================== */
/******************************************************************************/

#pragma pack(1)
typedef struct 
{
  MV_U8  subType;
  MV_U16 flags;
  MV_U8  code;
}S_OnuEponOamHeader;
#pragma pack(0)

#pragma pack(1)
typedef struct 
{
  S_OnuEponStdEthFrame        stdEthFrame;
  S_OnuEponOamHeader          stdOamHeader;
}S_OnuEponOamFrame;
#pragma pack(0)

#pragma pack(1)
typedef struct                                         
{                                                      
  S_OnuEponCtrlBuffer msg;
  MV_U32              macId;
  MV_U32              valid;
}S_OnuEponOamMsg;
#pragma pack(0)

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Macros
------------------------------------------------------------------------------*/    

#endif /* _ONU_EPON_STD_H */

