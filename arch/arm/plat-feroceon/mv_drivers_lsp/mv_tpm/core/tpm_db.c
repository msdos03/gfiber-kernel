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
*
* tpm_db.c
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
*               Revision: 1.4
*
*
*******************************************************************************/

#include "tpm_common.h"
#include "tpm_header.h"

#include <linux/vmalloc.h>

/* Local definitions */
#define PNC_RANGE_OPER(x)         tpm_db.pnc_range[x].pnc_range_oper
#define PNC_RANGE_CONF(x)         tpm_db.pnc_range[x].pnc_range_conf
#define TPM_DBVAL_CON(dbval)      ((dbval == TPM_DB_VALID) ? TPM_TRUE : TPM_FALSE)
#define TPM_DB_DIVIDED_BY_32(x)   (x >> 5)
#define TPM_DB_MOD_32(x)          (x & 0x1f)

#define MEM_MARGIN_PATTERN        (0xA3)
#define API_ENT_I(api_sec, i) tpm_db.api_ent_mem_area[(tpm_db.api_section[api_sec].table_start) + i]
#define IF_ERROR(ret) \
	if (ret != TPM_DB_OK) { \
		TPM_OS_ERROR(TPM_DB_MOD, " recvd ret_code(%d)\n", ret); \
		return(ret); \
	}
#define IF_ERROR_I(ret, i) \
	if (ret != TPM_DB_OK) { \
		TPM_OS_ERROR(TPM_DB_MOD, " For index(%d), recvd ret_code(%d)\n", i, ret); \
		return(ret); \
	}

/* TODO - change to static variable after updating print functions */
tpm_db_t tpm_db;
tpm_db_hot_swap_bak_db_t hot_swap_bak_db;

static uint32_t mem_alloc_start_ind;

const char *entry_state_str[TPM_MOD_ENTRY_STATUS_MAX] = { "F", "B", "O", "R", "S" };

uint32_t *tpm_db_mod2_jump_booked_entry_bm[TPM_MAX_NUM_GMACS];
uint32_t *tpm_db_mod2_jump_occupied_entry_bm[TPM_MAX_NUM_GMACS];
uint32_t *tpm_db_mod2_jump_split_mod_occupied_entry_bm[TPM_MAX_NUM_GMACS];
uint32_t *tpm_db_mod2_jump_split_mod_all_entry_bm[TPM_MAX_NUM_GMACS];

uint32_t tpm_db_mod2_jump_bm_group;

uint32_t *tpm_db_mod2_chain_booked_entry_bm[TPM_MAX_NUM_GMACS][TPM_CHAIN_TYPE_MAX];
uint32_t *tpm_db_mod2_chain_occupied_entry_bm[TPM_MAX_NUM_GMACS][TPM_CHAIN_TYPE_MAX];

uint32_t tpm_db_mod2_chain_bm_group[TPM_CHAIN_TYPE_MAX];

tpm_mod2_jump_pmt_info_t *tpm_db_mod2_jump_pmt_info[TPM_MAX_NUM_GMACS];
tpm_mod2_chain_pmt_info_t *tpm_db_mod2_chain_pmt_info[TPM_MAX_NUM_GMACS];

tpm_mod2_chain_info_t *tpm_db_mod2_chain_info[TPM_MAX_NUM_GMACS][TPM_CHAIN_TYPE_MAX];

tpm_mod2_jump_cfg_t tpm_db_mod2_jump_area_cfg;
tpm_mod2_chain_cfg_t tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAX];

uint16_t tpm_mod2_chain_area_base;
uint16_t tpm_mod2_total_chain_num;

uint8_t tpm_mod2_multicast_mh_enable;
uint8_t tpm_mod2_pppoe_add_mod_enable;
uint8_t tpm_mod2_double_tag_enable;
uint8_t tpm_mod2_udp_checksum_use_init_bm_enable;
uint8_t tpm_mod2_udp_checksum_enable;

tpm_db_mc_stream_entry_t *tpm_db_mc_stream_table[TPM_MC_MAX_STREAM_NUM];
tpm_db_mc_mac_entry_t *tpm_db_mc_mac_table[TPM_MC_MAX_MAC_NUM];

uint32_t tpm_db_mc_lpbk_entries_num;
tpm_db_mc_lpbk_entry_t *tpm_db_mc_lpbk_table[TPM_MC_MAX_LPBK_ENTRIES_NUM];
tpm_db_mc_vlan_entry_t *tpm_db_mc_vlan_xits_table[TPM_MC_MAX_MVLAN_XITS_NUM];

uint8_t tpm_db_mc_virt_uni_entry_state_table[TPM_MC_MAX_STREAM_NUM];

uint8_t tpm_db_mc_igmp_proxy_sa_mac[6];
uint8_t tpm_db_mc_igmp_proxy_sa_mac_valid;

static uint32_t tpm_db_global_rule_idx = 1000;

/* Function Declarations */
int32_t tpm_db_api_freeentry_get(tpm_api_sections_t api_section, int32_t *index);
int32_t tpm_db_api_entry_ind_get(tpm_api_sections_t api_section, uint32_t rule_num, int32_t *index);
int32_t tpm_db_mod_shdw_par_check(tpm_gmacs_enum_t gmac, uint32_t mod_entry);


/*******************************************************************************
* tpm_db_eth_port_conf_get()
*
* DESCRIPTION:      Function to get an ethernet port physical configuration from the DB
*
* INPUTS:
* ext_port         - External Port Identifier, can be any positive number
*
* OUTPUTS:
*                 - See explanation in tpm_db_eth_port_conf_t structure
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
bool tpm_db_eth_port_valid(tpm_src_port_type_t src_port_num)
{
	uint32_t i;

	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if ((tpm_db.eth_ports[i].port_src == src_port_num)
		    && (tpm_db.eth_ports[i].valid == TPM_DB_VALID))
			return (true);
	}

	return false;
}

/*******************************************************************************
* tpm_db_eth_port_conf_get()
*
* DESCRIPTION:      Function to get an ethernet port physical configuration from the DB
*
* INPUTS:
* ext_port         - External Port Identifier, can be any positive number
*
* OUTPUTS:
*                 - See explanation in tpm_db_eth_port_conf_t structure
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_eth_port_conf_get(tpm_src_port_type_t src_port_num,
				 tpm_db_chip_conn_t *chip_con,
				 tpm_db_int_conn_t *int_con,
				 uint32_t *switch_port)
{
	uint32_t i;

	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if ((tpm_db.eth_ports[i].port_src == src_port_num)
		    && (tpm_db.eth_ports[i].valid == TPM_DB_VALID)) {
			*chip_con = tpm_db.eth_ports[i].chip_connect;
			*int_con = tpm_db.eth_ports[i].int_connect;
			*switch_port = tpm_db.eth_ports[i].switch_port;
			return (TPM_DB_OK);
		}
	}

	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_eth_port_switch_port_get()
*
* DESCRIPTION:      Function to get an ethernet port physical configuration from the DB
*
* INPUTS:
* ext_port         - UNI Port Identifier, can be any positive number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns the switch port number.
* On error, TPM_DB_ERR_PORT_NUM = 0xFF is returned.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_eth_port_switch_port_get(tpm_src_port_type_t src_port)
{
	uint32_t i;

       if (TPM_SRC_PORT_UNI_VIRT == src_port) {
                if (TPM_DB_VALID == tpm_db.func_profile.virt_uni_info.enabled)
                        return tpm_db.func_profile.virt_uni_info.switch_port;
               else
                        return TPM_DB_ERR_PORT_NUM;
       } else if (TPM_SRC_PORT_WAN == src_port) {
               for (i = TPM_ENUM_GMAC_0; i < TPM_MAX_GMAC; i++) {
			/*  in case traffic from WAN, the dest is LAN,
			    so we search for switch port connected to GMAC LAN functionality */
                       if (TPM_GMAC_FUNC_LAN == tpm_db.gmac_func[i]){
                               if (TPM_GMAC_CON_SWITCH_4 == tpm_db.gmac_port_conf[i].conn)
                                       return TPM_GMAC0_AMBER_PORT_NUM;
                               else
                                       return TPM_GMAC1_AMBER_PORT_NUM;
                       }
               }
       } else {
               for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
                       if ((tpm_db.eth_ports[i].port_src == src_port) &&
                           (TPM_DB_VALID == tpm_db.eth_ports[i].valid))
                               return (tpm_db.eth_ports[i].switch_port);
		}

	}
	return (TPM_DB_ERR_PORT_NUM);
}
uint32_t tpm_db_rule_index_get(void)
{
	return tpm_db_global_rule_idx;
}
int32_t tpm_db_rule_index_set(uint32_t rule_index)
{
	tpm_db_global_rule_idx = rule_index;
	return TPM_DB_OK;
}
void tpm_db_rule_index_incrs(void)
{
	tpm_db_global_rule_idx++;
}
bool tpm_db_gmac1_lpbk_en_get(void)
{
	return tpm_db.func_profile.gmac1_loopback_en;
}
void tpm_db_gmac1_lpbk_en_set(bool en)
{
	tpm_db.func_profile.gmac1_loopback_en = en;
}
bool tpm_db_cpu_wan_lpbk_en_get(void)
{
	return tpm_db.func_profile.cpu_wan_loopback_en;
}
void tpm_db_cpu_wan_lpbk_en_set(bool en)
{
	tpm_db.func_profile.cpu_wan_loopback_en = en;
}
bool tpm_db_ds_load_bal_en_get(void)
{
	return tpm_db.func_profile.ds_load_bal_en;
}
void tpm_db_ds_load_bal_en_set(bool en)
{
	tpm_db.func_profile.ds_load_bal_en = en;
}
bool tpm_db_switch_active_wan_en_get(void)
{
	return tpm_db.func_profile.switch_active_wan_en;
}
void tpm_db_switch_active_wan_en_set(bool en)
{
	tpm_db.func_profile.switch_active_wan_en = en;
}

int32_t tpm_db_get_valid_uni_ports_num(uint32_t *num_ports)
{
	uint32_t i, _num_ports = 0;

	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (tpm_db.eth_ports[i].valid == TPM_DB_VALID) {
			if (tpm_db.eth_ports[i].port_src >= TPM_SRC_PORT_UNI_0 &&
				tpm_db.eth_ports[i].port_src <= TPM_SRC_PORT_UNI_7)
				_num_ports++;
		}
	}

    if (tpm_db.func_profile.virt_uni_info.enabled == TPM_DB_VALID)
		_num_ports++;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_phy_convert_port_index()
*
* DESCRIPTION:      Function to get an ethernet port physical configuration from the DB
*
* INPUTS:
* switch_port         - External Port Identifier, can be any positive number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns the switch port number.
* On error, TPM_DB_ERR_PORT_NUM = 0xFF is returned.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_phy_convert_port_index(int32_t switch_port)
{
	uint32_t i;

	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if ((tpm_db.eth_ports[i].switch_port == switch_port)
		    && (tpm_db.eth_ports[i].valid == TPM_DB_VALID))
			return (tpm_db.eth_ports[i].port_src);
	}

	return (TPM_DB_ERR_PORT_NUM);
}
EXPORT_SYMBOL(tpm_db_phy_convert_port_index);

/*******************************************************************************
* tpm_db_eth_cmplx_profile_set()
*
* DESCRIPTION:      Function to set the Ethernet Complex Profile in the DB
*
* INPUTS:
* eth_cmplx_profile	- the profile enumartion number
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_eth_cmplx_profile_set(tpm_eth_complex_profile_t eth_cmplx_profile)
{
	tpm_db.eth_cmplx_profile = eth_cmplx_profile;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_eth_cmplx_profile_get()
*
* DESCRIPTION:      Function returns the Ethernet Complex Profile from the DB
*
* INPUTS:
*
* OUTPUTS:
*		the profile enumartion number
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
tpm_eth_complex_profile_t tpm_db_eth_cmplx_profile_get(void)
{
	return tpm_db.eth_cmplx_profile;
}


/*******************************************************************************
* tpm_db_mac_func_set()
*
* DESCRIPTION:	Sets all gmacs functionality according to EthCmplx profile
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
* 	On success, returns TPM_OK
* 	On error, returns tpm_db_gmac_func_set error
*******************************************************************************/
int32_t tpm_db_mac_func_set(void)
{
	tpm_db_gmac_func_t pon = 0,
			   gmac0 = 0,
			   gmac1 = 0;
	int32_t ret_code;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	switch(tpm_db.eth_cmplx_profile)
	{
	case TPM_PON_WAN_DUAL_MAC_INT_SWITCH:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_LAN;
       	if (0 != tpm_db.func_profile.virt_uni_info.enabled)
			gmac1 = TPM_GMAC_FUNC_VIRT_UNI;
       	else
			gmac1 = TPM_GMAC_FUNC_NONE;
		break;

	case TPM_PON_WAN_DUAL_MAC_EXT_SWITCH:
       	/* ofer: phase#2, diferentiate between gmacs */
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_LAN;
		gmac1 = TPM_GMAC_FUNC_NONE;
		break;

	case TPM_PON_WAN_G1_LAN_G0_INT_SWITCH:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_LAN;
		gmac1 = TPM_GMAC_FUNC_LAN_UNI;
		break;

	case TPM_PON_WAN_G0_INT_SWITCH:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_LAN;
		gmac1 = TPM_GMAC_FUNC_NONE;
		break;

	case TPM_PON_WAN_G0_SINGLE_PORT:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_LAN;
		gmac1 = TPM_GMAC_FUNC_NONE;
		break;

	case TPM_G0_WAN_G1_INT_SWITCH:
		pon = TPM_GMAC_FUNC_NONE;
		gmac0 = TPM_GMAC_FUNC_WAN;
		gmac1 = TPM_GMAC_FUNC_LAN;
		break;

	case TPM_G1_WAN_G0_INT_SWITCH:
		pon = TPM_GMAC_FUNC_NONE;
		gmac0 = TPM_GMAC_FUNC_LAN;
		gmac1 = TPM_GMAC_FUNC_WAN;
		break;

	case TPM_PON_G1_WAN_G0_INT_SWITCH:
       		if (TPM_ENUM_GMAC_1 == tpm_db.init_misc.active_wan) {
			pon = TPM_GMAC_FUNC_NONE;
			gmac1 = TPM_GMAC_FUNC_WAN;
		}else{
			gmac1 = TPM_GMAC_FUNC_NONE;
			pon = TPM_GMAC_FUNC_WAN;
		}
		gmac0 = TPM_GMAC_FUNC_LAN;
		break;

	case TPM_PON_G1_WAN_G0_SINGLE_PORT:
       		if (TPM_ENUM_GMAC_1 == tpm_db.init_misc.active_wan) {
			pon = TPM_GMAC_FUNC_NONE;
			gmac1 = TPM_GMAC_FUNC_WAN;
		}else{
			gmac1 = TPM_GMAC_FUNC_NONE;
			pon = TPM_GMAC_FUNC_WAN;
       	}
		gmac0 = TPM_GMAC_FUNC_LAN_UNI;
		break;

	case TPM_PON_G0_WAN_G1_INT_SWITCH:
       		if (TPM_ENUM_GMAC_0 == tpm_db.init_misc.active_wan) {
			pon = TPM_GMAC_FUNC_NONE;
			gmac0 = TPM_GMAC_FUNC_WAN;
		}else{
			gmac0 = TPM_GMAC_FUNC_NONE;
			pon = TPM_GMAC_FUNC_WAN;
       	}
		gmac1 = TPM_GMAC_FUNC_LAN_UNI;
		break;

	case TPM_PON_G0_WAN_G1_SINGLE_PORT:
		if (TPM_ENUM_GMAC_0 == tpm_db.init_misc.active_wan) {
			pon = TPM_GMAC_FUNC_NONE;
			gmac0 = TPM_GMAC_FUNC_WAN;
		}else{
			gmac0 = TPM_GMAC_FUNC_NONE;
			pon = TPM_GMAC_FUNC_WAN;
       	}
		gmac1 = TPM_GMAC_FUNC_LAN;
		break;

	case TPM_PON_WAN_G1_MNG_EXT_SWITCH:
	case TPM_PON_WAN_G1_SINGLE_PORT:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_NONE;
		gmac1 = TPM_GMAC_FUNC_LAN_UNI;
		break;

	case TPM_PON_WAN_G0_G1_LPBK:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_US_MAC_LEARN_DS_LAN_UNI;
		gmac1 = TPM_GMAC_FUNC_LAN;
		break;
	case TPM_PON_WAN_G0_G1_DUAL_LAN:
		pon = TPM_GMAC_FUNC_WAN;
		gmac0 = TPM_GMAC_FUNC_LAN_UNI;
		gmac1 = TPM_GMAC_FUNC_LAN_UNI;
		break;
	}

	/* when ds load balance on G0 and G1 is enabled, G0/1 are both LAN */
	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
	if (TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable)
		gmac1 = TPM_GMAC_FUNC_LAN;

	ret_code = tpm_db_gmac_func_set(TPM_ENUM_PMAC, pon);
	IF_ERROR(ret_code);
	ret_code = tpm_db_gmac_func_set(TPM_ENUM_GMAC_0, gmac0);
	IF_ERROR(ret_code);
	ret_code = tpm_db_gmac_func_set(TPM_ENUM_GMAC_1, gmac1);
	IF_ERROR(ret_code);

	return (TPM_OK);
}

/*******************************************************************************
* tpm_db_eth_max_uni_port_set()
*
* DESCRIPTION:	Sets the max_uni_port_nr according to EthCmplx profile
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
* 	On success, returns TPM_OK
* 	On error, returns tpm_db_gmac_func_set error
*******************************************************************************/
int32_t tpm_db_eth_max_uni_port_set(void)
{
	switch (tpm_db.eth_cmplx_profile) {
	case TPM_PON_WAN_G0_SINGLE_PORT:
	case TPM_PON_WAN_G1_SINGLE_PORT:
	case TPM_PON_WAN_G0_G1_LPBK:
		tpm_db.max_uni_port_nr = TPM_SRC_PORT_UNI_0;
       	break;

	case TPM_PON_G0_WAN_G1_SINGLE_PORT:
	case TPM_PON_G1_WAN_G0_SINGLE_PORT:
	case TPM_PON_WAN_G0_G1_DUAL_LAN:
		tpm_db.max_uni_port_nr = TPM_SRC_PORT_UNI_1;
       	break;

	case TPM_PON_WAN_G0_INT_SWITCH:
	case TPM_PON_WAN_G1_LAN_G0_INT_SWITCH:
	case TPM_PON_WAN_DUAL_MAC_INT_SWITCH:
       case TPM_G0_WAN_G1_INT_SWITCH:
	case TPM_G1_WAN_G0_INT_SWITCH:
	case TPM_PON_G1_WAN_G0_INT_SWITCH:
	case TPM_PON_G0_WAN_G1_INT_SWITCH:
		tpm_db.max_uni_port_nr = TPM_SRC_PORT_UNI_3;
		break;

	case TPM_PON_WAN_G1_MNG_EXT_SWITCH:
		tpm_db.max_uni_port_nr = TPM_SRC_PORT_UNI_6;
       	break;

	case TPM_PON_WAN_DUAL_MAC_EXT_SWITCH:
		tpm_db.max_uni_port_nr = TPM_SRC_PORT_UNI_7;
       	break;

	default:
		return TPM_DB_ERR_REC_EXIST;
	}

	/* increment the counter since UNI enumeration starts @ zero */
	tpm_db.max_uni_port_nr++;

	return (TPM_DB_OK);
}

tpm_src_port_type_t tpm_db_trg_port_uni_any_bmp_get(bool inc_virt_uni)
{
	uint32_t dst_port, trg_port_uni_any_bmp = 0;
	tpm_src_port_type_t src_port;

	for (dst_port = TPM_TRG_UNI_0, src_port = TPM_SRC_PORT_UNI_0;
	     dst_port <= TPM_TRG_UNI_7;
	     dst_port = (dst_port << 1), src_port += 1) {

		/* if port is valid */
		if(tpm_db_eth_port_valid(src_port))
			trg_port_uni_any_bmp |= dst_port;
	}

	if (tpm_db.func_profile.virt_uni_info.enabled != 0 && true == inc_virt_uni)
		trg_port_uni_any_bmp |= TPM_TRG_UNI_VIRT;

	return trg_port_uni_any_bmp;
}

bool tpm_db_gmac_valid(tpm_gmacs_enum_t gmac)
{
	if (((TPM_ENUM_PMAC == gmac) && (TPM_NONE != tpm_db.init_misc.pon_type)) ||
	    ((1 == tpm_db.gmac_port_conf[gmac].valid) && (TPM_GMAC_CON_DISC != tpm_db.gmac_port_conf[gmac].conn)))
	    return true;

	return false;
}

int32_t tpm_db_target_to_gmac(tpm_pnc_trg_t pnc_target, tpm_gmacs_enum_t *gmac)
{
	if (TPM_PNC_TRG_GMAC0 == pnc_target)
		*gmac = TPM_ENUM_GMAC_0;
	else if (TPM_PNC_TRG_GMAC1 == pnc_target)
		*gmac = TPM_ENUM_GMAC_1;
	else if (TPM_PNC_TRG_CPU == pnc_target) {
		TPM_OS_ERROR(TPM_DB_MOD, "target to CPU, no GMAC valid\n");
		return TPM_DB_ERR_INV_INPUT;
	} else
		*gmac = TPM_ENUM_PMAC;

	return TPM_DB_OK;
}

/*******************************************************************************
* tpm_db_to_lan_gmac_get()
*
* DESCRIPTION: returns the GMAC target for traffic to LAN according to target
*		port bitmap
* INPUTS:
* trg_port	target port bitmap
*
* OUTPUTS:
* pnc_target	gmac target
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_to_lan_gmac_get(tpm_trg_port_type_t trg_port, tpm_pnc_trg_t *pnc_target)
{
	uint32_t i;
	tpm_pnc_trg_t gmac_vec = 0;
	bool trg_port_on_switch = false;


	if (NULL == pnc_target)
		return (TPM_DB_ERR_INV_INPUT);

	*pnc_target = TPM_INVALID_PNC_TRG;

	/* for UNI_ANY we search the gmac_func for lan cababilities */
	if (TPM_TRG_PORT_UNI_ANY == trg_port) {
	       for (i = TPM_ENUM_GMAC_0; i < TPM_MAX_GMAC; i++) {
	       		if (!tpm_db_gmac_valid(i))
					continue;
			if (!tpm_db_gmac1_lpbk_en_get()) {
				if (TPM_GMAC_FUNC_LAN == tpm_db.gmac_func[i]      ||
				    TPM_GMAC_FUNC_LAN_UNI == tpm_db.gmac_func[i]) {
					gmac_vec |= (TPM_ENUM_GMAC_0 == i) ? TPM_PNC_TRG_GMAC0 : TPM_PNC_TRG_GMAC1;
					break;/* target only one, no others */
				}
			} else {
				gmac_vec |= TPM_PNC_TRG_GMAC0;
				break;
			}
		}
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "trg_port(%x) gmac_vec(%x)\n", trg_port, gmac_vec);
		*pnc_target = gmac_vec;
		return TPM_DB_OK;
	}

	/* if target is virt uni, search for the gmac that acts as LAN */
	if (TPM_TRG_UNI_VIRT & trg_port) {
		for (i = TPM_ENUM_GMAC_0; i < TPM_MAX_GMAC; i++) {
			if (!tpm_db_gmac_valid(i))
				continue;

			/* need only virt uni lan ports */
			if (TPM_GMAC_FUNC_LAN == tpm_db.gmac_func[i]){
				 gmac_vec |= (TPM_ENUM_GMAC_0 == i) ? TPM_PNC_TRG_GMAC0 : TPM_PNC_TRG_GMAC1;
				 break;
			}
		}
	}

	/* search ethernet ports */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_DB_VALID != tpm_db.eth_ports[i].valid ||
		    TPM_SRC_PORT_ILLEGAL == tpm_db.eth_ports[i].port_src ||
		    TPM_SRC_PORT_WAN == tpm_db.eth_ports[i].port_src)
       		continue;

		if ((1 << tpm_db.eth_ports[i].port_src) & (trg_port >> TPM_TRG_UNI_OFFSET)){
			if (TPM_INTCON_SWITCH == tpm_db.eth_ports[i].int_connect) {
				/* port on switch, set the flag */
				trg_port_on_switch = true;
			} else if (TPM_INTCON_GMAC0 == tpm_db.eth_ports[i].int_connect){
				/* port on GMAC0 */
				gmac_vec |= TPM_PNC_TRG_GMAC0;
			} else if (TPM_INTCON_GMAC1 == tpm_db.eth_ports[i].int_connect){
				/* port on GMAC1 */
				gmac_vec |= TPM_PNC_TRG_GMAC1;
			}
		}
       }

	/* if port connected to switch, search gmac_func for the connected GMAC */
	if (true == trg_port_on_switch) {
		for (i = TPM_ENUM_GMAC_0; i < TPM_MAX_GMAC; i++) {
			if (!tpm_db_gmac_valid(i))
				continue;

			/* need only LAN ports */
			if (TPM_GMAC_FUNC_LAN == tpm_db.gmac_func[i] ||
			    TPM_GMAC_FUNC_LAN_UNI == tpm_db.gmac_func[i]){
				if (TPM_GMAC_CON_SWITCH_4 == tpm_db.gmac_port_conf[i].conn ||
				    TPM_GMAC_CON_SWITCH_5 == tpm_db.gmac_port_conf[i].conn) {
					gmac_vec |= (TPM_ENUM_GMAC_0 == i) ? TPM_PNC_TRG_GMAC0 : TPM_PNC_TRG_GMAC1;
				}
			}
		}
	}

	if (TPM_INVALID_PNC_TRG == gmac_vec)
	      	return TPM_DB_ERR_REC_NOT_EXIST;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "trg_port(%x) gmac_vec(%x)\n", trg_port, gmac_vec);
	*pnc_target = gmac_vec;
	return TPM_DB_OK;
}

/*******************************************************************************
* tpm_db_src_gmac_func_get()
*
* DESCRIPTION: returns the GMAC target for traffic to LAN according to target
*		port bitmap
* INPUTS:
* trg_port	target port bitmap
*
* OUTPUTS:
* pnc_target	gmac target
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_src_gmac_func_get(tpm_src_port_type_t src_port, tpm_db_gmac_func_t *gmac_func)
{
	uint32_t i;

	if (NULL == gmac_func)
		return (TPM_DB_ERR_INV_INPUT);

	*gmac_func = TPM_GMAC_FUNC_NONE;

	if (TPM_SRC_PORT_WAN == src_port){
		*gmac_func = TPM_GMAC_FUNC_WAN;
		return TPM_DB_OK;
	}

	if (TPM_SRC_PORT_UNI_VIRT == src_port){
		if (TPM_FALSE == tpm_db.func_profile.virt_uni_info.enabled)
			return TPM_DB_ERR_REC_NOT_EXIST;

		tpm_db_gmac_func_get(TPM_ENUM_GMAC_1, gmac_func);
		return TPM_DB_OK;
	}

	/* start gmac ports */
	for (i = 0; i < TPM_NUM_GMACS; i++) {
		if (TPM_FALSE == tpm_db.gmac_port_conf[i].valid)
       		continue;

		if (tpm_db.gmac_port_conf[i].port_src == src_port){
			*gmac_func = tpm_db.gmac_func[i];
       		return TPM_DB_OK;
		}
	}

	/* search switch ports */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_DB_VALID != tpm_db.eth_ports[i].valid)
       		continue;

		/* port_src found on switch */
		if (tpm_db.eth_ports[i].port_src == src_port){
       		uint32_t j;

			/* now search gmac table to find connecting port */
			for (j = 0; j < TPM_NUM_GMACS-1; j++) {
				if (TPM_FALSE == tpm_db.gmac_port_conf[j].valid)
       				continue;

				if ((TPM_SRC_PORT_ILLEGAL == tpm_db.gmac_port_conf[j].port_src) &&
				    ((TPM_GMAC_CON_SWITCH_5 == tpm_db.gmac_port_conf[j].conn) ||
				     (TPM_GMAC_CON_SWITCH_4 == tpm_db.gmac_port_conf[j].conn))){
					*gmac_func = (TPM_ENUM_GMAC_0 == i) ? tpm_db.gmac_func[0] : tpm_db.gmac_func[1];
					return TPM_DB_OK;
				}
       		}

       	}
       }

	return TPM_DB_ERR_REC_NOT_EXIST;
}

int32_t tpm_db_src_port_on_switch(tpm_src_port_type_t src_port)
{
	uint32_t i;

	/* switch ports can not act as WAN source port */
	if (TPM_SRC_PORT_WAN == src_port)
		return TPM_FALSE;

	/* start gmac ports */
	for (i = 0; i < TPM_NUM_GMACS; i++) {
		if (TPM_FALSE == tpm_db.gmac_port_conf[i].valid)
       		continue;

		if (tpm_db.gmac_port_conf[i].port_src == src_port)
			return TPM_FALSE;
	}

	/* search switch ports */
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (TPM_DB_VALID != tpm_db.eth_ports[i].valid)
       		continue;

		if (tpm_db.eth_ports[i].port_src == src_port)
			return TPM_TRUE;
	}
	return TPM_FALSE;
}

/*******************************************************************************
* tpm_db_trg_port_switch_port_get()
*
* DESCRIPTION:      Function to get the target port bitmap
*
* INPUTS:
* ext_port         - External Port Identifier, can be any positive number
*
* OUTPUTS:
*
* RETURNS:
* The function returns the target ports bitmap for an external port.
* In case of no matching - it returns empty bitmap 0x0.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_trg_port_switch_port_get(tpm_trg_port_type_t ext_port)
{
	uint32_t portVec = 0;

	if (ext_port & TPM_TRG_PORT_UNI_ANY) {
		ext_port |= (TPM_TRG_UNI_0 | TPM_TRG_UNI_1 | TPM_TRG_UNI_2 | TPM_TRG_UNI_3 |
		 	    TPM_TRG_UNI_4 | TPM_TRG_UNI_5 | TPM_TRG_UNI_6 | TPM_TRG_UNI_7);

		if (tpm_db.func_profile.virt_uni_info.enabled != 0)
			ext_port |= TPM_TRG_UNI_VIRT;
	}
	if (ext_port & TPM_TRG_UNI_0)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_0);

	if (ext_port & TPM_TRG_UNI_1)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_1);

	if (ext_port & TPM_TRG_UNI_2)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_2);

	if (ext_port & TPM_TRG_UNI_3)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_3);

	if (ext_port & TPM_TRG_UNI_4)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_4);

	if (ext_port & TPM_TRG_UNI_5)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_5);

	if (ext_port & TPM_TRG_UNI_6)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_6);

	if (ext_port & TPM_TRG_UNI_7)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_7);

	if (ext_port & TPM_TRG_UNI_VIRT)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_UNI_VIRT);

	if (ext_port & TPM_TRG_PORT_CPU)
		portVec |= 1 << tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_WAN);

	return (portVec);
}

/*******************************************************************************
* tpm_db_eth_port_conf_set()
*
* DESCRIPTION:      Function to set an ethernet physical and internal configuration in the DB
*
* INPUTS:
* ext_port         - External Port Identifier, can be any positive number
* other inputs     - See explanation in tpm_db_eth_port_conf_t structure
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_eth_port_conf_set(tpm_init_eth_port_conf_t *eth_port_conf)
{
	uint32_t i;
	int32_t free_entry = -1;

	if (NULL == eth_port_conf)
		return (TPM_DB_ERR_INV_INPUT);

	TPM_OS_DEBUG(TPM_DB_MOD, "src_port (%d)\n", eth_port_conf->port_src);
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		/* Check entry does not exist */
       	if ((tpm_db.eth_ports[i].port_src == eth_port_conf->port_src)
		    && (tpm_db.eth_ports[i].valid == TPM_DB_VALID))
			return (TPM_DB_ERR_REC_EXIST);

		/* Find first invalid entry */
		if ((free_entry == -1) && (tpm_db.eth_ports[i].valid == TPM_DB_INVALID))
			free_entry = i;
	}
	/* No free Entry */
	if (free_entry == -1)
		return (TPM_DB_ERR_DB_TBL_FULL);

	/* Set entry */
	tpm_db.eth_ports[free_entry].port_src = eth_port_conf->port_src;
	tpm_db.eth_ports[free_entry].int_connect = eth_port_conf->int_connect;
	tpm_db.eth_ports[free_entry].chip_connect = eth_port_conf->chip_connect;
	tpm_db.eth_ports[free_entry].switch_port = eth_port_conf->switch_port;
	tpm_db.eth_ports[free_entry].valid = TPM_DB_VALID;

	return (TPM_DB_OK);

}

int32_t tpm_db_gmac_conn_conf_set(tpm_init_gmac_conn_conf_t *gmac_port_conf, uint32_t arr_size)
{
	uint32_t i;

	if (NULL == gmac_port_conf)
		return (TPM_DB_ERR_INV_INPUT);

	for (i = 0; i < arr_size; i++) {
		TPM_OS_DEBUG(TPM_DB_MOD, "gmac src_port (%d)\n", gmac_port_conf[i].port_src);

		/* Set entry */
		tpm_db.gmac_port_conf[i].port_src = gmac_port_conf[i].port_src;
		tpm_db.gmac_port_conf[i].conn = gmac_port_conf[i].conn;
		tpm_db.gmac_port_conf[i].valid = gmac_port_conf[i].valid;
	}
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_conn_conf_get()
*
* DESCRIPTION:      Function to get GMAC connection information from DB
*
* INPUTS:
* gmac                   - GMAC port
*
* OUTPUTS:
* gmac_port_conf         - connection info
*
* RETURNS:
* On success, the function returns TPM_DB_OK.
*
*******************************************************************************/
int32_t tpm_db_gmac_conn_conf_get(tpm_gmacs_enum_t gmac, tpm_init_gmac_conn_conf_t *gmac_port_conf)
{
	if (NULL == gmac_port_conf)
		return (TPM_DB_ERR_INV_INPUT);

	memcpy(gmac_port_conf, &tpm_db.gmac_port_conf[gmac], sizeof(tpm_init_gmac_conn_conf_t));

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_tcont_llid_set()
*
* DESCRIPTION:      Function to set the GMAC connection and T-CONTs  in the DB
*
* INPUTS:
* num_tcont_llid   - Number of T-CONTs/LLIDs
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_tcont_llid_set(uint32_t num_tcont_llid)
{
	tpm_db.num_valid_tcont_llid = num_tcont_llid;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_conn_get()
*
* DESCRIPTION:      Function to get the Connection of an Ethernet GMAC
*
* INPUTS:
* gmac             - GMAC number
* OUTPUTS:
* gmac_con         - GMAC internal Physical connection (see tpm_db_gmac_conn_t)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_conn_get(tpm_gmacs_enum_t gmac, tpm_db_gmac_conn_t *gmac_con)
{
	if (gmac == TPM_ENUM_GMAC_0)
		*gmac_con = tpm_db.gmac_port_conf[0].conn;
	else if (gmac == TPM_ENUM_GMAC_1)
		*gmac_con = tpm_db.gmac_port_conf[1].conn;
	else
		return (TPM_DB_ERR_INV_INPUT);

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_mh_en_conf_set()
*
* DESCRIPTION:      Function to set the GMAC connection and T-CONTs  in the DB
*
* INPUTS:
* gmac             - GMAC
* mh_en            - MH enable TRUE/FALSE
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_mh_en_conf_set(tpm_gmacs_enum_t gmac, uint32_t mh_en)
{

	tpm_db.gmac_mh_en[gmac] = mh_en;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_mh_en_conf_get()
*
* DESCRIPTION:      Function to set the GMAC connection and T-CONTs  in the DB
*
* INPUTS:
* gmac             - GMAC
*
* OUTPUTS:
* mh_en            - MH enable TRUE/FALSE
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_mh_en_conf_get(tpm_gmacs_enum_t gmac, uint32_t *mh_en)
{

	*mh_en = tpm_db.gmac_mh_en[gmac];

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_bm_bufs_conf_set()
*
* DESCRIPTION:      Function to set the GMAC number of Buffer Mngmt large_bufers, short_buffers
*
* INPUTS:
* gmac              - GMAC
* large_pkt_buffers - number of buffers in the GMAC long packets pool
* small_pkt_buffers - number of buffers in the GMAC short packets pool
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_bm_bufs_conf_set(tpm_gmacs_enum_t gmac, uint32_t large_pkt_buffers, uint32_t small_pkt_buffers)
{
	tpm_db.gmac_bp_bufs[gmac].valid = TPM_DB_VALID;
	tpm_db.gmac_bp_bufs[gmac].large_pkt_buffers = large_pkt_buffers;
	tpm_db.gmac_bp_bufs[gmac].small_pkt_buffers = small_pkt_buffers;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_bm_bufs_conf_get()
*
* DESCRIPTION:      Function to set the GMAC number of Buffer Mngmt large_bufers, short_buffers
*
* INPUTS:
* gmac              - GMAC
* valid             - Is the entry valid in the db
* large_pkt_buffers - number of buffers in the GMAC long packets pool
* small_pkt_buffers - number of buffers in the GMAC short packets pool
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_bm_bufs_conf_get(tpm_gmacs_enum_t gmac,
				     uint32_t *valid, uint32_t *large_pkt_buffers, uint32_t *small_pkt_buffers)
{
	if ((valid == NULL) || (gmac >= TPM_MAX_NUM_GMACS))
		return (TPM_DB_ERR_INV_INPUT);
	*valid = (tpm_db.gmac_bp_bufs[gmac].valid == TPM_DB_VALID ? 1 : 0);

	if (large_pkt_buffers != NULL)
		*large_pkt_buffers = tpm_db.gmac_bp_bufs[gmac].large_pkt_buffers;

	if (small_pkt_buffers != NULL)
		*small_pkt_buffers = tpm_db.gmac_bp_bufs[gmac].small_pkt_buffers;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_intport_int_con_get()
*
* DESCRIPTION:      Function returns internal connectivity of an internal port num
*
* INPUTS:
* int_port_num     - Internal Port Identifier, UNI#0..UNI#4 or WAN Port
*
* OUTPUTS:
* int_con          - Port internal L2 Connection (G0, G1, Amber switch)
* switch_port      - Amber Switch Port number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_intport_int_con_get(tpm_src_port_type_t int_port_num,
				   tpm_db_int_conn_t	*int_con,
				   uint32_t		*switch_port)
{
	int32_t ret_code;
	tpm_db_chip_conn_t dummy_chip_con;
	tpm_db_int_conn_t l_int_conn;
	uint32_t l_switch_port;

	if (int_port_num == TPM_SRC_PORT_UNI_VIRT) {
		*int_con = TPM_INTCON_SWITCH;
		*switch_port = tpm_db.func_profile.virt_uni_info.switch_port;
		return (TPM_DB_OK);
	}

	/* Get External Port Config */
	ret_code = tpm_db_eth_port_conf_get(int_port_num,
					    &dummy_chip_con,
					    &l_int_conn,
					    &l_switch_port);
	IF_ERROR(ret_code);

	/* Return data */
	*int_con = l_int_conn;
	*switch_port = l_switch_port;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_num_tcont_llid_get()
*
* DESCRIPTION:      Function to get the number of T-CONTs/LLIDs
*
* INPUTS:
* OUTPUTS:
* num_tcont_llid   - Number of GPON TCONTs or EPON LLIDs
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
*
*******************************************************************************/
int32_t tpm_db_num_tcont_llid_get(uint32_t *num_tcont_llid)
{
	*num_tcont_llid = tpm_db.num_valid_tcont_llid;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_tx_q_conf_set()
*
* DESCRIPTION:      Function to set the parameters of a Packet Processor Component Tx queue
*
* INPUTS:
* tx_mod           - Tx module (G0, G1, or TCONT/LLID0-7)
* OUTPUTS:
* gmac_con         - GMAC0 connection
* queue_num        - Queue number in this Tx
* sched_method     - The Tx Scheduler this queue is added to (the WRR or the Strict scheduler)
* queue_owner      - The single hardware owner, that is feeding this queue.
*                    Can be the CPU, or one of the PAcket Processor Rx components.
* owner_queue_num  - The relative queue number of the owner of this queue.
* queue_weight     - Weight, if the queue is added to the WRR Scheduler.
* queue_size       -
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_tx_q_conf_set(tpm_db_tx_mod_t tx_mod,
				  uint32_t queue_num,
				  tpm_db_sched_t sched_method,
				  tpm_db_txq_owner_t queue_owner,
				  uint32_t owner_queue_num, uint32_t queue_size, uint32_t queue_weight)
{
	tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].queue_owner = queue_owner;
	tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].owner_queue_num = owner_queue_num;
	tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].sched_method = sched_method;
	tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].queue_size = queue_size;
	tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].queue_weight = queue_weight;
	tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].valid = TPM_DB_VALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_tx_q_conf_get()
*
* DESCRIPTION:      Function to set the parameters of a Packet Processor Tx Component
*
* INPUTS:
* tx_mod           - Tx module (G0, G1, or TCONT/LLID0-7)
* queue_num        - Queue number in this Tx
*
* OUTPUTS:
* valid             - Queue exists
* sched_method     - The Tx Scheduler this queue is added to (the WRR or the Strict scheduler)
* queue_owner      - The single hardware owner, that is feeding this queue.
*                    Can be the CPU, or one of the PAcket Processor Rx components.
* owner_queue_num  - The relative queue number of the owner of this queue.
* queue_weight     - Weight, if the queue is added to the WRR Scheduler.
* queue_size       -
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_tx_q_conf_get(tpm_db_tx_mod_t tx_mod,
				  uint32_t queue_num,
				  uint32_t *valid,
				  tpm_db_sched_t *sched_method,
				  tpm_db_txq_owner_t *queue_owner,
				  uint32_t *owner_queue_num, uint32_t *queue_size, uint32_t *queue_weight)
{
	if (valid == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if (tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].valid == TPM_DB_VALID) {
		*valid = TPM_TRUE;
		if (sched_method != NULL)
			*sched_method = tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].sched_method;
		if (queue_owner != NULL)
			*queue_owner = tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].queue_owner;
		if (owner_queue_num != NULL)
			*owner_queue_num = tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].owner_queue_num;
		if (queue_size != NULL)
			*queue_size = tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].queue_size;
		if (queue_weight != NULL)
			*queue_weight = tpm_db.gmac_tx[tx_mod].tx_queue[queue_num].queue_weight;
	} else {
		*valid = TPM_FALSE;
	}

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_rx_q_conf_set()
*
* DESCRIPTION:      Function to set the parameters of a Packet Processor Tx Component
*
* INPUTS:
* gmac             - GMAC
* OUTPUTS:
* queue_num        - Queue number in this Tx
* queue_size       -
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_rx_q_conf_set(tpm_gmacs_enum_t gmac, uint32_t queue_num, uint32_t queue_size)
{
	if ((gmac >= TPM_MAX_NUM_GMACS) || (queue_num >= TPM_MAX_NUM_RX_QUEUE))
		return (TPM_DB_ERR_INV_INPUT);

	tpm_db.gmac_rx[gmac].rx_queue[queue_num].queue_size = queue_size;
	tpm_db.gmac_rx[gmac].rx_queue[queue_num].valid = TPM_DB_VALID;

	return (TPM_DB_OK);
}

int32_t tpm_db_gmac_rx_q_conf_get(tpm_gmacs_enum_t gmac, uint32_t queue_num, uint32_t *valid, uint32_t *queue_size)
{

	if ((valid == NULL) || (gmac >= TPM_MAX_NUM_GMACS) || (queue_num >= TPM_MAX_NUM_RX_QUEUE))
		return (TPM_DB_ERR_INV_INPUT);

	if (tpm_db.gmac_rx[gmac].rx_queue[queue_num].valid == TPM_DB_VALID) {
		*valid = TPM_TRUE;
		if (queue_size != NULL)
			*queue_size = tpm_db.gmac_rx[gmac].rx_queue[queue_num].queue_size;
	} else
		*valid = TPM_FALSE;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_tx_val_set()
*
* DESCRIPTION:      Function to set the parameters of a Tx module
*
* INPUTS:
* tx_mod           - Tx Module to validate
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_tx_val_set(tpm_db_tx_mod_t tx_mod)
{
	tpm_db.gmac_tx[tx_mod].valid = TPM_DB_VALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_rx_val_set()
*
* DESCRIPTION:      Function to set the parameters of a Tx module
*
* INPUTS:
* gmac           - GMAC to set active
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_rx_val_set(tpm_gmacs_enum_t gmac)
{
	tpm_db.gmac_rx[gmac].valid = TPM_DB_VALID;

	return (TPM_DB_OK);
}

uint32_t tpm_db_gmac_rx_val_get(tpm_gmacs_enum_t gmac)
{
	if (tpm_db.gmac_rx[gmac].valid == TPM_DB_VALID)
		return (TPM_TRUE);
	else
		return (TPM_FALSE);
}

/*******************************************************************************
* tpm_db_gmac_func_get()
*
* DESCRIPTION:      Check the Function of a GMAC (LAN, WAN or no function)
*
* INPUTS:
* gmac              - GMAC to check
* OUTPUTS:
* gfunc             - Functionality the GMAC is performing (None, LAN or WAN)
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_func_get(tpm_gmacs_enum_t gmac, tpm_db_gmac_func_t *gfunc)
{
	if (gmac > TPM_MAX_GMAC)
		return (TPM_DB_ERR_INV_INPUT);

	*gfunc = tpm_db.gmac_func[gmac];

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_func_set()
*
* DESCRIPTION:      Sets the Function of a GMAC (LAN, WAN or no function)
*
* INPUTS:
* gmac              - GMAC to set
* gfunc             - Functionality the GMAC is performing (None, LAN or WAN)
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_gmac_func_set(tpm_gmacs_enum_t gmac, tpm_db_gmac_func_t gfunc)
{
	if (gmac > TPM_MAX_GMAC)
		return (TPM_DB_ERR_INV_INPUT);

	tpm_db.gmac_func[gmac] = gfunc;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_tx_val_get()
*
* DESCRIPTION:      Function check the validity of a Tx module
*
* INPUTS:
* tx_mod           - Tx module to check
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
uint32_t tpm_db_gmac_tx_val_get(tpm_db_tx_mod_t tx_mod)
{
	if (tpm_db.gmac_tx[tx_mod].valid == TPM_DB_VALID)
		return (TPM_TRUE);
	else
		return (TPM_FALSE);
}

/*******************************************************************************
* tpm_db_gmac_lpk_queue_get()
*
* DESCRIPTION:      Function to get the queue id used to do gmac loopback on MC, min queue id for data traffic,
*                   max queue id for MAC learning
* INPUTS:
* queue_type     - queue type for GMAC1 loopback, for data traffic or for MAC learning
* OUTPUTS:
* gmac           - gmac on which loopback is done
* queue_idx      - queue index which do loopback
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*******************************************************************************/
uint32_t tpm_db_gmac_lpk_queue_get(tpm_gmacs_enum_t *gmac,
				   uint32_t *queue_idx,
				   tpm_db_gmac1_lpk_queue_type_t queue_type)
{
	uint32_t queue_id, q_valid, gmac_id;
	tpm_db_txq_owner_t queue_owner, lpk_queue_owner = TPM_Q_OWNER_GMAC0;
	tpm_db_gmac_func_t lpk_gmac_func;
	tpm_db_tx_mod_t tx_mod;
	uint32_t q_expected = TPM_MAX_NUM_TX_QUEUE;

	/*check input parameters*/
	if (NULL == gmac || NULL == queue_idx) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input \n");
		return TPM_DB_ERR_INV_INPUT;
	}

	/*check func profile*/
	if (!tpm_db_gmac1_lpbk_en_get()) {
		TPM_OS_ERROR(TPM_DB_MOD, "GMAC1 loopback has not been enabled!\n");
		return TPM_FALSE;
	}

	/*Find the GMAC used to do loopback*/
	for (gmac_id = TPM_ENUM_GMAC_0; gmac_id < TPM_MAX_NUM_GMACS; gmac_id ++) {
		if (TPM_DB_OK == tpm_db_gmac_func_get(gmac_id, &lpk_gmac_func)) {
			if (TPM_GMAC_FUNC_LAN == lpk_gmac_func)
				break;
		} else {
			continue;
		}
	}

	if (gmac_id == TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Loopback GMAC get failed \n");
		return TPM_FALSE;
	} else {
		tx_mod = (tpm_db_tx_mod_t)gmac_id;
		lpk_queue_owner = TPM_Q_OWNER_GMAC0;
	}

	/* get the Tx queue of GMAC1, if more than 1 queue belong to gmac0, lowest index is selected */
	for (queue_id = 0; queue_id < TPM_MAX_NUM_TX_QUEUE; queue_id++) {
		if (TPM_DB_OK != tpm_db_gmac_tx_q_conf_get(tx_mod, queue_id, &q_valid, NULL,
							   &queue_owner, NULL, NULL, NULL))
			continue;
		if (q_valid == TPM_TRUE && queue_owner == lpk_queue_owner) {
			q_expected = queue_id;
			if (TPM_GMAC1_QUEUE_DATA_TRAFFIC == queue_type)
				break;
			else if (TPM_GMAC1_QUEUE_MAC_LEARN_TRAFFIC == queue_type)
				continue;
		}
	}
	if (q_expected >= TPM_MAX_NUM_TX_QUEUE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid Tx queue %d of GMAC1 assigned to GMAC0 \n", q_expected);
		return ERR_SW_TM_QUEUE_INVALID;
	}

	*gmac = gmac_id;
	*queue_idx = q_expected;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_rng_create()
*
* DESCRIPTION:      Function to create a range in the DB
*
* INPUTS:
* pnc_range_conf      - Pointer to all config information needed for an entry
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_create(tpm_db_pnc_range_conf_t *pnc_range_conf)
{
	uint32_t i;
	int32_t free_entry = -1;

	/* Check NULL Pointer */
	if (pnc_range_conf == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	/* Check range_id is not already in DB, and that DB is not full */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID && PNC_RANGE_CONF(i).range_id == pnc_range_conf->range_id)
			return (TPM_DB_ERR_RANGE_EXIST);

		if ((tpm_db.pnc_range[i].valid == TPM_DB_INVALID) && (free_entry == -1))
			free_entry = i;
	}
	if (free_entry == -1)
		return (TPM_DB_ERR_DB_TBL_FULL);

	/* Copy Config Structure into DB */
	memcpy(&(tpm_db.pnc_range[free_entry].pnc_range_conf), pnc_range_conf, sizeof(tpm_db_pnc_range_conf_t));

	/* Init Operational Data */
	PNC_RANGE_OPER(free_entry).free_entries = (pnc_range_conf->api_end - pnc_range_conf->api_start + 1);
	PNC_RANGE_OPER(free_entry).num_resets = 0;

	tpm_db.pnc_range[free_entry].valid = TPM_DB_VALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_rng_delete()
*
* DESCRIPTION:      Initializes and invalidates PnC range
*
* INPUTS:
* pnc_range         - The id of the pnc range that is deleted
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_delete(tpm_pnc_ranges_t pnc_range)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		/* Reset Operational values */
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID &&
		    tpm_db.pnc_range[i].pnc_range_conf.range_id == pnc_range) {
			memset(&(tpm_db.pnc_range[i]), 0, sizeof(tpm_pnc_ranges_t));
			tpm_db.pnc_range[i].valid = TPM_DB_INVALID;
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_reset()
*
* DESCRIPTION:      Resets PnC range, all entries are returned to free
*
* INPUTS:
* pnc_range         - The id of the pnc range that is reset
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_reset(tpm_pnc_ranges_t pnc_range)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		/* reach the range to be reset */
		if (tpm_db.pnc_range[i].pnc_range_conf.range_id == pnc_range) {
			/* Reset Operational values */
			if (tpm_db.pnc_range[i].valid == TPM_DB_VALID) {
				/* both hardcoded and non-hardcoded entries will behave the same from free_entries point of view */
				PNC_RANGE_OPER(i).free_entries =
					PNC_RANGE_CONF(i).api_end - PNC_RANGE_CONF(i).api_start + 1;
				(PNC_RANGE_OPER(i).num_resets)++;
				return (TPM_DB_OK);
			}
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_free_ent_inc()
*
* DESCRIPTION:      Increases number of free entries for a PnC range
*
* INPUTS:
* pnc_range         - The id of the pnc range that it's free entries is increased
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_free_ent_inc(tpm_pnc_ranges_t pnc_range)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID
		    && tpm_db.pnc_range[i].pnc_range_conf.range_id == pnc_range) {
			/* Check number of free entries is not too big */
			if (PNC_RANGE_OPER(i).free_entries
			    < (PNC_RANGE_CONF(i).api_end - PNC_RANGE_CONF(i).api_start + 1)) {
				(PNC_RANGE_OPER(i).free_entries)++;
				return (TPM_DB_OK);
			} else
				return (TPM_DB_ERR_DB_INCONSISTENCY);
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_free_ent_dec()
*
* DESCRIPTION:      Decreases number of free entries for a PnC range
*
* INPUTS:
* pnc_range         - The id of the pnc range that it's free entries is decreased
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_free_ent_dec(tpm_pnc_ranges_t pnc_range)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID && PNC_RANGE_CONF(i).range_id == pnc_range) {
			TPM_OS_DEBUG(TPM_DB_MOD, "range_id(%d), free_ent(%d)\n", pnc_range,
				     PNC_RANGE_OPER(i).free_entries);
			/* Check number of free entries is not zero */
			if (PNC_RANGE_OPER(i).free_entries > 0) {
				(PNC_RANGE_OPER(i).free_entries)--;
				return (TPM_DB_OK);
			} else {
				TPM_OS_ERROR(TPM_DB_MOD, "no free entries \n");
				return (TPM_DB_ERR_DB_INCONSISTENCY);
			}
		}
	}
	/* pnc_range not found */
	TPM_OS_ERROR(TPM_DB_MOD, "range_id(%d) not found\n", pnc_range);
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_get_lu_conf()
*
* DESCRIPTION:      Get PnC least used config for specific PnC range
*
* INPUTS:
* range_id         - The PnC range id
*
* OUTPUTS:
* lu_conf          - Least used configuration for PnC range

*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_get_lu_conf(tpm_pnc_ranges_t range_id, tpm_api_lu_conf_t *lu_conf)
{
	tpm_pnc_ranges_t l_range_idx;

	/* Check PnC range */
	if ((range_id < 0) || (range_id > (TPM_MAX_NUM_RANGES - 1))) {
		TPM_OS_ERROR(TPM_DB_MOD, "Error, range_id(%d) is illegal \n", range_id);
		return (TPM_DB_ERR_INV_INPUT);
	}

	/* Check input param */
	if (lu_conf == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Error, lu_conf is NULL \n");
		return (TPM_DB_ERR_INV_INPUT);
	}

	/* Get the config from TPM db */
	for (l_range_idx = 0; l_range_idx < TPM_MAX_RANGE; l_range_idx++) {
		if (tpm_db.pnc_range[l_range_idx].valid == TPM_DB_VALID
				&& (PNC_RANGE_CONF(l_range_idx).range_id == range_id)) {
			lu_conf->cntr_grp    = tpm_db.pnc_range[l_range_idx].pnc_range_conf.cntr_grp;
			lu_conf->lu_mask     = tpm_db.pnc_range[l_range_idx].pnc_range_conf.lu_mask;
			break;
		}
	}

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_rng_api_start_inc()
*
* DESCRIPTION:      Increases number of free entries for a PnC range
*
* INPUTS:
* pnc_range         - The id of the pnc range that it's free entries is increased
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_api_start_inc(tpm_pnc_ranges_t pnc_range)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID
		    && tpm_db.pnc_range[i].pnc_range_conf.range_id == pnc_range) {
			/* Check number of free entries is not too big */
			if (PNC_RANGE_OPER(i).free_entries >= 1) {
				(PNC_RANGE_CONF(i).api_start)++;
				(PNC_RANGE_OPER(i).free_entries)--;
				return (TPM_DB_OK);
			} else
				return (TPM_DB_ERR_DB_INCONSISTENCY);
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_api_end_dec()
*
* DESCRIPTION:      Decrease number of api_end for a PnC range
*                   The reason might be addition of an entry at the end of the range.
*
* INPUTS:
* pnc_range         - The id of the pnc range that it's free entries is increased
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_api_end_dec(tpm_pnc_ranges_t pnc_range)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID
		    && tpm_db.pnc_range[i].pnc_range_conf.range_id == pnc_range) {
			/* Check number of free entries is not too big */
			if (PNC_RANGE_OPER(i).free_entries >= 1) {
				(PNC_RANGE_CONF(i).api_end)--;
				(PNC_RANGE_OPER(i).free_entries)--;
				return (TPM_DB_OK);
			} else
				return (TPM_DB_ERR_DB_INCONSISTENCY);
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

int32_t tpm_db_api_section_get_from_api_type(tpm_api_type_t api_type, tpm_api_sections_t *api_section)
{
	int32_t error_code = TPM_DB_OK;

	/*TODO - replace this w/ conf structure */

	switch (api_type) {
	case TPM_API_MAC_LEARN:
		*api_section = TPM_PNC_MAC_LEARN_ACL;
		break;
	case TPM_API_CPU_LOOPBACK:
		*api_section = TPM_CPU_LOOPBACK_ACL;
		break;
	case TPM_API_DS_LOAD_BALANCE:
		*api_section = TPM_DS_LOAD_BALANCE_ACL;
		break;
	case TPM_API_L2_PRIM:
		*api_section = TPM_L2_PRIM_ACL;
		break;
	case TPM_API_L3_TYPE:
		*api_section = TPM_L3_TYPE_ACL;
		break;
	case TPM_API_IPV4:
		*api_section = TPM_IPV4_ACL;
		break;
	case TPM_API_IPV4_MC:
		*api_section = TPM_IPV4_MC;
		break;
	case TPM_API_IPV6_GEN:
		*api_section = TPM_IPV6_GEN_ACL;
		break;
	case TPM_API_IPV6_MC:
		*api_section = TPM_IPV6_MC_ACL;
		break;
	case TPM_API_IPV6_DIP:
		*api_section = TPM_IPV6_DIP_ACL;
		break;
	case TPM_API_IPV6_NH:
		*api_section = TPM_IPV6_NH_ACL;
		break;
	case TPM_API_IPV6_L4:
		*api_section = TPM_L4_ACL;
		break;
	case TPM_API_CNM:
		*api_section = TPM_CNM_MAIN_ACL;
		break;
	default:
		error_code = TPM_DB_ERR_REC_NOT_EXIST;
	}

	return (error_code);
}


int32_t tpm_db_api_type_get_from_api_section(tpm_api_sections_t api_section, tpm_api_type_t *api_type)
{
	int32_t error_code = TPM_DB_OK;

	/*TODO - replace this w/ conf structure */

	switch (api_section) {
	case TPM_PNC_MAC_LEARN_ACL:
		*api_type = TPM_API_MAC_LEARN;
		break;
	case TPM_CPU_LOOPBACK_ACL:
		*api_type = TPM_API_CPU_LOOPBACK;
		break;
	case TPM_DS_LOAD_BALANCE_ACL:
		*api_type = TPM_API_DS_LOAD_BALANCE;
		break;
	case TPM_L2_PRIM_ACL:
		*api_type = TPM_API_L2_PRIM;
		break;
	case TPM_L3_TYPE_ACL:
		*api_type = TPM_API_L3_TYPE;
		break;
	case TPM_IPV4_ACL:
		*api_type = TPM_API_IPV4;
		break;
	case TPM_IPV4_MC:
		*api_type = TPM_API_IPV4_MC;
		break;
	case TPM_IPV6_GEN_ACL:
		*api_type = TPM_API_IPV6_GEN;
		break;
	case TPM_IPV6_DIP_ACL:
		*api_type = TPM_API_IPV6_DIP;
		break;
	case TPM_IPV6_NH_ACL:
		*api_type = TPM_API_IPV6_NH;
		break;
	case TPM_L4_ACL:
		*api_type = TPM_API_IPV6_L4;
		break;
	default:
		error_code = TPM_DB_ERR_REC_NOT_EXIST;
	}

	return (error_code);
}

/*******************************************************************************
* tpm_db_pnc_rng_get()
*
* DESCRIPTION:      Returns DB info of PnC range
*
* INPUTS:
* pnc_range         - The id of the pnc range to get data for

* OUTPUTS:
* range_data       - Returned DB structure of returned entry.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_get(tpm_pnc_ranges_t pnc_range, tpm_db_pnc_range_t *range_data)
{
	uint32_t i;

	if ((pnc_range < 0) || (pnc_range > (TPM_MAX_NUM_RANGES - 1)))
		return (TPM_DB_ERR_INV_INPUT);

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID && PNC_RANGE_CONF(i).range_id == pnc_range) {
			memcpy(range_data, &(tpm_db.pnc_range[i]), sizeof(tpm_db_pnc_range_t));
			return (TPM_DB_OK);
		}
	}

	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_get_range_start_end()
*
* DESCRIPTION:      Returns DB info of PnC range
*
* INPUTS:
* pnc_range         - The id of the pnc range to get data for

* OUTPUTS:
* range_start      - Returned the start of specific PnC range.
* range_end        - Returned the end of specific PnC range.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_get_range_start_end(tpm_pnc_ranges_t pnc_range, uint32_t *range_start, uint32_t *range_end)
{
	uint32_t i;

	if ((pnc_range < 0) || (pnc_range > (TPM_MAX_NUM_RANGES - 1)))
		return(TPM_DB_ERR_INV_INPUT);

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if ((tpm_db.pnc_range[i].valid == TPM_DB_VALID)
				&& (PNC_RANGE_CONF(i).range_id == pnc_range)) {
			*range_start = PNC_RANGE_CONF(i).range_start + PNC_RANGE_CONF(i).api_start;
			*range_end   = PNC_RANGE_CONF(i).range_end;
			return(TPM_DB_OK);
		}
	}

	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_tbl_val_get_next()
*
* DESCRIPTION:      Returns DB info of PnC range
*
* INPUTS:
* cur_ind          - The current index (handle) to the pnc_range table. Function returns the next valid
*                    pnc range  entry > cur_index. cur_index < 0 indicates to return first valid entry in table,
*                    valid next_ind values are 0 or higher
* OUTPUTS:
* next_ind         - See cur_ind. If end of table was reached before valid entry, (-1) is returned in next_ind
* range_data       - Returned DB structure of returned entry. When entry is invalid, range_data filled with zero.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_tbl_val_get_next(int32_t cur_ind, int32_t *next_ind, tpm_db_pnc_range_t *range_data)
{
	int32_t i;

	/* Initialize first next_ind */
	if (cur_ind < 0)
		i = 0;
	else
		i = cur_ind + 1;

	/* Check PnC range exists in DB */
	for (; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID) {
			/* Return Data Structure */
			memcpy(range_data, &(tpm_db.pnc_range[i]), sizeof(tpm_db_pnc_range_t));
			*next_ind = i;
			return (TPM_DB_OK);
		}
	}
	*next_ind = -1;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_pnc_rng_val_get_next()
*
* DESCRIPTION:      Returns DB info of the next valid PnC range.
*                   Similar to "tpm_db_pnc_rng_tbl_val_get_next", but the handle is the range_id, not the table_index.
*
* INPUTS:
* cur_range        - The current pnc_range  in  the pnc_range table. Function returns the next valid
*                    pnc_range  > cur_range. cur_range < 0 indicates to return first valid entry in table,
*                    valid next_ind values are 0 or higher
* OUTPUTS:
* next_range       - See cur_range. If end of table was reached before valid entry, (-1) is returned in next_range
* range_data       - Returned DB structure of returned entry. When entry is invalid, range_data filled with zero.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_val_get_next(tpm_pnc_ranges_t cur_range,
				    tpm_pnc_ranges_t *next_range, tpm_db_pnc_range_t *range_data)
{
	uint32_t i;
	uint32_t l_min_index = 0;	/* oren - add init value, need to check */
	tpm_pnc_ranges_t l_min_range = TPM_MAX_RANGE + 1;

	/* Check  smallest valid next_PnC range in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		/*printf("i(%d) valid(%x) range_id(%d), cur_range(%d) l_min_range(%d)\n",
		   i,tpm_db.pnc_range[i].valid, PNC_RANGE_CONF(i).range_id, cur_range,l_min_range); */
		if ((tpm_db.pnc_range[i].valid == TPM_DB_VALID) && (PNC_RANGE_CONF(i).range_id > cur_range)
		    && (PNC_RANGE_CONF(i).range_id < l_min_range)) {
			l_min_range = PNC_RANGE_CONF(i).range_id;
			l_min_index = i;
		}
	}
	if (l_min_range <= TPM_MAX_RANGE) {
		/* Return Data Structure */
		memcpy(range_data, &(tpm_db.pnc_range[l_min_index]), sizeof(tpm_db_pnc_range_t));
		*next_range = l_min_range;
		return (TPM_DB_OK);
	}

	/* No valid next range found */
	*next_range = -1;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_rng_conf_get()
*
* DESCRIPTION:      Returns DB info of PnC range
*
* INPUTS:
* range_id         - Range number to retrieve configuration for

* OUTPUTS:
* range_conf       - Structure for pnc_range Configuration Info
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_conf_get(tpm_pnc_ranges_t range_id, tpm_db_pnc_range_conf_t *range_conf)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID
		    && tpm_db.pnc_range[i].pnc_range_conf.range_id == range_id) {
			/* Return Data Structure */
			memcpy(range_conf, &(tpm_db.pnc_range[i].pnc_range_conf), sizeof(tpm_db_pnc_range_conf_t));
			return (TPM_DB_OK);
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_oper_get()
*
* DESCRIPTION:      Returns DB info of PnC range
*
* INPUTS:
* range_id         - Range number to retrieve configuration for

* OUTPUTS:
* range_oper       - Structure for pnc_range Operational Info
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_oper_get(tpm_pnc_ranges_t range_id, tpm_db_pnc_range_oper_t *range_oper)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID
		    && tpm_db.pnc_range[i].pnc_range_conf.range_id == range_id) {
			/* Return Data Structure */
			memcpy(range_oper, &(tpm_db.pnc_range[i].pnc_range_oper), sizeof(tpm_db_pnc_range_oper_t));
			return (TPM_DB_OK);
		}
	}
	/* pnc_range not found */
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_pnc_rng_free_ent_get()
*
* DESCRIPTION:      Returns number of free entries of PnC range
*
* INPUTS:
* range_id         - Range number to retrieve number of free entries

* OUTPUTS:
* free_entries      - Number of free entries in Table
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_rng_free_ent_get(tpm_pnc_ranges_t range_id, uint32_t *free_entries)
{
	uint32_t i;

	/* Check PnC range exists in DB */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++) {
		if (tpm_db.pnc_range[i].valid == TPM_DB_VALID
		    && tpm_db.pnc_range[i].pnc_range_conf.range_id == range_id) {
			/* Return Free Entries */
			*free_entries = PNC_RANGE_OPER(i).free_entries;
			return (TPM_DB_OK);
		}
	}
	/* pnc_range not found */
	*free_entries = 0;
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_owner_set()
*
* DESCRIPTION:      Creates an owner_id in the DB.
*
* INPUTS:
* owner_id         - Id of new owner

* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_owner_set(uint32_t owner_id)
{
	uint32_t i;
	int32_t free_ind = -1;

	/* Check owner was not already set */
	for (i = 0; i < TPM_MAX_API_TYPES; i++) {
		if ((tpm_db.owners[i].valid == TPM_DB_VALID) && (tpm_db.owners[i].owner_id == owner_id))
			return (TPM_DB_ERR_REC_EXIST);
		if ((free_ind == -1) && (tpm_db.owners[i].valid == TPM_DB_INVALID))
			free_ind = i;
	}
	if (free_ind == -1)
		return (TPM_DB_ERR_DB_TBL_FULL);

	/* Set on first free index */
	tpm_db.owners[free_ind].valid = TPM_DB_VALID;
	tpm_db.owners[free_ind].owner_id = owner_id;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_owner_get_next()
*
* DESCRIPTION:      Returns DB info of PnC range
*
* INPUTS:
* cur_ind          - The current index (handle) to the owner table. Function returns the next
*                    valid table entry > cur_index. cur_index < 0 indicates to return first valid entry in table,
*                    valid next_ind values are 0 or higher.
* OUTPUTS:
* next_ind         - See cur_ind. If end of table was reached before valid entry, (-1) is returned in next_ind
* owner            - Returns the owner of the next index.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t. No more owners is not considered error.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_owner_get_next(int32_t cur_ind, int32_t *next_ind, uint32_t *owner)
{
	uint32_t i;

	/* Initialize first next_ind */
	if (cur_ind < 0)
		i = 0;
	else
		i = cur_ind + 1;

	/* Check owner exists in DB */
	for (; i < TPM_MAX_API_TYPES; i++) {
		if (tpm_db.owners[i].valid == TPM_DB_VALID) {	/* oren - changed = to ==, need to check */
			/* Return Data Structure */
			*next_ind = i;
			*owner = tpm_db.owners[i].owner_id;
			return (TPM_DB_OK);
		}
	}
	/* No valid entry found */
	*next_ind = -1;

	return (TPM_DB_OK);
}

#if 0
/*******************************************************************************
* tpm_db_apig_set()
*
* DESCRIPTION:      Creates an API Group in the DB.
*
* INPUTS:
* owner_id         - owner of the API Group
* api_type        - API Group being created

* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/

int32_t tpm_db_apig_set(uint32_t owner_id, tpm_api_type_t api_type)
{
	uint32_t i;

	/* Check owner exists */
	for (i = 0; i < TPM_MAX_API_TYPES; i++) {
		if ((tpm_db.owners[i].valid == TPM_DB_VALID) && (tpm_db.owners[i].owner_id == owner_id))
			break;
	}
	if (i == TPM_MAX_API_TYPES)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	/* Check api_type does not exist */
	if (tpm_db.api_groups[api_type].valid == TPM_DB_VALID)
		return (TPM_DB_ERR_REC_EXIST);

	tpm_db.api_groups[api_type].valid = TPM_DB_VALID;
	tpm_db.api_groups[api_type].api_owner_id = owner_id;
	tpm_db.api_groups[api_type].api_type = api_type;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_apig_get_next()
*
* DESCRIPTION:      Returns DB info of API Group
*
* INPUTS:
* cur_ind          - The current index (handle) to the owner table. Function returns the next
*                    valid table entry > cur_index. cur_index < 0 indicates to return first valid entry in table,
*                    valid next_ind values are 0 or higher.
* OUTPUTS:
* next_ind         - See cur_ind. If end of table was reached before valid entry, (-1) is returned in next_ind
* api_type         - Returns the API Group.
* owner            - Returns the owner of the API Group.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_apig_get_next(int32_t cur_ind, int32_t *next_ind, uint32_t *owner, tpm_api_type_t *api_type)
{
	uint32_t i;

	/* Initialize first next_ind */
	if (cur_ind < 0)
		i = 0;
	else
		i = cur_ind + 1;

	/* Check API Group exists in DB */
	for (; i < TPM_MAX_API_TYPES; i++) {
		if (tpm_db.api_groups[i].valid == TPM_DB_VALID) {
			/* Return Data  */
			*next_ind = i;
			*api_type = tpm_db.api_groups[i].api_type;
			*owner = tpm_db.api_groups[i].api_owner_id;
			return (TPM_DB_OK);
		}
	}
	/* No valid entry found */
	*next_ind = -1;
	return (TPM_DB_OK);
}
#endif

/*******************************************************************************
* tpm_db_igmp_set_port_frwd_mode()
*
* DESCRIPTION:      Set the IGMP status of a UNI port or WAN port
*
* INPUTS:
* src_port          - source port to set
* igmp_state       - how to process IGMP packets
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_igmp_set_port_frwd_mode(uint32_t port, uint32_t mode)
{
	tpm_db.igmp_def.frwd_mode[port] = mode;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_igmp_get_port_frwd_mode()
*
* DESCRIPTION:      Get the IGMP status of a UNI port or WAN port
*
* INPUTS:
* src_port          - source port to get
*
* OUTPUTS:
* igmp_state       - how to process IGMP packets
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_igmp_get_port_frwd_mode(uint32_t port, uint32_t *mode)
{
	*mode = tpm_db.igmp_def.frwd_mode[port];
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_igmp_set_cpu_queue()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_igmp_set_cpu_queue(uint32_t queue)
{
	tpm_db.igmp_def.cpu_queue = queue;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_igmp_get_cpu_queue()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_igmp_get_cpu_queue(uint32_t *queue)
{
	*queue = tpm_db.igmp_def.cpu_queue;
	return (TPM_DB_OK);
}


/*******************************************************************************
* tpm_db_igmp_set_snoop_enable()
*
* DESCRIPTION:      Set IGMP snoop enable
*
* INPUTS:
* igmp_snoop_enable       - IGMP snoop enable
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_igmp_set_snoop_enable(uint32_t igmp_snoop_enable)
{
	tpm_db.igmp_def.igmp_snoop_enable = igmp_snoop_enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_igmp_get_snoop_enable()
*
* DESCRIPTION:      Get IGMP snoop enable
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop_enable       - IGMP snoop enable
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_igmp_get_snoop_enable(uint32_t *igmp_snoop_enable)
{
	*igmp_snoop_enable = tpm_db.igmp_def.igmp_snoop_enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_omci_type_set()
*
* DESCRIPTION:      Set OMCI ethertype
*
* INPUTS:
* omci_etype       - OMCI Ethertype
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_omci_type_set(uint32_t omci_etype)
{
	tpm_db.init_misc.omci_etype = omci_etype;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_omci_type_get()
*
* DESCRIPTION:      Get OMCI ethertype
*
* OUTPUTS:
* omci_etype       - OMCI Ethertype
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_omci_type_get(uint32_t *omci_etype)
{
	*omci_etype = tpm_db.init_misc.omci_etype;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_omci_channel_set()
*
* DESCRIPTION:      Set OMCI channel parameters
*
* INPUTS:
* gemport       - OMCI GEM port ID
* cpu_rx_q      - OMCI cpu rx queue
* cpu_tx_q      - OMCI cpu tx queue
* tcont_id      - OMCI tcont ID
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_omci_channel_set(uint16_t gemport, uint32_t cpu_rx_q, uint32_t cpu_tx_q, uint32_t tcont_id)
{
	tpm_db.init_misc.omci_gemport = gemport;
	tpm_db.init_misc.oam_cpu_rx_q = cpu_rx_q;
	tpm_db.init_misc.oam_cpu_tx_q = cpu_tx_q;
	tpm_db.init_misc.oam_cpu_tx_port = tcont_id;
	tpm_db.init_misc.oam_channel_configured = 1;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_omci_channel_remove()
*
* DESCRIPTION:      Remove OMCI channel parameters
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_omci_channel_remove(void)
{
	tpm_db.init_misc.omci_gemport = 0;
	tpm_db.init_misc.oam_cpu_rx_q = 0;
	tpm_db.init_misc.oam_cpu_tx_q = 0;
	tpm_db.init_misc.oam_cpu_tx_port = 0;
	tpm_db.init_misc.oam_channel_configured = 0;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_oam_loopback_state_get()
*
* DESCRIPTION:      Get OAM loopback configure state
*
* OUTPUTS:
* omci_etype       - OMCI Ethertype
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_oam_loopback_state_get(uint32_t *loopback)
{
	*loopback = tpm_db.init_misc.oam_loopback_channel_configured;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_oam_loopback_channel_set()
*
* DESCRIPTION:      Set OAM loopback channel state
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_oam_loopback_channel_set(void)
{
	tpm_db.init_misc.oam_loopback_channel_configured = 1;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_omci_channel_remove()
*
* DESCRIPTION:      Remove OMCI channel parameters
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_oam_loopback_channel_remove(void)
{
	tpm_db.init_misc.oam_loopback_channel_configured = 0;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_omci_channel_get()
*
* DESCRIPTION:      Get OMCI channel parameters
*
* INPUTS:
* gemport       - OMCI GEM port ID
* cpu_rx_q      - OMCI cpu rx queue
* cpu_tx_q      - OMCI cpu tx queue
* tcont_id      - OMCI tcont ID
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_omci_channel_get(uint32_t *valid, uint16_t *gemport, uint32_t *cpu_rx_q, uint32_t *cpu_tx_q,
				uint32_t *tcont_id)
{
	*gemport = tpm_db.init_misc.omci_gemport;
	*cpu_rx_q = tpm_db.init_misc.oam_cpu_rx_q;
	*cpu_tx_q = tpm_db.init_misc.oam_cpu_tx_q;
	*tcont_id = tpm_db.init_misc.oam_cpu_tx_port;
	*valid = tpm_db.init_misc.oam_channel_configured;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_oam_channel_set()
*
* DESCRIPTION:      Set OAM channel parameters
*
* INPUTS:
* cpu_rx_q      - OAM cpu rx queue
* cpu_tx_q      - OAM cpu tx queue
* llid          - OAM LLID
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_oam_channel_set(uint32_t cpu_rx_q, uint32_t cpu_tx_q, uint32_t llid)
{
	tpm_db.init_misc.oam_cpu_rx_q = cpu_rx_q;
	tpm_db.init_misc.oam_cpu_tx_q = cpu_tx_q;
	tpm_db.init_misc.oam_cpu_tx_port = llid;
	tpm_db.init_misc.oam_channel_configured = 1;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_oam_channel_remove()
*
* DESCRIPTION:      Remove OAM channel parameters
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_oam_channel_remove(void)
{
	tpm_db.init_misc.omci_gemport = 0;
	tpm_db.init_misc.oam_cpu_rx_q = 0;
	tpm_db.init_misc.oam_cpu_tx_q = 0;
	tpm_db.init_misc.oam_cpu_tx_port = 0;
	tpm_db.init_misc.oam_channel_configured = 0;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_oam_channel_get()
*
* DESCRIPTION:      Get OAM channel parameters
*
* INPUTS:
* cpu_rx_q      - OAM cpu rx queue
* cpu_tx_q      - OAM cpu tx queue
* tcont_id      - OAM llid
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_oam_channel_get(uint32_t *valid, uint32_t *cpu_rx_q, uint32_t *cpu_tx_q, uint32_t *llid)
{
	*cpu_rx_q = tpm_db.init_misc.oam_cpu_rx_q;
	*cpu_tx_q = tpm_db.init_misc.oam_cpu_tx_q;
	*llid = tpm_db.init_misc.oam_cpu_tx_port;
	*valid = tpm_db.init_misc.oam_channel_configured;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_init_deb_port_set()
*
* DESCRIPTION:      Set Init Debug Port
*
* INPUTS:
* pnc_init_debug_port       - Set the debug Port, which will be preconfigured to send
*                             all traffic to CPU (NFS port for development)
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
#if 0
int32_t tpm_db_pnc_init_deb_port_set(uint32_t pnc_init_debug_port)
{
	tpm_db.init_misc.pnc_init_debug_port = pnc_init_debug_port;

	return (TPM_DB_OK);

}
#endif

/*******************************************************************************
* tpm_db_pnc_init_deb_port_get()
*
* DESCRIPTION:      Get Debug Port
*
* OUTPUTS:
* pnc_init_debug_port  - Get the debug Port, which will be preconfigured to send all
*                        traffic to CPU (NFS port for development)
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
#if 0
int32_t tpm_db_pnc_init_deb_port_get(uint32_t *pnc_init_debug_port)
{
	*pnc_init_debug_port = tpm_db.init_misc.pnc_init_debug_port;

	return (TPM_DB_OK);
}
#endif

/*******************************************************************************
* tpm_db_init_done_set()
*
* DESCRIPTION:      Set TPM was successfully Initialized
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_init_done_set(void)
{
	tpm_db.init_misc.tpm_init_succeeded = TPM_DB_VALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_init_done_get()
*
* DESCRIPTION:      Get value if TPM was successfully Initialized
*
* OUTPUTS:
* tpm_init_succeeded  -
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_init_done_get(void)
{
	if (tpm_db.init_misc.tpm_init_succeeded == TPM_DB_VALID)
		return (1);
	return (0);
}

/*******************************************************************************
* tpm_db_pon_type_set()
*
* DESCRIPTION:      Set WAN technology
*
* INPUTS:
* pon_type          - GPON, EPON, Ethernet or None
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pon_type_set(tpm_db_pon_type_t pon_type)
{
	tpm_db.init_misc.pon_type = pon_type;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pon_type_get()
*
* DESCRIPTION:      Get WAN technology
*
* OUTPUTS:
* pon_type          - GPON, EPON, Ethernet or None
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pon_type_get(tpm_db_pon_type_t *pon_type)
{
	*pon_type = tpm_db.init_misc.pon_type;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_active_wan_set()
*
* DESCRIPTION:      Set WAN technology
*
* INPUTS:
* pon_type          - GPON, EPON, Ethernet or None
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_active_wan_set(tpm_gmacs_enum_t active_wan)
{
	tpm_db.init_misc.active_wan = active_wan;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_active_wan_get()
*
* DESCRIPTION:      Set WAN technology
*
* INPUTS:
* pon_type          - GPON, EPON, Ethernet or None
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_gmacs_enum_t tpm_db_active_wan_get()
{
	return tpm_db.init_misc.active_wan;
}

/*******************************************************************************
* tpm_db_pnc_mac_learn_enable_set()
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_mac_learn_enable_set(tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable)
{
	tpm_db.init_misc.pnc_mac_learn_enable = pnc_mac_learn_enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_mac_learn_enable_get()
*
* DESCRIPTION:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_mac_learn_enable_get(tpm_init_pnc_mac_learn_enable_t *pnc_mac_learn_enable)
{
	*pnc_mac_learn_enable = tpm_db.init_misc.pnc_mac_learn_enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_fc_conf_set()
*
* DESCRIPTION:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_fc_conf_set(tpm_init_fc_params_t *port_fc_conf)
{
	memcpy(&tpm_db.port_fc_conf, port_fc_conf, sizeof(tpm_db_fc_params_t));
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_fc_conf_get()
*
* DESCRIPTION:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_fc_conf_get(tpm_init_fc_params_t *port_fc_conf)
{
	memcpy(port_fc_conf, &tpm_db.port_fc_conf, sizeof(tpm_db_fc_params_t));
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_switch_init_set()
*
* DESCRIPTION: Set Amber Initialization
*
* INPUTS:
* switch_init
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_switch_init_set(uint32_t switch_init)
{
	tpm_db.func_profile.switch_init = switch_init;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_switch_init_get()
*
* DESCRIPTION: Get Amber Initialization
*
* INPUTS:
* switch_init
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_switch_init_get(uint32_t *switch_init)
{
	*switch_init = tpm_db.func_profile.switch_init;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_ds_mh_set_conf_set()
*
* DESCRIPTION:      Set downstream marvell header source select
*
* INPUTS:
* ds_mh_set_conf    - Allow set MH in Pnc or not
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_ds_mh_set_conf_set(tpm_db_mh_src_t ds_mh_set_conf)
{
	tpm_db.init_misc.ds_mh_set_conf = ds_mh_set_conf;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_ds_mh_get_conf_set()
*
* DESCRIPTION:      Set downstream marvell header source select
*
* INPUTS:
*
* OUTPUTS:
* ds_mh_set_conf    - Allow set MH in Pnc or not
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_ds_mh_get_conf_set(tpm_db_mh_src_t *ds_mh_set_conf)
{
	*ds_mh_set_conf = tpm_db.init_misc.ds_mh_set_conf;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_cfg_pnc_parse_set()
*
* DESCRIPTION:      Set config PNC parse enabled/disabled in DB
*
* INPUTS:
* cfg_pnc_parse          0=DISABLED  /  1=ENABLED
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_cfg_pnc_parse_set(tpm_init_cfg_pnc_parse_t cfg_pnc_parse)
{
	tpm_db.init_misc.cfg_pnc_parse = cfg_pnc_parse;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_cpu_loopback_set()
*
* DESCRIPTION:      Set cpu loopback enabled/disabled in DB
*
* INPUTS:
* cpu_loopback          0=DISABLED  /  1=ENABLED
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_cpu_loopback_set(tpm_init_cpu_loopback_t cpu_loopback)
{
	tpm_db.init_misc.cpu_loopback = cpu_loopback;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_cfg_pnc_parse_get()
*
* DESCRIPTION:      Get config PNC parse enabled/disabled in DB
*
* OUTPUTS:
* cfg_pnc_parse_val          0=DISABLED  /  1=ENABLED
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_cfg_pnc_parse_get(tpm_init_cfg_pnc_parse_t *cfg_pnc_parse_val)
{
	*cfg_pnc_parse_val = tpm_db.init_misc.cfg_pnc_parse;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_cpu_loopback_get()
*
* DESCRIPTION:      Get CPU loopback enabled/disabled in DB
*
* OUTPUTS:
* cpu_loopback_val          0=DISABLED  /  1=ENABLED
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_cpu_loopback_get(tpm_init_cpu_loopback_t *cpu_loopback_val)
{
	*cpu_loopback_val = tpm_db.init_misc.cpu_loopback;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_double_tag_support_set()
*
* DESCRIPTION:      Set config double tagged supported in DB
*
* INPUTS:
* dbl_tag          0=DISABLED  /  1=ENABLED
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_double_tag_support_set(tpm_init_double_tag_t dbl_tag)
{
	tpm_db.init_misc.dbl_tag = dbl_tag;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_double_tag_support_get()
*
* DESCRIPTION:      Get config double tagged supported in DB
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_double_tag_support_get(tpm_init_double_tag_t *dbl_tag)
{
	*dbl_tag = tpm_db.init_misc.dbl_tag;

	return (TPM_DB_OK);
}

#if 0
/*******************************************************************************
* tpm_db_default_tag_tpid_set()
*
* DESCRIPTION:      Set tpid of one/two vlan tag in DB
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_default_tag_tpid_set(uint16_t vlan1_tpid, uint16_t vlan2_tpid)
{
	tpm_db.init_misc.vlan1_tpid = vlan1_tpid;
	tpm_db.init_misc.vlan2_tpid = vlan2_tpid;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_default_tag_tpid_get()
*
* DESCRIPTION:      Get tpid of one/two vlan tag in DB
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_default_tag_tpid_get(uint16_t *vlan1_tpid, uint16_t *vlan2_tpid)
{
	*vlan1_tpid = tpm_db.init_misc.vlan1_tpid;
	*vlan2_tpid = tpm_db.init_misc.vlan2_tpid;

	return (TPM_DB_OK);
}
#endif

/*******************************************************************************
* tpm_db_default_tag_tpid_set()
*
* DESCRIPTION:      Set tpid of one/two vlan tag in DB
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_default_tag_tpid_set(tpm_init_tpid_opt_t *tag_tpid)
{
	memcpy(&tpm_db.init_misc.tag_tpid, tag_tpid, sizeof(tpm_init_tpid_opt_t));
	return(TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_default_tag_tpid_get()
*
* DESCRIPTION:      Get tpid of one/two vlan tag in DB
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_default_tag_tpid_get(tpm_init_tpid_opt_t *tag_tpid)
{
	memcpy(tag_tpid, &tpm_db.init_misc.tag_tpid, sizeof(tpm_init_tpid_opt_t));
	return(TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_virt_info_set()
*
* DESCRIPTION:      Set wifi via UNI in TPM DB
*
* INPUTS:
* virt_uni_info.enabled          0=DISABLED  /  1=ENABLED
* virt_uni_info.uni_port             port#
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_virt_info_set(tpm_init_virt_uni_t virt_uni_info)
{
	tpm_db.func_profile.virt_uni_info.enabled = virt_uni_info.enabled;
	tpm_db.func_profile.virt_uni_info.uni_port = virt_uni_info.uni_port;
	tpm_db.func_profile.virt_uni_info.switch_port = virt_uni_info.switch_port;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_virt_info_get()
*
* DESCRIPTION:      Get wifi via UNI in TPM DB
*
* OUTPUTS:
* virt_uni_info.enabled          0=DISABLED  /  1=ENABLED
* virt_uni_info.uni_port             port#
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_virt_info_get(tpm_init_virt_uni_t *virt_uni_info)
{
	virt_uni_info->enabled 		= tpm_db.func_profile.virt_uni_info.enabled;
	virt_uni_info->uni_port 	= tpm_db.func_profile.virt_uni_info.uni_port;
	virt_uni_info->switch_port 	= tpm_db.func_profile.virt_uni_info.switch_port;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_ipv6_5t_enable_set()
*
* DESCRIPTION:
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_ipv6_5t_enable_set(tpm_init_ipv6_5t_enable_t ipv6_5t_enable)
{
	tpm_db.init_misc.ipv6_5t_enable = ipv6_5t_enable;
	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_ipv6_5t_enable_get()
*
* DESCRIPTION:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_ipv6_5t_enable_get(tpm_init_ipv6_5t_enable_t *ipv6_5t_enable)
{
	*ipv6_5t_enable = tpm_db.init_misc.ipv6_5t_enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_port_vector_tbl_info_set()
*
* DESCRIPTION:      Set MH port vector table in TPM DB.
*
* OUTPUTS:
* tpm_mh_port_vector_tbl_new
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_port_vector_tbl_info_set(uint32_t reg_num, uint32_t uni_vector, uint16_t amber_port_vector,
					uint32_t pnc_vector)
{
	if (TPM_TX_MAX_MH_REGS <= reg_num)
		return TPM_DB_ERR_REC_INV;

	tpm_db.tpm_mh_port_vector_tbl[reg_num].uni_vector = uni_vector;
	tpm_db.tpm_mh_port_vector_tbl[reg_num].pnc_vector = pnc_vector;
	tpm_db.tpm_mh_port_vector_tbl[reg_num].amber_port_vector = amber_port_vector;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_port_vector_tbl_info_get()
*
* DESCRIPTION:      Get MH port vector table in TPM DB.
*
* OUTPUTS:
* tpm_mh_port_vector_tbl_new
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_port_vector_tbl_info_get(uint32_t reg_num, uint32_t *uni_vector, uint16_t *amber_port_vector,
					uint32_t *pnc_vector)
{
	if (TPM_TX_MAX_MH_REGS <= reg_num)
		return TPM_DB_ERR_REC_INV;

	*uni_vector = tpm_db.tpm_mh_port_vector_tbl[reg_num].uni_vector;
	*pnc_vector = tpm_db.tpm_mh_port_vector_tbl[reg_num].pnc_vector;
	*amber_port_vector = tpm_db.tpm_mh_port_vector_tbl[reg_num].amber_port_vector;
	return (TPM_DB_OK);
}


/*******************************************************************************
* tpm_db_port_vector_tbl_info_search()
*
* DESCRIPTION:      Search in the MH port vector table in TPM DB for matching target port
*
* OUTPUTS:
* tpm_mh_port_vector_tbl_new
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_port_vector_tbl_info_search(tpm_trg_port_type_t  trg_port, uint32_t *uni_vector,
					    uint16_t *amber_port_vector, uint32_t *pnc_vector)
{
	uint32_t reg_num;

	for (reg_num = 0; reg_num < TPM_TX_MAX_MH_REGS; reg_num++) {
		if (trg_port == tpm_db.tpm_mh_port_vector_tbl[reg_num].uni_vector) {
			*uni_vector = tpm_db.tpm_mh_port_vector_tbl[reg_num].uni_vector;
			*pnc_vector = tpm_db.tpm_mh_port_vector_tbl[reg_num].pnc_vector;
			*amber_port_vector = tpm_db.tpm_mh_port_vector_tbl[reg_num].amber_port_vector;
			return TPM_DB_OK;
		}
	}

	return TPM_DB_ERR_REC_INV;
}



/*******************************************************************************
* tpm_db_api_section_ent_tbl_reset()
*
* DESCRIPTION:      Reset an API table/acl
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_ent_tbl_reset(uint32_t start_entry, uint32_t num_entries)
{
	uint32_t i;

	/*Init API Rng Entries */
	memset(&(tpm_db.api_ent_mem_area[mem_alloc_start_ind]), 0, (num_entries) * (sizeof(tpm_db_api_entry_t)));

	/* Invalidate API Rng Entries */
	for (i = start_entry; i < (start_entry + num_entries); i++)
		tpm_db.api_ent_mem_area[i].valid = TPM_DB_INVALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_ent_tbl_get()
*
* DESCRIPTION:      get an API table/acl entry
*
* INPUTS:
* api_sec      - Section of the API acl/table
* index        - rule index
* OUTPUTS:
* api_ent_mem_area - table/acl entry
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_ent_tbl_get(tpm_api_sections_t api_sec, tpm_db_api_entry_t *api_ent_mem_area, uint32_t index)
{
	if (ILLEGAL_API_SEC(api_sec))
		IF_ERROR_I(TPM_DB_ERR_REC_NOT_EXIST, api_sec);

	if (NULL == api_ent_mem_area)
		return (TPM_DB_ERR_INV_INPUT);

	memcpy(api_ent_mem_area, &tpm_db.api_ent_mem_area[(tpm_db.api_section[api_sec].table_start) + index], sizeof(tpm_db_api_entry_t));

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_init()
*
* DESCRIPTION:      Initialize an API table/acl
*
* INPUTS:
* api_section      - Section of the API acl/table
* api_rng_size     - Size of the table/acl
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_init(tpm_api_sections_t api_section, tpm_pnc_ranges_t prim_pnc_range, uint32_t api_rng_size)
{

	/* Check that new api_rng table does not cross allocation boundaries */
	if ((mem_alloc_start_ind + api_rng_size) > TPM_DB_API_ENTRIES_TBL_SIZE) {
		TPM_OS_ERROR(TPM_DB_MOD, " API Range not created, Entries Area overflow, "
			     "ind_start(%d), api_rng_size(%d), mem_area_size(%d) \n",
			     mem_alloc_start_ind, api_rng_size, TPM_DB_API_ENTRIES_TBL_SIZE);

		return (TPM_DB_ERR_DB_TBL_FULL);
	}
	if ((mem_alloc_start_ind + api_rng_size) > (TPM_DB_API_ENTRIES_TBL_SIZE - TPM_DB_API_SECTION_MARGIN_SIZE)) {
		TPM_OS_WARN(TPM_DB_MOD, "Entries Area has less than calculated end margin, "
			    "ind_start(%d), api_rng_size(%d), mem_area_size(%d)\n",
			    mem_alloc_start_ind, api_rng_size, TPM_DB_API_ENTRIES_TBL_SIZE);
	}

	/*Init API Range */
	memset(&(tpm_db.api_section[api_section]), 0, sizeof(tpm_db_api_section_t));

	tpm_db_api_section_ent_tbl_reset(mem_alloc_start_ind, api_rng_size);

	tpm_db.api_section[api_section].table_size = api_rng_size;
	tpm_db.api_section[api_section].table_start = mem_alloc_start_ind;
	tpm_db.api_section[api_section].valid = TPM_DB_VALID;
	tpm_db.api_section[api_section].prim_pnc_range = prim_pnc_range;
	tpm_db.api_section[api_section].last_valid_entry = -1;
	tpm_db.api_section[api_section].last_valid_index = -1;
	tpm_db.api_section[api_section].num_valid_entries = 0;

	/* Forward to next 'API Entries table' start_index.
	 * This also inserts 'margin_size' between subsequent 'API Entries table' */
	mem_alloc_start_ind += (api_rng_size + TPM_DB_API_SECTION_MARGIN_SIZE);

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entries_area_reset()
*
* DESCRIPTION:      Reset an API table/acl
*
* INPUTS:
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entries_area_reset(void)
{
	/* Stripe API Entries Area */
	memset(tpm_db.api_ent_mem_area, MEM_MARGIN_PATTERN, sizeof(tpm_db.api_ent_mem_area));

	/* Reset Area Index, Start Area offset at margin_size */
	mem_alloc_start_ind = TPM_DB_API_SECTION_MARGIN_SIZE;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_invalidate()
*
* DESCRIPTION:      Reset an API table/acl
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_invalidate(tpm_api_sections_t api_section)
{
	/*Init API Range */
	memset(&(tpm_db.api_section[api_section]), 0, sizeof(tpm_db_api_section_t));

	/* Invalidate Range */
	tpm_db.api_section[api_section].valid = TPM_DB_INVALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_reset()
*
* DESCRIPTION:      Initialize an API table/acl
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_reset(tpm_api_sections_t api_section)
{
	/* prim_pnc_range, table_size, table_start attributes are retained from init */

	tpm_db.api_section[api_section].num_valid_entries = 0;
	tpm_db.api_section[api_section].last_valid_entry = -1;
	tpm_db.api_section[api_section].last_valid_index = -1;

	/* Invalidate all API entries */
	tpm_db_api_section_ent_tbl_reset(tpm_db.api_section[api_section].table_start,
					 tpm_db.api_section[api_section].table_size);

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_get()
*
* DESCRIPTION:      Get
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
* api_rng_size      - API range size
* num_valid_entries- Number of valid entries
* last_valid - Last valid entry
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_get(tpm_api_sections_t api_section,
			       uint32_t 	*api_rng_size,
			       uint32_t 	*num_valid_entries,
			       tpm_pnc_ranges_t	*prim_pnc_range,
			       int32_t 		*last_valid_entry,
			       uint32_t 	*tbl_start)
{
	if (ILLEGAL_API_SEC(api_section))
		IF_ERROR_I(TPM_DB_ERR_REC_NOT_EXIST, api_section);

	if (tpm_db.api_section[api_section].valid == TPM_DB_INVALID)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	*api_rng_size = tpm_db.api_section[api_section].table_size;
	*num_valid_entries = tpm_db.api_section[api_section].num_valid_entries;
	*prim_pnc_range = tpm_db.api_section[api_section].prim_pnc_range;
	*last_valid_entry = tpm_db.api_section[api_section].last_valid_entry;
	*tbl_start = tpm_db.api_section[api_section].table_start;

	TPM_OS_DEBUG(TPM_DB_MOD,
		     "returned size(%d), num_entries(%d), prim_pnc_range(%d), last_valid_entry (%d) tbl_start(%d)\n",
		     *api_rng_size, *num_valid_entries, *prim_pnc_range, *last_valid_entry, *tbl_start);

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_lastentry_get()
*
* DESCRIPTION:      Get
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
* last_valid - Last valid entry
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_lastentry_get(tpm_api_sections_t api_section, int32_t *last_valid_entry)
{
	if (tpm_db.api_section[api_section].valid == TPM_DB_INVALID)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	*last_valid_entry = tpm_db.api_section[api_section].last_valid_entry;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_main_pnc_get()
*
* DESCRIPTION:      Get
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
* prim_pnc_range - Primary PNC Range for this API ACL/Table
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_main_pnc_get(tpm_api_sections_t api_section, tpm_pnc_ranges_t *prim_pnc_range)
{
	if (tpm_db.api_section[api_section].valid == TPM_DB_INVALID)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	*prim_pnc_range = tpm_db.api_section[api_section].prim_pnc_range;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_num_entries_get()
*
* DESCRIPTION:      Get
*
* INPUTS:
* api_section      - Section of the API acl/table
* OUTPUTS:
* num_entries - Number of valid entries
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_num_entries_get(tpm_api_sections_t api_section, uint32_t *num_entries)
{
	if (tpm_db.api_section[api_section].valid == TPM_DB_INVALID)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	*num_entries = tpm_db.api_section[api_section].num_valid_entries;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_section_val_get_next()
*
* DESCRIPTION:      Get next valid API range
*
* INPUTS:
* cur_api_section      - Section of the API acl/table
* OUTPUTS:
* api_rng_size      - API range size
* num_valid_entries- Number of valid entries
* last_valid - Last valid entry
* next_section
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_val_get_next(int32_t cur_api_section,
					int32_t *next_section,
					uint32_t *api_rng_size,
					uint32_t *num_valid_entries, int32_t *last_valid_entry, uint32_t *tbl_start)
{
	uint32_t x;

	/* Check Params */
	if (cur_api_section < -1 || cur_api_section >= TPM_MAX_NUM_API_SECTIONS)
		return (TPM_DB_ERR_INV_INPUT);

	/* Initialize next section  */
	x = (uint32_t) (cur_api_section + 1);

	/* Check PnC range exists in DB */
	for (; x < TPM_MAX_NUM_API_SECTIONS; x++) {
		if (tpm_db.api_section[x].valid == TPM_DB_VALID) {
			/* Return Data */
			*api_rng_size = tpm_db.api_section[x].table_size;
			*num_valid_entries = tpm_db.api_section[x].num_valid_entries;
			*last_valid_entry = tpm_db.api_section[x].last_valid_entry;
			*tbl_start = tpm_db.api_section[x].table_start;
			*next_section = x;
			return (TPM_DB_OK);
		}
	}
	/* No valid entry found */
	*next_section = -1;

	return (TPM_DB_OK);
}

#if 0	/* YUVAL_OK */
void tpm_db_api_area_bubble_sort(tpm_db_api_check_t *array, uint8_t num_elements)
{
	int32_t i, j;
	tpm_db_api_check_t temp_var;

	if (num_elements <= 1)
		return;

	for (i = 0; i < num_elements; i++) {
		/* back through the area bringing smallest remaining element to position i */
		for (j = num_elements - 1; j > i; j--) {
			if (array[j - 1].table_start > array[j].table_start) {
				memcpy(&temp_var, &(array[j]), sizeof(tpm_db_api_check_t));
				memcpy(&(array[j]), &(array[j - 1]), sizeof(tpm_db_api_check_t));
				memcpy(&(array[j - 1]), &temp_var, sizeof(tpm_db_api_check_t));

			}
		}
	}

	return;
}

/*******************************************************************************
* tpm_db_api_section_margins_check()
*
* DESCRIPTION:      Get next valid API range
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_rng_margins_self_check(void)
{

	tpm_api_sections_t cur_section = -1, next_section = 0;
	int32_t last_valid;
	int32_t ret_code, margin_area;
	uint32_t margin_entry, api_rng_size = 0, num_valid_entries;
	uint32_t tbl_start_ind = 0, table_size, margin_start_ind, area_num, num_ranges;
	uint32_t i = 0, k = 0;
	uint8_t *margin_p;
	tpm_db_api_check_t api_sections_areas[TPM_MAX_NUM_API_SECTIONS];

	/* Loop through valid API ranges read them into local array */
	ret_code = tpm_db_api_section_val_get_next(cur_section, &next_section, &api_rng_size,
						   &num_valid_entries, &last_valid, &tbl_start_ind);
	IF_ERROR(ret_code);

	while (next_section != TPM_INVALID_SECTION) {
		api_sections_areas[i].table_size = api_rng_size;
		api_sections_areas[i].table_start = tbl_start_ind;

		cur_section = next_section;
		i++;

		ret_code = tpm_db_api_section_val_get_next(cur_section, &next_section, &api_rng_size,
							   &num_valid_entries, &last_valid, &tbl_start_ind);
		IF_ERROR(ret_code);
	}

	num_ranges = i;

	/* Simple Ordering */
	tpm_db_api_area_bubble_sort(api_sections_areas, num_ranges);
	if (1) {
		printk("%s (%d)\n", __func__, __LINE__);
		for (k = 0; k < num_ranges; k++) {
			printk("entry(%d) table_start(%d) table_size(%d)\n", k, api_sections_areas[k].table_start,
			       api_sections_areas[k].table_size);
		}
	}

	/* Check margins */
	margin_start_ind = 0;
	i = 0;

	for (area_num = 0; area_num < num_ranges; area_num++) {
		tbl_start_ind = api_sections_areas[area_num].table_start;
		table_size = api_sections_areas[area_num].table_size;

		margin_p = (uint8_t *) &(tpm_db.api_ent_mem_area[margin_start_ind]);
		margin_area = (tbl_start_ind - margin_start_ind) * (sizeof(tpm_db_api_entry_t));
		i = 0;

		while (i < margin_area) {
			if (*(margin_p + i) != MEM_MARGIN_PATTERN) {
				/* Print complete tpm_db_api_entry_t from start to end, and also skips checking the rest of it */
				margin_entry = i / (sizeof(tpm_db_api_entry_t));

				printk("margin entry(%d):\n", margin_entry);
				for (i = margin_entry * sizeof(tpm_db_api_entry_t);
				     i < (margin_entry + 1) * sizeof(tpm_db_api_entry_t); i++) {
					printk("%x ", *(margin_p + i));
				}

				/* i now points to start of next entry */
			} else {
				i++;
			}
		}
		margin_start_ind = tbl_start_ind + api_sections_areas[area_num].table_size;
	}

	printk("===========================================\n");
	return (TPM_OK);

}
#endif

/*******************************************************************************
* tpm_db_api_section_entry_add()
*
* DESCRIPTION:      Update Range params to add new entry
*
* INPUTS:
* api_section       - Section of the API acl/table
* new_last_valid    - Update the last valid entry with this value
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_section_entry_add(tpm_api_sections_t api_section, int32_t new_last_valid)
{
	if (tpm_db.api_section[api_section].valid == TPM_DB_INVALID)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	(tpm_db.api_section[api_section].num_valid_entries)++;
	tpm_db.api_section[api_section].last_valid_entry = new_last_valid;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_freeentry_get()
*
* DESCRIPTION:      Get free API entry
*
* INPUTS:
* api_section       - Section of the API acl/table
* OUTPUTS:
* ind               - Return Free Index
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_freeentry_get(tpm_api_sections_t api_section, int32_t *index)
{
	uint32_t i;

	/* Find Empty Index */
	for (i = 0; i < tpm_db.api_section[api_section].table_size; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_INVALID)
			break;
	}
	if (i == tpm_db.api_section[api_section].table_size) {
		*index = -1;
		return (TPM_DB_ERR_DB_TBL_FULL);
	}

	*index = i;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entry_ind_get()
*
* DESCRIPTION:      Get Index (handle) of a rulenum
*
* INPUTS:
* api_section       - Section of the API acl/table
* rulenum           - Rulenum of the API acl/table
* OUTPUTS:
* index             - Return rule number's index
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_ind_get(tpm_api_sections_t api_section, uint32_t rule_num, int32_t *index)
{
	int32_t i;

	/* Find the rulenum index */
	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID && API_ENT_I(api_section, i).rule_num == rule_num)
			break;
	}

	if (i > tpm_db.api_section[api_section].last_valid_index) {
		*index = -1;
		return (TPM_DB_ERR_REC_NOT_EXIST);
	}

	*index = i;
	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_api_tcam_rule_idx_get()
*
* DESCRIPTION:      Get PnC rule index by API section and TCAM number
*
* INPUTS:
* api_section       - Section of the API acl/table
* tcam_num          - TCAM number of the API acl/table
* OUTPUTS:
* rule_idx          - Return rule index for PnC rule
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_tcam_rule_idx_get(tpm_api_sections_t api_section, uint32_t tcam_num, uint32_t *rule_idx)
{
	int32_t i;

	/* Find the rule index */
	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		/*
		   printk("tpm_db_api_tcam_rule_idx_get index[%d], valid[%d], num_pnc_ranges[%d], pnc_index[%d] \n",
		   i, API_ENT_I(api_section, i).valid, API_ENT_I(api_section, i).pnc_tbl_conn.num_pnc_ranges,
		   API_ENT_I(api_section, i).pnc_tbl_conn.pnc_conn_tbl[0].pnc_index);
		 */
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID
		    && API_ENT_I(api_section, i).pnc_tbl_conn.num_pnc_ranges > 0
		    && API_ENT_I(api_section, i).pnc_tbl_conn.pnc_conn_tbl[0].pnc_index == tcam_num)
			break;
	}

	if (i > tpm_db.api_section[api_section].last_valid_index) {
		*rule_idx = 0;
		return (TPM_DB_ERR_REC_NOT_EXIST);
	}

	*rule_idx = API_ENT_I(api_section, i).rule_idx;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entry_set()
*
* DESCRIPTION:      Set an API Entry
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_num         - Rule number in the API range
* bi_dir           - Is this rule Bi-directional
* api_data         - The API key data for this entry
* mod_con          - Definition of connection to modification tables
* pnc_con          - Definition of connection to PNC tables
* OUTPUTS:
* rule_idx         - Unique Rule Identifier, that does not change over the life span of the rule.
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_set(tpm_api_sections_t api_section,
			     uint32_t rule_num,
			     uint32_t bi_dir,
			     tpm_rule_entry_t *api_data,
			     tpm_db_mod_conn_t *mod_con, tpm_db_pnc_conn_t *pnc_con, uint32_t *rule_idx)
{
	int32_t ind;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pnc_range_conf_t rangConf;
	int32_t ret_code;
	uint32_t l_rule_idx;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d), - rule_num(%d)\n", api_section, rule_num);

	/* Find Empty Index */
	tpm_db_api_freeentry_get(api_section, &ind);
	if (ind == -1)
		return (TPM_DB_ERR_DB_TBL_FULL);

	/* Get Range_Id, rang configuration, to get range type */
	ret_code = tpm_db_api_section_main_pnc_get(api_section, &range_id);
	IF_ERROR(ret_code);
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Fill in API entry */
	memcpy(&(API_ENT_I(api_section, ind).api_rule_data), api_data, sizeof(tpm_rule_entry_t));
	if (mod_con)
		memcpy(&(API_ENT_I(api_section, ind).mod_tbl_conn), mod_con, sizeof(tpm_db_mod_conn_t));

	memcpy(&(API_ENT_I(api_section, ind).pnc_tbl_conn), pnc_con, sizeof(tpm_db_pnc_conn_t));

	API_ENT_I(api_section, ind).bi_dir = bi_dir;
	l_rule_idx = tpm_db_rule_index_get();
	if (TPM_RANGE_TYPE_ACL == rangConf.range_type)
		API_ENT_I(api_section, ind).rule_idx = l_rule_idx;
	else
		API_ENT_I(api_section, ind).rule_idx = rule_num;

	API_ENT_I(api_section, ind).rule_num = rule_num;
	API_ENT_I(api_section, ind).valid = TPM_DB_VALID;

	/* Increase highest tbl_index if needed */
	if (ind > tpm_db.api_section[api_section].last_valid_index)
		tpm_db.api_section[api_section].last_valid_index = ind;


	/* Set last_valid to rule_num if needed */
	if ((tpm_db.api_section[api_section].last_valid_entry < 0)
	    || ((int32_t) rule_num > tpm_db.api_section[api_section].last_valid_entry))
		tpm_db.api_section[api_section].last_valid_entry = (int32_t) rule_num;

	(tpm_db.api_section[api_section].num_valid_entries)++;

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
		*rule_idx = l_rule_idx;
		tpm_db_rule_index_incrs();
	} else
		*rule_idx = rule_num;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entry_invalidate()
*
* DESCRIPTION:      Invalidate an API Entry
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_num         - Rule number in the API range
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_invalidate(tpm_api_sections_t api_section, uint32_t rule_num)
{
	int32_t ind;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d), - rule_num(%d)\n", api_section, rule_num);

	/* Find rule_num Index */
	tpm_db_api_entry_ind_get(api_section, rule_num, &ind);
	if (ind == -1)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	TPM_OS_DEBUG(TPM_DB_MOD, " ind(%d), \n", ind);

	/* Invalidate the entry on the api_table */
	memset(&(API_ENT_I(api_section, ind)), 0, sizeof(tpm_db_api_entry_t));
	API_ENT_I(api_section, ind).valid = TPM_DB_INVALID;

	/* Decrease highest tbl_index if needed */
	if (ind == tpm_db.api_section[api_section].last_valid_index)
		tpm_db.api_section[api_section].last_valid_index--;

	/* Decrease last_valid if needed */
	if ((int32_t) rule_num == tpm_db.api_section[api_section].last_valid_entry)
		tpm_db.api_section[api_section].last_valid_entry--;

	/* Decrease the number of valid entries */
	(tpm_db.api_section[api_section].num_valid_entries)--;

	TPM_OS_DEBUG(TPM_DB_MOD, " last_valid_entry(%d), num_valid_entries(%d), \n",
		     tpm_db.api_section[api_section].last_valid_entry,
		     tpm_db.api_section[api_section].num_valid_entries);

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entry_rulenum_inc()
*
* DESCRIPTION:      Increase rulenum of a range of API entries
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_inc_start   - Start of Range of API entries to increase the rule_num
* rule_inc_end     - End of Range of API entries to increase the rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_rulenum_inc(tpm_api_sections_t api_section, uint32_t rule_inc_start, uint32_t rule_inc_end)
{
	int32_t i;
	int32_t ind, ret_code;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d) - range_inc_start(%d) range_inc_end(%d) \n",
		     api_section, rule_inc_start, rule_inc_end);

	if (rule_inc_end >= (tpm_db.api_section[api_section].table_size - 1)) {
		TPM_OS_ERROR(TPM_DB_MOD, " for api_section(%d), - range_inc_end(%d) too large \n",
			     api_section, rule_inc_end);
		return (TPM_DB_ERR_INV_INPUT);
	}

	/* Increase all entries */
	for (i = rule_inc_end; i >= (int32_t) (rule_inc_start); i--) {
		ret_code = tpm_db_api_entry_ind_get(api_section, i, &ind);
		IF_ERROR_I(ret_code, i);
		(API_ENT_I(api_section, ind).rule_num)++;
	}

	/* Increase last_valid rule_num, if necessary */
	if (tpm_db.api_section[api_section].last_valid_entry == (int32_t) rule_inc_end)
		(tpm_db.api_section[api_section].last_valid_entry)++;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entry_rulenum_dec()
*
* DESCRIPTION:      Decrease rulenum of a range of API entries
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_dec_start   - Start of Range of API entries to decrease the rule_num
* rule_dec_end     - End of Range of API entries to decrease the rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_rulenum_dec(tpm_api_sections_t api_section, uint32_t rule_dec_start, uint32_t rule_dec_end)
{

	uint32_t i;
	int32_t ind, ret_code;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d) - rule_dec_start(%d) rule_dec_end(%d) \n",
		     api_section, rule_dec_start, rule_dec_end);

	if (rule_dec_start == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "for api_section(%d) -  rule_dec_start cannot be 0\n", api_section);
		return (TPM_DB_ERR_INV_INPUT);
	}

	/* Decrease all entries */
	for (i = rule_dec_start; i <= rule_dec_end; i++) {
		ret_code = tpm_db_api_entry_ind_get(api_section, i, &ind);
		IF_ERROR_I(ret_code, i);
		(API_ENT_I(api_section, ind).rule_num)--;
	}

	/* Decrease last_valid, if it equaled last in range */
	if (tpm_db.api_section[api_section].last_valid_entry == (int32_t) rule_dec_end)
		(tpm_db.api_section[api_section].last_valid_entry)--;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_specific_pnc_dec()
*
* DESCRIPTION:      Decrease the connected PCN entry of a specific pnc range
*                   for a range of API entries
*
* INPUTS:
* api_section      - Section of the API acl/table
* range_id         - Pnc range id
* rule_inc_start   - Start of Range of API entries to decrease the rule_num
* rule_inc_end     - End of Range of API entries to decrease the rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_specific_pnc_dec(tpm_api_sections_t api_section,
				    tpm_pnc_ranges_t range_id, uint32_t rule_inc_start, uint32_t rule_inc_end)
{
	uint32_t i, j;
	int32_t ret_code, ind;
	tpm_db_pnc_conn_t *pnc_tbl_conn;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d), range(%d) - rule_inc_start(%d) rule_inc_end(%d) \n",
		     api_section, range_id, rule_inc_start, rule_inc_end);

	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	/* Increase pnc_entries of all rule_nums in the range */
	for (i = rule_inc_start; i <= rule_inc_end; i++) {
		ret_code = tpm_db_api_entry_ind_get(api_section, i, &ind);
		IF_ERROR_I(ret_code, i);
		if (API_ENT_I(api_section, ind).valid == TPM_DB_INVALID)
			continue;

		pnc_tbl_conn = &(API_ENT_I(api_section, ind).pnc_tbl_conn);

		/* Loop over all then pnc_entries of this api_entry */
		for (j = 0; j < (pnc_tbl_conn->num_pnc_ranges); j++) {

			if (pnc_tbl_conn->pnc_conn_tbl[j].pnc_range == (uint32_t) range_id)
				pnc_tbl_conn->pnc_conn_tbl[j].pnc_index--;
		}
	}

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_specific_pnc_inc()
*
* DESCRIPTION:      Increase the connected PCN entry of a specific pnc range
*                   for a range of API entries
*
* INPUTS:
* api_section      - Section of the API acl/table
* range_id         - Pnc range id
* rule_inc_start   - Start of Range of API entries to decrease the rule_num
* rule_inc_end     - End of Range of API entries to decrease the rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_specific_pnc_inc(tpm_api_sections_t api_section,
				    tpm_pnc_ranges_t range_id, uint32_t rule_inc_start, uint32_t rule_inc_end)
{
	uint32_t i, j;
	int32_t ret_code, ind;
	tpm_db_pnc_conn_t *pnc_tbl_conn;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d), range(%d) - rule_inc_start(%d) rule_inc_end(%d) \n",
		     api_section, range_id, rule_inc_start, rule_inc_end);

	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	/* Increase pnc_entries of all rule_nums in the range */
	for (i = rule_inc_start; i <= rule_inc_end; i++) {
		ret_code = tpm_db_api_entry_ind_get(api_section, i, &ind);
		IF_ERROR_I(ret_code, i);

		pnc_tbl_conn = &(API_ENT_I(api_section, ind).pnc_tbl_conn);

		/* Loop over all then pnc_entries of this api_entry */
		for (j = 0; j < (pnc_tbl_conn->num_pnc_ranges); j++) {

			if (pnc_tbl_conn->pnc_conn_tbl[j].pnc_range == (uint32_t) range_id)
				pnc_tbl_conn->pnc_conn_tbl[j].pnc_index++;
		}
	}

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_conn_inc()
*
* DESCRIPTION:      Increase the connected PCN entry for a range of API entries
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_inc_start   - Start of Range of API entries to decrease the rule_num
* rule_inc_end     - End of Range of API entries to decrease the rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_conn_inc(tpm_api_sections_t api_section, uint32_t rule_inc_start, uint32_t rule_inc_end)
{
	int32_t i;
	uint32_t j;
	int32_t ind, ret_code;
	tpm_db_pnc_conn_t *pnc_tbl_conn;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d) - rule_inc_start(%d) rule_inc_end(%d) \n",
		     api_section, rule_inc_start, rule_inc_end);

	if (rule_inc_end >= (tpm_db.api_section[api_section].table_size - 1)) {
		TPM_OS_ERROR(TPM_DB_MOD, " for api_section(%d)- range_inc_end(%d) too large \n",
			     api_section, rule_inc_end);
		return (TPM_DB_ERR_INV_INPUT);
	}

	/* Increase pnc_entries of all rule_nums in the range */
	for (i = rule_inc_end; i >= (int32_t) (rule_inc_start); i--) {
		ret_code = tpm_db_api_entry_ind_get(api_section, i, &ind);
		IF_ERROR_I(ret_code, i);
		pnc_tbl_conn = &(API_ENT_I(api_section, ind).pnc_tbl_conn);

		/* Loop over all then pnc_entries of this api_entry */
		for (j = 0; j < (pnc_tbl_conn->num_pnc_ranges); j++) {
			/* INcrease only the entries of the PNC Ranges that are TYPE_ACL */
			ret_code = tpm_db_pnc_rng_conf_get(pnc_tbl_conn->pnc_conn_tbl[j].pnc_range, &range_conf);
			IF_ERROR_I(ret_code, i);
			if (range_conf.range_type == TPM_RANGE_TYPE_ACL)
				pnc_tbl_conn->pnc_conn_tbl[j].pnc_index++;
		}
	}

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_conn_dec()
*
* DESCRIPTION:      Decrease the connected PCN entry for a range of API entries
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_dec_start   - Start of Range of API entries to decrease the rule_num
* rule_dec_end     - End of Range of API entries to decrease the rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_conn_dec(tpm_api_sections_t api_section, uint32_t rule_dec_start, uint32_t rule_dec_end)
{
	uint32_t i, j;
	int32_t ind, ret_code;
	tpm_db_pnc_conn_t *pnc_tbl_conn;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d) - rule_dec_start(%d) rule_dec_end(%d) \n",
		     api_section, rule_dec_start, rule_dec_end);

	if (rule_dec_start == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, " for api_section(%d) - rule_dec_start is zero \n", api_section);
		return (TPM_DB_ERR_INV_INPUT);
	}

	/* Decrease pnc_entries of all rule_nums in the range */
	for (i = rule_dec_start; i <= rule_dec_end; i++) {
		ret_code = tpm_db_api_entry_ind_get(api_section, i, &ind);
		IF_ERROR_I(ret_code, i);
		pnc_tbl_conn = &(API_ENT_I(api_section, ind).pnc_tbl_conn);

		/* Loop over all then pnc_entries of this api_entry */
		for (j = 0; j < (pnc_tbl_conn->num_pnc_ranges); j++) {
			/* Decrease only the entries of the PNC Ranges that are TYPE_ACL */
			ret_code = tpm_db_pnc_rng_conf_get(pnc_tbl_conn->pnc_conn_tbl[j].pnc_range, &range_conf);
			IF_ERROR_I(ret_code, i);
			if (range_conf.range_type == TPM_RANGE_TYPE_ACL)
				pnc_tbl_conn->pnc_conn_tbl[j].pnc_index--;
		}
	}

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_tcam_num_get()
*
* DESCRIPTION:      Get API TCAM number according to API section and rule index
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_idx         - Unique Rule Identifier, that does not change over the life span of the rule.
*
* OUTPUTS:
* tcam_num         - PnC TCAM number
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_tcam_num_get(tpm_api_sections_t api_section, uint32_t rule_idx, uint32_t *tcam_num)
{
	int32_t ret_code;
	uint32_t rule_num;
	uint32_t dummy_idx;
	uint32_t bi_dir;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con = { 0, 0 };
	tpm_db_pnc_conn_t pnc_con;

	/* Get rule number */
	ret_code = tpm_db_api_rulenum_get(api_section, rule_idx, &rule_num);
	IF_ERROR(ret_code);

	/* Get API entry */
	ret_code = tpm_db_api_entry_get(api_section, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
	IF_ERROR(ret_code);

	/* Get TCAM number, always return the first TCAM number even there are multiple  */
	/* TCAM entries to one rule index                                                */
	*tcam_num = pnc_con.pnc_conn_tbl[0].pnc_index;

	return (TPM_DB_OK);
}

int32_t tpm_db_api_entry_update_rule_idx(tpm_api_sections_t api_section,
			     			uint32_t rule_idx_pre,
			     			uint32_t rule_idx_new)
{
	int32_t ind;
	int32_t ret_code;
	uint32_t rule_num = 0;

	TPM_OS_DEBUG(TPM_DB_MOD, " api_section(%d), - rule_num(%d)\n", api_section, rule_num);

	/* Get the rule_num */
	ret_code = tpm_db_api_rulenum_get(api_section, rule_idx_pre, &rule_num);
	if (ret_code == TPM_DB_ERR_REC_NOT_EXIST) {
		TPM_OS_ERROR(TPM_DB_MOD, " The rule non-exist!\n");
		return ERR_RULE_IDX_INVALID;
	}
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "with rule_num(%d)\n", rule_num);

	tpm_db_api_entry_ind_get(api_section, rule_num, &ind);
	if (ind == -1)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	/* update rule_index */
	API_ENT_I(api_section, ind).rule_idx = rule_idx_new;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_rulenum_get()
*
* DESCRIPTION:      Get  API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_idx         - Unique Rule Identifier, that does not change over the life span of the rule.
*
* OUTPUTS:
* rule_num         - The API entry number
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_rulenum_get(tpm_api_sections_t api_section, uint32_t rule_idx, uint32_t *rule_num)
{
	int32_t i;

	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID && API_ENT_I(api_section, i).rule_idx == rule_idx) {
			*rule_num = API_ENT_I(api_section, i).rule_num;
			return (TPM_DB_OK);
		}
	}
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_api_rulenum_get_from_l2_key()
*
* DESCRIPTION:      Get  API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* parse_bm         - fields to be parsed in the key.
* l2_key           - structure of l2_key
*
* OUTPUTS:
* rule_num         - The API entry number
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_rulenum_get_from_l2_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
					   tpm_l2_acl_key_t *l2_key, uint32_t *rule_num)
{
	int32_t i, cmp_res;

	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID) {
			cmp_res = memcmp(&(API_ENT_I(api_section, i).api_rule_data.l2_prim_key.l2_key),
					 l2_key, sizeof(tpm_l2_acl_key_t));
			if (cmp_res == 0) {	/* found entry */
				/* check the parse_bm */
				if (parse_rule_bm == API_ENT_I(api_section, i).api_rule_data.l2_prim_key.parse_rule_bm) {
					*rule_num = API_ENT_I(api_section, i).rule_num;
					return (TPM_DB_OK);
				}
			}
		}
	}
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_api_rulenum_get_from_l3_key()
*
* DESCRIPTION:      Get  API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* parse_bm         - fields to be parsed in the key.
* l3_key           - structure of l3_key
*
* OUTPUTS:
* rule_num         - The API entry number
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_rulenum_get_from_l3_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
					   tpm_l3_type_key_t *l3_key, uint32_t *rule_num)
{
	int32_t i, cmp_res;

	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID) {
			cmp_res = memcmp(&API_ENT_I(api_section, i).api_rule_data.l3_type_key.l3_key,
					 l3_key, sizeof(tpm_l3_type_key_t));
			if (cmp_res == 0) {	/* found entry */
				/* check the parse_bm */
				if (parse_rule_bm == API_ENT_I(api_section, i).api_rule_data.l3_type_key.parse_rule_bm) {
					*rule_num = API_ENT_I(api_section, i).rule_num;
					return (TPM_DB_OK);
				}
			}
		}
	}
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_api_rulenum_get_from_ipv4_key()
*
* DESCRIPTION:      Get  API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* parse_bm         - fields to be parsed in the key.
* ipv4_key         - structure of ipv4_key
*
* OUTPUTS:
* rule_num         - The API entry number
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_rulenum_get_from_ipv4_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
					     tpm_ipv4_acl_key_t *ipv4_key, uint32_t *rule_num)
{
	int32_t i, cmp_res;

	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID) {
			cmp_res = memcmp(&API_ENT_I(api_section, i).api_rule_data.ipv4_key.ipv4_key,
					 ipv4_key, sizeof(tpm_ipv4_acl_key_t));
			if (cmp_res == 0) {	/* found entry */
				/* check the parse_bm */
				if (parse_rule_bm == API_ENT_I(api_section, i).api_rule_data.ipv4_key.parse_rule_bm) {
					*rule_num = API_ENT_I(api_section, i).rule_num;
					return (TPM_DB_OK);
				}
			}
		}
	}
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_api_rulenum_get_from_ipv6_key()
*
* DESCRIPTION:      Get  API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* parse_bm         - fields to be parsed in the key.
* ipv6_key         - structure of ipv6_key
*
* OUTPUTS:
* rule_num         - The API entry number
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_rulenum_get_from_ipv6_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
					     tpm_ipv6_acl_key_t *ipv6_key, uint32_t *rule_num)
{
	int32_t i, cmp_res;

	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (API_ENT_I(api_section, i).valid == TPM_DB_VALID) {
			cmp_res = memcmp(&API_ENT_I(api_section, i).api_rule_data.ipv6_key.ipv6_key,
					 ipv6_key, sizeof(tpm_ipv6_acl_key_t));
			if (cmp_res == 0) {	/* found entry */
				/* check the parse_bm */
				if (parse_rule_bm == API_ENT_I(api_section, i).api_rule_data.ipv6_key.parse_rule_bm) {
					*rule_num = API_ENT_I(api_section, i).rule_num;
					return (TPM_DB_OK);
				}
			}
		}
	}
	return (TPM_DB_ERR_REC_NOT_EXIST);
}

/*******************************************************************************
* tpm_db_api_entry_get()
*
* DESCRIPTION:      Get  API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* rule_num         - The API entry number
*
* OUTPUTS:
* rule_idx         - Unique Rule Identifier, that does not change over the life span of the rule.
* bi_dir           - Is this rule bi-directional (currently not used)
* api_data         - All config information received for this API call
* mod_con          - Connection to modification tables
* pnc_con          - Connection to PNC
*
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_get(tpm_api_sections_t api_section,
			     uint32_t rule_num,
			     uint32_t *rule_idx,
			     uint32_t *bi_dir,
			     tpm_rule_entry_t *api_data, tpm_db_mod_conn_t *mod_con, tpm_db_pnc_conn_t *pnc_con)
{
	int32_t ind;

	tpm_db_api_entry_ind_get(api_section, rule_num, &ind);
	if (ind == -1)
		return (TPM_DB_ERR_REC_NOT_EXIST);

	memcpy(api_data, &(API_ENT_I(api_section, ind).api_rule_data), sizeof(tpm_rule_entry_t));
	memcpy(mod_con, &(API_ENT_I(api_section, ind).mod_tbl_conn), sizeof(tpm_db_mod_conn_t));
	memcpy(pnc_con, &(API_ENT_I(api_section, ind).pnc_tbl_conn), sizeof(tpm_db_pnc_conn_t));
	*bi_dir = API_ENT_I(api_section, ind).bi_dir;
	*rule_idx = API_ENT_I(api_section, ind).rule_idx;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_api_entry_valid_get_next()
*
* DESCRIPTION:      Get  Next API Entry  Data
*
* INPUTS:
* api_section      - Section of the API acl/table
* cur_rule          - The current rule_num in  the API Range.
*                     Function returns the next valid rule number next_rule  > cur_rule
*
* OUTPUTS:
* next_rule        - See cur_rule. If end of table was reached before valid entry, (-1) is returned in next_rule
*                    All of the following outputs relate to rule_num=next_rule.
* rule_idx         - See tpm_db_api_entry_get
* bi_dir           - See tpm_db_api_entry_get
* api_data         - See tpm_db_api_entry_get
* mod_con          - See tpm_db_api_entry_get
* pnc_con          - See tpm_db_api_entry_get
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_api_entry_val_get_next(tpm_api_sections_t api_section,
				      int32_t cur_rule,
				      int32_t *next_rule,
				      uint32_t *rule_idx,
				      uint32_t *bi_dir,
				      tpm_rule_entry_t *api_data,
				      tpm_db_mod_conn_t *mod_con, tpm_db_pnc_conn_t *pnc_con)
{
	int32_t ind = -1;
	int32_t i;
	uint32_t min_rule, l_next_rule = ~0;
	tpm_db_api_entry_t *api_tbl = &(API_ENT_I(api_section, 0));

	/*TPM_OS_DEBUG(TPM_DB_MOD, "api_section(%d) cur_rule(%d)\n", api_section, cur_rule); */

	if (cur_rule <= -1)
		min_rule = 0;
	else
		min_rule = cur_rule + 1;

	/*TPM_OS_DEBUG(TPM_DB_MOD, "min_rule(%d) \n", min_rule); */

	for (i = 0; i <= tpm_db.api_section[api_section].last_valid_index; i++) {
		if (((api_tbl + i)->valid == TPM_DB_VALID) &&
		    ((api_tbl + i)->rule_num >= min_rule) && ((api_tbl + i)->rule_num < l_next_rule)) {
			l_next_rule = (api_tbl + i)->rule_num;
			ind = i;
		}
	}

	/*TPM_OS_DEBUG(TPM_DB_MOD, "l_next_rule(%d) ind(%d) \n", l_next_rule, ind); */

	if ((l_next_rule < (uint32_t) (~0)) && (ind != -1)) {
		memcpy(api_data, &(API_ENT_I(api_section, ind).api_rule_data), sizeof(tpm_rule_entry_t));
		memcpy(mod_con, &(API_ENT_I(api_section, ind).mod_tbl_conn), sizeof(tpm_db_mod_conn_t));
		memcpy(pnc_con, &(API_ENT_I(api_section, ind).pnc_tbl_conn), sizeof(tpm_db_pnc_conn_t));
		*bi_dir = API_ENT_I(api_section, ind).bi_dir;
		*rule_idx = API_ENT_I(api_section, ind).rule_idx;

		*next_rule = l_next_rule;
	} else		/* Not found */
		*next_rule = -1;

	/*TPM_OS_DEBUG(TPM_DB_MOD, "next_rule(%d) \n", *next_rule); */
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_shdw_ent_set()
*
* DESCRIPTION:      Set PNC  Entry in Shadow Table
*
* INPUTS:
* pnc_entry       - PnC Entry
*
* OUTPUTS:
* pnc_data        - PnC Data
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_shdw_ent_set(uint32_t pnc_entry, tpm_pnc_all_t *pnc_data)
{

	TPM_OS_DEBUG(TPM_DB_MOD, "pnc_entry(%d)\n", pnc_entry);
	if (pnc_entry >= TPM_PNC_SIZE)
		return (TPM_DB_ERR_INV_INPUT);

	tpm_db.pnc_shadow[pnc_entry].valid = TPM_DB_VALID;
	memcpy(&(tpm_db.pnc_shadow[pnc_entry].pnc_data), pnc_data, sizeof(tpm_pnc_all_t));

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_shdw_ent_inv()
*
* DESCRIPTION:      Invalidate PNC  Entry in Shadow Table
*
* INPUTS:
* pnc_entry       - PnC Entry
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_shdw_ent_inv(uint32_t pnc_entry)
{
	if (pnc_entry >= TPM_PNC_SIZE)
		return (TPM_DB_ERR_INV_INPUT);

	memset(&(tpm_db.pnc_shadow[pnc_entry].pnc_data), 0, sizeof(tpm_pnc_all_t));
	tpm_db.pnc_shadow[pnc_entry].valid = TPM_DB_INVALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_shdw_ent_get()
*
* DESCRIPTION:      Get PNC Entry from Shadow Table
*
* INPUTS:
* pnc_entry       - PnC Entry
*
* OUTPUTS:
* valid           - Is the PNC Entry valid in Shadow
* pnc_data        - PnC Data
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_shdw_ent_get(uint32_t pnc_entry, uint32_t *valid, tpm_pnc_all_t *pnc_data)
{
	if (pnc_entry >= TPM_PNC_SIZE)
		return (TPM_DB_ERR_INV_INPUT);

	memcpy(pnc_data, &(tpm_db.pnc_shadow[pnc_entry].pnc_data), sizeof(tpm_pnc_all_t));
	if (tpm_db.pnc_shadow[pnc_entry].valid == TPM_DB_VALID)
		*valid = TPM_TRUE;
	else
		*valid = TPM_FALSE;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mod_shdw_par_check()
*
* DESCRIPTION:      Function checks legality of modification table params
*
* INPUTS:
* gmac            - GMAC In
* entry           - modification entry
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mod_shdw_par_check(tpm_gmacs_enum_t gmac, uint32_t mod_entry)
{
	if ((mod_entry >= TPM_MOD_TABL_SIZE)
	    || /*(gmac < TPM_ENUM_GMAC_0) || */ (gmac >= TPM_MAX_NUM_GMACS))
		return (TPM_DB_ERR_INV_INPUT);

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_pnc_shdw_ent_del()
*
* DESCRIPTION:      Function deletes a PNC entry and "pushes up" the PNC Range
*
* INPUTS:
* pnc_start_entry   -
* pnc_stop_entry    -
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_shdw_ent_del(uint32_t pnc_start_entry, uint32_t pnc_stop_entry)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_DB_MOD, "pnc_start_entry(%d) pnc_stop_entry(%d)\n", pnc_start_entry, pnc_stop_entry);

	if ((pnc_start_entry >= TPM_PNC_SIZE) || (pnc_stop_entry >= TPM_PNC_SIZE))
		return (TPM_DB_ERR_INV_INPUT);

	/* Push up the existing range, except for the first one that is deleted */
	for (i = pnc_start_entry; i < pnc_stop_entry; i++)
		memcpy(&(tpm_db.pnc_shadow[i]), &(tpm_db.pnc_shadow[i + 1]), sizeof(tpm_db_pnc_shadow_t));

	/* Set last entry in range to invalid */
	memset(&(tpm_db.pnc_shadow[pnc_stop_entry].pnc_data), 0, sizeof(tpm_pnc_all_t));
	tpm_db.pnc_shadow[pnc_stop_entry].valid = TPM_DB_INVALID;

	return (TPM_DB_OK);

}

/*******************************************************************************
* tpm_db_pnc_shdw_ent_ins()
*
* DESCRIPTION:      Function inserts an empty entry into a PNC Range ("pushes down the range")
*
* INPUTS:
* pnc_start_entry   -
* pnc_stop_entry    -
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_pnc_shdw_ent_ins(uint32_t pnc_start_entry, uint32_t pnc_stop_entry)
{
	int32_t i;

	TPM_OS_DEBUG(TPM_DB_MOD, "pnc_start_entry(%d) pnc_stop_entry(%d)\n", pnc_start_entry, pnc_stop_entry);

	if ((pnc_start_entry >= (TPM_PNC_SIZE - 1)) || (pnc_stop_entry >= (TPM_PNC_SIZE - 1)))
		return (TPM_DB_ERR_INV_INPUT);

	/* Push down the existing range */
	for (i = (int32_t) pnc_stop_entry; i >= (int32_t) pnc_start_entry; i--)
		memcpy(&(tpm_db.pnc_shadow[i + 1]), &(tpm_db.pnc_shadow[i]), sizeof(tpm_db_pnc_shadow_t));

	/* Set empty (invalid) entry */
	memset(&(tpm_db.pnc_shadow[pnc_start_entry].pnc_data), 0, sizeof(tpm_pnc_all_t));
	tpm_db.pnc_shadow[pnc_start_entry].valid = TPM_DB_INVALID;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mod_etype_sel_get()
*
* DESCRIPTION:      Get select of Vlan Ethertype
*
* INPUTS:
* tpid            - Requested Vlan tpid
*
*
* OUTPUTS:
* etype_sel       - selector that matches this tpid
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mod_etype_sel_get(uint32_t tpid, uint32_t *etype_sel)
{
	uint32_t i;

	if (etype_sel == NULL) {
		TPM_OS_DEBUG(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	for (i = 0; i < TPM_NUM_VLAN_ETYPE_REGS; i++) {
		if (tpm_db.vlan_etype[i].tpid_ether_type == tpid && tpm_db.vlan_etype[i].valid == TPM_DB_VALID) {
			*etype_sel = i;
			return (TPM_DB_OK);
		}
	}
	return (TPM_DB_ERR_REC_EXIST);
}

/*******************************************************************************
* tpm_db_mod_etype_sel_free_get()
*
* DESCRIPTION:      Get an unused tpid selector
*
* INPUTS:
*
* OUTPUTS:
* etype_sel       - selector
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mod_etype_sel_free_get(uint32_t *etype_sel)
{
	uint32_t i;

	if (etype_sel == NULL) {
		TPM_OS_DEBUG(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	for (i = 0; i < TPM_NUM_VLAN_ETYPE_REGS; i++) {
		if (tpm_db.vlan_etype[i].valid == TPM_DB_INVALID) {
			*etype_sel = i;
			return (TPM_DB_OK);
		}
	}

	return (TPM_DB_ERR_DB_TBL_FULL);
}

/*******************************************************************************
* tpm_db_mod_etype_sel_set()
*
* DESCRIPTION:      Set a tpid in a selector
*
* INPUTS:
* tpid            - Requested Vlan tpid
* etype_sel       - selector that matches this tpid
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mod_etype_sel_set(uint32_t tpid, uint32_t etype_sel)
{
	if (etype_sel >= TPM_NUM_VLAN_ETYPE_REGS) {
		TPM_OS_DEBUG(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	tpm_db.vlan_etype[etype_sel].valid = TPM_DB_VALID;
	tpm_db.vlan_etype[etype_sel].tpid_ether_type = tpid;

	return (TPM_DB_OK);
}

void tpm_db_set_mc_filter_mode(uint32_t mode)
{
	tpm_db.igmp_def.filter_mode = mode;
}

uint32_t tpm_db_get_mc_filter_mode(void)
{
	return tpm_db.igmp_def.filter_mode;
}

void tpm_db_set_mc_pppoe_enable(uint32_t enable)
{
	tpm_db.igmp_def.mc_pppoe_enable = enable;
}

uint8_t tpm_db_get_mc_pppoe_enable(void)
{
	return tpm_db.igmp_def.mc_pppoe_enable;
}
void tpm_db_split_mod_set_enable(tpm_db_split_mod_enable_t enable)
{
	tpm_db.split_mod_conf.split_mod_enable = enable;
}

tpm_db_split_mod_enable_t tpm_db_split_mod_get_enable(void)
{
	return tpm_db.split_mod_conf.split_mod_enable;
}

void tpm_db_split_mod_set_num_vlans(uint32_t num_vlans)
{
	/* as default, vlan 0 is added during Init */
	tpm_db.split_mod_conf.num_vlans = (num_vlans + TPM_DB_SPLIT_MOD_INIT_VLANS_NUM);
}

uint32_t tpm_db_split_mod_get_num_vlans(void)
{
	return tpm_db.split_mod_conf.num_vlans;
}
void tpm_db_split_mod_set_mode(tpm_db_split_mod_mode_t mode)
{
	tpm_db.split_mod_conf.split_mod_mode = mode;
}

tpm_db_split_mod_mode_t tpm_db_split_mod_get_mode(void)
{
	return tpm_db.split_mod_conf.split_mod_mode;
}
void tpm_db_split_mod_set_num_p_bits(uint32_t num_p_bits, uint8_t  p_bits[TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX])
{
	tpm_db.split_mod_conf.num_p_bits = num_p_bits;
	memcpy(&(tpm_db.split_mod_conf.p_bits), p_bits, TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX);
}

uint32_t tpm_db_split_mod_get_num_p_bits(void)
{
	return tpm_db.split_mod_conf.num_p_bits;
}
int32_t tpm_db_split_mod_get_p_bits_by_index(uint8_t  index, uint8_t  *p_bits)
{
	if (p_bits == NULL)
		return TPM_DB_ERR_INV_INPUT;

	*p_bits = index;
	return TPM_DB_OK;
}
int32_t tpm_db_split_mod_get_index_by_p_bits(uint8_t  p_bits, uint32_t *index)
{
	uint32_t i;
	for(i = 0; i < tpm_db.split_mod_conf.num_p_bits; i++)
		if (p_bits == tpm_db.split_mod_conf.p_bits[i]) {
			*index = p_bits;
			return TPM_DB_OK;
		}

	return TPM_DB_ERR_REC_NOT_EXIST;
}

#if 0
void tpm_db_set_mc_igmp_mode(uint32_t mode)
{
   tpm_db.igmp_def.igmp_mode = mode;
}

uint32_t tpm_db_get_mc_igmp_mode(void)
{
   return tpm_db.igmp_def.igmp_mode;
}
#endif
void tpm_db_set_mc_per_uni_vlan_xlate(uint32_t enable)
{
	tpm_db.igmp_def.per_uni_vlan_xlat = enable;
}

uint8_t tpm_db_get_mc_per_uni_vlan_xlate(void)
{
	return tpm_db.igmp_def.per_uni_vlan_xlat;
}

void tpm_db_set_mc_hwf_queue(uint8_t hwf_queue)
{
	tpm_db.igmp_def.mc_hwf_queue = hwf_queue;
}

uint8_t tpm_db_get_mc_hwf_queue(void)
{
	return tpm_db.igmp_def.mc_hwf_queue;
}

void tpm_db_set_mc_cpu_queue(uint8_t cpu_queue)
{
	tpm_db.igmp_def.mc_cpu_queue = cpu_queue;
}

uint8_t tpm_db_get_mc_cpu_queue(void)
{
	return tpm_db.igmp_def.mc_cpu_queue;
}

void tpm_db_set_mc_igmp_proxy_sa_mac(uint8_t *sa_mac)
{
	memcpy(tpm_db_mc_igmp_proxy_sa_mac, sa_mac, 6 * sizeof(uint8_t));
	tpm_db_mc_igmp_proxy_sa_mac_valid = 1;
}

void tpm_db_get_mc_igmp_proxy_sa_mac(uint8_t *sa_mac, uint8_t *valid)
{
	*valid = tpm_db_mc_igmp_proxy_sa_mac_valid;
	if (tpm_db_mc_igmp_proxy_sa_mac_valid)
		memcpy(sa_mac, tpm_db_mc_igmp_proxy_sa_mac, 6 * sizeof(uint8_t));
}

void tpm_db_set_mc_lpbk_enable(tpm_ip_ver_t ver, uint8_t enable)
{
	if (ver < TPM_IP_VER_MAX)
		tpm_db.igmp_def.mc_lpbk_enable[ver] = enable;
}

void tpm_db_get_mc_lpbk_enable(tpm_ip_ver_t ver, uint8_t *enable)
{
	if (ver < TPM_IP_VER_MAX)
		*enable = tpm_db.igmp_def.mc_lpbk_enable[ver];
}

int32_t tpm_db_set_mc_stream_entry(uint32_t stream_num, tpm_db_mc_stream_entry_t *mc_stream)
{
	if (tpm_db_mc_stream_table[stream_num] == NULL) {
		tpm_db_mc_stream_table[stream_num] =
		/*(tpm_db_mc_stream_entry_t *)*/ kmalloc(sizeof(tpm_db_mc_stream_entry_t), GFP_KERNEL);
		if (tpm_db_mc_stream_table[stream_num] == NULL)
			return TPM_DB_ERR_MEM_ALLOC_FAIL;

		memcpy(tpm_db_mc_stream_table[stream_num], mc_stream, sizeof(tpm_db_mc_stream_entry_t));
	} else {
		if (tpm_db_mc_stream_table[stream_num]->igmp_mode == mc_stream->igmp_mode &&
		    tpm_db_mc_stream_table[stream_num]->mc_stream_pppoe == mc_stream->mc_stream_pppoe &&
		    tpm_db_mc_stream_table[stream_num]->vid == mc_stream->vid &&
		    tpm_db_mc_stream_table[stream_num]->src_valid == mc_stream->src_valid &&
		    !memcmp(tpm_db_mc_stream_table[stream_num]->src_addr, mc_stream->src_addr, 4 * sizeof(uint8_t)) &&
		    !memcmp(tpm_db_mc_stream_table[stream_num]->group_addr, mc_stream->group_addr, 4 * sizeof(uint8_t))) {
			tpm_db_mc_stream_table[stream_num]->dest_port_bm = mc_stream->dest_port_bm;
			tpm_db_mc_stream_table[stream_num]->u4_entry = mc_stream->u4_entry;
		} else
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	return TPM_DB_OK;
}

int32_t tpm_db_get_mc_stream_entry(uint32_t stream_num, tpm_db_mc_stream_entry_t *mc_stream)
{
	if (tpm_db_mc_stream_table[stream_num] == NULL)
		return TPM_DB_ERR_REC_NOT_EXIST;

	memcpy(mc_stream, tpm_db_mc_stream_table[stream_num], sizeof(tpm_db_mc_stream_entry_t));
	return TPM_DB_OK;
}

void tpm_db_reset_mc_stream_entry(uint32_t stream_num)
{
	if (tpm_db_mc_stream_table[stream_num] != NULL) {
		kfree(tpm_db_mc_stream_table[stream_num]);
		tpm_db_mc_stream_table[stream_num] = NULL;
	}
}

int32_t tpm_db_set_ipv6_mc_stream_entry(uint32_t stream_num, tpm_db_ipv6_mc_stream_entry_t *mc_stream)
{
	if (0 == tpm_db.ipv6_mc_stream[stream_num].valid) {
		memcpy(&(tpm_db.ipv6_mc_stream[stream_num]), mc_stream, sizeof(tpm_db_ipv6_mc_stream_entry_t));
		tpm_db.ipv6_mc_stream[stream_num].valid = 1;
	} else {
		if (tpm_db.ipv6_mc_stream[stream_num].igmp_mode == mc_stream->igmp_mode &&
			tpm_db.ipv6_mc_stream[stream_num].mc_stream_pppoe == mc_stream->mc_stream_pppoe &&
			tpm_db.ipv6_mc_stream[stream_num].vid == mc_stream->vid &&
			!memcmp(tpm_db.ipv6_mc_stream[stream_num].group_addr, mc_stream->group_addr, 16 * sizeof(uint8_t))) {
			tpm_db.ipv6_mc_stream[stream_num].dest_port_bm = mc_stream->dest_port_bm;
			tpm_db.ipv6_mc_stream[stream_num].u4_entry = mc_stream->u4_entry;
		} else
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv6_mc_stream_entry(uint32_t stream_num, tpm_db_ipv6_mc_stream_entry_t *mc_stream)
{
	if (0 == tpm_db.ipv6_mc_stream[stream_num].valid)
		return TPM_DB_ERR_REC_NOT_EXIST;

	memcpy(mc_stream, &(tpm_db.ipv6_mc_stream[stream_num]), sizeof(tpm_db_ipv6_mc_stream_entry_t));
	return TPM_DB_OK;
}

void tpm_db_reset_ipv6_mc_stream_entry(uint32_t stream_num)
{
	memset(&(tpm_db.ipv6_mc_stream[stream_num]), 0, sizeof(tpm_db_ipv6_mc_stream_entry_t));
}

int32_t tpm_db_increase_mc_mac_port_user_num(uint8_t *mac_addr, uint32_t uni_port)
{
	uint32_t entry_id, first_free = TPM_MC_MAX_MAC_NUM;
	uint32_t port_nr;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MAC_NUM; entry_id++) {
		if (tpm_db_mc_mac_table[entry_id] != NULL) {
			if (!memcmp(tpm_db_mc_mac_table[entry_id]->mac_addr, mac_addr, 6 * sizeof(uint8_t)))
				break;
		} else if (first_free == TPM_MC_MAX_MAC_NUM)
			first_free = entry_id;
	}

	if (entry_id == TPM_MC_MAX_MAC_NUM) {
		if (first_free == TPM_MC_MAX_MAC_NUM)
			return TPM_DB_ERR_DB_TBL_FULL;

		tpm_db_mc_mac_table[first_free] =
		/*(tpm_db_mc_mac_entry_t *)*/ kmalloc(sizeof(tpm_db_mc_mac_entry_t), GFP_KERNEL);
		if (tpm_db_mc_mac_table[first_free] == NULL)
			return TPM_DB_ERR_MEM_ALLOC_FAIL;

		memset(tpm_db_mc_mac_table[first_free], 0, sizeof(tpm_db_mc_mac_entry_t));
		memcpy(tpm_db_mc_mac_table[first_free]->mac_addr, mac_addr, 6 * sizeof(uint8_t));
		entry_id = first_free;
	}

	switch (uni_port) {
	case TPM_TRG_UNI_0:	port_nr = TPM_SRC_PORT_UNI_0;	break;
	case TPM_TRG_UNI_1:	port_nr = TPM_SRC_PORT_UNI_1;	break;
	case TPM_TRG_UNI_2:	port_nr = TPM_SRC_PORT_UNI_2;	break;
	case TPM_TRG_UNI_3:	port_nr = TPM_SRC_PORT_UNI_3;	break;
	case TPM_TRG_UNI_4:	port_nr = TPM_SRC_PORT_UNI_4;	break;
	case TPM_TRG_UNI_5:	port_nr = TPM_SRC_PORT_UNI_5;	break;
	case TPM_TRG_UNI_6:	port_nr = TPM_SRC_PORT_UNI_6;	break;
	case TPM_TRG_UNI_7:	port_nr = TPM_SRC_PORT_UNI_7;	break;
	case TPM_TRG_UNI_VIRT:	port_nr = TPM_SRC_PORT_UNI_VIRT;break;
	default:
		return TPM_DB_ERR_INV_INPUT;
	}
	tpm_db_mc_mac_table[entry_id]->user_num[port_nr] += 1;

	return TPM_DB_OK;
}

int32_t tpm_db_decrease_mc_mac_port_user_num(uint8_t *mac_addr, uint32_t uni_port)
{
	uint32_t entry_id;
	uint32_t port_nr;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MAC_NUM; entry_id++) {
		if (tpm_db_mc_mac_table[entry_id] != NULL) {
			if (!memcmp(tpm_db_mc_mac_table[entry_id]->mac_addr, mac_addr, 6 * sizeof(uint8_t)))
				break;
		}
	}

	if (entry_id == TPM_MC_MAX_MAC_NUM)
		return TPM_DB_ERR_REC_NOT_EXIST;

	switch (uni_port) {
	case TPM_TRG_UNI_0:	port_nr = TPM_SRC_PORT_UNI_0;	break;
	case TPM_TRG_UNI_1:	port_nr = TPM_SRC_PORT_UNI_1;	break;
	case TPM_TRG_UNI_2:	port_nr = TPM_SRC_PORT_UNI_2;	break;
	case TPM_TRG_UNI_3:	port_nr = TPM_SRC_PORT_UNI_3;	break;
	case TPM_TRG_UNI_4:	port_nr = TPM_SRC_PORT_UNI_4;	break;
	case TPM_TRG_UNI_5:	port_nr = TPM_SRC_PORT_UNI_5;	break;
	case TPM_TRG_UNI_6:	port_nr = TPM_SRC_PORT_UNI_6;	break;
	case TPM_TRG_UNI_7:	port_nr = TPM_SRC_PORT_UNI_7;	break;
	case TPM_TRG_UNI_VIRT:	port_nr = TPM_SRC_PORT_UNI_VIRT;break;
	default:
		return TPM_DB_ERR_INV_INPUT;
	}
	tpm_db_mc_mac_table[entry_id]->user_num[port_nr] -= 1;

	return TPM_DB_OK;
}

uint8_t tpm_db_get_mc_mac_port_user_num(uint8_t *mac_addr, uint32_t uni_port)
{
	uint32_t entry_id;
	uint32_t port_nr;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MAC_NUM; entry_id++) {
		if (tpm_db_mc_mac_table[entry_id] != NULL) {
			if (!memcmp(tpm_db_mc_mac_table[entry_id]->mac_addr, mac_addr, 6 * sizeof(uint8_t)))
				break;
		}
	}

	if (entry_id == TPM_MC_MAX_MAC_NUM)
		return TPM_DB_ERR_REC_NOT_EXIST;

	switch (uni_port) {
	case TPM_TRG_UNI_0:	port_nr = TPM_SRC_PORT_UNI_0;	break;
	case TPM_TRG_UNI_1:	port_nr = TPM_SRC_PORT_UNI_1;	break;
	case TPM_TRG_UNI_2:	port_nr = TPM_SRC_PORT_UNI_2;	break;
	case TPM_TRG_UNI_3:	port_nr = TPM_SRC_PORT_UNI_3;	break;
	case TPM_TRG_UNI_4:	port_nr = TPM_SRC_PORT_UNI_4;	break;
	case TPM_TRG_UNI_5:	port_nr = TPM_SRC_PORT_UNI_5;	break;
	case TPM_TRG_UNI_6:	port_nr = TPM_SRC_PORT_UNI_6;	break;
	case TPM_TRG_UNI_7:	port_nr = TPM_SRC_PORT_UNI_7;	break;
	case TPM_TRG_UNI_VIRT:	port_nr = TPM_SRC_PORT_UNI_VIRT;break;
	default:
		return 0;
	}

	if (TRG_UNI(uni_port)) {
		if (port_nr < TPM_MAX_NUM_UNI_PORTS)
			return tpm_db_mc_mac_table[entry_id]->user_num[port_nr];
	}

	return 0;
}

void tpm_db_reset_mc_mac_entry(uint8_t *mac_addr)
{
	uint32_t entry_id;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MAC_NUM; entry_id++) {
		if (tpm_db_mc_mac_table[entry_id] != NULL) {
			if (!memcmp(tpm_db_mc_mac_table[entry_id]->mac_addr, mac_addr, 6 * sizeof(uint8_t))) {
				kfree(tpm_db_mc_mac_table[entry_id]);
				tpm_db_mc_mac_table[entry_id] = NULL;
				return;
			}
		}
	}
}

uint32_t tpm_db_mc_lpbk_entries_num_get(void)
{
	return tpm_db_mc_lpbk_entries_num;
}

int32_t tpm_db_mc_lpbk_entry_set(uint32_t ip_ver, uint16_t mvlan, uint16_t in_vid, uint16_t out_vid, uint32_t rule_num,
				 uint32_t mod_entry)
{
	static uint32_t _rule_index = 1000;
	uint32_t entry_id, first_free = TPM_MC_MAX_MVLAN_XITS_NUM;
	tpm_db_mc_lpbk_entry_t *lpbk_entry = NULL;
	tpm_db_mc_vlan_entry_t *mvlan_entry = NULL;

	if (rule_num >= TPM_MC_MAX_LPBK_ENTRIES_NUM)
		return TPM_DB_ERR_DB_TBL_FULL;

	if (rule_num > tpm_db_mc_lpbk_entries_num)
		return TPM_DB_ERR_INV_INPUT;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MVLAN_XITS_NUM; entry_id++) {
		if (tpm_db_mc_vlan_xits_table[entry_id] != NULL) {
			if (mvlan == tpm_db_mc_vlan_xits_table[entry_id]->mvlan &&
				ip_ver == tpm_db_mc_vlan_xits_table[entry_id]->ip_ver) {
				mvlan_entry = tpm_db_mc_vlan_xits_table[entry_id];
				break;
			}
		} else if (first_free == TPM_MC_MAX_MVLAN_XITS_NUM)
			first_free = entry_id;
	}

	if (entry_id == TPM_MC_MAX_MVLAN_XITS_NUM) {
		if (first_free == TPM_MC_MAX_MVLAN_XITS_NUM)
			return TPM_DB_ERR_DB_TBL_FULL;

		mvlan_entry = /*(tpm_db_mc_vlan_entry_t *)*/ kmalloc(sizeof(tpm_db_mc_vlan_entry_t), GFP_KERNEL);
		if (mvlan_entry == NULL)
			return TPM_DB_ERR_MEM_ALLOC_FAIL;

		memset(mvlan_entry, 0, sizeof(tpm_db_mc_vlan_entry_t));
		mvlan_entry->mvlan = mvlan;
		mvlan_entry->ip_ver = ip_ver;

		tpm_db_mc_vlan_xits_table[first_free] = mvlan_entry;
	}

	first_free = TPM_DB_MAX_TRANSITION_NUM;

	for (entry_id = 0; entry_id < TPM_DB_MAX_TRANSITION_NUM; entry_id++) {
		if (mvlan_entry->xits[entry_id].valid == 0) {
			first_free = entry_id;
			break;
		}
	}

	if (first_free == TPM_DB_MAX_TRANSITION_NUM)
		return TPM_DB_ERR_DB_TBL_FULL;

	lpbk_entry = /*(tpm_db_mc_lpbk_entry_t *)*/ kmalloc(sizeof(tpm_db_mc_lpbk_entry_t), GFP_KERNEL);
	if (lpbk_entry == NULL)
		return TPM_DB_ERR_MEM_ALLOC_FAIL;

	memset(lpbk_entry, 0, sizeof(tpm_db_mc_lpbk_entry_t));

	mvlan_entry->xits[first_free].valid = 1;
	mvlan_entry->xits[first_free].in_vid = in_vid;
	mvlan_entry->xits[first_free].out_vid = out_vid;
	mvlan_entry->xits[first_free].lpbk_entry = lpbk_entry;
	mvlan_entry->xits_num++;

	_rule_index++;

	lpbk_entry->rule_index = _rule_index;
	lpbk_entry->mod_cmd = mod_entry;

	for (entry_id = tpm_db_mc_lpbk_entries_num; entry_id > rule_num; entry_id--)
		tpm_db_mc_lpbk_table[entry_id] = tpm_db_mc_lpbk_table[entry_id - 1];

	tpm_db_mc_lpbk_table[rule_num] = lpbk_entry;
	tpm_db_mc_lpbk_entries_num++;

	return TPM_DB_OK;
}

int32_t tpm_db_mc_vlan_xits_num_get(uint32_t ip_ver, uint16_t mvlan, uint32_t *xits_num)
{
	uint32_t entry_id;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MVLAN_XITS_NUM; entry_id++) {
		if (tpm_db_mc_vlan_xits_table[entry_id] != NULL) {
			if (tpm_db_mc_vlan_xits_table[entry_id]->mvlan == mvlan &&
				tpm_db_mc_vlan_xits_table[entry_id]->ip_ver == ip_ver) {
				*xits_num = tpm_db_mc_vlan_xits_table[entry_id]->xits_num;
				return TPM_DB_OK;
			}
		}
	}

	return TPM_DB_ERR_REC_NOT_EXIST;
}

int32_t tpm_db_mc_vlan_xit_entry_get(uint32_t ip_ver, uint16_t mvlan, uint32_t xit_id, uint32_t *rule_index, uint32_t *mod_entry)
{
	uint32_t entry_id, xit_cnt = 0;
	uint8_t found;
	tpm_db_mc_vlan_entry_t *mvlan_entry = NULL;
	tpm_db_mc_lpbk_entry_t *lpbk_entry = NULL;

	for (entry_id = 0; entry_id < TPM_MC_MAX_MVLAN_XITS_NUM; entry_id++) {
		if (tpm_db_mc_vlan_xits_table[entry_id] != NULL) {
			if (tpm_db_mc_vlan_xits_table[entry_id]->mvlan == mvlan &&
				tpm_db_mc_vlan_xits_table[entry_id]->ip_ver == ip_ver) {
				mvlan_entry = tpm_db_mc_vlan_xits_table[entry_id];
				break;
			}
		}
	}

	if (mvlan_entry == NULL)
		return TPM_DB_ERR_REC_NOT_EXIST;

	found = 0;
	for (entry_id = 0; entry_id < TPM_DB_MAX_TRANSITION_NUM; entry_id++) {
		if (xit_cnt == xit_id) {
			found = 1;
			break;
		}
		xit_cnt++;
	}

	if (found == 0)
		return TPM_DB_ERR_REC_NOT_EXIST;

	lpbk_entry = mvlan_entry->xits[entry_id].lpbk_entry;

	*rule_index = lpbk_entry->rule_index;
	*mod_entry = lpbk_entry->mod_cmd;

	return TPM_DB_OK;
}

int32_t tpm_db_mc_lpbk_rule_num_get(uint32_t rule_index, uint32_t *rule_num)
{
	uint32_t entry_id;

	for (entry_id = 0; entry_id < tpm_db_mc_lpbk_entries_num; entry_id++) {
		if (tpm_db_mc_lpbk_table[entry_id] != NULL) {
			if (tpm_db_mc_lpbk_table[entry_id]->rule_index == rule_index) {
				*rule_num = entry_id;
				return TPM_DB_OK;
			}
		}
	}

	return TPM_DB_ERR_REC_NOT_EXIST;
}

int32_t tpm_db_mc_lpbk_entry_invalidate(uint32_t rule_num)
{
	uint32_t entry_id, xit_id;
	uint8_t found;
	tpm_db_mc_lpbk_entry_t *lpbk_entry = NULL;
	tpm_db_mc_vlan_entry_t *mvlan_entry = NULL;

	if (rule_num > tpm_db_mc_lpbk_entries_num)
		return TPM_DB_ERR_INV_INPUT;

	lpbk_entry = tpm_db_mc_lpbk_table[rule_num];

	for (entry_id = rule_num; entry_id < (tpm_db_mc_lpbk_entries_num - 1); entry_id++)
		tpm_db_mc_lpbk_table[entry_id] = tpm_db_mc_lpbk_table[entry_id + 1];

	kfree(lpbk_entry);
	tpm_db_mc_lpbk_table[tpm_db_mc_lpbk_entries_num - 1] = NULL;
	tpm_db_mc_lpbk_entries_num--;

	found = 0;
	for (entry_id = 0; entry_id < TPM_MC_MAX_MVLAN_XITS_NUM; entry_id++) {
		mvlan_entry = tpm_db_mc_vlan_xits_table[entry_id];
		if (mvlan_entry != NULL) {
			if (mvlan_entry->xits_num > 0) {
				for (xit_id = 0; xit_id < TPM_DB_MAX_TRANSITION_NUM; xit_id++) {
					if (mvlan_entry->xits[xit_id].valid) {
						if (lpbk_entry == mvlan_entry->xits[xit_id].lpbk_entry) {
							mvlan_entry->xits[xit_id].valid = 0;
							found = 1;
							break;
						}
					}
				}

				if (found) {
					mvlan_entry->xits_num--;
					if (mvlan_entry->xits_num == 0) {
						kfree(mvlan_entry);
						tpm_db_mc_vlan_xits_table[entry_id] = NULL;
					}
					break;
				}
			}
		}
	}

	return TPM_DB_OK;
}

int32_t tpm_db_mc_alloc_virt_uni_entry(uint32_t *entry_id)
{
	uint32_t free;

	for (free = 0; free < TPM_MC_MAX_STREAM_NUM; free++) {
		if (tpm_db_mc_virt_uni_entry_state_table[free] == 0) {
			tpm_db_mc_virt_uni_entry_state_table[free] = 1;
			*entry_id = free;
			return TPM_DB_OK;
		}
	}

	return TPM_DB_ERR_DB_TBL_FULL;
}

int32_t tpm_db_mc_free_virt_uni_entry(uint32_t entry_id)
{
	if (entry_id >= TPM_MC_MAX_STREAM_NUM)
		return TPM_DB_ERR_REC_INV;

	tpm_db_mc_virt_uni_entry_state_table[entry_id] = 0;
	return TPM_DB_OK;
}

void tpm_db_mc_rule_reset(void)
{
	uint32_t i;

	/* Reset multicast stream table */
	for (i = 0; i < TPM_MC_MAX_STREAM_NUM; i++) {
		if (tpm_db_mc_stream_table[i] != NULL) {
			kfree(tpm_db_mc_stream_table[i]);
			tpm_db_mc_stream_table[i] = NULL;
		}
	}

	/* Reset multicast mac table */
	for (i = 0; i < TPM_MC_MAX_MAC_NUM; i++) {
		if (tpm_db_mc_mac_table[i] != NULL) {
			kfree(tpm_db_mc_mac_table[i]);
			tpm_db_mc_mac_table[i] = NULL;
		}
	}

	/* Reset multicast loopback tables */
	tpm_db_mc_lpbk_entries_num = 0;
	for (i = 0; i < TPM_MC_MAX_LPBK_ENTRIES_NUM; i++) {
		if (tpm_db_mc_lpbk_table[i] != NULL) {
			kfree(tpm_db_mc_lpbk_table[i]);
			tpm_db_mc_lpbk_table[i] = NULL;
		}
	}

	for (i = 0; i < TPM_MC_MAX_MVLAN_XITS_NUM; i++) {
		if (tpm_db_mc_vlan_xits_table[i] != NULL) {
			kfree(tpm_db_mc_vlan_xits_table[i]);
			tpm_db_mc_vlan_xits_table[i] = NULL;
		}
	}

	/* Reset state table of virt_uni entries */
	memset(tpm_db_mc_virt_uni_entry_state_table, 0, TPM_MC_MAX_STREAM_NUM * sizeof(uint8_t));

	/* reset IPv6 MC rule */
	memset(tpm_db.ipv6_mc_stream, 0, sizeof(tpm_db.ipv6_mc_stream));

}
int32_t tpm_db_mc_vlan_get_ai_bit_by_vid(uint32_t mc_vlan, uint32_t *ai_bit)
{
	uint32_t i;
	int32_t ret_val;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vlan(%d) \n", mc_vlan);

	/* first, get ai bits if mc vlan already exists */
	ret_val = tpm_db_mc_vlan_get_ai_bit(mc_vlan, ai_bit);
	if (TPM_DB_OK == ret_val) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc vlan already exists, ai bits(%d)\n", *ai_bit);
		return TPM_DB_OK;
	}

	/* alloc a new ai bits */
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if (!tpm_db.mc_ai_bits_table[i].valid) {
			tpm_db.mc_ai_bits_table[i].valid = true;
			tpm_db.mc_ai_bits_table[i].mc_vlan = mc_vlan;
			tpm_db.mc_ai_bits_table[i].pnc_index_num = 0;
			memset(tpm_db.mc_ai_bits_table[i].pnc_index, 0xff,
			       sizeof(tpm_db.mc_ai_bits_table[i].pnc_index));
			*ai_bit = i;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " alloc a new ai bits(%d)\n", i);
			return TPM_DB_OK;
		}
	}
	return (TPM_DB_ERR_DB_TBL_FULL);
}

bool tpm_db_mc_vlan_l2_rule_exist(uint32_t mc_vlan, uint32_t rule_num)
{
	uint32_t loop_in;
	uint32_t i;
	uint32_t ret_code;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pnc_range_conf_t rangConf;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vlan(%d), rule_num(%d) \n", mc_vlan, rule_num);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_L2_PRIM_ACL, &range_id);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	if (TPM_OK != ret_code) {
		TPM_OS_ERROR(TPM_DB_MOD, " recvd ret_code(%d)\n", ret_code);
		return false;
	}

	if (TPM_RANGE_TYPE_TABLE != rangConf.range_type) {
		/* not TABLE mode */
		return false;
	}

	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((mc_vlan == tpm_db.mc_ai_bits_table[i].mc_vlan) && (tpm_db.mc_ai_bits_table[i].valid)) {
			for (loop_in = 0; loop_in < TPM_MC_VLAN_MAX_PNC_RULE_NUM; loop_in++) {
				if (rule_num == tpm_db.mc_ai_bits_table[i].pnc_index[loop_in])
					return true;
			}
		}
	}
	return false;
}

int32_t tpm_db_mc_vlan_set_ai_bit_pnc_index(uint32_t mc_vlan, uint32_t l2_pnc_rule_index)
{
	uint32_t i;
	uint32_t loop_in;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vlan(%d) l2_pnc_rule_index(%d)\n", mc_vlan, l2_pnc_rule_index);

	/* in table mode, and rule_num already exist, do not record again */
	if (tpm_db_mc_vlan_l2_rule_exist(mc_vlan, l2_pnc_rule_index))
		return TPM_DB_OK;

	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((mc_vlan == tpm_db.mc_ai_bits_table[i].mc_vlan) && (tpm_db.mc_ai_bits_table[i].valid)) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " ai_index(%d)\n", i);
			for (loop_in = 0; loop_in < TPM_MC_VLAN_MAX_PNC_RULE_NUM; loop_in++) {
				if (~0 == tpm_db.mc_ai_bits_table[i].pnc_index[loop_in]) {
					tpm_db.mc_ai_bits_table[i].pnc_index[loop_in] = l2_pnc_rule_index;
					tpm_db.mc_ai_bits_table[i].pnc_index_num++;
					return TPM_DB_OK;
				}
			}
			return TPM_DB_ERR_DB_TBL_FULL;
		}
	}
	return (TPM_DB_ERR_DB_TBL_FULL);
}

int32_t tpm_db_mc_vlan_get_pnc_index_free_slot(uint32_t mc_vlan, uint32_t rule_num)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vlan(%d) \n", mc_vlan);

	/* in table mode, and rule_num already exist, always has free slot */
	if (tpm_db_mc_vlan_l2_rule_exist(mc_vlan, rule_num))
		return 1;

	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((mc_vlan == tpm_db.mc_ai_bits_table[i].mc_vlan) && (tpm_db.mc_ai_bits_table[i].valid)) {

			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " ai_index(%d), pnc_index_num(%d)\n",
				i, tpm_db.mc_ai_bits_table[i].pnc_index_num);
			/* ACL mode */
			if (TPM_MC_VLAN_MAX_PNC_RULE_NUM <= tpm_db.mc_ai_bits_table[i].pnc_index_num)
				return 0;
			return (TPM_MC_VLAN_MAX_PNC_RULE_NUM - tpm_db.mc_ai_bits_table[i].pnc_index_num);
		}
	}
	return TPM_MC_VLAN_MAX_PNC_RULE_NUM;
}

int32_t tpm_db_mc_vlan_reset_ai_bit(uint32_t l2_pnc_rule_index)
{
	uint32_t i;
	uint32_t loop_in;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "l2_pnc_rule_index(%d)\n", l2_pnc_rule_index);
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if (!tpm_db.mc_ai_bits_table[i].valid)
			continue;

		for (loop_in = 0; loop_in < TPM_MC_VLAN_MAX_PNC_RULE_NUM; loop_in++) {
			if (l2_pnc_rule_index == tpm_db.mc_ai_bits_table[i].pnc_index[loop_in]) {
				tpm_db.mc_ai_bits_table[i].pnc_index_num--;
				tpm_db.mc_ai_bits_table[i].pnc_index[loop_in] = ~0;

				if (0 == tpm_db.mc_ai_bits_table[i].pnc_index_num) {
					TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " remove this mc vlan(%d), ai_bits(%d)\n",
						     tpm_db.mc_ai_bits_table[i].mc_vlan, i);
					tpm_db.mc_ai_bits_table[i].valid = false;
					tpm_db.mc_ai_bits_table[i].mc_vlan = 0;
					memset(tpm_db.mc_ai_bits_table[i].pnc_index, 0xff,
					       sizeof(tpm_db.mc_ai_bits_table[i].pnc_index));
				} else {
					TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
						     " decrease pnc_index_num of this mc vlan(%d), pnc_index_num(%d)\n",
						     tpm_db.mc_ai_bits_table[i].mc_vlan,
						     tpm_db.mc_ai_bits_table[i].pnc_index_num);
				}

				return TPM_DB_OK;
			}
		}
		continue;
	}
	return (TPM_DB_ERR_DB_TBL_FULL);
}

int32_t tpm_db_mc_vlan_reset_mc_vlan(uint32_t mc_vlan)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vlan);
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((tpm_db.mc_ai_bits_table[i].valid) && (mc_vlan == tpm_db.mc_ai_bits_table[i].mc_vlan)) {
			if (0 == tpm_db.mc_ai_bits_table[i].pnc_index_num) {
				tpm_db.mc_ai_bits_table[i].valid = false;
				tpm_db.mc_ai_bits_table[i].mc_vlan = 0;
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " reset this vlan(%d)\n", mc_vlan);
			}
			return TPM_DB_OK;
		}
	}
	return (TPM_DB_ERR_DB_TBL_FULL);
}

int32_t tpm_db_mc_vlan_get_ai_bit(uint32_t mc_vlan, uint32_t *ai_bit)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vlan);
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if (mc_vlan == tpm_db.mc_ai_bits_table[i].mc_vlan) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " ai_index(%d)\n", i);
			*ai_bit = i;
			return TPM_DB_OK;
		}
	}
	return (TPM_DB_ERR_INV_INPUT);
}

int32_t tpm_db_set_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if (false == tpm_db.mc_vid_port_cfg[i].valid) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vid_index(%d)\n", i);
			tpm_db.mc_vid_port_cfg[i].valid = true;
			tpm_db.mc_vid_port_cfg[i].mc_vid = mc_vid;
			memcpy(&tpm_db.mc_vid_port_cfg[i].mc_vid_port_vids, mc_vid_uniports_config,
			       sizeof(tpm_mc_vid_port_vid_set_t));
			return TPM_DB_OK;
		}
	}
	return (TPM_DB_ERR_INV_INPUT);
}

int32_t tpm_db_get_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_cfg_t **mc_vid_uniports_config)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);

	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((true == tpm_db.mc_vid_port_cfg[i].valid) && (mc_vid == tpm_db.mc_vid_port_cfg[i].mc_vid)) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vid_index(%d)\n", i);
			*mc_vid_uniports_config = &(tpm_db.mc_vid_port_cfg[i].mc_vid_port_vids[0]);
			return TPM_DB_OK;
		}
	}

	return (TPM_DB_ERR_INV_INPUT);
}

bool tpm_db_mc_vid_exist(uint32_t mc_vid)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((true == tpm_db.mc_vid_port_cfg[i].valid) && (mc_vid == tpm_db.mc_vid_port_cfg[i].mc_vid)) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vid_index(%d)\n", i);
			return true;
		}
	}
	return (false);
}

uint32_t tpm_db_remove_mc_vid_cfg(uint32_t mc_vid)
{
	uint32_t i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if ((true == tpm_db.mc_vid_port_cfg[i].valid) && (mc_vid == tpm_db.mc_vid_port_cfg[i].mc_vid)) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mc_vid_index(%d)\n", i);
			tpm_db.mc_vid_port_cfg[i].valid = false;
			tpm_db.mc_vid_port_cfg[i].mc_vid = 0;
			return TPM_DB_OK;
		}
	}
	return (TPM_DB_ERR_INV_INPUT);
}
/*******************************************************************************
* tpm_db_mc_cfg_reset()
*
* DESCRIPTION:      Reset MC configuration part in DB. Set all structures to 0,
*                   and all valid fields to false
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_db_mc_cfg_reset(void)
{
    memset(&tpm_db.mc_ai_bits_table, 0, sizeof(tpm_db.mc_ai_bits_table));
    memset(&tpm_db.mc_vid_port_cfg, 0, sizeof(tpm_db.mc_vid_port_cfg));

    tpm_db_mc_igmp_proxy_sa_mac_valid = 0;
    memset(tpm_db_mc_igmp_proxy_sa_mac, 0, 6 * sizeof(uint8_t));

    return;
}

void tpm_db_mod2_init_jump_area_cfg(void)
{
	tpm_db_mod2_jump_area_cfg.base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_jump_area_cfg.total_num = 0;
	tpm_db_mod2_jump_area_cfg.split_num = 0;

	tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_PMAC] = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_jump_area_cfg.next_split[TPM_ENUM_GMAC_0] = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_jump_area_cfg.next_split[TPM_ENUM_GMAC_1] = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_jump_area_cfg.next_split[TPM_ENUM_PMAC] = TPM_MOD2_NULL_ENT_IDX;

	tpm_db_mod2_jump_area_cfg.split_pbit_num = 0;
}

void tpm_db_mod2_init_chain_cfg(void)
{
	tpm_mod2_multicast_mh_enable = 1;
	tpm_mod2_pppoe_add_mod_enable = 1;
	tpm_mod2_double_tag_enable = 1;
	tpm_mod2_udp_checksum_use_init_bm_enable = 0;
	tpm_mod2_udp_checksum_enable = 1;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].subr_chain = 1;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].max_size = TPM_MOD2_MAX_MH_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].subr_chain = 1;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].max_size = TPM_MOD2_MAX_MAC_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].subr_chain = 1;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].max_size = TPM_MOD2_MAX_VLAN_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].subr_chain = 1;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].max_size = TPM_MOD2_MAX_PPPOE_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].subr_chain = 1;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].max_size = TPM_MOD2_MAX_IPV6_PPPOE_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].subr_chain = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].max_size = TPM_MOD2_MAX_L2_TWO_TAG_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].subr_chain = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].max_size = TPM_MOD2_MAX_IPV4_NAPT_W_PPPOE_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].subr_chain = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].max_size = TPM_MOD2_MAX_IPV4_MC_W_MH_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].subr_chain = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].max_size = TPM_MOD2_MAX_ROUTE_W_PPPOE_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].base_entry = TPM_MOD2_NULL_ENT_IDX;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].subr_chain = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].max_size = TPM_MOD2_MAX_IPV6_MC_W_MH_CHAIN_SIZE;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num = 0;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].next_free[TPM_ENUM_GMAC_0] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].next_free[TPM_ENUM_GMAC_1] = TPM_MOD2_INVALID_CHAIN_ID;
	tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].next_free[TPM_ENUM_PMAC] = TPM_MOD2_INVALID_CHAIN_ID;

}

void tpm_db_mod2_init_pmt_info(void)
{
	tpm_mod2_chain_area_base = TPM_MOD2_NULL_ENT_IDX;
	tpm_mod2_total_chain_num = 0;

	memset(tpm_db_mod2_jump_pmt_info, 0, TPM_MAX_NUM_GMACS * sizeof(tpm_mod2_jump_pmt_info_t *));
	memset(tpm_db_mod2_chain_pmt_info, 0, TPM_MAX_NUM_GMACS * sizeof(tpm_mod2_jump_pmt_info_t *));
}

void tpm_db_mod2_init_chain_info(void)
{
	memset(tpm_db_mod2_jump_booked_entry_bm, 0, TPM_MAX_NUM_GMACS * sizeof(uint32_t *));
	memset(tpm_db_mod2_jump_occupied_entry_bm, 0, TPM_MAX_NUM_GMACS * sizeof(uint32_t *));

	tpm_db_mod2_jump_bm_group = 0;

	memset(tpm_db_mod2_chain_booked_entry_bm, 0, TPM_MAX_NUM_GMACS * TPM_CHAIN_TYPE_MAX * sizeof(uint32_t *));
	memset(tpm_db_mod2_chain_occupied_entry_bm, 0, TPM_MAX_NUM_GMACS * TPM_CHAIN_TYPE_MAX * sizeof(uint32_t *));

	memset(tpm_db_mod2_chain_bm_group, 0, TPM_CHAIN_TYPE_MAX * sizeof(uint32_t));

	memset(tpm_db_mod2_chain_info, 0, TPM_MAX_NUM_GMACS * TPM_CHAIN_TYPE_MAX * sizeof(tpm_mod2_chain_info_t *));
}

void tpm_db_mod2_set_chain_num(tpm_chain_type_t chain_type, uint16_t chain_num)
{
	if (chain_type < TPM_CHAIN_TYPE_MAX && chain_type != TPM_CHAIN_TYPE_NONE)
		tpm_db_mod2_chain_cfg[chain_type].total_num = chain_num;
}

uint16_t tpm_db_mod2_get_chain_max_size(tpm_chain_type_t chain_type)
{
	if (chain_type < TPM_CHAIN_TYPE_MAX && chain_type != TPM_CHAIN_TYPE_NONE)
		return tpm_db_mod2_chain_cfg[chain_type].max_size;

	return 0;
}

void tpm_db_mod2_set_multicast_mh_state(uint8_t enable)
{
	if (enable) {
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].max_size = TPM_MOD2_MAX_IPV4_MC_W_MH_CHAIN_SIZE;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].max_size = TPM_MOD2_MAX_IPV6_MC_W_MH_CHAIN_SIZE;
	} else {
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].max_size = TPM_MOD2_MAX_IPV4_MC_WO_MH_CHAIN_SIZE;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].max_size = TPM_MOD2_MAX_IPV6_MC_WO_MH_CHAIN_SIZE;
	}

	tpm_mod2_multicast_mh_enable = enable;
}

uint8_t tpm_db_mod2_get_multicast_mh_state(void)
{
	return tpm_mod2_multicast_mh_enable;
}

void tpm_db_mod2_set_pppoe_add_mod_state(uint8_t enable)
{
	if (enable) {
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].max_size = TPM_MOD2_MAX_IPV4_NAPT_W_PPPOE_CHAIN_SIZE;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].max_size = TPM_MOD2_MAX_ROUTE_W_PPPOE_CHAIN_SIZE;
	} else {
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].max_size = TPM_MOD2_MAX_IPV4_NAPT_WO_PPPOE_CHAIN_SIZE;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].max_size = TPM_MOD2_MAX_ROUTE_WO_PPPOE_CHAIN_SIZE;
	}

	tpm_mod2_pppoe_add_mod_enable = enable;
}

uint8_t tpm_db_mod2_get_pppoe_add_mod_state(void)
{
	return tpm_mod2_pppoe_add_mod_enable;
}

void tpm_db_mod2_set_double_tag_state(uint8_t enable)
{
	if (enable)
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].max_size = TPM_MOD2_MAX_L2_TWO_TAG_CHAIN_SIZE;
	else
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].max_size = TPM_MOD2_MAX_L2_ONE_TAG_CHAIN_SIZE;

	tpm_mod2_double_tag_enable = enable;
}

uint8_t tpm_db_mod2_get_double_tag_state(void)
{
	return tpm_mod2_double_tag_enable;
}

void tpm_db_mod2_set_udp_checksum_use_init_bm_state(uint8_t enable)
{
	tpm_mod2_udp_checksum_use_init_bm_enable = enable;
}

uint8_t tpm_db_mod2_get_udp_checksum_init_bm_state(void)
{
	return tpm_mod2_udp_checksum_use_init_bm_enable;
}

void tpm_db_mod2_set_udp_checksum_state(uint8_t enable)
{
	tpm_mod2_udp_checksum_enable = enable;
}

uint8_t tpm_db_mod2_get_udp_checksum_state(void)
{
	return tpm_mod2_udp_checksum_enable;
}

int32_t tpm_db_mod2_set_jump_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id, tpm_mod2_entry_t *pattern)
{
	tpm_mod2_jump_pmt_info_t *info_p = NULL;

	if (pattern == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (entry_id >= tpm_db_mod2_jump_area_cfg.total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	info_p = &tpm_db_mod2_jump_pmt_info[gmac_port][entry_id];

	if (info_p->status != TPM_MOD_ENTRY_BOOKED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid entry status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	memcpy(&(info_p->pattern), pattern, sizeof(tpm_mod2_entry_t));

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_activate_jump_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id)
{
	tpm_mod2_jump_pmt_info_t *info_p = NULL;
	uint32_t i, j, mask;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (entry_id >= tpm_db_mod2_jump_area_cfg.total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	info_p = &tpm_db_mod2_jump_pmt_info[gmac_port][entry_id];

	if (info_p->status != TPM_MOD_ENTRY_BOOKED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid entry status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}
	/* TODO: Check if the entry is split mod entry */

	i = j = 0;
	j = entry_id & 0x1f;
	i = entry_id >> 5;
	mask = 1 << j;

	tpm_db_mod2_jump_booked_entry_bm[gmac_port][i] &= ~mask;

	if (tpm_db_mod2_jump_split_mod_all_entry_bm[gmac_port][i] & mask) {
		/* this is a split mod entry */
		info_p->status = TPM_MOD_ENTRY_SPLIT_MOD;
		tpm_db_mod2_jump_split_mod_occupied_entry_bm[gmac_port][i] |= mask;
	}
	else {
		info_p->status = TPM_MOD_ENTRY_OCCUPIED;
		tpm_db_mod2_jump_occupied_entry_bm[gmac_port][i] |= mask;
	}

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_get_jump_entry_pattern_data(tpm_gmacs_enum_t gmac_port, uint16_t entry_id,
						tpm_mod2_entry_t *pattern)
{
	tpm_mod2_jump_pmt_info_t *info_p = NULL;

	if (pattern == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (entry_id >= tpm_db_mod2_jump_area_cfg.total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	info_p = &tpm_db_mod2_jump_pmt_info[gmac_port][entry_id];

	if (    (info_p->status != TPM_MOD_ENTRY_OCCUPIED)
		 && (info_p->status != TPM_MOD_ENTRY_SPLIT_MOD)) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid entry status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	memcpy(pattern, &(info_p->pattern), sizeof(tpm_mod2_entry_t));

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_del_jump_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id)
{
	tpm_mod2_jump_pmt_info_t *info_p = NULL;
	uint32_t i, j, mask;
	/*uint16_t jump_to; */

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (entry_id >= tpm_db_mod2_jump_area_cfg.total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	info_p = &tpm_db_mod2_jump_pmt_info[gmac_port][entry_id];

	if (   info_p->status != TPM_MOD_ENTRY_OCCUPIED
		&& info_p->status != TPM_MOD_ENTRY_SPLIT_MOD) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid entry status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	/* delete jump entry in db */
	memset(info_p, 0, sizeof(tpm_mod2_jump_pmt_info_t));

	/* TODO: Check if the entry is split mod entry */

	info_p->status = TPM_MOD_ENTRY_FREE;

	i = j = 0;
	j = entry_id & 0x1f;
	i = entry_id >> 5;
	mask = 1 << j;

	if (tpm_db_mod2_jump_split_mod_all_entry_bm[gmac_port][i] & mask) {
		/* this is a split mod pmt entry */
		tpm_db_mod2_jump_split_mod_occupied_entry_bm[gmac_port][i] &= ~mask;
	}
	else {
		tpm_db_mod2_jump_occupied_entry_bm[gmac_port][i] &= ~mask;
	}

    tpm_db_mod2_flush_jump_area(gmac_port, entry_id, 1, 0);

	return TPM_DB_OK;
}

uint16_t tpm_db_mod2_find_matched_chain(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t total_num,
					tpm_mod2_chain_info_t *chains, tpm_mod2_chain_pmt_info_t *pmt_data,
					uint16_t entry_num, tpm_mod2_entry_t *pattern)
{
	uint16_t chain_id, entry_id;
	uint32_t bm_idx, data;
	uint32_t *booked_bm = NULL, *occupied_bm = NULL;
	tpm_mod2_chain_pmt_info_t *pmt_ptr = NULL;

	booked_bm = tpm_db_mod2_chain_booked_entry_bm[gmac_port][chain_type];
	occupied_bm = tpm_db_mod2_chain_occupied_entry_bm[gmac_port][chain_type];

	for (bm_idx = 0; bm_idx < tpm_db_mod2_chain_bm_group[chain_type]; bm_idx++) {
		data = (booked_bm[bm_idx] | occupied_bm[bm_idx]);
		if (data) {
			chain_id = bm_idx << 5;	/* multiply 32 */
			while (data) {
				if (data & 0x1) {
					if (entry_num == chains[chain_id].entry_num) {
						pmt_ptr = &pmt_data[chains[chain_id].base_entry];
						for (entry_id = 0; entry_id < entry_num; entry_id++) {
							if (memcmp
							    (&(pmt_ptr[entry_id].pattern), &pattern[entry_id],
							     sizeof(tpm_mod2_entry_t))) {
								break;
							}
						}

						if (entry_id == entry_num)
							return chain_id;
					}
				}
				chain_id++;
				data = data >> 1;
			}
		}
	}

	return TPM_MOD2_INVALID_CHAIN_ID;
}

void tpm_db_mod2_increase_chain_user(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
	tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].user_num++;
}

void tpm_db_mod2_decrease_chain_user(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
	if (tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].user_num != 0)
		tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].user_num--;
}

uint16_t tpm_db_mod2_chain_user_num(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
	return tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].user_num;
}

uint16_t tpm_db_mod2_get_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t entry_num,
				     tpm_mod2_entry_t *pattern)
{
	if (pattern == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_MOD2_INVALID_CHAIN_ID;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_MOD2_INVALID_CHAIN_ID;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_MOD2_INVALID_CHAIN_ID;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (tpm_db_mod2_chain_pmt_info[gmac_port] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid pmt data\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	return tpm_db_mod2_find_matched_chain(gmac_port, chain_type,
					      tpm_db_mod2_chain_cfg[chain_type].total_num,
					      tpm_db_mod2_chain_info[gmac_port][chain_type],
					      tpm_db_mod2_chain_pmt_info[gmac_port], entry_num, pattern);
}

int32_t tpm_db_mod2_set_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id,
				    uint16_t entry_num, tpm_mod2_entry_t *pattern)
{
	uint16_t /*match_chain, */ base_offset, entry_idx;
	tpm_mod2_chain_info_t *info_p;
	tpm_mod2_chain_pmt_info_t *pmt_array;

	if (pattern == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero chain\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (chain_id >= tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain index\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];
	if (info_p->status == TPM_MOD_ENTRY_OCCUPIED) {
		tpm_db_mod2_increase_chain_user(gmac_port, chain_type, chain_id);
	} else if (info_p->status == TPM_MOD_ENTRY_BOOKED) {
		if (tpm_db_mod2_chain_user_num(gmac_port, chain_type, chain_id) == 0) {
			base_offset = tpm_db_mod2_chain_cfg[chain_type].base_entry - tpm_mod2_chain_area_base
			    + chain_id * tpm_db_mod2_chain_cfg[chain_type].max_size;

			pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][base_offset];

			for (entry_idx = 0; entry_idx < entry_num; entry_idx++) {
				if (pmt_array[entry_idx].status != TPM_MOD_ENTRY_FREE) {
					TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain status\n");
					return TPM_DB_ERR_REC_STATUS_ERR;
				}

				pmt_array[entry_idx].status = TPM_MOD_ENTRY_BOOKED;
				pmt_array[entry_idx].chain_type = chain_type;
				pmt_array[entry_idx].chain_id = chain_id;
				memcpy(&pmt_array[entry_idx].pattern, &pattern[entry_idx], sizeof(tpm_mod2_entry_t));
			}

			info_p->base_entry = base_offset;
			info_p->entry_num = entry_num;
		}

		tpm_db_mod2_increase_chain_user(gmac_port, chain_type, chain_id);
	} else {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid entry status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_activate_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
	tpm_mod2_chain_info_t *info_p;
	uint32_t i, j, mask;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero chain\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (chain_id >= tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain index\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];

	if (info_p->status == TPM_MOD_ENTRY_BOOKED) {
		info_p->status = TPM_MOD_ENTRY_OCCUPIED;

		i = j = 0;
		j = chain_id & 0x1f;
		i = chain_id >> 5;
		mask = 1 << j;

		tpm_db_mod2_chain_booked_entry_bm[gmac_port][chain_type][i] &= ~mask;
		tpm_db_mod2_chain_occupied_entry_bm[gmac_port][chain_type][i] |= mask;
	}

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_activate_chain_pmt_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
	uint16_t base_offset, entry_idx, entry_num;
	tpm_mod2_chain_info_t *info_p = NULL;
	tpm_mod2_chain_pmt_info_t *pmt_array = NULL;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero chain\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (chain_id >= tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain index\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];

	if (info_p->status != TPM_MOD_ENTRY_BOOKED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	base_offset = info_p->base_entry;
	entry_num = info_p->entry_num;

	pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][base_offset];

	for (entry_idx = 0; entry_idx < entry_num; entry_idx++)
		pmt_array[entry_idx].status = TPM_MOD_ENTRY_OCCUPIED;

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_if_chain_pmt_entries_exist(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					       uint16_t chain_id, uint8_t *exist)
{
	uint16_t base_offset, entry_idx, entry_num;
	tpm_mod2_chain_info_t *info_p = NULL;
	tpm_mod2_chain_pmt_info_t *pmt_array = NULL;

	if (exist == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero chain\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (chain_id >= tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain index\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];

	*exist = 0;

	if (info_p->status == TPM_MOD_ENTRY_BOOKED) {
		base_offset = info_p->base_entry;
		entry_num = info_p->entry_num;

		pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][base_offset];

		for (entry_idx = 0; entry_idx < entry_num; entry_idx++) {
			if (pmt_array[entry_idx].status != TPM_MOD_ENTRY_OCCUPIED)
				break;
		}

		if (entry_idx == entry_num)
			*exist = 1;

	} else if (info_p->status == TPM_MOD_ENTRY_OCCUPIED)
		*exist = 1;
	else {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_get_chain_entry_num(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					uint16_t chain_id, uint16_t *entry_num)
{
	tpm_mod2_chain_info_t *info_p;

	if (entry_num == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	*entry_num = 0;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero chain\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (chain_id >= tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain index\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];
	if (info_p->status != TPM_MOD_ENTRY_OCCUPIED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	*entry_num = info_p->entry_num;

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_get_chain_entry_pattern_data(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
						 uint16_t chain_id, uint16_t pattern_id, tpm_mod2_entry_t *pattern)
{
	tpm_mod2_chain_info_t *info_p;
	tpm_mod2_chain_pmt_info_t *entry_p;

	if (pattern == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero chain\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	if (chain_id >= tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain index\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain info\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];
	if (info_p->status != TPM_MOD_ENTRY_OCCUPIED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}

	if (pattern_id >= info_p->entry_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid pattern index\n");
		return TPM_DB_ERR_DB_TBL_INV;
	}

	entry_p = &tpm_db_mod2_chain_pmt_info[gmac_port][info_p->base_entry + pattern_id];
	if (entry_p->status != TPM_MOD_ENTRY_OCCUPIED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid entry status\n");
		return TPM_DB_ERR_REC_STATUS_ERR;
	}
	memcpy(pattern, &(entry_p->pattern), sizeof(tpm_mod2_entry_t));

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_del_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
	uint16_t /*entry_offset, chain_type, chain_id, */ entry_num, /*jump_to, */ entry_index;
	uint8_t remove_it;
	tpm_mod2_chain_info_t *info_p = NULL;
	tpm_mod2_chain_pmt_info_t *pmt_array = NULL;
	uint32_t i, j, mask;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain type \n");
		return TPM_DB_ERR_REC_INV;
	}

	if (chain_id > tpm_db_mod2_chain_cfg[chain_type].total_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain id \n");
		return TPM_DB_ERR_REC_INV;
	}

	info_p = &tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id];

	if (info_p->status != TPM_MOD_ENTRY_OCCUPIED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid chain entry \n");
		return TPM_DB_ERR_REC_INV;
	}

	pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][info_p->base_entry];

	tpm_db_mod2_decrease_chain_user(gmac_port, chain_type, chain_id);

	remove_it = 0;
	if (tpm_db_mod2_chain_user_num(gmac_port, chain_type, chain_id) == 0)
		remove_it = 1;

	entry_num = info_p->entry_num;

	for (entry_index = 0; entry_index < entry_num; entry_index++) {
		if (remove_it) {
			memset(&pmt_array[entry_index], 0, sizeof(tpm_mod2_chain_pmt_info_t));
			pmt_array[entry_index].status = TPM_MOD_ENTRY_FREE;
		}
	}

	if (remove_it) {
		memset(info_p, 0, sizeof(tpm_mod2_chain_info_t));
		info_p->status = TPM_MOD_ENTRY_FREE;

		i = j = 0;
		j = chain_id & 0x1f;
		i = chain_id >> 5;
		mask = 1 << j;

		tpm_db_mod2_chain_occupied_entry_bm[gmac_port][chain_type][i] &= ~mask;

		tpm_db_mod2_flush_chain_entry(gmac_port, chain_type, chain_id, 1, 0);
	}

	return TPM_DB_OK;
}

void tpm_db_mod2_update_next_free_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type)
{
	uint16_t chain_id;
	uint32_t bm_idx, bit_idx, data;
	uint32_t *booked_bm = NULL, *occupied_bm = NULL;

	booked_bm = tpm_db_mod2_chain_booked_entry_bm[gmac_port][chain_type];
	occupied_bm = tpm_db_mod2_chain_occupied_entry_bm[gmac_port][chain_type];

	for (bm_idx = 0; bm_idx < tpm_db_mod2_chain_bm_group[chain_type]; bm_idx++) {
		data = ((booked_bm[bm_idx] | occupied_bm[bm_idx]) ^ 0xffffffff);
		if (data) {
			chain_id = bm_idx << 5;	/* multiply 32 */
			bit_idx = 0;
			while (data) {
				if (data & 0x1) {
					chain_id += bit_idx;
					if (chain_id < tpm_db_mod2_chain_cfg[chain_type].total_num) {
						tpm_db_mod2_chain_cfg[chain_type].next_free[gmac_port] = chain_id;
						return;
					}
					break;
				}
				bit_idx++;
				data = data >> 1;
			}
		}
	}

	tpm_db_mod2_chain_cfg[chain_type].next_free[gmac_port] = TPM_MOD2_INVALID_CHAIN_ID;
	return;
}

uint16_t tpm_db_mod2_get_next_free_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type)
{
	uint16_t next_free;
	uint32_t i, j, mask;

	if (gmac_port >= TPM_MAX_NUM_GMACS)
		return TPM_MOD2_INVALID_CHAIN_ID;

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE)
		return TPM_MOD2_INVALID_CHAIN_ID;

	next_free = tpm_db_mod2_chain_cfg[chain_type].next_free[gmac_port];
	if (next_free != TPM_MOD2_INVALID_CHAIN_ID) {
		tpm_db_mod2_chain_info[gmac_port][chain_type][next_free].status = TPM_MOD_ENTRY_BOOKED;

		i = j = 0;
		j = TPM_DB_MOD_32(next_free);
		i = TPM_DB_DIVIDED_BY_32(next_free);
		mask = 1 << j;

		tpm_db_mod2_chain_booked_entry_bm[gmac_port][chain_type][i] |= mask;
	}

	tpm_db_mod2_update_next_free_chain_entry(gmac_port, chain_type);

	return next_free;
}

uint16_t tpm_db_mod2_convert_chain_to_pmt_entry(tpm_chain_type_t chain_type, uint16_t chain_id)
{
	tpm_mod2_chain_cfg_t *cfg_p = NULL;

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE)
		return TPM_MOD2_NULL_ENT_IDX;

	cfg_p = &tpm_db_mod2_chain_cfg[chain_type];

	if (chain_id >= cfg_p->total_num)
		return TPM_MOD2_NULL_ENT_IDX;

	return (chain_id * cfg_p->max_size + cfg_p->base_entry);
}

int32_t tpm_db_mod2_get_chain_id_by_pmt_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id,
					      tpm_chain_type_t *chain_type, uint16_t *chain_id)
{
	uint16_t entry_offset;
	tpm_mod2_chain_pmt_info_t *pmt_array = NULL;

	if (chain_type == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_id == NULL) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (entry_id >= TPM_MOD2_TOTAL_PMT_SIZE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	entry_offset = entry_id - tpm_mod2_chain_area_base;
	if (entry_offset >= tpm_mod2_total_chain_num) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][entry_offset];

	if (pmt_array->status != TPM_MOD_ENTRY_OCCUPIED) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid pmt entry \n");
		return TPM_DB_ERR_REC_INV;
	}

	*chain_type = pmt_array->chain_type;
	*chain_id = pmt_array->chain_id;

	return TPM_DB_OK;
}

void tpm_db_mod2_update_next_free_jump_entry(tpm_gmacs_enum_t gmac_port)
{
	uint16_t entry_id /*, total_num */ ;
	/*tpm_mod2_jump_pmt_info_t *info_array = NULL; */
	uint32_t bm_idx, bit_idx, data;
	uint32_t *booked_bm = NULL, *occupied_bm = NULL, *split_mod_occupied_bm = NULL;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return;
	}

	booked_bm = tpm_db_mod2_jump_booked_entry_bm[gmac_port];
	occupied_bm = tpm_db_mod2_jump_occupied_entry_bm[gmac_port];
	split_mod_occupied_bm = tpm_db_mod2_jump_split_mod_all_entry_bm[gmac_port];
#if 0
	info_array = tpm_db_mod2_jump_pmt_info[gmac_port];
	total_num  = tpm_db_mod2_jump_area_cfg.total_num;

	for (entry_id = 0; entry_id < total_num; entry_id++)
#endif
	for (bm_idx = 0; bm_idx < tpm_db_mod2_jump_bm_group; bm_idx++) {
		/*if (info_array[entry_id].status == TPM_MOD_ENTRY_FREE) */
		data = ((booked_bm[bm_idx] | occupied_bm[bm_idx] | split_mod_occupied_bm[bm_idx]) ^ 0xffffffff);
		if (data) {
			entry_id = bm_idx << 5;	/* multiply 32 */
			bit_idx = 0;
			while (data) {
				if (data & 0x1) {
					entry_id += bit_idx;
					if (entry_id < tpm_db_mod2_jump_area_cfg.total_num) {
						tpm_db_mod2_jump_area_cfg.next_free[gmac_port] = entry_id;
						return;
					}
					break;
				}
				bit_idx++;
				data = data >> 1;
			}
		}
	}

	tpm_db_mod2_jump_area_cfg.next_free[gmac_port] = TPM_MOD2_NULL_ENT_IDX;
	return;
}

uint16_t tpm_db_mod2_get_next_free_jump_entry(tpm_gmacs_enum_t gmac_port)
{
	uint16_t next_free;
	uint32_t i, j, mask;

	if (gmac_port >= TPM_MAX_NUM_GMACS)
		return TPM_MOD2_NULL_ENT_IDX;

	next_free = tpm_db_mod2_jump_area_cfg.next_free[gmac_port];
	if (next_free != TPM_MOD2_NULL_ENT_IDX) {
		tpm_db_mod2_jump_pmt_info[gmac_port][next_free].status = TPM_MOD_ENTRY_BOOKED;

		i = j = 0;
		j = TPM_DB_MOD_32(next_free);
		i = TPM_DB_DIVIDED_BY_32(next_free);
		mask = 1 << j;

		tpm_db_mod2_jump_booked_entry_bm[gmac_port][i] |= mask;
	}

	tpm_db_mod2_update_next_free_jump_entry(gmac_port);

	return next_free;
}

uint32_t tpm_db_mod2_split_mod_get_vlan_num_in_use(tpm_gmacs_enum_t gmac_port)
{
	uint32_t i;
	uint32_t vlan_num = 0;

	for (i = 0; i < TPM_DB_SPLIT_MOD_NUM_VLANS_MAX; i++) {
		if (tpm_db.split_mod_conf.gmac_vlan_conf[gmac_port].split_mod_vlan[i].valid)
			vlan_num++;
	}

	return vlan_num;
}

uint16_t tpm_db_mod2_get_next_split_mod_free_jump_entry(tpm_gmacs_enum_t gmac_port, tpm_pkt_mod_t *mod_data)
{
	uint16_t next_free;
	uint32_t i, j, mask;
	uint32_t vlan_index, db_ret;
	uint32_t  pbit_index = 0;
	tpm_vlan_key_t  *vlan_out;

	if (gmac_port >= TPM_MAX_NUM_GMACS)
		return TPM_MOD2_NULL_ENT_IDX;

	db_ret = tpm_db_mod2_split_mod_get_vlan_index(gmac_port, mod_data, &vlan_index);
	if (TPM_DB_OK != db_ret)
		return TPM_MOD2_NULL_ENT_IDX;

	/* get next_free pmt index from vid index */
	if (0 == vlan_index)
		/* first vlan, means vlan 0, default */
		next_free = 1;
	else
		next_free = (16 * vlan_index);

	if (VLANOP_EXT_TAG_MOD_INS == mod_data->vlan_mod.vlan_op)
		vlan_out = &mod_data->vlan_mod.vlan2_out;
	else
		vlan_out = &mod_data->vlan_mod.vlan1_out;

	if (0 == vlan_out->pbit_mask)
		/* pbit AS_IS */
		pbit_index = 0;
	else {
		pbit_index = vlan_out->pbit;
		if (next_free != 1)
			pbit_index++;
	}

	next_free += pbit_index;

	if (next_free != TPM_MOD2_NULL_ENT_IDX) {
		tpm_db_mod2_jump_pmt_info[gmac_port][next_free].status = TPM_MOD_ENTRY_BOOKED;

		i = j = 0;
		j = TPM_DB_MOD_32(next_free);
		i = TPM_DB_DIVIDED_BY_32(next_free);
		mask = 1 << j;

		tpm_db_mod2_jump_booked_entry_bm[gmac_port][i] |= mask;
	}

	return next_free;
}
int32_t tpm_db_mod2_split_mod_insert_vlan(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data)
{
	uint32_t i;
	uint32_t vlan_num;

	/* check vlan num right now */
	vlan_num = tpm_db_mod2_split_mod_get_vlan_num_in_use(port);
	if (vlan_num >= tpm_db_split_mod_get_num_vlans()){
		TPM_OS_ERROR(TPM_DB_MOD, "already has %d num_vlans\n", vlan_num - TPM_DB_SPLIT_MOD_INIT_VLANS_NUM);
		return TPM_DB_ERR_DB_TBL_FULL;
	}

	for (i = 0; i < TPM_DB_SPLIT_MOD_NUM_VLANS_MAX; i++) {
		if (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].valid)
			continue;

		//here finds a spot
		tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].valid = 1;

		if (mod_data->vlan_mod.vlan1_out.vid_mask)
			tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_1 = mod_data->vlan_mod.vlan1_out.vid;

		if (mod_data->vlan_mod.vlan2_out.vid_mask)
			tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_2 = mod_data->vlan_mod.vlan2_out.vid;

		if (mod_data->vlan_mod.vlan1_out.tpid_mask)
			tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].tpid_1 = mod_data->vlan_mod.vlan1_out.tpid;

		if (mod_data->vlan_mod.vlan2_out.tpid_mask)
			tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].tpid_2 = mod_data->vlan_mod.vlan2_out.tpid;

		tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_op = mod_data->vlan_mod.vlan_op;
		tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].user_num = 1;

		return TPM_DB_OK;
	}

	return TPM_DB_ERR_DB_TBL_FULL;
}
int32_t tpm_db_mod2_split_mod_get_vlan_index(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data, uint32_t *index)
{
	uint32_t i;
	//printk("input: port: %d, vlan_op: %d, vlan1: %d(%d), vlan2: %d(%d)\n",
		//port, mod_data->vlan_mod.vlan_op, mod_data->vlan_mod.vlan1_out.vid, mod_data->vlan_mod.vlan1_out.vid_mask,
		//mod_data->vlan_mod.vlan2_out.vid, mod_data->vlan_mod.vlan2_out.vid_mask);
	for (i = 0; i < TPM_DB_SPLIT_MOD_NUM_VLANS_MAX; i++) {
		//printk("output: port: %d, vlan_op: %d, vlan1: %d, vlan2: %d\n",
			//tpm_db.split_mod_conf.gmac_vlan_conf[i].gmac_port, tpm_db.split_mod_conf.gmac_vlan_conf[i].vlan_op,
			//tpm_db.split_mod_conf.gmac_vlan_conf[i].vlan_1, tpm_db.split_mod_conf.gmac_vlan_conf[i].vlan_2);
		if (!tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].valid)
			continue;

		if (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_op != mod_data->vlan_mod.vlan_op)
			continue;

		if (    (mod_data->vlan_mod.vlan1_out.vid_mask)
			 && (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_1 != mod_data->vlan_mod.vlan1_out.vid))
			continue;

		if (    (mod_data->vlan_mod.vlan2_out.vid_mask)
			 && (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_2 != mod_data->vlan_mod.vlan2_out.vid))
			continue;

		if (    (mod_data->vlan_mod.vlan1_out.tpid_mask)
			 && (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].tpid_1 != mod_data->vlan_mod.vlan1_out.tpid))
			continue;

		if (    (mod_data->vlan_mod.vlan2_out.tpid_mask)
			 && (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].tpid_2 != mod_data->vlan_mod.vlan2_out.tpid))
			continue;

		if (    (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_1)
			 && (!mod_data->vlan_mod.vlan1_out.vid_mask))
			continue;

		if (    (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].vlan_2)
			 && (!mod_data->vlan_mod.vlan2_out.vid_mask))
			continue;

		if (    (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].tpid_1)
			 && (!mod_data->vlan_mod.vlan1_out.tpid_mask))
			continue;

		if (    (tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[i].tpid_2)
			 && (!mod_data->vlan_mod.vlan2_out.tpid_mask))
			continue;

		//here finds the vlan
		*index = i;
		return TPM_DB_OK;
	}

	return TPM_DB_ERR_REC_NOT_EXIST;
}

int32_t tpm_db_mod2_split_mod_increase_vlan_user_num(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data)
{
	uint32_t index;
	int32_t int_ret_code;

	int_ret_code = tpm_db_mod2_split_mod_get_vlan_index(port, mod_data, &index);
	if (TPM_DB_OK != int_ret_code) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "failed to get split mod vlan index\n");
		return TPM_DB_ERR_REC_NOT_EXIST;
	}

	tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[index].user_num++;
	return TPM_DB_OK;
}

int32_t tpm_db_mod2_split_mod_decrease_vlan_user_num(tpm_gmacs_enum_t port,
						     uint32_t       vlan_index,
						     uint32_t      *user_num)
{
	if (port > TPM_MAX_GMAC) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid port: %d\n", port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (vlan_index >= TPM_DB_SPLIT_MOD_NUM_VLANS_MAX) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid vlan_index: %d\n", vlan_index);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (!tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[vlan_index].valid) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid vlan entry: %d\n", vlan_index);
		return TPM_DB_ERR_INV_INPUT;
	}

	tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[vlan_index].user_num--;
	*user_num = tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[vlan_index].user_num;

	if (!tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[vlan_index].user_num) {
		TPM_OS_INFO(TPM_DB_MOD, "user num is 0, remove this vlan\n");
		memset(&(tpm_db.split_mod_conf.gmac_vlan_conf[port].split_mod_vlan[vlan_index]), 0, sizeof(tpm_db_split_mod_vlan_t));
	}

	return TPM_DB_OK;
}

uint8_t tpm_db_mod2_rollback_jump_area(tpm_gmacs_enum_t gmac_port, uint16_t entry_id, uint8_t on_failure)
{
	/*uint16_t entry_idx, total_num; */
	uint8_t dirty = 0;
	tpm_mod2_jump_pmt_info_t *info_array = NULL;
	uint32_t i, j, mask;

	if (tpm_db_mod2_jump_pmt_info[gmac_port] == NULL)
		return 0;

	info_array = tpm_db_mod2_jump_pmt_info[gmac_port];
	if (info_array[entry_id].status == TPM_MOD_ENTRY_BOOKED) {
		/* TODO: check if the pmt entry is for split mod */
		i = j = 0;
		j = entry_id & 0x1f;
		i = entry_id >> 5;
		mask = 1 << j;

		tpm_db_mod2_jump_booked_entry_bm[gmac_port][i] &= ~mask;

		info_array[entry_id].status = TPM_MOD_ENTRY_FREE;
		dirty = 1;
	} else if (info_array[entry_id].status == TPM_MOD_ENTRY_OCCUPIED) {
		if (on_failure) {
			/* TODO: check if the pmt entry is for split mod */
			i = j = 0;
			j = entry_id & 0x1f;
			i = entry_id >> 5;
			mask = 1 << j;

			tpm_db_mod2_jump_occupied_entry_bm[gmac_port][i] &= ~mask;

			info_array[entry_id].status = TPM_MOD_ENTRY_FREE;
			dirty = 1;
		}
	}

	return dirty;
}

uint8_t tpm_db_mod2_rollback_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					 uint16_t chain_id, uint8_t on_failure)
{
	uint8_t dirty = 0;
	uint16_t entry_index, entry_num;
	tpm_mod2_chain_info_t *info_array = NULL;
	tpm_mod2_chain_pmt_info_t *pmt_array = NULL;
	uint32_t i, j, mask;

	if (tpm_db_mod2_chain_info[gmac_port][chain_type] == NULL)
		return 0;

	info_array = tpm_db_mod2_chain_info[gmac_port][chain_type];
	if (info_array[chain_id].status == TPM_MOD_ENTRY_BOOKED) {
		tpm_db_mod2_decrease_chain_user(gmac_port, chain_type, chain_id);

		if (tpm_db_mod2_chain_user_num(gmac_port, chain_type, chain_id) == 0) {
			pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][info_array[chain_id].base_entry];

			entry_num = info_array[chain_id].entry_num;

			for (entry_index = 0; entry_index < entry_num; entry_index++) {
				memset(&pmt_array[entry_index], 0, sizeof(tpm_mod2_chain_pmt_info_t));
				pmt_array[entry_index].status = TPM_MOD_ENTRY_FREE;
			}

			i = j = 0;
			j = chain_id & 0x1f;
			i = chain_id >> 5;
			mask = 1 << j;

			tpm_db_mod2_chain_booked_entry_bm[gmac_port][chain_type][i] &= ~mask;
			info_array[chain_id].status = TPM_MOD_ENTRY_FREE;
			dirty = 1;
		}
	} else if (info_array[chain_id].status == TPM_MOD_ENTRY_OCCUPIED) {
		if (on_failure)
			tpm_db_mod2_decrease_chain_user(gmac_port, chain_type, chain_id);

		if (tpm_db_mod2_chain_user_num(gmac_port, chain_type, chain_id) == 0) {
			pmt_array = &tpm_db_mod2_chain_pmt_info[gmac_port][info_array[chain_id].base_entry];

			entry_num = info_array[chain_id].entry_num;

			for (entry_index = 0; entry_index < entry_num; entry_index++) {
				memset(&pmt_array[entry_index], 0, sizeof(tpm_mod2_chain_pmt_info_t));
				pmt_array[entry_index].status = TPM_MOD_ENTRY_FREE;
			}

			i = j = 0;
			j = chain_id & 0x1f;
			i = chain_id >> 5;
			mask = 1 << j;

			tpm_db_mod2_chain_occupied_entry_bm[gmac_port][chain_type][i] &= ~mask;

			info_array[chain_id].status = TPM_MOD_ENTRY_FREE;
			dirty = 1;
		}
	}
	return dirty;
}

int32_t tpm_db_mod2_flush_jump_area(tpm_gmacs_enum_t gmac_port, uint16_t entry_id,
				    uint8_t updt_next_free, uint8_t on_failure)
{
	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_rollback_jump_area(gmac_port, entry_id, on_failure) || updt_next_free)
		tpm_db_mod2_update_next_free_jump_entry(gmac_port);

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_flush_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
				      uint16_t chain_id, uint8_t updt_next_free, uint8_t on_failure)
{

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		TPM_OS_ERROR(TPM_DB_MOD, "Invalid input\n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE)
		return TPM_DB_ERR_INV_INPUT;

    if (tpm_db_mod2_rollback_chain_entry(gmac_port, chain_type, chain_id, on_failure) || updt_next_free)
		tpm_db_mod2_update_next_free_chain_entry(gmac_port, chain_type);

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_validate_chain_config(void)
{
	uint16_t chain_type, total_entries, jump_entries, napt_chains, l2_chains, free_entries;
	uint16_t split_jump_entries, split_jump_free_entries, least_jump_entries;
	uint8_t naptv4_unlimited, l2_unlimited, main_chain_empty, subr_chain_empty;
	uint32_t split_vlan_num, split_pbit_num;

	main_chain_empty = subr_chain_empty = 1;
	/* check subroutine chain size */
	if (tpm_db_mod2_get_multicast_mh_state()) {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num == 0) {
			TPM_OS_WARN(TPM_DB_MOD, "Zero mh chain \n");
		} else {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num == TPM_CHAIN_NUM_UNLIMITED) {
				TPM_OS_ERROR(TPM_DB_MOD, "Invalid unlimited chain type %d \n", TPM_CHAIN_TYPE_MH);
				return TPM_DB_ERR_INV_INPUT;
			}
			subr_chain_empty = 0;
		}
	} else {
		TPM_OS_WARN(TPM_DB_MOD, "multicast mh is disabled, mh chain size should be zero \n");
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num = 0;
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero mac chain \n");
	} else {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num == TPM_CHAIN_NUM_UNLIMITED) {
			TPM_OS_ERROR(TPM_DB_MOD, "Invalid unlimited chain type %d \n", TPM_CHAIN_TYPE_MAC);
			return TPM_DB_ERR_INV_INPUT;
		}
		subr_chain_empty = 0;
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero vlan chain \n");
	} else {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num == TPM_CHAIN_NUM_UNLIMITED) {
			TPM_OS_ERROR(TPM_DB_MOD, "Invalid unlimited chain type %d \n", TPM_CHAIN_TYPE_VLAN);
			return TPM_DB_ERR_INV_INPUT;
		}
		subr_chain_empty = 0;
	}

	if (tpm_db_mod2_get_pppoe_add_mod_state()) {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num != 0 ||
			tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num != 0) {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].total_num == 0) {
				TPM_OS_WARN(TPM_DB_MOD, "Zero pppoev4 chain \n");
			} else {
				if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].total_num == TPM_CHAIN_NUM_UNLIMITED) {
					TPM_OS_ERROR(TPM_DB_MOD, "Invalid unlimited chain type %d \n", TPM_CHAIN_TYPE_PPPOE);
					return TPM_DB_ERR_INV_INPUT;
				}
				subr_chain_empty = 0;
			}
		}
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num != 0 ||
			tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num) {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].total_num == 0) {
				TPM_OS_WARN(TPM_DB_MOD, "Zero pppoev6 chain \n");
			} else {
				if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].total_num == TPM_CHAIN_NUM_UNLIMITED) {
					TPM_OS_ERROR(TPM_DB_MOD, "Invalid unlimited chain type %d \n", TPM_CHAIN_TYPE_IPV6_PPPOE);
					return TPM_DB_ERR_INV_INPUT;
				}
				subr_chain_empty = 0;
			}
		}
	} else {
		TPM_OS_WARN(TPM_DB_MOD, "pppoe mod is disabled, pppoe chain size should be zero \n");
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].total_num = 0;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].total_num = 0;
	}

	if (subr_chain_empty) {
		TPM_OS_ERROR(TPM_DB_MOD, "All subr chain is zero \n");
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero l2 main chain \n");
	} else {
		main_chain_empty = 0;
	}
	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero naptv4 main chain \n");
	} else {
		main_chain_empty = 0;
	}
	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero mcv4 main chain \n");
	} else {
		main_chain_empty = 0;
	}
	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero routev6 main chain \n");
	} else {
		main_chain_empty = 0;
	}
	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num == 0) {
		TPM_OS_WARN(TPM_DB_MOD, "Zero mcv6 main chain \n");
	} else {
		main_chain_empty = 0;
	}
	if (main_chain_empty) {
		TPM_OS_ERROR(TPM_DB_MOD, "All main chain is zero \n");
		return TPM_DB_ERR_INV_INPUT;
	}

	/* check total pmt entries for all the chains */
	total_entries = 0;
	naptv4_unlimited = l2_unlimited = 0;
	for (chain_type = TPM_CHAIN_TYPE_MH; chain_type < TPM_CHAIN_TYPE_MAX; chain_type++) {
		if (tpm_db_mod2_chain_cfg[chain_type].total_num == TPM_CHAIN_NUM_UNLIMITED) {
			if (naptv4_unlimited || l2_unlimited) {
				TPM_OS_ERROR(TPM_DB_MOD, "Only one chain type could be assigned unlimited size \n");
				return TPM_DB_ERR_INV_INPUT;
			}

			if (chain_type == TPM_CHAIN_TYPE_IPV4_NAPT) {
				naptv4_unlimited = 1;
				continue;
			} else if (chain_type == TPM_CHAIN_TYPE_L2) {
				l2_unlimited = 1;
				continue;
			} else {
				TPM_OS_ERROR(TPM_DB_MOD, "Invalid unlimited chain type %d \n", chain_type);
				return TPM_DB_ERR_INV_INPUT;
			}
		}
		total_entries +=
		    tpm_db_mod2_chain_cfg[chain_type].max_size * tpm_db_mod2_chain_cfg[chain_type].total_num;
		if (total_entries >= TPM_MOD2_TOTAL_PMT_SIZE) {
			TPM_OS_ERROR(TPM_DB_MOD, "Not enough chain area space \n");
			return TPM_DB_ERR_INV_INPUT;
		}
	}

	/*the jump entries need at least without split mod*/
	if (l2_unlimited)
		least_jump_entries = tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num;
	else if (naptv4_unlimited)
		least_jump_entries = tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num;
	else
		least_jump_entries = tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num +
				tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num;

	/*take split mod in consider*/
	split_jump_entries = 0;
	split_jump_free_entries = 0;
	if (TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) {
		split_vlan_num = tpm_db_split_mod_get_num_vlans() - TPM_DB_SPLIT_MOD_INIT_VLANS_NUM;
		split_pbit_num = TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX;
		split_jump_entries = 16 * 2/*init added*/ +
					(split_vlan_num - 1) * 16/*internal VLAN*/ +
					split_pbit_num + 1/*last VLAN*/;
		split_jump_free_entries = 2 * (15 - split_pbit_num)/*init occupy*/ +
					(split_vlan_num - 1) * (16 - split_pbit_num - 1)/*internal VLAN*/;
	}

	if (l2_unlimited) {
		if (split_jump_free_entries >= least_jump_entries)
			free_entries = TPM_MOD2_TOTAL_PMT_SIZE - total_entries - split_jump_entries;
		else
			free_entries = TPM_MOD2_TOTAL_PMT_SIZE - total_entries - split_jump_entries -
				(least_jump_entries - split_jump_free_entries);
		l2_chains = free_entries / (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].max_size + 1);
		if (l2_chains == 0) {
			TPM_OS_ERROR(TPM_DB_MOD, "Zero naptv4 main chain \n");
			return TPM_DB_ERR_INV_INPUT;
		}
		total_entries += l2_chains * tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].max_size;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num = l2_chains;
	} else if (naptv4_unlimited) {
		if (split_jump_free_entries >= least_jump_entries)
			free_entries = TPM_MOD2_TOTAL_PMT_SIZE - total_entries - split_jump_entries;
		else
			free_entries = TPM_MOD2_TOTAL_PMT_SIZE - total_entries - split_jump_entries -
				(least_jump_entries - split_jump_free_entries);
		napt_chains = free_entries / (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].max_size + 1);
		if (napt_chains == 0) {
			TPM_OS_ERROR(TPM_DB_MOD, "Zero naptv4 main chain \n");
			return TPM_DB_ERR_INV_INPUT;
		}
		total_entries += napt_chains * tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].max_size;
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num = napt_chains;
	}

	if (total_entries >= TPM_MOD2_TOTAL_PMT_SIZE) {
		TPM_OS_ERROR(TPM_DB_MOD, "Zero jump area \n");
		return TPM_DB_ERR_INV_INPUT;
	}

	jump_entries = TPM_MOD2_TOTAL_PMT_SIZE - total_entries;
	if (jump_entries > TPM_MOD2_MAX_JUMP_ENTRIES_NUM)
		jump_entries = TPM_MOD2_MAX_JUMP_ENTRIES_NUM;

	if ((tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num +
	     tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num +
	     tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num +
	     tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num +
	     tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num) > jump_entries - (split_jump_entries - split_jump_free_entries))
		TPM_OS_WARN(TPM_DB_MOD, "jump entries are not enough for main chain \n");

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num > 32)
		TPM_OS_WARN(TPM_DB_MOD, "mh sub chain is too large \n");

	if ((tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num < tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num)
	    && (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num <
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num))
		TPM_OS_WARN(TPM_DB_MOD, "mac sub chain is too large \n");

	if (((tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num + (split_jump_entries - split_jump_free_entries) <
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num))
	    && ((tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num + (split_jump_entries - split_jump_free_entries) <
		tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num)))
		TPM_OS_WARN(TPM_DB_MOD, "vlan sub chain is too large \n");

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num <
	    tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].total_num)
		TPM_OS_WARN(TPM_DB_MOD, "pppoe sub chain is too large \n");

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_L2].total_num > 0) {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num == 0) {
			TPM_OS_ERROR(TPM_DB_MOD, "Zero vlan subr chain while naptv4 main chain is activated \n");
			return TPM_DB_ERR_INV_INPUT;
		}
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_NAPT].total_num > 0) {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num == 0) {
			TPM_OS_ERROR(TPM_DB_MOD, "Zero mac subr chain while naptv4 main chain is activated \n");
			return TPM_DB_ERR_INV_INPUT;
		}

		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num == 0) {
			TPM_OS_WARN(TPM_DB_MOD, "Zero vlan subr chain while naptv4 main chain is activated \n");
		}

		if (tpm_db_mod2_get_pppoe_add_mod_state()) {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_PPPOE].total_num == 0) {
				TPM_OS_WARN(TPM_DB_MOD,
					     "Zero pppoe subr chain while naptv4 main chain is activated \n");
			}
		}
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_ROUTE].total_num > 0) {
		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MAC].total_num == 0) {
			TPM_OS_ERROR(TPM_DB_MOD, "Zero mac subr chain while routev6 main chain is activated \n");
			return TPM_DB_ERR_INV_INPUT;
		}

		if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_VLAN].total_num == 0) {
			TPM_OS_WARN(TPM_DB_MOD, "Zero vlan subr chain while routev6 main chain is activated \n");
		}

		if (tpm_db_mod2_get_pppoe_add_mod_state()) {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_PPPOE].total_num == 0) {
				TPM_OS_WARN(TPM_DB_MOD,
					     "Zero pppoe subr chain while routev6 main chain is activated \n");
			}
		}
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV4_MC].total_num > 0) {
		if (tpm_db_mod2_get_multicast_mh_state()) {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num == 0) {
				TPM_OS_WARN(TPM_DB_MOD, "Zero mh subr chain while mcv4 main chain is activated \n");
			}
		}
	}

	if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_IPV6_MC].total_num > 0) {
		if (tpm_db_mod2_get_multicast_mh_state()) {
			if (tpm_db_mod2_chain_cfg[TPM_CHAIN_TYPE_MH].total_num == 0) {
				TPM_OS_WARN(TPM_DB_MOD, "Zero mh subr chain while mcv6 main chain is activated \n");
			}
		}
	}

	if (jump_entries < split_jump_entries)
		TPM_OS_WARN(TPM_DB_MOD, "jump entries are not enough for split mod \n");

	/* TODO: Check if jump entries are  enough for split modification */

	tpm_mod2_chain_area_base = jump_entries + 1;
	tpm_mod2_total_chain_num = total_entries;

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_setup_jump_area(tpm_gmacs_enum_t port)
{
	uint32_t info_size, bm_size /*, port */ ;
	uint32_t	vlan_nums;
	uint32_t	p_bits_nums;
	uint32_t	i, j;
	uint32_t	loop;
	uint32_t	loop_in;
	uint32_t	cur_entry, mask;

	tpm_db_mod2_jump_area_cfg.base_entry = 0;
	tpm_db_mod2_jump_area_cfg.total_num = tpm_mod2_chain_area_base;
#if 0
	tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_GMAC_0] = 1;
	tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_GMAC_1] = 1;
	tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_PMAC]   = 1;
#endif
	tpm_db_mod2_jump_area_cfg.next_free[port] = 1;

	info_size = tpm_db_mod2_jump_area_cfg.total_num * sizeof(tpm_mod2_jump_pmt_info_t);
	if (tpm_db_mod2_jump_area_cfg.total_num % 32) {
		bm_size = (tpm_db_mod2_jump_area_cfg.total_num / 32 + 1) * sizeof(uint32_t);
		tpm_db_mod2_jump_bm_group = tpm_db_mod2_jump_area_cfg.total_num / 32 + 1;
	} else {
		bm_size = (tpm_db_mod2_jump_area_cfg.total_num / 32) * sizeof(uint32_t);
		tpm_db_mod2_jump_bm_group = tpm_db_mod2_jump_area_cfg.total_num / 32;
	}

	/*for (port = TPM_ENUM_GMAC_0; port < TPM_MAX_NUM_GMACS; port++) */
	{
		if (tpm_db_mod2_jump_pmt_info[port] != NULL) {
			vfree(tpm_db_mod2_jump_pmt_info[port]);
			tpm_db_mod2_jump_pmt_info[port] = NULL;
		}
		tpm_db_mod2_jump_pmt_info[port] = (tpm_mod2_jump_pmt_info_t *) vmalloc(info_size);
		if (tpm_db_mod2_jump_pmt_info[port] == NULL) {
			TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
			return TPM_DB_ERR_MEM_ALLOC_FAIL;
		}
		memset(tpm_db_mod2_jump_pmt_info[port], 0, info_size);
		tpm_db_mod2_jump_pmt_info[port][0].status = TPM_MOD_ENTRY_OCCUPIED;

		if (tpm_db_mod2_jump_booked_entry_bm[port] != NULL) {
			vfree(tpm_db_mod2_jump_booked_entry_bm[port]);
			tpm_db_mod2_jump_booked_entry_bm[port] = NULL;
		}
		tpm_db_mod2_jump_booked_entry_bm[port] = (uint32_t *) vmalloc(bm_size);
		if (tpm_db_mod2_jump_booked_entry_bm[port] == NULL) {
			TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
			return TPM_DB_ERR_MEM_ALLOC_FAIL;
		}
		memset(tpm_db_mod2_jump_booked_entry_bm[port], 0, bm_size);

		if (tpm_db_mod2_jump_occupied_entry_bm[port] != NULL) {
			vfree(tpm_db_mod2_jump_occupied_entry_bm[port]);
			tpm_db_mod2_jump_occupied_entry_bm[port] = NULL;
		}
		tpm_db_mod2_jump_occupied_entry_bm[port] = (uint32_t *) vmalloc(bm_size);
		if (tpm_db_mod2_jump_occupied_entry_bm[port] == NULL) {
			TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
			return TPM_DB_ERR_MEM_ALLOC_FAIL;
		}
		memset(tpm_db_mod2_jump_occupied_entry_bm[port], 0, bm_size);
		tpm_db_mod2_jump_occupied_entry_bm[port][0] |= 0x1;

		if (tpm_db_mod2_jump_split_mod_occupied_entry_bm[port] != NULL) {
			vfree(tpm_db_mod2_jump_split_mod_occupied_entry_bm[port]);
			tpm_db_mod2_jump_split_mod_occupied_entry_bm[port] = NULL;
		}
		tpm_db_mod2_jump_split_mod_occupied_entry_bm[port] = (uint32_t *) vmalloc(bm_size);
		if (tpm_db_mod2_jump_split_mod_occupied_entry_bm[port] == NULL) {
			TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
			return TPM_DB_ERR_MEM_ALLOC_FAIL;
		}
		memset(tpm_db_mod2_jump_split_mod_occupied_entry_bm[port], 0xff, bm_size);
		tpm_db_mod2_jump_split_mod_occupied_entry_bm[port][0] |= 0x1;

		if (tpm_db_mod2_jump_split_mod_all_entry_bm[port] != NULL) {
			vfree(tpm_db_mod2_jump_split_mod_all_entry_bm[port]);
			tpm_db_mod2_jump_split_mod_all_entry_bm[port] = NULL;
		}
		tpm_db_mod2_jump_split_mod_all_entry_bm[port] = (uint32_t *) vmalloc(bm_size);
		if (tpm_db_mod2_jump_split_mod_all_entry_bm[port] == NULL) {
			TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
			return TPM_DB_ERR_MEM_ALLOC_FAIL;
		}
		memset(tpm_db_mod2_jump_split_mod_all_entry_bm[port], 0, bm_size);
	}

	/* split mod init */
	if (TPM_SPLIT_MOD_DISABLED == tpm_db_split_mod_get_enable())
	{
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "SPLIT_MOD_DISABLED\n");
		return(TPM_OK);
	}

	vlan_nums = tpm_db_split_mod_get_num_vlans();
	p_bits_nums = TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX;


	tpm_db_mod2_jump_area_cfg.next_free[port] = p_bits_nums + 1;
	//tpm_db_mod2_jump_area_cfg.next_split[port] = 16;

	for (loop = 0; loop <= vlan_nums; loop++) {
		for (loop_in = 0; loop_in <= p_bits_nums; loop_in++) {
			cur_entry = (loop * 16) + loop_in;
			i = j = 0;
			j = cur_entry & 0x1f;
			i = cur_entry >> 5;
			mask = 1 << j;
			tpm_db_mod2_jump_split_mod_all_entry_bm[port][i] |= mask;
			//tpm_db_mod2_jump_occupied_entry_bm[port][i] |= mask;
			//tpm_db_mod2_jump_split_mod_occupied_entry_bm[port][i] &= (~mask);
		}
	}
	//tpm_db_mod2_jump_split_mod_occupied_entry_bm[port][0] |= (0x1);

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_setup_chain_area(tpm_gmacs_enum_t port)
{
	uint32_t size, /* port, */ chain;
	uint16_t base;

	printk(" ===== chain_area: base entry 0x%0x ===== \r\n", tpm_mod2_chain_area_base);

	/*for (port = TPM_ENUM_GMAC_0; port < TPM_MAX_NUM_GMACS; port++) */
	{
		printk("[GMAC%d]:\r\n", port);
		base = tpm_mod2_chain_area_base;
		for (chain = TPM_CHAIN_TYPE_MH; chain < TPM_CHAIN_TYPE_MAX; chain++) {
			if (tpm_db_mod2_chain_cfg[chain].total_num != 0) {
				tpm_db_mod2_chain_cfg[chain].base_entry = base;
				tpm_db_mod2_chain_cfg[chain].next_free[TPM_ENUM_GMAC_0] = 0;
				tpm_db_mod2_chain_cfg[chain].next_free[TPM_ENUM_GMAC_1] = 0;
				tpm_db_mod2_chain_cfg[chain].next_free[TPM_ENUM_PMAC] = 0;

				size = tpm_db_mod2_chain_cfg[chain].total_num * sizeof(tpm_mod2_chain_info_t);

				printk("\t chain-%d, 0x%0x - 0x%0x, chain_num: %d\r\n",
				       chain, base,
				       base +
				       tpm_db_mod2_chain_cfg[chain].total_num * tpm_db_mod2_chain_cfg[chain].max_size -
				       1, tpm_db_mod2_chain_cfg[chain].total_num);

				if (tpm_db_mod2_chain_info[port][chain] != NULL) {
					vfree(tpm_db_mod2_chain_info[port][chain]);
					tpm_db_mod2_chain_info[port][chain] = NULL;
				}
				tpm_db_mod2_chain_info[port][chain] = (tpm_mod2_chain_info_t *) vmalloc(size);
				if (tpm_db_mod2_chain_info[port][chain] == NULL) {
					TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
					return TPM_DB_ERR_MEM_ALLOC_FAIL;
				}
				memset(tpm_db_mod2_chain_info[port][chain], 0, size);

				if (tpm_db_mod2_chain_cfg[chain].total_num % 32) {
					size = (tpm_db_mod2_chain_cfg[chain].total_num / 32 + 1) * sizeof(uint32_t);
					tpm_db_mod2_chain_bm_group[chain] =
					    tpm_db_mod2_chain_cfg[chain].total_num / 32 + 1;
				} else {
					size = (tpm_db_mod2_chain_cfg[chain].total_num / 32) * sizeof(uint32_t);
					tpm_db_mod2_chain_bm_group[chain] = tpm_db_mod2_chain_cfg[chain].total_num / 32;
				}

				if (tpm_db_mod2_chain_booked_entry_bm[port][chain] != NULL) {
					vfree(tpm_db_mod2_chain_booked_entry_bm[port][chain]);
					tpm_db_mod2_chain_booked_entry_bm[port][chain] = NULL;
				}
				tpm_db_mod2_chain_booked_entry_bm[port][chain] = (uint32_t *) vmalloc(size);
				if (tpm_db_mod2_chain_booked_entry_bm[port][chain] == NULL) {
					TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
					return TPM_DB_ERR_MEM_ALLOC_FAIL;
				}
				memset(tpm_db_mod2_chain_booked_entry_bm[port][chain], 0, size);

				if (tpm_db_mod2_chain_occupied_entry_bm[port][chain] != NULL) {
					vfree(tpm_db_mod2_chain_occupied_entry_bm[port][chain]);
					tpm_db_mod2_chain_occupied_entry_bm[port][chain] = NULL;
				}
				tpm_db_mod2_chain_occupied_entry_bm[port][chain] = (uint32_t *) vmalloc(size);
				if (tpm_db_mod2_chain_occupied_entry_bm[port][chain] == NULL) {
					TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
					return TPM_DB_ERR_MEM_ALLOC_FAIL;
				}
				memset(tpm_db_mod2_chain_occupied_entry_bm[port][chain], 0, size);

				base += tpm_db_mod2_chain_cfg[chain].total_num * tpm_db_mod2_chain_cfg[chain].max_size;
			}
		}

		if (tpm_db_mod2_chain_pmt_info[port] != NULL) {
			vfree(tpm_db_mod2_chain_pmt_info[port]);
			tpm_db_mod2_chain_pmt_info[port] = NULL;
		}
		size = tpm_mod2_total_chain_num * sizeof(tpm_mod2_chain_pmt_info_t);
		tpm_db_mod2_chain_pmt_info[port] = (tpm_mod2_chain_pmt_info_t *) vmalloc(size);
		if (tpm_db_mod2_chain_pmt_info[port] == NULL) {
			TPM_OS_ERROR(TPM_DB_MOD, "Memory allocation failure \n");
			return TPM_DB_ERR_MEM_ALLOC_FAIL;
		}
		memset(tpm_db_mod2_chain_pmt_info[port], 0, size);
	}

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_setup(void)
{
	uint32_t port;

	if (tpm_db_mod2_validate_chain_config() != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_DB_MOD, "Chain config validation failure \n");
		return TPM_FAIL;
	}

	for (port = TPM_ENUM_GMAC_0; port < TPM_MAX_NUM_GMACS; port++) {
		if (tpm_db_mod2_setup_jump_area(port) != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_DB_MOD, "Failed to setup jump area \n");
			return TPM_FAIL;
		}
		if (tpm_db_mod2_setup_chain_area(port) != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_DB_MOD, "Failed to setup chain area \n");
			return TPM_FAIL;
		}
	}

	/* init split mod data */
	memset(&(tpm_db.split_mod_conf.gmac_vlan_conf), 0, sizeof(tpm_db.split_mod_conf.gmac_vlan_conf));

	return TPM_DB_OK;
}

int32_t tpm_db_mod2_inv_mac_entries(tpm_gmacs_enum_t gmac_port)
{
	if (tpm_db_mod2_setup_jump_area(gmac_port) != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_DB_MOD, "Failed to setup jump area \n");
		return TPM_FAIL;
	}
	if (tpm_db_mod2_setup_chain_area(gmac_port) != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_DB_MOD, "Failed to setup chain area \n");
		return TPM_FAIL;
	}

	return TPM_DB_OK;
}

void tpm_db_mod2_show_jump_pmt_entries(tpm_gmacs_enum_t gmac_port)
{
	uint16_t entry_idx;

	if (gmac_port >= TPM_MAX_NUM_GMACS)
		return;

	printk("              STATUS        OPCODE | DATA | LAST | UPDT_IP | UPDT_CHK \r\n");
	for (entry_idx = 0; entry_idx < tpm_db_mod2_jump_area_cfg.total_num; entry_idx++) {
		if (tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].status != TPM_MOD_ENTRY_FREE) {
			printk("[PMT 0x%04x]: %s             0x%04x | 0x%04x | %d | %d | %d \r\n", entry_idx,
			       entry_state_str[tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].status],
			       tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].pattern.opcode,
			       tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].pattern.data,
			       tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].pattern.last,
			       tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].pattern.updt_ipv4,
			       tpm_db_mod2_jump_pmt_info[gmac_port][entry_idx].pattern.updt_tcp);
		}
	}
}

void tpm_db_mod2_show_jump_cfg_data(void)
{
	uint32_t i;
	printk("[JUMP AREA]: base_entry 0x%04x, total_num %d, split_mod_num %d, split_mod_pbit_num %d\r\n",
	       tpm_db_mod2_jump_area_cfg.base_entry, tpm_db_mod2_jump_area_cfg.total_num,
	       tpm_db_mod2_jump_area_cfg.split_num, tpm_db_mod2_jump_area_cfg.split_pbit_num);

	printk("[NEXT FREE]: GMAC0(G/S)       GMAC1(G/S)       PMAC(G/S)    \r\n");
	printk("[NEXT FREE]: 0x%04x/0x%04x    0x%04x/0x%04x    0x%04x/0x%04x    \r\n",
	       tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_GMAC_0],
	       tpm_db_mod2_jump_area_cfg.next_split[TPM_ENUM_GMAC_0],
	       tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_GMAC_1],
	       tpm_db_mod2_jump_area_cfg.next_split[TPM_ENUM_GMAC_1],
	       tpm_db_mod2_jump_area_cfg.next_free[TPM_ENUM_PMAC], tpm_db_mod2_jump_area_cfg.next_split[TPM_ENUM_PMAC]);
#if 1
	printk("[BOOKED_MAP]:\r\n");

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		 printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_booked_entry_bm[TPM_ENUM_GMAC_0][i]);

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_booked_entry_bm[TPM_ENUM_GMAC_1][i]);

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_booked_entry_bm[TPM_ENUM_PMAC][i]);

	printk("[OCCUPIED_MAP]:\r\n");

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_occupied_entry_bm[TPM_ENUM_GMAC_0][i]);

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_occupied_entry_bm[TPM_ENUM_GMAC_1][i]);

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_occupied_entry_bm[TPM_ENUM_PMAC][i]);

	printk("[SPLIT MOD OCCUPIED_MAP]:\r\n");

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_split_mod_occupied_entry_bm[TPM_ENUM_GMAC_0][i]);

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_split_mod_occupied_entry_bm[TPM_ENUM_GMAC_1][i]);

	for (i = 0; i < tpm_db_mod2_jump_bm_group; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_jump_split_mod_occupied_entry_bm[TPM_ENUM_PMAC][i]);
#endif
}

void tpm_db_mod2_show_chain_pmt_entries(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type)
{
	uint16_t entry_idx, pmt_idx, entry_num;

	if (gmac_port >= TPM_MAX_NUM_GMACS)
		return;

	if (chain_type >= TPM_CHAIN_TYPE_MAX)
		return;

	printk("              STATUS  CHAIN/ID     OPCODE | DATA | LAST | UPDT_IP | UPDT_CHK \r\n");
	if (chain_type == TPM_CHAIN_TYPE_NONE) {
		for (chain_type = TPM_CHAIN_TYPE_MH; chain_type < TPM_CHAIN_TYPE_MAX; chain_type++) {
			if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0)
				continue;

			entry_num = tpm_db_mod2_chain_cfg[chain_type].total_num * tpm_db_mod2_chain_cfg[chain_type].max_size;

			for (entry_idx = 0; entry_idx < entry_num; entry_idx++) {
				pmt_idx = entry_idx + tpm_db_mod2_chain_cfg[chain_type].base_entry - tpm_mod2_chain_area_base;
				if (tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].status != TPM_MOD_ENTRY_FREE) {
					printk("[PMT 0x%04x]: %s       %d/%d          0x%04x | 0x%04x | %d | %d | %d \r\n",
					       tpm_mod2_chain_area_base + pmt_idx,
					       entry_state_str[tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].status],
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].chain_type,
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].chain_id,
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.opcode,
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.data,
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.last,
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.updt_ipv4,
					       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.updt_tcp);
				}
			}
		}
	} else {
		if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0)
			return;

		entry_num = tpm_db_mod2_chain_cfg[chain_type].total_num * tpm_db_mod2_chain_cfg[chain_type].max_size;

		for (entry_idx = 0; entry_idx < entry_num; entry_idx++) {
			pmt_idx = entry_idx + tpm_db_mod2_chain_cfg[chain_type].base_entry - tpm_mod2_chain_area_base;
			if (tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].status != TPM_MOD_ENTRY_FREE) {
				printk("[PMT 0x%04x]: %s       %d/%d          0x%04x | 0x%04x | %d | %d | %d \r\n",
				       tpm_mod2_chain_area_base + pmt_idx,
				       entry_state_str[tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].status],
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].chain_type,
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].chain_id,
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.opcode,
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.data,
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.last,
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.updt_ipv4,
				       tpm_db_mod2_chain_pmt_info[gmac_port][pmt_idx].pattern.updt_tcp);
			}
		}
	}
}

void tpm_db_mod2_show_chain_info_entries(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id)
{
    uint32_t i;

	if (gmac_port >= TPM_MAX_NUM_GMACS)
		return;

	if (chain_type > TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE)
		return;

	if (tpm_db_mod2_chain_cfg[chain_type].total_num == 0)
		return;

	printk("[CHAIN-%d Valid Entries]:\r\n", chain_type);

	if (chain_id == TPM_MOD2_INVALID_CHAIN_ID) {
		for (chain_id = 0; chain_id < tpm_db_mod2_chain_cfg[chain_type].total_num; chain_id++) {
			if (tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].status != TPM_MOD_ENTRY_FREE) {
				if (chain_id < tpm_db_mod2_chain_cfg[chain_type].total_num) {
					printk("                   STATUS   BASE_ENTR   ENTR_NUM USER_NUM\r\n");
					printk("[CHAIN-%d  0x%04x]: %s        0x%04x      %d        %d \r\n",
					       chain_type, chain_id,
					       entry_state_str[tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].
							       status],
					       tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].base_entry +
					       tpm_mod2_chain_area_base,
					       tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].entry_num,
					       tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].user_num);
				}
			}
		}
	} else {
		if (tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].status != TPM_MOD_ENTRY_FREE) {
			if (chain_id < tpm_db_mod2_chain_cfg[chain_type].total_num) {
				printk("[CHAIN-%d  0x%04x]: %s        0x%04x     %d      %d \r\n", chain_type, chain_id,
				       entry_state_str[tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].status],
				       tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].base_entry +
				       tpm_mod2_chain_area_base,
				       tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].entry_num,
				       tpm_db_mod2_chain_info[gmac_port][chain_type][chain_id].user_num);
			}
		}
	}
#if 1
	printk("[CHAIN-%d BOOKED_MAP]:\r\n", chain_type);

	for (i = 0; i < tpm_db_mod2_chain_bm_group[chain_type]; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_chain_booked_entry_bm[gmac_port][chain_type][i]);

	printk("[CHAIN-%d OCCUPIED_MAP]:\r\n", chain_type);

	for (i = 0; i < tpm_db_mod2_chain_bm_group[chain_type]; i++)
		printk("%02d - 0x%08x\r\n", i, tpm_db_mod2_chain_occupied_entry_bm[gmac_port][chain_type][i]);
#endif
	printk("\r\n");
}

void tpm_db_mod2_show_all_chain(tpm_gmacs_enum_t gmac_port)
{
	uint16_t chain_idx;

	for (chain_idx = TPM_CHAIN_TYPE_MH; chain_idx < TPM_CHAIN_TYPE_MAX; chain_idx++)
		tpm_db_mod2_show_chain_info_entries(gmac_port, chain_idx, TPM_MOD2_INVALID_CHAIN_ID);
}

void tpm_db_mod2_show_chain_cfg_data(tpm_chain_type_t chain_type)
{
	if (chain_type >= TPM_CHAIN_TYPE_MAX || chain_type == TPM_CHAIN_TYPE_NONE)
		return;

	printk("[CHAIN   %d]: base_entry 0x%04x, total_num %d, max_size %d, subr_chain %d\r\n", chain_type,
	       tpm_db_mod2_chain_cfg[chain_type].base_entry, tpm_db_mod2_chain_cfg[chain_type].total_num,
	       tpm_db_mod2_chain_cfg[chain_type].max_size, tpm_db_mod2_chain_cfg[chain_type].subr_chain);
	printk("[NEXT FREE]: GMAC0     GMAC1     PMAC    \r\n");
	printk("[NEXT FREE]: 0x%04x    0x%04x    0x%04x    \r\n",
	       tpm_db_mod2_chain_cfg[chain_type].next_free[TPM_ENUM_GMAC_0],
	       tpm_db_mod2_chain_cfg[chain_type].next_free[TPM_ENUM_GMAC_1],
	       tpm_db_mod2_chain_cfg[chain_type].next_free[TPM_ENUM_PMAC]);
}

/*******************************************************************************
* tpm_db_mod2_init()
*
* DESCRIPTION: The API initializes shadow DB and modification patterns
*
* INPUTS:   None
*
* OUTPUTS:
*           None
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_db_mod2_init(void)
{
	tpm_db_mod2_init_jump_area_cfg();
	tpm_db_mod2_init_chain_cfg();

	tpm_db_mod2_init_pmt_info();
	tpm_db_mod2_init_chain_info();

	return TPM_DB_OK;
}

/*******************************************************************************
* tpm_db_mtu_set_ipv4_us()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_set_ipv4_mtu_us(uint32_t mtu)
{
	tpm_db.tpm_mtu_cfg.ipv4_mtu_us = mtu;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv4_mtu_us()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_get_ipv4_mtu_us(uint32_t *mtu)
{
	*mtu = tpm_db.tpm_mtu_cfg.ipv4_mtu_us;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv4_pppoe_mtu_us()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_set_ipv4_pppoe_mtu_us(uint32_t mtu)
{
	tpm_db.tpm_mtu_cfg.ipv4_pppoe_mtu_us = mtu;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv4_pppoe_mtu_us()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_get_ipv4_pppoe_mtu_us(uint32_t *mtu)
{
	*mtu = tpm_db.tpm_mtu_cfg.ipv4_pppoe_mtu_us;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv6_mtu_us()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_set_ipv6_mtu_us(uint32_t mtu)
{
	tpm_db.tpm_mtu_cfg.ipv6_mtu_us = mtu;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv6_mtu_us()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_get_ipv6_mtu_us(uint32_t *mtu)
{
	*mtu = tpm_db.tpm_mtu_cfg.ipv6_mtu_us;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv6_pppoe_mtu_us()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_set_ipv6_pppoe_mtu_us(uint32_t mtu)
{
	tpm_db.tpm_mtu_cfg.ipv6_pppoe_mtu_us = mtu;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv6_pppoe_mtu_us()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_get_ipv6_pppoe_mtu_us(uint32_t *mtu)
{
	*mtu = tpm_db.tpm_mtu_cfg.ipv6_pppoe_mtu_us;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv4_mtu_ds()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_set_ipv4_mtu_ds(uint32_t mtu)
{
	tpm_db.tpm_mtu_cfg.ipv4_mtu_ds = mtu;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv4_mtu_ds()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_get_ipv4_mtu_ds(uint32_t *mtu)
{
	*mtu = tpm_db.tpm_mtu_cfg.ipv4_mtu_ds;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv6_mtu_ds()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_set_ipv6_mtu_ds(uint32_t mtu)
{
	tpm_db.tpm_mtu_cfg.ipv6_mtu_ds = mtu;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv6_mtu_ds()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mtu_get_ipv6_mtu_ds(uint32_t *mtu)
{
	*mtu = tpm_db.tpm_mtu_cfg.ipv6_mtu_ds;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv6_mtu_ds()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_set_num_vlan_tags(uint32_t number)
{
	tpm_db.num_vlan_tags = number;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv6_mtu_ds()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_get_num_vlan_tags(uint32_t *number)
{
	*number = tpm_db.num_vlan_tags;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv6_mtu_ds()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_set_pppoe_add_enable(tpm_db_pppoe_add_enable_t enable)
{
	tpm_db.tpm_pppoe_add_enable = enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv6_mtu_ds()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_get_pppoe_add_enable(tpm_db_pppoe_add_enable_t *enable)
{
	*enable = tpm_db.tpm_pppoe_add_enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_set_ipv6_mtu_ds()
*
* DESCRIPTION:      Set queue number which IGMP packets are forwarded to
*
* INPUTS:
* igmp_snoop       - queue number
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_set_mtu_enable(tpm_db_mtu_setting_enable_t enable)
{
	tpm_db.tpm_mtu_cfg.mtu_enable = enable;
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mtu_get_ipv6_mtu_ds()
*
* DESCRIPTION:      Get queue number which IGMP packets are forwarded to
*
* INPUTS:
*
* OUTPUTS:
* igmp_snoop       - queue number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_get_mtu_enable(tpm_db_mtu_setting_enable_t *enable)
{
	*enable = tpm_db.tpm_mtu_cfg.mtu_enable;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_cpu_rx_queue(uint32_t cpu_rx_queue)
{
	tpm_db.cpu_rx_queue = cpu_rx_queue;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_cpu_rx_queue(uint32_t *cpu_rx_queue)
{
	*cpu_rx_queue = tpm_db.cpu_rx_queue;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_ttl_illegal_action(uint32_t ttl_illegal_action)
{
	tpm_db.ttl_illegal_action = ttl_illegal_action;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_ttl_illegal_action(uint32_t *ttl_illegal_action)
{
	*ttl_illegal_action = tpm_db.ttl_illegal_action;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_tcp_flag_check(uint32_t tcp_flag_check)
{
	tpm_db.tcp_flag_check = tcp_flag_check;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_tcp_flag_check(uint32_t *tcp_flag_check)
{
	*tcp_flag_check = tpm_db.tcp_flag_check;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_mru(uint32_t mru)
{
	tpm_db.tpm_mtu_cfg.mru = mru;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_mru(uint32_t *mru)
{
	*mru = tpm_db.tpm_mtu_cfg.mru;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_catch_all_pkt_action(tpm_init_pnc_last_init_t catch_all_pkt_action)
{
	tpm_db.catch_all_pkt_action = catch_all_pkt_action;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_catch_all_pkt_action(tpm_init_pnc_last_init_t *catch_all_pkt_action)
{
	*catch_all_pkt_action = tpm_db.catch_all_pkt_action;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_switch_dev_num(int32_t switch_dev_num)
{
	tpm_db.switch_dev_num = switch_dev_num;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_switch_dev_num(int32_t *switch_dev_num)
{
	*switch_dev_num = tpm_db.switch_dev_num;
	return (TPM_DB_OK);
}

int32_t tpm_db_set_ety_dsa_enable(tpm_db_ety_dsa_enable_t ety_dsa_enable)
{
	tpm_db.func_profile.ety_dsa_enable = ety_dsa_enable;
	return (TPM_DB_OK);
}

int32_t tpm_db_get_ety_dsa_enable(tpm_db_ety_dsa_enable_t *ety_dsa_enable)
{
	*ety_dsa_enable = tpm_db.func_profile.ety_dsa_enable;
	return (TPM_DB_OK);
}

void tpm_db_show_mtu_cfg(void)
{
	tpm_db_mtu_setting_enable_t mtu_setting_enable;
	uint32_t mtu, mru;
	uint32_t num_vlan_tags;
	tpm_db_pppoe_add_enable_t pppoe_add_enable;

	tpm_db_get_mtu_enable(&mtu_setting_enable);
	printk("mtu_config: %d, \n", mtu_setting_enable);

	tpm_db_mtu_get_ipv4_mtu_us(&mtu);
	printk("ipv4_mtu_us: %d, \n", mtu);

	tpm_db_mtu_get_ipv4_pppoe_mtu_us(&mtu);
	printk("ipv4_pppoe_mtu_us: %d, \n", mtu);

	tpm_db_mtu_get_ipv6_mtu_us(&mtu);
	printk("ipv6_mtu_us: %d, \n", mtu);

	tpm_db_mtu_get_ipv6_pppoe_mtu_us(&mtu);
	printk("ipv6_pppoe_mtu_us: %d, \n", mtu);

	tpm_db_mtu_get_ipv4_mtu_ds(&mtu);
	printk("ipv4_mtu_ds: %d, \n", mtu);

	tpm_db_mtu_get_ipv6_mtu_ds(&mtu);
	printk("ipv6_mtu_ds: %d, \n", mtu);

	tpm_db_get_pppoe_add_enable(&pppoe_add_enable);
	printk("pppoe_add_enable: %d, \n", pppoe_add_enable);

	tpm_db_get_num_vlan_tags(&num_vlan_tags);
	printk("num_vlan_tags: %d, \n", num_vlan_tags);

	tpm_db_get_mru(&mru);
	printk("mru: %d, \n", mru);
}

void tpm_db_api_busy_init(void)
{
	memset(tpm_db.tpm_busy_apis, TPM_DB_API_NOT_BUSY, sizeof(tpm_db.tpm_busy_apis));
}

uint32_t tpm_db_get_api_busy(tpm_api_type_t api_type)
{
	uint32_t i;

	for (i=0;i<TPM_MAX_PARALLEL_API_CALLS;i++) {
		if (tpm_db.tpm_busy_apis[api_type][i] != TPM_DB_API_NOT_BUSY)
			return(TPM_TRUE);
	}
	return(TPM_FALSE);
}

uint32_t tpm_db_get_api_rule_num_busy(tpm_api_type_t api_type, uint32_t rule_num)
{
	uint32_t i;

	for (i=0;i<TPM_MAX_PARALLEL_API_CALLS;i++) {
		if ( tpm_db.tpm_busy_apis[api_type][i] == rule_num)
			return(TPM_TRUE);
	}
	return(TPM_FALSE);
}


void tpm_db_get_api_all_busy(tpm_api_type_t api_type, uint32_t *num_busy, uint32_t *rule_num_list)
{
	uint32_t i, l_num_busy=0;

	for (i=0;i<TPM_MAX_PARALLEL_API_CALLS;i++) {
		if ( tpm_db.tpm_busy_apis[api_type][i] != TPM_DB_API_NOT_BUSY) {
			rule_num_list[l_num_busy] = tpm_db.tpm_busy_apis[api_type][i];
			l_num_busy++;
		}
	}
	*num_busy = l_num_busy;
	return;
}



uint32_t tpm_db_set_api_busy(tpm_api_type_t api_type, uint32_t rule_num)
{
	uint32_t i;

	for (i=0;i<TPM_MAX_PARALLEL_API_CALLS;i++) {
		if ( tpm_db.tpm_busy_apis[api_type][i] == TPM_DB_API_NOT_BUSY){
			tpm_db.tpm_busy_apis[api_type][i] = rule_num;
			return (TPM_DB_OK);
		}
	}
	return(TPM_DB_ERR_DB_TBL_FULL);
}


uint32_t tpm_db_set_api_free(tpm_api_type_t api_type, uint32_t rule_num)
{
	uint32_t i;

	for (i=0;i<TPM_MAX_PARALLEL_API_CALLS;i++) {
		if ( tpm_db.tpm_busy_apis[api_type][i] == rule_num){
			tpm_db.tpm_busy_apis[api_type][i] = -1;
			return (TPM_DB_OK);
		}
	}
	return(TPM_DB_ERR_REC_NOT_EXIST);
}



uint32_t tpm_db_find_matched_ipv6_gen_key(tpm_ipv6_gen_acl_key_t *gen_key)
{
	uint32_t i;
	tpm_db_ipv6_gen_key_t *key_array = &tpm_db.ipv6_key_shadow.gen_key[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_KEY_NUM; i++) {
		if (key_array[i].valid == TPM_FALSE)
			continue;
		if (!memcmp(&key_array[i].gen_key, gen_key, sizeof(tpm_ipv6_gen_acl_key_t)))
			return i;
	}

	return TPM_DB_INVALID_IPV6_KEY_ID;
}

uint32_t tpm_db_get_ipv6_gen_key_user_num(uint32_t key_id)
{
	return tpm_db.ipv6_key_shadow.gen_key[key_id].user_num;
}

void tpm_db_increase_ipv6_gen_key_user_num(uint32_t key_id)
{
	tpm_db.ipv6_key_shadow.gen_key[key_id].user_num++;
}

void tpm_db_decrease_ipv6_gen_key_user_num(uint32_t key_id)
{
	if (tpm_db.ipv6_key_shadow.gen_key[key_id].user_num > 0)
		tpm_db.ipv6_key_shadow.gen_key[key_id].user_num--;
}

uint32_t tpm_db_set_ipv6_gen_key(tpm_ipv6_gen_acl_key_t *gen_key)
{
	uint32_t i, key_id = TPM_DB_INVALID_IPV6_KEY_ID;
	tpm_db_ipv6_gen_key_t *key_array = &tpm_db.ipv6_key_shadow.gen_key[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_KEY_NUM; i++) {
		if (key_array[i].valid == TPM_FALSE) {
			key_id = i;
			break;
		}
	}

	if (key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
		return TPM_DB_INVALID_IPV6_KEY_ID;
	}

	memset(&key_array[key_id], 0, sizeof(tpm_db_ipv6_gen_key_t));

	key_array[key_id].valid = TPM_TRUE;
	memcpy(&key_array[i].gen_key, gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	return key_id;
}

int32_t tpm_db_get_ipv6_gen_key(uint32_t key_id, tpm_ipv6_gen_acl_key_t *gen_key)
{
	tpm_db_ipv6_gen_key_t *key_array = &tpm_db.ipv6_key_shadow.gen_key[0];

	if (key_id >= TPM_DB_IPV6_MAX_KEY_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (key_array[key_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;

	memcpy(gen_key, &key_array[key_id].gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv6_gen_key(uint32_t key_id)
{
	uint32_t user_num;
	tpm_db_ipv6_gen_key_t *key_array = &tpm_db.ipv6_key_shadow.gen_key[0];

	if (key_id >= TPM_DB_IPV6_MAX_KEY_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (key_array[key_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_STATUS_ERR;

	user_num = tpm_db_get_ipv6_gen_key_user_num(key_id);

	if (user_num == 0)
		memset(&key_array[key_id], 0, sizeof(tpm_db_ipv6_gen_key_t));

	return TPM_DB_OK;
}

uint32_t tpm_db_find_matched_ipv6_dip_key(tpm_ipv6_addr_key_t *dip_key)
{
	uint32_t i;
	tpm_db_ipv6_dip_key_t *key_array = &tpm_db.ipv6_key_shadow.dip_key[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_KEY_NUM; i++) {
		if (key_array[i].valid == TPM_FALSE)
			continue;
		if (!memcmp(&key_array[i].dip_key, dip_key, sizeof(tpm_ipv6_addr_key_t)))
			return i;
	}

	return TPM_DB_INVALID_IPV6_KEY_ID;
}

uint32_t tpm_db_get_ipv6_dip_key_user_num(uint32_t key_id)
{
	return tpm_db.ipv6_key_shadow.dip_key[key_id].user_num;
}

void tpm_db_increase_ipv6_dip_key_user_num(uint32_t key_id)
{
	tpm_db.ipv6_key_shadow.dip_key[key_id].user_num++;
}

void tpm_db_decrease_ipv6_dip_key_user_num(uint32_t key_id)
{
	if (tpm_db.ipv6_key_shadow.dip_key[key_id].user_num > 0)
		tpm_db.ipv6_key_shadow.dip_key[key_id].user_num--;
}

uint32_t tpm_db_set_ipv6_dip_key(tpm_ipv6_addr_key_t *dip_key)
{
	uint32_t i, key_id = TPM_DB_INVALID_IPV6_KEY_ID;
	tpm_db_ipv6_dip_key_t *key_array = &tpm_db.ipv6_key_shadow.dip_key[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_KEY_NUM; i++) {
		if (key_array[i].valid == TPM_FALSE) {
			key_id = i;
			break;
		}
	}

	if (key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
		return TPM_DB_INVALID_IPV6_KEY_ID;
	}

	memset(&key_array[key_id], 0, sizeof(tpm_db_ipv6_dip_key_t));

	key_array[key_id].valid = TPM_TRUE;
	memcpy(&key_array[i].dip_key, dip_key, sizeof(tpm_ipv6_addr_key_t));

	return key_id;
}

int32_t tpm_db_get_ipv6_dip_key(uint32_t key_id, tpm_ipv6_addr_key_t *dip_key)
{
	tpm_db_ipv6_dip_key_t *key_array = &tpm_db.ipv6_key_shadow.dip_key[0];

	if (key_id >= TPM_DB_IPV6_MAX_KEY_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (key_array[key_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;

	memcpy(dip_key, &key_array[key_id].dip_key, sizeof(tpm_ipv6_addr_key_t));

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv6_dip_key(uint32_t key_id)
{
	uint32_t user_num;
	tpm_db_ipv6_dip_key_t *key_array = &tpm_db.ipv6_key_shadow.dip_key[0];

	if (key_id >= TPM_DB_IPV6_MAX_KEY_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (key_array[key_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_STATUS_ERR;

	user_num = tpm_db_get_ipv6_dip_key_user_num(key_id);

	if (user_num == 0)
		memset(&key_array[key_id], 0, sizeof(tpm_db_ipv6_dip_key_t));

	return TPM_DB_OK;
}

uint32_t tpm_db_find_matched_ipv6_l4_key(uint32_t protocol, tpm_l4_ports_key_t *l4_key)
{
	uint32_t i;
	tpm_db_ipv6_l4_key_t *key_array = &tpm_db.ipv6_key_shadow.l4_key[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_KEY_NUM; i++) {
		if (key_array[i].valid == TPM_FALSE)
			continue;
		if (!memcmp(&key_array[i].l4_key, l4_key, sizeof(tpm_l4_ports_key_t)) &&
			key_array[i].protocol == protocol)
			return i;
	}

	return TPM_DB_INVALID_IPV6_KEY_ID;
}

uint32_t tpm_db_get_ipv6_l4_key_user_num(uint32_t key_id)
{
	return tpm_db.ipv6_key_shadow.l4_key[key_id].user_num;
}

void tpm_db_increase_ipv6_l4_key_user_num(uint32_t key_id)
{
	tpm_db.ipv6_key_shadow.l4_key[key_id].user_num++;
}

void tpm_db_decrease_ipv6_l4_key_user_num(uint32_t key_id)
{
	if (tpm_db.ipv6_key_shadow.l4_key[key_id].user_num > 0)
		tpm_db.ipv6_key_shadow.l4_key[key_id].user_num--;
}

uint32_t tpm_db_set_ipv6_l4_key(uint32_t protocol, tpm_l4_ports_key_t *l4_key)
{
	uint32_t i, key_id = TPM_DB_INVALID_IPV6_KEY_ID;
	tpm_db_ipv6_l4_key_t *key_array = &tpm_db.ipv6_key_shadow.l4_key[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_KEY_NUM; i++) {
		if (key_array[i].valid == TPM_FALSE) {
			key_id = i;
			break;
		}
	}

	if (key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
		return TPM_DB_INVALID_IPV6_KEY_ID;
	}

	memset(&key_array[key_id], 0, sizeof(tpm_db_ipv6_l4_key_t));

	key_array[key_id].valid = TPM_TRUE;
	memcpy(&key_array[i].l4_key, l4_key, sizeof(tpm_l4_ports_key_t));
	key_array[key_id].protocol = protocol;

	return key_id;
}

int32_t tpm_db_get_ipv6_l4_key(uint32_t key_id, uint32_t *protocol, tpm_l4_ports_key_t *l4_key)
{
	tpm_db_ipv6_l4_key_t *key_array = &tpm_db.ipv6_key_shadow.l4_key[0];

	if (key_id >= TPM_DB_IPV6_MAX_KEY_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (key_array[key_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;

	memcpy(l4_key, &key_array[key_id].l4_key, sizeof(tpm_l4_ports_key_t));
	*protocol = key_array[key_id].protocol;

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv6_l4_key(uint32_t key_id)
{
	uint32_t user_num;
	tpm_db_ipv6_l4_key_t *key_array = &tpm_db.ipv6_key_shadow.l4_key[0];

	if (key_id >= TPM_DB_IPV6_MAX_KEY_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (key_array[key_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_STATUS_ERR;

	user_num = tpm_db_get_ipv6_l4_key_user_num(key_id);

	if (user_num == 0)
		memset(&key_array[key_id], 0, sizeof(tpm_db_ipv6_l4_key_t));

	return TPM_DB_OK;
}

uint32_t tpm_db_find_matched_ipv6_l4_subflow(uint32_t src_dir, uint32_t parse_bm, uint32_t protocol, tpm_l4_ports_key_t *l4_key)
{
	int32_t ret;
	uint32_t i, _protocol;
	tpm_db_ipv6_l4_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[0];
	tpm_l4_ports_key_t _l4_key;

	for (i = 0; i < TPM_DB_IPV6_MAX_SUBFLOW_NUM; i++) {
		if (flow_array[i].valid == TPM_FALSE)
			continue;

		if (parse_bm == flow_array[i].parse_bm) {
			memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
			ret = tpm_db_get_ipv6_l4_key(flow_array[i].l4_key, &_protocol, &_l4_key);
			if (ret != TPM_DB_OK)
				break;

			if (_protocol != protocol)
				continue;

			if (parse_bm & TPM_PARSE_L4_SRC) {
				if (_l4_key.l4_src_port != l4_key->l4_src_port)
					continue;
			}

			if (parse_bm & TPM_PARSE_L4_DST) {
				if (_l4_key.l4_dst_port != l4_key->l4_dst_port)
					continue;
			}

			return i;
		}
	}

	if (i < TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		TPM_OS_WARN(TPM_DB_MOD, "Warning: DB inconsistency\n");

	return TPM_DB_INVALID_IPV6_FLOW_ID;
}

uint32_t tpm_db_get_ipv6_l4_subflow_user_num(uint32_t src_dir, uint32_t flow_id)
{
	return tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[flow_id].user_num;
}

void tpm_db_increase_ipv6_l4_subflow_user_num(uint32_t src_dir, uint32_t flow_id)
{
	tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[flow_id].user_num++;
}

void tpm_db_decrease_ipv6_l4_subflow_user_num(uint32_t src_dir, uint32_t flow_id)
{
	if (tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[flow_id].user_num > 0)
		tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[flow_id].user_num--;
}

uint32_t tpm_db_get_free_ipv6_l4_subflow(uint32_t src_dir)
{
	uint32_t i;
	tpm_db_ipv6_l4_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_SUBFLOW_NUM; i++) {
		if (flow_array[i].valid == TPM_FALSE)
			return i;
	}

	return TPM_DB_INVALID_IPV6_FLOW_ID;
}

int32_t tpm_db_set_ipv6_l4_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t parse_bm, uint32_t protocol, tpm_l4_ports_key_t *l4_key)
{
	uint32_t l4_key_id;
	tpm_db_ipv6_l4_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[0];

	if (flow_id >= TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid != TPM_FALSE)
		return TPM_DB_ERR_REC_STATUS_ERR;

	l4_key_id = tpm_db_find_matched_ipv6_l4_key(protocol, l4_key);

	if (l4_key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
		l4_key_id = tpm_db_set_ipv6_l4_key(protocol, l4_key);
		if (l4_key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
			return TPM_DB_ERR_DB_TBL_FULL;
		}
	}

	tpm_db_increase_ipv6_l4_key_user_num(l4_key_id);

	memset(&flow_array[flow_id], 0, sizeof(tpm_db_ipv6_l4_flow_t));

	flow_array[flow_id].valid = TPM_TRUE;
	flow_array[flow_id].parse_bm = parse_bm;
	flow_array[flow_id].l4_key = l4_key_id;

	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv6_l4_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t *parse_bm, uint32_t *protocol, tpm_l4_ports_key_t *l4_key)
{
	int32_t ret;
	tpm_db_ipv6_l4_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[0];
	uint32_t _protocol;
	tpm_l4_ports_key_t _l4_key;

	if (flow_id >= TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;

	memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
	ret = tpm_db_get_ipv6_l4_key(flow_array[flow_id].l4_key, &_protocol, &_l4_key);

	if (ret != TPM_DB_OK)
		return TPM_DB_ERR_REC_STATUS_ERR;

	*parse_bm = flow_array[flow_id].parse_bm;
	*protocol = _protocol;
	memcpy(l4_key, &_l4_key, sizeof(tpm_l4_ports_key_t));

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv6_l4_subflow(uint32_t src_dir, uint32_t flow_id)
{
	uint32_t user_num;
	tpm_db_ipv6_l4_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[0];

	if (flow_id >= TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;


	user_num = tpm_db_get_ipv6_l4_subflow_user_num(src_dir, flow_id);
	if (user_num == 0) {
		tpm_db_decrease_ipv6_l4_key_user_num(flow_array[flow_id].l4_key);
		tpm_db_del_ipv6_l4_key(flow_array[flow_id].l4_key);
		memset(&flow_array[flow_id], 0, sizeof(tpm_db_ipv6_l4_flow_t));
	}

	return TPM_DB_OK;
}

uint32_t tpm_db_find_matched_ipv6_gen_subflow(uint32_t src_dir, uint32_t parse_bm, uint32_t protocol,
											tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key)
{
	uint32_t i, l4_key_id, gen_key_id;
	tpm_db_ipv6_gen_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_SUBFLOW_NUM; i++) {
		if (flow_array[i].valid == TPM_FALSE)
			continue;

		if (parse_bm == flow_array[i].parse_bm) {
			if (flow_array[i].l4_key != TPM_DB_INVALID_IPV6_KEY_ID) {
				l4_key_id = tpm_db_find_matched_ipv6_l4_key(protocol, l4_key);
				if (l4_key_id == TPM_DB_INVALID_IPV6_KEY_ID)
					continue;
				if (l4_key_id != flow_array[i].l4_key)
					continue;
			}

			if (flow_array[i].gen_key != TPM_DB_INVALID_IPV6_KEY_ID) {
				gen_key_id = tpm_db_find_matched_ipv6_gen_key(gen_key);
				if (gen_key_id == TPM_DB_INVALID_IPV6_KEY_ID)
					continue;
				if (gen_key_id != flow_array[i].gen_key)
					continue;
			}

			return i;
		}
	}

	return TPM_DB_INVALID_IPV6_FLOW_ID;
}

uint32_t tpm_db_get_ipv6_gen_subflow_user_num(uint32_t src_dir, uint32_t flow_id)
{
	return tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[flow_id].user_num;
}

void tpm_db_increase_ipv6_gen_subflow_user_num(uint32_t src_dir, uint32_t flow_id)
{
	tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[flow_id].user_num++;
}

void tpm_db_decrease_ipv6_gen_subflow_user_num(uint32_t src_dir, uint32_t flow_id)
{
	if (tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[flow_id].user_num > 0)
		tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[flow_id].user_num--;
}

uint32_t tpm_db_get_free_ipv6_gen_subflow(uint32_t src_dir)
{
	uint32_t i;
	tpm_db_ipv6_gen_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_SUBFLOW_NUM; i++) {
		if (flow_array[i].valid == TPM_FALSE)
			return i;
	}

	return TPM_DB_INVALID_IPV6_FLOW_ID;
}

int32_t tpm_db_set_ipv6_gen_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t parse_bm, uint32_t protocol,
									tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key)
{
	uint32_t l4_parse_bm, gen_parse_bm, l4_flow_id, l4_key_id, gen_key_id;
	tpm_db_ipv6_gen_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[0];

	l4_key_id = gen_key_id = TPM_DB_INVALID_IPV6_KEY_ID;
	l4_flow_id = TPM_DB_INVALID_IPV6_FLOW_ID;

	if (flow_id >= TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid != TPM_FALSE)
		return TPM_DB_ERR_REC_STATUS_ERR;

	l4_parse_bm = parse_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;

	if (l4_parse_bm != 0) {
		l4_flow_id = tpm_db_find_matched_ipv6_l4_subflow(src_dir, l4_parse_bm, protocol, l4_key);

		if (l4_flow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
			return TPM_DB_ERR_DB_INCONSISTENCY;

		l4_key_id = tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[l4_flow_id].l4_key;
	}

	gen_parse_bm = parse_bm & ~TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;

	if (gen_parse_bm != 0) {
		gen_key_id = tpm_db_find_matched_ipv6_gen_key(gen_key);

		if (gen_key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
			gen_key_id = tpm_db_set_ipv6_gen_key(gen_key);
			if (gen_key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
				return TPM_DB_ERR_DB_TBL_FULL;
			}
		}
	}

	if (l4_flow_id != TPM_DB_INVALID_IPV6_FLOW_ID)
		tpm_db_increase_ipv6_l4_subflow_user_num(src_dir, l4_flow_id);

	if (gen_key_id != TPM_DB_INVALID_IPV6_KEY_ID)
		tpm_db_increase_ipv6_gen_key_user_num(gen_key_id);

	memset(&flow_array[flow_id], 0, sizeof(tpm_db_ipv6_gen_flow_t));

	flow_array[flow_id].valid = TPM_TRUE;
	flow_array[flow_id].parse_bm = parse_bm;
	flow_array[flow_id].l4_key = l4_key_id;
	flow_array[flow_id].gen_key = gen_key_id;

	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv6_gen_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t *parse_bm, uint32_t *protocol,
				    tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key)
{
	int32_t ret;
	tpm_db_ipv6_gen_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[0];
	uint32_t _protocol = 0;
	tpm_l4_ports_key_t _l4_key;
	tpm_ipv6_gen_acl_key_t _gen_key;

	if (flow_id >= TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;

	if (flow_array[flow_id].l4_key != TPM_DB_INVALID_IPV6_KEY_ID) {
		memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
		ret = tpm_db_get_ipv6_l4_key(flow_array[flow_id].l4_key, &_protocol, &_l4_key);

		if (ret != TPM_DB_OK)
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	if (flow_array[flow_id].gen_key != TPM_DB_INVALID_IPV6_KEY_ID) {
		memset(&_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
		ret = tpm_db_get_ipv6_gen_key(flow_array[flow_id].gen_key, &_gen_key);

		if (ret != TPM_DB_OK)
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	*parse_bm = flow_array[flow_id].parse_bm;
	*protocol = _protocol;
	memcpy(l4_key, &_l4_key, sizeof(tpm_l4_ports_key_t));
	memcpy(gen_key, &_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv6_gen_subflow(uint32_t src_dir, uint32_t flow_id)
{
	int32_t ret;
	uint32_t user_num, l4_flow_id, l4_parse_bm, protocol;
	tpm_db_ipv6_gen_flow_t *flow_array = &tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[0];
	tpm_l4_ports_key_t l4_key;

	if (flow_id >= TPM_DB_IPV6_MAX_SUBFLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_STATUS_ERR;

	user_num = tpm_db_get_ipv6_gen_subflow_user_num(src_dir, flow_id);
	if (user_num == 0) {
		if (flow_array[flow_id].gen_key != TPM_DB_INVALID_IPV6_KEY_ID)
			tpm_db_decrease_ipv6_gen_key_user_num(flow_array[flow_id].gen_key);

		tpm_db_del_ipv6_gen_key(flow_array[flow_id].gen_key);

		if (flow_array[flow_id].l4_key != TPM_DB_INVALID_IPV6_KEY_ID) {
			memset(&l4_key, 0, sizeof(tpm_l4_ports_key_t));
			ret = tpm_db_get_ipv6_l4_key(flow_array[flow_id].l4_key, &protocol, &l4_key);

			if (ret != TPM_DB_OK)
				return TPM_DB_ERR_DB_INCONSISTENCY;

			l4_parse_bm = flow_array[flow_id].parse_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
			l4_flow_id = tpm_db_find_matched_ipv6_l4_subflow(src_dir, l4_parse_bm, protocol, &l4_key);

			if (l4_flow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
				return TPM_DB_ERR_DB_INCONSISTENCY;

			tpm_db_decrease_ipv6_l4_subflow_user_num(src_dir, l4_flow_id);
		}

		memset(&flow_array[flow_id], 0, sizeof(tpm_db_ipv6_gen_flow_t));
	}
	return TPM_DB_OK;
}

uint32_t tpm_db_find_matched_ipv6_5t_flow(uint32_t src_dir, uint32_t parse_bm, uint32_t is_pppoe, uint32_t protocol,
							tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key, tpm_ipv6_addr_key_t *dip_key)
{
	uint32_t i, key_id;
	tpm_db_ipv6_5t_flow_shadow_t *flow_array = &tpm_db.ipv6_5t_flow_shadow[0];

	for (i = 0; i < TPM_DB_IPV6_MAX_5T_FLOW_NUM; i++) {
		if (flow_array[i].valid == TPM_FALSE)
			continue;

		if (parse_bm == flow_array[i].parse_bm &&
			src_dir == flow_array[i].src_dir &&
			is_pppoe == flow_array[i].is_pppoe) {
			if (flow_array[i].l4_key != TPM_DB_INVALID_IPV6_KEY_ID) {
				key_id = tpm_db_find_matched_ipv6_l4_key(protocol, l4_key);
				if (key_id == TPM_DB_INVALID_IPV6_KEY_ID)
					continue;
				if (key_id != flow_array[i].l4_key)
					continue;
			}

			if (flow_array[i].gen_key != TPM_DB_INVALID_IPV6_KEY_ID) {
				key_id = tpm_db_find_matched_ipv6_gen_key(gen_key);
				if (key_id == TPM_DB_INVALID_IPV6_KEY_ID)
					continue;
				if (key_id != flow_array[i].gen_key)
					continue;
			}

			if (flow_array[i].dip_key != TPM_DB_INVALID_IPV6_KEY_ID) {
				key_id = tpm_db_find_matched_ipv6_dip_key(dip_key);
				if (key_id == TPM_DB_INVALID_IPV6_KEY_ID)
					continue;
				if (key_id != flow_array[i].dip_key)
					continue;
			}

			return i;
		}
	}

	return TPM_DB_INVALID_IPV6_FLOW_ID;
}

int32_t tpm_db_set_ipv6_5t_flow(uint32_t src_dir, uint32_t parse_bm,  uint32_t is_pppoe, uint32_t protocol,
							tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key, tpm_ipv6_addr_key_t *dip_key)
{
	uint32_t i, _parse_bm, subflow_id, l4_key_id, gen_key_id, dip_key_id, flow_id = TPM_DB_INVALID_IPV6_FLOW_ID;
	tpm_db_ipv6_5t_flow_shadow_t *flow_array = &tpm_db.ipv6_5t_flow_shadow[0];

	l4_key_id = gen_key_id = dip_key_id = TPM_DB_INVALID_IPV6_KEY_ID;

	for (i = 0; i < TPM_DB_IPV6_MAX_5T_FLOW_NUM; i++) {
		if (flow_array[i].valid == TPM_FALSE) {
			flow_id = i;
			break;
		}
	}

	if (flow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
		return TPM_DB_ERR_DB_TBL_FULL;

	_parse_bm = parse_bm & ~TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
	if (_parse_bm != 0) {
		dip_key_id = tpm_db_find_matched_ipv6_dip_key(dip_key);
		if (dip_key_id == TPM_DB_INVALID_IPV6_KEY_ID) {
			dip_key_id = tpm_db_set_ipv6_dip_key(dip_key);
			if (dip_key_id == TPM_DB_INVALID_IPV6_KEY_ID)
				return TPM_DB_ERR_DB_INCONSISTENCY;
		}
	}

	_parse_bm = parse_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
	if (_parse_bm != 0) {
		subflow_id = tpm_db_find_matched_ipv6_gen_subflow(src_dir, _parse_bm, protocol, l4_key, gen_key);
		if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
			tpm_db_increase_ipv6_gen_subflow_user_num(src_dir, subflow_id);
			gen_key_id = tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[subflow_id].gen_key;
			l4_key_id = tpm_db.ipv6_subflow_shadow[src_dir].gen_flow[subflow_id].l4_key;
		} else {
			_parse_bm = parse_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
			if (_parse_bm != 0) {
				subflow_id = tpm_db_find_matched_ipv6_l4_subflow(src_dir, _parse_bm, protocol, l4_key);
				if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
					tpm_db_increase_ipv6_l4_subflow_user_num(src_dir, subflow_id);
					l4_key_id = tpm_db.ipv6_subflow_shadow[src_dir].l4_flow[subflow_id].l4_key;
				}
			}
		}
	}

	if (dip_key_id != TPM_DB_INVALID_IPV6_KEY_ID)
		tpm_db_increase_ipv6_dip_key_user_num(dip_key_id);

	memset(&flow_array[flow_id], 0, sizeof(tpm_db_ipv6_5t_flow_shadow_t));

	flow_array[flow_id].valid = TPM_TRUE;
	flow_array[flow_id].src_dir = src_dir;
	flow_array[flow_id].parse_bm = parse_bm;
	flow_array[flow_id].is_pppoe = is_pppoe;
	flow_array[flow_id].gen_key = gen_key_id;
	flow_array[flow_id].dip_key = dip_key_id;
	flow_array[flow_id].l4_key = l4_key_id;

	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv6_5t_flow(uint32_t flow_id, uint32_t *src_dir, uint32_t *parse_bm, uint32_t *is_pppoe, uint32_t *protocol,
							tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key, tpm_ipv6_addr_key_t *dip_key)
{
	int32_t ret;
	tpm_db_ipv6_5t_flow_shadow_t *flow_array = &tpm_db.ipv6_5t_flow_shadow[0];
	tpm_l4_ports_key_t _l4_key;
	tpm_ipv6_gen_acl_key_t _gen_key;
	tpm_ipv6_addr_key_t _dip_key;
	uint32_t _protocol = 0;

	memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
	memset(&_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
	memset(&_dip_key, 0, sizeof(tpm_ipv6_addr_key_t));

	if (flow_id >= TPM_DB_IPV6_MAX_5T_FLOW_NUM)
		return TPM_DB_ERR_INV_INPUT;

	if (flow_array[flow_id].valid == TPM_FALSE)
		return TPM_DB_ERR_REC_INV;

	if (flow_array[flow_id].l4_key != TPM_DB_INVALID_IPV6_KEY_ID) {
		ret = tpm_db_get_ipv6_l4_key(flow_array[flow_id].l4_key, &_protocol, &_l4_key);
		if (ret != TPM_DB_OK)
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	if (flow_array[flow_id].gen_key != TPM_DB_INVALID_IPV6_KEY_ID) {
		ret = tpm_db_get_ipv6_gen_key(flow_array[flow_id].gen_key, &_gen_key);
		if (ret != TPM_DB_OK)
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	if (flow_array[flow_id].dip_key != TPM_DB_INVALID_IPV6_KEY_ID) {
		ret = tpm_db_get_ipv6_dip_key(flow_array[flow_id].dip_key, &_dip_key);
		if (ret != TPM_DB_OK)
			return TPM_DB_ERR_DB_INCONSISTENCY;
	}

	*src_dir = flow_array[flow_id].src_dir;
	*parse_bm = flow_array[flow_id].parse_bm;
	*is_pppoe = flow_array[flow_id].is_pppoe;
	*protocol = _protocol;
	memcpy(l4_key, &_l4_key, sizeof(tpm_l4_ports_key_t));
	memcpy(gen_key, &_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));
	memcpy(dip_key, &_dip_key, sizeof(tpm_ipv6_addr_key_t));

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv6_5t_flow(uint32_t flow_id)
{
	int32_t ret;
	tpm_db_ipv6_5t_flow_shadow_t *flow_array = &tpm_db.ipv6_5t_flow_shadow[0];
	tpm_l4_ports_key_t l4_key;
	tpm_ipv6_gen_acl_key_t gen_key;
	tpm_ipv6_addr_key_t dip_key;
	uint32_t src_dir, parse_bm, protocol, subflow_bm, subflow_id, is_pppoe;


	ret = tpm_db_get_ipv6_5t_flow(flow_id, &src_dir, &parse_bm, &is_pppoe, &protocol, &l4_key, &gen_key, &dip_key);

	if (ret != TPM_DB_OK)
		return TPM_DB_ERR_REC_INV;

	subflow_bm = parse_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
	if (subflow_bm != 0) {
		subflow_id = tpm_db_find_matched_ipv6_gen_subflow(src_dir, subflow_bm, protocol, &l4_key, &gen_key);
		if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID)
			tpm_db_decrease_ipv6_gen_subflow_user_num(src_dir, subflow_id);
		else {
			subflow_bm = parse_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
			if (subflow_bm != 0) {
				subflow_id = tpm_db_find_matched_ipv6_l4_subflow(src_dir, subflow_bm, protocol, &l4_key);
				if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID)
					tpm_db_decrease_ipv6_l4_subflow_user_num(src_dir, subflow_id);
			}
		}
	}

	if (flow_array[flow_id].dip_key != TPM_DB_INVALID_IPV6_KEY_ID) {
		tpm_db_decrease_ipv6_dip_key_user_num(flow_array[flow_id].dip_key);
		tpm_db_del_ipv6_dip_key(flow_array[flow_id].dip_key);
	}

	memset(&flow_array[flow_id], 0, sizeof(tpm_db_ipv6_5t_flow_shadow_t));

	return TPM_DB_OK;
}

int32_t tpm_db_init_ipv6_5t_flow_reset(void)
{
	memset(&tpm_db.ipv6_key_shadow, 0, sizeof(tpm_db_ipv6_key_shadow_t));
	memset(tpm_db.ipv6_subflow_shadow, 0, TPM_NUM_DIRECTIONS * sizeof(tpm_db_ipv6_subflow_shadow_t));
	memset(tpm_db.ipv6_5t_flow_shadow, 0, TPM_DB_IPV6_MAX_5T_FLOW_NUM * sizeof(tpm_db_ipv6_5t_flow_shadow_t));

	return TPM_DB_OK;
}
int32_t tpm_db_ctc_cm_rule_set(tpm_src_port_type_t src_port,
			       uint32_t precedence,
			       tpm_parse_fields_t l2_parse_rule_bm,
			       tpm_parse_fields_t ipv4_parse_rule_bm,
			       tpm_parse_fields_t ipv6_parse_rule_bm,
			       tpm_l2_acl_key_t *l2_key,
			       tpm_ipv4_acl_key_t *ipv4_key,
			       tpm_ipv6_acl_key_t *ipv6_key,
			       tpm_pkt_frwd_t *pkt_frwd,
			       tpm_pkt_action_t pkt_act,
			       uint32_t pbits,
			       uint32_t ipv4_sub_pattern_index,
			       uint32_t rule_index)
{
	tpm_db_ctc_cm_rule_entry_t *rule_entry;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if(precedence >= TPM_MAX_NUM_CTC_PRECEDENCE) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input precedence(%d)\n", precedence);
		return TPM_DB_ERR_INV_INPUT;
	}

	rule_entry = &tpm_db.ctc_cm_data.cm_rule[src_port][precedence];

	rule_entry->l2_parse_rule_bm   = l2_parse_rule_bm;
	rule_entry->ipv4_parse_rule_bm = ipv4_parse_rule_bm;
	rule_entry->ipv6_parse_rule_bm = ipv6_parse_rule_bm;
	memcpy(&rule_entry->l2_key, l2_key, sizeof(tpm_l2_acl_key_t));
	memcpy(&rule_entry->ipv4_key, ipv4_key, sizeof(tpm_ipv4_acl_key_t));
	memcpy(&rule_entry->ipv6_key, ipv6_key, sizeof(tpm_ipv6_acl_key_t));
	memcpy(&rule_entry->pkt_frwd, pkt_frwd, sizeof(tpm_pkt_frwd_t));
	rule_entry->pkt_act                = pkt_act;
	rule_entry->pbits                  = pbits;
	rule_entry->cm_main_rule_index     = rule_index;
	rule_entry->ipv4_sub_pattern_key   = ipv4_sub_pattern_index;
	rule_entry->valid                  = 1;

	if(l2_parse_rule_bm & ipv4_parse_rule_bm)
		rule_entry->rule_type = TPM_CTC_CM_COMBO;
	else if(l2_parse_rule_bm)
		rule_entry->rule_type = TPM_CTC_CM_L2;
	else if(ipv4_parse_rule_bm)
		rule_entry->rule_type = TPM_CTC_CM_IPv4;

	return TPM_DB_OK;
}

int32_t tpm_db_ctc_cm_rule_delete(tpm_src_port_type_t src_port, uint32_t precedence)
{
	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if(precedence >= TPM_MAX_NUM_CTC_PRECEDENCE) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input precedence(%d)\n", precedence);
		return TPM_DB_ERR_INV_INPUT;
	}

	memset(&tpm_db.ctc_cm_data.cm_rule[src_port][precedence], 0, sizeof(tpm_db_ctc_cm_rule_entry_t));
	return TPM_DB_OK;
}

int32_t tpm_db_ctc_cm_rule_get(tpm_src_port_type_t src_port, uint32_t precedence, tpm_db_ctc_cm_rule_entry_t *cm_rule)
{
	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if(precedence >= TPM_MAX_NUM_CTC_PRECEDENCE) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input precedence(%d)\n", precedence);
		return TPM_DB_ERR_INV_INPUT;
	}

	memcpy(cm_rule, &tpm_db.ctc_cm_data.cm_rule[src_port][precedence], sizeof(tpm_db_ctc_cm_rule_entry_t));
	return TPM_DB_OK;
}

int32_t tpm_db_ctc_cm_get_num_rules(tpm_src_port_type_t src_port)
{
	int32_t loop;
	int32_t num_rules = 0;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	for(loop = 0; loop < TPM_MAX_NUM_CTC_PRECEDENCE; loop++)
		if(tpm_db.ctc_cm_data.cm_rule[src_port][loop].valid)
			num_rules++;

	return num_rules;
}

int32_t tpm_db_ctc_cm_get_ipv6_num_rules(void)
{
	int32_t loop;
	int32_t num_rules = 0;
	tpm_src_port_type_t src_port;

	for (src_port = TPM_SRC_PORT_UNI_0; src_port < TPM_MAX_NUM_UNI_PORTS; src_port++)
		for(loop = 0; loop < TPM_MAX_NUM_CTC_PRECEDENCE; loop++)
			if(    (tpm_db.ctc_cm_data.cm_rule[src_port][loop].valid)
			    && (tpm_db.ctc_cm_data.cm_rule[src_port][loop].ipv6_parse_rule_bm))
				num_rules++;

	return num_rules;
}

int32_t tpm_db_ctc_cm_db_reset(void)
{
	memset(&tpm_db.ctc_cm_data.cm_rule, 0, sizeof(tpm_db.ctc_cm_data.cm_rule));
	memset(&tpm_db.ctc_cm_data.ipv4_pre_table, 0, sizeof(tpm_db_cnm_ipv4_pre_table_t));
	memset(&tpm_db.ctc_cm_data.ipv4_pre_filter, 0, TPM_MAX_NUM_UNI_PORTS * sizeof(tpm_db_cnm_ipv4_pre_filter_t));
	tpm_db.ctc_cm_data.ipv6_parse_win = tpm_db.ctc_cm_data.ipv6_parse_win_original;
	return TPM_DB_OK;
}

int32_t tpm_db_ipv6_mc_sip_db_reset(void)
{
	memset(&tpm_db.ipv6_mc_sip, 0, sizeof(tpm_db.ipv6_mc_sip));
	/* reserve the first IPv6 MC SIP entry for not matching any SIP */
	tpm_db.ipv6_mc_sip[0].valid = 1;
	return TPM_DB_OK;
}

int32_t tpm_db_ipv6_mc_sip_index_get(uint8_t  *src_ip)
{
	uint32_t i;
	int32_t ret;
	tpm_db_pnc_range_t range_data;

	/*check param*/
	if (src_ip == NULL)
		return 0;

	ret = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_SIP, &range_data);
	if(ret != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_DB_MOD, "TPM_PNC_IPV6_MC_SIP is not created!\n");
		return 0;
	}

	for (i = 1; i < range_data.pnc_range_conf.range_size; i++) {
		if (tpm_db.ipv6_mc_sip[i].valid == 0)
			continue;

		if (memcmp(tpm_db.ipv6_mc_sip[i].src_ip, src_ip, 16)== 0)
			return i;
	}

	/* does not find this src_ip */
	return 0;
}

int32_t tpm_db_ipv6_mc_sip_add(uint8_t  *src_ip)
{
	uint32_t i;
	int32_t ret;
	tpm_db_pnc_range_t range_data;

	/*check param*/
	if (src_ip == NULL)
		return 0;

	ret = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_SIP, &range_data);
	if(ret != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_DB_MOD, "TPM_PNC_IPV6_MC_SIP is not created!\n");
		return 0;
	}

	for (i = 1; i < range_data.pnc_range_conf.range_size; i++) {
		if (tpm_db.ipv6_mc_sip[i].valid == 1)
			continue;

		memcpy(tpm_db.ipv6_mc_sip[i].src_ip, src_ip, 16);
		tpm_db.ipv6_mc_sip[i].valid = 1;
		tpm_db.ipv6_mc_sip[i].reference_num = 1;
		return i;
	}

	/* do not have slot for this src_ip */
	return 0;
}
int32_t tpm_db_ipv6_mc_sip_ref_num_inc(uint8_t  src_ip_index)
{
	/*check param*/
	if (src_ip_index >= TPM_MC_IPv6_SIP_NUM_MAX)
		return (TPM_DB_ERR_INV_INPUT);

	if (tpm_db.ipv6_mc_sip[src_ip_index].valid == 0)
		return (TPM_DB_ERR_INV_INPUT);

	tpm_db.ipv6_mc_sip[src_ip_index].reference_num++;

	return TPM_DB_OK;
}
int32_t tpm_db_ipv6_mc_sip_ref_num_dec(uint8_t  src_ip_index, uint8_t  *new_ref_num)
{
	/*check param*/
	if (src_ip_index >= TPM_MC_IPv6_SIP_NUM_MAX)
		return (TPM_DB_ERR_INV_INPUT);

	if (tpm_db.ipv6_mc_sip[src_ip_index].valid == 0)
		return (TPM_DB_ERR_INV_INPUT);

	if (tpm_db.ipv6_mc_sip[src_ip_index].reference_num == 0)
		return (TPM_DB_ERR_INV_INPUT);

	tpm_db.ipv6_mc_sip[src_ip_index].reference_num--;

	*new_ref_num = tpm_db.ipv6_mc_sip[src_ip_index].reference_num;
	/* if new ref num is 0, remove this SIP in DB */
	if (tpm_db.ipv6_mc_sip[src_ip_index].reference_num == 0) {
		memset(&tpm_db.ipv6_mc_sip[src_ip_index], 0, sizeof(tpm_db.ipv6_mc_sip[src_ip_index]));
	}

	return TPM_DB_OK;
}

int32_t tpm_db_ipv6_mc_sip_free_slot_num_get(void)
{
	uint32_t i;
	int32_t ret;
	tpm_db_pnc_range_t range_data;
	uint32_t free_slot_num = 0;

	ret = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_SIP, &range_data);
	if(ret != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_DB_MOD, "TPM_PNC_IPV6_MC_SIP is not created!\n");
		return 0;
	}

	for (i = 1; i < range_data.pnc_range_conf.range_size; i++) {
		if (tpm_db.ipv6_mc_sip[i].valid == 1)
			continue;
		free_slot_num++;
	}

	return free_slot_num;
}

int32_t tpm_db_ctc_cm_enable_set(tpm_db_ctc_cm_enable_t ctc_cm_enable)
{
	tpm_db.ctc_cm_data.enable = ctc_cm_enable;
	return(TPM_DB_OK);
}

int32_t tpm_db_ctc_cm_enable_get(tpm_db_ctc_cm_enable_t *ctc_cm_enable)
{
	*ctc_cm_enable = tpm_db.ctc_cm_data.enable;
	return(TPM_DB_OK);
}
int32_t tpm_db_ctc_cm_ipv6_parse_win_set(tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win)
{
	tpm_db.ctc_cm_data.ipv6_parse_win = ctc_cm_ipv6_parse_win;
	return(TPM_DB_OK);
}
int32_t tpm_db_ctc_cm_ipv6_parse_win_orig_set(tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win)
{
	tpm_db.ctc_cm_data.ipv6_parse_win_original = ctc_cm_ipv6_parse_win;
	return(TPM_DB_OK);
}

int32_t tpm_db_ctc_cm_ipv6_parse_win_get(tpm_db_ctc_cm_ipv6_parse_win_t *ctc_cm_ipv6_parse_win)
{
	*ctc_cm_ipv6_parse_win = tpm_db.ctc_cm_data.ipv6_parse_win;
	return(TPM_DB_OK);
}
int32_t tpm_db_ctc_cm_ipv6_ety_rule_num_set(uint32_t ipv6_ety_rule_num)
{
	tpm_db.ctc_cm_data.ipv6_ety_rule_num = ipv6_ety_rule_num;
	return(TPM_DB_OK);
}

int32_t tpm_db_ctc_cm_ipv6_ety_rule_num_get(uint32_t *ipv6_ety_rule_num)
{
	*ipv6_ety_rule_num = tpm_db.ctc_cm_data.ipv6_ety_rule_num;
	return(TPM_DB_OK);
}

int32_t tpm_db_add_ipv4_pre_rule(tpm_src_port_type_t src_port, uint32_t key_pattern, uint32_t *rule_num)
{
	uint32_t rule_index, i, j;
	uint32_t *rule_table = &tpm_db.ctc_cm_data.ipv4_pre_table.rule_index[0];

	if (rule_num == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	rule_index = TPM_DB_CNM_GEN_IPV4_PRE_RULE_INDEX(src_port, key_pattern);
	if (rule_index == TPM_DB_CNM_INVALID_IPV4_PRE_RULE_INDEX)
		return TPM_DB_ERR_INV_INPUT;

	TPM_OS_DEBUG(TPM_DB_MOD, "add rule_index(0x%x)\n", rule_index);

	for (i = 0; i < tpm_db.ctc_cm_data.ipv4_pre_table.num_rules; i++) {
		if (rule_index == rule_table[i]) {
			TPM_OS_ERROR(TPM_DB_MOD, " ipv4_pre_rule already exists!\n");
			return TPM_DB_ERR_DB_INCONSISTENCY;
		} else if (rule_index > rule_table[i]) {
			break;
		}
	}

	for (j = tpm_db.ctc_cm_data.ipv4_pre_table.num_rules; j > i; j--)
		rule_table[j] = rule_table[j - 1];

	rule_table[j] = rule_index;
	tpm_db.ctc_cm_data.ipv4_pre_table.num_rules++;

	*rule_num = i;

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv4_pre_rule(tpm_src_port_type_t src_port, uint32_t key_pattern, uint32_t *rule_num)
{
	uint32_t rule_index, i, j;
	uint32_t *rule_table = &tpm_db.ctc_cm_data.ipv4_pre_table.rule_index[0];

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	rule_index = TPM_DB_CNM_GEN_IPV4_PRE_RULE_INDEX(src_port, key_pattern);
	if (rule_index == TPM_DB_CNM_INVALID_IPV4_PRE_RULE_INDEX)
		return TPM_DB_ERR_REC_EXIST;

	TPM_OS_DEBUG(TPM_DB_MOD, "delete rule_index(0x%x)\n", rule_index);

	for (i = 0; i < tpm_db.ctc_cm_data.ipv4_pre_table.num_rules; i++) {
		if (rule_index == rule_table[i])
			break;
	}

	if (i == tpm_db.ctc_cm_data.ipv4_pre_table.num_rules) {
		TPM_OS_ERROR(TPM_DB_MOD, " ipv4_pre_rule does not exist!\n");
		return TPM_DB_ERR_REC_NOT_EXIST;
	}

	for (j = i; j < tpm_db.ctc_cm_data.ipv4_pre_table.num_rules; j++)
		rule_table[j] = rule_table[j + 1];

	rule_table[j] = TPM_DB_CNM_INVALID_IPV4_PRE_RULE_INDEX;
	tpm_db.ctc_cm_data.ipv4_pre_table.num_rules--;

	*rule_num = i;

	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv4_pre_rule_num(uint32_t *num_rules)
{
	*num_rules = tpm_db.ctc_cm_data.ipv4_pre_table.num_rules;
	return TPM_DB_OK;
}

int32_t tpm_db_alloc_ipv4_pre_filter_key(tpm_src_port_type_t src_port, uint32_t *key_idx)
{
	uint32_t i;

	/*check param*/
	if (key_idx == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].num_keys >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_DB_TBL_FULL;

	for (i = 0; i < TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM; i++) {
		if (tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[i].valid == TPM_FALSE)
			break;
	}

	if (i >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_DB_INCONSISTENCY;

	*key_idx = i;

	return TPM_DB_OK;
}

int32_t tpm_db_set_ipv4_pre_filter_key(tpm_src_port_type_t src_port,
				       uint32_t key_idx,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_ipv4_acl_key_t *ipv4_key)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = NULL;

	/*check param*/
	if (ipv4_key == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (key_idx >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_REC_INV;

	key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[key_idx];

	if (key->valid != TPM_FALSE)
		return TPM_DB_ERR_REC_EXIST;

	key->valid = TPM_TRUE;
	key->num_users = 0;
	key->parse_rule_bm = parse_rule_bm;
	memcpy(&key->ipv4_key, ipv4_key, sizeof(tpm_ipv4_acl_key_t));

	tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].num_keys++;

	return TPM_DB_OK;
}

int32_t tpm_db_del_ipv4_pre_filter_key(tpm_src_port_type_t src_port, uint32_t key_idx)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = NULL;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (key_idx >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_REC_INV;

	key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[key_idx];

	if (key->valid == TPM_FALSE)
		return TPM_DB_ERR_REC_NOT_EXIST;

	memset(key, 0, sizeof(tpm_db_cnm_ipv4_pre_filter_key_t));

	tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].num_keys--;

	return TPM_DB_OK;
}

int32_t tpm_db_set_gmac_rate_limit(tpm_db_tx_mod_t gmac_i,
				   uint32_t bucket_size,
				   uint32_t rate_limit,
				   uint32_t prio)
{
	/*check param*/
	if (gmac_i >= TPM_MAX_NUM_TX_PORTS)
		return (TPM_DB_ERR_INV_INPUT);

	tpm_db.gmac_tx[gmac_i].bucket_size = bucket_size;
	tpm_db.gmac_tx[gmac_i].rate_limit = rate_limit;
	tpm_db.gmac_tx[gmac_i].prio = prio;

	return TPM_DB_OK;
}

int32_t tpm_db_get_gmac_rate_limit(tpm_gmacs_enum_t gmac_i,
				   uint32_t *bucket_size,
				   uint32_t *rate_limit,
				   uint32_t *prio)
{
	/*check param*/
	if (gmac_i >= TPM_MAX_NUM_TX_PORTS)
		return (TPM_DB_ERR_INV_INPUT);

	*bucket_size = tpm_db.gmac_tx[gmac_i].bucket_size;
	*rate_limit = tpm_db.gmac_tx[gmac_i].rate_limit;
	*prio = tpm_db.gmac_tx[gmac_i].prio;

	return TPM_DB_OK;
}
int32_t tpm_db_set_gmac_q_rate_limit(tpm_db_tx_mod_t gmac_i,
				   uint32_t queue,
				   uint32_t bucket_size,
				   uint32_t rate_limit,
				   uint32_t wrr)
{
	/*check param*/
	if (gmac_i >= TPM_MAX_NUM_TX_PORTS)
		return (TPM_DB_ERR_INV_INPUT);

	if (queue >= TPM_MAX_NUM_TX_QUEUE)
		return TPM_DB_ERR_INV_INPUT;

	tpm_db.gmac_tx[gmac_i].tx_queue[queue].bucket_size = bucket_size;
	tpm_db.gmac_tx[gmac_i].tx_queue[queue].rate_limit = rate_limit;
	tpm_db.gmac_tx[gmac_i].tx_queue[queue].wrr = wrr;

	return TPM_DB_OK;
}

int32_t tpm_db_get_gmac_q_rate_limit(tpm_gmacs_enum_t gmac_i,
				   uint32_t queue,
				   uint32_t *bucket_size,
				   uint32_t *rate_limit,
				   uint32_t *wrr)
{
	/*check param*/
	if (gmac_i >= TPM_MAX_NUM_TX_PORTS)
		return (TPM_DB_ERR_INV_INPUT);

	if (queue >= TPM_MAX_NUM_TX_QUEUE)
		return TPM_DB_ERR_INV_INPUT;

	*bucket_size = tpm_db.gmac_tx[gmac_i].tx_queue[queue].bucket_size;
	*rate_limit = tpm_db.gmac_tx[gmac_i].tx_queue[queue].rate_limit;
	*wrr = tpm_db.gmac_tx[gmac_i].tx_queue[queue].wrr;

	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv4_pre_filter_key_num(tpm_src_port_type_t src_port, uint32_t *num_keys)
{
	/*check param*/
	if (num_keys == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	*num_keys = tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].num_keys;
	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv4_pre_filter_key(tpm_src_port_type_t src_port,
				       uint32_t key_idx,
				       tpm_parse_fields_t *parse_rule_bm,
				       tpm_ipv4_acl_key_t *ipv4_key)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = NULL;

	/*check param*/
	if (parse_rule_bm == NULL || ipv4_key == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if (key_idx >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_REC_INV;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[key_idx];

	if (key->valid == TPM_FALSE)
		return TPM_DB_ERR_REC_NOT_EXIST;

	*parse_rule_bm = key->parse_rule_bm;
	memcpy(ipv4_key, &key->ipv4_key, sizeof(tpm_ipv4_acl_key_t));

	return TPM_DB_OK;
}

int32_t tpm_db_find_ipv4_pre_filter_key(tpm_src_port_type_t src_port,
					tpm_parse_fields_t parse_rule_bm,
					tpm_ipv4_acl_key_t *ipv4_key,
					uint32_t *key_idx)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[0];
	uint32_t i;

	/*check param*/
	if (key_idx == NULL || ipv4_key == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	for (i = 0; i < TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM; i++) {
		if (key[i].valid == TPM_FALSE)
			continue;
		if (key[i].parse_rule_bm != parse_rule_bm)
			continue;
		if (parse_rule_bm & TPM_IPv4_PARSE_SIP) {
			if (memcmp(ipv4_key->ipv4_src_ip_add, key[i].ipv4_key.ipv4_src_ip_add, 4 * sizeof(uint8_t)) ||
				memcmp(ipv4_key->ipv4_src_ip_add_mask, key[i].ipv4_key.ipv4_src_ip_add_mask, 4 * sizeof(uint8_t)))
				continue;
		}
		if (parse_rule_bm & TPM_IPv4_PARSE_DIP) {
			if (memcmp(ipv4_key->ipv4_dst_ip_add, key[i].ipv4_key.ipv4_dst_ip_add, 4 * sizeof(uint8_t)) ||
				memcmp(ipv4_key->ipv4_dst_ip_add_mask, key[i].ipv4_key.ipv4_dst_ip_add_mask, 4 * sizeof(uint8_t)))
				continue;
		}
		if (parse_rule_bm & TPM_IPv4_PARSE_DSCP) {
			if ((ipv4_key->ipv4_dscp & ipv4_key->ipv4_dscp_mask) != (key[i].ipv4_key.ipv4_dscp & key[i].ipv4_key.ipv4_dscp_mask))
				continue;
		}
		if (parse_rule_bm & TPM_IPv4_PARSE_PROTO) {
			if (ipv4_key->ipv4_proto != key[i].ipv4_key.ipv4_proto)
				continue;
		}
		if (parse_rule_bm & TPM_PARSE_L4_SRC) {
			if (ipv4_key->l4_src_port != key[i].ipv4_key.l4_src_port)
				continue;
		}
		if (parse_rule_bm & TPM_PARSE_L4_DST) {
			if (ipv4_key->l4_dst_port != key[i].ipv4_key.l4_dst_port)
				continue;
		}
		*key_idx = i;
		return TPM_DB_OK;
	}

	*key_idx = TPM_CNM_INVALID_IPV4_PRE_FILTER_KEY_ID;
	return TPM_DB_ERR_REC_NOT_EXIST;
}

int32_t tpm_db_inc_ipv4_pre_filter_key_user_num(tpm_src_port_type_t src_port, uint32_t key_idx)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = NULL;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (key_idx >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_REC_INV;

	key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[key_idx];

	if (key->valid == TPM_FALSE)
		return TPM_DB_ERR_REC_NOT_EXIST;

	key->num_users++;
	return TPM_DB_OK;
}

int32_t tpm_db_dec_ipv4_pre_filter_key_user_num(tpm_src_port_type_t src_port, uint32_t key_idx)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = NULL;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	if (key_idx >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_REC_INV;

	key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[key_idx];

	if (key->valid == TPM_FALSE)
		return TPM_DB_ERR_REC_NOT_EXIST;

	if (key->num_users > 0)
		key->num_users--;
	return TPM_DB_OK;
}

int32_t tpm_db_get_ipv4_pre_filter_key_user_num(tpm_src_port_type_t src_port, uint32_t key_idx, uint32_t *num_users)
{
	tpm_db_cnm_ipv4_pre_filter_key_t *key = NULL;

	/*check param*/
	if (num_users == NULL)
		return (TPM_DB_ERR_INV_INPUT);

	if (key_idx >= TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM)
		return TPM_DB_ERR_REC_INV;

	if(src_port >= TPM_MAX_NUM_UNI_PORTS) {
		TPM_OS_ERROR(TPM_DB_MOD, "invalid input Src Port(%d)\n", src_port);
		return TPM_DB_ERR_INV_INPUT;
	}

	key = &tpm_db.ctc_cm_data.ipv4_pre_filter[src_port].key[key_idx];

	if (key->valid == TPM_FALSE)
		return TPM_DB_ERR_REC_NOT_EXIST;

	*num_users = key->num_users;
	return TPM_DB_OK;
}


/*******************************************************************************
* tpm_db_mac_learn_mod_idx_set()
*
* DESCRIPTION: Set mac leanring pmt mod index, just for mac learning
*
* INPUTS:
* mod_idx
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mac_learn_mod_idx_set(uint32_t mod_idx)
{
	tpm_db.mac_learn_mod_idx = mod_idx;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_mac_learn_mod_idx_get()
*
* DESCRIPTION: Get mac leanring pmt mod index, just for mac learning
*
* INPUTS:
* mod_idx
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_mac_learn_mod_idx_get(uint32_t *mod_idx)
{
	*mod_idx = tpm_db.mac_learn_mod_idx;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_ds_mac_based_trunk_enable_set()
*
* DESCRIPTION: Set ds_mac_based_trunk_enable
*
* INPUTS:
* enable       -   ds_mac_based_trunk_enable
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_ds_mac_based_trunk_enable_set(tpm_db_ds_mac_based_trunk_enable_t enable)
{
	tpm_db.ds_mac_based_trunk_enable = enable;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_ds_mac_based_trunk_enable_get()
*
* DESCRIPTION: Get ds_mac_based_trunk_enable
*
* INPUTS:
*
* OUTPUTS:
* enable       -   ds_mac_based_trunk_enable
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_ds_mac_based_trunk_enable_get(tpm_db_ds_mac_based_trunk_enable_t *enable)
{
	*enable = tpm_db.ds_mac_based_trunk_enable;

	return (TPM_DB_OK);
}
int32_t tpm_db_api_data_backup(void)
{
	memcpy(hot_swap_bak_db.api_ent_mem_area_bak, tpm_db.api_ent_mem_area, sizeof(tpm_db.api_ent_mem_area));
	memcpy(hot_swap_bak_db.api_section_bak, tpm_db.api_section, sizeof(tpm_db.api_section));
	memcpy(hot_swap_bak_db.mc_vid_port_cfg_bak, tpm_db.mc_vid_port_cfg, sizeof(tpm_db.mc_vid_port_cfg));
	memcpy(hot_swap_bak_db.gmac_tx_bak, tpm_db.gmac_tx, sizeof(tpm_db.gmac_tx));
	memcpy(hot_swap_bak_db.igmp_proxy_sa_mac, tpm_db_mc_igmp_proxy_sa_mac, 6);
	hot_swap_bak_db.igmp_proxy_sa_mac_valid = tpm_db_mc_igmp_proxy_sa_mac_valid;
	hot_swap_bak_db.switch_init = tpm_db.func_profile.switch_init;

	return (TPM_DB_OK);
}
int32_t tpm_db_api_data_rcvr(void)
{
	memcpy(tpm_db.mc_vid_port_cfg, hot_swap_bak_db.mc_vid_port_cfg_bak, sizeof(tpm_db.mc_vid_port_cfg));
	memcpy(tpm_db.gmac_tx, hot_swap_bak_db.gmac_tx_bak, sizeof(tpm_db.gmac_tx));
	memcpy(tpm_db_mc_igmp_proxy_sa_mac, hot_swap_bak_db.igmp_proxy_sa_mac, 6);
	tpm_db_mc_igmp_proxy_sa_mac_valid = hot_swap_bak_db.igmp_proxy_sa_mac_valid;
	tpm_db.func_profile.switch_init = hot_swap_bak_db.switch_init;

	return (TPM_DB_OK);
}
void tpm_db_exchange_value(uint32_t *v1, uint32_t *v2)
{
	uint32_t tmp;

	tmp = *v1;
	*v1 = *v2;
	*v2 = tmp;

	return;
}
int32_t tpm_db_wan_lan_rate_limit_exchange_db(tpm_db_gmac_tx_t *gmac_tx)
{
	int32_t queue;

	for (queue = 0; queue < TPM_MAX_NUM_TX_QUEUE; queue++) {
		tpm_db_exchange_value(&(gmac_tx[TPM_TX_MOD_GMAC0].tx_queue[queue].bucket_size),
					&(gmac_tx[TPM_TX_MOD_GMAC1].tx_queue[queue].bucket_size));
		tpm_db_exchange_value(&(gmac_tx[TPM_TX_MOD_GMAC0].tx_queue[queue].rate_limit),
					&(gmac_tx[TPM_TX_MOD_GMAC1].tx_queue[queue].rate_limit));
		tpm_db_exchange_value(&(gmac_tx[TPM_TX_MOD_GMAC0].tx_queue[queue].wrr),
					&(gmac_tx[TPM_TX_MOD_GMAC1].tx_queue[queue].wrr));
	}

	tpm_db_exchange_value(&(gmac_tx[TPM_TX_MOD_GMAC0].bucket_size),
				&(gmac_tx[TPM_TX_MOD_GMAC1].bucket_size));
	tpm_db_exchange_value(&(gmac_tx[TPM_TX_MOD_GMAC0].rate_limit),
				&(gmac_tx[TPM_TX_MOD_GMAC1].rate_limit));
	tpm_db_exchange_value(&(gmac_tx[TPM_TX_MOD_GMAC0].prio),
				&(gmac_tx[TPM_TX_MOD_GMAC1].prio));

	return (TPM_DB_OK);
}
int32_t tpm_db_wan_lan_rate_limit_exchange(void)
{
	tpm_db_wan_lan_rate_limit_exchange_db(tpm_db.gmac_tx);
	tpm_db_wan_lan_rate_limit_exchange_db(hot_swap_bak_db.gmac_tx_bak);
	return (TPM_DB_OK);
}

int32_t tpm_db_api_section_bak_num_entries_get(tpm_api_sections_t api_section, uint32_t *num_entries)
{
	if (hot_swap_bak_db.api_section_bak[api_section].valid == TPM_DB_INVALID) {
		printk("api_section: %d is not valid\n", api_section);
		*num_entries = 0;
	} else
		*num_entries = hot_swap_bak_db.api_section_bak[api_section].num_valid_entries;

	printk("api_section: %d, number_valid: %d\n", api_section, *num_entries);
	return (TPM_DB_OK);
}
int32_t tpm_db_api_section_bak_ent_tbl_get(tpm_api_sections_t api_sec, tpm_db_api_entry_t *api_ent_mem_area, uint32_t index)
{
	if (ILLEGAL_API_SEC(api_sec))
		IF_ERROR_I(TPM_DB_ERR_REC_NOT_EXIST, api_sec);

	if (NULL == api_ent_mem_area)
		return (TPM_DB_ERR_INV_INPUT);

	memcpy(api_ent_mem_area,
		&hot_swap_bak_db.api_ent_mem_area_bak[(hot_swap_bak_db.api_section_bak[api_sec].table_start) + index],
		sizeof(tpm_db_api_entry_t));

	return (TPM_DB_OK);
}
int32_t tpm_db_api_entry_bak_get_next(tpm_api_sections_t api_section,
				      int32_t cur_rule,
				      int32_t *next_rule)
{
	int32_t ind = -1;
	int32_t i;
	uint32_t min_rule, l_next_rule = ~0;
	tpm_db_api_entry_t *api_tbl;

	api_tbl = &(hot_swap_bak_db.api_ent_mem_area_bak[hot_swap_bak_db.api_section_bak[api_section].table_start]);

	if (cur_rule <= -1)
		min_rule = 0;
	else
		min_rule = cur_rule + 1;

	/*TPM_OS_DEBUG(TPM_DB_MOD, "min_rule(%d) \n", min_rule); */

	for (i = 0; i <= hot_swap_bak_db.api_section_bak[api_section].last_valid_index; i++) {
		if (((api_tbl + i)->valid == TPM_DB_VALID) &&
		    ((api_tbl + i)->rule_num >= min_rule) && ((api_tbl + i)->rule_num < l_next_rule)) {
			l_next_rule = (api_tbl + i)->rule_num;
			ind = i;
			TPM_OS_INFO(TPM_DB_MOD, "rule_num(%d), rule_idx(%d) update\n", (api_tbl + i)->rule_num, (api_tbl + i)->rule_idx);
		} else
			TPM_OS_INFO(TPM_DB_MOD, "rule_num(%d), rule_idx(%d) \n", (api_tbl + i)->rule_num, (api_tbl + i)->rule_idx);
	}

	/*TPM_OS_DEBUG(TPM_DB_MOD, "l_next_rule(%d) ind(%d) \n", l_next_rule, ind); */

	if ((l_next_rule < (uint32_t) (~0)) && (ind != -1)) {
		*next_rule = ind;
	} else		/* Not found */
		*next_rule = -1;

	TPM_OS_INFO(TPM_DB_MOD, "next_rule(%d) \n", *next_rule);
	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_max_uni_port_nr_get()
*
* DESCRIPTION: Get max UNI port number
*
* INPUTS:
*
* OUTPUTS:
* max_uni_port_nr       -   max UNI port number
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_max_uni_port_nr_get(uint32_t *max_uni_port_nr)
{
	*max_uni_port_nr = tpm_db.max_uni_port_nr + 1;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_uni_egr_rate_limit_set()
*
* DESCRIPTION: Set GMAC UNI egress rate limit
*
* INPUTS:
*        port - GMAC UNI port to do rate limit
*        rate_limit - rate limit value
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_gmac_uni_egr_rate_limit_set(tpm_src_port_type_t port, uint32_t rate_limit)
{
	tpm_db.gmac_uni_egr_rate_limit[port] = rate_limit;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_uni_egr_rate_limit_get()
*
* DESCRIPTION: Get GMAC UNI egress rate limit
*
* INPUTS:
*        port - GMAC UNI port to do rate limit
* OUTPUTS:
*        rate_limit - rate limit value, if value got is 0, means no rate limit set
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_gmac_uni_egr_rate_limit_get(tpm_src_port_type_t port, uint32_t *rate_limit)
{
	if (NULL == rate_limit)
		return (TPM_DB_ERR_INV_INPUT);

	*rate_limit = tpm_db.gmac_uni_egr_rate_limit[port];

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_lpk_uni_ingr_rate_limit_set()
*
* DESCRIPTION: Set GMAC UNI ingress rate limit only with loopback mode
*
* INPUTS:
*        port - GMAC UNI port to do rate limit
*        rate_limit - rate limit value
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_gmac_lpk_uni_ingr_rate_limit_set(tpm_src_port_type_t port, tpm_db_gmac_lpk_uni_ingr_rate_limit_t rate_limit)
{
	tpm_db.gmac_lpk_uni_ingr_rate_limit[port].count_mode = rate_limit.count_mode;
	tpm_db.gmac_lpk_uni_ingr_rate_limit[port].cir = rate_limit.cir;
	tpm_db.gmac_lpk_uni_ingr_rate_limit[port].cbs = rate_limit.cbs;
	tpm_db.gmac_lpk_uni_ingr_rate_limit[port].ebs = rate_limit.ebs;

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_gmac_lpk_uni_ingr_rate_limit_get()
*
* DESCRIPTION: Get GMAC UNI ingress rate limit with loopback mode
*
* INPUTS:
*        port - GMAC UNI port to do rate limit
* OUTPUTS:
*        rate_limit - rate limit value, if value got is 0, means no rate limit set
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_gmac_lpk_uni_ingr_rate_limit_get(tpm_src_port_type_t port, tpm_db_gmac_lpk_uni_ingr_rate_limit_t *rate_limit)
{
	if (NULL == rate_limit)
		return (TPM_DB_ERR_INV_INPUT);

	memcpy(rate_limit,
	       &tpm_db.gmac_lpk_uni_ingr_rate_limit[port],
	       sizeof(tpm_db_gmac_lpk_uni_ingr_rate_limit_t));

	return (TPM_DB_OK);
}

/*******************************************************************************
* tpm_db_init()
*
* DESCRIPTION:      Perform DB Initialization. Set all structures to 0, and all valid fields to TPM_DB_INVALID
*
* INPUTS:
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_db_init(void)
{
	uint32_t i, j;

	/* Erase DB */
	memset(&tpm_db, 0, sizeof(tpm_db_t));

	/* Set All entries in all structures to invalid */

	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++)
		tpm_db.eth_ports[i].valid = TPM_DB_INVALID;

	/* GMAC Connect */
	for (i = 0; i < TPM_NUM_GMACS; i++)
		tpm_db.gmac_port_conf[i].valid = TPM_DB_INVALID;
	tpm_db.num_valid_tcont_llid = 0;

	/* GMAC Func. */
	for (i = 0; i < TPM_MAX_NUM_GMACS; i++)
		tpm_db.gmac_func[i] = TPM_GMAC_FUNC_NONE;

	for (i = 0; i < TPM_MAX_NUM_TX_PORTS; i++) {
		tpm_db.gmac_tx[i].valid = TPM_DB_INVALID;
		for (j = 0; j < TPM_MAX_NUM_TX_QUEUE; j++)
			tpm_db.gmac_tx[i].tx_queue[j].valid = TPM_DB_INVALID;
	}

	/* IGMP */
	tpm_db.igmp_def.frwd_mode[TPM_SRC_PORT_WAN] = TPM_IGMP_FRWD_MODE_DROP;
	for (i = TPM_SRC_PORT_UNI_0; i <= TPM_SRC_PORT_UNI_7; i++)
		 tpm_db.igmp_def.frwd_mode[i] = TPM_IGMP_FRWD_MODE_DROP;
	tpm_db.igmp_def.cpu_queue = 0;
	tpm_db.igmp_def.igmp_snoop_enable = TPM_FALSE;

	tpm_db.igmp_def.mc_lpbk_enable[TPM_IP_VER_4] = TPM_FALSE;
	tpm_db.igmp_def.mc_lpbk_enable[TPM_IP_VER_6] = TPM_FALSE;

	/* Misc. */
	tpm_db.init_misc.omci_etype = TPM_DB_INVALID;
	tpm_db.init_misc.pnc_init_debug_port = TPM_DB_INVALID;
	tpm_db.init_misc.pon_type = TPM_DB_INVALID;
	tpm_db.init_misc.ds_mh_set_conf = TPM_DB_INVALID;
	tpm_db.init_misc.cfg_pnc_parse = TPM_DB_INVALID;
	tpm_db.init_misc.cpu_loopback = TPM_DB_INVALID;
	tpm_db.init_misc.tpm_init_succeeded = TPM_DB_INVALID;
	tpm_db.func_profile.virt_uni_info.enabled = TPM_DB_INVALID;
	tpm_db.func_profile.virt_uni_info.switch_port = TPM_DB_INVALID;
	tpm_db.func_profile.virt_uni_info.uni_port = TPM_DB_INVALID;

	/* Ownership */
	for (i = 0; i < TPM_MAX_API_TYPES; i++)
		tpm_db.owners[i].valid = TPM_DB_INVALID;

	/* API Ranges */
	for (i = 0; i < TPM_MAX_NUM_API_SECTIONS; i++) {
		for (j = 0; j < TPM_NUM_DIRECTIONS; j++)
			tpm_db_api_section_invalidate(i);
	}

	/* API_Busy */
	tpm_db_api_busy_init();

	/* API Entries Area, stripe whole area */
	tpm_db_api_entries_area_reset();

	/* PNC Tables */
	for (i = 0; i < TPM_MAX_NUM_RANGES; i++)
		tpm_db.pnc_range[i].valid = TPM_DB_INVALID;

	for (i = 0; i < TPM_PNC_SIZE; i++)
		tpm_db.pnc_shadow[i].valid = TPM_DB_INVALID;

	/* Modification Tables */
	tpm_db_mod2_init();

	/* Vlan Ethertype */
	for (i = 0; i < TPM_NUM_VLAN_ETYPE_REGS; i++)
		tpm_db.vlan_etype[i].valid = TPM_DB_INVALID;

	/* Multicast Tables */
	tpm_db_mc_lpbk_entries_num = 0;
	memset(tpm_db_mc_stream_table, 0, TPM_MC_MAX_STREAM_NUM * sizeof(tpm_db_mc_stream_entry_t *));
	memset(tpm_db_mc_mac_table, 0, TPM_MC_MAX_MAC_NUM * sizeof(tpm_db_mc_mac_entry_t *));
	memset(tpm_db_mc_lpbk_table, 0, TPM_MC_MAX_LPBK_ENTRIES_NUM * sizeof(tpm_db_mc_lpbk_entry_t *));
	memset(tpm_db_mc_vlan_xits_table, 0, TPM_MC_MAX_MVLAN_XITS_NUM * sizeof(tpm_db_mc_vlan_entry_t *));
	memset(tpm_db_mc_virt_uni_entry_state_table, 0, TPM_MC_MAX_STREAM_NUM * sizeof(uint8_t));
	tpm_db_mc_igmp_proxy_sa_mac_valid = 0;
	memset(tpm_db_mc_igmp_proxy_sa_mac, 0, 6 * sizeof(uint8_t));

	/* IPV6 Flow Tables */
	tpm_db_init_ipv6_5t_flow_reset();

	/* CTC CnM db */
	tpm_db_ctc_cm_db_reset();

	/* IPv6 MC SIP db */
	tpm_db_ipv6_mc_sip_db_reset();

	return (TPM_DB_OK);
}
