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
* tpm_usr_tpmsysfs_help.h
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
#ifndef _TPM_USR_TPM_SYSFS_HELP_H_
#define _TPM_USR_TPM_SYSFS_HELP_H_

#ifdef CONFIG_MV_TPM_SYSFS_HELP
extern int sfs_tpm_cfg_index                (char *buf);
extern int sfs_help_frwd_rule_cfg           (char *buf);
extern int sfs_help_vlan_rule_cfg           (char *buf);
extern int sfs_help_mod_rule_cfg            (char *buf);
extern int sfs_help_l2_key_cfg              (char *buf);
extern int sfs_help_l3_key_cfg              (char *buf);
extern int sfs_help_ipv4_key_cfg            (char *buf);
extern int sfs_help_mtu_cfg                 (char *buf);
extern int sfs_help_ipv6_gen_key_cfg        (char *buf);
extern int sfs_help_rule_table_display      (char *buf);
extern int sfs_help_ipv6_dip_key_cfg        (char *buf);
extern int sfs_help_ipv6_l4_key_cfg         (char *buf);
extern int sfs_help_ipv6_ctc_cm_key_cfg     (char *buf);
extern int sfs_help_ds_load_balance_rule    (char* buf);
extern int sfs_help_key_rule_delete         (char *buf);
extern int sfs_help_l2_rule_add             (char *buf);
extern int sfs_help_l3_rule_add             (char *buf);
extern int sfs_help_ipv4_rule_add           (char *buf);
extern int sfs_help_ipv6_gen_rule_add       (char *buf);
extern int sfs_help_ipv6_nh_rule_add        (char *buf);
extern int sfs_help_mc_ipvx_stream_add      (char *buf);
extern int sfs_help_igmp_cfg                (char *buf);
extern int sfs_help_no_rule_add             (char *buf);
extern int sfs_help_no_mc_stream_add        (char *buf);
extern int sfs_help_oam_omci_channel        (char *buf);
extern int sfs_help_no_oam_omci_channel     (char *buf);
extern int sfs_help_setup                   (char *buf);
extern int sfs_help_mib_reset               (char *buf);
extern int sfs_help_set_active_wan          (char *buf);
extern int sfs_help_hot_swap_profile    (char* buf);
extern int sfs_help_set_port_hwf_admin(char* buf);
extern int sfs_help_cfg_cpu_lpbk            (char *buf);
extern int sfs_help_cfg_age_count           (char *buf);
extern int sfs_help_rate_limit              (char *buf);
extern int sfs_help_pkt_mod_add             (char *buf);
extern int sfs_help_pkt_mod_get_del_purge   (char *buf);

/* jinghuaxxxxxx */
extern int sys_help_ipv6_dip_acl_rule_add   (char *buf);
//extern int sys_help_del_ipv6_dip_acl_rule (char *buf);
extern int sfs_help_ipv6_l4_ports_rule_add  (char *buf);

extern int sfs_help_delete_entry_rule_table (char *buf);
extern int sfs_help_send_genquery_to_uni    (char *buf);

//extern int sys_help_del_ipv6_l4_ports_acl_rule  (char *buf);
/* jinghuaxxxxxx */

extern int sfs_help_ipv6_gen_5t_rule_add     (char *buf);
extern int sfs_help_ipv6_dip_5t_rule_add     (char *buf);
extern int sfs_help_ipv6_l4_ports_5t_rule_add(char *buf);
extern int sfs_help_ctc_cm_set_ipv6_parse_win(char *buf);

extern int sfs_help_ctc_cm_rule_add         (char *buf);

extern int sfs_help_erase_section           (char *buf);
extern int sfs_help_tpm_self_check          (char* buf);
extern int sfs_help_flush_vtu               (char *buf);
extern int sfs_help_flush_atu               (char *buf);
extern int sfs_help_mac_learn_rule_add      (char *buf);
extern int sfs_help_mac_learn_def_act_set   (char* buf);
#else

#define  sfs_tpm_cfg_index                NULL
#define  sfs_help_frwd_rule_cfg           NULL
#define  sfs_help_vlan_rule_cfg           NULL
#define  sfs_help_mod_rule_cfg            NULL
#define  sfs_help_l2_key_cfg              NULL
#define  sfs_help_l3_key_cfg              NULL
#define  sfs_help_ipv4_key_cfg            NULL
#define  sfs_help_mtu_cfg                 NULL
#define  sfs_help_ipv6_gen_key_cfg        NULL
#define  sfs_help_rule_table_display      NULL
#define  sfs_help_ipv6_ctc_cm_key_cfg     NULL
#define  sfs_help_ipv6_dip_key_cfg        NULL
#define  sfs_help_ipv6_l4_key_cfg         NULL
#define  sfs_help_key_rule_delete         NULL
#define  sfs_help_l2_rule_add             NULL
#define  sfs_help_l3_rule_add             NULL
#define  sfs_help_ipv4_rule_add           NULL
#define  sfs_help_ipv6_gen_rule_add       NULL
#define  sfs_help_ipv6_nh_rule_add        NULL
#define  sfs_help_mc_ipvx_stream_add      NULL
#define  sfs_help_igmp_cfg                NULL
#define  sfs_help_no_rule_add             NULL
#define  sfs_help_no_mc_stream_add        NULL
#define  sfs_help_oam_omci_channel        NULL
#define  sfs_help_no_oam_omci_channel     NULL
#define  sfs_help_setup                   NULL
#define  sfs_help_mib_reset               NULL
#define  sfs_help_set_active_wan          NULL
#define  sfs_help_hot_swap_profile        NULL
#define  sfs_help_set_port_hwf_admin      NULL
#define  sfs_help_erase_section           NULL
#define  sfs_help_cfg_cpu_lpbk            NULL
#define  sfs_help_cfg_age_count           NULL
#define  sfs_help_rate_limit              NULL
#define  sfs_help_pkt_mod_add             NULL
#define  sfs_help_pkt_mod_get_del_purge   NULL
#define  sys_help_ipv6_dip_acl_rule_add   NULL
#define  sfs_help_ipv6_l4_ports_rule_add  NULL
#define  sfs_help_delete_entry_rule_table NULL
#define  sfs_help_send_genquery_to_uni    NULL
#define  sfs_help_tpm_self_check          NULL


#define  sfs_help_ipv6_gen_5t_rule_add       NULL
#define  sfs_help_ipv6_dip_5t_rule_add       NULL
#define  sfs_help_ipv6_l4_ports_5t_rule_add  NULL
#define  sfs_help_ctc_cm_set_ipv6_parse_win  NULL

#define sfs_help_ctc_cm_rule_add          NULL
#define sfs_help_flush_vtu           NULL
#define sfs_help_flush_atu           NULL
#define sfs_help_mac_learn_rule_add       NULL
#define sfs_help_mac_learn_def_act_set    NULL
#define sfs_help_ds_load_balance_rule     NULL


#endif /* CONFIG_MV_TPM_SYSFS_HELP */
#endif  /* _TPM_USR_TPM_SYSFS_HELP_H_*/
