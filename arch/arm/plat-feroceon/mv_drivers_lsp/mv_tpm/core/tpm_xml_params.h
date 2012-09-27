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
#ifndef _TPM_XML_PARAMS_H_
#define _TPM_XML_PARAMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#define US_XML_PON_SN_LEN          (8)
#define US_XML_PON_PASSWD_LEN      (10)

#define US_RC_OK           0
#define US_RC_FAIL         1
#define US_RC_NOT_FOUND    2

typedef enum {
	NONE_SW_APP = 0,
	EPON_SW_APP,
	GPON_SW_APP,
	GBE_SW_APP,
	P2P_SW_APP,

	MAX_SW_APP
} us_sw_app_t;

typedef enum {
	US_DISABLED = 0,
	US_ENABLED
} us_enabled_t;

typedef enum {
	US_OFF = 0,
	US_ON
} us_state_t;

typedef enum {
	ONU_PON_DISCONNECT = 0,
	ONU_PON_CONNECT
} PON_connect_type_t;

typedef enum {
	PON_XML_DEF_PAR_SERIAL_NUM         = 0,
	PON_XML_DEF_PAR_PASSWORD           = 1,
	PON_XML_DEF_PAR_DIS_SERIAL_NUM     = 2,
	PON_XML_DEF_PARAM_CLEAR_GEM        = 3,
	PON_XML_DEF_PARAM_CLEAR_TCONT      = 4,
	PON_XML_DEF_PARAM_SERIAL_NUM_SRC   = 5,
	PON_XML_DEF_PARAM_XVR_POLARITY     = 6,
	PON_XML_DEF_PARAM_DG_POLARITY      = 7,
	PON_XML_DEF_PARAM_RESTORE_GEM      = 8,
	PON_XML_DEF_PARAM_MAX

} PON_XML_DEF_PARAMS;

#define US_RC_OK           0
#define US_RC_FAIL         1
#define US_RC_NOT_FOUND    2

#define US_XML_CFG_FILE            "/etc/xml_params/tpm_xml_cfg_file.xml"
#define US_XML_CFG_FILE_OMCI       "/etc/xml_params/omci_xml_cfg_file.xml"
#define US_XML_CFG_FILE_PON        "/etc/xml_params/pon_xml_cfg_file.xml"
#define US_XML_CFG_FILE_GLOBAL     "/etc/xml_params/pon_type_xml_cfg_file.xml"

#define US_DEBUG_PRINT

#define US_XML_PON_E               "PON"
#define US_XML_PON_TYPE_E          "WAN_type"
#define US_XML_PON_TYPE_FORCE_E    "WAN_type_force"
#define US_XML_PON_SN_E            "PON_serial_num"
#define US_XML_PON_PASSWD_E        "PON_passwd"
#define US_XML_PON_DIS_SN_E        "PON_dis_sn"
#define US_XML_PON_GEM_RST_E       "PON_gem_reset"
#define US_XML_PON_TCONT_RST_E     "PON_tcont_reset"
#define US_XML_PON_SN_SRC_E        "PON_serial_src"
#define US_XML_PON_DG_POL_E        "PON_DG_polarity"
#define US_XML_PON_XVR_POL_E       "PON_XVR_polarity"
#define US_XML_PON_GEM_RESTOR_E    "PON_gem_restore"

#define US_XML_EPON_E               "EPON"
#define US_XML_EPON_CONNECT_E       "EPON_connect_tpm"
#define US_XML_EPON_DG_POL_E        "EPON_DG_polarity"
#define US_XML_EPON_XVR_POL_E       "EPON_XVR_polarity"
#define US_XML_EPON_OAM_RXQ_E       "EPON_OAM_RX_queue"

#define US_XML_OMCI_E              "OMCI"
#define US_XML_OMCI_ETY_E          "OMCI_ETY"

#define US_XML_IGMP_SNOOP_E        "igmp_snoop"
#define US_XML_IGMP_SNOOP_ALL_E    "snoop_all"
#define US_XML_ENABLED_E           "enabled"
#define US_XML_IGMP_CPU_Q_E        "igmp_cpu_rx_queue"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_0           "igmp_frwrd_mode_uni0"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_1           "igmp_frwrd_mode_uni1"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_2           "igmp_frwrd_mode_uni2"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_3           "igmp_frwrd_mode_uni3"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_4           "igmp_frwrd_mode_uni4"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_5           "igmp_frwrd_mode_uni5"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_6           "igmp_frwrd_mode_uni6"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_7           "igmp_frwrd_mode_uni7"
#define US_XML_IGMP_PKT_FRWD_MODE_UNI_VIRT        "igmp_frwrd_mode_uni_virt"
#define US_XML_IGMP_PKT_FRWD_MODE_WAN             "igmp_frwrd_mode_wan"

#define US_XML_PORT_INIT_E         "port_init"
#define US_XML_DEBUG_PORT_E        "debug_port"
#define US_XML_INT_PORT_E          "int_port"

#define US_XML_ETH_PORT_PARAMS_E   "Ethernet_parameters"
#define US_XML_ETH_PORT_E          "Ethernet"
#define US_XML_ETH_CHIP_CONN_ATTR  "chip_connect"
#define US_XML_ETH_INT_CONN_ATTR   "int_connect"
#define US_XML_ETH_SW_PORT_ATTR    "switch_ports"
#define US_XML_GMAC_PORTS_E        "gmac_ports"
#define US_XML_ETH_ADMIN_ATTR      "admin_state"
#define US_XML_ETH_SPEED_ATTR      "speed"
#define US_XML_ETH_DUPLEX_ATTR     "duplex"
#define US_XML_ETH_PORT_SRC_ATTR   "src_port"
#define US_XML_ETH_PORT_CHIPCON_ATTR "chip_conn"
#define US_XML_ETH_GMAC_CONN_ATTR  "conn"
#define US_XML_ETH_PORT_INTCON_ATTR "intr_conn"
#define US_XML_ETH_CMPLX_CONFIG_E  "eth_complex_config"
#define US_XML_WAN_E		    "WAN"
#define US_XML_BACKUP_WAN_E        "backup_wan"
#define US_XML_VIRTUAL_UNI_E       "virtual_uni"

#define US_XML_ETH_PROFILE_E       "profile"

#define US_XML_GMAC_CONFIG_E       "gmac_config"
#define US_XML_GMAC_CONN_E         "pon_config"
#define US_XML_TCONT_LLID_E        "num_tcont_llid"
#define US_XML_GMAC_MH_ENA_E       "gmac_mh_en"
#define US_XML_GMAC_0_MH_ENA_E     "gmac0_mh_en"
#define US_XML_GMAC_1_MH_ENA_E     "gmac1_mh_en"

#define US_XML_GMAC_BM_BUF_E       "gmac_bm_buffers"
#define US_XML_LARGE_POOL_BUF_ATTR "large_pkt_pool_bufs"
#define US_XML_SHORT_POOL_BUF_ATTR "small_pkt_pool_bufs"

#define US_XML_GMAC_RXQUEUES_E     "gmac_rx_queues"

#define US_XML_TX_MOD_PARAMS_E     "tx_module_parameters"
#define US_XML_TX_MOD_E            "tx_mod"
#define US_XML_QUEUE_MAP_E         "queue_map"
#define US_XML_QUEUE_E             "queue"
#define US_XML_QUEUE_SHED_ATTR     "sched_method"
#define US_XML_QUEUE_OWNER_ATTR    "owner"
#define US_XML_QUEUE_OW_Q_NUM_ATTR "owner_q_num"
#define US_XML_QUEUE_WEIGHT_ATTR   "weight"

#define US_XML_FC_PARAMS_E         "flow_control"
#define US_XML_Q_SAMPLE_FREQ_E     "queue_sample_freq"
#define US_XML_THRESH_HIGH_E       "thresh_high"
#define US_XML_THRESH_LOW_E        "thresh_low"
#define US_XML_TGT_PORT_E          "tgt_port"
#define US_XML_TX_PORT_E           "tx_port"
#define US_XML_TX_Q_E              "tx_queue"


#define US_XML_TPM_E               "TPM"
#define US_XML_DS_MH_SET_E         "ds_mh_set"
#define US_XML_VALID_EN_E          "validation_enable"
#define US_XML_CFG_PNC_PARSER      "pnc_config"
#define US_XML_CFG_CPU_LOOPBACK    "cpu_wan_egr_loopback"
#define US_XML_TRACE_DEBUG_info    "trace_debug_info"
#define US_XML_IPV6_5T_ENA_E       "ipv6_5t_enable"
#define US_XML_CTC_CNM_ENA_E       "ctc_cnm_enable"
#define US_XML_WIFI_ENABLED        "gmac1_virt_uni"
#define US_XML_IPV4_MC_SUPPORT_E   "ipv4_mc_support"
#define US_XML_IPV6_MC_SUPPORT_E   "ipv6_mc_support"
#define US_XML_CTC_CNM_IPv6_PARSE_WIN_E       "ctc_cnm_ipv6_parsing_window"
#define US_XML_PNC_MAC_LEARN_ENA_E "pnc_mac_learn_enable"
#define US_XML_SWITCH_INIT_E       "switch_init"

/*#define US_XML_WIFI_PORT           "Vitual_UNI_GMAC1_port"*/

#if 0
#define US_XML_DEF_V1_TPID         "vlan1_tpid"
#define US_XML_DEF_V2_TPID         "vlan2_tpid"
#endif
#define US_XML_TPM_VLAN_FILTER_TPID_E     "vlan_filter_tpid"
#define US_XML_TPM_FILTER_TPID_E          "filter_tpid"


#define US_XML_TPM_PNC_E                  "PnC"
#define US_XML_TPM_PNC_RANGE_PARAMS_E     "range_parameters"
#define US_XML_TPM_PNC_RANGE_E            "range"
#define US_XML_TPM_CNTR_GRP_ATTR          "cntr_grp"
#define US_XML_TPM_LU_MASK_ATTR           "lu_mask"
#define US_XML_TPM_MIN_RES_LVL_ATTR       "min_reset_level"
#define US_XML_TPM_MOD_VLAN_TPID_E        "vlan_mod_tpid"
#define US_XML_TPM_MOD_TPID_E             "mod_tpid"
#define US_XML_TPM_MOD_E                  "modification"
#define US_XML_TPM_MOD_UDP_CKS_USE_INIT_E "udp_checksum_use_init"
#define US_XML_TPM_MOD_UDP_CKS_CALC_E     "udp_checksum_calc"
#define US_XML_SPLIT_MOD_SETTING_E        "split_mod"
#define US_XML_SPLIT_MOD_ENABLE_E         "enable"
#define US_XML_SPLIT_MOD_P_BITS_E         "p_bits"
#define US_XML_SPLIT_MOD_VLAN_NUM_E       "vlan_num"
#define US_XML_SPLIT_MOD_MODE_E           "split_mod_mode"


#define US_XML_TPM_MOD_CHAIN_PARAM_E     "chain_parameters"
#define US_XML_TPM_MOD_CHAIN_E           "chain"
#define US_XML_TPM_CHAIN_ID_ATTR         "id"
#define US_XML_TPM_CHAIN_TYPE_ATTR       "type"
#define US_XML_TPM_CHAIN_NUM_ATTR        "num"

#define US_XML_ID_ATTR             "id"
#define US_XML_GMAC_E              "gmac"
#define US_XML_PORT_E              "port"
#define US_XML_SIZE_ATTR           "size"
#define US_XML_NUM_ATTR            "num"
#define US_XML_TYPE_ATTR           "type"
#define US_XML_VALID_ATTR          "valid"


#define US_XML_MTU_SETTING_E          "mtu_setting"

#define US_XML_MTU_SETTING_ENABLE_E   "mtu_setting_enabled"
#define US_XML_IPV4_MTU_US_E          "ipv4_mtu_us"
#define US_XML_IPV4_PPPOE_MTU_US_E    "ipv4_pppoe_mtu_us"
#define US_XML_IPV6_MTU_US_E          "ipv6_mtu_us"
#define US_XML_IPV6_PPPOE_MTU_US_E    "ipv6_pppoe_mtu_us"
#define US_XML_IPV4_MTU_DS_E          "ipv4_mtu_ds"
#define US_XML_IPV6_MTU_DS_E          "ipv6_mtu_ds"

#define US_XML_TRAFFIC_SETTING_E      "traffic_setting"
#define US_XML_PPPOE_ENABLE_E         "pppoe_add_enable"
#define US_XML_NUM_VLAN_TAGS_E        "num_vlan_tags"
#define US_XML_CPU_RX_QUEUE_E         "cpu_trap_rx_queue"
#define US_XML_TTL_ZERO_ACTION_E      "ttl_illegal_action"
#define US_XML_TCP_FLAG_CHECK_E       "tcp_flag_check"
#define US_XML_ETY_DSA_ENABLE_E       "ety_dsa_enable"


#define US_XML_MC_SETTING_E                  "multicast"
#define US_XML_MC_FILTER_MODE_E              "mc_filter_mode"
#define US_XML_MC_PER_UNI_VLAN_XLAT_E        "mc_per_uni_vlan_xlat"
#define US_XML_MC_PPPOE_ENABLE_E             "mc_pppoe_enable"
#define US_XML_MC_HWF_Q_E                    "mc_hwf_queue"
#define US_XML_MC_CPU_Q_E                    "mc_cpu_queue"
#define US_XML_CATCH_ALL_PKT_ACTION_E        "catch_all_pkt_action"

/*******************************************************************************
*                             Configuration functions
*******************************************************************************/
int get_pon_type_param(tpm_init_pon_type_t *pon_type);
int get_omci_etype_param(uint32_t *ety);
int get_debug_port_params(uint32_t *valid, uint32_t *num);
int get_igmp_snoop_params_enable(uint32_t *enable);
int get_igmp_snoop_params_cpu_queue(uint32_t *que);
int get_igmp_snoop_params_port_frwd_mode(uint32_t *mode, tpm_src_port_type_t port);
int get_gmac_conn_params(uint32_t *num_tcont_llid);
int get_gmac_mh_en_params(uint32_t *gmac0_mh_en, uint32_t *gmac1_mh_en);
int get_eth_cmplx_profile_params(tpm_eth_complex_profile_t *eth_cmplx_profile);
int get_gmac_port_conf_params(tpm_init_gmac_conn_conf_t *gmac_port_conf_bufs, int max_gmac_port_num);
int get_eth_port_conf_params(tpm_init_eth_port_conf_t *eth_port_bufs, int max_eth_port_num);
int get_backup_wan_params(uint32_t *backup_wan);

int get_gmac_pool_bufs_params(tpm_init_gmac_bufs_t *gmac_bufs, int max_gmacs_num);
int get_trace_debug_info_param(uint32_t *trace_debug_info);
int get_validation_enabled_config_params(tpm_init_tpm_validation_t *validEn);
int get_vitual_uni_enable_params(uint32_t *enable);
#if 0
int get_default_vlan_tpid_params(uint16_t *v1_tpid, uint16_t *v2_tpid);
#endif
int get_default_vlan_tpid_params(uint32_t *opt_num, tpm_init_tpid_comb_t *opt);
int get_fc_def_params(tpm_init_fc_params_t *port_fc_conf);
int get_gmac_rxq_params(tpm_init_gmac_rx_t *gmac_rx, int max_gmacs_num, int max_rx_queues_num);
int get_gmac_tx_params(tpm_init_gmac_tx_t *gmac_tx, int max_tx_ports_num, int max_tx_queues_num);
int get_pnc_range_params(tpm_init_pnc_range_t *pnc_range, int max_pnc_ranges_num);
int get_ds_mh_config_params(tpm_init_mh_src_t *dsMhConf);
int get_vlan_mod_vlan_tpid_params(uint32_t *mod_vlan_tpid, int mod_vlan_tpid_reg_num);
int get_modification_params(tpm_init_mod_params_t *config);
int get_chain_config_params(tpm_init_mod_chain_t *mod_chain);
int get_cfg_pnc_parse_param(tpm_init_cfg_pnc_parse_t *cfgPncParse);
int get_cpu_loopback_param(tpm_init_cpu_loopback_t *cpuLoopback);

void free_xml_head_ptr(void);

/* jinghua add for MTU setting */

int get_ipv4_mtu_us(uint32_t *mtu);
int get_ipv4_pppoe_mtu_us(uint32_t *mtu);
int get_ipv6_mtu_us(uint32_t *mtu);
int get_ipv6_pppoe_mtu_us(uint32_t *mtu);
int get_ipv4_mtu_ds(uint32_t *mtu);
int get_ipv6_mtu_ds(uint32_t *mtu);
int get_pppoe_add_enable(tpm_init_pppoe_add_enable_t *enable);
int get_num_vlan_tags(uint32_t *num_vlan_tags);
int get_mtu_enable(tpm_init_mtu_setting_enable_t *enable);
int get_cpu_rx_queue(uint32_t *cpu_rx_queue);
int get_ttl_illegal_action(uint32_t *ttl_illegal_action);
int get_tcp_flag_check(uint32_t *tcp_flag_check);

int get_mc_filter_mode(tpm_mc_filter_mode_t *mc_filter_mode);
int get_mc_per_uni_vlan_xlat(uint32_t *mc_per_uni_vlan_xlat);
int get_mc_pppoe_enable(uint32_t *mc_pppoe_enable);
int get_mc_queue(uint32_t *hwf_queue, uint32_t *cpu_queue);
int get_catch_all_pkt_action(uint32_t *catch_all_pkt_action);
int get_ety_dsa_enable(uint32_t *ety_dsa_enable);
int get_ipv6_5t_enable(uint32_t *ipv6_5t_enable);
int get_ipv4_mc_support(uint32_t *ipv4_mc_support);
int get_ipv6_mc_support(uint32_t *ipv6_mc_support);
int get_split_mod_enable(uint32_t *split_mod_enable);
int get_split_mot_p_bits(uint8_t *p_bits, uint32_t *p_bit_num);
int get_split_mod_vlan_num(uint32_t *split_vlan_num);
int get_split_mod_mode(tpm_init_split_mod_mode_t *split_mod_mode);
int get_ctc_cm_enable(uint32_t *ctc_cm_enable);
int get_ctc_cm_ipv6_parse_window(uint32_t *ctc_cm_ipv6_parse_window);
int get_pnc_mac_learn_enable(uint32_t *pnc_mac_learn_enable);
int get_switch_init_params(uint32_t *switch_init);

#ifdef __cplusplus
}
#endif
#endif				/* _PARAMS_H_ */
