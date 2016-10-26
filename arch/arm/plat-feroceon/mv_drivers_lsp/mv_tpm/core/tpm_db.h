
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
* tpm_db.h
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
#ifndef _TPM_DB_H_
#define _TPM_DB_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/

typedef enum tpm_db_error {
	TPM_DB_OK = TPM_OK,
	TPM_DB_ERR_DB_TBL_FULL,
	TPM_DB_ERR_DB_TBL_INV,
	TPM_DB_ERR_INV_INPUT,
	TPM_DB_ERR_DB_INCONSISTENCY,
	TPM_DB_ERR_RANGE_EXIST,
	TPM_DB_ERR_REC_NOT_EXIST,
	TPM_DB_ERR_REC_EXIST,
	TPM_DB_ERR_REC_INV,
	TPM_DB_ERR_REC_STATUS_ERR,
	TPM_DB_ERR_MEM_ALLOC_FAIL,
	TPM_DB_ERR_PORT_NUM = 0xFF
} tpm_db_error_t;

/* Possible roles an Ethernet GMAC or PON MAC can play */
typedef enum {
	TPM_GMAC_FUNC_NONE,
	TPM_GMAC_FUNC_LAN,
	TPM_GMAC_FUNC_WAN,
	TPM_GMAC_FUNC_LAN_AND_WAN,
	TPM_GMAC_FUNC_VIRT_UNI,
	TPM_GMAC_FUNC_LAN_UNI,
	TPM_GMAC_FUNC_US_MAC_LEARN_DS_LAN_UNI /* For Media Convert Loopback mode */
} tpm_db_gmac_func_t;

typedef enum {
	TPM_MOD_ENTRY_FREE,
	TPM_MOD_ENTRY_BOOKED,
	TPM_MOD_ENTRY_OCCUPIED,
	TPM_MOD_ENTRY_RESERVED,
	TPM_MOD_ENTRY_SPLIT_MOD,
	TPM_MOD_ENTRY_STATUS_MAX,
} tpm_mod_entry_status_t;

typedef enum {
	TPM_PNC_RNG_LAST_INIT_DEF,	/* Last entry in PnC range is regular entry */
	TPM_PNC_RNG_LAST_INIT_DROP,	/* Last entry in PnC range is hardcoded to DROP */
	TPM_PNC_RNG_LAST_INIT_TRAP	/* Last entry in PnC range is hardcoded to TRAP to CPU */
} tpm_db_pnc_rng_last_ent_t;

typedef enum {
	TPM_GMAC1_QUEUE_DATA_TRAFFIC,
	TPM_GMAC1_QUEUE_MAC_LEARN_TRAFFIC,
}tpm_db_gmac1_lpk_queue_type_t;

typedef tpm_init_sched_t tpm_db_sched_t;
typedef tpm_init_port_admin_t tpm_db_port_admin_t;
typedef tpm_init_port_speed_t tpm_db_port_speed_t;
typedef tpm_init_port_duplex_t tpm_db_port_duplex_t;
typedef tpm_init_chip_conn_t tpm_db_chip_conn_t;
typedef tpm_init_int_conn_t tpm_db_int_conn_t;
typedef tpm_init_gmac_conn_t tpm_db_gmac_conn_t;
typedef tpm_init_gmac_bufs_t tpm_db_gmac_bufs_t;
typedef tpm_init_gmac_rx_t tpm_db_gmac_rx_t;
typedef tpm_init_gmac_rxq_t tpm_db_gmac_rxq_t;
typedef tpm_init_txq_owner_t tpm_db_txq_owner_t;
typedef tpm_init_pon_type_t tpm_db_pon_type_t;
typedef tpm_init_tx_mod_t tpm_db_tx_mod_t;
typedef tpm_init_mh_src_t tpm_db_mh_src_t;
typedef tpm_init_mtu_setting_enable_t tpm_db_mtu_setting_enable_t;
typedef tpm_init_pppoe_add_enable_t tpm_db_pppoe_add_enable_t;
typedef tpm_init_ttl_illegal_action_t tpm_db_ttl_illegal_action_t;
typedef tpm_init_fc_params_t tpm_db_fc_params_t;
typedef tpm_init_tcp_flag_check_t tpm_db_tcp_flag_check_t;
typedef tpm_init_ety_dsa_enable_t tpm_db_ety_dsa_enable_t;
typedef tpm_init_split_mod_enable_t tpm_db_split_mod_enable_t;
typedef tpm_init_ctc_cm_enable_t    tpm_db_ctc_cm_enable_t;
typedef tpm_init_split_mod_mode_t    tpm_db_split_mod_mode_t;
typedef tpm_ctc_cm_ipv6_parse_win_t tpm_db_ctc_cm_ipv6_parse_win_t;
typedef tpm_init_ds_mac_based_trunk_enable_t tpm_db_ds_mac_based_trunk_enable_t;

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/

#define TPM_DB_TOTAL_API_SIZE               (512)

/* Defines the amount of memory margin (in units of tpm_db_api_entry_t),
	between each pair of API Range Entries Table. */
#define TPM_DB_API_SECTION_MARGIN_SIZE      (3)

#define TPM_DB_API_NOT_BUSY		    (-1)

/* Define total   Margin memory allocated for the margin between each pair
	of API Range tables, + before the first API Range Entries table.
	(in units of tpm_db_api_entry_t) */
#define TPM_DB_API_SECTION_MARGIN_ALLOC     \
	(TPM_DB_API_SECTION_MARGIN_SIZE * (TPM_MAX_NUM_API_SECTIONS * TPM_NUM_DIRECTIONS + 1))

/* Total table size is tot_num_api_entries + tot_margin */
#define TPM_DB_API_ENTRIES_TBL_SIZE         (TPM_DB_TOTAL_API_SIZE + TPM_DB_API_SECTION_MARGIN_ALLOC)
#define TPM_DB_VALID                        (1)
#define TPM_DB_INVALID                      (0xBEEF)
#define TPM_DB_INVALID_OWNER                (0xFF)

#define TPM_MOD2_NULL_ENT_IDX               (0x1000)

#define TPM_MOD2_MAIN_CHAIN_SIZE            (19)
#define TPM_MOD2_SUBR_MAC_CHAIN_SIZE        (3)
#define TPM_MOD2_SUBR_IP_CHAIN_SIZE         (2)

#define TPM_MOD2_JUMP_AREA_SIZE             (203)
#define TPM_MOD2_MAIN_AREA_SIZE             (TPM_MOD2_MAIN_CHAIN_SIZE * TPM_MOD2_JUMP_AREA_SIZE)
#define TPM_MOD2_SUBR_AREA_SIZE             (0)
#define TPM_MOD2_APPL_SUBR_AREA_SIZE        (35)

#define TPM_MC_VID_NUM_MAX                  (1 << TPM_NUM_MC_VID_BITS)
#define TPM_MC_VLAN_MAX_PNC_RULE_NUM        (16)

#define TPM_MC_IPv6_SIP_NUM_MAX             (8)

#define TPM_MOD2_MAX_MH_CHAIN_SIZE                   (1)
#define TPM_MOD2_MAX_MAC_CHAIN_SIZE                  (3)
#define TPM_MOD2_MAX_VLAN_CHAIN_SIZE                 (2)
#define TPM_MOD2_MAX_PPPOE_CHAIN_SIZE                (2)
#define TPM_MOD2_MAX_IPV6_PPPOE_CHAIN_SIZE           (5)
#define TPM_MOD2_MAX_L2_TWO_TAG_CHAIN_SIZE           (4)
#define TPM_MOD2_MAX_L2_ONE_TAG_CHAIN_SIZE           (3)
#define TPM_MOD2_MAX_IPV4_NAPT_W_PPPOE_CHAIN_SIZE    (14)	/* Single Vlan Modification */
#define TPM_MOD2_MAX_IPV4_NAPT_WO_PPPOE_CHAIN_SIZE   (13)
#define TPM_MOD2_MAX_IPV4_MC_W_MH_CHAIN_SIZE         (8)	/* PPPoE + Proxy (Single Vlan Modification) */
#define TPM_MOD2_MAX_IPV4_MC_WO_MH_CHAIN_SIZE        (7)
#define TPM_MOD2_MAX_ROUTE_W_PPPOE_CHAIN_SIZE   (7)
#define TPM_MOD2_MAX_ROUTE_WO_PPPOE_CHAIN_SIZE  (6)
#define TPM_MOD2_MAX_IPV6_MC_W_MH_CHAIN_SIZE         (8)
#define TPM_MOD2_MAX_IPV6_MC_WO_MH_CHAIN_SIZE        (7)

#define TPM_MOD2_TOTAL_PMT_SIZE                (4096 - 1/* zero entry is reserved */)
#define TPM_MOD2_MAX_JUMP_ENTRIES_NUM          (1024 - 1/* zero entry is reserved */)

#define TPM_DB_IPV6_MAX_KEY_NUM               (128)

#define TPM_DB_IPV6_MAX_SUBFLOW_NUM           ((1 << TPM_NUM_IPV6_SUBFLOW_BITS) - 1)
#define TPM_DB_IPV6_MAX_5T_FLOW_NUM           (128)

#define TPM_DB_INVALID_IPV6_KEY_ID            (0xFFFF)
#define TPM_DB_INVALID_IPV6_FLOW_ID           (0xFFFF)

#define TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK  (TPM_IPv6_PARSE_NH | TPM_PARSE_L4_SRC | TPM_PARSE_L4_DST)
#define TPM_DB_IPV6_GEN_SUBFLOW_PARSE_BM_MASK (TPM_IPv6_PARSE_SIP | TPM_DB_IPV6_L4_SUBFLOW_PARSE_BM_MASK)

#define TPM_DB_SPLIT_MOD_P_BIT_MAX (7)
#define TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX (8)
#define TPM_DB_SPLIT_MOD_P_BIT_NO_SET (0)
#define TPM_DB_SPLIT_MOD_NUM_VLANS_MAX (18)
#define TPM_DB_SPLIT_MOD_INIT_VLANS_NUM (2)

#define TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM    (TPM_CNM_IPV4_PRE_FILTER_KEY_MAX)
#define TPM_DB_CNM_MAX_IPV4_PRE_FILTER_RULE_NUM   (TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_PORT * TPM_MAX_NUM_UNI_PORTS)

#define TPM_DB_CNM_INVALID_IPV4_PRE_RULE_INDEX    (0x0)

#define TPM_DB_CNM_GEN_IPV4_PRE_RULE_INDEX(p, k)  \
			((((k) & TPM_DB_CNM_IPV4_PRE_RULE_INDEX_KEY_MASK) << TPM_DB_CNM_IPV4_PRE_RULE_INDEX_KEY_OFFSET) | \
			(((p) & TPM_DB_CNM_IPV4_PRE_RULE_INDEX_PORT_MASK) << TPM_DB_CNM_IPV4_PRE_RULE_INDEX_PORT_OFFSET))

#define TPM_DB_CNM_IPV4_PRE_RULE_INDEX_PORT_OFFSET    (0)
#define TPM_DB_CNM_IPV4_PRE_RULE_INDEX_PORT_LEN       (16)
#define TPM_DB_CNM_IPV4_PRE_RULE_INDEX_PORT_MASK      ((1 << TPM_DB_CNM_IPV4_PRE_RULE_INDEX_PORT_LEN) - 1)

#define TPM_DB_CNM_IPV4_PRE_RULE_INDEX_KEY_OFFSET     (16)
#define TPM_DB_CNM_IPV4_PRE_RULE_INDEX_KEY_LEN        (16)
#define TPM_DB_CNM_IPV4_PRE_RULE_INDEX_KEY_MASK       ((1 << TPM_DB_CNM_IPV4_PRE_RULE_INDEX_KEY_LEN) - 1)

/*************************************************************/
/*               STRUCTURES                                  */
/*************************************************************/

/*****************************/
/* API DB related structures */
/*****************************/

/* Structure represents the connection between an API call and the modification table entry (if exists)
 * that implemented the modification request in that API call */
typedef struct {
	uint32_t mod_cmd_ind;	/* Index to the modification command table related to this API entry */
	tpm_gmacs_enum_t mod_cmd_mac; /* Only a single "dest MAC" can be set.If none are set, mod_cmd_ind is not used. */
} tpm_db_mod_conn_t;

/* Structure represents the connection between an API call and a PnC entry that implemented
 * or partially implemented that API call */
typedef struct {
	uint32_t pnc_range;	/* The pnc_range where the API call was implemented */
	uint32_t pnc_index;	/* The index in the PnC range where the API call is implemented */
} tpm_db_pnc_conn_entry_t;

/* Structure represents the connection between an API call and all the PnC entries that implemented the API call */
typedef struct {
	uint32_t num_pnc_ranges;	/* Number of PNC entries associated with this API entry,
					 * equals to '1' unless the API call spans over multiple PnC ranges,
					 * e.g. IPv6 DIP+SIP */
	/* Upto TPM_MAX_API_MULTI_RANGES PnC entries may be needed to implement an API call */
	tpm_db_pnc_conn_entry_t pnc_conn_tbl[TPM_MAX_API_MULTI_RANGES];
} tpm_db_pnc_conn_t;

/* Structure represents an API call, including all param data and the link to Pnc and modif_table implementation */
typedef struct {
	uint32_t valid;		/* invalid - 0xBEEF */
	uint32_t bi_dir;	/* Some API entries may be bi-directional */
	uint32_t rule_num;	/* Current rule number in acl/table */
	uint32_t rule_idx;	/* Original rule number in acl/table, when entry was created */
	tpm_rule_entry_t api_rule_data;	/* tpm_rule_entry_t is defined in tpm_types.h */
	tpm_db_mod_conn_t mod_tbl_conn;	/* API connection to the modification table */
	tpm_db_pnc_conn_t pnc_tbl_conn;	/* API connection to the PnC */
} tpm_db_api_entry_t;

/* Structure represents an API acl/table */
typedef struct {
	uint32_t valid;
	/* The primary pnc_range_id, for those api_sections that a uniquely linked onc_range. */
	tpm_pnc_ranges_t prim_pnc_range;
	uint32_t table_size;	/* (cont.) e.g. - All API's except for IPV6, IPV6 MC */
	uint32_t table_start;	/* Start index in the api_ent_mem_area */
	uint32_t num_valid_entries;
	int32_t last_valid_entry;	/* Equals to rule with highest rulenum in the api_table */
	int32_t last_valid_index;	/* Highest Occupied Index in the api_table */
} tpm_db_api_section_t;

typedef struct {
	uint32_t table_size;	/* (cont.) e.g. - All API's except for IPV6, IPV6 MC */
	uint32_t table_start;	/* Start index in the api_ent_mem_area */
} tpm_db_api_check_t;

/*****************************/
/* PNC  related structures   */
/*****************************/

typedef struct {
	tpm_range_type_t range_type;	/* Pnc Range type, ACL or Table */
	/* PnC range LookupId. If range spans over multiple lookupIds
		(e.g. IPV6 DIP/DIP), this is the first one. */
	uint32_t base_lu_id;
	/* Determines the minimum reset_level that will perform a reset to this PnC range */
	tpm_reset_level_enum_t min_reset_level;
	uint32_t num_luids;	/* Number of LookupIds this PnC range spans over, usually '1'.
					* Multiple lu_ids, when PnC range must support large number of sessions in the TCAM */
	tpm_pnc_ranges_t range_id;
	uint32_t range_start;	/* Range's Physical PnC start entry */
	uint32_t range_end;	/* Range's Physical PnC end entry */
	uint32_t range_size;	/* Range's Physical PnC size */
	uint32_t cntr_grp;	/* Range's counter group number, from 0 to 3 */
	/* Range's least used mask flag, 1: mask least used functions,
		0:enbale least used function of this PnC range */
	uint32_t lu_mask;
	uint32_t api_start;	/* First available entry for api's. (Relative to range_start) */
	uint32_t api_end;	/* Last available entry for api's. (Relative to range_start) */
	/* How to init last entry of the PNC range (drop, trap to CPU, or no special treatment). */
	tpm_db_pnc_rng_last_ent_t init_last_entry;
	/* For pnc_ranges with multiple lu_id, init_last_entry must be repeated for each lu_id. */
	/* GMAC's that are included in TCAM in last_entry (drop/trap) treatment. */
	tpm_gmac_bm_t last_ent_portid;
} tpm_db_pnc_range_conf_t;

/* Structure represents a PnC range */
typedef struct {
	uint32_t free_entries;	/* Number of unused PnC entries in Range */
	uint32_t num_resets;	/* Number of resets the range has been through */
} tpm_db_pnc_range_oper_t;

/* Structure represents a PnC range */
typedef struct {
	uint32_t valid;	/* invalid - 0xBEEF */
	tpm_db_pnc_range_conf_t pnc_range_conf;
	tpm_db_pnc_range_oper_t pnc_range_oper;
} tpm_db_pnc_range_t;

/* Structure represents DB info for a PnC entry */
typedef struct {
	uint32_t valid;		/* invalid - 0xBEEF */
	tpm_pnc_all_t pnc_data;	/* Shadow of the PnC configuration of this entry */
} tpm_db_pnc_shadow_t;

typedef struct {
	uint32_t valid;			/* invalid - 0xBEEF */
	uint32_t tpid_ether_type;	/* Modification data associated with this entry */
} tpm_db_etype_sel_t;

typedef struct {
	uint32_t tpm_reserved;	/* Reserved modification entries for TPM rules */
} tpm_db_mod_config_t;

typedef struct {
	tpm_mod2_entry_t entry;	/* Modification table entry */
	uint16_t pnc_num_ref;	/* Number of references from Pnc entries to this modification rule */
	uint16_t owner;		/* Owner of modification entry - TPM rule or CPU application */
	uint32_t valid;
} tpm_db_mod2_shadow_t;

typedef struct {
	uint8_t mac[TPM_SA_LEN];
	uint16_t mod_idx;	/* Start index of the commands chain in the modification table */
	uint16_t mod_num_ref;	/* Number of references from modification table to this MAC entry */
	uint32_t valid;

} tpm_mod2_mac_translation_t;

typedef struct {
	uint8_t ip[TPM_IPV4_SIP_LEN];
	uint16_t mod_idx;	/* Start index of the commands chain in the modification table */
	uint16_t mod_num_ref;	/* Number of references from modification table to this MAC entry */
	uint32_t valid;

} tpm_mod2_ip_translation_t;

/*typedef uint16_t (*parse_jump_index_func_t)(tpm_mod2_entry_t *pattern); */

typedef struct {
	tpm_mod_entry_status_t status;
	tpm_mod2_entry_t pattern;
} tpm_mod2_jump_pmt_info_t;

typedef struct {
	tpm_mod_entry_status_t status;
	tpm_chain_type_t chain_type;
	tpm_mod2_entry_t pattern;
	uint16_t chain_id;
} tpm_mod2_chain_pmt_info_t;

typedef struct {
	tpm_mod_entry_status_t status;
	uint16_t base_entry;
	uint16_t entry_num;
	uint16_t user_num;
} tpm_mod2_chain_info_t;

typedef struct {
	uint16_t base_entry;
	uint16_t total_num;
	uint16_t split_num;
	uint16_t next_free[TPM_MAX_NUM_GMACS];
	uint16_t next_split[TPM_MAX_NUM_GMACS];
	uint16_t split_pbit_num;
} tpm_mod2_jump_cfg_t;

typedef struct {
	uint16_t base_entry;
	uint16_t subr_chain;
	uint16_t total_num;
	uint16_t max_size;
	uint16_t next_free[TPM_MAX_NUM_GMACS];
} tpm_mod2_chain_cfg_t;

/*****************************/
/* API Ownership Structures  */
/*****************************/
/*
TBD
Structures
API Group enum
*/
/*****************************/
/* Initialization Structures */
/*****************************/

typedef struct {
	uint32_t omci_etype;
	uint32_t tpm_init_succeeded;

	uint32_t oam_channel_configured;/* omci or oam channel configured */
	uint16_t omci_gemport;
	uint32_t oam_cpu_rx_q;		/* omci cpu rx q or oam cpu rx q */
	uint32_t oam_cpu_tx_q;		/* omci cpu tx q or oam cpu tx q */
	uint32_t oam_cpu_tx_port;	/* omci tcount or oam llid */

	uint32_t pnc_init_debug_port;
	uint32_t oam_loopback_channel_configured;
	tpm_db_pon_type_t pon_type;
	tpm_gmacs_enum_t active_wan;

	tpm_db_mh_src_t ds_mh_set_conf;

	tpm_init_cfg_pnc_parse_t cfg_pnc_parse;

	tpm_init_cpu_loopback_t cpu_loopback;

	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	tpm_init_double_tag_t dbl_tag;

#if 0
	uint16_t vlan1_tpid;
	uint16_t vlan2_tpid;
#endif
	tpm_init_tpid_opt_t tag_tpid;
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable;

} tpm_db_misc_t;

/* Structure holds the IGMP/MLD settings */
typedef struct {
	uint32_t igmp_snoop_enable;
	uint32_t frwd_mode[TPM_MAX_NUM_PORTS];
	uint32_t cpu_queue;
	tpm_mc_filter_mode_t filter_mode;
	/*tpm_mc_igmp_mode_t      igmp_mode; */
	uint8_t per_uni_vlan_xlat;
	uint8_t mc_pppoe_enable;
	uint8_t mc_hwf_queue;
	uint8_t mc_cpu_queue;
	uint8_t mc_lpbk_enable[TPM_IP_VER_MAX];
} tpm_db_igmp_t;

/* Structure holds the physical connections of all external Ethernet ports */
typedef struct {
	uint32_t valid;
	tpm_src_port_type_t port_src;		/* Port number according to Device port numbering scheme 	*/
	tpm_db_chip_conn_t chip_connect;	/* Connection to physical output lines on SoC 			*/
	tpm_db_int_conn_t int_connect;		/* Internal Ethernet device port is connected to 		*/
	uint32_t switch_port;			/* Port of Internal_Switch, the port is connected to 		*/
} tpm_db_eth_port_conf_t;

/* Structure holds the physical connections of the KW2 SoC */
typedef struct {
	uint32_t		valid;
	tpm_init_gmac_conn_t	conn;
	tpm_src_port_type_t	port_src;
} tpm_db_gmac_conn_conf_t;

typedef struct {
	uint32_t valid;
	tpm_db_sched_t sched_method;
	tpm_db_txq_owner_t queue_owner;
	uint32_t owner_queue_num;
	uint32_t queue_size;
	uint32_t queue_weight;
	uint32_t bucket_size;
	uint32_t rate_limit;
	uint32_t wrr;
} tpm_db_gmac_txq_t;

/*Structure defines the details of a Packet Processor Tx component  */
typedef struct {
	uint32_t valid;						/* Defines if the Tx component is used */
	uint32_t bucket_size;
	uint32_t rate_limit;
	uint32_t prio;
	tpm_db_gmac_txq_t tx_queue[TPM_MAX_NUM_TX_QUEUE];	/* Config. of each of the 8 Tx queue of this Tx component */
} tpm_db_gmac_tx_t;

#if 0
/* Structure contains user ownership of an API Group */
typedef struct {
	uint32_t valid;
	uint32_t api_owner_id;
	tpm_api_type_t api_type;
} tpm_db_api_group_t;
#endif

/* */
typedef struct {
	uint32_t valid;
	uint32_t owner_id;
} tpm_db_owners_t;

/* Structure for defining MH_select to uni_vector and to amber_port_vector */
typedef struct {
	uint32_t uni_vector;
	uint16_t amber_port_vector;
	uint32_t pnc_vector;
} tpm_db_tx_ds_mh_sel_mapping_t;

typedef struct {
	tpm_init_mtu_setting_enable_t mtu_enable;
	uint32_t ipv4_mtu_us;
	uint32_t ipv4_pppoe_mtu_us;
	uint32_t ipv6_mtu_us;
	uint32_t ipv6_pppoe_mtu_us;
	uint32_t ipv4_mtu_ds;
	uint32_t ipv6_mtu_ds;
	uint32_t mru;
} tpm_db_mtu_t;

typedef struct {
	uint32_t mc_vlan;
	uint32_t valid;
	uint32_t pnc_index_num;
	uint32_t pnc_index[TPM_MC_VLAN_MAX_PNC_RULE_NUM];
} tpm_db_mc_ai_bits_t;

typedef struct {
	uint32_t valid;
	uint32_t pnc_index_num;
	uint32_t reference_num;
	uint8_t  src_ip[16];
} tpm_db_ipv6_mc_sip_t;

typedef struct {
	tpm_mc_vid_port_cfg_t mc_vid_port_vids[TPM_MAX_NUM_UNI_PORTS];
	uint32_t valid;
	uint32_t mc_vid;
} tpm_mc_vid_cfg_t;

typedef struct {
	uint32_t valid;
	uint32_t user_num;
	tpm_ipv6_gen_acl_key_t gen_key;
} tpm_db_ipv6_gen_key_t;

typedef struct {
	uint32_t valid;
	uint32_t user_num;
	tpm_ipv6_addr_key_t dip_key;
} tpm_db_ipv6_dip_key_t;

typedef struct {
	uint32_t valid;
	uint32_t user_num;
	uint32_t protocol;
	tpm_l4_ports_key_t l4_key;
} tpm_db_ipv6_l4_key_t;

typedef struct {
	tpm_db_ipv6_gen_key_t gen_key[TPM_DB_IPV6_MAX_KEY_NUM];
	tpm_db_ipv6_dip_key_t dip_key[TPM_DB_IPV6_MAX_KEY_NUM];
	tpm_db_ipv6_l4_key_t l4_key[TPM_DB_IPV6_MAX_KEY_NUM];
} tpm_db_ipv6_key_shadow_t;

typedef struct {
	uint32_t valid;
	uint32_t user_num;
	tpm_parse_fields_t parse_bm;
	uint32_t l4_key;
} tpm_db_ipv6_l4_flow_t;

typedef struct {
	uint32_t valid;
	uint32_t user_num;
	tpm_parse_fields_t parse_bm;
	uint32_t gen_key;
	uint32_t l4_key;
} tpm_db_ipv6_gen_flow_t;

typedef struct {
	tpm_db_ipv6_l4_flow_t l4_flow[TPM_DB_IPV6_MAX_SUBFLOW_NUM];
	tpm_db_ipv6_gen_flow_t gen_flow[TPM_DB_IPV6_MAX_SUBFLOW_NUM];
} tpm_db_ipv6_subflow_shadow_t;

typedef struct {
	uint32_t valid;
	tpm_dir_t src_dir;
	tpm_parse_fields_t parse_bm;
	uint32_t is_pppoe;
	uint32_t gen_key;
	uint32_t dip_key;
	uint32_t l4_key;
} tpm_db_ipv6_5t_flow_shadow_t;

typedef struct {
	uint32_t igmp_mode;
	uint8_t mc_stream_pppoe;
	uint8_t valid;
	uint16_t vid;
	uint8_t group_addr[16];
	uint8_t src_addr[16];
	uint8_t ignore_src_addr;
	uint16_t dest_queue;
	uint32_t dest_port_bm;
	uint32_t u4_entry;
} tpm_db_ipv6_mc_stream_entry_t;

typedef struct {
	uint32_t valid;
	uint32_t user_num;
	tpm_vlan_oper_t   vlan_op;
	uint32_t vlan_1;
	uint32_t vlan_2;
	uint32_t tpid_1;
	uint32_t tpid_2;
} tpm_db_split_mod_vlan_t;

typedef struct {
	tpm_db_split_mod_vlan_t split_mod_vlan[TPM_DB_SPLIT_MOD_NUM_VLANS_MAX];
} tpm_db_split_mod_gmac_vlan_t;

typedef struct {
	tpm_db_split_mod_enable_t   split_mod_enable;
	tpm_db_split_mod_mode_t   split_mod_mode;
	uint32_t num_p_bits;
	uint32_t num_vlans;
	uint8_t  p_bits[8];
	tpm_db_split_mod_gmac_vlan_t     gmac_vlan_conf[TPM_MAX_NUM_GMACS];
} tpm_db_split_mod_t;

typedef enum {
	TPM_CTC_CM_L2,
	TPM_CTC_CM_IPv4,
	TPM_CTC_CM_COMBO
} tpm_ctc_cm_rule_type_t;

typedef struct {
	uint32_t                    valid;
	uint32_t                    cm_main_rule_index;
	uint32_t                    ipv4_sub_pattern_key;
	tpm_ctc_cm_rule_type_t      rule_type;
	tpm_parse_fields_t          l2_parse_rule_bm;
	tpm_parse_fields_t          ipv4_parse_rule_bm;
	tpm_parse_fields_t          ipv6_parse_rule_bm;
	tpm_l2_acl_key_t            l2_key;
	tpm_ipv4_acl_key_t          ipv4_key;
	tpm_ipv6_acl_key_t          ipv6_key;
	tpm_pkt_frwd_t              pkt_frwd;
	tpm_pkt_action_t            pkt_act;
	uint32_t                    pbits;
} tpm_db_ctc_cm_rule_entry_t;

typedef struct {
	uint32_t num_rules;
	uint32_t rule_index[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_RULE_NUM];
} tpm_db_cnm_ipv4_pre_table_t;

typedef struct {
	uint32_t valid;
	uint32_t num_users;
    tpm_parse_fields_t parse_rule_bm;
    tpm_ipv4_acl_key_t ipv4_key;
} tpm_db_cnm_ipv4_pre_filter_key_t;

typedef struct {
	uint32_t num_keys;
	tpm_db_cnm_ipv4_pre_filter_key_t key[TPM_DB_CNM_MAX_IPV4_PRE_FILTER_KEY_NUM];
} tpm_db_cnm_ipv4_pre_filter_t;

typedef struct {
	/* api_ent_mem_area backup used for hot swap profile feature */
	tpm_db_api_entry_t api_ent_mem_area_bak[TPM_DB_API_ENTRIES_TBL_SIZE];

	/* API section backup used for hot swap profile feature */
	tpm_db_api_section_t api_section_bak[TPM_MAX_NUM_API_SECTIONS];

	/* MC VID setting backup */
	tpm_mc_vid_cfg_t mc_vid_port_cfg_bak[TPM_MC_VID_NUM_MAX];
	tpm_db_gmac_tx_t gmac_tx_bak[TPM_MAX_NUM_TX_PORTS];

	uint32_t igmp_proxy_sa_mac[6];
	uint32_t igmp_proxy_sa_mac_valid;

	bool     switch_init;
}tpm_db_hot_swap_bak_db_t;

typedef struct {
	uint32_t valid;
	uint32_t rule_index;
} tpm_db_drop_precedence_rule_entry_t;

typedef struct {
	tpm_drop_precedence_t mode;
	tpm_db_drop_precedence_rule_entry_t drop_rule[TPM_MAX_NUM_DROP_PRECEDENCE];
} tpm_db_drop_precedence_t;

typedef struct {
	tpm_db_ctc_cm_enable_t enable;
	tpm_db_ctc_cm_ipv6_parse_win_t ipv6_parse_win;
	/* init value from XML */
	tpm_db_ctc_cm_ipv6_parse_win_t ipv6_parse_win_original;
	uint32_t ipv6_ety_rule_num;
	tpm_db_cnm_ipv4_pre_table_t ipv4_pre_table;
	tpm_db_cnm_ipv4_pre_filter_t ipv4_pre_filter[TPM_MAX_NUM_UNI_PORTS];
	/* U0~U7 and vir_uni, 8 prec */
	tpm_db_ctc_cm_rule_entry_t cm_rule[TPM_MAX_NUM_UNI_PORTS][TPM_MAX_NUM_CTC_PRECEDENCE];
} tpm_db_ctc_cm_t;

typedef struct {
	bool switch_init;
	bool gmac1_loopback_en;
	bool cpu_wan_loopback_en;
	bool ds_load_bal_en;
	bool switch_active_wan_en;
	tpm_db_ety_dsa_enable_t  ety_dsa_enable;
	tpm_init_virt_uni_t virt_uni_info;

} tpm_db_func_profile_t;

typedef struct {
	tpm_limit_mode_t count_mode;
	uint32_t cir;
	uint32_t cbs;
	uint32_t ebs;
} tpm_db_gmac_lpk_uni_ingr_rate_limit_t;

typedef struct {
	/* Physical chip config */
	tpm_eth_complex_profile_t eth_cmplx_profile;
	tpm_src_port_type_t max_uni_port_nr;
	tpm_db_eth_port_conf_t eth_ports[TPM_MAX_NUM_ETH_PORTS];
	uint32_t num_valid_tcont_llid;
	tpm_db_gmac_tx_t gmac_tx[TPM_MAX_NUM_TX_PORTS];
	tpm_init_fc_params_t	port_fc_conf;
	tpm_db_gmac_rx_t gmac_rx[TPM_MAX_NUM_GMACS];

	/* GMAC Functional Info */
	tpm_db_gmac_func_t gmac_func[TPM_MAX_NUM_GMACS];

	tpm_db_gmac_conn_conf_t gmac_port_conf[TPM_NUM_GMACS];

	/* GMAC Marvell Header Info */
	uint32_t gmac_mh_en[TPM_MAX_NUM_GMACS];

	/* GMAC Buffer Mngmt Pools buffers Info */
	tpm_db_gmac_bufs_t gmac_bp_bufs[TPM_MAX_NUM_GMACS];

	/* IGMP */
	tpm_db_igmp_t igmp_def;

	/* Misc. */
	tpm_db_misc_t init_misc;

	/* Ownership */
	tpm_db_owners_t owners[TPM_MAX_API_TYPES];
#if 0
	tpm_db_api_group_t api_groups[TPM_MAX_API_TYPES];
#endif

	tpm_db_drop_precedence_t drop_precedence;

	/* API Ranges and Entries */
	tpm_db_api_section_t api_section[TPM_MAX_NUM_API_SECTIONS];

	/* Memory place holder array to be used for the api_section api_entry tables */
	tpm_db_api_entry_t api_ent_mem_area[TPM_DB_API_ENTRIES_TBL_SIZE];

	/* PNC Ranges */
	tpm_db_pnc_range_t pnc_range[TPM_MAX_NUM_RANGES];

	/* PNC Shadow Table */
	tpm_db_pnc_shadow_t pnc_shadow[TPM_PNC_SIZE];

	/* EtherType registers */
	tpm_db_etype_sel_t vlan_etype[TPM_NUM_VLAN_ETYPE_REGS];

	/* MH register selection per UNI and SWITCH port */
	tpm_db_tx_ds_mh_sel_mapping_t tpm_mh_port_vector_tbl[TPM_TX_MAX_MH_REGS];

	/* mtu setting */
	tpm_db_mtu_t tpm_mtu_cfg;
	tpm_db_pppoe_add_enable_t tpm_pppoe_add_enable;
	uint32_t num_vlan_tags;
	uint32_t cpu_rx_queue;
	tpm_db_ttl_illegal_action_t ttl_illegal_action;
	tpm_db_tcp_flag_check_t tcp_flag_check;
	tpm_db_mc_ai_bits_t mc_ai_bits_table[TPM_MC_VID_NUM_MAX];

	/* MC IPv6 SIP data */
	tpm_db_ipv6_mc_sip_t ipv6_mc_sip[TPM_MC_IPv6_SIP_NUM_MAX];

	/* MC VID setting */
	tpm_mc_vid_cfg_t mc_vid_port_cfg[TPM_MC_VID_NUM_MAX];

	tpm_init_pnc_last_init_t catch_all_pkt_action;
	uint32_t                 switch_dev_num;

	tpm_db_ipv6_key_shadow_t ipv6_key_shadow;
	tpm_db_ipv6_subflow_shadow_t ipv6_subflow_shadow[TPM_NUM_DIRECTIONS];
	tpm_db_ipv6_5t_flow_shadow_t ipv6_5t_flow_shadow[TPM_DB_IPV6_MAX_5T_FLOW_NUM];

	tpm_db_ipv6_mc_stream_entry_t  ipv6_mc_stream[TPM_MC_MAX_STREAM_NUM];
	tpm_db_split_mod_t   split_mod_conf;

	/* Array indicates which API's are currently called for synch purposes.
	   When API is not in called, invalid=-1 When API is called, it will set it's rule_num */
	int32_t tpm_busy_apis[TPM_MAX_API_TYPES][TPM_MAX_PARALLEL_API_CALLS];
	tpm_db_ctc_cm_t    ctc_cm_data;

	/* func profile, based on chip type and board profile */
	tpm_db_func_profile_t    func_profile;

	/* MC MAC learning pmt mod index */
	uint32_t mac_learn_mod_idx;

	tpm_db_ds_mac_based_trunk_enable_t  ds_mac_based_trunk_enable;

	/* To record GMAC UNI egress rate limit */
	uint32_t gmac_uni_egr_rate_limit[TPM_MAX_NUM_UNI_PORTS];
	/* To record GMAC LPK UNI ingress rate limit */
	tpm_db_gmac_lpk_uni_ingr_rate_limit_t gmac_lpk_uni_ingr_rate_limit[TPM_MAX_NUM_UNI_PORTS];
} tpm_db_t;

typedef struct {
	uint32_t igmp_mode;
	uint8_t mc_stream_pppoe;
	uint8_t src_valid;
	uint16_t vid;
	uint8_t group_addr[4];
	uint8_t src_addr[4];
	uint16_t dest_queue;
	uint32_t dest_port_bm;
	uint32_t u4_entry;
} tpm_db_mc_stream_entry_t;

typedef struct {
	uint8_t mac_addr[6];
	uint8_t user_num[TPM_MAX_NUM_UNI_PORTS];
} tpm_db_mc_mac_entry_t;

typedef struct {
	uint32_t rule_index;
	uint32_t mod_cmd;
} tpm_db_mc_lpbk_entry_t;

#define TPM_DB_MAX_TRANSITION_NUM    (TPM_MAX_NUM_UNI_PORTS)

typedef struct {
	uint8_t valid;
	uint16_t in_vid;
	uint16_t out_vid;
	tpm_db_mc_lpbk_entry_t *lpbk_entry;
} tpm_db_mc_vid_transit_t;

typedef struct {
	uint32_t ip_ver;
	uint16_t mvlan;
	uint16_t xits_num;
	tpm_db_mc_vid_transit_t xits[TPM_DB_MAX_TRANSITION_NUM];
} tpm_db_mc_vlan_entry_t;

/************/
/*  APIs    */
/************/

/* Init */

int32_t tpm_db_init(void);

/* Ethernet Port Config */
bool tpm_db_eth_port_valid(tpm_src_port_type_t src_port_num);
int32_t tpm_db_eth_port_conf_get(tpm_src_port_type_t src_port_num,
				  tpm_db_chip_conn_t *chip_con,
				  tpm_db_int_conn_t *int_con,
				  uint32_t *switch_port);
int32_t tpm_db_eth_port_switch_port_get(uint32_t ext_port);
int32_t tpm_db_get_valid_uni_ports_num(uint32_t *num_ports);
int32_t tpm_db_phy_convert_port_index(int32_t switch_port);
int32_t tpm_db_trg_port_switch_port_get(tpm_trg_port_type_t ext_port);
int32_t tpm_db_eth_port_conf_set(tpm_init_eth_port_conf_t *eth_port_conf);
int32_t tpm_db_gmac_conn_conf_set(tpm_init_gmac_conn_conf_t *gmac_port_conf, uint32_t arr_size);
int32_t tpm_db_gmac_conn_conf_get(tpm_gmacs_enum_t gmac, tpm_init_gmac_conn_conf_t *gmac_port_conf);

/* GMAC Config */
int32_t tpm_db_gmac_tcont_llid_set(uint32_t num_tcont_llid);
int32_t tpm_db_gmac_conn_get(tpm_gmacs_enum_t gmac, tpm_db_gmac_conn_t *gmac_con);
int32_t tpm_db_gmac_mh_en_conf_set(tpm_gmacs_enum_t gmac, uint32_t mh_en);
int32_t tpm_db_gmac_mh_en_conf_get(tpm_gmacs_enum_t gmac, uint32_t *mh_en);
int32_t tpm_db_target_to_gmac(tpm_pnc_trg_t pnc_target, tpm_gmacs_enum_t *gmac);
int32_t tpm_db_gmac_bm_bufs_conf_set(tpm_gmacs_enum_t gmac, uint32_t large_pkt_buffers,
					uint32_t small_pkt_buffers);
int32_t tpm_db_gmac_bm_bufs_conf_get(tpm_gmacs_enum_t gmac, uint32_t *valid, uint32_t *large_pkt_buffers,
					uint32_t *small_pkt_buffers);
int32_t tpm_db_num_tcont_llid_get(uint32_t *num_tcont_llid);

int32_t tpm_db_gmac_func_get(tpm_gmacs_enum_t gmac, tpm_db_gmac_func_t *gfunc);
int32_t tpm_db_gmac_func_set(tpm_gmacs_enum_t gmac, tpm_db_gmac_func_t gfunc);

/* TX Entities and queues Config */
int32_t tpm_db_gmac_tx_q_conf_set(tpm_db_tx_mod_t tx_mod,
					uint32_t queue_num,
					tpm_db_sched_t sched_method,
					tpm_db_txq_owner_t queue_owner,
					uint32_t owner_queue_num, uint32_t queue_size, uint32_t queue_weight);

int32_t tpm_db_gmac_tx_q_conf_get(tpm_db_tx_mod_t tx_mod,
					uint32_t queue_num,
					uint32_t *valid,
					tpm_db_sched_t *sched_method,
					tpm_db_txq_owner_t *queue_owner,
					uint32_t *owner_queue_num, uint32_t *queue_size, uint32_t *queue_weight);

int32_t tpm_db_gmac_tx_val_set(tpm_db_tx_mod_t tx_mod);
uint32_t tpm_db_gmac_tx_val_get(tpm_db_tx_mod_t tx_mod);
uint32_t tpm_db_gmac_lpk_queue_get(tpm_gmacs_enum_t *gmac,
				   uint32_t *queue_idx,
				   tpm_db_gmac1_lpk_queue_type_t queue_type);

/* RX queues Config */
int32_t tpm_db_gmac_rx_val_set(tpm_gmacs_enum_t gmac);
uint32_t tpm_db_gmac_rx_val_get(tpm_gmacs_enum_t gmac);

int32_t tpm_db_gmac_rx_q_conf_set(tpm_gmacs_enum_t gmac, uint32_t queue_num, uint32_t queue_size);

int32_t tpm_db_gmac_rx_q_conf_get(tpm_gmacs_enum_t gmac,
					uint32_t queue_num, uint32_t *valid, uint32_t *queue_size);

/* Get params by Internal Port (WAN, UNI#0, etc). */
int32_t tpm_db_intport_int_con_get(tpm_src_port_type_t int_port_num, tpm_db_int_conn_t *int_con,
					uint32_t *switch_port);

/* Ownership */
int32_t tpm_db_owner_set(uint32_t owner_id);
/* TODO - tpm_db_owner_del(uint32_t owner_id); */
int32_t tpm_db_owner_get_next(int32_t cur_ind, int32_t *next_ind, uint32_t *owner);

#if 0
	int32_t tpm_db_apig_set(uint32_t owner_id, tpm_api_type_t api_type);
/* TODO - tpm_db_apig_get(...) */
	int32_t tpm_db_apig_get_next(int32_t cur_ind, int32_t *next_ind, uint32_t *owner, tpm_api_type_t *api_type);
#endif

/* IGMP */
/*int32_t tpm_db_igmp_set(uint32_t igmp_snoop, uint32_t igmp_cpu_queue);*/
/*int32_t tpm_db_igmp_get(uint32_t *igmp_snoop, uint32_t *igmp_cpu_queue); */
int32_t tpm_db_igmp_set_port_frwd_mode(uint32_t port, uint32_t mode);
int32_t tpm_db_igmp_get_port_frwd_mode(uint32_t port, uint32_t *mode);
int32_t tpm_db_igmp_set_cpu_queue(uint32_t queue);
int32_t tpm_db_igmp_get_cpu_queue(uint32_t *queue);
int32_t tpm_db_igmp_get_snoop_enable(uint32_t *igmp_snoop_enable);
int32_t tpm_db_igmp_set_snoop_enable(uint32_t igmp_snoop_enable);

/* MISC */
int32_t tpm_db_omci_type_set(uint32_t omci_etype);
int32_t tpm_db_omci_type_get(uint32_t *omci_etype);
int32_t tpm_db_omci_channel_get(uint32_t *valid, uint16_t *gemport, uint32_t *cpu_rx_q, uint32_t *cpu_tx_q,
				uint32_t *tcont_id);
int32_t tpm_db_omci_channel_set(uint16_t gemport, uint32_t cpu_rx_q, uint32_t cpu_tx_q, uint32_t tcont_id);
int32_t tpm_db_omci_channel_remove(void);

int32_t tpm_db_oam_channel_get(uint32_t *valid, uint32_t *cpu_rx_q, uint32_t *cpu_tx_q, uint32_t *llid);
int32_t tpm_db_oam_channel_set(uint32_t cpu_rx_q, uint32_t cpu_tx_q, uint32_t llid);
int32_t tpm_db_oam_channel_remove(void);

int32_t tpm_db_oam_loopback_state_get(uint32_t *loopback);
int32_t tpm_db_oam_loopback_channel_set(void);
int32_t tpm_db_oam_loopback_channel_remove(void);

int32_t tpm_db_pon_type_set(tpm_db_pon_type_t pon_type);
int32_t tpm_db_pon_type_get(tpm_db_pon_type_t *pon_type);

int32_t tpm_db_ds_mh_set_conf_set(tpm_db_mh_src_t ds_mh_set_conf);
int32_t tpm_db_ds_mh_get_conf_set(tpm_db_mh_src_t *ds_mh_set_conf);

int32_t tpm_db_active_wan_set(tpm_gmacs_enum_t active_wan);
tpm_gmacs_enum_t tpm_db_active_wan_get(void);

#if 0				/*Keep to be added in future version */
	int32_t tpm_db_pnc_init_deb_port_set(uint32_t pnc_init_debug_port);
	int32_t tpm_db_pnc_init_deb_port_get(uint32_t *pnc_init_debug_port);
#endif
int32_t tpm_db_init_done_set(void);
int32_t tpm_db_init_done_get(void);

int32_t tpm_db_cfg_pnc_parse_set(tpm_init_cfg_pnc_parse_t cfg_pnc_parse);
int32_t tpm_db_cfg_pnc_parse_get(tpm_init_cfg_pnc_parse_t *cfg_pnc_parse_val);

int32_t tpm_db_cpu_loopback_set(tpm_init_cpu_loopback_t cpu_loopback);
int32_t tpm_db_cpu_loopback_get(tpm_init_cpu_loopback_t *cpu_loopback_val);

int32_t tpm_db_double_tag_support_set(tpm_init_double_tag_t dbl_tag);
int32_t tpm_db_double_tag_support_get(tpm_init_double_tag_t *dbl_tag);

#if 0
int32_t tpm_db_default_tag_tpid_set(uint16_t vlan1_tpid, uint16_t vlan2_tpid);
int32_t tpm_db_default_tag_tpid_get(uint16_t *vlan1_tpid, uint16_t *vlan2_tpid);
#endif
int32_t tpm_db_default_tag_tpid_set(tpm_init_tpid_opt_t *tag_tpid);
int32_t tpm_db_default_tag_tpid_get(tpm_init_tpid_opt_t *tag_tpid);

int32_t tpm_db_virt_info_set(tpm_init_virt_uni_t virt_uni_info);
int32_t tpm_db_virt_info_get(tpm_init_virt_uni_t *virt_uni_info);

int32_t tpm_db_ipv6_5t_enable_set(tpm_init_ipv6_5t_enable_t ipv6_5t_enable);
int32_t tpm_db_ipv6_5t_enable_get(tpm_init_ipv6_5t_enable_t *ipv6_5t_enable);

int32_t tpm_db_port_vector_tbl_info_get(uint32_t reg_num, uint32_t *uni_vector, uint16_t *amber_port_vector,
					uint32_t *pnc_vector);
int32_t tpm_db_port_vector_tbl_info_set(uint32_t reg_num, uint32_t un_vector, uint16_t amber_port_vector,
					uint32_t pnc_vector);
int32_t tpm_db_port_vector_tbl_info_search(tpm_trg_port_type_t  trg_port, uint32_t *uni_vector,
					    uint16_t *amber_port_vector, uint32_t *pnc_vector);
/* API Section */
int32_t tpm_db_api_section_init(tpm_api_sections_t api_section, tpm_pnc_ranges_t prim_pnc_range,
				uint32_t api_rng_size);
int32_t tpm_db_api_section_invalidate(tpm_api_sections_t api_section);

int32_t tpm_db_api_section_get_from_api_type(tpm_api_type_t api_type, tpm_api_sections_t *api_section);
int32_t tpm_db_api_type_get_from_api_section(tpm_api_sections_t api_section, tpm_api_type_t *api_type);
int32_t tpm_db_api_section_reset(tpm_api_sections_t api_section);
int32_t tpm_db_api_section_get(tpm_api_sections_t api_section, uint32_t *api_rng_size,
				uint32_t *num_valid_entries, tpm_pnc_ranges_t * prim_pnc_range,
				int32_t *last_valid, uint32_t *tbl_start);
int32_t tpm_db_api_section_lastentry_get(tpm_api_sections_t api_section, int32_t *last_valid_entry);
int32_t tpm_db_api_section_main_pnc_get(tpm_api_sections_t api_section, tpm_pnc_ranges_t *prim_pnc_range);
int32_t tpm_db_api_section_num_entries_get(tpm_api_sections_t api_section, uint32_t *num_entries);
int32_t tpm_db_api_section_val_get_next(int32_t cur_api_section, int32_t *next_section,
					uint32_t *api_rng_size, uint32_t *num_valid_entries,
					int32_t *last_valid_entry, uint32_t * tbl_start);
int32_t tpm_db_api_section_ent_tbl_get(tpm_api_sections_t api_sec,
					tpm_db_api_entry_t *api_ent_mem_area,
					uint32_t index);


int32_t tpm_db_api_section_entry_add(tpm_api_sections_t api_section, int32_t new_last_valid);
int32_t tpm_db_api_entries_area_reset(void);

int32_t tpm_db_drop_precedence_mode_set(tpm_drop_precedence_t mode);
int32_t tpm_db_drop_precedence_mode_get(tpm_drop_precedence_t *mode);
int32_t tpm_db_drop_precedence_rule_set(uint32_t rule_index);
int32_t tpm_db_drop_precedence_rule_delete(uint32_t rule_index);
int32_t tpm_db_drop_precedence_rule_get(uint32_t index,
	tpm_db_drop_precedence_rule_entry_t *rule_entry);
int32_t tpm_db_drop_precedence_db_reset(void);

/* API Entry */
int32_t tpm_db_api_entry_set(tpm_api_sections_t api_section,
				uint32_t rule_num, uint32_t bi_dir, tpm_rule_entry_t *api_data,
				tpm_db_mod_conn_t *mod_con, tpm_db_pnc_conn_t *pnc_con, uint32_t *rule_idx);

/*TODO: int32_t tpm_db_api_entry_del(tpm_api_sections_t api_section, uint32_t dir, uint32_t rule_num); */
int32_t tpm_db_api_entry_rulenum_inc(tpm_api_sections_t api_section, uint32_t rule_inc_start,
					uint32_t rule_inc_end);
int32_t tpm_db_api_entry_rulenum_dec(tpm_api_sections_t api_section, uint32_t rule_dec_start,
					uint32_t rule_dec_end);
int32_t tpm_db_api_entry_get(tpm_api_sections_t api_section, uint32_t rule_num, uint32_t *rule_idx,
				uint32_t *bi_dir, tpm_rule_entry_t *api_data, tpm_db_mod_conn_t *mod_con,
				tpm_db_pnc_conn_t *pnc_con);

int32_t tpm_db_api_tcam_num_get(tpm_api_sections_t api_section, uint32_t rule_idx, uint32_t *tcam_num);
int32_t tpm_db_api_entry_update_rule_idx(tpm_api_sections_t api_section,
			     			uint32_t rule_idx_pre,
			     			uint32_t rule_idx_new);
int32_t tpm_db_api_rulenum_get(tpm_api_sections_t api_section, uint32_t rule_idx, uint32_t *rule_num);
int32_t tpm_db_api_rulenum_get_from_l2_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
						tpm_l2_acl_key_t *l2_key, uint32_t *rule_num);
int32_t tpm_db_api_rulenum_get_from_l3_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
						tpm_l3_type_key_t *l3_key, uint32_t *rule_num);
int32_t tpm_db_api_rulenum_get_from_ipv4_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
						tpm_ipv4_acl_key_t *ipv4_key, uint32_t *rule_num);
int32_t tpm_db_api_rulenum_get_from_ipv6_key(tpm_api_sections_t api_section, tpm_parse_fields_t parse_rule_bm,
						tpm_ipv6_acl_key_t *ipv6_key, uint32_t *rule_num);

int32_t tpm_db_api_entry_val_get_next(tpm_api_sections_t api_section, int32_t cur_rule, int32_t *next_rule,
					uint32_t *rule_idx, uint32_t *bi_dir, tpm_rule_entry_t *api_data,
					tpm_db_mod_conn_t *mod_con, tpm_db_pnc_conn_t *pnc_con);
int32_t tpm_db_rule_index_set(uint32_t rule_index);
int32_t tpm_db_api_entry_invalidate(tpm_api_sections_t api_section, uint32_t rule_num);
int32_t tpm_db_api_entry_ind_get(tpm_api_sections_t api_section, uint32_t rule_num, int32_t *index);
int32_t tpm_db_api_tcam_rule_idx_get(tpm_api_sections_t api_section, uint32_t tcam_num, uint32_t *rule_idx);

int32_t tpm_db_pnc_conn_inc(tpm_api_sections_t api_section, uint32_t rule_inc_start, uint32_t rule_inc_end);
int32_t tpm_db_pnc_conn_dec(tpm_api_sections_t api_section, uint32_t rule_dec_start, uint32_t rule_dec_end);
int32_t tpm_db_pnc_get_lu_conf(tpm_pnc_ranges_t range_id, tpm_api_lu_conf_t *lu_conf);

int32_t tpm_db_api_specific_pnc_inc(tpm_api_sections_t api_section,
					tpm_pnc_ranges_t range_id, uint32_t rule_inc_start, uint32_t rule_inc_end);
int32_t tpm_db_api_specific_pnc_dec(tpm_api_sections_t api_section,
					tpm_pnc_ranges_t range_id, uint32_t rule_inc_start, uint32_t rule_inc_end);
/* PnC Range */
/*Set */
int32_t tpm_db_pnc_rng_create(tpm_db_pnc_range_conf_t *pnc_range_conf);
int32_t tpm_db_pnc_rng_delete(tpm_pnc_ranges_t pnc_range);
int32_t tpm_db_pnc_rng_reset(tpm_pnc_ranges_t pnc_range);
int32_t tpm_db_pnc_rng_free_ent_inc(tpm_pnc_ranges_t pnc_range);
int32_t tpm_db_pnc_rng_free_ent_dec(tpm_pnc_ranges_t pnc_range);
int32_t tpm_db_pnc_rng_api_start_inc(tpm_pnc_ranges_t pnc_range);
int32_t tpm_db_pnc_rng_api_end_dec(tpm_pnc_ranges_t pnc_range);
/*Get */
int32_t tpm_db_pnc_rng_get(tpm_pnc_ranges_t pnc_range, tpm_db_pnc_range_t *range_data);
int32_t tpm_db_pnc_rng_get_range_start_end(tpm_pnc_ranges_t pnc_range, uint32_t *range_start,
						uint32_t *range_end);

/*int32_t tpm_db_pnc_rng_tbl_val_get_next(int32_t cur_ind, int32_t *next_ind, tpm_db_pnc_range_t *range_data); */
int32_t tpm_db_pnc_rng_val_get_next(tpm_pnc_ranges_t cur_range, tpm_pnc_ranges_t *next_range,
					tpm_db_pnc_range_t *range_data);
int32_t tpm_db_pnc_rng_conf_get(tpm_pnc_ranges_t range_id, tpm_db_pnc_range_conf_t *range_conf);
int32_t tpm_db_pnc_rng_oper_get(tpm_pnc_ranges_t range_id, tpm_db_pnc_range_oper_t *range_oper);
int32_t tpm_db_pnc_rng_free_ent_get(tpm_pnc_ranges_t range_id, uint32_t *free_entries);

/* PnC Shadow */
int32_t tpm_db_pnc_shdw_ent_set(uint32_t pnc_entry, tpm_pnc_all_t *pnc_data);
int32_t tpm_db_pnc_shdw_ent_inv(uint32_t pnc_entry);
int32_t tpm_db_pnc_shdw_ent_ins(uint32_t pnc_start_entry, uint32_t pnc_stop_entry);
int32_t tpm_db_pnc_shdw_ent_del(uint32_t pnc_start_entry, uint32_t pnc_stop_entry);
int32_t tpm_db_pnc_shdw_ent_get(uint32_t pnc_entry, uint32_t *valid, tpm_pnc_all_t *pnc_data);

/* Mod2 Shadow */
int32_t tpm_db_mod2_set_jump_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id, tpm_mod2_entry_t *pattern);
int32_t tpm_db_mod2_activate_jump_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id);
int32_t tpm_db_mod2_get_jump_entry_pattern_data(tpm_gmacs_enum_t gmac_port, uint16_t entry_id,
						tpm_mod2_entry_t *pattern);
int32_t tpm_db_mod2_del_jump_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id);
int32_t tpm_db_mod2_get_chain_entry_num(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					uint16_t chain_id, uint16_t *entry_num);
int32_t tpm_db_mod2_get_chain_entry_pattern_data(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
							uint16_t chain_id, uint16_t pattern_id,
							tpm_mod2_entry_t *pattern);
int32_t tpm_db_mod2_del_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id);
int32_t  tpm_db_mod2_flush_jump_area(tpm_gmacs_enum_t gmac_port, uint16_t entry_id,
				     uint8_t updt_next_free, uint8_t on_failure);
int32_t  tpm_db_mod2_flush_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id,
				       uint8_t updt_next_free, uint8_t on_failure);
int32_t tpm_db_mod2_set_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type, uint16_t chain_id,
					uint16_t entry_num, tpm_mod2_entry_t *pattern);
int32_t tpm_db_mod2_activate_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
						uint16_t chain_id);
int32_t tpm_db_mod2_activate_chain_pmt_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
						uint16_t chain_id);
int32_t tpm_db_mod2_if_chain_pmt_entries_exist(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
						uint16_t chain_id, uint8_t *exist);
uint16_t tpm_db_mod2_get_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					uint16_t entry_num, tpm_mod2_entry_t *pattern);
uint16_t tpm_db_mod2_get_next_free_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type);
uint16_t tpm_db_mod2_convert_chain_to_pmt_entry(tpm_chain_type_t chain_type, uint16_t chain_id);
int32_t tpm_db_mod2_get_chain_id_by_pmt_entry(tpm_gmacs_enum_t gmac_port, uint16_t entry_id,
						tpm_chain_type_t *chain_type, uint16_t *chain_id);
uint16_t tpm_db_mod2_get_next_free_jump_entry(tpm_gmacs_enum_t gmac_port);
uint8_t tpm_db_mod2_rollback_chain_entry(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					 uint16_t chain_id, uint8_t on_failure);

uint16_t tpm_db_mod2_get_next_split_mod_free_jump_entry(tpm_gmacs_enum_t gmac_port, tpm_pkt_mod_t *mod_data);
//void tpm_db_mod2_update_split_mod_next_free_jump_entry(tpm_gmacs_enum_t gmac_port);
int32_t tpm_db_mod2_split_mod_insert_vlan(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data);
int32_t tpm_db_mod2_split_mod_get_vlan_index(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data, uint32_t *index);
int32_t tpm_db_mod2_split_mod_increase_vlan_user_num(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data);
int32_t tpm_db_mod2_split_mod_decrease_vlan_user_num(tpm_gmacs_enum_t port,
													 uint32_t       vlan_index,
													 uint32_t      *user_num);
void tpm_db_mod2_set_multicast_mh_state(uint8_t enable);
void tpm_db_mod2_set_pppoe_add_mod_state(uint8_t enable);
void tpm_db_mod2_set_double_tag_state(uint8_t enable);
void tpm_db_mod2_set_udp_checksum_use_init_bm_state(uint8_t enable);
void tpm_db_mod2_set_udp_checksum_state(uint8_t enable);
void tpm_db_mod2_set_chain_num(tpm_chain_type_t chain_type, uint16_t chain_num);
uint16_t tpm_db_mod2_get_chain_max_size(tpm_chain_type_t chain_type);
uint8_t tpm_db_mod2_get_multicast_mh_state(void);
uint8_t tpm_db_mod2_get_pppoe_add_mod_state(void);
uint8_t tpm_db_mod2_get_double_tag_state(void);
uint8_t tpm_db_mod2_get_udp_checksum_init_bm_state(void);
uint8_t tpm_db_mod2_get_udp_checksum_state(void);
int32_t tpm_db_mod2_setup(void);
int32_t tpm_db_mod2_inv_mac_entries(tpm_gmacs_enum_t gmac_port);

void tpm_db_mod2_show_jump_pmt_entries(tpm_gmacs_enum_t gmac_port);
void tpm_db_mod2_show_jump_cfg_data(void);
void tpm_db_mod2_show_chain_pmt_entries(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type);
void tpm_db_mod2_show_chain_info_entries(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
						uint16_t chain_id);
void tpm_db_mod2_show_chain_cfg_data(tpm_chain_type_t chain_type);
void tpm_db_mod2_show_all_chain(tpm_gmacs_enum_t gmac_port);

/* Vlan Ethertype Shadow */
int32_t tpm_db_mod_etype_sel_get(uint32_t tpid, uint32_t *etype_sel);
int32_t tpm_db_mod_etype_sel_free_get(uint32_t *etype_sel);
int32_t tpm_db_mod_etype_sel_set(uint32_t tpid, uint32_t etype_sel);

/* Multicast Shadow */
void tpm_db_set_mc_filter_mode(uint32_t mode);
void tpm_db_set_mc_pppoe_enable(uint32_t enable);
void tpm_db_split_mod_set_enable(tpm_db_split_mod_enable_t enable);
tpm_db_split_mod_enable_t tpm_db_split_mod_get_enable(void);
void tpm_db_split_mod_set_num_vlans(uint32_t num_vlans);
uint32_t tpm_db_split_mod_get_num_vlans(void);
void tpm_db_split_mod_set_mode(tpm_db_split_mod_mode_t mode);
tpm_db_split_mod_mode_t tpm_db_split_mod_get_mode(void);
void tpm_db_split_mod_set_num_p_bits(uint32_t num_p_bits, uint8_t  p_bits[8]);
uint32_t tpm_db_split_mod_get_num_p_bits(void);
int32_t tpm_db_split_mod_get_p_bits_by_index(uint8_t  index, uint8_t  *p_bits);
int32_t tpm_db_split_mod_get_index_by_p_bits(uint8_t  p_bits, uint32_t *index);
void tpm_db_set_mc_per_uni_vlan_xlate(uint32_t enable);
void tpm_db_set_mc_hwf_queue(uint8_t hwf_queue);
void tpm_db_set_mc_cpu_queue(uint8_t cpu_queue);
void tpm_db_set_mc_igmp_proxy_sa_mac(uint8_t *sa_mac);
uint32_t tpm_db_get_mc_filter_mode(void);
uint8_t tpm_db_get_mc_pppoe_enable(void);
uint8_t tpm_db_get_mc_per_uni_vlan_xlate(void);
uint8_t tpm_db_get_mc_hwf_queue(void);
uint8_t tpm_db_get_mc_cpu_queue(void);
void tpm_db_get_mc_igmp_proxy_sa_mac(uint8_t *sa_mac, uint8_t *valid);
int32_t tpm_db_set_mc_stream_entry(uint32_t stream_num, tpm_db_mc_stream_entry_t *mc_stream);
int32_t tpm_db_get_mc_stream_entry(uint32_t stream_num, tpm_db_mc_stream_entry_t *mc_stream);
void tpm_db_reset_mc_stream_entry(uint32_t stream_num);
int32_t tpm_db_set_ipv6_mc_stream_entry(uint32_t stream_num, tpm_db_ipv6_mc_stream_entry_t *mc_stream);
int32_t tpm_db_get_ipv6_mc_stream_entry(uint32_t stream_num, tpm_db_ipv6_mc_stream_entry_t *mc_stream);
void tpm_db_reset_ipv6_mc_stream_entry(uint32_t stream_num);
int32_t tpm_db_get_mc_mac_port_bm(uint8_t *mac_addr, uint32_t *uni_port_bm);
int32_t tpm_db_increase_mc_mac_port_user_num(uint8_t *mac_addr, uint32_t uni_port);
int32_t tpm_db_decrease_mc_mac_port_user_num(uint8_t *mac_addr, uint32_t uni_port);
uint8_t tpm_db_get_mc_mac_port_user_num(uint8_t *mac_addr, uint32_t uni_port);
void tpm_db_reset_mc_mac_entry(uint8_t *mac_addr);
uint32_t tpm_db_mc_lpbk_entries_num_get(void);
int32_t tpm_db_mc_lpbk_entry_set(uint32_t ip_ver, uint16_t mvlan, uint16_t in_vid, uint16_t out_vid, uint32_t rule_num,
					uint32_t mod_entry);
int32_t tpm_db_mc_vlan_xits_num_get(uint32_t ip_ver, uint16_t mvlan, uint32_t *xits_num);
int32_t tpm_db_mc_vlan_xit_entry_get(uint32_t ip_ver, uint16_t mvlan, uint32_t xit_id, uint32_t *rule_index,
					uint32_t *mod_entry);
int32_t tpm_db_mc_lpbk_rule_num_get(uint32_t rule_index, uint32_t *rule_num);
int32_t tpm_db_mc_lpbk_entry_invalidate(uint32_t rule_num);
int32_t tpm_db_mc_alloc_virt_uni_entry(uint32_t *entry_id);
int32_t tpm_db_mc_free_virt_uni_entry(uint32_t entry_id);
int32_t tpm_db_mc_vlan_set_ai_bit_pnc_index(uint32_t mc_vlan, uint32_t l2_pnc_rule_index);
int32_t tpm_db_mc_vlan_get_ai_bit_by_vid(uint32_t mc_vlan, uint32_t *ai_bit);
int32_t tpm_db_mc_vlan_reset_ai_bit(uint32_t l2_pnc_rule_index);
int32_t tpm_db_mc_vlan_get_pnc_index_free_slot(uint32_t mc_vlan, uint32_t rule_num);
int32_t tpm_db_mc_vlan_get_ai_bit(uint32_t mc_vlan, uint32_t *ai_bit);
int32_t tpm_db_mc_vlan_reset_mc_vlan(uint32_t mc_vlan);
uint32_t tpm_db_remove_mc_vid_cfg(uint32_t mc_vid);
bool tpm_db_mc_vid_exist(uint32_t mc_vid);
int32_t tpm_db_set_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_vid_set_t *mc_vid_uniports_config);
int32_t tpm_db_get_mc_vid_cfg(uint32_t mc_vid, tpm_mc_vid_port_cfg_t **mc_vid_uniports_config);
void     tpm_db_mc_rule_reset(void);
void     tpm_db_mc_cfg_reset(void);
void tpm_db_set_mc_lpbk_enable(tpm_ip_ver_t ver, uint8_t enable);
void tpm_db_get_mc_lpbk_enable(tpm_ip_ver_t ver, uint8_t *enable);

/* MTU shadow */
int32_t tpm_db_mtu_set_ipv4_mtu_us(uint32_t mtu);
int32_t tpm_db_mtu_get_ipv4_mtu_us(uint32_t *mtu);
int32_t tpm_db_mtu_set_ipv4_pppoe_mtu_us(uint32_t mtu);
int32_t tpm_db_mtu_get_ipv4_pppoe_mtu_us(uint32_t *mtu);
int32_t tpm_db_mtu_set_ipv6_mtu_us(uint32_t mtu);
int32_t tpm_db_mtu_get_ipv6_mtu_us(uint32_t *mtu);
int32_t tpm_db_mtu_set_ipv6_pppoe_mtu_us(uint32_t mtu);
int32_t tpm_db_mtu_get_ipv6_pppoe_mtu_us(uint32_t *mtu);
int32_t tpm_db_mtu_set_ipv4_mtu_ds(uint32_t mtu);
int32_t tpm_db_mtu_get_ipv4_mtu_ds(uint32_t *mtu);
int32_t tpm_db_mtu_set_ipv6_mtu_ds(uint32_t mtu);
int32_t tpm_db_mtu_get_ipv6_mtu_ds(uint32_t *mtu);
int32_t tpm_db_get_pppoe_add_enable(tpm_db_pppoe_add_enable_t *enable);
int32_t tpm_db_set_pppoe_add_enable(tpm_db_pppoe_add_enable_t enable);
int32_t tpm_db_get_num_vlan_tags(uint32_t *number);
int32_t tpm_db_set_num_vlan_tags(uint32_t number);

int32_t tpm_db_get_mtu_enable(tpm_db_mtu_setting_enable_t *enable);
int32_t tpm_db_set_mtu_enable(tpm_db_mtu_setting_enable_t enable);

int32_t tpm_db_set_cpu_rx_queue(uint32_t cpu_rx_queue);
int32_t tpm_db_get_cpu_rx_queue(uint32_t *cpu_rx_queue);
int32_t tpm_db_set_ttl_illegal_action(uint32_t ttl_illegal_action);
int32_t tpm_db_get_ttl_illegal_action(uint32_t *ttl_illegal_action);
int32_t tpm_db_set_tcp_flag_check(uint32_t tcp_flag_check);
int32_t tpm_db_get_tcp_flag_check(uint32_t *tcp_flag_check);
int32_t tpm_db_set_mru(uint32_t mru);
int32_t tpm_db_get_mru(uint32_t *mru);
int32_t tpm_db_set_catch_all_pkt_action(tpm_init_pnc_last_init_t catch_all_pkt_action);
int32_t tpm_db_get_catch_all_pkt_action(tpm_init_pnc_last_init_t *catch_all_pkt_action);
int32_t tpm_db_set_switch_dev_num(int32_t switch_dev_num);
int32_t tpm_db_get_switch_dev_num(int32_t *switch_dev_num);
int32_t tpm_db_set_ety_dsa_enable(tpm_db_ety_dsa_enable_t ety_dsa_enable);
int32_t tpm_db_get_ety_dsa_enable(tpm_db_ety_dsa_enable_t *ety_dsa_enable);

/* IPV6 5T flow shadow */
uint32_t tpm_db_find_matched_ipv6_l4_subflow(uint32_t src_dir, uint32_t parse_bm, uint32_t protocol, tpm_l4_ports_key_t *l4_key);
uint32_t tpm_db_get_ipv6_l4_subflow_user_num(uint32_t src_dir, uint32_t flow_id);
uint32_t tpm_db_get_free_ipv6_l4_subflow(uint32_t src_dir);
int32_t  tpm_db_set_ipv6_l4_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t parse_bm, uint32_t protocol, tpm_l4_ports_key_t *l4_key);
int32_t  tpm_db_get_ipv6_l4_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t *parse_bm, uint32_t *protocol, tpm_l4_ports_key_t *l4_key);
int32_t  tpm_db_del_ipv6_l4_subflow(uint32_t src_dir, uint32_t flow_id);

uint32_t tpm_db_find_matched_ipv6_gen_subflow(uint32_t src_dir, uint32_t parse_bm, uint32_t protocol, tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key);
uint32_t tpm_db_get_ipv6_gen_subflow_user_num(uint32_t src_dir, uint32_t flow_id);
uint32_t tpm_db_get_free_ipv6_gen_subflow(uint32_t src_dir);
int32_t  tpm_db_set_ipv6_gen_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t parse_bm, uint32_t protocol, tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key);
int32_t  tpm_db_get_ipv6_gen_subflow(uint32_t src_dir, uint32_t flow_id, uint32_t *parse_bm, uint32_t *protocol, tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key);
int32_t  tpm_db_del_ipv6_gen_subflow(uint32_t src_dir, uint32_t flow_id);

uint32_t tpm_db_find_matched_ipv6_5t_flow(uint32_t src_dir, uint32_t parse_bm, uint32_t is_pppoe, uint32_t protocol, tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key, tpm_ipv6_addr_key_t *dip_key);
int32_t  tpm_db_set_ipv6_5t_flow(uint32_t src_dir, uint32_t parse_bm, uint32_t is_pppoe, uint32_t protocol, tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key, tpm_ipv6_addr_key_t *dip_key);
int32_t  tpm_db_get_ipv6_5t_flow(uint32_t flow_id, uint32_t *src_dir, uint32_t *parse_bm, uint32_t *is_pppoe, uint32_t *protocol, tpm_l4_ports_key_t *l4_key, tpm_ipv6_gen_acl_key_t *gen_key, tpm_ipv6_addr_key_t *dip_key);
int32_t  tpm_db_del_ipv6_5t_flow(uint32_t flow_id);
int32_t  tpm_db_init_ipv6_5t_flow_reset(void);

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
			       uint32_t rule_index);
int32_t tpm_db_ctc_cm_rule_delete(tpm_src_port_type_t src_port, uint32_t precedence);
int32_t tpm_db_ctc_cm_rule_get(tpm_src_port_type_t src_port, uint32_t precedence, tpm_db_ctc_cm_rule_entry_t *cm_rule);
int32_t tpm_db_ctc_cm_get_num_rules(tpm_src_port_type_t src_port);
int32_t tpm_db_ctc_cm_get_ipv6_num_rules(void);

int32_t tpm_db_ctc_cm_db_reset(void);
int32_t tpm_db_ipv6_mc_sip_db_reset(void);
int32_t tpm_db_ipv6_mc_sip_index_get(uint8_t  *src_ip);
int32_t tpm_db_ipv6_mc_sip_free_slot_num_get(void);
int32_t tpm_db_ipv6_mc_sip_add(uint8_t  *src_ip);
int32_t tpm_db_ipv6_mc_sip_ref_num_inc(uint8_t  src_ip_index);
int32_t tpm_db_ipv6_mc_sip_ref_num_dec(uint8_t  src_ip_index, uint8_t  *new_ref_num);

int32_t tpm_db_ctc_cm_enable_set(tpm_db_ctc_cm_enable_t ctc_cm_enable);
int32_t tpm_db_ctc_cm_enable_get(tpm_db_ctc_cm_enable_t *ctc_cm_enable);

int32_t tpm_db_ctc_cm_ipv6_parse_win_set(tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win);
int32_t tpm_db_ctc_cm_ipv6_parse_win_orig_set(tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win);
int32_t tpm_db_ctc_cm_ipv6_parse_win_get(tpm_db_ctc_cm_ipv6_parse_win_t *ctc_cm_ipv6_parse_win);
int32_t tpm_db_ctc_cm_ipv6_ety_rule_num_set(uint32_t ipv6_ety_rule_num);
int32_t tpm_db_ctc_cm_ipv6_ety_rule_num_get(uint32_t *ipv6_ety_rule_num);

uint32_t tpm_db_get_api_busy(tpm_api_type_t api_type);
uint32_t tpm_db_get_api_rule_num_busy(tpm_api_type_t api_type, uint32_t rule_num);
void     tpm_db_get_api_all_busy(tpm_api_type_t api_type, uint32_t *num_busy, uint32_t *rule_num_list);
uint32_t tpm_db_set_api_busy(tpm_api_type_t api_type, uint32_t rule_num);
uint32_t tpm_db_set_api_free(tpm_api_type_t api_type, uint32_t rule_num);

void tpm_db_show_mtu_cfg(void);

int32_t tpm_db_eth_cmplx_profile_set(tpm_eth_complex_profile_t eth_cmplx_profile);
tpm_eth_complex_profile_t tpm_db_eth_cmplx_profile_get(void);
int32_t tpm_db_mac_func_set(void);
int32_t tpm_db_eth_max_uni_port_set(void);
tpm_src_port_type_t tpm_db_trg_port_uni_any_bmp_get(bool);
int32_t tpm_db_to_lan_gmac_get(tpm_trg_port_type_t trg_port, tpm_pnc_trg_t *pnc_target);
int32_t tpm_db_src_gmac_func_get(tpm_src_port_type_t src_port, tpm_db_gmac_func_t *gmac_func);
int32_t tpm_db_src_port_on_switch(tpm_src_port_type_t src_port);
bool    tpm_db_gmac_valid(tpm_gmacs_enum_t gmac);

int32_t tpm_db_add_ipv4_pre_rule(tpm_src_port_type_t src_port, uint32_t key_pattern, uint32_t *rule_num);
int32_t tpm_db_del_ipv4_pre_rule(tpm_src_port_type_t src_port, uint32_t key_pattern, uint32_t *rule_num);
int32_t tpm_db_get_ipv4_pre_rule_num(uint32_t *num_rules);
int32_t tpm_db_alloc_ipv4_pre_filter_key(tpm_src_port_type_t src_port, uint32_t *key_idx);
int32_t tpm_db_set_ipv4_pre_filter_key(tpm_src_port_type_t src_port, uint32_t key_idx, tpm_parse_fields_t parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_db_del_ipv4_pre_filter_key(tpm_src_port_type_t src_port, uint32_t key_idx);
int32_t tpm_db_get_ipv4_pre_filter_key_num(tpm_src_port_type_t src_port, uint32_t *num_keys);
int32_t tpm_db_get_ipv4_pre_filter_key(tpm_src_port_type_t src_port, uint32_t key_idx, tpm_parse_fields_t *parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key);
int32_t tpm_db_find_ipv4_pre_filter_key(tpm_src_port_type_t src_port, tpm_parse_fields_t parse_rule_bm, tpm_ipv4_acl_key_t *ipv4_key, uint32_t *key_idx);
int32_t tpm_db_inc_ipv4_pre_filter_key_user_num(tpm_src_port_type_t src_port, uint32_t key_idx);
int32_t tpm_db_dec_ipv4_pre_filter_key_user_num(tpm_src_port_type_t src_port, uint32_t key_idx);
int32_t tpm_db_get_ipv4_pre_filter_key_user_num(tpm_src_port_type_t src_port, uint32_t key_idx, uint32_t *num_users);
int32_t tpm_db_set_gmac_rate_limit(tpm_db_tx_mod_t gmac_i,
				   uint32_t bucket_size,
				   uint32_t rate_limit,
				   uint32_t prio);
int32_t tpm_db_get_gmac_rate_limit(tpm_gmacs_enum_t gmac_i,
				   uint32_t *bucket_size,
				   uint32_t *rate_limit,
				   uint32_t *prio);
int32_t tpm_db_set_gmac_q_rate_limit(tpm_db_tx_mod_t gmac_i,
				   uint32_t queue,
				   uint32_t bucket_size,
				   uint32_t rate_limit,
				   uint32_t wrr);
int32_t tpm_db_get_gmac_q_rate_limit(tpm_gmacs_enum_t gmac_i,
				   uint32_t queue,
				   uint32_t *bucket_size,
				   uint32_t *rate_limit,
				   uint32_t *wrr);
int32_t tpm_db_fc_conf_set(tpm_init_fc_params_t *port_fc_conf);
int32_t tpm_db_fc_conf_get(tpm_init_fc_params_t *port_fc_conf);
/* switch_init */
int32_t tpm_db_switch_init_set(uint32_t switch_init);
int32_t tpm_db_switch_init_get(uint32_t *switch_init);
int32_t tpm_db_pnc_mac_learn_enable_set(tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable);
int32_t tpm_db_pnc_mac_learn_enable_get(tpm_init_pnc_mac_learn_enable_t *pnc_mac_learn_enable);

bool tpm_db_gmac1_lpbk_en_get(void);
void tpm_db_gmac1_lpbk_en_set(bool en);
bool tpm_db_cpu_wan_lpbk_en_get(void);
void tpm_db_cpu_wan_lpbk_en_set(bool en);
bool tpm_db_ds_load_bal_en_get(void);
void tpm_db_ds_load_bal_en_set(bool en);
bool tpm_db_switch_active_wan_en_get(void);
void tpm_db_switch_active_wan_en_set(bool en);
int32_t tpm_db_api_data_backup(void);
int32_t tpm_db_api_data_rcvr(void);
int32_t tpm_db_api_section_bak_num_entries_get(tpm_api_sections_t api_section, uint32_t *num_entries);
int32_t tpm_db_api_section_bak_ent_tbl_get(tpm_api_sections_t api_sec, tpm_db_api_entry_t *api_ent_mem_area, uint32_t index);
int32_t tpm_db_api_entry_bak_get_next(tpm_api_sections_t api_section,
				      int32_t cur_rule,
				      int32_t *next_rule);
int32_t tpm_db_wan_lan_rate_limit_exchange(void);

int32_t tpm_db_mac_learn_mod_idx_set(uint32_t mod_idx);
int32_t tpm_db_mac_learn_mod_idx_get(uint32_t *mod_idx);
int32_t tpm_db_ds_mac_based_trunk_enable_set(tpm_db_ds_mac_based_trunk_enable_t enable);
int32_t tpm_db_ds_mac_based_trunk_enable_get(tpm_db_ds_mac_based_trunk_enable_t *enable);

int32_t tpm_db_max_uni_port_nr_get(uint32_t *max_uni_port_nr);

int32_t tpm_db_gmac_uni_egr_rate_limit_set(tpm_src_port_type_t port, uint32_t rate_limit);
int32_t tpm_db_gmac_uni_egr_rate_limit_get(tpm_src_port_type_t port, uint32_t *rate_limit);
int32_t tpm_db_gmac_lpk_uni_ingr_rate_limit_set(tpm_src_port_type_t port, tpm_db_gmac_lpk_uni_ingr_rate_limit_t rate_limit);
int32_t tpm_db_gmac_lpk_uni_ingr_rate_limit_get(tpm_src_port_type_t port, tpm_db_gmac_lpk_uni_ingr_rate_limit_t *rate_limit);

#ifdef CONFIG_MV_INCLUDE_PON
int32_t tpm_db_register_pon_callback(PON_SHUTDOWN_FUNC pon_func);
int32_t tpm_db_get_pon_callback(PON_SHUTDOWN_FUNC *pon_func);
#endif

#ifdef __cplusplus
}
#endif
#endif				/* _TPM_DB_H_ */
