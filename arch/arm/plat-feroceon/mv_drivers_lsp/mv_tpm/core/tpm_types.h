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
* tpm_types.h
*
* DESCRIPTION:
*               Traffic Processor Manager - types definition.
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

#ifndef _TPM_TYPES_H_
#define _TPM_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
/********************************** Administrative ****************************/
/******************************************************************************/

/* API group ownership error codes */
typedef enum tpm_api_ownership_error {
	API_OWNERSHIP_SUCCESS,
	API_OWNERID_UNKNOWN,
	API_TYPE_UNKNOWN,
	API_OWNED,
	API_OWNERSHIP_ERROR
} tpm_api_ownership_error_t;


/* Maximum number of parallel running API calls to a certain API */
#define TPM_MAX_PARALLEL_API_CALLS	(4)

/******************************************************************************/
/********************************** Packet Processing *************************/
/******************************************************************************/

/*************************************/
/* Packet Processor API Types  *******/
/*************************************/
typedef enum {
	TPM_API_MGMT,
	TPM_API_MAC_LEARN,
	TPM_API_CPU_LOOPBACK,
	TPM_API_L2_PRIM,
	TPM_API_L3_TYPE,
	TPM_API_IPV4,
	TPM_API_IPV4_MC,
	TPM_API_IPV6_GEN,
	TPM_API_IPV6_DIP,
	TPM_API_IPV6_MC,
	TPM_API_IPV6_NH,
	TPM_API_IPV6_L4,
	TPM_API_CNM,
	TPM_API_TYPE_ILLEGAL,
	TPM_MAX_API_TYPES = TPM_API_TYPE_ILLEGAL
} tpm_api_type_t;

/*************************************/
/* Parse fields **********************/
/* tpm_parse_fields_t bitmap **********/
/*************************************/

#define     TPM_L2_PARSE_MAC_DA             (0x0001)	/* parsing DEST MAC Address */
#define     TPM_L2_PARSE_MAC_SA             (0x0002)	/* parsing SRC MAC Address */
#define     TPM_L2_PARSE_ONE_VLAN_TAG       (0x0004)	/* parsing external VLAN tag */
#define     TPM_L2_PARSE_TWO_VLAN_TAG       (0x0008)	/* parsing external and inner VLAN tag */
#define     TPM_L2_PARSE_ETYPE              (0x0010)	/* parsing Ether type */
#define     TPM_L2_PARSE_PPPOE_SES          (0x0020)	/* parsing PPPOE session */
#define     TPM_L2_PARSE_PPP_PROT           (0x0040)	/* parsing PPP protocol */
#define     TPM_L2_PARSE_GEMPORT            (0x0080)	/* parsing the GEM port */

#define     TPM_IPv4_PARSE_SIP              (0x0001)	/* parsing IPv4 Source IP address */
#define     TPM_IPv4_PARSE_DIP              (0x0002)	/* parsing IPv4 Dest IP address */
#define     TPM_IPv4_PARSE_DSCP             (0x0004)	/* parsing IPv4 DSCP */
#define     TPM_IPv4_PARSE_PROTO            (0x0008)	/* parsing IPv4 protocol */

#define     TPM_IPv6_PARSE_SIP              (0x0001)	/* parsing IPv6 Source IP address */
#define     TPM_IPv6_PARSE_DIP              (0x0002)	/* parsing IPv6 Dest IP address */
#define     TPM_IPv6_PARSE_DSCP             (0x0004)	/* parsing IPv6 DSCP */
#define     TPM_IPv6_PARSE_NH               (0x0008)	/* parsing IPv6 next hop */
#define     TPM_IPv6_PARSE_HOPL             (0x0040)	/* parsing IPv6 hop limit */
/* for CnM IPv6, there is the possibility that parse_bm has IPv6_HOPL, L4_SRC and L4_DST together,
   so they can not have the same value, that is the reason HOPL is changed from 0x10 to 0x40, since
   L4_SRC has 0x10 and L4_DST has 0x20
*/

/* Both for IPV4 and IPV6 */
#define     TPM_PARSE_L4_SRC                (0x0010)	/* parsing L4 source port */
#define     TPM_PARSE_L4_DST                (0x0020)	/* parsing L4 destination port */

typedef uint32_t tpm_parse_fields_t;

/*************************************/
/* Parse flags ***********************/
/* tpm_parse_flags_t bitmap **********/
/*************************************/
#define     TPM_PARSE_FLAG_TAG1_TRUE        (0x00001)
#define     TPM_PARSE_FLAG_TAG1_FALSE       (0x00002)
#define     TPM_PARSE_FLAG_TAG1_DC          (0x0)
#define     TPM_PARSE_FLAG_TAG2_TRUE        (0x00004)
#define     TPM_PARSE_FLAG_TAG2_FALSE       (0x00008)
#define     TPM_PARSE_FLAG_TAG2_DC          (0x0)
#define     TPM_PARSE_FLAG_MTM_TRUE         (0x00010)
#define     TPM_PARSE_FLAG_MTM_FALSE        (0x00020)
#define     TPM_PARSE_FLAG_MTM_DC           (0x0)
#define     TPM_PARSE_FLAG_TO_CPU_TRUE      (0x00040)
#define     TPM_PARSE_FLAG_TO_CPU_FALSE     (0x00080)
#define     TPM_PARSE_FLAG_TO_CPU_DC        (0x0)
#define     TPM_PARSE_FLAG_L4_UDP           (0x00100)
#define     TPM_PARSE_FLAG_L4_TCP           (0x00200)
#define     TPM_PARSE_FLAG_L4_DC            (0x0)
#define     TPM_PARSE_FLAG_PPPOE_TRUE       (0x00400)
#define     TPM_PARSE_FLAG_PPPOE_FALSE      (0x00800)
#define     TPM_PARSE_FLAG_PPPOE_DC         (0x0)

typedef uint32_t tpm_parse_flags_t;

/* Source port - used to specify through which port the packet entered the processor  */
typedef enum {
	TPM_SRC_PORT_UNI_0,		/* upstream */
	TPM_SRC_PORT_UNI_1,		/* upstream */
	TPM_SRC_PORT_UNI_2,		/* upstream */
	TPM_SRC_PORT_UNI_3,		/* upstream */
	TPM_SRC_PORT_UNI_4,		/* upstream */
	TPM_SRC_PORT_UNI_5,		/* upstream */
	TPM_SRC_PORT_UNI_6,		/* upstream */
	TPM_SRC_PORT_UNI_7,		/* upstream */
	TPM_SRC_PORT_UNI_VIRT,		/* upstream */
	TPM_SRC_PORT_WAN,		/* downstram */
	TPM_SRC_PORT_UNI_ANY,		/* upstream - all UNI ports */
	TPM_SRC_PORT_WAN_OR_LAN,	/* Any Port, currently not supported */
	TPM_SRC_PORT_ILLEGAL = 0xFF
} tpm_src_port_type_t;

/* Source Port derivatives : */
#define     TPM_MAX_NUM_UNI_PORTS           (TPM_SRC_PORT_UNI_VIRT - TPM_SRC_PORT_UNI_0 + 1)
#define     TPM_MAX_NUM_ETH_PORTS           (1/*wan*/+TPM_MAX_NUM_UNI_PORTS)
#define     TPM_RATE_LIMIT_MIN_VAL          (64)

/***********************************************/
/* Target port - bitmap ************************/
/* Used for forwarding decision making *********/
/* For EPON/GPON - same bitmap for LLID/TCONTs */
/***********************************************/
#define	TPM_TRG_PORT_WAN		(0x00000001)	/* upstream */

#define	TPM_TRG_TCONT_0			(0x00000001)	/* upstream */
#define	TPM_TRG_TCONT_1			(0x00000002)	/* upstream */
#define	TPM_TRG_TCONT_2			(0x00000004)	/* upstream */
#define	TPM_TRG_TCONT_3			(0x00000008)	/* upstream */
#define	TPM_TRG_TCONT_4			(0x00000010)	/* upstream */
#define	TPM_TRG_TCONT_5			(0x00000020)	/* upstream */
#define	TPM_TRG_TCONT_6			(0x00000040)	/* upstream */
#define	TPM_TRG_TCONT_7			(0x00000080)	/* upstream */
#define	TPM_TRG_LLID_0 			(0x00000001)	/* upstream */
#define	TPM_TRG_LLID_1 			(0x00000002)	/* upstream */
#define	TPM_TRG_LLID_2 			(0x00000004)	/* upstream */
#define	TPM_TRG_LLID_3 			(0x00000008)	/* upstream */
#define	TPM_TRG_LLID_4 			(0x00000010)	/* upstream */
#define	TPM_TRG_LLID_5 			(0x00000020)	/* upstream */
#define	TPM_TRG_LLID_6 			(0x00000040)	/* upstream */
#define	TPM_TRG_LLID_7			(0x00000080)	/* upstream */
#define	TPM_TRG_UNI_0			(0x00000100)	/* downstream */
#define	TPM_TRG_UNI_1			(0x00000200)	/* downstream */
#define	TPM_TRG_UNI_2			(0x00000400)	/* downstream */
#define	TPM_TRG_UNI_3			(0x00000800)	/* downstream */
#define	TPM_TRG_UNI_4			(0x00001000)	/* downstream */
#define	TPM_TRG_UNI_5			(0x00002000)	/* downstream */
#define	TPM_TRG_UNI_6			(0x00004000)	/* downstream */
#define	TPM_TRG_UNI_7			(0x00008000)	/* downstream */
#define	TPM_TRG_UNI_VIRT		(0x00010000)	/* downstream */

#define	TPM_TRG_PORT_CPU		(0x00020000)	/* upstream / downstream - CPU port */
#define	TPM_TRG_PORT_UNI_ANY		(0x00040000)	/* downstream - all UNI ports */
#define	TPM_TRG_PORT_UNI_CPU_LOOP	(0x00080000)	/* downstream - loop to the CPU port */
#define	TPM_TRG_PORT_ILLEGAL		(0xFFFFFFFF)	/* illegal port number value */
#define	TPM_TRG_UNI_OFFSET		8
#define	TPM_TRG_UNI_MASK		0x1FF


/* target port type - bitmap */
typedef uint32_t tpm_trg_port_type_t;	/*ex: TPM_TRG_PORT_WAN or TPM_TRG_TCONT_0 */
/* YUVAL_NOW - add allowed bitmaps here */

/* GPON GEM port id */
typedef uint16_t tpm_gem_port_key_t;

/* Structure for packet forwarding decision making */
typedef struct tpm_pkt_frwd {
	tpm_trg_port_type_t trg_port;	/* Bitmap of the ports where the packet is targeted */
					/*(ATTENTION - today only single target port in bm */
	uint8_t trg_queue;		/* Queue id for the target port. */
	uint8_t dummy;
	tpm_gem_port_key_t gem_port;	/* GEM port id. */
} tpm_pkt_frwd_t;

/* Next Parsing Stage options */
typedef enum tpm_parse_stage {
	STAGE_L2_PRIM,
	STAGE_L3_TYPE,
	STAGE_IPv4,
	STAGE_IPv6_GEN,
	STAGE_IPv6_DIP,
	STAGE_IPv6_NH,
	STAGE_IPV6_L4,
	STAGE_CTC_CM,
	STAGE_DONE
} tpm_parse_stage_t;

/*************************************/
/* Classifying flags rules - bitmap **/
/* tpm_pkt_action_t bitmap **********/
/*************************************/
#define     TPM_ACTION_DROP_PK              (0x001)	/* drop packet */
#define     TPM_ACTION_SET_TARGET_PORT      (0x002)	/* set forwarding dest Tx_port(GPON:+GemPort) */
#define     TPM_ACTION_SET_TARGET_QUEUE     (0x004)	/* set forwarding dest Tx_queue */
#define     TPM_ACTION_SET_PKT_MOD          (0x008)	/* set packet modification command */
#define     TPM_ACTION_TO_CPU               (0x010)	/* packet should be trapped to CPU */
#define     TPM_ACTION_MTM                  (0x020)	/* packet is MAC-to-ME */
#define     TPM_ACTION_CUST_CPU_PKT_PARSE   (0x040)	/* packet should be sent to Customization package */
#define     TPM_ACTION_SPEC_MC_VID          (0x080)	/* Specific multicast VID  */
#define     TPM_ACTION_UDP_CHKSUM_CALC      (0x100)	/* UDP Checksum should be updated, due to non-zero udp_checksum */

typedef uint32_t tpm_pkt_action_t;	/*ex: TPM_ACTION_DROP_PK */

/*****************/
/* Packet Action */
/*****************/
typedef struct tpm_rule_action {
	tpm_pkt_action_t pkt_act;
	tpm_parse_stage_t next_phase;
} tpm_rule_action_t;

/******************* Packet fields modification command bitmap ***************/
#define    TPM_MH_SET                       0x00001
#define    TPM_MAC_DA_SET                   0x00002	/*Update the MAC DA Address */
#define    TPM_MAC_SA_SET                   0x00004	/*Update the MAC SA Address */
#define    TPM_VLAN_MOD                     0x00008	/*Update the VLAN Tags (add/del/update) */
#define    TPM_PPPOE_DEL                    0x00010	/*Delete a PPPoE encapsulation */
#define    TPM_PPPOE_ADD                    0x00020	/*Add a PPPoE encapsulation */
#define    TPM_DSCP_SET                     0x00040	/* Set the DSCP value */
#define    TPM_TTL_DEC                      0x00080	/* Decrease the TTL value */
#define    TPM_IPV4_UPDATE                  0x00100	/*Update the IPv4 Header */
#define    TPM_IPV4_SRC_SET                 0x00200	/* Set the IPV4 Source Address */
#define    TPM_IPV4_DST_SET                 0x00400	/* Set the IPV4 Destination Address */
#define    TPM_IPV6_UPDATE                  0x00800	/* Update the IPv6 Header */
#define    TPM_HOPLIM_DEC                   0x01000	/* Decrease the Hop Limit value */
#define    TPM_IPV6_SRC_SET                 0x02000	/* Set the IPV6 Source Address */
#define    TPM_IPV6_DST_SET                 0x04000	/* Set the IPV6 Destination Address */
#define    TPM_L4_SRC_SET                   0x08000	/* Set the L4 Source Port (UDP or TCP) */
#define    TPM_L4_DST_SET                   0x10000	/* Set the L4 Destination Port (UDP or TCP) */

typedef uint32_t tpm_pkt_mod_bm_t;	/*ex: TPM_MAC_DA_SET | TPM_MAC_SA_SET .... */
#define    TPM_CHAIN_NUM_UNLIMITED          (0x1000)

/* Add. Modification flags, used only for tpm_mod_entry_set API */
#define     TPM_INT_L4_TCP        (0x0001)
#define     TPM_INT_L4_UDP        (0x0002)
#define     TPM_INT_SPLIT_MOD     (0x0004)
#define     TPM_INT_UDP_CHECKSUM  (0x0008)
#define     TPM_INT_MC_MOD        (0x0010)

typedef uint32_t tpm_pkt_mod_int_bm_t;  /*ex: TPM_INT_L4_TCP | TPM_INT_L4_UDP ....*/

/****************************************************************/
/*          L2/L3 ACL rule related structures                   */
/****************************************************************/

/* L2 MAC key structure */
typedef struct tpm_mac_key {
	uint8_t mac_da[6];
	uint8_t mac_da_mask[6];
	uint8_t mac_sa[6];
	uint8_t mac_sa_mask[6];
} tpm_mac_key_t;

/* L2 VLAN key structure */
typedef struct tpm_vlan_key {
	uint16_t tpid;
	uint16_t tpid_mask;
	uint16_t vid;
	uint16_t vid_mask;
	uint8_t cfi;
	uint8_t cfi_mask;
	uint8_t pbit;
	uint8_t pbit_mask;
} tpm_vlan_key_t;

/* Ethernet type - for example 0x0800 */
typedef uint16_t tpm_ether_type_key_t;

/* PPP key structure */
typedef struct tpm_pppoe_key {
	uint16_t ppp_session;
	uint16_t ppp_proto;
} tpm_pppoe_key_t;

/* L2 ACL key structure - for creating a new L2 ACL rule */
typedef struct tpm_l2_acl_key {
	tpm_mac_key_t mac;
	tpm_vlan_key_t vlan1;
	tpm_vlan_key_t vlan2;
	tpm_pppoe_key_t pppoe_hdr;
	tpm_ether_type_key_t ether_type;
	tpm_gem_port_key_t gem_port;
} tpm_l2_acl_key_t;

/****************************************************************/
/*          L3 Type - ACL key structure                         */
/****************************************************************/
typedef struct tpm_l3_type_key {
	tpm_ether_type_key_t ether_type_key;
	uint16_t dummy;
	tpm_pppoe_key_t pppoe_key;
} tpm_l3_type_key_t;

/****************************************************************/
/*          IPv4 - ACL key structure                            */
/****************************************************************/
typedef struct tpm_ipv4_acl_key {
	uint8_t ipv4_dscp;
	uint8_t ipv4_dscp_mask;
	uint8_t ipv4_proto;
	uint8_t dummy;
	uint8_t ipv4_src_ip_add[4];
	uint8_t ipv4_src_ip_add_mask[4];
	uint8_t ipv4_dst_ip_add[4];
	uint8_t ipv4_dst_ip_add_mask[4];
	uint16_t l4_src_port;
	uint16_t l4_dst_port;
} tpm_ipv4_acl_key_t;

/****************************************************************/
/*          IPv6 - ACL key structure                            */
/****************************************************************/

typedef enum tpm_nh_iter {
	NH_ITER_0,
	NH_ITER_1,
} tpm_nh_iter_t;

typedef struct tpm_ipv6_gen_acl_key {
	uint8_t ipv6_dscp;
	uint8_t ipv6_dscp_mask;
	uint8_t hop_limit;
	uint8_t dummy;
	uint8_t ipv6_src_ip_add[16];
	uint8_t ipv6_src_ip_add_mask[16];
} tpm_ipv6_gen_acl_key_t;

typedef struct tpm_ipv6_acl_key {
	uint8_t ipv6_dscp;
	uint8_t ipv6_dscp_mask;
	uint8_t ipv6_next_header;
	uint8_t ipv6_hoplimit;
	uint8_t ipv6_src_ip_add[16];
	uint8_t ipv6_src_ip_add_mask[16];
	uint8_t ipv6_dst_ip_add[16];
	uint8_t ipv6_dst_ip_add_mask[16];
	uint16_t l4_src_port;
	uint16_t l4_dst_port;
} tpm_ipv6_acl_key_t;

typedef struct tpm_ipv6_addr_key {
	uint8_t ipv6_ip_add[16];
	uint8_t ipv6_ip_add_mask[16];
} tpm_ipv6_addr_key_t;

typedef struct tpm_l4_ports_key {
	uint16_t l4_src_port;
	uint16_t l4_dst_port;
} tpm_l4_ports_key_t;

/****************************************************************/
/*          IPV4 Multicast definitions                          */
/****************************************************************/
typedef enum tpm_mc_port_mode {
	TPM_MC_UNI_MODE_EXCLUDE,
	TPM_MC_UNI_MODE_TRANSPARENT,
	TPM_MC_UNI_MODE_STRIP,
	TPM_MC_UNI_MODE_TRANSLATE
} tpm_mc_port_mode_t;

typedef struct tpm_mc_vid_port_cfg {
	tpm_src_port_type_t tpm_src_port;
	tpm_mc_port_mode_t mc_uni_port_mode;
	uint32_t uni_port_vid;
} tpm_mc_vid_port_cfg_t;

typedef struct {
	tpm_mc_vid_port_cfg_t mc_vid_port_vids[TPM_MAX_NUM_UNI_PORTS];
} tpm_mc_vid_port_vid_set_t;

/* Port IGMP forward mode */
typedef enum {
	TPM_IGMP_FRWD_MODE_DROP,
	TPM_IGMP_FRWD_MODE_FORWARD,
	TPM_IGMP_FRWD_MODE_SNOOPING,
} tpm_igmp_frwd_mode_t;

/****************************************************************/
/*          Packet modification structures                      */
/****************************************************************/

/* VLAN modification commands */
typedef enum tpm_vlan_oper {
	VLANOP_NOOP,	/* no VLAN operation performed */
	VLANOP_EXT_TAG_MOD,	/* modify external tag */
	VLANOP_EXT_TAG_DEL,	/* delete external tag */
	VLANOP_EXT_TAG_INS,	/* insert(prepend) external tag */
	VLANOP_EXT_TAG_MOD_INS,	/* modify existing external tag and insert(prepend) new tag */
	VLANOP_INS_2TAG,	/* insert(prepend) 2 new tags */
	VLANOP_MOD_2TAG,	/* modify 2 tags */
	VLANOP_SWAP_TAGS,	/* swap internal and external tags */
	VLANOP_DEL_2TAG,	/* delete 2 existing tags */
	VLANOP_INT_TAG_MOD,	/* modify existing internal tag */
	VLANOP_EXT_TAG_DEL_INT_MOD,	/* delete existing external tag and modify internal tag */
	VLANOP_SPLIT_MOD_PBIT, /* split mod stage 2, only modify p-bit */
	VLANOP_ILLEGAL,	/* illegal VLAN operation */

	VLANOP_MAX_NUM
} tpm_vlan_oper_t;

typedef enum tpm_mod_owner {
	TPM_MOD_OWNER_TPM,
	TPM_MOD_OWNER_CPU,
	TPM_MAX_MOD_OWNERS
} tpm_mod_owner_t;

typedef struct tpm_vlan_mod {
	tpm_vlan_oper_t vlan_op;	/* Vlan operation/modification command */
	tpm_vlan_key_t vlan1_out;	/* structure for outer VLAN key */
	tpm_vlan_key_t vlan2_out;	/* structure for inner VLAN key. */
} tpm_vlan_mod_t;

typedef struct tpm_pkt_mod {
	uint16_t mh_mod;
	tpm_vlan_mod_t vlan_mod;
	tpm_mac_key_t mac_mod;
	tpm_pppoe_key_t pppoe_mod;
	union {
		tpm_ipv4_acl_key_t ipv4_mod;
		tpm_ipv6_acl_key_t ipv6_mod;
	} l3;
} tpm_pkt_mod_t;

/****************************************************************/
/*          CPU loopback related structures                     */
/****************************************************************/

typedef struct {
	uint32_t in_use;
	uint32_t trg_port;
	uint8_t trg_queue;
	uint16_t gem_port;
	uint32_t rule_idx;	/* PnC rule index returned when add PnC rule to GMAC0 and mod to PON MAC for CPU loopback */
	uint32_t mod_idx;	/* Modification index returned when add mod to GMAC1 for CPU loopback */
} tpm_cpu_loopback_t;

/****************************************************************/
/*          PnC aging counter structure & defines               */
/****************************************************************/

typedef struct {
	uint32_t rule_idx;
	uint32_t hit_count;
} tpm_api_entry_count_t;

typedef struct {
	uint32_t cntr_grp;
	uint32_t lu_mask;
} tpm_api_lu_conf_t;

#define TPM_MAX_LU_ENTRY_NUM  (100)	/* Max. allowed returned LU entries in single API call */
#define TPM_MAX_MOD_RULE_NUM  (32)	/* YUVAL_NOW - add description here */
#define TPM_MAX_PNC_COUNTER_NUM  (450)  /* Max allowed returned PNC counter entries in single API call
					   Root cause for not using 512 is 4K copy_to_user limitation  */

/****************************************************************/
/*          Generic key structure from PnC - for GetNext entry  */
/****************************************************************/

typedef struct tpm_rule_l2_prim_key {
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_l2_acl_key_t l2_key;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm;
	tpm_rule_action_t rule_action;
} tpm_rule_l2_prim_key_t;

typedef struct tpm_rule_l3_type_key {
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_l3_type_key_t l3_key;
	tpm_parse_flags_t parse_flags_bm;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_rule_action_t rule_action;
} tpm_rule_l3_type_key_t;

typedef struct tpm_rule_ipv4_key {
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t parse_rule_bm;
	tpm_ipv4_acl_key_t ipv4_key;
	tpm_parse_flags_t parse_flags_bm;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm;
	tpm_rule_action_t rule_action;
} tpm_rule_ipv4_key_t;

typedef struct tpm_rule_ipv4_mc_key {
	uint16_t vid;
	uint8_t ipv4_src_add[4];
	uint8_t ipv4_dest_add[4];
	uint8_t ignore_ipv4_src;
	tpm_trg_port_type_t dest_port_bm;
} tpm_rule_ipv4_mc_key_t;

typedef struct tpm_rule_ipv6_mc_key {
	uint16_t vid;
	uint8_t ipv6_dest_add[16];
	tpm_trg_port_type_t dest_port_bm;
} tpm_rule_ipv6_mc_key_t;

typedef struct tpm_rule_ipv6_key {
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_src_port_type_t src_port;
	tpm_ipv6_acl_key_t ipv6_key;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_mod_t pkt_mod;
	tpm_rule_action_t rule_action;
} tpm_rule_ipv6_key_t;

typedef struct tpm_rule_ipv6_gen_key {
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_src_port_type_t src_port;
	tpm_l4_ports_key_t l4_key;
	tpm_ipv6_gen_acl_key_t ipv6_gen_key;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm;
	tpm_rule_action_t rule_action;
} tpm_rule_ipv6_gen_key_t;

typedef struct tpm_rule_ipv6_dip_key {
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_src_port_type_t src_port;
	tpm_l4_ports_key_t l4_key;
	tpm_ipv6_gen_acl_key_t ipv6_gen_key;
	tpm_ipv6_addr_key_t ipv6_dipkey;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm;
	tpm_rule_action_t rule_action;
} tpm_rule_ipv6_dip_key_t;

typedef struct tpm_rule_ipv6_l4_key {
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_src_port_type_t src_port;
	tpm_l4_ports_key_t l4_key;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_mod_t pkt_mod;
	tpm_pkt_mod_bm_t pkt_mod_bm;
	tpm_rule_action_t rule_action;
} tpm_rule_ipv6_l4_key_t;

typedef struct tpm_rule_ipv6_nh_key {
	tpm_parse_fields_t parse_rule_bm;
	tpm_parse_flags_t parse_flags_bm;
	tpm_nh_iter_t nh_iter;
	uint32_t nh;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_rule_action_t rule_action;
} tpm_rule_ipv6_nh_key_t;

typedef struct tpm_rule_cnm_key {
	tpm_src_port_type_t src_port;
	tpm_parse_fields_t l2_parse_rule_bm;
	tpm_parse_fields_t ipv4_parse_rule_bm;
	tpm_parse_fields_t ipv6_parse_rule_bm;
	tpm_l2_acl_key_t l2_key;
	tpm_ipv4_acl_key_t ipv4_key;
	tpm_ipv6_acl_key_t ipv6_key;
	uint32_t pbits;
	tpm_pkt_frwd_t pkt_frwd;
	tpm_pkt_action_t pkt_act;
} tpm_rule_cnm_key_t;

typedef union tpm_rule_entry {
	tpm_rule_l2_prim_key_t l2_prim_key;
	tpm_rule_l3_type_key_t l3_type_key;
	tpm_rule_ipv4_key_t ipv4_key;
	tpm_rule_ipv6_key_t ipv6_key;
	tpm_rule_ipv4_mc_key_t ipv4_mc_key;
	tpm_rule_ipv6_gen_key_t ipv6_gen_key;
	tpm_rule_ipv6_mc_key_t ipv6_mc_key;
	tpm_rule_ipv6_dip_key_t ipv6_dip_key;
	tpm_rule_ipv6_nh_key_t ipv6_nh_key;
	tpm_rule_ipv6_l4_key_t ipv6_l4_key;
	tpm_rule_cnm_key_t cnm_key;
} tpm_rule_entry_t;

/*MRU type*/
typedef enum {
	TPM_NETA_MTU_GMAC0,
	TPM_NETA_MTU_GMAC1,
	TPM_NETA_MTU_PONMAC,
	TPM_NETA_MTU_SWITCH
} tpm_mru_type_t;

/******************************************************************************/
/********************************** Switch defs *******************************/
/******************************************************************************/
#define MEMBER_EGRESS_UNMODIFIED	0
#define NOT_A_MEMBER			1
#define MEMBER_EGRESS_UNTAGGED		2
#define MEMBER_EGRESS_TAGGED		3

typedef enum {
	TPM_FLOOD_UNKNOWN_UNICAST,
	TPM_FLOOD_UNKNOWN_MULTI_BROAD_CAST,
} tpm_flood_type_t;

/* switch scheduler type */
typedef enum tpm_sw_sched_type {
	TPM_SW_SCHED_WRR_ALL_Q,
	TPM_SW_SCHED_STRICT_Q3_WRR_Q210,
	TPM_SW_SCHED_STRICT_Q32_WRR_Q10,
	TPM_SW_SCHED_STRICT_ALL_Q,
	TPM_SW_SCHED_ILLEGAL
} tpm_sw_sched_type_t;

/*
* typedef: enum tpm_phy_speed_t
*
* Description: Enumeration of Phy Speed
*
* Enumerations:
*	TPM_PHY_SPEED_10_MBPS   - 10Mbps
*	TPM_PHY_SPEED_100_MBPS	- 100Mbps
*	TPM_PHY_SPEED_1000_MBPS - 1000Mbps
*/
typedef enum {
	TPM_PHY_SPEED_10_MBPS,
	TPM_PHY_SPEED_100_MBPS,
	TPM_PHY_SPEED_1000_MBPS
} tpm_phy_speed_t;

/* switch mirror type */


/* Switch mirror type
typedef struct {
	uint32_t sport;
	uint32_t dport;
	tpm_sw_mirror_type_t mode;
	bool enable;
} tpm_sw_mirror_t;*/

/*
 *  typedef: enum tpm_limit_mode_t
 *
 *  Description: Enumeration of the port ingress/egress rate limit counting mode.
 *
 *  Enumerations:
 *      TPM_SW_LIMIT_FRAME -
 *                Count the number of frames
 *      TPM_SW_LIMIT_LAYER1 -
 *                Count all Layer 1 bytes:
 *                Preamble (8bytes) + Frame's DA to CRC + IFG (12bytes)
 *      TPM_SW_LIMIT_LAYER2 -
 *                Count all Layer 2 bytes: Frame's DA to CRC
 *      TPM_SW_LIMIT_LAYER3 -
 *                Count all Layer 3 bytes:
 *                Frame's DA to CRC - 18 - 4 (if frame is tagged)
 */
typedef enum {
	TPM_SW_LIMIT_FRAME = 0,
	TPM_SW_LIMIT_LAYER1,
	TPM_SW_LIMIT_LAYER2,
	TPM_SW_LIMIT_LAYER3
} tpm_limit_mode_t;

/*
* typedef: enum tpm_autoneg_mode_t
*
* Description: Enumeration of Autonegotiation mode.
*Auto for both speed and duplex.
*Auto for speed only and Full duplex.
*Auto for speed only and Half duplex. (1000Mbps is not supported)
*Auto for duplex only and speed 1000Mbps.
*Auto for duplex only and speed 100Mbps.
*Auto for duplex only and speed 10Mbps.
*1000Mbps Full duplex.
*100Mbps Full duplex.
*100Mbps Half duplex.
*10Mbps Full duplex.
*10Mbps Half duplex.
*/
typedef enum {
	TPM_SPEED_AUTO_DUPLEX_AUTO,
	TPM_SPEED_1000_DUPLEX_AUTO,
	TPM_SPEED_100_DUPLEX_AUTO,
	TPM_SPEED_10_DUPLEX_AUTO,
	TPM_SPEED_AUTO_DUPLEX_FULL,
	TPM_SPEED_AUTO_DUPLEX_HALF,
	TPM_SPEED_1000_DUPLEX_FULL,
	TPM_SPEED_1000_DUPLEX_HALF,
	TPM_SPEED_100_DUPLEX_FULL,
	TPM_SPEED_100_DUPLEX_HALF,
	TPM_SPEED_10_DUPLEX_FULL,
	TPM_SPEED_10_DUPLEX_HALF
} tpm_autoneg_mode_t;

/*
* typedef: enum tpm_phy_loopback_mode_t
*
* Description: Enumeration of Phy loopback mode
*
* Enumerations:
	TPM_PHY_INTERNAL_LOOPBACK: internal loopback mode
	TPM_PHY_EXTERNAL_LOOPBACK:external loopback mode
*/
typedef enum {
	TPM_PHY_INTERNAL_LOOPBACK,
	TPM_PHY_EXTERNAL_LOOPBACK
} tpm_phy_loopback_mode_t;

/*
* typedef: enum tpm_vlan_member_mode_t
*
* Description: Enumeration of VLAN member mode
*
*/
typedef enum {
	TPM_VLAN_EGRESS_UNMODIFIED,
	TPM_VLAN_NOT_A_MEMBER,
	TPM_VLAN_EGRESS_UNTAGGED,
	TPM_VLAN_EGRESS_TAGGED
} tpm_vlan_member_mode_t;

/* switch mirror type */
typedef enum {
	TPM_SW_MIRROR_INGRESS,
	TPM_SW_MIRROR_EGRESS
} tpm_sw_mirror_type_t;

/* Switch mirror type */
typedef struct {
	uint32_t sport;
	uint32_t dport;
	tpm_sw_mirror_type_t mode;
	bool enable;
	uint8_t dummy1;
	uint16_t dummy2;
} tpm_sw_mirror_t;

/*  typedef: struct tpm_sw_pirl_customer_t*/
typedef struct {
	uint32_t ebsLimit;
	uint32_t cbsLimit;
	uint32_t bktIncrement;
	uint32_t bktRateFactor;
	bool isValid;
	uint8_t dummy1;
	uint16_t dummy2;
} tpm_sw_pirl_customer_t;

/******************* Scheduler definitions *****************************/
#define     TPM_PP_SCHED_STRICT             (0x001)	/* strict priority */
#define     TPM_PP_SCHED_WRR                (0x002)	/* WRR - weighted round robin */

/* Packet Processor - scheduler type - bitmap - ex: TPM_PP_SCHED_STRICT | TPM_PP_SCHED_WRR */
typedef uint32_t tpm_pp_sched_type_t;

/* Ethernet counters 1 */
typedef struct {
	uint32_t fcsErrors;
	uint32_t excessiveCollisionCounter;
	uint32_t lateCollisionCounter;
	uint32_t frameTooLongs;
	uint32_t bufferOverflowsOnReceive;
	uint32_t bufferOverflowsOnTransmit;
	uint32_t singleCollisionFrameCounter;
	uint32_t multipleCollisionsFrameCounter;
	uint32_t sqeCounter;
	uint32_t deferredTransmissionCounter;
	uint32_t internalMacTransmitErrorCounter;
	uint32_t carrierSenseErrorCounter;
	uint32_t alignmentErrorCounter;
	uint32_t internalMacReceiveErrorCounter;

} tpm_swport_pm_1_t;

/* Ethernet counters 3 */
typedef struct {
	uint32_t dropEvents;
	uint32_t octets;
	uint32_t packets;
	uint32_t broadcastPackets;
	uint32_t multicastPackets;
	uint32_t undersizePackets;
	uint32_t fragments;
	uint32_t jabbers;
	uint32_t packets_64Octets;
	uint32_t packets_65_127Octets;
	uint32_t packets_128_255Octets;
	uint32_t packets_256_511Octets;
	uint32_t packets_512_1023Octets;
	uint32_t packets_1024_1518Octets;
} tpm_swport_pm_3_t;

/******************************************************************************/
/********************************** Initialisation defs ***********************/
/******************************************************************************/

#define MV_TPM_UN_INITIALIZED_INIT_PARAM   (0xFFFF)

/* UpStream and DownStream general convention */
#define TPM_DOWNSTREAM          (0)
#define TPM_UPSTREAM            (1)

/* Definitions for array sizes */
#define     TPM_NUM_GMACS                   (3)
#define     TPM_MAX_NUM_TX_QUEUE            (8)
#define     TPM_MAX_NUM_RX_QUEUE            (8)
#define     TPM_MAX_NUM_PORTS               (10)	/* WAN 1 + UNI 0..7 + 1 VIRT_UNI see  tpm_src_port_type_t */
#define     TPM_NUM_VLAN_ETYPE_REGS         (4)
#define     TPM_MAX_MOD_CHAIN_NUM           (16)
#define     TPM_MAX_TPID_COMB_NUM           (16)

typedef enum {
	TPM_DIR_DS,
	TPM_DIR_US,
	TPM_NUM_DIRECTIONS
} tpm_dir_t;

typedef enum {
	TPM_PON_WAN_DUAL_MAC_INT_SWITCH = 1,
	TPM_PON_WAN_G0_INT_SWITCH,
	TPM_PON_WAN_G1_LAN_G0_INT_SWITCH,
	TPM_G0_WAN_G1_INT_SWITCH,
	TPM_G1_WAN_G0_INT_SWITCH,
	TPM_PON_G1_WAN_G0_INT_SWITCH,
	TPM_PON_G0_WAN_G1_INT_SWITCH,
	TPM_PON_WAN_DUAL_MAC_EXT_SWITCH,
	TPM_PON_WAN_G1_MNG_EXT_SWITCH,
	TPM_PON_WAN_G0_SINGLE_PORT,
	TPM_PON_WAN_G1_SINGLE_PORT,
	TPM_PON_G1_WAN_G0_SINGLE_PORT,
	TPM_PON_G0_WAN_G1_SINGLE_PORT,
	TPM_PON_WAN_G0_G1_LPBK
} tpm_eth_complex_profile_t;

typedef enum {
	TPM_PNC_LAST_ENTRY_INIT_DROP,	/* Last (default) entry in PnC is hardcoded to DROP */
	TPM_PNC_LAST_ENTRY_INIT_TRAP	/* Last (default) entry in PnC is hardcoded to TRAP to CPU */
} tpm_init_pnc_last_init_t;

typedef enum {
	TPM_SCHED_SP,
	TPM_SCHED_WRR
} tpm_init_sched_t;

typedef enum {
	TPM_PORT_DOWN,
	TPM_PORT_UP
} tpm_init_port_admin_t;

typedef enum {
	TPM_SPEED_10M,
	TPM_SPEED_100M,
	TPM_SPEED_1G,
	TPM_SPEED_AUTO
} tpm_init_port_speed_t;

typedef enum {
	TPM_DUP_HALF,
	TPM_DUP_FULL,
	TPM_DUP_AUTO
} tpm_init_port_duplex_t;

typedef enum {
	TPM_CONN_DISC,
	TPM_CONN_QSGMII,
	TPM_CONN_FE_PHY,
	TPM_CONN_GE_PHY,
	TPM_CONN_RGMII1,
	TPM_CONN_RGMII2
} tpm_init_chip_conn_t;

/* Possible internal ethernet connnection devices for an external ethernet port */
typedef enum {
	TPM_INTCON_GMAC0,
	TPM_INTCON_GMAC1,
	TPM_INTCON_SWITCH
} tpm_init_int_conn_t;

/* Possible Physical internal connections for a Packet Processor Ethernet GMAC (GMAC0/GMAC1) */
typedef enum {
	TPM_GMAC_CON_DISC,	/* Disconnected */
	TPM_GMAC_CON_QSGMII,	/* Quad SGMII */
	TPM_GMAC_CON_SWITCH_4,	/* Amber Switch Port #4 */
	TPM_GMAC_CON_SWITCH_5,	/* Amber Switch Port #5 */
	TPM_GMAC_CON_SWITCH_6,	/* Amber Switch Port #6 */
	TPM_GMAC_CON_RGMII1,
	TPM_GMAC_CON_RGMII2,
	TPM_GMAC_CON_GE_PHY	/* Internal Gig PHY */
} tpm_init_gmac_conn_t;

typedef enum {
	TPM_Q_OWNER_CPU,
	TPM_Q_OWNER_GMAC0,
	TPM_Q_OWNER_GMAC1,
	TPM_Q_OWNER_PMAC,
	TPM_Q_OWNER_MAX
} tpm_init_txq_owner_t;

typedef enum {
	TPM_EPON,
	TPM_GPON,
	TPM_P2P,
	TPM_NONE
} tpm_init_pon_type_t;

typedef enum {
	TPM_ACTIVE_WAN,
	TPM_STNDBY_WAN
} tpm_init_wan_mode_t;

typedef enum {
	TPM_TX_MOD_GMAC0,	/* = TPM_ENUM_GMAC_0 */
	TPM_TX_MOD_GMAC1,	/*= TPM_ENUM_GMAC_1 */
	TPM_TX_MOD_PMAC_0,	/* = TPM_ENUM_PMAC */
	TPM_TX_MOD_PMAC_1,
	TPM_TX_MOD_PMAC_2,
	TPM_TX_MOD_PMAC_3,
	TPM_TX_MOD_PMAC_4,
	TPM_TX_MOD_PMAC_5,
	TPM_TX_MOD_PMAC_6,
	TPM_TX_MOD_PMAC_7,
	TPM_MAX_NUM_TX_PORTS
} tpm_init_tx_mod_t;

typedef struct {
	uint32_t omci_etype;

	uint32_t oam_channel_configured;	/* omci or oam channel configured */
	uint16_t omci_gemport;
	uint32_t oam_cpu_rx_q;	/* omci cpu rx q or oam cpu rx q */
	uint32_t oam_cpu_tx_q;	/* omci cpu tx q or oam cpu tx q */
	uint32_t oam_cpu_tx_port;	/* omci tcount or oam llid */
	uint32_t pnc_init_debug_port;
	tpm_init_pon_type_t pon_type;

} tpm_init_misc_t;

/* Structure holds the IGMP/MLD settings */
typedef struct {
	uint32_t valid;
	uint32_t igmp_snoop;
	uint32_t igmp_cpu_queue;
} tpm_init_igmp_t;

/* Structure holds the physical connections of all external Ethernet ports */
typedef struct {
	uint32_t		valid;
	tpm_init_gmac_conn_t	conn;
	tpm_src_port_type_t	port_src;
} tpm_init_gmac_conn_conf_t;

/* Structure holds the BM Pool Buffers for small packets
 * (contribution to shared pool) and large packets(pool per port) */
typedef struct {
	uint32_t valid;
	uint32_t small_pkt_buffers;	/* Defines number of small packet pool BM buffers */
	uint32_t large_pkt_buffers;	/* Defines number of large packet pool BM buffers */
} tpm_init_gmac_bufs_t;

/* Structure holds the physical connections of all external Ethernet ports */
typedef struct {
	uint32_t valid;
	tpm_src_port_type_t port_src;		/* Port number according to Device port numbering scheme	*/
	tpm_init_chip_conn_t chip_connect;	/* Connection to physical output lines on SoC			*/
	tpm_init_int_conn_t int_connect;	/* Internal Ethernet device port is connected to		*/
	uint32_t switch_port;			/* Port of Internal_Switch, the port is connected to		*/
} tpm_init_eth_port_conf_t;

typedef struct {
	uint32_t valid;
	tpm_init_sched_t sched_method;
	tpm_init_txq_owner_t queue_owner;
	uint32_t owner_queue_num;
	uint32_t queue_size;
	uint32_t queue_weight;
} tpm_init_gmac_txq_t;

typedef struct {
	uint32_t valid;
	uint32_t queue_size;
} tpm_init_gmac_rxq_t;

/*Structure defines the details of a Packet Processor Tx component  */
typedef struct {
	uint32_t valid;	/* Defines if the Tx component is used */
	tpm_init_gmac_txq_t tx_queue[TPM_MAX_NUM_TX_QUEUE];	/* Config. of each of the 8 Tx queue of this Tx component */
} tpm_init_gmac_tx_t;

typedef struct {
	uint32_t valid;	/* Defines if any of the GMAC queues are defined */
	tpm_init_gmac_rxq_t rx_queue[TPM_MAX_NUM_RX_QUEUE];	/* Config. of each of the 8 Rx queue of this GMAC component */
} tpm_init_gmac_rx_t;

typedef enum {
	TPM_INVALID_RANGE = -1,
	TPM_MIN_RANGE,
	TPM_PNC_MNGMT_DS = TPM_MIN_RANGE,
	TPM_PNC_MAC_LEARN,
	TPM_PNC_CPU_WAN_LPBK_US,
	TPM_PNC_NUM_VLAN_TAGS,
	TPM_PNC_MULTI_LPBK,
	TPM_PNC_VIRT_UNI,
	TPM_PNC_LOOP_DET_US,
	TPM_PNC_L2_MAIN,
	TPM_PNC_ETH_TYPE,
	TPM_PNC_IGMP,
	TPM_PNC_IPV4_MC_DS,
	TPM_PNC_IPV4_MAIN,
	TPM_PNC_TCP_FLAG,
	TPM_PNC_TTL,
	TPM_PNC_IPV4_PROTO,
	TPM_PNC_IPV4_FRAG,
	TPM_PNC_IPV4_LEN,
	TPM_PNC_IPV6_NH,
	TPM_PNC_IPV6_L4_MC_DS,
	TPM_PNC_IPV6_L4,
	TPM_PNC_IPV6_HOPL,
	TPM_PNC_IPV6_MC_SIP,
	TPM_PNC_IPV6_GEN,
	TPM_PNC_IPV6_MC_DS,
	TPM_PNC_IPV6_DIP,
	TPM_PNC_CNM_IPV4_PRE,
	TPM_PNC_CNM_MAIN,
	TPM_PNC_CATCH_ALL,
	TPM_MAX_RANGE = TPM_PNC_CATCH_ALL,
	TPM_MAX_NUM_RANGES
} tpm_pnc_ranges_t;

typedef enum {
	TPM_RANGE_TYPE_ACL,
	TPM_RANGE_TYPE_TABLE
} tpm_range_type_t;

typedef enum {
	TPM_MH_SRC_RX_CTRL = 0,
	TPM_MH_SRC_PNC_RI,
} tpm_init_mh_src_t;

typedef enum {
	TPM_VALID_DISABLED = 0,
	TPM_VALID_ENABLED
} tpm_init_tpm_validation_t;

typedef enum {
	TPM_ENUM_RESET_LEVEL0,
	TPM_ENUM_RESET_LEVEL1,
	TPM_ENUM_MAX_RESET_LEVEL = TPM_ENUM_RESET_LEVEL1
} tpm_reset_level_enum_t;

typedef enum {
	TPM_IPV6_5T_DISABLED = 0,
	TPM_IPV6_5T_ENABLED,
} tpm_init_ipv6_5t_enable_t;

typedef enum {
	TPM_CTC_CM_DISABLED = 0,
	TPM_CTC_CM_ENABLED,
} tpm_init_ctc_cm_enable_t;

typedef enum {
	TPM_CTC_CM_IPv6_FIRST_24B = 0,
	TPM_CTC_CM_IPv6_SECOND_24B,
} tpm_ctc_cm_ipv6_parse_win_t;
typedef enum {
	TPM_SPLIT_MOD_MODE_CTC = 0,
	TPM_SPLIT_MOD_MODE_TR156,
} tpm_init_split_mod_mode_t;
typedef enum {
	TPM_PNC_MAC_LEARN_DISABLED = 0,
	TPM_PNC_MAC_LEARN_ENABLED,
} tpm_init_pnc_mac_learn_enable_t;

typedef enum {
	TPM_VIRT_UNI_DISABLED = 0,
	TPM_VIRT_UNI_ENABLED
} tpm_init_virt_uni_enable_t;

typedef enum {
	TPM_CFG_PNC_PARSE_DISABLED,
	TPM_CFG_PNC_PARSE_ENABLED
} tpm_init_cfg_pnc_parse_t;

typedef enum {
	TPM_CPU_LOOPBACK_DISABLED,
	TPM_CPU_LOOPBACK_ENABLED
} tpm_init_cpu_loopback_t;

typedef enum {
	TPM_DOUBLE_TAG_DISABLED,
	TPM_DOUBLE_TAG_ENABLED
} tpm_init_double_tag_t;

typedef enum {
	TPM_CFG_MODE_APPL = 0,
	TPM_CFG_MODE_KERNEL
} tpm_config_mode_t;

typedef enum {
	TPM_FLUSH_ATU_ALL = 0,
	TPM_FLUSH_ATU_DYNAMIC
} tpm_flush_atu_type_t;

typedef struct {
	tpm_pnc_ranges_t range_num;
	tpm_range_type_t range_type;
	uint32_t range_size;	/* Range's Physical PnC size */
	uint32_t cntr_grp;
	uint32_t lu_mask;
	tpm_reset_level_enum_t min_reset_level;
	uint32_t valid;
} tpm_init_pnc_range_t;

typedef struct {
	uint16_t v1_tpid;
	uint16_t v2_tpid;
} tpm_init_tpid_comb_t;

typedef struct {
	uint32_t opt_num;
	tpm_init_tpid_comb_t opt[TPM_MAX_TPID_COMB_NUM];
} tpm_init_tpid_opt_t;

typedef struct {
	tpm_init_virt_uni_enable_t enabled;
	tpm_src_port_type_t uni_port;
	uint32_t switch_port;
} tpm_init_virt_uni_t;

typedef struct {
	uint32_t udp_checksum_use_init_bm;
	uint32_t udp_checksum_update;
} tpm_init_mod_params_t;

typedef struct {
	uint16_t opcode;
	uint16_t data;
	uint8_t last;
	uint8_t updt_ipv4;
	uint8_t updt_tcp;
	uint8_t dummy;
} tpm_mod2_entry_t;

typedef struct {
	uint32_t entry_id;
	tpm_mod2_entry_t entry_data;
} tpm_mod_rule_t;

typedef enum {
	TPM_MTU_CHECK_DISABLED = 0,
	TPM_MTU_CHECK_ENABLED
} tpm_init_mtu_setting_enable_t;

typedef enum {
	TPM_TTL_ZERO_ACTION_NOTHING = 0,
	TPM_TTL_ZERO_ACTION_DROP,
	TPM_TTL_ZERO_ACTION_TO_CPU
} tpm_init_ttl_illegal_action_t;

typedef enum {
	TPM_TCP_FLAG_NOT_CHECK = 0,
	TPM_TCP_FLAG_CHECK
} tpm_init_tcp_flag_check_t;

typedef enum {
	TPM_ETY_DSA_DISABLE = 0,
	TPM_ETY_DSA_ENABLE
} tpm_init_ety_dsa_enable_t;

typedef enum {
	TPM_PPPOE_ADD_DISABLED = 0,
	TPM_PPPOE_ADD_ENABLED
} tpm_init_pppoe_add_enable_t;
typedef enum {
	TPM_SPLIT_MOD_DISABLED = 0,
	TPM_SPLIT_MOD_ENABLED
} tpm_init_split_mod_enable_t;

typedef enum {
	TPM_MC_ALL_CPU_FRWD,
	TPM_MC_MAC_ONLY_FILTER,
	TPM_MC_COMBINED_IP_MAC_FILTER,
	TPM_MC_IP_ONLY_FILTER,
	TPM_MC_FILTER_MODE_MAX,
} tpm_mc_filter_mode_t;

typedef enum {
	TPM_MC_IGMP_SNOOPING,
	TPM_MC_IGMP_PROXY,
	TPM_MC_IGMP_MODE_MAX,
} tpm_mc_igmp_mode_t;

typedef enum {
	TPM_MTU_IPV4,	/* IPV4 */
	TPM_MTU_IPV6,	/* IPV6 */
} tpm_mtu_ethertype_t;

typedef enum {
	TPM_SELF_CHECK_LEVEL0,
	TPM_SELF_CHECK_LEVEL1,
	TPM_ENUM_MAX_CHECK_LEVEL = TPM_SELF_CHECK_LEVEL1
} tpm_self_check_level_enum_t;

typedef enum tpm_unknown_mac_conf {
	TPM_UNK_MAC_TRAP,/*default rule trap packet to CPU*/
	TPM_UNK_MAC_DROP,/*default rule drop the packet*/
	TPM_UNK_MAC_CONTINUE/*default rule frwd packet to GMAC1*/
} tpm_unknown_mac_conf_t;

typedef struct {
	tpm_init_mtu_setting_enable_t mtu_enable;
	uint32_t ipv4_mtu_us;
	uint32_t ipv4_pppoe_mtu_us;
	uint32_t ipv6_mtu_us;
	uint32_t ipv6_pppoe_mtu_us;
	uint32_t ipv4_mtu_ds;
	uint32_t ipv6_mtu_ds;
} tpm_init_mtu_params_t;

typedef struct {
	uint32_t type;
	uint32_t num;
} tpm_init_chain_data_t;

typedef struct {
	uint32_t chain_num;
	tpm_init_chain_data_t chain_data[TPM_MAX_MOD_CHAIN_NUM];
} tpm_init_mod_chain_t;

typedef struct {
	tpm_mc_filter_mode_t filter_mode;
	uint32_t per_uni_vlan_xlat;
	uint32_t mc_pppoe_enable;
	uint32_t mc_hwf_queue;
	uint32_t mc_cpu_queue;
	uint32_t ipv4_mc_support;
	uint32_t ipv6_mc_support;
} tpm_init_mc_setting_t;

typedef struct {
	tpm_init_split_mod_enable_t split_mod_enable;
	uint8_t p_bit[8];
	uint32_t p_bit_num;
	uint32_t vlan_num;
	tpm_init_split_mod_mode_t split_mod_mode;
} tpm_init_split_mod_params_t;

typedef struct {
	uint8_t		enabled;
	uint8_t		port;
	uint8_t		tx_port;
	uint8_t		tx_queue;
	uint32_t	tgt_port;
	uint32_t	queue_sample_freq;
	uint32_t	thresh_high;
	uint32_t	thresh_low;
} tpm_init_fc_params_t;

typedef struct {
	uint32_t omci_etype;
	tpm_init_pon_type_t pon_type;
	uint32_t igmp_snoop;
	uint32_t igmp_cpu_queue;
	uint32_t igmp_pkt_frwd_mod[TPM_MAX_NUM_ETH_PORTS+1]; /* save space for UNI_ANY */
	uint32_t num_tcont_llid;
	tpm_init_gmac_bufs_t gmac_bp_bufs[TPM_NUM_GMACS];
	uint32_t gmac0_mh_en;
	uint32_t gmac1_mh_en;
	tpm_init_fc_params_t		port_fc_conf;
	tpm_eth_complex_profile_t	eth_cmplx_profile;
	tpm_init_eth_port_conf_t	eth_port_conf[TPM_MAX_NUM_ETH_PORTS];
	tpm_init_gmac_conn_conf_t	gmac_port_conf[TPM_NUM_GMACS];
	uint32_t			backup_wan;

	tpm_init_gmac_tx_t gmac_tx[TPM_MAX_NUM_TX_PORTS];
	tpm_init_gmac_rx_t gmac_rx[TPM_NUM_GMACS];
	tpm_init_pnc_range_t pnc_range[TPM_MAX_NUM_RANGES];
	tpm_init_pnc_last_init_t catch_all_pkt_action;

	tpm_init_mh_src_t ds_mh_set_conf;
	tpm_init_tpm_validation_t validation_en;
	tpm_init_tpid_opt_t tpid_opt;
	uint32_t vlan_etypes[TPM_NUM_VLAN_ETYPE_REGS];
	tpm_init_mod_params_t mod_config;
	tpm_init_mod_chain_t mod_chain;
	tpm_init_cfg_pnc_parse_t cfg_pnc_parse;
	tpm_init_cpu_loopback_t cpu_loopback;
	uint32_t trace_debug_info;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;
	tpm_init_ctc_cm_enable_t ctc_cm_enable;
	tpm_ctc_cm_ipv6_parse_win_t  ctc_cm_ipv6_parse_window;
	tpm_init_virt_uni_t virt_uni_info;

#if 0
	uint16_t vlan1_tpid;
	uint16_t vlan2_tpid;
#endif

	tpm_init_mtu_params_t mtu_config;
	tpm_init_pppoe_add_enable_t pppoe_add_enable;
	uint32_t num_vlan_tags;
	uint32_t cpu_rx_queue;
	tpm_init_ttl_illegal_action_t ttl_illegal_action;
	tpm_init_tcp_flag_check_t tcp_flag_check;
	tpm_init_ety_dsa_enable_t ety_dsa_enable;

	tpm_init_mc_setting_t mc_setting;
	tpm_init_split_mod_params_t split_mod_config;
	uint32_t switch_init;
	tpm_init_pnc_mac_learn_enable_t pnc_mac_learn_enable;
} tpm_init_t;

#define XML_FILE_PATH_LENGTH  (64)
typedef struct {
	char xml_file_path[XML_FILE_PATH_LENGTH];
} tpm_setup_t;

/******************* TPM global ERROR codes  **************************/
typedef enum tpm_error_code {
	TPM_RC_OK,	/*Return ok (=0). */
	ERR_GENERAL,	/*General purpose error. */
	ERR_OWNER_INVALID,	/*Illegal owner id. */
	ERR_SRC_PORT_INVALID,	/*Illegal source port. */
	ERR_RULE_NUM_INVALID,	/*Illegal rule number. */
	ERR_RULE_IDX_INVALID,	/*Illegal rule idx. */
	ERR_PARSE_MAP_INVALID,	/*Illegal parse mapping. */
	ERR_VLAN_OP_INVALID,	/*Illegal VLAN operation. */
	ERR_L2_KEY_INVALID,	/*Illegal key information for creating the ACL rule. */
	ERR_FRWD_INVALID,	/*Illegal forwarding decision for packet. */
	ERR_MOD_INVALID,	/*Illegal modification command on packet. */
	ERR_ACTION_INVALID,	/*Illegal action on the packet. */
	ERR_NEXT_PHASE_INVALID,	/*Illegal next phase for  ACL */
	ERR_RULE_KEY_MISMATCH,	/*Inconsistency between rule_num, src_port, l2_acl_key. */
	ERR_IPV4_NO_CONT_L4,	/*There is no continuous block with L4 src port and destination port in packet. */
	ERR_IPV6_ADD_FAIL,	/*Inconsistency in the building of the IPv6 rule. */
	ERR_MC_STREAM_INVALID,	/*Illegal stream number. */
	ERR_MC_STREAM_EXISTS,	/*Stream number already exists. */
	ERR_MC_DST_PORT_INVALID,	/*Destination port bitmap does not match the UNI ports. */
	ERR_IPV4_MC_DST_IP_INVALID,	/*Destination IP address is not in the MC range. */
	ERR_IPV6_MC_DST_IP_INVALID,	/*Destination IPv6 address is not in the MC range. */
	ERR_IPV6_MC_SRC_IP_INVALID,	/*SRC IPv6 address is not in the MC_SIP range. */
	ERR_OMCI_TCONT_INVALID,	/*Illegal TCONT. */
	ERR_MNGT_TX_Q_INVALID,	/*Illegal management TX queue. */
	ERR_MNGT_CREATE_DUPLICATE_CHANNEL,	/*Create channel that already exists, */
	ERR_MNGT_DEL_CHANNEL_INVALID,	/*Delete an unexisting management channel. */
	ERR_OAM_LLID_INVALID,	/*Illegal LLID. */
	ERR_CFG_GETNEXT_INDEX_INVALID,	/*Illegal get_next index for retrieving existing configuration. */
	ERR_CFG_GETNEXT_DIRECTION_INVALID,	/*Illegal direction set (0-upstream, 1-downstream). */
	ERR_CFG_GET_CHANNEL_INVALID,	/*Get an unexisting management channel. */
	ERR_SW_MAC_INVALID,	/*Illegal MAC address. */
	ERR_SW_MAC_STATIC_NOT_FOUND,	/*Delete an unexisting static MAC. */
	ERR_SW_MAC_PER_PORT_INVALID,	/*Illegal limit of MAC per port (1-265). */
	ERR_SW_NUM_OF_MAC_PER_PORT_INVALID,	/*Illegal limit of MAC per port (1-265). */
	ERR_SW_VID_INVALID,	/*Illegal VLAN ID. */
	ERR_SW_TM_QUEUE_INVALID,	/*Illegal queue id. */
	ERR_SW_TM_WEIGHT_INVALID,	/*Illegal weight value. */
	ERR_SW_TM_WRR_MODE_INVALID,	/*Illegal WRR mode. */
	ERR_API_TYPE_INVALID,	/*Illegal API type */
	ERR_DIR_INVALID,	/*Illegal direction value */
	ERR_NULL_POINTER,	/* NULL pointer */
	ERR_IGMP_NOT_ENABLED,	/* Try to disable IGMP which was not enabled */
	ERR_OUT_OF_RESOURCES,	/* No more resources of certain type */
	ERR_AI_SESS_EXISTS,	/* AI Session already exists */
	ERR_AI_SESS_DELETING,	/* AI Session was deleted in this API call */
	ERR_SW_TM_BUCKET_SIZE_INVALID,	/*Illegal bucket size. */
	ERR_SW_TM_RATE_LIMIT_INVALID,	/*Illegal rate limit. */
	ERR_DELETE_KEY_INVALID,	/* Illegal combination of key / rule_index for delete action */
	ERR_L3_KEY_INVALID,	/*Illegal key information for creating the ACL rule. */
	ERR_IPV4_KEY_INVALID,	/*Illegal key information for creating the ACL rule. */
	ERR_IPV6_KEY_INVALID,	/*Illegal key information for creating the ACL rule. */
	ERR_PORT_IGMP_FRWD_MODE_INVALID,	/*Illegal IGMP forward mode. */
	ERR_IPV6_API_ILLEGAL_CALL,  /* Invoke the IPV6 API which is forbidden in current mode */
	ERR_IPV6_5T_RULE_EXISTS,	/* IPV6 rule already exists */
	ERR_IPV6_5T_FLOW_PARTIAL_MATCHED,	/* IPV6 5-tuple flow is a subflow of another flow */
	ERR_IPV6_5T_RULE_IN_USE,	/* Try to delete a rule which is referred by other flows */
	ERR_IPV6_5T_FLOW_AI_BITS_EXHAUSED, /* No enough AI resource for new subflow */
	ERR_SW_NOT_INIT,	/* TPM was not successfully intiailized . */
	ERR_FEAT_UNSUPPORT,	/* TPM feature not supported  . */
	ERR_API_BUSY,
	ERR_CTC_CM_DB_ERR,  /* CTC CnM DB Error */
	ERR_CTC_CM_PREC_INVALID,   /* CTC CnM Precedence invalid */
	ERR_TPMCHECK_DB_CHAIN_TYPE_MISMATCH,/*DB main chain type check FAIL */
	ERR_TPMCHECK_HW_CHAIN_TYPE_MISMATCH,/*HW main chain type check FAIL */
	ERR_TPMCHECK_BOTH_CHAIN_TYPE_MISMATCH,/*HW & DB main chain type check FAIL */
	ERR_TPMCHECK_DB_MISMATCH,/*tpm check with DB FAIL */
	ERR_TPMCHECK_HW_MISMATCH,/*tpm check with HW FAIL */
	ERR_TPMCHECK_PNC_DB_MISMATCH,/*DB PNC check FAIL */
	ERR_TPMCHECK_PNC_HW_MISMATCH,/*HW PNC check FAIL */
	ERR_TPMCHECK_PMT_DB_MISMATCH,/*DB PMT check FAIL */
	ERR_TPMCHECK_PMT_HW_MISMATCH,/*HW PMT check FAIL */
	ERR_UNKNOWN_MAC_CONF_INVALID,/*Illegal MAC learn default conf*/
} tpm_error_code_t;

#ifdef __cplusplus
}
#endif
#endif
