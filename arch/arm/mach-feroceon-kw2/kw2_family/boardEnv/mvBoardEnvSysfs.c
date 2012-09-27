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
*******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>

#include "boardEnv/mvBoardEnvLib.h"

static ssize_t mvPonType(char *buf)
{
  MV_U32 ponType;
  int    off = 0;

  ponType = mvBoardPonConfigGet();

  if (ponType == BOARD_EPON_CONFIG) 
    off += sprintf(buf+off, "epon\n");
  else if (ponType == BOARD_GPON_CONFIG) 
    off += sprintf(buf+off, "gpon\n");

  return off;
}

static ssize_t mvBoardType(char *buf)
{
  MV_U32 boardType;
  int    off = 0;

  boardType = mvBoardIdGet();

  if (boardType == DB_88F6535_BP_ID) 
    off += sprintf(buf+off, "db_board\n");
  else if (boardType == RD_88F6510_SFU_ID) 
    off += sprintf(buf+off, "rd_board\n");
  
  return off;
}

static ssize_t mvBoardEnvShow(struct device *dev, 
				              struct device_attribute *attr, char *buf)
{
  const char*     name = attr->attr.name;

  if (!capable(CAP_NET_ADMIN))
  	return -EPERM;

  if (!strcmp(name, "pon_type")) 
    return(mvPonType(buf));
  else if (!strcmp(name, "board_type")) 
    return(mvBoardType(buf));

  return(0);
}

static ssize_t mvBoardEnvStore(struct device *dev, 
                               struct device_attribute *attr, 
                               const char *buf, size_t len)
{
  return(0);
}

static DEVICE_ATTR(pon_type,   S_IRUSR, mvBoardEnvShow, mvBoardEnvStore);
static DEVICE_ATTR(board_type, S_IRUSR, mvBoardEnvShow, mvBoardEnvStore);

static struct attribute *mvBoardEnvAttrs[] = {
	&dev_attr_pon_type.attr,
    &dev_attr_board_type.attr,
	NULL
};

static struct attribute_group mvBoardEnvGroup = {
	.name = "board",
	.attrs = mvBoardEnvAttrs,
};

int __devinit mvBoardEnvSysfsInit(void)
{
		int err;
		struct device *pd;

		pd = bus_find_device_by_name(&platform_bus_type, NULL, "boardEnv");
		if (!pd) {
			platform_device_register_simple("boardEnv", -1, NULL, 0);
			pd = bus_find_device_by_name(&platform_bus_type, NULL, "boardEnv");
		}

		if (!pd) {
			printk(KERN_ERR"%s: cannot find board device\n", __FUNCTION__);
			pd = &platform_bus;
		}

		err = sysfs_create_group(&pd->kobj, &mvBoardEnvGroup);
		if (err) {
			printk(KERN_INFO "sysfs group failed %d\n", err);
			goto out;
		}
out:
		return err;
}

module_init(mvBoardEnvSysfsInit); 

MODULE_AUTHOR("Oren Ben-Hayun");
MODULE_DESCRIPTION("sysfs for Marvell Board Env");
MODULE_LICENSE("GPL");

