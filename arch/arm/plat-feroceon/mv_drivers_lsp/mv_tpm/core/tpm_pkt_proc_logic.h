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
* tpm_pkt_proc_logic.h
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
*               Revision: 1.3
*
*
*******************************************************************************/
#ifndef _TPM_PKT_PROCL_H_
#define _TPM_PKT_PROCL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/
#define MASK_AI(value)   (0x7f & (value))

#define AI_TO_MASK(off, size)   (((1<<size) - 1) << off)

/* Definitions for statically configured AI bits */

#define      TPM_AI_TAG1_BIT_OFF                (0)
#define      TPM_AI_TAG1_BIT_SIZE               (1)
#define      TPM_AI_TAG1_MASK                   (AI_TO_MASK(TPM_AI_TAG1_BIT_OFF, TPM_AI_TAG1_BIT_SIZE))

#define      TPM_AI_NH2_ITER_BIT_OFF            (0)
#define      TPM_AI_NH2_ITER_BIT_SIZE           (1)
#define      TPM_AI_NH2_ITER_MASK               (AI_TO_MASK(TPM_AI_NH2_ITER_BIT_OFF, TPM_AI_NH2_ITER_BIT_SIZE))

#define      TPM_AI_L4P_BIT_OFF                 (0)
#define      TPM_AI_L4P_BIT_SIZE                (1)
#define      TPM_AI_L4P_MASK                    (AI_TO_MASK(TPM_AI_L4P_BIT_OFF, TPM_AI_L4P_BIT_SIZE))

#define      TPM_AI_IPV6_SUBFLOW_PART1_BIT_OFF  (0)
#define      TPM_AI_IPV6_SUBFLOW_PART1_BIT_SIZE (TPM_NUM_IPV6_SUBFLOW_BITS_P1)
#define      TPM_AI_IPV6_SUBFLOW_PART1_MASK     (AI_TO_MASK(TPM_AI_IPV6_SUBFLOW_PART1_BIT_OFF, \
															TPM_AI_IPV6_SUBFLOW_PART1_BIT_SIZE))
#define      TPM_AI_IPV6_MC_SIP_PART1_BIT_OFF  (0)
#define      TPM_AI_IPV6_MC_SIP_PART1_BIT_SIZE (TPM_NUM_IPV6_MC_SIP_BITS_P1)
#define      TPM_AI_IPV6_MC_SIP_PART1_MASK     (AI_TO_MASK(TPM_AI_IPV6_MC_SIP_PART1_BIT_OFF, \
							   TPM_AI_IPV6_MC_SIP_PART1_BIT_SIZE))

#define      TPM_AI_CNM_IPV4_BIT_OFF            (0)
#define      TPM_AI_CNM_IPV4_BIT_SIZE           (1)
#define      TPM_AI_CNM_IPV4_MASK               (AI_TO_MASK(TPM_AI_CNM_IPV4_BIT_OFF, TPM_AI_CNM_IPV4_BIT_SIZE))

#define      TPM_AI_TAG2_BIT_OFF                (1)
#define      TPM_AI_TAG2_BIT_SIZE               (1)
#define      TPM_AI_TAG2_MASK                   (AI_TO_MASK(TPM_AI_TAG2_BIT_OFF, TPM_AI_TAG2_BIT_SIZE))

#define      TPM_AI_PPPOE_BIT_OFF               (1)
#define      TPM_AI_PPPOE_BIT_SIZE              (1)
#define      TPM_AI_PPPOE_MASK                  (AI_TO_MASK(TPM_AI_PPPOE_BIT_OFF, TPM_AI_PPPOE_BIT_SIZE))

#define      TPM_AI_CNM_IPV4_PRE_KEY_BIT_OFF    (1)
#define      TPM_AI_CNM_IPV4_PRE_KEY_BIT_SIZE   (3)
#define      TPM_AI_CNM_IPV4_PRE_KEY_MASK       (AI_TO_MASK(TPM_AI_CNM_IPV4_PRE_KEY_BIT_OFF, \
							    TPM_AI_CNM_IPV4_PRE_KEY_BIT_SIZE))

#define      TPM_AI_CNM_PREC_BIT_OFF            (1)
#define      TPM_AI_CNM_PREC_BIT_SIZE           (3)
#define      TPM_AI_CNM_PREC_MASK               (AI_TO_MASK(TPM_AI_CNM_PREC_BIT_OFF, TPM_AI_CNM_PREC_BIT_SIZE))

#define      TPM_AI_DNRT_BIT_OFF                (2)
#define      TPM_AI_DNRT_BIT_SIZE               (1)
#define      TPM_AI_DNRT_MASK                   (AI_TO_MASK(TPM_AI_DNRT_BIT_OFF, TPM_AI_DNRT_BIT_SIZE))

#define      TPM_AI_DNRT_DS_TRUNK_BIT_OFF       (3)
#define      TPM_AI_DNRT_DS_TRUNK_BIT_SIZE      (1)
#define      TPM_AI_DNRT_DS_TRUNK_MASK          (AI_TO_MASK(TPM_AI_DNRT_DS_TRUNK_BIT_OFF, TPM_AI_DNRT_DS_TRUNK_BIT_SIZE))

#define      TPM_AI_MTM_BIT_OFF                 (2)
#define      TPM_AI_MTM_BIT_SIZE                (1)
#define      TPM_AI_MTM_MASK                    (AI_TO_MASK(TPM_AI_MTM_BIT_OFF, TPM_AI_MTM_BIT_SIZE))

#define      TPM_AI_IPV6_MC_SIP_PART2_BIT_OFF   (2)
#define      TPM_AI_IPV6_MC_SIP_PART2_BIT_SIZE  (TPM_NUM_IPV6_MC_SIP_BITS_P2)
#define      TPM_AI_IPV6_MC_SIP_PART2_MASK      (AI_TO_MASK(TPM_AI_IPV6_MC_SIP_PART2_BIT_OFF, \
							    TPM_AI_IPV6_MC_SIP_PART2_BIT_SIZE))
#define      TPM_AI_TO_CPU_BIT_OFF              (3)
#define      TPM_AI_TO_CPU_BIT_SIZE             (1)
#define      TPM_AI_TO_CPU_MASK                 (AI_TO_MASK(TPM_AI_TO_CPU_BIT_OFF, TPM_AI_TO_CPU_BIT_SIZE))

#define      TPM_AI_SPLIT_MOD_BIT_OFF           (3)
#define      TPM_AI_SPLIT_MOD_BIT_SIZE          (1)
#define      TPM_AI_SPLIT_MOD_MASK              (AI_TO_MASK(TPM_AI_SPLIT_MOD_BIT_OFF, TPM_AI_SPLIT_MOD_BIT_SIZE))

#define      TPM_AI_UNI_BIT_OFF                 (4)
#define      TPM_AI_UNI_BIT_SIZE                (3)
#define      TPM_AI_UNI_MASK                    (AI_TO_MASK(TPM_AI_UNI_BIT_OFF, TPM_AI_UNI_BIT_SIZE))

#define      TPM_AI_MC_VID_BIT_OFF              (4)
#define      TPM_AI_MC_VID_BIT_SIZE             (TPM_NUM_MC_VID_BITS)
#define      TPM_AI_MC_VID_MASK                 (AI_TO_MASK(TPM_AI_MC_VID_BIT_OFF, TPM_AI_MC_VID_BIT_SIZE))

#define      TPM_AI_PPPOE_ADD_BIT_OFF           (4)
#define      TPM_AI_PPPOE_ADD_BIT_SIZE          (1)
#define      TPM_AI_PPPOE_ADD_MASK              (AI_TO_MASK(TPM_AI_PPPOE_ADD_BIT_OFF, TPM_AI_PPPOE_ADD_BIT_SIZE))

#define      TPM_AI_IPV6_SUBFLOW_PART2_BIT_OFF  (4)
#define      TPM_AI_IPV6_SUBFLOW_PART2_BIT_SIZE (TPM_NUM_IPV6_SUBFLOW_BITS_P2)
#define      TPM_AI_IPV6_SUBFLOW_PART2_MASK     (AI_TO_MASK(TPM_AI_IPV6_SUBFLOW_PART2_BIT_OFF, \
															TPM_AI_IPV6_SUBFLOW_PART2_BIT_SIZE))

#define      TPM_AI_NO_PROTO_BIT_OFF            (5)
#define      TPM_AI_NO_PROTO_BIT_SIZE           (1)
#define      TPM_AI_NO_PROTO_MASK               (AI_TO_MASK(TPM_AI_NO_PROTO_BIT_OFF, TPM_AI_NO_PROTO_BIT_SIZE))

#define      TPM_AI_NO_FRAG_BIT_OFF             (6)
#define      TPM_AI_NO_FRAG_BIT_SIZE            (1)
#define      TPM_AI_NO_FRAG_MASK                (AI_TO_MASK(TPM_AI_NO_FRAG_BIT_OFF, TPM_AI_NO_FRAG_BIT_SIZE))

#define      TPM_AI_MC_VID_VALID_BIT_OFF        (6)
#define      TPM_AI_MC_VID_VALID_BIT_SIZE       (1)
#define      TPM_AI_MC_VID_VALID_MASK           (AI_TO_MASK(TPM_AI_MC_VID_VALID_BIT_OFF, TPM_AI_MC_VID_VALID_BIT_SIZE))

/* End of  statically configured AI bits */

#define      TPM_MAX_AS_SES_TBL_SIZE           (128)

#define      TPM_TCP_FIN                       (0x1)
#define      TPM_TCP_RES                       (0x4)

#define     TPM_NH_RESERVED                    (0xFFFF)
#define     TPM_ETHERTYPE_DSA_TAG              (0xDABC)

#define PKT_DROP(pkt_act)           (pkt_act & TPM_ACTION_DROP_PK)
#define SET_TARGET_PORT(pkt_act)    (pkt_act & TPM_ACTION_SET_TARGET_PORT)
#define SET_TARGET_QUEUE(pkt_act)   (pkt_act & TPM_ACTION_SET_TARGET_QUEUE)
#define SET_CPU(pkt_act)            (pkt_act & TPM_ACTION_TO_CPU)
#define SET_MOD(pkt_act)            (pkt_act & TPM_ACTION_SET_PKT_MOD)
#define SET_CUST(pkt_act)           (pkt_act & TPM_ACTION_CUST_CPU_PKT_PARSE)
#define SET_UDP_CHKSUM_CALC(pkt_act) (pkt_act & TPM_ACTION_UDP_CHKSUM_CALC)

#define TO_PON(dir, trg, wan, act_wan)		\
	 (TRG_PON(trg) && (TPM_ENUM_PMAC == act_wan) && (dir == TPM_DIR_US) && \
	 (wan == TPM_EPON || wan == TPM_GPON || wan == TPM_P2P))
#define TO_GPON(dir, trg, wan, act_wan)	\
	(TRG_PON(trg) && (dir == TPM_DIR_US) && (wan == TPM_GPON) && (act_wan == TPM_ENUM_PMAC))
#define TO_EPON(dir, trg, wan, act_wan)	\
	(TRG_PON(trg) && (dir == TPM_DIR_US) && (wan == TPM_EPON) && (act_wan == TPM_ENUM_PMAC))
#define TO_ETHWAN(dir, trg, act_wan)		\
	((trg == TPM_TRG_PORT_WAN) && (dir == TPM_DIR_US) && ((act_wan == TPM_ENUM_GMAC_0) || (act_wan == TPM_ENUM_GMAC_1)))
#define TO_LAN(dir, trg)            (TRG_UNI(trg) && (dir == TPM_DIR_DS))
#define TO_CPU(trg)                 (TRG_CPU(trg))

/* TODO - TPM_TRG_UNI_4 should be from DB, not hardcoded */
#define TRG_UNI(trg)                (trg & \
				     (TPM_TRG_UNI_0|TPM_TRG_UNI_1|TPM_TRG_UNI_2|TPM_TRG_UNI_3|TPM_TRG_UNI_4| \
				      TPM_TRG_UNI_5|TPM_TRG_UNI_6|TPM_TRG_UNI_7|TPM_TRG_UNI_VIRT|TPM_TRG_PORT_UNI_ANY))
#define TRG_PON(trg)                (trg >= TPM_TRG_TCONT_0 && trg <= TPM_TRG_TCONT_7)
#define TRG_WAN(trg)                ((trg >= TPM_TRG_TCONT_0 && trg <= TPM_TRG_TCONT_7) || (trg == TPM_TRG_PORT_WAN))
#define TRG_CPU(trg)                (trg == TPM_TRG_PORT_CPU)

#define FROM_WAN(src)               ((src == TPM_SRC_PORT_WAN) || (src == TPM_SRC_PORT_WAN_OR_LAN))
#define FROM_SPEC_UNI(src)          (src >= TPM_SRC_PORT_UNI_0 && src <= TPM_SRC_PORT_UNI_VIRT)
#define FROM_LAN(src)               (src != TPM_SRC_PORT_WAN)

#define GMAC_IS_LAN(gmac_func)          ((gmac_func == TPM_GMAC_FUNC_LAN_AND_WAN) || (gmac_func == TPM_GMAC_FUNC_LAN))
#define GMAC_IS_WAN(gmac_func)          ((gmac_func == TPM_GMAC_FUNC_LAN_AND_WAN) || (gmac_func == TPM_GMAC_FUNC_WAN))
#define GMAC_IS_UNI_LAN(gmac_func)      (gmac_func == TPM_GMAC_FUNC_LAN_UNI)
#define GMAC_IS_UNI_VIRT(gmac_func)     (gmac_func == TPM_GMAC_FUNC_VIRT_UNI)
#define GMAC_IS_DS_UNI_LAN(gmac_func)   (gmac_func == TPM_GMAC_FUNC_US_MAC_LEARN_DS_LAN_UNI)


#define TPM_CNM_L2_MAIN_LUID_OFFSET      (0)
#define TPM_CNM_ETY_MAIN_LUID_OFFSET     (1)
#define TPM_CNM_IPV4_MAIN_LUID_OFFSET    (2)
#define TPM_CNM_IPV6_MAIN_LUID_OFFSET    (3)

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/

typedef enum {
	TTL_ZERO_DROP,
	TTL_ZERO_TO_CPU
} ttl_illegal_action_t;

typedef enum {
	TPM_ADD_DS_LOAD_BALANCE_RULE,
	TPM_ADD_L2_PRIM_ACL_RULE,
	TPM_ADD_L3_TYPE_ACL_RULE,
	TPM_ADD_IPV4_ACL_RULE,
	TPM_ADD_IPV6_NH_ACL_RULE,
	TPM_ADD_IPV6_GEN_ACL_RULE,
	TPM_ADD_IPV6_DIP_ACL_RULE,
	TPM_ADD_IPV6_L4_PORTS_ACL_RULE,
	TPM_ADD_IPV6_GEN_5T_RULE,
	TPM_ADD_IPV6_DIP_5T_RULE,
	TPM_ADD_IPV6_L4_PORTS_5T_RULE,
	TPM_NUM_AI_APIS
} tpm_pnc_apis;

/*************************************************************/
/*               STRUCTURES                                  */
/*************************************************************/

typedef struct {
	tpm_src_port_type_t src_port;
	uint32_t mc_vid_entry;
	uint32_t ipv6_subflow;
	uint32_t ipv4_pre_key;
	uint32_t cnm_prec;
	uint32_t ipv6_mc_sip_indx;
} tpm_ai_vectors_t;

typedef struct {
	tpm_pnc_apis api_num;
	tpm_parse_fields_t sup_parse_fields;
	tpm_parse_flags_t sup_parse_flags;
	tpm_pkt_action_t forbidden_actions;
} tpm_api_sup_param_val_t;

/* Structure for add mc vid */
typedef struct {
	uint32_t port_bm;
	uint32_t mc_vid;
} tpm_mc_vid_port_t;

typedef struct {
	uint32_t vid_num;
	tpm_mc_vid_port_t mc_vid_port[TPM_MAX_NUM_UNI_PORTS];
} tpm_mc_vid_t;

/* CNM related */
typedef int32_t (* check_cnm_ipv4_pre_filter_key_fn_t)(tpm_src_port_type_t src_port, uint32_t partner_key_bm, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
typedef int32_t (* add_cnm_ipv4_pre_filter_key_fn_t)(tpm_src_port_type_t src_port, uint32_t key_idx, uint32_t key_pattern, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
typedef int32_t (* del_cnm_ipv4_pre_filter_key_fn_t)(tpm_src_port_type_t src_port, uint32_t key_pattern, uint32_t key_idx);


typedef struct {
	uint32_t key_pattern;
	check_cnm_ipv4_pre_filter_key_fn_t check_cnm_ipv4_pre_filter;
	add_cnm_ipv4_pre_filter_key_fn_t add_cnm_ipv4_pre_filter;
	del_cnm_ipv4_pre_filter_key_fn_t del_cnm_ipv4_pre_filter;
} tpm_cnm_ipv4_pre_filter_rule_oper_t;

typedef tpm_error_code_t (*tpm_acl_recovery_func)(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);

typedef struct {
	tpm_api_type_t api_type;
	tpm_acl_recovery_func func;
} tpm_hot_swap_acl_recovery_t;


/* API's */

tpm_error_code_t tpm_proc_mib_reset(uint32_t owner_id, tpm_reset_level_enum_t reset_level);

uint32_t tpm_proc_get_cpu_lpbk_entry_num(void);

tpm_error_code_t tpm_proc_increase_cpu_lpbk_entry_num(void);

tpm_error_code_t tpm_proc_decrease_cpu_lpbk_entry_num(void);

tpm_error_code_t tpm_proc_check_cpu_wan_loopback_param(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd);

tpm_error_code_t tpm_proc_calc_flow_id(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd, uint16_t *flow_id);

int32_t tpm_proc_init_cpu_wan_loopback(void);

tpm_cpu_loopback_t *tpm_proc_get_loopback_entry(uint16_t flow_id);

tpm_error_code_t tpm_proc_add_cpu_loopback_rule(uint32_t owner_id,
						uint32_t rule_num,
						uint32_t *rule_idx,
						tpm_pkt_frwd_t *pkt_frwd);

tpm_error_code_t tpm_proc_add_cpu_wan_loopback(uint32_t owner_id,
					       tpm_pkt_frwd_t *pkt_frwd,
					       uint32_t *mod_idx);

tpm_error_code_t tpm_proc_del_cpu_wan_loopback(uint32_t owner_id, tpm_pkt_frwd_t *pkt_frwd);
tpm_error_code_t tpm_proc_add_ds_load_balance_acl_rule(uint32_t owner_id,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l2_acl_key_t *l2_key,
					       tpm_ds_load_balance_tgrt_t tgrt_port);
tpm_error_code_t tpm_proc_del_ds_load_balance_acl_rule(uint32_t owner_id, uint32_t rule_idx, uint32_t ext_call);

int32_t tpm_proc_l2_tcam_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       tpm_l2_acl_key_t *l2_key,
			       tpm_parse_fields_t parse_rule_bm,
			       tpm_parse_flags_t parse_flags_bm,
			       tpm_pncl_tcam_data_t *tcam_data);

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
			       tpm_pkt_mod_bm_t pkt_mod_bm);

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
					       tpm_rule_action_t *rule_action);

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
					    tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_add_l3_type_acl_rule(uint32_t owner_id,
					       tpm_src_port_type_t src_port,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_fields_t parse_rule_bm,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_l3_type_key_t *l3_key,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_get_next_valid_rule(uint32_t owner_id,
					      int32_t current_index,
					      tpm_api_type_t rule_type,
					      int32_t *next_index,
					      uint32_t *rule_idx,
					      tpm_rule_entry_t *tpm_rule);

tpm_error_code_t tpm_proc_omci_add_channel(uint32_t owner_id,
					   tpm_gem_port_key_t gem_port,
					   uint32_t cpu_rx_queue,
					   tpm_trg_port_type_t tcont_num,
					   uint32_t cpu_tx_queue);

tpm_error_code_t tpm_proc_omci_del_channel(uint32_t owner_id);

tpm_error_code_t tpm_proc_oam_epon_add_channel(uint32_t owner_id,
					       uint32_t cpu_rx_queue,
					       tpm_trg_port_type_t llid_num);

tpm_error_code_t tpm_proc_loop_detect_add_channel(uint32_t owner_id, tpm_ether_type_key_t ety);

tpm_error_code_t tpm_proc_loop_detect_del_channel(uint32_t owner_id);

tpm_error_code_t tpm_proc_oam_loopback_add_channel(uint32_t owner_id);

tpm_error_code_t tpm_proc_oam_loopback_del_channel(uint32_t owner_id);

tpm_error_code_t tpm_proc_oam_epon_del_channel(uint32_t owner_id);

int32_t tpm_proc_l3_tcam_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       tpm_l3_type_key_t *l3_key,
			       tpm_parse_fields_t parse_rule_bm,
			       tpm_parse_flags_t parse_flags_bm,
			       tpm_rule_action_t *rule_action,
			       tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_l3_sram_build(tpm_src_port_type_t src_port,
			       tpm_dir_t dir,
			       uint32_t rule_num,
			       uint8_t is_pppoe,
			       tpm_db_pon_type_t pon_type,
			       tpm_pkt_frwd_t *pkt_frwd,
			       tpm_rule_action_t *rule_action,
			       tpm_parse_flags_t parse_flags_bm,
			       tpm_pncl_sram_data_t *sram_data);

tpm_error_code_t tpm_proc_del_l3_type_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

tpm_error_code_t tpm_proc_erase_section(uint32_t owner_id, tpm_api_type_t api_type);

tpm_error_code_t tpm_proc_del_ipv4_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

tpm_error_code_t tpm_proc_add_ipv4_mc_stream(uint32_t owner_id,
					     uint32_t stream_num,
					     tpm_mc_igmp_mode_t igmp_mode,
					     uint8_t mc_stream_pppoe,
					     uint16_t vid,
					     uint8_t ipv4_src_add[4],
					     uint8_t ipv4_dst_add[4],
					     uint8_t ignore_ipv4_src,
					     uint16_t dest_queue,
					     tpm_trg_port_type_t dest_port_bm);

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
				 tpm_pkt_mod_t *pkt_mod,
				 tpm_pncl_sram_data_t *sram_data);

int32_t tpm_proc_ipv4_tcam_build(tpm_src_port_type_t src_port,
				 tpm_dir_t dir,
				 uint32_t rule_num,
				 tpm_ipv4_acl_key_t *ipv4_key,
				 tpm_parse_fields_t parse_rule_bm,
				 tpm_parse_flags_t parse_flags_bm,
				 tpm_rule_action_t *rule_action,
				 tpm_pkt_mod_t *pkt_mod,
				 tpm_pkt_mod_bm_t pkt_mod_bm,
				 tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv4_mc_tcam_build(tpm_mc_filter_mode_t filter_mode,
				    uint32_t stream_num,
				    uint16_t vid,
				    uint8_t mc_stream_pppoe,
				    uint8_t ipv4_src_add[4],
				    uint8_t ipv4_dst_add[4],
				    uint8_t ignore_ipv4_src,
				    tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipvx_mc_sram_build(tpm_mc_filter_mode_t filter_mode,
				    tpm_mc_igmp_mode_t igmp_mode,
				    uint16_t dest_queue,
				    tpm_trg_port_type_t target_port,
				    uint32_t mod_entry,
				    tpm_pncl_sram_data_t *sram_data,
				    tpm_ip_ver_t ip_version);

tpm_error_code_t tpm_proc_updt_ipv4_mc_stream(uint32_t owner_id,
					      uint32_t stream_num, tpm_trg_port_type_t dest_port_bm);

tpm_error_code_t tpm_proc_del_ipv4_mc_stream(uint32_t owner_id, uint32_t stream_num);

int32_t tpm_proc_ipv6_mc_tcam_build(tpm_mc_filter_mode_t filter_mode,
				    uint32_t stream_num,
				    uint16_t vid,
				    uint8_t mc_stream_pppoe,
				    uint8_t ipv6_dst_add[16],
				    uint8_t sip_index,
				    uint8_t ignore_sip,
				    tpm_pncl_tcam_data_t *tcam_data);

tpm_error_code_t tpm_proc_add_ipv6_mc_stream(uint32_t owner_id,
					     uint32_t stream_num,
					     tpm_mc_igmp_mode_t igmp_mode,
					     uint8_t mc_stream_pppoe,
					     uint16_t vid,
					     uint8_t ipv6_src_add[16],
					     uint8_t ipv6_dst_add[16],
					     uint8_t ignore_ipv6_src,
					     uint16_t dest_queue,
					     tpm_trg_port_type_t dest_port_bm);

tpm_error_code_t tpm_proc_updt_ipv6_mc_stream(uint32_t owner_id,
					      uint32_t stream_num, tpm_trg_port_type_t dest_port_bm);
tpm_error_code_t tpm_proc_del_ipv6_mc_stream(uint32_t owner_id, uint32_t stream_num);

tpm_error_code_t tpm_proc_del_l2_prim_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

int32_t tpm_proc_src_port_gmac_bm_map(tpm_src_port_type_t src_port, tpm_gmac_bm_t *gmac_bm);

tpm_error_code_t tpm_proc_l2_num_vlan_tags_init(void);
tpm_error_code_t tpm_proc_ipv4_ttl_init(uint32_t ttl_illegal_action);
tpm_error_code_t tpm_proc_ds_load_balance_init(void);

tpm_error_code_t tpm_proc_tcp_flag_init(void);
tpm_error_code_t tpm_proc_ipv4_len_init(void);
tpm_error_code_t tpm_proc_ipv4_igmp_init(void);
tpm_error_code_t tpm_proc_ipv4_proto_init(void);
tpm_error_code_t tpm_proc_ipv4_frag_init(void);

tpm_error_code_t tpm_proc_virt_uni_init(void);

tpm_error_code_t tpm_proc_add_uni_port_igmp_rule(tpm_src_port_type_t src_port, tpm_igmp_frwd_mode_t mode);
tpm_error_code_t tpm_proc_add_wan_port_igmp_rule(tpm_igmp_frwd_mode_t mode);
tpm_error_code_t tpm_proc_del_port_igmp_rule(tpm_src_port_type_t src_port);

int32_t tpm_proc_ipv6_nh_tcam_build(uint32_t rule_num,
				    tpm_nh_iter_t nh_iter,
				    uint32_t nh,
				    tpm_parse_flags_t parse_flags_bm,
				    tpm_rule_action_t *rule_action,
				    tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_nh_sram_build(uint32_t rule_num,
				    tpm_nh_iter_t nh_iter,
				    uint32_t nh,
				    tpm_db_pon_type_t pon_type,
				    tpm_pkt_frwd_t *pkt_frwd,
				    tpm_rule_action_t *rule_action,
				    tpm_pncl_sram_data_t *sram_data);

tpm_error_code_t tpm_proc_add_ipv6_nh_acl_rule(uint32_t owner_id,
					       uint32_t rule_num,
					       uint32_t *rule_idx,
					       tpm_parse_flags_t parse_flags_bm,
					       tpm_nh_iter_t nh_iter,
					       uint32_t nh,
					       tpm_pkt_frwd_t *pkt_frwd,
					       tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_del_ipv6_nh_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

int32_t tpm_proc_ipv6_gen_tcam_build(tpm_src_port_type_t src_port,
				     tpm_dir_t dir,
				     uint32_t rule_num,
				     tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
				     tpm_parse_fields_t parse_rule_bm,
				     tpm_parse_flags_t parse_flags_bm,
				     tpm_pkt_mod_t *pkt_mod,
				     tpm_pkt_mod_bm_t pkt_mod_bm,
				     tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_gen_sram_build(tpm_src_port_type_t src_port,
				     tpm_dir_t dir,
				     uint32_t rule_num,
				     tpm_db_pon_type_t pon_type,
				     tpm_pkt_frwd_t *pkt_frwd,
				     tpm_rule_action_t *rule_action,
				     uint32_t mod_cmd,
				     tpm_pncl_sram_data_t *sram_data);

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
						tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_del_ipv6_gen_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

int32_t tpm_proc_ipv6_dip_tcam_build(tpm_src_port_type_t src_port,
				     tpm_dir_t dir,
				     uint32_t rule_num,
				     tpm_ipv6_addr_key_t *ipv6_dip_key,
				     tpm_parse_fields_t parse_rule_bm,
				     tpm_parse_flags_t parse_flags_bm,
				     tpm_pkt_mod_t *pkt_mod,
				     tpm_pkt_mod_bm_t pkt_mod_bm,
				     tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_dip_sram_build(tpm_src_port_type_t src_port,
				     tpm_dir_t dir,
				     uint32_t rule_num,
				     tpm_db_pon_type_t pon_type,
				     tpm_pkt_frwd_t *pkt_frwd,
				     tpm_rule_action_t *rule_action,
				     uint32_t mod_cmd,
				     tpm_pncl_sram_data_t *sram_data);

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
						tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_del_ipv6_dip_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

int32_t tpm_proc_ipv6_l4ports_tcam_build(tpm_src_port_type_t src_port,
					 tpm_dir_t dir,
					 uint32_t rule_num,
					 tpm_l4_ports_key_t *l4_key,
					 tpm_parse_fields_t parse_rule_bm,
					 tpm_parse_flags_t parse_flags_bm,
					 tpm_rule_action_t *rule_action,
					 tpm_pkt_mod_t *pkt_mod,
					 tpm_pkt_mod_bm_t pkt_mod_bm,
					 tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_l4ports_sram_build(tpm_src_port_type_t src_port,
					 tpm_dir_t dir,
					 uint32_t rule_num,
					 tpm_db_pon_type_t pon_type,
					 tpm_pkt_frwd_t *pkt_frwd,
					 tpm_rule_action_t *rule_action,
					 uint32_t mod_cmd,
					 tpm_pncl_sram_data_t *sram_data);

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
						     tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_del_ipv6_l4_ports_acl_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);

tpm_error_code_t tpm_proc_del_acl_rule(tpm_api_sections_t api_section, uint32_t owner_id, uint32_t rule_idx, uint32_t busy_check);

tpm_error_code_t tpm_proc_pnc_create(tpm_pnc_ranges_t range_id,
				     uint32_t pnc_entry,
				     tpm_pncl_pnc_full_t *pnc_data);

uint32_t tpm_proc_all_gmac_bm(void);

void tpm_proc_mc_vid_remove_ai_bits
(tpm_rule_action_t *rule_action,
 tpm_src_port_type_t src_port, tpm_l2_acl_key_t *l2_key, tpm_error_code_t ret_code);

tpm_error_code_t tpm_proc_add_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config);

tpm_error_code_t tpm_proc_remove_mc_vid_cfg(uint32_t mc_vid);

tpm_error_code_t tpm_proc_update_mc_vid_cfg(uint32_t mc_vid,
					    tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config);
tpm_error_code_t tpm_proc_set_mc_vid_port_vids(uint32_t owner_id,
					       uint32_t mc_vid,
					       tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config);
tpm_error_code_t tpm_proc_delete_ipvx_mc_pnc_entry(tpm_mc_filter_mode_t filter_mode,
						   uint32_t stream_num,
						   uint32_t dest_port_bm,
						   uint32_t u4_entry,
						   tpm_ip_ver_t ip_version);

tpm_error_code_t tpm_proc_catch_all_init(void);

int32_t tpm_proc_ipv6_gen_5t_tcam_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_l4_ports_key_t *l4_key,
					tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
					tpm_parse_fields_t parse_rule_bm,
					tpm_parse_flags_t parse_flags_bm,
					tpm_rule_action_t *rule_action,
					tpm_pkt_mod_t *pkt_mod,
					tpm_pkt_mod_bm_t pkt_mod_bm,
					tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_gen_5t_sram_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_db_pon_type_t pon_type,
					uint32_t ipv6_gen_subflow,
					tpm_pkt_frwd_t *pkt_frwd,
					tpm_rule_action_t *rule_action,
					long long int_pkt_act,
					tpm_pkt_mod_t *pkt_mod,
					uint32_t mod_cmd,
					tpm_pncl_sram_data_t *sram_data);

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
					tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_dip_5t_sram_build(tpm_dir_t dir,
					uint32_t rule_num,
					tpm_db_pon_type_t pon_type,
					tpm_pkt_frwd_t *pkt_frwd,
					tpm_rule_action_t *rule_action,
					long long int_pkt_act,
					tpm_pkt_mod_t *pkt_mod,
					uint32_t mod_cmd,
					tpm_pncl_sram_data_t *sram_data);

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
					       tpm_rule_action_t *rule_action);

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
					       tpm_rule_action_t *rule_action);

int32_t tpm_proc_ipv6_l4_ports_5t_tcam_build(tpm_dir_t dir,
					     uint32_t rule_num,
					     tpm_l4_ports_key_t *l4_key,
					     tpm_parse_fields_t parse_rule_bm,
					     tpm_parse_flags_t parse_flags_bm,
					     tpm_rule_action_t *rule_action,
					     tpm_pkt_mod_t *pkt_mod,
					     tpm_pkt_mod_bm_t pkt_mod_bm,
					     tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_ipv6_l4_ports_5t_sram_build(tpm_dir_t dir,
					     uint32_t rule_num,
					     tpm_db_pon_type_t pon_type,
					     uint32_t ipv6_l4_subflow,
					     tpm_pkt_frwd_t *pkt_frwd,
					     tpm_rule_action_t *rule_action,
					     long long int_pkt_act,
					     tpm_pkt_mod_t *pkt_mod,
					     uint32_t mod_cmd,
					     tpm_pncl_sram_data_t *sram_data);

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
						    tpm_rule_action_t *rule_action);

tpm_error_code_t tpm_proc_del_ipv6_gen_5t_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);
tpm_error_code_t tpm_proc_del_ipv6_dip_5t_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);
tpm_error_code_t tpm_proc_del_ipv6_l4_ports_5t_rule(uint32_t owner_id, uint32_t rule_idx, tpm_caller_t ext_call);
tpm_error_code_t tpm_proc_ipv6_hoplimit_init(uint32_t hoplimit_illegal_action);
tpm_error_code_t tpm_proc_send_genquery_to_uni(tpm_trg_port_type_t dest_port_bm, uint32_t packet_num);
tpm_error_code_t tpm_proc_ipv6_l4_mc_ds_init(void);
tpm_error_code_t tpm_proc_ipv6_mc_sip_init(void);

int32_t tpm_proc_cnm_l2_tcam_build(tpm_src_port_type_t src_port,
				   tpm_parse_fields_t l2_parse_rule_bm,
				   tpm_l2_acl_key_t *l2_key,
				   uint32_t ipv4_key_idx,
				   tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_cnm_l2_sram_build(uint32_t precedence,
				   tpm_pkt_frwd_t *pkt_frwd,
				   tpm_pkt_action_t pkt_act,
				   uint32_t mod_cmd,
				   tpm_pncl_sram_data_t *sram_data);

int32_t tpm_proc_cnm_ipv4_tcam_build(tpm_src_port_type_t src_port,
				     uint32_t precedence,
				     tpm_parse_fields_t ipv4_parse_rule_bm,
				     tpm_ipv4_acl_key_t *ipv4_key,
				     tpm_pncl_tcam_data_t *tcam_data);

int32_t tpm_proc_cnm_ipv4_sram_build(tpm_pkt_frwd_t *pkt_frwd,
				     tpm_pkt_action_t pkt_act,
				     uint32_t mod_cmd,
				     tpm_pncl_sram_data_t *sram_data);
tpm_pnc_trg_t tpm_proc_cnm_pnc_trg_get(tpm_trg_port_type_t trg_port);

uint8_t tpm_proc_if_ipv4_pre_range_is_full(tpm_src_port_type_t src_port, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
uint8_t tpm_proc_if_cnm_main_range_is_full(tpm_src_port_type_t src_port, uint32_t precedence);
int32_t tpm_proc_add_cnm_ipv4_pre_filter_key(uint32_t owner_id, tpm_src_port_type_t src_port, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key, uint32_t *key_idx);
int32_t tpm_proc_del_cnm_ipv4_pre_filter_key(uint32_t owner_id, tpm_src_port_type_t src_port, uint32_t key_idx);
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
				 uint32_t *rule_idx);
int32_t tpm_proc_del_l2_cnm_rule(uint32_t owner_id, uint32_t rule_idx);
int32_t tpm_proc_add_ipv4_cnm_rule(uint32_t owner_id, tpm_src_port_type_t src_port, uint32_t precedence, tpm_parse_fields_t ipv4_parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key, tpm_pkt_frwd_t *pkt_frwd, tpm_pkt_action_t pkt_act, uint32_t pbits, uint32_t *rule_idx);
int32_t tpm_proc_add_ipv6_cnm_rule(uint32_t owner_id,
				   tpm_src_port_type_t src_port,
				   uint32_t precedence,
				   tpm_parse_fields_t ipv6_parse_rule_bm,
				   tpm_ipv6_acl_key_t *ipv6_key,
				   tpm_pkt_frwd_t *pkt_frwd,
				   tpm_pkt_action_t pkt_act,
				   uint32_t pbits,
				   uint32_t *rule_idx);
int32_t tpm_proc_del_ipv4_cnm_rule(uint32_t owner_id, uint32_t rule_idx);
int32_t tpm_proc_del_ipv6_cnm_rule(uint32_t owner_id, uint32_t rule_idx);
int32_t tpm_proc_calc_cnm_rule_num(tpm_src_port_type_t src_port, uint32_t precedence, uint32_t *rule_num);
tpm_error_code_t tpm_proc_find_mac_addr_db(tpm_l2_acl_key_t *src_mac_addr, int32_t *addr_exist, uint32_t *rule_num, uint32_t *tbl_num);
tpm_error_code_t tpm_proc_add_static_mac_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr);
tpm_error_code_t tpm_proc_del_static_mac_rule(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr);
tpm_error_code_t tpm_proc_mac_learn_default_rule_act_set(uint32_t owner_id, tpm_unknown_mac_conf_t mac_conf);
tpm_error_code_t tpm_proc_mac_learn_entry_num_get(uint32_t *entry_num);
#ifdef CONFIG_MV_ETH_WAN_SWAP
tpm_error_code_t tpm_proc_hot_swap_profile(uint32_t owner_id,
				    tpm_eth_complex_profile_t profile_id);
#endif /* CONFIG_MV_ETH_WAN_SWAP */


int32_t tpm_proc_check_api_busy(tpm_api_type_t api_type, uint32_t rule_num);
int32_t tpm_proc_api_busy_done(tpm_api_type_t api_type, uint32_t rule_num);
int32_t tpm_proc_src_port_dir_map(tpm_src_port_type_t src_port, tpm_dir_t *dir);
int32_t tpm_proc_check_all_api_busy(void);
int32_t tpm_proc_all_api_busy_done(void);

bool tpm_split_mod_stage1_check(tpm_pkt_mod_bm_t pkt_mod_bm,
				tpm_pkt_mod_t *pkt_mod,
				tpm_rule_action_t *rule_action,
				bool check_vlan_noop);
int32_t tpm_proc_calc_cnm_precedence(tpm_src_port_type_t src_port, uint32_t rule_num, uint32_t *precedence);
tpm_error_code_t tpm_proc_hwf_admin_set(tpm_gmacs_enum_t port, uint8_t txp, uint8_t enable);

tpm_error_code_t tpm_proc_check_dst_uni_port(tpm_trg_port_type_t dest_port_bm);
tpm_error_code_t tpm_proc_set_active_wan(tpm_gmacs_enum_t active_wan);

/* hot swap profile acl recovery func api section */
tpm_error_code_t tpm_acl_rcvr_func_mac_learn(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ds_load_balance(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_cpu_loopback(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_l2_prim(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_l3_type(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv4(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv4_mc(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv6_gen(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv6_dip(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv6_mc(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv6_nh(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_ipv6_l4(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);
tpm_error_code_t tpm_acl_rcvr_func_cnm(uint32_t owner_id, tpm_db_api_entry_t *api_data, uint32_t rule_idx_updt_en);

tpm_error_code_t tpm_proc_set_drop_precedence_mode(uint32_t owner_id, tpm_drop_precedence_t mode);
tpm_error_code_t tpm_proc_get_drop_precedence_mode(uint32_t owner_id, tpm_drop_precedence_t *mode);

#ifdef __cplusplus
}
#endif
#endif				/* _TPM_PKT_PROCL_H_ */
