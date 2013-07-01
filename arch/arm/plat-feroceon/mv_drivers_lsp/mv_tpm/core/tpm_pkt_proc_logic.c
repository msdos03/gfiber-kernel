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
* tpm_pkt_proc_logic.c
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
*               Revision: 1.7
*
*
*******************************************************************************/

#include "tpm_common.h"
#include "tpm_header.h"

/* Local definitions */
extern MV_STATUS mv_cust_set_tcont_state(uint32_t tcont, bool state);

typedef tpm_error_code_t (*tpm_proc_common_int_del_func_t) (uint32_t, uint32_t);

int32_t tpm_proc_virt_uni_trg_port_validation(tpm_trg_port_type_t trg_port);



#define IPV6_MOD(mod_bm)		(mod_bm & (TPM_IPV6_UPDATE | TPM_HOPLIM_DEC | TPM_IPV6_DST_SET | TPM_IPV6_SRC_SET))
#define IPV4_MOD(mod_bm)		(mod_bm & (TPM_IPV4_UPDATE | TPM_TTL_DEC | TPM_IPV4_DST_SET | TPM_IPV4_SRC_SET))

#define L4_CHECK_MOD(mod_bm)		((mod_bm & TPM_IPV4_DST_SET) || (mod_bm & TPM_IPV4_SRC_SET) || \
						(mod_bm & TPM_L4_DST_SET) || (mod_bm & TPM_L4_SRC_SET))

#define PARSE_FLAG_CHECK(val, bit)  ((val >> bit) & TPM_PARSE_FLAG_CHECK_TRUE_FALSE)

#define IF_ERROR(ret)	\
		if (ret != TPM_OK) {\
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " recvd ret_code(%d)\n", ret);\
			return(ret);\
		}
#define IF_ERROR_I(ret, i)	\
		if (ret != TPM_OK) {\
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " recvd ret_code(%d), ind(%d)\n", ret, i);\
			return(ret);\
		}

#define MULTI_IP_2_MAC(macAdd, multiIp)		((uint8_t *)(macAdd))[0] = (uint8_t) 0x01;  \
						((uint8_t *)(macAdd))[1] = (uint8_t) 0x00;  \
						((uint8_t *)(macAdd))[2] = (uint8_t) 0x5e;  \
						((uint8_t *)(macAdd))[3] = (uint8_t) multiIp[1];   \
						((uint8_t *)(macAdd))[4] = (uint8_t) multiIp[2];   \
						((uint8_t *)(macAdd))[5] = (uint8_t) multiIp[3];
#define MULTI_IPV6_2_MAC(macAdd, multiIp)	((uint8_t *)(macAdd))[0] = (uint8_t) 0x33;  \
						((uint8_t *)(macAdd))[1] = (uint8_t) 0x33;  \
						((uint8_t *)(macAdd))[2] = (uint8_t) multiIp[12];  \
						((uint8_t *)(macAdd))[3] = (uint8_t) multiIp[13];   \
						((uint8_t *)(macAdd))[4] = (uint8_t) multiIp[14];   \
						((uint8_t *)(macAdd))[5] = (uint8_t) multiIp[15];

#define NO_FREE_ENTRIES()	\
		if (free_entries == 0) {\
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "No free entries\n");\
			return(TPM_FAIL);\
		}

/* Local variables */

spinlock_t tpm_proc_api_call_lock;

static tpm_cpu_loopback_t gs_cpu_loopback[TPM_MAX_CPU_LOOPBACK_NUM];
static uint32_t gn_cpu_lpbk_entry_num = 0;

static uint8_t tpm_igmp_gen_query_mac[6] = { 0x01, 0x00, 0x5e, 0x00, 0x00, 0x01};
static uint8_t tpm_mld_gen_query_mac[6] = { 0x33, 0x33, 0x00, 0x00, 0x00, 0x01};

/* Bitmap of PNC port_ids */
static uint32_t gmac_pnc_bm[3] = { TPM_BM_GMAC_0, TPM_BM_GMAC_1, TPM_BM_PMAC};

tpm_hot_swap_acl_recovery_t tpm_hot_swap_acl_recovery[] = {
	{TPM_API_MAC_LEARN,        tpm_acl_rcvr_func_mac_learn},
	{TPM_API_DS_LOAD_BALANCE,  tpm_acl_rcvr_func_ds_load_balance},
	{TPM_API_CPU_LOOPBACK,     tpm_acl_rcvr_func_cpu_loopback},
	{TPM_API_L2_PRIM,          tpm_acl_rcvr_func_l2_prim},
	{TPM_API_L3_TYPE,          tpm_acl_rcvr_func_l3_type},
	{TPM_API_IPV4,             tpm_acl_rcvr_func_ipv4},
	{TPM_API_IPV4_MC,          tpm_acl_rcvr_func_ipv4_mc},
	{TPM_API_IPV6_GEN,         tpm_acl_rcvr_func_ipv6_gen},
	{TPM_API_IPV6_DIP,         tpm_acl_rcvr_func_ipv6_dip},
	{TPM_API_IPV6_MC,          tpm_acl_rcvr_func_ipv6_mc},
	{TPM_API_IPV6_NH,          tpm_acl_rcvr_func_ipv6_nh},
	{TPM_API_IPV6_L4,          tpm_acl_rcvr_func_ipv6_l4},
	{TPM_API_CNM,              tpm_acl_rcvr_func_cnm},

};


static tpm_api_sup_param_val_t api_sup_param_val[] = {
	/* tpm_pnc_api_num		Supported parse field bits
		Supported parse flag bits
		Forbidden actions */
	{TPM_ADD_DS_LOAD_BALANCE_RULE,	TPM_DS_LOAD_BALNC_PARSE_BM_MASK,
		(TPM_PARSE_FLAG_TAG1_MASK | TPM_PARSE_FLAG_TAG2_MASK),
		(0)},
	{TPM_ADD_L2_PRIM_ACL_RULE,      TPM_L2_PARSE_BM_MASK,
		(TPM_PARSE_FLAG_TAG1_MASK | TPM_PARSE_FLAG_TAG2_MASK),
		(0)},
	{TPM_ADD_L3_TYPE_ACL_RULE,      TPM_L3_PARSE_BM_MASK,
		(TPM_PARSE_FLAG_TAG1_MASK | TPM_PARSE_FLAG_TAG2_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV4_ACL_RULE, TPM_IPV4_PARSE_BM_MASK,
		(TPM_PARSE_FLAG_TAG1_MASK | TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_NH_ACL_RULE,      0,
		(TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_GEN_ACL_RULE, TPM_IPV6_GEN_BM_MASK,
		(TPM_PARSE_FLAG_TAG1_MASK | TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_DIP_ACL_RULE, TPM_IPv6_PARSE_DIP,
		(TPM_PARSE_FLAG_TAG1_MASK | TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_L4_PORTS_ACL_RULE, TPM_IPV6_L4_BM_MASK,
		(TPM_PARSE_FLAG_L4P_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_GEN_5T_RULE, TPM_IPV6_GEN_BM_MASK | TPM_IPV6_L4_BM_MASK,
		(TPM_PARSE_FLAG_L4P_MASK | TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_DIP_5T_RULE, TPM_IPv6_PARSE_DIP | TPM_IPV6_GEN_BM_MASK | TPM_IPV6_L4_BM_MASK,
		(TPM_PARSE_FLAG_L4P_MASK | TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
	{TPM_ADD_IPV6_L4_PORTS_5T_RULE, TPM_IPV6_L4_BM_MASK,
		(TPM_PARSE_FLAG_L4P_MASK | TPM_PARSE_FLAG_PPPOE_MASK | TPM_PARSE_FLAG_MTM_MASK | TPM_PARSE_FLAG_TO_CPU_MASK),
		(TPM_ACTION_MTM | TPM_ACTION_SPEC_MC_VID)},
};


/* Function Declarations */
int32_t tpm_proc_calc_cnm_rule_num(tpm_src_port_type_t src_port, uint32_t precedence, uint32_t *rule_num);

int32_t tpm_proc_check_cnm_ipv4_pre_filter_triple_key_rule(tpm_src_port_type_t src_port, uint32_t partner_key_bm, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule(tpm_src_port_type_t src_port, uint32_t partner_key_bm, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_proc_check_cnm_ipv4_pre_filter_single_key_rule(tpm_src_port_type_t src_port, uint32_t partner_key_bm, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_proc_add_cnm_ipv4_pre_filter_triple_key_rule(tpm_src_port_type_t src_port, uint32_t key_idx, uint32_t key_pattern, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule(tpm_src_port_type_t src_port, uint32_t key_idx, uint32_t key_pattern, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_proc_add_cnm_ipv4_pre_filter_single_key_rule(tpm_src_port_type_t src_port, uint32_t key_idx, uint32_t key_pattern, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_proc_del_cnm_ipv4_pre_filter_rule(uint32_t src_port, uint32_t key_pattern, uint32_t key_idx);

tpm_cnm_ipv4_pre_filter_rule_oper_t gs_cnm_ipv4_pre_filter_rule_oper[TPM_CNM_IPV4_PRE_FILTER_KEY_MAX][TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_KEY] =
{
	/* TPM_CNM_IPV4_PRE_FILTER_KEY_1 */
	{
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_triple_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_triple_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_single_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_single_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
	},

	/* TPM_CNM_IPV4_PRE_FILTER_KEY_2 */
	{
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_triple_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_triple_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_single_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_single_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
	},

	/* TPM_CNM_IPV4_PRE_FILTER_KEY_3 */
	{
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_triple_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_triple_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK|TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
		{
			TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK,
			tpm_proc_check_cnm_ipv4_pre_filter_single_key_rule,
			tpm_proc_add_cnm_ipv4_pre_filter_single_key_rule,
			tpm_proc_del_cnm_ipv4_pre_filter_rule,
		},
	},

};

uint32_t sg_l2_cnm_prec_ind[TPM_MAX_NUM_CTC_PRECEDENCE] = {0x0, 0x0, 0x0, 0x1, 0x1, 0x3, 0x3, 0x7};
uint32_t sg_ipv4_cnm_prec_mask[TPM_MAX_NUM_CTC_PRECEDENCE] = {0x0, 0x0, 0x1, 0x1, 0x2, 0x2, 0x4, 0x4};


/* Local Functions */

/* Function Declarations */
tpm_error_code_t tpm_owner_id_check(tpm_api_type_t api_type, uint32_t owner_id);
int32_t tpm_proc_multicast_reset(void);

uint32_t tpm_proc_bc_check(tpm_l2_acl_key_t *l2_key)
{
	uint32_t i;

	for (i = 0; i < 6; i++) {
		if ((l2_key->mac.mac_da[i] & l2_key->mac.mac_da_mask[i]) != 0xFF)
			return(TPM_FALSE);
	}

	return(TPM_TRUE);
}


uint32_t tpm_proc_mc_check(tpm_l2_acl_key_t *l2_key)
{

	if ((l2_key->mac.mac_da[0] & l2_key->mac.mac_da_mask[0]) & 0x01)
		return(TPM_TRUE);

	return(TPM_FALSE);
}

/*******************************************************************************
* tpm_proc_src_port_gmac_bm_map()
*
* DESCRIPTION:    The function correlates a source_port to it's Rx GMAC(s) by means of the GMAC Functionality
*
*
* INPUTS:
* src_port         - source port in API format
*
* OUTPUTS:
* gmac_bm          - Bitmap of the GMACs relevant to set in TCAM
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_src_port_gmac_bm_map(tpm_src_port_type_t src_port,
				      tpm_gmac_bm_t *gmac_bm)
{
	tpm_gmacs_enum_t gmac_i;
	tpm_gmac_bm_t l_gmac_bm = 0;
	tpm_db_gmac_func_t gmac_func;
	tpm_init_gmac_conn_conf_t gmac_port_conf;

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i <= TPM_MAX_GMAC; gmac_i++) {
		if (!tpm_db_gmac_valid(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gmac_func);

		/* WAN possiblilties */
		if (FROM_WAN(src_port) && GMAC_IS_WAN(gmac_func))
			l_gmac_bm |= gmac_pnc_bm[gmac_i];

		/* LAN possiblilties  (Note: can be from both WAN or LAN) */

		/* From UNI_$, include UNI_virt Port */
		if (FROM_SPEC_UNI(src_port) && GMAC_IS_LAN(gmac_func)) {
			l_gmac_bm |= gmac_pnc_bm[gmac_i];
			//break;
		/* Any remaining LAN option (UNI_ANY or WAN_OR_LAN) */
		} else if (FROM_LAN(src_port) && (GMAC_IS_UNI_LAN(gmac_func) || GMAC_IS_LAN(gmac_func))) {
			if (GMAC_IS_UNI_LAN(gmac_func)) {
				if (tpm_db_gmac_conn_conf_get(gmac_i, &gmac_port_conf)) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "gmac(%d) connection info get fail\n", gmac_i);
					return(TPM_FAIL);
				}
				if (src_port == gmac_port_conf.port_src) {
					l_gmac_bm |= gmac_pnc_bm[gmac_i];
					break;
				} else if (src_port == TPM_SRC_PORT_UNI_ANY) {
					l_gmac_bm |= gmac_pnc_bm[gmac_i];
				}
			} else {
				l_gmac_bm |= gmac_pnc_bm[gmac_i];
			}
		}
	}

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "gmac_bm(0x%x)\n", l_gmac_bm);
	*gmac_bm = l_gmac_bm;
	return(TPM_OK);
}

void tpm_proc_set_int_structs(tpm_pncl_pnc_full_t *pnc_data, tpm_pncl_offset_t *start_offset,
			      tpm_rule_entry_t *api_data, tpm_db_pnc_conn_t *pnc_conn,
			      tpm_db_pnc_range_t *range_data)
{
	/* Set Structs to zero */
	memset(pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(api_data, 0, sizeof(tpm_rule_entry_t));
	memset(pnc_conn, 0, sizeof(tpm_db_pnc_conn_t));
	memset(range_data, 0, sizeof(tpm_db_pnc_range_t));

	return;
}

int32_t tpm_proc_create_acl_pnc_entry(tpm_api_sections_t api_section, uint32_t rule_num,
				      tpm_pncl_pnc_full_t *pnc_data, uint32_t *pnc_entry, uint32_t *api_rng_entries)
{

	tpm_db_pnc_range_t range_data;
	uint32_t pnc_range_id;
	uint32_t pnc_range_start, api_start, pnc_stop_entry, l_api_rng_entries;
	int32_t int_ret_code;

	/* Get pnc_range_id */
	int_ret_code = tpm_db_api_section_main_pnc_get(api_section, &pnc_range_id);
	IF_ERROR(int_ret_code);

	/*** Calculate PNC Entry ***/

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(pnc_range_id, &range_data);
	IF_ERROR(int_ret_code);

	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Get number of existing api entries */
	int_ret_code = tpm_db_api_section_num_entries_get(api_section, &l_api_rng_entries);
	IF_ERROR(int_ret_code);

	/* Calculate absolute PNC entry number to execute */
	*pnc_entry = (pnc_range_start + api_start) + rule_num;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d), l_api_rng_entries(%d)\n", rule_num, l_api_rng_entries);

	/* Call PNC Entry Insert, if this is not the api_section's new last entry */
	if (rule_num < l_api_rng_entries) {
		pnc_stop_entry = (pnc_range_start + api_start) + (l_api_rng_entries - 1);
		int_ret_code = tpm_pncl_entry_insert(*pnc_entry, pnc_stop_entry, pnc_data);
		IF_ERROR(int_ret_code);
	} else {		/* Otherwise just set the entry (no insertion) */

		int_ret_code = tpm_pncl_entry_set(*pnc_entry, pnc_data);
		IF_ERROR(int_ret_code);
	}

	/* Decrease number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_dec(pnc_range_id);
	IF_ERROR(int_ret_code);

	*api_rng_entries = l_api_rng_entries;

	return(TPM_OK);
}

int32_t tpm_proc_create_table_pnc_entry(tpm_api_sections_t api_section, uint32_t rule_num, uint32_t update_sram,
					tpm_pncl_pnc_full_t *pnc_data, uint32_t *pnc_entry,
					uint32_t *api_rng_entries)
{

	tpm_db_pnc_range_t range_data;
	uint32_t pnc_range_id;
	uint32_t pnc_range_start, api_start, l_api_rng_entries;
	int32_t int_ret_code;

	/* Get pnc_range_id */
	int_ret_code = tpm_db_api_section_main_pnc_get(api_section, &pnc_range_id);
	IF_ERROR(int_ret_code);

	/*** Calculate PNC Entry ***/

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(pnc_range_id, &range_data);
	IF_ERROR(int_ret_code);

	/* check the rule_number */
	if (rule_num > range_data.pnc_range_conf.api_end) {
		/* rule_number out of range */
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " rule_num(%d) out of range, range api_end(%d)\n",
			     rule_num, range_data.pnc_range_conf.api_end);
		return(TPM_FAIL);
	}
	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Get number of existing api entries */
	int_ret_code = tpm_db_api_section_num_entries_get(api_section, &l_api_rng_entries);
	IF_ERROR(int_ret_code);

	/* Calculate absolute PNC entry number to execute */
	*pnc_entry = (pnc_range_start + api_start) + rule_num;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d), l_api_rng_entries(%d)\n", rule_num, l_api_rng_entries);

	if (update_sram) {
		/* Just update sram  */
		int_ret_code = tpm_pncl_update_sram(*pnc_entry, pnc_data);
		IF_ERROR(int_ret_code);
	} else {
		/* Just set the entry (no insertion) */
		int_ret_code = tpm_pncl_entry_set(*pnc_entry, pnc_data);
		IF_ERROR(int_ret_code);

		/* Decrease number of free entries in pnc_range */
		int_ret_code = tpm_db_pnc_rng_free_ent_dec(pnc_range_id);
		IF_ERROR(int_ret_code);
	}

	*api_rng_entries = l_api_rng_entries;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_trg_port_gmac_map()
*
* DESCRIPTION:    The function correlates a source_port to it's Rx GMAC(s) by means of the GMAC Functionality
*
*
* INPUTS:
* trg_port         - source port in API format
*
* OUTPUTS:
* gmac_port        - Target GMAC
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_trg_port_gmac_map(tpm_trg_port_type_t trg_port, tpm_gmacs_enum_t *gmac_port)
{
	tpm_gmacs_enum_t gmac_i;
	tpm_db_gmac_func_t gmac_func;
	tpm_init_gmac_conn_conf_t gmac_port_conf;

	if (gmac_port == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid pointer-NULL \n");
		return(ERR_GENERAL);
	}

	(*gmac_port) = -1;
	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i <= TPM_MAX_GMAC; gmac_i++) {
		if (!tpm_db_gmac_valid(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gmac_func);
		if (tpm_db_gmac_conn_conf_get(gmac_i, &gmac_port_conf)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "gmac(%d) connection info get fail\n", gmac_i);
			return(TPM_FAIL);
		}
		/* TRG WAN */
		if ((TRG_WAN(trg_port)) && GMAC_IS_WAN(gmac_func)) {
			(*gmac_port) = gmac_i;
			break;
		/* TRG GMAC_UNI, such as MC lpk, dual gmac uni */
		} else if (TRG_UNI(trg_port) && (GMAC_IS_UNI_LAN(gmac_func) || GMAC_IS_DS_UNI_LAN(gmac_func))) {
			if ((gmac_port_conf.port_src != TPM_SRC_PORT_ILLEGAL) &&
			    ((trg_port == (1 << (gmac_port_conf.port_src + TPM_TRG_UNI_OFFSET))) ||
			     (trg_port == TPM_TRG_PORT_UNI_ANY))) {
				(*gmac_port) = gmac_i;
				break;
			}
		/* TRG UNI, such as KW2 */
		} else if (TRG_UNI(trg_port) && GMAC_IS_LAN(gmac_func)) {
			(*gmac_port) = gmac_i;
			break;
		}
	}
	return(TPM_OK);
}

uint32_t tpm_proc_gmac1_phy_src_port(tpm_src_port_type_t src_port)
{
	tpm_db_gmac_func_t gmac_func;

	if (src_port == TPM_SRC_PORT_UNI_VIRT) {
		tpm_db_gmac_func_get(TPM_ENUM_GMAC_1, &gmac_func);
		if (gmac_func != TPM_GMAC_FUNC_VIRT_UNI)
			return(TPM_TRUE);
	}

	return(TPM_FALSE);
}

/*******************************************************************************
* tpm_proc_all_gmac_bm()
*
* DESCRIPTION:  Gets a TCAMport_bitmap of all active GMACs
*
* INPUTS:
*
* RETURNS:
* Returns a bitmap of all the active GMACs
*
* COMMENTS:  Perfom only DB integrity checks, not data correctness checks
*
*******************************************************************************/
uint32_t tpm_proc_all_gmac_bm(void)
{
	tpm_gmacs_enum_t gmac_i;
	tpm_gmac_bm_t l_gmac_bm = 0;
	tpm_db_gmac_func_t gmac_func;

	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i <= TPM_MAX_GMAC; gmac_i++) {
		if (!tpm_db_gmac_valid(gmac_i))
			continue;

		tpm_db_gmac_func_get(gmac_i, &gmac_func);

		if (gmac_func != TPM_GMAC_FUNC_NONE)
			l_gmac_bm |= gmac_pnc_bm[gmac_i];
	}
	return(l_gmac_bm);
}

int32_t tpm_proc_delete_mod(tpm_mod_owner_t owner, tpm_gmacs_enum_t gmac_port, uint32_t mod_entry)
{
	tpm_gmacs_enum_t  duplicate_gmac;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;
	int32_t ret_code;

	ret_code = tpm_mod2_entry_del(owner, gmac_port, mod_entry);
	IF_ERROR(ret_code);

	/* when ds load balance on G0 and G1 is enabled, need to duplicate DS PMT on G0/1 */
	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
	if (	(TPM_DS_MAC_BASED_TRUNK_DISABLED == ds_mac_based_trunk_enable)
	     || ((gmac_port != TPM_ENUM_GMAC_0) && (gmac_port != TPM_ENUM_GMAC_1))) {
		/* if this is US or DS_MAC_BASED_TRUNK is DISABLED, do nothing */
		return(TPM_OK);
	}

	if (gmac_port == TPM_ENUM_GMAC_0)
		duplicate_gmac = TPM_ENUM_GMAC_1;
	else
		duplicate_gmac = TPM_ENUM_GMAC_0;

	ret_code = tpm_mod2_entry_del(owner, duplicate_gmac, mod_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

int32_t tpm_proc_create_mod(tpm_pkt_action_t pkt_act, tpm_trg_port_type_t trg_port, tpm_pkt_mod_t *pkt_mod,
			    tpm_pkt_mod_bm_t pkt_mod_bm, tpm_pkt_mod_int_bm_t int_mod_bm, uint32_t *mod_entry,
			    uint32_t *trg_gmac)
{
	int32_t ret_code;
	tpm_gmacs_enum_t  duplicate_gmac;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	if (SET_MOD(pkt_act)) {
		/* Currently supporting Vlan operation only */
		/* Get dest. gmac */
		if (TPM_TRG_LOAD_BAL & trg_port) {
			/* DS load balance, set trg port to G1 */
			*trg_gmac = TPM_ENUM_GMAC_1;
		} else {
			tpm_proc_trg_port_gmac_map(trg_port, trg_gmac);
			if (*trg_gmac == -1) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "pkt modification not possible on this target gmac(%d) \n",
					     *trg_gmac);
				return(ERR_ACTION_INVALID);
			}
		}
		ret_code = tpm_mod2_entry_set(TPM_MOD_OWNER_TPM, *trg_gmac, pkt_mod_bm, int_mod_bm, pkt_mod, mod_entry);
		IF_ERROR(ret_code);

		/* when ds load balance on G0 and G1 is enabled, need to duplicate DS PMT on G0/1 */
		tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
		if (    (TPM_DS_MAC_BASED_TRUNK_DISABLED == ds_mac_based_trunk_enable)
		     || (TRG_WAN(trg_port))) {
		     /* if this is US or DS_MAC_BASED_TRUNK is DISABLED, do nothing */
			return(TPM_OK);
		}

		if (*trg_gmac == TPM_ENUM_GMAC_0)
			duplicate_gmac = TPM_ENUM_GMAC_1;
		else if (*trg_gmac == TPM_ENUM_GMAC_1)
			duplicate_gmac = TPM_ENUM_GMAC_0;
		else {
			TPM_OS_INFO(TPM_TPM_LOG_MOD, "target gmac(%d) invalid\n", *trg_gmac);
			return(TPM_OK);
		}

		ret_code = tpm_mod2_entry_set(TPM_MOD_OWNER_TPM, duplicate_gmac,
						pkt_mod_bm, int_mod_bm, pkt_mod, mod_entry);
		IF_ERROR(ret_code);
	}
	return(TPM_OK);
}

void tpm_proc_set_api_moddata(tpm_pkt_action_t pkt_act, uint32_t trg_gmac, tpm_db_mod_conn_t *mod_con,
			      uint32_t mod_entry)
{
	/* Set modification data in API data */
	if (SET_MOD(pkt_act)) {
		mod_con->mod_cmd_ind = mod_entry;
		mod_con->mod_cmd_mac = trg_gmac;
	} else {
		mod_con->mod_cmd_ind = 0;
		mod_con->mod_cmd_mac = TPM_INVALID_GMAC;
	}
	return;
}

int32_t tpm_proc_check_missing_data(tpm_rule_action_t *rule_action,
				    tpm_pkt_mod_t *pkt_mod,
				    tpm_pkt_frwd_t *pkt_frwd,
				    void *parsing_key,
				    tpm_pkt_action_t pkt_act,
				    tpm_parse_fields_t parse_rule_bm)
{

	/* Check keys exist for parse fields */
	if (rule_action == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "rule_action cannot be NULL\n");
		return(ERR_ACTION_INVALID);
	}

	if ((pkt_mod == NULL) && (SET_MOD(pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Mod requested with NULL pointer\n");
		return(ERR_MOD_INVALID);
	}

	if ((pkt_frwd == NULL) && ((SET_TARGET_PORT(pkt_act)) || SET_TARGET_QUEUE(pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target set requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	if ((parsing_key == NULL) && (parse_rule_bm != 0)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parsing requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	return(TPM_OK);
}

int32_t tpm_proc_check_valid_target(tpm_dir_t dir,
				    tpm_db_pon_type_t pon_type,
				    tpm_src_port_type_t src_port,
				    tpm_trg_port_type_t trg_port,
				    uint8_t trg_queue,
				    tpm_pkt_action_t pkt_act,
				    uint8_t ds_load_bal_valid)
{

	tpm_init_virt_uni_t virt_uni_info;
	int32_t ret_code;
	uint32_t rx_queue_valid, rx_queue_size;
	tpm_gmac_bm_t gmac_bm;
	tpm_gmacs_enum_t gmac;
	uint32_t tx_queue_valid, tx_queue_size;
	tpm_db_txq_owner_t tx_owner;
	tpm_db_tx_mod_t tx_port;
	tpm_gmacs_enum_t act_wan= tpm_db_active_wan_get();

	/* Check Valid Target */
	if (SET_TARGET_PORT(pkt_act)) {
		if (TO_PON(dir, trg_port, pon_type, act_wan) ||
		    TO_ETHWAN(dir, trg_port, act_wan) || TO_CPU(trg_port)) {
		    /* PON CPU is OK */
		} else if (TO_LAN(dir, trg_port)){
			/* check target uni port valid or not */
			ret_code = tpm_proc_check_dst_uni_port(trg_port);
			IF_ERROR(ret_code);

			/* check ds load balance trg */
			if (trg_port & TPM_TRG_LOAD_BAL) {
				if (!ds_load_bal_valid) {
					/* TPM_TRG_LOAD_BAL should not be set */
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "TPM_TRG_LOAD_BAL should not be set\n");
					return(ERR_FRWD_INVALID);
				}
				if (!tpm_db_ds_load_bal_en_get()) {
					/* profile dose not support TPM_TRG_LOAD_BAL */
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "profile dose not support TPM_TRG_LOAD_BAL\n");
					return(ERR_FRWD_INVALID);
				}
			}
		} else {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "* dir=%d, trg_port=%d, pon_type=%d *\r\n", dir, trg_port,
				     pon_type);
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Illegal Target Port\n");
			return(ERR_FRWD_INVALID);
		}
	}
	/* check valid target when virt uni via GMAC1 feature is enabled */
	/* not all target port combinations are supported when WiFi via GMAC1 = UNI_4 feature is enabled */
	ret_code = tpm_db_virt_info_get(&virt_uni_info);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " virt uni recvd ret_code(%d)\n", ret_code);
		return(ERR_FRWD_INVALID);
	}
	if ((virt_uni_info.enabled == 1) && (dir == TPM_DIR_DS) && (SET_TARGET_PORT(pkt_act))) {
		/* virt uni feature is enabled - validate and recalculate the mh_reg */
		ret_code = tpm_proc_virt_uni_trg_port_validation(trg_port);
		if (ret_code != TPM_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "* dir=%d, trg_port=%d, pon_type=%d *\r\n", dir, trg_port,
				     pon_type);
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Illegal Virt UNI Target Port.\n");
			return(ERR_FRWD_INVALID);
		}
	}

	/* Check Valid Queue */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	/* TODO - Check Queue depending on actual queues in target or in Rx */
	if (SET_TARGET_QUEUE(pkt_act) && (trg_queue >= TPM_MAX_NUM_TX_QUEUE)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Out of Range\n");
		return(ERR_FRWD_INVALID);
	}
	/* Check Rx queue valid */
	if (SET_TARGET_PORT(pkt_act) && TO_CPU(trg_port) && SET_TARGET_QUEUE(pkt_act)) {
		for (gmac = TPM_ENUM_GMAC_0; gmac < TPM_MAX_NUM_GMACS; gmac++) {
			if (((gmac_bm & TPM_BM_GMAC_0) && (gmac == TPM_ENUM_GMAC_0)) ||
			    ((gmac_bm & TPM_BM_GMAC_1) && (gmac == TPM_ENUM_GMAC_1)) ||
			    ((gmac_bm & TPM_BM_PMAC) && (gmac == TPM_ENUM_PMAC))) {
				/* Get Rx queue info */
				ret_code = tpm_db_gmac_rx_q_conf_get(gmac, trg_queue, &rx_queue_valid, &rx_queue_size);
				if (ret_code != TPM_DB_OK) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, " rx queue recvd ret_code(%d)\n", ret_code);
					return(ERR_FRWD_INVALID);
				}

				/* Check queue valid state */
				if (TPM_FALSE == rx_queue_valid) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Invalid\n");
					return(ERR_FRWD_INVALID);
				}

				/* Check queue size */
				if (0 == rx_queue_size) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Size is Zero\n");
					return(ERR_FRWD_INVALID);
				}
			}
		}
	}

	/* Check Tx queue valid */
	if (SET_TARGET_PORT(pkt_act) && (!TO_CPU(trg_port)) && SET_TARGET_QUEUE(pkt_act)) {
		tpm_proc_trg_port_gmac_map(trg_port, &gmac);
		for (tx_port = TPM_TX_MOD_GMAC0; tx_port < TPM_MAX_NUM_TX_PORTS; tx_port++) {
			if (((trg_port & TPM_TRG_TCONT_0) && (act_wan == TPM_ENUM_PMAC) && (tx_port == TPM_TX_MOD_PMAC_0)) ||
			    ((trg_port & TPM_TRG_TCONT_0) && (act_wan == TPM_ENUM_GMAC_0) && (tx_port == TPM_TX_MOD_GMAC0)) ||
			    ((trg_port & TPM_TRG_TCONT_0) && (act_wan == TPM_ENUM_GMAC_1) && (tx_port == TPM_TX_MOD_GMAC1)) ||
			    ((trg_port & TPM_TRG_TCONT_1) && (tx_port == TPM_TX_MOD_PMAC_1)) ||
			    ((trg_port & TPM_TRG_TCONT_2) && (tx_port == TPM_TX_MOD_PMAC_2)) ||
			    ((trg_port & TPM_TRG_TCONT_3) && (tx_port == TPM_TX_MOD_PMAC_3)) ||
			    ((trg_port & TPM_TRG_TCONT_4) && (tx_port == TPM_TX_MOD_PMAC_4)) ||
			    ((trg_port & TPM_TRG_TCONT_5) && (tx_port == TPM_TX_MOD_PMAC_5)) ||
			    ((trg_port & TPM_TRG_TCONT_6) && (tx_port == TPM_TX_MOD_PMAC_6)) ||
			    ((trg_port & TPM_TRG_TCONT_7) && (tx_port == TPM_TX_MOD_PMAC_7)) ||
			    ((trg_port & (TPM_TRG_UNI_0 |
			    		  TPM_TRG_UNI_1 |
			    		  TPM_TRG_UNI_2 |
			    		  TPM_TRG_UNI_3 |
			    		  TPM_TRG_UNI_4 |
			    		  TPM_TRG_UNI_5 |
			    		  TPM_TRG_UNI_6 |
			    		  TPM_TRG_UNI_7 |
			    		  TPM_TRG_UNI_VIRT |
			    		  TPM_TRG_PORT_UNI_ANY)) && (tx_port == (tpm_db_tx_mod_t)gmac))) {
				/* Get Tx queue info */
				ret_code = tpm_db_gmac_tx_q_conf_get(tx_port,
								     trg_queue,
								     &tx_queue_valid,
								     NULL,
								     &tx_owner,
								     NULL,
								     &tx_queue_size,
								     NULL);
				if (ret_code != TPM_DB_OK) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, " tx queue recvd ret_code(%d)\n", ret_code);
					return(ERR_FRWD_INVALID);
				}

				/* Check queue valid state */
				if (TPM_FALSE == tx_queue_valid) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Invalid\n");
					return(ERR_FRWD_INVALID);
				}

				/* Check queue owner */
				if (((gmac_bm & TPM_BM_GMAC_0) && (tx_owner != TPM_Q_OWNER_GMAC0)) ||
				    ((gmac_bm & TPM_BM_GMAC_1) && (tx_owner != TPM_Q_OWNER_GMAC1)) ||
				    ((gmac_bm & TPM_BM_PMAC) && (tx_owner != TPM_Q_OWNER_PMAC))) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Owner Invalid, gmac_bm: [%d], tx_owner: [%d]\n",
									gmac_bm, tx_owner);
					return(ERR_FRWD_INVALID);
				}

				/* check queue size */
				if (0 == tx_queue_size) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Size is Zero\n");
					return(ERR_FRWD_INVALID);
				}
			}
		}
	}

	return(TPM_OK);
}

int32_t tpm_proc_check_pkt_action(tpm_pkt_action_t pkt_act,
				  tpm_trg_port_type_t trg_port,
				  tpm_pkt_mod_t *pkt_mod,
				  tpm_pkt_mod_bm_t pkt_mod_bm)
{
	if (PKT_DROP(pkt_act)) {
		if (SET_TARGET_PORT(pkt_act) || SET_TARGET_QUEUE(pkt_act) || SET_MOD(pkt_act) || SET_CPU(pkt_act)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet dropped action, no other packet actions are allowed \n");
			return(ERR_ACTION_INVALID);
		}
	}
	if (SET_CPU(pkt_act)) {
		if ((!SET_TARGET_PORT(pkt_act)) || (trg_port != TPM_TRG_PORT_CPU)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD,"trg_port %x \n", trg_port);
			TPM_OS_ERROR(TPM_TPM_LOG_MOD,
				     "For packet TO_CPU action, target port should be set and equal to CPU port.\n");
			return(ERR_ACTION_INVALID);
		}
	}

	if (SET_MOD(pkt_act)) {
		/* if split mod, do not check trgt port */
		if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable())
			&& (pkt_mod_bm == TPM_VLAN_MOD)
			&& (pkt_mod->vlan_mod.vlan_op == VLANOP_SPLIT_MOD_PBIT)) {

		} else if ((!SET_TARGET_PORT(pkt_act)) || (trg_port == TPM_TRG_PORT_CPU)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD,
				     "For rule SET_MOD action, target port should be set and should not be CPU port. \n");
			return(ERR_ACTION_INVALID);
		}
	}

	return(TPM_OK);
}

#if 0				/*Phase1 - no longer has below limitation */
int32_t tpm_proc_pkt_mod_check(tpm_pkt_action_t pkt_act, tpm_pkt_mod_bm_t pkt_mod_bm, tpm_pkt_mod_t *pkt_mod)
{
	if (SET_MOD(pkt_act)) {
		if (pkt_mod_bm != TPM_VLAN_MOD) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Support only VLAN operations \n");
			return(ERR_MOD_INVALID);
		}
		if (pkt_mod->vlan_mod.vlan_op >= VLANOP_ILLEGAL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Illegal VLAN Operation \n");
			return(ERR_MOD_INVALID);
		}
	}
	return(TPM_OK);
}
#endif

/* in PARSE_FLAG_BM - bit#0 and bit#1 - TRUE and FALSE - should not be set simultaneously
   check pairs of bits in the parse_flags_bm for validation */
int32_t tpm_proc_check_parse_flag_valid(tpm_parse_flags_t parse_flags_bm)
{
	int32_t i;

	for (i = 0; i < TPM_PARSE_FLAG_CHECK_FIELD_NUMBER; i++) {
		if (PARSE_FLAG_CHECK(parse_flags_bm, i * 2) == TPM_PARSE_FLAG_CHECK_TRUE_FALSE)
			return(ERR_PARSE_MAP_INVALID);
	}
	return(TPM_OK);
}

/* Get GMAC Lan UNI number and UNI port number */
/*******************************************************************************
* tpm_proc_gmaclanuni_uninum_get()
*
* DESCRIPTION:    The function Get GMAC Lan UNI number and UNI port number.
*
* INPUTS:
* src_port         - None
*
* OUTPUTS:
* gmac_is_uni_num  - number of GMAC which is LAN UNI
* max_uni_port_num - number os UNI ports
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_gmaclanuni_uninum_get(uint32_t *gmac_is_uni_num, uint32_t *max_uni_port_num)
{
	tpm_db_gmac_func_t gmac_func;
	tpm_gmacs_enum_t gmac_i;
	uint32_t temp1 = 0, temp2 = 0;

	/* Cal number of GMAC LAN UNI */
	for (gmac_i = TPM_ENUM_GMAC_0; gmac_i <= TPM_MAX_GMAC; gmac_i++) {
		tpm_db_gmac_func_get(gmac_i, &gmac_func);
		if (GMAC_IS_UNI_LAN(gmac_func))
			temp1++;
	}
	/* Get Max UNI port number */
	tpm_db_max_uni_port_nr_get(&temp2);

	*gmac_is_uni_num = temp1;
	*max_uni_port_num = temp2;

	return (TPM_OK);
}

/*******************************************************************************
* tpm_proc_src_port_check()
*
* DESCRIPTION:    The function checks if the requested source port is legit.
*
* INPUTS:
* src_port         - source port in API format
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_src_port_check(tpm_src_port_type_t src_port)
{
	tpm_db_pon_type_t pon_type;
	int32_t ret_code;
	tpm_db_chip_conn_t dummy_chip_con;
	tpm_db_int_conn_t dummy_int_conn;
	uint32_t dummy_switch_port;
	tpm_init_virt_uni_t virt_uni_info;
	uint32_t gmac_is_uni_num = 0, max_uni_port_num = 0;

	/* Check Port exists */
	if (src_port == TPM_SRC_PORT_WAN) {
		if (TPM_ENUM_PMAC == tpm_db_active_wan_get()){
       		tpm_db_pon_type_get(&pon_type);
			if (pon_type >= TPM_NONE) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "No pon_type defined (0x%x)\n", pon_type);
				return(TPM_FAIL);
			}
		}
	} else if (FROM_SPEC_UNI(src_port) && (src_port != TPM_SRC_PORT_UNI_VIRT)) {
		/* Check if port exists */
		ret_code = tpm_db_eth_port_conf_get(src_port,
						    &dummy_chip_con,
						    &dummy_int_conn,
						    &dummy_switch_port);
		if (ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Source UNI Port (%d) is not valid port \n", src_port);
			return(TPM_FAIL);
		}
	} else if (src_port == TPM_SRC_PORT_UNI_ANY) {
		/* Check UNI_ANY is supported or not */
		/* Get GMAC LAN_UNI and UNI ports number */
		tpm_proc_gmaclanuni_uninum_get(&gmac_is_uni_num, &max_uni_port_num);
		if (gmac_is_uni_num > TPM_SRC_PORT_UNI_1 ||
		    (gmac_is_uni_num == TPM_SRC_PORT_UNI_1 && max_uni_port_num > TPM_SRC_PORT_UNI_1)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Src port UNI_ANY is not supported\n");
			return(ERR_SRC_PORT_INVALID);
		}
		return(TPM_OK);
	} else if (src_port == TPM_SRC_PORT_UNI_VIRT) {
		tpm_db_virt_info_get(&virt_uni_info);
		if (TPM_VIRT_UNI_DISABLED == virt_uni_info.enabled) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "UNI_VIRT is not enabled\n");
			return(ERR_SRC_PORT_INVALID);
		}
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Source UNI Port (%d) out of range \n", src_port);
		return(TPM_FAIL);
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_src_port_dir_map()
*
* DESCRIPTION:    The function maps the source port to the packet direction.
*
* INPUTS:
* src_port         - source port in API format
*
* OUTPUTS:
* dir              - Upstream or downstream direction
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_src_port_dir_map(tpm_src_port_type_t src_port, tpm_dir_t *dir)
{
	if (src_port == TPM_SRC_PORT_WAN)
		(*dir) = TPM_DIR_DS;
	else
		(*dir) = TPM_DIR_US;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " Source port(%d), direction(%d) \n", src_port, (*dir));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_setstage_done()
*
* DESCRIPTION:
*
* INPUTS:
* rule_action      -
* sram_data
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
void tpm_proc_setstage_done(tpm_rule_action_t *rule_action, tpm_pncl_sram_data_t *sram_data)
{
	sram_data->next_lu_id = 0;
	sram_data->next_lu_off_reg = 0;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_LUD;
	return;
}

/*******************************************************************************
* tpm_proc_set_mod()
*
* DESCRIPTION:
*
* INPUTS:
* rule_action      -
* sram_data
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
void tpm_proc_set_mod(tpm_rule_action_t *rule_action, tpm_pncl_sram_data_t *sram_data, uint32_t mod_cmd)
{
	if (SET_MOD(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_cmd);
	}
}

/*******************************************************************************
* tpm_proc_set_cust_cpu_packet_parse()
*
* DESCRIPTION:
*
* INPUTS:
* rule_action      -
* sram_data
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
void tpm_proc_set_cust_cpu_packet_parse(tpm_rule_action_t *rule_action, tpm_pncl_sram_data_t *sram_data)
{
	/* set the RI = TPM_PNCL_SET_RX_SPECIAL for packets with CUSTOMIZE flag */
	if (SET_CUST(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_RX_SPECIAL;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Customization flag.\n");
	}

}

/*******************************************************************************
* tpm_proc_set_trgt()
*
* DESCRIPTION:
*
* INPUTS:
* rule_action      -
* sram_data
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_set_trgt_queue(tpm_rule_action_t *rule_action,
				tpm_pkt_frwd_t *pkt_frwd,
				tpm_dir_t dir,
				tpm_db_pon_type_t pon_type,
				tpm_pncl_sram_data_t *sram_data)
{
	uint32_t i;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	if (SET_TARGET_PORT(rule_action->pkt_act)) {
		tpm_gmacs_enum_t act_wan= tpm_db_active_wan_get();

		/* Add Target Txp to update BM */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_TXP;

		/* Set PNC FlowId Target */
		if (TO_PON(dir, pkt_frwd->trg_port, pon_type, act_wan)) {
			for (i = 0; i < 8; i++) {
				if (pkt_frwd->trg_port == (uint32_t) (TPM_TRG_TCONT_0 << i))
					break;
			}
			sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_PMAC0 + i;
		} else if (TO_ETHWAN(dir, pkt_frwd->trg_port, act_wan)) {
			switch (act_wan) {
			case TPM_ENUM_GMAC_0:
				sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_GMAC0;
				break;
			case TPM_ENUM_GMAC_1:
				sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_GMAC1;
				break;
			default:
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Set Target, act_wan(%d) invalid",act_wan);
				return(TPM_FAIL);
			}
		} else if (TO_LAN(dir, pkt_frwd->trg_port)) {
			tpm_pnc_trg_t pnc_target;

			/* when ds load balance on G0 and G1 is enabled, trgt port can only
			 * be set in the first range TPM_DS_MAC_BASED_TRUNKING.
			 */
			tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
			if (TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable) {
				sram_data->sram_updt_bm &= (~TPM_PNCL_SET_TXP);
 			} else if (TPM_TRG_LOAD_BAL & pkt_frwd->trg_port) {
				/* DS load balance, set trg port to G1 */
				pnc_target = TPM_PNC_TRG_GMAC1;
			} else if (tpm_db_to_lan_gmac_get(pkt_frwd->trg_port, &pnc_target) != TPM_DB_OK){
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_db_to_lan_gmac_get failed trg_port 0x%x\n",
					     pkt_frwd->trg_port);
				return(TPM_FAIL);
       			}
			sram_data->flow_id_sub.pnc_target = pnc_target;
		} else if (TO_CPU(pkt_frwd->trg_port)) {
			sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
		} else {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Set Target received, no legitimate Target Found \n");
			return(TPM_FAIL);
		}
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Target(%d)\n", sram_data->flow_id_sub.pnc_target);
		/* If target is GPON, set GEM and MH */
		if (TO_GPON(dir, pkt_frwd->trg_port, pon_type, act_wan)) {
			sram_data->mh_reg.mh_set = TPM_TRUE;
			sram_data->mh_reg.mh_reg = 1;
			sram_data->sram_updt_bm |= TPM_PNCL_SET_GEM;
			sram_data->flow_id_sub.gem_port = pkt_frwd->gem_port;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set GemPort(%d)\n", pkt_frwd->gem_port);
		}
	}
	if (SET_TARGET_QUEUE(rule_action->pkt_act)) {
		/* Set Queue */
		sram_data->pnc_queue = pkt_frwd->trg_queue;

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Tx Queue (%x)\n", pkt_frwd->trg_queue);
	} else {
		sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "No Tx Queue Updat\n");
	}
	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_add_api_ent_check()
*
* DESCRIPTION:    The function checks if there is a free_entry to add an API entry
*
* INPUTS:
* section          - Section of the API acl/table
* api_rng_type     - The Type of the API, ACL or Table
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_add_api_ent_check(tpm_api_sections_t section, tpm_range_type_t api_rng_type, uint32_t rule_num)
{
	int32_t ret_code, last_valid;
	uint32_t api_rng_size, num_valid_entries, tbl_start;
	tpm_pnc_ranges_t prim_pnc_range;

	/* TODO - make the api_rng_type part of the api_rng database configuration */

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " api_sec(%d), api_rng_type(%d) \n", section, api_rng_type);

	/* Check API range exists */
	ret_code = tpm_db_api_section_get(section, &api_rng_size,
					  &num_valid_entries, &prim_pnc_range, &last_valid, &tbl_start);
	IF_ERROR(ret_code);

	/* Check possible to add another entry */
	if (num_valid_entries > (api_rng_size-1)) {
		/* If the range mode is table, it is unnecessary to check */
		if (api_rng_type == TPM_RANGE_TYPE_ACL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " api_sec(%d), has no free entries \n", section);
			return(TPM_FAIL);
		}
	}
	/* Check last_valid consistency, for acl type range */
	if (api_rng_type == TPM_RANGE_TYPE_ACL) {
		if (last_valid != ((int32_t) (num_valid_entries) - (int32_t) (1))) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "DB problem, api_section(%d),num_entries(%d),last_valid(%d)\n",
				     section, num_valid_entries, last_valid);
			return(TPM_FAIL);
		}

		/* make sure in api table there will not be a gap after insert this rule */
		if (rule_num > num_valid_entries) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "api table there will not be a gap after insert the api,"
				     "api_section(%d), num_entries(%d),last_valid(%d), rule_num(%d)\n",
				     section, num_valid_entries, last_valid, rule_num);
			return(TPM_FAIL);
		}
	}
	return(TPM_OK);
}


int32_t tpm_proc_check_api_busy(tpm_api_type_t api_type, uint32_t rule_num)
{
	uint32_t  db_ret_code;
	int32_t rc_code = TPM_RC_OK;
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t rangConf;


	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "api_type(%d) rule_num(%d)\n", api_type, rule_num);


	/* Get api_section, range_Id, range configuration, to get range type */
	db_ret_code = tpm_db_api_section_get_from_api_type(api_type, &api_section);
	IF_ERROR(db_ret_code);
	db_ret_code = tpm_db_api_section_main_pnc_get(api_section, &range_id);
	IF_ERROR(db_ret_code);
	db_ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(db_ret_code);


	/* Check API Section Busy */
	/* In ACL Mode   - concurrency not supported.
	   In TABLE Mode - concurrency supported, but not on the same entry */

	spin_lock_bh(&tpm_proc_api_call_lock);
	if ((rangConf.range_type == TPM_RANGE_TYPE_ACL) && ((tpm_db_get_api_busy(api_type))== TPM_TRUE))
		rc_code = ERR_API_BUSY;
	else if ((rangConf.range_type == TPM_RANGE_TYPE_TABLE) &&
		 ((tpm_db_get_api_rule_num_busy(api_type, rule_num)) == TPM_TRUE))
		rc_code = ERR_API_BUSY;
	else {
		db_ret_code = tpm_db_set_api_busy(api_type, rule_num);
		if (db_ret_code != TPM_DB_OK)
			rc_code = ERR_API_BUSY; /* Table full */
	}
	spin_unlock_bh(&tpm_proc_api_call_lock);
	return(rc_code);
}

int32_t tpm_proc_check_all_api_busy(void)
{
	uint32_t  db_ret_code;
	int32_t rc_code = TPM_RC_OK;
	int32_t rule_num;
	int32_t rule_num_max;
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_api_type_t api_type;

	for (api_type = TPM_API_MAC_LEARN; api_type < TPM_MAX_API_TYPES; api_type++) {

		/* Get api_section, range_Id, range configuration, to get range type */
		db_ret_code = tpm_db_api_section_get_from_api_type(api_type, &api_section);
		IF_ERROR(db_ret_code);
		db_ret_code = tpm_db_api_section_main_pnc_get(api_section, &range_id);
		if (TPM_DB_OK != db_ret_code)
			continue;
		db_ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
		IF_ERROR(db_ret_code);

		if (rangConf.range_type == TPM_RANGE_TYPE_ACL)
			rule_num_max = 1;
		else
			rule_num_max = TPM_MAX_PARALLEL_API_CALLS;

		for (rule_num = 0; rule_num < rule_num_max; rule_num++) {
			rc_code = tpm_proc_check_api_busy(api_type, rule_num);
				IF_ERROR(rc_code);
		}
	}

	return(TPM_OK);
}

int32_t tpm_proc_api_busy_done(tpm_api_type_t api_type, uint32_t rule_num)
{
	int32_t rc_code = TPM_RC_OK;
	uint32_t  db_ret_code;

	spin_lock_bh(&tpm_proc_api_call_lock);
	db_ret_code = tpm_db_set_api_free(api_type, rule_num);
	if (db_ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Error releasing api_busy: api_type(%d) rule_num(%d)\n", api_type, rule_num);
		rc_code = ERR_GENERAL;
	}
	spin_unlock_bh(&tpm_proc_api_call_lock);
	return(rc_code);
}

int32_t tpm_proc_all_api_busy_done(void)
{
	uint32_t  db_ret_code;
	int32_t rc_code = TPM_RC_OK;
	int32_t rule_num;
	int32_t rule_num_max;
	tpm_api_sections_t api_section;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_api_type_t api_type;

	for (api_type = TPM_API_MAC_LEARN; api_type < TPM_MAX_API_TYPES; api_type++) {

		/* Get api_section, range_Id, range configuration, to get range type */
		db_ret_code = tpm_db_api_section_get_from_api_type(api_type, &api_section);
		IF_ERROR(db_ret_code);
		db_ret_code = tpm_db_api_section_main_pnc_get(api_section, &range_id);
		if (TPM_DB_OK != db_ret_code)
			continue;
		db_ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
		IF_ERROR(db_ret_code);

		if (rangConf.range_type == TPM_RANGE_TYPE_ACL)
			rule_num_max = 1;
		else
			rule_num_max = TPM_MAX_PARALLEL_API_CALLS;

		for (rule_num = 0; rule_num < rule_num_max; rule_num++) {
			rc_code = tpm_proc_api_busy_done(api_type, rule_num);
			IF_ERROR(rc_code);
		}
	}

	return(TPM_OK);
}



int32_t tpm_proc_parse_flag_ai_tcam_build(tpm_ai_vectors_t *ai_fields,
					  uint32_t parse_flags,
					  long long parse_int_flags,
					  uint32_t *ai_data,
					  uint32_t *ai_mask)
{
	uint32_t ai_val;
	tpm_init_double_tag_t dbl_tag;
	tpm_src_port_type_t src_port_tmp;

	/* Parsing flags */

	/*BIT_0 */
	if (parse_flags & TPM_PARSE_FLAG_TAG1_MASK) {
		ai_val = ((parse_flags & TPM_PARSE_FLAG_TAG1_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_TAG1_BIT_OFF);
		*ai_mask |= TPM_AI_TAG1_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_NH2_ITER_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLAG_NH2_ITER_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_TAG1_BIT_OFF);
		*ai_mask |= TPM_AI_TAG1_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_IPV6_SUBFLOW_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv6 ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (((ai_fields->ipv6_subflow) << TPM_AI_IPV6_SUBFLOW_PART1_BIT_OFF) & TPM_AI_IPV6_SUBFLOW_PART1_MASK);
		*ai_mask |= TPM_AI_IPV6_SUBFLOW_PART1_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_IPV6_MC_SIP_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv6 ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (((ai_fields->ipv6_mc_sip_indx) << TPM_AI_IPV6_MC_SIP_PART1_BIT_OFF) & TPM_AI_IPV6_MC_SIP_PART1_MASK);
		*ai_mask |= TPM_AI_IPV6_MC_SIP_PART1_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_CNM_IPV4_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLAG_CNM_IPV4_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_CNM_IPV4_BIT_OFF);
		*ai_mask |= TPM_AI_CNM_IPV4_MASK;
	}

	/*BIT_1 */
	if (parse_flags & TPM_PARSE_FLAG_TAG2_MASK) {
		tpm_db_double_tag_support_get(&dbl_tag);
		if (TPM_DOUBLE_TAG_ENABLED == dbl_tag) {
			ai_val = ((parse_flags & TPM_PARSE_FLAG_TAG2_TRUE) ? 1 : 0);
			*ai_data |= (ai_val << TPM_AI_TAG2_BIT_OFF);
			*ai_mask |= TPM_AI_TAG2_MASK;
		} else {
			ai_val = ((parse_flags & TPM_PARSE_FLAG_TAG1_TRUE) ? 1 : 0);
			*ai_data |= (ai_val << TPM_AI_TAG1_BIT_OFF);
			*ai_mask |= TPM_AI_TAG1_MASK;
		}
	}

	if (parse_flags & TPM_PARSE_FLAG_PPPOE_MASK) {
		ai_val = ((parse_flags & TPM_PARSE_FLAG_PPPOE_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_PPPOE_BIT_OFF);
		*ai_mask |= TPM_AI_PPPOE_MASK;
	}

	if (parse_flags & TPM_PARSE_FLAG_L4P_MASK) {
		ai_val = ((parse_flags & TPM_PARSE_FLAG_L4_UDP) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_L4P_BIT_OFF);
		*ai_mask |= TPM_AI_L4P_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_IPV4_PRE_KEY_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv4 pre-filter key ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (ai_fields->ipv4_pre_key << TPM_AI_CNM_IPV4_PRE_KEY_BIT_OFF);
		*ai_mask |= (ai_fields->ipv4_pre_key << TPM_AI_CNM_IPV4_PRE_KEY_BIT_OFF);
	}

	if (parse_int_flags & TPM_PARSE_FLAG_CNM_PREC_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " cnm precedence ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (ai_fields->cnm_prec << TPM_AI_CNM_PREC_BIT_OFF);
		*ai_mask |= (ai_fields->cnm_prec << TPM_AI_CNM_PREC_BIT_OFF);
	}

	/*BIT_2 */
	if (parse_flags & TPM_PARSE_FLAG_MTM_MASK) {
		ai_val = ((parse_flags & TPM_PARSE_FLAG_MTM_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_MTM_BIT_OFF);
		*ai_mask |= TPM_AI_MTM_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_IPV6_MC_SIP_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv6 ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (((ai_fields->ipv6_mc_sip_indx >> (TPM_AI_IPV6_MC_SIP_PART1_BIT_OFF + 1))
			      << TPM_AI_IPV6_MC_SIP_PART2_BIT_OFF) & TPM_AI_IPV6_MC_SIP_PART2_MASK);
		*ai_mask |= TPM_AI_IPV6_MC_SIP_PART2_MASK;
	}

	/*BIT_3 */
	if (parse_flags & TPM_PARSE_FLAG_TO_CPU_MASK) {
		ai_val = ((parse_flags & TPM_PARSE_FLAG_TO_CPU_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_TO_CPU_BIT_OFF);
		*ai_mask |= TPM_AI_TO_CPU_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_SPLIT_MOD_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLGA_SPLIT_MOD_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_SPLIT_MOD_BIT_OFF);
		*ai_mask |= TPM_AI_SPLIT_MOD_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_DNRT_DS_TRUNK) {
		*ai_data &= ~(TPM_AI_DNRT_DS_TRUNK_MASK);
		*ai_mask |= TPM_AI_DNRT_DS_TRUNK_MASK;
	}

	/*BIT_4 */
	if (parse_int_flags & TPM_PARSE_FLAG_UNI_PORT_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " src_port ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}
		if (ai_fields->src_port == TPM_SRC_PORT_UNI_VIRT)
			src_port_tmp = TPM_SRC_PORT_UNI_7;
		else
			src_port_tmp = ai_fields->src_port;

		*ai_data |= ((src_port_tmp - TPM_SRC_PORT_UNI_0) << TPM_AI_UNI_BIT_OFF);
		*ai_mask |= TPM_AI_UNI_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_PPPOE_ADD_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLAG_PPPOE_ADD_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_PPPOE_ADD_BIT_OFF);
		*ai_mask |= TPM_AI_PPPOE_ADD_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_MC_VID_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " vid ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= ((ai_fields->mc_vid_entry) << TPM_AI_MC_VID_BIT_OFF);
		*ai_mask |= TPM_AI_MC_VID_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_IPV6_SUBFLOW_PARSE) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv6 ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (((ai_fields->ipv6_subflow >> TPM_AI_IPV6_SUBFLOW_PART1_BIT_SIZE) << TPM_AI_IPV6_SUBFLOW_PART2_BIT_OFF) & TPM_AI_IPV6_SUBFLOW_PART2_MASK);
		*ai_mask |= TPM_AI_IPV6_SUBFLOW_PART2_MASK;
	}

	/*BIT_5 */

	if (parse_int_flags & TPM_PARSE_FLAG_NO_PROTO_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLAG_NO_PROTO_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_NO_PROTO_BIT_OFF);
		*ai_mask |= TPM_AI_NO_PROTO_MASK;
	}

	/*BIT_6 */

	if (parse_int_flags & TPM_PARSE_FLAG_NO_FRAG_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLAG_NO_FRAG_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_NO_FRAG_BIT_OFF);
		*ai_mask |= TPM_AI_NO_FRAG_MASK;
	}

	if (parse_int_flags & TPM_PARSE_FLAG_MC_VID_MASK) {
		ai_val = ((parse_int_flags & TPM_PARSE_FLAG_MC_VID_TRUE) ? 1 : 0);
		*ai_data |= (ai_val << TPM_AI_MC_VID_VALID_BIT_OFF);
		*ai_mask |= TPM_AI_MC_VID_VALID_MASK;
	}

	return(TPM_OK);
}

int32_t tpm_proc_static_ai_sram_build(tpm_ai_vectors_t *ai_fields,
				      tpm_pkt_action_t pkt_action,
				      long long int_pkt_action,
				      uint32_t *ai_data,
				      uint32_t *ai_mask)
{
	tpm_src_port_type_t src_port_tmp;

	/* Important Note: All TPM_ACTION_UNSET_XXX must be done first, because other bits may re-raise them */
	if (int_pkt_action & TPM_ACTION_UNSET_DNRT) {
		*ai_data &= ~(TPM_AI_DNRT_MASK);
		*ai_mask |= TPM_AI_DNRT_MASK;
	}
	if (int_pkt_action & TPM_ACTION_UNSET_DNRT_DS_TRUNK) {
		*ai_data &= ~(TPM_AI_DNRT_DS_TRUNK_MASK);
		*ai_mask |= TPM_AI_DNRT_DS_TRUNK_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_UNI_PORT) {
		*ai_data &= ~(TPM_AI_UNI_MASK);
		*ai_mask |= TPM_AI_UNI_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_IPV6_SUBFLOW) {
		*ai_data &= ~TPM_AI_IPV6_SUBFLOW_PART1_MASK;
		*ai_mask |= TPM_AI_IPV6_SUBFLOW_PART1_MASK;

		*ai_data &= ~TPM_AI_IPV6_SUBFLOW_PART2_MASK;
		*ai_mask |= TPM_AI_IPV6_SUBFLOW_PART2_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_PPPOE) {
		*ai_data &= ~(TPM_AI_PPPOE_MASK);
		*ai_mask |= TPM_AI_PPPOE_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_NH2_ITER) {
		*ai_data &= ~(TPM_AI_NH2_ITER_MASK);
		*ai_mask |= TPM_AI_NH2_ITER_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_CNM_IPV4) {
		*ai_data &= ~(TPM_AI_CNM_IPV4_MASK);
		*ai_mask |= TPM_AI_CNM_IPV4_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_SPLIT_MOD) {
		*ai_data &= ~(TPM_AI_SPLIT_MOD_MASK);
		*ai_mask |= TPM_AI_SPLIT_MOD_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_IPV4_PRE_KEY) {
		*ai_data &= ~TPM_AI_CNM_IPV4_PRE_KEY_MASK;
		*ai_mask |= TPM_AI_CNM_IPV4_PRE_KEY_MASK;
	}

	if (int_pkt_action & TPM_ACTION_UNSET_CNM_PREC) {
		*ai_data &= ~TPM_AI_CNM_PREC_MASK;
		*ai_mask |= TPM_AI_CNM_PREC_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_TAG1) {
		*ai_data |= (1 << TPM_AI_TAG1_BIT_OFF);
		*ai_mask |= TPM_AI_TAG1_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_TAG2) {
		*ai_data |= (1 << TPM_AI_TAG1_BIT_OFF);
		*ai_mask |= TPM_AI_TAG2_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_PPPOE) {
		*ai_data |= (1 << TPM_AI_PPPOE_BIT_OFF);
		*ai_mask |= TPM_AI_PPPOE_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_L4P_TOG_UDP) {
		*ai_data |= (1 << TPM_AI_L4P_BIT_OFF);
		*ai_mask |= TPM_AI_L4P_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_L4P_TOG_TCP) {
		*ai_data |= (0 << TPM_AI_L4P_BIT_OFF);
		*ai_mask |= TPM_AI_L4P_MASK;
	}

	if (pkt_action & TPM_ACTION_MTM) {
		*ai_data |= (1 << TPM_AI_MTM_BIT_OFF);
		*ai_mask |= TPM_AI_MTM_MASK;
	}

	if (pkt_action & TPM_ACTION_TO_CPU) {
		*ai_data |= (1 << TPM_AI_TO_CPU_BIT_OFF);
		*ai_mask |= TPM_AI_TO_CPU_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_ADD_PPPOE) {
		*ai_data |= (1 << TPM_AI_PPPOE_ADD_BIT_OFF);
		*ai_mask |= TPM_AI_PPPOE_ADD_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_UNI_PORT) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " uni_ports ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}
		if (ai_fields->src_port == TPM_SRC_PORT_UNI_VIRT)
			src_port_tmp = TPM_SRC_PORT_UNI_7;
		else
			src_port_tmp = ai_fields->src_port;

		*ai_data |= ((src_port_tmp - TPM_SRC_PORT_UNI_0) << TPM_AI_UNI_BIT_OFF);
		*ai_mask |= TPM_AI_UNI_MASK;
	}

	if (pkt_action & TPM_ACTION_SPEC_MC_VID) {
		/* Set mc_valid bit */
		*ai_data |= (1 << TPM_AI_MC_VID_VALID_BIT_OFF);
		*ai_mask |= TPM_AI_MC_VID_VALID_MASK;

		/* Set mc_vid vector */
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " vid ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}
		*ai_data |= ((ai_fields->mc_vid_entry) << TPM_AI_MC_VID_BIT_OFF);
		*ai_mask |= TPM_AI_MC_VID_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_NO_PROTO_CHECK) {
		*ai_data |= (1 << TPM_AI_NO_PROTO_BIT_OFF);
		*ai_mask |= TPM_AI_NO_PROTO_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_NO_FRAG_CHECK) {
		*ai_data |= (1 << TPM_AI_NO_FRAG_BIT_OFF);
		*ai_mask |= TPM_AI_NO_FRAG_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_NH2_ITER) {
		*ai_data |= (1 << TPM_AI_NH2_ITER_BIT_OFF);
		*ai_mask |= TPM_AI_NH2_ITER_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_IPV6_SUBFLOW) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv6 ai vector cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= (((ai_fields->ipv6_subflow) << TPM_AI_IPV6_SUBFLOW_PART1_BIT_OFF) & TPM_AI_IPV6_SUBFLOW_PART1_MASK);
		*ai_mask |= TPM_AI_IPV6_SUBFLOW_PART1_MASK;

		*ai_data |= (((ai_fields->ipv6_subflow >> TPM_AI_IPV6_SUBFLOW_PART1_BIT_SIZE) << TPM_AI_IPV6_SUBFLOW_PART2_BIT_OFF) & TPM_AI_IPV6_SUBFLOW_PART2_MASK);
		*ai_mask |= TPM_AI_IPV6_SUBFLOW_PART2_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_CNM_IPV4) {
		*ai_data |= (1 << TPM_AI_CNM_IPV4_BIT_OFF);
		*ai_mask |= TPM_AI_CNM_IPV4_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_SPLIT_MOD) {
		*ai_data |= (1 << TPM_AI_SPLIT_MOD_BIT_OFF);
		*ai_mask |= TPM_AI_SPLIT_MOD_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_IPV4_PRE_KEY) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " ipv4 pre filter key pattern cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= ((ai_fields->ipv4_pre_key << TPM_AI_CNM_IPV4_PRE_KEY_BIT_OFF) & TPM_AI_CNM_IPV4_PRE_KEY_MASK);
		*ai_mask |= TPM_AI_CNM_IPV4_PRE_KEY_MASK;
	}

	if (int_pkt_action & TPM_ACTION_SET_CNM_PREC) {
		if (ai_fields == NULL) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " cnm precedence pattern cannot be null pointer\n");
			return(TPM_FAIL);
		}

		*ai_data |= ((ai_fields->cnm_prec << TPM_AI_CNM_PREC_BIT_OFF) & TPM_AI_CNM_PREC_MASK);
		*ai_mask |= TPM_AI_CNM_PREC_MASK;
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_virt_uni_trg_port_validation()
*
* DESCRIPTION:
*   In case feature Virtual UNI_4 is enabled - only following target port are allowed:
*       TPM_TRG_UNI_0                       (0x0100)
*       TPM_TRG_UNI_1                       (0x0200)
*       TPM_TRG_UNI_2                       (0x0400)
*       TPM_TRG_UNI_3                       (0x0800)
*       TPM_TRG_PORT_UNI_ANY                (0x4000)
* INPUTS:
* tpm_trg_port_type_t     trg_port
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_virt_uni_trg_port_validation(tpm_trg_port_type_t trg_port)
{
	/* unset TPM_TRG_LOAD_BAL first, since it can be mixed with UNI port */
	trg_port &= (~TPM_TRG_LOAD_BAL);

	if ((trg_port == TPM_TRG_UNI_0) ||
	    (trg_port == TPM_TRG_UNI_1) ||
	    (trg_port == TPM_TRG_UNI_2) ||
	    (trg_port == TPM_TRG_UNI_3) ||
	    (trg_port == TPM_TRG_UNI_4) ||
	    (trg_port == TPM_TRG_UNI_5) ||
	    (trg_port == TPM_TRG_UNI_6) ||
	    (trg_port == TPM_TRG_UNI_7) ||
	    (trg_port == TPM_TRG_UNI_VIRT) ||
	    (trg_port == TPM_TRG_PORT_UNI_ANY) ||
	    (trg_port == TPM_TRG_PORT_CPU) ||
	    (trg_port == (TPM_TRG_UNI_0 | TPM_TRG_UNI_1 | TPM_TRG_UNI_2 | TPM_TRG_UNI_3 |
			  TPM_TRG_UNI_4 | TPM_TRG_UNI_5 | TPM_TRG_UNI_6 | TPM_TRG_UNI_7)))
		return TPM_OK;
	else
		return TPM_FAIL;
}

/*******************************************************************************
* tpm_proc_set_RI_mh()
*
* DESCRIPTION:
*
* INPUTS:
* rule_action
* pkt_frwd
* dir
* sram_data
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_set_RI_mh(tpm_rule_action_t *rule_action,
			   tpm_pkt_frwd_t *pkt_frwd,
			   tpm_dir_t dir,
			   tpm_pncl_sram_data_t *sram_data)
{
	uint32_t i;
	int32_t ret_code;
	tpm_db_mh_src_t ds_mh_src;
	tpm_init_virt_uni_t virt_uni_info;
	uint16_t amber_port_vector;
	uint32_t pnc_vector, uni_vector;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "tpm_proc_set_RI_mh: dir(%d), trg_port(0x%x) \r\n", dir, pkt_frwd->trg_port);

	tpm_db_ds_mh_get_conf_set(&ds_mh_src);

	if ((SET_TARGET_PORT(rule_action->pkt_act)) &&
	    (TO_LAN(dir, pkt_frwd->trg_port)) && (ds_mh_src == TPM_MH_SRC_PNC_RI)) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_MH_RI;
		sram_data->mh_reg.mh_set = TPM_TRUE;

		/* target port validation -
		   not all target port combinations are supported when WiFi via GMAC1 = UNI_4 feature is enabled */
		ret_code = tpm_db_virt_info_get(&virt_uni_info);
		if (ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Virt UNI recvd ret_code(%d)\n", ret_code);
			return(ret_code);
		}
		if ((virt_uni_info.enabled == 1) && (dir == TPM_DIR_DS)) {
			/* Virt UNI feature is enabled - validate and recalculate the mh_reg */
			ret_code = tpm_proc_virt_uni_trg_port_validation(pkt_frwd->trg_port);
			if (ret_code != TPM_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Virt UNI trg_port validation failed. (%d)\n", ret_code);
				return(ret_code);
			}

			/* get the MH_REG from the DB */
			ret_code = tpm_db_port_vector_tbl_info_search(pkt_frwd->trg_port, &uni_vector,
									   &amber_port_vector, &pnc_vector);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD,
					" Unable to retrieve port vector table from DB. (0x%x)\n", pkt_frwd->trg_port);
				return(ret_code);
			}
			sram_data->mh_reg.mh_reg = pnc_vector;
		} else {
			for (i = 0; i < TPM_MAX_NUM_UNI_PORTS; i++) {
				if ((pkt_frwd->trg_port & (TPM_TRG_UNI_0 << i)) ||
				    (pkt_frwd->trg_port & TPM_TRG_PORT_UNI_ANY))
					sram_data->mh_reg.mh_reg |= (TPM_MH_RI_BIT14 << i);
			}

		}
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_owner_id_check()
*
* DESCRIPTION:    The function checks if the owner_id is the valid owner of api_type
*
* INPUTS:
* owner_id         - API Group owner
* api_type        - API group the owner requests to act upon
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_owner_id_check(tpm_api_type_t api_type, uint32_t owner_id)
{
	/* TODO Implement */
	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_add_cpu_loopback_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_add_cpu_loopback_rule params.
*
* INPUTS:
* owner_id         - See tpm_proc_add_l2_prim_acl_rule
* rule_num         - See tpm_proc_add_l2_prim_acl_rule
* pkt_frwd         - See tpm_proc_add_l2_prim_acl_rule
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_cpu_loopback_check(uint32_t owner_id, uint32_t rule_num, tpm_pkt_frwd_t *pkt_frwd)
{
	int32_t ret_code;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_CPU_LOOPBACK, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_CPU_LOOPBACK_ACL, TPM_RANGE_TYPE_ACL, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_add_l2_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_add_l2_prim_acl_rule params.
*
* INPUTS:
* owner_id         - See tpm_proc_add_l2_prim_acl_rule
* src_port         - See tpm_proc_add_l2_prim_acl_rule
* rule_num         - See tpm_proc_add_l2_prim_acl_rule
* parse_rule_bm    - See tpm_proc_add_l2_prim_acl_rule
* l2_key           - See tpm_proc_add_l2_prim_acl_rule
* pkt_frwd         - See tpm_proc_add_l2_prim_acl_rule
* pkt_mod          - See tpm_proc_add_l2_prim_acl_rule
* rule_action      - See tpm_proc_add_l2_prim_acl_rule
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_l2_check(uint32_t owner_id,
				       tpm_src_port_type_t src_port,
				       uint32_t rule_num,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_l2_acl_key_t *l2_key,
				       tpm_parse_flags_t parse_flags_bm,
				       tpm_pkt_frwd_t *pkt_frwd,
				       tpm_pkt_mod_bm_t pkt_mod_bm,
				       tpm_pkt_mod_t *pkt_mod,
				       tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_dir_t dir;
	tpm_db_pon_type_t pon_type;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pnc_range_conf_t rangConf;
	int32_t mc_vlan_free_slot;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Range_Id, rang configuration, to get range type */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_L2_PRIM_ACL, &range_id);
	IF_ERROR(ret_code);
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Get Direction, PON type, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);
	tpm_db_pon_type_get(&pon_type);

	/* Check necessary pointers are valid */
	ret_code =
	tpm_proc_check_missing_data(rule_action, pkt_mod, pkt_frwd, (void *)l2_key, rule_action->pkt_act,
				    parse_rule_bm);
	IF_ERROR(ret_code);

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_L2_PRIM_ACL_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check Vlan Tag TPID mask */
	if (parse_rule_bm & (TPM_L2_PARSE_TWO_VLAN_TAG | TPM_L2_PARSE_ONE_VLAN_TAG)) {
		if ((l2_key->vlan1.tpid_mask != 0) && (l2_key->vlan1.tpid_mask != 0xffff)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid vlan1 tpid mask(0x%x) \n", l2_key->vlan1.tpid_mask);
			return(ERR_L2_KEY_INVALID);
		}

		if (parse_rule_bm & TPM_L2_PARSE_ONE_VLAN_TAG) {
			if ((l2_key->vlan2.tpid_mask != 0) && (l2_key->vlan2.tpid_mask != 0xffff)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid vlan2 tpid mask(0x%x) \n", l2_key->vlan2.tpid_mask);
				return(ERR_L2_KEY_INVALID);
			}
		}
	}

	/* Cannot do Double Vlan Tag with looking into PPPoE (up to 24Bytes) with MH */
	if ((parse_rule_bm & TPM_L2_PARSE_TWO_VLAN_TAG) &&
	    ((parse_rule_bm & TPM_L2_PARSE_PPP_PROT) || (parse_rule_bm & TPM_L2_PARSE_PPPOE_SES))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parse map of Double Vlan Tag + PPPoE not supported\n");
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Cannot do Single or Double Vlan Tag with checking   PPP protocol (up to 24Bytes) with MH */
	if (((parse_rule_bm & TPM_L2_PARSE_TWO_VLAN_TAG) || (parse_rule_bm & TPM_L2_PARSE_ONE_VLAN_TAG))
	    && (parse_rule_bm & TPM_L2_PARSE_PPP_PROT)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parse map of Single Vlan Tag + PPPoE proto not supported\n");
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_L2_PRIM_ACL_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/*do not allow user to create VLANOP_NOOP l2 PNC with no vlan parse flag.*/
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(TPM_VLAN_MOD == pkt_mod_bm) &&
		(VLANOP_NOOP == pkt_mod->vlan_mod.vlan_op)) {
		if (!(parse_flags_bm &
			(TPM_PARSE_FLAG_TAG1_TRUE |
			TPM_PARSE_FLAG_TAG2_TRUE |
			TPM_PARSE_FLAG_TAG1_FALSE |
			TPM_PARSE_FLAG_TAG2_FALSE))) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x), "
				"when split mod VLANOP_NOOP it must TAG FLAG be set\n", parse_flags_bm);
			return(ERR_PARSE_MAP_INVALID);
		}
	}

	/* check VLAN OP p_bit mask */
	if (pkt_mod_bm & TPM_VLAN_MOD) {
		if (VLANOP_EXT_TAG_INS == pkt_mod->vlan_mod.vlan_op) {
			if (pkt_mod->vlan_mod.vlan1_out.pbit_mask != 0xff) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid p_bit mask for VLAN Op (0x%x) \n", VLANOP_EXT_TAG_INS);
				return(ERR_GENERAL);
		    }
		}
		if (VLANOP_INS_2TAG == pkt_mod->vlan_mod.vlan_op) {
			if (pkt_mod->vlan_mod.vlan1_out.pbit_mask != 0xff ||
			    pkt_mod->vlan_mod.vlan2_out.pbit_mask != 0xff) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid p_bit mask for VLAN Op (0x%x) \n", VLANOP_INS_2TAG);
				return(ERR_GENERAL);
		    }
		}
		if (VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid VLAN Op (0x%x), do not support in L2 \n", VLANOP_SPLIT_MOD_PBIT);
			return(ERR_GENERAL);
		}
	}

	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check Packet Modification */
#if 0
	tpm_proc_pkt_mod_check(rule_action->pkt_act, pkt_mod_bm, pkt_mod);
	IF_ERROR(ret_code);
#endif

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_L2_PRIM, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_L2_PRIM_ACL, rangConf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	/* Check parse_rules */

	/* Check gem_port only for GPON DS */
	if ((parse_rule_bm & TPM_L2_PARSE_GEMPORT) && ((dir != TPM_DIR_DS) || (pon_type != TPM_GPON)))
		IF_ERROR(ERR_PARSE_MAP_INVALID);

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_L3_TYPE && rule_action->next_phase != STAGE_DONE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}
	/* Check that for l2_hwf rules (not mtm, not to_cpu), if the parsing is not done, the uni_port is specifc.
	 * Otherwise, rules in following sections cannot rely on the uni_port */
	if (((rule_action->pkt_act & (TPM_ACTION_TO_CPU | TPM_ACTION_MTM)) == 0) &&
	    (rule_action->next_phase != STAGE_DONE)) {
		if (!(FROM_SPEC_UNI(src_port)) && (src_port != TPM_SRC_PORT_WAN)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD,
				     " For L2_HWF rule (not mtm, not to_cpu), specific uni must be specified \n");
			return(ERR_SRC_PORT_INVALID);
		}
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);

	/* Check mc vlan set */
	if (TPM_ACTION_SPEC_MC_VID & rule_action->pkt_act) {
		if (src_port != TPM_SRC_PORT_WAN) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Multicast Vlan-ID can only be assigned in downstream \n");
			return(ERR_ACTION_INVALID);
		}
		if (0 == l2_key->vlan1.vid_mask) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " To spec MC vlan, Vlan-ID must be specified \n");
			return(ERR_ACTION_INVALID);
		} else if (l2_key->vlan1.vid == 0 || l2_key->vlan1.vid >= 4096) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Invalid multicast Vlan-ID is assigned \n");
			return(ERR_L2_KEY_INVALID);
		}
		mc_vlan_free_slot = tpm_db_mc_vlan_get_pnc_index_free_slot(l2_key->vlan1.vid, rule_num);
		if (0 == mc_vlan_free_slot) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " No more free slot for l2 rule of this MC vlan\n");
			return(ERR_GENERAL);
		}
	}

	return(TPM_RC_OK);
}
/*******************************************************************************
* tpm_proc_add_ds_load_balance_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_add_ds_load_balance_acl_rule params.
*
* INPUTS:
* owner_id         - See tpm_proc_add_ds_load_balance_acl_rule
* rule_num         - See tpm_proc_add_ds_load_balance_acl_rule
* parse_rule_bm    - See tpm_proc_add_ds_load_balance_acl_rule
* l2_key           - See tpm_proc_add_ds_load_balance_acl_rule
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_ds_load_balance_check(uint32_t owner_id,
						uint32_t rule_num,
						tpm_parse_fields_t parse_rule_bm,
						tpm_parse_flags_t parse_flags_bm,
						tpm_l2_acl_key_t *l2_key)
{
	int32_t ret_code;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pnc_range_conf_t rangConf;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Get Range_Id, rang configuration, to get range type */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_DS_LOAD_BALANCE_ACL, &range_id);
	IF_ERROR(ret_code);
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Check necessary pointers are valid */
	if ((l2_key == NULL) && (parse_rule_bm != 0)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parsing requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}
	IF_ERROR(ret_code);

	/* Check parse_rule_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_DS_LOAD_BALANCE_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flag_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_DS_LOAD_BALANCE_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm(0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check Vlan Tag TPID mask */
	if (parse_rule_bm & (TPM_L2_PARSE_TWO_VLAN_TAG | TPM_L2_PARSE_ONE_VLAN_TAG)) {
		if ((l2_key->vlan1.tpid_mask != 0) && (l2_key->vlan1.tpid_mask != 0xffff)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid vlan1 tpid mask(0x%x) \n", l2_key->vlan1.tpid_mask);
			return(ERR_L2_KEY_INVALID);
		}

		if (parse_rule_bm & TPM_L2_PARSE_ONE_VLAN_TAG) {
			if ((l2_key->vlan2.tpid_mask != 0) && (l2_key->vlan2.tpid_mask != 0xffff)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid vlan2 tpid mask(0x%x) \n", l2_key->vlan2.tpid_mask);
				return(ERR_L2_KEY_INVALID);
			}
		}
	}

	/* Cannot do Double Vlan Tag with looking into PPPoE (up to 24Bytes) with MH */
	if ((parse_rule_bm & TPM_L2_PARSE_TWO_VLAN_TAG) &&
	    ((parse_rule_bm & TPM_L2_PARSE_PPP_PROT) || (parse_rule_bm & TPM_L2_PARSE_PPPOE_SES))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parse map of Double Vlan Tag + PPPoE not supported\n");
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Cannot do Single or Double Vlan Tag with checking   PPP protocol (up to 24Bytes) with MH */
	if (((parse_rule_bm & TPM_L2_PARSE_TWO_VLAN_TAG) || (parse_rule_bm & TPM_L2_PARSE_ONE_VLAN_TAG))
	    && (parse_rule_bm & TPM_L2_PARSE_PPP_PROT)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parse map of Single Vlan Tag + PPPoE proto not supported\n");
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_DS_LOAD_BALANCE, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_DS_LOAD_BALANCE_ACL, rangConf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	return(TPM_RC_OK);
}


tpm_error_code_t tpm_proc_add_ipv6_gen_check(uint32_t owner_id,
					       tpm_src_port_type_t src_port,
					       uint32_t rule_num,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_pkt_mod_bm_t pkt_mod_bm,
					       tpm_pkt_mod_t *pkt_mod,
					       tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_dir_t dir;
	tpm_db_pon_type_t pon_type;
	tpm_db_pnc_range_t range_data;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check 5_tuple feature is disabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable != TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Direction, PON type, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);
	tpm_db_pon_type_get(&pon_type);

	/* Check necessary pointers are valid */
	ret_code =
	tpm_proc_check_missing_data(rule_action, pkt_mod, pkt_frwd, (void *)ipv6_gen_key, rule_action->pkt_act,
				    parse_rule_bm);
	IF_ERROR(ret_code);

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV6_GEN_ACL_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);

	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV6_GEN_ACL_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check Packet Modification */
#if 0
	tpm_proc_pkt_mod_check(rule_action->pkt_act, pkt_mod_bm, pkt_mod);
	IF_ERROR(ret_code);
#endif

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV6_GEN, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_GEN, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_IPV6_GEN_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		if ((rule_num < range_data.pnc_range_conf.api_start) || (rule_num > range_data.pnc_range_conf.api_end))
			IF_ERROR(ERR_RULE_NUM_INVALID);
	}

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_IPv6_DIP && rule_action->next_phase != STAGE_DONE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_GEN_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_dip_check(uint32_t owner_id,
					     tpm_src_port_type_t src_port,
					     uint32_t rule_num,
					     tpm_parse_fields_t parse_rule_bm,
					     tpm_parse_flags_t parse_flags_bm,
					     tpm_ipv6_addr_key_t *ipv6_dip_key,
					     tpm_pkt_frwd_t *pkt_frwd,
					     tpm_pkt_mod_bm_t pkt_mod_bm,
					     tpm_pkt_mod_t *pkt_mod,
					     tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_dir_t dir;
	tpm_db_pon_type_t pon_type;
	tpm_db_pnc_range_t range_data;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);


	/* Check 5_tuple feature is disabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable != TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Direction, PON type, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);
	tpm_db_pon_type_get(&pon_type);

	/* Check necessary pointers are valid */
	ret_code =
	tpm_proc_check_missing_data(rule_action, pkt_mod, pkt_frwd, (void *)ipv6_dip_key, rule_action->pkt_act,
				    parse_rule_bm);
	IF_ERROR(ret_code);

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_rule_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV6_DIP_ACL_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);

	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV6_DIP_ACL_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check Packet Modification */
#if 0
	tpm_proc_pkt_mod_check(rule_action->pkt_act, pkt_mod_bm, pkt_mod);
	IF_ERROR(ret_code);
#endif

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV6_DIP, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_DIP, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_IPV6_DIP_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		if ((rule_num < range_data.pnc_range_conf.api_start) || (rule_num > range_data.pnc_range_conf.api_end))
			IF_ERROR(ERR_RULE_NUM_INVALID);
	}

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_DONE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_DIP_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_l4ports_check(uint32_t owner_id,
						 tpm_src_port_type_t src_port,
						 uint32_t rule_num,
						 tpm_parse_fields_t parse_rule_bm,
						 tpm_parse_flags_t parse_flags_bm,
						 tpm_l4_ports_key_t *l4_key,
						 tpm_pkt_frwd_t *pkt_frwd,
						 tpm_pkt_mod_bm_t pkt_mod_bm,
						 tpm_pkt_mod_t *pkt_mod,
						 tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_dir_t dir;
	tpm_db_pon_type_t pon_type;
	tpm_db_pnc_range_t range_data;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check 5_tuple feature is disabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable != TPM_IPV6_5T_DISABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Direction, PON type, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);
	tpm_db_pon_type_get(&pon_type);

	/* Check necessary pointers are valid */
	ret_code =
	tpm_proc_check_missing_data(rule_action, pkt_mod, pkt_frwd, (void *)l4_key, rule_action->pkt_act,
				    parse_rule_bm);
	IF_ERROR(ret_code);

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_rule_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV6_L4_PORTS_ACL_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV6_L4_PORTS_ACL_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check Packet Modification */
#if 0
	tpm_proc_pkt_mod_check(rule_action->pkt_act, pkt_mod_bm, pkt_mod);
	IF_ERROR(ret_code);
#endif

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV6_L4, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_L4, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_L4_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		if ((rule_num < range_data.pnc_range_conf.api_start) || (rule_num > range_data.pnc_range_conf.api_end))
			IF_ERROR(ERR_RULE_NUM_INVALID);
	}

	/* Check forwarding rule,  STAGE_DONE */
	if (rule_action->next_phase != STAGE_IPv6_GEN && rule_action->next_phase != STAGE_DONE
	    && rule_action->next_phase != STAGE_CTC_CM) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_L4_PORTS_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	/* Check if next stage CTC CnM  */
	if ((rule_action->next_phase == STAGE_CTC_CM) && !(parse_flags_bm & TPM_PARSE_FLAG_MTM_FALSE)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CnM, packets must be MTM_FALSE\n");
		return(ERR_NEXT_PHASE_INVALID);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_nh_check(uint32_t owner_id,
					    uint32_t rule_num,
					    tpm_nh_iter_t nh_iter,
					    tpm_parse_flags_t parse_flags_bm,
					    uint32_t nh,
					    tpm_pkt_frwd_t *pkt_frwd,
					    tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_db_pnc_range_t range_data;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm = 0;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check keys exist for parse fields */
	if (rule_action == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "rule_action cannot be NULL\n");
		return(ERR_ACTION_INVALID);
	}

	if ((pkt_frwd == NULL) && ((SET_TARGET_PORT(rule_action->pkt_act)) || SET_TARGET_QUEUE(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target set requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	/* Check Valid Target, ==> only allowed to direct to CPU */
	if (SET_TARGET_PORT(rule_action->pkt_act)) {
		if (TO_CPU(pkt_frwd->trg_port)) {
			/*cont */
		} else {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "* trg_port=%d *\r\n", pkt_frwd->trg_port);
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Illegal Target Port\n");
			return(ERR_FRWD_INVALID);
		}
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check Valid Queue */
	/* TODO - Check Queue depending on actual queues in target or in Rx */
	if (SET_TARGET_QUEUE(rule_action->pkt_act) && (pkt_frwd->trg_queue >= TPM_MAX_NUM_TX_QUEUE)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target Queue Out of Range\n");
		return(ERR_FRWD_INVALID);
	}

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_NH, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_IPV6_NH_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		if ((rule_num < range_data.pnc_range_conf.api_start) || (rule_num > range_data.pnc_range_conf.api_end))
			IF_ERROR(ERR_RULE_NUM_INVALID);
	}

	/* Check forwarding rule */
	if (rule_action->next_phase != STAGE_IPV6_L4 &&
		rule_action->next_phase != STAGE_IPv6_NH &&
		rule_action->next_phase != STAGE_IPv6_GEN &&
		rule_action->next_phase != STAGE_CTC_CM &&
		rule_action->next_phase != STAGE_DONE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	if (rule_action->next_phase == STAGE_IPV6_L4) {
		if (nh != IPPROTO_UDP && nh != IPPROTO_TCP) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase is IPV6_L4 while NH(%d) is not UDP or TCP \n", nh);
			return(ERR_NEXT_PHASE_INVALID);
		}
	}

	if (rule_action->next_phase == STAGE_IPv6_NH && nh_iter == NH_ITER_1) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Multiple Ext Hdr is not supported \n");
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check if next stage CTC CnM  */
	if (rule_action->next_phase == STAGE_CTC_CM) {
		if (nh == IPPROTO_UDP || nh == IPPROTO_TCP) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next Phase is CTC_CM while NH(%d) is UDP or TCP \n", nh);
			return(ERR_NEXT_PHASE_INVALID);
		}
		if (0 == (parse_flags_bm & TPM_PARSE_FLAG_MTM_FALSE)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage is CnM, packets must be MTM_FALSE\n");
			return(ERR_NEXT_PHASE_INVALID);
		}
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, &pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_NH_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	/* when ds load balance on G0 and G1 is enabled, no 2 NH rule can be added */
	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
	if (	(TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable)
	     && (NH_ITER_1 == nh_iter)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "when ds load balance on G0 and G1 is enabled, "
			"no 2 NH rule can be added\n");
		return(ERR_FEAT_UNSUPPORT);
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_add_ipv4_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_add_ipv4_acl_rule params.
*
* INPUTS:
* - See tpm_proc_add_ipv4_acl_rule
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_ipv4_check(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t rule_num,
					 tpm_parse_fields_t parse_rule_bm,
					 tpm_parse_flags_t parse_flags_bm,
					 tpm_ipv4_acl_key_t *ipv4_key,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_pkt_mod_t *pkt_mod,
					 tpm_rule_action_t *rule_action,
					 tpm_pkt_mod_bm_t pkt_mod_bm)
{
	int32_t ret_code;
	tpm_dir_t dir;
	tpm_db_pon_type_t pon_type;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pnc_range_conf_t rangConf;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Direction, PON type, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id, rang configuration, to get range type */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_IPV4_ACL, &range_id);
	IF_ERROR(ret_code);
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Check necessary pointers are valid */
	ret_code =
	tpm_proc_check_missing_data(rule_action, pkt_mod, pkt_frwd, (void *)ipv4_key, rule_action->pkt_act,
				    parse_rule_bm);
	IF_ERROR(ret_code);

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_TRUE);
	IF_ERROR(ret_code);

	/* Check Packet Modification */
#if 0
	tpm_proc_pkt_mod_check(rule_action->pkt_act, pkt_mod_bm, pkt_mod);
	IF_ERROR(ret_code);
#endif

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV4_ACL_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV4_ACL_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV4, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_IPV4_ACL, rangConf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	/* Check forwarding rule, support STAGE_DONE */
	if (    rule_action->next_phase != STAGE_DONE
		 && rule_action->next_phase != STAGE_CTC_CM) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV4_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	if(rule_action->next_phase == STAGE_CTC_CM) {
		if(!tpm_ctc_cm_ipv4_rule2cm(parse_flags_bm, rule_action)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CTC_CM, parse flag must be MTM_FALSE, and action can not be TO_CPU\n");
			return(ERR_NEXT_PHASE_INVALID);
		}
		if(FROM_WAN(src_port)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CTC_CM, Src Port can not be WAN\n");
			return(ERR_SRC_PORT_INVALID);
		}
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_del_l2_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_del_l2_prim_acl_rule params.
*
* INPUTS:
* owner_id         - See tpm_proc_del_l2_prim_acl_rule
* src_port         - See tpm_proc_del_l2_prim_acl_rule
* rule_idx         - See tpm_proc_del_l2_prim_acl_rule
* parse_rule_bm    - See tpm_proc_del_l2_prim_acl_rule
* l2_key           - See tpm_proc_del_l2_prim_acl_rule
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_l2_check(uint32_t owner_id,
				       tpm_src_port_type_t src_port,
				       uint32_t rule_idx,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_l2_acl_key_t *l2_key)
{
	int32_t ret_code;
	uint32_t rule_num;
	tpm_dir_t dir;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* check that rule_idx or parse_bm or l2_key are valid - for deletion */
	if ((rule_idx == 0) && ((l2_key == NULL) || (parse_rule_bm == 0)))
		IF_ERROR(ERR_DELETE_KEY_INVALID);
	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_L2_PRIM, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Direction, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Check valid rule_idx */

	if (rule_idx != 0) {	/* when rule_idx = 0 -> caller indicates to work according to the l2_key */

		ret_code = tpm_db_api_rulenum_get(TPM_L2_PRIM_ACL, rule_idx, &rule_num);
		if (ret_code != TPM_OK)
			IF_ERROR(ERR_RULE_IDX_INVALID);
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_common_pncl_info_get()
*
* DESCRIPTION:
*
* INPUTS:
* api_section      - Section of the API acl/table
* dir              - Direction of the API acl/table
* api_rule_num     - Rulenum in the API table
*
* OUTPUTS:
* pnc_entry        - PnC Entry matching this api_table/rule_num
* lu_id            - LookupId of this PNC Range
* start_offset     - Logical TCAM start offset of this API Table
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_common_pncl_info_get(tpm_pnc_ranges_t range_id, uint32_t *lu_id, tpm_pncl_offset_t *start_offset)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "range_id(%d)\n", range_id);

	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	*lu_id = range_conf.base_lu_id;
	start_offset->range_id = range_id;

	if (    range_id == TPM_PNC_L2_MAIN || range_id == TPM_PNC_MAC_LEARN
	     || range_id == TPM_PNC_DS_LOAD_BALANCE) {
		start_offset->offset_base = TPM_PNCL_ZERO_OFFSET;
		start_offset->offset_sub.subf = TPM_L2_PARSE_MH;
	} else if (range_id == TPM_PNC_ETH_TYPE) {
		start_offset->offset_base = TPM_PNCL_L3_OFFSET;
		start_offset->offset_sub.subf = TPM_L2_PARSE_ETYPE;
	} else if ((range_id == TPM_PNC_IPV4_MAIN)
		   || (range_id == TPM_PNC_IPV4_MC_DS)
		   || (range_id == TPM_PNC_IPV4_PROTO)
		   || (range_id == TPM_PNC_CNM_IPV4_PRE)
		   || (range_id == TPM_PNC_IGMP)) {
		start_offset->offset_base = TPM_PNCL_IPV4_OFFSET;
		start_offset->offset_sub.subf = TPM_IPv4_PARSE_VER_OR_IHL;
	} else if (    (range_id == TPM_PNC_IPV6_NH)
		    || (range_id == TPM_PNC_IPV6_GEN)
		    || (range_id == TPM_PNC_IPV6_MC_SIP)) {
		start_offset->offset_base = TPM_PNCL_IPV6_OFFSET;
		start_offset->offset_sub.subf = TPM_IPv6_PARSE_VER_OR_DSCP;
	} else if ((range_id == TPM_PNC_IPV6_DIP)
		   || (range_id == TPM_PNC_IPV6_MC_DS)) {
		start_offset->offset_base = TPM_PNCL_IPV6_OFFSET;
		start_offset->offset_sub.subf = TPM_IPv6_PARSE_DIP;
	} else if (range_id == TPM_PNC_IPV6_L4) {
		start_offset->offset_base = TPM_PNCL_L4_OFFSET;
		start_offset->offset_sub.subf = TPM_PARSE_L4_SRC;
	} else if (range_id == TPM_PNC_CNM_MAIN) {
		/* Dummy */
	} else {
		TPM_OS_ERROR(TPM_PNCL_MOD, "range id not supported %d\n", range_id);
		return(TPM_FAIL);
	}
	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_pnc_con_del()
*
* DESCRIPTION:     Function deletes a pnc connection structure
*
* INPUTS:
* pnc_con          - APIPNC Table  Configuration
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_pnc_con_del(tpm_db_pnc_conn_t *pnc_con)
{
	uint32_t i, cur_pnc_range, cur_pnc_index, pnc_start, pnc_stop;
	int32_t ret_code;
	tpm_db_pnc_range_t range_data;

	for (i = 0; i < (pnc_con->num_pnc_ranges); i++) {
		cur_pnc_index = pnc_con->pnc_conn_tbl[i].pnc_index;
		cur_pnc_range = pnc_con->pnc_conn_tbl[i].pnc_range;

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "range(%d), ind(%d) \n", cur_pnc_range, cur_pnc_index);

		ret_code = tpm_db_pnc_rng_get(cur_pnc_range, &range_data);
		IF_ERROR(ret_code);

		if (range_data.pnc_range_conf.range_type == TPM_RANGE_TYPE_ACL) {
			pnc_start = cur_pnc_index;
			/* Pull range from this index untill last used entry in Pnc range */
			pnc_stop = range_data.pnc_range_conf.range_start +
				   (range_data.pnc_range_conf.api_end - range_data.pnc_range_oper.free_entries);

			ret_code = tpm_pncl_entry_delete(pnc_start, pnc_stop);
			IF_ERROR(ret_code);
		} else {
			ret_code = tpm_pncl_entry_reset(cur_pnc_index);
			IF_ERROR(ret_code);
		}
		/* Increase number of free entries in pnc_range */
		ret_code = tpm_db_pnc_rng_free_ent_inc(cur_pnc_range);
		IF_ERROR(ret_code);
	}
	/* Init the pnc_con structure */
	memset(pnc_con, 0, sizeof(tpm_db_pnc_conn_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_split_mod_stage1_check()
*
* DESCRIPTION:    function to check the mod in L2 observe to split mod or not
*
* INPUTS:
*           pkt_mod_bm
*           pkt_mod
*
* OUTPUTS:
*
* RETURNS:
*           if the mod observe to split mod, return true, or return false
*
* COMMENTS:
*
*******************************************************************************/
bool tpm_split_mod_stage1_check(tpm_pkt_mod_bm_t pkt_mod_bm,
				tpm_pkt_mod_t *pkt_mod,
				tpm_rule_action_t *rule_action,
				bool check_vlan_noop)
{
	bool temp;

	if (NULL == pkt_mod)
		return false;

	if (rule_action->pkt_act & TPM_ACTION_SPEC_MC_VID)
		return false;

	if (!SET_MOD(rule_action->pkt_act))
		return false;

	if (STAGE_DONE == rule_action->next_phase)
		return false;

	if (check_vlan_noop) {
		if(VLANOP_NOOP == pkt_mod->vlan_mod.vlan_op)
			temp = true;
		else
			temp = false;
	} else {
		temp = false;
	}

	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(TPM_VLAN_MOD == pkt_mod_bm) &&
		((VLANOP_EXT_TAG_MOD == pkt_mod->vlan_mod.vlan_op) ||
			(VLANOP_EXT_TAG_INS == pkt_mod->vlan_mod.vlan_op) ||
			(VLANOP_EXT_TAG_MOD_INS == pkt_mod->vlan_mod.vlan_op) || temp)) {

		/* L2 {VID+p-bit} translation. In GPON mode
		 * not set the SPLIT_MOD AI bit
		 */
		if (TPM_SPLIT_MOD_MODE_CTC == tpm_db_split_mod_get_mode())
			return true;
		else if ((VLANOP_EXT_TAG_MOD == pkt_mod->vlan_mod.vlan_op) &&
			(pkt_mod->vlan_mod.vlan1_out.vid_mask == 0xffff) &&
			(pkt_mod->vlan_mod.vlan1_out.pbit_mask == 0xff))
			return false;
		else
			return true;
	} else {
		return false;
	}

}

/*******************************************************************************
* tpm_proc_l2_tcam_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* l2_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
* OUTPUTS:
* l2_tcam_data      - Logical TCAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_l2_tcam_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       tpm_l2_acl_key_t *l2_key,
			       tpm_parse_fields_t parse_rule_bm,
			       tpm_parse_flags_t parse_flags_bm,
			       tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t src_port_field;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     src_port, dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* L2 Parsing, according to bm in param */
	tcam_data->l2_parse_bm = parse_rule_bm;

	/* Build the AI bits according to parse_flags */
	/* Parse the vlan tag number AI bits */
	src_port_field.src_port = src_port;
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field, parse_flags_bm, 0, &(tcam_data->add_info_data),
					  &(tcam_data->add_info_mask));

	/*Parse MH for specific src_port or for gemport_parse request */
	if (((FROM_SPEC_UNI(src_port)) && (!tpm_proc_gmac1_phy_src_port(src_port))) ||
	    (parse_rule_bm & TPM_L2_PARSE_GEMPORT))
		tcam_data->l2_parse_bm |= TPM_L2_PARSE_MH;

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;
	if (l2_key)
		memcpy(&(tcam_data->pkt_key.l2_key), l2_key, sizeof(tpm_l2_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_L2_MAIN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_l2_sram_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* pon_type          - WAN technology
* l2_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
* pkt_frwd          - See xxx
* rule_action       - See xxx
*
* OUTPUTS:
* l2_sram_data      - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_l2_sram_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       tpm_db_pon_type_t pon_type,
			       tpm_pkt_frwd_t *pkt_frwd,
			       tpm_rule_action_t *rule_action,
			       uint32_t mod_cmd,
			       tpm_pncl_sram_data_t *sram_data,
			       tpm_l2_acl_key_t *l2_key,
			       tpm_pkt_mod_t *pkt_mod,
			       tpm_pkt_mod_bm_t pkt_mod_bm)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_ai_vectors_t ai_vector;
	uint32_t ai_bits;
	long long int_pkt_act = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) \n", src_port, dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Unset DNRT bit (Do not Repeat Tags Phase), the bit is leftover from previous hardcoded Vlan_tags phase */
	int_pkt_act |= (TPM_ACTION_UNSET_DNRT | TPM_ACTION_UNSET_DNRT_DS_TRUNK);

	/* Set UNI Port */
	if (FROM_SPEC_UNI(src_port)) {
		int_pkt_act |= TPM_ACTION_SET_UNI_PORT;
		ai_vector.src_port = src_port;
	}
	/* alloc AI bits if spec mc vlan.
	 * Set_uni_port and mc_vid cannot happen at same time */
	else if (rule_action->pkt_act & TPM_ACTION_SPEC_MC_VID) {
		ret_code = tpm_db_mc_vlan_get_ai_bit_by_vid(l2_key->vlan1.vid, &ai_bits);
		IF_ERROR(ret_code);

		ai_vector.mc_vid_entry = ai_bits;
	}

	/* all tag/untag PNC with VLANOP_NOOP will set split mod AI */
	if (tpm_split_mod_stage1_check(pkt_mod_bm, pkt_mod, rule_action, true)) {

		/* all tag/untag PNC with VLANOP_NOOP will set split mod AI */
		int_pkt_act |= TPM_ACTION_SET_SPLIT_MOD;
	}

	/* Set AI Bits */
	tpm_proc_static_ai_sram_build(&ai_vector, rule_action->pkt_act, int_pkt_act,
				      &(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* Update dummy register (offset automatically=zero) */
	/* The L3_offset_register was already updated in the TPM_PNC_NUM_VLAN_TAGS range */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	if (rule_action->next_phase == STAGE_DONE) {
		tpm_proc_setstage_done(rule_action, sram_data);

		sram_data->sram_updt_bm |= (TPM_PNCL_SET_L3 | TPM_PNCL_SET_L4);
		/* Set L3, L4 to OTHER */
		sram_data->l3_type = TPM_PNCL_L3_OTHER;
		sram_data->l4_type = TPM_PNCL_L4_OTHER;
	} else if (rule_action->next_phase == STAGE_L3_TYPE) {
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_ETH_TYPE, &range_conf);
		IF_ERROR(ret_code);

		sram_data->next_lu_id = range_conf.base_lu_id;
		sram_data->next_lu_off_reg = TPM_PNC_ETY_REG;

	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Signal specific packet types to CPU */
	if (tpm_proc_bc_check(l2_key))
		sram_data->sram_updt_bm |= TPM_PNCL_SET_BC;
	else if (tpm_proc_mc_check(l2_key))
		sram_data->sram_updt_bm |= TPM_PNCL_SET_MC;
	else
		sram_data->sram_updt_bm	|= TPM_PNCL_SET_UC;

	/* Set Modification */
	tpm_proc_set_mod(rule_action, sram_data, mod_cmd);

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_ds_load_balance_tcam_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* rule_num          - API rule number
* l2_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
*
* OUTPUTS:
* tcam_data         - Logical TCAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ds_load_balance_tcam_build(uint32_t rule_num,
				       tpm_l2_acl_key_t *l2_key,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_parse_flags_t parse_flags_bm,
				       tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	int32_t ret_code;
	long long parse_int_flags = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d) parse_rule_bm(%x)  \n", rule_num, parse_rule_bm);

	/* L2 Parsing, according to bm in param */
	tcam_data->l2_parse_bm = parse_rule_bm;

	/* do not repeat this section again */
	parse_int_flags |= TPM_PARSE_FLAG_DNRT_DS_TRUNK;

	tpm_proc_parse_flag_ai_tcam_build(NULL, parse_flags_bm, parse_int_flags,
				&(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/*Parse MH for specific src_port or for gemport_parse request */
	if (parse_rule_bm & TPM_L2_PARSE_GEMPORT)
		tcam_data->l2_parse_bm |= TPM_L2_PARSE_MH;

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = TPM_SRC_PORT_WAN;
	if (l2_key)
		memcpy(&(tcam_data->pkt_key.l2_key), l2_key, sizeof(tpm_l2_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_DS_LOAD_BALANCE, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_ds_load_balance_sram_build()
*
* DESCRIPTION:     Function builds a logical SRAM entry from the API data
*
* INPUTS:
* rule_num          - API rule number
* l2_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
*
* OUTPUTS:
* sram_data         - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ds_load_balance_sram_build(uint32_t rule_num,
				       tpm_pncl_sram_data_t *sram_data,
				       tpm_l2_acl_key_t *l2_key,
				       tpm_ds_load_balance_tgrt_t tgrt_port)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;
	uint32_t cpu_rx_queue = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d) \n", rule_num);

	sram_data->add_info_data |= (1 << TPM_AI_DNRT_DS_TRUNK_BIT_OFF);
	sram_data->add_info_mask |= TPM_AI_DNRT_DS_TRUNK_MASK;

	/* Update dummy register (offset automatically=zero) */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_L2_MAIN, &range_conf);
	IF_ERROR(ret_code);

	sram_data->next_lu_id = range_conf.base_lu_id;
	sram_data->next_lu_off_reg = TPM_PNC_LU_REG0;

	/* For Target set PNC TXP, GemPort */
	sram_data->sram_updt_bm = TPM_PNCL_SET_TXP;
	sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	if (TPM_DS_TGRT_G0 == tgrt_port)
		sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_GMAC0;
	else if (TPM_DS_TGRT_G1 == tgrt_port)
		sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_GMAC1;
	else {
		/* Set lookup done and target */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_LUD;
		sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
		tpm_db_get_cpu_rx_queue(&cpu_rx_queue);
		sram_data->pnc_queue = cpu_rx_queue;

		/* Set L3, L4 to OTHER */
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_L3 | TPM_PNCL_SET_L4);
		sram_data->l3_type = TPM_PNCL_L3_OTHER;
		sram_data->l4_type = TPM_PNCL_L4_OTHER;
	}
	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_tcam_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* l2_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
* OUTPUTS:
* tcam_data         - Logical TCAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ipv4_tcam_build(tpm_src_port_type_t src_port,
				 tpm_dir_t dir,
				 uint32_t rule_num,
				 tpm_ipv4_acl_key_t *ipv4_key,
				 tpm_parse_fields_t parse_rule_bm,
				 tpm_parse_flags_t parse_flags_bm,
				 tpm_rule_action_t *rule_action,
				 tpm_pkt_mod_t *pkt_mod,
				 tpm_pkt_mod_bm_t pkt_mod_bm,
				 tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	uint8_t l4_parse = 0, l4_hdr_update = 0;
	/*uint8_t l4_hdr_update=0; */
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t src_port_field;
	long long parse_int_flags_bm = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) parse_rule_bm(%d) \n",
		     src_port, dir, rule_num, parse_rule_bm);

	/* IPV4 Parsing, according to bm in param */
	tcam_data->ipv4_parse_bm = parse_rule_bm;

	/*Also Check ipv4_ver */
	tcam_data->ipv4_parse_bm |= TPM_IPv4_PARSE_VER;
	tcam_data->pkt_key.ipv4_add_key.ipv4_ver = 0x4;
	tcam_data->pkt_key.ipv4_add_key.ipv4_ver_mask = 1;

	/* If (this rule looks at L4 ports) THEN
	   packet MUST not be fragmented AND
	   packet MUST not have options (ihl=0x5). */
	l4_parse = ((parse_rule_bm & TPM_PARSE_L4_SRC) || (parse_rule_bm & TPM_PARSE_L4_DST));

	/* If (this rule does not look at L4 data,
	   however it is performing packet modification that effects the L4 checksum) THEN
	   packet must not be fragmented  */
	l4_hdr_update = ((SET_MOD(rule_action->pkt_act)) && (L4_CHECK_MOD(pkt_mod_bm)));

	if (l4_parse || l4_hdr_update) {
		/* Must not be fragmented */
		tcam_data->ipv4_parse_bm |= (TPM_IPv4_PARSE_FLAG_MF | TPM_IPv4_PARSE_FRAG_OFFSET);
		tcam_data->pkt_key.ipv4_add_key.ipv4_frag_offset = 0;
		tcam_data->pkt_key.ipv4_add_key.ipv4_frag_offset_mask = 1;
		tcam_data->pkt_key.ipv4_add_key.ipv4_flags = 0;
		tcam_data->pkt_key.ipv4_add_key.ipv4_flags_mask = 1;
		if (l4_parse) {
			/* No options */
			tcam_data->ipv4_parse_bm |= (TPM_IPv4_PARSE_IHL);
			tcam_data->pkt_key.ipv4_add_key.ipv4_ihl = 0x5;
			tcam_data->pkt_key.ipv4_add_key.ipv4_ihl_mask = 1;
		}
	}

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* set CTC CnM */
	/*if Ipv4 split, check AI bit, update tcam*/
	if ((STAGE_CTC_CM == rule_action->next_phase) || ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD))) {

		/*if Ipv4 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	/* Add AI bits (src_port, parse_flag_bits) */
	src_port_field.src_port = src_port;
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field,
					  parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;
	if (ipv4_key)
		memcpy(&(tcam_data->pkt_key.ipv4_key), ipv4_key, sizeof(tpm_ipv4_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV4_MAIN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_dscp_sram_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* rule_num          - API rule number
* target_queue      - CPU (Rx) or Hwf (Tx) Queue to set to
* rule_num          - API rule number
*
* OUTPUTS:
* l2_sram_data      - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ipv4_dscp_sram_build(uint32_t rule_num, uint8_t target_queue, tpm_pncl_sram_data_t *sram_data)
{
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "rule_num(%d) \n", rule_num);

	sram_data->add_info_data = 0;
	sram_data->add_info_mask = 0;
	sram_data->mh_reg.mh_set = TPM_FALSE;
	sram_data->mh_reg.mh_reg = 0;
	sram_data->next_lu_id = 0;
	sram_data->next_lu_off_reg = 0;
	sram_data->next_offset.offset_base = 0;
	sram_data->next_offset.offset_sub.ipv4_subf = 0;
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Set Queue */
	sram_data->pnc_queue = target_queue;
	/* Set Lookup Done */
	sram_data->sram_updt_bm = TPM_PNCL_SET_LUD;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "shift_updt_reg(%d), add_in_data(%x) add_in_mask(%x)",
		     sram_data->shift_updt_reg, sram_data->add_info_data, sram_data->add_info_mask);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_sram_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* pon_type          - WAN technology
* l2_key            - layer2 key data
* ipv4_parse_bm     - Parse rules bitmap
* pkt_frwd          - See xxx
* rule_action       - See xxx
*
* OUTPUTS:
* l2_sram_data      - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ipv4_sram_build(tpm_src_port_type_t src_port,
				 tpm_dir_t dir,
				 uint32_t rule_num,
				 tpm_db_pon_type_t pon_type,
				 tpm_ipv4_acl_key_t *ipv4_key,
				 tpm_parse_fields_t ipv4_parse_bm,
				 tpm_parse_flags_t parse_flags_bm,
				 tpm_pkt_frwd_t *pkt_frwd,
				 tpm_rule_action_t *rule_action,
				 uint32_t mod_cmd,
				 tpm_pkt_mod_bm_t pkt_mod_bm,
				 tpm_pkt_mod_t	  *pkt_mod,
				 tpm_pncl_sram_data_t *sram_data)
{
	uint32_t l4_parse = 0;
	tpm_db_pnc_range_conf_t range_conf;
	long long int_pkt_act = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) \n", src_port, dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Check and set values for STAGE_DONE */
	if (rule_action->next_phase == STAGE_DONE) {
		/* DO NOTHING */
		/* Note: Also for STAGE_DONE,
		   the packet_len_check and ipv4_pkt_type lookups are performed. */
	} else if (rule_action->next_phase != STAGE_CTC_CM) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set next offset and update register */
	sram_data->shift_updt_reg = TPM_PNC_LU_REG1;
	sram_data->next_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	sram_data->next_offset.offset_sub.ipv4_subf = 0xFFFF;	/* For IPV4, this sets the offset to 126 */

	/* If the l4_proto is parsed, then set udp/tcp/other, no need to parse again in later stage */
	if (ipv4_parse_bm & TPM_IPv4_PARSE_PROTO) {
		/* Set correct protocol */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_L4;

		if (ipv4_key->ipv4_proto == IPPROTO_UDP)
			sram_data->l4_type = TPM_PNCL_L4_UDP;
		else if (ipv4_key->ipv4_proto == IPPROTO_TCP)
			sram_data->l4_type = TPM_PNCL_L4_TCP;
		else
			sram_data->l4_type = TPM_PNCL_L4_OTHER;

		/* No need to check proto in later stage */
		int_pkt_act |= TPM_ACTION_SET_NO_PROTO_CHECK;
	}

	/*** Set next lookup configuration ***/

	/* Following applies only to ROUTED/NAPT streams (mac-to-me=1), and not TO_CPU */
	if ((parse_flags_bm & TPM_PARSE_FLAG_MTM_TRUE) &&
	    ((parse_flags_bm & TPM_PARSE_FLAG_TO_CPU_TRUE) == 0) && ((rule_action->pkt_act & TPM_ACTION_TO_CPU) == 0)) {
		/* For TCP NAPT, check the tcp_flags */
		l4_parse = ((ipv4_parse_bm & TPM_PARSE_L4_SRC) || (ipv4_parse_bm & TPM_PARSE_L4_DST));
		if (l4_parse && (sram_data->l4_type == TPM_PNCL_L4_TCP)) {
			sram_data->next_lu_off_reg = TPM_PNC_LU_REG1;
			ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_TCP_FLAG, &range_conf);
			IF_ERROR(ret_code);
		}
		/* For any other NAPT/ROUTE, check TTL */
		else {
			ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_TTL, &range_conf);
			IF_ERROR(ret_code);
		}
	} else if (rule_action->next_phase == STAGE_CTC_CM){
		/* go to CTC stage */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_IPV4_PRE, &range_conf);
		IF_ERROR(ret_code);

		/* set IPv4 AI and set L4 type */
		sram_data->l4_type = TPM_PNCL_L4_OTHER;
		sram_data->sram_updt_bm |= TPM_PNCL_SET_L4;

		sram_data->next_lu_off_reg = TPM_PNC_LU_REG0;
		/* set action IPv4 */
		int_pkt_act = TPM_ACTION_SET_CNM_IPV4;
	} else {  /* All others - Next lu_id = IPV4_PROTO/IPV4_FRAG/IPV4_LEN */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_PROTO, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_off_reg = TPM_PNC_LU_REG0;
	}

	sram_data->next_lu_id = range_conf.base_lu_id;

	/* If fragmented packets are not allowed (set in tpm_proc_ipv4_tcam_build funct),
	   no need to do additional parsing stage  */
	/* TODO: A full check should also check the values of MF, FRAG_OFFSET,
	   not just check they are part of the parse_rule_bm */
	if (ipv4_parse_bm & (TPM_IPv4_PARSE_FLAG_MF | TPM_IPv4_PARSE_FRAG_OFFSET)) {
		sram_data->l3_type = TPM_PNCL_L3_IPV4_NFRAG;
		sram_data->sram_updt_bm |= TPM_PNCL_SET_L3;
		int_pkt_act |= TPM_ACTION_SET_NO_FRAG_CHECK;
	}

	if (pkt_mod_bm & TPM_PPPOE_ADD)
		int_pkt_act |= TPM_ACTION_SET_ADD_PPPOE;

	/* Reset AI bits for uni_ports  */
	if (rule_action->next_phase != STAGE_CTC_CM)
		int_pkt_act |= TPM_ACTION_UNSET_UNI_PORT;

	/* Build AI_bits according to pkt_actions bitmap */

	/* Set AI bits */
	tpm_proc_static_ai_sram_build(NULL,
				      rule_action->pkt_act, int_pkt_act, &(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* Set Modification */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) {
		/* split mod stage-2 */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}
	else {
		tpm_proc_set_mod(rule_action, sram_data, mod_cmd);
	}

	/* Set Target and Queue */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
		     "next_lu_id(%d), next_lu_off_reg(%d), next_off_base(%d), next_off_subf(%x), shift_updt_reg(%d)\n"
		     "add_info_data(%x) add_info_mask(%x)\n", sram_data->next_lu_id, sram_data->next_lu_off_reg,
		     sram_data->next_offset.offset_base, sram_data->next_offset.offset_sub.ipv4_subf,
		     sram_data->shift_updt_reg, sram_data->add_info_data, sram_data->add_info_mask);

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_gen_tcam_build(tpm_src_port_type_t src_port,
				       tpm_dir_t dir,
				       uint32_t rule_num,
				       tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_parse_flags_t parse_flags_bm,
				       tpm_pkt_mod_t *pkt_mod,
				       tpm_pkt_mod_bm_t pkt_mod_bm,
				       tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t src_port_field;
	long long parse_int_flags_bm = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     src_port, dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* IPV6 parsing, according to parse_rule */
	tcam_data->ipv6_parse_bm = parse_rule_bm;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* set CTC CnM */
	/*if Ipv6 split, check AI bit, update tcam*/
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {

		/*if Ipv6 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	/* Build the AI bits according to parse_flags */
	src_port_field.src_port = src_port;
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field,
					  parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;
	if (ipv6_gen_key)
		memcpy(&(tcam_data->pkt_key.ipv6_gen_key), ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_GEN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_gen_sram_build(tpm_src_port_type_t src_port,
				       tpm_dir_t dir,
				       uint32_t rule_num,
				       tpm_db_pon_type_t pon_type,
				       tpm_pkt_frwd_t *pkt_frwd,
				       tpm_rule_action_t *rule_action,
				       uint32_t mod_cmd,
				       tpm_pncl_sram_data_t *sram_data)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) \n", src_port, dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Set L3 */
	sram_data->l3_type = TPM_PNCL_L3_IPV6;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_L3;

	if (rule_action->next_phase == STAGE_DONE) {
		tpm_proc_setstage_done(rule_action, sram_data);

		/* Update dummy register (offset automatically=zero) */
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	} else if (rule_action->next_phase == STAGE_IPv6_DIP) {
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_DIP, &range_conf);
		IF_ERROR(ret_code);

		sram_data->next_lu_id = range_conf.base_lu_id;
		sram_data->next_lu_off_reg = TPM_PNC_IPV6_DIP_REG;

		sram_data->next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
		sram_data->next_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_DIP;
		sram_data->shift_updt_reg = TPM_PNC_IPV6_DIP_REG;
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set Modification */
	tpm_proc_set_mod(rule_action, sram_data, mod_cmd);

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_dip_tcam_build(tpm_src_port_type_t src_port,
				     tpm_dir_t dir,
				     uint32_t rule_num,
				     tpm_ipv6_addr_key_t *ipv6_dip_key,
				     tpm_parse_fields_t parse_rule_bm,
				     tpm_parse_flags_t parse_flags_bm,
				     tpm_pkt_mod_t *pkt_mod,
				     tpm_pkt_mod_bm_t pkt_mod_bm,
				     tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	int32_t ret_code;
	tpm_ai_vectors_t src_port_field;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     src_port, dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* IPV6 parsing, according to parse_rule */
	tcam_data->ipv6_parse_bm = parse_rule_bm;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* Build the AI bits according to parse_flags */
	/* set CTC CnM */
	/*if Ipv6 split, check AI bit, update tcam*/
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {

		/*if Ipv6 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	src_port_field.src_port = src_port;
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field,
					  parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;

	if (ipv6_dip_key)
		memcpy(&(tcam_data->pkt_key.ipv6_dip_key), ipv6_dip_key, sizeof(tpm_ipv6_addr_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_DIP, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_dip_sram_build(tpm_src_port_type_t src_port,
				     tpm_dir_t dir,
				     uint32_t rule_num,
				     tpm_db_pon_type_t pon_type,
				     tpm_pkt_frwd_t *pkt_frwd,
				     tpm_rule_action_t *rule_action,
				     uint32_t mod_cmd,
				     tpm_pncl_sram_data_t *sram_data)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) \n", src_port, dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Update dummy register (offset automatically=zero) */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Next Stage */
	if (rule_action->next_phase == STAGE_DONE) {
		tpm_proc_setstage_done(rule_action, sram_data);

	} else if (rule_action->next_phase == STAGE_IPv6_NH) {
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_NH, &range_conf);
		IF_ERROR(ret_code);

		/* Next phase offset already exists in REG_0 */
		sram_data->next_lu_id = range_conf.base_lu_id;
		sram_data->next_lu_off_reg = TPM_PNC_LU_REG0;
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set Modification */
	tpm_proc_set_mod(rule_action, sram_data, mod_cmd);

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_nh_tcam_build(uint32_t rule_num,
				    tpm_nh_iter_t nh_iter,
				    uint32_t nh,
				    tpm_parse_flags_t parse_flags_bm,
				    tpm_rule_action_t *rule_action,
				    tpm_pncl_tcam_data_t *tcam_data)
{
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	long long parse_int_flags_bm = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d) \n", rule_num);

	if (nh != TPM_NH_RESERVED)
		tcam_data->ipv6_parse_bm = TPM_IPv6_PARSE_NH;
	else
		tcam_data->ipv6_parse_bm = 0;

	if (nh_iter == NH_ITER_0)
		parse_int_flags_bm = TPM_PARSE_FLAG_NH2_ITER_FALSE;
	else
		parse_int_flags_bm = TPM_PARSE_FLAG_NH2_ITER_TRUE;

	if (STAGE_CTC_CM == rule_action->next_phase) {
		/*if next stage is CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	/* Insert AI_BITS */
	tpm_proc_parse_flag_ai_tcam_build(NULL,
					  parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Rule applies to LAN GMAC and WAN GMAC, use "special purpose" src_port */
	tcam_data->port_ids = tpm_proc_all_gmac_bm();

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = TPM_SRC_PORT_WAN_OR_LAN;
	tcam_data->pkt_key.ipv6_key.ipv6_next_header = nh;

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_NH, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;

	if (nh_iter == NH_ITER_1) {
		start_offset.offset_base = TPM_PNCL_IPV6_EXT_OFFSET;
		start_offset.offset_sub.subf = TPM_IPv6_PARSE_NH;
	}
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_nh_sram_build(uint32_t rule_num,
					tpm_nh_iter_t nh_iter,
					uint32_t nh,
				    tpm_db_pon_type_t pon_type,
				    tpm_pkt_frwd_t *pkt_frwd,
				    tpm_rule_action_t *rule_action,
				    tpm_pncl_sram_data_t *sram_data)
{

	tpm_db_pnc_range_conf_t range_conf;
	int32_t ret_code;
	tpm_dir_t dir;
	long long int_pkt_act = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d) \n", rule_num);

	memset(sram_data, 0, sizeof(tpm_pncl_sram_data_t));

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}
	dir = TPM_DIR_DS;	/*dummy_dir for bi_dir function */

	/* Default, don't set MH */
	sram_data->mh_reg.mh_set = TPM_FALSE;

	/* Set the L4 protocol & the L4 AI bits, if udp or tcp */
	sram_data->sram_updt_bm |= TPM_PNCL_SET_L4;
	if (nh == IPPROTO_UDP) {
		sram_data->l4_type = TPM_PNCL_L4_UDP;
		int_pkt_act |= TPM_ACTION_SET_L4P_TOG_UDP;
	} else if (nh == IPPROTO_TCP) {
		sram_data->l4_type = TPM_PNCL_L4_TCP;
		int_pkt_act |= TPM_ACTION_SET_L4P_TOG_TCP;
	} else
		sram_data->l4_type = TPM_PNCL_L4_OTHER;

	if (nh_iter == NH_ITER_0) {
		/* Calculated value, from start of IPV6_HDR untill end of it */
		sram_data->next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
		sram_data->next_offset.offset_sub.ipv6_subf = 0xffff;	/* End of IPV6 Header */
	} else {
		/* Calculated value, from start of IPV6_HDR untill end of it */
		sram_data->next_offset.offset_base = TPM_PNCL_IPV6_EXT_OFFSET;
		sram_data->next_offset.offset_sub.ipv6_subf = 0xffff;	/* For IPV6 Ext Header, this sets the offset to 127 */
	}

	/* Sram update */
	if (rule_action->next_phase == STAGE_IPV6_L4) {
		/* Update PNC_REG_1 (L4_OFFSET_REG) with the IPV6_HDR_LEN (40) */
		sram_data->shift_updt_reg = TPM_PNC_LU_REG1;	/* update L4_OFFSET_REG */
		sram_data->next_lu_off_reg = TPM_PNC_LU_REG1;	/* lookup reg 1 */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_L4, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
		int_pkt_act |= TPM_ACTION_UNSET_NH2_ITER;
	} else if (rule_action->next_phase == STAGE_IPv6_NH) {
		/* Update PNC_REG_1 (L4_OFFSET_REG) with the IPV6_HDR_LEN (40) */
		sram_data->shift_updt_reg = TPM_PNC_LU_REG1;	/* update L4_OFFSET_REG */
		sram_data->next_lu_off_reg = TPM_PNC_LU_REG1;	/* lookup reg 1 */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_NH, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
		int_pkt_act |= TPM_ACTION_SET_NH2_ITER;
	} else if (rule_action->next_phase == STAGE_IPv6_GEN) {
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_GEN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
		int_pkt_act |= TPM_ACTION_UNSET_NH2_ITER;
	} else if (rule_action->next_phase == STAGE_CTC_CM) {
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
		sram_data->next_lu_off_reg = TPM_PNC_CNM_L2_REG;
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
		int_pkt_act |= TPM_ACTION_UNSET_CNM_IPV4 | TPM_ACTION_UNSET_IPV4_PRE_KEY;
	} else if (rule_action->next_phase == STAGE_DONE)
		tpm_proc_setstage_done(rule_action, sram_data);
	else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " next phase not supported (%d)\n", rule_action->next_phase);
		return(TPM_FAIL);
	}

	/* Set AI bits */
	tpm_proc_static_ai_sram_build(NULL,
				      rule_action->pkt_act, int_pkt_act, &(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_l4ports_tcam_build(tpm_src_port_type_t src_port,
					 tpm_dir_t dir,
					 uint32_t rule_num,
					 tpm_l4_ports_key_t *l4_key,
					 tpm_parse_fields_t parse_rule_bm,
					 tpm_parse_flags_t parse_flags_bm,
					 tpm_rule_action_t *rule_action,
					 tpm_pkt_mod_t *pkt_mod,
					 tpm_pkt_mod_bm_t pkt_mod_bm,
					 tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t src_port_field;
	long long parse_int_flags_bm = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     src_port, dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* L4 ports parsing, according to parse_rule */
	tcam_data->l4_parse_bm = parse_rule_bm;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* Build the AI bits according to parse_flags */
	src_port_field.src_port = src_port;
	/*if Ipv6 split, check AI bit, update tcam*/
	if ((STAGE_CTC_CM == rule_action->next_phase) || ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD))) {

		/*if Ipv6 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field,
					  parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;

	if (l4_key)
		memcpy(&(tcam_data->pkt_key.l4_ports_key), l4_key, sizeof(tpm_l4_ports_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_L4, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_l4ports_sram_build(tpm_src_port_type_t src_port,
					 tpm_dir_t dir,
					 uint32_t rule_num,
					 tpm_db_pon_type_t pon_type,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_rule_action_t *rule_action,
					 uint32_t mod_cmd,
					 tpm_pncl_sram_data_t *sram_data)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) \n", src_port, dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Update dummy register (offset automatically=zero) */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Next Stage */
	if (rule_action->next_phase == STAGE_DONE)
		tpm_proc_setstage_done(rule_action, sram_data);
	else if (rule_action->next_phase == STAGE_CTC_CM) {
		/* nothing to Update */
		sram_data->next_lu_off_reg = TPM_PNC_CNM_L2_REG;
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;

		sram_data->add_info_data &= ~(TPM_AI_CNM_IPV4_MASK | TPM_AI_CNM_IPV4_PRE_KEY_MASK);
		sram_data->add_info_mask |= (TPM_AI_CNM_IPV4_MASK | TPM_AI_CNM_IPV4_PRE_KEY_MASK);
	}
	else if (rule_action->next_phase == STAGE_IPv6_GEN) {
		/* nothing to Update */
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
		sram_data->next_lu_off_reg = 0; /* lookup reg 1 */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_GEN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
	}
	else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set Modification */
	tpm_proc_set_mod(rule_action, sram_data, mod_cmd);

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_api_entry_rulenum_inc()
*
* DESCRIPTION:    Function increases the rule_number for a range of api_rule_nums
*
* INPUTS:
* api_section      - Section of the API acl/table
* dir              - Direction of the API acl/table
* rule_inc_start   - First entry to increase rule_num
* rule_inc_end     - Last entry to increase rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_api_entry_rulenum_inc(tpm_api_sections_t api_section, uint32_t rule_inc_start, uint32_t rule_inc_end)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " api_section(%d), rule_dec_start(%d), rule_dec_end(%d)\n",
		     api_section, rule_inc_start, rule_inc_end);

	/* Increase the pnc_connection of the API's in the range */
	ret_code = tpm_db_pnc_conn_inc(api_section, rule_inc_start, rule_inc_end);
	IF_ERROR(ret_code);

	/* Increase the rule_num of the API's in the range */
	ret_code = tpm_db_api_entry_rulenum_inc(api_section, rule_inc_start, rule_inc_end);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_api_entry_rulenum_dec()
*
* DESCRIPTION:    Function decreases the rule_number for a range of api_rule_nums
*
* INPUTS:
* api_section      - Section of the API acl/table
* dir              - Direction of the API acl/table
* rule_dec_start   - First entry to decrease rule_num
* rule_dec_end     - Last entry to decrease rule_num
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_api_entry_rulenum_dec(tpm_api_sections_t api_section, uint32_t rule_dec_start, uint32_t rule_dec_end)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " api_section(%d, rule_dec_start(%d), rule_dec_end(%d)\n",
		     api_section, rule_dec_start, rule_dec_end);

	/* Decrease the pnc_connection of the API's in the range */
	ret_code = tpm_db_pnc_conn_dec(api_section, rule_dec_start, rule_dec_end);
	IF_ERROR(ret_code);

	/* Decrease the rule_num of the API's in the range */
	ret_code = tpm_db_api_entry_rulenum_dec(api_section, rule_dec_start, rule_dec_end);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_get_cpu_lpbk_entry_num()
*
* DESCRIPTION: The API get the entry number from CPU loopback DB
*
* INPUTS:
*         None
*
* OUTPUTS:
*         None
*
* RETURNS:
* CPU loopback entry number.
*
* COMMENTS:
*           None
*
*******************************************************************************/
uint32_t tpm_proc_get_cpu_lpbk_entry_num(void)
{
	return gn_cpu_lpbk_entry_num;
}

/*******************************************************************************
* tpm_proc_increase_cpu_lpbk_entry_num()
*
* DESCRIPTION: The API get the entry number from CPU loopback DB
*
* INPUTS:
*         None
*
* OUTPUTS:
*         None
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_proc_increase_cpu_lpbk_entry_num(void)
{
	gn_cpu_lpbk_entry_num++;

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_decrease_cpu_lpbk_entry_num()
*
* DESCRIPTION: The API get the entry number from CPU loopback DB
*
* INPUTS:
*         None
*
* OUTPUTS:
*         None
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_proc_decrease_cpu_lpbk_entry_num(void)
{
	if (gn_cpu_lpbk_entry_num > 0)
		gn_cpu_lpbk_entry_num--;

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_check_cpu_wan_loopback_param()
*
* DESCRIPTION: The API check the parameters of CPU loopback
*
* INPUTS:
*           trg_port    - target tcont/LLID bitmap
*           trg_queue   - target queue, 0-7
*           gem_port    - target gem port ID
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_proc_check_cpu_wan_loopback_param(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd)
{
	tpm_gmacs_enum_t act_wan = tpm_db_active_wan_get();
	tpm_db_pon_type_t pon_type;

	tpm_db_pon_type_get(&pon_type);

	if (TO_PON(TPM_DIR_US, pkt_frwd->trg_port, pon_type, act_wan) ||
	    TO_ETHWAN(TPM_DIR_US, pkt_frwd->trg_port, act_wan)) {
		/*cont */
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "*Illegal Target Port: dir=%d, trg_port=%08x, pon_type=%d *\r\n",
			     TPM_DIR_US, pkt_frwd->trg_port, pon_type);
		return(ERR_FRWD_INVALID);
	}

	if (pkt_frwd->trg_queue >= TPM_MAX_NUM_TX_QUEUE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "trg_queue[%d] is illegal\n", pkt_frwd->trg_queue);
		return ERR_GENERAL;
	}

	if (pkt_frwd->gem_port >= TPM_MAX_NUM_GEM_PORT) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "gem_port[%d] is illegal\n", pkt_frwd->gem_port);
		return ERR_GENERAL;
	}

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_calc_flow_id()
*
* DESCRIPTION:    Calculate flow id according to frwd information.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_calc_flow_id(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd, uint16_t *flow_id)
{
	uint32_t llid_idx = 0;

	/*GPON mode */
	if (pkt_frwd->gem_port)
		*flow_id = pkt_frwd->gem_port;
	/*EPON mode */
	else {
		/* Convert target port bitmap to llid number */
		for (llid_idx = 0; llid_idx < 8; llid_idx++) {
			if ((1 << llid_idx) & pkt_frwd->trg_port)
				break;
		}

		/* Set default llid index */
		if (llid_idx >= 8)
			llid_idx = 0;

		*flow_id = (llid_idx * TPM_MAX_NUM_TX_QUEUE) + pkt_frwd->trg_queue;
	}

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_init_cpu_wan_loopback()
*
* DESCRIPTION: The API initialize CPU egress loopback data
*
* INPUTS:
*
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_proc_init_cpu_wan_loopback(void)
{

	/* PnC and Modification Initialization are handled in global Init procedures,
	   and not in this function. */

	memset((uint8_t *) &gs_cpu_loopback[0], 0, sizeof(gs_cpu_loopback));
	gn_cpu_lpbk_entry_num = 0;

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_init_cpu_wan_loopback()
*
* DESCRIPTION: The API initialize CPU egress loopback data
*
* INPUTS:
*           flow_id - Flow ID as the index of loopback db
*
* OUTPUTS:
*           None
*
* RETURNS:
*           The pointer to specific CPU WAN loopback entry
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_cpu_loopback_t *tpm_proc_get_loopback_entry(uint16_t flow_id)
{
	if (flow_id >= TPM_MAX_CPU_LOOPBACK_NUM) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " flow_id[%d] is illegal\n", flow_id);

		return NULL;
	}

	return &gs_cpu_loopback[flow_id];
}

/*******************************************************************************
* tpm_proc_add_cpu_loopback_rule()
*
* DESCRIPTION:    Main function for adding cpu loopback rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_cpu_loopback_rule(uint32_t owner_id,
						uint32_t rule_num,
						uint32_t *rule_idx,
						tpm_pkt_frwd_t *pkt_frwd)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint16_t flow_id = 0;
	uint32_t pnc_entry = 0;
	uint32_t mod_entry = 0;
	uint32_t api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	tpm_gmacs_enum_t trg_gmac;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;

	tpm_pkt_mod_t pkt_mod;
	tpm_l2_acl_key_t l2_key;
	tpm_rule_action_t rule_action;

	if (!tpm_db_cpu_wan_lpbk_en_get()){
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "CPU WAN loopback has not been enabled\n");
		IF_ERROR(ERR_FEAT_UNSUPPORT);
	}

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d)\n", owner_id, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_cpu_loopback_check(owner_id, rule_num, pkt_frwd);
	IF_ERROR(ret_code);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_CPU_LOOPBACK_ACL, &range_id);

	/*********** Create Modification Entries **********/
	memset(&pkt_mod, 0, sizeof(tpm_pkt_mod_t));
	pkt_mod.vlan_mod.vlan_op = VLANOP_EXT_TAG_DEL;
	ret_code =
	tpm_proc_create_mod(TPM_ACTION_SET_PKT_MOD, pkt_frwd->trg_port, &pkt_mod, TPM_VLAN_MOD, 0, &mod_entry,
			    &trg_gmac);
	IF_ERROR(ret_code);

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	memset(&l2_key, 0, sizeof(tpm_l2_acl_key_t));
	l2_key.vlan1.tpid = TPM_MOD2_CPU_LOOPBACK_ETY;
	l2_key.vlan1.tpid_mask = 0xffff;
	l2_key.vlan1.vid_mask = 0xffff;
	ret_code = tpm_proc_calc_flow_id(TPM_MOD_OWNER_TPM, pkt_frwd, &flow_id);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}
	l2_key.vlan1.vid = flow_id;

	int_ret_code =
	tpm_proc_l2_tcam_build(TPM_SRC_PORT_UNI_ANY, TPM_DIR_US, rule_num, &l2_key, TPM_L2_PARSE_ONE_VLAN_TAG, 0,
			       &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	memset(&rule_action, 0, sizeof(tpm_rule_action_t));
	rule_action.next_phase = STAGE_DONE;
	rule_action.pkt_act = TPM_ACTION_SET_TARGET_PORT | TPM_ACTION_SET_TARGET_QUEUE | TPM_ACTION_SET_PKT_MOD;
	int_ret_code =
	tpm_proc_l2_sram_build(TPM_SRC_PORT_UNI_ANY, TPM_DIR_US, rule_num, pon_type, pkt_frwd, &rule_action,
			       mod_entry, &(pnc_data.pncl_sram), &l2_key, &pkt_mod, 0);
	IF_ERROR(int_ret_code);

	/*** Insert the PNC Entry ***/
	tpm_proc_create_acl_pnc_entry(TPM_CPU_LOOPBACK_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
	IF_ERROR(int_ret_code);

	/*********** Update API Range in DB **********/

	/* Set PNC API data */
	api_data.l2_prim_key.src_port = TPM_SRC_PORT_UNI_ANY;
	api_data.l2_prim_key.parse_rule_bm = TPM_L2_PARSE_ONE_VLAN_TAG;
	api_data.l2_prim_key.parse_flags_bm = 0;
	api_data.l2_prim_key.pkt_mod_bm = TPM_VLAN_MOD;
	memcpy(&(api_data.l2_prim_key.rule_action), &rule_action, sizeof(tpm_rule_action_t));
	memcpy(&(api_data.l2_prim_key.l2_key), &l2_key, sizeof(tpm_l2_acl_key_t));
	memcpy(&(api_data.l2_prim_key.pkt_mod), &pkt_mod, sizeof(tpm_pkt_mod_t));
	if (pkt_frwd != NULL)
		memcpy(&(api_data.l2_prim_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.l2_prim_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	/* Set API modification data */
	tpm_proc_set_api_moddata(rule_action.pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
	if (rule_num < api_rng_entries) {
		int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_CPU_LOOPBACK_ACL, rule_num, (api_rng_entries - 1));
		IF_ERROR(int_ret_code);
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_CPU_LOOPBACK_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_add_cpu_wan_loopback()
*
* DESCRIPTION: The API add CPU egress loopback modification and PnC rules for
*              specific Tcont/queue/gem_port
*
* INPUTS:   owner_id    - APP owner id  should be used for all API calls.
*           pkt_frwd    - packet forwarding info: target tcont/LLID number, 0-7
*                         target queue, 0-7, target gem port ID
*
*
* OUTPUTS:
*           mod_idx     - Returned by HW modification, to fill Tx description
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_cpu_wan_loopback(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd, uint32_t *mod_idx)
{
	int32_t ret_code;
	tpm_init_cpu_loopback_t cpu_loopback_val;
	tpm_pkt_mod_t mod_data;
	uint32_t rule_idx = 0;
	uint16_t flow_id = 0;
	tpm_cpu_loopback_t *lpbk_entry = NULL;
	uint32_t lpbk_num = 0;

	if (!tpm_db_cpu_wan_lpbk_en_get()){
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "CPU WAN loopback has not been enabled\n");
		IF_ERROR(ERR_FEAT_UNSUPPORT);
	}

	/* Verify whether input parameters are legal */
	ret_code = tpm_proc_check_cpu_wan_loopback_param(owner_id, pkt_frwd);
	if (ret_code != TPM_RC_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Get flow ID */
	ret_code = tpm_proc_calc_flow_id(TPM_MOD_OWNER_TPM, pkt_frwd, &flow_id);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Get CPU WAN loopback entry from DB */
	lpbk_entry = tpm_proc_get_loopback_entry(flow_id);
	if (lpbk_entry == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return ERR_GENERAL;
	}

	/* Check whether the target has already been created, this validation is put at the */
	/* first because of performance consideration.                                      */
	if (TPM_TRUE == lpbk_entry->in_use) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "CPU loopback: target already exsit, return \r\n");
		*mod_idx = lpbk_entry->mod_idx;
		return TPM_RC_OK;
	}

	/* Get CPU loopback state: disabled / enabled */
	ret_code = tpm_db_cpu_loopback_get(&cpu_loopback_val);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Do nothing if CPU loopback function is disabled */
	if (TPM_CPU_LOOPBACK_DISABLED == cpu_loopback_val) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "CPU loopback disabled, return \r\n");
		return TPM_RC_OK;
	}

	/* Check whether CPU loopback entry number exceed the Max value */
	lpbk_num = tpm_proc_get_cpu_lpbk_entry_num();
	if (lpbk_num >= TPM_MAX_CPU_LOOPBACK_ENTRY) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]:(%d) Exceed MAX entry number\r\n", __func__, __LINE__,
			     lpbk_num);
		return ERR_GENERAL;
	}

	/* Add modification rules to GMAC1, and get mod index */
	memset((uint8_t *) &mod_data, 0, sizeof(tpm_pkt_mod_t));
	mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_INS;
	mod_data.vlan_mod.vlan1_out.tpid = TPM_MOD2_CPU_LOOPBACK_ETY;
	mod_data.vlan_mod.vlan1_out.tpid_mask = TPM_MOD2_NEW_TPID;
	mod_data.vlan_mod.vlan1_out.vid = flow_id;
	mod_data.vlan_mod.vlan1_out.vid_mask = TPM_MOD2_NEW_VID;
	mod_data.vlan_mod.vlan1_out.cfi = 0;
	mod_data.vlan_mod.vlan1_out.cfi_mask = TPM_MOD2_NEW_CFI;
	mod_data.vlan_mod.vlan1_out.pbit = 0;
	mod_data.vlan_mod.vlan1_out.pbit_mask = TPM_MOD2_NEW_PBIT;
	ret_code = tpm_mod2_entry_set(TPM_MOD_OWNER_TPM, TPM_ENUM_GMAC_1, TPM_VLAN_MOD, 0, &mod_data, mod_idx);
	if (ret_code != TPM_RC_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Add PnC rules including modification to GMAC0 and PON MAC */
	/* Rule number will always be 0 in ACL mode */
	ret_code = tpm_proc_add_cpu_loopback_rule(TPM_MOD_OWNER_TPM, 0, &rule_idx, pkt_frwd);
	if (ret_code != TPM_RC_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Save to DB */
	lpbk_entry->trg_port = pkt_frwd->trg_port;
	lpbk_entry->trg_queue = pkt_frwd->trg_queue;
	lpbk_entry->gem_port = pkt_frwd->gem_port;
	lpbk_entry->mod_idx = *mod_idx;
	lpbk_entry->rule_idx = rule_idx;
	lpbk_entry->in_use = TPM_TRUE;

	tpm_proc_increase_cpu_lpbk_entry_num();

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_del_cpu_wan_loopback()
*
* DESCRIPTION: The API delete CPU egress loopback modification and PnC rules for
*              specific Tcont/queue/gem_port
*
* INPUTS:   owner_id    - APP owner id  should be used for all API calls.
*           pkt_frwd    - packet forwarding info: target tcont/LLID number, 0-7
*                         target queue, 0-7, target gem port ID
*
* OUTPUTS:
*           NONE
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_cpu_wan_loopback(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd)
{
	int32_t ret_code;
	tpm_init_cpu_loopback_t cpu_loopback_val;
	uint16_t flow_id = 0;
	tpm_cpu_loopback_t *lpbk_entry = NULL;

	if (!tpm_db_cpu_wan_lpbk_en_get()){
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "CPU WAN loopback has not been enabled\n");
		IF_ERROR(ERR_FEAT_UNSUPPORT);
	}

	/* Verify whether input parameters are legal */
	ret_code = tpm_proc_check_cpu_wan_loopback_param(owner_id, pkt_frwd);
	if (ret_code != TPM_RC_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Get CPU loopback state: disabled / enabled */
	ret_code = tpm_db_cpu_loopback_get(&cpu_loopback_val);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " recvd ret_code(%d)\r\n", ret_code);
		return(ret_code);
	}

	/* Do nothing if CPU loopback function is disabled */
	if (TPM_CPU_LOOPBACK_DISABLED == cpu_loopback_val) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "CPU loopback disabled, return \r\n");
		return TPM_RC_OK;
	}

	/* Get flow ID */
	ret_code = tpm_proc_calc_flow_id(TPM_MOD_OWNER_TPM, pkt_frwd, &flow_id);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return(ret_code);
	}

	/* Get CPU WAN loopback entry from DB */
	lpbk_entry = tpm_proc_get_loopback_entry(flow_id);
	if (lpbk_entry == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "%s[Line(%d)]: recvd ret_code(%d) \r\n", __func__, __LINE__,
			     ret_code);
		return ERR_GENERAL;
	}

	/* Delete related GMAC1 mod and PnC rule if the entry already exsit */
	if (TPM_TRUE == lpbk_entry->in_use) {
		/* Delete GMAC1 modification */
		ret_code = tpm_mod2_entry_del(TPM_MOD_OWNER_TPM, TPM_ENUM_GMAC_1, lpbk_entry->mod_idx);
		if (ret_code != TPM_RC_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " tpm_mod2_entry_del error, ret_code(%d)\r\n", ret_code);
			return(ret_code);
		}

		/* Delete PnC rule and PON MAC modification */
		ret_code = tpm_proc_del_acl_rule(TPM_CPU_LOOPBACK_ACL, TPM_MOD_OWNER_TPM, lpbk_entry->rule_idx, TPM_FALSE);
		if (ret_code != TPM_RC_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " tpm_proc_del_acl_rule error, ret_code(%d)\r\n", ret_code);
			return(ret_code);
		}

		/* Delete entry from DB */
		memset(lpbk_entry, 0, sizeof(tpm_cpu_loopback_t));
		lpbk_entry->in_use = TPM_FALSE;
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "loopback entry non-existing, recvd ret_code(%d) \r\n", ret_code);
		return ERR_GENERAL;
	}


	tpm_proc_decrease_cpu_lpbk_entry_num();

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_add_l2_prim_acl_rule()
*
* DESCRIPTION:    Main function for adding L2 API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_l2_prim_acl_rule(uint32_t owner_id,
					       tpm_src_port_type_t src_port,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l2_acl_key_t *l2_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_pkt_mod_t *pkt_mod,
					       tpm_pkt_mod_bm_t pkt_mod_bm,
					       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, mod_entry_tmp = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0, bi_dir = 0, update_sram_only = 0;
	tpm_gmacs_enum_t trg_gmac, duplicate_gmac;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_api_lu_conf_t lu_conf;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d)\n", owner_id, src_port, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_l2_check(owner_id, src_port, rule_num, parse_rule_bm,
					 l2_key, parse_flags_bm, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_L2_PRIM_ACL, &range_id);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Only do it in table mode */
	if (TPM_RANGE_TYPE_TABLE == rangConf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_L2_PRIM_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d) already exists\n",
				     owner_id, src_port, rule_num);

			/* TCAM Key and modification info are not changed, only update SRAM */
			if ((0 == memcmp(l2_key, &api_data.l2_prim_key.l2_key, sizeof(tpm_l2_acl_key_t)))
			    && (src_port == api_data.l2_prim_key.src_port)
			    && (parse_rule_bm == api_data.l2_prim_key.parse_rule_bm)
			    && (parse_flags_bm == api_data.l2_prim_key.parse_flags_bm)
			    && (SET_MOD(rule_action->pkt_act) == SET_MOD(api_data.l2_prim_key.rule_action.pkt_act))) {
				/* No modification */
				if (!SET_MOD(rule_action->pkt_act))
					update_sram_only = 1;
				/* There are modification, then check furthur */
				else if ((pkt_mod_bm == api_data.l2_prim_key.pkt_mod_bm)
					 && (0 ==
					     memcmp(pkt_mod, &api_data.l2_prim_key.pkt_mod, sizeof(tpm_pkt_mod_t)))) {
					update_sram_only = 1;
					mod_entry = mod_con.mod_cmd_ind;
					trg_gmac = mod_con.mod_cmd_mac;
				}
			}

			/* TCAM Key or modification info is changed */
			if (!update_sram_only) {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
					     " owner(%d) src_port(%d), rule_num(%d) TCAM key/modification changed\n",
					     owner_id, src_port, rule_num);

				tpm_proc_del_l2_prim_acl_rule(owner_id, rule_num, TPM_INT_CALL);
				IF_ERROR(ret_code);
			}
		}
	}

	if (    (!update_sram_only)
	     && (SET_MOD(rule_action->pkt_act))) {

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* if split mod is enable, add split rule if possible*/
		if (tpm_split_mod_stage1_check(pkt_mod_bm, pkt_mod, rule_action, false)) {
			/* split mod stage-1 */
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-1\n");
			/* get mod index from p-bit value */
			ret_code = tpm_mod2_split_mod_create_l2_pmts(trg_gmac, pkt_mod, false);
			if (TPM_RC_OK != ret_code)
			{
				TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "failed to add pmt split mod stage-1\n");
				return(TPM_FAIL);
			}
			int_ret_code = tpm_db_mod2_split_mod_get_vlan_index(trg_gmac, pkt_mod, &mod_entry_tmp);
			if (TPM_DB_OK != int_ret_code) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-1, failed to get pmt entry\n");
				return ERR_MOD_INVALID;
			}

			mod_entry = mod_entry_tmp * 16;

			/* VLANOP_EXT_TAG_MOD_INS mod insert VLAN p_bit*/
			if (VLANOP_EXT_TAG_MOD_INS == pkt_mod->vlan_mod.vlan_op) {
				if (pkt_mod->vlan_mod.vlan2_out.pbit_mask == 0xff)
					mod_entry += (pkt_mod->vlan_mod.vlan2_out.pbit + 1);
			} else {
				if (pkt_mod->vlan_mod.vlan1_out.pbit_mask == 0xff)
					mod_entry += (pkt_mod->vlan_mod.vlan1_out.pbit + 1);
			}

			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);

			/* when ds load balance on G0 and G1 is enabled, need to duplicate DS PMT on G0/1 */
			tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
			if (    (TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable)
			     && (TPM_ENUM_GMAC_0 == trg_gmac || TPM_ENUM_GMAC_1 == trg_gmac)) {

				/* if this is DS and DS_MAC_BASED_TRUNK is ENABLED */
				if (trg_gmac == TPM_ENUM_GMAC_0)
					duplicate_gmac = TPM_ENUM_GMAC_1;
				else
					duplicate_gmac = TPM_ENUM_GMAC_0;

				ret_code = tpm_mod2_split_mod_create_l2_pmts(duplicate_gmac, pkt_mod, false);
				if (TPM_RC_OK != ret_code)
				{
					TPM_OS_ERROR(TPM_MODZ2_HM_MOD,
						"failed to add pmt split mod stage-1 for duplicate GMAC\n");
					tpm_mod2_split_mod_try_pmt_entry_del(TPM_L2_PRIM_ACL, trg_gmac, mod_entry_tmp);
					return(TPM_FAIL);
				}

			}
		} else if((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
							(TPM_VLAN_MOD == pkt_mod_bm) &&
							(VLANOP_NOOP == pkt_mod->vlan_mod.vlan_op) &&
							(parse_flags_bm & TPM_PARSE_FLAG_TAG1_FALSE)) {
			/*In L2 range, all untag PNC with VLANOP_NOOP will set PMT to 16*/
			mod_entry = 16;
		} else {
		    /*********** Create Modification Entries **********/
			ret_code =
			tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm,
					    0 /*int_mod_bm */ ,
					    &mod_entry, &trg_gmac);
			IF_ERROR(ret_code);
		}
	}

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_l2_tcam_build(src_port, dir, rule_num, l2_key, parse_rule_bm,
					      parse_flags_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_l2_sram_build(src_port, dir, rule_num, pon_type, pkt_frwd, rule_action,
					      mod_entry, &(pnc_data.pncl_sram), l2_key, pkt_mod, pkt_mod_bm);
	IF_ERROR(int_ret_code);

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_L2_PRIM_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
	/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_L2_PRIM_ACL, rule_num, update_sram_only, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/
	if (!update_sram_only) {
		/* Set PNC API data */
		api_data.l2_prim_key.src_port = src_port;
		api_data.l2_prim_key.parse_rule_bm = parse_rule_bm;
		api_data.l2_prim_key.parse_flags_bm = parse_flags_bm;
		api_data.l2_prim_key.pkt_mod_bm = pkt_mod_bm;
		memcpy(&(api_data.l2_prim_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
		if (l2_key)
			memcpy(&(api_data.l2_prim_key.l2_key), l2_key, sizeof(tpm_l2_acl_key_t));

		if (pkt_frwd != NULL)
			memcpy(&(api_data.l2_prim_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
		else
			memset(&(api_data.l2_prim_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

		if (pkt_mod != NULL)
			memcpy(&(api_data.l2_prim_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
		else
			memset(&(api_data.l2_prim_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

		/* Set API modification data */
		tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

		/* Set Pnc Connection data */
		pnc_conn.num_pnc_ranges = 1;
		pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
		pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

		if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
			/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
			if (rule_num < api_rng_entries) {
				int_ret_code =
				tpm_proc_api_entry_rulenum_inc(TPM_L2_PRIM_ACL, rule_num, (api_rng_entries - 1));
				IF_ERROR(int_ret_code);
			}
		}

		/* Set new API Entry */
		int_ret_code = tpm_db_api_entry_set(TPM_L2_PRIM_ACL, rule_num, 0 /*bi_dir */ ,
						    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
		IF_ERROR(int_ret_code);

		/* Return Output */
		*rule_idx = l_rule_idx;

		/* Set aging counter group nunmber and msk */
		int_ret_code = tpm_db_pnc_get_lu_conf(TPM_PNC_L2_MAIN, &lu_conf);
		IF_ERROR(int_ret_code);
		tpm_tcam_set_lu_mask(pnc_entry, (int32_t) lu_conf.lu_mask);
		tpm_tcam_set_cntr_group(pnc_entry, (int32_t) lu_conf.cntr_grp);

		/* alloc rule index if spec mc vlan */
		if ((rule_action->pkt_act & TPM_ACTION_SPEC_MC_VID) && (dir == TPM_DIR_DS)) {
			ret_code = tpm_db_mc_vlan_set_ai_bit_pnc_index(l2_key->vlan1.vid, l_rule_idx);
			IF_ERROR(ret_code);
		}
	} else {
		if (pkt_frwd != NULL)
			memcpy(&(api_data.l2_prim_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
		else
			memset(&(api_data.l2_prim_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

		int_ret_code = tpm_db_api_entry_invalidate(TPM_L2_PRIM_ACL, rule_num);
		IF_ERROR(int_ret_code);

		/* Set new API Entry */
		int_ret_code = tpm_db_api_entry_set(TPM_L2_PRIM_ACL, rule_num, 0 /*bi_dir */ ,
						    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
		IF_ERROR(int_ret_code);

		/* Return Output */
		*rule_idx = l_rule_idx;
	}

	return(TPM_RC_OK);
}


/*******************************************************************************
* tpm_proc_add_ds_load_balance_acl_rule()
*
* DESCRIPTION:    Main function for adding DS load balance API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_ds_load_balance_acl_rule(uint32_t owner_id,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l2_acl_key_t *l2_key,
					       tpm_ds_load_balance_tgrt_t tgrt_port)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	uint32_t bi_dir;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_api_lu_conf_t lu_conf;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d), rule_num(%d)\n", owner_id, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ds_load_balance_check(owner_id, rule_num, parse_rule_bm, parse_flags_bm, l2_key);
	IF_ERROR(ret_code);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_DS_LOAD_BALANCE_ACL, &range_id);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Only do it in table mode */
	if (TPM_RANGE_TYPE_TABLE == rangConf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_DS_LOAD_BALANCE_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) , rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ds_load_balance_acl_rule(owner_id, rule_num, TPM_INT_CALL);
			IF_ERROR(ret_code);
		} else {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) , rule_num(%d) is a new entry\n", owner_id, rule_num);
		}
	}
	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	int_ret_code = tpm_proc_ds_load_balance_tcam_build(rule_num, l2_key,
				parse_rule_bm, parse_flags_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ds_load_balance_sram_build(rule_num, &(pnc_data.pncl_sram), l2_key, tgrt_port);
	IF_ERROR(int_ret_code);

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_DS_LOAD_BALANCE_ACL, rule_num, &pnc_data,
					&pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
	/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_DS_LOAD_BALANCE_ACL, rule_num, 0,
						&pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/
	/* Set PNC API data */
	api_data.l2_prim_key.parse_rule_bm = parse_rule_bm;
	if (l2_key)
		memcpy(&(api_data.l2_prim_key.l2_key), l2_key, sizeof(tpm_l2_acl_key_t));

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			int_ret_code =
			tpm_proc_api_entry_rulenum_inc(TPM_DS_LOAD_BALANCE_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_DS_LOAD_BALANCE_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	/* Set aging counter group nunmber and msk */
	int_ret_code = tpm_db_pnc_get_lu_conf(TPM_PNC_DS_LOAD_BALANCE, &lu_conf);
	IF_ERROR(int_ret_code);
	tpm_tcam_set_lu_mask(pnc_entry, (int32_t) lu_conf.lu_mask);
	tpm_tcam_set_cntr_group(pnc_entry, (int32_t) lu_conf.cntr_grp);

	return(TPM_RC_OK);
}


/*******************************************************************************
* tpm_proc_del_ds_load_balance_acl_rule()
*
* DESCRIPTION:    Main function for deleting DS load balance API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_ds_load_balance_acl_rule(uint32_t owner_id, uint32_t rule_idx, uint32_t ext_call)
{

	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_DS_LOAD_BALANCE_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_pnc_create(tpm_pnc_ranges_t range_id, uint32_t pnc_entry, tpm_pncl_pnc_full_t *pnc_data)
{

	int32_t int_ret_code;

	/* Create Entry in PnC */
	int_ret_code = tpm_pncl_entry_set(pnc_entry, pnc_data);
	IF_ERROR(int_ret_code);

	/* Decrease number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_dec(range_id);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_l2_num_vlan_tags_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_l2_num_vlan_tags_init(void)
{
	int32_t int_ret_code;
	uint32_t free_entries, l2_lu, pnc_entry, dtag_entries, exp_entry;
	tpm_db_pnc_range_t range_data;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_init_tpid_opt_t tpid_opt;
	tpm_init_double_tag_t dbl_tag;
	uint32_t i;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&tpid_opt, 0, sizeof(tpm_init_tpid_opt_t));

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_NUM_VLAN_TAGS, &range_data);
	IF_ERROR(int_ret_code);
	l2_lu = range_data.pnc_range_conf.base_lu_id;
	free_entries = range_data.pnc_range_oper.free_entries;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "free_ent(%d)\n", free_entries);

	/* Get tag TPID options */
	int_ret_code = tpm_db_default_tag_tpid_get(&tpid_opt);
	IF_ERROR(int_ret_code);

	if (tpid_opt.opt_num == 0) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "none vlan tpid option! \n");
		IF_ERROR(ERR_GENERAL);
	}

	dtag_entries = 0;
	for (i = 0; i < tpid_opt.opt_num; i++) {
		if (tpid_opt.opt[i].v2_tpid != MV_TPM_UN_INITIALIZED_INIT_PARAM)
			dtag_entries++;
	}

	if (dtag_entries == tpid_opt.opt_num) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "none single tag tpid option! \n");
		IF_ERROR(ERR_GENERAL);
	}

	exp_entry = tpid_opt.opt_num;

	tpm_db_double_tag_support_get(&dbl_tag);
	if (dbl_tag == TPM_DOUBLE_TAG_DISABLED)
		exp_entry -= dtag_entries;

	if ((exp_entry+1) > free_entries) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "not enough tag_num rules! (%d/%d)\n", free_entries, tpid_opt.opt_num+1);
		IF_ERROR(ERR_GENERAL);
	} else if ((exp_entry+1) < free_entries)
		TPM_OS_WARN(TPM_TPM_LOG_MOD, "too much tag_num rules, waste. (%d/%d)\n", free_entries, tpid_opt.opt_num+1);

	pnc_entry = range_data.pnc_range_conf.range_start+range_data.pnc_range_conf.api_start;
	/* Basic TCAM/SRAM Config, valid for all following entries */
	/* L2 LUID */
	pnc_data.pncl_tcam.lu_id = range_data.pnc_range_conf.base_lu_id;
	/* Bi-direction */
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	/* Current Offset */
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;
	/* Set parse type for key */
	pnc_data.pncl_tcam.l3_parse_bm = 0;
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;
	pnc_data.pncl_tcam.ipv6_parse_bm = 0;
	pnc_data.pncl_tcam.tcp_parse_bm = 0;
	/* Set DO_NOT_REPEAT_TAG AI filter */
	pnc_data.pncl_tcam.add_info_data = 0 << TPM_AI_DNRT_BIT_OFF;
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_DNRT_MASK;
	/* Next LUID is still L2 */
	pnc_data.pncl_sram.next_lu_id = range_data.pnc_range_conf.base_lu_id;
	/* Using off_reg 1 */
	pnc_data.pncl_sram.next_lu_off_reg = 1;
	/* Update off_reg 0 -> ETY */
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv4_subf = TPM_L2_PARSE_ETYPE;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_ETY_REG;
	/* No queue update */
	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	/* No ResultInfo&FlowId update */
	pnc_data.pncl_sram.sram_updt_bm = 0;


	/*************************************/
	/* CREATE ENTRIES FOR DOUBLE TAGGED **/
	/*************************************/

	if (dbl_tag == TPM_DOUBLE_TAG_ENABLED) {
		/* Set L2 key value */
		pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_TWO_VLAN_TAG;

		/* Set DO_NOT_REPEAT_TAG AI filter */
		pnc_data.pncl_sram.add_info_data = 1 << TPM_AI_DNRT_BIT_OFF;
		pnc_data.pncl_sram.add_info_mask = TPM_AI_DNRT_MASK;
		/* Set DOUBLE_TAG AI filter */
		pnc_data.pncl_sram.add_info_data |= 1 << TPM_AI_TAG1_BIT_OFF;
		pnc_data.pncl_sram.add_info_mask |= TPM_AI_TAG1_MASK;
		pnc_data.pncl_sram.add_info_data |= 1 << TPM_AI_TAG2_BIT_OFF;
		pnc_data.pncl_sram.add_info_mask |= TPM_AI_TAG2_MASK;

		/* Set VLAN Result_Info bit */
		pnc_data.pncl_sram.sram_updt_bm  |= TPM_PNCL_SET_TAGGED;

		for (i = 0; i < tpid_opt.opt_num; i++) {
			if (tpid_opt.opt[i].v2_tpid != MV_TPM_UN_INITIALIZED_INIT_PARAM) {
				/* Set TPID of vlan tag */
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.tpid = tpid_opt.opt[i].v1_tpid;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.tpid_mask = 0xffff;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.pbit_mask = 0;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.cfi_mask = 0;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.vid_mask = 0;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.tpid = tpid_opt.opt[i].v2_tpid;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.tpid_mask = 0xffff;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.pbit_mask = 0;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.cfi_mask = 0;
				pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.vid_mask = 0;

				/* Create Entry in PnC */
				int_ret_code = tpm_proc_pnc_create(TPM_PNC_NUM_VLAN_TAGS, pnc_entry, &pnc_data);
				IF_ERROR(int_ret_code);
				free_entries--;
				pnc_entry++;
			}
		}
	}


	/*************************************/
	/* CREATE ENTRIES FOR SINGLE TAGGED **/
	/*************************************/

	/* Set L2 key value */
	pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_ONE_VLAN_TAG;

	/* Set DO_NOT_REPEAT_TAG AI filter */
	pnc_data.pncl_sram.add_info_data = 1 << TPM_AI_DNRT_BIT_OFF;
	pnc_data.pncl_sram.add_info_mask = TPM_AI_DNRT_MASK;
	/* Set SINGLE_TAG AI filter */
	pnc_data.pncl_sram.add_info_data &= (~TPM_AI_TAG2_MASK);
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TAG2_MASK;
	pnc_data.pncl_sram.add_info_data |= 1 << TPM_AI_TAG1_BIT_OFF;
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TAG1_MASK;

	for (i = 0; i < tpid_opt.opt_num; i++) {
		if (tpid_opt.opt[i].v2_tpid == MV_TPM_UN_INITIALIZED_INIT_PARAM) {
			/* Set TPID of vlan tag */
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.tpid = tpid_opt.opt[i].v1_tpid;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.tpid_mask = 0xffff;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.pbit_mask = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.cfi_mask = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.vid_mask = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.tpid = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.tpid_mask = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.pbit_mask = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.cfi_mask = 0;
			pnc_data.pncl_tcam.pkt_key.l2_key.vlan2.vid_mask = 0;

			/* Create Entry in PnC */
			int_ret_code = tpm_proc_pnc_create(TPM_PNC_NUM_VLAN_TAGS, pnc_entry, &pnc_data);
			IF_ERROR(int_ret_code);
			free_entries--;
			pnc_entry++;
		}
	}

	/*************************************/
	/* CREATE ENTRIES FOR UNTAGGED      **/
	/*************************************/

	/* Set L2 key value */
	pnc_data.pncl_tcam.l2_parse_bm = 0;

	/* Set DO_NOT_REPEAT_TAG AI filter */
	pnc_data.pncl_sram.add_info_data = 1 << TPM_AI_DNRT_BIT_OFF;
	pnc_data.pncl_sram.add_info_mask = TPM_AI_DNRT_MASK;
	/* Set ZERO_TAG AI filter */
	pnc_data.pncl_sram.add_info_data &= (~TPM_AI_TAG2_MASK);
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TAG2_MASK;
	pnc_data.pncl_sram.add_info_data &= (~TPM_AI_TAG1_MASK);
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TAG1_MASK;

	/* Do NOT set VLAN Result_Info bit */
	pnc_data.pncl_sram.sram_updt_bm = 0;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_NUM_VLAN_TAGS, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_proto_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_ipv4_proto_init(void)
{

	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry, ipv4_proto_lu, ipv4_frag_lu;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get NExt range IPV4_FRAG lu_id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_FRAG, &range_data);
	IF_ERROR(int_ret_code);
	ipv4_frag_lu = range_data.pnc_range_conf.base_lu_id;

	/* Get IPV4_PROTO lu_id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_PROTO, &range_data);
	IF_ERROR(int_ret_code);
	ipv4_proto_lu = range_data.pnc_range_conf.base_lu_id;

	/* Get pnc_range tcam_start_entry, and number of free entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start;

	/*************************************/
	/* CREATE ENTRIES FOR TCP UDP and    */
	/* other ipv4 proto                ** */
	/*************************************/

	/* Basic TCAM/SRAM Config, valid for all following entries */
	pnc_data.pncl_tcam.lu_id = ipv4_proto_lu;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_PROTO;

	pnc_data.pncl_tcam.add_info_data = (0 << TPM_AI_NO_PROTO_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_NO_PROTO_MASK;

	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	/* Update dummy register (offset automatically=zero) */
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_L4;
	pnc_data.pncl_sram.next_lu_off_reg = TPM_PNC_LU_REG0;
	pnc_data.pncl_sram.next_lu_id = ipv4_frag_lu;

	/* Do not repeat this stage again */
	pnc_data.pncl_sram.add_info_data = (1 << TPM_AI_NO_PROTO_BIT_OFF);
	pnc_data.pncl_sram.add_info_mask = TPM_AI_NO_PROTO_MASK;

	/***********************************************************/
	/* Create  TCP, for  packets                               */
	/***********************************************************/

	NO_FREE_ENTRIES();

	pnc_data.pncl_tcam.pkt_key.ipv4_key.ipv4_proto = IPPROTO_TCP;

	/* Build SRAM Entry */
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_TCP;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_PROTO, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/***********************************/
	/* Create  UDP                     */
	/***********************************/
	NO_FREE_ENTRIES();
	/* Build Tcam Entry */
	pnc_data.pncl_tcam.pkt_key.ipv4_key.ipv4_proto = IPPROTO_UDP;

	/* Build SRAM Entry */
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_UDP;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_PROTO, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/**************************************/
	/* Create Others                      */
	/**************************************/
	NO_FREE_ENTRIES();
	/* Build Tcam Entry */

	pnc_data.pncl_tcam.ipv4_parse_bm = 0;

	/* Build SRAM Entry */
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_PROTO, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_frag_opt_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_ipv4_frag_init(void)
{
	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry, ipv4_frag_lu, ipv4_len_lu;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get Next Range IPV4 LEN Lookup */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_LEN, &range_data);
	IF_ERROR(int_ret_code);
	ipv4_len_lu = range_data.pnc_range_conf.base_lu_id;

	/* Get Range IPV4 FRAG Lookup */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_FRAG, &range_data);
	IF_ERROR(int_ret_code);
	ipv4_frag_lu = range_data.pnc_range_conf.base_lu_id;

	/* Get number of entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "free_ent(%d)\n", free_entries);

	/* Basic TCAM/SRAM Config, valid for all following entries */
	pnc_data.pncl_tcam.lu_id = ipv4_frag_lu;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;

	/* frag_offset an MF TCAM keys */
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_flags = 0;
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_flags_mask = 1;
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_frag_offset = 0;
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_frag_offset_mask = 1;

	/* Only look at ai_no_frag=0 */
	pnc_data.pncl_tcam.add_info_data = (0 << TPM_AI_NO_FRAG_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_NO_FRAG_MASK;

	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	/* Update dummy register (offset automatically=zero) */
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.next_lu_off_reg = TPM_PNC_LU_REG0;
	pnc_data.pncl_sram.next_lu_id = ipv4_len_lu;

	/* Signal the packet is not going through this range again */
	pnc_data.pncl_sram.add_info_data = (1 << TPM_AI_NO_FRAG_BIT_OFF);
	pnc_data.pncl_sram.add_info_mask = TPM_AI_NO_FRAG_MASK;

	/* Set fragment information */
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_L3;

	/*********************************************/
	/* Create to CPU Unfragmented packets        */
	/*********************************************/

	NO_FREE_ENTRIES();

	/*** Build Tcam Entry ***/

	/* Only Unfragmented MF=0, frag_offset=0 */
	pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_FLAG_MF | TPM_IPv4_PARSE_FRAG_OFFSET;

	/* Only to_cpu packets */
	pnc_data.pncl_tcam.add_info_data |= (1 << TPM_AI_TO_CPU_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask |= TPM_AI_TO_CPU_MASK;

	/*** Build SRAM Entry ***/

	/* Packet not fragmented */
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_IPV4_NFRAG;

	/* LookupDone - it is to CPU */
	pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_LUD;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_FRAG, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/***********************************************************/
	/* Create to CPU Frag packets                              */
	/***********************************************************/
	NO_FREE_ENTRIES();

	/* Don't check fragmented */
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;

	/*** Build SRAM Entry ***/

	/* Packet=fragmented */
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_IPV4_FRAG;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_FRAG, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/**************************************/
	/* Create Non-CPU Non-Frag,           */
	/**************************************/
	NO_FREE_ENTRIES();

	/*** Build Tcam Entry ***/

	/* MF=0, frag_offset=0 */
	pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_FLAG_MF | TPM_IPv4_PARSE_FRAG_OFFSET;

	/* Don't check CPU/Non-CPU  */
	pnc_data.pncl_tcam.add_info_mask &= (~TPM_AI_TO_CPU_MASK);

	/*** Build SRAM Entry ***/

	/* Lookup not done, it is not to CPU */
	pnc_data.pncl_sram.sram_updt_bm &= (~TPM_PNCL_SET_LUD);

	/* Packet=not_fragmented */
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_IPV4_NFRAG;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_FRAG, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/***********************************************************/
	/* Create Non-CPU Frag packets                            */
	/***********************************************************/
	NO_FREE_ENTRIES();

	/* Don't check fragmented */
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;

	/*** Build SRAM Entry ***/

	/* Packet=fragmented */
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_IPV4_FRAG;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_FRAG, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_tcp_flag_pnc_entry_create()
*
* DESCRIPTION:
*
* INPUTS:
* ip_ver      - IP version, IPV4 or IPV6
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_tcp_flag_pnc_entry_create(tpm_ip_ver_t ip_ver)
{
	int32_t int_ret_code;
	uint32_t free_entries, tcp_flag_lu, ttl_lu = 0, pnc_entry;
	int32_t cpu_rx_queue;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_ranges_t   range_id;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "\n");

	/* Get default CPU Rx queue */
	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);

	if (TPM_IP_VER_4 == ip_ver) {
		range_id = TPM_PNC_IPV4_TCP_FLAG;

		/* Get Next Range TTL Info, only for IPv4 */
		int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_TTL, &range_data);
		if (int_ret_code != TPM_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " To create TCP Flag range, TPM_PNC_TTL range must exist \n");
			return(int_ret_code);
		}
		ttl_lu = range_data.pnc_range_conf.base_lu_id;
	} else
		range_id = TPM_PNC_IPV6_TCP_FLAG;

	/* Get TCP_FLAG Range Info */
	int_ret_code = tpm_db_pnc_rng_get(range_id, &range_data);
	IF_ERROR(int_ret_code);
	tcp_flag_lu = range_data.pnc_range_conf.base_lu_id;

	/* Set common TCAM params */
	pnc_data.pncl_tcam.lu_id = tcp_flag_lu;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_TCP_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.tcp_subf = TPM_PARSE_L4_SRC;

	/* Set common SRAM params */
	if (TPM_IP_VER_4 == ip_ver) {
		pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_TCP_OFFSET;
		pnc_data.pncl_sram.next_offset.offset_sub.tcp_subf = TPM_PARSE_TCPFLAGS;
		pnc_data.pncl_sram.next_lu_id = ttl_lu;
		pnc_data.pncl_sram.next_lu_off_reg = TPM_PNC_LU_REG0;
	} else {
		pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD;
	}

	pnc_data.pncl_sram.pnc_queue = cpu_rx_queue;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Get pnc_range tcam_start_entry, and number of free entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start;

	/********************/
	/* Create FIN FLAG=1 */
	/*******************/
	NO_FREE_ENTRIES();

	/* Build PnC Entry */
	/* Double check - only check packets that are MTM */
	pnc_data.pncl_tcam.add_info_data |= (1 << TPM_AI_MTM_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask |= TPM_AI_MTM_MASK;
	if (TPM_IP_VER_6 == ip_ver) {
		pnc_data.pncl_tcam.add_info_data &= ~(TPM_AI_L4P_MASK);
		pnc_data.pncl_tcam.add_info_mask |= TPM_AI_L4P_MASK;
	}

	pnc_data.pncl_tcam.tcp_parse_bm = TPM_PARSE_TCPFLAGS;
	pnc_data.pncl_tcam.pkt_key.tcp_key.tcp_flags = TPM_TCP_FIN;
	pnc_data.pncl_tcam.pkt_key.tcp_key.tcp_flags_mask = TPM_TCP_FIN;

	/* Build SRAM Entry */
	pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_TXP;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

	if (TPM_IP_VER_4 == ip_ver) {
		/* Signal the packet is going to CPU */
		pnc_data.pncl_sram.add_info_data |= (1 << TPM_AI_TO_CPU_BIT_OFF);
		pnc_data.pncl_sram.add_info_mask |= TPM_AI_TO_CPU_MASK;
	}

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(range_id, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/********************/
	/* Create RES FLAG=1 */
	/********************/
	NO_FREE_ENTRIES();
	/* Build PnC Entry (similar to previous entry */
	pnc_data.pncl_tcam.pkt_key.tcp_key.tcp_flags = TPM_TCP_RES;
	pnc_data.pncl_tcam.pkt_key.tcp_key.tcp_flags_mask = TPM_TCP_RES;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(range_id, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	if (TPM_IP_VER_6 == ip_ver) {
		/* no default rule for IPv6 */
		return(TPM_RC_OK);
	}
	/***************/
	/* All Others  */
	/***************/
	NO_FREE_ENTRIES();

	/* Build TCAM Entry */
	pnc_data.pncl_tcam.add_info_data = 0;
	pnc_data.pncl_tcam.add_info_mask = 0;
	pnc_data.pncl_tcam.tcp_parse_bm = 0;
	pnc_data.pncl_tcam.pkt_key.tcp_key.tcp_flags = 0;
	pnc_data.pncl_tcam.pkt_key.tcp_key.tcp_flags_mask = 0;

	/* Build SRAM Entry */
	pnc_data.pncl_sram.sram_updt_bm = 0;
	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(range_id, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;
	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_tcp_flag_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_tcp_flag_init(void)
{
	int32_t int_ret_code;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_tcp_flag_pnc_entry_create(TPM_IP_VER_4);
	IF_ERROR(int_ret_code);

	int_ret_code = tpm_proc_tcp_flag_pnc_entry_create(TPM_IP_VER_6);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}


/*******************************************************************************
* tpm_proc_virt_uni_init()
*
* DESCRIPTION:
* The function is responsible for adding a new entry in the PNC range of TPM_PNC_VIRT_UNI.
* The entry is added in case the xml parameters are defining this entry - even though the
* wifi_virt_uni_enable feature is disabled.
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_virt_uni_init(void)
{

	int32_t int_ret_code;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_db_pnc_range_t range_data;
	uint32_t free_entries, pnc_entry;
	tpm_init_virt_uni_t virt_uni;
	tpm_db_gmac_func_t g_func;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " WiFi entry \n");


	/* TODO: Currently hardcoded to G1 */
	tpm_db_gmac_func_get(TPM_ENUM_GMAC_1, &g_func);

	if (g_func != TPM_GMAC_FUNC_VIRT_UNI) {
		return(TPM_OK);
	}

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_VIRT_UNI, &range_data);
	IF_ERROR(int_ret_code);
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry =
	range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start +
	(range_data.pnc_range_conf.range_size - 1);

	tpm_db_virt_info_get(&virt_uni);

	/* Build PnC Entry */
	pnc_data.pncl_tcam.lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.pncl_tcam.port_ids = gmac_pnc_bm[TPM_ENUM_GMAC_1];
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	if (virt_uni.enabled == 1) {
		NO_FREE_ENTRIES();

		/* Build SRAM Entry */
		pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_TXP | TPM_PNCL_SET_LUD;
		pnc_data.pncl_sram.pnc_queue = 0;	/* hardcoded - send to Q0 to CPU */
		pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

		/* Create Entry in PnC */
		int_ret_code = tpm_proc_pnc_create(TPM_PNC_VIRT_UNI, pnc_entry, &pnc_data);
		IF_ERROR(int_ret_code);

		free_entries--;
		pnc_entry--;

		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0) {
			NO_FREE_ENTRIES();

			pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_MAC_DA;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[0] = 0x01;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[0] = 0xff;

			/* Build SRAM Entry */
			pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD;
			pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

			/* Create Entry in PnC */
			int_ret_code = tpm_proc_pnc_create(TPM_PNC_VIRT_UNI, pnc_entry, &pnc_data);
			IF_ERROR(int_ret_code);

			free_entries--;
			pnc_entry--;

			NO_FREE_ENTRIES();

			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[0] = 0x33;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[0] = 0xff;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[1] = 0x33;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[1] = 0xff;

			/* Create Entry in PnC */
			int_ret_code = tpm_proc_pnc_create(TPM_PNC_VIRT_UNI, pnc_entry, &pnc_data);
			IF_ERROR(int_ret_code);

			free_entries--;
			pnc_entry--;

			NO_FREE_ENTRIES();

			pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_MAC_DA;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[0] = 0x01;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[1] = 0x00;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[2] = 0x5e;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[3] = 0x00;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[4] = 0x00;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[5] = 0x01;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[0] = 0xff;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[1] = 0xff;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[2] = 0xff;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[3] = 0xff;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[4] = 0xff;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da_mask[5] = 0xff;

			/* Build SRAM Entry */
			pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_TXP | TPM_PNCL_SET_LUD;
			pnc_data.pncl_sram.pnc_queue = 1;	/* hardcoded - send to Q1 to CPU */
			pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

			/* Create Entry in PnC */
			int_ret_code = tpm_proc_pnc_create(TPM_PNC_VIRT_UNI, pnc_entry, &pnc_data);
			IF_ERROR(int_ret_code);

			free_entries--;
			pnc_entry--;

			NO_FREE_ENTRIES();

			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[0] = 0x33;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[1] = 0x33;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[2] = 0x00;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[3] = 0x00;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[4] = 0x00;
			pnc_data.pncl_tcam.pkt_key.l2_key.mac.mac_da[5] = 0x01;

			/* Create Entry in PnC */
			int_ret_code = tpm_proc_pnc_create(TPM_PNC_VIRT_UNI, pnc_entry, &pnc_data);
			IF_ERROR(int_ret_code);

		}
	}

	return(TPM_RC_OK);
}
/*******************************************************************************
* tpm_proc_ds_load_balance_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_ds_load_balance_init(void)
{
	int32_t ret_code;
	tpm_pnc_all_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data, nextphase_range_data;
	uint32_t pnc_entry;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pnc_all_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&nextphase_range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get Range data */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_DS_LOAD_BALANCE, &range_data);
	IF_ERROR(ret_code);

	/* Get Next Range data */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_L2_MAIN, &nextphase_range_data);
	IF_ERROR(ret_code);

	/* Get pnc_range tcam_start_entry, and number of free entries */
	pnc_entry = range_data.pnc_range_conf.range_end;

	/* Set common TCAM params */
	pnc_data.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.tcam_entry.port_ids = TPM_BM_PMAC;

	/* Set common SRAM params */
	pnc_data.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.sram_entry.next_lu_id = nextphase_range_data.pnc_range_conf.base_lu_id;

	/* do not repeat this section again */
	pnc_data.tcam_entry.add_info_data = 0;
	pnc_data.tcam_entry.add_info_mask = TPM_AI_DNRT_DS_TRUNK_MASK;

	/* Packet forwarded to GMAC0 */
	pnc_data.sram_entry.flowid_updt_mask = TPM_TXP_FL_UPDT_MASK;
	pnc_data.sram_entry.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	pnc_data.sram_entry.flowid_val = (TPM_PNC_TRG_GMAC0 << TPM_TXP_FL_SHIFT);

	pnc_data.sram_entry.add_info_data |= (1 << TPM_AI_DNRT_DS_TRUNK_BIT_OFF);
	pnc_data.sram_entry.add_info_mask |= TPM_AI_DNRT_DS_TRUNK_MASK;

	/* create default rule to frwd all DS packets to GMAC0 */
	ret_code = tpm_pnc_set(pnc_entry, 0, &pnc_data);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(pnc_entry, &pnc_data);
	IF_ERROR(ret_code);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_DS_LOAD_BALANCE);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}


/*******************************************************************************
* tpm_proc_ipv4_ttl_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_ipv4_ttl_init(uint32_t ttl_illegal_action)
{

	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry, ttl_lu, ipv4_proto_lu;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data, nextphase_range_data;
	int32_t cpu_rx_queue;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&nextphase_range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "ttl_illegal_action(%d)\n", ttl_illegal_action);

	/* Get default CPU Rx queue */
	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);

	/* Get TCP_FLAG Range Info */
	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_TTL, &range_data);
	IF_ERROR(int_ret_code);
	ttl_lu = range_data.pnc_range_conf.base_lu_id;

	/* Get Next Range IPV4_PROTO LU_ID */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_PROTO, &nextphase_range_data);
	IF_ERROR(int_ret_code);
	ipv4_proto_lu = nextphase_range_data.pnc_range_conf.base_lu_id;

	/* Get pnc_range tcam_start_entry, and number of free entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start;

	/* Set common TCAM params */
	pnc_data.pncl_tcam.lu_id = ttl_lu;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_ttl = 0;

	/* Set common SRAM params */
	pnc_data.pncl_sram.next_offset.offset_base = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.next_lu_id = ipv4_proto_lu;

	/******************/
	/* Create TTL=0   */
	/******************/
	/* Parse the TTL */
	pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_TTL;

	/* Double check - check only packets that are MTM */
	pnc_data.pncl_tcam.add_info_data = (1 << TPM_AI_MTM_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_MTM_MASK;

	/* Packet forwarded to CPU */
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_TXP;
	pnc_data.pncl_sram.pnc_queue = cpu_rx_queue;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

	/* or packet is dropped */
	if (TPM_TTL_ZERO_ACTION_DROP == ttl_illegal_action)
		pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD;

	/* Signal the packet is going to CPU */
	pnc_data.pncl_sram.add_info_data |= (1 << TPM_AI_TO_CPU_BIT_OFF);
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TO_CPU_MASK;

	NO_FREE_ENTRIES();
	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_TTL, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/******************/
	/* Create TTL=1   */
	/******************/
	NO_FREE_ENTRIES();
	/* Build PnC Entry, see TTL=0 */
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_ttl = 1;

	/* Build SRAM Entry, see TTL=0,
	   but always send to CPU, don't drop */
	pnc_data.pncl_sram.sram_updt_bm &= (~(TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD));

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_TTL, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/********************/
	/* Create TTL>1     */
	/********************/
	/* All packets */
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;
	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	/* Don't update anything (next_lu_id same as previous) */
	pnc_data.pncl_sram.sram_updt_bm = 0;
	pnc_data.pncl_sram.add_info_data = 0;
	pnc_data.pncl_sram.add_info_mask = 0;

	NO_FREE_ENTRIES();
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_TTL, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_igmp_init()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_ipv4_igmp_init(void)
{
	tpm_error_code_t ret_code = TPM_RC_OK;
	uint32_t mode;
	uint32_t cpu_queue;
	tpm_src_port_type_t src_port;

	/*set cpu queue */
	tpm_db_igmp_get_cpu_queue(&cpu_queue);
	tpm_set_igmp_cpu_rx_queue(cpu_queue);

	/*set igmp packet forward mode */
	for (src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_UNI_7; src_port++) {
		if (tpm_db_eth_port_switch_port_get(src_port) == TPM_DB_ERR_PORT_NUM)
       		continue;

		tpm_db_igmp_get_port_frwd_mode(src_port, &mode);
		ret_code = tpm_proc_add_uni_port_igmp_rule(src_port, mode);
		IF_ERROR(ret_code);
	}

	tpm_db_igmp_get_port_frwd_mode(TPM_SRC_PORT_WAN, &mode);
	ret_code = tpm_proc_add_wan_port_igmp_rule(mode);
	IF_ERROR(ret_code);

	return ret_code;
}

/*******************************************************************************
* tpm_proc_mib_reset()
*
* DESCRIPTION:      Performs MIB reset
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_mib_reset(uint32_t owner_id, tpm_reset_level_enum_t reset_level)
{
	int32_t int_ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "\n");

	if (reset_level > TPM_ENUM_MAX_RESET_LEVEL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Illegal reset_level (%d) \n", reset_level);
		return(ERR_GENERAL);
	}
	/* Init CPU WAN loopback DB */
	int_ret_code = tpm_proc_init_cpu_wan_loopback();

	/* Init Multicast DB */
	int_ret_code = tpm_proc_multicast_reset();
	IF_ERROR(int_ret_code);

	/* Init PnC Ranges in DB */
	int_ret_code = tpm_init_pncranges_set(reset_level);
	IF_ERROR(int_ret_code);

	/* Init API Ranges in DB */
	int_ret_code = tpm_init_api_rng_init_all();
	IF_ERROR(int_ret_code);

	/* Init IPV6 5t flows */
	int_ret_code = tpm_db_init_ipv6_5t_flow_reset();
	IF_ERROR(int_ret_code);

	/* Init CnM DB */
	int_ret_code = tpm_db_ctc_cm_db_reset();
	IF_ERROR(int_ret_code);

	/* Init IPv6 MC SIP db */
	tpm_db_ipv6_mc_sip_db_reset();

	/* Init Hardware */
	int_ret_code = tpm_init_system_mib_reset(reset_level);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_int_del_acl_rule(tpm_api_sections_t api_section, uint32_t owner_id, uint32_t rule_idx)
{

	int32_t ret_code;
	uint32_t rule_num, dummy_idx, bi_dir, api_rng_entries;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con = { 0, 0};
	tpm_db_pnc_conn_t pnc_con;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_pnc_ranges_t range_id = 0;

	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));
	memset(&api_data, 0, sizeof(tpm_rule_entry_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " api_section(%d) owner(%d) rule_idx (%d)", api_section, owner_id, rule_idx);

	/* Get Range_Id */
	ret_code = tpm_db_api_section_main_pnc_get(api_section, &range_id);
	IF_ERROR(ret_code);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Get the rule_num */
	ret_code = tpm_db_api_rulenum_get(api_section, rule_idx, &rule_num);
	if (ret_code == TPM_DB_ERR_REC_NOT_EXIST) {
		TPM_OS_ERROR(TPM_DB_MOD, " The rule non-exist!\n");
		return ERR_RULE_IDX_INVALID;
	}
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "with rule_num(%d)\n", rule_num);
	/*IF_ERROR(ret_code);*/

	ret_code = tpm_db_api_entry_get(api_section, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
	IF_ERROR(ret_code);

	ret_code = tpm_db_api_section_num_entries_get(api_section, &api_rng_entries);
	IF_ERROR(ret_code);

	/* Delete PNC Entry */
	ret_code = tpm_proc_pnc_con_del(&pnc_con);
	IF_ERROR(ret_code);

	/* Release related modification entry  (Important, after deleting PNC Entries) */
	if (mod_con.mod_cmd_mac != TPM_INVALID_GMAC) {
		/* handle split mod */
		ret_code = tpm_mod2_split_mod_try_pmt_entry_del(api_section, mod_con.mod_cmd_mac, mod_con.mod_cmd_ind);
		if (TPM_OK != ret_code) {
			/* this is not split mod entry, remove it */
			ret_code = tpm_proc_delete_mod(TPM_MOD_OWNER_TPM, mod_con.mod_cmd_mac, mod_con.mod_cmd_ind);
		}
	}
	IF_ERROR(ret_code);

	/* Delete API Rule Entry */
	ret_code = tpm_db_api_entry_invalidate(api_section, rule_num);
	IF_ERROR(ret_code);

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
		/* Decrease rule_numbers and PnC entries of the existing API entries that were "pulled up" */
		ret_code = tpm_proc_api_entry_rulenum_dec(api_section, (rule_num + 1), (api_rng_entries - 1));
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_acl_rule(tpm_api_sections_t api_section, uint32_t owner_id, uint32_t rule_idx, uint32_t busy_check)
{
	tpm_error_code_t ret_code, busy_ret_code = TPM_RC_OK;
	tpm_api_type_t api_type;

	/* Check API_type Busy */
	if (busy_check) {
		ret_code = tpm_db_api_type_get_from_api_section(api_section, &api_type);
		IF_ERROR(ret_code);
		ret_code = tpm_proc_check_api_busy(api_type, rule_idx);
		IF_ERROR(ret_code);
	}

	ret_code = tpm_proc_int_del_acl_rule(api_section, owner_id, rule_idx);

	/* Free API_type */
	if (busy_check)
		busy_ret_code = tpm_proc_api_busy_done(api_type, rule_idx);

	IF_ERROR(ret_code);
	IF_ERROR(busy_ret_code);

	return(TPM_RC_OK);
}


/*******************************************************************************
* tpm_add_ipv4_acl_rule()
*
* DESCRIPTION:      Creates a new IPv4 processing ACL.
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_ipv4_acl_rule(uint32_t owner_id,
					    tpm_src_port_type_t src_port,
					    uint32_t rule_num,
					    uint32_t *rule_idx,
					    tpm_parse_fields_t parse_rule_bm,
					    tpm_parse_flags_t parse_flags_bm,
					    tpm_ipv4_acl_key_t *ipv4_key,
					    tpm_pkt_frwd_t *pkt_frwd,
					    tpm_pkt_mod_t *pkt_mod,
					    tpm_pkt_mod_bm_t pkt_mod_bm,
					    tpm_rule_action_t *rule_action)
{

	tpm_error_code_t ret_code;
	int32_t int_ret_code, int_mod_bm;
	tpm_gmacs_enum_t trg_gmac;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0, bi_dir = 0, update_sram_only = 0;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_api_lu_conf_t lu_conf;
	tpm_trg_port_type_t trg_port;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d)\n", owner_id, src_port, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv4_check(owner_id, src_port, rule_num, parse_rule_bm, parse_flags_bm,
					   ipv4_key, pkt_frwd, pkt_mod, rule_action, pkt_mod_bm);
	IF_ERROR(ret_code);

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_IPV4_ACL, &range_id);
	IF_ERROR(ret_code);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Only do it in table mode */
	if (TPM_RANGE_TYPE_TABLE == rangConf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_IPV4_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d) already exists\n",
				     owner_id, src_port, rule_num);

			/* TCAM Key and modification info are not changed, only update SRAM */
			if ((0 == memcmp(ipv4_key, &api_data.ipv4_key.ipv4_key, sizeof(tpm_ipv4_acl_key_t)))
			    && (src_port == api_data.ipv4_key.src_port)
			    && (parse_rule_bm == api_data.ipv4_key.parse_rule_bm)
			    && (parse_flags_bm == api_data.ipv4_key.parse_flags_bm)
			    && (SET_MOD(rule_action->pkt_act) == SET_MOD(api_data.ipv4_key.rule_action.pkt_act))
			    && (SET_UDP_CHKSUM_CALC(rule_action->pkt_act) == \
					SET_UDP_CHKSUM_CALC(api_data.ipv4_key.rule_action.pkt_act))) {
				/* No modification */
					if (!SET_MOD(rule_action->pkt_act))
						update_sram_only = 1;
				/* There are modification, then check furthur */
					else if ((pkt_mod_bm == api_data.ipv4_key.pkt_mod_bm)
						 && (0 == memcmp(pkt_mod, &api_data.ipv4_key.pkt_mod, sizeof(tpm_pkt_mod_t)))) {
						update_sram_only = 1;
						mod_entry = mod_con.mod_cmd_ind;
						trg_gmac = mod_con.mod_cmd_mac;
					}
				}

			/* TCAM Key or modification info is changed */
			if (!update_sram_only) {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
					     " owner(%d) src_port(%d), rule_num(%d) TCAM key/modification changed\n",
					     owner_id, src_port, rule_num);

				tpm_proc_del_ipv4_acl_rule(owner_id, rule_num, TPM_INT_CALL);
				IF_ERROR(ret_code);
			}
		}
	}

	if (    (!update_sram_only)
	     && (SET_MOD(rule_action->pkt_act))) {
		/* if split mod stage-2 */
		if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
			VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
			(pkt_mod_bm == TPM_VLAN_MOD)) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

			/* in this case, trgt port should be set in L2 stage, IPv4 only perform pbit mod,
			   so here src port is used for get DS or US info, which tells the Dst GMAC.
			 */
			if (SET_TARGET_PORT(rule_action->pkt_act))
				trg_port = pkt_frwd->trg_port;
			else if (FROM_LAN(src_port))
				trg_port = TPM_TRG_PORT_WAN;
			else
				trg_port = TPM_TRG_PORT_UNI_ANY;

			if (tpm_proc_trg_port_gmac_map(trg_port, &trg_gmac)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
				return(ERR_MOD_INVALID);
			} else if (trg_gmac == TPM_INVALID_GMAC) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
				return(ERR_MOD_INVALID);
			}

			/* get mod index from p-bit value */
			int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
			if (TPM_DB_OK != int_ret_code) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
					pkt_mod->vlan_mod.vlan1_out.pbit);
				return ERR_MOD_INVALID;
			}

			/* the first PMT is for p-bit AS-IS */
			mod_entry++;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
		} else {
			/* Check if UDP or TCP or nothing */
			int_mod_bm = 0;
			if (parse_rule_bm & TPM_IPv4_PARSE_PROTO) {
				if (ipv4_key->ipv4_proto == IPPROTO_TCP)
					int_mod_bm = TPM_INT_L4_TCP;
				else if (ipv4_key->ipv4_proto == IPPROTO_UDP) {
					int_mod_bm = TPM_INT_L4_UDP;
					/* Set internal_bm according to API Action */
					if (rule_action->pkt_act & TPM_ACTION_UDP_CHKSUM_CALC)
						int_mod_bm = TPM_INT_L4_UDP | TPM_INT_UDP_CHECKSUM;
				}
			}
			ret_code =
			tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, int_mod_bm,
					    &mod_entry, &trg_gmac);
			IF_ERROR(ret_code);
		}
	}


	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv4_tcam_build(src_port, dir, rule_num, ipv4_key, parse_rule_bm,
					parse_flags_bm, rule_action, pkt_mod, pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code =  tpm_proc_ipv4_sram_build(src_port, dir, rule_num, pon_type, ipv4_key,
						 pnc_data.pncl_tcam.ipv4_parse_bm, parse_flags_bm,
						 pkt_frwd, rule_action, mod_entry, pkt_mod_bm, pkt_mod, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
		/*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_IPV4_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_IPV4_ACL, rule_num, update_sram_only, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/
	if (!update_sram_only) {
		/* Set API data */
		api_data.ipv4_key.src_port = src_port;
		api_data.ipv4_key.parse_rule_bm = parse_rule_bm;
		api_data.ipv4_key.parse_flags_bm = parse_flags_bm;
		api_data.ipv4_key.pkt_mod_bm = pkt_mod_bm;
		memcpy(&(api_data.ipv4_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
		if (ipv4_key)
			memcpy(&(api_data.ipv4_key.ipv4_key), ipv4_key, sizeof(tpm_ipv4_acl_key_t));

		if (pkt_frwd != NULL)
			memcpy(&(api_data.ipv4_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
		else
			memset(&(api_data.ipv4_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

		if (pkt_mod != NULL)
			memcpy(&(api_data.ipv4_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
		else
			memset(&(api_data.ipv4_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

		/* Set modification data */
		tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

		/* Set Pnc Connection data */
		pnc_conn.num_pnc_ranges = 1;
		pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
		pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

		if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
			/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
			if (rule_num < api_rng_entries) {
				int_ret_code =
				tpm_proc_api_entry_rulenum_inc(TPM_IPV4_ACL, rule_num, (api_rng_entries - 1));
				IF_ERROR(int_ret_code);
			}
		}

		/* Set new API Entry */
		int_ret_code = tpm_db_api_entry_set(TPM_IPV4_ACL, rule_num, 0 /*bi_dir */ ,
						    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
		IF_ERROR(int_ret_code);

		/* Return Output */
		*rule_idx = l_rule_idx;
	} else {
		if (pkt_frwd != NULL)
			memcpy(&(api_data.ipv4_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
		else
			memset(&(api_data.ipv4_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

		int_ret_code = tpm_db_api_entry_invalidate(TPM_IPV4_ACL, rule_num);
		IF_ERROR(int_ret_code);

		/* Set new API Entry */
		int_ret_code = tpm_db_api_entry_set(TPM_IPV4_ACL, rule_num, 0 /*bi_dir */ ,
						    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
		IF_ERROR(int_ret_code);

		/* Return Output */
		*rule_idx = l_rule_idx;
	}

	/* Set aging counter group nunmber and mask */
	int_ret_code = tpm_db_pnc_get_lu_conf(TPM_PNC_IPV4_MAIN, &lu_conf);
	IF_ERROR(int_ret_code);
	tpm_tcam_set_lu_mask(pnc_conn.pnc_conn_tbl[0].pnc_index, (int32_t) lu_conf.lu_mask);
	tpm_tcam_set_cntr_group(pnc_conn.pnc_conn_tbl[0].pnc_index, (int32_t) lu_conf.cntr_grp);

	return(TPM_RC_OK);

}

/*******************************************************************************
* tpm_del_ipv4_acl_rule()
*
* DESCRIPTION:      Delete a  IPv4 processing ACL.
*
* INPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_ipv4_acl_rule(uint32_t owner_id, uint32_t rule_idx, uint32_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_IPV4_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_check_dst_uni_port()
*
* DESCRIPTION:    The function checks Dst port params.
*
* INPUTS:
* dest_port_bm     -
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_check_dst_uni_port(tpm_trg_port_type_t dest_port_bm)
{
	uint32_t dst_port;
	tpm_src_port_type_t src_port;
	tpm_init_virt_uni_t virt_uni_info;
	uint32_t gmac_is_uni_num, max_uni_port_num;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "dest_port_bm(0x%x)\n", dest_port_bm);

	for (dst_port = TPM_TRG_UNI_0, src_port = TPM_SRC_PORT_UNI_0;
	     dst_port <= TPM_TRG_UNI_7;
	     dst_port = (dst_port << 1), src_port += 1) {

		if(0 == (dest_port_bm & dst_port))
			continue;

		/* if port is valid */
		if(!tpm_db_eth_port_valid(src_port)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "dest_port(0x%x) is not valid\n", dst_port);
			return ERR_MC_DST_PORT_INVALID;
		}
	}

	/* check vir uni */
	tpm_db_virt_info_get(&virt_uni_info);
	if (    (TPM_VIRT_UNI_DISABLED == virt_uni_info.enabled)
	     && (dest_port_bm & TPM_TRG_UNI_VIRT)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "TPM_TRG_UNI_VIRT is not enabled\n");
		return ERR_MC_DST_PORT_INVALID;
	}
	/* Get GMAC LAN_UNI and UNI ports number */
	tpm_proc_gmaclanuni_uninum_get(&gmac_is_uni_num, &max_uni_port_num);

	/* check UNI_ANY */
	if (dest_port_bm & TPM_TRG_PORT_UNI_ANY) {
		/* Check UNI_ANY is supported or not */
		if (gmac_is_uni_num > 1 ||
		    (gmac_is_uni_num == 1 && max_uni_port_num > 1)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "dest_port UNI_ANY is not supported\n");
			return ERR_MC_DST_PORT_INVALID;
		}
	}

	/* Check multi des port  in bm */
	if (dest_port_bm > TPM_TRG_UNI_1) {
		if (gmac_is_uni_num > 1) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "dest_port bit map combine is not supported\n");
			return ERR_MC_DST_PORT_INVALID;
		}
	}

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_add_ipv4_mc_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_add_ipv4_mc_stream params.
*
* INPUTS:
* owner_id         - See tpm_proc_add_ipv4_mc_stream
* stream_num       - See tpm_proc_add_ipv4_mc_stream
* vid              - See tpm_proc_add_ipv4_mc_stream
* ipv4_src_add     - See tpm_proc_add_ipv4_mc_stream
* ipv4_dst_add     - See tpm_proc_add_ipv4_mc_stream
* ignore_ipv4_src  - See tpm_proc_add_ipv4_mc_stream
* dest_port_bm     - See tpm_proc_add_ipv4_mc_stream
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_ipvx_mc_check(uint32_t owner_id,
					    uint32_t stream_num,
					    tpm_mc_igmp_mode_t igmp_mode,
					    uint8_t mc_stream_pppoe,
					    uint16_t vid,
					    uint8_t src_add[16],
					    uint8_t dst_add[16],
					    uint8_t ignore_ipvx_src,
					    uint16_t dest_queue,
					    tpm_trg_port_type_t dest_port_bm,
					    tpm_mc_filter_mode_t filter_mode,
					    tpm_ip_ver_t ip_version)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code, index;
	tpm_db_pnc_range_t range_data;
	uint32_t ai_bit;
	tpm_init_virt_uni_t virt_uni;
	uint32_t pnc_rng_free_size;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;
	tpm_pnc_ranges_t range;
	tpm_api_sections_t api_section;
	tpm_api_type_t api_type;

	if (TPM_IP_VER_4 == ip_version) {
		range = TPM_PNC_IPV4_MC_DS;
		api_section = TPM_IPV4_MC;
		api_type = TPM_API_IPV4_MC;
	} else {
		range = TPM_PNC_IPV6_MC_DS;
		api_section = TPM_IPV6_MC_ACL;
		api_type = TPM_API_IPV6_MC;
	}

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	ret_code = tpm_owner_id_check(api_type, owner_id);
	IF_ERROR(ret_code);

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(range, &range_data);
	IF_ERROR(int_ret_code);

	if (stream_num >= range_data.pnc_range_conf.range_size)
		IF_ERROR(ERR_MC_STREAM_INVALID);

	ret_code = tpm_proc_check_dst_uni_port(dest_port_bm);
	IF_ERROR(ret_code);

	/*check virt range size if necessary*/
	if(filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0 && (dest_port_bm & TPM_TRG_UNI_VIRT) != 0) {
			ret_code = tpm_db_pnc_rng_free_ent_get(TPM_PNC_VIRT_UNI, &pnc_rng_free_size);
			IF_ERROR(ret_code);
			if(pnc_rng_free_size == 0)
				IF_ERROR(ERR_OUT_OF_RESOURCES);
		}
	}

	/* fix bug of adding 1-2-3 instead of 0-1-2 => when 3 is over-writing the hardcoded entry */
	if ((stream_num < range_data.pnc_range_conf.api_start) || (stream_num > range_data.pnc_range_conf.api_end))
		IF_ERROR(ERR_MC_STREAM_INVALID);

	tpm_db_api_entry_ind_get(api_section, stream_num, &index);
	if (-1 != index)
		IF_ERROR(ERR_MC_STREAM_EXISTS);

	tpm_db_virt_info_get(&virt_uni);

	if (vid == 0xffff) {
		if (tpm_db_get_mc_filter_mode() == TPM_MC_IP_ONLY_FILTER) {
			if (virt_uni.enabled) {
				TPM_OS_WARN(TPM_TPM_LOG_MOD,
					    " filter mode fall back to MC_COMBINED_MAC_IP_FILTER \r\n");
			}
		}

		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD,
				    "when mc_per_uni_vlan_xlate is enabled, untagged mcast stream is not supported, "
				    "and MC VID must be specified \r\n");
			IF_ERROR(ERR_SW_VID_INVALID);
		}
	} else {
		if (tpm_db_mc_vlan_get_ai_bit(vid, &ai_bit) != TPM_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD,
				    "MC VID must be configured first \r\n");
			IF_ERROR(ERR_SW_VID_INVALID);
		}
	}

	if (mc_stream_pppoe) {
		if (tpm_db_get_mc_pppoe_enable() == 0)
			IF_ERROR(ERR_FEAT_UNSUPPORT);

		if (igmp_mode == TPM_MC_IGMP_SNOOPING && tpm_db_get_mc_per_uni_vlan_xlate())
			IF_ERROR(ERR_FEAT_UNSUPPORT);
	}

	if (TPM_IP_VER_4 == ip_version) {
		if ((dst_add[0] < 224) || (dst_add[0] > 239))
			IF_ERROR(ERR_IPV4_MC_DST_IP_INVALID);
	} else {
		if (dst_add[0] != 0xff)
			IF_ERROR(ERR_IPV6_MC_DST_IP_INVALID);

		/* check if there is MC SIP slot */
		if(0 == ignore_ipvx_src) {
			if(!tpm_db_ipv6_mc_sip_index_get(src_add)) {
				/* this is a new MC SIP */
				if(!tpm_db_ipv6_mc_sip_free_slot_num_get())
					IF_ERROR(ERR_OUT_OF_RESOURCES);
			}
		}
	}

	if ((dest_queue != TPM_INVALID_QUEUE) && (dest_queue >= TPM_MAX_NUM_RX_QUEUE))
		IF_ERROR(ERR_MC_DST_QUEUE_INVALID);

	if (dest_port_bm & TPM_TRG_UNI_VIRT) {
		if (virt_uni.enabled == 0)
			IF_ERROR(ERR_MC_DST_PORT_INVALID);
	}

	if (dest_port_bm & TPM_TRG_PORT_CPU) {
		if (dest_port_bm & (~TPM_TRG_PORT_CPU))
			IF_ERROR(ERR_MC_DST_PORT_INVALID);
	}

	/* when ds load balance on G0 and G1 is enabled, no Proxy stream is allowed */
	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
	if (TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable) {
		if (    (TPM_IP_VER_4 == ip_version)
		     && (TPM_MC_IGMP_PROXY == igmp_mode)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "when ds load balance on G0 and G1 is enabled, "
				"no Proxy stream is allowed\n");
			return(ERR_FEAT_UNSUPPORT);
		}
		if (TPM_IP_VER_6 == ip_version) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "when ds load balance on G0 and G1 is enabled, "
				"there is no IPv6 MC feature\n");
			return(ERR_FEAT_UNSUPPORT);
		}
	}
	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_ipv4_mc_tcam_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* stream_num          - MC stream number
* ipv4_src_add        - IPv4 source IP address in network order.
* ipv4_dst_add        - IPv4 destination IP address in network order.
* ignore_ipv4_src     - when set to 1 - the IP source is not part of the key.
* tcam_data           - Logical TCAM Structure
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ipv4_mc_tcam_build(tpm_mc_filter_mode_t filter_mode,
				    uint32_t stream_num,
				    uint16_t vid,
				    uint8_t mc_stream_pppoe,
				    uint8_t ipv4_src_add[4],
				    uint8_t ipv4_dst_add[4],
				    uint8_t ignore_ipv4_src,
				    tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t int_ret_code;
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id, ai_bits;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t vid_ai_field;
	tpm_parse_flags_t parse_flags_bm = 0;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " stream_num(%d), sip(%02x%02x%02x%02x), dip(%02x%02x%02x%02x), ignore_src(%d)\n",
		     stream_num, ipv4_src_add[0], ipv4_src_add[1], ipv4_src_add[2], ipv4_src_add[3],
		     ipv4_dst_add[0], ipv4_dst_add[1], ipv4_dst_add[2], ipv4_dst_add[3], ignore_ipv4_src);

	tcam_data->l2_parse_bm = 0;
	tcam_data->l3_parse_bm = 0;
	tcam_data->ipv6_parse_bm = 0;
	tcam_data->add_info_mask = 0;
	tcam_data->add_info_data = 0;
	if (ignore_ipv4_src)
		tcam_data->ipv4_parse_bm = TPM_IPv4_PARSE_DIP;
	else
		tcam_data->ipv4_parse_bm = TPM_IPv4_PARSE_DIP | TPM_IPv4_PARSE_SIP;

	/* src port */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (vid != 0xFFFF) {
		int_ret_code = tpm_db_mc_vlan_get_ai_bit(vid, &ai_bits);
		IF_ERROR(int_ret_code);

		vid_ai_field.mc_vid_entry = ai_bits;

		parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_TRUE;
		parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_PARSE;
	} else
		parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_FALSE;

	if (mc_stream_pppoe)
		parse_flags_bm |= TPM_PARSE_FLAG_PPPOE_TRUE;
	else
		parse_flags_bm |= TPM_PARSE_FLAG_PPPOE_FALSE;

	/* Update the AI bits */
	tpm_proc_parse_flag_ai_tcam_build(&vid_ai_field, parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data),
					  &(tcam_data->add_info_mask));

	/* Copy in logical PnC Key */
	if (!ignore_ipv4_src) {
		memcpy(tcam_data->pkt_key.ipv4_key.ipv4_src_ip_add, ipv4_src_add, sizeof(ipv4_src_add));
		memset(tcam_data->pkt_key.ipv4_key.ipv4_src_ip_add_mask, 0xff, 4);
	}
	memcpy(tcam_data->pkt_key.ipv4_key.ipv4_dst_ip_add, ipv4_dst_add, sizeof(ipv4_dst_add));
	memset(tcam_data->pkt_key.ipv4_key.ipv4_dst_ip_add_mask, 0xff, 4);

	/* Get PNC Range information */
	int_ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV4_MC_DS, &lu_id, &start_offset);
	IF_ERROR(int_ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&tcam_data->start_offset, &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_ipvx_mc_sram_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* pon_type          - WAN technology
* l2_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
* pkt_frwd          - See xxx
* rule_action       - See xxx
*
* OUTPUTS:
* l2_sram_data      - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_ipvx_mc_sram_build(tpm_mc_filter_mode_t filter_mode,
				    tpm_mc_igmp_mode_t igmp_mode,
				    uint16_t dest_queue,
				    tpm_trg_port_type_t target_port,
				    uint32_t mod_entry,
				    tpm_pncl_sram_data_t *sram_data,
				    tpm_ip_ver_t ip_version)
{
	tpm_db_mh_src_t ds_mh_src;
	tpm_init_virt_uni_t virt_uni;
	tpm_db_pnc_range_conf_t range_conf;
	int32_t ret_code;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " target_port(%d)\n", target_port);

	/* Update dummy register (offset automatically=zero) */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	if (TPM_IP_VER_4 == ip_version) {
		sram_data->sram_updt_bm = 0;
		/*Set next lookup */
		sram_data->next_lu_off_reg = 0;
		/* For igmp_proxy, check TTL, for igmp_snooping do not */
		if (igmp_mode == TPM_MC_IGMP_PROXY) {
			ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_TTL, &range_conf);
			IF_ERROR(ret_code);
		} else {
			ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_PROTO, &range_conf);
			IF_ERROR(ret_code);
		}
		sram_data->next_lu_id = range_conf.base_lu_id;
	} else {
		sram_data->sram_updt_bm = TPM_PNCL_SET_LUD;
	}


	/* Set MH */
	if (tpm_db_get_mc_per_uni_vlan_xlate() == 0) {
		tpm_db_ds_mh_get_conf_set(&ds_mh_src);

		if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
			if (TPM_MH_SRC_PNC_RI == ds_mh_src) {
				sram_data->sram_updt_bm |= TPM_PNCL_SET_MH_RI;
				sram_data->mh_reg.mh_set = TPM_TRUE;

				tpm_db_virt_info_get(&virt_uni);
				if (virt_uni.enabled)
					sram_data->mh_reg.mh_reg = (TPM_MH_RI_BIT16 | TPM_MH_RI_BIT15);
				else
					sram_data->mh_reg.mh_reg = (TPM_MH_RI_BIT17 | TPM_MH_RI_BIT16 |
								    TPM_MH_RI_BIT15 | TPM_MH_RI_BIT14);
			} else {
				sram_data->mh_reg.mh_set = TPM_FALSE;
				sram_data->mh_reg.mh_reg = 0;
			}
		} else if (filter_mode == TPM_MC_IP_ONLY_FILTER) {
			/* Target UNI is set by Modification Entry */
		}
	} else {
		/* Target UNI is set by Modification Entry */
	}

	/* Set modification command */
	if (mod_entry != 0) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_entry;
	}

	if (TPM_IP_VER_4 == ip_version) {
		/* Reset AI bits for following LU */
		sram_data->add_info_data = 0;
		sram_data->add_info_mask = (TPM_AI_MC_VID_MASK | TPM_AI_MC_VID_VALID_MASK);
		if (TPM_MC_IGMP_PROXY == igmp_mode) {
		/* set MTM AI since in proxy mode */
			sram_data->add_info_data |= (1 << TPM_AI_MTM_BIT_OFF);
			sram_data->add_info_mask |= TPM_AI_MTM_MASK;
		}

		/* Final Fragment and L4 is detremined in separate stage */
		sram_data->l3_type = TPM_PNCL_L3_IPV4_NFRAG;
		sram_data->l4_type = TPM_PNCL_L4_OTHER;
	}

	/* Set Target Port */
	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);

	sram_data->sram_updt_bm |= TPM_PNCL_SET_TXP;
	sram_data->pnc_queue = dest_queue;
	if (target_port == TPM_TRG_PORT_CPU) {
		sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
	} else if (TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable){
		sram_data->sram_updt_bm &= (~TPM_PNCL_SET_TXP);
	} else {
		tpm_pnc_trg_t pnc_target;
		ret_code = tpm_db_to_lan_gmac_get(target_port, &pnc_target);
		IF_ERROR(ret_code);
		sram_data->flow_id_sub.pnc_target = pnc_target;
	}

	return(TPM_OK);
}

int32_t tpm_proc_create_ipvx_mc_mod(tpm_mc_filter_mode_t filter_mode,
				    tpm_mc_igmp_mode_t igmp_mode,
				    uint8_t mc_stream_pppoe,
				    uint16_t vid,
				    uint8_t *group_addr,
				    uint32_t dest_port_bm,
				    uint32_t *mod_entry,
				    tpm_ip_ver_t ip_version)
{
	int32_t ret_code;
	tpm_pkt_mod_bm_t pkt_mod_bm = 0;
	tpm_pkt_mod_t pkt_mod;
	uint8_t mc_mac[6];
	uint32_t lpbk_port_bm = 0, entry_id;
	tpm_mc_vid_port_cfg_t *mc_vid_cfg = NULL;
	uint8_t valid;
	uint32_t mh_en;
	uint32_t switch_init, trgt_gmac;
	tpm_pnc_trg_t pnc_target;
	tpm_gmacs_enum_t gmac;
	/*struct net_device *dev = NULL;*/

	memset(&pkt_mod, 0, sizeof(tpm_pkt_mod_t));

	if (TPM_TRG_PORT_CPU == dest_port_bm) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "target to CPU, no pkt mod is needed\n");
		*mod_entry = 0;
		return TPM_OK;
	}

	/*get MH EN */
	ret_code = tpm_db_to_lan_gmac_get(dest_port_bm, &pnc_target);
	IF_ERROR(ret_code);
	ret_code = tpm_db_target_to_gmac(pnc_target, &gmac);
	IF_ERROR(ret_code);
	ret_code = tpm_db_gmac_mh_en_conf_get(gmac, &mh_en);
	IF_ERROR(ret_code);

	/*get switch init*/
	ret_code = tpm_db_switch_init_get(&switch_init);
	IF_ERROR(ret_code);

	if (filter_mode == TPM_MC_IP_ONLY_FILTER && mh_en) {
		/* TODO: Check virt_port status. If it is not enabled, set target_port via MH_Tx_reg in RI. */
		pkt_mod_bm |= TPM_MH_SET;
		pkt_mod.mh_mod = tpm_db_trg_port_switch_port_get(dest_port_bm);
	}

	if (tpm_db_get_mc_per_uni_vlan_xlate() == 0) {
		if (vid != 0xffff) {
			if (tpm_db_get_mc_vid_cfg(vid, &mc_vid_cfg) == TPM_OK) {
				for (entry_id = 0; entry_id < TPM_MAX_NUM_UNI_PORTS; entry_id++) {
					if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_TRANSLATE) {
						pkt_mod_bm |= TPM_VLAN_MOD;

						pkt_mod.vlan_mod.vlan_op = VLANOP_EXT_TAG_MOD;
						pkt_mod.vlan_mod.vlan1_out.tpid = 0x8100;
						pkt_mod.vlan_mod.vlan1_out.pbit = 0;
						pkt_mod.vlan_mod.vlan1_out.pbit_mask = 0x0;
						pkt_mod.vlan_mod.vlan1_out.cfi = 0;
						pkt_mod.vlan_mod.vlan1_out.cfi_mask = 0x0;
						pkt_mod.vlan_mod.vlan1_out.vid = mc_vid_cfg[entry_id].uni_port_vid;
						pkt_mod.vlan_mod.vlan1_out.vid_mask = 0xffff;
						break;
					} else if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_STRIP) {
						/* Just for MC, no switch */
						if (switch_init == 0) {
							pkt_mod_bm |= TPM_VLAN_MOD;

							pkt_mod.vlan_mod.vlan_op = VLANOP_EXT_TAG_DEL;
							break;
						}
					}
				}
			} else
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " not found mv_vlan_cfg %d, assume as transparent! \n", vid);
		}
	} else if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
		pkt_mod_bm |= TPM_MH_SET;

		ret_code = tpm_db_get_mc_vid_cfg(vid, &mc_vid_cfg);
		IF_ERROR(ret_code);

		for (entry_id = 0; entry_id < TPM_MAX_NUM_UNI_PORTS; entry_id++) {
			if (mc_vid_cfg[entry_id].tpm_src_port != TPM_SRC_PORT_UNI_VIRT) {
				if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_TRANSPARENT ||
				    mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_STRIP) {
					lpbk_port_bm |=
					TPM_TRG_UNI_0 << (mc_vid_cfg[entry_id].tpm_src_port - TPM_SRC_PORT_UNI_0);
				}
			}
		}
		lpbk_port_bm |= TPM_TRG_UNI_VIRT;

		pkt_mod.mh_mod = tpm_db_trg_port_switch_port_get(lpbk_port_bm);
	}

	if (igmp_mode == TPM_MC_IGMP_PROXY) {
#if 0
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 24)
		dev = dev_get_by_name("eth0");
#else
		dev = dev_get_by_name(&init_net, "eth0");
#endif
		if (dev == NULL)
			IF_ERROR(ERR_GENERAL);

		pkt_mod_bm |= (TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE);
		pkt_mod.mac_mod.mac_sa[0] = ((uint8_t *) dev->dev_addr)[0];
		pkt_mod.mac_mod.mac_sa[1] = ((uint8_t *) dev->dev_addr)[1];
		pkt_mod.mac_mod.mac_sa[2] = ((uint8_t *) dev->dev_addr)[2];
		pkt_mod.mac_mod.mac_sa[3] = ((uint8_t *) dev->dev_addr)[3];
		pkt_mod.mac_mod.mac_sa[4] = ((uint8_t *) dev->dev_addr)[4];
		pkt_mod.mac_mod.mac_sa[5] = ((uint8_t *) dev->dev_addr)[5];

		pkt_mod.mac_mod.mac_sa_mask[0] = 0xff;
		pkt_mod.mac_mod.mac_sa_mask[1] = 0xff;
		pkt_mod.mac_mod.mac_sa_mask[2] = 0xff;
		pkt_mod.mac_mod.mac_sa_mask[3] = 0xff;
		pkt_mod.mac_mod.mac_sa_mask[4] = 0xff;
		pkt_mod.mac_mod.mac_sa_mask[5] = 0xff;
#endif

		if (TPM_IP_VER_4 == ip_version)
			pkt_mod_bm |= (TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE);
		else
			pkt_mod_bm |= (TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);

		tpm_db_get_mc_igmp_proxy_sa_mac(mc_mac, &valid);

		if (valid) {
			memcpy(pkt_mod.mac_mod.mac_sa, mc_mac, 6 * sizeof(uint8_t));
			memset(pkt_mod.mac_mod.mac_sa_mask, 0xff, 6 * sizeof(uint8_t));
		} else
			IF_ERROR(ERR_GENERAL);

		if (mc_stream_pppoe) {
			pkt_mod_bm |= (TPM_MAC_DA_SET | TPM_PPPOE_DEL);

			if (TPM_IP_VER_4 == ip_version) {
				MULTI_IP_2_MAC(mc_mac, group_addr);
			} else {
				MULTI_IPV6_2_MAC(mc_mac, group_addr);
			}
			memcpy(pkt_mod.mac_mod.mac_da, mc_mac, 6 * sizeof(uint8_t));
			memset(pkt_mod.mac_mod.mac_da_mask, 0xff, 6 * sizeof(uint8_t));
		}
	}

	if (pkt_mod_bm != 0) {
		ret_code = tpm_proc_create_mod(TPM_ACTION_SET_PKT_MOD, dest_port_bm, &pkt_mod, pkt_mod_bm,
			TPM_INT_MC_MOD, mod_entry, &trgt_gmac);
		IF_ERROR(ret_code);
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_add_uni_port_igmp_rule()
*
* DESCRIPTION:    Create IGMP rule for UNI port.
*
* INPUTS:
* src_port        - uni port to set
* state           - how to process IGMP packets
*
* OUTPUTS:
*           NONE
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_uni_port_igmp_rule(tpm_src_port_type_t src_port, tpm_igmp_frwd_mode_t mode)
{
	int32_t ret_code;
	tpm_pncl_pnc_full_t igmp_pnc_data;
	tpm_gmac_bm_t gmac_bm;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_conf_t range_conf;
	uint32_t cpu_queue;
	tpm_db_pnc_range_t range_data;
	uint32_t pnc_entry;
	int32_t lPort = 0;
	uint32_t is_switch_port = 1;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, " UNI-%d IGMP Forward Mode -- %d.\r\n", src_port, mode);

	memset(&igmp_pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/* build TCAM */
	igmp_pnc_data.pncl_tcam.l2_parse_bm = 0;
	igmp_pnc_data.pncl_tcam.l3_parse_bm = 0;
	igmp_pnc_data.pncl_tcam.ipv6_parse_bm = 0;
	igmp_pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_PROTO;

	if (src_port < TPM_SRC_PORT_UNI_0 || src_port > TPM_SRC_PORT_UNI_VIRT) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " invalid src_port %d\n", src_port);
		return(ERR_SRC_PORT_INVALID);
	}

	is_switch_port = tpm_db_src_port_on_switch(src_port);

	if (is_switch_port == 1) {
		lPort = tpm_db_eth_port_switch_port_get(src_port);
		if (lPort == TPM_DB_ERR_PORT_NUM) {
			printk(KERN_ERR "ERROR: (%s:%d) source port(%d) is invalid\n", __func__, __LINE__, src_port);
			return ERR_SRC_PORT_INVALID;
		}

		igmp_pnc_data.pncl_tcam.add_info_data = ((lPort) << TPM_AI_UNI_BIT_OFF);
		igmp_pnc_data.pncl_tcam.add_info_mask = AI_TO_MASK(TPM_AI_UNI_BIT_OFF, TPM_AI_UNI_BIT_SIZE);
	}

	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	igmp_pnc_data.pncl_tcam.port_ids = gmac_bm;
	igmp_pnc_data.pncl_tcam.pkt_key.ipv4_key.ipv4_proto = IPPROTO_IGMP;

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IGMP, &range_conf);
	IF_ERROR(ret_code);
	igmp_pnc_data.pncl_tcam.lu_id = range_conf.base_lu_id;
	start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER_OR_IHL;
	memcpy(&igmp_pnc_data.pncl_tcam.start_offset, &start_offset, sizeof(tpm_pncl_offset_t));

	/* build SRAM */
	igmp_pnc_data.pncl_sram.next_lu_id = 0;
	igmp_pnc_data.pncl_sram.next_lu_off_reg = 0;
	igmp_pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	igmp_pnc_data.pncl_sram.next_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER_OR_IHL;
	igmp_pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	switch (mode) {
	case TPM_IGMP_FRWD_MODE_DROP:
		igmp_pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_DISC;
		break;

	case TPM_IGMP_FRWD_MODE_FORWARD:
		igmp_pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD;
		igmp_pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
		break;

	case TPM_IGMP_FRWD_MODE_SNOOPING:
		tpm_db_igmp_get_cpu_queue(&cpu_queue);
		igmp_pnc_data.pncl_sram.pnc_queue = cpu_queue;
		igmp_pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_TXP | TPM_PNCL_SET_IGMP;
		igmp_pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
		break;

	default:
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " invalid state %d\n", mode);
		return(ERR_PORT_IGMP_FRWD_MODE_INVALID);
	}

	igmp_pnc_data.pncl_sram.mh_reg.mh_set = TPM_FALSE;
	igmp_pnc_data.pncl_sram.mh_reg.mh_reg = 0;
	igmp_pnc_data.pncl_sram.add_info_data = 0;
	igmp_pnc_data.pncl_sram.add_info_mask = 0;
	igmp_pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_IPV4_NFRAG;
	igmp_pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;
	igmp_pnc_data.pncl_sram.sram_updt_bm |= (TPM_PNCL_SET_L3 | TPM_PNCL_SET_L4);
	igmp_pnc_data.pncl_sram.flow_id_sub.gem_port = 0;	/* ? */
	igmp_pnc_data.pncl_sram.flow_id_sub.mod_cmd = 0;

	/* set PNC entry */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_IGMP, &range_data);
	IF_ERROR(ret_code);
	/* since WAN port will use the first entry of this range,
       UNI port must start with the second one.
	 */
	pnc_entry = range_data.pnc_range_conf.range_start + src_port - TPM_SRC_PORT_UNI_0 + 1;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);
	if (pnc_entry > range_data.pnc_range_conf.range_end) {
		TPM_OS_ERROR(TPM_INIT_MOD, " IGMP range is not big enough for PNC entry of src_port %d\n", src_port);
		return(ERR_RULE_NUM_INVALID);
	}

	ret_code = tpm_pncl_entry_set(pnc_entry, &igmp_pnc_data);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_add_wan_port_igmp_rule()
*
* DESCRIPTION:    Create IGMP rule for PON port.
*
* INPUTS:
* state           - how to process IGMP packets
*
* OUTPUTS:
*           NONE
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_wan_port_igmp_rule(tpm_igmp_frwd_mode_t mode)
{
	int32_t ret_code;
	tpm_pncl_pnc_full_t igmp_pnc_data;
	tpm_gmac_bm_t gmac_bm;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_conf_t range_conf;
	uint32_t cpu_queue;
	tpm_db_pnc_range_t range_data;
	uint32_t pnc_entry;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, " PON IGMP Forward Mode -- %d.\r\n", mode);

	memset(&igmp_pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/* build TCAM */
	igmp_pnc_data.pncl_tcam.l2_parse_bm = 0;
	igmp_pnc_data.pncl_tcam.l3_parse_bm = 0;
	igmp_pnc_data.pncl_tcam.ipv6_parse_bm = 0;
	igmp_pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_PROTO;
	igmp_pnc_data.pncl_tcam.add_info_mask = 0;
	igmp_pnc_data.pncl_tcam.add_info_data = 0;

	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	igmp_pnc_data.pncl_tcam.port_ids = gmac_bm;
	igmp_pnc_data.pncl_tcam.pkt_key.ipv4_key.ipv4_proto = IPPROTO_IGMP;

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IGMP, &range_conf);
	IF_ERROR(ret_code);
	igmp_pnc_data.pncl_tcam.lu_id = range_conf.base_lu_id;
	start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER_OR_IHL;
	memcpy(&igmp_pnc_data.pncl_tcam.start_offset, &start_offset, sizeof(tpm_pncl_offset_t));

	/* build SRAM */
	igmp_pnc_data.pncl_sram.next_lu_id = 0;
	igmp_pnc_data.pncl_sram.next_lu_off_reg = 0;
	igmp_pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	igmp_pnc_data.pncl_sram.next_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER_OR_IHL;
	igmp_pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	switch (mode) {
	case TPM_IGMP_FRWD_MODE_DROP:
		igmp_pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_DISC;
		break;

	case TPM_IGMP_FRWD_MODE_FORWARD:
		igmp_pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD;
		igmp_pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
		break;

	case TPM_IGMP_FRWD_MODE_SNOOPING:
		tpm_db_igmp_get_cpu_queue(&cpu_queue);
		igmp_pnc_data.pncl_sram.pnc_queue = cpu_queue;
		igmp_pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_TXP | TPM_PNCL_SET_IGMP;
		igmp_pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
		break;

	default:
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " invalid state %d\n", mode);
		return(ERR_PORT_IGMP_FRWD_MODE_INVALID);
	}

	igmp_pnc_data.pncl_sram.mh_reg.mh_set = TPM_FALSE;
	igmp_pnc_data.pncl_sram.mh_reg.mh_reg = 0;
	igmp_pnc_data.pncl_sram.add_info_data = 0;
	igmp_pnc_data.pncl_sram.add_info_mask = 0;
	igmp_pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_IPV4_NFRAG;
	igmp_pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;
	igmp_pnc_data.pncl_sram.sram_updt_bm |= (TPM_PNCL_SET_L3 | TPM_PNCL_SET_L4);
	igmp_pnc_data.pncl_sram.flow_id_sub.gem_port = 0;	/* ? */
	igmp_pnc_data.pncl_sram.flow_id_sub.mod_cmd = 0;

	/* set PNC entry */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_IGMP, &range_data);
	IF_ERROR(ret_code);
	pnc_entry = range_data.pnc_range_conf.range_start;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);
	if (pnc_entry > range_data.pnc_range_conf.range_end) {
		TPM_OS_ERROR(TPM_INIT_MOD, " IGMP range is not big enough for PNC entry of WAN port\n");
		return(ERR_RULE_NUM_INVALID);
	}

	ret_code = tpm_pncl_entry_set(pnc_entry, &igmp_pnc_data);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_del_port_igmp_rule()
*
* DESCRIPTION:    Create IGMP rule for PON port.
*
* INPUTS:
* src_port        - port rule to delete
*
* OUTPUTS:
*           NONE
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_port_igmp_rule(tpm_src_port_type_t src_port)
{
	int32_t ret_code;
	tpm_db_pnc_conn_t pnc_con;
	tpm_db_pnc_range_t range_data;

	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));

	if (src_port >= TPM_SRC_PORT_UNI_ANY) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " invalid src_port %d\n", src_port);
		return(ERR_SRC_PORT_INVALID);
	}

	ret_code = tpm_db_pnc_rng_get(TPM_PNC_IGMP, &range_data);
	IF_ERROR(ret_code);

	pnc_con.num_pnc_ranges = 1;
	pnc_con.pnc_conn_tbl[0].pnc_range = TPM_PNC_IGMP;
	pnc_con.pnc_conn_tbl[0].pnc_index = range_data.pnc_range_conf.range_start + src_port;

	/* set PNC entry */
	ret_code = tpm_proc_pnc_con_del(&pnc_con);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

int32_t tpm_proc_mc_create_virt_uni_pnc_entry(uint32_t rule_num,
						uint16_t uni_vid,
						uint8_t group_addr[16],
						bool    ipv6_flag)
{
	int32_t ret_code;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_l2_acl_key_t l2_key;
	uint8_t mc_mac[6];
	uint32_t pnc_entry = 0, pnc_range_start = 0, api_start = 0;

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&l2_key, 0, sizeof(tpm_l2_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_VIRT_UNI, &range_conf);
	IF_ERROR(ret_code);
	pnc_range_start = range_conf.range_start;
	api_start = range_conf.api_start;

	if (rule_num >= (range_conf.range_size - 3)) {
		/* Excludes three default rules:
		   1. forward all queries to virt_uni
		   2. discard all other mc streams
		   3. forward all unicast to virt_uni */
		IF_ERROR(ERR_RULE_NUM_INVALID);
	}

	/* Build tcam data */
	pnc_data.pncl_tcam.lu_id = range_conf.base_lu_id;
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.subf = TPM_L2_PARSE_MH;

	/* Get GMAC(s) */
	pnc_data.pncl_tcam.port_ids = gmac_pnc_bm[TPM_ENUM_GMAC_1];

	/* Set L2 filter */
	pnc_data.pncl_tcam.l2_parse_bm = (TPM_L2_PARSE_MH | TPM_L2_PARSE_MAC_DA);

	pnc_data.pncl_tcam.pkt_key.src_port = TPM_SRC_PORT_UNI_VIRT;

	if (ipv6_flag) {
		MULTI_IPV6_2_MAC(mc_mac, group_addr);
	}
	else {
		MULTI_IP_2_MAC(mc_mac, group_addr);
	}

	memcpy(&(l2_key.mac.mac_da), mc_mac, 6 * sizeof(uint8_t));
	memset(&(l2_key.mac.mac_da_mask), 0xff, 6 * sizeof(uint8_t));

	if (uni_vid != 0xffff) {
		pnc_data.pncl_tcam.l2_parse_bm |= TPM_L2_PARSE_ONE_VLAN_TAG;

		l2_key.vlan1.tpid = 0x8100;
		l2_key.vlan1.tpid_mask = 0xffff;
		l2_key.vlan1.pbit = 0;
		l2_key.vlan1.pbit_mask = 0;
		l2_key.vlan1.cfi = 0;
		l2_key.vlan1.cfi_mask = 0;
		l2_key.vlan1.vid = uni_vid;
		l2_key.vlan1.vid_mask = 0xffff;
	}

	/* Copy L2 key */
	memcpy(&(pnc_data.pncl_tcam.pkt_key.l2_key), &l2_key, sizeof(tpm_l2_acl_key_t));

	/* Build sram data */
	pnc_data.pncl_sram.sram_updt_bm |= (TPM_PNCL_SET_TXP | TPM_PNCL_SET_LUD | TPM_PNCL_SET_MC);

	/* No next lookup */
	pnc_data.pncl_sram.next_lu_id = 0;
	pnc_data.pncl_sram.next_lu_off_reg = 0;

	/* Update dummy register (offset automatically=zero) */
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Set txp and txq */
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
	pnc_data.pncl_sram.pnc_queue = 1;	/* set default virt_uni mc queue via XML */

	/* Calculate absolute PNC entry number to execute */
	pnc_entry = (pnc_range_start + api_start) + rule_num;

	ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
	IF_ERROR(ret_code);

	/* Decrease number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_VIRT_UNI);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_delete_virt_uni_pnc_entry(uint32_t rule_num)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;
	uint32_t pnc_range_start = 0, api_start = 0, pnc_entry = 0;

	/* Get PNC Range Start */
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_VIRT_UNI, &range_conf);
	IF_ERROR(ret_code);
	pnc_range_start = range_conf.range_start;
	api_start = range_conf.api_start;

	/* Pull range from this index untill last used entry in Pnc range */
	pnc_entry = (pnc_range_start + api_start) + rule_num;

	/* Delete PNC entry */
	ret_code = tpm_pncl_entry_delete(pnc_entry, pnc_entry);
	IF_ERROR(ret_code);

	/* Increase number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_VIRT_UNI);
	IF_ERROR(ret_code);

	return TPM_OK;
}

tpm_error_code_t tpm_proc_add_ipv4_mc_pnc_entry(tpm_mc_filter_mode_t filter_mode,
						uint32_t stream_num,
						tpm_mc_igmp_mode_t igmp_mode,
						uint8_t mc_stream_pppoe,
						uint16_t vid,
						uint8_t ipv4_src_add[4],
						uint8_t ipv4_dst_add[4],
						uint8_t ignore_ipv4_src,
						uint16_t dest_queue,
						uint32_t dest_port_bm)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_range_t range_data;
	tpm_db_mc_stream_entry_t mc_stream;
	tpm_mc_vid_port_cfg_t *mc_vid_cfg = NULL;
	tpm_gmacs_enum_t gmac;

	int32_t ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, rule_num = 0xffff;
	uint32_t pnc_range_start = 0, api_start = 0, rule_idx = 0;
	uint32_t entry_id;
	uint16_t u4_vid;

	memset(&mc_stream, 0, sizeof(tpm_db_mc_stream_entry_t));

	/* In MC_MAC_ONLY_FILTER mode, forward all multicast traffic to internal switch by default rule. */
	if (filter_mode != TPM_MC_MAC_ONLY_FILTER) {
		/* Set Structs to zero */
		tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

		/*********** Create Modification Entries **********/
		ret_code =
		tpm_proc_create_ipvx_mc_mod(filter_mode, igmp_mode, mc_stream_pppoe, vid, ipv4_dst_add,
					    dest_port_bm, &mod_entry, TPM_IP_VER_4);
		IF_ERROR(ret_code);

		/*********** Create PNC Entries **********/
		/* Build PnC Entry */
		ret_code =
		tpm_proc_ipv4_mc_tcam_build(filter_mode, stream_num, vid, mc_stream_pppoe, ipv4_src_add,
					    ipv4_dst_add, ignore_ipv4_src, &(pnc_data.pncl_tcam));
		IF_ERROR(ret_code);

		/* Build SRAM Entry */
		ret_code =
		tpm_proc_ipvx_mc_sram_build(filter_mode, igmp_mode, dest_queue,
					    dest_port_bm, mod_entry,
					    &(pnc_data.pncl_sram), TPM_IP_VER_4);
		IF_ERROR(ret_code);

		/*** Calculate PNC Entry ***/

		/* Get PNC Range Start */
		ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_MC_DS, &range_data);
		IF_ERROR(ret_code);
		pnc_range_start = range_data.pnc_range_conf.range_start;
		api_start = range_data.pnc_range_conf.api_start;

		/* Calculate absolute PNC entry number to execute */
		pnc_entry = (pnc_range_start + api_start) + stream_num;

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " stream_num(%d), pnc_entry(%d)\n", stream_num, pnc_entry);

		ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
		IF_ERROR(ret_code);

		/* Decrease number of free entries in pnc_range */
		ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_IPV4_MC_DS);
		IF_ERROR(ret_code);

		/*********** Update API Range in DB **********/

		/* Set API data */
		memcpy(api_data.ipv4_mc_key.ipv4_src_add, ipv4_src_add, sizeof(ipv4_src_add));
		memcpy(api_data.ipv4_mc_key.ipv4_dest_add, ipv4_dst_add, sizeof(ipv4_dst_add));
		api_data.ipv4_mc_key.ignore_ipv4_src = ignore_ipv4_src;
		api_data.ipv4_mc_key.dest_port_bm = dest_port_bm;
		api_data.ipv4_mc_key.dest_queue = dest_queue;
		api_data.ipv4_mc_key.vid = vid;
		api_data.ipv4_mc_key.igmp_mode = igmp_mode;
		api_data.ipv4_mc_key.mc_stream_pppoe = mc_stream_pppoe;
		api_data.ipv4_mc_key.stream_num = stream_num;

		/* Set Pnc Connection data */
		pnc_conn.num_pnc_ranges = 1;
		pnc_conn.pnc_conn_tbl[0].pnc_range = TPM_PNC_IPV4_MC_DS;
		pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

		/* Set Modification Connection data */
		mod_con.mod_cmd_ind = mod_entry;
		if (mod_entry) {
			ret_code = tpm_db_target_to_gmac(pnc_data.pncl_sram.flow_id_sub.pnc_target, &gmac);
			IF_ERROR(ret_code);
			mod_con.mod_cmd_mac = gmac;
		}
		/* Set new API Entry */
		ret_code = tpm_db_api_entry_set(TPM_IPV4_MC, stream_num, 0 /*bi_dir */ ,
						&api_data, &mod_con, &pnc_conn, &rule_idx);
		IF_ERROR(ret_code);

		if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
			if (tpm_db_get_mc_per_uni_vlan_xlate() != 0 && (dest_port_bm & TPM_TRG_UNI_VIRT) != 0) {
				ret_code = tpm_db_get_mc_vid_cfg(vid, &mc_vid_cfg);
				IF_ERROR(ret_code);

				for (entry_id = 0; entry_id < TPM_MAX_NUM_UNI_PORTS; entry_id++) {
					if (mc_vid_cfg[entry_id].tpm_src_port == TPM_SRC_PORT_UNI_VIRT)
						break;
				}
				if (entry_id == TPM_MAX_NUM_UNI_PORTS) {
					TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
						     " virt_uni is unable to join the group when "
						     "virt_uni is not the member of multicast vlan \n");
				} else if (mc_vid_cfg[entry_id].mc_uni_port_mode != TPM_MC_UNI_MODE_EXCLUDE) {
					if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_TRANSPARENT)
						u4_vid = vid;
					else if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_STRIP)
						u4_vid = 0xffff;
					else
						u4_vid = mc_vid_cfg[entry_id].uni_port_vid;

					ret_code = tpm_db_mc_alloc_virt_uni_entry(&rule_num);
					IF_ERROR(ret_code);

					ret_code =
					tpm_proc_mc_create_virt_uni_pnc_entry(rule_num, u4_vid, ipv4_dst_add, false);
					IF_ERROR(ret_code);
				}
			}
		}
	}

	/* Set new stream entry */
	mc_stream.igmp_mode = igmp_mode;
	mc_stream.mc_stream_pppoe = mc_stream_pppoe;
	mc_stream.src_valid = ignore_ipv4_src;
	mc_stream.vid = vid;
	mc_stream.dest_queue = dest_queue;
	mc_stream.dest_port_bm = dest_port_bm;
	mc_stream.u4_entry = rule_num;
	memcpy(mc_stream.group_addr, ipv4_dst_add, 4 * sizeof(uint8_t));
	if (!ignore_ipv4_src)
		memcpy(mc_stream.src_addr, ipv4_src_add, 4 * sizeof(uint8_t));

	ret_code = tpm_db_set_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_update_ipv4_mc_pnc_entry(tpm_mc_filter_mode_t filter_mode,
						   uint32_t stream_num,
						   tpm_mc_igmp_mode_t igmp_mode,
						   uint8_t mc_stream_pppoe,
						   uint16_t vid,
						   uint8_t ipv4_src_add[4],
						   uint8_t ipv4_dst_add[4],
						   uint8_t ignore_ipv4_src,
						   uint16_t dest_queue,
						   uint32_t dest_port_bm)
{
	tpm_db_pnc_range_t range_data;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_mc_stream_entry_t mc_stream;
	tpm_mc_vid_port_cfg_t *mc_vid_cfg = NULL;

	int32_t ret_code;
	uint32_t bi_dir = 0, rule_idx = 0, pnc_entry = 0, mod_entry = 0, rule_num = 0xffff;
	uint32_t entry_id;
	uint16_t u4_vid;

	memset(&mc_stream, 0, sizeof(tpm_db_mc_stream_entry_t));

	/* Get old API Entry */
	ret_code = tpm_db_api_entry_get(TPM_IPV4_MC, stream_num, &rule_idx, &bi_dir,
					&api_data, &mod_con, &pnc_conn);
	IF_ERROR(ret_code);

	/* Only MC_IP_ONLY_FILTER mode, update the multicast group member ports by mh_mod. */
	if (filter_mode == TPM_MC_IP_ONLY_FILTER) {
		/* Get PNC Range Start */
		ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_MC_DS, &range_data);
		IF_ERROR(ret_code);

		if (stream_num >= range_data.pnc_range_conf.range_size)
			IF_ERROR(ERR_MC_STREAM_INVALID);

		pnc_entry = pnc_conn.pnc_conn_tbl[0].pnc_index;

		/* Create new Modification Entry */
		ret_code = tpm_proc_create_ipvx_mc_mod(filter_mode, igmp_mode, mc_stream_pppoe,
						       vid, ipv4_dst_add, dest_port_bm, &mod_entry, TPM_IP_VER_4);
		IF_ERROR(ret_code);

		/* Rebuild PnC Entry */
		ret_code = tpm_proc_ipv4_mc_tcam_build(filter_mode, stream_num, vid, mc_stream_pppoe, ipv4_src_add,
						       ipv4_dst_add, ignore_ipv4_src, &(pnc_data.pncl_tcam));
		IF_ERROR(ret_code);

		/* Rebuild SRAM Entry */
		ret_code =
		tpm_proc_ipvx_mc_sram_build(filter_mode, igmp_mode, dest_queue,
					    dest_port_bm, mod_entry,
					    &(pnc_data.pncl_sram), TPM_IP_VER_4);
		IF_ERROR(ret_code);

		/* Update only Sram of PNC Entry */
		ret_code = tpm_pncl_update_sram(pnc_entry, &pnc_data);
		IF_ERROR(ret_code);

		/* Delete old Modification Entry */
		if (mod_con.mod_cmd_ind != 0) {
			ret_code = tpm_proc_delete_mod(TPM_MOD_OWNER_TPM, mod_con.mod_cmd_mac, mod_con.mod_cmd_ind);
			IF_ERROR(ret_code);
		}
		/* Update new Modification Entry */
		mod_con.mod_cmd_ind = mod_entry;
	}

	if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0) {
			ret_code = tpm_db_get_mc_stream_entry(stream_num, &mc_stream);
			IF_ERROR(ret_code);

			rule_num = mc_stream.u4_entry;

			if ((dest_port_bm & TPM_TRG_UNI_VIRT) != 0 && (mc_stream.dest_port_bm & TPM_TRG_UNI_VIRT) == 0) {
				ret_code = tpm_db_mc_alloc_virt_uni_entry(&rule_num);
				IF_ERROR(ret_code);

				ret_code = tpm_db_get_mc_vid_cfg(vid, &mc_vid_cfg);
				IF_ERROR(ret_code);

				for (entry_id = 0; entry_id < TPM_MAX_NUM_UNI_PORTS; entry_id++) {
					if (mc_vid_cfg[entry_id].tpm_src_port == TPM_SRC_PORT_UNI_VIRT)
						break;
				}
				if (entry_id == TPM_MAX_NUM_UNI_PORTS) {
					TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
						     " virt_uni is unable to join the group when "
						     "virt_uni is not the member of multicast vlan \n");
				} else {
					if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_TRANSPARENT)
						u4_vid = vid;
					else if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_STRIP)
						u4_vid = 0xffff;
					else
						u4_vid = mc_vid_cfg[entry_id].uni_port_vid;

					ret_code =
					tpm_proc_mc_create_virt_uni_pnc_entry(rule_num, u4_vid, ipv4_dst_add, false);
					IF_ERROR(ret_code);
				}
			} else if ((dest_port_bm & TPM_TRG_UNI_VIRT) == 0 && (mc_stream.dest_port_bm & TPM_TRG_UNI_VIRT) != 0) {
				if (rule_num != 0xffff) {
					ret_code = tpm_proc_mc_delete_virt_uni_pnc_entry(rule_num);
					IF_ERROR(ret_code);
					ret_code = tpm_db_mc_free_virt_uni_entry(rule_num);
					IF_ERROR(ret_code);
					rule_num = 0xffff;
				}
			}
		}
	}

	/* Update stream entry */
	mc_stream.igmp_mode = igmp_mode;
	mc_stream.mc_stream_pppoe = mc_stream_pppoe;
	mc_stream.src_valid = ignore_ipv4_src;
	mc_stream.vid = vid;
	mc_stream.dest_port_bm = dest_port_bm;
	mc_stream.u4_entry = rule_num;
	memcpy(mc_stream.group_addr, ipv4_dst_add, 4 * sizeof(uint8_t));
	if (ignore_ipv4_src)
		memcpy(mc_stream.src_addr, ipv4_src_add, 4 * sizeof(uint8_t));

	ret_code = tpm_db_set_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	/* Update API entry */
	/* Invalidate old API Entry */
	ret_code = tpm_db_api_entry_invalidate(TPM_IPV4_MC, stream_num);
	IF_ERROR(ret_code);

	/* Set new API Entry */
	api_data.ipv4_mc_key.dest_port_bm = dest_port_bm;
	ret_code = tpm_db_api_entry_set(TPM_IPV4_MC, stream_num, 0 /*bi_dir */ ,
					&api_data, &mod_con, &pnc_conn, &rule_idx);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_set_ipv4_mc_switch(tpm_mc_filter_mode_t filter_mode,
					     uint8_t mc_mac[6],
					     uint32_t old_target,
					     uint32_t new_target)
{
	int32_t ret_code;
	uint32_t trg_port_bm = 0, diff_target = 0;
	uint32_t port;
	uint32_t switch_init = 0;

	/*get switch init*/
	ret_code = tpm_db_switch_init_get(&switch_init);
	IF_ERROR(ret_code);

	if (!switch_init) {
		/* no switch, no ATU operation */
		TPM_OS_INFO(TPM_TPM_LOG_MOD, "no switch, no ATU operation\n");
		return TPM_RC_OK;
	}

	diff_target = old_target ^ new_target;
	port = TPM_TRG_UNI_0;

	while (port != TPM_TRG_PORT_CPU) {
		if (diff_target & port) {
			if (old_target & port) {
				ret_code = tpm_db_decrease_mc_mac_port_user_num(mc_mac, port);
				IF_ERROR(ret_code);

				if (tpm_db_get_mc_mac_port_user_num(mc_mac, port) != 0)
					new_target |= port;
			} else {
				ret_code = tpm_db_increase_mc_mac_port_user_num(mc_mac, port);
				IF_ERROR(ret_code);
			}
		}

		port = port << 1;
	}

	if (new_target != 0)
		new_target |= TPM_TRG_PORT_CPU;	/* igmp report to CPU */

	switch (filter_mode) {
	case TPM_MC_COMBINED_IP_MAC_FILTER:
		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0 && new_target != 0)
			new_target |= TPM_TRG_UNI_VIRT;	/* loopback G1 */
	case TPM_MC_MAC_ONLY_FILTER:
		trg_port_bm = tpm_db_trg_port_switch_port_get(new_target);
		break;
	case TPM_MC_IP_ONLY_FILTER:
		trg_port_bm = 0x3F;	/* all uni ports + CPU + G1/wifi */
		break;
	default:
		break;
	}

	if (new_target == 0) {
		ret_code = tpm_sw_del_static_mac(TPM_MOD_OWNER_TPM, mc_mac);
		IF_ERROR(ret_code);
		tpm_db_reset_mc_mac_entry(mc_mac);
	} else {
		ret_code = tpm_sw_set_static_mac_w_ports_mask(TPM_MOD_OWNER_TPM, trg_port_bm, mc_mac);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_add_ipv4_mc_stream()
*
* DESCRIPTION:    Main function for adding IPv4 multicast streams.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_ipv4_mc_stream(uint32_t owner_id,
					     uint32_t stream_num,
					     tpm_mc_igmp_mode_t igmp_mode,
					     uint8_t mc_stream_pppoe,
					     uint16_t vid,
					     uint8_t ipv4_src_add[4],
					     uint8_t ipv4_dst_add[4],
					     uint8_t ignore_ipv4_src,
					     uint16_t dest_queue,
					     tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_mac[6];

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) stream_num(%d), igmp_mode(%d), mc_stream_pppoe(%d), vid(%d) "
		     "sip(0x%02x%02x%02x%02x), dip(0x%02x%02x%02x%02x), ignore_ipv4_src(%d), dest_port_bm(0x%x)\n",
		     owner_id, stream_num, igmp_mode, mc_stream_pppoe, vid, ipv4_src_add[0], ipv4_src_add[1],
		     ipv4_src_add[2], ipv4_src_add[3], ipv4_dst_add[0], ipv4_dst_add[1], ipv4_dst_add[2],
		     ipv4_dst_add[3], ignore_ipv4_src, dest_port_bm);

	filter_mode = tpm_db_get_mc_filter_mode();
	if (filter_mode == TPM_MC_ALL_CPU_FRWD) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " illegal mc_filter_mod (%d)\n", filter_mode);
		IF_ERROR(ERR_GENERAL);
	}

	if (dest_port_bm & TPM_TRG_PORT_UNI_ANY)
		dest_port_bm = tpm_db_trg_port_uni_any_bmp_get(true);

	/* Check parameters */
	ret_code = tpm_proc_add_ipvx_mc_check(owner_id, stream_num, igmp_mode, mc_stream_pppoe, vid,
					      ipv4_src_add, ipv4_dst_add, ignore_ipv4_src, dest_queue,
					      dest_port_bm, filter_mode, TPM_IP_VER_4);
	IF_ERROR(ret_code);

	/* get queue number */
	if (dest_queue == TPM_INVALID_QUEUE) {
		if (dest_port_bm & TPM_TRG_PORT_CPU)
			dest_queue = tpm_db_get_mc_cpu_queue();
		else
			dest_queue = tpm_db_get_mc_hwf_queue();
	}
	/* Create PNC entry */
	ret_code = tpm_proc_add_ipv4_mc_pnc_entry(filter_mode, stream_num, igmp_mode, mc_stream_pppoe, vid,
						  ipv4_src_add, ipv4_dst_add, ignore_ipv4_src, dest_queue, dest_port_bm);
	IF_ERROR(ret_code);

	/* Set switch port_map for multicast MAC, but don't overwrite 224.0.0.1 (IGMP General Query) MAC */
	MULTI_IP_2_MAC(mc_mac, ipv4_dst_add);
	if (memcmp(mc_mac, tpm_igmp_gen_query_mac, 6) != 0) {
		ret_code = tpm_proc_set_ipv4_mc_switch(filter_mode, mc_mac, 0, dest_port_bm);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_updt_ipv4_mc_stream()
*
* DESCRIPTION:    Main function for updating IPv4 multicast streams.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_updt_ipv4_mc_stream(uint32_t owner_id, uint32_t stream_num, tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_mac[6];
	tpm_init_virt_uni_t virt_uni;
	tpm_db_mc_stream_entry_t mc_stream;

	memset(&virt_uni, 0, sizeof(tpm_init_virt_uni_t));
	memset(&mc_stream, 0, sizeof(tpm_db_mc_stream_entry_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) stream_num(%d),dest_port_bm(0x%x)\n",
		     owner_id, stream_num, dest_port_bm);

	filter_mode = tpm_db_get_mc_filter_mode();
	if (filter_mode == TPM_MC_ALL_CPU_FRWD) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " illegal mc_filter_mod (%d)\n", filter_mode);
		IF_ERROR(ERR_GENERAL);
	}

	tpm_db_virt_info_get(&virt_uni);

	if (dest_port_bm & TPM_TRG_UNI_VIRT) {
		if (virt_uni.enabled == 0)
			IF_ERROR(ERR_MC_DST_PORT_INVALID);
	}

	if (dest_port_bm & TPM_TRG_PORT_CPU) {
		if (dest_port_bm & (~TPM_TRG_PORT_CPU))
			IF_ERROR(ERR_MC_DST_PORT_INVALID);
	}

	if (dest_port_bm & TPM_TRG_PORT_UNI_ANY)
		dest_port_bm = tpm_db_trg_port_uni_any_bmp_get(true);

	ret_code = tpm_proc_check_dst_uni_port(dest_port_bm);
	IF_ERROR(ret_code);

	ret_code = tpm_db_get_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	if (dest_port_bm == mc_stream.dest_port_bm) {
		/* nothing changed, return directly */
		TPM_OS_INFO(TPM_TPM_LOG_MOD, "dest_port_bm does not change, return directly\n");
		return (TPM_OK);
	}

	if (((dest_port_bm & TPM_TRG_PORT_CPU) != 0 && (mc_stream.dest_port_bm & TPM_TRG_PORT_CPU) == 0) ||
	    ((dest_port_bm & TPM_TRG_PORT_CPU) == 0 && (mc_stream.dest_port_bm & TPM_TRG_PORT_CPU) != 0)) {
		ret_code = tpm_proc_delete_ipvx_mc_pnc_entry(filter_mode, stream_num,
							mc_stream.dest_port_bm, mc_stream.u4_entry,
							TPM_IP_VER_4);
		IF_ERROR(ret_code);

		ret_code = tpm_proc_add_ipv4_mc_pnc_entry(filter_mode, stream_num, mc_stream.igmp_mode,
							mc_stream.mc_stream_pppoe, mc_stream.vid,
							mc_stream.src_addr, mc_stream.group_addr,
							mc_stream.src_valid, mc_stream.dest_queue,
							dest_port_bm);
	} else {
		ret_code = tpm_proc_update_ipv4_mc_pnc_entry(filter_mode, stream_num, mc_stream.igmp_mode,
							mc_stream.mc_stream_pppoe, mc_stream.vid,
							mc_stream.src_addr, mc_stream.group_addr,
							mc_stream.src_valid, mc_stream.dest_queue,
							dest_port_bm);
	}
	IF_ERROR(ret_code);

	/* Set switch port_map for multicast MAC, but don't overwrite 224.0.0.1 (IGMP General Query) MAC */
	MULTI_IP_2_MAC(mc_mac, mc_stream.group_addr);
	if (memcmp(mc_mac, tpm_igmp_gen_query_mac, 6) != 0) {
		ret_code = tpm_proc_set_ipv4_mc_switch(filter_mode, mc_mac, mc_stream.dest_port_bm, dest_port_bm);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_del_ipv4_mc_stream()
*
* DESCRIPTION:      Deletes an existing IPv4 MC stream.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* stream_num         - MC stream number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_ipv4_mc_stream(uint32_t owner_id, uint32_t stream_num)
{

	int32_t ret_code;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_mac[6];
	tpm_db_mc_stream_entry_t mc_stream;

	memset(&mc_stream, 0, sizeof(tpm_db_mc_stream_entry_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) stream_num(%d)\n", owner_id, stream_num);

	filter_mode = tpm_db_get_mc_filter_mode();
	if (filter_mode == TPM_MC_ALL_CPU_FRWD) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " illegal mc_filter_mod (%d)\n", filter_mode);
		IF_ERROR(ERR_GENERAL);
	}

	ret_code = tpm_db_get_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	ret_code =
	tpm_proc_delete_ipvx_mc_pnc_entry(filter_mode, stream_num,
	                                  mc_stream.dest_port_bm,
	                                  mc_stream.u4_entry,
	                                  TPM_IP_VER_4);
	IF_ERROR(ret_code);

	/* Set switch VID and multicast MAC */
	MULTI_IP_2_MAC(mc_mac, mc_stream.group_addr);
	/* Make sure that donot remove the mac 01-00-5e-00-00-01 from Switch which is used by Genery Query */
	if (memcmp(mc_mac, tpm_igmp_gen_query_mac, 6) != 0) {
		ret_code = tpm_proc_set_ipv4_mc_switch(filter_mode, mc_mac, mc_stream.dest_port_bm, 0);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

/*******************************/
/*     Multicast Loopback      */
/*******************************/

int32_t tpm_proc_mc_lpbk_xlat_tcam_build(tpm_ip_ver_t ip_ver, uint16_t mvlan, uint16_t in_vid, tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t ret_code;
	uint8_t first_lpbk;
	tpm_l2_acl_key_t l2_key;
	tpm_db_pnc_range_conf_t range_conf;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mvlan(%d), in_vid(%d) \n", mvlan, in_vid);

	tcam_data->l2_parse_bm = 0;
	memset(&l2_key, 0, sizeof(tpm_l2_acl_key_t));

	first_lpbk = 0;
	if (mvlan == in_vid)
		first_lpbk = 1;

	/* Get PNC Range information */
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_MULTI_LPBK, &range_conf);
	IF_ERROR(ret_code);
	tcam_data->lu_id = range_conf.base_lu_id;
	tcam_data->start_offset.range_id = TPM_PNC_MULTI_LPBK;
	tcam_data->start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	tcam_data->start_offset.offset_sub.subf = TPM_L2_PARSE_MH;

	/* Get GMAC(s) */
	tcam_data->port_ids = gmac_pnc_bm[TPM_ENUM_GMAC_1];

	/* Parse MH for src_port filter */
	tcam_data->l2_parse_bm |= TPM_L2_PARSE_MH;
	if (first_lpbk)
		tcam_data->pkt_key.src_port = TPM_SRC_PORT_WAN;
	else
		tcam_data->pkt_key.src_port = TPM_SRC_PORT_UNI_VIRT;

	/* Parse MC DA filter */
	tcam_data->l2_parse_bm |= TPM_L2_PARSE_MAC_DA;
	if (ip_ver == TPM_IP_VER_4) {
		l2_key.mac.mac_da[0] = 0x01;
		l2_key.mac.mac_da[1] = 0x00;
		l2_key.mac.mac_da[2] = 0x5e;
		l2_key.mac.mac_da_mask[0] = 0xff;
		l2_key.mac.mac_da_mask[1] = 0xff;
		l2_key.mac.mac_da_mask[2] = 0xff;
	} else {
		l2_key.mac.mac_da[0] = 0x33;
		l2_key.mac.mac_da[1] = 0x33;
		l2_key.mac.mac_da_mask[0] = 0xff;
		l2_key.mac.mac_da_mask[1] = 0xff;
	}

	/* Parse VLAN filter */
	if (first_lpbk) {
		tcam_data->l2_parse_bm |= TPM_L2_PARSE_ONE_VLAN_TAG;
		l2_key.vlan1.tpid = 0x8100;
		l2_key.vlan1.tpid_mask = 0xffff;
		l2_key.vlan1.pbit = 0;
		l2_key.vlan1.pbit_mask = 0;
		l2_key.vlan1.cfi = 0;
		l2_key.vlan1.cfi_mask = 0;
		l2_key.vlan1.vid = in_vid;
		l2_key.vlan1.vid_mask = 0xffff;
	} else {
		tcam_data->l2_parse_bm |= TPM_L2_PARSE_TWO_VLAN_TAG;

		l2_key.vlan1.tpid = 0x8100;
		l2_key.vlan1.tpid_mask = 0xffff;
		l2_key.vlan1.pbit = 0;
		l2_key.vlan1.pbit_mask = 0;
		l2_key.vlan1.cfi = 0;
		l2_key.vlan1.cfi_mask = 0;
		l2_key.vlan1.vid = mvlan;
		l2_key.vlan1.vid_mask = 0xffff;

		l2_key.vlan2.tpid = 0x8100;
		l2_key.vlan2.tpid_mask = 0xffff;
		l2_key.vlan2.pbit = 0;
		l2_key.vlan2.pbit_mask = 0;
		l2_key.vlan2.cfi = 0;
		l2_key.vlan2.cfi_mask = 0;
		l2_key.vlan2.vid = in_vid;
		l2_key.vlan2.vid_mask = 0xffff;
	}

	/* Copy in logical PnC Key */
	memcpy(&(tcam_data->pkt_key.l2_key), &l2_key, sizeof(tpm_l2_acl_key_t));

	return TPM_OK;
}

int32_t tpm_proc_mc_lpbk_xlat_sram_build(uint32_t mod_entry, tpm_pncl_sram_data_t *sram_data)
{
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " mod_entry(%d) \n", mod_entry);

	/* Set sram_updt bitmap */
	sram_data->sram_updt_bm |= (TPM_PNCL_SET_TXP | TPM_PNCL_SET_MOD | TPM_PNCL_SET_LUD | TPM_PNCL_SET_MC);

	/* No next lookup */
	sram_data->next_lu_id = 0;
	sram_data->next_lu_off_reg = 0;

	/* Update dummy register (offset automatically=zero) */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Set txp and txq */
	sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_GMAC1;
	sram_data->pnc_queue = 1;	/* set default mc_lpbk queue via XML */

	/* Set modification command */
	sram_data->flow_id_sub.mod_cmd = mod_entry;

	return TPM_OK;
}

int32_t tpm_proc_mc_create_lpbk_xlate_mod(uint16_t mvlan, uint16_t in_vid, uint16_t out_vid,
					  tpm_trg_port_type_t port_bm, uint32_t *mod_entry)
{
	int32_t ret_code;
	uint8_t first_lpbk;
	tpm_pkt_mod_bm_t mod_bm;
	tpm_pkt_mod_t mod_data;

	memset(&mod_data, 0, sizeof(tpm_pkt_mod_t));

	first_lpbk = 0;
	if (mvlan == in_vid)
		first_lpbk = 1;

	/* Set loopback target ports */
	mod_bm = TPM_MH_SET;
	mod_data.mh_mod = tpm_db_trg_port_switch_port_get(port_bm | TPM_TRG_UNI_VIRT);

	/* Set vlan modification data */
	if (port_bm & TPM_TRG_UNI_VIRT) {
		if (first_lpbk) {
			if (mvlan == out_vid) {
				/* uni-4 is transparent port while all the other uni_ports are transparent ports or strip ports. */
				/* Noop, just do a dummy loopback! */
			} else if (out_vid == 0xffff) {
				mod_bm |= TPM_VLAN_MOD;
				/* uni-4 is strip port and remove outer vlan tag */
				mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_DEL;
			} else {
				mod_bm |= TPM_VLAN_MOD;
				/* uni-4 is translated port and replace outer vlan tag */
				mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_MOD;
				mod_data.vlan_mod.vlan1_out.tpid = 0x8100;
				mod_data.vlan_mod.vlan1_out.pbit = 0;
				mod_data.vlan_mod.vlan1_out.pbit_mask = 0x0;
				mod_data.vlan_mod.vlan1_out.cfi = 0;
				mod_data.vlan_mod.vlan1_out.cfi_mask = 0x0;
				mod_data.vlan_mod.vlan1_out.vid = out_vid;
				mod_data.vlan_mod.vlan1_out.vid_mask = 0xffff;
			}
		} else {
			mod_bm |= TPM_VLAN_MOD;

			if (mvlan == out_vid) {
				/* uni-4 is transparent port while at least one of the other uni_ports is translated ports. */
				/* remove inner vlan tag */
				mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_DEL_INT_MOD;
				mod_data.vlan_mod.vlan2_out.tpid = 0x8100;
				mod_data.vlan_mod.vlan2_out.pbit = 0;
				mod_data.vlan_mod.vlan2_out.pbit_mask = 0x0;
				mod_data.vlan_mod.vlan2_out.cfi = 0;
				mod_data.vlan_mod.vlan2_out.cfi_mask = 0x0;
				mod_data.vlan_mod.vlan2_out.vid = out_vid;
				mod_data.vlan_mod.vlan2_out.vid_mask = 0xffff;
			} else if (out_vid == 0xffff) {
				/* uni-4 is strip port and remove outer and inner vlan tag */
				mod_data.vlan_mod.vlan_op = VLANOP_DEL_2TAG;
			} else {
				/* uni-4 is translated port and remove outer vlan tag, replace inner vlan tag */
				mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_DEL_INT_MOD;
				mod_data.vlan_mod.vlan2_out.tpid = 0x8100;
				mod_data.vlan_mod.vlan2_out.pbit = 0;
				mod_data.vlan_mod.vlan2_out.pbit_mask = 0x0;
				mod_data.vlan_mod.vlan2_out.cfi = 0;
				mod_data.vlan_mod.vlan2_out.cfi_mask = 0x0;
				mod_data.vlan_mod.vlan2_out.vid = out_vid;
				mod_data.vlan_mod.vlan2_out.vid_mask = 0xffff;
			}
		}
	} else {
		if (out_vid == 0xffff || mvlan == out_vid) {
			TPM_OS_WARN(TPM_PNCL_MOD, "Input vlan equals to output vlan, should be transparent, not translate!\n");
			return ERR_GENERAL;
		}
		mod_bm |= TPM_VLAN_MOD;

		if (first_lpbk) {
			mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_MOD_INS;
			mod_data.vlan_mod.vlan1_out.tpid = 0x8100;
			mod_data.vlan_mod.vlan1_out.pbit = 0;
			mod_data.vlan_mod.vlan1_out.pbit_mask = 0x0;
			mod_data.vlan_mod.vlan1_out.cfi = 0;
			mod_data.vlan_mod.vlan1_out.cfi_mask = 0x0;
			mod_data.vlan_mod.vlan1_out.vid = mvlan;
			mod_data.vlan_mod.vlan1_out.vid_mask = 0xffff;
			mod_data.vlan_mod.vlan2_out.tpid = 0x8100;
			mod_data.vlan_mod.vlan2_out.pbit = 0;
			mod_data.vlan_mod.vlan2_out.pbit_mask = 0xff;
			mod_data.vlan_mod.vlan2_out.cfi = 0;
			mod_data.vlan_mod.vlan2_out.cfi_mask = 0xff;
			mod_data.vlan_mod.vlan2_out.vid = out_vid;
			mod_data.vlan_mod.vlan2_out.vid_mask = 0xffff;
		} else {
			mod_data.vlan_mod.vlan_op = VLANOP_INT_TAG_MOD;
			mod_data.vlan_mod.vlan2_out.tpid = 0x8100;
			mod_data.vlan_mod.vlan2_out.pbit = 0;
			mod_data.vlan_mod.vlan2_out.pbit_mask = 0xff;
			mod_data.vlan_mod.vlan2_out.cfi = 0;
			mod_data.vlan_mod.vlan2_out.cfi_mask = 0xff;
			mod_data.vlan_mod.vlan2_out.vid = out_vid;
			mod_data.vlan_mod.vlan2_out.vid_mask = 0xffff;
		}
	}

	/* Create modification entry */
	ret_code = tpm_mod2_entry_set(TPM_MOD_OWNER_TPM, TPM_ENUM_GMAC_1, mod_bm, TPM_INT_MC_MOD, &mod_data, mod_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_lpbk_calc_rule_num(uint32_t *rule_num)
{
	int32_t ret_code;
	uint32_t _rule_num;
	tpm_db_pnc_range_t range_data;

	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MULTI_LPBK, &range_data);
	IF_ERROR(ret_code);

	_rule_num = tpm_db_mc_lpbk_entries_num_get();

	if (_rule_num > range_data.pnc_range_conf.range_size)
		return TPM_FAIL;

	*rule_num = _rule_num;

	return TPM_OK;
}

int32_t tpm_proc_mc_lpbk_create_acl_rule(uint32_t rule_num, tpm_pncl_pnc_full_t *pnc_data)
{
	int32_t ret_code;
	tpm_db_pnc_range_t range_data;
	uint32_t pnc_range_start = 0, pnc_entries_num = 0, api_start = 0, pnc_entry = 0, pnc_stop_entry = 0;

	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get PNC Range Start */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MULTI_LPBK, &range_data);
	IF_ERROR(ret_code);
	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Calculate absolute PNC entry number to execute */
	pnc_entry = (pnc_range_start + api_start) + rule_num;

	/* Get existing PNC entry number */
	pnc_entries_num = tpm_db_mc_lpbk_entries_num_get();

	/* Call PNC Entry Insert, if this is not the api_section's new last entry */
	if (rule_num < pnc_entries_num) {
		pnc_stop_entry = (pnc_range_start + api_start) + (pnc_entries_num - 1);
		ret_code = tpm_pncl_entry_insert(pnc_entry, pnc_stop_entry, pnc_data);
		IF_ERROR(ret_code);
	} else {		/* Otherwise just set the entry (no insertion) */

		ret_code = tpm_pncl_entry_set(pnc_entry, pnc_data);
		IF_ERROR(ret_code);
	}

	/* Decrease number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_MULTI_LPBK);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_lpbk_delete_acl_rule(uint32_t rule_num)
{
	int32_t ret_code;
	tpm_db_pnc_range_t range_data;
	uint32_t pnc_range_start = 0, pnc_entries_num = 0, api_start = 0, pnc_start_entry = 0, pnc_stop_entry = 0;

	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get PNC Range Start */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MULTI_LPBK, &range_data);
	IF_ERROR(ret_code);
	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Get existing PNC entry number */
	pnc_entries_num = tpm_db_mc_lpbk_entries_num_get();

	/* Pull range from this index untill last used entry in Pnc range */
	pnc_start_entry = (pnc_range_start + api_start) + rule_num;
	pnc_stop_entry = (pnc_range_start + api_start) + (pnc_entries_num - 1);

	/* Delete PNC entry */
	ret_code = tpm_pncl_entry_delete(pnc_start_entry, pnc_stop_entry);
	IF_ERROR(ret_code);

	/* Increase number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_MULTI_LPBK);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_set_translation_uni_x_first(tpm_ip_ver_t ip_ver, uint16_t mvlan, uint16_t out_vid, tpm_trg_port_type_t port_bm)
{
	int32_t ret_code;
	uint32_t rule_num = 0, mod_entry = 0;
	tpm_pncl_pnc_full_t pnc_data;

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/*********** Create Modification Entries **********/
	ret_code = tpm_proc_mc_create_lpbk_xlate_mod(mvlan, mvlan, out_vid, port_bm, &mod_entry);
	IF_ERROR(ret_code);

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_tcam_build(ip_ver, mvlan, mvlan, &(pnc_data.pncl_tcam));
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_sram_build(mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(ret_code);

	/* Calculate ACL Rule Num */
	ret_code = tpm_proc_mc_lpbk_calc_rule_num(&rule_num);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK ACL Rule */
	ret_code = tpm_proc_mc_lpbk_create_acl_rule(rule_num, &pnc_data);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK Rule DB */
	ret_code = tpm_db_mc_lpbk_entry_set(ip_ver, mvlan, mvlan, out_vid, rule_num, mod_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_set_translation_uni_x_next(tpm_ip_ver_t ip_ver, uint16_t mvlan, uint16_t in_vid, uint16_t out_vid,
					       tpm_trg_port_type_t port_bm)
{
	int32_t ret_code;
	uint32_t rule_num = 0, mod_entry = 0;
	tpm_pncl_pnc_full_t pnc_data;

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/*********** Create Modification Entries **********/
	ret_code = tpm_proc_mc_create_lpbk_xlate_mod(mvlan, in_vid, out_vid, port_bm, &mod_entry);
	IF_ERROR(ret_code);

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_tcam_build(ip_ver, mvlan, in_vid, &(pnc_data.pncl_tcam));
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_sram_build(mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(ret_code);

	/* Calculate ACL Rule Num */
	ret_code = tpm_proc_mc_lpbk_calc_rule_num(&rule_num);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK ACL Rule */
	ret_code = tpm_proc_mc_lpbk_create_acl_rule(rule_num, &pnc_data);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK Rule DB */
	ret_code = tpm_db_mc_lpbk_entry_set(ip_ver, mvlan, in_vid, out_vid, rule_num, mod_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_set_translation_max_uni_first(tpm_ip_ver_t ip_ver, uint16_t mvlan, uint16_t out_vid)
{
	int32_t ret_code;
	uint32_t rule_num = 0, mod_entry = 0, port_bm = TPM_TRG_UNI_VIRT;
	tpm_pncl_pnc_full_t pnc_data;

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/*********** Create Modification Entries **********/
	ret_code = tpm_proc_mc_create_lpbk_xlate_mod(mvlan, mvlan, out_vid, port_bm, &mod_entry);
	IF_ERROR(ret_code);

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_tcam_build(ip_ver, mvlan, mvlan, &(pnc_data.pncl_tcam));
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_sram_build(mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(ret_code);

	/* Calculate ACL Rule Num */
	ret_code = tpm_proc_mc_lpbk_calc_rule_num(&rule_num);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK ACL Rule */
	ret_code = tpm_proc_mc_lpbk_create_acl_rule(rule_num, &pnc_data);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK Rule DB */
	ret_code = tpm_db_mc_lpbk_entry_set(ip_ver, mvlan, mvlan, out_vid, rule_num, mod_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_set_translation_max_uni_next(tpm_ip_ver_t ip_ver, uint16_t mvlan, uint16_t in_vid, uint16_t out_vid)
{
	int32_t ret_code;
	uint32_t rule_num = 0, mod_entry = 0, port_bm = TPM_TRG_UNI_VIRT;
	tpm_pncl_pnc_full_t pnc_data;

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/*********** Create Modification Entries **********/
	ret_code = tpm_proc_mc_create_lpbk_xlate_mod(mvlan, in_vid, out_vid, port_bm, &mod_entry);
	IF_ERROR(ret_code);

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_tcam_build(ip_ver, mvlan, in_vid, &(pnc_data.pncl_tcam));
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_mc_lpbk_xlat_sram_build(mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(ret_code);

	/* Calculate ACL Rule Num */
	ret_code = tpm_proc_mc_lpbk_calc_rule_num(&rule_num);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK ACL Rule */
	ret_code = tpm_proc_mc_lpbk_create_acl_rule(rule_num, &pnc_data);
	IF_ERROR(ret_code);

	/* Set MULTI_LPBK Rule DB */
	ret_code = tpm_db_mc_lpbk_entry_set(ip_ver, mvlan, in_vid, out_vid, rule_num, mod_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_mc_reset_translation(uint16_t mvlan)
{
	int32_t ret_code;
	uint32_t xits_num = 0, xit_id, rule_index, rule_num, mod_entry;
	uint32_t ip_ver;
	uint8_t lpbk_enable;

	for (ip_ver = TPM_IP_VER_4; ip_ver < TPM_IP_VER_MAX; ip_ver++) {
		tpm_db_get_mc_lpbk_enable(ip_ver, &lpbk_enable);
		if (lpbk_enable == TPM_FALSE)
			continue;
		ret_code = tpm_db_mc_vlan_xits_num_get(ip_ver, mvlan, &xits_num);
		if (ret_code == TPM_OK) {
			for (xit_id = 0; xit_id < xits_num; xit_id++) {
				ret_code = tpm_db_mc_vlan_xit_entry_get(ip_ver, mvlan, xit_id, &rule_index, &mod_entry);
				IF_ERROR(ret_code);

				ret_code = tpm_db_mc_lpbk_rule_num_get(rule_index, &rule_num);
				IF_ERROR(ret_code);

				ret_code = tpm_proc_mc_lpbk_delete_acl_rule(rule_num);
				IF_ERROR(ret_code);

				ret_code = tpm_db_mc_lpbk_entry_invalidate(rule_num);
				IF_ERROR(ret_code);

				if (mod_entry != 0) {
					ret_code = tpm_mod2_entry_del(TPM_MOD_OWNER_TPM, TPM_ENUM_GMAC_1, mod_entry);
					IF_ERROR(ret_code);
				}
			}
		}
	}
	return TPM_OK;
}

int32_t tpm_proc_mc_refresh_translation(uint16_t mvlan)
{
	int32_t ret_code;
	uint32_t stream_id;
	tpm_db_mc_stream_entry_t stream_data;
	tpm_trg_port_type_t dest_port_bm;

	for (stream_id = 0; stream_id < TPM_MC_MAX_STREAM_NUM; stream_id++) {
		if (tpm_db_get_mc_stream_entry(stream_id, &stream_data) == TPM_OK) {
			if (stream_data.vid == mvlan) {
				if (tpm_db_get_mc_per_uni_vlan_xlate() == 0) {
					ret_code = tpm_proc_del_ipv4_mc_stream(0, stream_id);
					IF_ERROR(ret_code);

					ret_code = tpm_proc_add_ipv4_mc_stream(0, stream_id, stream_data.igmp_mode,
									       stream_data.mc_stream_pppoe,
									       stream_data.vid, stream_data.src_addr,
									       stream_data.group_addr,
									       stream_data.src_valid,
									       stream_data.dest_queue,
									       stream_data.dest_port_bm);
					IF_ERROR(ret_code);
				} else {
					if (stream_data.u4_entry != 0xffff) {
						dest_port_bm = (~TPM_TRG_UNI_VIRT) & stream_data.dest_port_bm;
						ret_code = tpm_proc_updt_ipv4_mc_stream(0, stream_id, dest_port_bm);
						IF_ERROR(ret_code);

						dest_port_bm |= TPM_TRG_UNI_VIRT;
						ret_code = tpm_proc_updt_ipv4_mc_stream(0, stream_id, dest_port_bm);
						IF_ERROR(ret_code);
					}
				}
			}
		}
	}

	return TPM_OK;
}

void tpm_proc_mc_vid_remove_ai_bits(tpm_rule_action_t *rule_action,
				    tpm_src_port_type_t src_port,
				    tpm_l2_acl_key_t *l2_key,
				    tpm_error_code_t ret_code)
{
	tpm_dir_t dir = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "\n");

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	if ((TPM_RC_OK == ret_code) || (TPM_DIR_US == dir)) {
		/* no need to remove mc_vid */
		return;
	}

	if (TPM_ACTION_SPEC_MC_VID & rule_action->pkt_act) {
		ret_code = tpm_db_mc_vlan_reset_mc_vlan(l2_key->vlan1.vid);
		/* no need to check if this action failed */
	}

	return;
}

tpm_error_code_t tpm_proc_add_mc_vid_cfg_vtu(uint32_t mc_vid, tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config)
{
	tpm_error_code_t ret;
	uint32_t i;
	uint32_t uni_xlate_enable;
	uint32_t switch_init;
	tpm_init_virt_uni_t    virt_uni_info;
	tpm_src_port_type_t switch_api_port;
	tpm_mc_vid_port_cfg_t *vir_uni_cfg = NULL;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);

	if (tpm_db_switch_init_get(&switch_init))
		return ERR_GENERAL;

	if (switch_init == 0) {
		/* do not have switch */
		TPM_OS_INFO(TPM_PNCL_MOD, "do not have switch, no VTU operation\n");
		return TPM_RC_OK;
	}

	uni_xlate_enable = tpm_db_get_mc_per_uni_vlan_xlate();
	tpm_db_virt_info_get(&virt_uni_info);


	for (i = 0; i < TPM_MAX_NUM_UNI_PORTS; i++) {
		if (TPM_SRC_PORT_UNI_VIRT == mc_vid_uniports_config->mc_vid_port_vids[i].tpm_src_port) {
			vir_uni_cfg = &(mc_vid_uniports_config->mc_vid_port_vids[i]);
			continue;
		}
		if (TPM_MC_UNI_MODE_EXCLUDE == mc_vid_uniports_config->mc_vid_port_vids[i].mc_uni_port_mode)
			continue;

		switch_api_port = mc_vid_uniports_config->mc_vid_port_vids[i].tpm_src_port;

		ret = tpm_sw_port_add_vid(0, switch_api_port, mc_vid);
		if (TPM_RC_OK != ret) {
			TPM_OS_ERROR(TPM_PNCL_MOD, "add port(%d) into vlan(%d) failed\n", switch_api_port, mc_vid);
			return ERR_GENERAL;
		}

		if (uni_xlate_enable) {
			if (TPM_MC_UNI_MODE_TRANSPARENT == mc_vid_uniports_config->mc_vid_port_vids[i].mc_uni_port_mode) {
				/* egress mode set to as-is */
				ret = tpm_sw_set_port_vid_egress_mode(0, switch_api_port, mc_vid, MEMBER_EGRESS_UNMODIFIED);
			} else {
				/* egress mode set to un-tag */
				ret = tpm_sw_set_port_vid_egress_mode(0, switch_api_port, mc_vid, MEMBER_EGRESS_UNTAGGED);
			}
		} else {
			if (TPM_MC_UNI_MODE_STRIP == mc_vid_uniports_config->mc_vid_port_vids[i].mc_uni_port_mode) {
				/* egress mode set to un-tag */
				ret = tpm_sw_set_port_vid_egress_mode(0, switch_api_port, mc_vid, MEMBER_EGRESS_UNTAGGED);
			} else {
				/* egress mode set to as-is */
				ret = tpm_sw_set_port_vid_egress_mode(0, switch_api_port, mc_vid, MEMBER_EGRESS_UNMODIFIED);
			}
		}

		if (TPM_RC_OK != ret) {
			TPM_OS_ERROR(TPM_PNCL_MOD, "set port(%d) egress mode failed\n",
				     switch_api_port);
			return ERR_GENERAL;
		}

	}

	if (TPM_VIRT_UNI_DISABLED == virt_uni_info.enabled) {
		TPM_OS_INFO(TPM_PNCL_MOD, "virtual uni is disabled, do not need to set VTU of uni_4!\n");
		return TPM_RC_OK;
	}

	if (uni_xlate_enable) {
		ret = tpm_sw_port_add_vid(0, TPM_SRC_PORT_UNI_VIRT, mc_vid);
		if (TPM_RC_OK != ret) {
			TPM_OS_ERROR(TPM_PNCL_MOD, "add port(%d) into vlan failed\n", TPM_SRC_PORT_UNI_VIRT);
			return ERR_GENERAL;
		}
	} else if (NULL != vir_uni_cfg) {
		if (TPM_MC_UNI_MODE_EXCLUDE == vir_uni_cfg->mc_uni_port_mode) {
			/* do not add port into VTU */
		} else {
			ret = tpm_sw_port_add_vid(0, TPM_SRC_PORT_UNI_VIRT, mc_vid);
			if (TPM_RC_OK != ret) {
				TPM_OS_ERROR(TPM_PNCL_MOD, "add port(%d) into vlan failed\n", TPM_SRC_PORT_UNI_VIRT);
				return ERR_GENERAL;
			}

			if (TPM_MC_UNI_MODE_STRIP == vir_uni_cfg->mc_uni_port_mode) {
				/* egress mode set to un-tag */
				ret = tpm_sw_set_port_vid_egress_mode(0, TPM_SRC_PORT_UNI_VIRT, mc_vid, MEMBER_EGRESS_UNTAGGED);
			} else if (TPM_MC_UNI_MODE_TRANSPARENT == vir_uni_cfg->mc_uni_port_mode) {
				/* egress mode set to as-is */
				ret = tpm_sw_set_port_vid_egress_mode(0, TPM_SRC_PORT_UNI_VIRT, mc_vid, MEMBER_EGRESS_UNMODIFIED);
			}
		}
	}

	return TPM_RC_OK;
}

tpm_error_code_t tpm_proc_del_mc_vid_cfg_vtu(uint32_t mc_vid)
{
	tpm_error_code_t ret;
	tpm_db_error_t db_ret;
	uint32_t vtu_vid;
	uint32_t i;
	uint32_t uni_xlate_enable;
	tpm_mc_vid_port_cfg_t *mc_vid_uniports_config = NULL;
	uint32_t switch_init;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);

	if (tpm_db_switch_init_get(&switch_init))
		return ERR_GENERAL;

	if (switch_init == 0) {
		/* do not have switch */
		TPM_OS_INFO(TPM_PNCL_MOD, "do not have switch, no VTU operation\n");
		return TPM_RC_OK;
	}

	uni_xlate_enable = tpm_db_get_mc_per_uni_vlan_xlate();

	db_ret = tpm_db_get_mc_vid_cfg(mc_vid, &mc_vid_uniports_config);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "get db of vlan(%d) failed\n", mc_vid);
		return ERR_GENERAL;
	}

	for (i = 0; i < TPM_MAX_NUM_UNI_PORTS; i++) {
		if (TPM_SRC_PORT_UNI_VIRT == mc_vid_uniports_config[i].tpm_src_port)
			continue;

		if (TPM_MC_UNI_MODE_EXCLUDE == mc_vid_uniports_config[i].mc_uni_port_mode)
			continue;

		if (uni_xlate_enable)
			vtu_vid = mc_vid;
		else
			vtu_vid	= mc_vid_uniports_config[i].uni_port_vid;

		ret = tpm_sw_port_del_vid(0, mc_vid_uniports_config[i].tpm_src_port, vtu_vid);
		if (TPM_RC_OK != ret) {
			TPM_OS_ERROR(TPM_PNCL_MOD, "del port(%d) into vlan(%d) failed\n",
				     mc_vid_uniports_config[i].tpm_src_port, vtu_vid);
			return ERR_GENERAL;
		}
	}

	return TPM_RC_OK;
}

tpm_error_code_t tpm_proc_add_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config)
{
	uint32_t ret;
	uint32_t i, in_loop;
	uint32_t out_vid = 0;
	uint32_t vtu_vid = mc_vid;
	uint32_t tranparent_port_num = 0;
	uint32_t ip_ver;
	uint8_t lpbk_enable = TPM_FALSE;
	tpm_mc_vid_t mc_vid_cfg_tmp;
	tpm_mc_vid_port_cfg_t *vir_uni_cfg = NULL;
	tpm_init_virt_uni_t virt_uni_info;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);

	memset(&mc_vid_cfg_tmp, 0, sizeof(mc_vid_cfg_tmp));

	for (i = 0; i < TPM_MAX_NUM_UNI_PORTS; i++) {
		if (TPM_SRC_PORT_UNI_VIRT == mc_vid_uniports_config->mc_vid_port_vids[i].tpm_src_port) {
			vir_uni_cfg = &mc_vid_uniports_config->mc_vid_port_vids[i];
			continue;
		}

		if (TPM_MC_UNI_MODE_TRANSPARENT == mc_vid_uniports_config->mc_vid_port_vids[i].mc_uni_port_mode)
			tranparent_port_num++;

		/* do not care TRANSPARENT and TRIP */
		if (TPM_MC_UNI_MODE_TRANSLATE != mc_vid_uniports_config->mc_vid_port_vids[i].mc_uni_port_mode)
			continue;

		/* here got a translate */
		for (in_loop = 0; in_loop < mc_vid_cfg_tmp.vid_num; in_loop++) {
			if (mc_vid_uniports_config->mc_vid_port_vids[i].uni_port_vid ==
			    mc_vid_cfg_tmp.mc_vid_port[in_loop].mc_vid) {
				mc_vid_cfg_tmp.mc_vid_port[in_loop].port_bm |=
				(TPM_TRG_UNI_0 << (mc_vid_uniports_config->mc_vid_port_vids[i].tpm_src_port - TPM_SRC_PORT_UNI_0));
				break;
			}
		}

		if (mc_vid_cfg_tmp.vid_num == in_loop) {
			mc_vid_cfg_tmp.mc_vid_port[in_loop].port_bm =
				(TPM_TRG_UNI_0 << (mc_vid_uniports_config->mc_vid_port_vids[i].tpm_src_port - TPM_SRC_PORT_UNI_0));
			mc_vid_cfg_tmp.mc_vid_port[in_loop].mc_vid =
				mc_vid_uniports_config->mc_vid_port_vids[i].uni_port_vid;
			mc_vid_cfg_tmp.vid_num++;
		}
	}

	/* validation */
	if (!tpm_db_get_mc_per_uni_vlan_xlate()) {
		/* all the ports should have the same xlate, or strip */
		if (mc_vid_cfg_tmp.vid_num > 1) {
			TPM_OS_ERROR(TPM_PNCL_MOD,
				     "with mc_per_uni_vlan_xlate disabled, all the ports should have the same xlate\n");
			return ERR_GENERAL;
		}

		/* there should be no transparent and xlate port at the same time */
		if ((0 != tranparent_port_num) && (0 != mc_vid_cfg_tmp.vid_num)) {
			TPM_OS_ERROR(TPM_PNCL_MOD,
				     "with mc_per_uni_vlan_xlate disabled, "\
				     "there should be no transparent and xlate port at the same time\n");
			return ERR_GENERAL;
		}

		if (mc_vid_cfg_tmp.vid_num > 0)
			vtu_vid = mc_vid_cfg_tmp.mc_vid_port[0].mc_vid;

	}

	ret = tpm_proc_add_mc_vid_cfg_vtu(vtu_vid, mc_vid_uniports_config);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "set VTU failed, ret: %d\n", ret);
		return ret;
	}

	/* add to db */
	ret = tpm_db_set_mc_vid_cfg(mc_vid, mc_vid_uniports_config);
	if (TPM_DB_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "add mc vid xlate to DB failed, ret: %d\n", ret);
		return ret;
	}

	if (!tpm_db_get_mc_per_uni_vlan_xlate()) {
		/* mc_per_uni_vlan_xlate is disabled, do not add xlate PNC rules */
		return TPM_RC_OK;
	}

	/* deal with IPV4 to V6 */
	for (ip_ver = TPM_IP_VER_4; ip_ver < TPM_IP_VER_MAX; ip_ver++) {
		tpm_db_get_mc_lpbk_enable(ip_ver, &lpbk_enable);
		if (lpbk_enable == TPM_FALSE)
			continue;
		/* first deal with U0 to U3 */
		for (i = 0; i < mc_vid_cfg_tmp.vid_num; i++) {
			if (0 == i) {
				/* first xlate */
				ret =
				tpm_proc_mc_set_translation_uni_x_first(ip_ver, mc_vid, mc_vid_cfg_tmp.mc_vid_port[i].mc_vid,
									mc_vid_cfg_tmp.mc_vid_port[i].port_bm);
				if (TPM_RC_OK != ret) {
					TPM_OS_ERROR(TPM_PNCL_MOD, "add mc vid xlate failed, ret: %d\n", ret);
					return ret;
				}
			} else {
				ret = tpm_proc_mc_set_translation_uni_x_next(ip_ver, mc_vid, mc_vid_cfg_tmp.mc_vid_port[i - 1].mc_vid,
									     mc_vid_cfg_tmp.mc_vid_port[i].mc_vid,
									     mc_vid_cfg_tmp.mc_vid_port[i].port_bm);

				if (TPM_RC_OK != ret) {
					TPM_OS_ERROR(TPM_PNCL_MOD, "add mc vid xlate failed, ret: %d\n", ret);
					return ret;
				}
			}
		}
	}

	/* deal with U4 */
	tpm_db_virt_info_get(&virt_uni_info);
	if (   (TPM_VIRT_UNI_DISABLED == virt_uni_info.enabled)
		|| (NULL == vir_uni_cfg)){
		/* VIRT_UNI_DISABLED, or nothing to do */
		return TPM_RC_OK;
	}
	if (TPM_MC_UNI_MODE_EXCLUDE == vir_uni_cfg->mc_uni_port_mode) {
		/* not member, nothing to do */
		return TPM_RC_OK;
	} else if (TPM_MC_UNI_MODE_TRANSPARENT == vir_uni_cfg->mc_uni_port_mode)
		out_vid = mc_vid;
	else if (TPM_MC_UNI_MODE_STRIP == vir_uni_cfg->mc_uni_port_mode)
		out_vid = 0xffff;
	else if (TPM_MC_UNI_MODE_TRANSLATE == vir_uni_cfg->mc_uni_port_mode)
		out_vid = vir_uni_cfg->uni_port_vid;

	for (ip_ver = TPM_IP_VER_4; ip_ver < TPM_IP_VER_MAX; ip_ver++) {
		tpm_db_get_mc_lpbk_enable(ip_ver, &lpbk_enable);
		if (lpbk_enable == TPM_FALSE)
			continue;
		if (0 == mc_vid_cfg_tmp.vid_num) {
			/* U4 is the first xlate */
			ret = tpm_proc_mc_set_translation_max_uni_first(ip_ver, mc_vid, out_vid);
			if (TPM_RC_OK != ret) {
				TPM_OS_ERROR(TPM_PNCL_MOD, "add U4 mc vid xlate failed, ret: %d\n", ret);
				return ret;
			}
		} else {
			ret = tpm_proc_mc_set_translation_max_uni_next(ip_ver, mc_vid, mc_vid_cfg_tmp.mc_vid_port[i - 1].mc_vid, out_vid);
			if (TPM_RC_OK != ret) {
				TPM_OS_ERROR(TPM_PNCL_MOD, "add U4 mc vid xlate failed, ret: %d\n", ret);
				return ret;
			}
		}
	}

	return TPM_RC_OK;
}

tpm_error_code_t tpm_proc_remove_mc_vid_cfg(uint32_t mc_vid)
{
	tpm_error_code_t ret;
	tpm_db_error_t db_ret;

	ret = tpm_proc_mc_reset_translation(mc_vid);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "remove VID failed, ret: %d\n", ret);
		return ret;
	}

	ret = tpm_proc_del_mc_vid_cfg_vtu(mc_vid);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "remove VID MTU, ret: %d\n", ret);
		return ret;
	}

	db_ret = tpm_db_remove_mc_vid_cfg(mc_vid);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "del mc vid xlate from DB failed, ret: %d\n", db_ret);
		return ERR_GENERAL;
	}

	return TPM_RC_OK;
}

tpm_error_code_t tpm_proc_update_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config)
{
	tpm_error_code_t ret;

	ret = tpm_proc_remove_mc_vid_cfg(mc_vid);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "remove VID failed, ret: %d\n", ret);
		return ret;
	}

	ret = tpm_proc_add_mc_vid_cfg(mc_vid, mc_vid_uniports_config);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "add VID failed, ret: %d\n", ret);
		return ret;
	}

	return TPM_RC_OK;
}

tpm_error_code_t tpm_proc_set_mc_vid_port_vids(uint32_t owner_id,
					       uint32_t mc_vid,
					       tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config)
{
	uint32_t ret;
	int32_t db_ret;
	uint32_t i;
	bool already_exist;
	tpm_src_port_type_t src_port;
	tpm_init_virt_uni_t virt_uni_info;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "mc_vlan(%d)\n", mc_vid);

	for (src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_UNI_7; src_port++) {

		if (TPM_MC_UNI_MODE_EXCLUDE == mc_vid_uniports_config->mc_vid_port_vids[src_port].mc_uni_port_mode)
			continue;

		/* if port is valid */
		if(!tpm_db_eth_port_valid(src_port)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "uni_port(%x) is not valid, should be EXCLUDE mode\n", src_port);
			return ERR_MC_DST_PORT_INVALID;
		}
	}

	/* check vir uni */
	db_ret = tpm_db_virt_info_get(&virt_uni_info);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "get virt uni cfg failed, ret: %d\n", db_ret);
		return ERR_GENERAL;
	}
	if (    (TPM_VIRT_UNI_DISABLED == virt_uni_info.enabled)
	     && (TPM_MC_UNI_MODE_EXCLUDE != mc_vid_uniports_config->mc_vid_port_vids[TPM_SRC_PORT_UNI_VIRT].mc_uni_port_mode)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "TPM_TRG_UNI_VIRT is not enabled, should be EXCLUDE mode\n");
		return ERR_MC_DST_PORT_INVALID;
	}

	/* if all the ports are TPM_MC_UNI_MODE_EXCLUDE, remove this VID
	   if this VID already exists, update it.
	   else add this VID
	 */
	already_exist = tpm_db_mc_vid_exist(mc_vid);

	for (i = 0; i < TPM_MAX_NUM_UNI_PORTS; i++) {
		if (TPM_MC_UNI_MODE_EXCLUDE == mc_vid_uniports_config->mc_vid_port_vids[i].mc_uni_port_mode)
			continue;

		if (true == already_exist) {
			/* VID already exists, update it */
			ret = tpm_proc_update_mc_vid_cfg(mc_vid, mc_vid_uniports_config);
			if (TPM_RC_OK != ret) {
				TPM_OS_ERROR(TPM_PNCL_MOD, "update VID failed, ret: %d\n", ret);
				return ERR_GENERAL;
			}
		} else {
			/* VID not exists, add it */
			ret = tpm_proc_add_mc_vid_cfg(mc_vid, mc_vid_uniports_config);
			if (TPM_RC_OK != ret) {
				TPM_OS_ERROR(TPM_PNCL_MOD, "add VID failed, ret: %d\n", ret);
				return ERR_GENERAL;
			}
		}

		return TPM_RC_OK;
	}

	/* all the ports are TPM_MC_UNI_MODE_EXCLUDE, remove this VID */
	if (false == already_exist) {
		/* VID not exists, error */
		TPM_OS_ERROR(TPM_PNCL_MOD, "all the ports are TPM_MC_UNI_MODE_EXCLUDE, VID not exists, error\n");
		return ERR_GENERAL;
	}

	ret = tpm_proc_remove_mc_vid_cfg(mc_vid);
	if (TPM_RC_OK != ret) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "VID not exists, error\n");
		return ERR_GENERAL;
	}

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_proc_multicast_reset()
*
* DESCRIPTION:    Main function for reset multicast configurations and shadows.
*
* INPUTS:
*
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_multicast_reset(void)
{
    /* reset multicast configuration database:
	   1. MC VID AI-bits allocation
	   2. MC VID per-port config
	   3. IGMP proxy sa addr */
	tpm_db_mc_cfg_reset();

    /* reset multicast rules database:
	   1. MC stream table
	   2. MC mac table
	   3. MC lpbk table
	   4. virt_uni entry table */
	tpm_db_mc_rule_reset();

	return TPM_OK;
}

/*******************************************************************************
* tpm_proc_del_l2_prim_acl_rule()
*
* DESCRIPTION:    Main function for deleting L2 API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_l2_prim_acl_rule(uint32_t owner_id, uint32_t rule_idx, uint32_t ext_call)
{

	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_L2_PRIM_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	/* release mc AI bits */
	tpm_db_mc_vlan_reset_ai_bit(rule_idx);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_get_next_valid_rule()
*
* DESCRIPTION:    Main function for getting next rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_get_next_valid_rule(uint32_t owner_id,
					      int32_t current_index,
					      tpm_api_type_t rule_type,
					      int32_t *next_index,
					      uint32_t *rule_idx,
					      tpm_rule_entry_t *tpm_rule)
{
	int32_t int_ret_code;
	uint32_t bi_dir = 0;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_con;
	tpm_api_sections_t api_section;

	if (!next_index || !tpm_rule)
		return ERR_NULL_POINTER;

	tpm_db_api_section_get_from_api_type(rule_type, &api_section);

	int_ret_code = tpm_db_api_entry_val_get_next(api_section, current_index,
						     next_index, rule_idx, &bi_dir, tpm_rule, &mod_con, &pnc_con);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_omci_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the OMCI management protocol.
*                   The API sets the gemportid, the Rx input queue in the CPU, and the
*                   Tx T-CONT and queue parameters, which are configured in the driver.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* gem_port           - for OMCI Rx frames - the gem port wherefrom the OMCI frames are received.
* cpu_rx_queue       - for OMCI Rx frames - the CPU rx queue number.
* tcont_num          - for OMCI Tx frames - the TCONT number where to send the OMCI frames.
* cpu_tx_queue       - for OMCI Tx frames - the CPU tx queue number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_omci_add_channel(uint32_t owner_id,
					   tpm_gem_port_key_t gem_port,
					   uint32_t cpu_rx_queue,
					   tpm_trg_port_type_t tcont_num,
					   uint32_t cpu_tx_queue)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_gmac_bm_t gmac_bm;
	uint32_t pnc_entry;
	tpm_db_pnc_range_t range_data;
	int32_t int_ret_code;
	tpm_error_code_t ret_code;
	uint32_t valid, rxq, txq, txp;
	uint16_t gem;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "owner(%d),gemport(%d),rx_q(%d),tcont(%d),tx_q(%d)\n",
		     owner_id, gem_port, cpu_rx_queue, tcont_num, cpu_tx_queue);

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	tpm_db_omci_channel_get(&valid, &gem, &rxq, &txq, &txp);
	if (valid)
		IF_ERROR(ERR_MNGT_CREATE_DUPLICATE_CHANNEL);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	/* build TCAM */
	pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_GEMPORT;
	pnc_data.pncl_tcam.l3_parse_bm = 0;
	pnc_data.pncl_tcam.ipv6_parse_bm = 0;
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;
	pnc_data.pncl_tcam.add_info_mask = 0;
	pnc_data.pncl_tcam.add_info_data = 0;
	/* src port */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	pnc_data.pncl_tcam.port_ids = gmac_bm;
	pnc_data.pncl_tcam.pkt_key.l2_key.gem_port = gem_port;
	pnc_data.pncl_tcam.lu_id = 0;
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	/* Build SRAM */
	pnc_data.pncl_sram.next_lu_id = 0;
	pnc_data.pncl_sram.next_lu_off_reg = 0;
	pnc_data.pncl_sram.next_offset.offset_base = 0;
	pnc_data.pncl_sram.next_offset.offset_sub.l2_subf = 0;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.pnc_queue = cpu_rx_queue;
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_TXP | TPM_PNCL_SET_RX_SPECIAL;
	pnc_data.pncl_sram.mh_reg.mh_set = TPM_FALSE;
	pnc_data.pncl_sram.mh_reg.mh_reg = 0;
	pnc_data.pncl_sram.add_info_data = 0;
	pnc_data.pncl_sram.add_info_mask = 0;
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_OTHER;
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
	pnc_data.pncl_sram.flow_id_sub.mod_cmd = 0;
	pnc_data.pncl_sram.flow_id_sub.gem_port = 0;

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_MNGMT_DS, &range_data);
	IF_ERROR(int_ret_code);
	pnc_entry = range_data.pnc_range_conf.range_start;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);
	/* Decrease number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_MNGMT_DS);
	IF_ERROR(int_ret_code);
	int_ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	{
		char cmd[64];
		uint32_t tcontid = 0;

		if (tcont_num == TPM_TRG_TCONT_0)
			tcontid = 0;
		else if (tcont_num == TPM_TRG_TCONT_1)
			tcontid = 1;
		else if (tcont_num == TPM_TRG_TCONT_2)
			tcontid = 2;
		else if (tcont_num == TPM_TRG_TCONT_3)
			tcontid = 3;
		else if (tcont_num == TPM_TRG_TCONT_4)
			tcontid = 4;
		else if (tcont_num == TPM_TRG_TCONT_5)
			tcontid = 5;
		else if (tcont_num == TPM_TRG_TCONT_6)
			tcontid = 6;
		else if (tcont_num == TPM_TRG_TCONT_7)
			tcontid = 7;

		/*sprintf(cmd, "echo %d  > /sys/devices/platform/neta/pon/omci_gp", gem_port); */
		pnc_mh_omci(gem_port, 0x0FFF, cpu_rx_queue);
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " %s\n", cmd);
	}

	/* update database */
	tpm_db_omci_channel_set(gem_port, cpu_rx_queue, cpu_tx_queue, tcont_num);

	return(TPM_RC_OK);

}

/*******************************************************************************
* tpm_proc_omci_del_channel()
*
* DESCRIPTION:      Deletes an existing communication channel for the OMCI management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_omci_del_channel(uint32_t owner_id)
{
	int32_t ret_code;
	tpm_db_pnc_conn_t pnc_con;
	uint32_t oam_configured, cpu_rx_queue, cpu_tx_queue, llid_num;
	tpm_db_pnc_range_t range_data;

	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d)\n", owner_id);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	tpm_db_oam_channel_get(&oam_configured, &cpu_rx_queue, &cpu_tx_queue, &llid_num);
	if (!oam_configured)
		IF_ERROR(ERR_MNGT_DEL_CHANNEL_INVALID);

	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MNGMT_DS, &range_data);
	IF_ERROR(ret_code);

	pnc_con.num_pnc_ranges = 1;
	pnc_con.pnc_conn_tbl[0].pnc_range = TPM_PNC_MNGMT_DS;
	pnc_con.pnc_conn_tbl[0].pnc_index = range_data.pnc_range_conf.range_start;

	/* Delete PNC Entry */
	ret_code = tpm_proc_pnc_con_del(&pnc_con);
	IF_ERROR(ret_code);

	tpm_db_omci_channel_remove();

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_oam_epon_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the OAM EPON management protocol.
*                   The API sets the Rx input queue in the CPU, and the
*                   Tx T-CONT and queue parameters, which are configured in the driver.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* cpu_rx_queue       - for OAM (EPON) Rx frames - the CPU rx queue number.
* llid_num           - for OAM (EPON)Tx frames - the LLID number where to send the OMCI frames.
* cpu_tx_queue       - for OAM (EPON) Tx frames - the CPU tx queue number.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_oam_epon_add_channel(uint32_t owner_id, uint32_t cpu_rx_queue, tpm_trg_port_type_t llid_num)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_gmac_bm_t gmac_bm;
	uint32_t pnc_entry;
	tpm_db_pnc_range_t range_data;
	int32_t int_ret_code;
	tpm_error_code_t ret_code;
	uint32_t valid, rxq, txq, txp;
	uint16_t gem;
	uint32_t q_size;
	uint32_t dummy_cpu_tx_queue = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "owner(%d), rx_q(%d),llid(%d),tx_q(%d)\n",
		     owner_id, cpu_rx_queue, llid_num, dummy_cpu_tx_queue);

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	tpm_db_omci_channel_get(&valid, &gem, &rxq, &txq, &txp);
	if (valid)
		IF_ERROR(ERR_MNGT_CREATE_DUPLICATE_CHANNEL);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	/* validate cpu_rx_queue - should be an existing queue of PON MAC - defined previously */
	tpm_db_gmac_rx_q_conf_get(TPM_ENUM_PMAC, cpu_rx_queue, &valid, &q_size);
	if (!valid)
		TPM_OS_WARN(TPM_INIT_MOD, "OAM CPU RX queue is not valid: queue(%d) \n", cpu_rx_queue);

	/* build TCAM */
	pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_ETYPE;
	pnc_data.pncl_tcam.l3_parse_bm = 0;
	pnc_data.pncl_tcam.ipv6_parse_bm = 0;
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;
	pnc_data.pncl_tcam.add_info_mask = 0;
	pnc_data.pncl_tcam.add_info_data = 0;

	/* src port */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	pnc_data.pncl_tcam.port_ids = gmac_bm;
	pnc_data.pncl_tcam.pkt_key.l2_key.ether_type = 0x8809;
	pnc_data.pncl_tcam.lu_id = 0;
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	/* Build SRAM */
	pnc_data.pncl_sram.next_lu_id = 0;
	pnc_data.pncl_sram.next_lu_off_reg = 0;
	pnc_data.pncl_sram.next_offset.offset_base = 0;
	pnc_data.pncl_sram.next_offset.offset_sub.l2_subf = 0;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.pnc_queue = cpu_rx_queue;
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_TXP | TPM_PNCL_SET_RX_SPECIAL;
	pnc_data.pncl_sram.mh_reg.mh_set = TPM_FALSE;
	pnc_data.pncl_sram.mh_reg.mh_reg = 0;
	pnc_data.pncl_sram.add_info_data = 0;
	pnc_data.pncl_sram.add_info_mask = 0;
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_OTHER;
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;
	pnc_data.pncl_sram.flow_id_sub.mod_cmd = 0;
	pnc_data.pncl_sram.flow_id_sub.gem_port = 0;

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_MNGMT_DS, &range_data);
	IF_ERROR(int_ret_code);
	pnc_entry = range_data.pnc_range_conf.range_start;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);

	/* Decrease number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_MNGMT_DS);
	IF_ERROR(int_ret_code);

	int_ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	{
		uint32_t llid = 0;

		if (llid_num == TPM_TRG_LLID_0)
			llid = 0;
		else if (llid_num == TPM_TRG_LLID_1)
			llid = 1;
		else if (llid_num == TPM_TRG_LLID_2)
			llid = 2;
		else if (llid_num == TPM_TRG_LLID_3)
			llid = 3;
		else if (llid_num == TPM_TRG_LLID_4)
			llid = 4;
		else if (llid_num == TPM_TRG_LLID_5)
			llid = 5;
		else if (llid_num == TPM_TRG_LLID_6)
			llid = 6;
		else if (llid_num == TPM_TRG_LLID_7)
			llid = 7;

		pnc_eoam(cpu_rx_queue);
	}

	/* update database */
	tpm_db_oam_channel_set(cpu_rx_queue, dummy_cpu_tx_queue, llid_num);

	return(TPM_RC_OK);
}
/*******************************************************************************
* tpm_proc_loop_detect_del_channel()
*
* DESCRIPTION:      remove the communication channel for the loop detection management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_loop_detect_del_channel(uint32_t owner_id)
{
	uint32_t pnc_entry;
	tpm_db_pnc_range_t range_data;
	tpm_error_code_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "owner(%d)\n", owner_id);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	memset(&range_data, 0, sizeof(range_data));
	/* Get PNC Range data */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_LOOP_DET_US, &range_data);
	IF_ERROR(ret_code);

	if (!range_data.valid) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "range TPM_PNC_LOOP_DET_US is not valid\n");
		return ERR_GENERAL;
	}

	if (range_data.pnc_range_conf.range_size == range_data.pnc_range_oper.free_entries) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "there is no loopback detect channel yet\n");
		return ERR_GENERAL;
	}

	pnc_entry = range_data.pnc_range_conf.range_start;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "remove pnc_entry(%d)\n", pnc_entry);

	ret_code = tpm_pnc_entry_inv(pnc_entry);
	IF_ERROR(ret_code);

	ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_LOOP_DET_US);
	IF_ERROR(ret_code);

	/* remove tag PNC rule */
	pnc_entry++;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "remove pnc_entry(%d)\n", pnc_entry);

	ret_code = tpm_pnc_entry_inv(pnc_entry);
	IF_ERROR(ret_code);

	ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_LOOP_DET_US);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}
/*******************************************************************************
* tpm_proc_loop_detect_add_channel()
*
* DESCRIPTION:      Establishes a communication channel for the loop detection management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* ety                    - EtherType of the loop detection Pkt.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_loop_detect_add_channel(uint32_t owner_id, tpm_ether_type_key_t ety)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_gmac_bm_t gmac_bm;
	uint32_t pnc_entry;
	tpm_db_pnc_range_t range_data;
	int32_t int_ret_code;
	tpm_error_code_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "owner(%d)\n", owner_id);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/* build TCAM */
	pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_ETYPE;
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_TAG1_MASK;
	pnc_data.pncl_tcam.add_info_data = 0;

	/* src port */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_UNI_ANY, &gmac_bm);
	pnc_data.pncl_tcam.port_ids = gmac_bm;
	pnc_data.pncl_tcam.pkt_key.l2_key.ether_type = ety;
	pnc_data.pncl_tcam.lu_id = 0;
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	/* Build SRAM */
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.pnc_queue = 0;	/*send to queue 0 by default */
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_TXP | TPM_PNCL_SET_RX_SPECIAL;
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_OTHER;
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

	/* Get PNC Range data */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_LOOP_DET_US, &range_data);
	IF_ERROR(int_ret_code);

	if ((!range_data.valid) || (range_data.pnc_range_conf.range_size < 2)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "range TPM_PNC_LOOP_DET_US is not big enough!\n");
		return ERR_OUT_OF_RESOURCES;
	}

	if (range_data.pnc_range_conf.range_size != range_data.pnc_range_oper.free_entries) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "loopback detect channel has already been added\n");
		return ERR_GENERAL;
	}

	pnc_entry = range_data.pnc_range_conf.range_start;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);

	int_ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	int_ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_LOOP_DET_US);
	IF_ERROR(int_ret_code);

	/* add tag PNC rule */
	pnc_data.pncl_tcam.l2_parse_bm = TPM_L2_PARSE_ONE_VLAN_TAG | TPM_L2_PARSE_ETYPE;
	pnc_data.pncl_tcam.add_info_data = TPM_AI_TAG1_MASK;

	pnc_entry++;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);

	int_ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	int_ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_LOOP_DET_US);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_oam_epon_del_channel()
*
* DESCRIPTION:      Deletes an existing communication channel for the OAM EPON management protocol.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_oam_epon_del_channel(uint32_t owner_id)
{
	int32_t ret_code;
	tpm_db_pnc_conn_t pnc_con;
	uint32_t oam_configured, cpu_rx_queue, cpu_tx_queue, llid_num;
	tpm_db_pnc_range_t range_data;

	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d)\n", owner_id);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	tpm_db_oam_channel_get(&oam_configured, &cpu_rx_queue, &cpu_tx_queue, &llid_num);
	if (!oam_configured)
		IF_ERROR(ERR_MNGT_DEL_CHANNEL_INVALID);

	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MNGMT_DS, &range_data);
	IF_ERROR(ret_code);

	pnc_con.num_pnc_ranges = 1;
	pnc_con.pnc_conn_tbl[0].pnc_range = TPM_PNC_MNGMT_DS;
	pnc_con.pnc_conn_tbl[0].pnc_index = range_data.pnc_range_conf.range_start;

	/* Delete PNC Entry */
	ret_code = tpm_proc_pnc_con_del(&pnc_con);
	IF_ERROR(ret_code);

	tpm_db_oam_channel_remove();

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_oam_loopback_add_channel ()
*
* DESCRIPTION:      Establishes a communication channel for the OAM loopback.
*
*
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_oam_loopback_add_channel(uint32_t owner_id)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_gmac_bm_t gmac_bm;
	uint32_t pnc_entry, pnc_stop_entry, api_rng_entries;
	tpm_db_pnc_range_t range_data;
	int32_t int_ret_code;
	tpm_error_code_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "owner(%d)\n", owner_id);

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));

	/* build TCAM */
	pnc_data.pncl_tcam.l2_parse_bm = 0;
	pnc_data.pncl_tcam.l3_parse_bm = 0;
	pnc_data.pncl_tcam.ipv6_parse_bm = 0;
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;
	pnc_data.pncl_tcam.add_info_mask = 0;
	pnc_data.pncl_tcam.add_info_data = 0;

	/* src port */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	pnc_data.pncl_tcam.port_ids = gmac_bm;
	pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.tpid = 0;
	pnc_data.pncl_tcam.pkt_key.l2_key.vlan1.tpid_mask = 0;
	pnc_data.pncl_tcam.pkt_key.l2_key.ether_type = 0;
	pnc_data.pncl_tcam.lu_id = 0;
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	/* Build SRAM */
	pnc_data.pncl_sram.next_lu_id = 0;
	pnc_data.pncl_sram.next_lu_off_reg = 0;
	pnc_data.pncl_sram.next_offset.offset_base = 0;
	pnc_data.pncl_sram.next_offset.offset_sub.l2_subf = 0;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.pnc_queue = 1;	/*send to queue 1 by default */
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD | TPM_PNCL_SET_TXP;
	pnc_data.pncl_sram.mh_reg.mh_set = TPM_FALSE;
	pnc_data.pncl_sram.mh_reg.mh_reg = 0;
	pnc_data.pncl_sram.add_info_data = 0;
	pnc_data.pncl_sram.add_info_mask = 0;
	pnc_data.pncl_sram.l3_type = TPM_PNCL_L3_OTHER;
	pnc_data.pncl_sram.l4_type = TPM_PNCL_L4_OTHER;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_PMAC0;
	pnc_data.pncl_sram.flow_id_sub.mod_cmd = 0;
	pnc_data.pncl_sram.flow_id_sub.gem_port = 0;

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_L2_MAIN, &range_data);
	IF_ERROR(int_ret_code);
	/*rule 1 is special for OAM loopback. */
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_entry(%d)\n", pnc_entry);

	int_ret_code = tpm_db_api_section_num_entries_get(TPM_L2_PRIM_ACL, &api_rng_entries);
	IF_ERROR(int_ret_code);

	if (api_rng_entries > 0) {
		pnc_stop_entry = pnc_entry + api_rng_entries - 1;
		int_ret_code = tpm_pncl_entry_insert(pnc_entry, pnc_stop_entry, &pnc_data);
		IF_ERROR(int_ret_code);
	} else {
		int_ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
		IF_ERROR(int_ret_code);
	}

	int_ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_L2_MAIN);
	IF_ERROR(int_ret_code);

	/*update datebase */
	tpm_db_oam_loopback_channel_set();

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_oam_loopback_del_channel ()
*
* DESCRIPTION:      Deletes an existing communication channel for the OAM loopback.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_oam_loopback_del_channel(uint32_t owner_id)
{

	int32_t ret_code;
	tpm_db_pnc_conn_t pnc_con;
	uint32_t oam_loopback_configured;
	tpm_db_pnc_range_t range_data;

	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d)\n", owner_id);

	ret_code = tpm_owner_id_check(TPM_API_MGMT, owner_id);
	IF_ERROR(ret_code);

	tpm_db_oam_loopback_state_get(&oam_loopback_configured);
	if (!oam_loopback_configured)
		IF_ERROR(ERR_MNGT_DEL_CHANNEL_INVALID);

	ret_code = tpm_db_pnc_rng_get(TPM_PNC_L2_MAIN, &range_data);
	IF_ERROR(ret_code);

	pnc_con.num_pnc_ranges = 1;
	pnc_con.pnc_conn_tbl[0].pnc_range = TPM_PNC_L2_MAIN;
	pnc_con.pnc_conn_tbl[0].pnc_index = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start;

	ret_code = tpm_proc_pnc_con_del(&pnc_con);
	IF_ERROR(ret_code);

	tpm_db_oam_loopback_channel_remove();

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_add_l3_check()
*
* DESCRIPTION:      The function checks consistency of the tpm_proc_add_l3_type_acl_rule params..
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
* l3_key             - Structure for PPPoE proto or ether type. In order to define a rule for
*                      any ether type, the ether type value should be set to 0xFFFF
* action_drop        - If this stage is dropping the packet.
* next_phase         - Set the next parsing stage for the packet.
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_l3_check(uint32_t owner_id,
				       tpm_src_port_type_t src_port,
				       uint32_t rule_num,
				       tpm_parse_fields_t parse_rule_bm,
				       tpm_parse_flags_t parse_flags_bm,
				       tpm_l3_type_key_t *l3_key,
				       tpm_pkt_frwd_t *pkt_frwd,
				       tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_dir_t dir;
	tpm_db_pon_type_t pon_type;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm = 0;

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Check Source Port */
	ret_code = tpm_proc_src_port_check(src_port);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_SRC_PORT_INVALID);

	/* Get Direction, PON type, Important before other tests */
	tpm_proc_src_port_dir_map(src_port, &dir);
	tpm_db_pon_type_get(&pon_type);

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_L3_TYPE_ACL_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_L3_TYPE_ACL_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check necessary pointers are valid */
	ret_code =
	tpm_proc_check_missing_data(rule_action, NULL /*pkt_mod */ , pkt_frwd, (void *)l3_key, rule_action->pkt_act,
				    parse_rule_bm);
	IF_ERROR(ret_code);

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_L3_TYPE, owner_id);
	IF_ERROR(ret_code);

	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_L3_TYPE_ACL, TPM_RANGE_TYPE_ACL, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_IPv4 && rule_action->next_phase != STAGE_IPv6_NH &&
	    rule_action->next_phase != STAGE_IPv6_GEN && rule_action->next_phase != STAGE_DONE &&
	    rule_action->next_phase != STAGE_CTC_CM) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, &pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_L3_TYPE_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	if (rule_action->next_phase == STAGE_CTC_CM) {
		if (!tpm_ctc_cm_l3_rule2cm_sram(parse_flags_bm, rule_action)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CTC_CM, parse_flag must be MTM_FALSE or CTC CM is disabled\n");
			return(ERR_NEXT_PHASE_INVALID);
		}
		if (FROM_WAN(src_port)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CTC_CM, Src Port can not be WAN\n");
			return(ERR_SRC_PORT_INVALID);
		}
		if ((parse_rule_bm & TPM_L2_PARSE_ETYPE) &&
			(l3_key->ether_type_key == 0x0800)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CTC_CM, ETY can not be IPv4\n");
			return(ERR_ACTION_INVALID);
		}
		if ((parse_rule_bm & TPM_L2_PARSE_PPP_PROT) &&
			(l3_key->pppoe_key.ppp_proto == 0x0021)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CTC_CM, ETY can not be IPv4 (over PPPoE)\n");
			return(ERR_ACTION_INVALID);
		}
	}

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_l3_tcam_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* l3_key            - layer2 key data
* parse_rule_bm     - Parse rules bitmap
* parse_flags_bm    - Parse flags in prim ACL rule
* rule_action       - rule action of this rule
* OUTPUTS:
* l3_tcam_data      - Logical TCAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_l3_tcam_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       tpm_l3_type_key_t *l3_key,
			       tpm_parse_fields_t parse_rule_bm,
			       tpm_parse_flags_t parse_flags_bm,
			       tpm_rule_action_t *rule_action,
			       tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t src_port_field;
	long long parse_int_flags_bm = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) parse_rule_bm(%d) \n",
		     src_port, dir, rule_num, parse_rule_bm);

	/* L3 Parsing, according to bm in param */
	tcam_data->l3_parse_bm = parse_rule_bm;
	/* If PPPoe, fill in pppoe_ses ethertype, incase user forgot */
	if (parse_rule_bm & (TPM_L2_PARSE_PPPOE_SES | TPM_L2_PARSE_PPP_PROT)) {
		l3_key->ether_type_key = ETH_P_PPP_SES;
		tcam_data->l3_parse_bm |= TPM_L2_PARSE_ETYPE;
	}

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* set CTC CnM */
	if(tpm_ctc_cm_l3_rule2cm_tcam(parse_flags_bm, l3_key, rule_action)) {
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	/* Parse the AI flag bits */
	src_port_field.src_port = src_port;
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field, parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data),
					  &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (l3_key) {
		memcpy(&(tcam_data->pkt_key.l3_key.ether_type_key), &l3_key->ether_type_key,
		       sizeof(tpm_ether_type_key_t));
		memcpy(&(tcam_data->pkt_key.l3_key.pppoe_key), &l3_key->pppoe_key, sizeof(tpm_pppoe_key_t));
	}

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_ETH_TYPE, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_l3_sram_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* src_port          - packet source port
* dir               - Packet direction
* rule_num          - API rule number
* pon_type          - WAN technology
* action_drop       - drop or not
* parse_flags_bm    - Parse flags in prim ACL
* next_phase        - next stage
*
* OUTPUTS:
* l3_sram_data      - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_l3_sram_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       uint8_t is_pppoe,
			       tpm_db_pon_type_t pon_type,
			       tpm_pkt_frwd_t *pkt_frwd,
			       tpm_rule_action_t *rule_action,
			       tpm_parse_flags_t parse_flags_bm,
			       tpm_pncl_sram_data_t *sram_data)
{
	tpm_db_pnc_range_conf_t range_conf;
	tpm_ai_vectors_t src_port_field;
	long long int_pkt_act = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), dir(%d), rule_num(%d) \n", src_port, dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Set pppoe_bit */
	if (is_pppoe == TPM_TRUE) {
		int_pkt_act |= TPM_ACTION_SET_PPPOE;
		sram_data->sram_updt_bm |= TPM_PNCL_SET_PPPOE;
	} else
		int_pkt_act |= TPM_ACTION_UNSET_PPPOE;

	/* Set AI Bits */
	src_port_field.src_port = src_port;
	/* if L3 is IPv6, reset AI NH2_lu */
	if (STAGE_IPv6_NH == rule_action->next_phase)
		int_pkt_act |= TPM_ACTION_UNSET_NH2_ITER;
	else if (STAGE_CTC_CM == rule_action->next_phase)
		int_pkt_act |= TPM_ACTION_UNSET_CNM_IPV4 | TPM_ACTION_UNSET_IPV4_PRE_KEY;

	tpm_proc_static_ai_sram_build(&src_port_field,
				      rule_action->pkt_act, int_pkt_act, &(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* Default, don't set MH */
	sram_data->mh_reg.mh_reg = 0;

	/* Update L3_offset_register with etype/pppoe length */
	sram_data->shift_updt_reg = 0;	/* update reg 0 */
	sram_data->next_offset.offset_base = TPM_PNCL_L3_OFFSET;
	sram_data->next_offset.offset_sub.l3_subf = 0xffff;	/* End of Ether_type/pppoe parsing */

	if (rule_action->next_phase == STAGE_DONE) {
		tpm_proc_setstage_done(rule_action, sram_data);

		/* Set L3, L4 to OTHER */
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_L3 | TPM_PNCL_SET_L4);
		sram_data->l3_type = TPM_PNCL_L3_OTHER;
		sram_data->l4_type = TPM_PNCL_L4_OTHER;
	} else if ((rule_action->next_phase == STAGE_IPv4) || (rule_action->next_phase == STAGE_IPv6_NH)) {
		/* next lookup at reg0 */
		sram_data->next_lu_off_reg = 0;

		if (rule_action->next_phase == STAGE_IPv6_NH) {
			ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_NH, &range_conf);
			IF_ERROR(ret_code);
			sram_data->l3_type = TPM_PNCL_L3_IPV6;
			sram_data->sram_updt_bm |= TPM_PNCL_SET_L3;
		}
		if (rule_action->next_phase == STAGE_IPv4) {
			ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_MAIN, &range_conf);
			IF_ERROR(ret_code);
			/* Note: l3_type cannot be set, since ipv4_fragmentation is not known yet*/
		}
		sram_data->next_lu_id = range_conf.base_lu_id;
	} else if (rule_action->next_phase == STAGE_CTC_CM) {
		/* next lookup at reg0 */
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
		sram_data->next_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
		sram_data->next_offset.offset_sub.l2_subf = 0;
		sram_data->next_lu_off_reg = TPM_PNC_CNM_L2_REG;

		/* Set L3, L4 to OTHER */
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_L3 | TPM_PNCL_SET_L4);
		sram_data->l3_type = TPM_PNCL_L3_OTHER;
		sram_data->l4_type = TPM_PNCL_L4_OTHER;

		/* set next loopup id */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " next phase not supported (%d)\n", rule_action->next_phase);
		return(TPM_FAIL);
	}

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_add_l3_type_acl_rule()
*
* DESCRIPTION:      Creates a new L3 type (ether type of pppoe proto) processing ACL.
*                   It is used for operations that are not possible to be performed
*                   in a single ACL or to ease the primary ACL processing
*                   (as a helper of the primary L2 ACL).
*                   The L3 type ACL is optional.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_num           - Entry index to be added in the current ACL
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
* l3_key             - Structure for PPPoE proto or ether type. In order to define a rule for
*                      any ether type, the ether type value should be set to 0xFFFF
* action_drop        - If this stage is dropping the packet.
* next_phase         - Set the next parsing stage for the packet.
*
* OUTPUTS:
*  rule_idx         - Unique rule identification number, which is used when deleting the rule.
*                     (this is not the rule_num)
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_l3_type_acl_rule(uint32_t owner_id,
					       tpm_src_port_type_t src_port,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l3_type_key_t *l3_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	uint8_t is_pppoe = TPM_FALSE;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_api_lu_conf_t lu_conf;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d)\n", owner_id, src_port, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_l3_check(owner_id, src_port, rule_num, parse_rule_bm,
					 parse_flags_bm, l3_key, pkt_frwd, rule_action);
	IF_ERROR(ret_code);

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_L3_TYPE_ACL, &range_id);

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_l3_tcam_build(src_port, dir, rule_num, l3_key, parse_rule_bm,
					      parse_flags_bm, rule_action, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	if (pnc_data.pncl_tcam.pkt_key.l3_key.ether_type_key == ETH_P_PPP_SES)
		is_pppoe = TPM_TRUE;

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_l3_sram_build(src_port, dir, rule_num, is_pppoe, pon_type, pkt_frwd,
					      rule_action, parse_flags_bm, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*** Insert the PNC Entry ***/
	tpm_proc_create_acl_pnc_entry(TPM_L3_TYPE_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
	IF_ERROR(int_ret_code);

	/*********** Update API Range in DB **********/

	/* Set API data */
	api_data.l3_type_key.src_port = src_port;
	api_data.l3_type_key.parse_rule_bm = parse_rule_bm;
	api_data.l3_type_key.parse_flags_bm = parse_flags_bm;
	memcpy(&(api_data.l3_type_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (l3_key)
		memcpy(&(api_data.l3_type_key.l3_key), l3_key, sizeof(tpm_l3_type_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.l3_type_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.l3_type_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Set API modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, TPM_INVALID_GMAC	/*DO NOT SUPPORT L3 MOD!! */ , &mod_con,
				 mod_entry);

	/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
	if (rule_num < api_rng_entries) {
		int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_L3_TYPE_ACL, rule_num, (api_rng_entries - 1));
		IF_ERROR(int_ret_code);
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_L3_TYPE_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Set aging counter group nunmber and msk */
	int_ret_code = tpm_db_pnc_get_lu_conf(TPM_PNC_ETH_TYPE, &lu_conf);
	IF_ERROR(int_ret_code);
	tpm_tcam_set_lu_mask(pnc_entry, (int32_t) lu_conf.lu_mask);
	tpm_tcam_set_cntr_group(pnc_entry, (int32_t) lu_conf.cntr_grp);

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_del_l3_type_acl_rule()
*
* DESCRIPTION:      Deletes an existing ethernet type or PPPoE proto access-list entry .
*                   Any of the existing access-list entries may be deleted. The src_port parameter
*                   determines if the rule to be deleted belongs to the upstream access-list
*                   or the downstream access-list. All parameters are compulsory. If the rule number
*                   does not match the internally stored src_port and parsing key
*                   (parse_rule_bm and l3_key), the API will return an error.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* src_port           - The packet originating source port. The parameter has a double function:
*                       . The source port is part of the rules parsing key.
*                       . The source port determines if the acl entry being
*                         created is for the upstream acl or the downstream acl.
*                      In case of an upstream entry, the parameter determines if the packet
*                      arrives from a specific LAN port or ANY LAN port.
* rule_idx           - Unique rule idenitifcation number specifying the rule to be deleted.
* parse_rule_bm      - Bitmap containing the significant flags for parsing fields of the packet.
* l3_key             - Information to create a parsing key for PPPoE proto or Ethernet type key.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_l3_type_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{

	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_L3_TYPE_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_erase_section()
*
* DESCRIPTION:      Erases a section per an application group.
*
* INPUTS:
* owner_id           - APP owner id  should be used for all API calls.
* api_type          - the API group area to be deleted.
*
* OUTPUTS:
*  None.
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_erase_section(uint32_t owner_id, tpm_api_type_t api_type)
{
	tpm_error_code_t ret_code;
	tpm_api_sections_t api_section;
	int32_t next_idx = 0, cur_idx = -1;
	uint32_t rule_idx, rule_num, dummy_idx, bi_dir;
	tpm_rule_entry_t tpm_rule;
	tpm_db_pnc_conn_t pnc_con;
	tpm_db_mod_conn_t mod_con;
	tpm_rule_entry_t api_data;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) api_type(%d)\n", owner_id, api_type);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(api_type, owner_id);
	IF_ERROR(ret_code);

	if (ILLEGAL_API(api_type))
		IF_ERROR(ERR_API_TYPE_INVALID);

	tpm_db_api_section_get_from_api_type(api_type, &api_section);

	while (next_idx != -1) {
		ret_code = tpm_get_next_valid_rule(owner_id, cur_idx, api_type, &next_idx, &rule_idx, &tpm_rule);
		IF_ERROR(ret_code);
		ret_code = tpm_db_api_rulenum_get(api_section, rule_idx, &rule_num);
		IF_ERROR(ret_code);
		ret_code =
		tpm_db_api_entry_get(api_section, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
		IF_ERROR(ret_code);

		/* Delete PNC Entry */
		ret_code = tpm_proc_pnc_con_del(&pnc_con);
		IF_ERROR(ret_code);
		/* Release related modification entry  (Important, after deleting PNC Entries) */
		if (mod_con.mod_cmd_mac != TPM_INVALID_GMAC)
			ret_code = tpm_mod2_entry_del(TPM_MOD_OWNER_TPM, mod_con.mod_cmd_mac, mod_con.mod_cmd_ind);

		IF_ERROR(ret_code);
	}
	tpm_db_api_section_reset(api_section);
	IF_ERROR(ret_code);

	return ret_code;
}

tpm_error_code_t tpm_proc_add_ipv6_nh_acl_rule(uint32_t owner_id,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_nh_iter_t nh_iter,
					       uint32_t nh,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	uint32_t bi_dir = 0;
	uint32_t pnc_entry, mod_entry = 0;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d), rule_num(%d)\n", owner_id, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_nh_check(owner_id, rule_num, nh_iter, parse_flags_bm, nh, pkt_frwd, rule_action);
	IF_ERROR(ret_code);

	/* Simulate DS only (BI_DIR) */
	dir = TPM_DIR_DS;

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_IPV6_NH_ACL, &range_id);

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_nh_tcam_build(rule_num, nh_iter, nh,
				parse_flags_bm, rule_action, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code =
	tpm_proc_ipv6_nh_sram_build(rule_num, nh_iter, nh, pon_type, pkt_frwd, rule_action, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(range_id, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_IPV6_NH_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_nh_acl_rule(owner_id, l_rule_idx, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_IPV6_NH_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_IPV6_NH_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	api_data.ipv6_nh_key.nh_iter = nh_iter;
	api_data.ipv6_nh_key.nh = nh;
	api_data.ipv6_nh_key.parse_rule_bm = TPM_IPv6_PARSE_NH;
	api_data.ipv6_nh_key.parse_flags_bm = parse_flags_bm;
	if (rule_action != NULL)
		memcpy(&(api_data.ipv6_nh_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	else
		memset(&(api_data.ipv6_nh_key.rule_action), 0, sizeof(tpm_rule_action_t));
	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_nh_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_nh_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Set API modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, TPM_INVALID_GMAC	/*DO NOT SUPPORT NH MOD!! */ , &mod_con,
				 mod_entry);

	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_IPV6_NH_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_IPV6_NH_ACL, rule_num, 1 /*bi_dir */ ,
					    &api_data, &mod_con	/*no mod_con */ , &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_nh_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_IPV6_NH_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_gen_acl_rule(uint32_t owner_id,
						  tpm_src_port_type_t src_port,
						  uint32_t rule_num,
						  uint32_t *rule_idx,
						  tpm_parse_fields_t parse_rule_bm,
						  tpm_parse_flags_t parse_flags_bm,
						  tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
						  tpm_pkt_frwd_t *pkt_frwd,
						  tpm_pkt_mod_t *pkt_mod,
						  tpm_pkt_mod_bm_t pkt_mod_bm,
						  tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	tpm_gmacs_enum_t trg_gmac;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	uint32_t bi_dir = 0;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d)\n", owner_id, src_port, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_gen_check(owner_id, src_port, rule_num, parse_rule_bm, parse_flags_bm,
						 ipv6_gen_key, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_IPV6_GEN_ACL, &range_id);

	/*********** Create Modification Entries **********/

	/* if split mod stage-2 */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* get mod index from p-bit value */
		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
		if (TPM_DB_OK != int_ret_code) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
				pkt_mod->vlan_mod.vlan1_out.pbit);
			return ERR_MOD_INVALID;
		}

		/* the first PMT is for p-bit AS-IS */
		mod_entry++;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
	} else {
		ret_code =
		tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, 0 /*int_mod_bm */ ,
				    &mod_entry, &trg_gmac);
		IF_ERROR(ret_code);
	}

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_gen_tcam_build(src_port, dir, rule_num, ipv6_gen_key,
						      parse_rule_bm, parse_flags_bm, pkt_mod,
						      pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ipv6_gen_sram_build(src_port, dir, rule_num, pon_type, pkt_frwd, rule_action,
						      mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(range_id, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_IPV6_GEN_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_gen_acl_rule(owner_id, l_rule_idx, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_IPV6_GEN_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_IPV6_GEN_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	api_data.ipv6_gen_key.src_port = src_port;
	api_data.ipv6_gen_key.parse_rule_bm = parse_rule_bm;
	api_data.ipv6_gen_key.parse_flags_bm = parse_flags_bm;
	api_data.ipv6_gen_key.pkt_mod_bm = pkt_mod_bm;

	memcpy(&(api_data.ipv6_gen_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (ipv6_gen_key)
		memcpy(&(api_data.ipv6_gen_key.ipv6_gen_key), ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_gen_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_gen_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	if (pkt_mod != NULL)
		memcpy(&(api_data.ipv6_gen_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
	else
		memset(&(api_data.ipv6_gen_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

	/* Set modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_IPV6_GEN_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}
	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_IPV6_GEN_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_gen_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_IPV6_GEN_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_dip_acl_rule(uint32_t owner_id,
						tpm_src_port_type_t src_port,
						uint32_t rule_num,
						uint32_t *rule_idx,
						tpm_parse_fields_t parse_rule_bm,
						tpm_parse_flags_t parse_flags_bm,
						tpm_ipv6_addr_key_t *ipv6_dip_key,
						tpm_pkt_frwd_t *pkt_frwd,
						tpm_pkt_mod_t *pkt_mod,
						tpm_pkt_mod_bm_t pkt_mod_bm,
						tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	tpm_gmacs_enum_t trg_gmac;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	uint32_t bi_dir = 0;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d)\n", owner_id, src_port, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_dip_check(owner_id, src_port, rule_num, parse_rule_bm, parse_flags_bm,
					       ipv6_dip_key, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_IPV6_DIP_ACL, &range_id);

	/*********** Create Modification Entries **********/
	/* if split mod stage-2 */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* get mod index from p-bit value */
		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
		if (TPM_DB_OK != int_ret_code) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
				pkt_mod->vlan_mod.vlan1_out.pbit);
			return ERR_MOD_INVALID;
		}

		/* the first PMT is for p-bit AS-IS */
		mod_entry++;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
	} else {
		ret_code =
		tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, 0 /*int_mod_bm */ ,
				    &mod_entry, &trg_gmac);
		IF_ERROR(ret_code);
	}

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_dip_tcam_build(src_port, dir, rule_num, ipv6_dip_key,
						    parse_rule_bm, parse_flags_bm, pkt_mod,
						    pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ipv6_dip_sram_build(src_port, dir, rule_num, pon_type, pkt_frwd, rule_action,
						    mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(range_id, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_IPV6_DIP_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_dip_acl_rule(owner_id, l_rule_idx, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_IPV6_DIP_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_IPV6_DIP_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	api_data.ipv6_dip_key.src_port = src_port;
	api_data.ipv6_dip_key.parse_rule_bm = parse_rule_bm;
	api_data.ipv6_dip_key.parse_flags_bm = parse_flags_bm;
	api_data.ipv6_dip_key.pkt_mod_bm = pkt_mod_bm;

	memcpy(&(api_data.ipv6_dip_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (ipv6_dip_key)
		memcpy(&(api_data.ipv6_dip_key.ipv6_dipkey), ipv6_dip_key, sizeof(tpm_ipv6_addr_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_dip_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_dip_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	if (pkt_mod != NULL)
		memcpy(&(api_data.ipv6_dip_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
	else
		memset(&(api_data.ipv6_dip_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

	/* Set modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_IPV6_DIP_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}
	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_IPV6_DIP_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_dip_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_IPV6_DIP_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_l4_ports_acl_rule(uint32_t owner_id,
						     tpm_src_port_type_t src_port,
						     uint32_t rule_num,
						     uint32_t *rule_idx,
						     tpm_parse_fields_t parse_rule_bm,
						     tpm_parse_flags_t parse_flags_bm,
						     tpm_l4_ports_key_t *l4_key,
						     tpm_pkt_frwd_t *pkt_frwd,
						     tpm_pkt_mod_t *pkt_mod,
						     tpm_pkt_mod_bm_t pkt_mod_bm,
						     tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0;
	tpm_gmacs_enum_t trg_gmac;
	tpm_dir_t dir = 0;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	uint32_t bi_dir = 0;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d)\n", owner_id, src_port, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_l4ports_check(owner_id, src_port, rule_num, parse_rule_bm, parse_flags_bm,
						   l4_key, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get direction */
	tpm_proc_src_port_dir_map(src_port, &dir);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_L4_ACL, &range_id);

	/*********** Create Modification Entries **********/
	/* if split mod stage-2 */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* get mod index from p-bit value */
		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
		if (TPM_DB_OK != int_ret_code) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
				pkt_mod->vlan_mod.vlan1_out.pbit);
			return ERR_MOD_INVALID;
		}

		/* the first PMT is for p-bit AS-IS */
		mod_entry++;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
	} else {
		/*int_mod_bm must be modified in Phase2 */
		ret_code =
		tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, 0 /*int_mod_bm */ ,
				    &mod_entry, &trg_gmac);
		IF_ERROR(ret_code);
	}

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_l4ports_tcam_build(src_port, dir, rule_num, l4_key,
							parse_rule_bm, parse_flags_bm, rule_action,
							pkt_mod, pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ipv6_l4ports_sram_build(src_port, dir, rule_num, pon_type, pkt_frwd, rule_action,
							mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(range_id, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_L4_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_l4_ports_acl_rule(owner_id, l_rule_idx, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_L4_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_L4_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	api_data.ipv6_l4_key.src_port = src_port;
	api_data.ipv6_l4_key.parse_rule_bm = parse_rule_bm;
	api_data.ipv6_l4_key.parse_flags_bm = parse_flags_bm;
	api_data.ipv6_l4_key.pkt_mod_bm = pkt_mod_bm;

	memcpy(&(api_data.ipv6_l4_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (l4_key)
		memcpy(&(api_data.ipv6_l4_key.l4_key), l4_key, sizeof(tpm_l4_ports_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_l4_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_l4_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	if (pkt_mod != NULL)
		memcpy(&(api_data.ipv6_l4_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
	else
		memset(&(api_data.ipv6_l4_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

	/* Set modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_L4_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}
	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_L4_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_l4_ports_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{

	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	ret_code = tpm_proc_del_acl_rule(TPM_L4_ACL, owner_id, rule_idx, ext_call);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_catch_all_init(void)
{
	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry, cpu_def_rx_queue;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_db_pnc_range_t range_data;
	tpm_init_pnc_last_init_t catch_all_pkt_action;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_CATCH_ALL, &range_data);
	IF_ERROR(int_ret_code);

	/* Get number of entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_start;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "free_ent(%d)\n", free_entries);

	tpm_db_get_catch_all_pkt_action(&catch_all_pkt_action);

	/* TCAM  : Any LU_ID, all GMACs */
	pnc_data.pncl_tcam.lu_id = TPM_PNC_ANY_LUID;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();

	/* Fictive L2 table */
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	 /*SRAM*/
	/* LookupDone */
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Trap or drop packets */
	if (TPM_PNC_LAST_ENTRY_INIT_TRAP == catch_all_pkt_action) {
		/* Get CPU Queue */
		tpm_db_get_cpu_rx_queue(&cpu_def_rx_queue);

		/* Set Target to CPU */
		pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_TXP;
		pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

		/* Set Queue to default CPU Queue */
		pnc_data.pncl_sram.pnc_queue = cpu_def_rx_queue;
	} else {
		/* Discard packet */
		pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_DISC;
	}

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_CATCH_ALL, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

void tpm_proc_check_ipv6_gen_5t_key(uint32_t parse_bm, tpm_ipv6_gen_acl_key_t *gen_key)
{
	if ((parse_bm & TPM_IPv6_PARSE_SIP) == 0) {
		memset(gen_key->ipv6_src_ip_add, 0, 16 * sizeof(uint8_t));
		memset(gen_key->ipv6_src_ip_add_mask, 0, 16 * sizeof(uint8_t));
	}

	if ((parse_bm & TPM_IPv6_PARSE_DSCP) == 0) {
		gen_key->ipv6_dscp = 0;
		gen_key->ipv6_dscp_mask = 0;
	}

	if ((parse_bm & TPM_IPv6_PARSE_HOPL) == 0) {
		gen_key->hop_limit = 0;
	}
}

void tpm_proc_check_ipv6_dip_5t_key(uint32_t parse_bm, tpm_ipv6_addr_key_t *dip_key)
{
	if ((parse_bm & TPM_IPv6_PARSE_DIP) == 0) {
		memset(dip_key->ipv6_ip_add, 0, 16 * sizeof(uint8_t));
		memset(dip_key->ipv6_ip_add_mask, 0, 16 * sizeof(uint8_t));
	}
}

void tpm_proc_check_ipv6_l4_ports_5t_key(uint32_t parse_bm, tpm_l4_ports_key_t *l4_key)
{
	if ((parse_bm & TPM_PARSE_L4_SRC) == 0) {
		l4_key->l4_src_port = 0;
	}

	if ((parse_bm & TPM_PARSE_L4_DST) == 0) {
		l4_key->l4_dst_port = 0;
	}
}

uint8_t tpm_proc_check_ipv6_5t_flow_a_equal_b(uint32_t parse_bm,
												uint32_t proto_a,
												tpm_l4_ports_key_t *l4_key_a,
												tpm_ipv6_gen_acl_key_t *gen_key_a,
												tpm_ipv6_addr_key_t *dip_key_a,
												uint32_t proto_b,
												tpm_l4_ports_key_t *l4_key_b,
												tpm_ipv6_gen_acl_key_t *gen_key_b,
												tpm_ipv6_addr_key_t *dip_key_b)
{
	if (parse_bm & TPM_IPv6_PARSE_NH) {
		if (proto_a != proto_b)
			return TPM_FALSE;
	}

	if (parse_bm & TPM_IPV6_L4_BM_MASK) {
		if (parse_bm & TPM_PARSE_L4_SRC) {
			if (l4_key_a->l4_src_port != l4_key_b->l4_src_port)
				return TPM_FALSE;
		}
		if (parse_bm & TPM_PARSE_L4_DST) {
			if (l4_key_a->l4_dst_port != l4_key_b->l4_dst_port)
				return TPM_FALSE;
		}
	}

	if (parse_bm & TPM_IPV6_GEN_BM_MASK) {
		if (parse_bm & TPM_IPv6_PARSE_SIP) {
			if (memcmp(gen_key_a->ipv6_src_ip_add, gen_key_b->ipv6_src_ip_add, 16 * sizeof(uint8_t)) ||
				memcmp(gen_key_a->ipv6_src_ip_add_mask, gen_key_b->ipv6_src_ip_add_mask, 16 * sizeof(uint8_t)))
				return TPM_FALSE;
		}
	}

	if (parse_bm & TPM_IPv6_PARSE_DIP) {
		if (memcmp(dip_key_a->ipv6_ip_add, dip_key_b->ipv6_ip_add, 16 * sizeof(uint8_t)) ||
			memcmp(dip_key_a->ipv6_ip_add_mask, dip_key_b->ipv6_ip_add_mask, 16 * sizeof(uint8_t)))
			return TPM_FALSE;
	}

	return TPM_TRUE;
}

tpm_error_code_t tpm_proc_add_ipv6_gen_5t_check(uint32_t owner_id,
						tpm_dir_t dir,
						uint32_t rule_num,
						tpm_parse_fields_t parse_rule_bm,
						tpm_parse_flags_t parse_flags_bm,
						tpm_l4_ports_key_t *l4_key,
						tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
						tpm_pkt_frwd_t *pkt_frwd,
						tpm_pkt_mod_bm_t pkt_mod_bm,
						tpm_pkt_mod_t *pkt_mod,
						tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_db_pon_type_t pon_type;
	uint32_t i, subflow_bm, subflow_id, protocol = 0, _subflow_bm, _protocol, _dir, _is_pppoe;
	tpm_db_pnc_range_t range_data;
	tpm_l4_ports_key_t _l4_key;
	tpm_ipv6_gen_acl_key_t _gen_key;
	tpm_ipv6_addr_key_t _dip_key, dip_key;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_src_port_type_t src_port;

	memset(&dip_key, 0, sizeof(tpm_ipv6_addr_key_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Get Direction, PON type, Important before other tests */
	tpm_db_pon_type_get(&pon_type);

	/* Check 5_tuple feature is enabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable != TPM_IPV6_5T_ENABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	/* Check necessary pointers are valid */

	if (rule_action == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "rule_action cannot be NULL\n");
		return(ERR_ACTION_INVALID);
	}

	if ((pkt_mod == NULL) && (SET_MOD(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Mod requested with NULL pointer\n");
		return(ERR_MOD_INVALID);
	}

	if ((pkt_frwd == NULL) && ((SET_TARGET_PORT(rule_action->pkt_act)) || SET_TARGET_QUEUE(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target set requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	if ((l4_key == NULL) || (ipv6_gen_key == NULL)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parsing requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	/* Get GMAC(s) */
	if (dir == TPM_DIR_DS)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV6_GEN_5T_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV6_GEN_5T_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV6_GEN, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_GEN, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_IPV6_GEN_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_IPv6_DIP && rule_action->next_phase != STAGE_DONE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_GEN_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	/* Check 5T-flow keys */
	tpm_proc_check_ipv6_l4_ports_5t_key(parse_rule_bm, l4_key);
	tpm_proc_check_ipv6_gen_5t_key(parse_rule_bm, ipv6_gen_key);

	if (parse_rule_bm & TPM_IPV6_L4_BM_MASK) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) == 0)
			return(ERR_IPV6_KEY_INVALID);
	}

	subflow_bm = parse_rule_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
	if (subflow_bm != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
				protocol = IPPROTO_TCP;
			else
				protocol = IPPROTO_UDP;
			subflow_bm |= TPM_IPv6_PARSE_NH;
		}
		subflow_id = tpm_db_find_matched_ipv6_gen_subflow(dir, subflow_bm, protocol, l4_key, ipv6_gen_key);
		if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID)
			return ERR_IPV6_5T_RULE_EXISTS;

		if (rule_action->next_phase == STAGE_DONE) {
			for (i = 0; i < TPM_DB_IPV6_MAX_5T_FLOW_NUM; i++) {
				memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
				memset(&_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
				memset(&_dip_key, 0, sizeof(tpm_ipv6_addr_key_t));
				ret_code = tpm_db_get_ipv6_5t_flow(i, &_dir, &_subflow_bm, &_is_pppoe, &_protocol, &_l4_key, &_gen_key, &_dip_key);
				if (ret_code != TPM_DB_OK)
					continue;

				if ((subflow_bm & _subflow_bm) == subflow_bm) {
					if (tpm_proc_check_ipv6_5t_flow_a_equal_b(subflow_bm, protocol, l4_key, ipv6_gen_key, &dip_key,
															_protocol, &_l4_key, &_gen_key, &_dip_key))
						return ERR_IPV6_5T_FLOW_PARTIAL_MATCHED;
				} else if ((subflow_bm & _subflow_bm) == _subflow_bm) {
					if (tpm_proc_check_ipv6_5t_flow_a_equal_b(_subflow_bm, _protocol, &_l4_key, &_gen_key, &_dip_key,
															protocol, l4_key, ipv6_gen_key, &dip_key))
						return ERR_IPV6_5T_FLOW_PARTIAL_MATCHED;
				}
			}
		}
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_dip_5t_check(uint32_t owner_id,
						tpm_dir_t dir,
						uint32_t rule_num,
						tpm_parse_fields_t parse_rule_bm,
						tpm_parse_flags_t parse_flags_bm,
						tpm_l4_ports_key_t *l4_key,
						tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
						tpm_ipv6_addr_key_t *ipv6_dip_key,
						tpm_pkt_frwd_t *pkt_frwd,
						tpm_pkt_mod_bm_t pkt_mod_bm,
						tpm_pkt_mod_t *pkt_mod,
						tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_db_pon_type_t pon_type;
	uint32_t i, flow_bm, flow_id, protocol = 0, is_pppoe, _flow_bm, _protocol, _dir, _is_pppoe;
	tpm_db_pnc_range_t range_data;
	tpm_l4_ports_key_t _l4_key;
	tpm_ipv6_gen_acl_key_t _gen_key;
	tpm_ipv6_addr_key_t _dip_key;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_src_port_type_t src_port;

	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Get Direction, PON type, Important before other tests */
	tpm_db_pon_type_get(&pon_type);

	/* Check 5_tuple feature is enabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable != TPM_IPV6_5T_ENABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	/* Check necessary pointers are valid */

	if (rule_action == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "rule_action cannot be NULL\n");
		return(ERR_ACTION_INVALID);
	}

	if ((pkt_mod == NULL) && (SET_MOD(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Mod requested with NULL pointer\n");
		return(ERR_MOD_INVALID);
	}

	if ((pkt_frwd == NULL) && ((SET_TARGET_PORT(rule_action->pkt_act)) || SET_TARGET_QUEUE(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target set requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	if ((l4_key == NULL) || (ipv6_gen_key == NULL) || (ipv6_dip_key == NULL)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parsing requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	/* Get GMAC(s) */
	if (dir == TPM_DIR_DS)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port,
				pkt_frwd->trg_queue, rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV6_DIP_5T_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV6_DIP_5T_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV6_DIP, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_DIP, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_IPV6_DIP_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_DONE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_DIP_ACL_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	/* Check 5T-flow keys */
	tpm_proc_check_ipv6_l4_ports_5t_key(parse_rule_bm, l4_key);
	tpm_proc_check_ipv6_gen_5t_key(parse_rule_bm, ipv6_gen_key);
	tpm_proc_check_ipv6_dip_5t_key(parse_rule_bm, ipv6_dip_key);

	if (parse_rule_bm & TPM_IPV6_L4_BM_MASK) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) == 0)
			return(ERR_IPV6_KEY_INVALID);
	}

	if (parse_rule_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK) {
		flow_bm = parse_rule_bm;
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
				protocol = IPPROTO_TCP;
			else
				protocol = IPPROTO_UDP;
			flow_bm |= TPM_IPv6_PARSE_NH;
		}
		if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
			is_pppoe = TPM_TRUE;
		else
			is_pppoe = TPM_FALSE;
		flow_id = tpm_db_find_matched_ipv6_5t_flow(dir, flow_bm, is_pppoe, protocol, l4_key, ipv6_gen_key, ipv6_dip_key);
		if (flow_id != TPM_DB_INVALID_IPV6_FLOW_ID)
			return ERR_IPV6_5T_RULE_EXISTS;

		for (i = 0; i < TPM_DB_IPV6_MAX_5T_FLOW_NUM; i++) {
			memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
			memset(&_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
			memset(&_dip_key, 0, sizeof(tpm_ipv6_addr_key_t));
			ret_code = tpm_db_get_ipv6_5t_flow(i, &_dir, &_flow_bm, &_is_pppoe, &_protocol, &_l4_key, &_gen_key, &_dip_key);
			if (ret_code != TPM_DB_OK)
				continue;

			if ((flow_bm & _flow_bm) == flow_bm) {
				if (tpm_proc_check_ipv6_5t_flow_a_equal_b(flow_bm, protocol, l4_key, ipv6_gen_key, ipv6_dip_key,
														_protocol, &_l4_key, &_gen_key, &_dip_key))
					return ERR_IPV6_5T_FLOW_PARTIAL_MATCHED;
			} else if ((flow_bm & _flow_bm) == _flow_bm) {
				if (tpm_proc_check_ipv6_5t_flow_a_equal_b(_flow_bm, _protocol, &_l4_key, &_gen_key, &_dip_key,
														protocol, l4_key, ipv6_gen_key, ipv6_dip_key))
					return ERR_IPV6_5T_FLOW_PARTIAL_MATCHED;
			}
		}
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_l4_ports_5t_check(uint32_t owner_id,
						     tpm_dir_t dir,
						     uint32_t rule_num,
						     tpm_parse_fields_t parse_rule_bm,
						     tpm_parse_flags_t parse_flags_bm,
						     tpm_l4_ports_key_t *l4_key,
						     tpm_pkt_frwd_t *pkt_frwd,
						     tpm_pkt_mod_bm_t pkt_mod_bm,
						     tpm_pkt_mod_t *pkt_mod,
						     tpm_rule_action_t *rule_action)
{
	int32_t ret_code;
	tpm_db_pon_type_t pon_type;
	uint32_t i, subflow_bm, subflow_id, protocol = 0, _subflow_bm, _protocol, _dir, _is_pppoe;
	tpm_db_pnc_range_t range_data;
	tpm_l4_ports_key_t _l4_key;
	tpm_ipv6_gen_acl_key_t _gen_key, gen_key;
	tpm_ipv6_addr_key_t _dip_key, dip_key;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_src_port_type_t src_port;

	memset(&gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
	memset(&dip_key, 0, sizeof(tpm_ipv6_addr_key_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Check TPM was successfully initialized */
	if (!tpm_db_init_done_get())
		IF_ERROR(ERR_SW_NOT_INIT);

	/* Get Direction, PON type, Important before other tests */
	tpm_db_pon_type_get(&pon_type);


	/* Check 5_tuple feature is enabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable != TPM_IPV6_5T_ENABLED)
		return ERR_IPV6_API_ILLEGAL_CALL;

	/* Check necessary pointers are valid */

	if (rule_action == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "rule_action cannot be NULL\n");
		return(ERR_ACTION_INVALID);
	}

	if ((pkt_mod == NULL) && (SET_MOD(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Mod requested with NULL pointer\n");
		return(ERR_MOD_INVALID);
	}

	if ((pkt_frwd == NULL) && ((SET_TARGET_PORT(rule_action->pkt_act)) || SET_TARGET_QUEUE(rule_action->pkt_act))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target set requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	if (l4_key == NULL) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Parsing requested with NULL pointer\n");
		return(ERR_FRWD_INVALID);
	}

	/* Get GMAC(s) */
	if (dir == TPM_DIR_DS)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;

	/* Check Target_port and Queue are valid */
	ret_code =
	tpm_proc_check_valid_target(dir, pon_type, src_port, pkt_frwd->trg_port, pkt_frwd->trg_queue,
				rule_action->pkt_act, TPM_FALSE);
	IF_ERROR(ret_code);

	/* Check parse_bm */
	if (parse_rule_bm & (~(api_sup_param_val[TPM_ADD_IPV6_L4_PORTS_5T_RULE].sup_parse_fields))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_rule_bm(0x%x) \n", parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* Check parse_flags_bm */
	if (parse_flags_bm & (~(api_sup_param_val[TPM_ADD_IPV6_L4_PORTS_5T_RULE].sup_parse_flags))) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid parse_flags_bm (0x%x) \n", parse_flags_bm);
		return(ERR_PARSE_MAP_INVALID);
	}
	/* Check parse_flags_bm - TRUE and FALSE are not set together */
	ret_code = tpm_proc_check_parse_flag_valid(parse_flags_bm);
	IF_ERROR(ret_code);

	/* Check owner_id */
	ret_code = tpm_owner_id_check(TPM_API_IPV6_L4, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);

	/* Check rule_num, and api_section is active */
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_L4, &range_data);

	ret_code = tpm_proc_add_api_ent_check(TPM_L4_ACL, range_data.pnc_range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);

	/* Check forwarding rule, currently only support STAGE_DONE */
	if (rule_action->next_phase != STAGE_IPv6_GEN && rule_action->next_phase != STAGE_DONE
		&& rule_action->next_phase != STAGE_CTC_CM) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " Next Phase (%d) is not supported \n", rule_action->next_phase);
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check rule action */
	ret_code = tpm_proc_check_pkt_action(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm);
	IF_ERROR(ret_code);
	if (rule_action->pkt_act & api_sup_param_val[TPM_ADD_IPV6_L4_PORTS_5T_RULE].forbidden_actions) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Packet Action (0x%x) includes forbidden action\n", rule_action->pkt_act);
		return(ERR_ACTION_INVALID);
	}

	/* Check if next stage CTC CnM  */
	if ((rule_action->next_phase == STAGE_CTC_CM) && !(parse_flags_bm & TPM_PARSE_FLAG_MTM_FALSE)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "next stage is CnM, packets must be MTM_FALSE\n");
		return(ERR_NEXT_PHASE_INVALID);
	}

	/* Check 5T-flow keys */
	tpm_proc_check_ipv6_l4_ports_5t_key(parse_rule_bm, l4_key);

	if (parse_rule_bm & TPM_IPV6_L4_BM_MASK) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) == 0)
			return(ERR_IPV6_KEY_INVALID);
	}

	subflow_bm = parse_rule_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
	if (subflow_bm != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
				protocol = IPPROTO_TCP;
			else
				protocol = IPPROTO_UDP;
			subflow_bm |= TPM_IPv6_PARSE_NH;
		}
		subflow_id = tpm_db_find_matched_ipv6_l4_subflow(dir, subflow_bm, protocol, l4_key);
		if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID)
			return ERR_IPV6_5T_RULE_EXISTS;

		if (rule_action->next_phase == STAGE_DONE) {
			for (i = 0; i < TPM_DB_IPV6_MAX_5T_FLOW_NUM; i++) {
				memset(&_l4_key, 0, sizeof(tpm_l4_ports_key_t));
				memset(&_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
				memset(&_dip_key, 0, sizeof(tpm_ipv6_addr_key_t));
				ret_code = tpm_db_get_ipv6_5t_flow(i, &_dir, &_subflow_bm, &_is_pppoe, &_protocol, &_l4_key, &_gen_key, &_dip_key);
				if (ret_code != TPM_DB_OK)
					continue;

				if ((subflow_bm & _subflow_bm) == subflow_bm) {
					if (tpm_proc_check_ipv6_5t_flow_a_equal_b(subflow_bm, protocol, l4_key, &gen_key, &dip_key,
															_protocol, &_l4_key, &_gen_key, &_dip_key))
						return ERR_IPV6_5T_FLOW_PARTIAL_MATCHED;
				} else if ((subflow_bm & _subflow_bm) == _subflow_bm) {
					if (tpm_proc_check_ipv6_5t_flow_a_equal_b(_subflow_bm, _protocol, &_l4_key, &_gen_key, &_dip_key,
															protocol, l4_key, &gen_key, &dip_key))
						return ERR_IPV6_5T_FLOW_PARTIAL_MATCHED;
				}
			}
		}
	}

	return(TPM_RC_OK);
}

int32_t tpm_proc_ipv6_gen_5t_tcam_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_l4_ports_key_t *l4_key,
					tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					tpm_parse_fields_t parse_rule_bm,
					tpm_parse_flags_t parse_flags_bm,
					tpm_rule_action_t *rule_action,
					tpm_pkt_mod_t *pkt_mod,
					tpm_pkt_mod_bm_t pkt_mod_bm,
					tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_src_port_type_t src_port;
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t subflow_field;
	int32_t ret_code;
	uint32_t subflow_bm, subflow_id, protocol = 0;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     	dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* IPV6 parsing, according to parse_rule */
	tcam_data->ipv6_parse_bm = parse_rule_bm & TPM_IPV6_GEN_BM_MASK;

	/* Build the AI bits according to parse_flags */
	subflow_bm = parse_rule_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
	if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
			protocol = IPPROTO_TCP;
		else {
			protocol = IPPROTO_UDP;
			if (dir == TPM_DIR_DS)
				parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_FALSE;
		}
		subflow_bm |= TPM_IPv6_PARSE_NH;
	}
	if (subflow_bm != 0) {
		subflow_id = tpm_db_find_matched_ipv6_l4_subflow(dir, subflow_bm, protocol, l4_key);
		if (subflow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
			return TPM_FAIL;
		subflow_field.ipv6_subflow = subflow_id + 1;
		parse_int_flags_bm |= TPM_PARSE_FLAG_IPV6_SUBFLOW_PARSE;
	}
	parse_flags_bm &= ~TPM_PARSE_FLAG_L4P_MASK;
	/*if Ipv6 split, check AI bit, update tcam*/
	if ((STAGE_CTC_CM == rule_action->next_phase) || ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD))) {

		/*if Ipv6 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}
	tpm_proc_parse_flag_ai_tcam_build(&subflow_field, parse_flags_bm, parse_int_flags_bm,
					  				&(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	if (dir == TPM_DIR_DS)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;
	if (ipv6_gen_key)
		memcpy(&(tcam_data->pkt_key.ipv6_gen_key), ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_GEN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_dip_5t_tcam_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_l4_ports_key_t *l4_key,
					tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					tpm_ipv6_addr_key_t *ipv6_dip_key,
					tpm_parse_fields_t parse_rule_bm,
					tpm_parse_flags_t parse_flags_bm,
					tpm_rule_action_t *rule_action,
					tpm_pkt_mod_t *pkt_mod,
					tpm_pkt_mod_bm_t pkt_mod_bm,
					tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_src_port_type_t src_port;
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t subflow_field;
	int32_t ret_code;
	uint32_t subflow_bm, subflow_id, protocol = 0;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     	dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* IPV6 parsing, according to parse_rule */
	tcam_data->ipv6_parse_bm = parse_rule_bm & TPM_IPv6_PARSE_DIP;

	/* Build the AI bits according to parse_flags */
	if ((parse_rule_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK) ||
		(parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK)){
		subflow_bm = parse_rule_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
				protocol = IPPROTO_TCP;
			else
				protocol = IPPROTO_UDP;
			parse_flags_bm &= ~TPM_PARSE_FLAG_L4P_MASK;
			subflow_bm |= TPM_IPv6_PARSE_NH;
		}
		subflow_id = tpm_db_find_matched_ipv6_gen_subflow(dir, subflow_bm, protocol, l4_key, ipv6_gen_key);
		if (subflow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
			return TPM_FAIL;
		subflow_field.ipv6_subflow = subflow_id + 1;
		parse_int_flags_bm |= TPM_PARSE_FLAG_IPV6_SUBFLOW_PARSE;
	}
	/*if Ipv6 split, check AI bit, update tcam*/
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD)){

		/*if Ipv6 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	tpm_proc_parse_flag_ai_tcam_build(&subflow_field, parse_flags_bm, parse_int_flags_bm,
					  				&(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	if (dir == TPM_DIR_DS)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;
	if (ipv6_dip_key)
		memcpy(&(tcam_data->pkt_key.ipv6_dip_key), ipv6_dip_key, sizeof(tpm_ipv6_addr_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_DIP, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_l4_ports_5t_tcam_build(tpm_dir_t dir,
					     uint32_t rule_num,
					     tpm_l4_ports_key_t *l4_key,
					     tpm_parse_fields_t parse_rule_bm,
					     tpm_parse_flags_t parse_flags_bm,
					     tpm_rule_action_t *rule_action,
					     tpm_pkt_mod_t *pkt_mod,
					     tpm_pkt_mod_bm_t pkt_mod_bm,
					     tpm_pncl_tcam_data_t *tcam_data)
{
	tpm_src_port_type_t src_port;
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	long long parse_int_flags_bm = 0;
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " dir(%d), rule_num(%d) parse_rule_bm(%x) parse_flag_bm(%x) \n",
		     	dir, rule_num, parse_rule_bm, parse_flags_bm);

	/* IPV6 parsing, according to parse_rule */
	tcam_data->l4_parse_bm = parse_rule_bm;

	if (parse_flags_bm & TPM_PARSE_FLAG_L4_UDP) {
		if (dir == TPM_DIR_DS)
			parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_FALSE;
	}
	/*if Ipv6 split, check AI bit, update tcam*/
	if ((STAGE_CTC_CM == rule_action->next_phase) || ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		(VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) &&
		(0xff == pkt_mod->vlan_mod.vlan1_out.pbit_mask) &&
		(pkt_mod_bm == TPM_VLAN_MOD))) {

		/*if Ipv6 split or CTC CnM, check AI bit, update tcam*/
		parse_int_flags_bm |= TPM_PARSE_FLGA_SPLIT_MOD_TRUE;
	}

	/* Build the AI bits according to parse_flags */
	tpm_proc_parse_flag_ai_tcam_build(NULL, parse_flags_bm, parse_int_flags_bm,
					  &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	if (dir == TPM_DIR_DS)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	tcam_data->pkt_key.src_port = src_port;
	if (l4_key)
		memcpy(&(tcam_data->pkt_key.l4_ports_key), l4_key, sizeof(tpm_l4_ports_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_L4, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_gen_5t_sram_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_db_pon_type_t pon_type,
					uint32_t ipv6_gen_subflow,
					tpm_pkt_frwd_t *pkt_frwd,
					tpm_rule_action_t *rule_action,
					long long int_pkt_act,
					tpm_pkt_mod_t *pkt_mod,
					uint32_t mod_cmd,
					tpm_pncl_sram_data_t *sram_data)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_ai_vectors_t subflow_field;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " dir(%d), rule_num(%d) \n", dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Set AI Bits */
	subflow_field.ipv6_subflow = ipv6_gen_subflow + 1;
	tpm_proc_static_ai_sram_build(&subflow_field, rule_action->pkt_act, int_pkt_act,
								&(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* Set L3 */
	sram_data->l3_type = TPM_PNCL_L3_IPV6;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_L3;

	if (rule_action->next_phase == STAGE_DONE) {
		tpm_proc_setstage_done(rule_action, sram_data);

		/* Update dummy register (offset automatically=zero) */
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	} else if (rule_action->next_phase == STAGE_IPv6_DIP) {
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_DIP, &range_conf);
		IF_ERROR(ret_code);

		sram_data->next_lu_id = range_conf.base_lu_id;
		sram_data->next_lu_off_reg = TPM_PNC_IPV6_DIP_REG;

		sram_data->next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
		sram_data->next_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_DIP;
		sram_data->shift_updt_reg = TPM_PNC_IPV6_DIP_REG;
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set Modification */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) {
		/* split mod stage-2 */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}
	else {
		tpm_proc_set_mod(rule_action, sram_data, mod_cmd);
	}

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_dip_5t_sram_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_db_pon_type_t pon_type,
					tpm_pkt_frwd_t *pkt_frwd,
					tpm_rule_action_t *rule_action,
					long long int_pkt_act,
					tpm_pkt_mod_t *pkt_mod,
					uint32_t mod_cmd,
					tpm_pncl_sram_data_t *sram_data)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " dir(%d), rule_num(%d) \n", dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Set AI Bits */
	tpm_proc_static_ai_sram_build(NULL, rule_action->pkt_act, int_pkt_act,
								&(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* Next Stage */
	if (rule_action->next_phase == STAGE_DONE) {
		tpm_proc_setstage_done(rule_action, sram_data);
		sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set Modification */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) {
		/* split mod stage-2 */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}
	else {
		tpm_proc_set_mod(rule_action, sram_data, mod_cmd);
	}

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_l4_ports_5t_sram_build(tpm_dir_t dir,
					     uint32_t rule_num,
					     tpm_db_pon_type_t pon_type,
					     uint32_t ipv6_l4_subflow,
					     tpm_pkt_frwd_t *pkt_frwd,
					     tpm_rule_action_t *rule_action,
					     long long int_pkt_act,
					     tpm_pkt_mod_t *pkt_mod,
					     uint32_t mod_cmd,
					     tpm_pncl_sram_data_t *sram_data)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_ai_vectors_t subflow_field;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " dir(%d), rule_num(%d) \n", dir, rule_num);

	/* If packet Drop, nothing else to do */
	if (PKT_DROP(rule_action->pkt_act)) {
		sram_data->sram_updt_bm |= (TPM_PNCL_SET_DISC | TPM_PNCL_SET_LUD);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Packet Drop\n");
		return(TPM_OK);
	}

	/* Set AI Bits */
	subflow_field.ipv6_subflow = ipv6_l4_subflow + 1;
	tpm_proc_static_ai_sram_build(&subflow_field, rule_action->pkt_act, int_pkt_act,
								&(sram_data->add_info_data), &(sram_data->add_info_mask));

	/* Set L3 */
	sram_data->l3_type = TPM_PNCL_L3_IPV6;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_L3;
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	if (rule_action->next_phase == STAGE_DONE)
		tpm_proc_setstage_done(rule_action, sram_data);
	else if (rule_action->next_phase == STAGE_CTC_CM) {
		/* nothing to Update */
		sram_data->next_lu_off_reg = TPM_PNC_CNM_L2_REG;
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
	}
	else if (rule_action->next_phase == STAGE_IPv6_GEN) {
		/* nothing to Update */
		sram_data->next_lu_off_reg = TPM_PNC_LU_REG0; /* lookup reg 0 */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_GEN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id;
	}
	else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Next stage not supported\n");
		return(TPM_FAIL);
	}

	/* Set Modification */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op) {
		/* split mod stage-2 */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}
	else {
		tpm_proc_set_mod(rule_action, sram_data, mod_cmd);
	}

	/* For Target set PNC TXP, GemPort */
	ret_code = tpm_proc_set_trgt_queue(rule_action, pkt_frwd, dir, pon_type, sram_data);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_set_RI_mh(rule_action, pkt_frwd, dir, sram_data);
	IF_ERROR(ret_code);

	/* Set Customization flag */
	tpm_proc_set_cust_cpu_packet_parse(rule_action, sram_data);

	return(TPM_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_gen_5t_rule(uint32_t owner_id,
					       tpm_dir_t src_dir,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l4_ports_key_t *l4_key,
					       tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_pkt_mod_t *pkt_mod,
					       tpm_pkt_mod_bm_t pkt_mod_bm,
					       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0, bi_dir = 0, is_pppoe;
	tpm_gmacs_enum_t trg_gmac;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;

	uint32_t subflow_bm, subflow_id, protocol = 0;
	tpm_ipv6_addr_key_t dip_key;
	long long int_pkt_act = 0;

	memset(&dip_key, 0, sizeof(tpm_ipv6_addr_key_t));

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_dir(%d), rule_num(%d)\n", owner_id, src_dir, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_gen_5t_check(owner_id, src_dir, rule_num, parse_rule_bm, parse_flags_bm,
						 					l4_key, ipv6_gen_key, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_IPV6_GEN_ACL, &range_id);


	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_GEN, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_IPV6_GEN_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_gen_5t_rule(owner_id, l_rule_idx, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*********** Create Subflow Entries **********/
	subflow_bm = parse_rule_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
	if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
			protocol = IPPROTO_TCP;
		else
			protocol = IPPROTO_UDP;
		subflow_bm |= TPM_IPv6_PARSE_NH;
	}
	subflow_id = TPM_DB_INVALID_IPV6_FLOW_ID;
	if (subflow_bm != 0 && rule_action->next_phase != STAGE_DONE) {
		int_pkt_act |= TPM_ACTION_SET_IPV6_SUBFLOW;

		subflow_id = tpm_db_get_free_ipv6_gen_subflow(src_dir);
		if (subflow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
			return ERR_IPV6_5T_FLOW_AI_BITS_EXHAUSED;
	} else {
		int_pkt_act |= TPM_ACTION_UNSET_IPV6_SUBFLOW;
	}

	/*********** Create Modification Entries **********/
	/* if split mod stage-2 */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* get mod index from p-bit value */
		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
		if (TPM_DB_OK != int_ret_code) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
				pkt_mod->vlan_mod.vlan1_out.pbit);
			return ERR_MOD_INVALID;
		}

		/* the first PMT is for p-bit AS-IS */
		mod_entry++;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
	} else {
		ret_code =
		tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, 0 /*int_mod_bm */ ,
				    &mod_entry, &trg_gmac);
		IF_ERROR(ret_code);
	}
	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_gen_5t_tcam_build(src_dir, rule_num, l4_key, ipv6_gen_key,
		      					parse_rule_bm, parse_flags_bm, rule_action,
		      					pkt_mod, pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ipv6_gen_5t_sram_build(src_dir, rule_num, pon_type, subflow_id,
							pkt_frwd, rule_action, int_pkt_act,
							pkt_mod, mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_IPV6_GEN_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_IPV6_GEN_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	if (src_dir == TPM_DIR_DS)
		api_data.ipv6_gen_key.src_port = TPM_SRC_PORT_WAN;
	else
		api_data.ipv6_gen_key.src_port = TPM_SRC_PORT_UNI_ANY;
	api_data.ipv6_gen_key.parse_rule_bm = parse_rule_bm;
	api_data.ipv6_gen_key.parse_flags_bm = parse_flags_bm;
	api_data.ipv6_gen_key.pkt_mod_bm = pkt_mod_bm;

	memcpy(&(api_data.ipv6_gen_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (l4_key)
		memcpy(&(api_data.ipv6_gen_key.l4_key), l4_key, sizeof(tpm_l4_ports_key_t));
	if (ipv6_gen_key)
		memcpy(&(api_data.ipv6_gen_key.ipv6_gen_key), ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_gen_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_gen_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	if (pkt_mod != NULL)
		memcpy(&(api_data.ipv6_gen_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
	else
		memset(&(api_data.ipv6_gen_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

	/* Set modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_IPV6_GEN_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_IPV6_GEN_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Set 5T Flow Entry */
	if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
		int_ret_code = tpm_db_set_ipv6_gen_subflow(src_dir, subflow_id, subflow_bm, protocol, l4_key, ipv6_gen_key);
		IF_ERROR(int_ret_code);
	}
	if (rule_action->next_phase == STAGE_DONE) {
		if (subflow_bm != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
				is_pppoe = TPM_TRUE;
			else
				is_pppoe = TPM_FALSE;
			int_ret_code = tpm_db_set_ipv6_5t_flow(src_dir, subflow_bm, is_pppoe, protocol, l4_key, ipv6_gen_key, &dip_key);
			IF_ERROR(int_ret_code);
		}
	}

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_dip_5t_rule(uint32_t owner_id,
					       tpm_dir_t src_dir,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l4_ports_key_t *l4_key,
					       tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					       tpm_ipv6_addr_key_t *ipv6_dip_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_pkt_mod_t *pkt_mod,
					       tpm_pkt_mod_bm_t pkt_mod_bm,
					       tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0, bi_dir = 0, is_pppoe;
	tpm_gmacs_enum_t trg_gmac;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;

	uint32_t flow_bm, protocol = 0;
	long long int_pkt_act = 0;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_dir(%d), rule_num(%d)\n", owner_id, src_dir, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_dip_5t_check(owner_id, src_dir, rule_num, parse_rule_bm, parse_flags_bm,
						 				l4_key, ipv6_gen_key, ipv6_dip_key, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_IPV6_DIP_ACL, &range_id);


	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_DIP, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_IPV6_DIP_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_dip_5t_rule(owner_id, l_rule_idx, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*********** Create Subflow Entries **********/
	int_pkt_act |= TPM_ACTION_UNSET_IPV6_SUBFLOW;
	flow_bm = parse_rule_bm & (TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK | TPM_IPv6_PARSE_DIP);
	if (parse_rule_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
				protocol = IPPROTO_TCP;
			else
				protocol = IPPROTO_UDP;
			flow_bm |= TPM_IPv6_PARSE_NH;
		}
	}

	/*********** Create Modification Entries **********/
	/* if split mod stage-2 */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* get mod index from p-bit value */
		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
		if (TPM_DB_OK != int_ret_code) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
				pkt_mod->vlan_mod.vlan1_out.pbit);
			return ERR_MOD_INVALID;
		}

		/* the first PMT is for p-bit AS-IS */
		mod_entry++;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
	} else {
		ret_code =
		tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, 0 /*int_mod_bm */ ,
				    &mod_entry, &trg_gmac);
		IF_ERROR(ret_code);
	}
	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_dip_5t_tcam_build(src_dir, rule_num, l4_key, ipv6_gen_key, ipv6_dip_key,
						      	parse_rule_bm, parse_flags_bm, rule_action,
						      	pkt_mod, pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ipv6_dip_5t_sram_build(src_dir, rule_num, pon_type,
							pkt_frwd, rule_action, int_pkt_act,
							pkt_mod, mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_IPV6_DIP_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_IPV6_DIP_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	if (src_dir == TPM_DIR_DS)
		api_data.ipv6_dip_key.src_port = TPM_SRC_PORT_WAN;
	else
		api_data.ipv6_dip_key.src_port = TPM_SRC_PORT_UNI_ANY;
	api_data.ipv6_dip_key.parse_rule_bm = parse_rule_bm;
	api_data.ipv6_dip_key.parse_flags_bm = parse_flags_bm;
	api_data.ipv6_dip_key.pkt_mod_bm = pkt_mod_bm;

	memcpy(&(api_data.ipv6_dip_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (l4_key)
		memcpy(&(api_data.ipv6_dip_key.l4_key), l4_key, sizeof(tpm_l4_ports_key_t));
	if (ipv6_gen_key)
		memcpy(&(api_data.ipv6_dip_key.ipv6_gen_key), ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));
	if (ipv6_dip_key)
		memcpy(&(api_data.ipv6_dip_key.ipv6_dipkey), ipv6_dip_key, sizeof(tpm_ipv6_addr_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_dip_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_dip_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	if (pkt_mod != NULL)
		memcpy(&(api_data.ipv6_dip_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
	else
		memset(&(api_data.ipv6_dip_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

	/* Set modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_IPV6_DIP_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_IPV6_DIP_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Set 5T Flow Entry */
	if (flow_bm != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
			is_pppoe = TPM_TRUE;
		else
			is_pppoe = TPM_FALSE;
		int_ret_code = tpm_db_set_ipv6_5t_flow(src_dir, flow_bm, is_pppoe, protocol, l4_key, ipv6_gen_key, ipv6_dip_key);
		IF_ERROR(int_ret_code);
	}

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_l4_ports_5t_rule(uint32_t owner_id,
						    tpm_dir_t src_dir,
						    uint32_t rule_num,
						    uint32_t *rule_idx,
						    tpm_parse_fields_t parse_rule_bm,
						    tpm_parse_flags_t parse_flags_bm,
						    tpm_l4_ports_key_t *l4_key,
						    tpm_pkt_frwd_t *pkt_frwd,
						    tpm_pkt_mod_t *pkt_mod,
						    tpm_pkt_mod_bm_t pkt_mod_bm,
						    tpm_rule_action_t *rule_action)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, api_rng_entries = 0;
	uint32_t l_rule_idx = 0, bi_dir = 0;
	tpm_gmacs_enum_t trg_gmac;
	tpm_pnc_ranges_t range_id = 0;
	tpm_db_pon_type_t pon_type = 0;
	tpm_db_mod_conn_t mod_con = { 0, 0};

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;

	uint32_t subflow_bm, subflow_id, protocol = 0, is_pppoe;
	tpm_ipv6_gen_acl_key_t gen_key;
	tpm_ipv6_addr_key_t dip_key;
	long long int_pkt_act = 0;

	memset(&gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
	memset(&dip_key, 0, sizeof(tpm_ipv6_addr_key_t));

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_dir(%d), rule_num(%d)\n", owner_id, src_dir, rule_num);

	/* Check parameters */
	ret_code = tpm_proc_add_ipv6_l4_ports_5t_check(owner_id, src_dir, rule_num, parse_rule_bm, parse_flags_bm,
						 					l4_key, pkt_frwd, pkt_mod_bm, pkt_mod, rule_action);
	IF_ERROR(ret_code);

	/* Get pon_type */
	tpm_db_pon_type_get(&pon_type);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_L4_ACL, &range_id);


	/*********** Remove Current Entries **********/
	tpm_db_pnc_rng_get(TPM_PNC_IPV6_L4, &range_data);

	if (TPM_RANGE_TYPE_TABLE == range_data.pnc_range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_L4_ACL, rule_num, &l_rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_num(%d) already exists\n", owner_id, rule_num);

			ret_code = tpm_proc_del_ipv6_l4_ports_5t_rule(owner_id, rule_num, TPM_INT_CALL);
			IF_ERROR(ret_code);
		}
	}

	/*********** Create Subflow Entries **********/
	subflow_bm = parse_rule_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
	if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
			protocol = IPPROTO_TCP;
		else
			protocol = IPPROTO_UDP;
		subflow_bm |= TPM_IPv6_PARSE_NH;
	}
	subflow_id = TPM_DB_INVALID_IPV6_FLOW_ID;
	if (rule_action->next_phase == STAGE_CTC_CM) {
		/* do not touch sub-flow AI, because they are for UNI also */
		int_pkt_act |= TPM_ACTION_UNSET_CNM_IPV4 | TPM_ACTION_UNSET_IPV4_PRE_KEY;
	} else if (subflow_bm != 0 && rule_action->next_phase != STAGE_DONE) {
		int_pkt_act |= TPM_ACTION_SET_IPV6_SUBFLOW;

		subflow_id = tpm_db_get_free_ipv6_l4_subflow(src_dir);
		if (subflow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
			return ERR_IPV6_5T_FLOW_AI_BITS_EXHAUSED;
	} else {
		int_pkt_act |= TPM_ACTION_UNSET_IPV6_SUBFLOW;
	}

	/*********** Create Modification Entries **********/
	/* if split mod stage-2 */
	if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) &&
		VLANOP_SPLIT_MOD_PBIT == pkt_mod->vlan_mod.vlan_op &&
		(pkt_mod_bm == TPM_VLAN_MOD)) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod stage-2\n");

		if (tpm_proc_trg_port_gmac_map(pkt_frwd->trg_port, &trg_gmac)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "tpm_proc_trg_port_gmac_map failed \n");
			return(ERR_MOD_INVALID);
		} else if (trg_gmac == TPM_INVALID_GMAC) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Target gmac invalid (%d) \n", trg_gmac);
			return(ERR_MOD_INVALID);
		}

		/* get mod index from p-bit value */
		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pkt_mod->vlan_mod.vlan1_out.pbit, &mod_entry);
		if (TPM_DB_OK != int_ret_code) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "split mod stage-2, illeagal p-bit value: %d\n",
				pkt_mod->vlan_mod.vlan1_out.pbit);
			return ERR_MOD_INVALID;
		}

		/* the first PMT is for p-bit AS-IS */
		mod_entry++;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Modification mod_cmd(%d)\n", mod_entry);
	} else {
		ret_code =
		tpm_proc_create_mod(rule_action->pkt_act, pkt_frwd->trg_port, pkt_mod, pkt_mod_bm, 0 /*int_mod_bm */ ,
				    &mod_entry, &trg_gmac);
		IF_ERROR(ret_code);
	}
	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_ipv6_l4_ports_5t_tcam_build(src_dir, rule_num, l4_key,
						      	parse_rule_bm, parse_flags_bm, rule_action,
						      	pkt_mod, pkt_mod_bm, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	/* Build SRAM Entry */
	int_ret_code = tpm_proc_ipv6_l4_ports_5t_sram_build(src_dir, rule_num, pon_type, subflow_id,
							pkt_frwd, rule_action, int_pkt_act,
							pkt_mod, mod_entry, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	/*** Insert the PNC Entry ***/
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
	    /*** Insert the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_acl_pnc_entry(TPM_L4_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code =
		tpm_proc_create_table_pnc_entry(TPM_L4_ACL, rule_num, TPM_FALSE, &pnc_data, &pnc_entry,
						&api_rng_entries);
		IF_ERROR(int_ret_code);
	}

	/*********** Update API Range in DB **********/

	/* Set API data */
	if (src_dir == TPM_DIR_DS)
		api_data.ipv6_l4_key.src_port = TPM_SRC_PORT_WAN;
	else
		api_data.ipv6_l4_key.src_port = TPM_SRC_PORT_UNI_ANY;
	api_data.ipv6_l4_key.parse_rule_bm = parse_rule_bm;
	api_data.ipv6_l4_key.parse_flags_bm = parse_flags_bm;
	api_data.ipv6_l4_key.pkt_mod_bm = pkt_mod_bm;

	memcpy(&(api_data.ipv6_l4_key.rule_action), rule_action, sizeof(tpm_rule_action_t));
	if (l4_key)
		memcpy(&(api_data.ipv6_l4_key.l4_key), l4_key, sizeof(tpm_l4_ports_key_t));

	if (pkt_frwd != NULL)
		memcpy(&(api_data.ipv6_l4_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.ipv6_l4_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	if (pkt_mod != NULL)
		memcpy(&(api_data.ipv6_l4_key.pkt_mod), pkt_mod, sizeof(tpm_pkt_mod_t));
	else
		memset(&(api_data.ipv6_l4_key.pkt_mod), 0, sizeof(tpm_pkt_mod_t));

	/* Set modification data */
	tpm_proc_set_api_moddata(rule_action->pkt_act, trg_gmac, &mod_con, mod_entry);

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
	if (TPM_RANGE_TYPE_ACL == range_data.pnc_range_conf.range_type) {
		if (rule_num < api_rng_entries) {
			int_ret_code = tpm_proc_api_entry_rulenum_inc(TPM_L4_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(int_ret_code);
		}
	}

	/* Set new API Entry */
	int_ret_code = tpm_db_api_entry_set(TPM_L4_ACL, rule_num, 0 /*bi_dir */ ,
					    &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(int_ret_code);

	/* Set 5T Flow Entry */
	if (subflow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
		int_ret_code = tpm_db_set_ipv6_l4_subflow(src_dir, subflow_id, subflow_bm, protocol, l4_key);
		IF_ERROR(int_ret_code);
	}
	if (rule_action->next_phase == STAGE_DONE) {
		if (subflow_bm != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
				is_pppoe = TPM_TRUE;
			else
				is_pppoe = TPM_FALSE;
			int_ret_code = tpm_db_set_ipv6_5t_flow(src_dir, subflow_bm, is_pppoe, protocol, l4_key, &gen_key, &dip_key);
			IF_ERROR(int_ret_code);
		}
	}

	/* Return Output */
	*rule_idx = l_rule_idx;

	return(TPM_RC_OK);
}


tpm_error_code_t tpm_proc_int_del_5t_rule(tpm_api_type_t api_type, tpm_proc_common_int_del_func_t int_del_func,
					  uint32_t busy_check, uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code, busy_ret_code = TPM_RC_OK;

	/* Check API_type busy */
	if (TPM_TRUE == busy_check) {
		ret_code = tpm_proc_check_api_busy(api_type, rule_idx);
		IF_ERROR(ret_code);
	}

	/* Call internal function */
	ret_code = (*int_del_func)(owner_id, rule_idx);

	/* Free API_type */
	if (TPM_TRUE == busy_check)
		busy_ret_code = tpm_proc_api_busy_done(api_type, rule_idx);

	IF_ERROR(ret_code);
	IF_ERROR(busy_ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_int_del_ipv6_gen_5t_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;
	uint32_t rule_num, dummy_idx, src_dir, bi_dir, is_pppoe;
	tpm_db_pnc_range_t range_data;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con = { 0, 0};
	tpm_db_pnc_conn_t pnc_con;

	uint32_t parse_rule_bm, parse_flags_bm, flow_bm, flow_id, flow_user, protocol = 0;
	tpm_rule_action_t rule_action;
	tpm_l4_ports_key_t *l4_key = NULL;
	tpm_ipv6_gen_acl_key_t *gen_key = NULL;
	tpm_ipv6_addr_key_t dip_key;

	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));
	memset(&api_data, 0, sizeof(tpm_rule_entry_t));
	memset(&rule_action, 0, sizeof(rule_action));
	memset(&dip_key, 0, sizeof(tpm_ipv6_addr_key_t));

	tpm_db_pnc_rng_get(TPM_PNC_IPV6_GEN, &range_data);

	/* Get the rule_num */
	if (range_data.pnc_range_conf.range_type == TPM_RANGE_TYPE_ACL) {
		ret_code = tpm_db_api_rulenum_get(TPM_IPV6_GEN_ACL, rule_idx, &rule_num);
		if (ret_code == TPM_DB_ERR_REC_NOT_EXIST) {
			TPM_OS_ERROR(TPM_DB_MOD, " The rule non-exist!\n");
			return ERR_RULE_IDX_INVALID;
		}
	} else
		rule_num = rule_idx;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "with rule_num(%d)\n", rule_num);

	ret_code = tpm_db_api_entry_get(TPM_IPV6_GEN_ACL, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
	IF_ERROR(ret_code);

	tpm_proc_src_port_dir_map(api_data.ipv6_gen_key.src_port, &src_dir);

	parse_rule_bm = api_data.ipv6_gen_key.parse_rule_bm;
	parse_flags_bm = api_data.ipv6_gen_key.parse_flags_bm;
	rule_action = api_data.ipv6_gen_key.rule_action;

	l4_key = &api_data.ipv6_gen_key.l4_key;
	gen_key = &api_data.ipv6_gen_key.ipv6_gen_key;


	flow_bm = parse_rule_bm & TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK;
	if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
			protocol = IPPROTO_TCP;
		else
			protocol = IPPROTO_UDP;
		flow_bm |= TPM_IPv6_PARSE_NH;
	}
	if (flow_bm != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
			is_pppoe = TPM_TRUE;
		else
			is_pppoe = TPM_FALSE;
		flow_id = tpm_db_find_matched_ipv6_5t_flow(src_dir, flow_bm, is_pppoe, protocol, l4_key, gen_key, &dip_key);
		if (flow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
			ret_code = tpm_db_del_ipv6_5t_flow(flow_id);
			IF_ERROR(ret_code);
		}
		if (rule_action.next_phase != STAGE_DONE) {
			flow_id = tpm_db_find_matched_ipv6_gen_subflow(src_dir, flow_bm, protocol, l4_key, gen_key);
			if (flow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
				IF_ERROR(ERR_GENERAL);

			flow_user = tpm_db_get_ipv6_gen_subflow_user_num(src_dir, flow_id);

			if (flow_user == 0) {
				ret_code = tpm_db_del_ipv6_gen_subflow(src_dir, flow_id);
				IF_ERROR(ret_code);
			} else {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d) is used by other ipv6 flows! \n", rule_num);
				IF_ERROR(ERR_IPV6_5T_RULE_IN_USE);
			}
		}
	}
	/* Call generic del function without locking, since locking has already been done */
	ret_code = tpm_proc_del_acl_rule(TPM_IPV6_GEN_ACL, owner_id, rule_idx, TPM_FALSE);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_gen_5t_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d), ext_call(%d)", owner_id, rule_idx, ext_call);

	ret_code = tpm_proc_int_del_5t_rule(TPM_API_IPV6_GEN, &tpm_proc_int_del_ipv6_gen_5t_rule,
					    ext_call, owner_id, rule_idx);
	return(ret_code);
}


tpm_error_code_t tpm_proc_int_del_ipv6_dip_5t_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;
	uint32_t rule_num, dummy_idx, src_dir, bi_dir, is_pppoe;
	tpm_db_pnc_range_t range_data;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con = {0, 0};
	tpm_db_pnc_conn_t pnc_con;

	uint32_t parse_rule_bm, parse_flags_bm, flow_bm, flow_id, protocol = 0;
	tpm_rule_action_t rule_action;
	tpm_l4_ports_key_t *l4_key = NULL;
	tpm_ipv6_gen_acl_key_t *gen_key = NULL;
	tpm_ipv6_addr_key_t *dip_key = NULL;

	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));
	memset(&api_data, 0, sizeof(tpm_rule_entry_t));
	memset(&rule_action, 0, sizeof(rule_action));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	tpm_db_pnc_rng_get(TPM_PNC_IPV6_DIP, &range_data);

	/* Get the rule_num */
	if (range_data.pnc_range_conf.range_type == TPM_RANGE_TYPE_ACL) {
		ret_code = tpm_db_api_rulenum_get(TPM_IPV6_DIP_ACL, rule_idx, &rule_num);
		if (ret_code == TPM_DB_ERR_REC_NOT_EXIST) {
			TPM_OS_ERROR(TPM_DB_MOD, " The rule non-exist!\n");
			return ERR_RULE_IDX_INVALID;
		}
	} else
		rule_num = rule_idx;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "with rule_num(%d)\n", rule_num);

	ret_code = tpm_db_api_entry_get(TPM_IPV6_DIP_ACL, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
	IF_ERROR(ret_code);

	tpm_proc_src_port_dir_map(api_data.ipv6_dip_key.src_port, &src_dir);

	parse_rule_bm = api_data.ipv6_dip_key.parse_rule_bm;
	parse_flags_bm = api_data.ipv6_dip_key.parse_flags_bm;
	rule_action = api_data.ipv6_dip_key.rule_action;

	l4_key = &api_data.ipv6_dip_key.l4_key;
	gen_key = &api_data.ipv6_dip_key.ipv6_gen_key;
	dip_key = &api_data.ipv6_dip_key.ipv6_dipkey;


	flow_bm = parse_rule_bm & (TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK | TPM_IPv6_PARSE_DIP);
	if (flow_bm != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
			if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
				protocol = IPPROTO_TCP;
			else
				protocol = IPPROTO_UDP;
			flow_bm |= TPM_IPv6_PARSE_NH;
		}
		if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
			is_pppoe = TPM_TRUE;
		else
			is_pppoe = TPM_FALSE;
		flow_id = tpm_db_find_matched_ipv6_5t_flow(src_dir, flow_bm, is_pppoe, protocol, l4_key, gen_key, dip_key);
		if (flow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
			ret_code = tpm_db_del_ipv6_5t_flow(flow_id);
			IF_ERROR(ret_code);
		}
	}

	/* Call generic del function without locking, since locking has already been done */
	ret_code = tpm_proc_del_acl_rule(TPM_IPV6_DIP_ACL, owner_id, rule_idx, TPM_FALSE);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_dip_5t_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d), ext_call(%d)", owner_id, rule_idx, ext_call);

	ret_code = tpm_proc_int_del_5t_rule(TPM_IPV6_DIP_ACL, &tpm_proc_int_del_ipv6_dip_5t_rule,
					    ext_call, owner_id, rule_idx);
	return(ret_code);
}

tpm_error_code_t tpm_proc_int_del_ipv6_l4_ports_5t_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;
	uint32_t rule_num, dummy_idx, src_dir, bi_dir, is_pppoe;
	tpm_db_pnc_range_t range_data;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con = { 0, 0};
	tpm_db_pnc_conn_t pnc_con;

	uint32_t parse_rule_bm, parse_flags_bm, flow_bm, flow_id, flow_user, protocol = 0;
	tpm_rule_action_t rule_action;
	tpm_l4_ports_key_t *l4_key = NULL;
	tpm_ipv6_gen_acl_key_t gen_key;
	tpm_ipv6_addr_key_t dip_key;

	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));
	memset(&api_data, 0, sizeof(tpm_rule_entry_t));
	memset(&rule_action, 0, sizeof(rule_action));
	memset(&gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));
	memset(&dip_key, 0, sizeof(tpm_ipv6_addr_key_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_idx);

	tpm_db_pnc_rng_get(TPM_PNC_IPV6_L4, &range_data);

	/* Get the rule_num */
	if (range_data.pnc_range_conf.range_type == TPM_RANGE_TYPE_ACL) {
		ret_code = tpm_db_api_rulenum_get(TPM_L4_ACL, rule_idx, &rule_num);
		if (ret_code == TPM_DB_ERR_REC_NOT_EXIST) {
			TPM_OS_ERROR(TPM_DB_MOD, " The rule non-exist!\n");
			return ERR_RULE_IDX_INVALID;
		}
	} else
		rule_num = rule_idx;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "with rule_num(%d)\n", rule_num);

	ret_code = tpm_db_api_entry_get(TPM_L4_ACL, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
	IF_ERROR(ret_code);

	tpm_proc_src_port_dir_map(api_data.ipv6_l4_key.src_port, &src_dir);

	parse_rule_bm = api_data.ipv6_l4_key.parse_rule_bm;
	parse_flags_bm = api_data.ipv6_l4_key.parse_flags_bm;
	rule_action = api_data.ipv6_l4_key.rule_action;

	l4_key = &api_data.ipv6_l4_key.l4_key;


	flow_bm = parse_rule_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
	if ((parse_flags_bm & TPM_PARSE_FLAG_L4P_MASK) != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_L4_TCP) != 0)
			protocol = IPPROTO_TCP;
		else
			protocol = IPPROTO_UDP;
		flow_bm |= TPM_IPv6_PARSE_NH;
	}
	if (flow_bm != 0) {
		if ((parse_flags_bm & TPM_PARSE_FLAG_PPPOE_TRUE) != 0)
			is_pppoe = TPM_TRUE;
		else
			is_pppoe = TPM_FALSE;
		flow_id = tpm_db_find_matched_ipv6_5t_flow(src_dir, flow_bm, is_pppoe, protocol, l4_key, &gen_key, &dip_key);
		if (flow_id != TPM_DB_INVALID_IPV6_FLOW_ID) {
			ret_code = tpm_db_del_ipv6_5t_flow(flow_id);
			IF_ERROR(ret_code);
		}
		if (rule_action.next_phase != STAGE_DONE) {
			flow_id = tpm_db_find_matched_ipv6_l4_subflow(src_dir, flow_bm, protocol, l4_key);
			if (flow_id == TPM_DB_INVALID_IPV6_FLOW_ID)
				IF_ERROR(ERR_GENERAL);

			flow_user = tpm_db_get_ipv6_l4_subflow_user_num(src_dir, flow_id);

			if (flow_user == 0) {
				ret_code = tpm_db_del_ipv6_l4_subflow(src_dir, flow_id);
				IF_ERROR(ret_code);
			} else {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d) is used by other ipv6 flows! \n", rule_num);
				IF_ERROR(ERR_IPV6_5T_RULE_IN_USE);
			}
		}
	}

	/* Call generic del function without locking, since locking has already been done */
	ret_code = tpm_proc_del_acl_rule(TPM_L4_ACL, owner_id, rule_idx, TPM_FALSE);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_l4_ports_5t_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d), ext_call(%d)", owner_id, rule_idx, ext_call);

	ret_code = tpm_proc_int_del_5t_rule(TPM_L4_ACL, &tpm_proc_int_del_ipv6_l4_ports_5t_rule,
					    ext_call, owner_id, rule_idx);
	return(ret_code);
}


int32_t tpm_proc_ipv6_mc_tcam_build(tpm_mc_filter_mode_t filter_mode,
				    uint32_t stream_num,
				    uint16_t vid,
				    uint8_t mc_stream_pppoe,
				    uint8_t ipv6_dst_add[16],
				    uint8_t sip_index,
				    uint8_t ignore_sip,
				    tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t int_ret_code;
	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id, ai_bits;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t vid_ai_field;
	tpm_parse_flags_t parse_flags_bm = 0;
	long long parse_int_flags_bm = 0;

	tcam_data->l2_parse_bm = 0;
	tcam_data->l3_parse_bm = 0;
	tcam_data->ipv4_parse_bm = 0;
	tcam_data->add_info_mask = 0;
	tcam_data->add_info_data = 0;
	tcam_data->ipv6_parse_bm = TPM_IPv6_PARSE_DIP;

	/* src port */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (vid != 0xFFFF) {
		int_ret_code = tpm_db_mc_vlan_get_ai_bit(vid, &ai_bits);
		IF_ERROR(int_ret_code);

		vid_ai_field.mc_vid_entry = ai_bits;

		parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_TRUE;
		parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_PARSE;
	} else
		parse_int_flags_bm |= TPM_PARSE_FLAG_MC_VID_FALSE;

	if (mc_stream_pppoe)
		parse_flags_bm |= TPM_PARSE_FLAG_PPPOE_TRUE;
	else
		parse_flags_bm |= TPM_PARSE_FLAG_PPPOE_FALSE;

	if (!ignore_sip) {
		parse_int_flags_bm |= TPM_PARSE_FLAG_IPV6_MC_SIP_PARSE;
		vid_ai_field.ipv6_mc_sip_indx = sip_index;
	}

	/* Update the AI bits */
	tpm_proc_parse_flag_ai_tcam_build(&vid_ai_field, parse_flags_bm, parse_int_flags_bm, &(tcam_data->add_info_data),
					  &(tcam_data->add_info_mask));

	/* Copy in logical PnC Key */
	memcpy(tcam_data->pkt_key.ipv6_dip_key.ipv6_ip_add, ipv6_dst_add, 16 * sizeof(uint8_t));
	memset(tcam_data->pkt_key.ipv6_dip_key.ipv6_ip_add_mask, 0xff, 16);

	/* Get PNC Range information */
	int_ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_MC_DS, &lu_id, &start_offset);
	IF_ERROR(int_ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&tcam_data->start_offset, &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_ipv6_mc_sip_entry_create(uint32_t sip_index, uint8_t *ipv6_src_add)
{
	int32_t int_ret_code;
	uint32_t pnc_entry, lu_id;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;
	tpm_gmac_bm_t gmac_bm;
	uint8_t ipv6_src_zero[16];

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&ipv6_src_zero, 0, sizeof(ipv6_src_zero));

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_SIP, &range_data);
	IF_ERROR(int_ret_code);
	lu_id = range_data.pnc_range_conf.base_lu_id;

	pnc_entry = range_data.pnc_range_conf.range_start + sip_index;

	/* Set common TCAM params */
	/* Get PNC Range information */
	int_ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_MC_SIP, &lu_id, &start_offset);
	IF_ERROR(int_ret_code);
	pnc_data.pncl_tcam.lu_id = lu_id;
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	pnc_data.pncl_tcam.port_ids = gmac_bm;
	memcpy(&(pnc_data.pncl_tcam.start_offset), &start_offset, sizeof(tpm_pncl_offset_t));
	pnc_data.pncl_tcam.ipv6_parse_bm = TPM_IPv6_PARSE_SIP;

	/* Set common SRAM params */
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_DIP;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_IPV6_DIP_REG;
	pnc_data.pncl_sram.next_lu_off_reg = TPM_PNC_IPV6_DIP_REG; /* lookup reg 6 */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_DS, &range_data);
	IF_ERROR(int_ret_code);
	pnc_data.pncl_sram.next_lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	/* Double check - check only packets that are MC */
	pnc_data.pncl_tcam.add_info_data = (1 << TPM_AI_MC_VID_VALID_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_MC_VID_VALID_MASK;

	if (0 == memcmp(ipv6_src_zero, ipv6_src_add, sizeof(ipv6_src_zero))) {
		/* this is the default rule */
		pnc_data.pncl_sram.add_info_data &= ~(TPM_AI_IPV6_MC_SIP_PART1_MASK);
		pnc_data.pncl_sram.add_info_data &= ~(TPM_AI_IPV6_MC_SIP_PART2_MASK);
	} else {
		/* update MC_SIP AI to sip_index */
		pnc_data.pncl_sram.add_info_data |= ((sip_index << TPM_AI_IPV6_MC_SIP_PART1_BIT_OFF)& TPM_AI_IPV6_MC_SIP_PART1_MASK);
		pnc_data.pncl_sram.add_info_data |= (((sip_index >> (TPM_AI_IPV6_MC_SIP_PART1_BIT_OFF + 1))
			<< TPM_AI_IPV6_MC_SIP_PART2_BIT_OFF)& TPM_AI_IPV6_MC_SIP_PART2_MASK);
		memcpy(pnc_data.pncl_tcam.pkt_key.ipv6_gen_key.ipv6_src_ip_add, ipv6_src_add, 16);
		memset(pnc_data.pncl_tcam.pkt_key.ipv6_gen_key.ipv6_src_ip_add_mask, 0xff, 16);

		pnc_entry--;
	}

	pnc_data.pncl_sram.add_info_mask |= TPM_AI_IPV6_MC_SIP_PART1_MASK;
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_IPV6_MC_SIP_PART2_MASK;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV6_MC_SIP, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	return TPM_RC_OK;
}
int32_t tpm_proc_ipv6_mc_sip_entry_del(uint32_t sip_index)
{
	int32_t int_ret_code;
	uint32_t pnc_entry;
	tpm_db_pnc_range_t range_data;

	/* Get Range data */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_SIP, &range_data);
	IF_ERROR(int_ret_code);

	pnc_entry = range_data.pnc_range_conf.range_start + sip_index - 1;

	/* del Entry in PnC */
	int_ret_code = tpm_pncl_entry_reset(pnc_entry);
	IF_ERROR(int_ret_code);

	/* Increase number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_IPV6_MC_SIP);
	IF_ERROR(int_ret_code);

	return TPM_RC_OK;
}

tpm_error_code_t tpm_proc_add_ipv6_mc_pnc_entry(tpm_mc_filter_mode_t filter_mode,
						uint32_t stream_num,
						tpm_mc_igmp_mode_t igmp_mode,
						uint8_t mc_stream_pppoe,
						uint16_t vid,
						uint8_t ipv6_src_add[16],
						uint8_t ipv6_dst_add[16],
						uint8_t ignore_ipv6_src,
						uint16_t dest_queue,
						uint32_t dest_port_bm)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_range_t range_data;
	tpm_db_ipv6_mc_stream_entry_t mc_stream;
	tpm_mc_vid_port_cfg_t *mc_vid_cfg = NULL;

	int32_t ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, rule_num = 0xffff;
	uint32_t pnc_range_start = 0, api_start = 0, rule_idx = 0;
	uint32_t entry_id;
	uint16_t u4_vid;
	int32_t  sip_index = 0;
	tpm_gmacs_enum_t gmac;

	memset(&mc_stream, 0, sizeof(tpm_db_mc_stream_entry_t));

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	/*********** Create Modification Entries **********/
	ret_code =
	tpm_proc_create_ipvx_mc_mod(filter_mode, igmp_mode, mc_stream_pppoe, vid, ipv6_dst_add,
				    dest_port_bm, &mod_entry, TPM_IP_VER_6);
	IF_ERROR(ret_code);

	/* Handle IPv6 SSM */
	if (0 == ignore_ipv6_src) {
		/* get index of this IPv6 MC SIP */
		sip_index = tpm_db_ipv6_mc_sip_index_get(ipv6_src_add);
		if (0 == sip_index) {
			/* this is a new SIP, add into DB */
			sip_index = tpm_db_ipv6_mc_sip_add(ipv6_src_add);

			/* create PNC entry in range TPM_PNC_IPV6_MC_SIP */
			ret_code = tpm_proc_ipv6_mc_sip_entry_create(sip_index, ipv6_src_add);

		} else {
			/* this is a exising SIP, add reference number in DB */
			ret_code = tpm_db_ipv6_mc_sip_ref_num_inc(sip_index);
			IF_ERROR(ret_code);
		}
	}

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code =
	tpm_proc_ipv6_mc_tcam_build(filter_mode, stream_num, vid, mc_stream_pppoe,
				    ipv6_dst_add, sip_index, ignore_ipv6_src, &(pnc_data.pncl_tcam));
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code =
	tpm_proc_ipvx_mc_sram_build(filter_mode, igmp_mode,
				    dest_queue, dest_port_bm,
				    mod_entry, &(pnc_data.pncl_sram),
				    TPM_IP_VER_6);
	IF_ERROR(ret_code);

	/*** Calculate PNC Entry ***/

	/* Get PNC Range Start */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_DS, &range_data);
	IF_ERROR(ret_code);
	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Calculate absolute PNC entry number to execute */
	pnc_entry = (pnc_range_start + api_start) + stream_num;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " stream_num(%d), pnc_entry(%d)\n", stream_num, pnc_entry);

	ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
	IF_ERROR(ret_code);

	/* Decrease number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_IPV6_MC_DS);
	IF_ERROR(ret_code);

	/*********** Update API Range in DB **********/

	/* Set API data */
	memcpy(api_data.ipv6_mc_key.ipv6_dest_add, ipv6_dst_add, 16 * sizeof(uint8_t));
	memcpy(api_data.ipv6_mc_key.ipv6_src_add, ipv6_src_add, 16 * sizeof(uint8_t));
	api_data.ipv6_mc_key.dest_port_bm = dest_port_bm;
	api_data.ipv6_mc_key.dest_queue = dest_queue;
	api_data.ipv6_mc_key.vid = vid;
	api_data.ipv6_mc_key.igmp_mode = igmp_mode;
	api_data.ipv6_mc_key.mc_stream_pppoe = mc_stream_pppoe;
	api_data.ipv6_mc_key.stream_num = stream_num;
	api_data.ipv6_mc_key.ignore_ipv6_src = ignore_ipv6_src;

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = TPM_PNC_IPV6_MC_DS;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	/* Set Modification Connection data */
	mod_con.mod_cmd_ind = mod_entry;
	if (mod_entry) {
		ret_code = tpm_db_target_to_gmac(pnc_data.pncl_sram.flow_id_sub.pnc_target, &gmac);
		IF_ERROR(ret_code);
		mod_con.mod_cmd_mac = gmac;
	}

	/* Set new API Entry */
	ret_code = tpm_db_api_entry_set(TPM_IPV6_MC_ACL, stream_num, 0 /*bi_dir */ ,
					&api_data, &mod_con, &pnc_conn, &rule_idx);
	IF_ERROR(ret_code);

	if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0 && (dest_port_bm & TPM_TRG_UNI_VIRT) != 0) {
			ret_code = tpm_db_get_mc_vid_cfg(vid, &mc_vid_cfg);
			IF_ERROR(ret_code);

			for (entry_id = 0; entry_id < TPM_MAX_NUM_UNI_PORTS; entry_id++) {
				if (mc_vid_cfg[entry_id].tpm_src_port == TPM_SRC_PORT_UNI_VIRT)
					break;
			}
			if (entry_id == TPM_MAX_NUM_UNI_PORTS) {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
						 " virt_uni is unable to join the group when "
						 "virt_uni is not the member of multicast vlan \n");
			} else if (mc_vid_cfg[entry_id].mc_uni_port_mode != TPM_MC_UNI_MODE_EXCLUDE) {
				if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_TRANSPARENT)
					u4_vid = vid;
				else if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_STRIP)
					u4_vid = 0xffff;
				else
					u4_vid = mc_vid_cfg[entry_id].uni_port_vid;

				ret_code = tpm_db_mc_alloc_virt_uni_entry(&rule_num);
				IF_ERROR(ret_code);

				ret_code =
				tpm_proc_mc_create_virt_uni_pnc_entry(rule_num, u4_vid, ipv6_dst_add, true);
				IF_ERROR(ret_code);
			}
		}
	}

	/* Set new stream entry */
	mc_stream.igmp_mode = igmp_mode;
	mc_stream.mc_stream_pppoe = mc_stream_pppoe;
	mc_stream.vid = vid;
	mc_stream.dest_queue = dest_queue;
	mc_stream.dest_port_bm = dest_port_bm;
	mc_stream.u4_entry = rule_num;
	mc_stream.ignore_src_addr = ignore_ipv6_src;
	memcpy(mc_stream.group_addr, ipv6_dst_add, 16 * sizeof(uint8_t));
	memcpy(mc_stream.src_addr, ipv6_src_add, 16 * sizeof(uint8_t));

	ret_code = tpm_db_set_ipv6_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_add_ipv6_mc_stream(uint32_t owner_id,
					     uint32_t stream_num,
					     tpm_mc_igmp_mode_t igmp_mode,
					     uint8_t mc_stream_pppoe,
					     uint16_t vid,
					     uint8_t ipv6_src_add[16],
					     uint8_t ipv6_dst_add[16],
					     uint8_t ignore_ipv6_src,
					     uint16_t dest_queue,
					     tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_mac[6];
	tpm_init_virt_uni_t virt_uni;

	memset(&virt_uni, 0, sizeof(tpm_init_virt_uni_t));

	/*
	filter_mode = tpm_db_get_mc_filter_mode();
	right now we only have TPM_MC_COMBINED_IP_MAC_FILTER mode
	*/
	filter_mode = TPM_MC_COMBINED_IP_MAC_FILTER;
	if (filter_mode == TPM_MC_ALL_CPU_FRWD) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " illegal mc_filter_mod (%d)\n", filter_mode);
		IF_ERROR(ERR_GENERAL);
	}

	if (dest_port_bm & TPM_TRG_PORT_UNI_ANY)
		dest_port_bm = tpm_db_trg_port_uni_any_bmp_get(true);

	/* Check parameters */
	ret_code = tpm_proc_add_ipvx_mc_check(owner_id, stream_num, igmp_mode, mc_stream_pppoe, vid,
						  ipv6_src_add, ipv6_dst_add, ignore_ipv6_src,
						  dest_queue, dest_port_bm, filter_mode, TPM_IP_VER_6);
	IF_ERROR(ret_code);

	/* get queue number */
	if (dest_queue == TPM_INVALID_QUEUE) {
		if (dest_port_bm & TPM_TRG_PORT_CPU)
			dest_queue = tpm_db_get_mc_cpu_queue();
		else
			dest_queue = tpm_db_get_mc_hwf_queue();
	}
	/* Create PNC entry */
	ret_code = tpm_proc_add_ipv6_mc_pnc_entry(filter_mode, stream_num, igmp_mode, mc_stream_pppoe, vid,
						  ipv6_src_add, ipv6_dst_add, ignore_ipv6_src,
						  dest_queue, dest_port_bm);
	IF_ERROR(ret_code);

	/* Set switch port_map for multicast MAC, but don't overwrite FF02::1 (MLD General Query) MAC */
	MULTI_IPV6_2_MAC(mc_mac, ipv6_dst_add);
	if (memcmp(mc_mac, tpm_mld_gen_query_mac, 6) != 0) {
		ret_code = tpm_proc_set_ipv4_mc_switch(filter_mode, mc_mac, 0, dest_port_bm);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_delete_ipvx_mc_pnc_entry(tpm_mc_filter_mode_t filter_mode,
						   uint32_t stream_num,
						   uint32_t dest_port_bm,
						   uint32_t u4_entry,
						   tpm_ip_ver_t ip_version)
{
	tpm_db_pnc_range_t range_data;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_ipv6_mc_stream_entry_t mc_stream;

	int32_t ret_code;
	uint32_t bi_dir = 0, rule_idx = 0;
	uint32_t pnc_range_start = 0, api_start = 0, pnc_entry = 0;
	uint32_t sip_index = 0;
	uint8_t	 new_ref_num = 0;
	tpm_pnc_ranges_t range;
	tpm_api_sections_t api_section;

	if (TPM_IP_VER_4 == ip_version) {
		range = TPM_PNC_IPV4_MC_DS;
		api_section = TPM_IPV4_MC;
	} else {
		range = TPM_PNC_IPV6_MC_DS;
		api_section = TPM_IPV6_MC_ACL;
	}

	if (filter_mode != TPM_MC_MAC_ONLY_FILTER) {
		/* Get PNC Range Start */
		ret_code = tpm_db_pnc_rng_get(range, &range_data);
		IF_ERROR(ret_code);

		if (stream_num >= range_data.pnc_range_conf.range_size)
			IF_ERROR(ERR_MC_STREAM_INVALID);

		/* Check parameters */
		ret_code = tpm_db_api_entry_get(api_section, stream_num, &rule_idx, &bi_dir,
						&api_data, &mod_con, &pnc_conn);
		IF_ERROR(ret_code);

		/* Delete PNC Entry */
#if 0
		ret_code = tpm_proc_pnc_con_del(&pnc_conn);
		IF_ERROR(ret_code);
#endif
		pnc_range_start = range_data.pnc_range_conf.range_start;
		api_start = range_data.pnc_range_conf.api_start;

		/* Pull range from this index untill last used entry in Pnc range */
		pnc_entry = (pnc_range_start + api_start) + stream_num;

		/* Delete PNC entry */
		ret_code = tpm_pncl_entry_delete(pnc_entry, pnc_entry);
		IF_ERROR(ret_code);

		/* Increase number of free entries in pnc_range */
		ret_code = tpm_db_pnc_rng_free_ent_inc(range);
		IF_ERROR(ret_code);

		if (mod_con.mod_cmd_ind != 0) {
			ret_code = tpm_proc_delete_mod(TPM_MOD_OWNER_TPM, mod_con.mod_cmd_mac, mod_con.mod_cmd_ind);
			IF_ERROR(ret_code);
		}

		/* Delete API Rule Entry */
		ret_code = tpm_db_api_entry_invalidate(api_section, stream_num);
		IF_ERROR(ret_code);

		
		if (TPM_IP_VER_6 == ip_version) {
			/* remove SIP PNC entry */
			ret_code = tpm_db_get_ipv6_mc_stream_entry(stream_num, &mc_stream);
			IF_ERROR(ret_code);
			if (0 == mc_stream.ignore_src_addr) {
				/* get index of this IPv6 MC SIP */
				sip_index = tpm_db_ipv6_mc_sip_index_get(mc_stream.src_addr);
				if (0 == sip_index) {
					/* SIP is not in DB, error */
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, "get index of IPv6 MC SIP failed!\n");
					return(ERR_IPV6_MC_SRC_IP_INVALID);
				}

				/* dec reference number in DB */
				ret_code = tpm_db_ipv6_mc_sip_ref_num_dec(sip_index, &new_ref_num);
				IF_ERROR(ret_code);

				/* if new ref num is 0, remove this SIP in PNC */
				if (new_ref_num == 0) {
					ret_code = tpm_proc_ipv6_mc_sip_entry_del(sip_index);
					IF_ERROR(ret_code);
				}
			}
		}

	}

	if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0) {
			if (u4_entry != 0xffff) {
				ret_code = tpm_proc_mc_delete_virt_uni_pnc_entry(u4_entry);
				IF_ERROR(ret_code);

				ret_code = tpm_db_mc_free_virt_uni_entry(u4_entry);
				IF_ERROR(ret_code);
			}
		}
	}

	/* Remove stream entry */
	if (TPM_IP_VER_4 == ip_version)
		tpm_db_reset_mc_stream_entry(stream_num);
	else
		tpm_db_reset_ipv6_mc_stream_entry(stream_num);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_update_ipv6_mc_pnc_entry(tpm_mc_filter_mode_t filter_mode,
						   uint32_t stream_num,
						   tpm_mc_igmp_mode_t igmp_mode,
						   uint8_t mc_stream_pppoe,
						   uint16_t vid,
						   uint8_t ipv6_dst_add[16],
						   uint8_t sip_index,
						   uint8_t ignore_sip,
						   uint16_t dest_queue,
						   uint32_t dest_port_bm)
{
	tpm_db_pnc_range_t range_data;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_ipv6_mc_stream_entry_t mc_stream;
	tpm_mc_vid_port_cfg_t *mc_vid_cfg = NULL;

	int32_t ret_code;
	uint32_t bi_dir = 0, rule_idx = 0, pnc_entry = 0, mod_entry = 0, rule_num = 0xffff;
	uint32_t entry_id;
	uint16_t u4_vid;

	memset(&mc_stream, 0, sizeof(tpm_db_ipv6_mc_stream_entry_t));

	/* Get old API Entry */
	ret_code = tpm_db_api_entry_get(TPM_IPV6_MC_ACL, stream_num, &rule_idx, &bi_dir,
					&api_data, &mod_con, &pnc_conn);
	IF_ERROR(ret_code);

	/* Only MC_IP_ONLY_FILTER mode, update the multicast group member ports by mh_mod. */
	if (filter_mode == TPM_MC_IP_ONLY_FILTER) {
		/* Get PNC Range Start */
		ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_DS, &range_data);
		IF_ERROR(ret_code);

		if (stream_num >= range_data.pnc_range_conf.range_size)
			IF_ERROR(ERR_MC_STREAM_INVALID);

		pnc_entry = pnc_conn.pnc_conn_tbl[0].pnc_index;

		/* Create new Modification Entry */
		ret_code = tpm_proc_create_ipvx_mc_mod(filter_mode, igmp_mode, mc_stream_pppoe,
						       vid, ipv6_dst_add, dest_port_bm, &mod_entry, TPM_IP_VER_6);
		IF_ERROR(ret_code);

		/* Rebuild PnC Entry */
		ret_code = tpm_proc_ipv6_mc_tcam_build(filter_mode, stream_num, vid, mc_stream_pppoe,
						       ipv6_dst_add, sip_index, ignore_sip, &(pnc_data.pncl_tcam));
		IF_ERROR(ret_code);

		/* Rebuild SRAM Entry */
		ret_code =
		tpm_proc_ipvx_mc_sram_build(filter_mode, igmp_mode, dest_queue,
					dest_port_bm, mod_entry, &(pnc_data.pncl_sram),
					TPM_IP_VER_6);
		IF_ERROR(ret_code);

		/* Update only Sram of PNC Entry */
		ret_code = tpm_pncl_update_sram(pnc_entry, &pnc_data);
		IF_ERROR(ret_code);

		/* Delete old Modification Entry */
		if (mod_con.mod_cmd_ind != 0) {
			ret_code = tpm_proc_delete_mod(TPM_MOD_OWNER_TPM, mod_con.mod_cmd_mac, mod_con.mod_cmd_ind);
			IF_ERROR(ret_code);
		}

		/* Update new Modification Entry */
		mod_con.mod_cmd_ind = mod_entry;
	}

	if (filter_mode == TPM_MC_COMBINED_IP_MAC_FILTER) {
		if (tpm_db_get_mc_per_uni_vlan_xlate() != 0) {
			ret_code = tpm_db_get_ipv6_mc_stream_entry(stream_num, &mc_stream);
			IF_ERROR(ret_code);

			rule_num = mc_stream.u4_entry;

			if ((dest_port_bm & TPM_TRG_UNI_VIRT) != 0 && (mc_stream.dest_port_bm & TPM_TRG_UNI_VIRT) == 0) {
				ret_code = tpm_db_mc_alloc_virt_uni_entry(&rule_num);
				IF_ERROR(ret_code);

				ret_code = tpm_db_get_mc_vid_cfg(vid, &mc_vid_cfg);
				IF_ERROR(ret_code);

				for (entry_id = 0; entry_id < TPM_MAX_NUM_UNI_PORTS; entry_id++) {
					if (mc_vid_cfg[entry_id].tpm_src_port == TPM_SRC_PORT_UNI_VIRT)
						break;
				}
				if (entry_id == TPM_MAX_NUM_UNI_PORTS) {
					TPM_OS_DEBUG(TPM_TPM_LOG_MOD,
						     " virt_uni is unable to join the group when "
						     "virt_uni is not the member of multicast vlan \n");
				} else {
					if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_TRANSPARENT)
						u4_vid = vid;
					else if (mc_vid_cfg[entry_id].mc_uni_port_mode == TPM_MC_UNI_MODE_STRIP)
						u4_vid = 0xffff;
					else
						u4_vid = mc_vid_cfg[entry_id].uni_port_vid;

					ret_code =
					tpm_proc_mc_create_virt_uni_pnc_entry(rule_num, u4_vid, ipv6_dst_add, true);
					IF_ERROR(ret_code);
				}
			} else if ((dest_port_bm & TPM_TRG_UNI_VIRT) == 0 && (mc_stream.dest_port_bm & TPM_TRG_UNI_VIRT) != 0) {
				if (rule_num != 0xffff) {
					ret_code = tpm_proc_mc_delete_virt_uni_pnc_entry(rule_num);
					IF_ERROR(ret_code);
					ret_code = tpm_db_mc_free_virt_uni_entry(rule_num);
					IF_ERROR(ret_code);
					rule_num = 0xffff;
				}
		}
		}
	}

	/* Update stream entry */
	mc_stream.igmp_mode = igmp_mode;
	mc_stream.mc_stream_pppoe = mc_stream_pppoe;
	mc_stream.vid = vid;
	mc_stream.dest_port_bm = dest_port_bm;
	mc_stream.u4_entry = rule_num;
	memcpy(mc_stream.group_addr, ipv6_dst_add, 16 * sizeof(uint8_t));

	ret_code = tpm_db_set_ipv6_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	/* Update API entry */
	/* Invalidate old API Entry */
	ret_code = tpm_db_api_entry_invalidate(TPM_IPV6_MC_ACL, stream_num);
	IF_ERROR(ret_code);

	/* Set new API Entry */
	api_data.ipv6_mc_key.dest_port_bm = dest_port_bm;
	ret_code = tpm_db_api_entry_set(TPM_IPV6_MC_ACL, stream_num, 0 /*bi_dir */ ,
					&api_data, &mod_con, &pnc_conn, &rule_idx);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_updt_ipv6_mc_stream(uint32_t owner_id, uint32_t stream_num, tpm_trg_port_type_t dest_port_bm)
{
	tpm_error_code_t ret_code;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_mac[6];
	tpm_init_virt_uni_t virt_uni;
	tpm_db_ipv6_mc_stream_entry_t mc_stream;
	uint8_t sip_index = 0;

	memset(&virt_uni, 0, sizeof(tpm_init_virt_uni_t));
	memset(&mc_stream, 0, sizeof(tpm_db_mc_stream_entry_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) stream_num(%d),dest_port_bm(0x%x)\n",
		     owner_id, stream_num, dest_port_bm);

	/*
	filter_mode = tpm_db_get_mc_filter_mode();
	right now we only have TPM_MC_COMBINED_IP_MAC_FILTER mode
	*/
	filter_mode = TPM_MC_COMBINED_IP_MAC_FILTER;
	if (filter_mode == TPM_MC_ALL_CPU_FRWD) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " illegal mc_filter_mod (%d)\n", filter_mode);
		IF_ERROR(ERR_GENERAL);
	}

	tpm_db_virt_info_get(&virt_uni);

	if (dest_port_bm & TPM_TRG_UNI_VIRT) {
		if (virt_uni.enabled == 0)
			IF_ERROR(ERR_MC_DST_PORT_INVALID);
	}

	if (dest_port_bm & TPM_TRG_PORT_CPU) {
		if (dest_port_bm & (~TPM_TRG_PORT_CPU))
			IF_ERROR(ERR_MC_DST_PORT_INVALID);
	}

	if (dest_port_bm & TPM_TRG_PORT_UNI_ANY)
		dest_port_bm = tpm_db_trg_port_uni_any_bmp_get(true);

	ret_code = tpm_proc_check_dst_uni_port(dest_port_bm);
	IF_ERROR(ret_code);

	ret_code = tpm_db_get_ipv6_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	if (dest_port_bm == mc_stream.dest_port_bm) {
		/* nothing changed, return directly */
		TPM_OS_INFO(TPM_TPM_LOG_MOD, "dest_port_bm does not change, return directly\n");
		return (TPM_OK);
	}

	/* get sip_index */
	if (0 == mc_stream.ignore_src_addr) {
		/* get index of this IPv6 MC SIP */
		sip_index = tpm_db_ipv6_mc_sip_index_get(mc_stream.src_addr);
		if (0 == sip_index) {
			/* SIP is not in DB, error */
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "get index of IPv6 MC SIP failed!\n");
			return(ERR_IPV6_MC_SRC_IP_INVALID);
		}
	}

	if (((dest_port_bm & TPM_TRG_PORT_CPU) != 0 && (mc_stream.dest_port_bm & TPM_TRG_PORT_CPU) == 0) ||
	    ((dest_port_bm & TPM_TRG_PORT_CPU) == 0 && (mc_stream.dest_port_bm & TPM_TRG_PORT_CPU) != 0)) {
		ret_code = tpm_proc_delete_ipvx_mc_pnc_entry(filter_mode, stream_num,
							     mc_stream.dest_port_bm,
							     mc_stream.u4_entry,
							     TPM_IP_VER_6);
		IF_ERROR(ret_code);

		ret_code = tpm_proc_add_ipv6_mc_pnc_entry(filter_mode, stream_num, mc_stream.igmp_mode,
							  mc_stream.mc_stream_pppoe, mc_stream.vid,
							  mc_stream.src_addr, mc_stream.group_addr,
							  mc_stream.ignore_src_addr, mc_stream.dest_queue,
							  dest_port_bm);
	} else {
		ret_code = tpm_proc_update_ipv6_mc_pnc_entry(filter_mode, stream_num, mc_stream.igmp_mode,
							     mc_stream.mc_stream_pppoe, mc_stream.vid,
							     mc_stream.group_addr, sip_index, mc_stream.ignore_src_addr,
							     mc_stream.dest_queue, dest_port_bm);
	}
	IF_ERROR(ret_code);

	/* Set switch port_map for multicast MAC, but don't overwrite FF02::1 (MLD General Query) MAC */
	MULTI_IPV6_2_MAC(mc_mac, mc_stream.group_addr);
	if (memcmp(mc_mac, tpm_mld_gen_query_mac, 6) != 0) {
		ret_code = tpm_proc_set_ipv4_mc_switch(filter_mode, mc_mac, mc_stream.dest_port_bm, dest_port_bm);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_del_ipv6_mc_stream(uint32_t owner_id, uint32_t stream_num)
{

	int32_t ret_code;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_mac[6];
	tpm_db_ipv6_mc_stream_entry_t mc_stream;

	memset(&mc_stream, 0, sizeof(tpm_db_ipv6_mc_stream_entry_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) stream_num(%d)\n", owner_id, stream_num);

	/*
	filter_mode = tpm_db_get_mc_filter_mode();
	right now we only have TPM_MC_COMBINED_IP_MAC_FILTER mode
	*/
	filter_mode = TPM_MC_COMBINED_IP_MAC_FILTER;
	if (filter_mode == TPM_MC_ALL_CPU_FRWD) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " illegal mc_filter_mod (%d)\n", filter_mode);
		IF_ERROR(ERR_GENERAL);
	}

	ret_code = tpm_db_get_ipv6_mc_stream_entry(stream_num, &mc_stream);
	IF_ERROR(ret_code);

	ret_code =
	tpm_proc_delete_ipvx_mc_pnc_entry(filter_mode, stream_num,
					  mc_stream.dest_port_bm,
					  mc_stream.u4_entry,
					  TPM_IP_VER_6);
	IF_ERROR(ret_code);

	/* Set switch VID and multicast MAC */
	MULTI_IPV6_2_MAC(mc_mac, mc_stream.group_addr);
	/* Set switch port_map for multicast MAC, but don't overwrite FF02::1 (MLD General Query) MAC */
	if (memcmp(mc_mac, tpm_mld_gen_query_mac, 6) != 0) {
		ret_code = tpm_proc_set_ipv4_mc_switch(filter_mode, mc_mac, mc_stream.dest_port_bm, 0);
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_ipv6_hoplimit_init(uint32_t hoplimit_illegal_action)
{
	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry, hoplimit_lu;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;
	int32_t cpu_rx_queue;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "hoplimit_illegal_action(%d)\n", hoplimit_illegal_action);

	/* Get default CPU Rx queue */
	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_HOPL, &range_data);
	IF_ERROR(int_ret_code);
	hoplimit_lu = range_data.pnc_range_conf.base_lu_id;

	/* Get pnc_range tcam_start_entry, and number of free entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start;

	/* Set common TCAM params */
	pnc_data.pncl_tcam.lu_id = hoplimit_lu;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_VER;
	pnc_data.pncl_tcam.pkt_key.ipv6_add_key.ipv6_hopl = 0;

	/* Set common SRAM params */
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_VER;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_LUD;

	/***********************/
	/* Create HopLimit=0   */
	/***********************/
	/* Parse the HopLimit  */
	pnc_data.pncl_tcam.ipv6_parse_bm = TPM_IPv6_PARSE_HOPL;

	/* Double check - check only packets that are MTM */
	pnc_data.pncl_tcam.add_info_data = (1 << TPM_AI_MTM_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_MTM_MASK;

	/* Packet forwarded to CPU */
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_TXP;
	pnc_data.pncl_sram.pnc_queue = cpu_rx_queue;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

	/* or packet is dropped */
	if (TPM_TTL_ZERO_ACTION_DROP == hoplimit_illegal_action)
		pnc_data.pncl_sram.sram_updt_bm |= TPM_PNCL_SET_DISC;

	/* Signal the packet is going to CPU */
	pnc_data.pncl_sram.add_info_data |= (1 << TPM_AI_TO_CPU_BIT_OFF);
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TO_CPU_MASK;

	NO_FREE_ENTRIES();
	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV6_HOPL, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	/***********************/
	/* Create HopLimit=1   */
	/***********************/
	NO_FREE_ENTRIES();
	/* Build PnC Entry, see HopLimit=0 */
	pnc_data.pncl_tcam.pkt_key.ipv6_gen_key.hop_limit = 1;

	/* Build SRAM Entry, see HopLimit=0,
	   but always send to CPU, don't drop */
	pnc_data.pncl_sram.sram_updt_bm &= (~(TPM_PNCL_SET_DISC));

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV6_HOPL, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);
	free_entries--;
	pnc_entry++;

	return(TPM_RC_OK);
}
tpm_error_code_t tpm_proc_set_active_wan_check(tpm_gmacs_enum_t active_wan)
{
	tpm_gmacs_enum_t active_wan_current;
	tpm_eth_complex_profile_t profile_id;

	if(!tpm_db_switch_active_wan_en_get()) {
		TPM_OS_ERROR(TPM_DB_MOD, "current profile does not support switching active wan\n");
		return ERR_FEAT_UNSUPPORT;
	}

	active_wan_current = tpm_db_active_wan_get();
	profile_id = tpm_db_eth_cmplx_profile_get();

	if (active_wan == active_wan_current) {
		TPM_OS_ERROR(TPM_DB_MOD, "new active wan port should not be the same with current one\n");
		return ERR_FEAT_UNSUPPORT;
	}

	if (    ((profile_id == TPM_PON_G1_WAN_G0_INT_SWITCH) || (profile_id == TPM_PON_G1_WAN_G0_SINGLE_PORT))
	     && ((active_wan == TPM_ENUM_GMAC_1) || (active_wan == TPM_ENUM_PMAC)))
		;/* OK */
	else if  (    ((profile_id == TPM_PON_G0_WAN_G1_INT_SWITCH) || (profile_id == TPM_PON_G0_WAN_G1_SINGLE_PORT))
	           && ((active_wan == TPM_ENUM_GMAC_0) || (active_wan == TPM_ENUM_PMAC)))
		;/* OK */
	else {
		TPM_OS_ERROR(TPM_DB_MOD, "new active wan port is invalid according to current profile id\n");
		return ERR_FEAT_UNSUPPORT;
	}

	return TPM_OK;
}
tpm_error_code_t tpm_proc_set_active_wan(tpm_gmacs_enum_t active_wan)
{
	int32_t db_ret;
	tpm_error_code_t tpm_ret;

	tpm_ret = tpm_proc_set_active_wan_check(active_wan);
	if(TPM_OK != tpm_ret) {
		TPM_OS_ERROR(TPM_DB_MOD, "input active wan is invalid, error code (%d)\n", tpm_ret);
		return ERR_GENERAL;
	}

	db_ret = tpm_db_active_wan_set(active_wan);
	if(TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_DB_MOD, "set current active wan failed(%d)\n", db_ret);
		return ERR_GENERAL;
	}
	/* reset gmac fun */
	tpm_db_mac_func_set();

	return (TPM_OK);
}


tpm_error_code_t tpm_proc_ipv6_mc_sip_init(void)
{
	int32_t int_ret_code;
	uint32_t sip_index;
	uint8_t  ipv6_mc[16];
	tpm_db_pnc_range_t range_data;
	tpm_pncl_pnc_full_t pnc_data;

	/* Set Structs to zero */
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&ipv6_mc, 0, sizeof(ipv6_mc));

	/* Get Range data */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_MC_SIP, &range_data);
	IF_ERROR(int_ret_code);
	pnc_data.pncl_sram.next_lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.pncl_sram.sram_updt_bm = 0;
	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	sip_index = range_data.pnc_range_conf.range_size - 1;


	int_ret_code = tpm_proc_ipv6_mc_sip_entry_create(sip_index, ipv6_mc);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

tpm_error_code_t tpm_proc_ipv6_l4_mc_ds_init(void)
{
	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry, lu_id;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;
	tpm_gmac_bm_t gmac_bm;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_L4_MC_DS, &range_data);
	IF_ERROR(int_ret_code);
	lu_id = range_data.pnc_range_conf.base_lu_id;

	/* Get pnc_range tcam_start_entry, and number of free entries */
	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start;

	/* Set common TCAM params */
	/* Get PNC Range information */
	int_ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_IPV6_L4, &lu_id, &start_offset);
	IF_ERROR(int_ret_code);
	pnc_data.pncl_tcam.lu_id = lu_id;
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	pnc_data.pncl_tcam.port_ids = gmac_bm;
	memcpy(&(pnc_data.pncl_tcam.start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	/* Set common SRAM params */
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_VER;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.next_lu_off_reg = TPM_PNC_LU_REG0; /* lookup reg 0 */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_HOPL, &range_data);
	IF_ERROR(int_ret_code);
	pnc_data.pncl_sram.next_lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	/* Double check - check only packets that are MC */
	pnc_data.pncl_tcam.add_info_data = (1 << TPM_AI_MC_VID_VALID_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_MC_VID_VALID_MASK;

	NO_FREE_ENTRIES();
	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV6_L4_MC_DS, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	return(TPM_RC_OK);
}

/***************************/
/**          CNM Functions            **/
/***************************/

uint8_t tpm_proc_if_ipv4_pre_filter_key_conflict(uint32_t parse_bm_key_a,
						uint32_t parse_bm_key_b,
						tpm_ipv4_acl_key_t *key_a,
						tpm_ipv4_acl_key_t *key_b)
{
	uint32_t com_parse_bm = parse_bm_key_a & parse_bm_key_b;

	if (com_parse_bm != 0)
	{
		if (com_parse_bm & TPM_IPv4_PARSE_SIP) {
			if (memcmp(key_a->ipv4_src_ip_add, key_b->ipv4_src_ip_add, 4 * sizeof(uint8_t)) ||
				memcmp(key_a->ipv4_src_ip_add_mask, key_b->ipv4_src_ip_add_mask, 4 * sizeof(uint8_t)))
				return TPM_TRUE;
		}
		if (com_parse_bm & TPM_IPv4_PARSE_DIP) {
			if (memcmp(key_a->ipv4_dst_ip_add, key_b->ipv4_dst_ip_add, 4 * sizeof(uint8_t)) ||
				memcmp(key_a->ipv4_dst_ip_add_mask, key_b->ipv4_dst_ip_add_mask, 4 * sizeof(uint8_t)))
				return TPM_TRUE;
		}
		if (com_parse_bm & TPM_IPv4_PARSE_DSCP) {
			if ((key_a->ipv4_dscp & key_a->ipv4_dscp_mask) != (key_b->ipv4_dscp & key_b->ipv4_dscp_mask))
				return TPM_TRUE;
		}
		if (com_parse_bm & TPM_IPv4_PARSE_PROTO) {
			if (key_a->ipv4_proto != key_b->ipv4_proto)
				return TPM_TRUE;
		}
		if (com_parse_bm & TPM_PARSE_L4_SRC) {
			if (key_a->l4_src_port != key_b->l4_src_port)
				return TPM_TRUE;
		}
		if (com_parse_bm & TPM_PARSE_L4_DST) {
			if (key_a->l4_dst_port != key_b->l4_dst_port)
				return TPM_TRUE;
		}
	}

	return TPM_FALSE;
}

uint8_t tpm_proc_if_ipv4_pre_range_is_full(tpm_src_port_type_t src_port,
					   tpm_parse_fields_t ipv4_parse_rule_bm,
					   tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	uint32_t key_idx, num_keys, num_rules = 1, i, j;
	tpm_db_pnc_range_oper_t range_oper;
	tpm_parse_fields_t parse_bm[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM-1] = {0, 0};
	tpm_ipv4_acl_key_t key[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM-1];
	uint8_t is_conflict = TPM_FALSE;

	memset(&range_oper, 0, sizeof(tpm_db_pnc_range_oper_t));

	ret_code = tpm_db_find_ipv4_pre_filter_key(src_port, ipv4_parse_rule_bm, ipv4_key, &key_idx);
	if (ret_code != TPM_DB_OK) {
		tpm_db_get_ipv4_pre_filter_key_num(src_port, &num_keys);
		if (num_keys >= 3)
			return TPM_TRUE;
	} else
		return TPM_FALSE;

	tpm_db_pnc_rng_oper_get(TPM_PNC_CNM_IPV4_PRE, &range_oper);
	if (range_oper.free_entries == 0)
		return TPM_TRUE;

	if ((num_keys == 1 && range_oper.free_entries < 2) ||
		(num_keys == 2 && range_oper.free_entries < 4)) {
		for (i = j = 0; i < TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM; i++) {
			ret_code = tpm_db_get_ipv4_pre_filter_key(src_port, i, &parse_bm[j], &key[j]);
			if (ret_code == TPM_DB_OK)
				j++;
		}

		is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(ipv4_parse_rule_bm, parse_bm[0], ipv4_key, &key[0]);
		if (!is_conflict)
			num_rules++;

		if (num_keys == 2) {
			is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(ipv4_parse_rule_bm, parse_bm[1], ipv4_key, &key[1]);
			if (!is_conflict)
				num_rules++;

			is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(parse_bm[0], parse_bm[1], &key[0], &key[1]);
			if (!is_conflict)
				num_rules++;
		}

		if (range_oper.free_entries < num_rules)
			return TPM_TRUE;
	}

	return TPM_FALSE;
}

uint8_t tpm_proc_if_cnm_main_range_is_full(tpm_src_port_type_t src_port, uint32_t precedence)
{
	int32_t ret_code;
	uint32_t rule_num;
	tpm_db_pnc_range_conf_t range_conf;

	memset(&range_conf, 0, sizeof(tpm_db_pnc_range_conf_t));

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
	if (ret_code != TPM_OK)
		return TPM_TRUE;

	tpm_proc_calc_cnm_rule_num(src_port, precedence, &rule_num);

	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_CNM_MAIN_ACL, range_conf.range_type, rule_num);
	if (ret_code != TPM_OK)
		return TPM_TRUE;

	return TPM_FALSE;
}

int32_t tpm_proc_merge_ipv4_pre_filter_key_b_to_a(uint32_t *parse_bm_key_a,
						 uint32_t parse_bm_key_b,
						 tpm_ipv4_acl_key_t *key_a,
						 tpm_ipv4_acl_key_t *key_b)
{
	uint32_t diff_parse_bm_b_to_a = (~(*parse_bm_key_a)) & parse_bm_key_b;

	if (diff_parse_bm_b_to_a != 0)
	{
		if (diff_parse_bm_b_to_a & TPM_IPv4_PARSE_SIP) {
			memcpy(key_a->ipv4_src_ip_add, key_b->ipv4_src_ip_add, 4 * sizeof(uint8_t));
			memcpy(key_a->ipv4_src_ip_add_mask, key_b->ipv4_src_ip_add_mask, 4 * sizeof(uint8_t));
		}
		if (diff_parse_bm_b_to_a & TPM_IPv4_PARSE_DIP) {
			memcpy(key_a->ipv4_dst_ip_add, key_b->ipv4_dst_ip_add, 4 * sizeof(uint8_t));
			memcpy(key_a->ipv4_dst_ip_add_mask, key_b->ipv4_dst_ip_add_mask, 4 * sizeof(uint8_t));
		}
		if (diff_parse_bm_b_to_a & TPM_IPv4_PARSE_DSCP) {
			key_a->ipv4_dscp = key_b->ipv4_dscp;
			key_a->ipv4_dscp_mask = key_b->ipv4_dscp_mask;
		}
		if (diff_parse_bm_b_to_a & TPM_IPv4_PARSE_PROTO)
			key_a->ipv4_proto = key_b->ipv4_proto;
		if (diff_parse_bm_b_to_a & TPM_PARSE_L4_SRC)
			key_a->l4_src_port = key_b->l4_src_port;
		if (diff_parse_bm_b_to_a & TPM_PARSE_L4_DST)
			key_a->l4_dst_port = key_b->l4_dst_port;

		*parse_bm_key_a |= diff_parse_bm_b_to_a;
	}

	return TPM_OK;
}

int32_t tpm_proc_check_cnm_ipv4_pre_filter_triple_key_rule(tpm_src_port_type_t src_port,
							   uint32_t partner_key_bm,
							   tpm_parse_fields_t ipv4_parse_rule_bm,
							   tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	uint32_t num_keys = 0, i, j;
	tpm_parse_fields_t parse_bm[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM-1] = {0, 0};
	tpm_ipv4_acl_key_t key[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM-1];
	uint8_t is_conflict = TPM_FALSE;

	memset(key, 0, sizeof(key));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " check src_port(%d), partner_key_bm(0x%x)\n", src_port, partner_key_bm);

	tpm_db_get_ipv4_pre_filter_key_num(src_port, &num_keys);
	if (num_keys < 2) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " not enough keys(%d) to set triple key rule - SKIP!\n", num_keys);
		return TPM_FAIL;
	}

	for (i = 0, j = 0; i < TPM_CNM_IPV4_PRE_FILTER_KEY_MAX; i++) {
		if ((1 << i) & partner_key_bm) {
			ret_code = tpm_db_get_ipv4_pre_filter_key(src_port, i, &parse_bm[j], &key[j]);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " fail to get ipv4 pre-filter key(%d)!\n", i);
				return TPM_FAIL;
			}
			j++;
		}
	}

	is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(ipv4_parse_rule_bm, parse_bm[0], ipv4_key, &key[0]);
	if (is_conflict) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " new key conflicts with FIRST existing key!\n");
		return TPM_FAIL;
	}

	is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(ipv4_parse_rule_bm, parse_bm[1], ipv4_key, &key[1]);
	if (is_conflict) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " new key conflicts with SECOND existing key!\n");
		return TPM_FAIL;
	}

	is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(parse_bm[0], parse_bm[1], &key[0], &key[1]);
	if (is_conflict) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " FIRST existing key conflicts with SECOND existing key!\n");
		return TPM_FAIL;
	}

	return TPM_OK;
}

int32_t tpm_proc_check_cnm_ipv4_pre_filter_double_key_rule(tpm_src_port_type_t src_port,
							   uint32_t partner_key_bm,
							   tpm_parse_fields_t ipv4_parse_rule_bm,
							   tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	uint32_t num_keys = 0, i;
	tpm_parse_fields_t parse_bm = 0;
	tpm_ipv4_acl_key_t key;
	uint8_t is_conflict = TPM_FALSE;

	memset(&key, 0, sizeof(tpm_ipv4_acl_key_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " check src_port(%d), partner_key_bm(0x%x)\n", src_port, partner_key_bm);

	tpm_db_get_ipv4_pre_filter_key_num(src_port, &num_keys);
	if (num_keys < 1) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " not enough keys(%d) to set triple key rule - SKIP!\n", num_keys);
		return TPM_FAIL;
	}

	for (i = 0; i < TPM_CNM_IPV4_PRE_FILTER_KEY_MAX; i++) {
		if ((1 << i) & partner_key_bm) {
			ret_code = tpm_db_get_ipv4_pre_filter_key(src_port, i, &parse_bm, &key);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " fail to get ipv4 pre-filter key(%d)!\n", i);
				return TPM_FAIL;
			}
		}
	}

	is_conflict = tpm_proc_if_ipv4_pre_filter_key_conflict(ipv4_parse_rule_bm, parse_bm, ipv4_key, &key);
	if (is_conflict) {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " new key conflicts with existing key!\n");
		return TPM_FAIL;
	}

	return TPM_OK;
}

int32_t tpm_proc_check_cnm_ipv4_pre_filter_single_key_rule(tpm_src_port_type_t src_port,
							   uint32_t partner_key_bm,
							   tpm_parse_fields_t ipv4_parse_rule_bm,
							   tpm_ipv4_acl_key_t *ipv4_key)
{
	return TPM_OK;
}

int32_t tpm_proc_cnm_ipv4_pre_tcam_build(tpm_src_port_type_t src_port,
					 tpm_parse_fields_t ipv4_parse_rule_bm,
					 tpm_ipv4_acl_key_t *ipv4_key,
					 tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t ret_code;

	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t src_port_field;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), parse_rule_bm(%d) \n",
		     src_port, ipv4_parse_rule_bm);

	/* IPV4 Parsing, according to bm in param */
	tcam_data->ipv4_parse_bm = ipv4_parse_rule_bm;

	parse_int_flags_bm |= TPM_PARSE_FLAG_CNM_IPV4_TRUE;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* Parse the AI flag bits */
	src_port_field.src_port = src_port;
	tpm_proc_parse_flag_ai_tcam_build(&src_port_field, 0, parse_int_flags_bm,
									&(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (ipv4_key) {
		memcpy(&(tcam_data->pkt_key.ipv4_key), ipv4_key, sizeof(tpm_ipv4_acl_key_t));
	}

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_CNM_IPV4_PRE, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_cnm_ipv4_pre_sram_build(uint32_t key_pattern, tpm_pncl_sram_data_t *sram_data)
{
	tpm_db_pnc_range_conf_t range_conf;
	long long int_pkt_act = 0;
	tpm_ai_vectors_t key_field;
	int32_t ret_code;

	memset(&key_field, 0, sizeof(tpm_ai_vectors_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " key_pattern(0x%x)n", key_pattern);

	/* Set next offset and update register */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	/*** Set next lookup configuration ***/

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
	IF_ERROR(ret_code);
	sram_data->next_lu_id = range_conf.base_lu_id;
	sram_data->next_lu_off_reg = TPM_PNC_CNM_L2_REG;

	sram_data->next_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	sram_data->next_offset.offset_sub.l2_subf = TPM_L2_PARSE_MH;

	/* Set AI bits for IPV4 Pre-filter key pattern  */
	int_pkt_act |= TPM_ACTION_SET_IPV4_PRE_KEY;

	key_field.ipv4_pre_key = key_pattern;

	/* Set AI bits */
	tpm_proc_static_ai_sram_build(&key_field, 0, int_pkt_act, &(sram_data->add_info_data), &(sram_data->add_info_mask));

	return(TPM_OK);
}


int32_t tpm_proc_set_ipv4_pre_filter_pnc_entry(tpm_src_port_type_t src_port,
					       uint32_t rule_num,
					       uint32_t key_pattern,
					       tpm_parse_fields_t ipv4_parse_rule_bm,
					       tpm_ipv4_acl_key_t *ipv4_key)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;

	int32_t ret_code;
	uint32_t pnc_entry = 0, num_valid_rules = 0, pnc_start_entry = 0, pnc_stop_entry = 0;
	uint32_t pnc_range_start = 0, api_start = 0;

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_cnm_ipv4_pre_tcam_build(src_port, ipv4_parse_rule_bm, ipv4_key, &pnc_data.pncl_tcam);
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_cnm_ipv4_pre_sram_build(key_pattern, &pnc_data.pncl_sram);
	IF_ERROR(ret_code);

	/*** Calculate PNC Entry ***/

	/* Get PNC Range Start */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_CNM_IPV4_PRE, &range_data);
	IF_ERROR(ret_code);
	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Calculate absolute PNC entry number to execute */
	pnc_entry = (pnc_range_start + api_start) + rule_num;

	/* Get valid IPV4 PRE rule number */
	tpm_db_get_ipv4_pre_rule_num(&num_valid_rules);
	/* Decrease by 1 for the new rule */
	num_valid_rules--;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " set pnc_entry(%d), num_valid_rules(%d)\n", pnc_entry, num_valid_rules);

	/* Call PNC Entry Insert, if this is not the api_section's new last entry */
	if (rule_num < num_valid_rules) {
		pnc_start_entry = pnc_entry;
		pnc_stop_entry = (pnc_range_start + api_start) + (num_valid_rules - 1);
		ret_code = tpm_pncl_entry_insert(pnc_start_entry, pnc_stop_entry, &pnc_data);
		IF_ERROR(ret_code);
	} else {
		/* Otherwise just set the entry (no insertion) */
		ret_code = tpm_pncl_entry_set(pnc_entry, &pnc_data);
		IF_ERROR(ret_code);
	}

	/* Decrease number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_dec(TPM_PNC_CNM_IPV4_PRE);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_unset_ipv4_pre_filter_pnc_entry(uint32_t rule_num)
{
	tpm_db_pnc_range_t range_data;

	int32_t ret_code;
	uint32_t num_valid_rules = 0, pnc_start_entry = 0, pnc_stop_entry = 0;
	uint32_t pnc_range_start = 0, api_start = 0;

	/* Get PNC Range Start */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_CNM_IPV4_PRE, &range_data);
	IF_ERROR(ret_code);
	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Get valid IPV4 PRE rule number */
	tpm_db_get_ipv4_pre_rule_num(&num_valid_rules);
	/* Increased by 1 for the new rule */
	num_valid_rules++;

	/* Calculate absolute PNC entry number to execute */
	pnc_start_entry = (pnc_range_start + api_start) + rule_num;
	pnc_stop_entry = (pnc_range_start + api_start) + (num_valid_rules - 1);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " delete pnc_start_entry(%d), pnc_stop_entry(%d)\n",
									pnc_start_entry, pnc_stop_entry);

	ret_code = tpm_pncl_entry_delete(pnc_start_entry, pnc_stop_entry);
	IF_ERROR(ret_code);

	/* Decrease number of free entries in pnc_range */
	ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_CNM_IPV4_PRE);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_proc_add_cnm_ipv4_pre_filter_triple_key_rule(tpm_src_port_type_t src_port,
							 uint32_t key_idx,
							 uint32_t key_pattern,
							 tpm_parse_fields_t ipv4_parse_rule_bm,
							 tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	uint32_t partner_key_bm = 0, rule_num = 0, i, j;
	tpm_parse_fields_t parse_bm[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM-1] = {0, 0};
	tpm_ipv4_acl_key_t key[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM-1];
	tpm_parse_fields_t merge_parse_bm = 0;
	tpm_ipv4_acl_key_t merge_key;

	memset(key, 0, sizeof(key));
	memset(&merge_key, 0, sizeof(tpm_ipv4_acl_key_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " add src_port(%d), key_idx(%d), key_pattern(0x%x)\n", src_port, key_idx, key_pattern);

	partner_key_bm = key_pattern & ~(1 << key_idx);

	for (i = 0, j = 0; i < TPM_CNM_IPV4_PRE_FILTER_KEY_MAX; i++) {
		if ((1 << i) & partner_key_bm) {
			ret_code = tpm_db_get_ipv4_pre_filter_key(src_port, i, &parse_bm[j], &key[j]);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to get ipv4 pre-filter key(%d)!\n", i);
				return TPM_FAIL;
			}
			j++;
		}
	}

	tpm_proc_merge_ipv4_pre_filter_key_b_to_a(&merge_parse_bm, ipv4_parse_rule_bm, &merge_key, ipv4_key);
	tpm_proc_merge_ipv4_pre_filter_key_b_to_a(&merge_parse_bm, parse_bm[0], &merge_key, &key[0]);
	tpm_proc_merge_ipv4_pre_filter_key_b_to_a(&merge_parse_bm, parse_bm[1], &merge_key, &key[1]);

	ret_code = tpm_db_add_ipv4_pre_rule(src_port, (key_pattern << (TPM_CNM_IPV4_PRE_FILTER_KEY_MAX << 1)), &rule_num);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to add ipv4 pre-filter rule!\n");
		return TPM_FAIL;
	}
	ret_code = tpm_proc_set_ipv4_pre_filter_pnc_entry(src_port, rule_num, key_pattern, merge_parse_bm, &merge_key);
	if (ret_code != TPM_DB_OK) {
		tpm_db_del_ipv4_pre_rule(src_port, (key_pattern << (TPM_CNM_IPV4_PRE_FILTER_KEY_MAX << 1)), &rule_num);
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to set ipv4 pre-filter pnc_entry(%d)!\n", rule_num);
		return TPM_FAIL;
	}

	return TPM_OK;
}

int32_t tpm_proc_add_cnm_ipv4_pre_filter_double_key_rule(tpm_src_port_type_t src_port,
							 uint32_t key_idx,
							 uint32_t key_pattern,
							 tpm_parse_fields_t ipv4_parse_rule_bm,
							 tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	uint32_t partner_key_bm = 0, rule_num = 0, i;
	tpm_parse_fields_t parse_bm = 0;
	tpm_ipv4_acl_key_t key;
	tpm_parse_fields_t merge_parse_bm = 0;
	tpm_ipv4_acl_key_t merge_key;

	memset(&key, 0, sizeof(tpm_ipv4_acl_key_t));
	memset(&merge_key, 0, sizeof(tpm_ipv4_acl_key_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " add src_port(%d), key_idx(%d), key_pattern(0x%x)\n", src_port, key_idx, key_pattern);

	partner_key_bm = key_pattern & ~(1 << key_idx);

	for (i = 0; i < TPM_CNM_IPV4_PRE_FILTER_KEY_MAX; i++) {
		if ((1 << i) & partner_key_bm) {
			ret_code = tpm_db_get_ipv4_pre_filter_key(src_port, i, &parse_bm, &key);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to get ipv4 pre-filter key(%d)!\n", i);
				return TPM_FAIL;
			}
		}
	}

	tpm_proc_merge_ipv4_pre_filter_key_b_to_a(&merge_parse_bm, ipv4_parse_rule_bm, &merge_key, ipv4_key);
	tpm_proc_merge_ipv4_pre_filter_key_b_to_a(&merge_parse_bm, parse_bm, &merge_key, &key);

	ret_code = tpm_db_add_ipv4_pre_rule(src_port, (key_pattern << TPM_CNM_IPV4_PRE_FILTER_KEY_MAX), &rule_num);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to add ipv4 pre-filter rule!\n");
		return TPM_FAIL;
	}
	ret_code = tpm_proc_set_ipv4_pre_filter_pnc_entry(src_port, rule_num, key_pattern, merge_parse_bm, &merge_key);
	if (ret_code != TPM_DB_OK) {
		tpm_db_del_ipv4_pre_rule(src_port, (key_pattern << TPM_CNM_IPV4_PRE_FILTER_KEY_MAX), &rule_num);
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to set ipv4 pre-filter pnc_entry(%d)!\n", rule_num);
		return TPM_FAIL;
	}

	return TPM_OK;
}

int32_t tpm_proc_add_cnm_ipv4_pre_filter_single_key_rule(tpm_src_port_type_t src_port,
							 uint32_t key_idx,
							 uint32_t key_pattern,
							 tpm_parse_fields_t ipv4_parse_rule_bm,
							 tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	uint32_t rule_num = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " add src_port(%d), key_idx(%d), key_pattern(0x%x)\n", src_port, key_idx, key_pattern);

	ret_code = tpm_db_add_ipv4_pre_rule(src_port, key_pattern, &rule_num);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to add ipv4 pre-filter rule!\n");
		return TPM_FAIL;
	}
	ret_code = tpm_proc_set_ipv4_pre_filter_pnc_entry(src_port, rule_num, key_pattern, ipv4_parse_rule_bm, ipv4_key);
	if (ret_code != TPM_DB_OK) {
		tpm_db_del_ipv4_pre_rule(src_port, key_pattern, &rule_num);
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to set ipv4 pre-filter pnc_entry(%d)!\n", rule_num);
		return TPM_FAIL;
	}

	return TPM_OK;
}

int32_t tpm_proc_del_cnm_ipv4_pre_filter_rule(uint32_t src_port, uint32_t key_pattern, uint32_t key_idx)
{
	int32_t ret_code;
	uint32_t rule_num = 0, num_keys = 0, _key_pattern = 0, i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " del src_port(%d), key_idx(%d), key_pattern(0x%x)\n", src_port, key_idx, key_pattern);

	for (i = 0; i < TPM_CNM_IPV4_PRE_FILTER_KEY_MAX; i++) {
		if ((1 << i) & key_pattern)
			num_keys++;
	}

	if (num_keys == 1)
		_key_pattern = key_pattern;
	else
		_key_pattern = key_pattern << (TPM_CNM_IPV4_PRE_FILTER_KEY_MAX << (num_keys - 2));

	ret_code = tpm_db_del_ipv4_pre_rule(src_port, _key_pattern, &rule_num);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to delete ipv4 pre-filter rule!\n");
		return TPM_FAIL;
	}
	ret_code = tpm_proc_unset_ipv4_pre_filter_pnc_entry(rule_num);
	if (ret_code != TPM_DB_OK) {
		tpm_db_add_ipv4_pre_rule(src_port, _key_pattern, &rule_num);
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to unset ipv4 pre-filter pnc_entry(%d)!\n", rule_num);
		return TPM_FAIL;
	}

	return TPM_OK;
}

int32_t tpm_proc_check_cnm_ipv4_pre_filter_key_rule(uint32_t key_idx,
						    uint32_t rule_type,
						    tpm_src_port_type_t src_port,
						    tpm_parse_fields_t ipv4_parse_rule_bm,
						    tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	tpm_cnm_ipv4_pre_filter_rule_oper_t *rule_oper = &gs_cnm_ipv4_pre_filter_rule_oper[key_idx][rule_type];
	uint32_t partner_key_bm;

	partner_key_bm = rule_oper->key_pattern & ~(1 << key_idx);
	ret_code = rule_oper->check_cnm_ipv4_pre_filter(src_port, partner_key_bm, ipv4_parse_rule_bm, ipv4_key);

	return ret_code;
}

int32_t tpm_proc_add_cnm_ipv4_pre_filter_key_rule(uint32_t key_idx,
						  uint32_t rule_type,
						  tpm_src_port_type_t src_port,
						  tpm_parse_fields_t ipv4_parse_rule_bm,
						  tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	tpm_cnm_ipv4_pre_filter_rule_oper_t *rule_oper = &gs_cnm_ipv4_pre_filter_rule_oper[key_idx][rule_type];

	ret_code = rule_oper->add_cnm_ipv4_pre_filter(src_port, key_idx, rule_oper->key_pattern,
						      ipv4_parse_rule_bm, ipv4_key);

	return ret_code;
}

int32_t tpm_proc_del_cnm_ipv4_pre_filter_key_rule(uint32_t key_idx,
						  uint32_t rule_type,
						  tpm_src_port_type_t src_port,
						  tpm_parse_fields_t ipv4_parse_rule_bm,
						  tpm_ipv4_acl_key_t *ipv4_key)
{
	int32_t ret_code;
	tpm_cnm_ipv4_pre_filter_rule_oper_t *rule_oper = &gs_cnm_ipv4_pre_filter_rule_oper[key_idx][rule_type];

	ret_code = rule_oper->del_cnm_ipv4_pre_filter(src_port, rule_oper->key_pattern, key_idx);

	return ret_code;
}

int32_t tpm_proc_add_cnm_ipv4_pre_filter_key(uint32_t owner_id,
					     tpm_src_port_type_t src_port,
					     tpm_parse_fields_t ipv4_parse_rule_bm,
					     tpm_ipv4_acl_key_t *ipv4_key,
					     uint32_t *key_idx)
{
	int32_t ret_code;
	uint32_t _key_idx, i;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), ipv4_parse_rule_bm(0x%x)\n", src_port, ipv4_parse_rule_bm);

	ret_code = tpm_db_find_ipv4_pre_filter_key(src_port, ipv4_parse_rule_bm, ipv4_key, &_key_idx);
	if (ret_code != TPM_DB_OK)
	{
		ret_code = tpm_db_alloc_ipv4_pre_filter_key(src_port, &_key_idx);
		if (ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " no room for new ipv4 pre-filter key!\n");
			IF_ERROR(ERR_GENERAL);
		}

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " alloc new key(%d)\n", _key_idx);


		for (i = 0; i < TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_KEY; i++) {
			ret_code = tpm_proc_check_cnm_ipv4_pre_filter_key_rule(_key_idx, i, src_port, ipv4_parse_rule_bm, ipv4_key);
			if (ret_code == TPM_DB_OK) {
				ret_code = tpm_proc_add_cnm_ipv4_pre_filter_key_rule(_key_idx, i, src_port, ipv4_parse_rule_bm, ipv4_key);
				if (ret_code != TPM_DB_OK) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to add src_port(%d) key(%d), rule(%d)!\n",
								src_port, _key_idx, i);
					IF_ERROR(ERR_GENERAL);
				}
			}
		}

		ret_code = tpm_db_set_ipv4_pre_filter_key(src_port, _key_idx, ipv4_parse_rule_bm, ipv4_key);
		if (ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to set ipv4 pre-filter key(%d) to db!\n", _key_idx);
			IF_ERROR(ERR_GENERAL);
		}
	} else
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " find existing key(%d)\n", _key_idx);

	tpm_db_inc_ipv4_pre_filter_key_user_num(src_port, _key_idx);
	*key_idx = _key_idx;

	return TPM_OK;
}

int32_t tpm_proc_del_cnm_ipv4_pre_filter_key(uint32_t owner_id, tpm_src_port_type_t src_port, uint32_t key_idx)
{
	int32_t ret_code;
	tpm_parse_fields_t ipv4_parse_rule_bm = 0;
	tpm_ipv4_acl_key_t ipv4_key;
	uint32_t num_users = 0, i;

	memset(&ipv4_key, 0, sizeof(tpm_ipv4_acl_key_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), key_idx(%d)\n", src_port, key_idx);

	ret_code = tpm_db_get_ipv4_pre_filter_key(src_port, key_idx, &ipv4_parse_rule_bm, &ipv4_key);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " not find ipv4 pre-filter key(%d)!\n", key_idx);
		IF_ERROR(ERR_GENERAL);
	}

	tpm_db_dec_ipv4_pre_filter_key_user_num(src_port, key_idx);

	tpm_db_get_ipv4_pre_filter_key_user_num(src_port, key_idx, &num_users);
	if (num_users == 0) {
		for (i = 0; i < TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_KEY; i++) {
			ret_code = tpm_proc_check_cnm_ipv4_pre_filter_key_rule(key_idx, i, src_port, ipv4_parse_rule_bm, &ipv4_key);
			if (ret_code == TPM_DB_OK) {
				ret_code = tpm_proc_del_cnm_ipv4_pre_filter_key_rule(key_idx, i, src_port, ipv4_parse_rule_bm, &ipv4_key);
				if (ret_code != TPM_DB_OK) {
					TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to del src_port(%d) key(%d), rule(%d)!\n",
								src_port, key_idx, i);
					IF_ERROR(ERR_GENERAL);
				}
			}
		}

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " del ipv4 pre-filter key(%d)\n", key_idx);

		ret_code = tpm_db_del_ipv4_pre_filter_key(src_port, key_idx);
		if (ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " fail to del ipv4 pre-filter key(%d)!\n", key_idx);
			IF_ERROR(ERR_GENERAL);
		}
	}

	return TPM_OK;
}


int32_t tpm_proc_cnm_l2_tcam_build(tpm_src_port_type_t src_port,
				   tpm_parse_fields_t l2_parse_rule_bm,
				   tpm_l2_acl_key_t *l2_key,
				   uint32_t ipv4_key_idx,
				   tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t ret_code;

	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t ai_vectors;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), parse_rule_bm(%d), ipv4_key_idx(%d) \n",
		     src_port, l2_parse_rule_bm, ipv4_key_idx);

	/* IPV4 Parsing, according to bm in param */
	tcam_data->l2_parse_bm = l2_parse_rule_bm;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port)) {
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;
		ai_vectors.src_port = src_port;
	}

	/* Parse the AI flag bits */
	if (ipv4_key_idx != TPM_CNM_INVALID_IPV4_PRE_FILTER_KEY_ID) {
		parse_int_flags_bm |= TPM_PARSE_FLAG_IPV4_PRE_KEY_PARSE;
		ai_vectors.ipv4_pre_key = 1 << ipv4_key_idx;
	}
	tpm_proc_parse_flag_ai_tcam_build(&ai_vectors, 0, parse_int_flags_bm,
									&(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (l2_key) {
		memcpy(&(tcam_data->pkt_key.l2_key), l2_key, sizeof(tpm_l2_acl_key_t));
	}

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_CNM_MAIN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id + TPM_CNM_L2_MAIN_LUID_OFFSET;
	start_offset.offset_base = TPM_PNCL_ZERO_OFFSET;
	start_offset.offset_sub.subf = TPM_L2_PARSE_MH;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_cnm_l2_sram_build(uint32_t precedence,
				   tpm_pkt_frwd_t *pkt_frwd,
				   tpm_pkt_action_t pkt_act,
				   uint32_t mod_cmd,
				   tpm_pncl_sram_data_t *sram_data)
{
	tpm_db_pnc_range_conf_t range_conf;
	long long int_pkt_act = 0;
	tpm_ai_vectors_t key_field;
	tpm_db_pon_type_t pon_type;
	int32_t ret_code;

	memset(&key_field, 0, sizeof(tpm_ai_vectors_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " precedence(%d), pkt_act(%d), mod_cmd(%d) \n", precedence, pkt_act, mod_cmd);

	/* Set next offset and update register */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Update split modification */
	if (SET_MOD(pkt_act)) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}

	/*** Set next lookup configuration ***/
	if (precedence == 0) {
		sram_data->next_lu_id = 0;
		sram_data->next_lu_off_reg = 0;
		sram_data->sram_updt_bm |= TPM_PNCL_SET_LUD;
	} else {
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
		IF_ERROR(ret_code);
		sram_data->next_lu_id = range_conf.base_lu_id + TPM_CNM_ETY_MAIN_LUID_OFFSET;
		sram_data->next_lu_off_reg = TPM_PNC_ETY_REG;

		sram_data->next_offset.offset_base = TPM_PNCL_L3_OFFSET;
		sram_data->next_offset.offset_sub.l3_subf = 0xffff;

		/* Set AI bits for CNM precedence  */
		int_pkt_act |= TPM_ACTION_SET_CNM_PREC;

		key_field.cnm_prec = sg_l2_cnm_prec_ind[precedence];

		/* Set AI bits */
		tpm_proc_static_ai_sram_build(&key_field, 0, int_pkt_act, &(sram_data->add_info_data), &(sram_data->add_info_mask));
	}

	if (SET_TARGET_PORT(pkt_act)) {
		/* Add Target Txp to update BM */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_TXP;

		/* Set PNC FlowId Target */
		sram_data->flow_id_sub.pnc_target = tpm_proc_cnm_pnc_trg_get(pkt_frwd->trg_port);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Target(%d)\n", sram_data->flow_id_sub.pnc_target);

		/* If target is GPON, set GEM and MH */
		tpm_db_pon_type_get(&pon_type);
		if (pon_type == TPM_GPON) {
			sram_data->mh_reg.mh_set = TPM_TRUE;
			sram_data->mh_reg.mh_reg = 1;
			sram_data->sram_updt_bm |= TPM_PNCL_SET_GEM;
			sram_data->flow_id_sub.gem_port = pkt_frwd->gem_port;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set GemPort(%d)\n", pkt_frwd->gem_port);
		}
	}

	if (SET_TARGET_QUEUE(pkt_act)) {
		/* Set Queue */
		sram_data->pnc_queue = pkt_frwd->trg_queue;

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Tx Queue (%x) \n", pkt_frwd->trg_queue);
	} else {
		sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "No Tx Queue Update \n");
	}

	return(TPM_OK);
}

int32_t tpm_proc_cnm_ipv6_tcam_build(tpm_src_port_type_t src_port,
				     uint32_t precedence,
				     tpm_parse_fields_t ipv6_parse_rule_bm,
				     tpm_ipv6_acl_key_t *ipv6_key,
				     tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t ret_code;

	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t ai_vectors;
	long long parse_int_flags_bm = 0;
	tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win;


	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), parse_rule_bm(%d) \n",
		     src_port, ipv6_parse_rule_bm);

	/* ipv6 Parsing, according to bm in param */
	tcam_data->ipv6_parse_bm = ipv6_parse_rule_bm;

	parse_int_flags_bm |= TPM_PARSE_FLAG_CNM_PREC_PARSE;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* Parse the AI flag bits */
	ai_vectors.src_port = src_port;
	ai_vectors.cnm_prec = sg_ipv4_cnm_prec_mask[precedence];
	tpm_proc_parse_flag_ai_tcam_build(&ai_vectors, 0, parse_int_flags_bm,
					  &(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (ipv6_key) {
		memcpy(&(tcam_data->pkt_key.ipv6_key), ipv6_key, sizeof(tpm_ipv6_acl_key_t));
	}

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_CNM_MAIN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id + TPM_CNM_IPV6_MAIN_LUID_OFFSET;

	/* start_offset depends on ipv6 CnM parse window */
	tpm_db_ctc_cm_ipv6_parse_win_get(&ctc_cm_ipv6_parse_win);

	if(TPM_CTC_CM_IPv6_FIRST_24B == ctc_cm_ipv6_parse_win) {
		start_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
		start_offset.offset_sub.subf = TPM_IPv6_PARSE_VER;
		memcpy(&(tcam_data->pkt_key.ipv6_gen_key.ipv6_src_ip_add),
			ipv6_key->ipv6_src_ip_add, sizeof(ipv6_key->ipv6_src_ip_add));
		memcpy(&(tcam_data->pkt_key.ipv6_gen_key.ipv6_src_ip_add_mask),
			ipv6_key->ipv6_src_ip_add_mask, sizeof(ipv6_key->ipv6_src_ip_add_mask));
		tcam_data->pkt_key.ipv6_gen_key.ipv6_dscp = ipv6_key->ipv6_dscp;
		tcam_data->pkt_key.ipv6_gen_key.ipv6_dscp_mask = ipv6_key->ipv6_dscp_mask;
		tcam_data->pkt_key.ipv6_gen_key.hop_limit = ipv6_key->ipv6_hoplimit;
	} else {
		start_offset.offset_base = TPM_PNCL_IPV6_DIP_OFFSET;
		start_offset.offset_sub.subf = TPM_IPv6_PARSE_DIP;
		memcpy(&(tcam_data->pkt_key.ipv6_dip_key.ipv6_ip_add),
			ipv6_key->ipv6_dst_ip_add, sizeof(ipv6_key->ipv6_dst_ip_add));
		memcpy(&(tcam_data->pkt_key.ipv6_dip_key.ipv6_ip_add_mask),
			ipv6_key->ipv6_dst_ip_add_mask, sizeof(ipv6_key->ipv6_dst_ip_add_mask));
		tcam_data->pkt_key.l4_ports_key.l4_dst_port = ipv6_key->l4_dst_port;
		tcam_data->pkt_key.l4_ports_key.l4_src_port = ipv6_key->l4_src_port;
	}
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_cnm_ipv6_sram_build(tpm_pkt_frwd_t *pkt_frwd,
				     tpm_pkt_action_t pkt_act,
				     uint32_t mod_cmd,
				     tpm_pncl_sram_data_t *sram_data)
{
	tpm_ai_vectors_t key_field;
	tpm_db_pon_type_t pon_type;

	memset(&key_field, 0, sizeof(tpm_ai_vectors_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pkt_act(%d), mod_cmd(%d) \n", pkt_act, mod_cmd);

	/* Set next offset and update register */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/*** Set next lookup configuration ***/
	sram_data->next_lu_id = 0;
	sram_data->next_lu_off_reg = 0;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_LUD;

	/* Update split modification */
	if (SET_MOD(pkt_act)) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}

	if (SET_TARGET_PORT(pkt_act)) {
		/* Add Target Txp to update BM */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_TXP;

		/* Set PNC FlowId Target */
		sram_data->flow_id_sub.pnc_target = tpm_proc_cnm_pnc_trg_get(pkt_frwd->trg_port);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Target(%d)\n", sram_data->flow_id_sub.pnc_target);

		/* If target is GPON, set GEM and MH */
		tpm_db_pon_type_get(&pon_type);
		if (pon_type == TPM_GPON) {
			sram_data->mh_reg.mh_set = TPM_TRUE;
			sram_data->mh_reg.mh_reg = 1;
			sram_data->sram_updt_bm |= TPM_PNCL_SET_GEM;
			sram_data->flow_id_sub.gem_port = pkt_frwd->gem_port;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set GemPort(%d)\n", pkt_frwd->gem_port);
		}
	}

	if (SET_TARGET_QUEUE(pkt_act)) {
		/* Set Queue */
		sram_data->pnc_queue = pkt_frwd->trg_queue;

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Tx Queue (%x)\n", pkt_frwd->trg_queue);
	} else {
		sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "No Tx Queue Update \n");
	}

	return(TPM_OK);
}
tpm_pnc_trg_t tpm_proc_cnm_pnc_trg_get(tpm_trg_port_type_t trg_port)
{
	uint32_t i;
	tpm_pnc_trg_t pnc_trgt = 0;
	tpm_gmacs_enum_t   active_wan;

	active_wan = tpm_db_active_wan_get();

	/* Set PNC FlowId Target */
	if (TPM_ENUM_PMAC == active_wan) {
		for (i = 0; i < 8; i++) {
			if (trg_port == (uint32_t)(TPM_TRG_TCONT_0 << i))
				break;
		}
		pnc_trgt = TPM_PNC_TRG_PMAC0 + i;
	} else if (TPM_ENUM_GMAC_0 == active_wan)
		pnc_trgt = TPM_PNC_TRG_GMAC0;
	else if (TPM_ENUM_GMAC_1 == active_wan)
		pnc_trgt = TPM_PNC_TRG_GMAC1;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Target(%d)\n", pnc_trgt);
	return pnc_trgt;
}
int32_t tpm_proc_cnm_ipv4_tcam_build(tpm_src_port_type_t src_port,
				     uint32_t precedence,
				     tpm_parse_fields_t ipv4_parse_rule_bm,
				     tpm_ipv4_acl_key_t *ipv4_key,
				     tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t ret_code;

	tpm_gmac_bm_t gmac_bm;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_ai_vectors_t ai_vectors;
	long long parse_int_flags_bm = 0;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " src_port(%d), parse_rule_bm(%d) \n",
		     src_port, ipv4_parse_rule_bm);

	/* IPV4 Parsing, according to bm in param */
	tcam_data->ipv4_parse_bm = ipv4_parse_rule_bm;

	parse_int_flags_bm |= TPM_PARSE_FLAG_CNM_PREC_PARSE | TPM_PARSE_FLAG_CNM_IPV4_TRUE;

	/* Parse the uni_port AI bits */
	if (FROM_SPEC_UNI(src_port))
		parse_int_flags_bm |= TPM_PARSE_FLAG_UNI_PORT_PARSE;

	/* Parse the AI flag bits */
	ai_vectors.src_port = src_port;
	ai_vectors.cnm_prec = sg_ipv4_cnm_prec_mask[precedence];
	tpm_proc_parse_flag_ai_tcam_build(&ai_vectors, 0, parse_int_flags_bm,
									&(tcam_data->add_info_data), &(tcam_data->add_info_mask));

	/* Get GMAC(s) */
	tpm_proc_src_port_gmac_bm_map(src_port, &gmac_bm);
	tcam_data->port_ids = gmac_bm;

	if (ipv4_key) {
		memcpy(&(tcam_data->pkt_key.ipv4_key), ipv4_key, sizeof(tpm_ipv4_acl_key_t));
	}

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_CNM_MAIN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id + TPM_CNM_IPV4_MAIN_LUID_OFFSET;
	start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	start_offset.offset_sub.subf = TPM_IPv4_PARSE_VER;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

int32_t tpm_proc_cnm_ipv4_sram_build(tpm_pkt_frwd_t *pkt_frwd,
				     tpm_pkt_action_t pkt_act,
				     uint32_t mod_cmd,
				     tpm_pncl_sram_data_t *sram_data)
{
	tpm_ai_vectors_t key_field;
	tpm_db_pon_type_t pon_type;

	memset(&key_field, 0, sizeof(tpm_ai_vectors_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pkt_act(%d), mod_cmd(%d) \n", pkt_act, mod_cmd);

	/* Set next offset and update register */
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/*** Set next lookup configuration ***/
	sram_data->next_lu_id = 0;
	sram_data->next_lu_off_reg = 0;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_LUD;

	/* Update split modification */
	if (SET_MOD(pkt_act)) {
		sram_data->sram_updt_bm |= TPM_PNCL_SET_SPLIT_MOD;
		sram_data->flow_id_sub.mod_cmd = mod_cmd;
	}

	if (SET_TARGET_PORT(pkt_act)) {
		/* Add Target Txp to update BM */
		sram_data->sram_updt_bm |= TPM_PNCL_SET_TXP;

		sram_data->flow_id_sub.pnc_target = tpm_proc_cnm_pnc_trg_get(pkt_frwd->trg_port);

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Target(%d)\n", sram_data->flow_id_sub.pnc_target);

		/* If target is GPON, set GEM and MH */
		tpm_db_pon_type_get(&pon_type);
		if (pon_type == TPM_GPON) {
			sram_data->mh_reg.mh_set = TPM_TRUE;
			sram_data->mh_reg.mh_reg = 1;
			sram_data->sram_updt_bm |= TPM_PNCL_SET_GEM;
			sram_data->flow_id_sub.gem_port = pkt_frwd->gem_port;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set GemPort(%d)\n", pkt_frwd->gem_port);
		}
	}

	if (SET_TARGET_QUEUE(pkt_act)) {
		/* Set Queue */
		sram_data->pnc_queue = pkt_frwd->trg_queue;

		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "Set Tx Queue (%x)\n", pkt_frwd->trg_queue);
	} else {
		sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "No Tx Queue Update \n");
	}

	return(TPM_OK);
}

int32_t tpm_proc_calc_cnm_rule_num(tpm_src_port_type_t src_port, uint32_t precedence, uint32_t *rule_num)
{
	uint32_t num_rules = 0, i;

	for (i = TPM_SRC_PORT_UNI_0; i < src_port; i++)
		num_rules += tpm_db_ctc_cm_get_num_rules(i);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " total rule num before src_port(%d): %d\n", src_port, num_rules);

	*rule_num = precedence + num_rules;

	return TPM_OK;
}

int32_t tpm_proc_calc_cnm_precedence(tpm_src_port_type_t src_port, uint32_t rule_num, uint32_t *precedence)
{
	uint32_t i;
	uint32_t num_rules = 0;

	if (NULL == precedence) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " invalid pointer--precedence!\n");
		return ERR_GENERAL;
	}

	for (i = TPM_SRC_PORT_UNI_0; i < src_port; i++)
		num_rules += tpm_db_ctc_cm_get_num_rules(i);

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " total rule num before src_port(%d): %d\n", src_port, num_rules);

	if (rule_num < num_rules) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " rule num(%d) before src_port(%d) greater than current rule index(%d)!\n", num_rules, src_port, rule_num);
		return ERR_GENERAL;
	}

	if ((rule_num - num_rules) >= TPM_MAX_NUM_CTC_PRECEDENCE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " calculated precedence(%d) invalid!\n", rule_num - num_rules);
		return ERR_CTC_CM_PREC_INVALID;
	}

	*precedence = rule_num - num_rules;

	return TPM_OK;
}

int32_t tpm_proc_add_l2_cnm_rule(uint32_t owner_id,
				 tpm_src_port_type_t src_port,
				 uint32_t precedence,
				 tpm_parse_fields_t l2_parse_rule_bm,
				 tpm_l2_acl_key_t *l2_key,
				 tpm_parse_fields_t ipv4_parse_rule_bm,
				 tpm_ipv4_acl_key_t *ipv4_key,
				 uint32_t ipv4_key_idx,
				 tpm_pkt_frwd_t *pkt_frwd,
				 tpm_pkt_action_t pkt_act,
				 uint32_t pbits,
				 uint32_t *rule_idx)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_db_mod_conn_t mod_con;

	int32_t ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, rule_num = 0, _rule_idx = 0, bi_dir, api_rng_entries;

	memset(&range_conf, 0, sizeof(tpm_db_pnc_range_conf_t));
	memset(&mod_con, 0, sizeof(tpm_db_mod_conn_t));

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_CNM_MAIN_ACL, &range_id);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	/* Calculate rule num */
	tpm_proc_calc_cnm_rule_num(src_port, precedence, &rule_num);

	if (TPM_RANGE_TYPE_TABLE == range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_CNM_MAIN_ACL, rule_num, &_rule_idx, &bi_dir, &api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d) already exists\n",
				     owner_id, src_port, rule_num);

			tpm_proc_del_l2_cnm_rule(owner_id, rule_num);
			IF_ERROR(ret_code);
		}
	}

	/* Get split modification update */
	if (SET_MOD(pkt_act)) {
		if (tpm_db_split_mod_get_enable() == TPM_SPLIT_MOD_ENABLED) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod for prec-0 \n");

			/* get mod index from p-bit value */
			ret_code = tpm_db_split_mod_get_index_by_p_bits(pbits, &mod_entry);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, " unsupported p-bit value: %d\n", pbits);
				return ERR_MOD_INVALID;
			}

			/* skip first PMT for p-bit AS-IS */
			mod_entry++;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " updated mod_cmd(%d) \n", mod_entry);
		} else {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " split_mod is in-active \n");
			return ERR_MOD_INVALID;
		}
	}

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_cnm_l2_tcam_build(src_port, l2_parse_rule_bm, l2_key, ipv4_key_idx, &pnc_data.pncl_tcam);
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_cnm_l2_sram_build(precedence, pkt_frwd, pkt_act, mod_entry, &pnc_data.pncl_sram);
	IF_ERROR(ret_code);


	if (range_conf.range_type == TPM_RANGE_TYPE_ACL) {
	    /*** Insert the PNC Entry ***/
		ret_code = tpm_proc_create_acl_pnc_entry(TPM_CNM_MAIN_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(ret_code);
	} else {
		/*** Set the PNC Entry ***/
		ret_code = tpm_proc_create_table_pnc_entry(TPM_CNM_MAIN_ACL, rule_num, 0, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(ret_code);
	}

	/*********** Update API Range in DB **********/
	/* Set PNC API data */
	api_data.cnm_key.src_port = src_port;
	api_data.cnm_key.l2_parse_rule_bm = l2_parse_rule_bm;
	api_data.cnm_key.ipv4_parse_rule_bm = ipv4_parse_rule_bm;
	api_data.cnm_key.pkt_act = pkt_act;
	api_data.cnm_key.pbits = pbits;
	if (l2_key)
		memcpy(&(api_data.cnm_key.l2_key), l2_key, sizeof(tpm_l2_acl_key_t));
	if (ipv4_key)
		memcpy(&(api_data.cnm_key.ipv4_key), ipv4_key, sizeof(tpm_ipv4_acl_key_t));
	if (pkt_frwd != NULL)
		memcpy(&(api_data.cnm_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.cnm_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (range_conf.range_type == TPM_RANGE_TYPE_ACL) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			ret_code = tpm_proc_api_entry_rulenum_inc(TPM_CNM_MAIN_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(ret_code);
		}
	}

	/* Set new API Entry */
	ret_code = tpm_db_api_entry_set(TPM_CNM_MAIN_ACL, rule_num, 0 /*bi_dir */, &api_data, &mod_con, &pnc_conn, &_rule_idx);
	IF_ERROR(ret_code);

	/* Return Output */
	*rule_idx = _rule_idx;

	return TPM_OK;
}

int32_t tpm_proc_add_ipv4_cnm_rule(uint32_t owner_id,
				   tpm_src_port_type_t src_port,
				   uint32_t precedence,
				   tpm_parse_fields_t ipv4_parse_rule_bm,
				   tpm_ipv4_acl_key_t *ipv4_key,
				   tpm_pkt_frwd_t *pkt_frwd,
				   tpm_pkt_action_t pkt_act,
				   uint32_t pbits,
				   uint32_t *rule_idx)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_db_mod_conn_t mod_con;

	int32_t ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, rule_num = 0, _rule_idx = 0, bi_dir, api_rng_entries;

	memset(&range_conf, 0, sizeof(tpm_db_pnc_range_conf_t));
	memset(&mod_con, 0, sizeof(tpm_db_mod_conn_t));

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_CNM_MAIN_ACL, &range_id);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	/* Calculate rule num */
	tpm_proc_calc_cnm_rule_num(src_port, precedence, &rule_num);

	if (TPM_RANGE_TYPE_TABLE == range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_CNM_MAIN_ACL, rule_num, &_rule_idx, &bi_dir, &api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d) already exists\n",
				     owner_id, src_port, rule_num);

			tpm_proc_del_ipv4_cnm_rule(owner_id, rule_num);
			IF_ERROR(ret_code);
		}
	}

	/* Get split modification update */
	if (SET_MOD(pkt_act)) {
		if (tpm_db_split_mod_get_enable() == TPM_SPLIT_MOD_ENABLED) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod for prec-0 \n");

			/* get mod index from p-bit value */
			ret_code = tpm_db_split_mod_get_index_by_p_bits(pbits, &mod_entry);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, " unsupported p-bit value: %d\n", pbits);
				return ERR_MOD_INVALID;
			}

			/* skip first PMT for p-bit AS-IS */
			mod_entry++;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " updated mod_cmd(%d) \n", mod_entry);
		} else {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " split_mod is in-active \n");
			return ERR_MOD_INVALID;
		}
	}

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_cnm_ipv4_tcam_build(src_port, precedence, ipv4_parse_rule_bm, ipv4_key, &pnc_data.pncl_tcam);
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_cnm_ipv4_sram_build(pkt_frwd, pkt_act, mod_entry, &pnc_data.pncl_sram);
	IF_ERROR(ret_code);


	if (range_conf.range_type == TPM_RANGE_TYPE_ACL) {
	    /*** Insert the PNC Entry ***/
		ret_code = tpm_proc_create_acl_pnc_entry(TPM_CNM_MAIN_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(ret_code);
	} else {
		/*** Set the PNC Entry ***/
		ret_code = tpm_proc_create_table_pnc_entry(TPM_CNM_MAIN_ACL, rule_num, 0, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(ret_code);
	}

	/*********** Update API Range in DB **********/
	/* Set PNC API data */
	api_data.cnm_key.src_port = src_port;
	api_data.cnm_key.ipv4_parse_rule_bm = ipv4_parse_rule_bm;
	api_data.cnm_key.pkt_act = pkt_act;
	api_data.cnm_key.pbits = pbits;
	if (ipv4_key)
		memcpy(&(api_data.cnm_key.ipv4_key), ipv4_key, sizeof(tpm_ipv4_acl_key_t));
	if (pkt_frwd != NULL)
		memcpy(&(api_data.cnm_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.cnm_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (range_conf.range_type == TPM_RANGE_TYPE_ACL) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			ret_code = tpm_proc_api_entry_rulenum_inc(TPM_CNM_MAIN_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(ret_code);
		}
	}

	/* Set new API Entry */
	ret_code = tpm_db_api_entry_set(TPM_CNM_MAIN_ACL, rule_num, 0 /*bi_dir */, &api_data, &mod_con, &pnc_conn, &_rule_idx);
	IF_ERROR(ret_code);

	/* Return Output */
	*rule_idx = _rule_idx;

	return TPM_OK;
}

int32_t tpm_proc_add_ipv6_cnm_rule(uint32_t owner_id,
				   tpm_src_port_type_t src_port,
				   uint32_t precedence,
				   tpm_parse_fields_t ipv6_parse_rule_bm,
				   tpm_ipv6_acl_key_t *ipv6_key,
				   tpm_pkt_frwd_t *pkt_frwd,
				   tpm_pkt_action_t pkt_act,
				   uint32_t pbits,
				   uint32_t *rule_idx)
{
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_db_mod_conn_t mod_con;

	int32_t ret_code;
	uint32_t pnc_entry = 0, mod_entry = 0, rule_num = 0, _rule_idx = 0, bi_dir, api_rng_entries;

	memset(&range_conf, 0, sizeof(tpm_db_pnc_range_conf_t));
	memset(&mod_con, 0, sizeof(tpm_db_mod_conn_t));

	/* Set Structs to zero */
	tpm_proc_set_int_structs(&pnc_data, &start_offset, &api_data, &pnc_conn, &range_data);

	/* Get Range_Id */
	tpm_db_api_section_main_pnc_get(TPM_CNM_MAIN_ACL, &range_id);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	/* Calculate rule num */
	tpm_proc_calc_cnm_rule_num(src_port, precedence, &rule_num);

	if (TPM_RANGE_TYPE_TABLE == range_conf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_CNM_MAIN_ACL, rule_num, &_rule_idx, &bi_dir, &api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) src_port(%d), rule_num(%d) already exists\n",
				     owner_id, src_port, rule_num);

			tpm_proc_del_ipv6_cnm_rule(owner_id, rule_num);
			IF_ERROR(ret_code);
		}
	}

	/* Get split modification update */
	if (SET_MOD(pkt_act)) {
		if (tpm_db_split_mod_get_enable() == TPM_SPLIT_MOD_ENABLED) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " split mod for prec-0 \n");

			/* get mod index from p-bit value */
			ret_code = tpm_db_split_mod_get_index_by_p_bits(pbits, &mod_entry);
			if (ret_code != TPM_DB_OK) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, " unsupported p-bit value: %d\n", pbits);
				return ERR_MOD_INVALID;
			}

			/* skip first PMT for p-bit AS-IS */
			mod_entry++;
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " updated mod_cmd(%d) \n", mod_entry);
		} else {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " split_mod is in-active \n");
			return ERR_MOD_INVALID;
		}
	}

	/*********** Create PNC Entries **********/
	/* Build PnC Entry */
	ret_code = tpm_proc_cnm_ipv6_tcam_build(src_port, precedence, ipv6_parse_rule_bm, ipv6_key, &pnc_data.pncl_tcam);
	IF_ERROR(ret_code);

	/* Build SRAM Entry */
	ret_code = tpm_proc_cnm_ipv6_sram_build(pkt_frwd, pkt_act, mod_entry, &pnc_data.pncl_sram);
	IF_ERROR(ret_code);


	if (range_conf.range_type == TPM_RANGE_TYPE_ACL) {
	    /*** Insert the PNC Entry ***/
		ret_code = tpm_proc_create_acl_pnc_entry(TPM_CNM_MAIN_ACL, rule_num, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(ret_code);
	} else {
		/*** Set the PNC Entry ***/
		ret_code = tpm_proc_create_table_pnc_entry(TPM_CNM_MAIN_ACL, rule_num, 0, &pnc_data, &pnc_entry, &api_rng_entries);
		IF_ERROR(ret_code);
	}

	/*********** Update API Range in DB **********/
	/* Set PNC API data */
	api_data.cnm_key.src_port = src_port;
	api_data.cnm_key.ipv6_parse_rule_bm = ipv6_parse_rule_bm;
	api_data.cnm_key.pkt_act = pkt_act;
	api_data.cnm_key.pbits = pbits;
	if (ipv6_key)
		memcpy(&(api_data.cnm_key.ipv6_key), ipv6_key, sizeof(tpm_ipv6_acl_key_t));
	if (pkt_frwd != NULL)
		memcpy(&(api_data.cnm_key.pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));
	else
		memset(&(api_data.cnm_key.pkt_frwd), 0, sizeof(tpm_pkt_frwd_t));

	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (range_conf.range_type == TPM_RANGE_TYPE_ACL) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			ret_code = tpm_proc_api_entry_rulenum_inc(TPM_CNM_MAIN_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(ret_code);
		}
	}

	/* Set new API Entry */
	ret_code = tpm_db_api_entry_set(TPM_CNM_MAIN_ACL, rule_num, 0 /*bi_dir */, &api_data, &mod_con, &pnc_conn, &_rule_idx);
	IF_ERROR(ret_code);

	/* Return Output */
	*rule_idx = _rule_idx;

	return TPM_OK;
}


int32_t tpm_proc_del_cnm_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;
	uint32_t rule_num, dummy_idx, bi_dir, api_rng_entries;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con = { 0, 0};
	tpm_db_pnc_conn_t pnc_con;
	tpm_db_pnc_range_conf_t rang_conf;
	tpm_pnc_ranges_t range_id = 0;

	memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));
	memset(&api_data, 0, sizeof(tpm_rule_entry_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_idx (%d)", rule_idx);

	/* Get Range_Id */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_CNM_MAIN_ACL, &range_id);
	IF_ERROR(ret_code);

	/* Get Range Conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rang_conf);
	IF_ERROR(ret_code);

	/* Get the rule_num */
	ret_code = tpm_db_api_rulenum_get(TPM_CNM_MAIN_ACL, rule_idx, &rule_num);
	if (ret_code == TPM_DB_ERR_REC_NOT_EXIST) {
		TPM_OS_ERROR(TPM_DB_MOD, " The rule non-exist!\n");
		return ERR_RULE_IDX_INVALID;
	}
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "with rule_num(%d)\n", rule_num);
	/*IF_ERROR(ret_code);*/

	ret_code = tpm_db_api_entry_get(TPM_CNM_MAIN_ACL, rule_num, &dummy_idx, &bi_dir, &api_data, &mod_con, &pnc_con);
	IF_ERROR(ret_code);

	ret_code = tpm_db_api_section_num_entries_get(TPM_CNM_MAIN_ACL, &api_rng_entries);
	IF_ERROR(ret_code);

	/* Delete PNC Entry */
	ret_code = tpm_proc_pnc_con_del(&pnc_con);
	IF_ERROR(ret_code);

	/* Delete API Rule Entry */
	ret_code = tpm_db_api_entry_invalidate(TPM_CNM_MAIN_ACL, rule_num);
	IF_ERROR(ret_code);

	if (TPM_RANGE_TYPE_ACL == rang_conf.range_type) {
		/* Decrease rule_numbers and PnC entries of the existing API entries that were "pulled up" */
		ret_code = tpm_proc_api_entry_rulenum_dec(TPM_CNM_MAIN_ACL, (rule_num + 1), (api_rng_entries - 1));
		IF_ERROR(ret_code);
	}

	return(TPM_RC_OK);
}
int32_t tpm_proc_del_ipv6_cnm_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;

	ret_code = tpm_proc_del_cnm_rule(owner_id, rule_idx);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

int32_t tpm_proc_del_l2_cnm_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;

	ret_code = tpm_proc_del_cnm_rule(owner_id, rule_idx);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

int32_t tpm_proc_del_ipv4_cnm_rule(uint32_t owner_id, uint32_t rule_idx)
{
	int32_t ret_code;

	ret_code = tpm_proc_del_cnm_rule(owner_id, rule_idx);
	IF_ERROR(ret_code);

	return(TPM_RC_OK);
}

/*******************************************************************************
* tpm_proc_mac_learn_port_gmac_bm_map()
*
* DESCRIPTION:    The function get the MAC learn port by means of the GMAC Functionality
*                 for Media convert with loopback mode.
*
* INPUTS:
* None
*
* OUTPUTS:
* gmac_bm          - Bitmap of the GMACs relevant to set in TCAM
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_mac_learn_port_gmac_bm_map(tpm_gmac_bm_t *gmac_bm)
{
	int32_t ret_code = TPM_DB_OK;
	uint32_t gmac_id;
	tpm_db_gmac_func_t gfunc;
	tpm_gmac_bm_t l_gmac_bm = 0;

	/* Find the GMAC used to do MAC learn on MC */
	for (gmac_id = TPM_ENUM_GMAC_0; gmac_id < TPM_MAX_NUM_GMACS; gmac_id ++) {
		ret_code = tpm_db_gmac_func_get(gmac_id, &gfunc);
		if (TPM_DB_OK == ret_code) {
			if (TPM_GMAC_FUNC_US_MAC_LEARN_DS_LAN_UNI == gfunc)
				l_gmac_bm |= gmac_pnc_bm[gmac_id];
		} else {
			TPM_OS_ERROR(TPM_DB_MOD, "GMAC%d Func get failed \n", gmac_id);
			return ret_code;
		}
	}

	*gmac_bm = l_gmac_bm;

	return (TPM_OK);
}

/*******************************************************************************
* tpm_proc_mac_learn_tcam_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* rule_num          - API rule number
* src_mac_addr      - layer2 key data
* OUTPUTS:
* l2_tcam_data      - Logical TCAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_mac_learn_tcam_build(uint32_t rule_num,
				      tpm_l2_acl_key_t *src_mac_addr,
				      tpm_pncl_tcam_data_t *tcam_data)
{
	int32_t ret_code;
	uint32_t lu_id;
	tpm_pncl_offset_t start_offset;
	tpm_gmac_bm_t gmac_bm;

	/* L2 Parsing, according to bm in param */
	tcam_data->l2_parse_bm = TPM_L2_PARSE_MAC_SA;

	/* Get GMAC(s) */
	ret_code = tpm_proc_mac_learn_port_gmac_bm_map(&gmac_bm);
	IF_ERROR(ret_code);
	tcam_data->port_ids = gmac_bm;

	/* Copy in logical PnC Key */
	if (src_mac_addr)
		memcpy(&(tcam_data->pkt_key.l2_key), src_mac_addr, sizeof(tpm_l2_acl_key_t));

	/* Get PNC Range information */
	ret_code = tpm_proc_common_pncl_info_get(TPM_PNC_MAC_LEARN, &lu_id, &start_offset);
	IF_ERROR(ret_code);
	tcam_data->lu_id = lu_id;
	memcpy(&(tcam_data->start_offset), &start_offset, sizeof(tpm_pncl_offset_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_mac_learn_sram_build()
*
* DESCRIPTION:     Function builds a logical TCAM entry from the API data
*
* INPUTS:
* queue             - queue to update
*
* OUTPUTS:
* l2_sram_data      - Logical SRAM Structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_proc_mac_learn_sram_build(uint32_t queue,
				      tpm_pncl_sram_data_t *sram_data)
{

	sram_data->next_lu_id = 0;
	sram_data->next_lu_off_reg = 0;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_LUD;
	sram_data->shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	if (queue > 7)
		sram_data->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	else
		sram_data->pnc_queue = queue;
	sram_data->flow_id_sub.pnc_target = TPM_PNC_TRG_GMAC1;
	sram_data->sram_updt_bm |= TPM_PNCL_SET_TXP;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_proc_add_mac_learn_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_proc_add_static_mac_rule params.
*
* INPUTS:
* owner_id         -
* rule_num         - See tpm_proc_add_l2_prim_acl_rule
* queue            - See tpm_proc_add_l2_prim_acl_rule
* src_mac_addr     - See tpm_proc_add_l2_prim_acl_rule
* rule_idx         - See tpm_proc_add_l2_prim_acl_rule
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_mac_learn_check(uint32_t owner_id,
					      uint32_t rule_num,
					      uint32_t queue,
					      tpm_l2_acl_key_t *src_mac_addr,
					      uint32_t *rule_idx)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t rangConf;
	uint32_t l_api_rng_entries;

	/* Get Range_Id */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_PNC_MAC_LEARN_ACL, &range_id);
	IF_ERROR(ret_code);
	/* Get range conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);
	/* Get number of existing api entries */
	int_ret_code = tpm_db_api_section_num_entries_get(TPM_PNC_MAC_LEARN_ACL, &l_api_rng_entries);
	IF_ERROR(int_ret_code);
	/*check if there are free entry*/
	if (l_api_rng_entries >= (rangConf.api_end - rangConf.api_start + 1)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, " api_sec(%d), has no free entries \n", TPM_PNC_MAC_LEARN_ACL);
		return(TPM_FAIL);
	}

	ret_code = tpm_owner_id_check(TPM_API_MAC_LEARN, owner_id);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_OWNER_INVALID);
	/* Check rule_num, and api_section is active */
	ret_code = tpm_proc_add_api_ent_check(TPM_PNC_MAC_LEARN_ACL, rangConf.range_type, rule_num);
	if (ret_code != TPM_OK)
		IF_ERROR(ERR_RULE_NUM_INVALID);
	/* Check src_mac_addr */
	if (NULL != src_mac_addr) {
		if (!(src_mac_addr->mac.mac_sa_mask[0] ||
		      src_mac_addr->mac.mac_sa_mask[1] ||
		      src_mac_addr->mac.mac_sa_mask[2] ||
		      src_mac_addr->mac.mac_sa_mask[3] ||
		      src_mac_addr->mac.mac_sa_mask[4] ||
		      src_mac_addr->mac.mac_sa_mask[5]))
		      IF_ERROR(ERR_L2_KEY_INVALID);
	} else {
		IF_ERROR(ERR_L2_KEY_INVALID);
	}

	if (queue > 7)
		TPM_OS_WARN(TPM_TPM_LOG_MOD, " Invalid Queue(0x%x)\n", queue);

	if (NULL == rule_idx)
		IF_ERROR(ERR_NULL_POINTER);

	return TPM_OK;
}

/*******************************************************************************
* tpm_add_static_mac_rule()
*
* DESCRIPTION:    Main function for adding source MAC address learn API rule.
*
* INPUTS:
* queue             - queue to update on GMAC1
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_mac_learn_rule(uint32_t owner_id,
					     uint32_t rule_num,
					     uint32_t queue,
					     tpm_l2_acl_key_t *src_mac_addr,
					     uint32_t *rule_idx)
{
	tpm_error_code_t ret_code;
	int32_t int_ret_code;
	uint32_t pnc_entry = 0, api_rng_entries = 0;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t rangConf;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_rule_entry_t api_data;
	tpm_db_pnc_conn_t pnc_conn;
	tpm_db_mod_conn_t mod_con;
	tpm_api_lu_conf_t lu_conf;
	uint32_t l_rule_idx, bi_dir;

	/* Get Range_Id */
	ret_code = tpm_db_api_section_main_pnc_get(TPM_PNC_MAC_LEARN_ACL, &range_id);
	IF_ERROR(ret_code);
	/* Get range conf */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &rangConf);
	IF_ERROR(ret_code);

	/* Check parameters */
	ret_code = tpm_proc_add_mac_learn_check(owner_id, rule_num, queue, src_mac_addr, rule_idx);
	IF_ERROR(ret_code);

	if (TPM_RANGE_TYPE_TABLE == rangConf.range_type) {
		/* Try to getting the current entry */
		ret_code = tpm_db_api_entry_get(TPM_PNC_MAC_LEARN_ACL, rule_num, &l_rule_idx, &bi_dir, &api_data, &mod_con, &pnc_conn);
		/* if current entry with this rule num is valid */
		if (TPM_DB_OK == ret_code) {
			TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d), rule_num(%d) already exists\n", owner_id, rule_num);
			ret_code = tpm_proc_del_static_mac_rule(owner_id, src_mac_addr);
			IF_ERROR(ret_code);
		}
	}
	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&api_data, 0, sizeof(tpm_rule_entry_t));
	memset(&pnc_conn, 0, sizeof(tpm_db_pnc_conn_t));
	memset(&mod_con, 0, sizeof(tpm_db_mod_conn_t));

	/*********** Create PNC Entries **********/

	/* Build PnC Entry */
	int_ret_code = tpm_proc_mac_learn_tcam_build(rule_num, src_mac_addr, &(pnc_data.pncl_tcam));
	IF_ERROR(int_ret_code);

	int_ret_code = tpm_proc_mac_learn_sram_build(queue, &(pnc_data.pncl_sram));
	IF_ERROR(int_ret_code);

	if (TPM_RANGE_TYPE_ACL == rangConf.range_type) {
		/*** Insert the PNC Entry ***/
		int_ret_code = tpm_proc_create_acl_pnc_entry(TPM_PNC_MAC_LEARN_ACL,
							     rule_num,
							     &pnc_data,
							     &pnc_entry,
							     &api_rng_entries);
		IF_ERROR(int_ret_code);
	} else {
		/*** Set the PNC Entry ***/
		int_ret_code = tpm_proc_create_table_pnc_entry(TPM_PNC_MAC_LEARN_ACL,
							       rule_num,
							       0,
							       &pnc_data,
							       &pnc_entry,
							       &api_rng_entries);
		IF_ERROR(int_ret_code);
	}
	/*********** Update API Range in DB **********/
	/* Set PNC API data */
	api_data.l2_prim_key.parse_rule_bm = TPM_L2_PARSE_MAC_SA;
	memcpy(&(api_data.l2_prim_key.l2_key), src_mac_addr, sizeof(tpm_l2_acl_key_t));
	/* Invalide mod */
	mod_con.mod_cmd_ind = 0;
	mod_con.mod_cmd_mac = TPM_INVALID_GMAC;
	/* Set Pnc Connection data */
	pnc_conn.num_pnc_ranges = 1;
	pnc_conn.pnc_conn_tbl[0].pnc_range = range_id;
	pnc_conn.pnc_conn_tbl[0].pnc_index = pnc_entry;

	if (rangConf.range_type == TPM_RANGE_TYPE_ACL) {
		/* Increase rule_numbers and PnC entries of the existing API entries that were "moved down" */
		if (rule_num < api_rng_entries) {
			ret_code = tpm_proc_api_entry_rulenum_inc(TPM_PNC_MAC_LEARN_ACL, rule_num, (api_rng_entries - 1));
			IF_ERROR(ret_code);
		}
	}

	/* Set new API Entry */
	ret_code = tpm_db_api_entry_set(TPM_PNC_MAC_LEARN_ACL, rule_num, 0, &api_data, &mod_con, &pnc_conn, &l_rule_idx);
	IF_ERROR(ret_code);

	/* Return Output */
	*rule_idx = l_rule_idx;

	/* Set aging counter group nunmber and msk */
	int_ret_code = tpm_db_pnc_get_lu_conf(range_id, &lu_conf);
	IF_ERROR(int_ret_code);
	tpm_tcam_set_lu_mask(pnc_entry, (int32_t) lu_conf.lu_mask);
	tpm_tcam_set_cntr_group(pnc_entry, (int32_t) lu_conf.cntr_grp);

	return TPM_OK;
}

/*******************************************************************************
* tpm_proc_find_mac_addr_db()
*
* DESCRIPTION:    To find MAC address for mac learn in DB
*
* INPUTS:
* src_mac_addr             - address to search
*
* OUTPUTS:
* addr_exist               - the address exist in DB or not, 0-not, 1-exist
* rule_num                 - the unique rule index to pnc rule, will be used in pnc del
* tbl_num                  - the table serial number, unique
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_find_mac_addr_db(tpm_l2_acl_key_t *src_mac_addr, int32_t *addr_exist, uint32_t *rule_num, uint32_t *tbl_num)
{
	tpm_error_code_t ret_code;
	tpm_db_api_entry_t api_ent_mem_area;
	uint32_t valid_entry_num = 0;
	uint32_t api_rng_size, tbl_start;
	int32_t apisec_last_valid_index;
	int32_t rule_index;
	tpm_rule_entry_t tpm_rule;
	tpm_pnc_ranges_t pnc_range;

	/* get API section info */
	ret_code = tpm_db_api_section_get(TPM_PNC_MAC_LEARN_ACL,
					  &api_rng_size,
					  &valid_entry_num,
					  &pnc_range,
					  &apisec_last_valid_index,
					  &tbl_start);
	IF_ERROR(ret_code);
	if (valid_entry_num == 0) {
		*addr_exist = 0;
		*rule_num = 0;
		*tbl_num = 0;
		return TPM_OK;
	}

	for (rule_index = 0; rule_index < (api_rng_size - 1); rule_index++) {
		/* clear structure */
		memset(&tpm_rule, 0, sizeof(tpm_rule_entry_t));
		memset(&api_ent_mem_area, 0, sizeof(tpm_db_api_entry_t));

		ret_code = tpm_db_api_section_ent_tbl_get(TPM_PNC_MAC_LEARN_ACL, &api_ent_mem_area, rule_index);
		IF_ERROR(ret_code);
		/*get tpm_rule*/
		if (api_ent_mem_area.valid == TPM_DB_VALID) {
			memcpy(&tpm_rule, &(api_ent_mem_area.api_rule_data), sizeof(tpm_rule_entry_t));
			if (!memcmp(&tpm_rule.l2_prim_key.l2_key.mac, &(src_mac_addr->mac), sizeof(tpm_mac_key_t))) {
				*addr_exist = 1;
				*rule_num = api_ent_mem_area.rule_idx;
				*tbl_num = api_ent_mem_area.rule_num;
				return TPM_OK;
			}
		}
	}

	*addr_exist = 0;
	*rule_num = 0;
	*tbl_num = 0;

	return TPM_OK;
}

/*******************************************************************************
* tpm_proc_add_static_mac_rule()
*
* DESCRIPTION:    Main function for adding source MAC address learn API rule.
*
* INPUTS:
* src_mac_addr             - address to learn
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_add_static_mac_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr)
{
	tpm_error_code_t ret_code;
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable = TPM_PNC_MAC_LEARN_DISABLED;
	uint32_t rule_idx;
	int32_t addr_exist = 0;
	uint32_t rule_num = 0;
	uint32_t queue_idx = 1;
	tpm_gmacs_enum_t gmac_num;
	uint32_t tbl_num = 0;

	/* check whether mac_learn_enable is enabled or not */
	ret_code = tpm_db_pnc_mac_learn_enable_get(&pnc_mac_learn_enable);
	IF_ERROR(ret_code);
	if (pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_DISABLED) {
		printk("MAC learn is not enabled\n");
		return TPM_FAIL;
	}

	if (src_mac_addr == NULL) {
		printk("NULL pointer\n");
		return TPM_FAIL;
	}
	/* check whether the MAC exist in system or not */
	ret_code = tpm_proc_find_mac_addr_db(src_mac_addr, &addr_exist, &rule_num, &tbl_num);
	IF_ERROR(ret_code);
	if (addr_exist)
		return TPM_OK;
	/* Get Tx queue */
	if (tpm_db_gmac_lpk_queue_get(&gmac_num, &queue_idx, TPM_GMAC1_QUEUE_DATA_TRAFFIC)) {
		printk("Loopback Tx queue index get failed\n");
		return TPM_FAIL;
	}
	/* rule num is always 0 */
	ret_code = tpm_proc_add_mac_learn_rule(TPM_MOD_OWNER_TPM, 0, queue_idx, src_mac_addr, &rule_idx);
	IF_ERROR(ret_code);

	return TPM_OK;
}


/*******************************************************************************
* tpm_proc_del_static_mac_rule()
*
* DESCRIPTION:    Main function for deleting source MAC address learn API rule.
*
* INPUTS:
* owner_id
* rule_idx
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_del_static_mac_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr)
{
	tpm_error_code_t ret_code;
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable = TPM_PNC_MAC_LEARN_DISABLED;
	int32_t addr_exist = 0;
	uint32_t rule_num = 0;
	uint32_t tbl_num = 0;

	/* check whether mac_learn_enable is enabled or not */
	ret_code = tpm_db_pnc_mac_learn_enable_get(&pnc_mac_learn_enable);
	IF_ERROR(ret_code);
	if (pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_DISABLED) {
		printk("MAC learn is not enabled\n");
		return TPM_FAIL;
	}

	/* check whether the MAC exist in system or not */
	ret_code = tpm_proc_find_mac_addr_db(src_mac_addr, &addr_exist, &rule_num, &tbl_num);
	IF_ERROR(ret_code);
	if (addr_exist == 0) {
		printk("The src mac entry not exist\n");
		return TPM_FAIL;
	} else {
		TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " owner(%d) rule_idx(%d)", owner_id, rule_num);

		ret_code = tpm_proc_del_acl_rule(TPM_PNC_MAC_LEARN_ACL, owner_id, rule_num, TPM_EXT_CALL);
		IF_ERROR(ret_code);
	}

	return TPM_OK;
}

/*******************************************************************************
* tpm_proc_mac_learn_default_rule_act_set()
*
* DESCRIPTION:    Function used to set mac learn default rule action dynamiclly.
*
* INPUTS:
* owner_id   - see L2 rule add
* mac_conf   - default rule conf, 3 options: trap to CPU, drop packet, frwd to GMAC1
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_mac_learn_default_rule_act_set(uint32_t owner_id, tpm_unknown_mac_conf_t mac_conf)
{
	tpm_error_code_t ret_code;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_all_t pnc_entry;
	uint32_t entry_id;
	uint32_t queue_id;
	tpm_gmacs_enum_t lpk_gmac;
	uint32_t switch_init;
	uint32_t mod_idx = 0;
	tpm_pkt_mod_t mod_data;

	/* check switch init */
	ret_code = tpm_db_switch_init_get(&switch_init);
	IF_ERROR(ret_code);
	if (switch_init != 0) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Switch is initialized, no need for PnC MAC learn \n");
		return TPM_FAIL;
	}
	/*check MAC learn defaut rule conf*/
	if (mac_conf < TPM_UNK_MAC_TRAP || mac_conf > TPM_UNK_MAC_CONTINUE) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Invalid MAC learn default rule conf(%d) \n", mac_conf);
		return ERR_UNKNOWN_MAC_CONF_INVALID;
	}

	/* Set Structs to zero */
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	/* Get default rul entry ID */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MAC_LEARN, &range_data);
	IF_ERROR(ret_code);
	entry_id = range_data.pnc_range_conf.range_end;

	/* Get mod_idx */
	if (TPM_DB_OK != tpm_db_mac_learn_mod_idx_get(&mod_idx)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "MAC learn mod index get failed \n");
		return TPM_FAIL;
	}

	/* construct default rule */
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id;
	/* Set port_id */
	pnc_entry.tcam_entry.port_ids = TPM_BM_GMAC_0;
	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = TPM_TRUE;

	pnc_entry.sram_entry.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	pnc_entry.sram_entry.flowid_updt_mask = TPM_TXP_FL_UPDT_MASK;
	switch (mac_conf) {
	case TPM_UNK_MAC_TRAP:
		if (TPM_DB_OK != tpm_db_gmac_lpk_queue_get(&lpk_gmac,
							   &queue_id,
							   TPM_GMAC1_QUEUE_MAC_LEARN_TRAFFIC)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "loopback gmac queue get failed \n");
			return TPM_FAIL;
		}
		/* Alloc PMT for MAC learn if needed */
		if (mod_idx == 0) {
			memset((uint8_t *) &mod_data, 0, sizeof(tpm_pkt_mod_t));
			mod_data.mh_mod = TPM_MOD2_MAC_LEARN_MH;
			if (tpm_mod2_entry_set(owner_id,
					       lpk_gmac,
					       TPM_MH_SET,
					       0,
					       &mod_data,
					       &mod_idx)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "PMT entry for MAC leanrn get failed\n");
				return TPM_FAIL;
			}
			/* store mod_idx in TPM_DB */
			if (TPM_DB_OK != tpm_db_mac_learn_mod_idx_set(mod_idx)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "Mod index set to tpm db failed \n");
				return TPM_FAIL;
			}
		}
		/* Frwd to loopback GMAC1 */
		pnc_entry.sram_entry.flowid_val = (TPM_PNC_TRG_GMAC1 << TPM_TXP_FL_SHIFT);
		pnc_entry.sram_entry.pnc_queue = queue_id;
		/* Set MH */
		pnc_entry.sram_entry.flowid_val |= mod_idx;
		pnc_entry.sram_entry.flowid_updt_mask |= TPM_MOD_FL_UPDT_MASK;
		break;
	case TPM_UNK_MAC_DROP:
		/* Drop the packet */
		pnc_entry.sram_entry.res_info_15_0_data |= (1 << TPM_PNC_RI_DISC_BIT);
		pnc_entry.sram_entry.res_info_15_0_mask |= (1 << TPM_PNC_RI_DISC_BIT);
		/* delete MAC learn PMT entry if there is */
		if (mod_idx) {
			if (tpm_mod2_entry_del(owner_id, TPM_ENUM_GMAC_1, mod_idx)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "PMT entry del failed\n");
				return TPM_FAIL;
			}
			/* clearn mod index in TPM_DB */
			if (TPM_DB_OK != tpm_db_mac_learn_mod_idx_set(0)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "MAC learn mod index clear failed \n");
				return TPM_FAIL;
			}
		}
		break;
	case TPM_UNK_MAC_CONTINUE:
		if (TPM_DB_OK != tpm_db_gmac_lpk_queue_get(&lpk_gmac,
							   &queue_id,
							   TPM_GMAC1_QUEUE_DATA_TRAFFIC)) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "loopback gmac queue get failed \n");
			return TPM_FAIL;
		}
		/* Frwd to loopback GMAC1 */
		pnc_entry.sram_entry.flowid_val = (TPM_PNC_TRG_GMAC1 << TPM_TXP_FL_SHIFT);
		pnc_entry.sram_entry.pnc_queue = queue_id;
		/* delete MAC learn PMT entry if there is */
		if (mod_idx) {
			if (tpm_mod2_entry_del(owner_id, TPM_ENUM_GMAC_1, mod_idx)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "PMT entry del failed\n");
				return TPM_FAIL;
			}
			/* clearn mod index in TPM_DB */
			if (TPM_DB_OK != tpm_db_mac_learn_mod_idx_set(0)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "MAC learn mod index clear failed \n");
				return TPM_FAIL;
			}
		}
		break;
	default:
		break;
	}

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

tpm_error_code_t tpm_acl_rcvr_func_mac_learn(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	ret_code = tpm_proc_add_static_mac_rule(owner_id, &(api_data->api_rule_data.l2_prim_key.l2_key));
	IF_ERROR(ret_code);

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ds_load_balance(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	ret_code = tpm_proc_add_ds_load_balance_acl_rule(owner_id, api_data->rule_num, &rule_index_tmp,
					api_data->api_rule_data.l2_prim_key.parse_rule_bm,
					api_data->api_rule_data.l2_prim_key.parse_flags_bm,
					&(api_data->api_rule_data.l2_prim_key.l2_key),
					api_data->api_rule_data.l2_prim_key.pkt_frwd.trg_port);
	IF_ERROR(ret_code);

	ret_code = tpm_db_api_entry_update_rule_idx(TPM_DS_LOAD_BALANCE_ACL, rule_index_tmp, api_data->rule_idx);
	IF_ERROR(ret_code);

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_cpu_loopback(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	ret_code = tpm_proc_add_cpu_loopback_rule(owner_id, api_data->rule_num, &rule_index_tmp,
					&(api_data->api_rule_data.l2_prim_key.pkt_frwd));
	IF_ERROR(ret_code);

	ret_code = tpm_db_api_entry_update_rule_idx(TPM_CPU_LOOPBACK_ACL, rule_index_tmp, api_data->rule_idx);
	IF_ERROR(ret_code);

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_l2_prim(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	ret_code = tpm_proc_add_l2_prim_acl_rule(owner_id, api_data->api_rule_data.l2_prim_key.src_port,
					api_data->rule_num, &rule_index_tmp,
					api_data->api_rule_data.l2_prim_key.parse_rule_bm,
					api_data->api_rule_data.l2_prim_key.parse_flags_bm,
					&(api_data->api_rule_data.l2_prim_key.l2_key),
					&(api_data->api_rule_data.l2_prim_key.pkt_frwd),
					&(api_data->api_rule_data.l2_prim_key.pkt_mod),
					api_data->api_rule_data.l2_prim_key.pkt_mod_bm,
					&(api_data->api_rule_data.l2_prim_key.rule_action));
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_L2_PRIM_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_l3_type(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	ret_code = tpm_proc_add_l3_type_acl_rule(owner_id, api_data->api_rule_data.l3_type_key.src_port,
					api_data->rule_num, &rule_index_tmp,
					api_data->api_rule_data.l3_type_key.parse_rule_bm,
					api_data->api_rule_data.l3_type_key.parse_flags_bm,
					&(api_data->api_rule_data.l3_type_key.l3_key),
					&(api_data->api_rule_data.l3_type_key.pkt_frwd),
					&(api_data->api_rule_data.l3_type_key.rule_action));
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_L3_TYPE_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv4(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	ret_code = tpm_proc_add_ipv4_acl_rule(owner_id, api_data->api_rule_data.ipv4_key.src_port,
					api_data->rule_num, &rule_index_tmp,
					api_data->api_rule_data.ipv4_key.parse_rule_bm,
					api_data->api_rule_data.ipv4_key.parse_flags_bm,
					&(api_data->api_rule_data.ipv4_key.ipv4_key),
					&(api_data->api_rule_data.ipv4_key.pkt_frwd),
					&(api_data->api_rule_data.ipv4_key.pkt_mod),
					api_data->api_rule_data.ipv4_key.pkt_mod_bm,
					&(api_data->api_rule_data.ipv4_key.rule_action));
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_IPV4_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv4_mc(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);

	ret_code = tpm_proc_add_ipv4_mc_stream(owner_id,
					api_data->api_rule_data.ipv4_mc_key.stream_num,
					api_data->api_rule_data.ipv4_mc_key.igmp_mode,
					api_data->api_rule_data.ipv4_mc_key.mc_stream_pppoe,
					api_data->api_rule_data.ipv4_mc_key.vid,
					api_data->api_rule_data.ipv4_mc_key.ipv4_src_add,
					api_data->api_rule_data.ipv4_mc_key.ipv4_dest_add,
					api_data->api_rule_data.ipv4_mc_key.ignore_ipv4_src,
					api_data->api_rule_data.ipv4_mc_key.dest_queue,
					api_data->api_rule_data.ipv4_mc_key.dest_port_bm);
	IF_ERROR(ret_code);

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv6_gen(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_dir_t src_dir;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);

	/* Check 5_tuple feature is enable/disabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED)
		ret_code = tpm_proc_add_ipv6_gen_acl_rule(owner_id, api_data->api_rule_data.ipv6_gen_key.src_port,
						api_data->rule_num, &rule_index_tmp,
						api_data->api_rule_data.ipv6_gen_key.parse_rule_bm,
						api_data->api_rule_data.ipv6_gen_key.parse_flags_bm,
						&(api_data->api_rule_data.ipv6_gen_key.ipv6_gen_key),
						&(api_data->api_rule_data.ipv6_gen_key.pkt_frwd),
						&(api_data->api_rule_data.ipv6_gen_key.pkt_mod),
						api_data->api_rule_data.ipv6_gen_key.pkt_mod_bm,
						&(api_data->api_rule_data.ipv6_gen_key.rule_action));
	else {
		/* get direction */
		if (FROM_LAN(api_data->api_rule_data.ipv6_gen_key.src_port))
			src_dir = TPM_DIR_US;
		else
			src_dir = TPM_DIR_DS;
		ret_code = tpm_proc_add_ipv6_gen_5t_rule(owner_id, src_dir,
						api_data->rule_num, &rule_index_tmp,
						api_data->api_rule_data.ipv6_gen_key.parse_rule_bm,
						api_data->api_rule_data.ipv6_gen_key.parse_flags_bm,
						&(api_data->api_rule_data.ipv6_gen_key.l4_key),
						&(api_data->api_rule_data.ipv6_gen_key.ipv6_gen_key),
						&(api_data->api_rule_data.ipv6_gen_key.pkt_frwd),
						&(api_data->api_rule_data.ipv6_gen_key.pkt_mod),
						api_data->api_rule_data.ipv6_gen_key.pkt_mod_bm,
						&(api_data->api_rule_data.ipv6_gen_key.rule_action));
	}
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_IPV6_GEN_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}
	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv6_dip(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_dir_t src_dir;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);

	/* Check 5_tuple feature is enable/disabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED)
		ret_code = tpm_proc_add_ipv6_dip_acl_rule(owner_id, api_data->api_rule_data.ipv6_dip_key.src_port,
						api_data->rule_num, &rule_index_tmp,
						api_data->api_rule_data.ipv6_dip_key.parse_rule_bm,
						api_data->api_rule_data.ipv6_dip_key.parse_flags_bm,
						&(api_data->api_rule_data.ipv6_dip_key.ipv6_dipkey),
						&(api_data->api_rule_data.ipv6_dip_key.pkt_frwd),
						&(api_data->api_rule_data.ipv6_dip_key.pkt_mod),
						api_data->api_rule_data.ipv6_dip_key.pkt_mod_bm,
						&(api_data->api_rule_data.ipv6_dip_key.rule_action));
	else {
		/* get direction */
		if (FROM_LAN(api_data->api_rule_data.ipv6_dip_key.src_port))
			src_dir = TPM_DIR_US;
		else
			src_dir = TPM_DIR_DS;
		ret_code = tpm_proc_add_ipv6_dip_5t_rule(owner_id, src_dir,
						api_data->rule_num, &rule_index_tmp,
						api_data->api_rule_data.ipv6_dip_key.parse_rule_bm,
						api_data->api_rule_data.ipv6_dip_key.parse_flags_bm,
						&(api_data->api_rule_data.ipv6_dip_key.l4_key),
						&(api_data->api_rule_data.ipv6_dip_key.ipv6_gen_key),
						&(api_data->api_rule_data.ipv6_dip_key.ipv6_dipkey),
						&(api_data->api_rule_data.ipv6_dip_key.pkt_frwd),
						&(api_data->api_rule_data.ipv6_dip_key.pkt_mod),
						api_data->api_rule_data.ipv6_dip_key.pkt_mod_bm,
						&(api_data->api_rule_data.ipv6_dip_key.rule_action));
	}
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_IPV6_DIP_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}
	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv6_mc(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);

	ret_code = tpm_proc_add_ipv6_mc_stream(owner_id,
					api_data->api_rule_data.ipv6_mc_key.stream_num,
					api_data->api_rule_data.ipv6_mc_key.igmp_mode,
					api_data->api_rule_data.ipv6_mc_key.mc_stream_pppoe,
					api_data->api_rule_data.ipv6_mc_key.vid,
					api_data->api_rule_data.ipv6_mc_key.ipv6_src_add,
					api_data->api_rule_data.ipv6_mc_key.ipv6_dest_add,
					api_data->api_rule_data.ipv6_mc_key.ignore_ipv6_src,
					api_data->api_rule_data.ipv6_mc_key.dest_queue,
					api_data->api_rule_data.ipv6_mc_key.dest_port_bm);
	IF_ERROR(ret_code);

	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv6_nh(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);

	ret_code = tpm_proc_add_ipv6_nh_acl_rule(owner_id,
					api_data->rule_num, &rule_index_tmp,
					api_data->api_rule_data.ipv6_nh_key.parse_flags_bm,
					api_data->api_rule_data.ipv6_nh_key.nh_iter,
					api_data->api_rule_data.ipv6_nh_key.nh,
					&(api_data->api_rule_data.ipv6_nh_key.pkt_frwd),
					&(api_data->api_rule_data.ipv6_nh_key.rule_action));
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_IPV6_NH_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}
	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_ipv6_l4(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	uint32_t rule_index_tmp;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_dir_t src_dir;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);

	/* Check 5_tuple feature is enable/disabled */
	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED)
		ret_code = tpm_proc_add_ipv6_l4_ports_acl_rule(owner_id, api_data->api_rule_data.ipv6_l4_key.src_port,
						api_data->rule_num, &rule_index_tmp,
						api_data->api_rule_data.ipv6_l4_key.parse_rule_bm,
						api_data->api_rule_data.ipv6_l4_key.parse_flags_bm,
						&(api_data->api_rule_data.ipv6_l4_key.l4_key),
						&(api_data->api_rule_data.ipv6_l4_key.pkt_frwd),
						&(api_data->api_rule_data.ipv6_l4_key.pkt_mod),
						api_data->api_rule_data.ipv6_l4_key.pkt_mod_bm,
						&(api_data->api_rule_data.ipv6_l4_key.rule_action));
	else {
		/* get direction */
		if (FROM_LAN(api_data->api_rule_data.ipv6_l4_key.src_port))
			src_dir = TPM_DIR_US;
		else
			src_dir = TPM_DIR_DS;
		ret_code = tpm_proc_add_ipv6_l4_ports_5t_rule(owner_id, src_dir,
						api_data->rule_num, &rule_index_tmp,
						api_data->api_rule_data.ipv6_l4_key.parse_rule_bm,
						api_data->api_rule_data.ipv6_l4_key.parse_flags_bm,
						&(api_data->api_rule_data.ipv6_l4_key.l4_key),
						&(api_data->api_rule_data.ipv6_l4_key.pkt_frwd),
						&(api_data->api_rule_data.ipv6_l4_key.pkt_mod),
						api_data->api_rule_data.ipv6_l4_key.pkt_mod_bm,
						&(api_data->api_rule_data.ipv6_l4_key.rule_action));
	}
	IF_ERROR(ret_code);

	if (rule_idx_updt_en) {
		ret_code = tpm_db_api_entry_update_rule_idx(TPM_L4_ACL, rule_index_tmp, api_data->rule_idx);
		IF_ERROR(ret_code);
	}
	return (TPM_OK);
}
tpm_error_code_t tpm_acl_rcvr_func_cnm(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en)
{
	tpm_error_code_t ret_code;
	static tpm_src_port_type_t src_port = TPM_SRC_PORT_UNI_0;
	static uint32_t precedence_base = 0;

	TPM_OS_INFO(TPM_TPM_LOG_MOD, "rule_index:[%d]\n", api_data->rule_idx);
	if (src_port < api_data->api_rule_data.cnm_key.src_port) {
		src_port = api_data->api_rule_data.cnm_key.src_port;
		precedence_base = api_data->rule_num;
	} else if (src_port > api_data->api_rule_data.cnm_key.src_port) {
		src_port = api_data->api_rule_data.cnm_key.src_port;
		precedence_base = 0;
	}
	ret_code = tpm_ctc_cm_acl_rule_add(owner_id, api_data->api_rule_data.cnm_key.src_port,
					api_data->rule_num - precedence_base,
					api_data->api_rule_data.cnm_key.l2_parse_rule_bm,
					api_data->api_rule_data.cnm_key.ipv4_parse_rule_bm,
					api_data->api_rule_data.cnm_key.ipv6_parse_rule_bm,
					&(api_data->api_rule_data.cnm_key.l2_key),
					&(api_data->api_rule_data.cnm_key.ipv4_key),
					&(api_data->api_rule_data.cnm_key.ipv6_key),
					&(api_data->api_rule_data.cnm_key.pkt_frwd),
					api_data->api_rule_data.cnm_key.pkt_act,
					api_data->api_rule_data.cnm_key.pbits);
	IF_ERROR(ret_code);

	return (TPM_OK);
}

tpm_error_code_t tpm_acl_rcvr_func_get(tpm_api_type_t api_type, tpm_acl_recovery_func *func)
{
	uint32_t api_loop;
	uint32_t api_max;

	api_max = sizeof(tpm_hot_swap_acl_recovery)/sizeof(tpm_hot_swap_acl_recovery_t);

	for (api_loop = 0; api_loop < api_max; api_loop++) {

		if (tpm_hot_swap_acl_recovery[api_loop].api_type == api_type) {
			*func = tpm_hot_swap_acl_recovery[api_loop].func;
			return TPM_OK;
		}
	}
	return (TPM_OK);
}
#ifdef CONFIG_MV_ETH_WAN_SWAP

tpm_error_code_t tpm_proc_check_hot_swap_profile(uint32_t owner_id,
				    			tpm_eth_complex_profile_t profile_id)
{
	tpm_error_code_t ret_code;
	tpm_api_type_t api_type;
	tpm_eth_complex_profile_t profile_current;

	/* Check owner_id */
	for (api_type = TPM_API_MAC_LEARN; api_type < TPM_MAX_API_TYPES; api_type++) {
		ret_code = tpm_owner_id_check(api_type, owner_id);
		IF_ERROR(ret_code);
	}

	/* check profile id */
	profile_current = tpm_db_eth_cmplx_profile_get();
	if (    TPM_G0_WAN_G1_INT_SWITCH == profile_current
	     && TPM_G1_WAN_G0_INT_SWITCH == profile_id)
		/* allowed */
		;
	else if (    TPM_G1_WAN_G0_INT_SWITCH == profile_current
	          && TPM_G0_WAN_G1_INT_SWITCH == profile_id)
		/* allowed */
		;
	else {
		/* not support */
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "So far only swapping  from TPM_PON_G1_WAN_G0_INT_SWITCH "
				"to TPM_PON_G0_WAN_G1_INT_SWITCH or vice versa are supported,"
				"current profile: [%d], new profile: [%d]\n", profile_current, profile_id);
		return ERR_FEAT_UNSUPPORT;
	}
	return (TPM_OK);
}
tpm_error_code_t tpm_proc_hot_swap_update_acl_rules(uint32_t owner_id)
{
	tpm_db_api_entry_t api_data;
	tpm_api_type_t api_type;
	tpm_acl_recovery_func func = NULL;
	tpm_api_sections_t api_section;
	int32_t db_ret = 0;
	int32_t next_rule = 0;
	uint32_t current_rule = 0;
	uint32_t rule_index = 0;
	uint32_t rule_index_max = 0;
	tpm_error_code_t ret_code;
	uint32_t rule_idx_updt_en = 0;
	tpm_pnc_ranges_t range_id;
	tpm_db_pnc_range_conf_t range_conf;

	//tpm_glob_trace = ~0;

	rule_index_max = 1000;

	/* add ACL rules again */
	for (api_type = TPM_API_MAC_LEARN; api_type < TPM_MAX_API_TYPES; api_type++) {

		/* Get acl recovery func */
		ret_code = tpm_acl_rcvr_func_get(api_type, &func);
		IF_ERROR(ret_code);

		/* Get the api_section */
		ret_code = tpm_db_api_section_get_from_api_type(api_type, &api_section);
		IF_ERROR(ret_code);

		current_rule = -1;

		TPM_OS_INFO(TPM_DB_MOD, "api_type(%d)\n\n", api_type);

		ret_code = tpm_db_api_entry_bak_get_next(api_section, current_rule, &next_rule);
		IF_ERROR(ret_code);

		if (-1 == next_rule) {
			continue;
		}

		/* Get Range_Id */
		tpm_db_api_section_main_pnc_get(api_section, &range_id);

		/* Get Range Conf */
		ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
		IF_ERROR(ret_code);

		if (range_conf.range_type == TPM_RANGE_TYPE_ACL)
			rule_idx_updt_en = TPM_TRUE;
		else
			rule_idx_updt_en = TPM_FALSE;

		while (-1 != next_rule) {
			/*get api section table entry*/
			if (tpm_db_api_section_bak_ent_tbl_get(api_section, &api_data, next_rule)) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "get API data failed, api_section:[%d], rule_index:[%d] \n",
						api_section, rule_index);
				return TPM_FAIL;
			}

			if (api_data.valid != TPM_DB_VALID) {
				TPM_OS_ERROR(TPM_TPM_LOG_MOD, "API data is not valid, api_section:[%d], rule_index:[%d] \n",
						api_section, rule_index);
				return TPM_FAIL;
			}
			TPM_OS_INFO(TPM_DB_MOD, "rule_num(%d), rule_idx(%d) \n", api_data.rule_num, api_data.rule_idx);

			ret_code = (*func)(owner_id, &api_data, rule_idx_updt_en);
			IF_ERROR(ret_code);

			if (rule_index_max < api_data.rule_idx)
				rule_index_max = api_data.rule_idx;

			ret_code = tpm_db_api_entry_bak_get_next(api_section, api_data.rule_num, &next_rule);
			IF_ERROR(ret_code);
		}

		IF_ERROR(ret_code);
	}

	/* update the max rule index */
	db_ret = tpm_db_rule_index_set(rule_index_max + 1);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "update rule_index failed, before hot swap, max rule_index:[%d]\n", rule_index_max);
		return TPM_FAIL;
	}
	return (TPM_OK);
}
tpm_error_code_t tpm_proc_hot_swap_misc_cfg(tpm_eth_complex_profile_t profile_id)
{
	tpm_error_code_t ret_code;
	tpm_init_gmac_conn_conf_t	gmac_port_conf[TPM_NUM_GMACS];
	tpm_gmacs_enum_t gmac_i;
	uint32_t gmac0_mh_en = 0;
	uint32_t gmac1_mh_en = 0;
	tpm_db_mh_src_t ds_mh_set_conf;

	/* keep current API data */
	tpm_db_api_data_backup();

	/* do not update switch */
	tpm_db_switch_init_set(false);

	/* restore normal IPG of GMAC and switch */
	ret_code = tpm_init_ipg(TPM_MH_LEN);
	IF_ERROR(ret_code);

	/* set new profile_id */
	tpm_db_eth_cmplx_profile_set(profile_id);

	/* update active wan and GMAC conf */
	for (gmac_i = 0; gmac_i < TPM_MAX_NUM_GMACS; gmac_i++)
		tpm_db_gmac_conn_conf_get(gmac_i, &(gmac_port_conf[gmac_i]));

	if (TPM_G0_WAN_G1_INT_SWITCH == profile_id) {
		tpm_db_active_wan_set(TPM_ENUM_GMAC_0);
		gmac_port_conf[TPM_ENUM_GMAC_0].conn = TPM_GMAC_CON_RGMII2;
		gmac_port_conf[TPM_ENUM_GMAC_0].port_src = TPM_SRC_PORT_WAN;
		gmac_port_conf[TPM_ENUM_GMAC_1].conn = TPM_GMAC_CON_SWITCH_5;
		gmac_port_conf[TPM_ENUM_GMAC_1].port_src = TPM_SRC_PORT_ILLEGAL;
		gmac0_mh_en = TPM_FALSE;
		gmac1_mh_en = TPM_TRUE;
	} else if (TPM_G1_WAN_G0_INT_SWITCH == profile_id){
		tpm_db_active_wan_set(TPM_ENUM_GMAC_1);
		gmac_port_conf[TPM_ENUM_GMAC_0].conn = TPM_GMAC_CON_SWITCH_4;
		gmac_port_conf[TPM_ENUM_GMAC_0].port_src = TPM_SRC_PORT_ILLEGAL;
		gmac_port_conf[TPM_ENUM_GMAC_1].conn = TPM_GMAC_CON_GE_PHY;
		gmac_port_conf[TPM_ENUM_GMAC_1].port_src = TPM_SRC_PORT_WAN;
		gmac0_mh_en = TPM_TRUE;
		gmac1_mh_en = TPM_FALSE;
	}

	/* Set GMAC Port Config */
	ret_code = tpm_db_gmac_conn_conf_set(gmac_port_conf, TPM_NUM_GMACS);
	IF_ERROR(ret_code);

	/* update GMAC func */
	tpm_db_mac_func_set();

	tpm_db_ds_mh_get_conf_set(&ds_mh_set_conf);

	/* update MH conf */
	ret_code = tpm_init_mh(ds_mh_set_conf, gmac0_mh_en, gmac1_mh_en, gmac_port_conf);
	IF_ERROR(ret_code);

	ret_code = tpm_db_gmac_mh_en_conf_set(TPM_ENUM_GMAC_0, gmac0_mh_en);
	IF_ERROR(ret_code);
	ret_code = tpm_db_gmac_mh_en_conf_set(TPM_ENUM_GMAC_1, gmac1_mh_en);
	IF_ERROR(ret_code);

	/* update rate limitation */
	ret_code = tpm_init_get_gmac_queue_rate_limit();
	IF_ERROR(ret_code);

	/* exchange rate limitation between G0/G1 */
	tpm_init_get_gmac_queue_rate_limit();
	tpm_db_wan_lan_rate_limit_exchange();

	/* set new IPG of GMAC and switch */
	ret_code = tpm_init_ipg((-TPM_MH_LEN));
	IF_ERROR(ret_code);

	/* switch pppoe length reg's value */
	tpm_mod2_registers_init(TPM_ENUM_GMAC_0, 0);
	tpm_mod2_registers_init(TPM_ENUM_GMAC_1, 0);

#if 0
	/* Set GMAC mh_enable */
	if (mv_eth_ctrl_flag(TPM_ENUM_GMAC_0, MV_ETH_F_MH, gmac0_mh_en)){
		TPM_OS_ERROR(TPM_INIT_MOD, " set GMAC0 MH en (%d) failed\n", gmac0_mh_en);
		return (TPM_FAIL);
	}
	if (mv_eth_ctrl_flag(TPM_ENUM_GMAC_1, MV_ETH_F_MH, gmac1_mh_en)){
		TPM_OS_ERROR(TPM_INIT_MOD, " set GMAC1 MH en (%d) failed\n", gmac1_mh_en);
		return (TPM_FAIL);
	}
#endif

	return (TPM_OK);
}

extern int mv_eth_wan_swap(int wan_mode);
tpm_error_code_t tpm_proc_hot_swap_lsp(tpm_eth_complex_profile_t profile_id)
{
	int lsp_ret = 0;

	if (TPM_G0_WAN_G1_INT_SWITCH == profile_id)
		lsp_ret = mv_eth_wan_swap(0);
	else if (TPM_G1_WAN_G0_INT_SWITCH == profile_id)
		lsp_ret = mv_eth_wan_swap(1);

	if (lsp_ret) {
		TPM_OS_ERROR(TPM_HWM_MOD, "LSP hot swap failed, return value (%d)\n", lsp_ret);
		return (TPM_FAIL);
	}
	return (TPM_OK);
}

tpm_error_code_t tpm_proc_hot_swap_profile(uint32_t owner_id,
				    tpm_eth_complex_profile_t profile_id)
{
	tpm_error_code_t ret_code = TPM_OK;

	ret_code = tpm_proc_check_hot_swap_profile(owner_id, profile_id);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_hot_swap_lsp(profile_id);
	IF_ERROR(ret_code);

	ret_code = tpm_proc_hot_swap_misc_cfg(profile_id);
	IF_ERROR(ret_code);

	/* perform mib-reset */
	tpm_proc_mib_reset(owner_id, TPM_ENUM_RESET_LEVEL0);

	/* tpm_db recovery */
	tpm_db_api_data_rcvr();

	ret_code = tpm_proc_hot_swap_update_acl_rules(owner_id);
	IF_ERROR(ret_code);

	return (TPM_OK);
}
#endif /* CONFIG_MV_ETH_WAN_SWAP */

/*******************************************************************************
* tpm_proc_mac_learn_entry_num_get()
*
* DESCRIPTION:    Function used to set mac learn default rule action dynamiclly.
*
* INPUTS:
* none
*
* OUTPUTS:
* entry_num  - current MAC learn entry count in MAC_LEARN range, not including default one
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_mac_learn_entry_num_get(uint32_t *entry_num)
{
	tpm_error_code_t ret_code;
	uint32_t valid_entry_num = 0;
	uint32_t api_rng_size, tbl_start;
	int32_t apisec_last_valid_index;
	tpm_pnc_ranges_t pnc_range;
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable = TPM_PNC_MAC_LEARN_DISABLED;

	/* check whether mac_learn_enable is enabled or not */
	ret_code = tpm_db_pnc_mac_learn_enable_get(&pnc_mac_learn_enable);
	IF_ERROR(ret_code);
	if (pnc_mac_learn_enable == TPM_PNC_MAC_LEARN_DISABLED) {
		printk("MAC learn is not enabled\n");
		return TPM_FAIL;
	}

	/* get API section info */
	ret_code = tpm_db_api_section_get(TPM_PNC_MAC_LEARN_ACL,
					  &api_rng_size,
					  &valid_entry_num,
					  &pnc_range,
					  &apisec_last_valid_index,
					  &tbl_start);
	IF_ERROR(ret_code);

	*entry_num = valid_entry_num;

	return TPM_OK;
}

/*******************************************************************************
* tpm_proc_hwf_admin_set()
*
* DESCRIPTION:    Function used to enable/disable hwf to certain port.
*
* INPUTS:
*
* port
* txp
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_proc_hwf_admin_set(tpm_gmacs_enum_t port, uint8_t txp, uint8_t enable)
{
	uint32_t valid, q_num;
	tpm_db_sched_t sched_method;
	tpm_db_txq_owner_t queue_owner;
	uint32_t owner_queue_num;
	uint32_t queue_size;
	uint32_t queue_weight;
	int32_t db_ret;
	tpm_db_tx_mod_t tx_mod;
	uint32_t curr_state = 0;

	//TPM_OS_INFO(TPM_PNCL_MOD, "port = %d txp = %d enable = %d\n", port, txp, enable);

    printk(KERN_DEBUG "TPM T-Cont API, port(%d), txp(%d) enable(%d)\n", port, txp, enable);

	tx_mod = port + txp;

	if (tx_mod >= TPM_MAX_NUM_TX_PORTS) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "input invalid, port = %d txp = %d\n", port, txp);
		return ERR_OMCI_TCONT_INVALID;
	}

	for (q_num = 0; q_num < TPM_MAX_NUM_TX_QUEUE; q_num++) {
		db_ret = tpm_db_gmac_tx_q_conf_get(tx_mod, q_num, &valid, &sched_method,
					&queue_owner, &owner_queue_num, &queue_size,
					&queue_weight);
		if (!valid)
		{
			//TPM_OS_INFO(TPM_PNCL_MOD, "Q-num = %d not valid, continue\n", q_num);
			continue;
		}

		/* HWF Traffic */
		/* =========== */
		if (queue_owner != TPM_Q_OWNER_CPU)
		{
			/* get the hwf en first */
			curr_state = NETA_HWF_TX_PORT_MASK(port + txp) | NETA_HWF_TXQ_MASK(q_num);
			MV_REG_WRITE(NETA_HWF_TX_PTR_REG((queue_owner - 1)), curr_state);
			curr_state = MV_REG_READ(NETA_HWF_TXQ_ENABLE_REG((queue_owner - 1)));

			/* enable/disable hwf */
			if (enable == true)
                printk(KERN_DEBUG "HWF T-Cont active, port(%d), txp(%d), que(%d)\n", port, txp, q_num);

			mvNetaHwfTxqEnable((queue_owner - 1), port, txp, q_num, enable);

			//TPM_OS_INFO(TPM_PNCL_MOD,"GMAC = %d TCONT = %d Q = %d HWF: owner_rx %d\n", port, txp, q_num, (queue_owner - 1));

			if ((!enable) && (curr_state))
			{
				//TPM_OS_INFO(TPM_PNCL_MOD, "Call mv_eth_txq_clean - current state: %d, enable %d\n", curr_state, enable);
                printk(KERN_DEBUG "HWF T-Cont stop/clean, port(%d), txp(%d), que(%d)\n", port, txp, q_num);
				mv_eth_txq_clean(port, txp, q_num);
			}
		}
		/* SWF Traffic */
		/* =========== */
		else
		{
			if (!enable)
			{
                printk(KERN_DEBUG "SWF T-Cont stop/clean, port(%d), txp(%d), que(%d)\n", port, txp, q_num);
				mv_cust_set_tcont_state(txp, false);
				mv_eth_txq_clean(port, txp, q_num);
			}
			else
			{
                printk(KERN_DEBUG "SWF T-Cont active, port(%d), txp(%d), que(%d)\n", port, txp, q_num);
				mv_cust_set_tcont_state(txp, true);
			}
		}
	}

	return TPM_OK;
}

