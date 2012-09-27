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
**  FILE        : ponOnuLnxKsMI.c                                            **
**                                                                           **
**  DESCRIPTION : This file implements ONU PON Management Interface MUX      **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   27 Nov 2011  Konstantin Porotchkin   created                              *
* =========================================================================== *
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include <linux/version.h>
#include "ponOnuHeader.h"
#include "ponOnuLnxKsMI.h"
#include "eponOnuHeader.h"
#include "gponOnuHeader.h"

/* Local Constant
------------------------------------------------------------------------------*/
#define __FILE_DESC__ "mv_pon/perf/ponOnuLnxKsMI.c"

/* Global Variables
------------------------------------------------------------------------------*/

/* Local Variables
------------------------------------------------------------------------------*/
S_PonModuleCdev  ponDev;
/*
static char *drvMode = "epon";
module_param(drvMode, charp, 0);
MODULE_PARM_DESC(drvMode, "PON driver mode (epon/gpon)");
*/
/* udev class's */
struct class  *pon_udev_class;
struct device *pon_udev_dev;

struct fasync_struct *mvPonAsqueue;

/* Export Functions
------------------------------------------------------------------------------*/
extern int mvGponCdevIoctl(struct inode *inode, struct file  *filp, unsigned int  cmd, unsigned long arg);
extern int mvEponCdevIoctl(struct inode *inode, struct file  *filp, unsigned int  cmd, unsigned long arg);

/* Local Functions
------------------------------------------------------------------------------*/
void onuPonMiNotifyCallback(MV_U32 onuState);

/*******************************************************************************
**
**  mvPonMiInit
**  ___________________________________________________________________________
**
**  DESCRIPTION: Initialize driver
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     (0) on success or error
**
*******************************************************************************/
int mvPonMiInit (void)
{
	MV_STATUS status = 0;

	status = ponDev.func.ponOnuSetup();
	if (status != MV_OK) {
		printk(KERN_INFO "= PON Module setup failed =\n");
		goto ponInitError;
	}

	status = ponDev.func.ponOnuSwitchOn();
	if (status != MV_OK) {
		printk(KERN_INFO "= PON Module switch ON failed =\n");
		goto ponInitError;
	}

	status = ponDev.func.ponOnuOperate();
	if (status != MV_OK) {
		printk(KERN_INFO "= PON Module operate failed =\n");
		goto ponInitError;
	}

	/* create user interface */
	status = ponDev.func.userInterfaceCreate();
	if (status != MV_OK) {
		printk(KERN_INFO "= PON Module UI creation failed =\n");
		goto ponInitError;
	}

	/* register mng notification callback */
	status = ponDev.func.apiStatusNotifyRegister(onuPonMiNotifyCallback);
	if (status != MV_OK) {
		printk(KERN_INFO "= PON Module API status notification callback registration failed =\n");
		goto ponInitError;
	}

	/* register link status callback */
	status = ponDev.func.apiLinkStatusCallbackRegister();
	if (status != MV_OK) {
		printk(KERN_INFO "= PON Module link status notification callback registration failed =\n");
		goto ponInitError;
	}

	printk(KERN_INFO "= PON Module Init ended successfully =\n");

ponInitError:
		return(-status);
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
int mvPonCdevIoctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int				ret = -EINVAL;
	E_PonDriverMode drvMode;


	switch (cmd) {
	/* ====== MVPON_IOCTL_START ======= */
	case MVPON_IOCTL_START:
		if (ponDev.drvMode == E_PON_DRIVER_UNDEF_MODE) {
			/* The driver only alows transition from Undefined to EPON or GPON mode */
			ret = get_user(drvMode, (E_PonDriverMode __user *)arg);
			if (ret != 0) {
				mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
					"ERROR: (%s:%d) copy from user failed\n", __FILE_DESC__, __LINE__);
				goto ioctlErr;
			}
			if (drvMode == E_PON_DRIVER_EPON_MODE) {

				ponDev.func.ponOnuSetup = onuEponSetup;
				ponDev.func.ponOnuSwitchOn = onuEponSwitchOn;
				ponDev.func.ponOnuOperate = onuEponOperate;
				ponDev.func.userInterfaceCreate = onuEponUsrInterfaceCreate;
				ponDev.func.usrInterfaceRelease = onuEponUsrInterfaceRelease;
				ponDev.func.rtosResourceRelease = onuEponRtosResourceRelease;
				ponDev.func.memRelease = 0;
				ponDev.func.apiStatusNotifyRegister = onuEponApiStatusNotifyRegister;
				ponDev.func.apiLinkStatusCallbackRegister = onuEponApiLinkStatusCallbackRegister;
				ponDev.func.cdevIoctl = mvEponCdevIoctl;

			} else if (drvMode == E_PON_DRIVER_GPON_MODE) {

				ponDev.func.ponOnuSetup = onuGponSetup;
				ponDev.func.ponOnuSwitchOn = onuGponSwitchOn;
				ponDev.func.ponOnuOperate = onuGponOperate;
				ponDev.func.userInterfaceCreate = onuGponUsrInterfaceCreate;
				ponDev.func.usrInterfaceRelease = onuGponUsrInterfaceRelease;
				ponDev.func.rtosResourceRelease = onuGponRtosResourceRelease;
				ponDev.func.memRelease = onuPonMemRelease;
				ponDev.func.apiStatusNotifyRegister = onuGponApiStatusNotifyRegister;
				ponDev.func.apiLinkStatusCallbackRegister = onuGponApiLinkStatusCallbackRegister;
				ponDev.func.cdevIoctl = mvGponCdevIoctl;

			} else {
				mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
					"ERROR: (%s:%d) requested unsupported PON mode 0x%x\n", __FILE_DESC__, __LINE__, arg);
				goto ioctlErr;
			}

			ret = mvPonMiInit();
			if (ret != 0) {
				mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
					"ERROR: (%s:%d) error switching to %s mode\n",
					__FILE_DESC__, __LINE__, (drvMode == E_PON_DRIVER_EPON_MODE) ? "EPON" : "GPON");
				goto ioctlErr;
			}
			ponDev.drvMode = drvMode;

		} else {
			/* Switching between EPON-GPON modes requires reboot initiated by US application */
			mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
					"ERROR: (%s:%d) command MVPON_IOCTL_INIT is not supported in this mode\n",
					__FILE_DESC__, __LINE__);
			goto ioctlErr;
		}
		break;

	/* ====== MVPON_IOCTL_MODE_GET ======= */
	case MVPON_IOCTL_MODE_GET:
		ret = put_user(ponDev.drvMode, (int __user *)arg);
		if (ret != 0) {
			mvPonPrint(PON_PRINT_ERROR, PON_API_MODULE,
				"ERROR: (%s:%d) copy to user failed\n", __FILE_DESC__, __LINE__);
			goto ioctlErr;
		}
		break;

	/* ====== All GPON/EPON-specific IOCTLs ======= */
	default:
		if (ponDev.drvMode != E_PON_DRIVER_UNDEF_MODE)
			ret = ponDev.func.cdevIoctl(inode, filp, cmd, arg);
		else
			ret = -EINVAL;
	}


ioctlErr:

	return(ret);
}

/*******************************************************************************
**
**  mvPonCdevFasync
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function execute notification to User space
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

void onuPonMiNotifyCallback(MV_U32 onuState)
{
	if (mvPonAsqueue)
		kill_fasync(&mvPonAsqueue, SIGIO, POLL_IN);
}

int mvPonCdevFasync(int fd, struct file *filp, int mode)
{
	return(fasync_helper(fd, filp, mode, &mvPonAsqueue));
}


/*******************************************************************************
**
**  mvEponCdevOpen
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function opens the EPON Char device
**
**  PARAMETERS:  struct inode *inode
**               struct file *filp
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
int mvPonCdevOpen(struct inode *inode, struct file *filp)
{
	S_PonModuleCdev *dev;

	/* find the device structure */
	dev = container_of(inode->i_cdev, S_PonModuleCdev, cdev);
	dev->devOpen++;
	filp->private_data = dev;

	return(0);
}

/*******************************************************************************
**
**  mvPonCdevRelease
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function releases the EPON Char device
**
**  PARAMETERS:  struct inode *inode
**               struct file *filp
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
int mvPonCdevRelease(struct inode *inode, struct file *filp)
{
	S_PonModuleCdev *dev = filp->private_data;

	dev->devOpen--;

	return(0);
}

/*******************************************************************************
**  PON device operations
*******************************************************************************/
const struct file_operations ponCdevFops = {
	.owner   = THIS_MODULE,
	.open    = mvPonCdevOpen,
	.release = mvPonCdevRelease,
	.ioctl   = mvPonCdevIoctl,
	.fasync  = mvPonCdevFasync
};

/*******************************************************************************
**
**  onuPonMngInterfaceCreate
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function create management interface - char device
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuPonMngInterfaceCreate(void)
{
	int   rcode;
	dev_t dev;

	dev   = MKDEV(MV_PON_MAJOR, 0);
	rcode = register_chrdev_region(dev, PON_NUM_DEVICES, PON_DEV_NAME);
	if (rcode < 0) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) Pon Char Device\n", __FILE_DESC__, __LINE__);
		return(MV_ERROR);
	}

	cdev_init(&ponDev.cdev, &ponCdevFops);
	ponDev.cdev.owner = THIS_MODULE;
	ponDev.drvMode = E_PON_DRIVER_UNDEF_MODE;

	rcode = cdev_add(&ponDev.cdev, dev, 1);
	if (rcode < 0) {
		mvPonPrint(PON_PRINT_ERROR, PON_INIT_MODULE,
				   "ERROR: (%s:%d) Pon Char Device Add\n", __FILE_DESC__, __LINE__);

		cdev_del(&ponDev.cdev);
		unregister_chrdev_region(dev, PON_NUM_DEVICES);

		return(MV_ERROR);
	}

	/* create device for udev */
	pon_udev_class = class_create(THIS_MODULE, PON_DEV_NAME);
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 22)
	pon_udev_dev   = device_create(pon_udev_class, NULL, dev, PON_DEV_NAME);
#else
	pon_udev_dev   = device_create(pon_udev_class, NULL, dev, NULL, PON_DEV_NAME);
#endif
	return(MV_OK);
}

/*******************************************************************************
**
**  onuPonMngInterfaceRelease
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function release management interface - char device
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     MV_OK or MV_ERROR
**
*******************************************************************************/
MV_STATUS onuPonMngInterfaceRelease(void)
{
	dev_t dev = MKDEV(MV_PON_MAJOR, 0);

	device_destroy(pon_udev_class, dev);
	class_unregister(pon_udev_class);
	class_destroy(pon_udev_class);

	unregister_chrdev_region(dev, PON_NUM_DEVICES);

	return(MV_OK);
}

/*******************************************************************************
**
**  mvOnuPonModule_init
**  ____________________________________________________________________________
**
**  DESCRIPTION: Driver INIT function
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     0
**
*******************************************************************************/
int __init mvOnuPonModule_init(void)
{
	MV_STATUS	status;

	printk(KERN_INFO "= PON Module Init =\n");

	memset(&ponDev, 0, sizeof(ponDev));
	status = onuPonMngInterfaceCreate();
	if (status != MV_OK) {
		printk(KERN_ERR "= PON Module Init FAILED =\n");
		return(status);
	}

	printk(KERN_INFO "= PON Module Init ended successfully =\n");
	return(0);
}

/*******************************************************************************
**
**  mvOnuPonModule_exit
**  ____________________________________________________________________________
**
**  DESCRIPTION: Driver EXIT function
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void __exit mvOnuPonModule_exit(void)
{
	printk(KERN_INFO "= PON Module Exit =\n");

	/* release allocated memory */
	if (ponDev.func.memRelease != 0)
		ponDev.func.memRelease();

	/* release rtos resources */
	if (ponDev.func.rtosResourceRelease != 0)
		ponDev.func.rtosResourceRelease();

	/* release user interface */
	if (ponDev.func.usrInterfaceRelease != 0)
		ponDev.func.usrInterfaceRelease();

	/* release mng interface */
	onuPonMngInterfaceRelease();

	return;
}

module_init(mvOnuPonModule_init);
module_exit(mvOnuPonModule_exit);

MODULE_AUTHOR("Konstantin Porotchkin");
MODULE_DESCRIPTION("United PON Driver for Marvell MV65xxx");
MODULE_LICENSE("GPL");
