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

#ifndef _TPM_MNG_IF_H_
#define _TPM_MNG_IF_H_

/* Include Files
------------------------------------------------------------------------------*/

/* Definitions
------------------------------------------------------------------------------*/
#define TPM_NUM_DEVICES     (1)
#define TPM_DEV_NAME        ("tpm")
#define MV_TPM_IOCTL_MAGIC  ('T')

#define MAX_ETH_PORT_ALARM 10
#define MAX_FRAME_SIZE     1600

#define MV_TPM_IOCTL_INIT_SECTION          _IOW(MV_TPM_IOCTL_MAGIC,   1, unsigned int)
#define MV_TPM_IOCTL_ADMIN_SECTION         _IOWR(MV_TPM_IOCTL_MAGIC,  2, unsigned int)
#define MV_TPM_IOCTL_ADD_ACL_SECTION       _IOW(MV_TPM_IOCTL_MAGIC,   3, unsigned int)
#define MV_TPM_IOCTL_DEL_ACL_SECTION       _IOW(MV_TPM_IOCTL_MAGIC,   4, unsigned int)
#define MV_TPM_IOCTL_CTRL_ACL_SECTION      _IOWR(MV_TPM_IOCTL_MAGIC,  5, unsigned int)
#define MV_TPM_IOCTL_MC_STREAM_SECTION     _IOW(MV_TPM_IOCTL_MAGIC,   6, unsigned int)
#define MV_TPM_IOCTL_MNG_SECTION           _IOWR(MV_TPM_IOCTL_MAGIC,  7, unsigned int)
#define MV_TPM_IOCTL_SW_SECURITY_SECTION   _IOW(MV_TPM_IOCTL_MAGIC,   8, unsigned int)
#define MV_TPM_IOCTL_SW_VLAN_SECTION       _IOW(MV_TPM_IOCTL_MAGIC,   9, unsigned int)
#define MV_TPM_IOCTL_SW_TM_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  10, unsigned int)
#define MV_TPM_IOCTL_IGMP_SECTION          _IOW(MV_TPM_IOCTL_MAGIC,  11, unsigned int)
#define MV_TPM_IOCTL_PP_TM_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  12, unsigned int)
#define MV_TPM_IOCTL_PRINT_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  13, unsigned int)
#define MV_TPM_IOCTL_TRACE_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  14, unsigned int)
#define MV_TPM_IOCTL_SW_PHY_SECTION        _IOW(MV_TPM_IOCTL_MAGIC,  15, unsigned int)
#define MV_TPM_IOCTL_RESET_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  16, unsigned int)
#define MV_TPM_IOCTL_ALARM_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  17, unsigned int)
#define MV_TPM_IOCTL_COUNTER_CLEAR_SECTION _IOW(MV_TPM_IOCTL_MAGIC,  18, unsigned int)
#define MV_TPM_IOCTL_GET_MOD_SECTION       _IOWR(MV_TPM_IOCTL_MAGIC, 19, unsigned int)
#define MV_TPM_IOCTL_RX_IGMP               _IOWR(MV_TPM_IOCTL_MAGIC, 20, unsigned int)
#define MV_TPM_IOCTL_TX_IGMP               _IOW(MV_TPM_IOCTL_MAGIC,  21, unsigned int)
#define MV_TPM_IOCTL_SETUP_SECTION         _IOW(MV_TPM_IOCTL_MAGIC,  22, unsigned int)
#define MV_TPM_IOCTL_CPU_LPBK_SECTION      _IOWR(MV_TPM_IOCTL_MAGIC, 23, unsigned int)
#define MV_TPM_IOCTL_AGE_COUNT_SECTION     _IOWR(MV_TPM_IOCTL_MAGIC, 24, unsigned int)
#define MV_TPM_IOCTL_MC_VLAN_SECTION       _IOWR(MV_TPM_IOCTL_MAGIC, 25, unsigned int)
#define MV_TPM_IOCTL_MTU_SECTION           _IOWR(MV_TPM_IOCTL_MAGIC, 26, unsigned int)
#define MV_TPM_IOCTL_TPM_CHECK_SECTION     _IOW(MV_TPM_IOCTL_MAGIC,  27, unsigned int)
#define MV_TPM_IOCTL_FLUSH_ATU_SECTION     _IOWR(MV_TPM_IOCTL_MAGIC, 28, unsigned int)
#define MV_TPM_IOCTL_FLUSH_VTU_SECTION     _IOWR(MV_TPM_IOCTL_MAGIC, 29, unsigned int)
#define MV_TPM_IOCTL_SET_IPV6_CM_PARSE_WIN_SECTION     _IOWR(MV_TPM_IOCTL_MAGIC, 30, unsigned int)


#define MV_APM_IOCTL_MAGIC  ('A')

#define MV_APM_IOCTL_PM_1_SWPORT_SECTION   _IOWR(MV_APM_IOCTL_MAGIC,  1, unsigned int)
#define MV_APM_IOCTL_PM_3_SWPORT_SECTION   _IOWR(MV_APM_IOCTL_MAGIC,  2, unsigned int)


/* Enums
------------------------------------------------------------------------------*/

typedef enum tpm_ioctl_cmd_type
{
    MV_TPM_IOCTL_INIT =  1,
    MV_TPM_IOCTL_CRT_OWNER,
    MV_TPM_IOCTL_REQ_API_G_OWNER,
    MV_TPM_IOCTL_GET_API_G_OWNER,
    MV_TPM_IOCTL_DEL_SECTION,
    MV_TPM_IOCTL_GET_SECTION_SIZE,
    MV_TPM_IOCTL_ADD_L2_ACL_RULE,
    MV_TPM_IOCTL_ADD_L3_ACL_RULE,
    MV_TPM_IOCTL_ADD_IPv4_ACL_RULE,
    MV_TPM_IOCTL_DEL_L2_ACL_RULE,
    MV_TPM_IOCTL_DEL_L3_ACL_RULE,
    MV_TPM_IOCTL_DEL_IPv4_ACL_RULE,
    MV_TPM_IOCTL_GET_NEXT_VALID_RULE,
    MV_TPM_IOCTL_ADD_IPv4_MC_STREAM,
    MV_TPM_IOCTL_MOD_IPv4_MC_STREAM,
    MV_TPM_IOCTL_DEL_IPv4_MC_STREAM,
    MV_TPM_IOCTL_ADD_OMCI_CHNL,
    MV_TPM_IOCTL_DEL_OMCI_CHNL,
    MV_TPM_IOCTL_GET_OMCI_CHNL,
    MV_TPM_IOCTL_ADD_OAM_CHNL,
    MV_TPM_IOCTL_DEL_OAM_CHNL,
    MV_TPM_IOCTL_GET_OAM_CHNL,
    MV_TPM_IOCTL_SW_ADD_STATIC_MAC,
    MV_TPM_IOCTL_SW_DEL_STATIC_MAC,
    MV_TPM_IOCTL_SW_SET_PORT_MAC_LIMIT,
    MV_TPM_IOCTL_SW_SET_PORT_FLOOD,
    MV_TPM_IOCTL_SW_SET_PORT_TAGGED,
    MV_TPM_IOCTL_SW_SET_PORT_UNTAGGED,
    MV_TPM_IOCTL_SW_PORT_ADD_VID,
    MV_TPM_IOCTL_SW_PORT_DEL_VID,
    MV_TPM_IOCTL_SW_PORT_SER_VID_FILTER,
    MV_TPM_IOCTL_SW_SET_UNI_SCHED,
    MV_TPM_IOCTL_SW_UNI_QUE_WEIGHT,
    MV_TPM_IOCTL_SW_SET_UNI_INGR_POLICE_RATE,
    MV_TPM_IOCTL_SW_UNI_TC_INGR_POLICE_RATE,
    MV_TPM_IOCTL_SW_SET_UNI_EGR_RATE_LIMIT,
    MV_TPM_IOCTL_TM_SET_WAN_EGR_QUE_SCHED,
    MV_TPM_IOCTL_TM_SET_WAN_INGR_QUE_SCHED,
    MV_TPM_IOCTL_TM_SET_WAN_SCHED_EGR_RATE_LIMIT,
    MV_TPM_IOCTL_TM_SET_WAN_QUE_EGR_RATE_LIMIT,
    MV_TPM_IOCTL_TM_SET_WAN_INGR_RATE_LIMIT,
    MV_TPM_IOCTL_TM_SET_WAN_QUE_INGR_RATE_LIMIT,
    MV_TPM_IOCTL_IGMP_SET_PORT_FRWD_MODE,
    MV_TPM_IOCTL_IGMP_GET_PORT_FRWD_MODE,
    MV_TPM_IOCTL_PRINT_ETHER_PORTS,
    MV_TPM_IOCTL_PRINT_TX_MODULES,
    MV_TPM_IOCTL_PRINT_GMAC_CONFIG,
    MV_TPM_IOCTL_PRINT_GMAC_FUNC,
    MV_TPM_IOCTL_PRINT_EXT_PORT_MAP,        /*not used anymore - can be reused*/
    MV_TPM_IOCTL_PRINT_IGMP,
    MV_TPM_IOCTL_PRINT_MISC,
    MV_TPM_IOCTL_PRINT_OWNERS,
    MV_TPM_IOCTL_PRINT_API_GROUP,           /*not used anymore - can be reused*/
    MV_TPM_IOCTL_PRINT_VLAN_TYPE,
    MV_TPM_IOCTL_PRINT_VALID_API_SECTIONS,
    MV_TPM_IOCTL_PRINT_FULL_API_SECTIONS,
    MV_TPM_IOCTL_PRINT_MOD_SHADOW_RANGE,    /*not used anymore - can be reused*/
    MV_TPM_IOCTL_PRINT_PNC_SHADOW_RANGE,
    MV_TPM_IOCTL_PRINT_VALID_PNC_RANGES,
    MV_TPM_IOCTL_PRINT_INIT_TABLES,

    MV_TPM_IOCTL_PRINT_MOD2_TMP_PATTERN,
    MV_TPM_IOCTL_PRINT_MOD2_JUMP_RANGE,     /*not used anymore - can be reused*/
    MV_TPM_IOCTL_PRINT_MOD2_JUMP_ALL,
    MV_TPM_IOCTL_PRINT_MOD2_MAIN_RANGE,     /*not used anymore - can be reused*/
    MV_TPM_IOCTL_PRINT_MOD2_MAIN_ALL,
    MV_TPM_IOCTL_PRINT_MOD2_CFG,
    MV_TPM_IOCTL_PRINT_MOD2_RULE,

    MV_TPM_IOCTL_TRACE_STATUS,
    MV_TPM_IOCTL_TRACE_SET,
    MV_TPM_IOCTL_TRACE_MODULE_SET,

    MV_TPM_IOCTL_RESERVED_1,
    MV_TPM_IOCTL_RESERVED_2,

    MV_TPM_IOCTL_SW_PHY_SET_AUTONEG_MODE,
    MV_TPM_IOCTL_SW_PHY_GET_AUTONEG_MODE,
    MV_TPM_IOCTL_SW_PHY_RESTART_AUTONEG,
    MV_TPM_IOCTL_SW_PHY_SET_PORT_STATE,
    MV_TPM_IOCTL_SW_PHY_GET_PORT_STATE,
    MV_TPM_IOCTL_SW_PHY_GET_LINK_STATUS,
    MV_TPM_IOCTL_SW_PHY_SET_PORT_FLOW_CONTROL,
    MV_TPM_IOCTL_SW_PHY_GET_PORT_FLOW_CONTROL,
    MV_TPM_IOCTL_SW_PHY_SET_PORT_SPEED,
    MV_TPM_IOCTL_SW_PHY_GET_PORT_SPEED,
    MV_TPM_IOCTL_SW_PORT_VLAN_EGRESS_MODE,
    MV_TPM_IOCTL_SW_PORT_SET_DEFAULT_VLAN,
    MV_TPM_IOCTL_SW_PORT_GET_DEFAULT_VLAN,
    MV_TPM_IOCTL_SW_PORT_SET_DEFAULT_PRIORITY,
    MV_TPM_IOCTL_SW_PORT_GET_DEFAULT_PRIORITY,
    MV_TPM_IOCTL_SW_PORT_CLEAR_VID,
    MV_TPM_IOCTL_SW_PHY_SET_PORT_LOOPBACK,
    MV_TPM_IOCTL_SW_PHY_GET_PORT_LOOPBACK,

    MV_TPM_IOCTL_ALARM_GET_ETH_PORT,
    MV_TPM_IOCTL_SW_PHY_CONVERT_PORT_INDEX,
    MV_TPM_IOCTL_ADD_LOOP_DETECT_CHNL,
    MV_TPM_IOCTL_ADD_OAM_LOOPBACK_CHNL,
    MV_TPM_IOCTL_DEL_OAM_LOOPBACK_CHNL,
    MV_TPM_IOCTL_SW_PHY_SET_PORT_DUPLEX_MODE,
    MV_TPM_IOCTL_SW_SET_MAC_AGE_TIME,
    MV_TPM_IOCTL_SW_GET_MAC_AGE_TIME,
    MV_TPM_IOCTL_SW_SET_MAC_LEARN,
    MV_TPM_IOCTL_SW_GET_PORT_FLOOD,

    MV_TPM_IOCTL_RESERVED_3,
    MV_TPM_IOCTL_RESERVED_4,
    MV_TPM_IOCTL_RESERVED_5,
    MV_TPM_IOCTL_RESERVED_6,

    MV_TPM_IOCTL_SW_PHY_GET_DUPLEX_STATUS,
    MV_TPM_IOCTL_SW_PHY_GET_SPEED_MODE,
    MV_TPM_IOCTL_SW_CLEAR_DYNAMIC_MAC,
    MV_TPM_IOCTL_SW_SET_MIRROR,
    MV_TPM_IOCTL_SW_GET_MIRROR,
    MV_TPM_IOCTL_SW_GET_MAC_LEARN,
    MV_TPM_IOCTL_SW_PHY_GET_PORT_DUPLEX_MODE,
    MV_TPM_IOCTL_SET_MTU_SIZE,
    MV_TPM_IOCTL_GET_MTU_SIZE,
    MV_TPM_IOCTL_GET_PM_COUNTERS,
    MV_TPM_IOCTL_CLEAR_PM_COUNTERS,
    MV_TPM_IOCTL_SW_SET_ISOLATE_PORT_VECTOR,
    MV_TPM_IOCTL_SW_GET_UNI_EGR_RATE_LIMIT,
    MV_TPM_IOCTL_SW_GET_UNI_INGR_POLICE_RATE,

    MV_TPM_IOCTL_SET_MOD_RULE,
    MV_TPM_IOCTL_DEL_MOD_RULE,
    MV_TPM_IOCTL_GET_MOD_RULE,
    MV_TPM_IOCTL_INV_MOD_RULES,

    MV_TPM_IOCTL_PRINT_RX_MODULES,
    MV_TPM_IOCTL_SW_PORT_ADD_ALL_VID,
    MV_TPM_IOCTL_SW_GET_ISOLATE_PORT_VECTOR,
    MV_TPM_IOCTL_SW_PHY_GET_PORT_FC_STATE,
    MV_TPM_IOCTL_SW_ADD_VID_GROUP,
    MV_TPM_IOCTL_SW_DEL_VID_GROUP,
    MV_TPM_IOCTL_IGMP_SET_CPU_RX_Q,
    MV_TPM_IOCTL_IGMP_GET_CPU_RX_Q,

    MV_TPM_IOCTL_ADD_CPU_WAN_LPBK,
    MV_TPM_IOCTL_DEL_CPU_WAN_LPBK,

    MV_TPM_IOCTL_GET_LU_ENTRIES,
    MV_TPM_IOCTL_GET_ALL_HIT_COUNTERS,
    MV_TPM_IOCTL_SET_LU_COUNT_MASK,
    MV_TPM_IOCTL_GET_HIT_COUNT,
    MV_TPM_IOCTL_SET_LU_THESHOLD,
    MV_TPM_IOCTL_RESET_COUNT_GROUP,

    MV_TPM_IOCTL_ADD_IPV6_NH_ACL_RULE,
    MV_TPM_IOCTL_DEL_IPV6_NH_ACL_RULE,
    MV_TPM_IOCTL_ADD_IPV6_GEN_ACL_RULE,
    MV_TPM_IOCTL_DEL_IPV6_GEN_ACL_RULE,
    MV_TPM_IOCTL_ADD_IPV6_DIP_ACL_RULE,
    MV_TPM_IOCTL_DEL_IPV6_DIP_ACL_RULE,
    MV_TPM_IOCTL_ADD_IPV6_L4_PORTS_ACL_RULE,
    MV_TPM_IOCTL_DEL_IPV6_L4_PORTS_ACL_RULE,

    MV_TPM_IOCTL_SET_MC_VID_PORT_VIDS,
    MV_TPM_IOCTL_SET_MTU_ADMIN,
    MV_TPM_IOCTL_SET_MTU,
    MV_TPM_IOCTL_SET_PPPOE_MTU,

    MV_TPM_IOCTL_SET_IGMP_PROXY_SA_MAC,
    MV_TPM_IOCTL_GET_IGMP_PROXY_SA_MAC,
	MV_TPM_IOCTL_ADD_IPv6_MC_STREAM,
	MV_TPM_IOCTL_MOD_IPv6_MC_STREAM,
	MV_TPM_IOCTL_DEL_IPv6_MC_STREAM,

    MV_TPM_IOCTL_ADD_IPV6_GEN_5T_RULE,
    MV_TPM_IOCTL_DEL_IPV6_GEN_5T_RULE,
    MV_TPM_IOCTL_ADD_IPV6_DIP_5T_RULE,
    MV_TPM_IOCTL_DEL_IPV6_DIP_5T_RULE,
    MV_TPM_IOCTL_ADD_IPV6_L4_PORTS_5T_RULE,
    MV_TPM_IOCTL_DEL_IPV6_L4_PORTS_5T_RULE,

    MV_TPM_IOCTL_ADD_CTC_CM_ACL_RULE,
    MV_TPM_IOCTL_DEL_CTC_CM_ACL_RULE,
    MV_TPM_IOCTL_ADD_CTC_CM_IPV6_ACL_RULE,
    MV_TPM_IOCTL_SET_CTC_CM_IPV6_PARSE_WIN,
    MV_TPM_IOCTL_ADD_MAC_LEARN_ACL_RULE,
    MV_TPM_IOCTL_DEL_MAC_LEARN_ACL_RULE,
    MV_TPM_IOCTL_SET_MAC_LEARN_DEFAULT_ACTION,
    MV_TPM_IOCTL_TM_SET_GMAC0_INGR_RATE_LIMIT,
    MV_TPM_IOCTL_GET_MAC_LEARN_ENTRY_NUM,

} tpm_ioctl_cmd_type_t;

/* Typedefs
------------------------------------------------------------------------------*/

/* Print Section
** ==============
** Function list:
** 1.  tpm_print_etherports
** 2.  tpm_print_tx_modules
** 3.  tpm_print_gmac_config
** 4.  tpm_print_gmac_func
** 5.  tpm_print_ext_port_map
** 6.  tpm_print_igmp
** 7.  tpm_print_misc
** 8.  tpm_print_owners
** 9.
** 10. tpm_print_vlan_etype
** 11. tpm_print_valid_api_sections
** 12. tpm_print_full_api_section
** 13. tpm_print_mod_shadow_range
** 14. tpm_print_pnc_shadow_range
** 15. tpm_print_valid_pnc_ranges
** 16. tpm_print_init_tables
** 17. tpm_mod2_print_tmp_pattern
** 18. tpm_mod2_print_jump_range
** 19. tpm_mod2_print_jump_all
** 20. tpm_mod2_print_main_range
** 21. tpm_mod2_print_main_all
** 22. tpm_mod2_print_config
*/
typedef struct
{
    uint32_t           print_cmd;
    tpm_api_sections_t api_section;
    tpm_gmacs_enum_t   gmac;
    uint32_t           valid;
    uint32_t           start;
    uint32_t           end;
    uint32_t           dir;
    uint32_t           entry;
    uint32_t           num;
} tpm_ioctl_print_t;

/* Trace Section
** ==============
** Function list:
** 1.  tpm_trace_status_print
** 2.  tpm_trace_set
** 3.  tpm_trace_module_set
*/

typedef struct
{
    uint32_t trace_cmd;
    uint32_t level;
    uint32_t module;
    uint32_t flag;
} tpm_ioctl_trace_t;

/* Admin Section
** ==============
** Function list:
** 1. tpm_create_ownerid
** 2. tpm_request_api_ownership
** 3. tpm_get_api_ownership
** 4. tpm_erase_section
** 5. tpm_get_section_free_size
*/

typedef struct
{
    uint32_t                  amdin_cmd;
    uint32_t                  owner_id;
    tpm_api_type_t            api_type;
    tpm_api_ownership_error_t api_ownership_error;
    int32_t                   section_size;
    uint8_t                   section_dir;
    uint8_t                   dummy1;
    uint16_t                  dummy2;
} tpm_ioctl_admin_t;

/* ACL Rule Add Section
** ====================
** Function list:
** 1. tpm_add_l2_rule
** 2. tpm_add_l3_type_rule
** 3. tpm_add_ipv4_rule
** 4. tpm_add_ipv6_gen_rule
** 5. tpm_add_ipv6_nh_rule
** 6. tpm_add_ipv6_dip_rule
** 7. tpm_add_ipv6_l4_ports_rule
** 8. tpm_mod_entry_set
** 9. tpm_add_ipv6_gen_5t_rule
** 10. tpm_add_ipv6_dip_5t_rule
** 11. tpm_add_ipv6_l4_ports_5t_rule
** 12. tpm_add_ctc_cm_acl_rule
*/

/* L2 */
typedef struct
{
    tpm_l2_acl_key_t   l2_key;
    tpm_pkt_frwd_t     pkt_frwd;
    tpm_pkt_mod_t      pkt_mod;
    tpm_rule_action_t  rule_action;
    tpm_parse_flags_t  parse_flags_bm;
    tpm_pkt_mod_bm_t   pkt_mod_bm;
    tpm_unknown_mac_conf_t mac_conf;/*Just for PnC MAC learn*/
} l2_acl_rule_t;

/* L3 */
typedef struct
{
    tpm_l3_type_key_t   l3_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_rule_action_t   rule_action;
    tpm_parse_flags_t   parse_flags_bm;
} l3_acl_rule_t;

/* IPv4 */
typedef struct
{
    tpm_parse_flags_t   parse_flags_bm;
    tpm_ipv4_acl_key_t  ipv4_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_pkt_mod_t       pkt_mod;
    tpm_pkt_mod_bm_t    pkt_mod_bm;
    tpm_rule_action_t   rule_action;
} ipv4_acl_rule_t;

/* IPv6 */
typedef struct
{
    tpm_parse_flags_t   parse_flags_bm;
    tpm_ipv6_gen_acl_key_t ipv6_gen_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_pkt_mod_t       pkt_mod;
    tpm_pkt_mod_bm_t    pkt_mod_bm;
    tpm_rule_action_t   rule_action;
} ipv6_gen_acl_rule_t;

typedef struct
{
	tpm_nh_iter_t            nh_iter;
    uint32_t                 nh;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_rule_action_t        rule_action;
} ipv6_nh_acl_rule_t;

typedef struct
{
    tpm_parse_flags_t        parse_flags_bm;
    tpm_ipv6_addr_key_t      ipv6_dip_key;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    tpm_pkt_mod_bm_t         pkt_mod_bm;
    tpm_rule_action_t        rule_action;
} ipv6_dip_acl_rule_t;

typedef struct
{
    tpm_parse_flags_t        parse_flags_bm;
    tpm_l4_ports_key_t       l4_key;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    tpm_pkt_mod_bm_t         pkt_mod_bm;
    tpm_rule_action_t        rule_action;
} ipv6_l4_ports_acl_rule_t;

/* IPv6 5 tuple */
typedef struct
{
    tpm_parse_flags_t      parse_flags_bm;
    tpm_l4_ports_key_t     l4_key;
    tpm_ipv6_gen_acl_key_t ipv6_gen_key;
    tpm_pkt_frwd_t         pkt_frwd;
    tpm_pkt_mod_t          pkt_mod;
    tpm_pkt_mod_bm_t       pkt_mod_bm;
    tpm_rule_action_t      rule_action;
} ipv6_gen_5t_rule_t;

typedef struct
{
    tpm_parse_flags_t        parse_flags_bm;
    tpm_l4_ports_key_t       l4_key;
    tpm_ipv6_gen_acl_key_t   ipv6_gen_key;
    tpm_ipv6_addr_key_t      ipv6_dip_key;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    tpm_pkt_mod_bm_t         pkt_mod_bm;
    tpm_rule_action_t        rule_action;
} ipv6_dip_5t_rule_t;

typedef struct
{
    tpm_parse_flags_t        parse_flags_bm;
    tpm_l4_ports_key_t       l4_key;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    tpm_pkt_mod_bm_t         pkt_mod_bm;
    tpm_rule_action_t        rule_action;
} ipv6_l4_ports_5t_rule_t;

/* CTC CnM */
typedef struct
{
    uint32_t           precedence;
    uint32_t           p_bits;
    tpm_parse_fields_t l2_parse_rule_bm;
    tpm_parse_fields_t ipv4_parse_rule_bm;
    tpm_parse_fields_t ipv6_parse_rule_bm;
    tpm_l2_acl_key_t   l2_key;
    tpm_ipv4_acl_key_t ipv4_key;
    tpm_ipv6_acl_key_t ipv6_key;
    tpm_pkt_frwd_t     pkt_frwd;
    tpm_pkt_action_t   pkt_act;
} ctc_cm_acl_rule_t;

/* Pkt modification */
typedef struct
{
    tpm_trg_port_type_t  trg_port;
    tpm_pkt_mod_bm_t     mod_bm;
    tpm_pkt_mod_int_bm_t int_bm;
    tpm_pkt_mod_t        mod_data;
} pkt_mod_rule_t;

typedef struct
{
    uint32_t             add_acl_cmd;
    uint32_t             owner_id;
    tpm_src_port_type_t  src_port;
	tpm_dir_t            src_dir;
    uint32_t             rule_num;
    uint32_t             rule_idx;
    tpm_parse_fields_t   parse_rule_bm;

    union
    {
      l2_acl_rule_t            l2_acl_rule;
      l3_acl_rule_t            l3_acl_rule;
      ipv4_acl_rule_t          ipv4_acl_rule;
      ipv6_gen_acl_rule_t      ipv6_gen_acl_rule;
      ipv6_nh_acl_rule_t       ipv6_nh_acl_rule;
      ipv6_dip_acl_rule_t      ipv6_dip_acl_rule;
      ipv6_l4_ports_acl_rule_t ipv6_l4_ports_acl_rule;
      ipv6_gen_5t_rule_t       ipv6_gen_5t_rule;
      ipv6_dip_5t_rule_t       ipv6_dip_5t_rule;
      ipv6_l4_ports_5t_rule_t  ipv6_l4_ports_5t_rule;
      ctc_cm_acl_rule_t        ctc_cm_acl_rule;
      pkt_mod_rule_t           mod_rule;
    };
} tpm_ioctl_add_acl_rule_t;

/* ACL Rule Delete Section
** =======================
** Function list:
** 1. tpm_del_l2_rule
** 2. tpm_del_l3_type_rule
** 3. tpm_del_ipv4_rule
** 4. tpm_del_ipv6_gen_rule
** 5. tpm_del_ipv6_nh_rule
** 6. tpm_del_ipv6_dip_rule
** 7. tpm_del_ipv6_l4_ports_rule
** 8. tpm_mod_entry_del
** 9. tpm_mod_mac_inv
** 10. tpm_del_ipv6_gen_5t_rule
** 11. tpm_del_ipv6_dip_5t_rule
** 12. tpm_del_ipv6_l4_ports_5t_rule
** 13. tpm_del_ctc_cm_acl_rule
*/

typedef struct
{
    uint32_t             del_acl_cmd;
    uint32_t             owner_id;
    tpm_src_port_type_t  src_port;
    tpm_trg_port_type_t  trg_port;
    uint32_t             rule_idx;
    uint32_t           	 precedence;
    tpm_parse_fields_t   parse_rule_bm;
    tpm_l2_acl_key_t     l2_key;/*for MAC learn*/
} tpm_ioctl_del_acl_rule_t;

/* Modification Rule Get Section
** =======================
** Function list:
** 1. tpm_mod_entry_get
*/
typedef struct
{
    uint32_t            get_mod_cmd;
    tpm_trg_port_type_t trg_port;
    uint32_t            rule_idx;
    uint16_t            pnc_ref;
    uint16_t            valid_num;
    tpm_mod_rule_t      rule[TPM_MAX_MOD_RULE_NUM];
} tpm_ioctl_get_mod_rule_t;

/* ACL Ctrl Rule Section
** =====================
** Function list:
** 1. tpm_get_next_valid_rule
*/

typedef struct
{
    uint32_t         ctrl_acl_cmd;
    uint32_t         owner_id;
    int32_t          current_index;
    uint32_t         next_index;
    uint32_t         rule_idx;
    tpm_api_type_t   rule_type;
    uint8_t          direction;
    uint8_t          dummy1;
    uint16_t         dummy2;
    tpm_rule_entry_t tpm_rule;
    uint32_t         entry_count;
} tpm_ioctl_ctrl_acl_rule_t;

/* MC Rule Section
** ===============
** Function list:
** 1. tpm_add_ipv4_mc_stream
** 2. tpm_updt_ipv4_mc_stream
** 3. tpm_del_ipv4_mc_stream
** 4. tpm_add_ipv6_mc_stream
** 5. tpm_updt_ipv6_mc_stream
** 6. tpm_del_ipv6_mc_stream
*/

/* IPv4 */
typedef struct
{
    uint8_t  ipv4_src_add[4];
    uint8_t  ipv4_dst_add[4];
    uint8_t  ignore_ipv4_src;
    uint8_t  dummy1;
    uint16_t dummy2;
} ipv4_mc_rule_t;

/* IPv6 */
typedef struct
{
    uint32_t vid;
    uint8_t  ipv6_src_add[16];
    uint8_t  ipv6_dst_add[16];
    uint8_t  ignore_ipv6_src;
    uint8_t  dummy1;
    uint16_t dummy2;
} ipv6_mc_rule_t;

typedef struct
{
    uint32_t             mc_cmd;
    uint32_t             owner_id;
    uint32_t             stream_num;
    tpm_mc_igmp_mode_t   igmp_mode;
    uint8_t              mc_stream_pppoe;
    uint16_t             vid;
    tpm_trg_port_type_t  dest_port_bm;

    union
    {
      ipv4_mc_rule_t  ipv4_mc;
      ipv6_mc_rule_t  ipv6_mc;
    };
} tpm_ioctl_mc_rule_t;

/* Mng Rule Section
** ================
** Function list:
** 1. tpm_omci_add_channel
** 2. tpm_omci_del_channel
** 3. tpm_omci_get_channel
** 4. tpm_oam_epon_add_channel
** 5. tpm_oam_epon_del_channel
** 6. tpm_oam_epon_get_channel
*/

typedef struct
{
    uint32_t              owner_id;
    uint32_t              is_valid;
    uint32_t              cpu_rx_queue;
    tpm_trg_port_type_t   tcont_num;
    uint32_t              cpu_tx_queue;
    tpm_gem_port_key_t    gem_port;
    uint16_t              dummy;
} tpm_ioctl_omci_ch_t;

typedef struct
{
    uint32_t              owner_id;
    uint32_t              is_valid;
    uint32_t              cpu_rx_queue;
    tpm_trg_port_type_t   llid_num;
} tpm_ioctl_oam_ch_t;

typedef struct
{
    uint32_t              mng_cmd;
    tpm_ioctl_omci_ch_t   tpm_ioctl_omci_ch;
    tpm_ioctl_oam_ch_t    tpm_ioctl_oam_ch;
} tpm_ioctl_mng_ch_t;

/* Switch Section
** ==============
** Function list:
** 1. tpm_sw_add_static_mac
** 2. tpm_sw_del_static_mac
** 3. tpm_sw_set_port_max_macs
** 4. tpm_sw_set_port_flooding
**
** 5. tpm_sw_set_port_tagged
** 6. tpm_sw_set_port_untagged
** 7. tpm_sw_port_add_vid
** 8. tpm_sw_port_del_vid
** 9. tpm_sw_port_set_vid_filter
**
** 10.tpm_sw_set_uni_sched
** 11.tpm_sw_set_uni_q_weight
** 12.tpm_sw_set_uni_ingr_police_rate
** 13.tpm_sw_set_uni_tc_ingr_police_rate
** 14.tpm_sw_set_uni_egr_rate_limit
*/

/* Switch MAC security */
typedef struct
{
    uint32_t              sw_security_cmd;
    uint32_t              owner_id;
    tpm_src_port_type_t   port;
    uint8_t               static_mac[6];
    uint8_t               mac_per_port;
    uint8_t               allow_flood;
    bool                  enable_mac_learn;
    tpm_flood_type_t      flood_mode;
    uint32_t              mac_age_time;
    tpm_sw_mirror_t       port_mirror;
    tpm_mru_type_t        mtu_type;
    uint32_t              mtu_size;
    uint32_t              port_vector;
} tpm_ioctl_sw_mac_security_t;

/* Switch Vlan filtering */
typedef struct
{
    uint32_t              sw_vlan_cmd;
    uint32_t              owner_id;
    tpm_src_port_type_t   port;
    uint16_t              vid;
    uint8_t               pri;
    uint8_t               vid_filter;
    uint8_t               allow_tagged;
    uint8_t               allow_untagged;
    uint8_t               egress_mode;
    uint8_t               dummy;
    uint16_t              min_vid;
    uint16_t              max_vid;
} tpm_ioctl_sw_vlan_filter_t;

/* Switch Traffic management */
typedef struct
{
    uint32_t            sw_tm_cmd;
    uint32_t            owner_id;
    tpm_src_port_type_t uni_port;
    tpm_sw_sched_type_t sw_sched_mode;
    uint8_t             queue_id;
    uint8_t             weight;
    uint16_t            dummy;
    uint32_t            limit_mode;
    uint32_t            cir;
    uint32_t            cbs;
    uint32_t            ebs;
    uint32_t            tc;
    uint32_t            rate_limit_val;
} tpm_ioctl_sw_tm_t;

/* Switch PHY port management */
typedef struct
{
    uint32_t                sw_phy_cmd;
    uint32_t                owner_id;
    tpm_src_port_type_t     port;
    tpm_phy_speed_t         port_speed;
    tpm_phy_loopback_mode_t port_loopback_mode;
    tpm_src_port_type_t     extern_port_id;
    uint32_t                switch_port_id;
    tpm_autoneg_mode_t      port_autoneg_mode;
    bool                    phy_port_state;
    bool                    port_autoneg_state;
    bool                    port_link_status;
    bool                    port_pause_state;
    bool                    port_duplex_enable;
    bool                    port_loopback_state;
    uint16_t                dummy;
} tpm_ioctl_sw_phy_t;

/* Packet Processor Section
** ========================
** Function list:
** 1. tpm_tm_set_wan_egr_queue_sched
** 2. tpm_tm_set_wan_sched_egr_rate_lim
** 3. tpm_tm_set_wan_queue_egr_rate_lim
** 4. tpm_tm_set_wan_ingr_rate_lim
** 5. tpm_tm_set_wan_q_ingr_rate_lim
*/

/* Packet Processor Traffic management */
typedef struct
{
    uint32_t            pp_tm_cmd;
    uint32_t            owner_id;
    tpm_trg_port_type_t sched_ent;
    tpm_trg_port_type_t uni_port;
    tpm_pp_sched_type_t pp_sched_mode;
    uint32_t            queue_id;
    uint32_t            wrr_weight;
    uint32_t            rate_limit_val;
    uint32_t            bucket_size;
} tpm_ioctl_tm_tm_t;

/* IGMP Section
** =============
** Function list:
** 1. tpm_proc_enable_igmp
** 2. tpm_proc_edisable_igmp
*/

/* Packet Processor Traffic management */
typedef struct
{
    uint32_t             igmp_cmd;
    uint32_t             owner_id;
    tpm_src_port_type_t  src_port;
    tpm_igmp_frwd_mode_t frwd_mode;
    uint32_t             cpu_queue;
    uint8_t              sa_mac[6];
} tpm_ioctl_igmp_t;


/* MIB Reset Section
** =============
** Function list:
** 1. tpm_proc_enable_igmp
*/

/* MIB reset */
typedef struct
{
    uint32_t               owner_id;
    tpm_reset_level_enum_t reset_level;
} tpm_ioctl_mib_reset_t;

/* ALARM Section */
typedef struct
{
    uint32_t            alarm_cmd;
    uint32_t            owner_id;
    uint32_t            alarm_type;
    uint8_t             port_bitmap[2][MAX_ETH_PORT_ALARM];
} tpm_ioctl_alarm_t;


/* Management structure for Ethernet counters 1 */
typedef struct
{
  tpm_src_port_type_t  port;
  uint32_t             owner_id;
  tpm_swport_pm_1_t    tpm_swport_pm_1;

} tpm_ioctl_swport_pm_1_t;


/* Management structure for Ethernet counters 3 */
typedef struct
{
  tpm_src_port_type_t  port;
  uint32_t             owner_id;
  tpm_swport_pm_3_t    tpm_swport_pm_3;

} tpm_ioctl_swport_pm_3_t;


typedef struct
{
    uint32_t             owner_id;
    tpm_trg_port_type_t  tgt_port;
    uint8_t              tgt_queue;
    uint8_t              dummy;
    uint16_t             gem_port;
    uint8_t              buf[MAX_FRAME_SIZE];
    uint32_t             len;
} tpm_ioctl_tx_igmp_t;

typedef struct
{
    uint32_t             owner_id;
    uint8_t              dummy[2];
    uint16_t             src_llid;
    tpm_src_port_type_t  src_port;
    uint8_t              buf[MAX_FRAME_SIZE];
    uint32_t             len;
} tpm_ioctl_rx_igmp_t;


typedef struct
{
    uint32_t             cpu_lpbk_cmd;
    uint32_t             owner_id;
    tpm_pkt_frwd_t       pkt_frwd;
    uint32_t             mod_idx;
} tpm_ioctl_cpu_lpbk_t;

typedef struct
{
    uint32_t             owner_id;
} tpm_ioctl_flush_vtu_t;
typedef struct
{
    uint32_t             owner_id;
    tpm_flush_atu_type_t flush_type;
    uint16_t             db_num;
} tpm_ioctl_flush_atu_t;

typedef struct
{
    uint32_t              age_count_cmd;
    uint32_t              owner_id;
    tpm_api_type_t        api_type;
    uint32_t              lu_thresh_pkts;
    uint32_t              rule_idx;
    uint32_t              lu_rule_mask;
    uint32_t              hit_count;
    uint16_t              lu_num;
    uint16_t              valid_num;
    uint16_t              unrelated_num;
    uint8_t               lu_reset;
    uint8_t               hit_reset;
    tpm_api_entry_count_t count_array[TPM_MAX_LU_ENTRY_NUM];
} tpm_ioctl_age_count_t;

typedef struct
{
    uint32_t              age_count_cmd;
    uint32_t              owner_id;
    tpm_api_type_t        api_type;
    uint32_t              high_thresh_pkts;
    uint16_t              valid_counters;
    uint8_t               counter_reset;
    uint8_t               dummy;
    tpm_api_entry_count_t count_array[TPM_MAX_PNC_COUNTER_NUM];
} tpm_ioctl_pnc_hit_cnt_t;

typedef struct
{
    uint32_t                  mc_vid_cmd;
    uint32_t                  owner_id;
    uint32_t                  mc_vid;
    tpm_mc_vid_port_vid_set_t port_vid_set;
} tpm_ioctl_mc_vid_t;

typedef struct
{
    uint32_t                  mtu_setting_cmd;
    uint32_t                  mtu;
    uint32_t                  pppoe_mtu;
    tpm_mtu_ethertype_t       ethertype;
    uint32_t                  direction;
    tpm_init_mtu_setting_enable_t  enable;
} tpm_ioctl_mtu_t;

/* TPM check */
typedef struct
{
    uint32_t               owner_id;
    tpm_self_check_level_enum_t check_level;
} tpm_ioctl_tpm_check_t;

/* ipv6 parse window */
typedef struct
{
    uint32_t               owner_id;
    tpm_ctc_cm_ipv6_parse_win_t ipv6_parse_window;
} tpm_ioctl_ipv6_parse_window_t;

/* this union aggregates all ioctl commands supported by SFS_TO_IOCTL mode */
typedef union  tpm_cmd_data
{
    tpm_ioctl_add_acl_rule_t    tpm_add_acl_rule;
    tpm_ioctl_del_acl_rule_t    tpm_del_acl_rule;
    tpm_ioctl_ctrl_acl_rule_t   tpm_ctrl_acl_rule;
    tpm_ioctl_mc_rule_t         tpm_mc_rule;
    tpm_ioctl_igmp_t            tpm_igmp;
    tpm_ioctl_cpu_lpbk_t        tpm_ioctl_cpu_lpbk;
    tpm_ioctl_mib_reset_t       tpm_mib_reset_param;
    tpm_ioctl_age_count_t       tpm_ioctl_age_count;
    tpm_ioctl_pnc_hit_cnt_t     tpm_ioctl_pnc_hit_cnt;
    tpm_ioctl_mtu_t		        tpm_ioctl_mtu_set;
    tpm_ioctl_sw_mac_security_t	tpm_ioctl_sw_mac_set;
    tpm_ioctl_tm_tm_t		    tpm_ioctl_tm_tm;
    tpm_ioctl_mng_ch_t		    tpm_mng_channel;
    tpm_ioctl_get_mod_rule_t	tpm_ioctl_get_mod;
    tpm_ioctl_tpm_check_t       tpm_check_param;
    tpm_ioctl_flush_vtu_t       tpm_ioctl_flush_vtu;
    tpm_ioctl_flush_atu_t       tpm_ioctl_flush_atu;
    tpm_ioctl_ipv6_parse_window_t   tpm_ipv6_parse_window;
} tpm_cmd_data_t;

/* this structure is used for passing sysfs request from kernel to userspace
   to be ioctl (SFS_TO_IOCTL mode)  */
typedef struct
{
    unsigned int    cmd;            /* the ioctl command            */
    tpm_cmd_data_t  tpm_cmd_data;   /* the ioctl additional data    */
} tpm_ioctl_mng_t;


/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/
int32_t tpm_module_mng_if_create(void);
int32_t tpm_module_mng_if_release(void);

/* Macros
------------------------------------------------------------------------------*/

#endif /* _TPM_MNG_IF_H_ */
