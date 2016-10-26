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
* tpm_usr_tpmsysfs_db.h
*
* DESCRIPTION:
*               Traffic Processor Manager = TPM
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   Zeev
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               $Revision: 1.1.1.1 $
*
*
*******************************************************************************/
#ifndef _TPM_USR_TPM_SYSFS_HWCALL_H_
#define _TPM_USR_TPM_SYSFS_HWCALL_H_

extern void sfs_tpm_cfg_set_l2_rule_add                  (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l3_rule_add                  (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv4_rule_add                (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_gen_rule_add            (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_ipv4_stream_add           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_ipv4_stream_set_queue_add           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_ipv4_stream_update        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_ipv6_stream_add           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_ipv6_stream_set_queue_add           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_ipv6_stream_update        (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_igmp_port_forward_mode_cfg   (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_igmp_cpu_queue_cfg           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_igmp_proxy_sa_mac            (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_l2               (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_l3               (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv4             (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_no_mc_stream_add_ipv6        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_mc_stream_add_ipv4        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_oam_channel                  (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_omci_channel                 (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_oam_channel               (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_omci_channel              (const char *buf, size_t len);
extern void sfs_tpm_cfg_setup                            (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mib_reset                    (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_active_wan                   (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_port_hwf_admin        (const char *buf, size_t len);
extern void sfs_tpm_cfg_hot_swap_profile            (const char *buf, size_t len);
extern void sfs_tpm_cfg_add_cpu_lpbk                     (const char *buf, size_t len);
extern void sfs_tpm_cfg_del_cpu_lpbk                     (const char *buf, size_t len);
extern void sfs_tpm_cfg_dump_cpu_lpbk                    (const char *buf, size_t len);
extern void sfs_tpm_cfg_get_lu_entry                     (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_count_mask                   (const char *buf, size_t len);
extern void sfs_tpm_cfg_get_hit_count                    (const char *buf, size_t len);
extern void sfs_tpm_cfg_get_pnc_all_hit_counters         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_lu_thresh                    (const char *buf, size_t len);
extern void sfs_tpm_cfg_reset_age_group                  (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_rate_limit_queue_set         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_scheduling_mode_queue_set    (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_rate_limit_if_set            (const char *buf, size_t len);

/* jinghuaxxxxxx */
extern void sfs_tpm_cfg_set_ipv6_dip_key_rule            (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_l4_key_rule             (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_dip_acl_rule_add        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv6_dip         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_l4_ports_acl_rule_add   (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_acl(const char *buf, size_t len);
/* jinghuaxxxxxx */

extern void sfs_tpm_cfg_set_ipv6_gen_5t_rule_add         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv6_gen_5t      (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_dip_5t_rule_add         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv6_dip_5t      (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_l4_ports_5t_rule_add    (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_5t (const char *buf, size_t len);

// zeev
extern void sfs_tpm_cfg_set_ipv6_nh_acl_rule_add         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ipv6_nh_acl      (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_ctc_cm_rule_add              (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ctc_cm_acl       (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ctc_cm_ipv6_rule_add         (const char *buf, size_t len);
extern void sfs_tpm_cfg_ctc_cm_set_ipv6_parse_win        (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_no_rule_add_ipv6_gen_acl     (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_mtu_enable                   (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mtu                          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_pppoe_mtu                    (const char *buf, size_t len);


extern void sfs_tpm_cfg_set_mc_vid_key_reset             (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_vid_key_set               (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mc_vid_cfg_set               (const char *buf, size_t len);
extern void sfs_tpm_cfg_send_genquery_to_uni             (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_erase_section                (const char *buf, size_t len);
extern void sfs_tpm_rule_self_check                      (const char *buf, size_t len);
extern void sfs_tpm_cfg_flush_atu                        (const char *buf, size_t len);
extern void sfs_tpm_cfg_flush_vtu                        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mac_learn_rule_add           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_mac_learn        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mac_learn_default_rule_action(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_gmac0_ingr_rate_limit        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ds_load_balance_rule_add     (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_no_rule_add_ds_load_balance  (const char *buf, size_t len);

/* KostaP */
extern void sfs_tpm_cfg_set_fc(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_fc_period(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_fc_oneshot(const char *buf, size_t len);
extern void sfs_tpm_cfg_fc_enable(const char *buf, size_t len);

extern void sfs_tpm_drop_pre_set(const char *buf, size_t len);
extern void sfs_tpm_drop_pre_show(const char *buf, size_t len);

extern char *ipv6_gen_key_empty_name;
extern char *ipv6_dip_key_empty_name;
extern char *ipv6_l4_key_empty_name;

#endif  /* _TPM_USR_TPM_SYSFS_HWCALL_H_*/
