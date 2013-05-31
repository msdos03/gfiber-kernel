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
* tpm_ctc_cm.c
*
*
*  MODULE : TPM
*
*  DESCRIPTION : This file config CTC CnM rule
*
*  MODIFICATION HISTORY:
*           7Feb12   jinghua - initial version created.
*
* FILE REVISION NUMBER:
*       Revision: 1.12
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_ctc_cm.h"
#include "tpm_pkt_proc_logic.h"

#define IF_ERROR(ret)		\
	if (ret != TPM_OK) {\
		TPM_OS_ERROR(TPM_CTC_CM_MOD, " recvd ret_code(%d)\n", ret);\
		return(ret);\
	}

/*******************************************************************************
* tpm_ctc_cm_rule_add()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
* ipv4_key_idx       - key for IPv4 sub pattern.
* Other inputs/outputs are same as API call
*
* OUTPUTS:
* rule_index         - CTC CnM rule index
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_rule_add(uint32_t owner_id,
				     tpm_src_port_type_t src_port,
				     uint32_t precedence,
				     tpm_parse_fields_t l2_parse_rule_bm,
				     tpm_parse_fields_t ipv4_parse_rule_bm,
				     tpm_parse_fields_t ipv6_parse_rule_bm,
				     tpm_l2_acl_key_t   *l2_key,
				     tpm_ipv4_acl_key_t *ipv4_key,
				     tpm_ipv6_acl_key_t *ipv6_key,
				     tpm_pkt_frwd_t *pkt_frwd,
				     tpm_pkt_action_t pkt_act,
				     uint32_t pbits,
				     uint32_t ipv4_key_idx,
				     uint32_t *rule_index)
{
	tpm_error_code_t ret_code = ERR_GENERAL;

	if (l2_parse_rule_bm)
		ret_code = tpm_proc_add_l2_cnm_rule(owner_id, src_port, precedence, l2_parse_rule_bm, l2_key,
						    ipv4_parse_rule_bm, ipv4_key, ipv4_key_idx, pkt_frwd,
						    pkt_act, pbits, rule_index);
	else if (ipv4_parse_rule_bm)
		ret_code = tpm_proc_add_ipv4_cnm_rule(owner_id, src_port, precedence, ipv4_parse_rule_bm,
						      ipv4_key, pkt_frwd, pkt_act, pbits, rule_index);
	else if (ipv6_parse_rule_bm)
		ret_code = tpm_proc_add_ipv6_cnm_rule(owner_id, src_port, precedence, ipv6_parse_rule_bm,
						      ipv6_key, pkt_frwd, pkt_act, pbits, rule_index);
	return ret_code;
}

/*******************************************************************************
* tpm_ctc_cm_acl_rule_del()
*
* DESCRIPTION:    Main function for deleting CnM API rule.
*
* INPUTS:
*
* owner_id           - APP owner id  should be used for all API calls.
* cm_rule            - CTC CnM rule data pointer
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_rule_del(uint32_t owner_id, tpm_db_ctc_cm_rule_entry_t  *cm_rule)
{
	tpm_error_code_t ret_code = ERR_GENERAL;

	if ((cm_rule)->l2_parse_rule_bm)
		ret_code = tpm_proc_del_l2_cnm_rule(owner_id, cm_rule->cm_main_rule_index);
	else if ((cm_rule)->ipv4_parse_rule_bm)
		ret_code = tpm_proc_del_ipv4_cnm_rule(owner_id, cm_rule->cm_main_rule_index);
	else if ((cm_rule)->ipv6_parse_rule_bm)
		ret_code = tpm_proc_del_ipv4_cnm_rule(owner_id, cm_rule->cm_main_rule_index);

	return ret_code;
}
/*******************************************************************************
* tpm_ctc_cm_add_rule_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_ctc_cm_acl_rule_add params.
*
* INPUTS:
*                 - See tpm_ctc_cm_acl_rule_add
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_add_rule_check(uint32_t owner_id,
					   tpm_src_port_type_t src_port,
					   uint32_t precedence,
					   tpm_parse_fields_t l2_parse_rule_bm,
					   tpm_parse_fields_t ipv4_parse_rule_bm,
					   tpm_parse_fields_t ipv6_parse_rule_bm,
					   tpm_l2_acl_key_t *l2_key,
					   tpm_ipv4_acl_key_t *ipv4_key,
					   tpm_ipv6_acl_key_t *ipv6_key,
					   tpm_pkt_frwd_t *pkt_frwd,
					   tpm_pkt_action_t pkt_act,
					   uint32_t pbits)
{
	int32_t int_ret_code;
	uint32_t num_rules, dummy_entry = 0;
	uint8_t is_range_full;
	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;
	tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win;

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	/* if CTC CnM enabled */
	if (ctc_cm_enable == TPM_CTC_CM_DISABLED) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "CTC CnM is disabled!\n");
		return(ERR_GENERAL);
	}

	if (src_port < TPM_SRC_PORT_UNI_0 || src_port > TPM_SRC_PORT_UNI_VIRT) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "src_port (%d) illegal!\n", src_port);
		return(ERR_SRC_PORT_INVALID);
	}

	if (!tpm_db_eth_port_valid(src_port)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "src_port (%d) illegal!\n", src_port);
		return(ERR_SRC_PORT_INVALID);
	}

	if (precedence >= TPM_MAX_NUM_CTC_PRECEDENCE) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "precedence (%d) illegal!\n", precedence);
		return(ERR_CTC_CM_PREC_INVALID);
	}

	num_rules = tpm_db_ctc_cm_get_num_rules(src_port);
	if (precedence > num_rules) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "precedence (%d) must be continous!\n", precedence);
		return(ERR_CTC_CM_PREC_INVALID);
	}

	if (!l2_parse_rule_bm && !ipv4_parse_rule_bm && !ipv6_parse_rule_bm) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "both l2_parse_rule_bm and ipv4_parse_rule_bm are Zero!\n");
		return(ERR_PARSE_MAP_INVALID);
	}

	if (l2_parse_rule_bm & ~(TPM_L2_PARSE_MAC_DA | TPM_L2_PARSE_MAC_SA | TPM_L2_PARSE_ONE_VLAN_TAG |
				 TPM_L2_PARSE_TWO_VLAN_TAG | TPM_L2_PARSE_ETYPE)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "l2_parse_rule_bm (%d) is invalid!\n", l2_parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	if (ipv4_parse_rule_bm & ~(TPM_IPv4_PARSE_SIP | TPM_IPv4_PARSE_DIP | TPM_IPv4_PARSE_DSCP |
				   TPM_IPv4_PARSE_PROTO | TPM_PARSE_L4_SRC | TPM_PARSE_L4_DST)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "ipv4_parse_rule_bm (%d) is invalid!\n", ipv4_parse_rule_bm);
		return(ERR_PARSE_MAP_INVALID);
	}

	/* ipv6_parse_rule_bm depends on ipv6 CnM parse window */
	tpm_db_ctc_cm_ipv6_parse_win_get(&ctc_cm_ipv6_parse_win);
	if(TPM_CTC_CM_IPv6_FIRST_24B == ctc_cm_ipv6_parse_win) {
		if (ipv6_parse_rule_bm & ~(TPM_IPv6_PARSE_SIP | TPM_IPv6_PARSE_DSCP | TPM_IPv6_PARSE_NH |
					   TPM_IPv6_PARSE_HOPL)) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, "ipv6_parse_rule_bm (%d) is invalid!\n", ipv6_parse_rule_bm);
			return(ERR_PARSE_MAP_INVALID);
		}
	} else {
		if (ipv6_parse_rule_bm & ~(TPM_IPv6_PARSE_DIP | TPM_PARSE_L4_SRC | TPM_PARSE_L4_DST)) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, "ipv6_parse_rule_bm (%d) is invalid!\n", ipv6_parse_rule_bm);
			return(ERR_PARSE_MAP_INVALID);
		}
	}

	if (l2_parse_rule_bm && (NULL == l2_key)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "l2_key can not be NULL!\n");
		return(ERR_NULL_POINTER);
	}

	if (ipv4_parse_rule_bm && (NULL == ipv4_key)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "ipv4_key can not be NULL!\n");
		return(ERR_NULL_POINTER);
	}

	if (ipv6_parse_rule_bm && (NULL == ipv6_key)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "ipv6_key can not be NULL!\n");
		return(ERR_NULL_POINTER);
	}

	if (!pkt_act) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "pkt_act is Zero!\n");
		return(ERR_ACTION_INVALID);
	}

	if ((pkt_act & (TPM_ACTION_SET_TARGET_PORT | TPM_ACTION_SET_TARGET_QUEUE)) &&
		(NULL == pkt_frwd)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "pkt_frwd is NULL when TPM_ACTION_SET_TARGET_PORT or"
					     " TPM_ACTION_SET_TARGET_QUEUE!\n");
		return(ERR_NULL_POINTER);
	}

	if (pkt_frwd->trg_port < TPM_TRG_PORT_WAN || pkt_frwd->trg_port > TPM_TRG_LLID_7) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "trg_port (%d) is invalid!\n", pkt_frwd->trg_port);
		return(ERR_FRWD_INVALID);
	}

	if (pkt_act & ~(TPM_ACTION_SET_TARGET_PORT | TPM_ACTION_SET_TARGET_QUEUE | TPM_ACTION_SET_PKT_MOD)) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "pkt_act (%d) is invalid!\n", pkt_act);
		return(ERR_ACTION_INVALID);
	}

	if (pkt_act & TPM_ACTION_SET_PKT_MOD) {
		if (pbits > 7) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, "pbits (%d) is invalid!\n", pbits);
			return(ERR_L2_KEY_INVALID);
		}

		int_ret_code = tpm_db_split_mod_get_index_by_p_bits(pbits, &dummy_entry);
		if (int_ret_code != TPM_DB_OK) {
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " unsupported p-bit value: %d\n", pbits);
			return(ERR_MOD_INVALID);
		}
	}

	if (l2_parse_rule_bm && ipv4_parse_rule_bm) {
		is_range_full = tpm_proc_if_ipv4_pre_range_is_full(src_port, ipv4_parse_rule_bm, ipv4_key);
		if (is_range_full != TPM_FALSE) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, "not enough room for new ipv4 pre-filter key!\n");
			return(ERR_OUT_OF_RESOURCES);
		}
	}

	is_range_full = tpm_proc_if_cnm_main_range_is_full(src_port, precedence);
	if (is_range_full != TPM_FALSE) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "not enough room for new cnm rule!\n");
		return(ERR_OUT_OF_RESOURCES);
	}

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_ctc_cm_acl_rule_add()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_acl_rule_add(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t precedence,
					 tpm_parse_fields_t l2_parse_rule_bm,
					 tpm_parse_fields_t ipv4_parse_rule_bm,
					 tpm_parse_fields_t ipv6_parse_rule_bm,
					 tpm_l2_acl_key_t *l2_key,
					 tpm_ipv4_acl_key_t *ipv4_key,
					 tpm_ipv6_acl_key_t *ipv6_key,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_pkt_action_t pkt_act,
					 uint32_t pbits)
{
	tpm_error_code_t            ret_code;
	tpm_db_error_t              db_ret;
	tpm_db_ctc_cm_rule_entry_t  cm_rule;
	uint32_t                    ipv4_sub_pattern_key_idx = TPM_CNM_INVALID_IPV4_PRE_FILTER_KEY_ID;
	uint32_t                    rule_index;
	uint32_t                    rule_index_tmp;
	uint32_t                    num_prec;
	int32_t                     loop;
	int32_t                     prece_tmp;

	/* Check parameters */
	ret_code = tpm_ctc_cm_add_rule_check(owner_id, src_port, precedence, l2_parse_rule_bm, ipv4_parse_rule_bm,
					     ipv6_parse_rule_bm, l2_key, ipv4_key, ipv6_key, pkt_frwd, pkt_act, pbits);
	IF_ERROR(ret_code);

	/* add IPv4_Pre of Combo rule */
	if (l2_parse_rule_bm && ipv4_parse_rule_bm) {
		ret_code = tpm_proc_add_cnm_ipv4_pre_filter_key(owner_id, src_port, ipv4_parse_rule_bm, ipv4_key,
								&ipv4_sub_pattern_key_idx);
		IF_ERROR(ret_code);
	}

	/* insert new rule */
	ret_code = tpm_ctc_cm_rule_add(owner_id, src_port, precedence, l2_parse_rule_bm, ipv4_parse_rule_bm,
				       ipv6_parse_rule_bm, l2_key, ipv4_key, ipv6_key, pkt_frwd, pkt_act, pbits,
				       ipv4_sub_pattern_key_idx, &rule_index);
	IF_ERROR(ret_code);

	/* update existing rules if needed */
	num_prec = tpm_db_ctc_cm_get_num_rules(src_port);

	/* from the lowest precedence to current precedence, decreased by 1 */
	for (loop = num_prec; loop > precedence; loop--) {

		prece_tmp = loop - 1;
		db_ret = tpm_db_ctc_cm_rule_get(src_port, prece_tmp, &cm_rule);
		if (TPM_DB_OK != db_ret) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, " DB failed, recvd ret_code(%d)\n", db_ret);
			return ERR_CTC_CM_DB_ERR;
		}

		/* first remvoe the old one, then add the new one  */
		ret_code = tpm_ctc_cm_rule_del(owner_id, &cm_rule);
		IF_ERROR(ret_code);

		/* last prece is 7, do not need to add the rule which has just
		 * been remvoed.
		 */
		if ((TPM_MAX_NUM_CTC_PRECEDENCE - 1) == prece_tmp)
			continue;

		/* add this rule with prece++ */
		ret_code = tpm_ctc_cm_rule_add(owner_id, src_port, prece_tmp + 1, cm_rule.l2_parse_rule_bm,
					       cm_rule.ipv4_parse_rule_bm, cm_rule.ipv6_parse_rule_bm,
					       &(cm_rule.l2_key), &(cm_rule.ipv4_key), &(cm_rule.ipv6_key),
					       &(cm_rule.pkt_frwd), (cm_rule.pkt_act), cm_rule.pbits,
					       cm_rule.ipv4_sub_pattern_key, &rule_index_tmp);
		IF_ERROR(ret_code);

		/* overwrite DB entry of lower precedence */
		db_ret = tpm_db_ctc_cm_rule_set(src_port, prece_tmp + 1, cm_rule.l2_parse_rule_bm,
						cm_rule.ipv4_parse_rule_bm, cm_rule.ipv6_parse_rule_bm,
						&(cm_rule.l2_key), &(cm_rule.ipv4_key), &(cm_rule.ipv6_key),
						&(cm_rule.pkt_frwd), (cm_rule.pkt_act),
						cm_rule.pbits, cm_rule.ipv4_sub_pattern_key, rule_index_tmp);
		if (TPM_DB_OK != db_ret) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, " DB failed, recvd ret_code(%d)\n", db_ret);
			return ERR_CTC_CM_DB_ERR;
		}

	}

	/* set db of this CnM rule */
	db_ret = tpm_db_ctc_cm_rule_set(src_port, precedence, l2_parse_rule_bm, ipv4_parse_rule_bm,
					ipv6_parse_rule_bm, l2_key, ipv4_key,
					ipv6_key, pkt_frwd, pkt_act, pbits,
					ipv4_sub_pattern_key_idx, rule_index);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, " DB failed, recvd ret_code(%d)\n", db_ret);
		return ERR_CTC_CM_DB_ERR;
	}

	return TPM_RC_OK;
}
/*******************************************************************************
* tpm_ctc_cm_del_rule_check()
*
* DESCRIPTION:    The function checks consistency of the tpm_ctc_cm_del_rule_check params.
*
* INPUTS:
*                 - See tpm_ctc_cm_del_rule_check
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_del_rule_check(uint32_t owner_id,
					   tpm_src_port_type_t src_port,
					   uint32_t precedence)
{
	tpm_db_error_t   db_ret;
	tpm_db_ctc_cm_rule_entry_t    cm_rule;
	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	/* if CTC CnM enabled */
	if (ctc_cm_enable == TPM_CTC_CM_DISABLED) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "CTC CnM is disabled!\n");
		return(ERR_GENERAL);
	}

	if (src_port < TPM_SRC_PORT_UNI_0 || src_port > TPM_SRC_PORT_UNI_VIRT) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "src_port (%d) illegal!\n", src_port);
		return(ERR_SRC_PORT_INVALID);
	}

	if (precedence >= TPM_MAX_NUM_CTC_PRECEDENCE) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "precedence (%d) illegal!\n", precedence);
		return(ERR_CTC_CM_PREC_INVALID);
	}

	db_ret = tpm_db_ctc_cm_rule_get(src_port, precedence, &cm_rule);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "get DB failed, ret_code: (%d), src_port: (%d), precedence: (%d)\n",
			 db_ret, src_port, precedence);
		return(ERR_CTC_CM_DB_ERR);
	}

	return TPM_RC_OK;
}

/*******************************************************************************
* tpm_ctc_cm_acl_rule_del()
*
* DESCRIPTION:    Main function for deleting CnM API rule.
*
* INPUTS:
*		  All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_acl_rule_del(uint32_t owner_id,
					 tpm_src_port_type_t src_port,
					 uint32_t precedence)
{
	tpm_error_code_t ret_code;
	tpm_db_error_t   db_ret;
	tpm_db_ctc_cm_rule_entry_t  cm_rule;
	uint32_t                    rule_index;
	uint32_t                    num_prec;
	uint32_t                    loop;

	/* Check parameters */
	ret_code = tpm_ctc_cm_del_rule_check(owner_id, src_port, precedence);
	IF_ERROR(ret_code);

	db_ret = tpm_db_ctc_cm_rule_get(src_port, precedence, &cm_rule);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "get DB failed, ret_code: (%d), src_port: (%d), precedence: (%d)\n",
			     db_ret, src_port, precedence);
		return(ERR_CTC_CM_DB_ERR);
	}

	/* remove rule */
	ret_code = tpm_ctc_cm_rule_del(owner_id, &cm_rule);
	IF_ERROR(ret_code);

	/* remove IPv4_Pre of Combo rule */
	if (cm_rule.l2_parse_rule_bm && cm_rule.ipv4_parse_rule_bm) {
		ret_code = tpm_proc_del_cnm_ipv4_pre_filter_key(owner_id, src_port, cm_rule.ipv4_sub_pattern_key);
		IF_ERROR(ret_code);
	}

	/* update existing rules if needed */
	num_prec = tpm_db_ctc_cm_get_num_rules(src_port);

	/* from the next lower-to current precedence to the lowest precedence, increased by 1 */
	for (loop = precedence + 1; loop < num_prec; loop++) {

		db_ret = tpm_db_ctc_cm_rule_get(src_port, loop, &cm_rule);
		if (TPM_DB_OK != db_ret) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, " DB failed, recvd ret_code(%d)\n", db_ret);
			return ERR_CTC_CM_DB_ERR;
		}

		/* add this rule with prece++ */
		ret_code = tpm_ctc_cm_rule_add(owner_id, src_port, loop - 1, cm_rule.l2_parse_rule_bm, cm_rule.ipv4_parse_rule_bm,
					       cm_rule.ipv6_parse_rule_bm, &(cm_rule.l2_key), &(cm_rule.ipv4_key),
					       &(cm_rule.ipv6_key), &(cm_rule.pkt_frwd),
					       cm_rule.pkt_act, cm_rule.pbits, cm_rule.ipv4_sub_pattern_key, &rule_index);
		IF_ERROR(ret_code);

		/* overwrite DB entry of higher precedence */
		db_ret = tpm_db_ctc_cm_rule_set(src_port, loop - 1, cm_rule.l2_parse_rule_bm, cm_rule.ipv4_parse_rule_bm,
					cm_rule.ipv6_parse_rule_bm, &(cm_rule.l2_key),
					&(cm_rule.ipv4_key), &(cm_rule.ipv6_key),
					&(cm_rule.pkt_frwd), cm_rule.pkt_act,
					cm_rule.pbits, cm_rule.ipv4_sub_pattern_key, rule_index);
		if (TPM_DB_OK != db_ret) {
			TPM_OS_ERROR(TPM_CTC_CM_MOD, " DB failed, recvd ret_code(%d)\n", db_ret);
			return ERR_CTC_CM_DB_ERR;
		}

		/* remove the old one */
		ret_code = tpm_ctc_cm_rule_del(owner_id, &cm_rule);
		IF_ERROR(ret_code);
	}

	/* reset the db of last rule */
	db_ret = tpm_db_ctc_cm_rule_delete(src_port, num_prec - 1);
	if (TPM_DB_OK != db_ret) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, " DB failed, recvd ret_code(%d)\n", db_ret);
		return ERR_CTC_CM_DB_ERR;
	}

	return ret_code;
}

/*******************************************************************************
* tpm_ctc_cm_ipv4_rule2cm()
*
* DESCRIPTION:    check if this rule can goto CnM stage from IPv4 stage.
*
* INPUTS:
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
* pkt_act            - Action associated to the rule
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns true. On error returns false.
*
* COMMENTS:
*
*******************************************************************************/
bool tpm_ctc_cm_ipv4_rule2cm(tpm_parse_flags_t parse_flags_bm, tpm_rule_action_t *rule_action)
{
	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	if (ctc_cm_enable == TPM_CTC_CM_DISABLED) {
		/* CTC CnM is disabled */
		return false;
	}

	/* not MTM, not to CPU */
	if (     (parse_flags_bm & TPM_PARSE_FLAG_MTM_FALSE)
	     && !(rule_action->pkt_act & TPM_ACTION_TO_CPU))
		return true;
	else
		return false;
}
/*******************************************************************************
* tpm_ctc_cm_l3_rule2cm_sram()
*
* DESCRIPTION:    check if this rule can goto CnM stage from ETH stage when building sram of ETH rule.
*
* INPUTS:
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
* pkt_act            - Action associated to the rule
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns true. On error returns false.
*
* COMMENTS:
*
*******************************************************************************/
bool tpm_ctc_cm_l3_rule2cm_sram(tpm_parse_flags_t parse_flags_bm, tpm_rule_action_t *rule_action)
{
	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	if (ctc_cm_enable == TPM_CTC_CM_DISABLED) {
		/* CTC CnM is disabled */
		return false;
	}

	/* [!mac-to-me] */
	if (parse_flags_bm & TPM_PARSE_FLAG_MTM_FALSE)
		return true;
	else
		return false;
}
/*******************************************************************************
* tpm_ctc_cm_l3_rule2cm_sram()
*
* DESCRIPTION:    check if this rule can goto CnM stage from ETH stage when building tcam of ETH rule.
*
* INPUTS:
* parse_flags_bm     - Bitmap containing the significant flags result of the primary ACL filtering.
* l3_key             - Structure for PPPoE proto or ether type. In order to define a rule for
*                      any ether type, the ether type value should be set to 0xFFFF
* rule_action        -used to get the next_stage of this rule

* OUTPUTS:
*
* RETURNS:
* On success, the function returns true. On error returns false.
*
* COMMENTS:
*
*******************************************************************************/
bool tpm_ctc_cm_l3_rule2cm_tcam(tpm_parse_flags_t parse_flags_bm, tpm_l3_type_key_t *l3_key, tpm_rule_action_t *rule_action)
{
	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	if (ctc_cm_enable == TPM_CTC_CM_DISABLED) {
		/* CTC CnM is disabled */
		return false;
	}

	/* not ipv4/6 or pppoe and [!mac-to-me] */
	if (   (0x0800 != l3_key->ether_type_key)
	    && (0x86dd != l3_key->ether_type_key)
	    && (ETH_P_PPP_SES != l3_key->ether_type_key)
	    && (parse_flags_bm & TPM_PARSE_FLAG_MTM_FALSE)
	    && (STAGE_CTC_CM == rule_action->next_phase))
		return true;
	else
		return false;
}

/*******************************************************************************
* tpm_ctc_cm_ipv6_acl_rule_add()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_ipv6_acl_rule_add(uint32_t owner_id,
					      tpm_src_port_type_t src_port,
					      uint32_t precedence,
					      tpm_parse_fields_t ipv6_parse_rule_bm,
					      tpm_ipv6_acl_key_t *ipv6_key,
					      tpm_pkt_frwd_t *pkt_frwd,
					      tpm_pkt_action_t pkt_act,
					      uint32_t pbits)
{
	tpm_error_code_t            ret_code;
	tpm_parse_fields_t          ipv4_parse_rule_bm = 0;
	tpm_parse_fields_t	    l2_parse_rule_bm = 0;
	tpm_l2_acl_key_t            l2_key;
	tpm_ipv4_acl_key_t          ipv4_key;

	ret_code = tpm_ctc_cm_acl_rule_add(owner_id, src_port, precedence, l2_parse_rule_bm,
					   ipv4_parse_rule_bm, ipv6_parse_rule_bm, &l2_key,
					   &ipv4_key, ipv6_key, pkt_frwd, pkt_act, pbits);
	IF_ERROR(ret_code);

	return TPM_RC_OK;
}
/*******************************************************************************
* tpm_ctc_cm_set_ipv6_parse_win()
*
* DESCRIPTION:    Main function for adding CnM API rule.
*
* INPUTS:
*           All inputs/outputs are same as API call
*
* OUTPUTS:
*
* RETURNS:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
tpm_error_code_t tpm_ctc_cm_set_ipv6_parse_win(uint32_t owner_id, tpm_ctc_cm_ipv6_parse_win_t ipv6_parse_window)
{
	uint32_t                    entry_id;
	uint32_t                    ipv6_rule_nums;
	int32_t                     ret_code = TPM_OK;
	tpm_db_ctc_cm_ipv6_parse_win_t ipv6_parse_win_orig;

	tpm_db_ctc_cm_ipv6_parse_win_get(&ipv6_parse_win_orig);
	if (ipv6_parse_window == ipv6_parse_win_orig) {
		TPM_OS_INFO(TPM_CTC_CM_MOD, "ipv6 parse win remains the same\n");
		return TPM_OK;
	}

	/* if there is still IPv6 CnM rule exist, reture error */
	ipv6_rule_nums = tpm_db_ctc_cm_get_ipv6_num_rules();
	if (0 != ipv6_rule_nums) {
		TPM_OS_ERROR(TPM_CTC_CM_MOD, "there is still IPv6 CnM rule exist, remove first!\n");
		return ERR_CTC_CM_DB_ERR;
	}

	/* get ipv6_ety_rule_num */
	tpm_db_ctc_cm_ipv6_ety_rule_num_get(&entry_id);

	ret_code = tpm_pncl_init_cnm_main_ety_ipv6(entry_id, ipv6_parse_window);
	IF_ERROR(ret_code);

	/* set new ipv6 parse window */
	tpm_db_ctc_cm_ipv6_parse_win_set(ipv6_parse_window);

	return TPM_RC_OK;
}

