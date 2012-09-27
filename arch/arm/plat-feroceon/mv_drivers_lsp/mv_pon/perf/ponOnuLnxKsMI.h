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
**  FILE        : ponOnuLnxKsMI.h                                            **
**                                                                           **
**  DESCRIPTION : This file contains ONU PON Management Interface            **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   27 Nov 2011  Konstantin Porotchkin   created                              *
* =========================================================================== *
******************************************************************************/
#ifndef _ONU_PON_LINUX_KS_MNG_INTERFACE_H
#define _ONU_PON_LINUX_KS_MNG_INTERFACE_H

/* Include Files
------------------------------------------------------------------------------*/
#include "ponOnuHeader.h"

/* Definitions
------------------------------------------------------------------------------*/
#define MVPON_IOCTL_START         _IOW(MVPON_IOCTL_MAGIC, 30, unsigned int)
#define MVPON_IOCTL_MODE_GET      _IOR(MVPON_IOCTL_MAGIC, 31, unsigned int)

typedef enum {
	E_PON_DRIVER_UNDEF_MODE	= 0,
	E_PON_DRIVER_EPON_MODE	= 1,
	E_PON_DRIVER_GPON_MODE	= 2,
	E_PON_DRIVER_MAX_MODE
} E_PonDriverMode;

typedef struct {
	MV_STATUS (*ponOnuSetup)(void);
	MV_STATUS (*ponOnuSwitchOn)(void);
	MV_STATUS (*ponOnuOperate)(void);
	MV_STATUS (*userInterfaceCreate)(void);
	MV_STATUS (*usrInterfaceRelease)(void);
	MV_STATUS (*rtosResourceRelease)(void);
	void (*memRelease)(void);
	MV_STATUS (*apiStatusNotifyRegister)(STATUSNOTIFYFUNC notifyCallBack);
	MV_STATUS (*apiLinkStatusCallbackRegister)(void);
	int (*cdevIoctl)(struct inode *, struct file *, unsigned int, unsigned long);
} S_PonFunctions;


typedef struct {
	E_PonDriverMode		drvMode;
	int					devOpen;
	S_PonFunctions		func;
	struct cdev			cdev;
} S_PonModuleCdev;

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

MV_STATUS onuPonMngInterfaceCreate(void);
MV_STATUS onuPonMngInterfaceRelease(void);
void onuPonMiNotifyCallback(MV_U32 onuState);

/* Macros
------------------------------------------------------------------------------*/

#endif /* _ONU_PON_LINUX_KS_MNG_INTERFACE_H */



