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
**  FILE        : gponOnuLnxKsMI.h                                           **
**                                                                           **
**  DESCRIPTION : This file contains ONU GPON Management Interface           **
*******************************************************************************
*                                                                             *                              
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   29Oct06  Oren Ben Hayun   created                                         *  
* =========================================================================== *      
******************************************************************************/
#ifndef _ONU_GPON_LINUX_KS_MNG_INTERFACE_H
#define _ONU_GPON_LINUX_KS_MNG_INTERFACE_H

/* Include Files
------------------------------------------------------------------------------*/
 
/* Definitions
------------------------------------------------------------------------------*/ 
#define MVGPON_IOCTL_INIT               _IOW(MVPON_IOCTL_MAGIC, 1,  unsigned int)
#define MVGPON_IOCTL_BEN_INIT           _IOW(MVPON_IOCTL_MAGIC, 2,  unsigned int)
#define MVGPON_IOCTL_DATA_TCONT_CONFIG	_IOW(MVPON_IOCTL_MAGIC, 3,  unsigned int)
#define MVGPON_IOCTL_DATA_TCONT_RESET	 _IO(MVPON_IOCTL_MAGIC, 4)
#define MVGPON_IOCTL_INFO               _IOR(MVPON_IOCTL_MAGIC, 5,  unsigned int)
#define MVGPON_IOCTL_ALARM              _IOR(MVPON_IOCTL_MAGIC, 6,  unsigned int)
#define MVGPON_IOCTL_PM                 _IOR(MVPON_IOCTL_MAGIC, 7,  unsigned int)
#define MVGPON_IOCTL_GEM                _IOW(MVPON_IOCTL_MAGIC, 8,  unsigned int)
#define MVGPON_IOCTL_GEMPORT_PM_CONFIG  _IOW(MVPON_IOCTL_MAGIC, 9,  unsigned int)
#define MVGPON_IOCTL_GEMPORT_PM_GET     _IOR(MVPON_IOCTL_MAGIC, 10, unsigned int)
#define MVGPON_IOCTL_GEMPORT_PM_RESET   _IOW(MVPON_IOCTL_MAGIC, 11, unsigned int)
#define MVGPON_IOCTL_GEMPORT_STATE_GET  _IOW(MVPON_IOCTL_MAGIC, 12, unsigned int)
#define MVGPON_IOCTL_GEMPORT_STATE_SET  _IOW(MVPON_IOCTL_MAGIC, 13, unsigned int)

#define ONU_GPON_DEBUG_STATE        (0xFF)

/* Enums                              
------------------------------------------------------------------------------*/ 
typedef enum
{
  E_GPON_IOCTL_PM_PLOAM_RX = 1,
  E_GPON_IOCTL_PM_PLOAM_TX = 2,
  E_GPON_IOCTL_PM_BW_MAP   = 3,
  E_GPON_IOCTL_PM_FEC      = 4,
  E_GPON_IOCTL_PM_GEM_RX   = 5,
  E_GPON_IOCTL_PM_GEM_TX   = 6
}E_GponIoctlPmSection;

typedef enum
{
  E_GEPON_IOCTL_GEM_ADD         = 1,
  E_GEPON_IOCTL_GEM_REMOVE      = 2,
  E_GEPON_IOCTL_GEM_CLEARALL    = 3
}E_GponIoctlGemAction;

typedef enum
{
  GEMPORTPMCMD_STOP  = 0, 
  GEMPORTPMCMD_START = 1
}E_GponIoctlGemPortCmd;

/* Typedefs
------------------------------------------------------------------------------*/
typedef struct                                                                                                                                
{                                                                                                                                             
  MV_U32 rxIdlePloam;                                                                                                                       
  MV_U32 rxCrcErrorPloam;                                                                                                                   
  MV_U32 rxFifoOverErrorPloam;                                                                                                              
  MV_U32 rxBroadcastPloam;                                                                                                                  
  MV_U32 rxOnuIdPloam;                                                                                                                      
  MV_U32 rxMsgIdPloam[ONU_GPON_DS_MSG_LAST+1];                                                                                              
  MV_U32 rxMsgTotalPloam;                                                                                                                   
}S_GponIoctlPloamRxPm;                                                                                                                            

typedef struct                                                                                                                                
{                                                                                                                                             
  MV_U32 txErrMsgIdPloam[ONU_GPON_US_MSG_LAST+1];                                                                                           
  MV_U32 txMsgIdPloam[ONU_GPON_US_MSG_LAST+1];                                                                                                                                  
  MV_U32 txMsgTotalPloam;                                                                    
}S_GponIoctlPloamTxPm;                                                                             
                                                                                               
typedef struct                                                                                 
{                                                                                              
  MV_U32 allocCrcErr;                                                                        
  MV_U32 allocCorrectableCrcErr;                                                                                                                                                  
  MV_U32 allocUnCorrectableCrcErr;                                                                                                             
  MV_U32 allocCorrec;                                                                                                                          
  MV_U32 totalReceivedAllocBytes;                                                                                                              
}S_GponIoctlBwMapPm;                                                                                                                                 
                                                                                                                                                 
typedef struct                                                                                                                                   
{                                                                                                          
  MV_U32 receivedBytes;                                                                                  
  MV_U32 correctedBytes;                                                                                                                    
  MV_U32 correctedBits;                                                                                                                     
  MV_U32 receivedCodeWords;                                                                                                                 
  MV_U32 uncorrectedCodeWords;                                                                                                              
}S_GponIoctlFecPm;                                                                                                                                
                                                                                                                                              
typedef struct                                                                                                                                
{                                                                                                                                             
  MV_U32 gemRxIdleGemFrames;                                                                                
  MV_U32 gemRxValidGemFrames;                                                                               
  MV_U32 gemRxUndefinedGemFrames;                                                                                                                 
  MV_U32 gemRxOmciFrames;                                                                                                                         
  MV_U32 gemRxDroppedGemFrames;                                                                                                                   
  MV_U32 gemRxDroppedOmciFrames;                                                                                                                  
  MV_U32 gemRxGemFramesWithUncorrHecErr;                                                                                                          
  MV_U32 gemRxGemFramesWithOneFixedHecErr;                                                                                             
  MV_U32 gemRxGemFramesWithTwoFixedHecErr;                                                                                             
  MV_U32 gemRxTotalByteCountOfReceivedValidGemFrames;                                                                                  
  MV_U32 gemRxTotalByteCountOfReceivedUndefinedGemFrames;                                                                              
  MV_U32 gemRxGemReassembleMemoryFlush;                                                                                                
  MV_U32 gemRxGemSynchLost;                                                                                                            
  MV_U32 gemRxEthFramesWithCorrFcs;                                                                                                    
  MV_U32 gemRxEthFramesWithFcsError;                                                                                                   
  MV_U32 gemRxOmciFramesWithCorrCrc;                                                                                                   
  MV_U32 gemRxOmciFramesWithCrcError;                                                                                                  
}S_GponIoctlGemRxPm;              

typedef struct                                                                                                            
{                                                                                                                         
  MV_U32 gemTxGemPtiTypeOneFrames;                                                                                      
  MV_U32 gemTxGemPtiTypeZeroFrames;                                                                                     
  MV_U32 gemTxIdleGemFrames;                                                                                            
  MV_U32 gemTxEthFramesViaTconti[ONU_GPON_MAX_NUM_OF_T_CONTS];                                                          
  MV_U32 gemTxEthBytesViaTconti[ONU_GPON_MAX_NUM_OF_T_CONTS];                                                           
  MV_U32 gemTxGemFramesViaTconti[ONU_GPON_MAX_NUM_OF_T_CONTS];                                                           
  MV_U32 gemTxIdleGemFramesViaTconti[ONU_GPON_MAX_NUM_OF_T_CONTS];                                                      
}S_GponIoctlGemTxPm;                                                                                                          
                                                                                                                          
typedef struct                                                                                                            
{                                                                                                                         
  unsigned int  section;                                                                                                  
  union                                                                                                                   
  {                                                                                                                       
    S_GponIoctlPloamRxPm ploamRx;                                                                                             
    S_GponIoctlPloamTxPm ploamTx;                                                                                             
    S_GponIoctlBwMapPm   bwMap;                                                             
    S_GponIoctlFecPm     fec;                                                                             
    S_GponIoctlGemRxPm   gemRx;                                                                           
    S_GponIoctlGemTxPm   gemTx;                                                                           
  };                                                                                                  
}S_GponIoctlPm;                                                                                           
                                                                                                      
typedef struct                                                                                        
{                                                                                                     
  MV_U32 alarmTbl[ONU_GPON_NUM_OF_ALARMS];                                                      
}S_GponIoctlAlarm;                                                                                        
                                                                                                      
typedef struct                                                                                         
{                                                                                        
  MV_U32 onuId;      
  MV_U32 onuState;      
  MV_U32 onuSignalDetect;      
  MV_U32 onuDsSyncOn;
  MV_U32 omccPort;                                                                 
  MV_U32 omccValid;  
  MV_U32 serialNumSource;
  MV_U8  serialNum[8];                                                            
  MV_U8  password[10];     
  MV_U32 disableSn;
  MV_U32 clearGem;  
  MV_U32 clearTcont;
  MV_U32 restoreGem;
  MV_U32 dgPolarity;
  MV_U32 xvrPolarity;
}S_GponIoctlInfo;  
                                                                                         
typedef struct                                                                           
{                                                                                        
  MV_U32 alloc;                                                                    
  MV_U32 tcont;    
  MV_U32 gemPort;                                                                    
  MV_U32 gemState;    
}S_GponIoctlData;                                                                            
                                                                                         
typedef struct                                                                           
{
  MV_U32 mask;   
  MV_U32 polarity;
  MV_U32 delay;  
  MV_U32 enStop; 
  MV_U32 enStart;
}S_GponIoctlXvr;

/* Gem */
typedef struct                                                                           
{   
  uint32_t             gemmap[GPON_ONU_MAX_GEM_PORTS/32];   
  E_GponIoctlGemAction action;
}S_GponIoctlGem;

typedef struct
{
  E_GponIoctlGemPortCmd command;
  MV_U16                gem_port;
}S_GponIoctlGemPortPmConfig;

typedef struct
{
  MV_U16  gem_port;

  MV_U64  good_octets_received;
  MV_U32  bad_octets_received;
  MV_U32  mac_trans_error;
  MV_U32  good_frames_received;
  MV_U32  bad_frames_received;
  MV_U32  broadcast_frames_received;
  MV_U32  multicast_frames_received;
  MV_U32  frames_64_octets;
  MV_U32  frames_65_to_127_octets;
  MV_U32  frames_128_to_255octets;
  MV_U32  frames_256_to_511_octets;
  MV_U32  frames_512_to_1023_octets; 
  MV_U32  frames_1024_to_max_octets; 
  MV_U64  good_octets_sent;
  MV_U32  good_frames_sent;
  MV_U32  multicast_frames_sent;
  MV_U32  broadcast_frames_sent;
  MV_U32  bad_crc_received;
} S_GponIoctlGemPortMibCounters;

typedef struct 
{
  S_GponIoctlInfo  g_info;
  S_GponIoctlAlarm alarm;
  S_GponIoctlPm    g_pm;
  S_GponIoctlData  data;
  struct cdev      cdev;
}S_GponModuleCdev;

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

MV_STATUS onuGponMngInterfaceCreate(void);
MV_STATUS onuGponMngInterfaceRelease(void);
void      onuGponMiNotifyCallback(MV_U32 onuState);

/* Macros
------------------------------------------------------------------------------*/    

#endif /* _ONU_GPON_LINUX_KS_MNG_INTERFACE_H */

  

