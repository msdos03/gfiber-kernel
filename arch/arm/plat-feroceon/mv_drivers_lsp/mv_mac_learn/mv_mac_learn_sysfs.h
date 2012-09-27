/************************************************************************
* Copyright (C) 2010, Marvell Technology Group Ltd.
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Marvell Technology Group;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Marvell Technology Group.
*
*********************************************************************************
* Marvell GPL License Option
*
* If you received this File from Marvell, you may opt to use, redistribute and/or
* modify this File in accordance with the terms and conditions of the General
* Public License Version 2, June 1991 (the "GPL License"), a copy of which is
* available along with the File in the license.txt file or by writing to the Free
* Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
* on the worldwide web at http://www.gnu.org/licenses/gpl.txt.
*
* THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
* WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
* DISCLAIMED.  The GPL License provides additional details about this warranty
* disclaimer.
*
*********************************************************************************
* mv_mac_learn_sysfs.h
*
* DESCRIPTION:
*
*
*******************************************************************************/
#ifndef __mv_mac_learn_sysfs_h__
#define __mv_mac_learn_sysfs_h__

/* Include files
------------------------------------------------------------------------------*/
#include <linux/capability.h>
#include <linux/platform_device.h>

/* Typedefs
------------------------------------------------------------------------------*/
typedef struct {
	char                   *folder_name;
	struct attribute_group *pattrgroup;
} mv_attr_group_pair_t;

typedef struct
{
	char *sysfs_name;
	void (*sysfs_func)(const char *buf, size_t len);
} mac_learn_store_sysfs_func_t;

typedef struct
{
	char *sysfs_name;
	int (*sysfs_func)(char *buf);
} mac_learn_show_sysfs_func_t;

/* Function protype
------------------------------------------------------------------------------*/
int mv_mac_learn_sysfs_init(void);
void mv_mac_learn_sysfs_exit(void);

#endif
