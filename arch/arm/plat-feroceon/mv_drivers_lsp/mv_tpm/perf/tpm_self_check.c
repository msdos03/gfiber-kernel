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
* tpm_self_check.c
*
* DESCRIPTION:
*       API definitions for TPM checks the hw and the db are synchronized or not

*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_sysfs_utils.h"

#define IF_ERROR(ret)	\
		if (ret != TPM_OK) {\
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, " recvd ret_code(%d)\n", ret);\
			return(ret);\
		}

#define TPM_CHECK_DIV_BY_32(x)   ((x) >> 5)
#define TPM_CHECK_MOD_32(x)      ((x) & 0x1f)

extern char *opCodeOperationStr[30];

/*******************************************************************************
* pnc_mismatch_info_show - Show detailed difference between two tpm rule entry
*
* INPUT:
*       tpm_pnc_all_t *entry1
*       tpm_pnc_all_t *entry2
*
* OUTPUT:
*       None
*
* RETURN:
*
* COMMENTS:
* None
*******************************************************************************/
static void pnc_mismatch_info_show(tpm_pnc_all_t *entry1, tpm_pnc_all_t *entry2)
{
	if (entry1 == NULL || entry2 == NULL)
		return;

	printk("======================================================================================\n");

	printk("Tcam mismatch element:  ");

	if (memcmp(&entry1->tcam_entry.lu_id, &entry2->tcam_entry.lu_id, sizeof(uint32_t)))
		printk("Lookup ID, ");
	if (memcmp(&entry1->tcam_entry.port_ids, &entry2->tcam_entry.port_ids, sizeof(tpm_gmac_bm_t)))
		printk("Port ID, ");
	if (memcmp(&entry1->tcam_entry.add_info_data, &entry2->tcam_entry.add_info_data, sizeof(uint32_t)))
		printk("AddInfo data, ");
	if (memcmp(&entry1->tcam_entry.add_info_mask, &entry2->tcam_entry.add_info_mask, sizeof(uint32_t)))
		printk("AddInfo mask, ");
	if (memcmp(&entry1->tcam_entry.pkt_data.pkt_byte[0], &entry2->tcam_entry.pkt_data.pkt_byte[0],
			TPM_TCAM_PKT_WIDTH * sizeof(uint8_t)))
		printk("Header data");

	printk("\n");

	printk("Sram mismatch element:  ");

	if (memcmp(&entry1->sram_entry.add_info_data, &entry2->sram_entry.add_info_data, sizeof(uint32_t)))
		printk("Additional Info, ");
	if (memcmp(&entry1->sram_entry.add_info_mask, &entry2->sram_entry.add_info_mask, sizeof(uint32_t)))
		printk("Add Info Mask, ");
	if (memcmp(&entry1->sram_entry.flowid_updt_mask, &entry2->sram_entry.flowid_updt_mask, sizeof(uint32_t)))
		printk("FlowID_updt_mask, ");
	if (memcmp(&entry1->sram_entry.flowid_val, &entry2->sram_entry.flowid_val, sizeof(uint32_t)))
		printk("Flow ID, ");
	if (memcmp(&entry1->sram_entry.lookup_done, &entry2->sram_entry.lookup_done, sizeof(uint32_t)))
		printk("Lookup done, ");
	if (memcmp(&entry1->sram_entry.next_lu_id, &entry2->sram_entry.next_lu_id, sizeof(uint32_t)))
		printk("Next_lu_id, ");
	if (memcmp(&entry1->sram_entry.next_lu_off_reg, &entry2->sram_entry.next_lu_off_reg, sizeof(uint32_t)))
		printk("Next_lu_off_reg, ");
	if (memcmp(&entry1->sram_entry.pnc_queue, &entry2->sram_entry.pnc_queue, sizeof(uint32_t)))
		printk("Pnc queue, ");
	if (memcmp(&entry1->sram_entry.res_info_15_0_data, &entry2->sram_entry.res_info_15_0_data, sizeof(uint32_t)))
		printk("Res_info_15_0, ");
	if (memcmp(&entry1->sram_entry.res_info_15_0_mask, &entry2->sram_entry.res_info_15_0_mask, sizeof(uint32_t)))
		printk("Res_info_15_0 mask, ");
	if (memcmp(&entry1->sram_entry.res_info_23_16_data, &entry2->sram_entry.res_info_23_16_data, sizeof(uint32_t)))
		printk("Res_info_23_16, ");
	if (memcmp(&entry1->sram_entry.res_info_23_16_mask, &entry2->sram_entry.res_info_23_16_mask, sizeof(uint32_t)))
		printk("Res_info_23_16 mask, ");
	if (memcmp(&entry1->sram_entry.shift_updt_reg, &entry2->sram_entry.shift_updt_reg, sizeof(uint32_t)))
		printk("Shift_updt_reg, ");
	if (memcmp(&entry1->sram_entry.shift_updt_val, &entry2->sram_entry.shift_updt_val, sizeof(uint32_t)))
		printk("Shift_updt_val, ");

	printk("\n======================================================================================\n\n");
}

/*******************************************************************************
* tran_tcam_entry_to_pnc_all_t - translate tcam_entry structure to tpm_pnc_all_t
*
* INPUT:
*	in_tcam: source struct tcam_entry
*	rule_action: rule action of pnc rule
*	api_section: pnc rule belong to
*
* OUTPUT:
*	out_pnc: target tpm_pnc_all_t structure
*
* RETURN:None
*******************************************************************************/
static void tran_tcam_entry_to_pnc_all_t(tpm_pnc_all_t *out_pnc,
					 struct tcam_entry *in_tcam,
					 tpm_rule_action_t *rule_action,
					 tpm_api_sections_t api_section)
{
	unsigned int lookup_mask;
	int tcam_off;

	/*check parameters*/
	if (out_pnc == NULL || in_tcam == NULL || rule_action == NULL) {
		printk(KERN_ERR "NULL pointer\n");
		return;
	}
	if (api_section > TPM_CNM_MAIN_ACL || api_section < TPM_CPU_LOOPBACK_ACL) {
		printk(KERN_ERR "api_section out of range\n");
		return;
	}
	/*get tcam entry*/
	tcam_sw_get_lookup(in_tcam, &(out_pnc->tcam_entry.lu_id), &lookup_mask);
	out_pnc->tcam_entry.port_ids = (~(in_tcam->mask.u.word[PORT_WORD] >> PORT_OFFS)) & PORT_MASK;
	tcam_sw_get_ainfo(in_tcam,
			&(out_pnc->tcam_entry.add_info_data),
			&(out_pnc->tcam_entry.add_info_mask));
	/* Get tcam packet data */
	for (tcam_off = 0; tcam_off < TPM_TCAM_PKT_WIDTH; tcam_off++) {
		out_pnc->tcam_entry.pkt_data.pkt_byte[tcam_off] = in_tcam->data.u.byte[tcam_off];
		out_pnc->tcam_entry.pkt_mask.pkt_byte[tcam_off] = in_tcam->mask.u.byte[tcam_off];
	}
	/*Get sram entry*/
	out_pnc->sram_entry.flowid_val = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(FLOW_VALUE_OFFS)] >>
							TPM_CHECK_MOD_32(FLOW_VALUE_OFFS)) & FLOW_VALUE_MASK;
	out_pnc->sram_entry.flowid_updt_mask = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(FLOW_CTRL_OFFS)] >>
							TPM_CHECK_MOD_32(FLOW_CTRL_OFFS)) & FLOW_CTRL_MASK;
	out_pnc->sram_entry.res_info_15_0_data = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(RI_VALUE_OFFS)] >>
							TPM_CHECK_MOD_32(RI_VALUE_OFFS)) & RI_VALUE_15_0_MASK;
	out_pnc->sram_entry.res_info_23_16_data = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(RI_VALUE_OFFS)] >>
							(TPM_CHECK_MOD_32(RI_VALUE_OFFS) + 16)) & RI_VALUE_23_16_MASK;
	out_pnc->sram_entry.res_info_15_0_mask = in_tcam->sram.word[TPM_CHECK_DIV_BY_32(RI_MASK_OFFS)] &
							RI_VALUE_15_0_MASK;
	out_pnc->sram_entry.res_info_23_16_mask = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(RI_MASK_OFFS)] >> 16) &
							RI_VALUE_23_16_MASK;
	out_pnc->sram_entry.res_info_15_0_data = out_pnc->sram_entry.res_info_15_0_data &
							out_pnc->sram_entry.res_info_15_0_mask;
	out_pnc->sram_entry.res_info_23_16_data = out_pnc->sram_entry.res_info_23_16_data &
							out_pnc->sram_entry.res_info_23_16_mask;
	out_pnc->sram_entry.shift_updt_val = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(SHIFT_VAL_OFFS)] >>
							TPM_CHECK_MOD_32(SHIFT_VAL_OFFS)) & SHIFT_VAL_MASK;
	out_pnc->sram_entry.shift_updt_reg = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(SHIFT_IDX_OFFS)] >>
							TPM_CHECK_MOD_32(SHIFT_IDX_OFFS)) & SHIFT_IDX_MASK;

	/*update target queue, supposed that API DB is always right, here to avoid error*/
	if (SET_TARGET_QUEUE(rule_action->pkt_act) ||
				(api_section == TPM_IPV4_MC) ||
				(api_section == TPM_IPV6_MC_ACL))
		out_pnc->sram_entry.pnc_queue = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(RXQ_QUEUE_OFFS)] >>
							TPM_CHECK_MOD_32(RXQ_QUEUE_OFFS)) & RXQ_MASK;
	else
		out_pnc->sram_entry.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;

	out_pnc->sram_entry.next_lu_off_reg = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(NEXT_LU_SHIFT_OFFS)] >>
							TPM_CHECK_MOD_32(NEXT_LU_SHIFT_OFFS)) & NEXT_LU_SHIFT_MASK;
	out_pnc->sram_entry.lookup_done = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(LU_DONE_OFFS)] >>
							TPM_CHECK_MOD_32(LU_DONE_OFFS)) & LU_DONE_MASK;
	out_pnc->sram_entry.add_info_data = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(AI_VALUE_OFFS)] >>
							TPM_CHECK_MOD_32(AI_VALUE_OFFS)) & AI_MASK;
	out_pnc->sram_entry.add_info_mask = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(AI_MASK_OFFS)] >>
							TPM_CHECK_MOD_32(AI_MASK_OFFS)) & AI_MASK;
	out_pnc->sram_entry.next_lu_id = (in_tcam->sram.word[TPM_CHECK_DIV_BY_32(LU_ID_OFFS)] >>
							TPM_CHECK_MOD_32(LU_ID_OFFS)) & LU_MASK;

	return;
}

/*******************************************************************************
* tpm_check_update_sram - update sram according to some special condition
*                       - packet drop, flowid_update_mask
*                       - just to avoid meaningless pnc check error
* INPUT:
*	rule_action
* OUTPUT:
*	sram_entry
* RETURN:None
*******************************************************************************/
static void tpm_check_update_sram(tpm_sram_all_t *sram_entry, tpm_rule_action_t *rule_action)
{
	uint32_t mask = 0;
	uint32_t loop;

	if (NULL == sram_entry || NULL == rule_action)
		return;

	/*update flow id*/
	for (loop = 0; loop < 8; loop++) {
		if ((1 << loop) & sram_entry->flowid_updt_mask)
			mask |= (0xF << (4 * loop));
	}
	sram_entry->flowid_val = sram_entry->flowid_val & mask;

	/*update pnc queue, if rule action is drop, set pnc queue to 0xFFFF*/
	if (PKT_DROP(rule_action->pkt_act))
		sram_entry->pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
}

int32_t tpm_check_main_chain_type(tpm_gmacs_enum_t gmac_port, tpm_pkt_mod_bm_t mod_bm, tpm_pkt_mod_int_bm_t int_mod_bm,
				  uint16_t jump_idx, tpm_main_chain_check_type_t *result)
{
	tpm_chain_type_t main_chain_type, db_main_chain_type, hw_main_chain_type;
	uint16_t main_chain_entry, main_idx;
	int32_t ret;
	uint8_t error_db, error_hw;

	error_db = 0;
	error_hw = 0;
	/* Get main chain type from mod_bm */
	main_chain_type = tpm_mod2_parse_chain_type(mod_bm, int_mod_bm);

	/*Get main chain type from db*/
	ret = tpm_mod2_main_chain_index_get(gmac_port, jump_idx, &main_chain_entry, TPM_CHECK_MAIN_CHAIN_IDX_DB);
	if (ret)
		return TPM_FAIL;
	ret = tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port, main_chain_entry, &db_main_chain_type, &main_idx);
	if (ret)
		return TPM_FAIL;
	if (main_chain_type != db_main_chain_type)
		error_db++;

	/*Get main chain type from HW*/
	ret = tpm_mod2_main_chain_index_get(gmac_port, jump_idx, &main_chain_entry, TPM_CHECK_MAIN_CHAIN_IDX_HW);
	if (ret)
		return TPM_FAIL;
	ret = tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port, main_chain_entry, &hw_main_chain_type, &main_idx);
	if (ret)
		return TPM_FAIL;
	if (main_chain_type != hw_main_chain_type)
		error_hw++;

	if (error_db && error_hw)
		*result = TPMCHECK_BOTH_CHAIN_TYPE_MISMATCH;
	else if (error_db)
		*result = TPMCHECK_DB_CHAIN_TYPE_MISMATCH;
	else if (error_hw)
		*result = TPMCHECK_HW_CHAIN_TYPE_MISMATCH;
	else
		*result = TPMCHECK_CHAIN_TYPE_OK;

	return TPM_OK;
}

static void tpm_check_pmt_mismatch_show_header(void)
{
    printk("======================================================================================\n");
    printk(" Info_Src  Index    OpCode     Operation   Data  Last  IPv4    L4   \n");
    printk("                                                      update update \n");
    printk("======================================================================================\n");
}

/*******************************************************************************
* tpm_pmt_check - check PMT entries
*
* INPUT:
*       tpm_gmacs_enum_t gmac_port
*	tpm_mod_pattern_data_t *pattern_data
*	tpm_self_check_enable_t check_with, check with DB or HW
*
* OUTPUT:
*	tpm_self_check_result_t *result, record check result
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
static int32_t tpm_pmt_check(tpm_gmacs_enum_t gmac_port, tpm_mod_pattern_data_t *pattern_data,
			     tpm_error_code_t *result, tpm_self_check_enable_t check_with)
{
	uint32_t set_id, line_id;
	tpm_pattern_entry_t *entry_p = NULL;
	uint16_t pmt_entry;
	MV_NETA_PMT pEntry;
	tpm_mod2_entry_t pattern_jump, pattern_chain;
	/*error flag*/
	int8_t modification_data = 0;
	int8_t mod_command = 0;
	int8_t up_ipv4_checksum = 0;
	int8_t up_tcp_udp_checksum = 0;
	int8_t last = 0;
	int8_t hw_error_count, sw_error_count;

	if (gmac_port >= TPM_MAX_NUM_GMACS) {
		printk("Invalid parameter: port %d\n", gmac_port);
		return TPM_FAIL;
	}

	if (NULL == pattern_data) {
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
		return TPM_FAIL;
	}

	hw_error_count = 0;
	sw_error_count = 0;

	for (set_id = 0; set_id < TPM_MOD2_MAX_PATTERN_SETS; set_id++) {
		if (pattern_data->pattern_set[set_id].is_valid) {
			entry_p = &(pattern_data->pattern_set[set_id]);
			if (set_id == 0)
				pmt_entry = entry_p->index;
			else
				pmt_entry = tpm_db_mod2_convert_chain_to_pmt_entry(entry_p->chain_type, entry_p->index);

			for (line_id = 0; line_id < TPM_MOD2_MAX_PATTERN_ENTRIES && line_id < entry_p->line_num;
				line_id++) {
				if (check_with == TPM_CHECK_WITH_HW) {
					/*read HW PMT table*/
					if (tpm_mod2_neta_pmt_get(gmac_port, pmt_entry + line_id, &pEntry)) {
						TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get hw jump data\n");
						return TPM_FAIL;
					}

					/*compare HW PMT*/
					if ((pEntry.word & MV_NETA_PMT_DATA_MASK) != entry_p->line[line_id].data)
						modification_data = 1;
					if (((pEntry.word >> MV_NETA_PMT_CMD_OFFS) & (MV_NETA_PMT_CMD_ALL_MASK >>
							MV_NETA_PMT_CMD_OFFS)) != entry_p->line[line_id].opcode)
						mod_command = 1;
					if (((pEntry.word >> MV_NETA_PMT_IP4_CSUM_BIT) & (MV_NETA_PMT_IP4_CSUM_MASK >>
							MV_NETA_PMT_IP4_CSUM_BIT)) != entry_p->line[line_id].updt_ipv4)
						up_ipv4_checksum = 1;
					if (((pEntry.word >> MV_NETA_PMT_L4_CSUM_BIT) & (MV_NETA_PMT_L4_CSUM_MASK >>
							MV_NETA_PMT_L4_CSUM_BIT)) != entry_p->line[line_id].updt_tcp)
						up_tcp_udp_checksum = 1;
					if (((pEntry.word >> MV_NETA_PMT_LAST_BIT) & (MV_NETA_PMT_LAST_MASK >>
							MV_NETA_PMT_LAST_BIT)) != entry_p->line[line_id].last)
						last = 1;

					/*show HW error info*/
					if (modification_data || mod_command || up_ipv4_checksum ||
						up_tcp_udp_checksum || last) {
						hw_error_count++;
						printk("\nPMT Mismatch: [set %d] area_type %d, chain_type %d, "
							"chain_index %d ,line_num %d\r\n",
							set_id, entry_p->area_type, entry_p->chain_type,
							entry_p->index, entry_p->line_num);
						tpm_check_pmt_mismatch_show_header();
						printk(" %5s      %03d      0x%2x %15s  0x%4.4x  %1d     "
							"%1d      %1d\n",
							"Build",
							line_id,
							entry_p->line[line_id].opcode,
							opCodeOperationStr[entry_p->line[line_id].opcode],
							entry_p->line[line_id].data, entry_p->line[line_id].last,
							entry_p->line[line_id].updt_ipv4,
							entry_p->line[line_id].updt_tcp);
						printk(" %5s      %03d      0x%2x %15s  0x%4.4x  %1d     "
							"%1d      %1d\n",
							"HWPMT",
							line_id,
							((pEntry.word >> MV_NETA_PMT_CMD_OFFS) &
							(MV_NETA_PMT_CMD_ALL_MASK >> MV_NETA_PMT_CMD_OFFS)),
							opCodeOperationStr[((pEntry.word >> MV_NETA_PMT_CMD_OFFS) &
							(MV_NETA_PMT_CMD_ALL_MASK >> MV_NETA_PMT_CMD_OFFS))],
							(pEntry.word & MV_NETA_PMT_DATA_MASK),
							((pEntry.word >> MV_NETA_PMT_LAST_BIT) &
							(MV_NETA_PMT_LAST_MASK >> MV_NETA_PMT_LAST_BIT)),
							((pEntry.word >> MV_NETA_PMT_IP4_CSUM_BIT) &
							(MV_NETA_PMT_IP4_CSUM_MASK >> MV_NETA_PMT_IP4_CSUM_BIT)),
							((pEntry.word >> MV_NETA_PMT_L4_CSUM_BIT) &
							(MV_NETA_PMT_L4_CSUM_MASK >> MV_NETA_PMT_L4_CSUM_BIT)));
					}

					/*clear error flags*/
					modification_data = 0;
					mod_command = 0;
					up_ipv4_checksum = 0;
					up_tcp_udp_checksum = 0;
					last = 0;
				} else if (check_with == TPM_CHECK_WITH_DB) {
					/*read PMT db info*/
					if (set_id == 0) {
						if (tpm_mod2_db_jump_pattern_data_get(gmac_port, entry_p->index,
											&pattern_jump)) {
							TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Get Jump pattern failed\n");
							return TPM_FAIL;
						}
					} else {
						if (tpm_mod2_db_chain_pattern_data_get(gmac_port, entry_p->chain_type,
									entry_p->index, line_id, &pattern_chain)) {
							TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Get Chain pattern failed\n");
							return TPM_FAIL;
						}
					}

					/*compare to db*/
					if (set_id == 0) {
						if (pattern_jump.data != entry_p->line[line_id].data)
							modification_data = 1;
						if (pattern_jump.opcode != entry_p->line[line_id].opcode)
							mod_command = 1;
						if (pattern_jump.updt_ipv4 != entry_p->line[line_id].updt_ipv4)
							up_ipv4_checksum = 1;
						if (pattern_jump.updt_tcp != entry_p->line[line_id].updt_tcp)
							up_tcp_udp_checksum = 1;
						if (pattern_jump.last != entry_p->line[line_id].last)
							last = 1;
					} else {
						if (pattern_chain.data != entry_p->line[line_id].data)
							modification_data = 1;
						if (pattern_chain.opcode != entry_p->line[line_id].opcode)
							mod_command = 1;
						if (pattern_chain.updt_ipv4 != entry_p->line[line_id].updt_ipv4)
							up_ipv4_checksum = 1;
						if (pattern_chain.updt_tcp != entry_p->line[line_id].updt_tcp)
							up_tcp_udp_checksum = 1;
						if (pattern_chain.last != entry_p->line[line_id].last)
							last = 1;
					}

					/*show SW error info*/
					if (modification_data || mod_command || up_ipv4_checksum ||
									up_tcp_udp_checksum || last) {
						sw_error_count++;
						printk("\nPMT Mismatch: [set %d] area_type %d, chain_type %d, "
							"chain_index %d ,line_num %d\r\n",
							set_id, entry_p->area_type, entry_p->chain_type,
							entry_p->index, entry_p->line_num);
						tpm_check_pmt_mismatch_show_header();
						printk(" %5s      %03d      0x%2x %15s  0x%4.4x  "
							"%1d     %1d      %1d\n",
							"Build",
							line_id,
							entry_p->line[line_id].opcode,
							opCodeOperationStr[entry_p->line[line_id].opcode],
							entry_p->line[line_id].data, entry_p->line[line_id].last,
							entry_p->line[line_id].updt_ipv4,
							entry_p->line[line_id].updt_tcp);
						if (set_id == 0) {
							printk(" %5s      %03d      0x%2x %15s  0x%4.4x  "
								"%1d     %1d      %1d\n",
								"DB",
								line_id,
								pattern_jump.opcode,
								opCodeOperationStr[pattern_jump.opcode],
								pattern_jump.data, pattern_jump.last,
								pattern_jump.updt_ipv4, pattern_jump.updt_tcp);
						} else {

							printk(" %5s      %03d      0x%2x %15s  0x%4.4x  "
								"%1d     %1d      %1d\n",
								"DB",
								line_id,
								pattern_chain.opcode,
								opCodeOperationStr[pattern_chain.opcode],
								pattern_chain.data, pattern_chain.last,
								pattern_chain.updt_ipv4, pattern_chain.updt_tcp);
						}
					}

					/*clear error flags*/
					modification_data = 0;
					mod_command = 0;
					up_ipv4_checksum = 0;
					up_tcp_udp_checksum = 0;
					last = 0;
				}
			}

		}
	}

	/*record check result*/
	if (hw_error_count)
		*result = ERR_TPMCHECK_PMT_HW_MISMATCH;
	else if (sw_error_count)
		*result = ERR_TPMCHECK_PMT_DB_MISMATCH;
	else
		*result = TPM_RC_OK;

	return TPM_OK;
}

int32_t tpm_check_param_check(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
			      tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
			      tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
			      tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	if (pon_type < TPM_EPON || pon_type > TPM_P2P) {
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "PON type is invalid\n");
		return TPM_FAIL;
	}

	if (tpm_rule == NULL ||
		mod_con == NULL ||
		api_ent_mem_area == NULL ||
		pnc_data == NULL ||
		mod_entry == NULL ||
		pkt_mod == NULL ||
		pkt_mod_bm == NULL ||
		int_mod_bm == NULL ||
		trg_port == NULL ||
		rule_action == NULL) {
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Pointer invalid (NULL)\n");
		return TPM_FAIL;
	}

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_l2 - translate L2 APi data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	trg_port
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_l2(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
			     tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
			     tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
			     tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_l2_acl_key_t l2_key;
	tpm_dir_t dir = 0;
	tpm_pkt_frwd_t pkt_frwd;
	uint32_t rule_num;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	src_port = tpm_rule->l2_prim_key.src_port;
	memcpy(&l2_key, &(tpm_rule->l2_prim_key.l2_key), sizeof(tpm_l2_acl_key_t));
	parse_rule_bm = tpm_rule->l2_prim_key.parse_rule_bm;
	parse_flags_bm = tpm_rule->l2_prim_key.parse_flags_bm;
	memcpy(&pkt_frwd, &(tpm_rule->l2_prim_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(tpm_rule->l2_prim_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	*trg_port = tpm_rule->l2_prim_key.pkt_frwd.trg_port;
	memcpy(pkt_mod, &(tpm_rule->l2_prim_key.pkt_mod), sizeof(tpm_pkt_mod_t));
	*pkt_mod_bm = tpm_rule->l2_prim_key.pkt_mod_bm;
	*int_mod_bm = 0;
	rule_num = api_ent_mem_area->rule_idx;
	/* Get direction */
	tpm_ret = tpm_proc_src_port_dir_map(src_port, &dir);
	IF_ERROR(tpm_ret);
	/*******************generate PnC Rule Start********************/
	/*generate tcam */
	tpm_ret = tpm_proc_l2_tcam_build(src_port,
					 dir,
					 rule_num,
					 &l2_key,
					 parse_rule_bm,
					 parse_flags_bm,
					 &(pnc_data->pncl_tcam));
	IF_ERROR(tpm_ret);
	/*generate sram*/
	tpm_ret = tpm_proc_l2_sram_build(src_port,
					 dir,
					 rule_num,
					 pon_type,
					 &pkt_frwd,
					 rule_action,
					 mod_con->mod_cmd_ind,
					 &(pnc_data->pncl_sram),
					 &l2_key,
					 &(tpm_rule->l2_prim_key.pkt_mod),
					 tpm_rule->l2_prim_key.pkt_mod_bm);
	IF_ERROR(tpm_ret);

	return TPM_OK;

}

/*******************************************************************************
* api_data2pnc_data_l3 - translate L3 APi data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_l3(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
			     tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
			     tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
			     tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_l3_type_key_t l3_key;
	tpm_dir_t dir = 0;
	tpm_pkt_frwd_t pkt_frwd;
	uint8_t l3_is_pppoe = TPM_FALSE;
	uint32_t rule_num;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	src_port = tpm_rule->l3_type_key.src_port;
	memcpy(&l3_key, &(tpm_rule->l3_type_key.l3_key), sizeof(tpm_l3_type_key_t));
	parse_rule_bm = tpm_rule->l3_type_key.parse_rule_bm;
	parse_flags_bm = tpm_rule->l3_type_key.parse_flags_bm;
	memcpy(&pkt_frwd, &(tpm_rule->l3_type_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(tpm_rule->l3_type_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	rule_num = api_ent_mem_area->rule_idx;
	/* Get direction */
	tpm_ret = tpm_proc_src_port_dir_map(src_port, &dir);
	IF_ERROR(tpm_ret);
	/*******************generate PnC Rule Start********************/
	/*generate tcam */
	tpm_ret = tpm_proc_l3_tcam_build(src_port,
					 dir,
					 rule_num,
					 &l3_key,
					 parse_rule_bm,
					 parse_flags_bm,
					 rule_action,
					 &(pnc_data->pncl_tcam));
	IF_ERROR(tpm_ret);
	/*generate sram*/
	if (pnc_data->pncl_tcam.pkt_key.l3_key.ether_type_key == ETH_P_PPP_SES)
		l3_is_pppoe = TPM_TRUE;
	tpm_ret = tpm_proc_l3_sram_build(src_port,
					 dir,
					 rule_num,
					 l3_is_pppoe,
					 pon_type,
					 &pkt_frwd,
					 rule_action,
					 parse_flags_bm,
					 &(pnc_data->pncl_sram));
	IF_ERROR(tpm_ret);

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv4 - translate IPV4 ACL API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv4(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
			       tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
			       tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
			       tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_ipv4_acl_key_t ipv4_acl_key;
	tpm_dir_t dir = 0;
	tpm_pkt_frwd_t pkt_frwd;
	uint32_t rule_num;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	src_port = tpm_rule->ipv4_key.src_port;
	memcpy(&ipv4_acl_key, &(tpm_rule->ipv4_key.ipv4_key), sizeof(tpm_ipv4_acl_key_t));
	parse_rule_bm = tpm_rule->ipv4_key.parse_rule_bm;
	parse_flags_bm = tpm_rule->ipv4_key.parse_flags_bm;
	memcpy(&pkt_frwd, &(tpm_rule->ipv4_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(tpm_rule->ipv4_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	*trg_port = tpm_rule->ipv4_key.pkt_frwd.trg_port;
	memcpy(pkt_mod, &(tpm_rule->ipv4_key.pkt_mod), sizeof(tpm_pkt_mod_t));
	*pkt_mod_bm = tpm_rule->ipv4_key.pkt_mod_bm;
	*int_mod_bm = 0;
	rule_num = api_ent_mem_area->rule_idx;
	/* Get direction */
	tpm_ret = tpm_proc_src_port_dir_map(src_port, &dir);
	IF_ERROR(tpm_ret);
	/*******************generate PnC Rule Start********************/
	/*generate tcam */
	tpm_ret = tpm_proc_ipv4_tcam_build(src_port,
					   dir,
					   rule_num,
					   &ipv4_acl_key,
					   parse_rule_bm,
					   parse_flags_bm,
					   rule_action,
					   &(tpm_rule->ipv4_key.pkt_mod),
					   tpm_rule->ipv4_key.pkt_mod_bm,
					   &(pnc_data->pncl_tcam));
	IF_ERROR(tpm_ret);

	/*generate sram*/
	tpm_ret = tpm_proc_ipv4_sram_build(src_port,
					   dir,
					   rule_num,
					   pon_type,
					   &ipv4_acl_key,
					   pnc_data->pncl_tcam.ipv4_parse_bm,
					   parse_flags_bm,
					   &pkt_frwd,
					   rule_action,
					   mod_con->mod_cmd_ind,
					   tpm_rule->ipv4_key.pkt_mod_bm,
					   &(tpm_rule->ipv4_key.pkt_mod),
					   &(pnc_data->pncl_sram));
	IF_ERROR(tpm_ret);
	/*update L4 checksum*/
	if (parse_rule_bm & TPM_IPv4_PARSE_PROTO) {
		if (ipv4_acl_key.ipv4_proto == IPPROTO_TCP)
			*int_mod_bm = TPM_INT_L4_TCP;
		else if (ipv4_acl_key.ipv4_proto == IPPROTO_UDP) {
			*int_mod_bm = TPM_INT_L4_UDP;
			/* Set internal_bm according to API Action */
			if (rule_action->pkt_act & TPM_ACTION_UDP_CHKSUM_CALC)
				*int_mod_bm = TPM_INT_L4_UDP | TPM_INT_UDP_CHECKSUM;
		}
	}

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv4_mc - translate IPV4 MC API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv4_mc(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				  tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				  tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				  tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_db_mc_stream_entry_t mc_stream_entry;
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_stream_pppoe, ignore_ipv4_src, ipv4_src_add[4], ipv4_dst_add[4];
	uint16_t mc_vid;
	uint16_t dest_queue;
	uint32_t stream_num, igmp_mode, dest_port_bm;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	filter_mode = tpm_db_get_mc_filter_mode();
	stream_num = api_ent_mem_area->rule_num;
	tpm_ret = tpm_db_get_mc_stream_entry(stream_num, &mc_stream_entry);
	IF_ERROR(tpm_ret);
	mc_stream_pppoe = mc_stream_entry.mc_stream_pppoe;
	igmp_mode = mc_stream_entry.igmp_mode;
	dest_port_bm = mc_stream_entry.dest_port_bm;
	memcpy(ipv4_src_add, tpm_rule->ipv4_mc_key.ipv4_src_add, sizeof(ipv4_src_add));
	memcpy(ipv4_dst_add, tpm_rule->ipv4_mc_key.ipv4_dest_add, sizeof(ipv4_dst_add));
	rule_action->pkt_act = 0;
	mc_vid = tpm_rule->ipv4_mc_key.vid;
	ignore_ipv4_src = tpm_rule->ipv4_mc_key.ignore_ipv4_src;
	*mod_entry = mod_con->mod_cmd_ind;
	dest_queue = tpm_rule->ipv4_mc_key.dest_queue;

	/*******************generate PnC Rule Start********************/
	/*generate tcam entry*/
	tpm_ret = tpm_proc_ipv4_mc_tcam_build(filter_mode,
					      stream_num,
					      mc_vid,
					      mc_stream_pppoe,
					      ipv4_src_add,
					      ipv4_dst_add,
					      ignore_ipv4_src,
					      &(pnc_data->pncl_tcam));
	IF_ERROR(tpm_ret);
	/* Build SRAM Entry */
	tpm_ret = tpm_proc_ipvx_mc_sram_build(filter_mode,
					      igmp_mode,
					      dest_queue,
					      dest_port_bm,
					      mod_con->mod_cmd_ind,
					      &(pnc_data->pncl_sram),
					      TPM_IP_VER_4 );
	IF_ERROR(tpm_ret);

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv6_gen - translate IPV6 GEN API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv6_gen(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				   tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				   tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				   tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_dir_t dir = 0;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_rule_ipv6_gen_key_t ipv6_gen_acl_key;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable = TPM_IPV6_5T_DISABLED;
	long long int_pkt_act = 0;
	uint32_t subflow_bm, subflow_id, protocol = 0;
	uint32_t rule_num;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	src_port = tpm_rule->ipv6_gen_key.src_port;
	memcpy(&ipv6_gen_acl_key, &(tpm_rule->ipv6_gen_key), sizeof(tpm_rule_ipv6_gen_key_t));
	parse_rule_bm = tpm_rule->ipv6_gen_key.parse_rule_bm;
	parse_flags_bm = tpm_rule->ipv6_gen_key.parse_flags_bm;
	memcpy(&pkt_frwd, &(tpm_rule->ipv6_gen_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(tpm_rule->ipv6_gen_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	*trg_port = tpm_rule->ipv6_gen_key.pkt_frwd.trg_port;
	memcpy(pkt_mod, &(tpm_rule->ipv6_gen_key.pkt_mod), sizeof(tpm_pkt_mod_t));
	*pkt_mod_bm = tpm_rule->ipv6_gen_key.pkt_mod_bm;
	*int_mod_bm = 0;
	rule_num = api_ent_mem_area->rule_idx;
	/*check ipv6_5t is enable or not*/
	tpm_ret = tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	IF_ERROR(tpm_ret);
	/* Get direction */
	tpm_ret = tpm_proc_src_port_dir_map(src_port, &dir);
	IF_ERROR(tpm_ret);

	/*******************generate PnC Rule Start********************/
	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED) {
		/*generate tcam */
		tpm_ret = tpm_proc_ipv6_gen_tcam_build(src_port,
						       dir,
						       rule_num,
						       &ipv6_gen_acl_key.ipv6_gen_key,
						       parse_rule_bm,
						       parse_flags_bm,
						       pkt_mod,
						       *pkt_mod_bm,
						       &(pnc_data->pncl_tcam));
		IF_ERROR(tpm_ret);
		/*generate sram*/
		tpm_ret = tpm_proc_ipv6_gen_sram_build(src_port,
						       dir,
						       rule_num,
						       pon_type,
						       &pkt_frwd,
						       rule_action,
						       mod_con->mod_cmd_ind,
						       &(pnc_data->pncl_sram));
		IF_ERROR(tpm_ret);
	} else {
		/*********** Get Subflow Info **********/
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
			subflow_id = tpm_db_find_matched_ipv6_gen_subflow(dir,
									  subflow_bm,
									  protocol,
									  &ipv6_gen_acl_key.l4_key,
									  &ipv6_gen_acl_key.ipv6_gen_key);
			if (subflow_id == TPM_DB_INVALID_IPV6_FLOW_ID) {
				printk("Subflow_id get Failed\n");
				return TPM_FAIL;
			}
		} else {
			int_pkt_act |= TPM_ACTION_UNSET_IPV6_SUBFLOW;
		}
		/*generate tcam */
		tpm_ret = tpm_proc_ipv6_gen_5t_tcam_build(dir,
							  rule_num,
							  &ipv6_gen_acl_key.l4_key,
							  &ipv6_gen_acl_key.ipv6_gen_key,
							  parse_rule_bm,
							  parse_flags_bm,
							  rule_action,
							  pkt_mod,
							  *pkt_mod_bm,
							  &(pnc_data->pncl_tcam));
		IF_ERROR(tpm_ret);
		/*generate sram*/
		tpm_ret = tpm_proc_ipv6_gen_5t_sram_build(dir,
							  rule_num,
							  pon_type,
							  subflow_id,
							  &pkt_frwd,
							  rule_action,
							  int_pkt_act,
							  pkt_mod,
							  mod_con->mod_cmd_ind,
							  &(pnc_data->pncl_sram));
		IF_ERROR(tpm_ret);
	}

	return TPM_OK;
}


/*******************************************************************************
* api_data2pnc_data_ipv6_dip - translate IPV6 DIP API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv6_dip(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				   tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				   tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				   tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_dir_t dir = 0;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_rule_ipv6_gen_key_t ipv6_gen_acl_key;
	tpm_rule_ipv6_dip_key_t ipv6_dip_key;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable = TPM_IPV6_5T_DISABLED;
	long long int_pkt_act = 0;
	uint32_t rule_num;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	src_port = tpm_rule->ipv6_dip_key.src_port;
	memcpy(&ipv6_dip_key, &(tpm_rule->ipv6_dip_key), sizeof(tpm_rule_ipv6_dip_key_t));
	parse_rule_bm = tpm_rule->ipv6_dip_key.parse_rule_bm;
	parse_flags_bm = tpm_rule->ipv6_dip_key.parse_flags_bm;
	memcpy(&pkt_frwd, &(tpm_rule->ipv6_dip_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(tpm_rule->ipv6_dip_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	*trg_port = tpm_rule->ipv6_dip_key.pkt_frwd.trg_port;
	memcpy(pkt_mod, &(tpm_rule->ipv6_dip_key.pkt_mod), sizeof(tpm_pkt_mod_t));
	*pkt_mod_bm = tpm_rule->ipv6_dip_key.pkt_mod_bm;
	*int_mod_bm = 0;
	rule_num = api_ent_mem_area->rule_idx;
	/*check ipv6_5t is enable or not*/
	tpm_ret = tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	IF_ERROR(tpm_ret);
	/* Get direction */
	tpm_ret = tpm_proc_src_port_dir_map(src_port, &dir);
	IF_ERROR(tpm_ret);
	/*******************generate PnC Rule Start********************/
	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED) {
		/*generate tcam */
		tpm_ret = tpm_proc_ipv6_dip_tcam_build(src_port,
						       dir,
						       rule_num,
						       &ipv6_dip_key.ipv6_dipkey,
						       parse_rule_bm,
						       parse_flags_bm,
						       pkt_mod,
						       *pkt_mod_bm,
						       &(pnc_data->pncl_tcam));
		IF_ERROR(tpm_ret);
		/*generate sram*/
		tpm_ret = tpm_proc_ipv6_dip_sram_build(src_port,
						       dir,
						       rule_num,
						       pon_type,
						       &pkt_frwd,
						       rule_action,
						       mod_con->mod_cmd_ind,
						       &(pnc_data->pncl_sram));
		IF_ERROR(tpm_ret);
	} else {
		int_pkt_act |= TPM_ACTION_UNSET_IPV6_SUBFLOW;
		/*generate tcam */
		memcpy(&ipv6_gen_acl_key, &(tpm_rule->ipv6_gen_key), sizeof(tpm_rule_ipv6_gen_key_t));
		tpm_ret = tpm_proc_ipv6_dip_5t_tcam_build(dir,
							  rule_num,
							  &ipv6_dip_key.l4_key,
							  &ipv6_gen_acl_key.ipv6_gen_key,
							  &ipv6_dip_key.ipv6_dipkey,
							  parse_rule_bm,
							  parse_flags_bm,
							  rule_action,
							  pkt_mod,
							  *pkt_mod_bm,
							  &(pnc_data->pncl_tcam));
		IF_ERROR(tpm_ret);
		/*generate sram*/
		tpm_ret = tpm_proc_ipv6_dip_5t_sram_build(dir,
							  rule_num,
							  pon_type,
							  &pkt_frwd,
							  rule_action,
							  int_pkt_act,
							  pkt_mod,
							  mod_con->mod_cmd_ind,
							  &(pnc_data->pncl_sram));
		IF_ERROR(tpm_ret);
	}

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv6_mc - translate IPV6 MC API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv6_mc(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				  tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				  tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				  tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_mc_filter_mode_t filter_mode;
	uint8_t mc_stream_pppoe;
	uint16_t mc_vid;
	uint32_t stream_num, igmp_mode, dest_port_bm;
	tpm_db_ipv6_mc_stream_entry_t ipv6_mc_stream_entry;
	uint8_t ipv6_dst_add[16];
	int32_t tpm_ret = 0;
	uint8_t sip_index = 0;
	uint16_t dest_queue;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	filter_mode = tpm_db_get_mc_filter_mode();
	stream_num = api_ent_mem_area->rule_num;
	tpm_ret = tpm_db_get_ipv6_mc_stream_entry(stream_num, &ipv6_mc_stream_entry);
	IF_ERROR(tpm_ret);
	mc_stream_pppoe = ipv6_mc_stream_entry.mc_stream_pppoe;
	igmp_mode = ipv6_mc_stream_entry.igmp_mode;
	dest_port_bm = ipv6_mc_stream_entry.dest_port_bm;
	memcpy(ipv6_dst_add, tpm_rule->ipv6_mc_key.ipv6_dest_add, sizeof(ipv6_dst_add));
	rule_action->pkt_act = 0;
	mc_vid = tpm_rule->ipv6_mc_key.vid;
	*mod_entry = mod_con->mod_cmd_ind;
	dest_queue = tpm_rule->ipv6_mc_key.dest_queue;

	/* get sip_index */
	if (0 == ipv6_mc_stream_entry.ignore_src_addr) {
		/* get index of this IPv6 MC SIP */
		sip_index = tpm_db_ipv6_mc_sip_index_get(ipv6_mc_stream_entry.src_addr);
		if (0 == sip_index) {
			/* SIP is not in DB, error */
			TPM_OS_ERROR(TPM_TPM_LOG_MOD, "get index of IPv6 MC SIP failed!\n");
			return(ERR_IPV6_MC_SRC_IP_INVALID);
		}
	}

	/*generate tcam entry*/
	tpm_ret = tpm_proc_ipv6_mc_tcam_build(filter_mode,
					      stream_num,
					      mc_vid,
					      mc_stream_pppoe,
					      ipv6_dst_add,
					      sip_index,
					      ipv6_mc_stream_entry.ignore_src_addr,
					      &(pnc_data->pncl_tcam));
	IF_ERROR(tpm_ret);
	/*generate sram*/
	tpm_ret = tpm_proc_ipvx_mc_sram_build(filter_mode,
					      igmp_mode,
					      dest_queue,
					      dest_port_bm,
					      mod_con->mod_cmd_ind,
					      &(pnc_data->pncl_sram),
					      TPM_IP_VER_6);
	IF_ERROR(tpm_ret);

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv6_nh - translate IPV6 NH API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv6_nh(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				  tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				  tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				  tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_rule_ipv6_nh_key_t ipv6_nh_key;
	tpm_nh_iter_t nh_iter;
	uint32_t rule_num;
	uint32_t nh;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	memcpy(&ipv6_nh_key, &(tpm_rule->ipv6_nh_key), sizeof(tpm_rule_ipv6_nh_key_t));
	parse_rule_bm = ipv6_nh_key.parse_rule_bm;
	parse_flags_bm = ipv6_nh_key.parse_flags_bm;
	nh = ipv6_nh_key.nh;
	nh_iter = ipv6_nh_key.nh_iter;
	memcpy(&pkt_frwd, &(ipv6_nh_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(ipv6_nh_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	rule_num = api_ent_mem_area->rule_idx;

	/*******************generate PnC Rule Start********************/
	/*generate tcam */
	tpm_ret = tpm_proc_ipv6_nh_tcam_build(rule_num,
					      nh_iter,
					      nh,
					      parse_flags_bm,
					      rule_action,
					      &(pnc_data->pncl_tcam));
	IF_ERROR(tpm_ret);
	/*generate sram*/
	tpm_ret = tpm_proc_ipv6_nh_sram_build(rule_num,
					      nh_iter,
					      nh,
					      pon_type,
					      &pkt_frwd,
					      rule_action,
					      &(pnc_data->pncl_sram));
	IF_ERROR(tpm_ret);

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv6_l4 - translate IPV6 L4 API data to PNC data
*
* INPUT:
*       pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv6_l4(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				  tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				  tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				  tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_dir_t dir = 0;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_rule_ipv6_l4_key_t ipv6_l4_key;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable = TPM_IPV6_5T_DISABLED;
	long long int_pkt_act = 0;
	uint32_t subflow_bm, subflow_id, protocol = 0;
	uint32_t rule_num;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	src_port = tpm_rule->ipv6_l4_key.src_port;
	memcpy(&ipv6_l4_key, &(tpm_rule->ipv6_l4_key), sizeof(tpm_rule_ipv6_l4_key_t));
	parse_rule_bm = tpm_rule->ipv6_l4_key.parse_rule_bm;
	parse_flags_bm = tpm_rule->ipv6_l4_key.parse_flags_bm;
	memcpy(&pkt_frwd, &(tpm_rule->ipv6_l4_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(rule_action, &(tpm_rule->ipv6_l4_key.rule_action), sizeof(tpm_rule_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	*trg_port = tpm_rule->ipv6_l4_key.pkt_frwd.trg_port;
	memcpy(pkt_mod, &(tpm_rule->ipv6_l4_key.pkt_mod), sizeof(tpm_pkt_mod_t));
	*pkt_mod_bm = tpm_rule->ipv6_l4_key.pkt_mod_bm;
	*int_mod_bm = 0;
	rule_num = api_ent_mem_area->rule_idx;

	/*check ipv6_5t is enable or not*/
	tpm_ret = tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	IF_ERROR(tpm_ret);

	/* Get direction */
	tpm_ret = tpm_proc_src_port_dir_map(src_port, &dir);
	IF_ERROR(tpm_ret);

	if (ipv6_5t_enable == TPM_IPV6_5T_DISABLED) {
		/*generate tcam */
		tpm_ret = tpm_proc_ipv6_l4ports_tcam_build(src_port,
							   dir,
							   rule_num,
							   &ipv6_l4_key.l4_key,
							   parse_rule_bm,
							   parse_flags_bm,
							   rule_action,
							   pkt_mod,
							   *pkt_mod_bm,
							   &(pnc_data->pncl_tcam));
		IF_ERROR(tpm_ret);

		/*generate sram*/
		tpm_ret = tpm_proc_ipv6_l4ports_sram_build(src_port,
							   dir,
							   rule_num,
							   pon_type,
							   &pkt_frwd,
							   rule_action,
							   mod_con->mod_cmd_ind,
							   &(pnc_data->pncl_sram));
		IF_ERROR(tpm_ret);
	} else {
		/*********** Get Subflow Info **********/
		subflow_bm = parse_rule_bm & TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK;
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
			subflow_id = tpm_db_find_matched_ipv6_l4_subflow(dir,
									 subflow_bm,
									 protocol,
									 &ipv6_l4_key.l4_key);
			if (subflow_id == TPM_DB_INVALID_IPV6_FLOW_ID) {
				printk("Subflow_id get Failed\n");
				return TPM_FAIL;
			}
		} else {
			int_pkt_act |= TPM_ACTION_UNSET_IPV6_SUBFLOW;
		}
		/*generate tcam */
		tpm_ret = tpm_proc_ipv6_l4_ports_5t_tcam_build(dir,
							       rule_num,
							       &ipv6_l4_key.l4_key,
							       parse_rule_bm,
							       parse_flags_bm,
							       rule_action,
							       pkt_mod,
							       *pkt_mod_bm,
							       &(pnc_data->pncl_tcam));
		IF_ERROR(tpm_ret);

		tpm_ret = tpm_proc_ipv6_l4_ports_5t_sram_build(dir,
							       rule_num,
							       pon_type,
							       subflow_id,
							       &pkt_frwd,
							       rule_action,
							       int_pkt_act,
							       pkt_mod,
							       mod_con->mod_cmd_ind,
							       &(pnc_data->pncl_sram));
		IF_ERROR(tpm_ret);
	}

	return TPM_OK;
}

/*******************************************************************************
* api_data2pnc_data_ipv4_cnm - translate IPV4 CnM API data to PNC data
*
* INPUT:
*	pon_type
*	tpm_rule
*	mod_con
*	api_ent_mem_area
*
* OUTPUT:
*	pnc_data
*	mod_entry
*	pkt_mod
*	pkt_mod_bm
*	int_mod_bm
*	rule_action
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t api_data2pnc_data_ipv4_cnm(tpm_db_pon_type_t pon_type, tpm_rule_entry_t *tpm_rule, tpm_db_mod_conn_t *mod_con,
				   tpm_db_api_entry_t *api_ent_mem_area, tpm_pncl_pnc_full_t *pnc_data, uint32_t *mod_entry,
				   tpm_pkt_mod_t *pkt_mod, tpm_pkt_mod_bm_t *pkt_mod_bm, tpm_pkt_mod_int_bm_t *int_mod_bm,
				   tpm_trg_port_type_t *trg_port, tpm_rule_action_t *rule_action)
{
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t l2_parse_rule_bm;
	tpm_parse_fields_t ipv4_parse_rule_bm;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_l2_acl_key_t l2_key;
	tpm_ipv4_acl_key_t ipv4_key;
	tpm_pkt_action_t pkt_act;
	tpm_db_ctc_cm_rule_entry_t cnm_rule;
	uint32_t jump_idx = 0;
	uint32_t precedence = 0;
	uint32_t rule_num, pbits;
	uint32_t ipv4_pre_key_idx = TPM_CNM_INVALID_IPV4_PRE_FILTER_KEY_ID;
	int32_t tpm_ret = 0;

	/*params check*/
	tpm_ret = tpm_check_param_check(pon_type, tpm_rule, mod_con, api_ent_mem_area, pnc_data, mod_entry,
					pkt_mod, pkt_mod_bm, int_mod_bm, trg_port, rule_action);
	IF_ERROR(tpm_ret);

	/*get params from API db*/
	src_port = tpm_rule->cnm_key.src_port;
	ipv4_parse_rule_bm = tpm_rule->cnm_key.ipv4_parse_rule_bm;
	pkt_act = tpm_rule->cnm_key.pkt_act;
	pbits = tpm_rule->cnm_key.pbits;
	l2_parse_rule_bm = tpm_rule->cnm_key.l2_parse_rule_bm;
	memcpy(&l2_key, &(tpm_rule->cnm_key.l2_key), sizeof(tpm_l2_acl_key_t));
	memcpy(&ipv4_key, &(tpm_rule->cnm_key.ipv4_key), sizeof(tpm_ipv4_acl_key_t));
	memcpy(&pkt_frwd, &(tpm_rule->cnm_key.pkt_frwd), sizeof(tpm_pkt_frwd_t));
	memcpy(&rule_action->pkt_act, &(tpm_rule->cnm_key.pkt_act), sizeof(tpm_pkt_action_t));
	*mod_entry = mod_con->mod_cmd_ind;
	*trg_port = tpm_rule->cnm_key.pkt_frwd.trg_port;
	*pkt_mod_bm = 0;
	*int_mod_bm = 0;
	rule_num = api_ent_mem_area->rule_num;

	/*check whether CnM_MAIN_L2 or CnM_MAIN_IPv4*/
	/*get rule precedence*/
	tpm_ret = tpm_proc_calc_cnm_precedence(src_port, rule_num, &precedence);
	IF_ERROR(tpm_ret);
	/*get cnm rule*/
	tpm_ret = tpm_db_ctc_cm_rule_get(src_port, precedence, &cnm_rule);
	IF_ERROR(tpm_ret);
	/* Get split modification update */
	if (SET_MOD(pkt_act)) {
		if (tpm_db_split_mod_get_enable() == TPM_SPLIT_MOD_ENABLED) {
			/* get mod index from p-bit value */
			tpm_ret = tpm_db_split_mod_get_index_by_p_bits(pbits, &jump_idx);
			IF_ERROR(tpm_ret);
			/* skip first PMT for p-bit AS-IS */
			jump_idx++;
		} else {
			printk("split mod is not active\n");
			return TPM_FAIL;
		}
	}
	/*check l2_parse_rule_bm*/
	if (cnm_rule.l2_parse_rule_bm) {
		/*is CnM L2*/
		/*get ipv4 pre key index for comb rule if it is*/
		tpm_ret = tpm_db_find_ipv4_pre_filter_key(src_port,
							  cnm_rule.ipv4_parse_rule_bm,
							  &cnm_rule.ipv4_key,
							  &ipv4_pre_key_idx);
		/*rebuild tcam*/
		tpm_ret = tpm_proc_cnm_l2_tcam_build(src_port,
						     l2_parse_rule_bm,
						     &l2_key,
						     ipv4_pre_key_idx,
						     &pnc_data->pncl_tcam);
		IF_ERROR(tpm_ret);
		/*rebuild sram*/
		tpm_ret = tpm_proc_cnm_l2_sram_build(precedence,
						     &pkt_frwd,
						     pkt_act,
						     jump_idx,
						     &pnc_data->pncl_sram);
		IF_ERROR(tpm_ret);
	} else if ((cnm_rule.ipv4_parse_rule_bm) && (!cnm_rule.l2_parse_rule_bm)) {
		/*is CnM IPV4*/
		/*rebuild tcam*/
		tpm_ret = tpm_proc_cnm_ipv4_tcam_build(src_port,
						       precedence,
						       ipv4_parse_rule_bm,
						       &ipv4_key,
						       &pnc_data->pncl_tcam);
		IF_ERROR(tpm_ret);
		/*rebuild sram*/
		tpm_ret = tpm_proc_cnm_ipv4_sram_build(&pkt_frwd,
						       pkt_act,
						       jump_idx,
						       &pnc_data->pncl_sram);
		IF_ERROR(tpm_ret);
	} else {
		/*error*/
		printk("Invalid CnM parse rule bm!\n");
		tpm_ret = TPM_FAIL;
		IF_ERROR(tpm_ret);
	}

	return TPM_OK;
}

/*pnc rebuild functions array*/
static tpm_check_rebuild_pnc_t tpm_check_rebuild_pnc_array[TPM_MAX_NUM_API_SECTIONS] = {
								tpm_check_param_check,
								api_data2pnc_data_l2,
								api_data2pnc_data_l3,
								api_data2pnc_data_ipv4,
								api_data2pnc_data_ipv4_mc,
								api_data2pnc_data_ipv6_gen,
								api_data2pnc_data_ipv6_dip,
								api_data2pnc_data_ipv6_mc,
								api_data2pnc_data_ipv6_nh,
								api_data2pnc_data_ipv6_l4,
								api_data2pnc_data_ipv4_cnm};

/*******************************************************************************
* tpm_pmt_rebuild_and_check()
*
* DESCRIPTION: The API is to rebuild mod pattern data and check it with HW PMT and DB.
*
* INPUTS:   api_type    - the rule to check belong to
*           rule_index  - the rule index in rule's API range
*           trg_port    - targe port
*           pkt_mod_bm  - set of flags described which fields in the packet
*                         to be changed
*           int_mod_bm  - set of internal flags
*           mod_data    - modification entry data
*           mod_entry   - start entry number which has been set for this
*                         modification <j>
*
* OUTPUTS:
*           pmt_hw_error_count - check result for HW
*           pmt_sw_error_count - check result for SW
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_pmt_rebuild_and_check(tpm_api_type_t api_type,
				  uint32_t rule_index,
				  tpm_trg_port_type_t trg_port,
				  tpm_pkt_mod_bm_t pkt_mod_bm,
				  tpm_pkt_mod_int_bm_t int_mod_bm,
				  tpm_pkt_mod_t *pkt_mod,
				  uint32_t *mod_entry,
				  uint32_t *pmt_hw_error_count,
				  uint32_t *pmt_sw_error_count)
{
	tpm_gmacs_enum_t trg_gmac;
	tpm_mod_pattern_data_t pattern_data;
	tpm_main_chain_check_type_t main_chain_check;
	tpm_error_code_t pmt_check_result;
	int32_t tpm_ret = 0;

	tpm_ret = tpm_proc_trg_port_gmac_map(trg_port, &trg_gmac);
	IF_ERROR(tpm_ret);

	/*check main chain type*/
	if (tpm_check_main_chain_type(trg_gmac, pkt_mod_bm, int_mod_bm, *mod_entry,
				      &main_chain_check)) {
		printk("(Warn) main chain type check failed, API: %s, PnC index %d\n",
			api_type_to_str(api_type), rule_index);
	} else if (main_chain_check) {
		printk("main chain type mismatch, API: %s, PnC index %d\n",
			api_type_to_str(api_type), rule_index);
	}

	/*clear structure*/
	memset(&pattern_data, 0, sizeof(tpm_mod_pattern_data_t));

	/*build internal mod pattern data*/
	tpm_ret = tpm_mod2_fill_in_pattern(trg_gmac,
					   pkt_mod_bm,
					   int_mod_bm,
					   pkt_mod,
					   mod_entry,
					   &pattern_data,
					   TPM_CHECK_MAIN_CHAIN_IDX_DB);
	IF_ERROR(tpm_ret);
	/*check with DB*/
	if (tpm_pmt_check(trg_gmac, &pattern_data, &pmt_check_result,
			  TPM_CHECK_WITH_DB)) {
		printk("SW PMT check error, API type: %s, PnC rule index %d\n",
			api_type_to_str(api_type), rule_index);
	} else if (pmt_check_result) {
		*pmt_sw_error_count = *pmt_sw_error_count + 1;
		printk("SW PMT mismatch, API type: %s, PnC rule index %d\n",
			api_type_to_str(api_type), rule_index);
	}
	/*clear structure*/
	memset(&pattern_data, 0, sizeof(tpm_mod_pattern_data_t));

	/*build internal mod pattern data*/
	tpm_ret = tpm_mod2_fill_in_pattern(trg_gmac,
					   pkt_mod_bm,
					   int_mod_bm,
					   pkt_mod,
					   mod_entry,
					   &pattern_data,
					   TPM_CHECK_MAIN_CHAIN_IDX_HW);
	IF_ERROR(tpm_ret);
	/*check with HW*/
	if (tpm_pmt_check(trg_gmac, &pattern_data, &pmt_check_result,
			  TPM_CHECK_WITH_HW)) {
		printk("HW PMT check error, API type: %s, PnC rule index %d\n",
			api_type_to_str(api_type), rule_index);
	} else if (pmt_check_result) {
		*pmt_hw_error_count = *pmt_hw_error_count + 1;
		printk("HW PMT mismatch, API type: %s, PnC rule index %d\n",
			api_type_to_str(api_type), rule_index);
	}

	return TPM_OK;
}

/*******************************************************************************
* tpm_self_check - Check existed TPM rules and PMT table
*
* INPUT:
*       owner_id
*	check_level, 0--only check, 1--if there's error in HW table,
*					it will try to correct it, done later
*
* OUTPUT:
*	None
*
* RETURN:
* On success, the function returns TPM_RC_OK. On error different types are returned
* according to the case - see tpm_error_code_t.
*
* COMMENTS:
* It is APIs caller responsibility to maintain the correct number of each rule.
*******************************************************************************/
tpm_error_code_t tpm_self_check(uint32_t owner_id, tpm_self_check_level_enum_t check_level)
{
	tpm_api_type_t api_type;
	tpm_db_api_entry_t api_ent_mem_area;
	tpm_pncl_pnc_full_t pnc_data;
	tpm_pnc_all_t pnc_out;
	tpm_rule_entry_t tpm_rule;
	tpm_api_sections_t api_section;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_con;
	tpm_pkt_mod_int_bm_t int_mod_bm = 0;
	tpm_pkt_mod_bm_t pkt_mod_bm = 0;
	tpm_rule_action_t rule_action;
	tpm_db_pon_type_t pon_type = 0;
	struct tcam_entry hw_te;
	tpm_pnc_all_t read_pnc;
	tpm_pnc_all_t shadow_pnc;
	tpm_trg_port_type_t trg_port = 0;
	tpm_pkt_mod_t pkt_mod;
	tpm_vlan_key_t *vlan_out = NULL;
	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;
	uint8_t p_bits_tpm;
	uint32_t loop;
	uint32_t rule_num, rule_index, bi_dir;
	int32_t apisec_last_valid_index;
	uint32_t mod_entry = 0;
	uint32_t valid_entry_num = 0;
	uint32_t shadow_ent_valid;
	uint32_t hw_jump_idx, shadow_jump_idx;
	uint32_t p_bits_nums;
	/*error statisc*/
	uint32_t pnc_hw_error_count = 0;
	uint32_t pnc_sw_error_count = 0;
	uint32_t pmt_hw_error_count = 0;
	uint32_t pmt_sw_error_count = 0;
	uint32_t unnormal_end_count = 0;
	uint32_t section_valid_entry_count = 0;/*the count of api section with valid entry*/

	int32_t tpm_ret = 0;

	/* Get pon_type */
	if (tpm_db_pon_type_get(&pon_type))
		return ERR_GENERAL;
	/* get ctc cm enable status */
	if (tpm_db_ctc_cm_enable_get(&ctc_cm_enable))
		return ERR_GENERAL;

	for (api_type = TPM_API_L2_PRIM; api_type <= TPM_API_CNM; api_type++) {
		/*check whether CnM is enable or not*/
		if ((api_type == TPM_API_CNM) && (TPM_CTC_CM_DISABLED == ctc_cm_enable))
			continue;
		/* Get the api_section */
		tpm_ret = tpm_db_api_section_get_from_api_type(api_type, &api_section);
		if (tpm_ret != TPM_RC_OK) {
			printk("Get API_SECTION Failed-%s\n", api_type_to_str(api_type));
			continue;
		}

		valid_entry_num = 0;
		if (tpm_db_api_section_num_entries_get(api_section, &valid_entry_num))
			continue;

		if (valid_entry_num == 0)
			continue;

		section_valid_entry_count++;

		/*get the last valid rule index in the api_section*/
		if (tpm_db_api_section_lastentry_get(api_section, &apisec_last_valid_index))
			continue;

		/* Get the rulenum index */
		for (rule_index = 0; rule_index <= apisec_last_valid_index; rule_index++) {
			/*clear structure*/
			memset(&pnc_data, 0, sizeof(tpm_pncl_pnc_full_t));
			memset(&pnc_out, 0, sizeof(tpm_pnc_all_t));
			memset(&hw_te, 0, sizeof(struct tcam_entry));
			memset(&read_pnc, 0, sizeof(tpm_pnc_all_t));
			memset(&shadow_pnc, 0, sizeof(tpm_pnc_all_t));
			memset(&tpm_rule, 0, sizeof(tpm_rule_entry_t));
			memset(&rule_action, 0, sizeof(tpm_rule_action_t));
			memset(&mod_con, 0, sizeof(tpm_db_mod_conn_t));
			memset(&pnc_con, 0, sizeof(tpm_db_pnc_conn_t));
			memset(&api_ent_mem_area, 0, sizeof(tpm_db_api_entry_t));

			/*get api section table entry*/
			if (tpm_db_api_section_ent_tbl_get(api_section, &api_ent_mem_area, rule_index))
				break;

			if (api_ent_mem_area.valid == TPM_DB_VALID) {
				/*get tpm_rule*/
				memcpy(&tpm_rule, &(api_ent_mem_area.api_rule_data),
					sizeof(tpm_rule_entry_t));
				memcpy(&mod_con, &(api_ent_mem_area.mod_tbl_conn),
					sizeof(tpm_db_mod_conn_t));
				memcpy(&pnc_con, &(api_ent_mem_area.pnc_tbl_conn), sizeof(tpm_db_pnc_conn_t));
				bi_dir = api_ent_mem_area.bi_dir;
				rule_num = api_ent_mem_area.rule_idx;
				tpm_ret = tpm_check_rebuild_pnc_array[api_section](pon_type,
										   &tpm_rule,
										   &mod_con,
										   &api_ent_mem_area,
										   &pnc_data,
										   &mod_entry,
										   &pkt_mod,
										   &pkt_mod_bm,
										   &int_mod_bm,
										   &trg_port,
										   &rule_action);
				if (tpm_ret) {
					printk("API type: %s, the %d th PnC Rule Rebuild Failed\n",
						api_type_to_str(api_type), rule_index);
					unnormal_end_count++;
					break;
				}
				/* TCAM map to tpm_tcam_all_t of tpm_pnc_all_t */
				tpm_ret = tpm_pncl_tcam_map(&(pnc_data.pncl_tcam), &(pnc_out.tcam_entry));
				IF_ERROR(tpm_ret);

				/* SRAM  map to tpm_sram_all_t of tpm_pnc_all_t */
				tpm_ret = tpm_pncl_sram_map(&(pnc_data.pncl_sram),
							    &(pnc_out.sram_entry),
							    &(pnc_data.pncl_tcam));
				IF_ERROR(tpm_ret);

				/*read Pnc from HW table*/
				tpm_ret = tcam_hw_read(&hw_te, pnc_con.pnc_conn_tbl[0].pnc_index);
				IF_ERROR(tpm_ret);
				/*translate tcam_entry to tpm_pnc_all_t*/
				tran_tcam_entry_to_pnc_all_t(&read_pnc, &hw_te, &rule_action, api_section);

				/*read PnC from PnC shadow table*/
				tpm_ret = tpm_db_pnc_shdw_ent_get(pnc_con.pnc_conn_tbl[0].pnc_index,
								  &shadow_ent_valid,
								  &shadow_pnc);
				IF_ERROR(tpm_ret);

				/*check PNC*/
				tpm_check_update_sram(&pnc_out.sram_entry, &rule_action);
				tpm_check_update_sram(&shadow_pnc.sram_entry, &rule_action);
				if (memcmp(&pnc_out, &read_pnc, sizeof(tpm_pnc_all_t))) {
					printk("HW PNC mismatch, API type: %s, PnC rule index %d\n",
						api_type_to_str(api_type), rule_index);
					pnc_mismatch_info_show(&pnc_out, &read_pnc);
					pnc_hw_error_count++;
					/*correct the bad tpm rule, TODO done in future*/
					/*if (check_level == TPM_SELF_CHECK_LEVEL1)*/
				}
				if (memcmp(&pnc_out, &shadow_pnc, sizeof(tpm_pnc_all_t))) {
					printk("SW PNC mismatch, API type: %s, PnC rule index %d\n",
						api_type_to_str(api_type), rule_index);
					pnc_mismatch_info_show(&pnc_out, &shadow_pnc);
					pnc_sw_error_count++;
				}

				/*******************check PMT if necessary********************/
				/*check precondition: SET_MOD, jump_entry index is same in API DB, TPM DB and HW*/
				/*get HW JUPM Index*/
				hw_jump_idx = read_pnc.sram_entry.flowid_val & 0xFFF;
				/*get shadow JUPM Index*/
				shadow_jump_idx = shadow_pnc.sram_entry.flowid_val & 0xFFF;
				if (SET_MOD(rule_action.pkt_act) &&
				    (hw_jump_idx == shadow_jump_idx) &&
				    (hw_jump_idx == mod_entry)) {
					/*only modify VLAN and VLAN OP is NOOP, do not check*/
					if (TPM_VLAN_MOD == pkt_mod_bm && VLANOP_NOOP == pkt_mod.vlan_mod.vlan_op)
						continue;
					if ((TPM_API_L2_PRIM == api_type) &&
					    tpm_split_mod_stage1_check(pkt_mod_bm, &pkt_mod, &rule_action, false)) {
						int_mod_bm = TPM_INT_SPLIT_MOD;
						p_bits_nums = TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX;
						/* VLANOP_EXT_TAG_MOD_INS mod insert VLAN p_bit */
						if (VLANOP_EXT_TAG_MOD_INS == pkt_mod.vlan_mod.vlan_op)
							vlan_out = &pkt_mod.vlan_mod.vlan2_out;
						else
							vlan_out = &pkt_mod.vlan_mod.vlan1_out;
						/* AS-IS pmt check */
						vlan_out->pbit_mask = 0;
						mod_entry = mod_entry / 16;
						mod_entry *= 16;
						tpm_ret = tpm_pmt_rebuild_and_check(api_type,
										    rule_index,
										    trg_port,
										    pkt_mod_bm,
										    int_mod_bm,
										    &pkt_mod,
										    &mod_entry,
										    &pmt_hw_error_count,
										    &pmt_sw_error_count);
						IF_ERROR(tpm_ret);
						/* supported pbits pmt check */
						vlan_out->pbit_mask = 0xFF;
						for (loop = 0; loop < p_bits_nums; loop++) {
							tpm_ret = tpm_db_split_mod_get_p_bits_by_index(loop, &p_bits_tpm);
							IF_ERROR(tpm_ret);
							vlan_out->pbit = p_bits_tpm;
							mod_entry++;
							tpm_ret = tpm_pmt_rebuild_and_check(api_type,
											    rule_index,
											    trg_port,
											    pkt_mod_bm,
											    int_mod_bm,
											    &pkt_mod,
											    &mod_entry,
											    &pmt_hw_error_count,
											    &pmt_sw_error_count);
							IF_ERROR(tpm_ret);
						}
					} else {
						/* Normal PMT check */
						tpm_ret = tpm_pmt_rebuild_and_check(api_type,
										    rule_index,
										    trg_port,
										    pkt_mod_bm,
										    int_mod_bm,
										    &pkt_mod,
										    &mod_entry,
										    &pmt_hw_error_count,
										    &pmt_sw_error_count);
						IF_ERROR(tpm_ret);
					}
				}

			}
		}
	}

	if (section_valid_entry_count) {
		if (unnormal_end_count) {
			if ((pnc_hw_error_count == 0) && (pnc_sw_error_count == 0))
				printk("\nPartly PNC Check, OK\n");
			if ((pmt_hw_error_count == 0) && (pmt_sw_error_count == 0))
				printk("Partly PMT Check, OK\n");
		} else {
			if ((pnc_hw_error_count == 0) && (pnc_sw_error_count == 0))
				printk("\nPNC Check, OK\n");
			if ((pmt_hw_error_count == 0) && (pmt_sw_error_count == 0))
				printk("PMT Check, OK\n");
		}
	} else {
		printk("(Warn)No Valid Rule exist, Nothing to check\n");
	}

	return TPM_RC_OK;
}

