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
/*******************************************************************************
* tpm_init.c
*
* DESCRIPTION:
*               Traffic Processor Manager = TPM
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   OctaviaP
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               Revision: 1.13
*
*
*******************************************************************************/

#include "tpm_common.h"
#include "tpm_header.h"

/* Global  Variables */
extern spinlock_t tpmPncLock;
extern spinlock_t tpmModLock;
extern spinlock_t tpmTcamAgingLock;

/* Local definitions */
#define TPM_L2_BASE_LUID   0
#define TPM_L3_BASE_LUID   (TPM_L2_BASE_LUID + 1)
#define TPM_IPV4_BASE_LUID (TPM_L3_BASE_LUID + 1)
#define TPM_IPV6_BASE_LUID (TPM_IPV4_BASE_LUID + 3)

#define IF_ERROR(ret)       \
	if (ret != TPM_OK) {\
		TPM_OS_ERROR(TPM_INIT_MOD, " recvd ret_code(%d)\n", ret);\
		return(ret);\
	}
#define IF_ERROR_I(ret, i)  \
	if (ret != TPM_OK) {\
		TPM_OS_ERROR(TPM_INIT_MOD, " For index(%d), recvd ret_code(%d)\n", i, ret);\
		return(ret);\
	}

/* Local Variables */
static tpm_init_t tpm_init;

static tpm_tx_mod_hal_map_t tpm_tx_mod_hal_attrs[TPM_MAX_NUM_TX_PORTS] = {
	/* <TCONT number> */
	/*TPM_TX_MOD_GMAC0 */ {0, 0},
	/*TPM_TX_MOD_GMAC1 */ {1, 0},
	/*TPM_TX_MOD_PMAC_0 */ {2, 0},
	/*TPM_TX_MOD_PMAC_1 */ {2, 1},
	/*TPM_TX_MOD_PMAC_2 */ {2, 2},
	/*TPM_TX_MOD_PMAC_3 */ {2, 3},
	/*TPM_TX_MOD_PMAC_4 */ {2, 4},
	/*TPM_TX_MOD_PMAC_5 */ {2, 5},
	/*TPM_TX_MOD_PMAC_6 */ {2, 6},
	/*TPM_TX_MOD_PMAC_7 */ {2, 7}
};


#define INIT_GMAC_VALID(gmac)							\
	(((TPM_ENUM_PMAC == gmac) && (TPM_NONE != tpm_init.pon_type)) ||	\
	 ((1 == tpm_init.gmac_port_conf[gmac].valid) && (TPM_GMAC_CON_DISC != tpm_init.gmac_port_conf[gmac].conn)))

uint32_t opt_tbl[] = { 	ESC_OPT_RGMIIA_MAC0, ESC_OPT_RGMIIA_MAC1, ESC_OPT_RGMIIA_SW_P5, ESC_OPT_RGMIIA_SW_P6,
			 	ESC_OPT_RGMIIB_MAC0, ESC_OPT_MAC0_2_SW_P4, ESC_OPT_MAC1_2_SW_P5, ESC_OPT_GEPHY_MAC1,
			    	ESC_OPT_GEPHY_SW_P0, ESC_OPT_GEPHY_SW_P5, ESC_OPT_FE3PHY, ESC_OPT_SGMII_2_SW_P1, ESC_OPT_SGMII,
				ESC_OPT_QSGMII,	ESC_OPT_SATA, ESC_OPT_SGMII_2_5 };
char *opt_str_tlb[] = { "RGMIIA_MAC0", "RGMIIA_MAC1", "RGMIIA_SW_P5", "RGMIIA_SW_P6",
		 	"RGMIIB_MAC0", "MAC0_2_SW_P4", "MAC1_2_SW_P5", "GEPHY_MAC1",
		    	"GEPHY_SW_P0", "GEPHY_SW_P5", "FE3PHY", "PCS", "SGMII", "QSGMII", "SATA" , "SGMII_2_5"};
char *prof_str_tlb[] = { "", "PON_WAN_DUAL_MAC_INT_SWITCH", 	"PON_WAN_G0_INT_SWITCH",
			"PON_WAN_G1_LAN_G0_INT_SWITCH", 	"G0_WAN_G1_INT_SWITCH",
		 	"G1_WAN_G0_INT_SWITCH", 		"PON_G1_WAN_G0_INT_SWITCH",
			"PON_G0_WAN_G1_INT_SWITCH",		"PON_WAN_DUAL_MAC_EXT_SWITCH",
		    	"PON_WAN_G1_MNG_EXT_SWITCH",		"PON_WAN_G0_SINGLE_PORT",
			"PON_WAN_G1_SINGLE_PORT",		"PON_G1_WAN_G0_SINGLE_PORT",
			"PON_G0_WAN_G1_SINGLE_PORT",		"PON_WAN_G0_G1_LPBK",
			"TPM_PON_WAN_G0_G1_DUAL_LAN"};

static tpm_pnc_range_lookup_map_t pnc_range_lookup_tbl[TPM_MAX_NUM_RANGES] = {
/*  Range_num             lu_id  last_range  valid */
	{TPM_PNC_MNGMT_DS, 0, 0, 1},
	{TPM_PNC_MAC_LEARN, 0, 1, 1},
	{TPM_PNC_CPU_WAN_LPBK_US, 0, 0, 1},
	{TPM_PNC_NUM_VLAN_TAGS, 0, 0, 1},
	{TPM_PNC_DS_LOAD_BALANCE, 0, 0, 1},
	{TPM_PNC_MULTI_LPBK, 0, 0, 1},
	{TPM_PNC_VIRT_UNI, 0, 0, 1},
	{TPM_PNC_LOOP_DET_US, 0, 0, 1},
	{TPM_PNC_L2_MAIN, 0, 1, 1},
	{TPM_PNC_ETH_TYPE, 1, 1, 1},
	{TPM_PNC_IGMP, 2, 0, 1},
	{TPM_PNC_IPV4_MC_DS, 2, 0, 1},
	{TPM_PNC_IPV4_MAIN, 2, 1, 1},
	{TPM_PNC_IPV4_TCP_FLAG, 3, 1, 1},
	{TPM_PNC_TTL, 4, 1, 1},
	{TPM_PNC_IPV4_PROTO, 5, 0, 1},
	{TPM_PNC_IPV4_FRAG, 5, 0, 1},
	{TPM_PNC_IPV4_LEN, 5, 1, 1},
	{TPM_PNC_IPV6_NH, 6, 1, 1},
	{TPM_PNC_IPV6_L4_MC_DS, 7, 1, 1},
	{TPM_PNC_IPV6_TCP_FLAG, 7, 1, 1},
	{TPM_PNC_IPV6_L4, 7, 1, 1},
	{TPM_PNC_IPV6_HOPL, 8, 1, 1},
	{TPM_PNC_IPV6_MC_SIP, 8, 1, 1},
	{TPM_PNC_IPV6_GEN, 8, 1, 1},
	{TPM_PNC_IPV6_MC_DS, 9, 1, 1},
	{TPM_PNC_IPV6_DIP, 9, 1, 1},
	{TPM_PNC_CNM_IPV4_PRE, 10, 1, 1},
	/* The range 'TPM_PNC_CNM_MAIN' spans over two lookup ids.
		LU_ID_CNM_BASE := 11
		LU_ID_L2_CNM   := LU_ID_CNM_BASE+0[/11]
		LU_ID_IPV4_CNM := LU_ID_CNM_BASE+1[/12] */
	{TPM_PNC_CNM_MAIN, 11, 1, 1},
	{TPM_PNC_CATCH_ALL, 0, 0, 1},

};

bool tpm_init_check_gmac_init(tpm_gmacs_enum_t gmac_i)
{
	/* Check Port Available or not */
	if (mvNetaPortCheck(gmac_i) || (NULL == mvNetaPortHndlGet(gmac_i))) {
		return false;
	}
	return true;
}

bool tpm_init_gmac_in_gateway_mode(tpm_gmacs_enum_t gmac_i)
{
	struct eth_port *pp = NULL;

	pp = mv_eth_port_by_id(gmac_i);
	if (!pp) {
		TPM_OS_WARN(TPM_INIT_MOD, "Port (%d) has not been initialized\n", gmac_i);
		return true;
	}

	if (pp->flags & MV_ETH_F_SWITCH) {
		TPM_OS_INFO(TPM_INIT_MOD, "Port (%d) is in GateWay mode, driver has already set MH\n", gmac_i);
		return true;
	}

	return false;
}

uint32_t tpm_init_check_gmac_mh_gtwy_mode(tpm_gmacs_enum_t gmac_i, uint32_t mh)
{
	struct eth_port *pp = NULL;

	pp = mv_eth_port_by_id(gmac_i);
	if (!pp) {
		TPM_OS_WARN(TPM_INIT_MOD, "Port (%d) has not been initialized\n", gmac_i);
		return TPM_OK;
	}

	if (tpm_init_gmac_in_gateway_mode(gmac_i) && (0 == mh)) {
		/* Port is in GateWay mode, MH must be enabled */
		return TPM_FAIL;
	}

	return TPM_OK;
}

void tpm_init_config_params_init(tpm_init_t *tpm_init_params)
{
	tpm_init.omci_etype = tpm_init_params->omci_etype;
	tpm_init.pon_type = tpm_init_params->pon_type;
	tpm_init.ds_mh_set_conf = tpm_init_params->ds_mh_set_conf;
	tpm_init.validation_en = tpm_init_params->validation_en;
	tpm_init.igmp_snoop = tpm_init_params->igmp_snoop;
	tpm_init.igmp_cpu_queue = tpm_init_params->igmp_cpu_queue;
	memcpy(&(tpm_init.igmp_pkt_frwd_mod), &(tpm_init_params->igmp_pkt_frwd_mod),
	       sizeof(tpm_init.igmp_pkt_frwd_mod));
	tpm_init.num_tcont_llid = tpm_init_params->num_tcont_llid;
	tpm_init.gmac0_mh_en = tpm_init_params->gmac0_mh_en;
	tpm_init.gmac1_mh_en = tpm_init_params->gmac1_mh_en;
#if 0				/*Keep to be added in future version */
	tpm_init.deb_port_valid = tpm_init_params->deb_port_valid;
	tpm_init.deb_port = tpm_init_params->deb_port;
#endif
	tpm_init.cfg_pnc_parse = tpm_init_params->cfg_pnc_parse;
	tpm_init.cpu_loopback = tpm_init_params->cpu_loopback;
	tpm_init.trace_debug_info = tpm_init_params->trace_debug_info;
	tpm_init.virt_uni_info.enabled = tpm_init_params->virt_uni_info.enabled;
	tpm_init.virt_uni_info.uni_port = tpm_init_params->virt_uni_info.uni_port;
	tpm_init.virt_uni_info.switch_port = tpm_init_params->virt_uni_info.switch_port;
	tpm_init.ipv6_5t_enable = tpm_init_params->ipv6_5t_enable;
	tpm_init.ctc_cm_enable = tpm_init_params->ctc_cm_enable;
	tpm_init.ctc_cm_ipv6_parse_window = tpm_init_params->ctc_cm_ipv6_parse_window;
	tpm_init.pnc_mac_learn_enable = tpm_init_params->pnc_mac_learn_enable;
#if 0
	tpm_init.vlan1_tpid = tpm_init_params->vlan1_tpid;
	tpm_init.vlan2_tpid = tpm_init_params->vlan2_tpid;
#endif

	memcpy(&(tpm_init.eth_port_conf), &(tpm_init_params->eth_port_conf),
	       ((sizeof(tpm_init_eth_port_conf_t)) * TPM_MAX_NUM_ETH_PORTS));
	memcpy(&(tpm_init.gmac_tx), &(tpm_init_params->gmac_tx), ((sizeof(tpm_init_gmac_tx_t)) * TPM_MAX_NUM_TX_PORTS));
	memcpy(&(tpm_init.gmac_rx), &(tpm_init_params->gmac_rx), ((sizeof(tpm_init_gmac_rx_t)) * TPM_MAX_NUM_GMACS));
	memcpy(&(tpm_init.gmac_bp_bufs), &(tpm_init_params->gmac_bp_bufs),
	       ((sizeof(tpm_init_gmac_bufs_t)) * TPM_MAX_NUM_GMACS));
	memcpy(&(tpm_init.pnc_range), &(tpm_init_params->pnc_range),
	       ((sizeof(tpm_init_pnc_range_t)) * TPM_MAX_NUM_RANGES));
	memcpy(&(tpm_init.vlan_etypes), &(tpm_init_params->vlan_etypes),
	       ((sizeof(uint32_t)) * TPM_NUM_VLAN_ETYPE_REGS));
	memcpy(&(tpm_init.tpid_opt), &(tpm_init_params->tpid_opt), sizeof(tpm_init_tpid_opt_t));
	memcpy(&(tpm_init.mod_config), &(tpm_init_params->mod_config), sizeof(tpm_init_mod_params_t));
	memcpy(&(tpm_init.mod_chain), &(tpm_init_params->mod_chain), sizeof(tpm_init_mod_chain_t));
	memcpy(&(tpm_init.mc_setting), &(tpm_init_params->mc_setting), sizeof(tpm_init_mc_setting_t));

	memcpy(&(tpm_init.mtu_config), &(tpm_init_params->mtu_config), sizeof(tpm_init_mtu_params_t));
	memcpy(&(tpm_init.port_fc_conf), &(tpm_init_params->port_fc_conf), sizeof(tpm_init_fc_params_t));
	tpm_init.pppoe_add_enable = tpm_init_params->pppoe_add_enable;
	tpm_init.num_vlan_tags = tpm_init_params->num_vlan_tags;
	tpm_init.cpu_rx_queue = tpm_init_params->cpu_rx_queue;
	tpm_init.ttl_illegal_action = tpm_init_params->ttl_illegal_action;
	tpm_init.tcp_flag_check = tpm_init_params->tcp_flag_check;
	tpm_init.catch_all_pkt_action = tpm_init_params->catch_all_pkt_action;
	tpm_init.ety_dsa_enable = tpm_init_params->ety_dsa_enable;

	tpm_init.eth_cmplx_profile = tpm_init_params->eth_cmplx_profile;
	memcpy(&(tpm_init.gmac_port_conf), &(tpm_init_params->gmac_port_conf),
		sizeof(tpm_init.gmac_port_conf));
	tpm_init.active_wan = tpm_init_params->active_wan;
	memcpy(&(tpm_init.split_mod_config), &(tpm_init_params->split_mod_config), sizeof(tpm_init_split_mod_params_t));
	tpm_init.switch_init = tpm_init_params->switch_init;
	tpm_init.ds_mac_based_trunk_enable = tpm_init_params->ds_mac_based_trunk_enable;
}

void tpm_init_pon_type_get(void)
{
	if (tpm_init.pon_type == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.pon_type = TPM_NONE;

		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_ERROR(TPM_INIT_MOD, "\n PON_TYPE - missing initialization. ");
	}
}

void tpm_init_ds_mh_set_conf_get(void)
{
	if (tpm_init.ds_mh_set_conf == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.ds_mh_set_conf = TPM_MH_SRC_RX_CTRL;
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_WARN(TPM_INIT_MOD,
				    " MH_SRC_RX_CTRL - missing initialization - set to default = TPM_MH_SRC_RX_CTRL = 0. \n");
	}
}

void tpm_init_validation_get(void)
{
	if (tpm_init.validation_en == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.validation_en = TPM_VALID_DISABLED;
}

void tpm_init_cfg_pnc_parse_get(void)
{
	if (tpm_init.cfg_pnc_parse == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.cfg_pnc_parse = TPM_CFG_PNC_PARSE_ENABLED;
}

void tpm_init_cpu_loopback_get(void)
{
	if (tpm_init.cpu_loopback == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.cpu_loopback = TPM_CPU_LOOPBACK_DISABLED;
}

void tpm_init_trace_debug_info_get(void)
{
	if (tpm_init.trace_debug_info == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.trace_debug_info = 0xE000FFFF;

	/* update the global param with debug info from xml */
	tpm_glob_trace = tpm_init.trace_debug_info;
}

void tpm_init_vitual_uni_info_get(void)
{
	if (tpm_init.virt_uni_info.enabled == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.virt_uni_info.enabled = 0;
		tpm_init.virt_uni_info.uni_port = TPM_SRC_PORT_UNI_VIRT;
		tpm_init.virt_uni_info.switch_port = 5;
	}
}

void tpm_init_omci_get(void)
{
	if (tpm_init.omci_etype == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.omci_etype = 0xBABA;
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_ERROR(TPM_INIT_MOD, "\n OMCI type - missing initialization. ");
	}
}

void tpm_init_igmp_get(void)
{
	tpm_src_port_type_t src_port;

	if (tpm_init.igmp_snoop == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.igmp_snoop = 1;
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_ERROR(TPM_INIT_MOD, "IGMP snooping - missing initialization. \n ");
	}

	if (tpm_init.igmp_cpu_queue == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.igmp_cpu_queue = 0;
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_ERROR(TPM_INIT_MOD, "IGMP cpu_queue - missing initialization. \n ");
	}

	for (src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_UNI_VIRT; src_port++) {
		if (tpm_init.igmp_pkt_frwd_mod[src_port] == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
			tpm_init.igmp_pkt_frwd_mod[src_port] = TPM_IGMP_FRWD_MODE_SNOOPING;
			if (tpm_init.validation_en == TPM_VALID_ENABLED)
				TPM_OS_ERROR(TPM_INIT_MOD, "IGMP pkt_frwd_mod port: %d - missing initialization. \n ",
					     src_port);
		}
	}

}

void tpm_init_mc_get(void)
{

	if (tpm_init.mc_setting.filter_mode == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mc_setting.filter_mode = TPM_MC_COMBINED_IP_MAC_FILTER;
#if 0
	if (tpm_init.mc_setting.igmp_mode == MV_TPM_UN_INITIALIZED_INIT_PARAM)
	   tpm_init.mc_setting.igmp_mode = TPM_MC_IGMP_SNOOPING;
#endif
	if (tpm_init.mc_setting.per_uni_vlan_xlat == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mc_setting.per_uni_vlan_xlat = false;

	if (tpm_init.mc_setting.mc_pppoe_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mc_setting.mc_pppoe_enable = false;

	if (tpm_init.mc_setting.ipv4_mc_support == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mc_setting.ipv4_mc_support = TPM_TRUE;

	if (tpm_init.mc_setting.ipv6_mc_support == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mc_setting.ipv6_mc_support = TPM_FALSE;

}

uint32_t tpm_init_pkt_len_max_us_get(void)
{
	if (TPM_GPON == tpm_init.pon_type)
		return 2000;
	else
		return 1593;
}

uint32_t tpm_init_ipv4_mtu_us_default_get(void)
{
	uint32_t ipv4_mtu_default;

	ipv4_mtu_default = tpm_init_pkt_len_max_us_get();

	/* vlan */
	ipv4_mtu_default -= (4 * tpm_init.num_vlan_tags);
	/* da + sa + ety */
	ipv4_mtu_default -= 14;

	return ipv4_mtu_default;
}

uint32_t tpm_init_ipv4_pppoe_mtu_us_default_get(void)
{
	uint32_t ipv4_pppoe_mtu_default;

	ipv4_pppoe_mtu_default = tpm_init_pkt_len_max_us_get();

	if (TPM_PPPOE_ADD_ENABLED == tpm_init.pppoe_add_enable)
		ipv4_pppoe_mtu_default -= 8;

	ipv4_pppoe_mtu_default -= (4 * tpm_init.num_vlan_tags);
	/* da + sa + ety */
	ipv4_pppoe_mtu_default -= 14;

	return ipv4_pppoe_mtu_default;
}

uint32_t tpm_init_ipv6_mtu_us_default_get(void)
{
	uint32_t ipv6_mtu_default;

	ipv6_mtu_default = tpm_init_pkt_len_max_us_get();

	ipv6_mtu_default -= (4 * tpm_init.num_vlan_tags);

	/* in ipv6 header, payload length does not contain the ipv6 header itself */
	ipv6_mtu_default -= 40;
	/* da + sa + ety */
	ipv6_mtu_default -= 14;

	return ipv6_mtu_default;
}

uint32_t tpm_init_ipv6_pppoe_mtu_us_default_get(void)
{
	uint32_t ipv6_pppoe_mtu_default;

	ipv6_pppoe_mtu_default = tpm_init_pkt_len_max_us_get();

	if (TPM_PPPOE_ADD_ENABLED == tpm_init.pppoe_add_enable)
		ipv6_pppoe_mtu_default -= 8;

	ipv6_pppoe_mtu_default -= (4 * tpm_init.num_vlan_tags);

	/* in ipv6 header, payload length does not contain the ipv6 header itself */
	ipv6_pppoe_mtu_default -= 40;
	/* da + sa + ety */
	ipv6_pppoe_mtu_default -= 14;

	return ipv6_pppoe_mtu_default;
}

void tpm_init_mtu_get(void)
{

	if (tpm_init.pppoe_add_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.pppoe_add_enable = TPM_PPPOE_ADD_ENABLED;

	if (tpm_init.num_vlan_tags == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.num_vlan_tags = 1;

	if (tpm_init.mtu_config.mtu_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mtu_config.mtu_enable = TPM_MTU_CHECK_ENABLED;

	if (tpm_init.mtu_config.ipv4_mtu_us == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mtu_config.ipv4_mtu_us = tpm_init_ipv4_mtu_us_default_get();

	if (tpm_init.mtu_config.ipv6_mtu_us == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mtu_config.ipv6_mtu_us = tpm_init_ipv6_mtu_us_default_get();

	if (tpm_init.cpu_rx_queue == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.cpu_rx_queue = 0;

	if (tpm_init.ttl_illegal_action == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.ttl_illegal_action = TPM_TTL_ZERO_ACTION_NOTHING;

	if (tpm_init.tcp_flag_check == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.tcp_flag_check = TPM_TCP_FLAG_NOT_CHECK;

	/* if the pppoe is not enabled, do not need to set
	   ipv4_pppoe_mtu_us and ipv6_pppoe_mtu_us */
	if (TPM_PPPOE_ADD_DISABLED == tpm_init.pppoe_add_enable)
		return;

	if (tpm_init.mtu_config.ipv4_pppoe_mtu_us == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mtu_config.ipv4_pppoe_mtu_us = tpm_init_ipv4_pppoe_mtu_us_default_get();

	if (tpm_init.mtu_config.ipv6_pppoe_mtu_us == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.mtu_config.ipv6_pppoe_mtu_us = tpm_init_ipv6_pppoe_mtu_us_default_get();

	/* in DS, mtu is optional, so do not set default value */

}

void tpm_init_pnc_config_get(void)
{
	if (tpm_init.catch_all_pkt_action == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.catch_all_pkt_action = TPM_PNC_LAST_ENTRY_INIT_DROP;
}

void tpm_init_ety_dsa_enable_get(void)
{
	if (tpm_init.ety_dsa_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.ety_dsa_enable = TPM_ETY_DSA_DISABLE;
}

void tpm_init_ctc_cm_enable_get(void)
{
	if (tpm_init.ctc_cm_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.ctc_cm_enable = TPM_CTC_CM_DISABLED;
}

void tpm_init_ctc_cm_ipv6_parse_window_get(void)
{
	if (tpm_init.ctc_cm_ipv6_parse_window == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.ctc_cm_ipv6_parse_window = TPM_CTC_CM_IPv6_FIRST_24B;
}

void tpm_init_split_mod_get(void)
{
	uint32_t i = 0;
	if (tpm_init.split_mod_config.split_mod_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.split_mod_config.split_mod_enable = TPM_SPLIT_MOD_DISABLED;
	if (tpm_init.split_mod_config.p_bit_num == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.split_mod_config.p_bit_num = 8;
		for(i = 0; i < tpm_init.split_mod_config.p_bit_num; i++) {
			tpm_init.split_mod_config.p_bit[i] = i;
		}
	}
	if (tpm_init.split_mod_config.vlan_num == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.split_mod_config.vlan_num = 10;
	if (tpm_init.split_mod_config.split_mod_mode == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.split_mod_config.split_mod_mode = TPM_SPLIT_MOD_MODE_CTC;
}
void tpm_init_switch_init_get(void)
{
	if (tpm_init.switch_init == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		if (   tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_SINGLE_PORT
		    || tpm_init.eth_cmplx_profile == TPM_PON_WAN_G1_SINGLE_PORT
		    || tpm_init.eth_cmplx_profile == TPM_PON_G1_WAN_G0_SINGLE_PORT
		    || tpm_init.eth_cmplx_profile == TPM_PON_G0_WAN_G1_SINGLE_PORT
		    || tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_LPBK
		    || tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_DUAL_LAN)
			tpm_init.switch_init = 0;
		else
			tpm_init.switch_init = 1;
	}
}
void tpm_init_ds_mac_based_trunk_enable_get(void)
{
	if (tpm_init.ds_mac_based_trunk_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.ds_mac_based_trunk_enable = TPM_DS_MAC_BASED_TRUNK_DISABLED;
}

void tpm_init_ipv6_5t_enable_get(void)
{
	if (tpm_init.ipv6_5t_enable == MV_TPM_UN_INITIALIZED_INIT_PARAM)
		tpm_init.ipv6_5t_enable = TPM_IPV6_5T_DISABLED;
}

#define	HW_OPT_ON(on, hw) ((hw & on)  == on)
#define	VALID_ONLY(on, hw) (((hw & on)  == on) & ((hw & ~on)  == 0))

void tpm_init_eth_cmplx_setup_error_print(uint32_t hwEthCmplx, bool sysfs_call)
{
	uint32_t i,j, off = 0;
	char buff[1024];
	uint32_t profile[7]= {0};

	off += sprintf(buff+off, "\nSelected Eth Complex Profile: %s", prof_str_tlb[tpm_init.eth_cmplx_profile]);
	off += sprintf(buff+off, "\nHW enabled options:\n\t");
	for (i = 0;  i<sizeof(opt_tbl)/sizeof(uint32_t); i++) {
		if (opt_tbl[i] & hwEthCmplx)
			off += sprintf(buff+off, "%s ", opt_str_tlb[i]);
	}

	switch (tpm_init.eth_cmplx_profile)
	{
	case TPM_PON_WAN_DUAL_MAC_INT_SWITCH:
		profile[0] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII;
		profile[2] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6;
		profile[3] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY;
		break;

	case TPM_PON_WAN_G0_INT_SWITCH:
		profile[0] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_QSGMII;
		profile[2] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6;
		profile[3] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY;
		break;

	case TPM_PON_WAN_G1_LAN_G0_INT_SWITCH:
		profile[0] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_QSGMII;
		profile[2] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY;
		profile[3] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY | ESC_OPT_RGMIIA_SW_P6;
		profile[4] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII;
		profile[5] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6;
		break;

	case TPM_G0_WAN_G1_INT_SWITCH:
		profile[0] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII;
		break;

	case TPM_G1_WAN_G0_INT_SWITCH:
		profile[0] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_QSGMII;
		profile[2] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY;
		profile[3] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY;
		profile[4] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII;
		profile[5] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6;
		break;

	case TPM_PON_G1_WAN_G0_INT_SWITCH:
		profile[0] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_QSGMII;
		profile[2] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY;
		profile[3] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY;
		profile[4] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII;
		profile[5] = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6;
		break;

	case TPM_PON_G0_WAN_G1_INT_SWITCH:
		profile[0] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY;
		profile[1] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII;
		break;

	case TPM_PON_WAN_DUAL_MAC_EXT_SWITCH:
		profile[0] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_RGMIIA_MAC1;
		break;

	case TPM_PON_WAN_G1_MNG_EXT_SWITCH:
		profile[0] = ESC_OPT_GEPHY_MAC1;
		break;

	case TPM_PON_WAN_G0_SINGLE_PORT:
		profile[0] = ESC_OPT_RGMIIB_MAC0;
		break;

	case TPM_PON_WAN_G1_SINGLE_PORT:
		profile[0] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_RGMIIA_MAC1;
		profile[1] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_GEPHY_MAC1;
		break;

	case TPM_PON_G1_WAN_G0_SINGLE_PORT:
	case TPM_PON_G0_WAN_G1_SINGLE_PORT:
		profile[0] = ESC_OPT_RGMIIB_MAC0 | ESC_OPT_RGMIIA_MAC1;
		profile[1] = ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_MAC0;
		break;

	case TPM_PON_WAN_G0_G1_LPBK:
		if (DB_88F6535_BP_ID == mvBoardIdGet())
			profile[0] = ESC_OPT_RGMIIA_MAC0 | ESC_OPT_GEPHY_MAC1;
		if (DB_88F6601_BP_ID == mvBoardIdGet())
			profile[0] = ESC_OPT_SGMII | ESC_OPT_GEPHY_MAC0 | ESC_OPT_LP_SERDES_FE_GE_PHY;
		if ((RD_88F6601_MC_ID == mvBoardIdGet())
			|| (GFLT200_ID == mvBoardIdGet()) || (GFLT110_ID == mvBoardIdGet()))
			profile[0] = ESC_OPT_GEPHY_MAC0;
		break;
	case TPM_PON_WAN_G0_G1_DUAL_LAN:
		profile[0] = ESC_OPT_GEPHY_MAC0 | ESC_OPT_RGMIIA_MAC1;
		break;
	}

	off += sprintf(buff+off, "\nProfile supported options:\n");
	for (i = 0; profile[i]; i++) {
		off += sprintf(buff+off, "\t");
		for (j = 0; j<32; j++) {
			if (profile[i] & (1<<j)) {
				off += sprintf(buff+off, "%s ", opt_str_tlb[j]);
			}
		}
		off += sprintf(buff+off, "\n");
	}

	off += sprintf(buff+off, "\n");
	if (true == sysfs_call)
		printk("%s", buff);
	else
		TPM_OS_ERROR(TPM_INIT_MOD, "%s", buff);
}

static uint32_t tpm_init_eth_cmplx_update_conf(void)
{
	uint32_t i;
	uint32_t hwEthCmplx = mvBoardEthComplexConfigGet();

	if (hwEthCmplx & (ESC_OPT_AUTO | ESC_OPT_ILLEGAL))
	{
		TPM_OS_ERROR(TPM_INIT_MOD, "\n Illegal values in mvBoardEthComplexConfigGet 0x%x\n", hwEthCmplx);
		return TPM_FAIL;
	}

	/* do not check SATA */
	hwEthCmplx &= (~ESC_OPT_SATA);

	if (tpm_init.eth_cmplx_profile == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		if (TPM_VALID_ENABLED == tpm_init.validation_en){
			TPM_OS_ERROR(TPM_INIT_MOD, "\n ETH COMPLEX PROFILE - missing initialization\n");
       		return TPM_FAIL;
		}
	}

	if (MV_TPM_UN_INITIALIZED_INIT_PARAM == tpm_init.active_wan)
		tpm_init.active_wan = TPM_ENUM_PMAC;

	/* set default values for all ports and GMACs */
	switch (tpm_init.eth_cmplx_profile)
	{
	case TPM_PON_WAN_DUAL_MAC_INT_SWITCH:
		if (!VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_SWITCH_4;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_SWITCH_5;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			if (TPM_TRUE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 0))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_RGMIIA_SW_P6, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 6))
				 	tpm_init.eth_port_conf[i].chip_connect =TPM_CONN_RGMII1;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}
		}

		if (tpm_init.virt_uni_info.enabled) {
			tpm_init.virt_uni_info.uni_port = TPM_SRC_PORT_UNI_VIRT;
			tpm_init.virt_uni_info.switch_port = TPM_GMAC1_AMBER_PORT_NUM;
		}
		break;

	case TPM_PON_WAN_G0_INT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_QSGMII), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_SWITCH_4;

		tpm_init.gmac_port_conf[1].valid = TPM_FALSE;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_DISC;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			if (TPM_TRUE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
			 	    (tpm_init.eth_port_conf[i].switch_port == 0))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_RGMIIA_SW_P6, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 6))
					tpm_init.eth_port_conf[i].chip_connect =TPM_CONN_RGMII1;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}
		}
		break;

	case TPM_PON_WAN_G1_LAN_G0_INT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_SWITCH_4;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		// tpm_init.gmac_port_conf[1].port_src = according to XML
		// tpm_init.gmac_port_conf[1].conn = configured above

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
       		if (TPM_TRUE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
       				tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 0))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_RGMIIA_SW_P6, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 6))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}

		if (VALID_ONLY((ESC_OPT_RGMIIA_MAC1 | ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_FE3PHY), hwEthCmplx) ||
		    VALID_ONLY((ESC_OPT_RGMIIA_MAC1 | ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_QSGMII), hwEthCmplx))
		{
			for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
				if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
					tpm_init.eth_port_conf[i].valid = TPM_TRUE;
					tpm_init.eth_port_conf[i].port_src = tpm_init.gmac_port_conf[1].port_src;
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
					tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
					tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;
					break;
				}
		} else if (VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6), hwEthCmplx))
		{
				for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
					if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
						tpm_init.eth_port_conf[i].valid = TPM_TRUE;
						tpm_init.eth_port_conf[i].port_src = tpm_init.gmac_port_conf[1].port_src;
						tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
						tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
						tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;
						break;
       				 }
		} else
			goto setup_err;

		break;

	case TPM_G0_WAN_G1_INT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_RGMIIA_SW_P6 |ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII), hwEthCmplx))
			goto setup_err;

		tpm_init.pon_type = TPM_NONE;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_WAN;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII2;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_SWITCH_5;

		tpm_init.gmac_port_conf[2].valid = TPM_FALSE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
			if (TPM_TRUE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 0))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_RGMIIB_MAC0, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 5))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII2;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}


		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
				 tpm_init.eth_port_conf[i].valid = TPM_TRUE;
				 tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_WAN;
				 tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII2;
				 tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
				 break;
			}
		}
		break;

	case TPM_G1_WAN_G0_INT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		tpm_init.pon_type = TPM_NONE;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_SWITCH_4;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_WAN;
		// tpm_init.gmac_port_conf[1].conn = set below

		tpm_init.gmac_port_conf[2].valid = TPM_FALSE;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
       		if (TPM_TRUE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
					(tpm_init.eth_port_conf[i].switch_port == 0))
       				tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_RGMIIA_SW_P6, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 6))
       				tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}

		if (VALID_ONLY((ESC_OPT_RGMIIA_MAC1 | ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY), hwEthCmplx) ||
		    VALID_ONLY((ESC_OPT_RGMIIA_MAC1 | ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_QSGMII), hwEthCmplx)){
			for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
				if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
					tpm_init.eth_port_conf[i].valid = TPM_TRUE;
					tpm_init.eth_port_conf[i].port_src = tpm_init.gmac_port_conf[1].port_src;
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
					tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
					tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;
					break;
				}
			}
		} else if (VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII | ESC_OPT_RGMIIA_SW_P6), hwEthCmplx)){
			for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
				if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
					tpm_init.eth_port_conf[i].valid = TPM_TRUE;
					tpm_init.eth_port_conf[i].port_src = tpm_init.gmac_port_conf[1].port_src;
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
					tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
					tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;
					break;
       			 }
			}
		} else
			goto setup_err;

		break;

	case TPM_PON_G1_WAN_G0_INT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_SWITCH_4;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_WAN;
		// tpm_init.gmac_port_conf[1].conn = set in XML

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
       		if (TPM_TRUE == tpm_init.eth_port_conf[i].valid) {
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 0))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_RGMIIA_SW_P6, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 6))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}

		if (VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY), hwEthCmplx) ||
		    VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_RGMIIA_MAC1 | ESC_OPT_QSGMII), hwEthCmplx))	{
			for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
				if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
					tpm_init.eth_port_conf[i].valid = TPM_TRUE;
					tpm_init.eth_port_conf[i].port_src = tpm_init.gmac_port_conf[1].port_src;
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
					tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
					tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;
					break;
				}
		} else if (VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_FE3PHY), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_SW_P6 | ESC_OPT_FE3PHY), hwEthCmplx) ||
			   VALID_ONLY((ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_GEPHY_MAC1 | ESC_OPT_QSGMII), hwEthCmplx)){
				for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
					if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
						tpm_init.eth_port_conf[i].valid = TPM_TRUE;
						tpm_init.eth_port_conf[i].port_src = tpm_init.gmac_port_conf[1].port_src;
						tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
						tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
						tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;
						break;
       				 }
		} else
			goto setup_err;

		break;

	case TPM_PON_G0_WAN_G1_INT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_GEPHY_SW_P0 | ESC_OPT_FE3PHY), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_RGMIIB_MAC0 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_QSGMII), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_WAN;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII2;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_SWITCH_5;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;


		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
			if (TPM_TRUE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_SWITCH;

				if (HW_OPT_ON(ESC_OPT_QSGMII, hwEthCmplx) &&
				    (tpm_init.eth_port_conf[i].switch_port >= 0) &&
				    (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_QSGMII;
				else if (HW_OPT_ON(ESC_OPT_GEPHY_SW_P0, hwEthCmplx) &&
					 (tpm_init.eth_port_conf[i].switch_port == 0))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				else if (HW_OPT_ON(ESC_OPT_FE3PHY, hwEthCmplx) &&
				    	 (tpm_init.eth_port_conf[i].switch_port >= 1) &&
				    	 (tpm_init.eth_port_conf[i].switch_port <= 3))
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_FE_PHY;
				else
				{
					TPM_OS_WARN(TPM_INIT_MOD, "switch port %d could not be configured\n",
						tpm_init.eth_port_conf[i].switch_port);
					tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_DISC;
				}
			}

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].valid = TPM_TRUE;
				tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_WAN;
				tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
				break;
			}
		}
		break;

	case TPM_PON_WAN_DUAL_MAC_EXT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_RGMIIB_MAC0 | ESC_OPT_RGMIIA_MAC1), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII2;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		}
		break;

	case TPM_PON_WAN_G1_MNG_EXT_SWITCH:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_GEPHY_MAC1), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_FALSE;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;

		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++){
			if (TPM_FALSE == tpm_init.eth_port_conf[i].valid){
				tpm_init.eth_port_conf[i].valid = TPM_TRUE;
				tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
				tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
				tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
				break;
			}
		}
		break;

	case TPM_PON_WAN_G0_SINGLE_PORT:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		/* FIXME - Add correct condition, after answers from lsp team.
		if (!VALID_ONLY((ESC_OPT_RGMIIB_MAC0), hwEthCmplx))
			goto setup_err;*/

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII2;

		tpm_init.gmac_port_conf[1].valid = TPM_FALSE;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_DISC;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		i = 0;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII2;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
		i++;
		for (; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		break;

	case TPM_PON_WAN_G1_SINGLE_PORT:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_RGMIIA_MAC1), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_GEPHY_MAC1), hwEthCmplx))
			goto setup_err;

		if (HW_OPT_ON((ESC_OPT_RGMIIA_MAC1), hwEthCmplx)){
			 tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;
			 tpm_init.eth_port_conf[0].chip_connect = TPM_CONN_RGMII1;
		}else if (HW_OPT_ON((ESC_OPT_GEPHY_MAC1), hwEthCmplx)){
			 tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;
			 tpm_init.eth_port_conf[0].chip_connect = TPM_CONN_GE_PHY;
		}

		tpm_init.gmac_port_conf[0].valid = TPM_FALSE;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_DISC;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_UNI_0;
		// tpm_init.gmac_port_conf[1].conn = set above

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		i = 0;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
		//tpm_init.eth_port_conf[i].chip_connect = set by XML
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
		i++;

		for (; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		break;

	case TPM_PON_G1_WAN_G0_SINGLE_PORT:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_RGMIIB_MAC0 | ESC_OPT_RGMIIA_MAC1), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_MAC0), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_UNI_0;

		if (HW_OPT_ON(ESC_OPT_RGMIIB_MAC0, hwEthCmplx))
			tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII2;
		else if (HW_OPT_ON(ESC_OPT_RGMIIA_MAC0, hwEthCmplx))
			tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII1;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_WAN;

		if (HW_OPT_ON(ESC_OPT_RGMIIA_MAC1, hwEthCmplx))
			tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;
		else if (HW_OPT_ON(ESC_OPT_GEPHY_MAC1, hwEthCmplx))
			tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		i = 0;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
		i++;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_WAN;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
		i++;
		for (; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		break;

	case TPM_PON_G0_WAN_G1_SINGLE_PORT:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		if (!VALID_ONLY((ESC_OPT_RGMIIB_MAC0 | ESC_OPT_RGMIIA_MAC1), hwEthCmplx) &&
		    !VALID_ONLY((ESC_OPT_GEPHY_MAC1 | ESC_OPT_RGMIIA_MAC0), hwEthCmplx))
			goto setup_err;

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_WAN;

		if (HW_OPT_ON(ESC_OPT_RGMIIB_MAC0, hwEthCmplx))
			tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII2;
		else if (HW_OPT_ON(ESC_OPT_RGMIIA_MAC0, hwEthCmplx))
			tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_RGMII1;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_UNI_0;

		if (HW_OPT_ON(ESC_OPT_RGMIIA_MAC1, hwEthCmplx))
			tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;
		else if (HW_OPT_ON(ESC_OPT_GEPHY_MAC1, hwEthCmplx))
			tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		i = 0;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_WAN;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
		i++;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
		i++;
		for (; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		break;

	case TPM_PON_WAN_G0_G1_LPBK:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		/* FIXME - Add correct condition, after answers from lsp team. */
		/*if (!VALID_ONLY((ESC_OPT_SGMII | ESC_OPT_GEPHY_MAC0 | ESC_OPT_LP_SERDES_FE_GE_PHY), hwEthCmplx))
			goto setup_err;*/

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_GE_PHY;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_ILLEGAL;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_GE_PHY;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		i = 0;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
		i++;
		for (; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		break;
	case TPM_PON_WAN_G0_G1_DUAL_LAN:
		if (1 == tpm_init.virt_uni_info.enabled)
			goto virt_uni_err;

		/* FIXME - Add correct condition, after answers from lsp team. */
		/*if (!VALID_ONLY((ESC_OPT_GEPHY_MAC0 | ESC_OPT_RGMIIA_MAC1), hwEthCmplx))
			goto setup_err;*/

		tpm_init.gmac_port_conf[0].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[0].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.gmac_port_conf[0].conn = TPM_GMAC_CON_GE_PHY;

		tpm_init.gmac_port_conf[1].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[1].port_src = TPM_SRC_PORT_UNI_1;
		tpm_init.gmac_port_conf[1].conn = TPM_GMAC_CON_RGMII1;

		tpm_init.gmac_port_conf[2].valid = TPM_TRUE;
		tpm_init.gmac_port_conf[2].port_src = TPM_SRC_PORT_WAN;

		i = 0;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_0;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_GE_PHY;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC0;
		i++;
		tpm_init.eth_port_conf[i].valid = TPM_TRUE;
		tpm_init.eth_port_conf[i].port_src = TPM_SRC_PORT_UNI_1;
		tpm_init.eth_port_conf[i].chip_connect = TPM_CONN_RGMII1;
		tpm_init.eth_port_conf[i].int_connect = TPM_INTCON_GMAC1;
		i++;
		for (; i < TPM_MAX_NUM_ETH_PORTS; i++)
			tpm_init.eth_port_conf[i].valid = TPM_FALSE;
		break;
	}

	return TPM_OK;

setup_err:
	TPM_OS_ERROR(TPM_INIT_MOD, "\n\nHW enabled options and Eth Complex setting do not comply!");
	tpm_init_eth_cmplx_setup_error_print(hwEthCmplx, false);
	return (TPM_FAIL);

virt_uni_err:
	TPM_OS_ERROR(TPM_INIT_MOD, "\n Virt-UNI not supported in profile %s\n",	prof_str_tlb[tpm_init.eth_cmplx_profile]);
	return (TPM_FAIL);
}

uint32_t tpm_init_tcont_llid_get(void)
{
	if (tpm_init.num_tcont_llid == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		tpm_init.num_tcont_llid = 1;	/* for ZTE fix - changed from 8; */
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_WARN(TPM_INIT_MOD, "\n TCONT / LLID - missing initialization - set to default <8>.\n ");
	}
	return (TPM_OK);
}

void tpm_init_gmac_mh_en_get(void)
{
	if (tpm_init.gmac0_mh_en == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		if (tpm_init_gmac_in_gateway_mode(TPM_ENUM_GMAC_0))
			tpm_init.gmac0_mh_en = TPM_TRUE;
		else
			tpm_init.gmac0_mh_en = TPM_FALSE;
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_ERROR(TPM_INIT_MOD, "\n GMAC_0 MH - missing initialization. ");
	} else if (tpm_init.gmac0_mh_en == 2) {
		if (tpm_init.gmac_port_conf[0].conn == TPM_GMAC_CON_SWITCH_4)
			tpm_init.gmac0_mh_en = 1;
		else
			tpm_init.gmac0_mh_en = 0;
	}

	if (tpm_init.gmac1_mh_en == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
		if (tpm_init_gmac_in_gateway_mode(TPM_ENUM_GMAC_1))
			tpm_init.gmac1_mh_en = TPM_TRUE;
		else
			tpm_init.gmac1_mh_en = TPM_FALSE;
		if (tpm_init.validation_en == TPM_VALID_ENABLED)
			TPM_OS_ERROR(TPM_INIT_MOD, "\n GMAC_1 MH - missing initialization. ");
	} else if (tpm_init.gmac1_mh_en == 2) {
		if (tpm_init.gmac_port_conf[1].conn == TPM_GMAC_CON_SWITCH_5)
			tpm_init.gmac1_mh_en = 1;
		else
			tpm_init.gmac1_mh_en = 0;
	}
}

void tpm_init_lookup_get(tpm_pnc_ranges_t range_num, uint32_t *lu_id, uint32_t *last_lu_range, uint32_t *valid)
{
	uint32_t i;

	*valid = 0;

	for (i = 0; i < (sizeof(pnc_range_lookup_tbl) / sizeof(pnc_range_lookup_tbl[0])); i++) {
		if (range_num == pnc_range_lookup_tbl[i].pnc_range) {
			*lu_id = pnc_range_lookup_tbl[i].lookup_id;
			*last_lu_range = pnc_range_lookup_tbl[i].last_lookup_range;
			*valid = pnc_range_lookup_tbl[i].valid;
		}
	}
	return;
}

int32_t tpm_init_config_params_update(void)
{

	int32_t rc;

	tpm_init_trace_debug_info_get();
	tpm_init_pon_type_get();
	tpm_init_ds_mh_set_conf_get();
	tpm_init_ipv6_5t_enable_get();
	tpm_init_ctc_cm_ipv6_parse_window_get();
	tpm_init_ctc_cm_enable_get();
	tpm_init_vitual_uni_info_get();
	tpm_init_omci_get();
	tpm_init_igmp_get();
	tpm_init_cpu_loopback_get();
	tpm_init_mtu_get();
	tpm_init_cfg_pnc_parse_get();
	tpm_init_mc_get();
	rc = tpm_init_eth_cmplx_update_conf();
	if (rc != TPM_OK) {
		TPM_OS_FATAL(TPM_INIT_MOD, "\n Ethernet Complex configuration failed!\n");
		return (TPM_FAIL);
	}

	rc = tpm_init_tcont_llid_get();	/* LLID / TCONT taken from XML */
	if (rc != TPM_OK) {
		TPM_OS_FATAL(TPM_INIT_MOD, "\n GMAC internal configuration READ failed!.\n");
		return (TPM_FAIL);
	}

	tpm_init_gmac_mh_en_get();
	tpm_init_pnc_config_get();
	tpm_init_ety_dsa_enable_get();
	tpm_init_split_mod_get();
	tpm_init_switch_init_get();
	tpm_init_ds_mac_based_trunk_enable_get();

	return (TPM_OK);
}

int32_t tpm_init_info_validate(void)
{
	int i = 0, j = 0, rc = 0;
	tpm_init_tx_mod_t tx_mod;
	unsigned int config_pnc_parser_val;
	int txp, txq, min_tcont_llid;
	tpm_gmacs_enum_t gmac_i;
	uint8_t found_ipv4_pre, found_cnm_main;
	uint32_t ipv4_pre_size, cnm_main_size, exp_range_size, num_uni_ports = 0;
	uint32_t gmac0_owner = 0;
	uint32_t cpu_owner = 0;

	/********************************************************************/
	if (tpm_init.validation_en != TPM_VALID_ENABLED)
		return TPM_OK;

	/******************** EPON/GPON system - check num of LLID / TCONT : legal values: 1..8 *********************/
	if ((tpm_init.pon_type == TPM_EPON) || (tpm_init.pon_type == TPM_GPON)) {
		if ((tpm_init.num_tcont_llid <= 0) || (tpm_init.num_tcont_llid > 8)) {
			TPM_OS_FATAL(TPM_INIT_MOD, "\n TCONT/LLID: illegal value(%d) => legal values <1-8>.\n",
				tpm_init.num_tcont_llid);
			return (TPM_FAIL);
		}
	}
	/***** EPON case: validate vs .config value for EPON *****/
	if (tpm_init.pon_type == TPM_EPON) {
		min_tcont_llid = min(TPM_GPON_MAX_NUM_OF_T_CONTS, TPM_EPON_MAX_MAC_NUM);
		if (tpm_init.num_tcont_llid > min_tcont_llid) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n LLID: illegal value(%d) => max legal value defined in kernel is %d.\n",
				tpm_init.num_tcont_llid, TPM_EPON_MAX_MAC_NUM);
			return (TPM_FAIL);
		}
	}
	/***** GPON case: validate vs .config value for GPON *****/
	if (tpm_init.pon_type == TPM_GPON) {
		if (tpm_init.num_tcont_llid > TPM_GPON_MAX_NUM_OF_T_CONTS) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n TCONT: illegal value(%d) => max legal value defined in kernel is %d.\n",
				tpm_init.num_tcont_llid, TPM_GPON_MAX_NUM_OF_T_CONTS);
			return (TPM_FAIL);
		}
	}
#if 0				/*Keep to be added in future version */
/******************** Debug port setting - validation **********************************/
	if ((tpm_init.deb_port_valid != 0) && (tpm_init.deb_port_valid != 1)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			"\n Debug port valid is wrong => legal values <0=invalid/1=valid>. \n");
		return (TPM_FAIL);
	}
	if ((tpm_init.deb_port_valid == 1) &&
	    ((tpm_init.deb_port < TPM_SRC_PORT_UNI_0) || (tpm_init.deb_port > TPM_SRC_PORT_UNI_3))) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			"\n Bad debug port => legal values <TPM_SRC_PORT_UNI_0-TPM_SRC_PORT_UNI_3>. \n");
		return (TPM_FAIL);
	}
#endif
	/********************* pon type validation *********************************************/
	/* for FPGA systems - the WAN tech is defined as TPM_NONE */
	if (tpm_init.pon_type > TPM_NONE) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			"\n pon type: wrong init value(%d) => legal values "
			"<%d=TPM_EPON/%d=TPM_GPON/%d=TPM_P2P/%d=TPM_NONE>. \n",
			tpm_init.pon_type, TPM_EPON, TPM_GPON, TPM_P2P, TPM_NONE);
		return (TPM_FAIL);
	}

	/********************* CFG PNC PARSE validation *******************************************/
	if ((tpm_init.cfg_pnc_parse < TPM_CFG_PNC_PARSE_DISABLED)
	    || (tpm_init.cfg_pnc_parse > TPM_CFG_PNC_PARSE_ENABLED)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			"\n CFG PNC parse: wrong init value(%d) => legal values <0=DISABLED/1=ENABLED. \n",
			tpm_init.cfg_pnc_parse);
		return (TPM_FAIL);
	}
	/* get the config_pnc_parser value */
	config_pnc_parser_val = mv_eth_ctrl_pnc_get();

        /* logical validation */

#ifdef CONFIG_MV_ETH_PNC
	if (tpm_init.cfg_pnc_parse == 0) {
		if (config_pnc_parser_val == 0) {
			/* the intention is to give the control to mv neta PNC configuration
			   do not permit moving from 0 to 1 */
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n CFG PNC bad value: PNC in LSP cannot move from 0 to 1 \n");
			return (TPM_FAIL);
		} else {
			/* config_pnc_parser == 1 */
			/* nothing to do - the control is in LSP config */
		}
	}
	if (tpm_init.cfg_pnc_parse == 1) {
		if (config_pnc_parser_val == 0) {
			/* nothing to do - the control is in TPM */
		} else {	/* config_pnc_parser == 1 */
			/* set the config_pnc_parser to 0 - control is set to TPM */
			rc = mv_eth_ctrl_pnc(0);
			if (rc != 0) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"\n Failed to SET the config PNC parse parameter. \n");
				return (TPM_FAIL);
			}
		}
	}
#else
	/* if compilation flag is turned off - there are no relevant functions for PNC_PARSER
	   therefore do not permit the flag to be 0 - meaning the LSP is taking the responsibility */
	if (tpm_init.cfg_pnc_parse == 0) {
		TPM_OS_FATAL(TPM_INIT_MOD, "\n CFG PNC bad value: PNC in LSP does not support PNC PARSER \n");
		return (TPM_FAIL);
	} else {
		if (config_pnc_parser_val == 0) {
			/*do nothing */
		} else {
			/* set the config_pnc_parser to 0 - control is set to TPM */
			rc = mv_eth_ctrl_pnc(0);
			if (rc != 0) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"\n Failed to SET the config PNC parse parameter. \n");
				return (TPM_FAIL);
			}
		}
	}
#endif

	/********************* CPU loopback type validation  ********************************************/
		if ((tpm_init.cpu_loopback < TPM_CPU_LOOPBACK_DISABLED)
		    || (tpm_init.cpu_loopback > TPM_CPU_LOOPBACK_ENABLED)) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n CPU loopback type: wrong init value(%d) => legal values <0=disabled/1=enabled. \n",
				tpm_init.cpu_loopback);
			return (TPM_FAIL);
		}
		if (tpm_init.cpu_loopback == TPM_CPU_LOOPBACK_ENABLED) {
			if (tpm_init.gmac_port_conf[1].valid == TPM_TRUE &&
			    tpm_init.gmac_port_conf[1].port_src != TPM_SRC_PORT_ILLEGAL) {
				TPM_OS_FATAL(TPM_INIT_MOD, "\n CPU loopback not supported for GMAC function \n");
				return (TPM_FAIL);
			}
		}

	/********************* TRACE DEBUG INFO validation *******************************************/
	if (tpm_init.trace_debug_info == 0) {
		TPM_OS_WARN(TPM_INIT_MOD,
			"\n TRACE DEBUG info: init value is %d - no ERRORs will be displayed. \n ",
			tpm_init.trace_debug_info);
	}

	/********************* IGMP snooping validation *********************************************/
	if ((tpm_init.igmp_snoop != 0) && (tpm_init.igmp_snoop != 1)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n IGMP snooping: wrong init value(%d) => legal values <0=disabled/1=enabled>. \n",
			     tpm_init.igmp_snoop);
		return (TPM_FAIL);
	}
	if (tpm_init.igmp_snoop == 1) {
		if (tpm_init.igmp_cpu_queue > 7) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				     "\n IGMP snooping: wrong CPU queue(%d) => legal values <0-7>. \n",
				     tpm_init.igmp_cpu_queue);
			return (TPM_FAIL);
		}
	}

	/********************* Multicast validation *********************************************/
	if (tpm_init.mc_setting.per_uni_vlan_xlat) {
		if (tpm_init.mc_setting.filter_mode != TPM_MC_COMBINED_IP_MAC_FILTER) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				     "\n multicast per uni vlan translation is not supported in filter_mode (%d). \n",
				     tpm_init.mc_setting.filter_mode);
			return (TPM_FAIL);
		}
#if 0
		if (tpm_init.mc_setting.igmp_mode == TPM_MC_IGMP_SNOOPING && tpm_init.mc_setting.mc_pppoe_enable) {
		   TPM_OS_FATAL(TPM_INIT_MOD, "\n multicast per uni vlan translation is not supported "
					"in igmp snooping over pppoe. \n");
		   return(TPM_FAIL);
		}
#endif
	}

	/* check that igmp_cpu_queue is CPU's reserved queue */
	/* oct*>>> to do - if per system Q6 is the IGMP CPU - check in all GMACs that Q6 is of CPU ownership */

	if (tpm_init.mc_setting.mc_hwf_queue > 7) {
		TPM_OS_FATAL(TPM_INIT_MOD, "\n MC setting: wrong MC HWF queue(%d) => legal values <0-7>. \n",
			     tpm_init.mc_setting.mc_hwf_queue);
		return (TPM_FAIL);
	}

	if (tpm_init.mc_setting.mc_cpu_queue > 7) {
		TPM_OS_FATAL(TPM_INIT_MOD, "\n MC setting: wrong MC CPU queue(%d) => legal values <0-7>. \n",
			     tpm_init.mc_setting.mc_cpu_queue);
		return (TPM_FAIL);
	}

	/********************** GMAC_0 connectivity validation *************************************/
	if ((tpm_init.gmac_port_conf[0].conn < TPM_GMAC_CON_DISC) || (tpm_init.gmac_port_conf[0].conn > TPM_GMAC_CON_GE_PHY)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n GMAC_0 connectivity: wrong init value(%d) => legal values <0-7> \n",
			     tpm_init.gmac_port_conf[0].conn);
		return (TPM_FAIL);
	}
	/********************** GMAC_1 connectivity validation *************************************/
	if ((tpm_init.gmac_port_conf[1].conn < TPM_GMAC_CON_DISC) || (tpm_init.gmac_port_conf[1].conn > TPM_GMAC_CON_GE_PHY)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n GMAC_1 connectivity: wrong init value(%d) => legal values <0-7> \n",
			     tpm_init.gmac_port_conf[1].conn);
		return (TPM_FAIL);
	}
	/********************** GMAC_0 MH enable validation   *************************************/
	if ((tpm_init.gmac0_mh_en != 0) && (tpm_init.gmac0_mh_en != 1)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n GMAC_0 MH enable: wrong init value(%d) => legal values <0=disabled,1=enabled> \n",
			     tpm_init.gmac0_mh_en);
		return (TPM_FAIL);
	}
	rc = tpm_init_check_gmac_mh_gtwy_mode(TPM_ENUM_GMAC_0, tpm_init.gmac0_mh_en);
	if (rc != TPM_OK) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n GMAC_0 is in GateWay mode, MH can not be disabled\n");
		return (TPM_FAIL);
	}
	/********************** GMAC_1 MH enable validation   *************************************/
	if ((tpm_init.gmac1_mh_en != 0) && (tpm_init.gmac1_mh_en != 1)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n GMAC_1 MH enable: wrong init value(%d) => legal values <0=disabled,1=enabled> \n",
			     tpm_init.gmac1_mh_en);
		return (TPM_FAIL);
	}
	rc = tpm_init_check_gmac_mh_gtwy_mode(TPM_ENUM_GMAC_1, tpm_init.gmac1_mh_en);
	if (rc != TPM_OK) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n GMAC_1 is in GateWay mode, MH can not be disabled\n");
		return (TPM_FAIL);
	}
	/********************** GMAC_ Buffer Mngmt Pool_sizes validation ***********************************/
	for (i = 0; i < sizeof(tpm_init.gmac_bp_bufs) / sizeof(tpm_init_gmac_bufs_t); i++) {
		if (tpm_init.gmac_bp_bufs[i].valid) {
			if (((tpm_init.gmac_bp_bufs[i].large_pkt_buffers != 0) &&
			     (tpm_init.gmac_bp_bufs[i].large_pkt_buffers < MV_BM_POOL_CAP_MIN)) ||
			    ((tpm_init.gmac_bp_bufs[i].small_pkt_buffers != 0) &&
			     (tpm_init.gmac_bp_bufs[i].small_pkt_buffers < MV_BM_POOL_CAP_MIN))) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"\n GMAC%d BM Pool has too small buffer assigment "
					"large_buf %d, small_buf %d\n",
					i, tpm_init.gmac_bp_bufs[i].large_pkt_buffers,
					tpm_init.gmac_bp_bufs[i].small_pkt_buffers);
				return (TPM_FAIL);
			}
		}
	}
	/********************** PNC MH enabled allow for DS   *************************************/
	if ((tpm_init.ds_mh_set_conf != 0) && (tpm_init.ds_mh_set_conf != 1)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			"\n PNC - DS MH allow: wrong init value(%d) => "
			"legal values <0=MH not allowed/DS,1=MH allowed/DS> \n",
			tpm_init.ds_mh_set_conf);
		return (TPM_FAIL);
	}

	if (tpm_init.port_fc_conf.enabled) {
		MV_U32 device_id = mvCtrlModelGet();

		if (device_id != MV_6601_DEV_ID) {
			TPM_OS_FATAL(TPM_INIT_MOD, "\n port SW Flow-Control is not supported by this device");
			return (TPM_FAIL);
		}

		if ((tpm_init.port_fc_conf.port > TPM_MAX_GMAC) 	||
		    (tpm_init.port_fc_conf.tgt_port > TPM_MAX_GMAC)	||
		    (tpm_init.port_fc_conf.tx_port > TPM_MAX_GMAC)){
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n port SW Flow-Control invalid port number:"
				"port=%d tgt_port=%d tx_port=%d\n",
				tpm_init.port_fc_conf.port,
				tpm_init.port_fc_conf.tgt_port,
				tpm_init.port_fc_conf.tx_port);
			return (TPM_FAIL);
		}

		if (tpm_init.port_fc_conf.tx_queue >= TPM_MAX_NUM_TX_QUEUE){
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n port SW Flow-Control invalid port number: tx_queue=%d",
				tpm_init.port_fc_conf.tx_queue);
			return (TPM_FAIL);
		}
	}
	/********************** ethernet ports validation ******************************************/
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (tpm_init.eth_port_conf[i].valid == TPM_TRUE) {
			if ((tpm_init.eth_port_conf[i].chip_connect < TPM_CONN_DISC) ||
					(tpm_init.eth_port_conf[i].chip_connect > TPM_CONN_RGMII2)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"\n ETH_port[(%d)]: chip_connect - wrong init value(%d)"
					" => legal values <0-5> \n",
					i, tpm_init.eth_port_conf[i].chip_connect);
				return (TPM_FAIL);
			}
			if ((tpm_init.eth_port_conf[i].int_connect < TPM_INTCON_GMAC0) ||
					(tpm_init.eth_port_conf[i].int_connect > TPM_INTCON_SWITCH)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"\n ETH_port[(%d)]: chip_connect - wrong init "
					"value(%d) => legal values <0-2> \n",
					i, tpm_init.eth_port_conf[i].int_connect);
				return (TPM_FAIL);
			}
			if ((tpm_init.eth_port_conf[i].int_connect == TPM_INTCON_SWITCH) &&
					(tpm_init.eth_port_conf[i].switch_port > 6)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"\n ETH_port[(%d)]: switch_port - wrong init "
					"value(%d) => legal values <0-6> \n",
					i, tpm_init.eth_port_conf[i].switch_port);
				return (TPM_FAIL);
			}

			if ((tpm_init.eth_port_conf[i].int_connect == TPM_INTCON_SWITCH) &&
				 (tpm_init.eth_port_conf[i].switch_port <= 6)) {
				num_uni_ports++;
			}
		}
	}
	/********************* Virtual UNI validation *************************************/

	if ((tpm_init.virt_uni_info.enabled < TPM_VIRT_UNI_DISABLED) ||
			(tpm_init.virt_uni_info.enabled > TPM_VIRT_UNI_ENABLED)) {
		TPM_OS_FATAL(TPM_INIT_MOD,
			"\n Virtual UNI: wrong init value(%d) => legal "
			"values <0=TPM_WIFI_VIRT_UNI_DISABLED/1=TPM_VIRT_UNI_ENABLED. \n",
			tpm_init.virt_uni_info.enabled);
		return (TPM_FAIL);
	}
	if (tpm_init.virt_uni_info.enabled == TPM_VIRT_UNI_ENABLED) {
	/*oct* - open this validation in next LSP - meantime it works only on RD */
#if 0
		/* check that GMAC1 is connected to internal switch port #5 */
		rc = mvBoardIsInternalSwitchConnected(1);
		if (rc == 0) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n WiFi virtual UNI: feature ENABLED - GMAC1 "
				"is NOT HW-connected to Switch  port #5. \n");
			return (TPM_FAIL);
		}
#endif
		/* fail eth complex other than dual MAC */
		if (TPM_PON_WAN_DUAL_MAC_INT_SWITCH != tpm_init.eth_cmplx_profile)
		{
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n Virtual UNI suppoerted only by ethernet Complex %x (used %x) \n",
				TPM_PON_WAN_DUAL_MAC_INT_SWITCH, tpm_init.eth_cmplx_profile);
			return (TPM_FAIL);
		}

		/* currently support only UNI_VIRT port for WIFI virtual UNI port */
		if (tpm_init.virt_uni_info.uni_port != TPM_SRC_PORT_UNI_VIRT) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n Virtual UNI: wrong port value(%d) => "
				"legal values <0-%d> - default value<%d. \n",
				tpm_init.virt_uni_info.uni_port, TPM_SRC_PORT_UNI_VIRT ,TPM_SRC_PORT_UNI_VIRT);
			return (TPM_FAIL);
		}

		num_uni_ports++;

		/* if feature enabled and missing PNC range in the xml - return ERROR */
		for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
			if (tpm_init.pnc_range[i].range_num == TPM_PNC_VIRT_UNI) {
				if (tpm_init.pnc_range[i].valid != TPM_TRUE) {
					TPM_OS_FATAL(TPM_INIT_MOD,
						"\n Virtual UNI: feature ENABLED - missing "
						"PNC range <TPM_PNC_VIRT_UNI> in XML config file. \n");
					return (TPM_FAIL);
				}
			}
		}	/* for */
		/* TODO: check all GMAC1 TX queues are owned by CPU */

	}

	/*if wifi feature enabled - end validation */
	/********************* PNC validations *****************************************************/
	found_ipv4_pre = found_cnm_main = TPM_FALSE;
	ipv4_pre_size = cnm_main_size = 0;

	/* Validate total number of Pnc Entries */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_init.pnc_range[i].valid == TPM_TRUE) {
			j += tpm_init.pnc_range[i].range_size;
			if (tpm_init.pnc_range[i].range_num >= TPM_MAX_NUM_RANGES) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"PNC range[%d]: range_num - wrong init value(%d) => "
					"legal values <0-%d>\n",
					i, tpm_init.pnc_range[i].range_num, TPM_MAX_RANGE);
				return (TPM_FAIL);
			}
			if (tpm_init.pnc_range[i].range_type > TPM_RANGE_TYPE_TABLE) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"PNC range[%d]: range_type - wrong init value(%d) => "
					"legal values <0=TYPE_ACL,1=TYPE_TABLE>\n",
					i, tpm_init.pnc_range[i].range_type);
				return (TPM_FAIL);
			}
			if ((tpm_init.pnc_range[i].cntr_grp < 0) || (tpm_init.pnc_range[i].cntr_grp > 3)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					     "PNC range[%d] cntr_grp - wrong init value(%d) => legal values <0-3>\n",
					     i, tpm_init.pnc_range[i].cntr_grp);
				return (TPM_FAIL);
			}
			if ((tpm_init.pnc_range[i].lu_mask < 0) || (tpm_init.pnc_range[i].lu_mask > 1)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					     "PNC range[%d] lu_mask - wrong init value(%d) => legal values <0-1>\n",
					     i, tpm_init.pnc_range[i].lu_mask);
				return (TPM_FAIL);
			}
			if (tpm_init.pnc_range[i].range_num == TPM_PNC_CNM_IPV4_PRE) {
				found_ipv4_pre = TPM_TRUE;
				ipv4_pre_size = tpm_init.pnc_range[i].range_size;
			} else if (tpm_init.pnc_range[i].range_num == TPM_PNC_CNM_MAIN) {
				found_cnm_main = TPM_TRUE;
				cnm_main_size = tpm_init.pnc_range[i].range_size;
			}
		}
	}
	if (j > TPM_PNC_SIZE) {
		TPM_OS_FATAL(TPM_INIT_MOD, "Sum of Pnc ranges(%d) is bigger than PnC size(%d)\n", j,
			     TPM_PNC_SIZE);
		return (TPM_FAIL);
	}

	/********************* MOD validations *****************************************************/
	/* Validate TPM reserved modification entries */
	/********************* TX module validations *****************************************************/

	for (tx_mod = TPM_TX_MOD_GMAC0; tx_mod < TPM_MAX_NUM_TX_PORTS; (tx_mod)++) {
		/* validate gmac_tx - according to tpm_init.num_tcont_llid */
		if (tx_mod >= TPM_TX_MOD_PMAC_0) {
			if (((tx_mod - TPM_TX_MOD_GMAC1) > tpm_init.num_tcont_llid) &&
			    (tpm_init.gmac_tx[tx_mod].valid == 1)) {
				tpm_init.gmac_tx[tx_mod].valid = 0;
				TPM_OS_WARN(TPM_INIT_MOD,
					    " Illegal TCONT/LLID %d configuration - max legal value is %d.\n ",
					    tx_mod, tpm_init.num_tcont_llid);
			}
		}
		for (i = 0; i < TPM_MAX_NUM_TX_QUEUE; i++) {
			if (tpm_init.gmac_tx[tx_mod].tx_queue[i].valid != 1)
				continue;

			if ((tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_owner < TPM_Q_OWNER_CPU) ||
			    (tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_owner >= TPM_Q_OWNER_MAX)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"TX module queue [%d]: queue_owner - wrong init value(%d)"
					" => legal values <%d-%d>\n",
					tx_mod, tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_owner,
					TPM_Q_OWNER_CPU, TPM_Q_OWNER_PMAC);
				return (TPM_FAIL);
			}
			if (tpm_init.gmac_tx[tx_mod].tx_queue[i].owner_queue_num >= TPM_MAX_NUM_TX_QUEUE) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"TX module queue [%d]: owner_queue_num - wrong init value(%d) "
					"is bigger than maximum queue number (%d)\n",
					tx_mod, tpm_init.gmac_tx[tx_mod].tx_queue[i].owner_queue_num,
					TPM_MAX_NUM_TX_QUEUE - 1);
				return (TPM_FAIL);
			}
			if ((tpm_init.gmac_tx[tx_mod].tx_queue[i].sched_method < TPM_SCHED_SP) ||
			    (tpm_init.gmac_tx[tx_mod].tx_queue[i].sched_method > TPM_SCHED_WRR)) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"TX module queue [%d]: sched_method  - wrong init value(%d) => "
					"legal values <%d-%d>\n\n",
					tx_mod, tpm_init.gmac_tx[tx_mod].tx_queue[i].owner_queue_num,
					TPM_SCHED_SP, TPM_SCHED_WRR);
				return (TPM_FAIL);
			}
			if (tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_weight > TPM_MAX_WRR_WEIGHT) {
				TPM_OS_FATAL(TPM_INIT_MOD,
					"TX module queue [%d]: queue_weight  - wrong init value(%d) "
					"=> legal values <0-%d>\n\n",
					tx_mod, tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_weight,
					TPM_MAX_WRR_WEIGHT);
				return (TPM_FAIL);
			}
		}
	}

	/********* per GMAC - validate that default TCONT & Queue - are not set as HWF in xml *****/
	for (gmac_i = 0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		rc = mv_eth_get_txq_cpu_def(gmac_i, &txp, &txq, 0);
		if (rc != 0) {
			TPM_OS_WARN(TPM_INIT_MOD, "\n Failed to GET the default queue per GMAC%d - rc= %d. \n",
				    gmac_i, rc);
			continue;
		}
		if (gmac_i >= TPM_TX_MOD_PMAC_0) {
			if (tpm_init.gmac_tx[gmac_i + txp].tx_queue[txq].queue_owner != TPM_Q_OWNER_CPU) {
				TPM_OS_WARN(TPM_INIT_MOD,
					" Default TX queue(%d) per GMAC (%d) must not be set in "
					"hardware forwarding mode in config params.\n\n", txq, gmac_i);
			}
		} else {
			if (tpm_init.gmac_tx[gmac_i].tx_queue[txq].queue_owner != TPM_Q_OWNER_CPU) {
				TPM_OS_WARN(TPM_INIT_MOD,
					" Default TX queue(%d) per GMAC (%d) must not be set in "
					"hardware forwarding mode in config params.\n\n", txq, gmac_i);
			}
		}
	}

	/*****split mod setting validation******/
	if (tpm_init.split_mod_config.split_mod_enable == TPM_SPLIT_MOD_ENABLED) {
		if(tpm_init.split_mod_config.vlan_num > (TPM_DB_SPLIT_MOD_NUM_VLANS_MAX - TPM_DB_SPLIT_MOD_INIT_VLANS_NUM)) {
			TPM_OS_FATAL(TPM_INIT_MOD,
			     "\n Split Mod VLAN num %d, it should not larger than %d \n",
			     tpm_init.split_mod_config.vlan_num, (TPM_DB_SPLIT_MOD_NUM_VLANS_MAX - TPM_DB_SPLIT_MOD_INIT_VLANS_NUM));
			return (TPM_FAIL);
		}
		if (tpm_init.split_mod_config.p_bit_num > TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX || tpm_init.split_mod_config.p_bit_num == TPM_DB_SPLIT_MOD_P_BIT_NO_SET) {
			TPM_OS_FATAL(TPM_INIT_MOD, "\n Split Mod P_bit number out of range.\n");
			return (TPM_FAIL);
		}
		for (i = 0; i < tpm_init.split_mod_config.p_bit_num; i++) {
			if(tpm_init.split_mod_config.p_bit[i] > TPM_DB_SPLIT_MOD_P_BIT_MAX) {
				TPM_OS_FATAL(TPM_INIT_MOD,
			     		"\n Split Mod P_bit %d No Valid\n",
					tpm_init.split_mod_config.p_bit[i]);
				return (TPM_FAIL);
			}
			for (j = i + 1; j < tpm_init.split_mod_config.p_bit_num; j++) {
				if(tpm_init.split_mod_config.p_bit[i] == tpm_init.split_mod_config.p_bit[j]) {
					TPM_OS_FATAL(TPM_INIT_MOD,
			     			"\n Split Mod P_bit %d Repeat\n",
						tpm_init.split_mod_config.p_bit[i]);
					return (TPM_FAIL);
				}
			}
		}
	}

	/********************* CTC CNM validation *************************************/
	if (tpm_init.ctc_cm_enable != TPM_CTC_CM_DISABLED) {
		if (tpm_init.split_mod_config.split_mod_enable == TPM_SPLIT_MOD_DISABLED) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n CTC CM: CTC CnM is enabled while split modification is disabled! \n");
			return (TPM_FAIL);
		}

		if (!found_ipv4_pre) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n CTC CM: CTC CnM is enabled while CNM_IPV4_PRE range size is Zero! \n");
			return (TPM_FAIL);
		} else {
			exp_range_size = num_uni_ports * TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_PORT + 1;
			if (exp_range_size > ipv4_pre_size) {
				TPM_OS_WARN(TPM_INIT_MOD,
					"\n CTC CM: CNM IPV4 PRE FILTER is not enough for "
					"L2 & IPV4 combo rules (%d/%d) of %d UNI ports! \n",
					exp_range_size, ipv4_pre_size, num_uni_ports);
			}
		}

		if (!found_cnm_main) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n CTC CM: CTC CnM is enabled while CNM_MAIN range size is Zero! \n");
			return (TPM_FAIL);
		} else {
			exp_range_size = num_uni_ports * TPM_MAX_NUM_CTC_PRECEDENCE + 2;
			if (exp_range_size > cnm_main_size) {
				TPM_OS_WARN(TPM_INIT_MOD,
					"\n CTC CM: CNM MAIN is not enough for "
					"8 precedence rules (%d/%d) of %d UNI ports! \n",
					exp_range_size, cnm_main_size, num_uni_ports);
			}
		}
	} else {
		if (found_ipv4_pre) {
			TPM_OS_WARN(TPM_INIT_MOD,
				"\n CTC CM: Since CTC CnM is disabled, CNM_IPV4_PRE range size should be Zero! \n");
		}
		if (found_cnm_main) {
			TPM_OS_WARN(TPM_INIT_MOD,
				"\n CTC CM: Since CTC CnM is disabled, CNM_MAIN range size should be Zero! \n");
		}
	}

	/********************* ds_mac_based_trunking validation *************************************/
	if (tpm_init.ds_mac_based_trunk_enable == TPM_DS_MAC_BASED_TRUNK_ENABLED) {
		if (    (tpm_init.cpu_loopback == TPM_CPU_LOOPBACK_ENABLED)
		     || (tpm_init.mc_setting.per_uni_vlan_xlat)
		     || (tpm_init.virt_uni_info.enabled == TPM_VIRT_UNI_ENABLED)) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n when ds_mac_based_trunk is enabled, cpu_loopback, per_uni_vlan_xlat"
				" and virt_uni can not be enabled! \n");
			return (TPM_FAIL);
		}

		if (    (tpm_init.gmac0_mh_en == 0)
		     || (tpm_init.gmac1_mh_en == 0)) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n when ds_mac_based_trunk is enabled, MH on GMAC0/1 must be enabled\n");
			return (TPM_FAIL);
		}

		if (    (tpm_init.eth_cmplx_profile != TPM_PON_WAN_DUAL_MAC_INT_SWITCH)
		     && (tpm_init.eth_cmplx_profile != TPM_PON_WAN_DUAL_MAC_EXT_SWITCH)) {
			TPM_OS_FATAL(TPM_INIT_MOD,
				"\n when ds_mac_based_trunk is enabled, eth_cmplx_profile must be "
				"TPM_PON_WAN_DUAL_MAC_INT_SWITCH"
				" or TPM_PON_WAN_DUAL_MAC_EXT_SWITCH! \n");
			return (TPM_FAIL);
		}

	}
	/********************* No switch init(MC) validation *************************************/
	if (tpm_init.switch_init == 0) {
		if (tpm_init.virt_uni_info.enabled == TPM_VIRT_UNI_ENABLED) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n No Switch Init: Virt UNI is not supported! \n");
			return (TPM_FAIL);
		}

		if (tpm_init.mc_setting.per_uni_vlan_xlat) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n No Switch Init: MC Per UNI Xlate is not supported! \n");
			return (TPM_FAIL);
		}

		if (tpm_init.mc_setting.filter_mode != TPM_MC_ALL_CPU_FRWD && tpm_init.mc_setting.filter_mode != TPM_MC_IP_ONLY_FILTER) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n No Switch Init: MC filter mode(%d) not supported! \n", tpm_init.mc_setting.filter_mode);
			return (TPM_FAIL);
		}

		if (tpm_init.ety_dsa_enable == TPM_ETY_DSA_ENABLE) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n No Switch Init: DSA is not supported! \n");
			return (TPM_FAIL);
		}

		if (tpm_init.gmac0_mh_en == 1) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n No Switch Init: GMAC0 Marvell header should disabled! \n");
			return (TPM_FAIL);
		}

		if (tpm_init.pnc_range[TPM_PNC_MAC_LEARN].range_size == 0 &&
		    tpm_init.pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_ENABLED) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n No Switch Init: PNC range[%d] size is 0! \n", TPM_PNC_MAC_LEARN);
			return (TPM_FAIL);
		}

		if (tpm_init.pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_DISABLED &&
		    tpm_init.pnc_range[TPM_PNC_MAC_LEARN].range_size > 1) {
			TPM_OS_WARN(TPM_INIT_MOD,
					"\n No Switch Init: MAC learn disabled, PNC range[%d] size is too bigger! \n", TPM_PNC_MAC_LEARN);
		}

		if (tpm_init.pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_ENABLED &&
		    tpm_init.pnc_range[TPM_PNC_MAC_LEARN].range_size <= 1) {
			TPM_OS_WARN(TPM_INIT_MOD,
					"\n No Switch Init: MAC learn enabled, PNC range[%d] size is too small! \n", TPM_PNC_MAC_LEARN);
		}

		if (tpm_init.pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_ENABLED &&
		    tpm_init.eth_cmplx_profile != TPM_PON_WAN_G0_G1_LPBK) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n PNC MAC learning not supported with profile (%d) \n",
					tpm_init.eth_cmplx_profile);
			return (TPM_FAIL);
		}

		if (tpm_init.ipv6_5t_enable == TPM_IPV6_5T_ENABLED &&
		    tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_DUAL_LAN) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n IPV6 5-tuple supported with profile (%d) \n",
					tpm_init.eth_cmplx_profile);
			return (TPM_FAIL);
		}
	} else if ((tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_SINGLE_PORT ||
		    tpm_init.eth_cmplx_profile == TPM_PON_WAN_G1_SINGLE_PORT ||
		    tpm_init.eth_cmplx_profile == TPM_PON_G1_WAN_G0_SINGLE_PORT ||
		    tpm_init.eth_cmplx_profile == TPM_PON_G0_WAN_G1_SINGLE_PORT ||
		    tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_LPBK ||
		    tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_DUAL_LAN)
		 && tpm_init.switch_init == 1) {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n Switch can not be Init at this profile: [%d]! \n", tpm_init.eth_cmplx_profile);
			tpm_init.switch_init = 0;
	}

	/* check XML Tx queue for MC chip with profile TPM_PON_WAN_G0_G1_LPBK */
	if((MV_6601_DEV_ID == mvCtrlModelGet()) && (TPM_PON_WAN_G0_G1_LPBK == tpm_init.eth_cmplx_profile)) {
		/* check GMAC1 Tx queue configuration in XML */
		if (tpm_init.gmac_tx[TPM_TX_MOD_GMAC1].valid == TPM_TRUE) {
			for (i = 0; i < TPM_MAX_NUM_TX_QUEUE; i++) {
				if (tpm_init.gmac_tx[TPM_TX_MOD_GMAC1].tx_queue[i].valid == TPM_TRUE) {
					if (tpm_init.gmac_tx[TPM_TX_MOD_GMAC1].tx_queue[i].queue_owner == TPM_Q_OWNER_GMAC0)
						gmac0_owner++;
					if (tpm_init.gmac_tx[TPM_TX_MOD_GMAC1].tx_queue[i].queue_owner == TPM_Q_OWNER_CPU)
						cpu_owner++;
				}
			}
			if (gmac0_owner == 0) {
				TPM_OS_FATAL(TPM_INIT_MOD,
						"\n TPM_PON_WAN_G0_G1_LPBK: MC chip GMAC1 Tx queue no assigned to GMAC0 \n");
				return (TPM_FAIL);
			} else if (gmac0_owner > 1) {
				TPM_OS_WARN(TPM_INIT_MOD,
						"\n TPM_PON_WAN_G0_G1_LPBK: One GMAC1 Tx queue assigned to GMAC0 enough! \n");
			}
			if (cpu_owner == 0) {
				TPM_OS_FATAL(TPM_INIT_MOD,
						"\n TPM_PON_WAN_G0_G1_LPBK: MC chip GMAC1 Tx queue no assigned to CPU \n");
				return (TPM_FAIL);
			}
		} else {
			TPM_OS_FATAL(TPM_INIT_MOD,
					"\n TPM_PON_WAN_G0_G1_LPBK: MC chip GMAC1 Invalid! \n");
			return (TPM_FAIL);
		}

		/* check PON Tx queue configuration in XML */
		for (i = TPM_TX_MOD_PMAC_0; i < TPM_MAX_NUM_TX_PORTS; i++) {
			if (tpm_init.gmac_tx[i].valid == TPM_TRUE) {
				for(j = 0; j < TPM_MAX_NUM_TX_QUEUE; j++) {
					if (tpm_init.gmac_tx[i].tx_queue[j].valid != TPM_TRUE ||
					    tpm_init.gmac_tx[i].tx_queue[j].queue_owner != TPM_Q_OWNER_GMAC1) {
						TPM_OS_WARN(TPM_INIT_MOD,
							"\n TPM_PON_WAN_G0_G1_LPBK: All PON-%d Tx queue should be assign to GMAC1! \n", (i - TPM_TX_MOD_PMAC_0));
						break;
					}
				}
			}
		}
	}

	/* check all the loopback features */
	if (tpm_init.mc_setting.per_uni_vlan_xlat) {
		if (tpm_init.virt_uni_info.enabled != TPM_VIRT_UNI_ENABLED) {
			TPM_OS_FATAL(TPM_INIT_MOD, "\n per_uni_vlan_xlat can not be supported when virt_uni is disabled\n");
			return (TPM_FAIL);
		}
	}
	if (tpm_init.cpu_loopback == TPM_CPU_LOOPBACK_ENABLED) {
		if (tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_LPBK)
			;/* OK */
		else if (    (tpm_init.gmac_port_conf[TPM_ENUM_GMAC_1].conn == TPM_GMAC_CON_SWITCH_5)
		          && (tpm_init.eth_cmplx_profile != TPM_G0_WAN_G1_INT_SWITCH)
		          && (tpm_init.eth_cmplx_profile != TPM_PON_G0_WAN_G1_INT_SWITCH))
			;/* OK */
		else {
			TPM_OS_FATAL(TPM_INIT_MOD, "\n cpu_wan_loopback can not be supported on this profile\n");
			return (TPM_FAIL);
		}
	}

	return (TPM_OK);
}

/*******************************************************************************
* tpm_init_tx_mod_set()
*
* DESCRIPTION:      Set the configuration of a Tx module
*
* INPUTS:
* tx_mod           -

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
void tpm_init_tx_mod_set(tpm_db_tx_mod_t tx_mod)
{
	uint32_t i;

	for (i = 0; i < TPM_MAX_NUM_TX_QUEUE; i++) {
		if (tpm_init.gmac_tx[tx_mod].tx_queue[i].valid == TPM_TRUE) {
			tpm_db_gmac_tx_q_conf_set(tx_mod, i,
						  tpm_init.gmac_tx[tx_mod].tx_queue[i].sched_method,
						  tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_owner,
						  tpm_init.gmac_tx[tx_mod].tx_queue[i].owner_queue_num,
						  tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_size,
						  tpm_init.gmac_tx[tx_mod].tx_queue[i].queue_weight);
		}
	}

}

/*******************************************************************************
* tpm_init_gmac_rxq_set()
*
* DESCRIPTION:      Set the queue configuration of a GMAC
*
* INPUTS:
* tx_mod           -

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
void tpm_init_gmac_rxq_set(tpm_gmacs_enum_t gmac)
{
	uint32_t i;

	for (i = 0; i < TPM_MAX_NUM_RX_QUEUE; i++) {
		if (tpm_init.gmac_rx[gmac].rx_queue[i].valid == TPM_TRUE)
			tpm_db_gmac_rx_q_conf_set(gmac, i, tpm_init.gmac_rx[gmac].rx_queue[i].queue_size);
	}
}

/*******************************************************************************
* tpm_init_pncranges_set()
*
* DESCRIPTION:      Initialize the PnC ranges
*
* INPUTS:
* tx_mod           -

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_init_pncranges_set(tpm_reset_level_enum_t reset_type)
{
	tpm_db_pnc_range_conf_t pnc_range_conf;
	uint32_t cur_pnc_entry = 0;
	uint32_t luid = 0, last_lu_range, val;
	uint32_t i;
	tpm_pnc_ranges_t cur_range;
	int32_t ret_code;

	/* Init basics which are same for all Ranges */
	pnc_range_conf.num_luids = 1;

	/* Insert valid PnC Ranges by enum order */
	for (cur_range = 0; cur_range <= TPM_MAX_RANGE; cur_range++) {
		/* Search Input Table */
		for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
			if ((tpm_init.pnc_range[i].range_num == cur_range) && (tpm_init.pnc_range[i].valid == TPM_TRUE)
			    && (tpm_init.pnc_range[i].range_size > 0)) {
				pnc_range_conf.range_id = tpm_init.pnc_range[i].range_num;
				pnc_range_conf.min_reset_level = tpm_init.pnc_range[i].min_reset_level;
				pnc_range_conf.range_type = tpm_init.pnc_range[i].range_type;
				pnc_range_conf.range_size = tpm_init.pnc_range[i].range_size;
				pnc_range_conf.range_start = cur_pnc_entry;
				pnc_range_conf.range_end = cur_pnc_entry + tpm_init.pnc_range[i].range_size - 1;
				pnc_range_conf.cntr_grp = tpm_init.pnc_range[i].cntr_grp;
				pnc_range_conf.lu_mask = tpm_init.pnc_range[i].lu_mask;
				pnc_range_conf.api_start = 0;
				/* For the dscp_ds or dscp_us range, reserve hardcoded  entry at the beginning
				 * of the pnc_range (see tpm_proc_ipv4_dscp_init) */
				if (tpm_init.pnc_range[i].range_size < (pnc_range_conf.api_start + 1)) {
					TPM_OS_ERROR(TPM_INIT_MOD,
						"pnc_range(%d) cannot set first api_entry to (%d), pnc_range  "
						"is too small\n",
						cur_range, pnc_range_conf.api_start);
					return (TPM_FAIL);
				}
				tpm_init_lookup_get(tpm_init.pnc_range[i].range_num, &luid, &last_lu_range, &val);
				if (val == 0) {
					TPM_OS_ERROR(TPM_INIT_MOD, "pnc_range(%d) has no defined lookup_id\n",
						     cur_range);
					return (TPM_FAIL);
				}

				pnc_range_conf.base_lu_id = luid;

				/* Set PortId for last entry drop/trap usage, currently last entry will be valid for all active gmacs */
				pnc_range_conf.last_ent_portid = tpm_proc_all_gmac_bm();

				/* Depending if this is last Range of a LookupId, reserve last entry for debug */
				if (0 && last_lu_range == 1) {	/* YUVAL_NOW, add xml_param here */
					if (tpm_init.pnc_range[i].range_size < ((pnc_range_conf.api_start + 1) + 1)) {
						TPM_OS_ERROR(TPM_INIT_MOD,
							"pnc_range(%d) cannot set last entry to trap, pnc_range "
							" is is too small\n",
							cur_range);
						return (TPM_FAIL);
					}
					/* TODO - this is harcdoded to trap, should be configurable trap/drop/default */
					pnc_range_conf.init_last_entry = TPM_PNC_RNG_LAST_INIT_TRAP;
					pnc_range_conf.api_end = pnc_range_conf.range_size - 2;
				} else {
					pnc_range_conf.init_last_entry = TPM_PNC_RNG_LAST_INIT_DEF;
					pnc_range_conf.api_end = pnc_range_conf.range_size - 1;
				}

				/* Create or Reset the Range (Delete+Create), or skip resetting it */
				if (reset_type < pnc_range_conf.min_reset_level) {
					TPM_OS_DEBUG(TPM_INIT_MOD, " skip resetting range (%d)\n",
						     pnc_range_conf.range_id);
				} else {
					TPM_OS_DEBUG(TPM_INIT_MOD, " set range (%d)\n", pnc_range_conf.range_id);
					/* Never mind if it did or did not exist */
					ret_code = tpm_db_pnc_rng_delete(pnc_range_conf.range_id);
					ret_code = tpm_db_pnc_rng_create(&pnc_range_conf);
					IF_ERROR(ret_code);
				}
				cur_pnc_entry += tpm_init.pnc_range[i].range_size;

				TPM_OS_DEBUG(TPM_INIT_MOD,
					     "\t rng_type(%d)  rng_id(%d)    rng_start(%d)     rng_end(%d)\n"
					     "\t rng_size(%d)  cntr_grp(%d)  lu_mask(%d)   api_start(%d) \n"
					     "\t api_end(%d)   lu_id(%d)\n"
					     "\t init_last_e(%d) last_ent_port_id(%x)\n",
					     pnc_range_conf.range_type,
					     pnc_range_conf.range_id,
					     pnc_range_conf.range_start,
					     pnc_range_conf.range_end,
					     pnc_range_conf.range_size,
					     pnc_range_conf.cntr_grp,
					     pnc_range_conf.lu_mask,
					     pnc_range_conf.api_start,
					     pnc_range_conf.api_end,
					     pnc_range_conf.base_lu_id,
					     pnc_range_conf.init_last_entry, pnc_range_conf.last_ent_portid);

				TPM_OS_DEBUG(TPM_INIT_MOD, "cur_pnc_entry(%d)\n", cur_pnc_entry);
				break;
			}
		}
	}
	return (TPM_OK);
}

int32_t tpm_init_api_rng_init(tpm_pnc_ranges_t range, tpm_api_sections_t api_sec, tpm_dir_t dir)
{
	int32_t ret_code;
	uint32_t api_size;
	tpm_db_pnc_range_conf_t range_conf;

	ret_code = tpm_db_pnc_rng_conf_get(range, &range_conf);
	if (ret_code != TPM_OK) {
		TPM_OS_WARN(TPM_INIT_MOD, " PNC Range(%d) was not created - rc(%d) \n", range, ret_code);
	} else {
		api_size = (range_conf.api_end - range_conf.api_start + 1);
		ret_code = tpm_db_api_section_init(api_sec, range, api_size);
		IF_ERROR(ret_code);
		return (ret_code);
	}
	return (TPM_OK);
}

int32_t tpm_init_api_rng_init_all(void)
{
	int32_t ret_code;
	/* Init the API Entries Memeory Area */
	tpm_db_api_entries_area_reset();

	/* Init API Ranges - MAC LEARN */
	ret_code = tpm_init_api_rng_init(TPM_PNC_MAC_LEARN, TPM_PNC_MAC_LEARN_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	/* Init API Ranges - CPU WAN LPBK */
	ret_code = tpm_init_api_rng_init(TPM_PNC_CPU_WAN_LPBK_US, TPM_CPU_LOOPBACK_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	/* Init API Ranges - L2DS, L2US */
	ret_code = tpm_init_api_rng_init(TPM_PNC_L2_MAIN, TPM_L2_PRIM_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	/* Init API Ranges - L3DS, L3US */
	ret_code = tpm_init_api_rng_init(TPM_PNC_ETH_TYPE, TPM_L3_TYPE_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	/* Init API Ranges - IPV4 US, IPV4 DS */
	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV4_MAIN, TPM_IPV4_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	/* Init API Ranges - IPV4 MC */
	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV4_MC_DS, TPM_IPV4_MC, TPM_DIR_DS);
	IF_ERROR(ret_code);

	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV6_GEN, TPM_IPV6_GEN_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);
	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV6_NH, TPM_IPV6_NH_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);
	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV6_DIP, TPM_IPV6_DIP_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);
	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV6_L4, TPM_L4_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);
	ret_code = tpm_init_api_rng_init(TPM_PNC_IPV6_MC_DS, TPM_IPV6_MC_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	ret_code = tpm_init_api_rng_init(TPM_PNC_CNM_MAIN, TPM_CNM_MAIN_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	ret_code = tpm_init_api_rng_init(TPM_PNC_DS_LOAD_BALANCE, TPM_DS_LOAD_BALANCE_ACL, TPM_DIR_DS);
	IF_ERROR(ret_code);

	return (TPM_OK);
}

uint16_t tpm_init_uni_amber_vec_map(uint16_t uni_vec)
{
	uint16_t amber_vec = 0;
	uint16_t amber_port = 0;
	uint32_t uni_i;
	tpm_src_port_type_t uni_src_port;

	for (uni_i = 0; uni_i < TPM_MAX_NUM_ETH_PORTS; uni_i++) {
		if (uni_vec & (1 << uni_i)) {
			uni_src_port = TPM_SRC_PORT_UNI_0 + uni_i;
			amber_port = (uint16_t) tpm_db_eth_port_switch_port_get(uni_src_port);
			amber_vec |= (1 << amber_port);
		}
	}
	return (amber_vec);
}

int32_t tpm_init_port_vector_tbl(void)
{
	uint32_t i, pnc_vector, tbl_entry, uni_port_nr;
	int32_t ret_code, virt_uni_amber_port_bmp = 0;
	tpm_init_virt_uni_t virt_uni_info;
	uint16_t amber_port_vector;
	uint32_t trg_port_uni_vec = 0, uni_vector, uni_vector_bmp;

	/* use all 16 registers with straight mapping uni_vector to amber_port_vector
	   only UNI_0 - UNI_3 are used */
	ret_code = tpm_db_virt_info_get(&virt_uni_info);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " tpm_db_virt_info_get ret_code(%d)\n", ret_code);
		return (ret_code);
	}

	/* virt uni not enabled */
	if (virt_uni_info.enabled == 0) {
		/* Convert UNI_Vector to Amber Vector, applies to uni_0..7, uni_vector from 0x00 to 0xFF */
		for (i = 0; i < TPM_TX_MAX_MH_REGS; i++) {
			amber_port_vector = tpm_init_uni_amber_vec_map(i + 1);
			tpm_db_port_vector_tbl_info_set(i, i + 1, amber_port_vector, 0);
		}
		return TPM_DB_OK;
       }

	/* virt uni enabled */

	/* first map all 8 UNI ports */
	amber_port_vector = 0;
	pnc_vector = 1;
	tbl_entry = 0;
	uni_vector_bmp = 0;
	uni_port_nr = TPM_SRC_PORT_UNI_0;
	for (uni_vector = TPM_TRG_UNI_0; uni_vector <= TPM_TRG_UNI_VIRT; uni_vector = uni_vector << 1) {
       	uint16_t switch_port = tpm_db_eth_port_switch_port_get(uni_port_nr);

		if (TPM_DB_ERR_PORT_NUM != switch_port){
       		tpm_db_port_vector_tbl_info_set(tbl_entry, uni_vector, (1 << switch_port),
							pnc_vector << TPM_MH_RI_OFFSET);

			/* save the internal switch vector for later use */
			amber_port_vector |= (1 << switch_port);

			/* save the virt uni and target port vec w/o virt port for later use */
			if ((1 == virt_uni_info.enabled) && (uni_port_nr == virt_uni_info.uni_port))
				virt_uni_amber_port_bmp = (1 << switch_port);
			else
				uni_vector_bmp |= uni_vector;

			trg_port_uni_vec |= (pnc_vector << TPM_MH_RI_OFFSET);
		}
		pnc_vector++;
		tbl_entry++;
		uni_port_nr++;
      	}

	/* now add UNI_ANY mapping */
	tpm_db_port_vector_tbl_info_set(tbl_entry, TPM_TRG_PORT_UNI_ANY, amber_port_vector,
					pnc_vector << TPM_MH_RI_OFFSET);
	tbl_entry++;

	/* now add UNI_ANY_SPECIAL mapping (exluding virt-uni port) */
	if (1 == virt_uni_info.enabled)
		amber_port_vector &= ~virt_uni_amber_port_bmp;

	tpm_db_port_vector_tbl_info_set(tbl_entry, uni_vector_bmp, amber_port_vector, trg_port_uni_vec);

	return TPM_DB_OK;
}

int32_t tpm_init_info_set(void)
{
	int32_t ret_code;
	uint32_t i;
	tpm_src_port_type_t src_port;

	tpm_db_eth_cmplx_profile_set(tpm_init.eth_cmplx_profile);
	ret_code = tpm_db_eth_max_uni_port_set();
	IF_ERROR(ret_code);
	ret_code = tpm_db_pon_type_set(tpm_init.pon_type);
	IF_ERROR(ret_code);
	ret_code = tpm_db_active_wan_set(tpm_init.active_wan);
	IF_ERROR(ret_code);
	ret_code = tpm_db_ds_mh_set_conf_set(tpm_init.ds_mh_set_conf);
	IF_ERROR(ret_code);
	ret_code = tpm_db_cfg_pnc_parse_set(tpm_init.cfg_pnc_parse);
	IF_ERROR(ret_code);
	ret_code = tpm_db_cpu_loopback_set(tpm_init.cpu_loopback);
	IF_ERROR(ret_code);
	ret_code = tpm_db_ipv6_5t_enable_set(tpm_init.ipv6_5t_enable);
	IF_ERROR(ret_code);
	ret_code = tpm_db_ctc_cm_enable_set(tpm_init.ctc_cm_enable);
	IF_ERROR(ret_code);
	ret_code = tpm_db_ctc_cm_ipv6_parse_win_set(tpm_init.ctc_cm_ipv6_parse_window);
	IF_ERROR(ret_code);
	ret_code = tpm_db_ctc_cm_ipv6_parse_win_orig_set(tpm_init.ctc_cm_ipv6_parse_window);
	IF_ERROR(ret_code);
	ret_code = tpm_db_virt_info_set(tpm_init.virt_uni_info);
	IF_ERROR(ret_code);
	ret_code = tpm_db_double_tag_support_set((tpm_init.num_vlan_tags > 1) ? 1 : 0);
	IF_ERROR(ret_code);
#if 0
	ret_code = tpm_db_default_tag_tpid_set(tpm_init.vlan1_tpid, tpm_init.vlan2_tpid);
	IF_ERROR(ret_code);
#endif
	ret_code = tpm_db_default_tag_tpid_set(&tpm_init.tpid_opt);
	IF_ERROR(ret_code);
	ret_code = tpm_db_switch_init_set(tpm_init.switch_init);
	IF_ERROR(ret_code);
	ret_code = tpm_db_pnc_mac_learn_enable_set(tpm_init.pnc_mac_learn_enable);
	IF_ERROR(ret_code);
	ret_code = tpm_db_fc_conf_set(&tpm_init.port_fc_conf);
	IF_ERROR(ret_code);
	ret_code = tpm_db_ds_mac_based_trunk_enable_set(tpm_init.ds_mac_based_trunk_enable);
	IF_ERROR(ret_code);

	/* Set GMAC Logical Functions */

	/* PON_MAC */
	if (tpm_db_active_wan_get() == TPM_ENUM_PMAC)
		if ((tpm_init.pon_type == TPM_GPON || tpm_init.pon_type == TPM_EPON) &&
		    (tpm_init.pon_type != TPM_P2P)) {
			ret_code = tpm_db_omci_type_set(tpm_init.omci_etype);
			IF_ERROR(ret_code);
		}

	/* set GMAC/PMAC functionality */
	tpm_db_mac_func_set();

#if 0				/*Keep to be added in future version */
	/* Set Debug Port */
	if (tpm_init.deb_port_valid == 1) {
		ret_code = tpm_db_pnc_init_deb_port_set(tpm_init.deb_port);
		IF_ERROR(ret_code);
	}
#endif

	/* Set IGMP Conf */
	for (src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_WAN; src_port++) {
		tpm_db_igmp_set_port_frwd_mode(src_port, tpm_init.igmp_pkt_frwd_mod[src_port]);
		printk(KERN_INFO "TPM_SRC_PORT: %d, value: %d\n", src_port, tpm_init.igmp_pkt_frwd_mod[src_port]);
	}
	tpm_db_igmp_set_cpu_queue(tpm_init.igmp_cpu_queue);
	tpm_db_igmp_set_snoop_enable(tpm_init.igmp_snoop);
	/*printk("TPM_SRC_PORT_WAN: %d, TPM_SRC_PORT_UNI_0: %d, TPM_SRC_PORT_UNI_1: %d, TPM_SRC_PORT_UNI_2: %d, "
	   "TPM_SRC_PORT_UNI_3: %d, igmp_cpu_queue: %d\n",
	   tpm_init.igmp_pkt_frwd_mod[4],tpm_init.igmp_pkt_frwd_mod[0],tpm_init.igmp_pkt_frwd_mod[1],
	   tpm_init.igmp_pkt_frwd_mod[2],tpm_init.igmp_pkt_frwd_mod[3],tpm_init.igmp_cpu_queue); */

	/* Set ethernet Port Config */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_TRUE == tpm_init.eth_port_conf[i].valid) {
			ret_code = tpm_db_eth_port_conf_set(&tpm_init.eth_port_conf[i]);
			IF_ERROR(ret_code);
		}
	}

	/* Set GMAC Port Config */
	ret_code = tpm_db_gmac_conn_conf_set(tpm_init.gmac_port_conf, TPM_NUM_GMACS);
	IF_ERROR(ret_code);

	/* Init MH Tx Amber Port Vectors Table */
	tpm_init_port_vector_tbl();

	/* Set tcont_llid */
	ret_code = tpm_db_gmac_tcont_llid_set(tpm_init.num_tcont_llid);
	IF_ERROR(ret_code);

	/* Set GMAC mh_enable */
	ret_code = tpm_db_gmac_mh_en_conf_set(TPM_ENUM_GMAC_0, tpm_init.gmac0_mh_en);
	IF_ERROR(ret_code);
	ret_code = tpm_db_gmac_mh_en_conf_set(TPM_ENUM_GMAC_1, tpm_init.gmac1_mh_en);
	IF_ERROR(ret_code);
	ret_code = tpm_db_gmac_mh_en_conf_set(TPM_ENUM_PMAC, TPM_TRUE);
	IF_ERROR(ret_code);

	/* Set GMAC BM pools */
	for (i = 0; i < sizeof(tpm_init.gmac_bp_bufs) / sizeof(tpm_init_gmac_bufs_t); i++) {
		if (tpm_init.gmac_bp_bufs[i].valid == TPM_TRUE) {
			tpm_db_gmac_bm_bufs_conf_set(i,
						     tpm_init.gmac_bp_bufs[i].large_pkt_buffers,
						     tpm_init.gmac_bp_bufs[i].small_pkt_buffers);
		}
	}

	/* Set GMAC/TCONT Tx Queues */

	/* GMAC0 tx */
	if (tpm_init.gmac_port_conf[0].conn != TPM_GMAC_CON_DISC) {
		tpm_db_gmac_tx_val_set(TPM_TX_MOD_GMAC0);
		tpm_init_tx_mod_set(TPM_TX_MOD_GMAC0);

	}
	/* GMAC1 tx */
	if (tpm_init.gmac_port_conf[1].conn != TPM_GMAC_CON_DISC) {
		tpm_db_gmac_tx_val_set(TPM_TX_MOD_GMAC1);
		tpm_init_tx_mod_set(TPM_TX_MOD_GMAC1);
	}
	/* TCONTs */
	for (i = TPM_TX_MOD_PMAC_0; i < (TPM_TX_MOD_PMAC_0 + tpm_init.num_tcont_llid); i++) {
		tpm_db_gmac_tx_val_set(i);
		tpm_init_tx_mod_set(i);
	}

	/* Set GMAC Rx Queues */

	/* GMAC0 rx */
	if (tpm_init.gmac_port_conf[0].conn != TPM_GMAC_CON_DISC) {
		tpm_db_gmac_rx_val_set(TPM_ENUM_GMAC_0);
		tpm_init_gmac_rxq_set(TPM_ENUM_GMAC_0);
	}

	/* GMAC1 rx */
	if (tpm_init.gmac_port_conf[1].conn != TPM_GMAC_CON_DISC) {
		tpm_db_gmac_rx_val_set(TPM_ENUM_GMAC_1);
		tpm_init_gmac_rxq_set(TPM_ENUM_GMAC_1);
	}
	/* PMAC Rx */
	if (    tpm_init.pon_type == TPM_GPON
	     || tpm_init.pon_type == TPM_EPON
	     || tpm_init.pon_type == TPM_P2P) {
		tpm_db_gmac_rx_val_set(TPM_ENUM_PMAC);
		tpm_init_gmac_rxq_set(TPM_ENUM_PMAC);
	}

	/* Init PNC Ranges */
	/* Init ALL PnC Ranges in the DB, even PnC Ranges with highest reset_level */
	ret_code = tpm_init_pncranges_set(TPM_ENUM_MAX_RESET_LEVEL + 1);
	IF_ERROR(ret_code);

	/* Init API Ranges */
	ret_code = tpm_init_api_rng_init_all();
	IF_ERROR(ret_code);

	tpm_db_mod2_set_udp_checksum_use_init_bm_state(tpm_init.mod_config.udp_checksum_use_init_bm);
	tpm_db_mod2_set_udp_checksum_state(tpm_init.mod_config.udp_checksum_update);

	for (i = 0; i < tpm_init.mod_chain.chain_num; i++)
		tpm_db_mod2_set_chain_num(tpm_init.mod_chain.chain_data[i].type, tpm_init.mod_chain.chain_data[i].num);

	/*** Set state of MH modification in PMT for Multicast ***/

	/* For ip_only_filter_mode, multicast will ALWAYS update the MH, and NEVER use the MH_tx_registers
	   This way it works the same way for both with_virtual_uni and without_virtual_uni */
	if (tpm_init.mc_setting.filter_mode == TPM_MC_IP_ONLY_FILTER)
		tpm_db_mod2_set_multicast_mh_state(1);
	else {
		/* For mac_ip_combo_filter_mode, MH is not modified.
		 * The MH_tx_register are set to :
		 *   - no_uni_translation  - The "broadcast register", to all_uni_ports.
		 *   - yes_uni_translation - to switch_port of GMAC1.
		 * For other modes (not supported yet), it will be same as mac_ip_combo_filter_mode
		 */
		if (tpm_init.mc_setting.per_uni_vlan_xlat)
			tpm_db_mod2_set_multicast_mh_state(1);
		else
			tpm_db_mod2_set_multicast_mh_state(0);
	}
	/* Set system support for adding PPPoE header as part of PMT modification */
	tpm_db_mod2_set_pppoe_add_mod_state(tpm_init.pppoe_add_enable);

	/* Set modification double_tag according to num_vlan_tags parameter */
	tpm_db_mod2_set_double_tag_state((tpm_init.num_vlan_tags >= 2));

	/* set mtu config */
	tpm_db_mtu_set_ipv4_mtu_us(tpm_init.mtu_config.ipv4_mtu_us);

	tpm_db_mtu_set_ipv4_pppoe_mtu_us(tpm_init.mtu_config.ipv4_pppoe_mtu_us);

	tpm_db_mtu_set_ipv6_mtu_us(tpm_init.mtu_config.ipv6_mtu_us);

	tpm_db_mtu_set_ipv6_pppoe_mtu_us(tpm_init.mtu_config.ipv6_pppoe_mtu_us);

	tpm_db_mtu_set_ipv4_mtu_ds(tpm_init.mtu_config.ipv4_mtu_ds);

	tpm_db_mtu_set_ipv6_mtu_ds(tpm_init.mtu_config.ipv6_mtu_ds);

	tpm_db_set_pppoe_add_enable(tpm_init.pppoe_add_enable);

	tpm_db_set_num_vlan_tags(tpm_init.num_vlan_tags);

	tpm_db_set_mtu_enable(tpm_init.mtu_config.mtu_enable);

	tpm_db_set_cpu_rx_queue(tpm_init.cpu_rx_queue);

	tpm_db_set_ttl_illegal_action(tpm_init.ttl_illegal_action);

	tpm_db_set_tcp_flag_check(tpm_init.tcp_flag_check);

	tpm_db_set_mc_filter_mode(tpm_init.mc_setting.filter_mode);
	/*tpm_db_set_mc_igmp_mode(tpm_init.mc_setting.igmp_mode); */
	tpm_db_set_mc_per_uni_vlan_xlate(tpm_init.mc_setting.per_uni_vlan_xlat);
	tpm_db_set_mc_lpbk_enable(TPM_IP_VER_4, (uint8_t)tpm_init.mc_setting.ipv4_mc_support);
	tpm_db_set_mc_lpbk_enable(TPM_IP_VER_6, (uint8_t)tpm_init.mc_setting.ipv6_mc_support);
	tpm_db_set_mc_pppoe_enable(tpm_init.mc_setting.mc_pppoe_enable);
	tpm_db_set_mc_hwf_queue(tpm_init.mc_setting.mc_hwf_queue);
	tpm_db_set_mc_cpu_queue(tpm_init.mc_setting.mc_cpu_queue);

	tpm_db_set_catch_all_pkt_action(tpm_init.catch_all_pkt_action);
	tpm_db_set_ety_dsa_enable(tpm_init.ety_dsa_enable);

	/* set split mod config */
	tpm_db_split_mod_set_enable(tpm_init.split_mod_config.split_mod_enable);
	tpm_db_split_mod_set_num_vlans(tpm_init.split_mod_config.vlan_num);
	tpm_db_split_mod_set_num_p_bits(tpm_init.split_mod_config.p_bit_num, tpm_init.split_mod_config.p_bit);
	tpm_db_split_mod_set_mode(tpm_init.split_mod_config.split_mod_mode);

	/* set function profile */
	if (tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_LPBK)
		tpm_db_gmac1_lpbk_en_set(true);
	else
		tpm_db_gmac1_lpbk_en_set(false);

	/* set CPU WAN loopback en */
	tpm_db_cpu_wan_lpbk_en_set(tpm_init.cpu_loopback);

	tpm_db_switch_init_set(tpm_init.switch_init);
	if (	(tpm_init.eth_cmplx_profile == TPM_PON_WAN_DUAL_MAC_INT_SWITCH)
	     || (tpm_init.eth_cmplx_profile == TPM_PON_WAN_DUAL_MAC_EXT_SWITCH))
		tpm_db_ds_load_bal_en_set(true);
	else
		tpm_db_ds_load_bal_en_set(false);


	/* set switch active WAN en */
	if (    (tpm_init.eth_cmplx_profile == TPM_PON_G1_WAN_G0_INT_SWITCH)
	     || (tpm_init.eth_cmplx_profile == TPM_PON_G0_WAN_G1_INT_SWITCH)
	     || (tpm_init.eth_cmplx_profile == TPM_PON_G1_WAN_G0_SINGLE_PORT)
	     || (tpm_init.eth_cmplx_profile == TPM_PON_G0_WAN_G1_SINGLE_PORT))
		tpm_db_switch_active_wan_en_set(true);
	else
		tpm_db_switch_active_wan_en_set(false);

	return (TPM_OK);
}

void tpm_init_locks_init(void)
{
	spin_lock_init(&tpmPncLock);
	spin_lock_init(&tpmModLock);
	spin_lock_init(&tpmTcamAgingLock);
}

/* Note : The function assumes that the port numbers in the LSP are identical
 * to the GMAC numbers (port 0 = GMAC0, etc. )*/
void tpm_init_hwf_set(void)
{
	int tx_num, q_num, dest_port, dest_port_txp, owner_rx_port;

	for (tx_num = TPM_TX_MOD_GMAC0; tx_num < TPM_MAX_NUM_TX_PORTS; tx_num++) {
		dest_port = tpm_tx_mod_hal_attrs[tx_num].hal_port;
		dest_port_txp = tpm_tx_mod_hal_attrs[tx_num].hal_txp;
		if (!tpm_init.gmac_tx[tx_num].valid)
       		continue;

		for (q_num = 0; q_num < TPM_MAX_NUM_TX_QUEUE; q_num++) {
			if (tpm_init.gmac_tx[tx_num].tx_queue[q_num].valid) {
				/* No HW forwarding for CPU */
				if (tpm_init.gmac_tx[tx_num].tx_queue[q_num].queue_owner != TPM_Q_OWNER_CPU) {
					owner_rx_port =
					    tpm_init.gmac_tx[tx_num].tx_queue[q_num].queue_owner - 1;
					if (mv_eth_ctrl_txq_hwf_own(dest_port, dest_port_txp, q_num, owner_rx_port)){
						TPM_OS_ERROR(TPM_INIT_MOD,
				    		"mv_eth_ctrl_txq_hwf_own err GMAC = %d TCONT = %d Q = %d HWF: owner_rx %d\n",
						     dest_port, dest_port_txp, q_num, owner_rx_port);
					}

					mvNetaHwfTxqEnable(owner_rx_port, dest_port, dest_port_txp, q_num, 1);

					TPM_OS_DEBUG(TPM_INIT_MOD,
						     "GMAC = %d TCONT = %d Q = %d HWF: owner_rx %d\n",
						     dest_port, dest_port_txp, q_num, owner_rx_port);
				}
				else
				{
					if (mv_eth_ctrl_txq_cpu_own(dest_port, dest_port_txp, q_num, 1)){
						TPM_OS_ERROR(TPM_INIT_MOD,
						"mv_eth_ctrl_txq_cpu_own err GMAC = %d TCONT = %d Q = %d SWF: owner_rx %d\n",
						     dest_port, dest_port_txp, q_num, owner_rx_port);
					}
				}
			} else {
				if (mv_eth_ctrl_txq_hwf_own(dest_port, dest_port_txp, q_num, -1)){
						TPM_OS_DEBUG(TPM_INIT_MOD,
				    		"mv_eth_ctrl_txq_hwf_own err GMAC = %d TCONT = %d Q = %d HWF: owner_rx -1\n",
						     dest_port, dest_port_txp, q_num);
				}
				mvNetaHwfTxqEnable(0, dest_port, dest_port_txp, q_num, 0);
				TPM_OS_DEBUG(TPM_INIT_MOD, "GMAC = %d TCONT = %d Q = %d CPU owner\n",
					     dest_port, dest_port_txp, q_num);
			}
		}	/* All queues */
	}			/* All TX ports */
}

/* Note : The function assumes that the port numbers in the LSP are
 * identical to the GMAC numbers (port 0 = GMAC0, etc. )*/
void tpm_init_txq_size_set(void)
{
	int tx_num, q_num, dest_port, prev_dest_port = 0xFF, dest_port_txp, status;
	uint32_t q_valid, q_size;

	for (tx_num = TPM_TX_MOD_GMAC0; tx_num < TPM_MAX_NUM_TX_PORTS; tx_num++) {
		dest_port = tpm_tx_mod_hal_attrs[tx_num].hal_port;
		dest_port_txp = tpm_tx_mod_hal_attrs[tx_num].hal_txp;

		if ((dest_port == 0 && tpm_init.gmac_port_conf[0].conn == TPM_GMAC_CON_DISC) ||
		    (dest_port == 1 && tpm_init.gmac_port_conf[1].conn == TPM_GMAC_CON_DISC)) {
			continue;
		}

		if (mv_eth_ctrl_port_started_get(dest_port)) {
			if (prev_dest_port != dest_port)
				TPM_OS_WARN(TPM_INIT_MOD,
					    "port%d already started, tx_queue sizes will not be updated\n", dest_port);
			prev_dest_port = dest_port;
			continue;
		}

		for (q_num = 0; q_num < TPM_MAX_NUM_TX_QUEUE; q_num++) {

			if (TPM_DB_OK != tpm_db_gmac_tx_q_conf_get(tx_num, q_num, &q_valid,
								   NULL, NULL, NULL, &q_size, NULL)){
				TPM_OS_WARN(TPM_INIT_MOD,
					    "Could not get port(%d) tx_queue sizes\n", dest_port);
				continue;
			}

			if (!q_valid)
				q_size = 0;

			status = mv_eth_ctrl_txq_size_set(dest_port, dest_port_txp, q_num, q_size);
			if (status) {
				TPM_OS_DEBUG(TPM_INIT_MOD, "mv_eth_ctrl_txq_size_set err, port = %d TCONT = %d Q = %d size = %d\n",
					     dest_port, dest_port_txp, q_num, q_size);
			}
		}		/* All TX queues */
	}			/* All TX ports */
}

/* Note : The function assumes that the port numbers in the LSP are
 * identical to the GMAC numbers (port 0 = GMAC0, etc. )*/
void tpm_init_rxq_size_set(void)
{
	tpm_gmacs_enum_t gmac_num;
	int q_num, status;
	uint32_t q_valid, q_size, rc;

	for (gmac_num = TPM_ENUM_GMAC_0; gmac_num < TPM_MAX_NUM_GMACS; gmac_num++) {
		if (!tpm_db_gmac_rx_val_get(gmac_num))
			continue;
		/* treat only valid ports */
		if (!INIT_GMAC_VALID(gmac_num))
			continue;

		if (mv_eth_ctrl_port_started_get(gmac_num)) {
			TPM_OS_WARN(TPM_INIT_MOD, "port%d already started, rx_queue sizes will not be updated\n", gmac_num);
			continue;
		}
		for (q_num = 0; q_num < TPM_MAX_NUM_RX_QUEUE; q_num++) {
			rc = tpm_db_gmac_rx_q_conf_get(gmac_num, q_num, &q_valid, &q_size);

			if (TPM_DB_OK != rc) {
				TPM_OS_WARN(TPM_INIT_MOD, "tpm_db_gmac_rx_q_conf_get port%d error\n", gmac_num);
				continue;
			}
			if (q_valid)
				status = mv_eth_ctrl_rxq_size_set(gmac_num, q_num, q_size);
			else
				status = mv_eth_ctrl_rxq_size_set(gmac_num, q_num, 0);

			if (status)
				TPM_OS_DEBUG(TPM_INIT_MOD, "gmac_num = %d  Q = %d size = %d\n", gmac_num, q_num, q_size);
		}		/* All RX queues */
	}			/* All GMACs */
}

int32_t tpm_init_mh_select(tpm_init_mh_src_t ds_mh_set_conf)
{
	tpm_gmacs_enum_t gmac_i;
	tpm_db_gmac_func_t gmac_func;

	/* MH select config */
	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gmac_func);

		if ((gmac_func == TPM_GMAC_FUNC_LAN_AND_WAN) || (gmac_func == TPM_GMAC_FUNC_WAN)) {
			switch (ds_mh_set_conf) {
			case TPM_MH_SRC_RX_CTRL:
				mvNetaHwfMhSrcSet(gmac_i, MV_NETA_HWF_MH_REG);
				mvNetaHwfMhSelSet(gmac_i, (uint8_t) NETA_MH_REPLACE_MH_REG(0));
				/* Keep all TX_MH_registers(1-15) 'Zero' !! */
				break;
			case TPM_MH_SRC_PNC_RI:
				mvNetaHwfMhSrcSet(gmac_i, MV_NETA_HWF_MH_PNC);
				mvNetaHwfMhSelSet(gmac_i, (uint8_t) NETA_MH_DONT_CHANGE);
				break;
			default:
				TPM_OS_ERROR(TPM_INIT_MOD, " Unknown d/s MH source (%d)\n", ds_mh_set_conf);
				return (TPM_FAIL);
			}
		} else if ((gmac_func == TPM_GMAC_FUNC_LAN_AND_WAN) || (gmac_func == TPM_GMAC_FUNC_LAN) || (gmac_func == TPM_GMAC_FUNC_LAN_UNI)) {
			mvNetaHwfMhSrcSet(gmac_i, MV_NETA_HWF_MH_REG);
			mvNetaHwfMhSelSet(gmac_i, (uint8_t) NETA_MH_REPLACE_GPON_HDR);
		}
	}

	return (TPM_OK);
}

int32_t tpm_init_tx_queue_sched(void)
{

	tpm_init_tx_mod_t tx_mod;
	tpm_db_sched_t sched_method;
	uint32_t queue_weight = 0;
	int32_t ret_code = TPM_OK;
	int i = 0, port = 0, sched_ent = 0;
	uint32_t q_valid;

	for (tx_mod = TPM_TX_MOD_GMAC0; tx_mod < TPM_MAX_NUM_TX_PORTS; (tx_mod)++) {

		for (i = 0; i < TPM_MAX_NUM_TX_QUEUE; i++) {

			ret_code = tpm_db_gmac_tx_q_conf_get(tx_mod, i, &q_valid, &sched_method,
							     NULL, NULL, NULL, &queue_weight);
			if (q_valid == TPM_FALSE)
				continue;

			if (tx_mod == TPM_TX_MOD_GMAC0) {
				port = SW_GMAC_0;
				sched_ent = 0;
			} else if (tx_mod >= TPM_TX_MOD_PMAC_0) {
				port = PON_PORT;
				sched_ent = tx_mod - TPM_TX_MOD_PMAC_0;
			} else {
				ret_code = TPM_FAIL;
				break;
			}

			switch (sched_method) {
			case TPM_SCHED_SP:
				mvNetaTxqFixPrioSet(port, sched_ent, i);
				break;

			case TPM_SCHED_WRR:
				mvNetaTxqWrrPrioSet(port, sched_ent, i, queue_weight);
				break;

			default:
				printk(KERN_ERR " Unknown scheduling method command \n");
				return (TPM_NOT_FOUND);

			}
		}
	}

	return ret_code;
}

/*******************************************************************************
* tpm_init_gmac_PHY_poll()
*
* DESCRIPTION:      Initialize the GMAC PHY polling state accordint to it connect to PHY or not
*
* INPUTS:
*	port -- GMAC port
*	state -- true: enable PHY polling; false: disable PHY polling
* OUTPUTS:None
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_init_gmac_PHY_poll(int port, bool state)
{
	unsigned int regData;

	regData = MV_REG_READ(ETH_UNIT_CONTROL_REG(port));
	if (state)
		regData |= ETH_PHY_POLLING_ENABLE_MASK;
	else
		regData &= (~ETH_PHY_POLLING_ENABLE_MASK);

	MV_REG_WRITE(ETH_UNIT_CONTROL_REG(port), regData);

	return TPM_OK;
}

int32_t tpm_init_switch(void)
{
	uint32_t i;
	uint32_t sw_port_bmp = 0;
	unsigned char gq_da[6] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x01 };
	unsigned char gq_mld_da[6] = { 0x33, 0x33, 0x00, 0x00, 0x00, 0x01 };
	tpm_gmacs_enum_t gmac_i;
	int32_t switch_port;

	/* no need to initialize switch for these profiles */
	switch (tpm_init.eth_cmplx_profile) {
	case TPM_PON_WAN_DUAL_MAC_EXT_SWITCH:
	case TPM_PON_WAN_G1_MNG_EXT_SWITCH:
	case TPM_PON_WAN_G0_SINGLE_PORT:
	case TPM_PON_WAN_G1_SINGLE_PORT:
	case TPM_PON_G1_WAN_G0_SINGLE_PORT:
	case TPM_PON_G0_WAN_G1_SINGLE_PORT:
	case TPM_PON_WAN_G0_G1_LPBK:
	case TPM_PON_WAN_G0_G1_DUAL_LAN:
		return (TPM_OK);
       default:
       	break;
	}

	mv_switch_drv_init();

	/* Set 802.1q mode => fallback for all ports */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_FALSE == tpm_init.eth_port_conf[i].valid)
           		continue;

		/* need only switch ports */
		if (TPM_INTCON_SWITCH != tpm_init.eth_port_conf[i].int_connect)
			continue;

	    	if (mv_switch_set_vid_filter_per_port(tpm_init.eth_port_conf[i].switch_port, GT_FALSE))
			TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_set_vid_filter_per_port err. sw port %d\n",
					    tpm_init.eth_port_conf[i].switch_port);
	}

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		switch_port = -1;

		if (TPM_GMAC_CON_SWITCH_4 == tpm_init.gmac_port_conf[gmac_i].conn)
			switch_port = TPM_GMAC0_AMBER_PORT_NUM;
		else if (TPM_GMAC_CON_SWITCH_5 == tpm_init.gmac_port_conf[gmac_i].conn){
			if (1 == tpm_init.virt_uni_info.enabled || tpm_init.mc_setting.per_uni_vlan_xlat == 1)
				switch_port = tpm_init.virt_uni_info.switch_port;
			else
				switch_port = TPM_GMAC1_AMBER_PORT_NUM;
		 }
		 if (switch_port != -1) {
       		 if (mv_switch_set_vid_filter_per_port(switch_port, GT_FALSE))
				 TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_set_vid_filter_per_port err. sw port %d\n",
					    switch_port);
		}
	}

	/* Enable BC floods */
	mv_switch_set_broadcast_flood(GT_TRUE);

	/* Disable unknown MC floods for all UNI ports */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_FALSE == tpm_init.eth_port_conf[i].valid)
           		continue;

		/* need only switch ports */
		if (TPM_INTCON_SWITCH != tpm_init.eth_port_conf[i].int_connect)
			continue;

		if (mv_switch_set_unknown_multicast_flood(tpm_init.eth_port_conf[i].switch_port, GT_FALSE))
			TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_set_unknown_multicast_flood err. sw port %d\n",
					    tpm_init.eth_port_conf[i].switch_port);
	}

	/*set default VLAN ID to 0 */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_FALSE == tpm_init.eth_port_conf[i].valid)
           		continue;

		/* need only switch ports */
		if (TPM_INTCON_SWITCH != tpm_init.eth_port_conf[i].int_connect)
			continue;

		if (mv_switch_set_port_def_vlan(tpm_init.eth_port_conf[i].switch_port, 0))
			TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_set_port_def_vlan err. sw port %d\n",
					    tpm_init.eth_port_conf[i].switch_port);
	}

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (TPM_FALSE == tpm_init.gmac_port_conf[gmac_i].valid)
			continue;

		switch_port = -1;

		if (TPM_GMAC_CON_SWITCH_4 == tpm_init.gmac_port_conf[gmac_i].conn)
			switch_port = TPM_GMAC0_AMBER_PORT_NUM;
		else if (TPM_GMAC_CON_SWITCH_5 == tpm_init.gmac_port_conf[gmac_i].conn){
			if (1 == tpm_init.virt_uni_info.enabled)
				switch_port = tpm_init.virt_uni_info.switch_port;
			else
				switch_port = TPM_GMAC1_AMBER_PORT_NUM;
		}

		if (switch_port != -1) {
			if (mv_switch_set_port_def_vlan(switch_port, 0))
				TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_set_vid_filter_per_port err. sw port %d\n",
					    switch_port);
		}
	}

	switch_port = -1;

	/* Add all ports to the member list of default VID(0) */
	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (TPM_FALSE == tpm_init.gmac_port_conf[gmac_i].valid)
			continue;

		if (TPM_GMAC_CON_SWITCH_5 == tpm_init.gmac_port_conf[gmac_i].conn){
			if (1 == tpm_init.virt_uni_info.enabled)
				switch_port = tpm_init.virt_uni_info.switch_port;
			else
				switch_port = TPM_GMAC1_AMBER_PORT_NUM;

       		 if (mv_switch_port_add_vid(switch_port, 0, TPM_GMAC0_AMBER_PORT_NUM))
				 TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_set_port_def_vlan err. sw port %d\n",
					    switch_port);
		}
	}

	if (switch_port != -1) {
		for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
			if (TPM_FALSE == tpm_init.eth_port_conf[i].valid)
				continue;

			/* need only switch ports */
			if (TPM_INTCON_SWITCH != tpm_init.eth_port_conf[i].int_connect)
				continue;

			if (switch_port != -1) {
				if (mv_switch_port_add_vid(tpm_init.eth_port_conf[i].switch_port, 0, switch_port))
					TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_port_add_vid err. sw port %d\n",
						tpm_init.eth_port_conf[i].switch_port);
			}
		}
	}


	/* config amber port#5 - same as UNI ports */

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (TPM_FALSE == tpm_init.gmac_port_conf[gmac_i].valid)
			continue;

		if (TPM_GMAC_CON_SWITCH_4 == tpm_init.gmac_port_conf[gmac_i].conn)
			sw_port_bmp |= 1 << TPM_GMAC0_AMBER_PORT_NUM;
		else if (TPM_GMAC_CON_SWITCH_5 == tpm_init.gmac_port_conf[gmac_i].conn)
			sw_port_bmp |= 1 << TPM_GMAC1_AMBER_PORT_NUM;
	}


	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_FALSE == tpm_init.eth_port_conf[i].valid)
           		continue;

		/* need only switch ports */
		if (TPM_INTCON_SWITCH != tpm_init.eth_port_conf[i].int_connect)
			continue;

		sw_port_bmp |= 1 << tpm_init.eth_port_conf[i].switch_port;
	}
	if (mv_switch_mac_addr_set(gq_da, 0, sw_port_bmp, 1))
		TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_mac_addr_set err. sw_port_bmp 0X%x\n", sw_port_bmp);
	if (mv_switch_mac_addr_set(gq_mld_da, 0, sw_port_bmp, 1))
		TPM_OS_WARN(TPM_INIT_MOD, "mv_switch_mac_addr_set err.for mld general query sw_port_bmp 0X%x\n", sw_port_bmp);
    
	/* If Switch port MAC no connected to PHY, disable GMAC PHY polling */
	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (TPM_FALSE == tpm_init.gmac_port_conf[gmac_i].valid)
			continue;

		if (TPM_GMAC_CON_SWITCH_4 == tpm_init.gmac_port_conf[gmac_i].conn ||
		    TPM_GMAC_CON_SWITCH_5 == tpm_init.gmac_port_conf[gmac_i].conn)
			tpm_init_gmac_PHY_poll(gmac_i, false);
	}

	return (TPM_OK);
}

int32_t tpm_init_mh_conf_set(uint32_t gmac0_mh_en, uint32_t gmac1_mh_en, tpm_init_gmac_conn_conf_t *gmac_port_conf)
{
	tpm_gmacs_enum_t gmac_i;
	uint32_t amber_port_num, mh_en;
	/*uint16_t amber_port_cfg_reg; */

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		/* if port in GateWay mode, do not set MH */
		if (tpm_init_gmac_in_gateway_mode(gmac_i))
			continue;

		amber_port_num = -1;

		switch (gmac_i) {
		case TPM_ENUM_GMAC_0:
			mh_en = gmac0_mh_en;
			if (TPM_GMAC_CON_SWITCH_4 == gmac_port_conf[gmac_i].conn)
				amber_port_num = TPM_GMAC0_AMBER_PORT_NUM;
			break;
		case TPM_ENUM_GMAC_1:
			mh_en = gmac1_mh_en;
			if (TPM_GMAC_CON_SWITCH_5 == gmac_port_conf[gmac_i].conn)
				amber_port_num = TPM_GMAC1_AMBER_PORT_NUM;
			break;
		case TPM_ENUM_PMAC:
			mh_en = 1;
			/*no amber port */
			break;
		default:
			TPM_OS_ERROR(TPM_INIT_MOD, " Unknown GMAC (%d)\n", gmac_i);
			return (TPM_FAIL);
		}

		/* Configure PP */
		TPM_OS_DEBUG(TPM_INIT_MOD, "GMAC(%d) mh_en(%d)\n", gmac_i, mh_en);
		printk("GMAC(%d) mh_en(%d) amber_port_num(%d)\n", gmac_i, mh_en,amber_port_num);
		if (mh_en == TPM_TRUE)
		{
			if (mv_eth_ctrl_flag(gmac_i, MV_ETH_F_MH, 1)){
				TPM_OS_ERROR(TPM_INIT_MOD, " mv_eth_ctrl_flag err port %d flag %X VAL %X\n",
					gmac_i, MV_ETH_F_MH, 1);
				return (TPM_FAIL);
			}
		}
		else
			if (mv_eth_ctrl_flag(gmac_i, MV_ETH_F_MH, 0)){
				TPM_OS_ERROR(TPM_INIT_MOD, " mv_eth_ctrl_flag err port %d flag %X VAL %X\n",
					gmac_i, MV_ETH_F_MH, 0);
				return (TPM_FAIL);
       		}
		/* Configure Amber */
		if (amber_port_num != -1)
			mv_switch_set_marvell_header_mode(amber_port_num, mh_en);
	}
	return (TPM_OK);
}

int32_t tpm_init_mh_reg_set(tpm_init_mh_src_t ds_mh_set_conf)
{
	tpm_gmacs_enum_t gmac_i;
	uint32_t pp_port_num;
	uint32_t num_txp = 0, txp_i = 0, tx_reg_i = 0;
	tpm_db_gmac_func_t gmac_func;
	uint32_t gmac_mh_en;
	uint16_t regVal;
	uint16_t amber_port_vector;
	uint32_t pnc_vector, uni_vector;

	/* MH Select Tx registers config */
	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		if (tpm_db_gmac_func_get(gmac_i, &gmac_func) != TPM_OK) {
			TPM_OS_ERROR(TPM_INIT_MOD, " tpm_db_gmac_func_get err port %d\n", gmac_i);
			return (TPM_FAIL);
		}
		if (tpm_db_gmac_mh_en_conf_get(gmac_i, &gmac_mh_en) != TPM_OK) {
			TPM_OS_ERROR(TPM_INIT_MOD, " tpm_db_gmac_mh_en_conf_get err port %d\n", gmac_i);
			return (TPM_FAIL);
		}
		pp_port_num = TPM_GMAC_TO_PP_PORT(gmac_i);

		if (((gmac_func == TPM_GMAC_FUNC_LAN_AND_WAN) || (gmac_func == TPM_GMAC_FUNC_LAN) || (gmac_func == TPM_GMAC_FUNC_LAN_UNI)) &&
		    gmac_mh_en && (tpm_init.ds_mh_set_conf == TPM_MH_SRC_PNC_RI)) {
			/* Set MH Tx registers to full table */
			for (tx_reg_i = 0; tx_reg_i < TPM_TX_MAX_MH_REGS; tx_reg_i++) {
				regVal = 0;	/* Marvell Header (FID, LearnDisable) fields are both zero */
				if (tpm_db_port_vector_tbl_info_get(tx_reg_i, &uni_vector,
								     &amber_port_vector, &pnc_vector) != TPM_OK) {
						TPM_OS_ERROR(TPM_INIT_MOD, "tpm_db_port_vector_tbl_info_get err tx_reg %d\n",
								tx_reg_i);
						return (TPM_FAIL);
				}
				regVal = amber_port_vector;
				if (mvNetaTxMhRegSet(pp_port_num, 0 /*txp */ , tx_reg_i, regVal)) {
					TPM_OS_ERROR(TPM_INIT_MOD,
						     "mvNetaTxMhRegSet err pp_port_num %d tx_reg %d regVal 0x%x\n",
						     pp_port_num, tx_reg_i, regVal);
					return (TPM_FAIL);
				}
			}
		} else {
			/* Get GMAC number of tx_modules & default register value */
			if (gmac_i != TPM_ENUM_PMAC) {
				num_txp = 1;
				/* Marvell Header -  FID/LearnDisable fields are zero */
				/*                   VLANTable field is  (re)set to 'all_other_switch_ports' */

				if (gmac_i == TPM_ENUM_GMAC_0)
					regVal = 0x7f & (~(1 << TPM_GMAC0_AMBER_PORT_NUM));
				else
					regVal = 0x7f & (~(1 << TPM_GMAC1_AMBER_PORT_NUM));
			} else {
				if (tpm_db_num_tcont_llid_get(&num_txp) != TPM_OK) {
					TPM_OS_ERROR(TPM_INIT_MOD,
						     "tpm_db_num_tcont_llid_get err\n");
					return (TPM_FAIL);
				}
				regVal = 0;
			}

			/* Set MH_tx registers for all tx_modules */
			for (txp_i = 0; txp_i < num_txp; txp_i++) {
				for (tx_reg_i = 0; tx_reg_i < TPM_TX_MAX_MH_REGS; tx_reg_i++)
					if (mvNetaTxMhRegSet(pp_port_num, txp_i, tx_reg_i, regVal)) {
						TPM_OS_ERROR(TPM_INIT_MOD,
						     "mvNetaTxMhRegSet err pp_port_num %d txp %d tx_reg 0x%x, regVal 0x%X\n",
						     pp_port_num, txp_i, tx_reg_i, regVal);
						return (TPM_FAIL);
					 }
			}

		}
	}

	return (TPM_OK);
}
int32_t tpm_init_get_gmac_queue_rate_limit(void)
{
	tpm_db_tx_mod_t tx_mod_i;
	tpm_gmacs_enum_t gmac_i;
	uint32_t txp_i = 0, prio = 0;
	uint32_t size = 0, queue = 0, rate = 0, wrr = 0;

	for (tx_mod_i = 0; tx_mod_i < TPM_MAX_NUM_TX_PORTS; tx_mod_i++) {
		if (TPM_FALSE == tpm_db_gmac_tx_val_get(tx_mod_i)) {
			TPM_OS_INFO(TPM_INIT_MOD, "tx_mod_i (%d) is invalid\n", tx_mod_i);
			continue;
		}

		if (tx_mod_i < TPM_TX_MOD_PMAC_0) {
			gmac_i = tx_mod_i;
		} else {
			gmac_i = TPM_ENUM_PMAC;
			txp_i = tx_mod_i - TPM_TX_MOD_PMAC_0;
		}

		/* get Tx queue bucket size */
		for (queue = 0; queue < TPM_MAX_NUM_TX_QUEUE; queue++) {
#ifdef MV_ETH_WRR_NEW
			size = MV_REG_READ(NETA_TXQ_TOKEN_SIZE_REG(gmac_i, txp_i, queue));
			rate = MV_REG_READ(NETA_TXQ_REFILL_REG(gmac_i, txp_i, queue));
			wrr = MV_REG_READ(NETA_TXQ_WRR_ARBITER_REG(gmac_i, txp_i, queue));
#else /* Old WRR/EJP module */
			/* token size and rate are in the same reg */
			rate = MV_REG_READ(ETH_TXQ_TOKEN_CFG_REG(gmac_i, txp_i, queue));
#endif /* MV_ETH_WRR_NEW */
			TPM_OS_INFO(TPM_INIT_MOD, "gmac_i (%d), txp_i (%d), queue (%d), size (%d), rate (%d)\n",
				gmac_i, txp_i, queue, size, rate);
			tpm_db_set_gmac_q_rate_limit(tx_mod_i, queue, size, rate, wrr);
		}

		/* get TX bucket and rate */
#ifdef MV_ETH_WRR_NEW
		size = MV_REG_READ(NETA_TXP_TOKEN_SIZE_REG(gmac_i, txp_i));
		rate = MV_REG_READ(NETA_TXP_REFILL_REG(gmac_i, txp_i));
		prio = MV_REG_READ(NETA_TX_FIXED_PRIO_CFG_REG(gmac_i, txp_i));
#else /* Old WRR/EJP module */
		rate = MV_REG_READ(ETH_TXP_TOKEN_RATE_CFG_REG(gmac_i, txp_i));
		size = MV_REG_READ(ETH_TXP_TOKEN_SIZE_REG(gmac_i, txp_i));
#endif /* MV_ETH_WRR_NEW */
		TPM_OS_INFO(TPM_INIT_MOD, "gmac_i (%d), txp_i (%d), queue (%d), size (%d), rate (%d)\n",
			gmac_i, txp_i, queue, size, rate);
		tpm_db_set_gmac_rate_limit(tx_mod_i, size, rate, prio);
	}
	return (TPM_OK);
}
int32_t tpm_init_set_gmac_queue_rate_limit(void)
{
	tpm_db_tx_mod_t tx_mod_i;
	tpm_gmacs_enum_t gmac_i;
	uint32_t txp_i = 0;
	uint32_t size = 0, queue = 0, rate = 0;
	uint32_t wrr = 0, prio = 0, regVal = 0;

	/* keep the compiler quiet */
	regVal = 0;

	for (tx_mod_i = 0; tx_mod_i < TPM_MAX_NUM_TX_PORTS; tx_mod_i++) {
		if (TPM_FALSE == tpm_db_gmac_tx_val_get(tx_mod_i))
			continue;

		if (tx_mod_i < TPM_TX_MOD_PMAC_0) {
			gmac_i = tx_mod_i;
		} else {
			gmac_i = TPM_ENUM_PMAC;
			txp_i = tx_mod_i - TPM_TX_MOD_PMAC_0;
		}

		/* set Tx queue bucket size */
		for (queue = 0; queue < TPM_MAX_NUM_TX_QUEUE; queue++) {
			tpm_db_get_gmac_q_rate_limit(tx_mod_i, queue, &size, &rate, &wrr);
			TPM_OS_INFO(TPM_INIT_MOD, "gmac_i (%d), txp_i (%d), queue (%d), size (%d), rate (%d)\n",
				gmac_i, txp_i, queue, size, rate);
#ifdef MV_ETH_WRR_NEW
			MV_REG_WRITE(NETA_TXQ_REFILL_REG(gmac_i, txp_i, queue), rate);
			MV_REG_WRITE(NETA_TXQ_TOKEN_SIZE_REG(gmac_i, txp_i, queue), size);
			MV_REG_WRITE(NETA_TXQ_WRR_ARBITER_REG(gmac_i, txp_i, queue), wrr);
#else /* Old WRR/EJP module */
			MV_REG_WRITE(ETH_TXQ_TOKEN_CFG_REG(gmac_i, txp_i, queue), rate);
#endif /* MV_ETH_WRR_NEW */
		}

		tpm_db_get_gmac_rate_limit(tx_mod_i, &size, &rate, &prio);
		TPM_OS_INFO(TPM_INIT_MOD, "gmac_i (%d), txp_i (%d), size (%d), rate (%d)\n",
			gmac_i, txp_i, size, rate);
		/* set TX bucket and rate */
#ifdef MV_ETH_WRR_NEW
		MV_REG_WRITE(NETA_TXP_TOKEN_SIZE_REG(gmac_i, txp_i), size);
		MV_REG_WRITE(NETA_TXP_REFILL_REG(gmac_i, txp_i), rate);
		MV_REG_WRITE(NETA_TX_FIXED_PRIO_CFG_REG(gmac_i, txp_i), prio);
#else /* Old WRR/EJP module */
		regVal = MV_REG_READ(ETH_TXP_TOKEN_RATE_CFG_REG(gmac_i, txp_i));
		regVal &= ~ETH_TXP_TOKEN_RATE_ALL_MASK;
		regVal |= ETH_TXP_TOKEN_RATE_MASK(rate);
		MV_REG_WRITE(ETH_TXP_TOKEN_RATE_CFG_REG(gmac_i, txp_i), regVal);
		regVal = MV_REG_READ(ETH_TXP_TOKEN_SIZE_REG(gmac_i, txp_i));
		regVal &= ~ETH_TXP_TOKEN_SIZE_ALL_MASK;
		regVal |= ETH_TXP_TOKEN_SIZE_MASK(size);
		MV_REG_WRITE(ETH_TXP_TOKEN_SIZE_REG(gmac_i, txp_i), regVal);
#endif /* MV_ETH_WRR_NEW */
	}
	return (TPM_OK);
}
int32_t tpm_init_reset_gmac_queue_rate_limit(void)
{
	static uint32_t first_time = 0;
	int32_t tpm_ret = 0;

	if (first_time == 0) {
		/* tpm module start up, do not reset gmac q */
		TPM_OS_INFO(TPM_INIT_MOD, "tpm module start up, get gmac queue rate limit\n");
		first_time = 1;

		/* get queue rate limit */
		tpm_ret = tpm_init_get_gmac_queue_rate_limit();
		IF_ERROR(tpm_ret);

		return (TPM_OK);
	}

	TPM_OS_INFO(TPM_INIT_MOD, "tpm mib-reset, reset gmac queue rage limit\n");

	/* set queue rate limit */
	tpm_ret = tpm_init_set_gmac_queue_rate_limit();
	IF_ERROR(tpm_ret);

	return (TPM_OK);
}

int32_t tpm_init_mh(tpm_init_mh_src_t ds_mh_set_conf,
			uint32_t gmac0_mh_en,
			uint32_t gmac1_mh_en,
			tpm_init_gmac_conn_conf_t *gmac_port_conf)
{
	if (tpm_init_mh_select(ds_mh_set_conf) != TPM_OK) {
		TPM_OS_ERROR(TPM_INIT_MOD, "tpm_init_mh_select err\n");
		return (TPM_FAIL);
	}
	if (tpm_init_mh_conf_set(gmac0_mh_en, gmac1_mh_en, gmac_port_conf) != TPM_OK) {
		TPM_OS_ERROR(TPM_INIT_MOD, "tpm_init_mh_conf_set err\n");
		return (TPM_FAIL);
	}
	if (tpm_init_mh_reg_set(ds_mh_set_conf) != TPM_OK) {
		TPM_OS_ERROR(TPM_INIT_MOD, "tpm_init_mh_reg_set err\n");
		return (TPM_FAIL);
	}

	return (TPM_OK);
}

int32_t tpm_init_bm_pool(void)
{
	int status;
	tpm_gmacs_enum_t gmac_i;
	uint32_t val, large_buf, small_buf;

	for (gmac_i = 0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		tpm_db_gmac_bm_bufs_conf_get(gmac_i, &val, &large_buf, &small_buf);
		if (val) {
			if ((gmac_i == 0 && tpm_init.gmac_port_conf[0].conn == TPM_GMAC_CON_DISC) ||
			    (gmac_i == 1 && tpm_init.gmac_port_conf[1].conn == TPM_GMAC_CON_DISC))
				continue;

			if (mv_eth_ctrl_port_started_get(gmac_i)) {
				TPM_OS_WARN(TPM_INIT_MOD, "port%d already started, port bm_pools will not be updated\n",
					    gmac_i);
				continue;
			}

#ifdef CONFIG_MV_ETH_BM_CPU
			status = mv_eth_ctrl_port_buf_num_set(TPM_GMAC_TO_PP_PORT(gmac_i), large_buf, small_buf);
#else
			status = mv_eth_bm_config_buf_num_set(TPM_GMAC_TO_PP_PORT(gmac_i), large_buf, small_buf);
#endif
			if (status) {
				TPM_OS_ERROR(TPM_INIT_MOD, "BM Pool error gmac(%d) large_buf(%d), small_buf(%d)\n",
					     gmac_i, large_buf, small_buf);
				return (TPM_FAIL);
			}
		}
	}
	return (TPM_OK);
}

int32_t tpm_init_ipg(int32_t mod_value)
{
	int status;
	tpm_gmacs_enum_t gmac_i;
	uint16_t ipg_val, preamble;
	uint8_t port;
	tpm_db_gmac_func_t gmac_func;
	tpm_db_gmac_conn_t gmac_con;
	MV_BOOL link_is_up;
	tpm_db_error_t db_ret_code;
	uint32_t gmac0_mh_en = 0;
	uint32_t gmac1_mh_en = 0;

	db_ret_code = tpm_db_gmac_mh_en_conf_get(TPM_ENUM_GMAC_0, &gmac0_mh_en);
	IF_ERROR(db_ret_code);
	db_ret_code = tpm_db_gmac_mh_en_conf_get(TPM_ENUM_GMAC_1, &gmac1_mh_en);
	IF_ERROR(db_ret_code);

	for (gmac_i = 0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		tpm_db_gmac_conn_get(gmac_i, &gmac_con);
		tpm_db_gmac_func_get(gmac_i, &gmac_func);

		if (gmac_func != TPM_GMAC_FUNC_NONE) {

			if ((gmac_i == 0 && gmac0_mh_en != TPM_FALSE && gmac_con == TPM_GMAC_CON_SWITCH_4) ||
			    (gmac_i == 1 && gmac1_mh_en != TPM_FALSE && (gmac_con == TPM_GMAC_CON_SWITCH_5 ||
			     tpm_init.eth_cmplx_profile == TPM_PON_WAN_G0_G1_LPBK))) {
				mvNetaPortDisable(gmac_i);
				mvNetaHwfEnable(gmac_i, 0);
				ipg_val = mvNetaPortIpgGet(gmac_i);
				ipg_val += mod_value;
				mvNetaPortIpgSet(gmac_i, ipg_val);
				mvNetaHwfEnable(gmac_i, 1);
				status = mvNetaPortEnable(gmac_i);
				if (status != MV_OK) {
					link_is_up = mvNetaLinkIsUp(gmac_i);

					if (link_is_up)
						mvNetaPortUp(gmac_i);
				}

				if ((gmac_con == TPM_GMAC_CON_SWITCH_4) || (gmac_con == TPM_GMAC_CON_SWITCH_5)) {
					if (gmac_i == 0)
						port = TPM_GMAC0_AMBER_PORT_NUM;
					else
						port = TPM_GMAC1_AMBER_PORT_NUM;

					/* TODO: Need to check if switch is attached to GMAC */
					status = mv_switch_get_port_preamble(port, &preamble);
					if (status) {
						TPM_OS_ERROR(TPM_INIT_MOD, "Fail to get port(%d) preamble_len\n", port);
						return (TPM_FAIL);
					}
					preamble += mod_value;
					status = mv_switch_set_port_preamble(port, preamble);
					if (status) {
						TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set port(%d) preamble_len(%d)\n"
							     , port, preamble);
						return (TPM_FAIL);
					}
				}
			}
		}
	}

	return (TPM_OK);
}

int32_t tpm_init_ethertype_dsa_tag(void)
{
	uint16_t  reg_tmp = 0;
	uint8_t   ret;
	int       cpu_port = 4;
	int       frame_mode_reg = 4;
	int       portetype_reg = 0xf;
	int       switch_dev_num_reg = 0x1c;
	int       rsvd2cpu2x_reg = 2;
	int       rsvd2cpu0x_reg = 3;
	tpm_db_ety_dsa_enable_t ety_dsa_enable;

	tpm_db_get_ety_dsa_enable(&ety_dsa_enable);

	if (TPM_ETY_DSA_DISABLE == ety_dsa_enable)
		return TPM_OK;

	/*
	For switch port 4: change the Frame Mode (Port reg offset 0x04) to Ethertype_DSA.
	For switch port 4: set Port EType register (Port reg offset 0x0F) to 0xDABC.
	Set zero to MGMT Enables 0x register (Global 2 offset 0x03) and MGMT Enables 2x register (Global 2 offset 0x02).
	*/

	/* get Frame Mode reg from HW */
	ret = mv_switch_reg_read(cpu_port, frame_mode_reg, MV_SWITCH_PORT_ACCESS, &reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to get cpu port frame mode reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}

	/* set bits 8, 9 to 11, as Ethertype_DSA */
	reg_tmp |= (3 << 8);
	ret = mv_switch_reg_write(cpu_port, frame_mode_reg, MV_SWITCH_PORT_ACCESS, reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set cpu port frame mode reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}

	/* set PortEType reg to HW */
	reg_tmp = TPM_ETHERTYPE_DSA_TAG;
	ret = mv_switch_reg_write(cpu_port, portetype_reg, MV_SWITCH_PORT_ACCESS, reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set cpu port PortEType reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}

	/* get switch_dev_num value reg from HW */
	ret = mv_switch_reg_read(0, switch_dev_num_reg, MV_SWITCH_GLOBAL_ACCESS, &reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to get switch_dev_num reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}
	tpm_db_set_switch_dev_num((reg_tmp & 0x1f));

	/* set PortEType reg to HW */
	reg_tmp = 0;
	ret = mv_switch_reg_write(0, rsvd2cpu2x_reg, MV_SWITCH_GLOBAL2_ACCESS, reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set zero to rsvd2cpu2x reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}
	ret = mv_switch_reg_write(0, rsvd2cpu0x_reg, MV_SWITCH_GLOBAL2_ACCESS, reg_tmp);
	if (0 != ret) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set zero to rsvd2cpu0x reg, ret(%d)\n", ret);
		return (TPM_FAIL);
	}

	return (TPM_OK);
}

/*******************************************************************************
* tpm_init_gmac_loopback()
*
* DESCRIPTION:      Initialize the GMAC to loopback mode if necessary, and force
*		gmac linkup to 1G
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_init_gmac_loopback(tpm_gmacs_enum_t port, uint8_t enable)
{
	uint32_t regVal;

	if (port > TPM_MAX_GMAC) {
		TPM_OS_ERROR(TPM_INIT_MOD, "GMAC port(%d) is invalid\n", port);
		return TPM_FAIL;
	}

	/* Check Port Available or not */
	if (!tpm_init_check_gmac_init(port)) {
		TPM_OS_ERROR(TPM_INIT_MOD, "GMAC port(%d) is not available\n", port);
		return TPM_FAIL;
	}

	if (mvNetaPortDisable(port)) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to disable GMAC port(%d)\n", port);
		return (TPM_FAIL);
	}
	if (mvNetaHwfEnable(port, 0)) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to disable GMAC port(%d) HWF\n", port);
		return (TPM_FAIL);
	}

	if (mvNetaForceLinkModeSet(port, 1, 0)) {/*for link up*/
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set GMAC port(%d) force link\n", port);
		return (TPM_FAIL);
	}
	if (mvNetaSpeedDuplexSet(port, MV_ETH_SPEED_1000, MV_ETH_DUPLEX_FULL)) {/*set 1G*/
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to set GMAC port(%d) speed and duplex\n", port);
		return (TPM_FAIL);
	}

	if (mvNetaHwfEnable(port, 1)) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to enable GMAC port(%d) HWF\n", port);
		return (TPM_FAIL);
	}
	if (mvNetaPortEnable(port)) {
		TPM_OS_ERROR(TPM_INIT_MOD, "Fail to enable GMAC port(%d)\n", port);
		return (TPM_FAIL);
	}

	/* set GMAC_CTRL_2_REG */
	regVal = MV_REG_READ(NETA_GMAC_CTRL_2_REG(port));
	regVal &= ~NETA_GMAC_PSC_ENABLE_MASK;
	MV_REG_WRITE(NETA_GMAC_CTRL_2_REG(port), regVal);

	if (enable)
		MV_REG_WRITE(NETA_GMAC_CTRL_1_REG(port), 0x31); /* set gmac to loopback mode */
	else
		MV_REG_WRITE(NETA_GMAC_CTRL_1_REG(port), 0x11); /* set gmac to no-loopback mode */

	regVal = MV_REG_READ(NETA_GMAC_AN_CTRL_REG(port));
	regVal |=  NETA_FORCE_LINK_FAIL_MASK;  			/* enable Force Link Pass 	*/
	regVal |=  NETA_SET_GMII_SPEED_1000_MASK;  		/* set GMII Speed 1000		*/
	regVal &= ~NETA_ENABLE_SPEED_AUTO_NEG_MASK;  		/* disable AutoNeg speed 	*/
	regVal |=  NETA_SET_FLOW_CONTROL_MASK;  		/* enable AutoNeg FC		*/
	regVal |=  NETA_FLOW_CONTROL_ADVERTISE_MASK;  		/* enable advertise FC		*/
	regVal &= ~NETA_ENABLE_FLOW_CONTROL_AUTO_NEG_MASK;  	/* disable AutoNeg FC		*/
	regVal |=  NETA_SET_FULL_DUPLEX_MASK;			/* enable full duplex mode	*/
	regVal &= ~NETA_ENABLE_DUPLEX_AUTO_NEG_MASK;		/* disable duplex AutoNeg 	*/
	MV_REG_WRITE(NETA_GMAC_AN_CTRL_REG(port), regVal);

	return TPM_OK;
}

/*******************************************************************************
* tpm_init_system_mib_reset()
*
* DESCRIPTION:      Initialize the dynamic parts of the system hardware
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_init_system_mib_reset(tpm_reset_level_enum_t reset_type)
{
	tpm_pnc_ranges_t cur_range = TPM_INVALID_RANGE, next_range = 0;
	tpm_db_pnc_range_t next_range_data;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_db_pnc_range_conf_t range_conf_tcp_flag;
	uint32_t switch_init;
	int32_t ret_code, ret_code_tcp_flag;
	tpm_db_ttl_illegal_action_t ttl_illegal_action;
	tpm_db_tcp_flag_check_t tcp_flag_check;
	tpm_gmacs_enum_t gmac_i;
	tpm_db_gmac_func_t gfunc;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;
	uint32_t snoop_enable;

	TPM_OS_DEBUG(TPM_INIT_MOD, "\n");

	ret_code = tpm_db_mod2_setup();
	IF_ERROR(ret_code);

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gfunc);
		if (gfunc != TPM_GMAC_FUNC_NONE) {
			ret_code = tpm_mod2_mac_inv(gmac_i);
			IF_ERROR(ret_code);
		}

		ret_code = tpm_mod2_split_mod_init(gmac_i);
		IF_ERROR(ret_code);
	}

	/* Init Amber Switch */
	tpm_db_switch_init_get(&switch_init);
	if (switch_init)
		tpm_init_switch();
	else {
		if (tpm_db_gmac1_lpbk_en_get()) {
			/* Init GMAC1 in loopback mod if GMAC loopback is enabled */
			ret_code = tpm_init_gmac_loopback(TPM_ENUM_GMAC_1, 1);
			IF_ERROR(ret_code);
		}
	}
	/*Init TX scheduling */
	ret_code = tpm_init_tx_queue_sched();
	IF_ERROR(ret_code);

	/* Init Tx queue rate limit */
	ret_code = tpm_init_reset_gmac_queue_rate_limit();
	IF_ERROR(ret_code);

	/* Initialize (wipe out) the PNC ranges in PnC HW */
	tpm_db_pnc_rng_val_get_next(cur_range, &next_range, &next_range_data);
	TPM_OS_DEBUG(TPM_INIT_MOD, "next_range(%d)\n", next_range);
	while (next_range != TPM_INVALID_RANGE) {
		if (reset_type >= next_range_data.pnc_range_conf.min_reset_level) {
			ret_code = tpm_pncl_range_reset(next_range);
			IF_ERROR(ret_code);
		}
		cur_range = next_range;
		tpm_db_pnc_rng_val_get_next(cur_range, &next_range, &next_range_data);
		TPM_OS_DEBUG(TPM_INIT_MOD, "next_range(%d)\n", next_range);
	}
	/* Initialize the PNC Hardcoded Ranges */
	/* @Wilson
	   New bi-direction range: NUM_VLAN_TAGS three hardcoded rules
	*/
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_NUM_VLAN_TAGS, &range_conf);
	if (ret_code == TPM_DB_OK) {
		ret_code = tpm_proc_l2_num_vlan_tags_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_NUM_VLAN_TAGS);

	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_DS_LOAD_BALANCE, &range_conf);
	if (TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable) {
		if (ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_INIT_MOD, "PNC range TPM_PNC_DS_LOAD_BALANCE is not created "
				"when ds_mac_based_trunk is enabled\n");
			return ERR_GENERAL;
		} else if (range_conf.range_size < 3) {
			TPM_OS_ERROR(TPM_INIT_MOD, "PNC range TPM_PNC_DS_LOAD_BALANCE size should be at least "
				"3 when ds_mac_based_trunk is enabled\n");
			return ERR_GENERAL;
		}
	} else {
		if ((ret_code == TPM_DB_OK) && (range_conf.range_size != 0)) {
			TPM_OS_WARN(TPM_INIT_MOD, "do not perform ds_mac_based_trunk, "
				"no need to create TPM_PNC_DS_LOAD_BALANCE range\n");
		}
	}

	tpm_db_get_ttl_illegal_action(&ttl_illegal_action);
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_TTL, &range_conf);
	if (TPM_TTL_ZERO_ACTION_NOTHING == ttl_illegal_action) {
		if (ret_code == TPM_DB_OK) {
			/* do not check ttl, no need to create ttl range */
			TPM_OS_WARN(TPM_INIT_MOD, "do not check ttl, no need to create ttl range\n");
		}
	} else {
		if ((ret_code != TPM_DB_OK) || (range_conf.range_size < 4)) {
			/* check ttl, need to create ttl range */
			TPM_OS_WARN(TPM_INIT_MOD, "check ttl, need to create ttl range with size of 4\n");
		} else {
			ret_code = tpm_proc_ipv4_ttl_init(ttl_illegal_action);
			IF_ERROR(ret_code);
		}
	}

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_L4_MC_DS, &range_conf);
	if (ret_code == TPM_DB_OK) {
		if (range_conf.range_size > 1)
			/* SIZE too big, it is a waste */
			TPM_OS_WARN(TPM_INIT_MOD, "TPM_PNC_IPV6_L4_MC_DS SIZE should be 1, too big now, it is a waste\n");
		ret_code = tpm_proc_ipv6_l4_mc_ds_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_IPV6_L4_MC_DS);

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_MC_SIP, &range_conf);
	if (ret_code == TPM_DB_OK) {
		if (range_conf.range_size > 8)
			/* SIZE too big, it is a waste */
			TPM_OS_WARN(TPM_INIT_MOD, "TPM_PNC_IPV6_MC_SIP SIZE should be 8, too big now, it is a waste\n");
		if (range_conf.range_size < 8)
			/* SIZE too small, it might not be enough */
			TPM_OS_WARN(TPM_INIT_MOD, "TPM_PNC_IPV6_MC_SIP SIZE should be 8, a little bit smaller now\n");
		ret_code = tpm_proc_ipv6_mc_sip_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_IPV6_L4_MC_DS);

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_HOPL, &range_conf);
	if (TPM_TTL_ZERO_ACTION_NOTHING == ttl_illegal_action) {
		if (ret_code == TPM_DB_OK) {
			/* do not check ttl, no need to create hoplimit range */
			TPM_OS_WARN(TPM_INIT_MOD, "do not check hoplimit, no need to create hoplimit range\n");
		}
	} else {
		if ((ret_code != TPM_DB_OK) || (range_conf.range_size < 3)) {
			/* check hoplimit, need to create hoplimit range */
			TPM_OS_WARN(TPM_INIT_MOD, "check hoplimit, need to create hoplimit range with size of 3\n");
		} else {
			ret_code = tpm_proc_ipv6_hoplimit_init(ttl_illegal_action);
			IF_ERROR(ret_code);
		}
	}
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IGMP, &range_conf);
	tpm_db_igmp_get_snoop_enable(&snoop_enable);
	if (TPM_TRUE == snoop_enable) {
		if (ret_code == TPM_DB_OK) {
			ret_code = tpm_proc_ipv4_igmp_init();
			IF_ERROR(ret_code);
		} else {
			TPM_OS_ERROR(TPM_INIT_MOD, "PNC range(TPM_PNC_IGMP) not initialized with IGMP snoop enabled\n");
			return ERR_GENERAL;
		}
	} else if (ret_code == TPM_DB_OK) {
		if (range_conf.range_size != 0) {
			/* igmp snoop is disabled, no need to create TPM_PNC_IGMP range */
			TPM_OS_WARN(TPM_INIT_MOD, "igmp snoop is disabled, no need to create TPM_PNC_IGMP range\n");
		}
	}

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_PROTO, &range_conf);
	if (ret_code == TPM_DB_OK) {
		ret_code = tpm_proc_ipv4_proto_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_IPV4_PROTO);

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_FRAG, &range_conf);
	if (ret_code == TPM_DB_OK) {
		ret_code = tpm_proc_ipv4_frag_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_IPV4_FRAG);

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CATCH_ALL, &range_conf);
	if (ret_code == TPM_DB_OK) {
		ret_code = tpm_proc_catch_all_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_CATCH_ALL);

	tpm_db_get_tcp_flag_check(&tcp_flag_check);
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_TCP_FLAG, &range_conf);
	ret_code_tcp_flag = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_TCP_FLAG, &range_conf_tcp_flag);
	if (TPM_TCP_FLAG_NOT_CHECK == tcp_flag_check) {
		if (ret_code == TPM_DB_OK || ret_code_tcp_flag == TPM_DB_OK) {
			/* do not check tcp flag, no need to create tcp flag range */
			TPM_OS_WARN(TPM_INIT_MOD, "do not check tcp flag, no need to create IPv4/6 tcp flag range\n");
		}
	} else {
		if ((ret_code != TPM_DB_OK) || (range_conf.range_size < 3)) {
			/* check tcp flag, need to create tcp flag range */
			TPM_OS_WARN(TPM_INIT_MOD, "check tcp flag, need to create IPv4 tcp flag range with size of 3\n");
		} else if ((ret_code_tcp_flag != TPM_DB_OK) || (range_conf_tcp_flag.range_size < 2)) {
			/* check tcp flag, need to create tcp flag range */
			TPM_OS_WARN(TPM_INIT_MOD, "check tcp flag, need to create IPv6 tcp flag range with size of 2\n");
		} else {
			ret_code = tpm_proc_tcp_flag_init();
			IF_ERROR(ret_code);
		}
	}

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_VIRT_UNI, &range_conf);
	if (ret_code == TPM_DB_OK) {
		ret_code = tpm_proc_virt_uni_init();
		IF_ERROR(ret_code);
	} else
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) not initialized\n", TPM_PNC_VIRT_UNI);

	ret_code = tpm_proc_mtu_init();
	IF_ERROR(ret_code);

	ret_code = tpm_init_ethertype_dsa_tag();
	IF_ERROR(ret_code);

	ret_code = tpm_fc_engine_init();
	IF_ERROR(ret_code);

	return (TPM_OK);
}

/*******************************************************************************
* tpm_init_vlan_etype_set()
*
* DESCRIPTION:      Set configured VLAN ETYs in the TPM DB and hardware
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_init_vlan_etype_set(void)
{
	int32_t ret_code = TPM_OK;
	uint32_t etype_sel;
	int i;

	for (i = 0; i < TPM_NUM_VLAN_ETYPE_REGS; i++) {
		if (tpm_init.vlan_etypes[i] != MV_TPM_UN_INITIALIZED_INIT_PARAM) {
			ret_code = tpm_mod2_tpid_get(tpm_init.vlan_etypes[i], &etype_sel);
			IF_ERROR(ret_code);
		}

	}

	return (ret_code);
}

/*******************************************************************************
* tpm_pnc_init_temp()
*
* DESCRIPTION:      when TPM is responsible to configure the PNC:
*                   * PNC clean ALL
*                   * Add entry #0 for HW forwarding enabled
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
void tpm_pnc_init_temp(void)
{
	int rc;
	unsigned int config_pnc_parser_val;

	/* get the config_pnc_parser value */
	config_pnc_parser_val = mv_eth_ctrl_pnc_get();
	if (config_pnc_parser_val == 0) {
		rc = tcam_hw_init();
		if (rc != TPM_OK)
			TPM_OS_WARN(TPM_INIT_MOD, " recvd ret_code(%d)\n", rc);
		pnc_def_entry_init(0, 0);	/* rxq =0 */
	}
	return;
}

/*******************************************************************************
* tpm_init_system_init()
*
* DESCRIPTION:      Initialize the system hardware.
*                   First all the one-time Inits are performed,
*                   and then a call to the dynamic sections is performed by calling mib_reset
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_init_system_init(tpm_reset_level_enum_t reset_type)
{
	int32_t ret_code;
	tpm_db_gmac_func_t gfunc;
	tpm_db_gmac_conn_t gconn = TPM_GMAC_CON_DISC;
	tpm_gmacs_enum_t gmac_i;
	uint16_t txp;

	TPM_OS_DEBUG(TPM_INIT_MOD, "\n");

	/* check PON and GMAC state */
	for (gmac_i = 0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gfunc);

		if (gfunc == TPM_GMAC_FUNC_NONE)
			continue;
		if (!tpm_init_check_gmac_init(gmac_i)) {
			TPM_OS_ERROR(TPM_INIT_MOD, "GMAC port(%d) is not init, system is not fully functional!\n", gmac_i);
			return TPM_FAIL;
		}
	}
	/* Init Vlan Ethertype Registers */
	ret_code = tpm_init_vlan_etype_set();
	IF_ERROR(ret_code);

	/* Init HW Modification table and registers */
	for (gmac_i = 0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++) {
		if (!INIT_GMAC_VALID(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gfunc);
		if (gmac_i != TPM_ENUM_PMAC)
			tpm_db_gmac_conn_get(gmac_i, &gconn);

		if (gfunc != TPM_GMAC_FUNC_NONE &&
				(gconn != TPM_GMAC_CON_DISC || gmac_i == TPM_ENUM_PMAC)) {
			/*ret_code = tpm_mod2_inv_all(); */
			ret_code = tpm_mod2_mac_inv(gmac_i);
			IF_ERROR(ret_code);
			if (gmac_i == TPM_ENUM_PMAC) {
				for (txp = 0; txp < 8; txp++)
					ret_code = tpm_mod2_registers_init(gmac_i, txp);
			} else
				ret_code = tpm_mod2_registers_init(gmac_i, 0);

			IF_ERROR(ret_code);
		}
	}
	/* temporarily = clean the PNC if needed */
	tpm_pnc_init_temp();

	/* Init hardware forwarding configuration */
	tpm_init_hwf_set();

	/* Init tx_queue sizes */
	tpm_init_txq_size_set();

	/* Init rx_queue sizes */
	tpm_init_rxq_size_set();

	/* Init marvell header configuration */
	ret_code = tpm_init_mh(tpm_init.ds_mh_set_conf,
				tpm_init.gmac0_mh_en,
				tpm_init.gmac1_mh_en,
				tpm_init.gmac_port_conf);
	IF_ERROR(ret_code);

	/* Init Buffer Mngmt Pool configuration */
	tpm_init_bm_pool();

	/* Init IPG */
	tpm_init_ipg((-TPM_MH_LEN));

	ret_code = tpm_init_system_mib_reset(reset_type);
	IF_ERROR(ret_code);

	return (TPM_OK);
}

/*******************************************************************************
* tpm_module_init()
*
* DESCRIPTION:      Initialize the TPM module
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_module_init(void)
{
	int32_t ret_code;

	/* Reset DB */
	ret_code = tpm_db_init();
	if (ret_code != TPM_DB_OK) {
		printk(KERN_ERR "Error in Init DB\n");
		return (TPM_FAIL);
	}

	ret_code = tpm_mod2_init();
	if (ret_code != TPM_OK) {
		printk(KERN_ERR "Error in Init MOD2\n");
		return (TPM_FAIL);
	}

	return (TPM_OK);
}

/*******************************************************************************
* tpm_module_start()
*
* DESCRIPTION:      Initialize the TPM module
*
* INPUTS:

* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
int32_t tpm_module_start(tpm_init_t *tpm_init)
{
	int32_t ret_code;
	static uint32_t first_time = 0;

	if (first_time == 0) {

		/* Initialize XML configuration to the init database */
		tpm_init_config_params_init(tpm_init);

		/* Update database hardcoded configuration */
		ret_code = tpm_init_config_params_update();
		if (ret_code != TPM_OK) {
			printk(KERN_ERR "Error in Init Configuration \n");
			return (TPM_FAIL);
		}

		/* Check all Configuration params are consistent */
		ret_code = tpm_init_info_validate();
		if (ret_code != TPM_OK) {
			printk(KERN_ERR "Error in Init Info Validate \n");
			return (TPM_FAIL);
		}

		/* Transform raw form Init Info into TPM format and update DB
		 * Involves internal mapping logic */
		ret_code = tpm_init_info_set();
		if (ret_code != TPM_OK) {
			printk(KERN_ERR "Error in  Init Info Set \n");
			return (TPM_FAIL);
		}

		/* Initialize TPM Locks */
		tpm_init_locks_init();

		/* Initialize System (HW) */
		/* In HW reset all PnC Ranges, except for those that should never be reset (e.g. PNC_INIT_US for NFS) */
		ret_code = tpm_init_system_init(TPM_ENUM_RESET_LEVEL1);
		if (ret_code != TPM_OK) {
			printk(KERN_ERR "\n= TPM Module - Error in System Init =\n");
			return (TPM_FAIL);
		} else {
			printk(KERN_INFO "\n= TPM Module Init ended successfully =\n");
			tpm_db_init_done_set();
		}

	} else
		TPM_OS_INFO(TPM_INIT_MOD, " skipped\n");

	first_time++;

	return (TPM_OK);
}
