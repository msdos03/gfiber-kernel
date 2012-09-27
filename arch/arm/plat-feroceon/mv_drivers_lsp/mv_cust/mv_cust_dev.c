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
* mv_cust_dev.c
*
* DESCRIPTION:
*       
* 
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/wait.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <linux/poll.h>
#include <linux/clk.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include "mv_cust_dev.h"
#include "mv_cust_netdev.h"
#include "mv_cust_flow_map.h"
#include "mv_cust_mng_if.h"


/* Used to prevent multiple access to device */
static int               mvcust_device_open = 0;
static struct miscdevice mvcust_misc_dev;

/*******************************************************************************
**
**  mvcust_dev_open
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function execute device open actions
**
**  PARAMETERS:  
**               
**               
**               
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
static int mvcust_dev_open(struct inode *inode, struct file *file)
{
	MVCUST_DEBUG_PRINT("Enter");

   // if (mvcust_device_open > 0)
   //     return -EBUSY;

	mvcust_device_open++;

	return 0;
}

/*******************************************************************************
**
**  mvcust_dev_release
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function execute device release actions
**
**  PARAMETERS:  
**               
**               
**               
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
static int mvcust_dev_release(struct inode *inode, struct file *file)
{
    MVCUST_DEBUG_PRINT("Enter");

   // if (mvcust_device_open > 0)
   //     mvcust_device_open--;

	return 0;
}

/*******************************************************************************
**
**  mv_cust_dev_ioctl
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
static int mvcust_dev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    mv_cust_ioctl_omci_set_t      cust_omci_set;
    mv_cust_ioctl_llid_set_t      cust_llid_set;
    mv_cust_ioctl_flow_map_t      cust_flow_map;
    mv_cust_ioctl_dscp_map_t      cust_dscp_map;    
    int                           enable;
    int                           rc;

  int  ret = -EINVAL;

    MVCUST_DEBUG_PRINT("Enter");

  switch(cmd)
  {
      case MV_CUST_IOCTL_OMCI_SET:

          if(copy_from_user(&cust_omci_set, (mv_cust_ioctl_omci_set_t*)arg, sizeof(mv_cust_ioctl_omci_set_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }

          rc = mv_cust_omci_set(cust_omci_set.tcont, cust_omci_set.txq, cust_omci_set.gemport, cust_omci_set.keep_rx_mh);
          if (rc != 0) {
              MVCUST_ERR_PRINT("mv_cust_omci_set failed\n");
          }
          ret = 0;
          break;

      case MV_CUST_IOCTL_OMCI_ENABLE:

          if(copy_from_user(&enable, (int *)arg, sizeof(int)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }
          mv_cust_omci_enable(enable);
          ret = 0;
          break;

      case MV_CUST_IOCTL_EOAM_ENABLE:

          if(copy_from_user(&enable, (int *)arg, sizeof(int)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }
          mv_cust_eoam_enable(enable);
          ret = 0;
          break;

      case MV_CUST_IOCTL_EOAM_LLID_SET:
          if(copy_from_user(&cust_llid_set, (int *)arg, sizeof(mv_cust_ioctl_llid_set_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }

          mv_cust_eoam_llid_set(cust_llid_set.llid, &(cust_llid_set.llid_mac[0]), cust_llid_set.txq);
          ret = 0;
          break;

      case MV_CUST_IOCTL_MAP_RULE_SET:
          if(copy_from_user(&cust_flow_map, (mv_cust_ioctl_flow_map_t *)arg, sizeof(mv_cust_ioctl_flow_map_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }

          ret = mv_cust_map_rule_set(&cust_flow_map);
          break;

      case MV_CUST_IOCTL_DSCP_MAP_SET:
          if(copy_from_user(&cust_dscp_map, (mv_cust_ioctl_dscp_map_t *)arg, sizeof(mv_cust_ioctl_dscp_map_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }

          ret = mv_cust_dscp_map_set(&cust_dscp_map.dscp_map);
          break;          

      case MV_CUST_IOCTL_MAP_RULE_DEL:
          if(copy_from_user(&cust_flow_map, (mv_cust_ioctl_flow_map_t *)arg, sizeof(mv_cust_ioctl_flow_map_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }

          ret = mv_cust_map_rule_del((uint16_t)cust_flow_map.vid, (uint8_t)cust_flow_map.pbits, cust_flow_map.dir);
          break; 

      case MV_CUST_IOCTL_DSCP_MAP_DEL:

          ret = mv_cust_dscp_map_del();
          break; 
          
      case MV_CUST_IOCTL_MAP_RULE_CLEAR:
      
          ret = mv_cust_map_rule_clear();
          break; 
          
      case MV_CUST_IOCTL_TAG_MAP_RULE_GET:
          if(copy_from_user(&cust_flow_map, (mv_cust_ioctl_flow_map_t *)arg, sizeof(mv_cust_ioctl_flow_map_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }
      
          ret = mv_cust_tag_map_rule_get(&cust_flow_map);

          if(ret != MV_CUST_OK)
            goto ioctlErr;

          if(copy_to_user((mv_cust_ioctl_flow_map_t*)arg, &cust_flow_map, sizeof(mv_cust_ioctl_flow_map_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }          
          break;

      case MV_CUST_IOCTL_UNTAG_MAP_RULE_GET:
          if(copy_from_user(&cust_flow_map, (mv_cust_ioctl_flow_map_t *)arg, sizeof(mv_cust_ioctl_flow_map_t)))
          {
            MVCUST_ERR_PRINT("copy_from_user failed\n");
            goto ioctlErr;
          }
      
          ret = mv_cust_untag_map_rule_get(&cust_flow_map);

          if(ret != MV_CUST_OK)
            goto ioctlErr;

          if(copy_to_user((mv_cust_ioctl_flow_map_t*)arg, &cust_flow_map, sizeof(mv_cust_ioctl_flow_map_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }          
          break;

      default:
          ret = -EINVAL;
  }

ioctlErr:
    return(ret);
}


static const struct file_operations mvcust_dev_fops = 
{
	.open    = mvcust_dev_open,
	.release = mvcust_dev_release,
	.ioctl   = mvcust_dev_ioctl,
};

/*******************************************************************************
**
**  mvcust_dev_init
**  ___________________________________________________________________________
**
**  DESCRIPTION: 
**
**  PARAMETERS:  
**               
**               
**               
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
int32_t mvcust_dev_init(void)
{
    int rc;

    MVCUST_DEBUG_PRINT("Enter");
    
    mvcust_misc_dev.minor = MISC_DYNAMIC_MINOR;
    mvcust_misc_dev.name  = MVCUST_DEVICE_NAME;
    mvcust_misc_dev.fops  = &mvcust_dev_fops;

    rc = misc_register(&mvcust_misc_dev);
    if (rc != 0) 
    {
        MVCUST_ERR_PRINT("rc=%d",rc);
        return rc;
    }

    rc = mvcust_netdev_init();
    if (rc != 0) 
    {
        MVCUST_ERR_PRINT("rc=%d",rc);
        return rc;
    }

    rc = mvcust_sysfs_init();
    if (rc != 0) 
    {
        MVCUST_ERR_PRINT("rc=%d",rc);
        return rc;
    }

    printk("MVCUST: misc device %s registered with minor: %d\n", MVCUST_DEVICE_NAME, mvcust_misc_dev.minor);
    return 0;
}


/*******************************************************************************
**
**  mvcust_dev_shutdown
**  ___________________________________________________________________________
**
**  DESCRIPTION: 
**
**  PARAMETERS:  
**               
**               
**               
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
void mvcust_dev_shutdown(void)
{
	MVCUST_DEBUG_PRINT("Enter");

    mvcust_sysfs_delete();

    misc_deregister(&mvcust_misc_dev);
}





