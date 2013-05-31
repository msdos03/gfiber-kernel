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

#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_sysfs_help.h"
#include "tpm_sysfs_rule_db.h"
#include "tpm_sysfs_hwcall.h"
#include "tpm_sysfs_utils.h"
#include "tpm_sysfs_setup.h"


/******************************************************************************/
/* ========================================================================== */
/*             TPM CFG SYS FS STORE ROUTINE SWITCHER                          */
/* ========================================================================== */

typedef struct
{
    char *sysfs_name;
    void (*sysfs_func)( const char *buf, size_t len);
} store_sysfs_name_func_t;

static store_sysfs_name_func_t store_sysfs_name_func_ara[] =
{
    {"frwd_rule_set",                      sfs_tpm_cfg_set_frwd_rule},
    {"vlan_rule_set",                      sfs_tpm_cfg_set_vlan_rule},
    {"mod_mh_rule_set",                    sfs_tpm_cfg_set_mod_mh_rule},
    {"mod_vlan_rule_set",                  sfs_tpm_cfg_set_mod_vlan_rule},
    {"mod_ipv4_addr_rule_set",             sfs_tpm_cfg_set_mod_ipv4_addr_rule},
    {"mod_ipv4_port_rule_set",             sfs_tpm_cfg_set_mod_ipv4_port_rule},
    {"mod_ipv6_addr_rule_set",             sfs_tpm_cfg_set_mod_ipv6_addr_rule},
    {"mod_ipv6_port_rule_set",             sfs_tpm_cfg_set_mod_ipv6_port_rule},
    {"mod_mac_addr_rule_set",              sfs_tpm_cfg_set_mod_mac_addr_rule},
    {"mod_pppoe_rule_set",                 sfs_tpm_cfg_set_mod_pppoe_rule},
    {"l2_key_ethertype_rule_set",          sfs_tpm_cfg_set_l2_key_ethertype_rule},
    {"l2_key_gemport_rule_set",            sfs_tpm_cfg_set_l2_key_gemport_rule},
    {"l2_key_mac_addr_rule_set",           sfs_tpm_cfg_set_l2_key_mac_addr_rule},
    {"l2_key_pppoe_rule_set",              sfs_tpm_cfg_set_l2_key_pppoe_rule},
    {"l2_key_vlan_rule_set",               sfs_tpm_cfg_set_l2_key_vlan_rule},
    {"l3_key_ethertype_rule_set",          sfs_tpm_cfg_set_l3_key_ethertype_rule},
    {"l3_key_pppoe_rule_set",              sfs_tpm_cfg_set_l3_key_pppoe_rule},
    {"ipv4_key_addr_rule_set",             sfs_tpm_cfg_set_ipv4_key_addr_rule},
    {"ipv4_key_port_rule_set",             sfs_tpm_cfg_set_ipv4_key_port_rule},
    {"ipv4_key_proto_rule_set",            sfs_tpm_cfg_set_ipv4_key_protocol_rule},
    {"ipv4_key_dscp_rule_set",             sfs_tpm_cfg_set_ipv4_key_dscp_rule},
    {"ipv6_ctc_cm_key_rule_set",           sfs_tpm_cfg_set_ipv6_ctc_cm_key_rule},
    {"ipv6_gen_key_sip_rule_set",          sfs_tpm_cfg_set_ipv6_gen_key_sip_rule},
    {"ipv6_gen_key_dscp_rule_set",         sfs_tpm_cfg_set_ipv6_gen_key_dscp_rule},
    {"key_rule_delete",                    sfs_tpm_cfg_set_key_rule_delete},
    {"rule_table_display",                 sfs_tpm_cfg_set_rule_table_display},
    {"l2_rule_add",                        sfs_tpm_cfg_set_l2_rule_add},
    {"l3_rule_add",                        sfs_tpm_cfg_set_l3_rule_add},
    {"ipv4_rule_add",                      sfs_tpm_cfg_set_ipv4_rule_add},
    {"ipv6_gen_rule_add",                  sfs_tpm_cfg_set_ipv6_gen_rule_add},
    {"mc_ipv4_stream_add",                 sfs_tpm_cfg_set_mc_ipv4_stream_add},
    {"mc_ipv4_stream_set_queue_add",	   sfs_tpm_cfg_set_mc_ipv4_stream_set_queue_add},
    {"mc_ipv4_stream_update",              sfs_tpm_cfg_set_mc_ipv4_stream_update},
    {"mc_ipv6_stream_add",		   sfs_tpm_cfg_set_mc_ipv6_stream_add},
    {"mc_ipv6_stream_set_queue_add",	   sfs_tpm_cfg_set_mc_ipv6_stream_set_queue_add},
    {"mc_ipv6_stream_update",		   sfs_tpm_cfg_set_mc_ipv6_stream_update},
    {"igmp_port_forward_mode_cfg",         sfs_tpm_cfg_set_igmp_port_forward_mode_cfg},
    {"mc_vid_key_reset",                   sfs_tpm_cfg_set_mc_vid_key_reset},
    {"mc_vid_key_set",                     sfs_tpm_cfg_set_mc_vid_key_set},
    {"mc_vid_cfg_set",                     sfs_tpm_cfg_set_mc_vid_cfg_set},
    {"igmp_cpu_queue_cfg",                 sfs_tpm_cfg_set_igmp_cpu_queue_cfg},
    {"igmp_proxy_sa_mac",                  sfs_tpm_cfg_set_igmp_proxy_sa_mac},
    {"no_rule_add_l2",                     sfs_tpm_cfg_set_no_rule_add_l2},
    {"no_rule_add_l3",                     sfs_tpm_cfg_set_no_rule_add_l3},
    {"no_rule_add_ipv4",                   sfs_tpm_cfg_set_no_rule_add_ipv4},
	{"no_mc_stream_add_ipv6",			   sfs_tpm_cfg_set_no_mc_stream_add_ipv6},
    {"no_mc_stream_add_ipv4",              sfs_tpm_cfg_set_no_mc_stream_add_ipv4},
    {"oam_channel",                        sfs_tpm_cfg_set_oam_channel},
    {"omci_channel",                       sfs_tpm_cfg_set_omci_channel},
    {"no_oam_channel",                     sfs_tpm_cfg_set_no_oam_channel},
    {"no_omci_channel",                    sfs_tpm_cfg_set_no_omci_channel},
    {"tpm_setup",                          sfs_tpm_cfg_setup},
    {"mib_reset",                          sfs_tpm_cfg_set_mib_reset},
    {"set_active_wan",                     sfs_tpm_cfg_set_active_wan},
    {"hot_swap_profile",		   sfs_tpm_cfg_hot_swap_profile},
    {"set_port_hwf_admin",		   sfs_tpm_cfg_set_port_hwf_admin},
    {"erase_section",                      sfs_tpm_cfg_set_erase_section},
    {"add_cpu_lpbk",                       sfs_tpm_cfg_add_cpu_lpbk},
    {"del_cpu_lpbk",                       sfs_tpm_cfg_del_cpu_lpbk},
    {"dump_cpu_lpbk",                      sfs_tpm_cfg_dump_cpu_lpbk},
    {"get_lu_entry",                       sfs_tpm_cfg_get_lu_entry},
    {"set_count_mask",                     sfs_tpm_cfg_set_count_mask},
    {"get_hit_count",                      sfs_tpm_cfg_get_hit_count},
    {"get_pnc_all_hit_cntrs",              sfs_tpm_cfg_get_pnc_all_hit_counters},
    {"set_lu_thresh",                      sfs_tpm_cfg_set_lu_thresh},
    {"reset_age_group",                    sfs_tpm_cfg_reset_age_group},
    {"rate_limit_queue_set",               sfs_tpm_cfg_set_rate_limit_queue_set},
    {"scheduling_mode_queue_set",          sfs_tpm_cfg_set_scheduling_mode_queue_set},
    {"rate_limit_if_set",                  sfs_tpm_cfg_set_rate_limit_if_set},
    {"pkt_mod_eng_entry_add",              sfs_tpm_cfg_set_pkt_mod_eng_entry_add},
    {"pkt_mod_eng_entry_show",             sfs_tpm_cfg_set_pkt_mod_eng_entry_show},
    {"pkt_mod_eng_entry_del",              sfs_tpm_cfg_set_pkt_mod_eng_entry_del},
    {"pkt_mod_eng_purge",                  sfs_tpm_cfg_set_pkt_mod_eng_purge},
    {"delete_entry_l2_rule_table",         sfs_tpm_cfg_set_delete_entry_l2_rule_table},
    {"delete_entry_l3_rule_table",         sfs_tpm_cfg_set_delete_entry_l3_rule_table},
    {"delete_entry_ipv4_rule_table",       sfs_tpm_cfg_set_delete_entry_ipv4_rule_table},
    {"delete_entry_ipv6_gen_rule_table",   sfs_tpm_cfg_set_delete_entry_ipv6_gen_rule_table},
    {"delete_entry_ipv6_dip_rule_table",   sfs_tpm_cfg_set_delete_entry_ipv6_dip_rule_table},
    {"delete_entry_ipv6_l4_rule_table",    sfs_tpm_cfg_set_delete_entry_ipv6_l4_rule_table},
    {"delete_entry_frwd_rule_table",       sfs_tpm_cfg_set_delete_entry_frwd_rule_table},
    {"delete_entry_vlan_rule_table",       sfs_tpm_cfg_set_delete_entry_vlan_rule_table},
    {"delete_entry_mod_rule_table",        sfs_tpm_cfg_set_delete_entry_mod_rule_table},

    /* jinghuaxxxxxx */
    {"ipv6_dip_key_rule_set",              sfs_tpm_cfg_set_ipv6_dip_key_rule},
    {"ipv6_l4_key_rule_set",               sfs_tpm_cfg_set_ipv6_l4_key_rule},
    {"ipv6_dip_rule_add",                  sfs_tpm_cfg_set_ipv6_dip_acl_rule_add},
    {"no_rule_add_ipv6_dip_acl",           sfs_tpm_cfg_set_no_rule_add_ipv6_dip},
    {"ipv6_l4_ports_acl_rule_add",         sfs_tpm_cfg_set_ipv6_l4_ports_acl_rule_add},
    {"no_rule_add_ipv6_l4_ports_acl",      sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_acl},

    {"ipv6_gen_5t_rule_add",               sfs_tpm_cfg_set_ipv6_gen_5t_rule_add},
    {"no_rule_add_ipv6_gen_5t",            sfs_tpm_cfg_set_no_rule_add_ipv6_gen_5t},
    {"ipv6_dip_5t_rule_add",               sfs_tpm_cfg_set_ipv6_dip_5t_rule_add},
    {"no_rule_add_ipv6_dip_5t",            sfs_tpm_cfg_set_no_rule_add_ipv6_dip_5t},
    {"ipv6_l4_ports_5t_rule_add",          sfs_tpm_cfg_set_ipv6_l4_ports_5t_rule_add},
    {"no_rule_add_ipv6_l4_ports_5t",       sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_5t},
    {"add_ds_load_balance",	           sfs_tpm_cfg_set_ds_load_balance_rule_add},
    {"del_ds_load_balance",		   sfs_tpm_cfg_set_no_rule_add_ds_load_balance},

    // zeev
    {"ipv6_nh_acl_rule_add",               sfs_tpm_cfg_set_ipv6_nh_acl_rule_add},
    {"no_rule_add_ipv6_nh_acl",            sfs_tpm_cfg_set_no_rule_add_ipv6_nh_acl},
    {"no_rule_add_ipv6_gen_acl",           sfs_tpm_cfg_set_no_rule_add_ipv6_gen_acl},

    {"ctc_cm_acl_rule_add",                sfs_tpm_cfg_set_ctc_cm_rule_add},
    {"ctc_cm_ipv6_acl_rule_add",	   sfs_tpm_cfg_set_ctc_cm_ipv6_rule_add},
    {"ctc_cm_set_ipv6_parse_win",	   sfs_tpm_cfg_ctc_cm_set_ipv6_parse_win},
    {"no_rule_add_ctc_cm_acl",             sfs_tpm_cfg_set_no_rule_add_ctc_cm_acl},

    {"enable_mtu",                         sfs_tpm_cfg_set_mtu_enable},
    {"set_mtu",                            sfs_tpm_cfg_set_mtu},
    {"set_pppoe_mtu",                      sfs_tpm_cfg_set_pppoe_mtu},
    {"send_genquery_to_uni",               sfs_tpm_cfg_send_genquery_to_uni},
    {"flush_atu",                          sfs_tpm_cfg_flush_atu},
    {"flush_vtu",                          sfs_tpm_cfg_flush_vtu},

    {"tpm_self_check",                     sfs_tpm_rule_self_check},
    {"mac_learn_rule_add",                 sfs_tpm_cfg_set_mac_learn_rule_add},
    {"no_rule_add_mac_learn",              sfs_tpm_cfg_set_no_rule_add_mac_learn},
    {"mac_learn_default_rule_act_set",     sfs_tpm_cfg_set_mac_learn_default_rule_action},
    {"gmac0_ingr_rate_limit_set",          sfs_tpm_cfg_set_gmac0_ingr_rate_limit},

    /* KostaP */
    {"fc_config_set",                      sfs_tpm_cfg_set_fc},
    {"fc_us_period_set",                   sfs_tpm_cfg_set_fc_period},
#ifdef TPM_FC_DEBUG
    {"fc_oneshot_set",                     sfs_tpm_cfg_set_fc_oneshot},
#endif
    {"fc_enable",                  	    sfs_tpm_cfg_fc_enable}
};
static int num_store_name_funcs = sizeof(store_sysfs_name_func_ara)/sizeof(store_sysfs_name_func_ara[0]);

static ssize_t tpm_cfg_store(struct device *dev,
                             struct device_attribute *attr,
                             const char *buf, size_t len)
{
    const char*             name = attr->attr.name;
/*    unsigned long           flags  = 0;*/
    store_sysfs_name_func_t *pnamefunc;
    int                     indx;
    GT_BOOL                 found = GT_FALSE;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

//     raw_local_irq_save(flags);

    for (indx = 0; indx < num_store_name_funcs; indx++){
        pnamefunc = &store_sysfs_name_func_ara[indx];
        if (!strcmp(name, pnamefunc->sysfs_name)) {
            found = GT_TRUE;
            (pnamefunc->sysfs_func)(buf, len);
            break;
        }
    }

    if (found == GT_FALSE){
        printk("%s: operation <%s> not found\n", __FUNCTION__, attr->attr.name);
    }

//     raw_local_irq_restore(flags);

    return(len);
}


/******************************************************************************/
/* ========================================================================== */
/*             TPM CFG SYS FS ROUTINE SHOW SWITCHER                           */
/* ========================================================================== */
#ifdef CONFIG_MV_TPM_SYSFS_HELP

typedef struct
{
    char *sysfs_name;
    int (*sysfs_func)( char *buf);
} show_sysfs_name_func_t;

static show_sysfs_name_func_t show_sysfs_name_func_ara[] =
{
    {"help_tpm_cfg_index",           sfs_tpm_cfg_index},
    {"help_frwd_rule_cfg",           sfs_help_frwd_rule_cfg},
    {"help_vlan_rule_cfg",           sfs_help_vlan_rule_cfg},
    {"help_mod_rule_cfg",            sfs_help_mod_rule_cfg},
    {"help_l2_key_cfg",              sfs_help_l2_key_cfg},
    {"help_l3_key_cfg",              sfs_help_l3_key_cfg},
    {"help_ipv4_key_cfg",            sfs_help_ipv4_key_cfg},
    {"help_rule_table_display",      sfs_help_rule_table_display},
    {"help_l2_rule_add",             sfs_help_l2_rule_add},
    {"help_l3_rule_add",             sfs_help_l3_rule_add},
    {"help_ipv4_rule_add",           sfs_help_ipv4_rule_add},
    {"help_ipv6_dip_rule_add",       sys_help_ipv6_dip_acl_rule_add},
    {"help_ipv6_gen_rule_add",       sfs_help_ipv6_gen_rule_add},
    {"help_mc_ipvx_stream_add",      sfs_help_mc_ipvx_stream_add},
    {"help_igmp_cfg",                sfs_help_igmp_cfg},
    {"help_no_rule_add",             sfs_help_no_rule_add},
    {"help_key_rule_delete",         sfs_help_key_rule_delete},
    {"help_no_mc_stream_add",        sfs_help_no_mc_stream_add},
    {"help_oam_omci_channel",        sfs_help_oam_omci_channel},
    {"help_no_oam_omci_channel",     sfs_help_no_oam_omci_channel},
    {"help_tpm_setup",               sfs_help_setup},
    {"help_mib_reset",               sfs_help_mib_reset},
    {"help_set_active_wan",          sfs_help_set_active_wan},
    {"help_hot_swap_profile",	     sfs_help_hot_swap_profile},
    {"help_set_port_hwf_admin",	     sfs_help_set_port_hwf_admin},
    {"help_cfg_cpu_lpbk",            sfs_help_cfg_cpu_lpbk},
    {"help_ds_load_balance_rule",    sfs_help_ds_load_balance_rule},
    {"help_cfg_age_count",           sfs_help_cfg_age_count},
    {"help_rate_limit",              sfs_help_rate_limit},
    {"help_pkt_mod_add",             sfs_help_pkt_mod_add},
    {"help_pkt_mod_get_del_purge",   sfs_help_pkt_mod_get_del_purge},
    {"help_ipv6_nh_rule_add",        sfs_help_ipv6_nh_rule_add},
    {"help_ipv6_l4_ports_rule_add",  sfs_help_ipv6_l4_ports_rule_add},
    {"help_ipv6_l4_key_cfg",         sfs_help_ipv6_l4_key_cfg},
    {"help_ipv6_dip_key_cfg",        sfs_help_ipv6_dip_key_cfg},
    {"help_ipv6_gen_key_cfg",	     sfs_help_ipv6_gen_key_cfg},
    {"help_ipv6_ctc_cm_key_cfg",     sfs_help_ipv6_ctc_cm_key_cfg},
    {"help_mtu_cfg",                 sfs_help_mtu_cfg},
    {"help_delete_entry_rule_table", sfs_help_delete_entry_rule_table},
    {"help_erase_section",           sfs_help_erase_section},
    {"help_send_genquery_to_uni",    sfs_help_send_genquery_to_uni},
    {"help_tpm_self_check",          sfs_help_tpm_self_check},
    {"help_ipv6_gen_5t_rule_add",      sfs_help_ipv6_gen_5t_rule_add},
    {"help_ipv6_dip_5t_rule_add",      sfs_help_ipv6_dip_5t_rule_add},
    {"help_ipv6_l4_ports_5t_rule_add", sfs_help_ipv6_l4_ports_5t_rule_add},

    {"help_ctc_cm_rule_add",           sfs_help_ctc_cm_rule_add},
    {"help_ctc_cm_set_ipv6_parse_win", sfs_help_ctc_cm_set_ipv6_parse_win},

    {"help_flush_atu",               sfs_help_flush_atu},
    {"help_flush_vtu",               sfs_help_flush_vtu},
    {"help_mac_learn_rule_add",      sfs_help_mac_learn_rule_add},
    {"help_mac_learn_def_act_set",   sfs_help_mac_learn_def_act_set},

    {"show_frwd_rule",               sfs_show_frwd_rule},
    {"show_l2_key_ethertype",        sfs_show_l2_key_ethertype},
    {"show_l2_key_mac_addr",         sfs_show_l2_key_mac_addr},
    {"show_l2_key_vlan",             sfs_show_l2_key_vlan},
    {"show_l2_rule",                 sfs_show_l2_rule},
    {"show_vlan_rule",               sfs_show_vlan_rule},
};
static int num_show_name_funcs = sizeof(show_sysfs_name_func_ara)/sizeof(show_sysfs_name_func_ara[0]);


static ssize_t tpm_cfg_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
    const char*            name = attr->attr.name;
    show_sysfs_name_func_t *pnamefunc;
    int                    indx;
    GT_BOOL                found = GT_FALSE;
    int                    buflen = 0;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    for (indx = 0; indx < num_show_name_funcs; indx++){
        pnamefunc = &show_sysfs_name_func_ara[indx];
        if (!strcmp(name, pnamefunc->sysfs_name)) {
            found = GT_TRUE;
            buflen = (pnamefunc->sysfs_func)(buf);
            break;
        }
    }
    if (found == GT_FALSE){
        printk("%s: operation <%s> not found\n", __FUNCTION__, attr->attr.name);
    }
    return buflen;
}
#else
static ssize_t tpm_cfg_show(struct device *dev,
                            struct device_attribute *attr,
                            char *buf)
{
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __FUNCTION__);
	return 0;
}
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

/******************************************************************************/
/* ========================================================================== */
/*                            Cfg SW Attributes                               */
/* ========================================================================== */

#ifdef CONFIG_MV_TPM_SYSFS_HELP
// help
static DEVICE_ATTR(help_tpm_cfg_index,                              S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_frwd_rule_cfg,                              S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_vlan_rule_cfg,                              S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_mod_rule_cfg,                               S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_l2_key_cfg,                                 S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_l3_key_cfg,                                 S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv4_key_cfg,                               S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_gen_key_cfg,                           S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_dip_key_cfg,                           S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_ctc_cm_key_cfg,                        S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_l4_key_cfg,                            S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_rule_table_display,                         S_IRUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(help_l2_rule_add,                                S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_mac_learn_rule_add,                         S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_mac_learn_def_act_set,                      S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_l3_rule_add,                                S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv4_rule_add,                              S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_dip_rule_add,                          S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_gen_rule_add,                          S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_nh_rule_add,                           S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_l4_ports_rule_add,                     S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_gen_5t_rule_add,                       S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_dip_5t_rule_add,                       S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ipv6_l4_ports_5t_rule_add,                  S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ctc_cm_rule_add,                            S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ctc_cm_set_ipv6_parse_win,                  S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_mc_ipvx_stream_add,                         S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_igmp_cfg,                                   S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_no_rule_add,                                S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_key_rule_delete,                            S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_no_mc_stream_add,                           S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_oam_omci_channel,                           S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_no_oam_omci_channel,                        S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_tpm_setup,                                  S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_mib_reset,                                  S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_set_active_wan,                             S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_hot_swap_profile,                           S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_set_port_hwf_admin,                         S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_cfg_cpu_lpbk,                               S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_ds_load_balance_rule,                       S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_cfg_age_count,                              S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_rate_limit,                                 S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_pkt_mod_add,                                S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_pkt_mod_get_del_purge,                      S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_mtu_cfg,                                    S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_delete_entry_rule_table,                    S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_erase_section,                              S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_send_genquery_to_uni,                       S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_tpm_self_check,                             S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_flush_atu,                                  S_IRUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(help_flush_vtu,                                  S_IRUSR, tpm_cfg_show, tpm_cfg_store);
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

/* sysfs for dumping forwarding table in yaml. */
static DEVICE_ATTR(show_frwd_rule,                                  S_IRUSR, tpm_cfg_show, NULL);
static DEVICE_ATTR(show_l2_key_ethertype,                           S_IRUSR, tpm_cfg_show, NULL);
static DEVICE_ATTR(show_l2_key_mac_addr,                            S_IRUSR, tpm_cfg_show, NULL);
static DEVICE_ATTR(show_l2_key_vlan,                                S_IRUSR, tpm_cfg_show, NULL);
static DEVICE_ATTR(show_l2_rule,                                    S_IRUSR, tpm_cfg_show, NULL);
static DEVICE_ATTR(show_vlan_rule,                                  S_IRUSR, tpm_cfg_show, NULL);

// rule set
static DEVICE_ATTR(frwd_rule_set,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(vlan_rule_set,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_mh_rule_set,                                 S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_vlan_rule_set,                               S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_ipv4_addr_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_ipv4_port_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_ipv6_addr_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_ipv6_port_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_pppoe_rule_set,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mod_mac_addr_rule_set,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(l2_key_ethertype_rule_set,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(l2_key_gemport_rule_set,                         S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(l2_key_mac_addr_rule_set,                        S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(l2_key_pppoe_rule_set,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(l2_key_vlan_rule_set,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(l3_key_ethertype_rule_set,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(l3_key_pppoe_rule_set,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(ipv4_key_proto_rule_set,                         S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv4_key_dscp_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv4_key_addr_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv4_key_port_rule_set,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);


static DEVICE_ATTR(ipv6_gen_key_sip_rule_set,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_ctc_cm_key_rule_set,                        S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_gen_key_dscp_rule_set,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_dip_key_rule_set,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_l4_key_rule_set,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(key_rule_delete,                                 S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(rule_table_display,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_l2_rule_table,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_l3_rule_table,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_ipv4_rule_table,                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_ipv6_gen_rule_table,                S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_ipv6_dip_rule_table,                S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_ipv6_l4_rule_table,                 S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_frwd_rule_table,                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_vlan_rule_table,                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(delete_entry_mod_rule_table,                     S_IWUSR, tpm_cfg_show, tpm_cfg_store);

// Rule add
static DEVICE_ATTR(l2_rule_add,                                     S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mac_learn_rule_add,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mac_learn_default_rule_act_set,                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(l3_rule_add,                                     S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv4_rule_add,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_gen_rule_add,                               S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_dip_rule_add,                               S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_ipv4_stream_add,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_ipv4_stream_set_queue_add,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_ipv4_stream_update,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_ipv6_stream_add,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_ipv6_stream_set_queue_add,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_ipv6_stream_update,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_nh_acl_rule_add,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_l4_ports_acl_rule_add,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(ipv6_gen_5t_rule_add,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_dip_5t_rule_add,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ipv6_l4_ports_5t_rule_add,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(ctc_cm_acl_rule_add,                             S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ctc_cm_ipv6_acl_rule_add,                        S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(ctc_cm_set_ipv6_parse_win,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(igmp_port_forward_mode_cfg,                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(igmp_cpu_queue_cfg,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(igmp_proxy_sa_mac,                               S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_vid_key_reset,                                S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_vid_key_set,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mc_vid_cfg_set,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(no_rule_add_l2,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_l3,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv4,                                S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_mc_stream_add_ipv4,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_mc_stream_add_ipv6,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv6_nh_acl,                         S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv6_l4_ports_acl,                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv6_dip_acl,                        S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv6_gen_acl,                        S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(no_rule_add_ipv6_gen_5t,                         S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv6_dip_5t,                         S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_ipv6_l4_ports_5t,                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(no_rule_add_ctc_cm_acl,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_rule_add_mac_learn,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);


static DEVICE_ATTR(oam_channel,                                     S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(omci_channel,                                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_oam_channel,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(no_omci_channel,                                 S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(tpm_setup,                                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(mib_reset,                                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(set_active_wan,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(hot_swap_profile,                                S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(set_port_hwf_admin,                              S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(add_cpu_lpbk,                                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(del_cpu_lpbk,                                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(dump_cpu_lpbk,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(get_lu_entry,                                    S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(set_count_mask,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(get_hit_count,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(get_pnc_all_hit_cntrs,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(set_lu_thresh,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(reset_age_group,                                 S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(rate_limit_queue_set,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(gmac0_ingr_rate_limit_set,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(scheduling_mode_queue_set,                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(rate_limit_if_set,                               S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(pkt_mod_eng_entry_add,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(pkt_mod_eng_entry_show,                          S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(pkt_mod_eng_entry_del,                           S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(pkt_mod_eng_purge,                               S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(enable_mtu,                                      S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(set_mtu,                                         S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(set_pppoe_mtu,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(send_genquery_to_uni,                            S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(erase_section,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(tpm_self_check,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(flush_atu,                                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(flush_vtu,                                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(add_ds_load_balance,                             S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(del_ds_load_balance,                             S_IWUSR, tpm_cfg_show, tpm_cfg_store);

static DEVICE_ATTR(fc_config_set,                                   S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(fc_us_period_set,                                S_IWUSR, tpm_cfg_show, tpm_cfg_store);
static DEVICE_ATTR(fc_enable,                                       S_IWUSR, tpm_cfg_show, tpm_cfg_store);
#ifdef TPM_FC_DEBUG
static DEVICE_ATTR(fc_oneshot_set,                                  S_IWUSR, tpm_cfg_show, tpm_cfg_store);
#endif

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_mngt SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_mngt_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP

    &dev_attr_help_oam_omci_channel.attr,
    &dev_attr_help_no_oam_omci_channel.attr,
#endif
    &dev_attr_oam_channel.attr,
    &dev_attr_omci_channel.attr,
    &dev_attr_no_oam_channel.attr,
    &dev_attr_no_omci_channel.attr,

    NULL
};

static struct attribute_group tpm_cfg_mngt_sw_group =
{
    .name = "cfg_mngt",
    .attrs = tpm_cfg_mngt_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_mod SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_mod_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_mod_rule_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_pkt_mod_add.attr,
    &dev_attr_help_pkt_mod_get_del_purge.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_mod_mh_rule_set.attr,
    &dev_attr_mod_vlan_rule_set.attr,
    &dev_attr_mod_ipv4_addr_rule_set.attr,
    &dev_attr_mod_ipv4_port_rule_set.attr,
    &dev_attr_mod_ipv6_addr_rule_set.attr,
    &dev_attr_mod_ipv6_port_rule_set.attr,
    &dev_attr_mod_pppoe_rule_set.attr,
    &dev_attr_mod_mac_addr_rule_set.attr,
    &dev_attr_pkt_mod_eng_entry_add.attr,
    &dev_attr_pkt_mod_eng_entry_show.attr,
    &dev_attr_pkt_mod_eng_entry_del.attr,
    &dev_attr_pkt_mod_eng_purge.attr,

    &dev_attr_delete_entry_mod_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    NULL
};

static struct attribute_group tpm_cfg_mod_sw_group =
{
    .name = "cfg_mod",
    .attrs = tpm_cfg_mod_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_l2 SYS FS STORE ROUTINE SWITCHER                       */
/* ========================================================================== */

static struct attribute *tpm_cfg_l2_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_l2_key_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_l2_rule_add.attr,
    &dev_attr_help_no_rule_add.attr,
    &dev_attr_help_delete_entry_rule_table.attr,
    &dev_attr_help_mac_learn_rule_add.attr,
    &dev_attr_help_mac_learn_def_act_set.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_show_l2_key_ethertype.attr,
    &dev_attr_show_l2_key_mac_addr.attr,
    &dev_attr_show_l2_key_vlan.attr,
    &dev_attr_show_l2_rule.attr,

    &dev_attr_l2_key_ethertype_rule_set.attr,
    &dev_attr_l2_key_gemport_rule_set.attr,
    &dev_attr_l2_key_mac_addr_rule_set.attr,
    &dev_attr_l2_key_pppoe_rule_set.attr,
    &dev_attr_l2_key_vlan_rule_set.attr,

    &dev_attr_delete_entry_l2_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    &dev_attr_l2_rule_add.attr,
    &dev_attr_no_rule_add_l2.attr,

    &dev_attr_mac_learn_rule_add.attr,
    &dev_attr_mac_learn_default_rule_act_set.attr,
    &dev_attr_no_rule_add_mac_learn.attr,

    NULL
};

static struct attribute_group tpm_cfg_l2_sw_group =
{
    .name = "cfg_l2",
    .attrs = tpm_cfg_l2_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_l3 SYS FS STORE ROUTINE SWITCHER                       */
/* ========================================================================== */

static struct attribute *tpm_cfg_l3_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_l3_key_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_l3_rule_add.attr,
    &dev_attr_help_no_rule_add.attr,
    &dev_attr_help_delete_entry_rule_table.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_l3_key_ethertype_rule_set.attr,
    &dev_attr_l3_key_pppoe_rule_set.attr,

    &dev_attr_delete_entry_l3_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    &dev_attr_l3_rule_add.attr,
    &dev_attr_no_rule_add_l3.attr,

    NULL
};

static struct attribute_group tpm_cfg_l3_sw_group =
{
    .name = "cfg_l3",
    .attrs = tpm_cfg_l3_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_ipv4 SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_ipv4_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_ipv4_key_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_ipv4_rule_add.attr,
    &dev_attr_help_no_rule_add.attr,
    &dev_attr_help_delete_entry_rule_table.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_ipv4_key_proto_rule_set.attr,
    &dev_attr_ipv4_key_dscp_rule_set.attr,
    &dev_attr_ipv4_key_addr_rule_set.attr,
    &dev_attr_ipv4_key_port_rule_set.attr,

    &dev_attr_delete_entry_ipv4_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    &dev_attr_ipv4_rule_add.attr,
    &dev_attr_no_rule_add_ipv4.attr,


    NULL
};

static struct attribute_group tpm_cfg_ipv4_sw_group =
{
    .name = "cfg_ipv4",
    .attrs = tpm_cfg_ipv4_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_ipv6 SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_ipv6_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_ipv6_gen_key_cfg.attr,
    &dev_attr_help_ipv6_ctc_cm_key_cfg.attr,
    &dev_attr_help_ipv6_dip_key_cfg.attr,
    &dev_attr_help_ipv6_l4_key_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_ipv6_gen_rule_add.attr,
    &dev_attr_help_ipv6_dip_rule_add.attr,
    &dev_attr_help_ipv6_nh_rule_add.attr,
    &dev_attr_help_ipv6_l4_ports_rule_add.attr,
    &dev_attr_help_ipv6_gen_5t_rule_add.attr,
    &dev_attr_help_ipv6_dip_5t_rule_add.attr,
    &dev_attr_help_ipv6_l4_ports_5t_rule_add.attr,
    &dev_attr_help_no_rule_add.attr,
    &dev_attr_help_delete_entry_rule_table.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_ipv6_dip_key_rule_set.attr,
    &dev_attr_ipv6_l4_key_rule_set.attr,
    &dev_attr_ipv6_ctc_cm_key_rule_set.attr,
    &dev_attr_ipv6_gen_key_sip_rule_set.attr,
    &dev_attr_ipv6_gen_key_dscp_rule_set.attr,
    &dev_attr_delete_entry_ipv6_gen_rule_table.attr,
    &dev_attr_delete_entry_ipv6_dip_rule_table.attr,
    &dev_attr_delete_entry_ipv6_l4_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    &dev_attr_ipv6_gen_rule_add.attr,
    &dev_attr_ipv6_dip_rule_add.attr,
    &dev_attr_ipv6_nh_acl_rule_add.attr,
    &dev_attr_ipv6_l4_ports_acl_rule_add.attr,
    &dev_attr_no_rule_add_ipv6_nh_acl.attr,
    &dev_attr_no_rule_add_ipv6_l4_ports_acl.attr,
    &dev_attr_no_rule_add_ipv6_gen_acl.attr,
    &dev_attr_no_rule_add_ipv6_dip_acl.attr,

    &dev_attr_ipv6_gen_5t_rule_add.attr,
    &dev_attr_ipv6_dip_5t_rule_add.attr,
    &dev_attr_ipv6_l4_ports_5t_rule_add.attr,
    &dev_attr_no_rule_add_ipv6_gen_5t.attr,
    &dev_attr_no_rule_add_ipv6_dip_5t.attr,
    &dev_attr_no_rule_add_ipv6_l4_ports_5t.attr,

    NULL
};

static struct attribute *tpm_cfg_mtu_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_mtu_cfg.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_enable_mtu.attr,
    &dev_attr_set_mtu.attr,
    &dev_attr_set_pppoe_mtu.attr,

    NULL
};

static struct attribute_group tpm_cfg_ipv6_sw_group =
{
    .name = "cfg_ipv6",
    .attrs = tpm_cfg_ipv6_sw_attrs
};

static struct attribute_group tpm_cfg_mtu_sw_group =
{
    .name = "cfg_mtu",
    .attrs = tpm_cfg_mtu_sw_attrs
};

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_frwd SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_frwd_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_frwd_rule_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_delete_entry_rule_table.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_show_frwd_rule.attr,

    &dev_attr_frwd_rule_set.attr,

    &dev_attr_delete_entry_frwd_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    NULL
};

static struct attribute_group tpm_cfg_frwd_sw_group =
{
    .name = "cfg_frwd",
    .attrs = tpm_cfg_frwd_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_vlan SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_vlan_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_vlan_rule_cfg.attr,
    &dev_attr_help_rule_table_display.attr,
    &dev_attr_help_delete_entry_rule_table.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_show_vlan_rule.attr,

    &dev_attr_vlan_rule_set.attr,

    &dev_attr_delete_entry_vlan_rule_table.attr,
    &dev_attr_rule_table_display.attr,

    NULL
};

static struct attribute_group tpm_cfg_vlan_sw_group =
{
    .name = "cfg_vlan",
    .attrs = tpm_cfg_vlan_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_traffic SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_traffic_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_rate_limit.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_rate_limit_queue_set.attr,
    &dev_attr_scheduling_mode_queue_set.attr,
    &dev_attr_rate_limit_if_set.attr,
    &dev_attr_gmac0_ingr_rate_limit_set.attr,
    &dev_attr_fc_config_set.attr,
    &dev_attr_fc_us_period_set.attr,
    &dev_attr_fc_enable.attr,
#ifdef TPM_FC_DEBUG
    &dev_attr_fc_oneshot_set.attr,
#endif


    NULL
};

static struct attribute_group tpm_cfg_traffic_sw_group =
{
    .name = "cfg_traffic",
    .attrs = tpm_cfg_traffic_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_mc SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_mc_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_mc_ipvx_stream_add.attr,
    &dev_attr_help_igmp_cfg.attr,
    &dev_attr_help_no_mc_stream_add.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_mc_ipv4_stream_add.attr,
    &dev_attr_mc_ipv4_stream_set_queue_add.attr,
    &dev_attr_igmp_port_forward_mode_cfg.attr,
    &dev_attr_igmp_cpu_queue_cfg.attr,
    &dev_attr_igmp_proxy_sa_mac.attr,
    &dev_attr_no_mc_stream_add_ipv4.attr,
    &dev_attr_mc_ipv4_stream_update.attr,

    &dev_attr_mc_vid_key_reset.attr,
    &dev_attr_mc_vid_key_set.attr,
    &dev_attr_mc_vid_cfg_set.attr,

    &dev_attr_mc_ipv6_stream_add.attr,
    &dev_attr_mc_ipv6_stream_set_queue_add.attr,
    &dev_attr_no_mc_stream_add_ipv6.attr,
    &dev_attr_mc_ipv6_stream_update.attr,

    NULL
};

static struct attribute_group tpm_cfg_mc_sw_group =
{
    .name = "cfg_mc",
    .attrs = tpm_cfg_mc_sw_attrs
};


/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_misc SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_misc_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_tpm_setup.attr,
    &dev_attr_help_mib_reset.attr,
    &dev_attr_help_set_active_wan.attr,
    &dev_attr_help_hot_swap_profile.attr,
    &dev_attr_help_set_port_hwf_admin.attr,
    &dev_attr_help_erase_section.attr,
    &dev_attr_help_send_genquery_to_uni.attr,
    &dev_attr_help_tpm_self_check.attr,
    &dev_attr_help_flush_atu.attr,
    &dev_attr_help_flush_vtu.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_tpm_setup.attr,

    &dev_attr_send_genquery_to_uni.attr,

    &dev_attr_mib_reset.attr,
    &dev_attr_set_active_wan.attr,
    &dev_attr_hot_swap_profile.attr,
    &dev_attr_set_port_hwf_admin.attr,
    &dev_attr_erase_section.attr,
    &dev_attr_tpm_self_check.attr,
    &dev_attr_flush_vtu.attr,
    &dev_attr_flush_atu.attr,

    NULL
};

static struct attribute_group tpm_cfg_misc_sw_group =
{
    .name = "cfg_misc",
    .attrs = tpm_cfg_misc_sw_attrs
};

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_cpu_lpbk SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_cpu_lpbk_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_cfg_cpu_lpbk.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_add_cpu_lpbk.attr,
    &dev_attr_del_cpu_lpbk.attr,
    &dev_attr_dump_cpu_lpbk.attr,
    NULL
};

static struct attribute_group tpm_cfg_cpu_lpbk_sw_group =
{
    .name = "cfg_cpu_lpbk",
    .attrs = tpm_cfg_cpu_lpbk_sw_attrs
};

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_ds_load_balance SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_ds_load_balance_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_ds_load_balance_rule.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_add_ds_load_balance.attr,
    &dev_attr_del_ds_load_balance.attr,
    NULL
};

static struct attribute_group tpm_cfg_ds_load_balance_sw_group =
{
    .name = "cfg_ds_load_balance",
    .attrs = tpm_cfg_ds_load_balance_sw_attrs
};

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_age_count SYS FS STORE ROUTINE SWITCHER                */
/* ========================================================================== */

static struct attribute *tpm_cfg_age_count_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_cfg_age_count.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_get_lu_entry.attr,
    &dev_attr_set_count_mask.attr,
    &dev_attr_get_hit_count.attr,
    &dev_attr_get_pnc_all_hit_cntrs.attr,
    &dev_attr_set_lu_thresh.attr,
    &dev_attr_reset_age_group.attr,
    NULL
};

static struct attribute_group tpm_cfg_age_count_sw_group =
{
    .name = "cfg_age_count",
    .attrs = tpm_cfg_age_count_sw_attrs
};

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg_ctc_cm SYS FS STORE ROUTINE SWITCHER                     */
/* ========================================================================== */

static struct attribute *tpm_cfg_ctc_cm_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_ctc_cm_rule_add.attr,
    &dev_attr_help_ctc_cm_set_ipv6_parse_win.attr,
    &dev_attr_help_no_rule_add.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */
    &dev_attr_ctc_cm_acl_rule_add.attr,
    &dev_attr_ctc_cm_ipv6_acl_rule_add.attr,
    &dev_attr_ctc_cm_set_ipv6_parse_win.attr,
    &dev_attr_no_rule_add_ctc_cm_acl.attr,
    NULL
};

static struct attribute_group tpm_cfg_ctc_cm_sw_group =
{
    .name = "cfg_ctc_cm",
    .attrs = tpm_cfg_ctc_cm_sw_attrs
};

//#if 0

// !!@@##$$%%
//
// KEEP THIS - WE MAY GO BACK TO FLAT cfg FOLDER
//
// !!@@##$$%%

/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg SYS FS STORE ROUTINE SWITCHER                          */
/* ========================================================================== */

static struct attribute *tpm_cfg_flat_sw_attrs[] =
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    &dev_attr_help_tpm_cfg_index.attr,
    &dev_attr_help_frwd_rule_cfg.attr,
    &dev_attr_help_vlan_rule_cfg.attr,
    &dev_attr_help_mod_rule_cfg.attr,
    &dev_attr_help_l2_key_cfg.attr,
    &dev_attr_help_l3_key_cfg.attr,
    &dev_attr_help_ipv4_key_cfg.attr,
    &dev_attr_help_ipv6_l4_key_cfg.attr,
    &dev_attr_help_ipv6_ctc_cm_key_cfg.attr,
    &dev_attr_help_ipv6_dip_key_cfg.attr,
    &dev_attr_help_ipv6_gen_key_cfg.attr,
    &dev_attr_help_rule_table_display.attr,

    &dev_attr_help_l2_rule_add.attr,
    &dev_attr_help_l3_rule_add.attr,
    &dev_attr_help_ipv4_rule_add.attr,
    &dev_attr_help_ipv6_gen_rule_add.attr,
    &dev_attr_help_ipv6_dip_rule_add.attr,
    &dev_attr_help_ipv6_nh_rule_add.attr,
    &dev_attr_help_ipv6_l4_ports_rule_add.attr,
    &dev_attr_help_mc_ipvx_stream_add.attr,
    &dev_attr_help_igmp_cfg.attr,
    &dev_attr_help_no_rule_add.attr,
    &dev_attr_help_key_rule_delete.attr,
    &dev_attr_help_no_mc_stream_add.attr,
    &dev_attr_help_oam_omci_channel.attr,
    &dev_attr_help_no_oam_omci_channel.attr,
    &dev_attr_help_tpm_setup.attr,
    &dev_attr_help_mib_reset.attr,
    &dev_attr_help_set_active_wan.attr,
    &dev_attr_help_hot_swap_profile.attr,
    &dev_attr_help_set_port_hwf_admin.attr,
    &dev_attr_help_ds_load_balance_rule.attr,
    &dev_attr_help_cfg_cpu_lpbk.attr,
    &dev_attr_help_rate_limit.attr,
    &dev_attr_help_pkt_mod_add.attr,
    &dev_attr_help_pkt_mod_get_del_purge.attr,

    &dev_attr_help_ipv6_gen_5t_rule_add.attr,
    &dev_attr_help_ipv6_dip_5t_rule_add.attr,
    &dev_attr_help_ipv6_l4_ports_5t_rule_add.attr,
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

    &dev_attr_frwd_rule_set.attr,
    &dev_attr_vlan_rule_set.attr,
    &dev_attr_mod_vlan_rule_set.attr,
    &dev_attr_mod_ipv4_addr_rule_set.attr,
    &dev_attr_mod_ipv4_port_rule_set.attr,
    &dev_attr_mod_ipv6_addr_rule_set.attr,
    &dev_attr_mod_ipv6_port_rule_set.attr,
    &dev_attr_mod_pppoe_rule_set.attr,
    &dev_attr_mod_mac_addr_rule_set.attr,

    &dev_attr_l2_key_ethertype_rule_set.attr,
    &dev_attr_l2_key_gemport_rule_set.attr,
    &dev_attr_l2_key_mac_addr_rule_set.attr,
    &dev_attr_l2_key_pppoe_rule_set.attr,
    &dev_attr_l2_key_vlan_rule_set.attr,

    &dev_attr_l3_key_ethertype_rule_set.attr,
    &dev_attr_l3_key_pppoe_rule_set.attr,

    &dev_attr_ipv4_key_proto_rule_set.attr,
    &dev_attr_ipv4_key_dscp_rule_set.attr,
    &dev_attr_ipv4_key_addr_rule_set.attr,
    &dev_attr_ipv4_key_port_rule_set.attr,

    &dev_attr_ipv6_ctc_cm_key_rule_set.attr,
    &dev_attr_ipv6_gen_key_sip_rule_set.attr,
    &dev_attr_ipv6_gen_key_dscp_rule_set.attr,
    &dev_attr_ipv6_dip_key_rule_set.attr,
    &dev_attr_ipv6_l4_key_rule_set.attr,

    &dev_attr_key_rule_delete.attr,
    &dev_attr_rule_table_display.attr,

    &dev_attr_l2_rule_add.attr,
    &dev_attr_l3_rule_add.attr,
    &dev_attr_ipv4_rule_add.attr,
    &dev_attr_ipv6_gen_rule_add.attr,
    &dev_attr_ipv6_dip_rule_add.attr,
    &dev_attr_mc_ipv4_stream_add.attr,
    &dev_attr_mc_ipv4_stream_set_queue_add.attr,
    &dev_attr_mc_ipv4_stream_update.attr,
    &dev_attr_mc_ipv6_stream_add.attr,
    &dev_attr_mc_ipv6_stream_set_queue_add.attr,
    &dev_attr_mc_ipv6_stream_update.attr,
    &dev_attr_ipv6_nh_acl_rule_add.attr,
    &dev_attr_ipv6_l4_ports_acl_rule_add.attr,

    &dev_attr_ipv6_gen_5t_rule_add.attr,
    &dev_attr_ipv6_dip_5t_rule_add.attr,
    &dev_attr_ipv6_l4_ports_5t_rule_add.attr,

    &dev_attr_igmp_port_forward_mode_cfg.attr,
    &dev_attr_igmp_cpu_queue_cfg.attr,
    &dev_attr_igmp_proxy_sa_mac.attr,

    &dev_attr_no_rule_add_l2.attr,
    &dev_attr_no_rule_add_l3.attr,
    &dev_attr_no_rule_add_ipv4.attr,
    &dev_attr_no_rule_add_ipv6_nh_acl.attr,
    &dev_attr_no_rule_add_ipv6_l4_ports_acl.attr,
    &dev_attr_no_rule_add_ipv6_gen_acl.attr,
    &dev_attr_no_mc_stream_add_ipv6.attr,
    &dev_attr_no_mc_stream_add_ipv4.attr,

    &dev_attr_no_rule_add_ipv6_gen_5t.attr,
    &dev_attr_no_rule_add_ipv6_dip_5t.attr,
    &dev_attr_no_rule_add_ipv6_l4_ports_5t.attr,

    &dev_attr_oam_channel.attr,
    &dev_attr_omci_channel.attr,
    &dev_attr_no_oam_channel.attr,
    &dev_attr_no_omci_channel.attr,

    &dev_attr_tpm_setup.attr,
    &dev_attr_mib_reset.attr,
    &dev_attr_set_active_wan.attr,
    &dev_attr_hot_swap_profile.attr,
    &dev_attr_set_port_hwf_admin.attr,

    &dev_attr_rate_limit_queue_set.attr,
    &dev_attr_scheduling_mode_queue_set.attr,
    &dev_attr_rate_limit_if_set.attr,
    &dev_attr_pkt_mod_eng_entry_add.attr,
    &dev_attr_pkt_mod_eng_entry_show.attr,
    &dev_attr_pkt_mod_eng_entry_del.attr,
    &dev_attr_pkt_mod_eng_purge.attr,

    NULL
};

static struct attribute_group tpm_cfg_flat_sw_group =
{
    .name = "cfg_flat",
    .attrs = tpm_cfg_flat_sw_attrs
};
//#endif




/******************************************************************************/
/* ========================================================================== */
/*             TPM cfg SETUP: incl. folder creation + FS population           */
/* ========================================================================== */

typedef struct
{
    char                   *folder_name;
    struct attribute_group *pattrgroup;
} attr_group_pair_t;

static attr_group_pair_t attr_group_pair_ara[] =
{
    {"cfg_mngt",               &tpm_cfg_mngt_sw_group},
    {"cfg_mod",                &tpm_cfg_mod_sw_group},
    {"cfg_frwd",               &tpm_cfg_frwd_sw_group},
    {"cfg_vlan",               &tpm_cfg_vlan_sw_group},
    {"cfg_l2",                 &tpm_cfg_l2_sw_group},
    {"cfg_l3",                 &tpm_cfg_l3_sw_group},
    {"cfg_ipv4",               &tpm_cfg_ipv4_sw_group},
    {"cfg_ipv6",               &tpm_cfg_ipv6_sw_group},
    {"cfg_mc",                 &tpm_cfg_mc_sw_group},
    {"cfg_traffic",            &tpm_cfg_traffic_sw_group},
    {"cfg_misc",               &tpm_cfg_misc_sw_group},
    {"cfg_cpu_lpbk",           &tpm_cfg_cpu_lpbk_sw_group},
    {"cfg_ds_load_balance",    &tpm_cfg_ds_load_balance_sw_group},
    {"cfg_age_count",          &tpm_cfg_age_count_sw_group},
    {"cfg_flat",               &tpm_cfg_flat_sw_group},
    {"cfg_mtu",                &tpm_cfg_mtu_sw_group},
    {"cfg_ctc_cm",             &tpm_cfg_ctc_cm_sw_group},
};
static int num_attr_group_pairs = sizeof(attr_group_pair_ara)/sizeof(attr_group_pair_ara[0]);

int tpm_sysfs_init(void)
{
    int           err;
    struct device *pd;
    int           indx;

    tpm_sysfs_rule_db_init();

    pd = bus_find_device_by_name(&platform_bus_type, NULL, "tpm");
    if (!pd) {
        platform_device_register_simple("tpm", -1, NULL, 0);
        pd = bus_find_device_by_name(&platform_bus_type, NULL, "tpm");
    }

    if (!pd) {
        printk(KERN_ERR"%s: cannot find tpm device\n", __FUNCTION__);
        pd = &platform_bus;
    }

    for (indx = 0; indx < num_attr_group_pairs; indx++)
    {
        attr_group_pair_t *ppair;

        ppair = &attr_group_pair_ara[indx];

        err = sysfs_create_group(&pd->kobj, ppair->pattrgroup);
        if (err)
        {
            printk(KERN_INFO "sysfs_create_group failed for %s, err = %d\n", ppair->folder_name, err);
            goto out;
        }
    }
    printk(KERN_INFO "= SW Module SYS FS Init for tpm ended successfully =\n");

out:
    return err;
}

