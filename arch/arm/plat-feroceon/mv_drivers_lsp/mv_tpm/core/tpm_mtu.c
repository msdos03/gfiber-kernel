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
* tpm_mtu.c
*
*
*  MODULE : TPM
*
*  DESCRIPTION : This file config tpm mtu
*
*  MODIFICATION HISTORY:
*           15Aug11   jinghua - initial version created.
*
* FILE REVISION NUMBER:
*       Revision: 1.12
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_mtu.h"
#include "tpm_pkt_proc_logic.h"

tpm_mtu_pnc_cfg_t g_tpm_mtu_cfg;
tpm_mtu_pnc_entries_t g_tpm_mtu_pnc_entry_tmp;

#define IF_ERROR(ret)		\
	if (ret != TPM_OK) {\
		TPM_OS_ERROR(TPM_MTU_MOD, " recvd ret_code(%d)\n", ret);\
		return(ret);\
	}

uint32_t tpm_mtu_get_pnc_free_index(void)
{
	static uint32_t l_rule_idx = 100;
	return (l_rule_idx++);
}

int32_t tpm_mtu_del_acl_pnc_entry(uint32_t ethertype, uint32_t pnc_index)
{
	tpm_db_pnc_range_t range_data;
	uint32_t pnc_range_id;
	uint32_t pnc_start, pnc_stop;
	int32_t int_ret_code;
	tpm_mtu_pnc_index_t *pnc_index_cfg;
	int32_t loop;

	/* Get pnc_range_id */
	if (TPM_MTU_IPV4 == ethertype) {
		pnc_range_id = TPM_PNC_IPV4_LEN;
		pnc_index_cfg = &(g_tpm_mtu_cfg.ipv4_mtu_cfg.pncIndex_cfg);
	} else {
		/*pnc_range_id = TPM_PNC_IPV6_LEN; 
		pnc_index_cfg = &(g_tpm_mtu_cfg.ipv6_mtu_cfg.pncIndex_cfg);*/
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "there is no length check for IPv6\n");
		return ERR_GENERAL;
	}

	/*** Calculate PNC Entry ***/

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(pnc_range_id, &range_data);
	IF_ERROR(int_ret_code);

	for (loop = 0; loop < pnc_index_cfg->pncNumber; loop++) {
		if (pnc_index == pnc_index_cfg->pncIndex[loop])
			break;
	}

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d), pncIndex(%d)\n", loop, pnc_index);

	/* Pull range from this index untill last used entry in Pnc range */
	pnc_start = range_data.pnc_range_conf.range_start + loop;
	pnc_stop = range_data.pnc_range_conf.range_start + pnc_index_cfg->pncNumber - 1;

	int_ret_code = tpm_pncl_entry_delete(pnc_start, pnc_stop);
	IF_ERROR(int_ret_code);

	/* Increase number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_inc(pnc_range_id);
	IF_ERROR(int_ret_code);

	/* update the pnc index cfg */
	if (pnc_index_cfg->pncNumber > 1) {
		for (; loop < pnc_index_cfg->pncNumber - 1; loop++)
			pnc_index_cfg->pncIndex[loop] = pnc_index_cfg->pncIndex[loop + 1];
	}

	pnc_index_cfg->pncNumber--;
	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_index_cfg->pncNumber : %d, pncIndex: %d\n", pnc_index_cfg->pncNumber,
		     pnc_index);

	return (TPM_OK);
}

int32_t tpm_mtu_create_acl_pnc_entry(uint32_t rule_num, uint32_t ethertype,
				     tpm_pncl_pnc_full_t *pnc_data, uint32_t *pnc_index)
{
	tpm_db_pnc_range_t range_data;
	uint32_t pnc_range_id;
	uint32_t pnc_range_start, api_start, pnc_stop_entry, pnc_entries_number, pnc_entry;
	int32_t int_ret_code;
	tpm_mtu_pnc_index_t *pnc_index_cfg;
	int loop;

	/* Get pnc_range_id */
	if (TPM_MTU_IPV4 == ethertype) {
		pnc_range_id = TPM_PNC_IPV4_LEN;
		pnc_index_cfg = &(g_tpm_mtu_cfg.ipv4_mtu_cfg.pncIndex_cfg);
	} else {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "there is no length check for IPv6\n");
		return ERR_GENERAL;
		/*pnc_range_id = TPM_PNC_IPV6_LEN; 
		pnc_index_cfg = &(g_tpm_mtu_cfg.ipv6_mtu_cfg.pncIndex_cfg);*/
	}

	/*** Calculate PNC Entry ***/

	/* Get PNC Range Start */
	int_ret_code = tpm_db_pnc_rng_get(pnc_range_id, &range_data);
	IF_ERROR(int_ret_code);

	pnc_range_start = range_data.pnc_range_conf.range_start;
	api_start = range_data.pnc_range_conf.api_start;

	/* Calculate absolute PNC entry number to execute */
	pnc_entry = (pnc_range_start + api_start) + rule_num;

	/* Get number of existing pnc entries */
	pnc_entries_number = pnc_index_cfg->pncNumber;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " rule_num(%d), pnc_entries_number(%d)\n", rule_num, pnc_entries_number);

	/* Call PNC Entry Insert, if this is not the api_section's new last entry */
	if (rule_num < pnc_entries_number) {
		pnc_stop_entry = (pnc_range_start + api_start) + (pnc_entries_number - 1);
		int_ret_code = tpm_pncl_entry_insert(pnc_entry, pnc_stop_entry, pnc_data);
		IF_ERROR(int_ret_code);
	} else {		/* Otherwise just set the entry (no insertion) */

		int_ret_code = tpm_pncl_entry_set(pnc_entry, pnc_data);
		IF_ERROR(int_ret_code);
	}

	/* Decrease number of free entries in pnc_range */
	int_ret_code = tpm_db_pnc_rng_free_ent_dec(pnc_range_id);
	IF_ERROR(int_ret_code);

	/* update the pnc index cfg */
	*pnc_index = tpm_mtu_get_pnc_free_index();

	if (pnc_index_cfg->pncNumber > 0) {
		for (loop = pnc_index_cfg->pncNumber - 1; loop >= rule_num; loop--) {
			pnc_index_cfg->pncIndex[loop + 1] = pnc_index_cfg->pncIndex[loop];
			if (0 == loop)
				break;
		}

	}

	pnc_index_cfg->pncIndex[rule_num] = *pnc_index;
	pnc_index_cfg->pncNumber++;

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, " pnc_index_cfg->pncNumber : %d, pncIndex: %d\n", pnc_index_cfg->pncNumber,
		     pnc_index_cfg->pncIndex[rule_num]);

	return (TPM_OK);
}

tpm_error_code_t tpm_mtu_ipv4_default_pnc_entry_insert(void)
{
	int32_t int_ret_code;
	uint32_t free_entries, pnc_entry;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "\n");

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_LEN, &range_data);
	IF_ERROR(int_ret_code);

	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_end;

	/*************************************************/
	/* Basic TCAM/SRAM Config, valid for all entries */
	/*************************************************/
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));

	pnc_data.pncl_tcam.lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_tcam.ipv4_parse_bm = 0;

	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_LUD;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV4_LEN, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	int_ret_code = tpm_db_pnc_rng_free_ent_inc(TPM_PNC_IPV4_LEN);
	IF_ERROR(int_ret_code);

	return (TPM_RC_OK);
}

#if 0
tpm_error_code_t tpm_mtu_ipv6_default_pnc_entry_insert(void)
{
	int32_t int_ret_code;
	uint32_t free_entries, ipv6_frag_type_lu, pnc_entry;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data, nextphase_range_data;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&nextphase_range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_TPM_LOG_MOD, "\n");

	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_TCP_FLAG, &nextphase_range_data);
	IF_ERROR(int_ret_code);
	ipv6_frag_type_lu = nextphase_range_data.pnc_range_conf.base_lu_id;

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV6_LEN, &range_data);
	IF_ERROR(int_ret_code);

	free_entries = range_data.pnc_range_oper.free_entries;
	pnc_entry = range_data.pnc_range_conf.range_start + range_data.pnc_range_conf.api_end;

	/*************************************************/
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));

	pnc_data.pncl_tcam.lu_id = range_data.pnc_range_conf.base_lu_id;
	pnc_data.pncl_tcam.port_ids = tpm_proc_all_gmac_bm();
	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_VER;
	pnc_data.pncl_tcam.ipv6_parse_bm = 0;

	pnc_data.pncl_sram.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV6_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv6_subf = TPM_IPv6_PARSE_VER;
	pnc_data.pncl_sram.sram_updt_bm = 0;
	pnc_data.pncl_sram.next_lu_id = ipv6_frag_type_lu;

	/* Create Entry in PnC */
	int_ret_code = tpm_proc_pnc_create(TPM_PNC_IPV6_LEN, pnc_entry, &pnc_data);
	IF_ERROR(int_ret_code);

	return (TPM_RC_OK);

}
#endif

void tpm_mtu_printBinary(uint32_t inValue)
{
	uint32_t desArray[16] = { 0 };
	uint32_t i;
	for (i = 0; i < 16; i++) {
		if (0 != (inValue & (1 << i)))
			desArray[i] = 1;
	}

	for (i = 16; i > 0; i--)
		printk("%d", desArray[i - 1]);

	return;
}

uint32_t tpm_mtu_get_binary_length(uint32_t inValue)
{
	uint32_t i;
	for (i = 16; i > 0; i--) {
		if (0 != (inValue & (1 << (i - 1))))
			return i;
	}

	return 0;
}

void tpm_mtu_add_len_chk_entry(uint32_t pkt_value, uint32_t pkt_mask)
{
/*    printf("\npkt_value: ");
    printBinary(pkt_value);
    printf(" ,   %d", pkt_value);
    printf("\npkt_mask : ");
    printBinary(pkt_mask);
    printf(" ,   %d", pkt_mask);
    printf("\n===========================================================");
*/
	g_tpm_mtu_pnc_entry_tmp.pktMask[g_tpm_mtu_pnc_entry_tmp.pncEntryNum] = pkt_mask;
	g_tpm_mtu_pnc_entry_tmp.pktValue[g_tpm_mtu_pnc_entry_tmp.pncEntryNum] = pkt_value;
	g_tpm_mtu_pnc_entry_tmp.pncEntryNum++;

	return;
}

void tpm_mtu_caculate_value_mask(uint32_t min, uint32_t max)
{
	uint32_t max_tmp, min_tmp;
	uint32_t max_length = 0, min_length = 0;
	uint32_t i = 0, tmp_range_max;
	uint32_t pkt_value, pkt_mask;

	memset(&g_tpm_mtu_pnc_entry_tmp, 0, sizeof(g_tpm_mtu_pnc_entry_tmp));

	if (min > max)
		return;

	min++;
	max_length = tpm_mtu_get_binary_length(max);
	min_length = tpm_mtu_get_binary_length(min);

	/* get the length of the max */
	max_tmp = max;
	min_tmp = min;

	for (i = 0; i < max_length; i++) {
		tmp_range_max = min_tmp | (0xffff >> (15 - i));

		if (max == min_tmp) {
			pkt_mask = 0xffff;

			pkt_value = min_tmp;

			tpm_mtu_add_len_chk_entry(pkt_value, pkt_mask);

			return;
		}

		if (max == tmp_range_max) {
			if (0 != (min_tmp & (1 << i)))
				pkt_mask = 0xffff << i;
			else
				pkt_mask = 0xffff << (i + 1);

			pkt_value = min_tmp & pkt_mask;

			tpm_mtu_add_len_chk_entry(pkt_value, pkt_mask);

			return;
		}

		if (0 != (min_tmp & (1 << i))) {
			/*here we get the 1. */
			if (max > tmp_range_max) {
				/*here we get the 1 in min_tmp. */
				pkt_mask = 0xffff << i;

				pkt_value = min_tmp & pkt_mask;

				tpm_mtu_add_len_chk_entry(pkt_value, pkt_mask);

				min_tmp = min_tmp + (1 << i);
			} else {
				/*here we get some problem. */
				break;
			}
		} else {
			if (max > tmp_range_max)
				continue;
			else {
				/*here we get some problem. */
				break;
			}
		}
	}

	if (i == max_length) {
		/* here we got nothing left to do */
		return;
	}

	/* here is for the thing like this:
	   min: 10000
	   max: 10100
	   in this case, i = 2;
	 */
	for (; i > 0; i--) {
		if (GET_BIT(max, i) == GET_BIT(min_tmp, i))
			continue;

		pkt_mask = 0xffff << i;

		pkt_value = min_tmp & pkt_mask;

		tpm_mtu_add_len_chk_entry(pkt_value, pkt_mask);

		min_tmp = min_tmp + (1 << i);
	}

	/* here we set the last one, i = 0
	   min: 10100
	   max: 10100
	   or
	   min: 10100
	   max: 10101
	 */
	if (GET_BIT(max, i) == GET_BIT(min_tmp, i))
		pkt_mask = 0xffff;
	else
		pkt_mask = 0xffff << 1;

	pkt_value = min_tmp;

	tpm_mtu_add_len_chk_entry(pkt_value, pkt_mask);

	return;
}

tpm_error_code_t tpm_mtu_ipv4_len_entry_create(uint32_t len_start,
					       uint32_t len_mask,
					       bool pppoe_only,
					       uint32_t direction, uint32_t rule_num, uint32_t *rule_idx)
{
	int32_t int_ret_code;
	uint32_t free_entries;
	tpm_gmac_bm_t gmac_bm;
	uint32_t cpu_rx_queue;

	tpm_pncl_pnc_full_t pnc_data;
	tpm_pncl_offset_t start_offset;
	tpm_db_pnc_range_t range_data;

	/* Set Structs to zero */
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	TPM_OS_DEBUG(TPM_MTU_MOD, "\n");

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_LEN, &range_data);
	IF_ERROR(int_ret_code);

	free_entries = range_data.pnc_range_oper.free_entries;

	if (free_entries < 1) {
		TPM_OS_WARN(TPM_MTU_MOD, "PNC length range(%d) too small, can not insert anymore \n", TPM_PNC_IPV4_LEN);
		return TPM_FAIL;
	}

	/*************************************************/
	/* Basic TCAM/SRAM Config, valid for all entries */
	/*************************************************/
	memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
	memset(&start_offset, 0, sizeof(tpm_pncl_offset_t));

	pnc_data.pncl_tcam.lu_id = range_data.pnc_range_conf.base_lu_id;
	if (TPM_DOWNSTREAM == direction) {
		/* Get PMAC(s) */
		tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &gmac_bm);
	} else {
		/* Get GMAC(s) */
		tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_UNI_ANY, &gmac_bm);
	}
	pnc_data.pncl_tcam.port_ids = gmac_bm;

	pnc_data.pncl_tcam.start_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_tcam.start_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_tcam.ipv4_parse_bm = TPM_IPv4_PARSE_TOTLEN;

	/* Only check length for packets that are not already going to CPU */
	pnc_data.pncl_tcam.add_info_data = (0 << TPM_AI_TO_CPU_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_TO_CPU_MASK;

	/* Only check length for packets that are MTM */
	pnc_data.pncl_tcam.add_info_data = (1 << TPM_AI_MTM_BIT_OFF);
	pnc_data.pncl_tcam.add_info_mask = TPM_AI_MTM_MASK;

	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);

	pnc_data.pncl_sram.pnc_queue = cpu_rx_queue;
	pnc_data.pncl_sram.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	pnc_data.pncl_sram.next_offset.offset_base = TPM_PNCL_IPV4_OFFSET;
	pnc_data.pncl_sram.next_offset.offset_sub.ipv4_subf = TPM_IPv4_PARSE_VER;
	pnc_data.pncl_sram.sram_updt_bm = TPM_PNCL_SET_TXP | TPM_PNCL_SET_LUD;
	pnc_data.pncl_sram.flow_id_sub.pnc_target = TPM_PNC_TRG_CPU;

	/* Signal the packet is going to CPU */
	pnc_data.pncl_sram.add_info_data |= (1 << TPM_AI_TO_CPU_BIT_OFF);
	pnc_data.pncl_sram.add_info_mask |= TPM_AI_TO_CPU_MASK;

	if (free_entries == 0) {
		TPM_OS_ERROR(TPM_MTU_MOD, "No free entries\n");
		return (TPM_FAIL);
	}

	/* Build Tcam Entry */
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_totlen = len_start;
	pnc_data.pncl_tcam.pkt_key.ipv4_add_key.ipv4_totlen_mask = len_mask;

	if (pppoe_only) {
		/* YUVAL - this only handles pppoe_add, which is ok as long as L2_hwf does not need to be len_checked */
		pnc_data.pncl_tcam.add_info_data |= (1 << TPM_AI_PPPOE_ADD_BIT_OFF);
		pnc_data.pncl_tcam.add_info_mask |= TPM_AI_PPPOE_ADD_MASK;

	}

	/*** Insert the PNC Entry ***/
	int_ret_code = tpm_mtu_create_acl_pnc_entry(rule_num, TPM_MTU_IPV4, &pnc_data, rule_idx);
	IF_ERROR(int_ret_code);

	TPM_OS_INFO(TPM_MTU_MOD,
		    "insert new len chk PNC rule: pktValue(%d), pktMask(%d), pnc_entry(%d), pncIndex(%d),\n", len_start,
		    len_mask, rule_num, *rule_idx);

	return (TPM_OK);
}

tpm_error_code_t tpm_mtu_safety_net_entry_create(uint32_t *entry_index, uint32_t direction, uint32_t ethertype)
{
	tpm_error_code_t tpm_ret;
	uint32_t safety_net_entry;

	TPM_OS_INFO(TPM_MTU_MOD, "direction(%d), ethertype(%d), \n", direction, ethertype);

	if (TPM_MTU_IPV4 == ethertype) {
		safety_net_entry = g_tpm_mtu_cfg.ipv4_mtu_cfg.mtu_ds_cfg.pncEntryNum
		    + g_tpm_mtu_cfg.ipv4_mtu_cfg.mtu_pppoe2ipoe_us_cfg.pncEntryNum
		    + g_tpm_mtu_cfg.ipv4_mtu_cfg.mtu_ipoe2max_us_cfg.pncEntryNum;

		tpm_ret = tpm_mtu_ipv4_len_entry_create(g_tpm_mtu_cfg.safety_net_value,
							g_tpm_mtu_cfg.safety_net_mask,
							false, direction, safety_net_entry, entry_index);
	} else
		tpm_ret = TPM_OK;

	TPM_OS_INFO(TPM_MTU_MOD, "entry_index(%d)\n", *entry_index);

	return tpm_ret;
}

uint32_t tpm_mtu_update_pnc(tpm_mtu_pnc_entries_t *mtu_cfg,
			    uint32_t start_pnc_entry,
			    uint32_t ethertype, uint32_t direction, uint32_t free_pnc_entry_number, bool pppoe_only)
{
	uint32_t pnc_entry_num_tmp;
	uint32_t pnc_entry_num_original;
	tpm_error_code_t tpm_ret;
	uint32_t safety_net_entry;
	tpm_src_port_type_t src_port;

	pnc_entry_num_tmp = 0;
	pnc_entry_num_original = 0;

	TPM_OS_INFO(TPM_MTU_MOD, "direction(%d), ethertype(%d), start_pnc_entry(%d), free_pnc_entry_number(%d),\n",
		    direction, ethertype, start_pnc_entry, free_pnc_entry_number);

	if (TPM_MTU_IPV6 == ethertype)
		return TPM_OK;

	/* insert safety net first */
	tpm_ret = tpm_mtu_safety_net_entry_create(&safety_net_entry, direction, ethertype);
	if (TPM_RC_OK != tpm_ret) {
		TPM_OS_ERROR(TPM_MTU_MOD, "fail to insert safety net pnc entry, number: %d\n", pnc_entry_num_tmp);
		return TPM_FAIL;
	}

	if (TPM_DOWNSTREAM == direction)
		src_port = TPM_SRC_PORT_WAN;
	else
		src_port = TPM_SRC_PORT_UNI_ANY;

	while ((free_pnc_entry_number > 0) && (pnc_entry_num_tmp < g_tpm_mtu_pnc_entry_tmp.pncEntryNum)) {
		/* insert new */
		tpm_ret = tpm_mtu_ipv4_len_entry_create(g_tpm_mtu_pnc_entry_tmp.pktValue[pnc_entry_num_tmp],
							g_tpm_mtu_pnc_entry_tmp.pktMask[pnc_entry_num_tmp],
							pppoe_only,
							direction,
							pnc_entry_num_tmp + start_pnc_entry,
							&g_tpm_mtu_pnc_entry_tmp.pncIndex[pnc_entry_num_tmp]);
		if (TPM_RC_OK != tpm_ret) {
			TPM_OS_ERROR(TPM_MTU_MOD, "fail to insert len chk pnc entry, number: %d\n", pnc_entry_num_tmp);
			return TPM_FAIL;
		}

		pnc_entry_num_tmp++;
		free_pnc_entry_number--;

		TPM_OS_INFO(TPM_MTU_MOD, "pnc_entry_inert_num(%d), free_pnc_entry_number(%d)\n",
			    pnc_entry_num_tmp, free_pnc_entry_number);
	}

	while (pnc_entry_num_original < mtu_cfg->pncEntryNum) {
		/* remove old one */
		tpm_ret = tpm_mtu_del_acl_pnc_entry(ethertype, mtu_cfg->pncIndex[pnc_entry_num_original]);
		IF_ERROR(tpm_ret);

		TPM_OS_INFO(TPM_MTU_MOD, "remove (%d) PNC entry pncIndex(%d)\n",
			    pnc_entry_num_original, mtu_cfg->pncIndex[pnc_entry_num_original]);

		pnc_entry_num_original++;

		/* insert new one if there is */
		if (pnc_entry_num_tmp < g_tpm_mtu_pnc_entry_tmp.pncEntryNum) {
			tpm_ret = tpm_mtu_ipv4_len_entry_create(g_tpm_mtu_pnc_entry_tmp.pktValue[pnc_entry_num_tmp],
								g_tpm_mtu_pnc_entry_tmp.pktMask[pnc_entry_num_tmp],
								pppoe_only,
								direction,
								pnc_entry_num_tmp + start_pnc_entry,
								&g_tpm_mtu_pnc_entry_tmp.pncIndex[pnc_entry_num_tmp]);
			if (TPM_RC_OK != tpm_ret) {
				TPM_OS_ERROR(TPM_MTU_MOD, "fail to insert len chk pnc entry, number: %d\n",
					     pnc_entry_num_tmp);
				return TPM_FAIL;
			}
			TPM_OS_INFO(TPM_MTU_MOD, "pnc_entry_inert_num(%d)\n", pnc_entry_num_tmp);
		}

		pnc_entry_num_tmp++;
	}

	/* set the config */
	memcpy(mtu_cfg, &g_tpm_mtu_pnc_entry_tmp, sizeof(g_tpm_mtu_pnc_entry_tmp));

	/* remove safety net */
	tpm_ret = tpm_mtu_del_acl_pnc_entry(ethertype, safety_net_entry);
	IF_ERROR(tpm_ret);

	return TPM_OK;
}

uint32_t tpm_proc_set_mtu(uint32_t ethertype, uint32_t direction, uint32_t mtu)
{

/*
	1. If ipv4_pppoe_mtu has been set,

		if ipv4_pppoe_mtu > ipv4_MTU, return error.
		else use algorism to get the PNC entries for (ipv4_pppoe_MTU to ipv4_MTU) with pppoe AI bits

	2. Use algorism to get the PNC entries for (ipv4_MTU to tpm_max_pkt_len)

	3. If the PNC entries number for this two sets is bigger than the size of IPv4 len check range,
		return error and quit;

	4. If any of the two sets of PNC entries for pkt len check have already been set,
		use the algorism in section 3.4 to update the PNC entries.
*/

	tpm_mtu_ethtype_pnc_cfg_t *ethtype_pnc_cfg;
	uint32_t pppoe_mtu;
	uint32_t pre_mtu;
	uint32_t new_pnc_entry_num = 0;
	uint32_t free_entry_number;
	uint32_t int_ret_code;
	tpm_mtu_pnc_entries_t *mtu_cfg;
	tpm_db_mtu_setting_enable_t enable;
	tpm_db_pppoe_add_enable_t pppoe_add_enable;

	tpm_db_get_mtu_enable(&enable);
	if (TPM_MTU_CHECK_DISABLED == enable) {
		TPM_OS_WARN(TPM_MTU_MOD, "mtu check is not enabled!\n");
		return TPM_OK;
	}

	tpm_db_get_pppoe_add_enable(&pppoe_add_enable);

	if (TPM_MTU_IPV4 == ethertype) {
		ethtype_pnc_cfg = &(g_tpm_mtu_cfg.ipv4_mtu_cfg);
		tpm_db_mtu_get_ipv4_pppoe_mtu_us(&pppoe_mtu);
	} else {
		ethtype_pnc_cfg = &(g_tpm_mtu_cfg.ipv6_mtu_cfg);
		tpm_db_mtu_get_ipv6_pppoe_mtu_us(&pppoe_mtu);
	}

	if ((TPM_UPSTREAM == direction) && (pppoe_mtu >= mtu)) {
		/* pppoe_mtu is bigger than ipoe_mtu, does not make sense */
		TPM_OS_WARN(TPM_MTU_MOD, "pppoe_mtu is bigger than ipoe_mtu, does not make sense\n");
	}

	TPM_OS_DEBUG(TPM_MTU_MOD, "pppoe_mtu: %d, ipoe_mtu: %d\n", pppoe_mtu, mtu);

	/* get PNC entries */
	tpm_mtu_caculate_value_mask(mtu, g_tpm_mtu_cfg.sys_pkt_len_max);

	new_pnc_entry_num = g_tpm_mtu_pnc_entry_tmp.pncEntryNum;

	if ((TPM_UPSTREAM == direction) && (TPM_PPPOE_ADD_ENABLED == pppoe_add_enable)) {
		/* pppoe_mtu to ipoe need to be calculate again */
		tpm_mtu_caculate_value_mask(pppoe_mtu, mtu);
		new_pnc_entry_num += g_tpm_mtu_pnc_entry_tmp.pncEntryNum;
		new_pnc_entry_num += ethtype_pnc_cfg->mtu_ds_cfg.pncEntryNum;
	} else {
		new_pnc_entry_num += ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum;
		new_pnc_entry_num += ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum;
	}

	TPM_OS_DEBUG(TPM_MTU_MOD, "new_pnc_entry_num: %d\n", new_pnc_entry_num);

	if ((ethtype_pnc_cfg->rangeSize) < new_pnc_entry_num) {
		TPM_OS_ERROR(TPM_MTU_MOD,
			     "the new pnc entry number is bigger than the range size,  range size: %d, new size: %d\n",
			     (ethtype_pnc_cfg->rangeSize), new_pnc_entry_num);

		return TPM_FAIL;
	}

	if (TPM_UPSTREAM == direction) {
		if (TPM_MTU_IPV4 == ethertype)
			tpm_db_mtu_get_ipv4_mtu_us(&pre_mtu);
		else
			tpm_db_mtu_get_ipv6_mtu_us(&pre_mtu);

		mtu_cfg = &ethtype_pnc_cfg->mtu_ipoe2max_us_cfg;
	} else {
		if (TPM_MTU_IPV4 == ethertype)
			tpm_db_mtu_get_ipv4_mtu_ds(&pre_mtu);
		else
			tpm_db_mtu_get_ipv6_mtu_ds(&pre_mtu);

		mtu_cfg = &ethtype_pnc_cfg->mtu_ds_cfg;
	}

	free_entry_number = (ethtype_pnc_cfg->rangeSize) -
		(ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum + ethtype_pnc_cfg->mtu_ds_cfg.pncEntryNum
		+ ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum);

	if (TPM_DOWNSTREAM == direction) {
		/* IN DS, ONLY mtu to max would be updated */
		/* get PNC entries */
		tpm_mtu_caculate_value_mask(mtu, g_tpm_mtu_cfg.sys_pkt_len_max);

		/* update the PNC entry on HW */
		int_ret_code = tpm_mtu_update_pnc(mtu_cfg,
						  ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum +
						  ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum, ethertype,
						  direction, free_entry_number, false);

		IF_ERROR(int_ret_code);

		return TPM_OK;
	}

	/* IN US, both mtu to max and pppoe to mtu need to be updated */
	if (pre_mtu > mtu) {
		/* mtu getting smaller, handle mtu to max first */
		/* get PNC entries */
		tpm_mtu_caculate_value_mask(mtu, g_tpm_mtu_cfg.sys_pkt_len_max);

		/* update the PNC entry on HW */
		int_ret_code = tpm_mtu_update_pnc(mtu_cfg, ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum,
						  ethertype, direction, free_entry_number, false);
		IF_ERROR(int_ret_code);

		/* handle pppoe to mtu */
		if (TPM_PPPOE_ADD_DISABLED == pppoe_add_enable) {
			/* no pppoe_mtu */
			TPM_OS_DEBUG(TPM_MTU_MOD, "no pppoe_mtu, return\n");
			return TPM_OK;
		}

		/* update free entry number */
		free_entry_number = (ethtype_pnc_cfg->rangeSize) -
			(ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum + ethtype_pnc_cfg->mtu_ds_cfg.pncEntryNum
			+ ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum);

		/* get PNC entries */
		tpm_mtu_caculate_value_mask(pppoe_mtu, mtu);

		/* update the PNC entry on HW */
		int_ret_code =
		    tpm_mtu_update_pnc(&(ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg), 0, ethertype, direction,
				       free_entry_number, true);

		IF_ERROR(int_ret_code);
	} else {
		/* mtu getting bigger, handle pppoe to mtu first */
		if (TPM_PPPOE_ADD_ENABLED == pppoe_add_enable) {
			/* no pppoe_mtu */
			TPM_OS_DEBUG(TPM_MTU_MOD, " pppoe_mtu ENABLED, handle pppoe to mtu first\n");
			/* get PNC entries */
			tpm_mtu_caculate_value_mask(pppoe_mtu, mtu);

			/* update the PNC entry on HW */
			int_ret_code =
				tpm_mtu_update_pnc(&(ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg), 0, ethertype, direction,
						free_entry_number, true);

			IF_ERROR(int_ret_code);
		}

		/* handle mtu to max */

		/* update free entry number */
		free_entry_number = (ethtype_pnc_cfg->rangeSize) -
			(ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum + ethtype_pnc_cfg->mtu_ds_cfg.pncEntryNum
			+ ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum);

		/* get PNC entries */
		tpm_mtu_caculate_value_mask(mtu, g_tpm_mtu_cfg.sys_pkt_len_max);

		/* update the PNC entry on HW */
		int_ret_code = tpm_mtu_update_pnc(mtu_cfg, ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum,
						ethertype, direction, free_entry_number, false);

		IF_ERROR(int_ret_code);
	}

	return TPM_OK;
}

uint32_t tpm_proc_set_pppoe_mtu(uint32_t ethertype, uint32_t direction, uint32_t pppoe_mtu)
{
	/*
	   1. if ipv4_ mtu has been set,
	   if  ipv4_pppoe_mtu > ipv4_mtu, generate a warning.
	   else use algorism to get the PNC entries for (ipv4_pppoe_max to ipv4_mtu) with pppoe AI bits
	   else
	   use algorism to get the PNC entries for (ipv4_pppoe_max  to   tpm_max_pkt_len) with pppoe AI bits.

	   2. if ipv4_mtu has been set, use algorism to get the PNC entries for
	   (ipv4_mtu to tpm_mtu)

	   3. if the PNC entries number for this two sets is bigger than the size of
	   IPv4 len check range, return error and quit;

	   4. if any of the two sets of PNC entries for pkt len check
	   have already been set, , use the algorism in section 3.4 to update the PNC entries
	 */

	tpm_mtu_ethtype_pnc_cfg_t *ethtype_pnc_cfg;
	uint32_t mtu_tmp;
	uint32_t new_pnc_entry_num;
	uint32_t free_entry_number;
	tpm_db_mtu_setting_enable_t enable;
	tpm_db_pppoe_add_enable_t pppoe_add_enable;

	tpm_db_get_mtu_enable(&enable);
	if (TPM_MTU_CHECK_DISABLED == enable) {
		TPM_OS_WARN(TPM_MTU_MOD, "mtu check is not enabled!\n");
		return TPM_OK;
	}

	tpm_db_get_pppoe_add_enable(&pppoe_add_enable);
	if (TPM_PPPOE_ADD_DISABLED == pppoe_add_enable) {
		TPM_OS_WARN(TPM_MTU_MOD, "pppoe mtu check is not enabled!\n");
		return TPM_OK;
	}

	if (TPM_DOWNSTREAM == direction) {
		/* no pppoe mtu in DS */
		TPM_OS_WARN(TPM_MTU_MOD, " no pppoe mtu in DS\n");
		return TPM_OK;
	}

	if (TPM_MTU_IPV4 == ethertype)
		ethtype_pnc_cfg = &(g_tpm_mtu_cfg.ipv4_mtu_cfg);
	else
		ethtype_pnc_cfg = &(g_tpm_mtu_cfg.ipv6_mtu_cfg);

	if (TPM_MTU_IPV4 == ethertype) {
		ethtype_pnc_cfg = &(g_tpm_mtu_cfg.ipv4_mtu_cfg);
		tpm_db_mtu_get_ipv4_mtu_us(&mtu_tmp);
	} else {
		ethtype_pnc_cfg = &(g_tpm_mtu_cfg.ipv6_mtu_cfg);
		tpm_db_mtu_get_ipv6_mtu_us(&mtu_tmp);
	}

	if (MV_TPM_UN_INITIALIZED_INIT_PARAM == mtu_tmp) {
		/* no ipoe mtu set */
		mtu_tmp = g_tpm_mtu_cfg.sys_pkt_len_max;
	}

	if (pppoe_mtu >= mtu_tmp) {
		/* pppoe_mtu is bigger than ipoe_mtu, does not make sense */
		TPM_OS_WARN(TPM_MTU_MOD, "pppoe_mtu is bigger than ipoe_mtu, does not make sense\n");
	}

	TPM_OS_DEBUG(TPM_MTU_MOD, "pppoe_mtu: %d, ipoe_mtu: %d\n", pppoe_mtu, mtu_tmp);

	/* get PNC entries */
	tpm_mtu_caculate_value_mask(pppoe_mtu, mtu_tmp);

	new_pnc_entry_num = ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum + ethtype_pnc_cfg->mtu_ds_cfg.pncEntryNum
		+ g_tpm_mtu_pnc_entry_tmp.pncEntryNum;

	if ((ethtype_pnc_cfg->rangeSize) < new_pnc_entry_num) {
		/* the new pnc entry number is bigger than the range size
		 */
		TPM_OS_ERROR(TPM_MTU_MOD,
				"the new pnc entry number is bigger than the range size,  range size: %d, new size: %d\n",
				(ethtype_pnc_cfg->rangeSize), new_pnc_entry_num);

		return TPM_FAIL;
	}

	free_entry_number = (ethtype_pnc_cfg->rangeSize) -
		(ethtype_pnc_cfg->mtu_ipoe2max_us_cfg.pncEntryNum + ethtype_pnc_cfg->mtu_ds_cfg.pncEntryNum
		+ ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg.pncEntryNum);

	/* update the PNC entry on HW */
	tpm_mtu_update_pnc(&ethtype_pnc_cfg->mtu_pppoe2ipoe_us_cfg, 0, ethertype, direction, free_entry_number, true);

	if (TPM_MTU_IPV4 == ethertype)
		tpm_db_mtu_set_ipv4_pppoe_mtu_us(pppoe_mtu);
	else
		tpm_db_mtu_set_ipv6_pppoe_mtu_us(pppoe_mtu);

	return TPM_OK;
}

void tpm_mtu_set_mru(void)
{
	uint32_t mru = 1500;
	tpm_db_pon_type_t pon_type;
	uint32_t num_vlan_tags;
	tpm_db_pppoe_add_enable_t pppoe_add_enable;
	tpm_db_mtu_setting_enable_t mtu_enable;
	tpm_gmacs_enum_t act_wan= tpm_db_active_wan_get();

	tpm_db_pon_type_get(&pon_type);
	tpm_db_get_num_vlan_tags(&num_vlan_tags);
	tpm_db_get_mtu_enable(&mtu_enable);

	tpm_db_get_pppoe_add_enable(&pppoe_add_enable);

	if (TPM_ENUM_PMAC == act_wan){
       	switch (pon_type) {
		case TPM_P2P:
			mru = 2048;
       		break;

		case TPM_EPON:
			mru = 1593;
			break;

		case TPM_GPON:
			/* [YuvalC] According to current info, for GPON 2048 bytes.
		           define it 2032 for now, to be on the safe side. */
		    	mru = 2032;
		    	break;

		default:
			TPM_OS_WARN(TPM_MTU_MOD, "act_wan is PMAC but no valid pon_type (%d)\n", pon_type);
			return;
       	}
	} else if (TPM_ENUM_GMAC_0 == act_wan || TPM_ENUM_GMAC_1 == act_wan)
		mru = 2048;

	mru -= ((num_vlan_tags) * 4);

	if (TPM_PPPOE_ADD_ENABLED == pppoe_add_enable && TPM_MTU_CHECK_DISABLED == mtu_enable) {
		mru -= 8;
		/*  This mru_reduction is only needed when mtu is disabled,
		 */
	}

	tpm_db_set_mru(mru);

}

int32_t tpm_proc_mtu_check_cpu_queue_size(uint32_t queue)
{
	tpm_db_gmac_conn_t gmac_con;
	uint32_t valid;
	uint32_t queue_size;
	/*tpm_db_pon_type_t pon_type; */

	/* GMAC0 rx */
	tpm_db_gmac_conn_get(TPM_ENUM_GMAC_0, &gmac_con);
	if (TPM_GMAC_CON_DISC != gmac_con) {
		tpm_db_gmac_rx_q_conf_get(TPM_ENUM_GMAC_0, queue, &valid, &queue_size);
		if (0 == queue_size) {
			TPM_OS_WARN(TPM_MTU_MOD, "cpu rx queue size is 0 from GMAC_0!\n");
			return TPM_FAIL;
		}
	}

	/* GMAC1 rx */
	tpm_db_gmac_conn_get(TPM_ENUM_GMAC_1, &gmac_con);
	if (TPM_GMAC_CON_DISC != gmac_con) {
		tpm_db_gmac_rx_q_conf_get(TPM_ENUM_GMAC_1, queue, &valid, &queue_size);
		if (0 == queue_size) {
			TPM_OS_WARN(TPM_MTU_MOD, "cpu rx queue size is 0 from GMAC_1!\n");
			return TPM_FAIL;
		}
	}

	/* PMAC Rx */
#if 0
	tpm_db_pon_type_get(&pon_type);
	if (TPM_GPON == pon_type || TPM_EPON == pon_type) {
		/* do not need to check */
		return TPM_OK;
	}
#endif
	tpm_db_gmac_conn_get(TPM_ENUM_PMAC, &gmac_con);
	if (TPM_GMAC_CON_DISC != gmac_con) {
		tpm_db_gmac_rx_q_conf_get(TPM_ENUM_PMAC, queue, &valid, &queue_size);
		if (0 == queue_size) {
			TPM_OS_WARN(TPM_MTU_MOD, "cpu rx queue size is 0 from TPM_ENUM_PMAC!\n");
			return TPM_FAIL;
		}
	}

	return TPM_OK;
}

int32_t tpm_proc_mtu_init(void)
{
	tpm_db_mtu_setting_enable_t enable;
	tpm_db_pnc_range_conf_t range_conf;
	int32_t ret_code;
	int32_t mtu_tmp, cpu_rx_queue;

	TPM_OS_DEBUG(TPM_MTU_MOD, "MTU init started!\n");

	/* Phase I - set MRU */
	tpm_mtu_set_mru();

	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);
	ret_code = tpm_proc_mtu_check_cpu_queue_size(cpu_rx_queue);
	if (ret_code != TPM_OK) {
		TPM_OS_WARN(TPM_MTU_MOD, "check_cpu_queue_size failed\n");
		return TPM_FAIL;
	}

	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_LEN, &range_conf);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_WARN(TPM_MTU_MOD, " no ipv4 PNC range\n");
		return TPM_FAIL;
	}
	tpm_db_get_mtu_enable(&enable);
	if (TPM_MTU_CHECK_DISABLED == enable) {
	/* range size smaller than 2, illegal */
		if (range_conf.range_size < 2) {
			TPM_OS_ERROR(TPM_INIT_MOD, "PNC range(%d) too small to be initialized, "
						   "current size(%d). Min size should be 2 \n",
				     TPM_PNC_IPV4_LEN, range_conf.range_size);
			return TPM_FAIL;
		} else if (range_conf.range_size > 2) {
			TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) too big, current size(%d). "
						  "it is a waste with MTU disabled, size should be 2\n",
				    TPM_PNC_IPV4_LEN, range_conf.range_size);
		}
	} else {
	/* range size smaller than 2, illegal */
		if (range_conf.range_size <= 2) {
			TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) too small to be initialized, size(%d) \n",
				    TPM_PNC_IPV4_LEN, range_conf.range_size);
			return TPM_FAIL;
		}
	}

	/* create default rule for ipv4 packets */
	tpm_mtu_ipv4_default_pnc_entry_insert();

	/* Phase II - Set MTU entries, only if */
	if (TPM_MTU_CHECK_DISABLED == enable) {
		TPM_OS_WARN(TPM_MTU_MOD, "mtu check is not enabled!\n");
		return TPM_OK;
	}

	memset(&g_tpm_mtu_cfg, 0, sizeof(g_tpm_mtu_cfg));
	g_tpm_mtu_cfg.sys_pkt_len_max = 2048;

	/* safety net is from 1536 to 2047 */
	g_tpm_mtu_cfg.safety_net_value = 0x600;
	g_tpm_mtu_cfg.safety_net_mask = 0xfe00;

	/*************************************************/
	/* IPv4 MTU init                                 */
	/*************************************************/
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_LEN, &range_conf);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_WARN(TPM_MTU_MOD, " no ipv4 PNC range\n");
		return TPM_FAIL;
	}
	/* range size smaller than 2, illegal */
	if (range_conf.range_size <= 2) {
		TPM_OS_WARN(TPM_INIT_MOD, "PNC range(%d) too small to be initialized, size(%d) \n", TPM_PNC_IPV4_LEN,
			    range_conf.range_size);
		return TPM_FAIL;
	}

	/* we need to reserve 3 PNC entry for default and safety net     */
	g_tpm_mtu_cfg.ipv4_mtu_cfg.rangeSize = range_conf.range_size - 3;

	/*************************************************/
	/* IPv6 MTU init                                 */
	/*************************************************/
#if 0
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_LEN, &range_conf);
	if (ret_code != TPM_DB_OK) {
		TPM_OS_WARN(TPM_MTU_MOD, " no ipv6 PNC range\n");
		return TPM_FAIL;
	}
	if (range_conf.range_size <= 2) {
		TPM_OS_WARN(TPM_MTU_MOD, "PNC range(%d) too small to be initialized, size(%d) \n", TPM_PNC_IPV6_LEN,
			    range_conf.range_size);
		return TPM_FAIL;
	}
#endif
	/* we need to reserve 3 PNC entry for default and safety net     */
	g_tpm_mtu_cfg.ipv6_mtu_cfg.rangeSize = range_conf.range_size - 3;

	/*tpm_mtu_ipv6_default_pnc_entry_insert(); */

	/* set ipv4 US */
	tpm_db_mtu_get_ipv4_mtu_us(&mtu_tmp);
	tpm_proc_set_mtu(TPM_MTU_IPV4, TPM_UPSTREAM, mtu_tmp);
	tpm_db_mtu_get_ipv4_pppoe_mtu_us(&mtu_tmp);
	tpm_proc_set_pppoe_mtu(TPM_MTU_IPV4, TPM_UPSTREAM, mtu_tmp);

	/* set ipv6 US
	   tpm_db_mtu_get_ipv6_mtu_us(&mtu_tmp);
	   tpm_proc_set_mtu(TPM_MTU_IPV6, TPM_UPSTREAM, mtu_tmp);
	   tpm_db_mtu_get_ipv6_pppoe_mtu_us(&mtu_tmp);
	   tpm_proc_set_pppoe_mtu(TPM_MTU_IPV6, TPM_UPSTREAM, mtu_tmp);
	 */

	/* set ipv4 DS */
	tpm_db_mtu_get_ipv4_mtu_ds(&mtu_tmp);
	tpm_proc_set_mtu(TPM_MTU_IPV4, TPM_DOWNSTREAM, mtu_tmp);

	/* set ipv6 DS
	   tpm_db_mtu_get_ipv6_mtu_ds(&mtu_tmp);
	   tpm_proc_set_mtu(TPM_MTU_IPV6, TPM_DOWNSTREAM, mtu_tmp);
	 */

	return TPM_OK;
}

int32_t tpm_mtu_get_pnc_entry_from_index(int32_t index)
{
	int32_t loop;

	for (loop = 0; loop < g_tpm_mtu_cfg.ipv4_mtu_cfg.pncIndex_cfg.pncNumber; loop++) {
		if (index == g_tpm_mtu_cfg.ipv4_mtu_cfg.pncIndex_cfg.pncIndex[loop])
			return loop;
	}

	return 0;
}

void tpm_mtu_print_len_chk_entry(tpm_mtu_pnc_entries_t *mtu_pnc_entries)
{
	int32_t entry_number;
	int32_t loop, int_ret_code;
	tpm_db_pnc_range_t range_data;

	/* Get Range_Id */
	int_ret_code = tpm_db_pnc_rng_get(TPM_PNC_IPV4_LEN, &range_data);
	if (TPM_OK != int_ret_code) {
		printk(KERN_WARNING "there is no Len check range\n");
		return;
	}

	for (loop = 0; loop < mtu_pnc_entries->pncEntryNum; loop++) {
		entry_number = tpm_mtu_get_pnc_entry_from_index(mtu_pnc_entries->pncIndex[loop]);
		entry_number += range_data.pnc_range_conf.range_start;
		printk("PNC entry number: %d\n", entry_number);
		printk("length start: %d\n", mtu_pnc_entries->pktValue[loop]);
		tpm_mtu_printBinary(mtu_pnc_entries->pktValue[loop]);
		printk("\n");
		tpm_mtu_printBinary(mtu_pnc_entries->pktMask[loop]);
		printk("\n");
	}

}

void tpm_mtu_print_len_chk_range(void)
{
	printk("==========================\n");
	printk(" TPM IPv4 Length Check Range                \n");
	printk("==========================\n");
	printk(" Range size                        : %d\n", g_tpm_mtu_cfg.ipv4_mtu_cfg.rangeSize);
	printk(" Current PNC entry number: %d\n", g_tpm_mtu_cfg.ipv4_mtu_cfg.pncIndex_cfg.pncNumber);

	printk("__________________________\n");
	printk("US pppoe to ipoe\n");
	tpm_mtu_print_len_chk_entry(&g_tpm_mtu_cfg.ipv4_mtu_cfg.mtu_pppoe2ipoe_us_cfg);
	printk("__________________________\n");
	printk("US ipoe to max length \n");
	tpm_mtu_print_len_chk_entry(&g_tpm_mtu_cfg.ipv4_mtu_cfg.mtu_ipoe2max_us_cfg);
	printk("__________________________\n");
	printk("DS ipoe to max length \n");
	tpm_mtu_print_len_chk_entry(&g_tpm_mtu_cfg.ipv4_mtu_cfg.mtu_ds_cfg);
	printk("__________________________\n");

}
