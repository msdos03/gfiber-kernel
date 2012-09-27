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
/**  FILE        : eponOnuApi.h                                              **/
/**                                                                          **/
/**  DESCRIPTION : This file contains ONU EPON API definitions               **/
/**                                                                          **/
/******************************************************************************
 *                                                                            *                              
 *  MODIFICATION HISTORY:                                                     *
 *                                                                            *
 *   26Jan10  oren_ben_hayun    created                                       *  
 * ========================================================================== *      
 *                                                                          
 ******************************************************************************/
#ifndef _ONU_EPON_API_H
#define _ONU_EPON_API_H

/* Include Files
------------------------------------------------------------------------------*/
 
/* Definitions
------------------------------------------------------------------------------*/ 

/* Enums                              
------------------------------------------------------------------------------*/ 

/* Structs                              
------------------------------------------------------------------------------*/ 

/* Typedefs
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Notify API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiStatusNotifyRegister(STATUSNOTIFYFUNC notifyCallBack);
MV_STATUS onuEponStatusNotify(MV_U32 status);

/* Information API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiInformationGet(S_EponIoctlInfo *onuInfo, MV_U32 macId);
MV_STATUS onuEponApiLinkStateGet(MV_U32 *linkState, MV_U32 macId);

/* FEC API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiFecConfig(MV_U32 rxGenFecEn, MV_U32 txGenFecEn, MV_U32 *txMacFecEn, MV_U32 source);
MV_STATUS onuEponApiFecReConfig(void);

/* Encryption API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiEncryptionConfig(MV_U32 onuEncryptCfg);
MV_STATUS onuEponApiEncryptionKeyConfig(MV_U32 encryptKey, MV_U32 encryptKeyIndex, MV_U32 macId);

/* PM API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiRxPmGet(S_EponIoctlRxPm *rxPm, MV_BOOL clear, MV_U32 macId);
MV_STATUS onuEponApiTxPmGet(S_EponIoctlTxPm *txPm, MV_BOOL clear, MV_U32 macId);
MV_STATUS onuEponApiSwPmGet(S_EponIoctlSwPm *swPm, MV_BOOL clear, MV_U32 macId);
MV_STATUS onuEponApiGpmPmGet(S_EponIoctlGpmPm *gpmPm, MV_BOOL clear, MV_U32 macId);

/* OAM Frame API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiTransmitOamFrame(S_OnuEponCtrlBuffer *frame, MV_U32 macId);

/* Dying Gasp API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiDgCallbackRegister(DYINGGASPFUNC dgCallBack);

/* Link status API
------------------------------------------------------------------------------*/
MV_STATUS onuEponApiLinkStatusCallbackRegister(void);
MV_BOOL onuEponLinkIsUp(void);

/* Macros
------------------------------------------------------------------------------*/    

#endif /* _ONU_EPON_API_H */

