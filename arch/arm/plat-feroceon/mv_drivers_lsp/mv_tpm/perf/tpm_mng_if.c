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
**  FILE        : tpm_mng_if.c                                               **
**                                                                           **
**  DESCRIPTION : This file implements ONU TPM Management Interface          **
*******************************************************************************
*                                                                             *
*  MODIFICATION HISTORY:                                                      *
*                                                                             *
*   11Aug10  Oren Ben Hayun   created                                         *
* =========================================================================== *
******************************************************************************/

/* Include Files
------------------------------------------------------------------------------*/
#include <linux/thread_info.h>
#include <linux/version.h>
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_mempool.h"




/* Local Structures
------------------------------------------------------------------------------*/
/* Mempools used by ioctl, 3 size: Small, Medium Large and Huge */
typedef struct
{
    tpm_common_mempool_t *mpool_s;
    tpm_common_mempool_t *mpool_m;
    tpm_common_mempool_t *mpool_l;
    tpm_common_mempool_t *mpool_h;
} tpm_ioctl_mpools_t;

#define MPOOL_S_MAX_BUFF_SIZE   300
#define MPOOL_M_MAX_BUFF_SIZE  1024
#define MPOOL_L_MAX_BUFF_SIZE  2048
#define MPOOL_H_MAX_BUFF_SIZE  4096

/* TPM Char Device Structure */
/* ========================= */
typedef struct
{
  tpm_ioctl_admin_t           tpm_ioctl_admin;
  tpm_ioctl_add_acl_rule_t    tpm_ioctl_add_acl_rule;
  tpm_ioctl_del_acl_rule_t    tpm_ioctl_del_acl_rule;
  tpm_ioctl_ctrl_acl_rule_t   tpm_ioctl_ctrl_acl_rule;
  tpm_ioctl_mc_rule_t         tpm_ioctl_mc_rule;
  tpm_ioctl_mng_ch_t          tpm_ioctl_mng_ch;
  tpm_ioctl_sw_mac_security_t tpm_ioctl_sw_mac_security;
  tpm_ioctl_sw_vlan_filter_t  tpm_ioctl_sw_vlan_filter;
  tpm_ioctl_sw_tm_t           tpm_ioctl_sw_tm;
  tpm_ioctl_tm_tm_t           tpm_ioctl_tm_tm;
  tpm_ioctl_igmp_t            tpm_ioctl_igmp;
  tpm_ioctl_mib_reset_t       tpm_ioctl_mib_reset;
  tpm_ioctl_set_active_wan_t  tpm_ioctl_set_active_wan;
  tpm_ioctl_swport_pm_3_t     apm_ioctl_pm_ethernet_3;
  tpm_ioctl_rx_igmp_t         tpm_ioctl_rx_igmp;
  tpm_ioctl_tx_igmp_t         tpm_ioctl_tx_igmp;
  tpm_ioctl_cpu_lpbk_t        tpm_ioctl_cpu_lpbk;
  tpm_ioctl_age_count_t       tpm_ioctl_age_count;
  tpm_ioctl_mc_vid_t          tpm_ioctl_mc_vid;
  struct cdev  cdev;
}tpm_module_cdev_t;



/* Local Constant
------------------------------------------------------------------------------*/

/* Global Variables
------------------------------------------------------------------------------*/
extern GT_QD_DEV *qd_dev;
extern int32_t module_tpmSetup(tpm_setup_t *tpm_initp);

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
struct semaphore    tpm_sfs_2_ioctl_sem;
tpm_ioctl_mng_t     tpm_sfs_2_ioctl_command;
#endif

/* Local Variables
------------------------------------------------------------------------------*/
static int        deviceOpen = 0;
tpm_module_cdev_t tpm_dev;
static tpm_ioctl_mpools_t  tpm_ioctl_mpools= {0,0,0,0};

/* udev class's */
struct class  *tpm_udev_class;
struct device *tpm_udev_dev;

/* Export Functions
------------------------------------------------------------------------------*/

/* Local Functions
------------------------------------------------------------------------------*/

/*******************************************************************************
**
**  mv_tpm_cdev_ioctl
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
int mv_tpm_cdev_ioctl(struct inode *inode, struct file *filp, unsigned int cmd,
                      unsigned long arg)
{

  tpm_init_t                    *tpm_initp      = NULL;
  void                          *mpool_s_buff   = NULL;
  void                          *mpool_m_buff   = NULL;
  void                          *mpool_l_buff   = NULL;
  void                          *mpool_h_buff   = NULL;
  tpm_ioctl_mib_reset_t         tpm_mib_reset_param;
  GT_STATUS                     rc = GT_OK;
  tpm_error_code_t              rcode;
  int                           ret = -EINVAL;


  //onuTpmIrqLock();

  /* Init Struct created by malloc, due to kernel_stack_size limit reached */
  /* TODO: Seperate init_api's into seperate ioctl */

  switch(cmd)
  {
      /* =========================== */
      /* ====== Init Section ======= */
      /* =========================== */
      case MV_TPM_IOCTL_INIT_SECTION:
          tpm_initp = (tpm_init_t *) kmalloc(sizeof(tpm_init_t), GFP_USER);
          if (tpm_initp == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) kmalloc failed\n", __FUNCTION__, __LINE__);
              ret = -ENOMEM;
              goto ioctlErr;
          }

          if(copy_from_user(tpm_initp, (tpm_init_t*)arg, sizeof(tpm_init_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          rcode = tpm_module_start(tpm_initp);
          if(rcode != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;

      case MV_TPM_IOCTL_SETUP_SECTION:
      {
          tpm_setup_t *tpm_setup = (tpm_setup_t *)tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_setup == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_setup;

          if(copy_from_user(tpm_setup, (tpm_setup_t*)arg, sizeof(tpm_setup_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }
          rcode = module_tpmSetup(tpm_setup);
          if(rcode != TPM_OK)
            goto ioctlErr;
          ret=0;
          break;
      }

      /* =========================== */
      /* ====== Admin Section ====== */
      /* =========================== */
      case MV_TPM_IOCTL_ADMIN_SECTION:
      {
          tpm_ioctl_admin_t *tpm_admin = (tpm_ioctl_admin_t *)tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_admin == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_admin;

          if(copy_from_user(tpm_admin, (tpm_ioctl_admin_t*)arg, sizeof(tpm_ioctl_admin_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_admin->amdin_cmd)
          {
              /* ====== MV_TPM_IOCTL_CRT_OWNER =============== */
              case MV_TPM_IOCTL_CRT_OWNER:

                  rcode = tpm_create_ownerid(&(tpm_admin->owner_id));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_admin_t*)arg, tpm_admin, sizeof(tpm_ioctl_admin_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;


              /* ====== MV_TPM_IOCTL_REQ_API_G_OWNER ========= */
              case MV_TPM_IOCTL_REQ_API_G_OWNER:

                  tpm_admin->api_ownership_error = tpm_request_api_ownership(tpm_admin->owner_id,
                                                                             tpm_admin->api_type);

                  if(copy_to_user((tpm_ioctl_admin_t*)arg, tpm_admin, sizeof(tpm_ioctl_admin_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;


              /* ====== MV_TPM_IOCTL_GET_API_G_OWNER ========= */
              case MV_TPM_IOCTL_GET_API_G_OWNER:

                  tpm_admin->api_ownership_error = tpm_request_api_ownership(tpm_admin->owner_id,
                                                                             tpm_admin->api_type);

                  if(copy_to_user((tpm_ioctl_admin_t*)arg, tpm_admin, sizeof(tpm_ioctl_admin_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_SECTION ========= */
              case MV_TPM_IOCTL_DEL_SECTION:

                  rcode = tpm_erase_section(tpm_admin->owner_id,
                                            tpm_admin->api_type);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_GET_SECTION_SIZE ========= */
              case MV_TPM_IOCTL_GET_SECTION_SIZE:

                  rcode = tpm_get_section_free_size(tpm_admin->api_type,
                                                    &(tpm_admin->section_size));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_admin_t*)arg, tpm_admin, sizeof(tpm_ioctl_admin_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

		/* ====== MV_TPM_IOCTL_GET_SECTION_SIZE ========= */
		case MV_TPM_IOCTL_SET_DROP_PRECEDENCE_MODE:
	  
			rcode = tpm_set_drop_precedence_mode(tpm_admin->owner_id,
					tpm_admin->drop_pre_mode);
			if(rcode != TPM_OK)
				goto ioctlErr;

			ret = 0;
			break;
              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== ACL Rule Add Section ====== */
      /* ================================== */
      case MV_TPM_IOCTL_ADD_ACL_SECTION:
      {
          tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = (tpm_ioctl_add_acl_rule_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);

          if (tpm_add_acl_rule == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_add_acl_rule;
          if(copy_from_user(tpm_add_acl_rule, (tpm_ioctl_add_acl_rule_t*)arg, sizeof(tpm_ioctl_add_acl_rule_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_add_acl_rule->add_acl_cmd)
          {
              /* ====== MV_TPM_IOCTL_ADD_L2_ACL_RULE ========= */
              case MV_TPM_IOCTL_ADD_L2_ACL_RULE:

                  rcode = tpm_add_l2_rule( tpm_add_acl_rule->owner_id,
                                           tpm_add_acl_rule->src_port,
                                           tpm_add_acl_rule->rule_num,
                                          &(tpm_add_acl_rule->rule_idx),
                                           tpm_add_acl_rule->parse_rule_bm,
                                           tpm_add_acl_rule->l2_acl_rule.parse_flags_bm,
                                          &(tpm_add_acl_rule->l2_acl_rule.l2_key),
                                          &(tpm_add_acl_rule->l2_acl_rule.pkt_frwd),
                                          &(tpm_add_acl_rule->l2_acl_rule.pkt_mod),
                                           tpm_add_acl_rule->l2_acl_rule.pkt_mod_bm,
                                          &(tpm_add_acl_rule->l2_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_MAC_LEARN_ACL_RULE ========= */
              case MV_TPM_IOCTL_ADD_MAC_LEARN_ACL_RULE:
                  rcode = tpm_add_mac_learn_rule(tpm_add_acl_rule->owner_id,
                                                 &(tpm_add_acl_rule->l2_acl_rule.l2_key));
                  if(rcode != TPM_OK)
                      goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                      printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                      goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SET_MAC_LEARN_DEFAULT_ACTION ========= */
              case MV_TPM_IOCTL_SET_MAC_LEARN_DEFAULT_ACTION:
                  rcode = tpm_mac_learn_default_rule_act_set(tpm_add_acl_rule->owner_id,
                                                             tpm_add_acl_rule->l2_acl_rule.mac_conf);
                  if(rcode != TPM_OK)
                      goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                      printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                      goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_L3_ACL_RULE ========= */
              case MV_TPM_IOCTL_ADD_L3_ACL_RULE:

                  rcode = tpm_add_l3_type_rule(  tpm_add_acl_rule->owner_id,
                                                 tpm_add_acl_rule->src_port,
                                                 tpm_add_acl_rule->rule_num,
                                                &(tpm_add_acl_rule->rule_idx),
                                                 tpm_add_acl_rule->parse_rule_bm,
                                                 tpm_add_acl_rule->l3_acl_rule.parse_flags_bm,
                                                &(tpm_add_acl_rule->l3_acl_rule.l3_key),
                                                &(tpm_add_acl_rule->l3_acl_rule.pkt_frwd),
                                                &(tpm_add_acl_rule->l3_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_IPv4_ACL_RULE ========= */
              case MV_TPM_IOCTL_ADD_IPv4_ACL_RULE:

                  rcode = tpm_add_ipv4_rule(  tpm_add_acl_rule->owner_id,
                                              tpm_add_acl_rule->src_port,
                                              tpm_add_acl_rule->rule_num,
                                             &(tpm_add_acl_rule->rule_idx),
                                              tpm_add_acl_rule->parse_rule_bm,
                                              tpm_add_acl_rule->ipv4_acl_rule.parse_flags_bm,
                                             &(tpm_add_acl_rule->ipv4_acl_rule.ipv4_key),
                                             &(tpm_add_acl_rule->ipv4_acl_rule.pkt_frwd),
                                             &(tpm_add_acl_rule->ipv4_acl_rule.pkt_mod),
                                              tpm_add_acl_rule->ipv4_acl_rule.pkt_mod_bm,
                                             &(tpm_add_acl_rule->ipv4_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_IPv6_ACL_RULE ========= */
              case MV_TPM_IOCTL_ADD_IPV6_GEN_ACL_RULE:

                  rcode = tpm_add_ipv6_gen_rule(  tpm_add_acl_rule->owner_id,
                                                    tpm_add_acl_rule->src_port,
                                                    tpm_add_acl_rule->rule_num,
                                                  &(tpm_add_acl_rule->rule_idx),
                                                    tpm_add_acl_rule->parse_rule_bm,
                                                    tpm_add_acl_rule->ipv6_gen_acl_rule.parse_flags_bm,
                                                  &(tpm_add_acl_rule->ipv6_gen_acl_rule.ipv6_gen_key),
                                                  &(tpm_add_acl_rule->ipv6_gen_acl_rule.pkt_frwd),
                                                  &(tpm_add_acl_rule->ipv6_gen_acl_rule.pkt_mod),
                                                    tpm_add_acl_rule->ipv6_gen_acl_rule.pkt_mod_bm,
                                                  &(tpm_add_acl_rule->ipv6_gen_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_ADD_IPV6_NH_ACL_RULE:

                  rcode = tpm_add_ipv6_nh_rule(  tpm_add_acl_rule->owner_id,
                                                 tpm_add_acl_rule->rule_num,
                                               &(tpm_add_acl_rule->rule_idx),
                                                 tpm_add_acl_rule->parse_rule_bm,
                                                 tpm_add_acl_rule->ipv6_nh_acl_rule.nh_iter,
                                                 tpm_add_acl_rule->ipv6_nh_acl_rule.nh,
                                               &(tpm_add_acl_rule->ipv6_nh_acl_rule.pkt_frwd),
                                               &(tpm_add_acl_rule->ipv6_nh_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_ADD_IPV6_DIP_ACL_RULE:

                  rcode = tpm_add_ipv6_dip_rule(  tpm_add_acl_rule->owner_id,
                                                  tpm_add_acl_rule->src_port,
                                                  tpm_add_acl_rule->rule_num,
                                                &(tpm_add_acl_rule->rule_idx),
                                                  tpm_add_acl_rule->parse_rule_bm,
                                                  tpm_add_acl_rule->ipv6_dip_acl_rule.parse_flags_bm,
                                                &(tpm_add_acl_rule->ipv6_dip_acl_rule.ipv6_dip_key),
                                                &(tpm_add_acl_rule->ipv6_dip_acl_rule.pkt_frwd),
                                                &(tpm_add_acl_rule->ipv6_dip_acl_rule.pkt_mod),
                                                  tpm_add_acl_rule->ipv6_dip_acl_rule.pkt_mod_bm,
                                                &(tpm_add_acl_rule->ipv6_dip_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_ADD_IPV6_L4_PORTS_ACL_RULE:

                  rcode = tpm_add_ipv6_l4_ports_rule(  tpm_add_acl_rule->owner_id,
                                                       tpm_add_acl_rule->src_port,
                                                       tpm_add_acl_rule->rule_num,
                                                     &(tpm_add_acl_rule->rule_idx),
                                                       tpm_add_acl_rule->parse_rule_bm,
                                                       tpm_add_acl_rule->ipv6_l4_ports_acl_rule.parse_flags_bm,
                                                     &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.l4_key),
                                                     &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_frwd),
                                                     &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_mod),
                                                       tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_mod_bm,
                                                     &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_IPv6_5T_RULE ========= */
              case MV_TPM_IOCTL_ADD_IPV6_GEN_5T_RULE:

                  rcode = tpm_add_ipv6_gen_5t_rule( tpm_add_acl_rule->owner_id,
                                                    tpm_add_acl_rule->src_dir,
                                                    tpm_add_acl_rule->rule_num,
                                                  &(tpm_add_acl_rule->rule_idx),
                                                    tpm_add_acl_rule->parse_rule_bm,
                                                    tpm_add_acl_rule->ipv6_gen_5t_rule.parse_flags_bm,
                                                  &(tpm_add_acl_rule->ipv6_gen_5t_rule.l4_key),
                                                  &(tpm_add_acl_rule->ipv6_gen_5t_rule.ipv6_gen_key),
                                                  &(tpm_add_acl_rule->ipv6_gen_5t_rule.pkt_frwd),
                                                  &(tpm_add_acl_rule->ipv6_gen_5t_rule.pkt_mod),
                                                    tpm_add_acl_rule->ipv6_gen_5t_rule.pkt_mod_bm,
                                                  &(tpm_add_acl_rule->ipv6_gen_5t_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_ADD_IPV6_DIP_5T_RULE:

                  rcode = tpm_add_ipv6_dip_5t_rule( tpm_add_acl_rule->owner_id,
                                                    tpm_add_acl_rule->src_dir,
                                                    tpm_add_acl_rule->rule_num,
                                                  &(tpm_add_acl_rule->rule_idx),
                                                    tpm_add_acl_rule->parse_rule_bm,
                                                    tpm_add_acl_rule->ipv6_dip_5t_rule.parse_flags_bm,
                                                  &(tpm_add_acl_rule->ipv6_dip_5t_rule.l4_key),
                                                  &(tpm_add_acl_rule->ipv6_dip_5t_rule.ipv6_gen_key),
                                                  &(tpm_add_acl_rule->ipv6_dip_5t_rule.ipv6_dip_key),
                                                  &(tpm_add_acl_rule->ipv6_dip_5t_rule.pkt_frwd),
                                                  &(tpm_add_acl_rule->ipv6_dip_5t_rule.pkt_mod),
                                                    tpm_add_acl_rule->ipv6_dip_5t_rule.pkt_mod_bm,
                                                  &(tpm_add_acl_rule->ipv6_dip_5t_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_ADD_IPV6_L4_PORTS_5T_RULE:

                  rcode = tpm_add_ipv6_l4_ports_5t_rule(  tpm_add_acl_rule->owner_id,
                                                          tpm_add_acl_rule->src_dir,
                                                          tpm_add_acl_rule->rule_num,
                                                        &(tpm_add_acl_rule->rule_idx),
                                                          tpm_add_acl_rule->parse_rule_bm,
                                                          tpm_add_acl_rule->ipv6_l4_ports_acl_rule.parse_flags_bm,
                                                        &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.l4_key),
                                                        &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_frwd),
                                                        &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_mod),
                                                          tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_mod_bm,
                                                        &(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.rule_action));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

	      /* ====== MV_TPM_IOCTL_ADD_CTC_CM_ACL_RULE ========= */
	      case MV_TPM_IOCTL_ADD_CTC_CM_ACL_RULE:
	      	  rcode = tpm_add_ctc_cm_acl_rule(tpm_add_acl_rule->owner_id,
		  				tpm_add_acl_rule->src_port,
		  				tpm_add_acl_rule->ctc_cm_acl_rule.precedence,
		  				tpm_add_acl_rule->ctc_cm_acl_rule.l2_parse_rule_bm,
		  				tpm_add_acl_rule->ctc_cm_acl_rule.ipv4_parse_rule_bm,
		  				&tpm_add_acl_rule->ctc_cm_acl_rule.l2_key,
		  				&tpm_add_acl_rule->ctc_cm_acl_rule.ipv4_key,
		  				&tpm_add_acl_rule->ctc_cm_acl_rule.pkt_frwd,
		  				tpm_add_acl_rule->ctc_cm_acl_rule.pkt_act,
		  				tpm_add_acl_rule->ctc_cm_acl_rule.p_bits);
		  if (rcode != TPM_OK)
		  	goto ioctlErr;

		  if (copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t))) {
		  	printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
			goto ioctlErr;
		  }
		  ret = 0;
	      	  break;
	      /* ====== MV_TPM_IOCTL_ADD_CTC_CM_ACL_RULE ========= */
	      case MV_TPM_IOCTL_ADD_CTC_CM_IPV6_ACL_RULE:
	          rcode = tpm_add_ctc_cm_ipv6_acl_rule(tpm_add_acl_rule->owner_id,
				    tpm_add_acl_rule->src_port,
				    tpm_add_acl_rule->ctc_cm_acl_rule.precedence,
				    tpm_add_acl_rule->ctc_cm_acl_rule.ipv6_parse_rule_bm,
				    &tpm_add_acl_rule->ctc_cm_acl_rule.ipv6_key,
				    &tpm_add_acl_rule->ctc_cm_acl_rule.pkt_frwd,
				    tpm_add_acl_rule->ctc_cm_acl_rule.pkt_act,
				    tpm_add_acl_rule->ctc_cm_acl_rule.p_bits);
	           if (rcode != TPM_OK)
		         goto ioctlErr;

                   if (copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t))) {
		           printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
		           goto ioctlErr;
	           }
	           ret = 0;
	           break;
              /* ====== MV_TPM_IOCTL_ADD_DS_LOAD_BALANCE_RULE ========= */
	      case MV_TPM_IOCTL_ADD_DS_LOAD_BALANCE_RULE:
	          rcode = tpm_add_ds_load_balance_rule(tpm_add_acl_rule->owner_id,
				    tpm_add_acl_rule->rule_num,
				    &tpm_add_acl_rule->rule_idx,
				    tpm_add_acl_rule->parse_rule_bm,
				    tpm_add_acl_rule->ds_load_balance_acl_rule.parse_flags_bm,
				    &tpm_add_acl_rule->ds_load_balance_acl_rule.l2_key,
				    tpm_add_acl_rule->ds_load_balance_acl_rule.tgrt);
	           if (rcode != TPM_OK)
		         goto ioctlErr;

                   if (copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t))) {
		           printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
		           goto ioctlErr;
	           }
	           ret = 0;
	           break;

              /* ====== MV_TPM_IOCTL_SET_MOD_RULE ========= */
              case MV_TPM_IOCTL_SET_MOD_RULE:

                  rcode = tpm_mod_entry_set (  tpm_add_acl_rule->mod_rule.trg_port,
                                               tpm_add_acl_rule->mod_rule.mod_bm,
                                               tpm_add_acl_rule->mod_rule.int_bm,
                                               &(tpm_add_acl_rule->mod_rule.mod_data),
                                               &(tpm_add_acl_rule->rule_idx) );
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_add_acl_rule_t*)arg, tpm_add_acl_rule, sizeof(tpm_ioctl_add_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== ACL Rule Del Section ====== */
      /* ================================== */
      case MV_TPM_IOCTL_DEL_ACL_SECTION:
      {
          tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = (tpm_ioctl_del_acl_rule_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_del_acl_rule == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_del_acl_rule;

          if(copy_from_user(tpm_del_acl_rule, (tpm_ioctl_del_acl_rule_t*)arg, sizeof(tpm_ioctl_del_acl_rule_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_del_acl_rule->del_acl_cmd)
          {
              /* ====== MV_TPM_IOCTL_DEL_L2_ACL_RULE ========= */
              case MV_TPM_IOCTL_DEL_L2_ACL_RULE:

                  rcode = tpm_del_l2_rule(  tpm_del_acl_rule->owner_id,
                                            tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_DS_LOAD_BALANCE_RULE ========= */
              case MV_TPM_IOCTL_DEL_DS_LOAD_BALANCE_RULE:

                  rcode = tpm_del_ds_load_balance_rule(  tpm_del_acl_rule->owner_id,
                                            tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

		  /* ====== MV_TPM_IOCTL_DEL_MAC_LEARN_ACL_RULE ========= */
		  case MV_TPM_IOCTL_DEL_MAC_LEARN_ACL_RULE:
		      rcode = tpm_del_mac_learn_rule(tpm_del_acl_rule->owner_id,
						     &tpm_del_acl_rule->l2_key);
		      if(rcode != TPM_OK)
			goto ioctlErr;

		      if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
		      {
			printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
			goto ioctlErr;
		      }

		      ret = 0;
		      break;

              /* ====== MV_TPM_IOCTL_DEL_L3_ACL_RULE ========= */
              case MV_TPM_IOCTL_DEL_L3_ACL_RULE:

                  rcode = tpm_del_l3_type_rule(  tpm_del_acl_rule->owner_id,
                                                 tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_IPv4_ACL_RULE ========= */
              case MV_TPM_IOCTL_DEL_IPv4_ACL_RULE:

                  rcode = tpm_del_ipv4_rule(  tpm_del_acl_rule->owner_id,
                                              tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_IPv6_ACL_RULE ========= */
              case MV_TPM_IOCTL_DEL_IPV6_GEN_ACL_RULE:

                  rcode = tpm_del_ipv6_gen_rule(  tpm_del_acl_rule->owner_id,
                                                    tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_DEL_IPV6_NH_ACL_RULE:

                  rcode = tpm_del_ipv6_nh_rule(  tpm_del_acl_rule->owner_id,
                                                 tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_DEL_IPV6_DIP_ACL_RULE:

                  rcode = tpm_del_ipv6_dip_rule(  tpm_del_acl_rule->owner_id,
                                                  tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_DEL_IPV6_L4_PORTS_ACL_RULE:

                  rcode = tpm_del_ipv6_l4_ports_rule(  tpm_del_acl_rule->owner_id,
                                                       tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_IPv6_5T_RULE ========= */
              case MV_TPM_IOCTL_DEL_IPV6_GEN_5T_RULE:

                  rcode = tpm_del_ipv6_gen_5t_rule(  tpm_del_acl_rule->owner_id,
                                                     tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_DEL_IPV6_DIP_5T_RULE:

                  rcode = tpm_del_ipv6_dip_5t_rule(  tpm_del_acl_rule->owner_id,
                                                     tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_DEL_IPV6_L4_PORTS_5T_RULE:

                  rcode = tpm_del_ipv6_l4_ports_5t_rule(  tpm_del_acl_rule->owner_id,
                                                          tpm_del_acl_rule->rule_idx);

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;
	       /* ====== MV_TPM_IOCTL_DEL_CTC_CM_ACL_RULE ========= */
	       case MV_TPM_IOCTL_DEL_CTC_CM_ACL_RULE:

		   rcode = tpm_del_ctc_cm_acl_rule(tpm_del_acl_rule->owner_id,
						tpm_del_acl_rule->src_port,
						tpm_del_acl_rule->precedence);

		   if (rcode != TPM_OK)
		   	goto ioctlErr;

		   if (copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t))) {
			printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
			goto ioctlErr;
		   }

		   ret = 0;
		   break;

              /* ====== MV_TPM_IOCTL_DEL_MOD_RULE ========= */
              case MV_TPM_IOCTL_DEL_MOD_RULE:

                  rcode = tpm_mod_entry_del ( tpm_del_acl_rule->trg_port,
                                              tpm_del_acl_rule->rule_idx );
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

	          /* ====== MV_TPM_IOCTL_INV_MOD_RULES ========= */
	          case MV_TPM_IOCTL_INV_MOD_RULES:

	              rcode = tpm_mod_mac_inv ( tpm_del_acl_rule->trg_port );
	              if(rcode != TPM_OK)
	                goto ioctlErr;

	              if(copy_to_user((tpm_ioctl_del_acl_rule_t*)arg, tpm_del_acl_rule, sizeof(tpm_ioctl_del_acl_rule_t)))
	              {
	                printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
	                goto ioctlErr;
	              }

	              ret = 0;
	              break;

	            default:
	              printk(KERN_ERR "ERROR: (%s:%d) invalid cmd(%d)\n", __FUNCTION__, __LINE__, tpm_del_acl_rule->del_acl_cmd);
	              ret = -EINVAL;
	          }
	          break;
      }

      /* ================================== */
      /* ====== ACL Rule Ctrl Section ===== */
      /* ================================== */
      case MV_TPM_IOCTL_CTRL_ACL_SECTION:
      {
          tpm_ioctl_ctrl_acl_rule_t *tpm_ctrl_acl_rule = (tpm_ioctl_ctrl_acl_rule_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_ctrl_acl_rule == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_ctrl_acl_rule;

          if(copy_from_user(tpm_ctrl_acl_rule, (tpm_ioctl_ctrl_acl_rule_t*)arg, sizeof(tpm_ioctl_ctrl_acl_rule_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_ctrl_acl_rule->ctrl_acl_cmd)
          {
              /* ====== MV_TPM_IOCTL_GET_NEXT_VALID_RULE ========= */
              case MV_TPM_IOCTL_GET_NEXT_VALID_RULE:

                  rcode = tpm_get_next_valid_rule(  tpm_ctrl_acl_rule->owner_id,
                                                    tpm_ctrl_acl_rule->current_index,
                                                    tpm_ctrl_acl_rule->rule_type,
                                                  &(tpm_ctrl_acl_rule->next_index),
                                                  &(tpm_ctrl_acl_rule->rule_idx),
                                                  &(tpm_ctrl_acl_rule->tpm_rule));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_ctrl_acl_rule_t*)arg, tpm_ctrl_acl_rule, sizeof(tpm_ioctl_ctrl_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;
               /* ====== MV_TPM_IOCTL_GET_MAC_LEARN_ENTRY_NUM ========= */
               case MV_TPM_IOCTL_GET_MAC_LEARN_ENTRY_NUM:
                  rcode = tpm_mac_learn_entry_num_get(&(tpm_ctrl_acl_rule->entry_count));
                  if (rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_ctrl_acl_rule_t*)arg, tpm_ctrl_acl_rule, sizeof(tpm_ioctl_ctrl_acl_rule_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;

                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Mod Rule GET Section  ===== */
      /* ================================== */
      case MV_TPM_IOCTL_GET_MOD_SECTION:
      {
          tpm_ioctl_get_mod_rule_t *tpm_get_mod_rule = (tpm_ioctl_get_mod_rule_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_m);
          if (tpm_get_mod_rule == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_m);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_m_buff = tpm_get_mod_rule;

          if(copy_from_user(tpm_get_mod_rule, (tpm_ioctl_get_mod_rule_t*)arg, sizeof(tpm_ioctl_get_mod_rule_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_get_mod_rule->get_mod_cmd)
          {
              /* ====== MV_TPM_IOCTL_GET_MOD_RULE ========= */
              case MV_TPM_IOCTL_GET_MOD_RULE:

                  rcode = tpm_mod_entry_get (tpm_get_mod_rule->trg_port,
                                             tpm_get_mod_rule->rule_idx,
                                            &(tpm_get_mod_rule->valid_num),
                                            &(tpm_get_mod_rule->pnc_ref),
                                             tpm_get_mod_rule->rule);

                  if(copy_to_user((tpm_ioctl_get_mod_rule_t*)arg, tpm_get_mod_rule, sizeof(tpm_ioctl_get_mod_rule_t)))
                  {
                      printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                      goto ioctlErr;
                  }

                  ret = 0;
                  break;

              default:
                  ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ===== MC Stream Rule Section ===== */
      /* ================================== */
      case MV_TPM_IOCTL_MC_STREAM_SECTION:
      {
          tpm_ioctl_mc_rule_t *tpm_mc_rule = (tpm_ioctl_mc_rule_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_mc_rule == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_mc_rule;

          if(copy_from_user(tpm_mc_rule, (tpm_ioctl_mc_rule_t*)arg, sizeof(tpm_ioctl_mc_rule_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_mc_rule->mc_cmd)
          {
              /* ====== MV_TPM_IOCTL_ADD_IPv4_MC_STREAM ========= */
              case MV_TPM_IOCTL_ADD_IPv4_MC_STREAM:


                  rcode = tpm_add_ipv4_mc_stream(  tpm_mc_rule->owner_id,
                                                   tpm_mc_rule->stream_num,
                                                   tpm_mc_rule->igmp_mode,
                                                   tpm_mc_rule->mc_stream_pppoe,
                                                   tpm_mc_rule->vid,
                                                 &(tpm_mc_rule->ipv4_mc.ipv4_src_add[0]),
                                                 &(tpm_mc_rule->ipv4_mc.ipv4_dst_add[0]),
                                                   tpm_mc_rule->ipv4_mc.ignore_ipv4_src,
                                                   tpm_mc_rule->dest_port_bm);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;
              /* ====== MV_TPM_IOCTL_ADD_IPv4_MC_STREAM_SET_QUEUE ========= */
              case MV_TPM_IOCTL_ADD_IPv4_MC_STREAM_SET_QUEUE:


                  rcode = tpm_add_ipv4_mc_stream_set_queue(  tpm_mc_rule->owner_id,
                                                   tpm_mc_rule->stream_num,
                                                   tpm_mc_rule->igmp_mode,
                                                   tpm_mc_rule->mc_stream_pppoe,
                                                   tpm_mc_rule->vid,
                                                 &(tpm_mc_rule->ipv4_mc.ipv4_src_add[0]),
                                                 &(tpm_mc_rule->ipv4_mc.ipv4_dst_add[0]),
                                                   tpm_mc_rule->ipv4_mc.ignore_ipv4_src,
                                                   tpm_mc_rule->dest_queue,
                                                   tpm_mc_rule->dest_port_bm);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_MOD_IPv4_MC_STREAM ========= */
              case MV_TPM_IOCTL_MOD_IPv4_MC_STREAM:

                  rcode = tpm_updt_ipv4_mc_stream(tpm_mc_rule->owner_id,
                                                  tpm_mc_rule->stream_num,
                                                  tpm_mc_rule->dest_port_bm);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_IPv4_MC_STREAM ========= */
              case MV_TPM_IOCTL_DEL_IPv4_MC_STREAM:

                  rcode = tpm_del_ipv4_mc_stream(tpm_mc_rule->owner_id,
                                                 tpm_mc_rule->stream_num);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

	      /* ====== MV_TPM_IOCTL_ADD_IPv6_MC_STREAM ========= */
	      case MV_TPM_IOCTL_ADD_IPv6_MC_STREAM:

		      rcode = tpm_add_ipv6_mc_stream(  tpm_mc_rule->owner_id,
						       tpm_mc_rule->stream_num,
						       tpm_mc_rule->igmp_mode,
						       tpm_mc_rule->mc_stream_pppoe,
						       tpm_mc_rule->vid,
						       &(tpm_mc_rule->ipv6_mc.ipv6_src_add[0]),
						       &(tpm_mc_rule->ipv6_mc.ipv6_dst_add[0]),
						       tpm_mc_rule->ipv6_mc.ignore_ipv6_src,
						       tpm_mc_rule->dest_port_bm);
		      if(rcode != TPM_OK)
			    goto ioctlErr;

		      ret = 0;
		      break;

	      /* ====== MV_TPM_IOCTL_ADD_IPv6_MC_STREAM_SET_QUEUE ========= */
	      case MV_TPM_IOCTL_ADD_IPv6_MC_STREAM_SET_QUEUE:

		      rcode = tpm_add_ipv6_mc_stream_set_queue(tpm_mc_rule->owner_id,
						       tpm_mc_rule->stream_num,
						       tpm_mc_rule->igmp_mode,
						       tpm_mc_rule->mc_stream_pppoe,
						       tpm_mc_rule->vid,
						       &(tpm_mc_rule->ipv6_mc.ipv6_src_add[0]),
						       &(tpm_mc_rule->ipv6_mc.ipv6_dst_add[0]),
						       tpm_mc_rule->ipv6_mc.ignore_ipv6_src,
						       tpm_mc_rule->dest_queue,
						       tpm_mc_rule->dest_port_bm);
		      if(rcode != TPM_OK)
			    goto ioctlErr;

		      ret = 0;
		      break;

	      /* ====== MV_TPM_IOCTL_MOD_IPv6_MC_STREAM ========= */
	      case MV_TPM_IOCTL_MOD_IPv6_MC_STREAM:

		      rcode = tpm_updt_ipv6_mc_stream(tpm_mc_rule->owner_id,
										      tpm_mc_rule->stream_num,
										      tpm_mc_rule->dest_port_bm);
		      if(rcode != TPM_OK)
			    goto ioctlErr;

		      ret = 0;
		      break;

	      /* ====== MV_TPM_IOCTL_DEL_IPv6_MC_STREAM ========= */
	      case MV_TPM_IOCTL_DEL_IPv6_MC_STREAM:

		      rcode = tpm_del_ipv6_mc_stream(tpm_mc_rule->owner_id,
										     tpm_mc_rule->stream_num);
		      if(rcode != TPM_OK)
			    goto ioctlErr;

		      ret = 0;
		      break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Mng Rule Section ========== */
      /* ================================== */
  case MV_TPM_IOCTL_MNG_SECTION:
  {
      tpm_ioctl_mng_ch_t *tpm_mng_ch = (tpm_ioctl_mng_ch_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_mng_ch == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_mng_ch;

          if(copy_from_user(tpm_mng_ch, (tpm_ioctl_mng_ch_t*)arg, sizeof(tpm_ioctl_mng_ch_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_mng_ch->mng_cmd)
          {
              /* ====== MV_TPM_IOCTL_ADD_OMCI_CHNL ========= */
              case MV_TPM_IOCTL_ADD_OMCI_CHNL:

                  rcode = tpm_omci_add_channel(tpm_mng_ch->tpm_ioctl_omci_ch.owner_id,
                                               tpm_mng_ch->tpm_ioctl_omci_ch.gem_port,
                                               tpm_mng_ch->tpm_ioctl_omci_ch.cpu_rx_queue,
                                               tpm_mng_ch->tpm_ioctl_omci_ch.tcont_num,
                                               tpm_mng_ch->tpm_ioctl_omci_ch.cpu_tx_queue);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_OMCI_CHNL ========= */
              case MV_TPM_IOCTL_DEL_OMCI_CHNL:

                  rcode = tpm_omci_del_channel(tpm_mng_ch->tpm_ioctl_omci_ch.owner_id);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_GET_OMCI_CHNL ========= */
              case MV_TPM_IOCTL_GET_OMCI_CHNL:

                  rcode = tpm_omci_get_channel(&(tpm_mng_ch->tpm_ioctl_omci_ch.is_valid),
                                               &(tpm_mng_ch->tpm_ioctl_omci_ch.gem_port),
                                               &(tpm_mng_ch->tpm_ioctl_omci_ch.cpu_rx_queue),
                                               &(tpm_mng_ch->tpm_ioctl_omci_ch.tcont_num),
                                               &(tpm_mng_ch->tpm_ioctl_omci_ch.cpu_tx_queue));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_mng_ch_t*)arg, tpm_mng_ch, sizeof(tpm_ioctl_mng_ch_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_OAM_CHNL ========= */
              case MV_TPM_IOCTL_ADD_OAM_CHNL:

                  rcode = tpm_oam_epon_add_channel(tpm_mng_ch->tpm_ioctl_oam_ch.owner_id,
                                                   tpm_mng_ch->tpm_ioctl_oam_ch.cpu_rx_queue,
                                                   tpm_mng_ch->tpm_ioctl_oam_ch.llid_num);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_LOOP_DETECT_CHNL ========= */
              case MV_TPM_IOCTL_ADD_LOOP_DETECT_CHNL:

                  rcode = tpm_loop_detect_add_channel(tpm_mng_ch->tpm_ioctl_oam_ch.owner_id,
		  				      tpm_mng_ch->loopback_detect_ety);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_LOOP_DETECT_CHNL ========= */
              case MV_TPM_IOCTL_DEL_LOOP_DETECT_CHNL:

                  rcode = tpm_loop_detect_del_channel(tpm_mng_ch->tpm_ioctl_oam_ch.owner_id);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_OAM_CHNL ========= */
              case MV_TPM_IOCTL_DEL_OAM_CHNL:

                  rcode = tpm_oam_epon_del_channel(tpm_mng_ch->tpm_ioctl_oam_ch.owner_id);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_GET_OAM_CHNL ========= */
              case MV_TPM_IOCTL_GET_OAM_CHNL:

                  rcode = tpm_oam_epon_get_channel(&(tpm_mng_ch->tpm_ioctl_oam_ch.is_valid),
                                                   &(tpm_mng_ch->tpm_ioctl_oam_ch.cpu_rx_queue),
                                                   &(tpm_mng_ch->tpm_ioctl_oam_ch.llid_num));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_mng_ch_t*)arg, tpm_mng_ch, sizeof(tpm_ioctl_mng_ch_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_ADD_LOOP_DETECT_CHNL ========= */
              case MV_TPM_IOCTL_ADD_OAM_LOOPBACK_CHNL:

                  rcode = tpm_oam_loopback_add_channel(tpm_mng_ch->tpm_ioctl_oam_ch.owner_id);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              case MV_TPM_IOCTL_DEL_OAM_LOOPBACK_CHNL:

                  rcode = tpm_oam_loopback_del_channel(tpm_mng_ch->tpm_ioctl_oam_ch.owner_id);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
        }

      /* ================================== */
      /* ====== Switch Security Section === */
      /* ================================== */
      case MV_TPM_IOCTL_SW_SECURITY_SECTION:
      {
          tpm_ioctl_sw_mac_security_t *tpm_sw_mac_security = (tpm_ioctl_sw_mac_security_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_sw_mac_security == NULL)
          {
              printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
              ret = -ENOMEM;
              goto ioctlErr;
          }
          mpool_s_buff = tpm_sw_mac_security;

          if(copy_from_user(tpm_sw_mac_security, (tpm_ioctl_sw_mac_security_t*)arg, sizeof(tpm_ioctl_sw_mac_security_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_sw_mac_security->sw_security_cmd)
          {
              /* ====== MV_TPM_IOCTL_SW_ADD_STATIC_MAC ========= */
              case MV_TPM_IOCTL_SW_ADD_STATIC_MAC:
                  rcode = tpm_sw_add_static_mac(  tpm_sw_mac_security->owner_id,
                                                  tpm_sw_mac_security->port,
                                                &(tpm_sw_mac_security->static_mac[0]));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_DEL_STATIC_MAC ========= */
              case MV_TPM_IOCTL_SW_DEL_STATIC_MAC:
                  rcode = tpm_sw_del_static_mac(  tpm_sw_mac_security->owner_id,
                                                &(tpm_sw_mac_security->static_mac[0]));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_PORT_MAC_LIMIT ========= */
              case MV_TPM_IOCTL_SW_SET_PORT_MAC_LIMIT:
                  rcode = tpm_sw_set_port_max_macs(tpm_sw_mac_security->owner_id,
                                                   tpm_sw_mac_security->port,
                                                   tpm_sw_mac_security->mac_per_port);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_MAC_AGE_TIME ========= */
              case MV_TPM_IOCTL_SW_SET_MAC_AGE_TIME:
                  rcode = tpm_sw_set_mac_age_time(tpm_sw_mac_security->owner_id,
                                                  tpm_sw_mac_security->mac_age_time);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_GET_MAC_AGE_TIME ========= */
              case MV_TPM_IOCTL_SW_GET_MAC_AGE_TIME:
                  rcode = tpm_sw_get_mac_age_time(tpm_sw_mac_security->owner_id,
                                                 &(tpm_sw_mac_security->mac_age_time));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_mac_security_t*)arg, tpm_sw_mac_security, sizeof(tpm_ioctl_sw_mac_security_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_MAC_LEARN ========= */
              case MV_TPM_IOCTL_SW_SET_MAC_LEARN:
                  rcode = tpm_sw_set_mac_learn(tpm_sw_mac_security->owner_id,
                                               tpm_sw_mac_security->port,
                                               tpm_sw_mac_security->enable_mac_learn);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_GET_MAC_LEARN ========= */
              case MV_TPM_IOCTL_SW_GET_MAC_LEARN:
                  rcode = tpm_sw_get_mac_learn(  tpm_sw_mac_security->owner_id,
                                                 tpm_sw_mac_security->port,
                                               &(tpm_sw_mac_security->enable_mac_learn));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_mac_security_t*)arg, tpm_sw_mac_security, sizeof(tpm_ioctl_sw_mac_security_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SET_MTU_SIZE ========= */
              case MV_TPM_IOCTL_SET_MTU_SIZE:
                  rcode = tpm_set_mtu_size(tpm_sw_mac_security->owner_id,
                                           tpm_sw_mac_security->mtu_type,
                                           tpm_sw_mac_security->mtu_size);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_GET_MTU_SIZE ========= */
              case MV_TPM_IOCTL_GET_MTU_SIZE:
                  rcode = tpm_get_mtu_size(  tpm_sw_mac_security->owner_id,
                                             tpm_sw_mac_security->mtu_type,
                                           &(tpm_sw_mac_security->mtu_size));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_mac_security_t*)arg, tpm_sw_mac_security, sizeof(tpm_ioctl_sw_mac_security_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_PORT_FLOOD ========= */
              case MV_TPM_IOCTL_SW_SET_PORT_FLOOD:
                  rcode = tpm_sw_set_port_flooding(tpm_sw_mac_security->owner_id,
                                                   tpm_sw_mac_security->port,
                                                   tpm_sw_mac_security->flood_mode,
                                                   tpm_sw_mac_security->allow_flood);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_GET_PORT_FLOOD ========= */
              case MV_TPM_IOCTL_SW_GET_PORT_FLOOD:
                  rcode = tpm_sw_get_port_flooding(  tpm_sw_mac_security->owner_id,
                                                     tpm_sw_mac_security->port,
                                                     tpm_sw_mac_security->flood_mode,
                                                   &(tpm_sw_mac_security->allow_flood));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_mac_security_t*)arg, tpm_sw_mac_security, sizeof(tpm_ioctl_sw_mac_security_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_CLEAR_DYNAMIC_MAC ========= */
              case MV_TPM_IOCTL_SW_CLEAR_DYNAMIC_MAC:
                  rcode = tpm_sw_clear_dynamic_mac(tpm_sw_mac_security->owner_id);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_MIRROR ========= */
              case MV_TPM_IOCTL_SW_SET_MIRROR:
                  rcode = tpm_sw_set_port_mirror(tpm_sw_mac_security->owner_id,
                                                 tpm_sw_mac_security->port_mirror.sport,
                                                 tpm_sw_mac_security->port_mirror.dport,
                                                 tpm_sw_mac_security->port_mirror.mode,
                                                 tpm_sw_mac_security->port_mirror.enable);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;
              /* ====== MV_TPM_IOCTL_SW_SET_TRUNK_PORT ========= */
              case MV_TPM_IOCTL_SW_SET_TRUNK_PORT:
                  rcode = tpm_sw_set_trunk_ports(tpm_sw_mac_security->owner_id,
						 tpm_sw_mac_security->trunk.trunk_id,
						 tpm_sw_mac_security->trunk.trunk_mask);

		   if(rcode != TPM_OK)
		       goto ioctlErr;
		   ret = 0;
		   break;
              /* ====== MV_TPM_IOCTL_SW_SET_TRUNK_MASK ========= */
              case MV_TPM_IOCTL_SW_SET_TRUNK_MASK:
              rcode = tpm_sw_set_trunk_mask(tpm_sw_mac_security->owner_id,
                                            tpm_sw_mac_security->trunk.mask_num,
                                            tpm_sw_mac_security->trunk.trunk_mask);

              if(rcode != TPM_OK)
                  goto ioctlErr;
              ret = 0;
              break;

              /* ====== MV_TPM_IOCTL_SW_GET_MIRROR ========= */
              case MV_TPM_IOCTL_SW_GET_MIRROR:
                  rcode = tpm_sw_get_port_mirror(  tpm_sw_mac_security->owner_id,
                                                   tpm_sw_mac_security->port_mirror.sport,
                                                   tpm_sw_mac_security->port_mirror.dport,
                                                   tpm_sw_mac_security->port_mirror.mode,
                                                 &(tpm_sw_mac_security->port_mirror.enable));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_mac_security_t*)arg, tpm_sw_mac_security, sizeof(tpm_ioctl_sw_mac_security_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_ISOLATE_PORT_VECTOR ========= */
              case MV_TPM_IOCTL_SW_SET_ISOLATE_PORT_VECTOR:
                  rcode = tpm_sw_set_isolate_eth_port_vector(tpm_sw_mac_security->owner_id,
                                                             tpm_sw_mac_security->port,
                                                             tpm_sw_mac_security->port_vector);

                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

	          /* ====== MV_TPM_IOCTL_SW_GET_ISOLATE_PORT_VECTOR ========= */
              case MV_TPM_IOCTL_SW_GET_ISOLATE_PORT_VECTOR:
                  rcode = tpm_sw_get_isolate_eth_port_vector(  tpm_sw_mac_security->owner_id,
                                                               tpm_sw_mac_security->port,
                                                             &(tpm_sw_mac_security->port_vector));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_mac_security_t*)arg, tpm_sw_mac_security, sizeof(tpm_ioctl_sw_mac_security_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_STATIC_MULTICAST_MAC ========= */
              case MV_TPM_IOCTL_SW_SET_STATIC_MULTICAST_MAC:
                  rcode = tpm_sw_set_static_multicast_mac(  tpm_sw_mac_security->owner_id,
                                                  tpm_sw_mac_security->port_vector,
                                                &(tpm_sw_mac_security->static_mac[0]));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Switch Vlan Section ======= */
      /* ================================== */
      case MV_TPM_IOCTL_SW_VLAN_SECTION:
      {
        tpm_ioctl_sw_vlan_filter_t *tpm_sw_vlan_filter = (tpm_ioctl_sw_vlan_filter_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
        if (tpm_sw_vlan_filter == NULL)
        {
          printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
          ret = -ENOMEM;
          goto ioctlErr;
        }
        mpool_s_buff = tpm_sw_vlan_filter;

        if(copy_from_user(tpm_sw_vlan_filter, (tpm_ioctl_sw_vlan_filter_t*)arg, sizeof(tpm_ioctl_sw_vlan_filter_t)))
        {
          printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
          goto ioctlErr;
        }

         switch (tpm_sw_vlan_filter->sw_vlan_cmd)
        {
          /* ====== MV_TPM_IOCTL_SW_SET_PORT_TAGGED ========= */
          case MV_TPM_IOCTL_SW_SET_PORT_TAGGED:
            rcode = tpm_sw_set_port_tagged(tpm_sw_vlan_filter->owner_id,
                                           tpm_sw_vlan_filter->port,
                                           tpm_sw_vlan_filter->allow_tagged);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_SET_PORT_UNTAGGED ========= */
          case MV_TPM_IOCTL_SW_SET_PORT_UNTAGGED:
            rcode = tpm_sw_set_port_untagged(tpm_sw_vlan_filter->owner_id,
                                             tpm_sw_vlan_filter->port,
                                             tpm_sw_vlan_filter->allow_untagged);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_ADD_VID ========= */
          case MV_TPM_IOCTL_SW_PORT_ADD_VID:
            rcode = tpm_sw_port_add_vid(tpm_sw_vlan_filter->owner_id,
                                        tpm_sw_vlan_filter->port,
                                        tpm_sw_vlan_filter->vid);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_DEL_VID ========= */
          case MV_TPM_IOCTL_SW_PORT_DEL_VID:
            rcode = tpm_sw_port_del_vid(tpm_sw_vlan_filter->owner_id,
                                        tpm_sw_vlan_filter->port,
                                        tpm_sw_vlan_filter->vid);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_CLEAR_VID ========= */
          case MV_TPM_IOCTL_SW_PORT_CLEAR_VID:
            rcode = tpm_sw_clear_vid_per_port(tpm_sw_vlan_filter->owner_id,
                                              tpm_sw_vlan_filter->port);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_ADD_ALL_VID ========= */
          case MV_TPM_IOCTL_SW_PORT_ADD_ALL_VID:
            rcode = tpm_sw_add_all_vid_per_port(tpm_sw_vlan_filter->owner_id,
                                                tpm_sw_vlan_filter->port);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_ADD_VID_GROUP ========= */
          case MV_TPM_IOCTL_SW_ADD_VID_GROUP:
            rcode = tpm_sw_port_add_vid_group(tpm_sw_vlan_filter->owner_id,
                                              tpm_sw_vlan_filter->port,
                                              (uint8_t)tpm_sw_vlan_filter->egress_mode,
                                              tpm_sw_vlan_filter->min_vid,
                                              tpm_sw_vlan_filter->max_vid);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_DEL_VID_GROUP ========= */
          case MV_TPM_IOCTL_SW_DEL_VID_GROUP:
            rcode = tpm_sw_port_del_vid_group(tpm_sw_vlan_filter->owner_id,
                                              tpm_sw_vlan_filter->port,
                                              tpm_sw_vlan_filter->min_vid,
                                              tpm_sw_vlan_filter->max_vid);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_SER_VID_FILTER ========= */
          case MV_TPM_IOCTL_SW_PORT_SER_VID_FILTER:
            rcode = tpm_sw_port_set_vid_filter(tpm_sw_vlan_filter->owner_id,
                                                tpm_sw_vlan_filter->port,
                                                tpm_sw_vlan_filter->vid_filter);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_VLAN_EGRESS_MODE ========= */
          case MV_TPM_IOCTL_SW_PORT_VLAN_EGRESS_MODE:
            rcode = tpm_sw_set_port_vid_egress_mode(tpm_sw_vlan_filter->owner_id,
                                                    tpm_sw_vlan_filter->port,
                                                    tpm_sw_vlan_filter->vid,
                                                    tpm_sw_vlan_filter->egress_mode);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_SET_DEFAULT_VLAN ========= */
          case MV_TPM_IOCTL_SW_PORT_SET_DEFAULT_VLAN:
            rcode = tpm_sw_set_port_def_vlan(tpm_sw_vlan_filter->owner_id,
                                             tpm_sw_vlan_filter->port,
                                             tpm_sw_vlan_filter->vid);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_GET_DEFAULT_VLAN ========= */
          case MV_TPM_IOCTL_SW_PORT_GET_DEFAULT_VLAN:
            rcode = tpm_sw_get_port_def_vlan(tpm_sw_vlan_filter->owner_id,
                                             tpm_sw_vlan_filter->port,
                                             &(tpm_sw_vlan_filter->vid));
            if(rcode != TPM_OK)
              goto ioctlErr;

            if(copy_to_user((tpm_ioctl_sw_vlan_filter_t*)arg, tpm_sw_vlan_filter, sizeof(tpm_ioctl_sw_vlan_filter_t)))
            {
              printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
            }
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_SET_DEFAULT_PRIORITY ========= */
          case MV_TPM_IOCTL_SW_PORT_SET_DEFAULT_PRIORITY:
            rcode = tpm_sw_set_port_def_pri(tpm_sw_vlan_filter->owner_id,
                                            tpm_sw_vlan_filter->port,
                                            tpm_sw_vlan_filter->pri);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_GET_DEFAULT_PRIORITY ========= */
          case MV_TPM_IOCTL_SW_PORT_GET_DEFAULT_PRIORITY:
            rcode = tpm_sw_get_port_def_pri(tpm_sw_vlan_filter->owner_id,
                                            tpm_sw_vlan_filter->port,
                                            &(tpm_sw_vlan_filter->pri));
            if(rcode != TPM_OK)
              goto ioctlErr;

            if(copy_to_user((tpm_ioctl_sw_vlan_filter_t*)arg, tpm_sw_vlan_filter, sizeof(tpm_ioctl_sw_vlan_filter_t)))
            {
              printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
            }
            ret = 0;
            break;

          /* ====== MV_TPM_IOCTL_SW_PORT_ADD_VID_SET_EGRESS_MODE ========= */
          case MV_TPM_IOCTL_SW_PORT_ADD_VID_SET_EGRESS_MODE:
            rcode = tpm_sw_port_add_vid_set_egrs_mode(tpm_sw_vlan_filter->owner_id,
                                                      tpm_sw_vlan_filter->port,
                                                      tpm_sw_vlan_filter->vid,
                                                      tpm_sw_vlan_filter->egress_mode);
            if(rcode != TPM_OK)
              goto ioctlErr;
            ret = 0;
            break;

          default:
            ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Switch TM Section ========= */
      /* ================================== */
      case MV_TPM_IOCTL_SW_TM_SECTION:
      {
          tpm_ioctl_sw_tm_t *tpm_sw_tm = (tpm_ioctl_sw_tm_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_sw_tm == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_sw_tm;

          if(copy_from_user(tpm_sw_tm, (tpm_ioctl_sw_tm_t*)arg, sizeof(tpm_ioctl_sw_tm_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_sw_tm->sw_tm_cmd)
          {
              /* ====== MV_TPM_IOCTL_SW_SET_UNI_SCHED ========= */
              case MV_TPM_IOCTL_SW_SET_UNI_SCHED:
                  rcode = tpm_sw_set_uni_sched(tpm_sw_tm->owner_id,
                                               tpm_sw_tm->uni_port,
                                               tpm_sw_tm->sw_sched_mode);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_UNI_QUE_WEIGHT ========= */
              case MV_TPM_IOCTL_SW_UNI_QUE_WEIGHT:
                  rcode = tpm_sw_set_uni_q_weight(tpm_sw_tm->owner_id,
                                                  tpm_sw_tm->queue_id,
                                                  tpm_sw_tm->weight);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_UNI_INGR_POLICE_RATE ========= */
              case MV_TPM_IOCTL_SW_SET_UNI_INGR_POLICE_RATE:
                  rcode = tpm_sw_set_uni_ingr_police_rate(tpm_sw_tm->owner_id,
                                                          tpm_sw_tm->uni_port,
                                                          tpm_sw_tm->limit_mode,
                                                          tpm_sw_tm->cir,
                                                          tpm_sw_tm->cbs,
                                                          tpm_sw_tm->ebs);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_GET_UNI_INGR_POLICE_RATE ========= */
              case MV_TPM_IOCTL_SW_GET_UNI_INGR_POLICE_RATE:

                  rcode = tpm_sw_get_uni_ingr_police_rate(  tpm_sw_tm->owner_id,
                                                            tpm_sw_tm->uni_port,
                                                          &(tpm_sw_tm->limit_mode),
                                                          &(tpm_sw_tm->cir),
                                                          &(tpm_sw_tm->cbs),
                                                          &(tpm_sw_tm->ebs));

                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_tm_t*)arg, tpm_sw_tm, sizeof(tpm_ioctl_sw_tm_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_UNI_TC_INGR_POLICE_RATE ========= */
              case MV_TPM_IOCTL_SW_UNI_TC_INGR_POLICE_RATE:
                  rcode = tpm_sw_set_uni_tc_ingr_police_rate(tpm_sw_tm->owner_id,
                                                             tpm_sw_tm->uni_port,
                                                             tpm_sw_tm->tc,
                                                             tpm_sw_tm->cir,
                                                             tpm_sw_tm->cbs);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_SET_UNI_EGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_SW_SET_UNI_EGR_RATE_LIMIT:
                  rcode = tpm_sw_set_uni_egr_rate_limit(tpm_sw_tm->owner_id,
                                                        tpm_sw_tm->uni_port,
                                                        tpm_sw_tm->limit_mode,
                                                        tpm_sw_tm->rate_limit_val);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_GET_UNI_EGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_SW_GET_UNI_EGR_RATE_LIMIT:
                  rcode = tpm_sw_get_uni_egr_rate_limit(  tpm_sw_tm->owner_id,
                                                          tpm_sw_tm->uni_port,
                                                        &(tpm_sw_tm->limit_mode),
                                                        &(tpm_sw_tm->rate_limit_val));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_sw_tm_t*)arg, tpm_sw_tm, sizeof(tpm_ioctl_sw_tm_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Switch PHY Section ========= */
      /* ================================== */
      case MV_TPM_IOCTL_SW_PHY_SECTION:
      {
          tpm_ioctl_sw_phy_t *tpm_sw_phy = (tpm_ioctl_sw_phy_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_sw_phy == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_sw_phy;

          if(copy_from_user(tpm_sw_phy, (tpm_ioctl_sw_phy_t*)arg, sizeof(tpm_ioctl_sw_phy_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_sw_phy->sw_phy_cmd)
          {
              /* ====== MV_TPM_IOCTL_SW_PHY_SET_AUTONEG_MODE ========= */
              case MV_TPM_IOCTL_SW_PHY_SET_AUTONEG_MODE:
                  rcode = tpm_phy_set_port_autoneg_mode(tpm_sw_phy->owner_id,
                                                        tpm_sw_phy->port,
                                                        tpm_sw_phy->port_autoneg_state,
                                                        tpm_sw_phy->port_autoneg_mode);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_AUTONEG_MODE ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_AUTONEG_MODE:
                  rcode = tpm_phy_get_port_autoneg_mode(  tpm_sw_phy->owner_id,
                                                          tpm_sw_phy->port,
                                                        &(tpm_sw_phy->port_autoneg_state),
                                                        &(tpm_sw_phy->port_autoneg_mode));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_RESTART_AUTONEG ========= */
              case MV_TPM_IOCTL_SW_PHY_RESTART_AUTONEG:
                  rcode = tpm_phy_restart_port_autoneg(tpm_sw_phy->owner_id,
                                                       tpm_sw_phy->port);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_SET_PORT_STATE ========= */
              case MV_TPM_IOCTL_SW_PHY_SET_PORT_STATE:
                  rcode = tpm_phy_set_port_admin_state(tpm_sw_phy->owner_id,
                                                       tpm_sw_phy->port,
                                                       tpm_sw_phy->phy_port_state);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_PORT_STATE ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_PORT_STATE:
                  rcode = tpm_phy_get_port_admin_state(  tpm_sw_phy->owner_id,
                                                         tpm_sw_phy->port,
                                                       &(tpm_sw_phy->phy_port_state));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_LINK_STATUS ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_LINK_STATUS:
                  rcode = tpm_phy_get_port_link_status(  tpm_sw_phy->owner_id,
                                                         tpm_sw_phy->port,
                                                       &(tpm_sw_phy->port_link_status));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_DUPLEX_STATUS ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_DUPLEX_STATUS:
                  rcode = tpm_phy_get_port_duplex_status(  tpm_sw_phy->owner_id,
                                                           tpm_sw_phy->port,
                                                         &(tpm_sw_phy->port_duplex_enable));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;


              /* ====== MV_TPM_IOCTL_SW_PHY_GET_SPEED_MODE ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_SPEED_MODE:
                  rcode = tpm_phy_get_port_speed_mode(  tpm_sw_phy->owner_id,
                                                        tpm_sw_phy->port,
                                                      &(tpm_sw_phy->port_speed));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;


              /* ====== MV_TPM_IOCTL_SW_PHY_SET_PORT_FLOW_CONTROL ========= */
              case MV_TPM_IOCTL_SW_PHY_SET_PORT_FLOW_CONTROL:
                  rcode = tpm_phy_set_port_flow_control_support(tpm_sw_phy->owner_id,
                                                                tpm_sw_phy->port,
                                                                tpm_sw_phy->port_pause_state);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_PORT_FLOW_CONTROL ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_PORT_FLOW_CONTROL:
                  rcode = tpm_phy_get_port_flow_control_support(  tpm_sw_phy->owner_id,
                                                                  tpm_sw_phy->port,
                                                                &(tpm_sw_phy->port_pause_state));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_PORT_FC_STATE ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_PORT_FC_STATE:
                  rcode = tpm_phy_get_port_flow_control_state(  tpm_sw_phy->owner_id,
                                                                tpm_sw_phy->port,
                                                              &(tpm_sw_phy->port_pause_state));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_SET_PORT_SPEED ========= */
              case MV_TPM_IOCTL_SW_PHY_SET_PORT_SPEED:
                  rcode = tpm_phy_set_port_speed(tpm_sw_phy->owner_id,
                                                 tpm_sw_phy->port,
                                                 tpm_sw_phy->port_speed);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_PORT_SPEED ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_PORT_SPEED:
                  rcode = tpm_phy_get_port_speed(  tpm_sw_phy->owner_id,
                                                   tpm_sw_phy->port,
                                                 &(tpm_sw_phy->port_speed));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_SET_PORT_LOOPBACK ========= */
              case MV_TPM_IOCTL_SW_PHY_SET_PORT_LOOPBACK:
                  rcode = tpm_phy_set_port_loopback(tpm_sw_phy->owner_id,
                                                    tpm_sw_phy->port,
                                                    tpm_sw_phy->port_loopback_mode,
                                                    tpm_sw_phy->port_loopback_state);
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_PORT_LOOPBACK ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_PORT_LOOPBACK:
                  rcode = tpm_phy_get_port_loopback(  tpm_sw_phy->owner_id,
                                                      tpm_sw_phy->port,
                                                      tpm_sw_phy->port_loopback_mode,
                                                    &(tpm_sw_phy->port_loopback_state));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_SET_PORT_DUPLEX_MODE ========= */
              case MV_TPM_IOCTL_SW_PHY_SET_PORT_DUPLEX_MODE:
                  rcode = tpm_phy_set_port_duplex_mode(tpm_sw_phy->owner_id,
                                                       tpm_sw_phy->port,
                                                       tpm_sw_phy->port_duplex_enable);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_GET_PORT_DUPLEX_MODE ========= */
              case MV_TPM_IOCTL_SW_PHY_GET_PORT_DUPLEX_MODE:
                  rcode = tpm_phy_get_port_duplex_mode(  tpm_sw_phy->owner_id,
                                                         tpm_sw_phy->port,
                                                       &(tpm_sw_phy->port_duplex_enable));
                  if(rcode != TPM_OK)
                    goto ioctlErr;
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SW_PHY_CONVERT_PORT_INDEX ========= */
              case MV_TPM_IOCTL_SW_PHY_CONVERT_PORT_INDEX:
                  rcode = tpm_phy_convert_port_index( tpm_sw_phy->owner_id,
                                                      tpm_sw_phy->switch_port_id,
                                                     &(tpm_sw_phy->extern_port_id));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_UNI_2_SW_PORT ========= */
              case MV_TPM_IOCTL_UNI_2_SW_PORT:
                  rcode = tpm_xlate_uni_2_switch_port(  tpm_sw_phy->owner_id,
                                                        tpm_sw_phy->extern_port_id,
                                                        &(tpm_sw_phy->switch_port_id));
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

             default:
               ret = -EINVAL;
          }
          if (ret == 0 && (tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_AUTONEG_MODE ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_LINK_STATUS   ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_PORT_FLOW_CONTROL    ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_PORT_SPEED    ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_PORT_LOOPBACK ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_PORT_DUPLEX_MODE    ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_LINK_STATUS    ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_DUPLEX_STATUS    ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_SPEED_MODE    ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_PORT_STATE||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_GET_PORT_FC_STATE ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_SW_PHY_CONVERT_PORT_INDEX ||
                           tpm_sw_phy->sw_phy_cmd == MV_TPM_IOCTL_UNI_2_SW_PORT))
          {
              if(copy_to_user((tpm_ioctl_sw_phy_t*)arg, tpm_sw_phy, sizeof(tpm_ioctl_sw_phy_t)))
              {
                printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto ioctlErr;
              }
          }
          break;
      }

      /* ================================== */
      /* ===== Packet Processor Section === */
      /* ================================== */
      case MV_TPM_IOCTL_PP_TM_SECTION:
      {
          tpm_ioctl_tm_tm_t *tpm_tm_tm = (tpm_ioctl_tm_tm_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_tm_tm == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_tm_tm;

          if(copy_from_user(tpm_tm_tm, (tpm_ioctl_tm_tm_t*)arg, sizeof(tpm_ioctl_tm_tm_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_tm_tm->pp_tm_cmd)
          {
              /* ====== MV_TPM_IOCTL_TM_SET_WAN_EGR_QUE_SCHED ========= */
              case MV_TPM_IOCTL_TM_SET_WAN_EGR_QUE_SCHED:

                  rcode = tpm_tm_set_wan_egr_queue_sched(tpm_tm_tm->owner_id,
                                                         tpm_tm_tm->sched_ent,
                                                         tpm_tm_tm->pp_sched_mode,
                                                         tpm_tm_tm->queue_id,
                                                         tpm_tm_tm->wrr_weight);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TM_SET_WAN_SCHED_EGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_TM_SET_WAN_SCHED_EGR_RATE_LIMIT:

                  rcode = tpm_tm_set_wan_sched_egr_rate_lim(tpm_tm_tm->owner_id,
                                                            tpm_tm_tm->sched_ent,
                                                            tpm_tm_tm->rate_limit_val,
                                                            tpm_tm_tm->bucket_size);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TM_SET_WAN_QUE_EGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_TM_SET_WAN_QUE_EGR_RATE_LIMIT:

                  rcode = tpm_tm_set_wan_queue_egr_rate_lim(tpm_tm_tm->owner_id,
                                                            tpm_tm_tm->sched_ent,
                                                            tpm_tm_tm->queue_id,
                                                            tpm_tm_tm->rate_limit_val,
                                                            tpm_tm_tm->bucket_size);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TM_SET_WAN_INGR_QUE_SCHED  =========*/
              case MV_TPM_IOCTL_TM_SET_WAN_INGR_QUE_SCHED:

                rcode = tpm_tm_set_wan_ingr_queue_sched(tpm_tm_tm->owner_id,
                                                        tpm_tm_tm->pp_sched_mode,
                                                        tpm_tm_tm->queue_id,
                                                        tpm_tm_tm->wrr_weight);
                if(rcode != TPM_OK)
                  goto ioctlErr;

                ret = 0;
                break;

              /* ====== MV_TPM_IOCTL_TM_SET_WAN_INGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_TM_SET_WAN_INGR_RATE_LIMIT:

                  rcode = tpm_tm_set_wan_ingr_rate_lim(tpm_tm_tm->owner_id,
                                                       tpm_tm_tm->rate_limit_val,
                                                       tpm_tm_tm->bucket_size);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TM_SET_WAN_QUE_INGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_TM_SET_WAN_QUE_INGR_RATE_LIMIT:

                  rcode = tpm_tm_set_wan_q_ingr_rate_lim(tpm_tm_tm->owner_id,
                                                         tpm_tm_tm->queue_id,
                                                         tpm_tm_tm->rate_limit_val,
                                                         tpm_tm_tm->bucket_size);
                  if(rcode != TPM_OK)
                    goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TM_SET_GMAC0_INGR_RATE_LIMIT ========= */
              case MV_TPM_IOCTL_TM_SET_GMAC0_INGR_RATE_LIMIT:

                  rcode = tpm_tm_set_gmac0_ingr_rate_lim(tpm_tm_tm->owner_id,
                                                         tpm_tm_tm->rate_limit_val,
                                                         tpm_tm_tm->bucket_size);
                  if(rcode != TPM_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== IGMP Section ============== */
      /* ================================== */
      case MV_TPM_IOCTL_IGMP_SECTION:
      {
          tpm_ioctl_igmp_t *tpm_igmp = (tpm_ioctl_igmp_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_igmp == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_igmp;

          if(copy_from_user(tpm_igmp, (tpm_ioctl_igmp_t*)arg, sizeof(tpm_ioctl_igmp_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_igmp->igmp_cmd)
          {
              case MV_TPM_IOCTL_IGMP_SET_PORT_FRWD_MODE:
                  rcode = tpm_set_port_igmp_frwd_mode(tpm_igmp->src_port, tpm_igmp->frwd_mode);
                  if(rcode != TPM_OK)
                     goto ioctlErr;
                  ret = 0;
                  break;

              case MV_TPM_IOCTL_IGMP_GET_PORT_FRWD_MODE:
                  rcode = tpm_get_port_igmp_frwd_mode(tpm_igmp->src_port, &(tpm_igmp->frwd_mode));
                  if(rcode != TPM_OK)
                     goto ioctlErr;
                  if(copy_to_user((tpm_ioctl_igmp_t*)arg, tpm_igmp, sizeof(tpm_ioctl_igmp_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              case MV_TPM_IOCTL_IGMP_SET_CPU_RX_Q:
                  rcode = tpm_set_igmp_cpu_rx_queue(tpm_igmp->cpu_queue);
                  if(rcode != TPM_OK)
                     goto ioctlErr;
                  ret = 0;
                  break;

              case MV_TPM_IOCTL_IGMP_GET_CPU_RX_Q:
                  rcode = tpm_get_igmp_cpu_rx_queue(&(tpm_igmp->cpu_queue));
                  if(rcode != TPM_OK)
                     goto ioctlErr;
                  if(copy_to_user((tpm_ioctl_igmp_t*)arg, tpm_igmp, sizeof(tpm_ioctl_igmp_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              case MV_TPM_IOCTL_SET_IGMP_PROXY_SA_MAC:
                  rcode = tpm_set_igmp_proxy_sa_mac(tpm_igmp->sa_mac);
				  if(rcode != TPM_OK)
                     goto ioctlErr;
                  ret = 0;
                  break;

              case MV_TPM_IOCTL_GET_IGMP_PROXY_SA_MAC:
                  rcode = tpm_get_igmp_proxy_sa_mac(tpm_igmp->sa_mac);
				  if(rcode != TPM_OK)
                     goto ioctlErr;
                  if(copy_to_user((tpm_ioctl_igmp_t*)arg, tpm_igmp, sizeof(tpm_ioctl_igmp_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }
                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Print Section ============= */
      /* ================================== */
      case MV_TPM_IOCTL_PRINT_SECTION:
      {
          tpm_ioctl_print_t *tpm_print = (tpm_ioctl_print_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_print == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_print;

          if(copy_from_user(tpm_print, (tpm_ioctl_print_t*)arg, sizeof(tpm_ioctl_print_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_print->print_cmd)
          {
              /* ====== MV_TPM_IOCTL_PRINT_ETHER_PORTS ========= */
              case MV_TPM_IOCTL_PRINT_ETHER_PORTS:

                  tpm_print_etherports();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_TX_MODULES ========= */
              case MV_TPM_IOCTL_PRINT_TX_MODULES:

                  tpm_print_tx_modules();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_RX_MODULES ========= */
              case MV_TPM_IOCTL_PRINT_RX_MODULES:

                  tpm_print_rx_modules();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_GMAC_CONFIG ========= */
              case MV_TPM_IOCTL_PRINT_GMAC_CONFIG:

                  tpm_print_gmac_config();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_GMAC_FUNC ========= */
              case MV_TPM_IOCTL_PRINT_GMAC_FUNC:

                  tpm_print_gmac_func();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_IGMP ========= */
              case MV_TPM_IOCTL_PRINT_IGMP:

                  tpm_print_igmp();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_MISC ========= */
              case MV_TPM_IOCTL_PRINT_MISC:

                  tpm_print_misc();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_OWNERS ========= */
              case MV_TPM_IOCTL_PRINT_OWNERS:

                  tpm_print_owners();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_VLAN_TYPE ========= */
              case MV_TPM_IOCTL_PRINT_VLAN_TYPE:

                  tpm_print_vlan_etype();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_VALID_API_SECTIONS ========= */
              case MV_TPM_IOCTL_PRINT_VALID_API_SECTIONS:

                  tpm_print_valid_api_sections();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_FULL_API_SECTIONS ========= */
              case MV_TPM_IOCTL_PRINT_FULL_API_SECTIONS:

                  tpm_print_full_api_section(tpm_print->api_section);
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_PNC_SHADOW_RANGE ========= */
              case MV_TPM_IOCTL_PRINT_PNC_SHADOW_RANGE:

                  tpm_print_pnc_shadow_range(tpm_print->valid,
                                             tpm_print->start,
                                             tpm_print->end);
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_VALID_PNC_RANGES ========= */
              case MV_TPM_IOCTL_PRINT_VALID_PNC_RANGES:

                  tpm_print_valid_pnc_ranges();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_INIT_TABLES ========= */
              case MV_TPM_IOCTL_PRINT_INIT_TABLES:

                  tpm_print_init_tables();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_MOD2_JUMP_ALL ========= */
              case MV_TPM_IOCTL_PRINT_MOD2_JUMP_ALL:
                  tpm_mod2_print_jump_all(tpm_print->gmac);
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_MOD2_MAIN_ALL ========= */
              case MV_TPM_IOCTL_PRINT_MOD2_MAIN_ALL:
                  tpm_mod2_print_main_all(tpm_print->gmac);
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_MOD2_CFG ========= */
              case MV_TPM_IOCTL_PRINT_MOD2_CFG:
                  tpm_mod2_print_config(tpm_print->gmac);
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_PRINT_MOD2_RULE ========= */
              case MV_TPM_IOCTL_PRINT_MOD2_RULE:
                  tpm_mod2_print_rule(tpm_print->gmac,
                                      tpm_print->entry);
                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================== */
      /* ====== Trace Section ============= */
      /* ================================== */
      case MV_TPM_IOCTL_TRACE_SECTION:
      {
          tpm_ioctl_trace_t *tpm_trace = (tpm_ioctl_trace_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_trace == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_trace;

          if(copy_from_user(tpm_trace, (tpm_ioctl_trace_t*)arg, sizeof(tpm_ioctl_trace_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_trace->trace_cmd)
          {
              /* ====== MV_TPM_IOCTL_TRACE_STATUS ========= */
              case MV_TPM_IOCTL_TRACE_STATUS:

                  tpm_trace_status_print();
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TRACE_SET ========= */
              case MV_TPM_IOCTL_TRACE_SET:

                  tpm_trace_set(tpm_trace->level);
                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_TRACE_MODULE_SET ========= */
              case MV_TPM_IOCTL_TRACE_MODULE_SET:

                  tpm_trace_module_set(tpm_trace->module,
                                       tpm_trace->flag);
                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }
          break;
      }

      /* ================================ */
      /* ====== MIB Reset Section ======= */
      /* ================================ */
      case MV_TPM_IOCTL_RESET_SECTION:

          if(copy_from_user(&tpm_mib_reset_param, (tpm_ioctl_mib_reset_t*)arg, sizeof(tpm_ioctl_mib_reset_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }
          rcode = tpm_mib_reset(tpm_mib_reset_param.owner_id, tpm_mib_reset_param.reset_level);
          if(rcode != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;

      /* ================================ */
      /* ====== SET active wan Section == */
      /* ================================ */
      case MV_TPM_IOCTL_SET_ACTIVE_WAN_SECTION:
      {
          tpm_ioctl_set_active_wan_t   set_active_wan;
          if(copy_from_user(&set_active_wan, (tpm_ioctl_set_active_wan_t*)arg, sizeof(tpm_ioctl_set_active_wan_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }
          rcode = tpm_set_active_wan(set_active_wan.owner_id, set_active_wan.active_wan);
          if(rcode != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;
      }
      /* ================================ */
      /* ====== SET GMAC LPBK Section                    == */
      /* ================================ */
      case MV_TPM_IOCTL_SET_GMAC_LPBK_SECTION:
      {
          tpm_ioctl_set_gmac_loopback_t   set_gmac_lpbk;
          if(copy_from_user(&set_gmac_lpbk, (tpm_ioctl_set_gmac_loopback_t*)arg, sizeof(tpm_ioctl_set_gmac_loopback_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }
          rcode = tpm_set_gmac_loopback(set_gmac_lpbk.owner_id, set_gmac_lpbk.gmac, set_gmac_lpbk.enable);
          if(rcode != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;
      }
      /* ================================ */
      /* ====== hot swap profile Section == */
      /* ================================ */
      case MV_TPM_IOCTL_HOT_SWAP_PROFILE_SECTION:
      {
          tpm_ioctl_hot_swap_profile_t  swap_profile;
          if(copy_from_user(&swap_profile, (tpm_ioctl_hot_swap_profile_t*)arg, sizeof(tpm_ioctl_hot_swap_profile_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }
          rcode = tpm_hot_swap_profile(swap_profile.owner_id, swap_profile.profile_id);
          if(rcode != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;
      }
      /* ================================ */
      /* ====== set hwf admin Section == */
      /* ================================ */
      case MV_TPM_IOCTL_SET_PORT_HWF_ADMIN_SECTION:
      {
          tpm_ioctl_set_port_hwf_admin_t  set_port_hwf_admin;
          if(copy_from_user(&set_port_hwf_admin, (tpm_ioctl_set_port_hwf_admin_t*)arg, sizeof(tpm_ioctl_set_port_hwf_admin_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          rcode = tpm_proc_hwf_admin_set(set_port_hwf_admin.port, set_port_hwf_admin.txp, set_port_hwf_admin.enable);
          if(rcode != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;
      }
      /* ================================ */
      /* ====== ALARM  Section ======= */
      /* ================================ */
      case MV_TPM_IOCTL_ALARM_SECTION:
      {
          tpm_ioctl_alarm_t *tpm_alarm = (tpm_ioctl_alarm_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_alarm == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_alarm;

          if(copy_from_user(tpm_alarm, (tpm_ioctl_alarm_t*)arg, sizeof(tpm_ioctl_alarm_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }
          switch (tpm_alarm->alarm_cmd)
          {
              /* ====== MV_TPM_IOCTL_ALARM_GET_ETH_PORT ========= */
              case MV_TPM_IOCTL_ALARM_GET_ETH_PORT:

                  rcode = tpm_alarm_get_eth_port(tpm_alarm->owner_id, &tpm_alarm->alarm_type, &tpm_alarm->port_bitmap[0][0]);
                  if(rcode != TPM_OK)
                     goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_alarm_t*)arg, tpm_alarm, sizeof(tpm_ioctl_alarm_t)))
                  {
                    printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                    goto ioctlErr;
                  }

                  ret = 0;
                  break;

              default:
                ret = -EINVAL;
          }

          ret=0;
          break;
      }

      /* ================================= */
      /* ====== TPM PM clear Section   ======= */
      /* ================================= */
      case MV_TPM_IOCTL_COUNTER_CLEAR_SECTION:
      {
          tpm_ioctl_swport_pm_3_t *tpm_ioctl_swport_pm_3 = (tpm_ioctl_swport_pm_3_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_ioctl_swport_pm_3 == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_ioctl_swport_pm_3;

          if(copy_from_user(tpm_ioctl_swport_pm_3, (tpm_ioctl_swport_pm_3_t *)arg, sizeof(tpm_ioctl_swport_pm_3_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          rc = tpm_sw_clear_port_counter(tpm_ioctl_swport_pm_3->owner_id, tpm_ioctl_swport_pm_3->port);
          if (rc != TPM_OK)
            goto ioctlErr;

          ret=0;
          break;
      }

      /* ====================================== */
      /* ====== TPM Data Path Section   ======= */
      /* ====================================== */

      case MV_TPM_IOCTL_RX_IGMP:
      {
          tpm_ioctl_rx_igmp_t *tpm_ioctl_rx_igmp = (tpm_ioctl_rx_igmp_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_l);
          if (tpm_ioctl_rx_igmp == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_l);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_l_buff = tpm_ioctl_rx_igmp;

          tpm_ioctl_rx_igmp->owner_id = ((tpm_ioctl_rx_igmp_t*)arg)->owner_id;

          rcode = tpm_rx_igmp_frame(  tpm_ioctl_rx_igmp->owner_id,
                                    &(tpm_ioctl_rx_igmp->src_llid),
                                    &(tpm_ioctl_rx_igmp->src_port),
                                      tpm_ioctl_rx_igmp->buf,
                                    &(tpm_ioctl_rx_igmp->len));
          if (rcode != TPM_RC_OK)
            goto ioctlErr;

          ret = 0;

          if(copy_to_user((tpm_ioctl_rx_igmp_t*)arg, tpm_ioctl_rx_igmp, sizeof(tpm_ioctl_rx_igmp_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          break;
      }

     case MV_TPM_IOCTL_TX_IGMP:
     {
         tpm_ioctl_tx_igmp_t *tpm_ioctl_tx_igmp = (tpm_ioctl_tx_igmp_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_l);
         if (tpm_ioctl_tx_igmp == NULL)
         {
           printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_m);
           ret = -ENOMEM;
            goto ioctlErr;
         }
         mpool_l_buff = tpm_ioctl_tx_igmp;

        if(copy_from_user(tpm_ioctl_tx_igmp, (tpm_ioctl_tx_igmp_t *)arg, sizeof(tpm_ioctl_tx_igmp_t)))
        {
          printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
          goto ioctlErr;
        }
        rcode = tpm_tx_igmp_frame(tpm_ioctl_tx_igmp->owner_id,
                                    tpm_ioctl_tx_igmp->tgt_port,
                                    tpm_ioctl_tx_igmp->tgt_queue,
                                    tpm_ioctl_tx_igmp->gem_port,
                                    tpm_ioctl_tx_igmp->buf,
                                    tpm_ioctl_tx_igmp->len);
        if (rcode != TPM_RC_OK)
          goto ioctlErr;

        ret = 0;

        break;
      }

      /* ================================= */
      /* ====== APM PM 1 Section   ======= */
      /* ================================= */
      case MV_APM_IOCTL_PM_1_SWPORT_SECTION:
      {
          tpm_ioctl_swport_pm_1_t *tpm_ioctl_swport_pm_1 = (tpm_ioctl_swport_pm_1_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_ioctl_swport_pm_1 == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_ioctl_swport_pm_1;

          if(copy_from_user(tpm_ioctl_swport_pm_1, (tpm_ioctl_swport_pm_1_t *)arg, sizeof(tpm_ioctl_swport_pm_1_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          rcode = tpm_sw_pm_1_read(  tpm_ioctl_swport_pm_1->owner_id,
                                     tpm_ioctl_swport_pm_1->port,
                                   &(tpm_ioctl_swport_pm_1->tpm_swport_pm_1));
          if(rcode != TPM_OK)
             goto ioctlErr;

          if(copy_to_user((tpm_ioctl_swport_pm_1_t *)arg, tpm_ioctl_swport_pm_1, sizeof(tpm_ioctl_swport_pm_1_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          ret=0;
          break;
      }

      /* ================================= */
      /* ====== APM PM 3 Section   ======= */
      /* ================================= */
      case MV_APM_IOCTL_PM_3_SWPORT_SECTION:
      {
          tpm_ioctl_swport_pm_3_t *tpm_ioctl_swport_pm_3 = (tpm_ioctl_swport_pm_3_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_ioctl_swport_pm_3 == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_ioctl_swport_pm_3;

          if(copy_from_user(tpm_ioctl_swport_pm_3, (tpm_ioctl_swport_pm_3_t *)arg, sizeof(tpm_ioctl_swport_pm_3_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          rcode = tpm_sw_pm_3_read(  tpm_ioctl_swport_pm_3->owner_id,
                                     tpm_ioctl_swport_pm_3->port,
                                   &(tpm_ioctl_swport_pm_3->tpm_swport_pm_3));
          if(rcode != TPM_OK)
             goto ioctlErr;


          if(copy_to_user((tpm_ioctl_swport_pm_3_t *)arg, tpm_ioctl_swport_pm_3, sizeof(tpm_ioctl_swport_pm_3_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          ret=0;
          break;
      }

  /* ====================================== */
  /* ====== CPU WAN loopback Section  ===== */
  /* ====================================== */
      case MV_TPM_IOCTL_CPU_LPBK_SECTION:
      {
          tpm_ioctl_cpu_lpbk_t *tpm_ioctl_cpu_lpbk = (tpm_ioctl_cpu_lpbk_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_ioctl_cpu_lpbk == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_ioctl_cpu_lpbk;

          if(copy_from_user(tpm_ioctl_cpu_lpbk, (tpm_ioctl_cpu_lpbk_t*)arg, sizeof(tpm_ioctl_cpu_lpbk_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_ioctl_cpu_lpbk->cpu_lpbk_cmd)
          {
              /* ====== MV_TPM_IOCTL_ADD_CPU_WAN_LPBK ========= */
              case MV_TPM_IOCTL_ADD_CPU_WAN_LPBK:

                  rcode = tpm_add_cpu_wan_loopback (  tpm_ioctl_cpu_lpbk->owner_id,
                                                    &(tpm_ioctl_cpu_lpbk->pkt_frwd),
                                                    &(tpm_ioctl_cpu_lpbk->mod_idx));

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  if(copy_to_user((tpm_ioctl_cpu_lpbk_t*)arg, tpm_ioctl_cpu_lpbk, sizeof(tpm_ioctl_cpu_lpbk_t)))
                  {
                      printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                      goto ioctlErr;
                  }

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_DEL_CPU_WAN_LPBK ========= */
              case MV_TPM_IOCTL_DEL_CPU_WAN_LPBK:

                  rcode = tpm_del_cpu_wan_loopback (  tpm_ioctl_cpu_lpbk->owner_id,
                                                    &(tpm_ioctl_cpu_lpbk->pkt_frwd));

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              default:
                  ret = -EINVAL;
          }
          break;
      }

      /* ======================================= */
      /* ====== CPU Aging counter Section  ===== */
      /* ======================================= */
      case MV_TPM_IOCTL_AGE_COUNT_SECTION:
      {
          uint32_t  age_count_cmd;

          /* AGE_COUNT_SECTION uses two structure types, first identify the type */
          if(copy_from_user(&age_count_cmd, (uint32_t*)arg, sizeof(uint32_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          /* HIT_COUNTERS uses huge buffer pool */
          if (age_count_cmd == MV_TPM_IOCTL_GET_ALL_HIT_COUNTERS)
          {
            tpm_ioctl_pnc_hit_cnt_t *tpm_ioctl_pnc_hit_cnt =
                (tpm_ioctl_pnc_hit_cnt_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_h);
            if (tpm_ioctl_pnc_hit_cnt == NULL)
            {
                printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n",
                        __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_h);
                ret = -ENOMEM;
                goto ioctlErr;
            }
            mpool_h_buff = tpm_ioctl_pnc_hit_cnt;

            /* copy the data from user space */
            if(copy_from_user(tpm_ioctl_pnc_hit_cnt, (tpm_ioctl_pnc_hit_cnt_t*)arg, sizeof(tpm_ioctl_pnc_hit_cnt_t)))
            {
              printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
            }

            rcode = tpm_get_pnc_all_hit_counters (tpm_ioctl_pnc_hit_cnt->owner_id,
                                                  tpm_ioctl_pnc_hit_cnt->api_type,
                                                  tpm_ioctl_pnc_hit_cnt->high_thresh_pkts,
                                                  tpm_ioctl_pnc_hit_cnt->counter_reset,
                                                 &(tpm_ioctl_pnc_hit_cnt->valid_counters),
                                                 &(tpm_ioctl_pnc_hit_cnt->count_array[0]));

            if (rcode != TPM_RC_OK)
                goto ioctlErr;

            ret = 0;

            /* copy back the returned data to user space */
            if(copy_to_user((tpm_ioctl_pnc_hit_cnt_t*)arg, tpm_ioctl_pnc_hit_cnt, sizeof(tpm_ioctl_pnc_hit_cnt_t)))
            {
                printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
                goto ioctlErr;
            }
          } /* MV_TPM_IOCTL_GET_ALL_HIT_COUNTERS */
          else
          {
            tpm_ioctl_age_count_t *tpm_ioctl_age_count =
                (tpm_ioctl_age_count_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_m);
            if (tpm_ioctl_age_count == NULL)
            {
                printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n",
                        __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_m);
                ret = -ENOMEM;
                goto ioctlErr;
            }
            mpool_m_buff = tpm_ioctl_age_count;

          if(copy_from_user(tpm_ioctl_age_count, (tpm_ioctl_age_count_t*)arg, sizeof(tpm_ioctl_age_count_t)))
          {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
          }

          switch (tpm_ioctl_age_count->age_count_cmd)
          {
              /* ====== MV_TPM_IOCTL_GET_LU_ENTRIES ========= */
              case MV_TPM_IOCTL_GET_LU_ENTRIES:

                  rcode = tpm_get_pnc_lu_entry (tpm_ioctl_age_count->owner_id,
                                                tpm_ioctl_age_count->api_type,
                                                tpm_ioctl_age_count->lu_num,
                                                tpm_ioctl_age_count->lu_reset,
                                                &(tpm_ioctl_age_count->valid_num),
                                                &(tpm_ioctl_age_count->count_array[0]),
                                                &(tpm_ioctl_age_count->unrelated_num));

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SET_LU_COUNT_MASK ========= */
              case MV_TPM_IOCTL_SET_LU_COUNT_MASK:

                  rcode = tpm_set_pnc_counter_mask (tpm_ioctl_age_count->owner_id,
                                                    tpm_ioctl_age_count->api_type,
                                                    tpm_ioctl_age_count->rule_idx,
                                                    tpm_ioctl_age_count->lu_rule_mask);

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_GET_HIT_COUNT ========= */
              case MV_TPM_IOCTL_GET_HIT_COUNT:

                  rcode = tpm_get_pnc_hit_count (tpm_ioctl_age_count->owner_id,
                                                 tpm_ioctl_age_count->api_type,
                                                 tpm_ioctl_age_count->rule_idx,
                                                 tpm_ioctl_age_count->hit_reset,
                                                 &(tpm_ioctl_age_count->hit_count));

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SET_LU_THESHOLD ========= */
              case MV_TPM_IOCTL_SET_LU_THESHOLD:

                  rcode = tpm_set_pnc_lu_threshold (tpm_ioctl_age_count->owner_id,
                                                    tpm_ioctl_age_count->api_type,
                                                    tpm_ioctl_age_count->lu_thresh_pkts);

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_RESET_COUNT_GROUP ========= */
              case MV_TPM_IOCTL_RESET_COUNT_GROUP:

                  rcode = tpm_reset_pnc_age_group (tpm_ioctl_age_count->owner_id,
                                                   tpm_ioctl_age_count->api_type);

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              default:
                  ret = -EINVAL;
          }

          if (ret == 0 &&
              (tpm_ioctl_age_count->age_count_cmd == MV_TPM_IOCTL_GET_LU_ENTRIES ||
               tpm_ioctl_age_count->age_count_cmd == MV_TPM_IOCTL_GET_HIT_COUNT))
          {
            if(copy_to_user((tpm_ioctl_age_count_t*)arg, tpm_ioctl_age_count, sizeof(tpm_ioctl_age_count_t)))
            {
              printk(KERN_ERR "ERROR: (%s:%d) copy_to_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
            }
          }
        }
        break;
      } /* MV_TPM_IOCTL_AGE_COUNT_SECTION */

      /* ======================================= */
      /* ====== MC VLAN Section  ===== */
      /* ======================================= */
      case MV_TPM_IOCTL_MC_VLAN_SECTION:
      {
          tpm_ioctl_mc_vid_t *tpm_ioctl_mc_vid = (tpm_ioctl_mc_vid_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
          if (tpm_ioctl_mc_vid == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_s_buff = tpm_ioctl_mc_vid;

          if(copy_from_user(tpm_ioctl_mc_vid, (tpm_ioctl_mc_vid_t*)arg, sizeof(tpm_ioctl_mc_vid_t)))
          {
              printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
          }

          switch (tpm_ioctl_mc_vid->mc_vid_cmd)
          {
              /* ====== MV_TPM_IOCTL_SET_MC_VID_PORT_VIDS ========= */
              case MV_TPM_IOCTL_SET_MC_VID_PORT_VIDS:

                  rcode = tpm_set_mc_vid_port_vids(  tpm_ioctl_mc_vid->owner_id,
                                                     tpm_ioctl_mc_vid->mc_vid,
                                                   &(tpm_ioctl_mc_vid->port_vid_set));

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              default:
                  ret = -EINVAL;
          }

          break;
      }

      /* ======================================= */
      /* ====== MC VLAN Section  ===== */
      /* ======================================= */
      case MV_TPM_IOCTL_MTU_SECTION:
      {
          tpm_ioctl_mtu_t *tpm_ioctl_mtu = (tpm_ioctl_mtu_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_m);
          if (tpm_ioctl_mtu == NULL)
          {
            printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_m);
            ret = -ENOMEM;
            goto ioctlErr;
          }
          mpool_m_buff = tpm_ioctl_mtu;

          if(copy_from_user(tpm_ioctl_mtu, (tpm_ioctl_mtu_t*)arg, sizeof(tpm_ioctl_mtu_t)))
          {
              printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
              goto ioctlErr;
          }

          switch (tpm_ioctl_mtu->mtu_setting_cmd)
          {
              /* ====== MV_TPM_IOCTL_SET_MTU_ADMIN ========= */
              case MV_TPM_IOCTL_SET_MTU_ADMIN:

                  rcode = tpm_db_set_mtu_enable(  tpm_ioctl_mtu->enable);

                  if (rcode != TPM_DB_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;
              /* ====== MV_TPM_IOCTL_SET_MTU ========= */
              case MV_TPM_IOCTL_SET_MTU:

                  rcode = tpm_proc_set_mtu(tpm_ioctl_mtu->ethertype,
                                           tpm_ioctl_mtu->direction,
                                           tpm_ioctl_mtu->mtu);

                  if (rcode != TPM_RC_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;

              /* ====== MV_TPM_IOCTL_SET_PPPOE_MTU ========= */
              case MV_TPM_IOCTL_SET_PPPOE_MTU:

                  rcode = tpm_proc_set_pppoe_mtu(tpm_ioctl_mtu->ethertype,
                                                 tpm_ioctl_mtu->direction,
                                                 tpm_ioctl_mtu->pppoe_mtu);

                  if (rcode != TPM_DB_OK)
                      goto ioctlErr;

                  ret = 0;
                  break;
              default:
                  ret = -EINVAL;
          }

          break;
      }

  /* ====== TPM SELF CHECK Section  ===== */
  case MV_TPM_IOCTL_TPM_CHECK_SECTION:
  {
	tpm_ioctl_tpm_check_t *tpm_ioctl_tpm_check = (tpm_ioctl_tpm_check_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
	if (NULL == tpm_ioctl_tpm_check) {
		printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
		ret = -ENOMEM;
		goto ioctlErr;
	}
	mpool_s_buff = tpm_ioctl_tpm_check;
  	if(copy_from_user(tpm_ioctl_tpm_check, (tpm_ioctl_tpm_check_t*)arg, sizeof(tpm_ioctl_tpm_check_t)))
        {
            printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
            goto ioctlErr;
        }
        rcode = tpm_rule_self_check(tpm_ioctl_tpm_check->owner_id, tpm_ioctl_tpm_check->check_level);
        if(rcode != TPM_OK)
          goto ioctlErr;

        ret=0;
        break;
  }

  case MV_TPM_IOCTL_FLUSH_ATU_SECTION:
  {
	tpm_ioctl_flush_atu_t *tpm_ioctl_flush_atu = (tpm_ioctl_flush_atu_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
	if (NULL == tpm_ioctl_flush_atu) {
		printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
		ret = -ENOMEM;
		goto ioctlErr;
	}
	mpool_s_buff = tpm_ioctl_flush_atu;
	if (copy_from_user(tpm_ioctl_flush_atu, (tpm_ioctl_flush_atu_t*)arg, sizeof(tpm_ioctl_flush_atu_t))) {
		printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
		goto ioctlErr;
	}
	rcode = tpm_flush_atu(tpm_ioctl_flush_atu->owner_id, tpm_ioctl_flush_atu->flush_type, tpm_ioctl_flush_atu->db_num);
	if (rcode != TPM_OK)
          goto ioctlErr;

        ret = 0;
	break;
  }

  case MV_TPM_IOCTL_FLUSH_VTU_SECTION:
  {
	tpm_ioctl_flush_vtu_t *tpm_ioctl_flush_vtu = (tpm_ioctl_flush_vtu_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
	if (NULL == tpm_ioctl_flush_vtu) {
		printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
		ret = -ENOMEM;
		goto ioctlErr;
	}
	mpool_s_buff = tpm_ioctl_flush_vtu;
	if (copy_from_user(tpm_ioctl_flush_vtu, (tpm_ioctl_flush_vtu_t*)arg, sizeof(tpm_ioctl_flush_vtu_t))) {
		printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
		goto ioctlErr;
	}
	rcode = tpm_flush_vtu(tpm_ioctl_flush_vtu->owner_id);
	if (rcode != TPM_OK)
          goto ioctlErr;

        ret = 0;
	break;
  }

  case MV_TPM_IOCTL_SET_IPV6_CM_PARSE_WIN_SECTION:
  {
      tpm_ioctl_ipv6_parse_window_t *ipv6_parse_window = (tpm_ioctl_ipv6_parse_window_t *) tpm_common_mempool_alloc(tpm_ioctl_mpools.mpool_s);
      if (NULL == ipv6_parse_window) {
          printk(KERN_ERR "ERROR: (%s:%d) tpm_common_mempool_alloc(%p) failed\n", __FUNCTION__, __LINE__, tpm_ioctl_mpools.mpool_s);
          ret = -ENOMEM;
          goto ioctlErr;
      }
      mpool_s_buff = ipv6_parse_window;
      if (copy_from_user(ipv6_parse_window, (tpm_ioctl_ipv6_parse_window_t*)arg, sizeof(tpm_ioctl_ipv6_parse_window_t))) {
          printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
          goto ioctlErr;
      }
      rcode = tpm_ctc_cm_set_ipv6_parse_window(ipv6_parse_window->owner_id, ipv6_parse_window->ipv6_parse_window);
      if (rcode != TPM_OK)
          goto ioctlErr;

      ret = 0;
      break;
  }

  default:
        ret = -EINVAL;
  }

ioctlErr:
    if (mpool_s_buff != NULL)
        tpm_common_mempool_free(tpm_ioctl_mpools.mpool_s, mpool_s_buff);
    else if (mpool_m_buff != NULL)
        tpm_common_mempool_free(tpm_ioctl_mpools.mpool_m, mpool_m_buff);
    else if (mpool_l_buff != NULL)
        tpm_common_mempool_free(tpm_ioctl_mpools.mpool_l, mpool_l_buff);
    else if (mpool_h_buff != NULL)
        tpm_common_mempool_free(tpm_ioctl_mpools.mpool_h, mpool_h_buff);
    else if (tpm_initp != NULL)
        kfree(tpm_initp);

    return(ret);
}

/*******************************************************************************
**
**  mv_tpm_cdev_open
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function opens the TPM Char device
**
**  PARAMETERS:  struct inode *inode
**               struct file *filp
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
int mv_tpm_cdev_open(struct inode *inode, struct file *filp)
{
  tpm_module_cdev_t *dev;

  //if (deviceOpen)
  //  return(-EBUSY);

  deviceOpen++;
  try_module_get(THIS_MODULE);

  /* find the device structure */
  dev = container_of(inode->i_cdev, tpm_module_cdev_t, cdev);
  filp->private_data = dev;

  return(0);
}

/*******************************************************************************
**
**  mv_tpm_cdev_release
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function releases the TPM Char device
**
**  PARAMETERS:  struct inode *inode
**               struct file *filp
**
**  OUTPUTS:     None
**
**  RETURNS:     (0)
**
*******************************************************************************/
int mv_tpm_cdev_release(struct inode *inode, struct file *filp)
{
  deviceOpen--;
  module_put(THIS_MODULE);

  return(0);
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
/*******************************************************************************
**  mv_tpm_cdev_read
**  ___________________________________________________________________________
**
**  DESCRIPTION: The function waites for data to be written to tpm_sfs_2_ioctl_command
**               and copies it to user space to be used by IOCTLed
**
**  PARAMETERS:  struct file *file- file descriptor
**               char __user *buf-  user buffer
**               size_t count-      user buffer byte size
**
**  OUTPUTS:    loff_t *ppos-      position in file
**
**  RETURNS:    number of bytes read
**
*******************************************************************************/
static ssize_t mv_tpm_cdev_read(struct file *file, char __user *buf,
                                size_t count, loff_t *ppos)
{
    int val;

    val = down_interruptible(&tpm_sfs_2_ioctl_sem);
    if (val != 0)
    {
        return val;
    }

    if(copy_to_user(buf, &tpm_sfs_2_ioctl_command, sizeof(tpm_ioctl_mng_t)))
    {
        printk(KERN_ERR "ERROR: (%s:%d) copy_from_user failed\n", __FUNCTION__, __LINE__);
        return -EFAULT;
    }

    return sizeof(tpm_ioctl_mng_t);
}
#endif

/*******************************************************************************
**  TPM device operations
*******************************************************************************/
struct file_operations tpmCdevFops =
{
  .owner   = THIS_MODULE,
  .open    = mv_tpm_cdev_open,
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
  .read    = mv_tpm_cdev_read,
#endif
  .release = mv_tpm_cdev_release,
  .ioctl   = mv_tpm_cdev_ioctl
};

/*******************************************************************************
**  tpm_init_ioctl_mempools
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function allocates 3 memory pools for IOCTL operations
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     TPM_OK or TPM_FAIL
**
*******************************************************************************/
static int32_t tpm_init_ioctl_mempools(void)
{
    if (tpm_ioctl_mpools.mpool_s == NULL){
        uint32_t i;
        uint32_t max_val= 0;
        uint32_t struct_size[]= {
            sizeof(tpm_setup_t),                sizeof(tpm_ioctl_admin_t),          sizeof(tpm_ioctl_add_acl_rule_t),
            sizeof(tpm_ioctl_del_acl_rule_t),   sizeof(tpm_ioctl_ctrl_acl_rule_t),  sizeof(tpm_ioctl_mc_rule_t),
            sizeof(tpm_ioctl_mng_ch_t),         sizeof(tpm_ioctl_sw_mac_security_t),sizeof(tpm_ioctl_sw_vlan_filter_t),
            sizeof(tpm_ioctl_sw_tm_t),          sizeof(tpm_ioctl_sw_phy_t),         sizeof(tpm_ioctl_tm_tm_t),
            sizeof(tpm_ioctl_print_t),          sizeof(tpm_ioctl_trace_t),          sizeof(tpm_ioctl_igmp_t),
            sizeof(tpm_ioctl_alarm_t),          sizeof(tpm_ioctl_swport_pm_1_t),    sizeof(tpm_ioctl_swport_pm_3_t),
            sizeof(tpm_ioctl_cpu_lpbk_t),       sizeof(tpm_ioctl_mtu_t),            sizeof(tpm_ioctl_mc_vid_t),
            0 };

        /* find the max truct size */
        for (i=0; struct_size[i] != 0; i++){
            printk(KERN_DEBUG "struct_size[%d] = %d\n", i, struct_size[i]);
            if (struct_size[i] > max_val)
                max_val= struct_size[i];
        }

        /* validate that max_val does not exceed threashold */
        if (max_val > MPOOL_S_MAX_BUFF_SIZE){
            printk(KERN_ERR "ERROR: (%s:%d) requested buffer size is too large %d/%d (requested/max_val)\n",
                   __FUNCTION__, __LINE__, max_val, MPOOL_S_MAX_BUFF_SIZE);
            return TPM_FAIL;
        }

        tpm_ioctl_mpools.mpool_s = tpm_common_mempool_create(i, max_val, GFP_USER);
        if (NULL == tpm_ioctl_mpools.mpool_s){
            printk(KERN_ERR "ERROR: (%s:%d) failed allocating tpm_ioctl_mpools.mpool_s\n",
                   __FUNCTION__, __LINE__);
            return TPM_FAIL;
        }
        printk(KERN_INFO "(%s:%d) init tpm_ioctl_mpools.mpool_s, number_of_entries=%d, entry_size=%d pool=0x%p\n",
               __FUNCTION__, __LINE__, i, max_val, tpm_ioctl_mpools.mpool_s);
    }

    if (tpm_ioctl_mpools.mpool_m == NULL){
        uint32_t i;
        uint32_t max_val= 0;
        uint32_t struct_size[]= {
            sizeof(tpm_ioctl_get_mod_rule_t),   sizeof(tpm_ioctl_age_count_t),  0 };

        /* find the max truct size */
        for (i=0; struct_size[i] != 0; i++){
            printk(KERN_DEBUG "struct_size[%d] = %d\n", i, struct_size[i]);
            if (struct_size[i] > max_val)
                max_val= struct_size[i];
        }

        /* validate that max_val does not exceed threashold */
        if (max_val > MPOOL_M_MAX_BUFF_SIZE){
            printk(KERN_ERR "ERROR: (%s:%d) requested buffer size is too large %d/%d (requested/max_val)\n",
                   __FUNCTION__, __LINE__, max_val, MPOOL_M_MAX_BUFF_SIZE);
            return TPM_FAIL;
        }

        tpm_ioctl_mpools.mpool_m = tpm_common_mempool_create(i, max_val, GFP_USER);
        if (NULL == tpm_ioctl_mpools.mpool_m){
            printk(KERN_ERR "ERROR: (%s:%d) failed allocating tpm_ioctl_mpools.mpool_m\n",
                   __FUNCTION__, __LINE__);
            return TPM_FAIL;
        }
        printk(KERN_INFO "(%s:%d) init tpm_ioctl_mpools.mpool_m, number_of_entries=%d, entry_size=%d pool=0x%p\n",
               __FUNCTION__, __LINE__, i, max_val, tpm_ioctl_mpools.mpool_m);
    }

    if (tpm_ioctl_mpools.mpool_l == NULL){
        uint32_t i;
        uint32_t max_val= 0;
        uint32_t struct_size[]= {
            sizeof(tpm_ioctl_tx_igmp_t),        sizeof(tpm_ioctl_rx_igmp_t),    0 };

        /* find the max truct size */
        for (i=0; struct_size[i] != 0; i++){
            printk(KERN_DEBUG "struct_size[%d] = %d\n", i, struct_size[i]);
            if (struct_size[i] > max_val)
                max_val= struct_size[i];
        }

        /* validate that max_val does not exceed threashold */
        if (max_val > MPOOL_L_MAX_BUFF_SIZE){
            printk(KERN_ERR "ERROR: (%s:%d) requested buffer size is too large %d/%d (requested/max_val)\n",
                   __FUNCTION__, __LINE__, max_val, MPOOL_L_MAX_BUFF_SIZE);
            return TPM_FAIL;
        }

        tpm_ioctl_mpools.mpool_l = tpm_common_mempool_create(i, max_val, GFP_USER);
        if (NULL == tpm_ioctl_mpools.mpool_l){
            printk(KERN_ERR "ERROR: (%s:%d) failed allocating tpm_ioctl_mpools.mpool_l\n",
                   __FUNCTION__, __LINE__);
            return TPM_FAIL;
        }
        printk(KERN_INFO "(%s:%d) init tpm_ioctl_mpools.mpool_l, number_of_entries=%d, entry_size=%d pool=0x%p\n",
               __FUNCTION__, __LINE__, i, max_val, tpm_ioctl_mpools.mpool_l);
    }

    if (tpm_ioctl_mpools.mpool_h == NULL){
        uint32_t i;
        uint32_t max_val= 0;
        uint32_t struct_size[]= {
            sizeof(tpm_ioctl_pnc_hit_cnt_t),    0 };

        /* find the max truct size */
        for (i=0; struct_size[i] != 0; i++){
            printk(KERN_DEBUG "struct_size[%d] = %d\n", i, struct_size[i]);
            if (struct_size[i] > max_val)
                max_val= struct_size[i];
        }

        /* validate that max_val does not exceed threashold */
        if (max_val > MPOOL_H_MAX_BUFF_SIZE){
            printk(KERN_ERR "ERROR: (%s:%d) Huge mempool requested buffer size is too large %d/%d (requested/max_val)\n",
                   __FUNCTION__, __LINE__, max_val, MPOOL_L_MAX_BUFF_SIZE);
            return TPM_FAIL;
        }

        tpm_ioctl_mpools.mpool_h = tpm_common_mempool_create(i, max_val, GFP_USER);
        if (NULL == tpm_ioctl_mpools.mpool_h){
            printk(KERN_ERR "ERROR: (%s:%d) failed allocating tpm_ioctl_mpools.mpool_h\n",
                   __FUNCTION__, __LINE__);
            return TPM_FAIL;
        }
        printk(KERN_INFO "(%s:%d) init tpm_ioctl_mpools.mpool_h, number_of_entries=%d, entry_size=%d pool=0x%p\n",
               __FUNCTION__, __LINE__, i, max_val, tpm_ioctl_mpools.mpool_h);
    }

    return TPM_OK;
}


/*******************************************************************************
**
**  tpm_destroy_ioctl_mempools
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function destroys 3 pool memory for IOCTL operations
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     TPM_OK or TPM_FAIL
**
*******************************************************************************/
static int32_t tpm_destroy_ioctl_mempools(void)
{
      if (tpm_ioctl_mpools.mpool_s != NULL){
        tpm_common_mempool_destroy(tpm_ioctl_mpools.mpool_s);
        tpm_ioctl_mpools.mpool_s = NULL;
      }
      if (tpm_ioctl_mpools.mpool_m != NULL){
        tpm_common_mempool_destroy(tpm_ioctl_mpools.mpool_m);
        tpm_ioctl_mpools.mpool_m = NULL;
      }
      if (tpm_ioctl_mpools.mpool_l != NULL){
        tpm_common_mempool_destroy(tpm_ioctl_mpools.mpool_l);
        tpm_ioctl_mpools.mpool_l = NULL;
      }
      if (tpm_ioctl_mpools.mpool_h != NULL){
        tpm_common_mempool_destroy(tpm_ioctl_mpools.mpool_h);
        tpm_ioctl_mpools.mpool_h = NULL;
      }
      return TPM_OK;
}

/*******************************************************************************
**
**  tpm_module_mng_if_create
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function create management interface - char device
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     TPM_OK or TPM_FAIL
**
*******************************************************************************/
int32_t tpm_module_mng_if_create(void)
{
  int   rcode;
  dev_t dev;

  dev   = MKDEV(MV_TPM_MAJOR, 0);
  rcode = register_chrdev_region(dev, TPM_NUM_DEVICES, TPM_DEV_NAME);
  if(rcode < 0)
  {
    printk(KERN_ERR "ERROR: (%s:%d) Pon Char Device\n", __FUNCTION__, __LINE__);
    return(TPM_FAIL);
  }
  /* initialize ioctl mempools */
  if (tpm_init_ioctl_mempools() != TPM_OK)
    return(TPM_FAIL);

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
  /* initialize ioctl debug semaphore, used for signalling between user space and sysfs  */
  sema_init(&tpm_sfs_2_ioctl_sem, 0);
#endif

  cdev_init(&tpm_dev.cdev, &tpmCdevFops);
  tpm_dev.cdev.owner = THIS_MODULE;
  tpm_dev.cdev.ops   = &tpmCdevFops;

  rcode = cdev_add(&tpm_dev.cdev, dev, 1);
  if(rcode < 0)
  {
    printk(KERN_ERR "ERROR: (%s:%d) Gpon Char Device Add\n", __FUNCTION__, __LINE__);

    cdev_del(&tpm_dev.cdev);
    unregister_chrdev_region(dev, TPM_NUM_DEVICES);

    return(TPM_FAIL);
  }

  /* create device for udev */
  tpm_udev_class = class_create(THIS_MODULE, TPM_DEV_NAME);
  #if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
    tpm_udev_dev   = device_create(tpm_udev_class, NULL, dev, TPM_DEV_NAME);
  #else
      tpm_udev_dev   = device_create(tpm_udev_class, NULL, dev, NULL, TPM_DEV_NAME);
  #endif
  return(TPM_OK);
}

/*******************************************************************************
**
**  tpm_module_mng_if_release
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function release management interface - char device
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     TPM_OK or TPM_FAIL
**
*******************************************************************************/
int32_t tpm_module_mng_if_release(void)
{
  dev_t dev = MKDEV(MV_TPM_MAJOR, 0);

  if (tpm_destroy_ioctl_mempools() != TPM_OK)
    return(TPM_FAIL);

  device_destroy(tpm_udev_class, dev);
  class_unregister(tpm_udev_class);
  class_destroy(tpm_udev_class);

  unregister_chrdev_region(dev, TPM_NUM_DEVICES);

  return(MV_OK);
}


EXPORT_SYMBOL(tpm_sw_add_static_mac);
EXPORT_SYMBOL(tpm_sw_del_static_mac);
EXPORT_SYMBOL(tpm_sw_set_port_max_macs);
EXPORT_SYMBOL(tpm_sw_set_mac_age_time);
EXPORT_SYMBOL(tpm_sw_get_mac_age_time);
EXPORT_SYMBOL(tpm_sw_set_mac_learn);
EXPORT_SYMBOL(tpm_sw_get_mac_learn);
EXPORT_SYMBOL(tpm_set_mtu_size);
EXPORT_SYMBOL(tpm_get_mtu_size);
//EXPORT_SYMBOL(tpm_sw_set_port_flooding);
//EXPORT_SYMBOL(tpm_sw_get_port_flooding);
EXPORT_SYMBOL(tpm_sw_clear_dynamic_mac);
EXPORT_SYMBOL(tpm_sw_set_port_mirror);
EXPORT_SYMBOL(tpm_sw_get_port_mirror);
EXPORT_SYMBOL(tpm_sw_set_isolate_eth_port_vector);
EXPORT_SYMBOL(tpm_sw_get_isolate_eth_port_vector);
EXPORT_SYMBOL(tpm_sw_set_static_multicast_mac);
EXPORT_SYMBOL(tpm_sw_set_port_tagged);
EXPORT_SYMBOL(tpm_sw_set_port_untagged);
EXPORT_SYMBOL(tpm_sw_set_port_def_vlan);
EXPORT_SYMBOL(tpm_sw_get_port_def_vlan);
EXPORT_SYMBOL(tpm_sw_set_port_def_pri);
EXPORT_SYMBOL(tpm_sw_get_port_def_pri);
EXPORT_SYMBOL(tpm_sw_port_add_vid);
EXPORT_SYMBOL(tpm_sw_clear_vid_per_port);
EXPORT_SYMBOL(tpm_sw_add_all_vid_per_port);
EXPORT_SYMBOL(tpm_sw_port_add_vid_group);
EXPORT_SYMBOL(tpm_sw_port_del_vid_group);
EXPORT_SYMBOL(tpm_sw_set_port_vid_egress_mode);
EXPORT_SYMBOL(tpm_sw_port_del_vid);
EXPORT_SYMBOL(tpm_sw_port_set_vid_filter);
EXPORT_SYMBOL(tpm_sw_set_uni_sched);
EXPORT_SYMBOL(tpm_sw_set_uni_q_weight);
EXPORT_SYMBOL(tpm_sw_set_uni_ingr_police_rate);
EXPORT_SYMBOL(tpm_sw_get_uni_ingr_police_rate);
EXPORT_SYMBOL(tpm_sw_set_uni_tc_ingr_police_rate);
EXPORT_SYMBOL(tpm_sw_set_uni_egr_rate_limit);
EXPORT_SYMBOL(tpm_sw_get_uni_egr_rate_limit);
EXPORT_SYMBOL(tpm_phy_convert_port_index);
EXPORT_SYMBOL(tpm_phy_set_port_autoneg_mode);
EXPORT_SYMBOL(tpm_phy_get_port_autoneg_mode);
EXPORT_SYMBOL(tpm_phy_restart_port_autoneg);
EXPORT_SYMBOL(tpm_phy_set_port_admin_state);
EXPORT_SYMBOL(tpm_phy_get_port_admin_state);
EXPORT_SYMBOL(tpm_phy_set_port_flow_control_support);
EXPORT_SYMBOL(tpm_phy_get_port_flow_control_support);
EXPORT_SYMBOL(tpm_phy_get_port_flow_control_state);
EXPORT_SYMBOL(tpm_phy_get_port_link_status);
EXPORT_SYMBOL(tpm_phy_get_port_duplex_status);
EXPORT_SYMBOL(tpm_phy_get_port_speed_mode);
EXPORT_SYMBOL(tpm_phy_set_port_loopback);
EXPORT_SYMBOL(tpm_phy_get_port_loopback);
EXPORT_SYMBOL(tpm_phy_set_port_duplex_mode);
EXPORT_SYMBOL(tpm_phy_get_port_duplex_mode);
EXPORT_SYMBOL(tpm_phy_set_port_speed);
EXPORT_SYMBOL(tpm_phy_get_port_speed);
EXPORT_SYMBOL(tpm_alarm_get_eth_port);
EXPORT_SYMBOL(tpm_sw_pm_1_read);
EXPORT_SYMBOL(tpm_sw_pm_3_read);
