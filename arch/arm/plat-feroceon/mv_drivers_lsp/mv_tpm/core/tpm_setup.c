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
/*
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include "globals.h"
#include "errorCode.h"
#include "OsGlueLayer.h"
*/
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_types.h"
#include "tpm_internal_types.h"
#include "tpm_mng_if.h"
#include "tpm_xml_params.h"

tpm_init_t tpm_setup;
int tpmDrvFd;
/*GL_SEMAPHORE_ID tpmApiSemId;*/
extern int32_t tpm_module_start(tpm_init_t *tpm_init);
extern char *g_pstr_xml_cfg_file;

int32_t tpm_init_pon_type_get_para(void)
{

	int32_t	rc = TPM_OK;
	int 	app_rc;

	app_rc = get_pon_type_param(&(tpm_setup.pon_type));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.pon_type = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_omci_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_omci_etype_param(&(tpm_setup.omci_etype));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.omci_etype = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

#if 0
int32_t tpm_init_debport_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_debug_port_params(&(tpm_setup.deb_port_valid), &(tpm_setup.deb_port));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;

	return rc;
}
#endif

/* jinghua add for MTU setting */
int32_t tpm_init_mtu_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_ipv4_mtu_us(&(tpm_setup.mtu_config.ipv4_mtu_us));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.ipv4_mtu_us = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ipv4_pppoe_mtu_us(&(tpm_setup.mtu_config.ipv4_pppoe_mtu_us));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.ipv4_pppoe_mtu_us = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ipv6_mtu_us(&(tpm_setup.mtu_config.ipv6_mtu_us));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.ipv6_mtu_us = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ipv6_pppoe_mtu_us(&(tpm_setup.mtu_config.ipv6_pppoe_mtu_us));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.ipv6_pppoe_mtu_us = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ipv6_mtu_ds(&(tpm_setup.mtu_config.ipv6_mtu_ds));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.ipv6_mtu_ds = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ipv4_mtu_ds(&(tpm_setup.mtu_config.ipv4_mtu_ds));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.ipv4_mtu_ds = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_pppoe_add_enable(&(tpm_setup.pppoe_add_enable));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.pppoe_add_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_num_vlan_tags(&(tpm_setup.num_vlan_tags));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.num_vlan_tags = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_mtu_enable(&(tpm_setup.mtu_config.mtu_enable));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mtu_config.mtu_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_cpu_rx_queue(&(tpm_setup.cpu_rx_queue));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.cpu_rx_queue = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ttl_illegal_action(&(tpm_setup.ttl_illegal_action));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.ttl_illegal_action = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_tcp_flag_check(&(tpm_setup.tcp_flag_check));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.tcp_flag_check = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_igmp_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;
	tpm_src_port_type_t src_port;
	uint32_t mode;

	app_rc = get_igmp_snoop_params_enable(&(tpm_setup.igmp_snoop));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.igmp_snoop = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_igmp_snoop_params_cpu_queue(&(tpm_setup.igmp_cpu_queue));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.igmp_cpu_queue = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	for (src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_WAN; src_port++) {
		app_rc = get_igmp_snoop_params_port_frwd_mode(&mode, src_port);
		if (app_rc == TPM_FAIL)
			rc = TPM_FAIL;
		else if (app_rc == TPM_NOT_FOUND)
			tpm_setup.igmp_pkt_frwd_mod[src_port] = MV_TPM_UN_INITIALIZED_INIT_PARAM;
		else
			tpm_setup.igmp_pkt_frwd_mod[src_port] = mode;
	}

	return rc;
}

int32_t tpm_init_mc_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_mc_filter_mode(&(tpm_setup.mc_setting.filter_mode));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mc_setting.filter_mode = MV_TPM_UN_INITIALIZED_INIT_PARAM;
/*
	app_rc = get_igmp_mode(&(tpm_setup.mc_setting.igmp_mode));
	if (app_rc == TPM_FAIL)
	{
	   rc = TPM_FAIL;
	}
	else if (app_rc == TPM_NOT_FOUND)
	{
	   tpm_setup.mc_setting.igmp_mode = MV_TPM_UN_INITIALIZED_INIT_PARAM;
	}
*/
	app_rc = get_mc_per_uni_vlan_xlat(&(tpm_setup.mc_setting.per_uni_vlan_xlat));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mc_setting.per_uni_vlan_xlat = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_mc_pppoe_enable(&(tpm_setup.mc_setting.mc_pppoe_enable));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mc_setting.mc_pppoe_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_mc_queue(&(tpm_setup.mc_setting.mc_hwf_queue), &(tpm_setup.mc_setting.mc_cpu_queue));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND) {
		tpm_setup.mc_setting.mc_hwf_queue = MV_TPM_UN_INITIALIZED_INIT_PARAM;
		tpm_setup.mc_setting.mc_cpu_queue = MV_TPM_UN_INITIALIZED_INIT_PARAM;
	}
	
	app_rc = get_ipv4_mc_support(&tpm_setup.mc_setting.ipv4_mc_support);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mc_setting.ipv4_mc_support = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_ipv6_mc_support(&tpm_setup.mc_setting.ipv6_mc_support);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.mc_setting.ipv6_mc_support = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_gmac_conn_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_gmac_conn_params(&(tpm_setup.num_tcont_llid));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.num_tcont_llid = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_eth_cmplx_profile_get_para(void)
{
	int32_t	rc = TPM_OK;
	int 	app_rc;

	app_rc = get_eth_cmplx_profile_params(&(tpm_setup.eth_cmplx_profile));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.eth_cmplx_profile = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_eth_port_conf_get_para(void)
{
	int32_t	rc = TPM_OK;
	int 	app_rc;

	app_rc = get_eth_port_conf_params(&(tpm_setup.eth_port_conf[0]), TPM_MAX_NUM_ETH_PORTS);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
	{
		int32_t i;
		for (i=0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			tpm_setup.eth_port_conf[i].valid = TPM_FALSE;
		}
	}
	return rc;
}

int32_t tpm_init_gmac_port_conf_get_para(void)
{
	int32_t	rc = TPM_OK;
	int 	app_rc;

	app_rc = get_gmac_port_conf_params(&(tpm_setup.gmac_port_conf[0]), TPM_NUM_GMACS);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
	{
		int32_t i;
		for (i=0; i < TPM_NUM_GMACS; i++){
			tpm_setup.gmac_port_conf[i].valid = TPM_FALSE;
		}
	}

	return rc;
}

int32_t tpm_init_backup_wan_get_para(void)
{
	int32_t	rc = TPM_OK;
	int 	app_rc;

	app_rc = get_backup_wan_params(&(tpm_setup.backup_wan));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.backup_wan = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_gmac_mh_en_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_gmac_mh_en_params(&(tpm_setup.gmac0_mh_en), &(tpm_setup.gmac1_mh_en));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND) {
		tpm_setup.gmac0_mh_en = MV_TPM_UN_INITIALIZED_INIT_PARAM;
		tpm_setup.gmac1_mh_en = MV_TPM_UN_INITIALIZED_INIT_PARAM;
	}
	return rc;
}

int32_t tpm_init_gmac_pool_bufs_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_gmac_pool_bufs_params(&(tpm_setup.gmac_bp_bufs[0]), TPM_MAX_NUM_GMACS);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;

	return rc;
}

int32_t tpm_init_gmac_rx_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_gmac_rxq_params(&(tpm_setup.gmac_rx[0]), TPM_MAX_NUM_GMACS, TPM_MAX_NUM_RX_QUEUE);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;

	return rc;
}

int32_t tpm_init_gmac_tx_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_gmac_tx_params(&(tpm_setup.gmac_tx[0]), TPM_MAX_NUM_TX_PORTS, TPM_MAX_NUM_TX_QUEUE);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;

	return rc;
}

int32_t tpm_init_pnc_config_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_pnc_range_params(&(tpm_setup.pnc_range[0]), TPM_MAX_NUM_RANGES);
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;

	app_rc = get_catch_all_pkt_action(&(tpm_setup.catch_all_pkt_action));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.catch_all_pkt_action = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_ds_mh_config_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_ds_mh_config_params(&(tpm_setup.ds_mh_set_conf));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.ds_mh_set_conf = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_validation_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_validation_enabled_config_params(&(tpm_setup.validation_en));

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.validation_en = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_vlan_etypes_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_vlan_mod_vlan_tpid_params(&(tpm_setup.vlan_etypes[0]), TPM_NUM_VLAN_ETYPE_REGS);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.vlan_etypes[0] = 0x8100;

	return rc;
}

int32_t tpm_init_mod_config_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_modification_params(&(tpm_setup.mod_config));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;

	app_rc = get_chain_config_params(&(tpm_setup.mod_chain));
	if (app_rc == TPM_FAIL)
		rc |= TPM_FAIL;

	return rc;
}

int32_t tpm_init_cfg_pnc_parse_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_cfg_pnc_parse_param(&(tpm_setup.cfg_pnc_parse));

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.cfg_pnc_parse = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_cpu_loopback_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_cpu_loopback_param(&(tpm_setup.cpu_loopback));

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.cpu_loopback = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_trace_debug_info_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_trace_debug_info_param(&(tpm_setup.trace_debug_info));

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.trace_debug_info = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_vitual_uni_info_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_vitual_uni_enable_params(&(tpm_setup.virt_uni_info.enabled));

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND) {
		tpm_setup.virt_uni_info.enabled = MV_TPM_UN_INITIALIZED_INIT_PARAM;
		tpm_setup.virt_uni_info.uni_port = MV_TPM_UN_INITIALIZED_INIT_PARAM;
		tpm_setup.virt_uni_info.switch_port = MV_TPM_UN_INITIALIZED_INIT_PARAM;
	}

	return rc;
}

int32_t tpm_init_default_tag_tpid_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

#if 0
	app_rc = get_default_vlan_tpid_params(&(tpm_setup.vlan1_tpid), &(tpm_setup.vlan2_tpid));

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND) {
		tpm_setup.vlan1_tpid = 0x8100;
		tpm_setup.vlan2_tpid = 0x8100;
	}
#endif

	app_rc = get_default_vlan_tpid_params(&tpm_setup.tpid_opt.opt_num, tpm_setup.tpid_opt.opt);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND) {
		tpm_setup.tpid_opt.opt_num = 2;

		tpm_setup.tpid_opt.opt[0].v1_tpid = 0x8100;
		tpm_setup.tpid_opt.opt[0].v2_tpid = 0x8100;

		tpm_setup.tpid_opt.opt[1].v1_tpid = 0x8100;
		tpm_setup.tpid_opt.opt[1].v2_tpid = MV_TPM_UN_INITIALIZED_INIT_PARAM;
	}

	return rc;
}


int32_t tpm_init_fc_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_fc_def_params(&tpm_setup.port_fc_conf);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND) {
		tpm_setup.port_fc_conf.enabled 		= 0;
		tpm_setup.port_fc_conf.port 		= 1;
		tpm_setup.port_fc_conf.queue_sample_freq = 1000;
		tpm_setup.port_fc_conf.tgt_port 	= 1;
		tpm_setup.port_fc_conf.thresh_high 	= 0xffffffff;
		tpm_setup.port_fc_conf.thresh_low 	= 0xffffffff;
		tpm_setup.port_fc_conf.tx_port 		= 1;
	}

	return rc;
}


int32_t tpm_init_ety_dsa_enable_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_ety_dsa_enable(&tpm_setup.ety_dsa_enable);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.ety_dsa_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_ipv6_5t_enable_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_ipv6_5t_enable(&tpm_setup.ipv6_5t_enable);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.ipv6_5t_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_split_mod_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_split_mod_enable(&tpm_setup.split_mod_config.split_mod_enable);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.split_mod_config.split_mod_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_split_mot_p_bits(&tpm_setup.split_mod_config.p_bit[0], &tpm_setup.split_mod_config.p_bit_num);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.split_mod_config.p_bit_num = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_split_mod_vlan_num(&tpm_setup.split_mod_config.vlan_num);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.split_mod_config.vlan_num = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	app_rc = get_split_mod_mode(&tpm_setup.split_mod_config.split_mod_mode);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.split_mod_config.split_mod_mode = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_ctc_cm_enable_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_ctc_cm_enable(&tpm_setup.ctc_cm_enable);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.ctc_cm_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}

int32_t tpm_init_ctc_cm_ipv6_parse_window_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;
	app_rc = get_ctc_cm_ipv6_parse_window(&tpm_setup.ctc_cm_ipv6_parse_window);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.ctc_cm_ipv6_parse_window = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}
int32_t tpm_init_pnc_mac_learn_enable_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_pnc_mac_learn_enable(&tpm_setup.pnc_mac_learn_enable);

	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.pnc_mac_learn_enable = MV_TPM_UN_INITIALIZED_INIT_PARAM;

	return rc;
}
int32_t tpm_init_switch_init_get_para(void)
{
	int32_t rc = TPM_OK;
	int app_rc;

	app_rc = get_switch_init_params(&(tpm_setup.switch_init));
	if (app_rc == TPM_FAIL)
		rc = TPM_FAIL;
	else if (app_rc == TPM_NOT_FOUND)
		tpm_setup.switch_init = MV_TPM_UN_INITIALIZED_INIT_PARAM;
	return rc;
}

/*******************************************************************************
* module_tpmSetup()
*
* DESCRIPTION:
*
* INPUTS:
*
*
* OUTPUTS:
*
*
* RETURNS:
*
*
* COMMENTS:
*
*******************************************************************************/
int32_t module_tpmSetup(tpm_setup_t *tpm_initp)
{
	int32_t rc;

	if (0 != tpm_initp->xml_file_path[0]) {
		g_pstr_xml_cfg_file = tpm_initp->xml_file_path;
		printk(KERN_ERR "ERROR: (%s:%d) change to cfg file path: %s \n", __func__, __LINE__,
		       g_pstr_xml_cfg_file);
	} else {
		printk(KERN_ERR "WARNING: using default (%s:%d) cfg file path: %s \n", __func__, __LINE__,
		       g_pstr_xml_cfg_file);
	}

	tpm_init_validation_get_para();

	/* Initialized as sem_full
	if (osSemCreate(&tpmApiSemId, "/TpmApiSem", 1, SEM_Q_FIFO, TRUE) != IAMBA_OK) {
		printf("module_tpmSetup: Failed to create TPM API semaphore\n\r");
		return ERROR;
	}
	 */

	tpm_init_pon_type_get_para();
	tpm_init_vlan_etypes_get_para();
	tpm_init_vitual_uni_info_get_para();
	tpm_init_default_tag_tpid_get_para();
	tpm_init_cfg_pnc_parse_get_para();
	tpm_init_cpu_loopback_get_para();
	tpm_init_trace_debug_info_get_para();
	tpm_init_omci_get_para();
#if 0
	tpm_init_debport_get_para();
#endif
	tpm_init_igmp_get_para();
	tpm_init_mc_get_para();
	/* jinghua add for MTU setting */
	tpm_init_mtu_get_para();
	tpm_init_fc_get_para();
	tpm_init_eth_cmplx_profile_get_para();
	tpm_init_eth_port_conf_get_para();
	tpm_init_gmac_port_conf_get_para();
	tpm_init_backup_wan_get_para();
	tpm_init_gmac_conn_get_para();
	tpm_init_gmac_rx_get_para();
	tpm_init_gmac_tx_get_para();
	tpm_init_gmac_mh_en_get_para();
	tpm_init_gmac_pool_bufs_para();
	tpm_init_pnc_config_get_para();
	tpm_init_ds_mh_config_get_para();
	tpm_init_mod_config_get_para();
	tpm_init_ety_dsa_enable_get_para();
	tpm_init_ipv6_5t_enable_get_para();
	tpm_init_split_mod_get_para();
	tpm_init_ctc_cm_enable_get_para();
	tpm_init_ctc_cm_ipv6_parse_window_get_para();
	tpm_init_pnc_mac_learn_enable_get_para();
	tpm_init_switch_init_get_para();

	free_xml_head_ptr();

	rc = tpm_module_start(&tpm_setup);
	if (rc != TPM_OK)
		return (rc);

	return (TPM_OK);
}

#if 0
/*******************************************************************************
* getTpmDrvFd()
*
* DESCRIPTION:   Get TPM driver file descriptor
*
* INPUTS:
*
*
* OUTPUTS:
*
*
* RETURNS:
*
*
* COMMENTS:
*
*******************************************************************************/
int getTpmDrvFd(void)
{
	return (tpmDrvFd);
}

/*******************************************************************************
* ponKernelExist()
*
* DESCRIPTION:   Verify whether the TPM driver exists
*
* INPUTS:
*
*
* OUTPUTS:
*
*
* RETURNS:
*
*
* COMMENTS:
*
*******************************************************************************/
void tpmKernelExist(void)
{
	FILE *fptr;
	uint8_t *filename = "/proc/devices";
	uint8_t line[120];
	uint8_t *token;
	uint8_t *search = " \n\r";

	fptr = fopen(filename, "r");
	if (fptr == 0) {
		printf("%s: failed to open %s\n", __func__, filename);
		return;
	}

	while (1) {
		if (fgets(line, sizeof(line), fptr) != 0) {
			token = strtok(line, search);
			if (token != NULL) {
				token = strtok(NULL, search);
				if (token != NULL) {
					if (strcmp("tpm", token) == 0) {
						tpmDrvFd = open("/dev/tpm", O_RDWR);
						if (tpmDrvFd < 0) {
							printf("Failed to open /dev/tpm\n\r");
							return;
						}

						break;
					}
				}
			}
		}
	}

	fclose(fptr);
}

char *get_TPM_sw_version()
{
	return (TPM_SW_VERSION);
}
#endif
