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
* mv_cust_dev.h
*
* DESCRIPTION:
*
*
*******************************************************************************/

#ifndef __MV_CUST_DEV_H__
#define __MV_CUST_DEV_H__


#ifdef __cplusplus
extern "C" {
#endif


#define MV_CUST_VERSION     "V2.6.25"
#define MVCUST_DEVICE_NAME  "cust"
#define MV_CUST_IOCTL_MAGIC ('C')

#ifdef MV_CUST_DEBUG
#define MVCUST_DEBUG_PRINT(format, ...)   printk("%s(%d):  "format,__FUNCTION__,__LINE__, ##__VA_ARGS__)
#else
#define MVCUST_DEBUG_PRINT(format, ...)
#endif

#define MVCUST_ERR_PRINT(format, ...)     printk("%s(%d) ERROR:  "format,__FUNCTION__,__LINE__, ##__VA_ARGS__)

extern struct bus_type platform_bus_type;

/******************************************************
 * Function prototypes --                             *
*******************************************************/
int32_t mvcust_dev_init        (void);
void    mvcust_dev_shutdown    (void);

int     mvcust_netdev_init     (void);
int     mvcust_sysfs_init      (void);
void    mvcust_sysfs_delete    (void);

#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
void    mv_cust_udp_spec_print_all (void);
#endif
void    mv_cust_debug_info_set     (int val);

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
void mv_cust_flow_map_print(void);

#endif


#ifdef __cplusplus
}
#endif

#endif /*__MV_CUST_DEV_H__*/
