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
* mv_cust_mng_if.h                                           
*                                                                           
* DESCRIPTION : This file contains ONU MV CUST Management Interface        
********************************************************************************/
#ifndef _MV_CUST_MNG_IF_H_
#define _MV_CUST_MNG_IF_H_

/* Include Files
------------------------------------------------------------------------------*/
#include <linux/cdev.h>

/* Definitions
------------------------------------------------------------------------------*/
#define MV_CUST_IOCTL_OMCI_SET                _IOW(MV_CUST_IOCTL_MAGIC,  1,  unsigned int)
#define MV_CUST_IOCTL_EOAM_LLID_SET           _IOW(MV_CUST_IOCTL_MAGIC,  2,  unsigned int)
#define MV_CUST_IOCTL_EOAM_ENABLE             _IOW(MV_CUST_IOCTL_MAGIC,  3,  unsigned int)
#define MV_CUST_IOCTL_OMCI_ENABLE             _IOW(MV_CUST_IOCTL_MAGIC,  4,  unsigned int)
#define MV_CUST_IOCTL_MAP_RULE_SET            _IOW(MV_CUST_IOCTL_MAGIC,  5,  unsigned int)
#define MV_CUST_IOCTL_DSCP_MAP_SET            _IOW(MV_CUST_IOCTL_MAGIC,  6,  unsigned int)
#define MV_CUST_IOCTL_MAP_RULE_DEL            _IOW(MV_CUST_IOCTL_MAGIC,  7,  unsigned int)
#define MV_CUST_IOCTL_DSCP_MAP_DEL            _IOW(MV_CUST_IOCTL_MAGIC,  8,  unsigned int)
#define MV_CUST_IOCTL_MAP_RULE_CLEAR          _IOW(MV_CUST_IOCTL_MAGIC,  9,  unsigned int)
#define MV_CUST_IOCTL_TAG_MAP_RULE_GET        _IOR(MV_CUST_IOCTL_MAGIC,  10, unsigned int)
#define MV_CUST_IOCTL_UNTAG_MAP_RULE_GET      _IOR(MV_CUST_IOCTL_MAGIC,  11, unsigned int)





/* Enums
------------------------------------------------------------------------------*/

/* Typedefs
------------------------------------------------------------------------------*/
typedef struct
{
    int                tcont;
    int                txq;
    int                gemport;
    int                keep_rx_mh;
} mv_cust_ioctl_omci_set_t;

typedef struct
{
    int                llid;
    int                txq;
    uint8_t            llid_mac[6];
} mv_cust_ioctl_llid_set_t;

typedef struct
{
    mv_cust_dscp_pbits_t dscp_map;
} mv_cust_ioctl_dscp_map_t;

/* MV_CUST Char Device Structure */
/* ========================= */
typedef struct
{
  mv_cust_ioctl_omci_set_t      mv_cust_ioctl_omci_set;
  mv_cust_ioctl_llid_set_t      mv_cust_ioctl_llid_set;
  mv_cust_ioctl_flow_map_t      mv_cust_ioctl_flow_map;
  mv_cust_ioctl_dscp_map_t      mv_cust_ioctl_dscp_map;

  struct cdev  cdev;
} mv_cust_cdev_t;

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Macros
------------------------------------------------------------------------------*/

#endif /* _MV_CUST_MNG_IF_H_ */




















