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
* tpm_internal_types.h
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

#ifndef _TPM_INT_TYPES_H_
#define _TPM_INT_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/

#define TPM_MAX_NUM_GEM_PORT		(4096)

#define TPM_CHAIN_NUM_UNLIMITED		(0x1000)

/* PNC Target TXP, in exact numbering scheme, do not change !! */
typedef enum {
	TPM_INVALID_PNC_TRG = -1,
	TPM_PNC_TRG_CPU,
	TPM_PNC_TRG_GMAC0,
	TPM_PNC_TRG_GMAC1,
	TPM_PNC_TRG_PMAC0,
	TPM_PNC_TRG_PMAC1,
	TPM_PNC_TRG_PMAC2,
	TPM_PNC_TRG_PMAC3,
	TPM_PNC_TRG_PMAC4,
	TPM_PNC_TRG_PMAC5,
	TPM_PNC_TRG_PMAC6,
	TPM_PNC_TRG_PMAC7
} tpm_pnc_trg_t;

typedef enum {
	TPM_INT_CALL = 0,
	TPM_EXT_CALL
} tpm_caller_t;


#define TPM_MAX_VID			(4096)	/* legal VLAN ID = 0-4095 */
#define TPM_MAX_NUM_CTC_PRECEDENCE		(8)

#define PON_PORT			(2)
#define SW_GMAC_0			(0)
#define SW_GMAC_1			(1)


/* MH configuration - fixed values - will be supported via xml params */
#define TPM_GMAC0_AMBER_PORT_NUM	4
#define TPM_GMAC1_AMBER_PORT_NUM	5

/* Hardware GMAC number to HAL Port number conversion, note that the numbers are currently equal */
#define TPM_GMAC0_PP_PORT_NUM		0
#define TPM_GMAC1_PP_PORT_NUM		1
#define TPM_PMAC_PP_PORT_NUM		2

/* retrieve the num of TCONTs and LLIDs from the .config - kernel global params */
#define TPM_GPON_MAX_NUM_OF_T_CONTS	(CONFIG_MV_PON_TCONTS)
#define TPM_EPON_MAX_MAC_NUM		(CONFIG_MV_EPON_LLID)
/* Currently hardcoded, must update if pp_port_numbers will be reshuffled */
#define TPM_GMAC_TO_PP_PORT(x)		(x)

#define TPM_NUM_MC_VID_BITS		     (2)

#define TPM_NUM_IPV6_SUBFLOW_BITS_P1 (1)
#define TPM_NUM_IPV6_SUBFLOW_BITS_P2 (2)
#define TPM_NUM_IPV6_SUBFLOW_BITS    (TPM_NUM_IPV6_SUBFLOW_BITS_P1 + TPM_NUM_IPV6_SUBFLOW_BITS_P2)

#define TPM_NUM_IPV6_MC_SIP_BITS_P1 (1)
#define TPM_NUM_IPV6_MC_SIP_BITS_P2 (2)
#define TPM_NUM_IPV6_MC_SIP_BITS    (TPM_NUM_IPV6_MC_SIP_BITS_P1 + TPM_NUM_IPV6_MC_SIP_BITS_P2)

#define ILLEGAL_API(api_type)		((api_type >= TPM_API_TYPE_ILLEGAL) || (api_type < 0))

#define TPM_INTERNAL_OWNER_ID		(0xABBACDDC)

#define TPM_INVALID_QUEUE		(0xFFFF)

typedef enum {
	TPM_INVALID_SECTION = -1,
	TPM_PNC_MAC_LEARN_ACL,
	TPM_DS_LOAD_BALANCE_ACL,
	TPM_CPU_LOOPBACK_ACL,
	TPM_L2_PRIM_ACL,
	TPM_L3_TYPE_ACL,
	TPM_IPV4_ACL,
	TPM_IPV4_MC,
	TPM_IPV6_NH_ACL,
	TPM_L4_ACL,
	TPM_IPV6_GEN_ACL,
	TPM_IPV6_DIP_ACL,
	TPM_IPV6_MC_ACL,
	TPM_CNM_MAIN_ACL,
	TPM_MAX_NUM_API_SECTIONS	/* Equals to number of entries in enum */
} tpm_api_sections_t;

#define ILLEGAL_API_SEC(api_sec)	((api_sec >= TPM_MAX_NUM_API_SECTIONS) || (api_sec < 0))

typedef enum {
	TPM_AI_DS_MAC_IP_SES,
	TPM_AI_US_MAC_IP_SES,
	TPM_AI_IPV6_DS_SIP_SES,
	TPM_AI_IPV6_US_SIP_SES,
	TPM_AI_IPV6_DS_NH_SES,
	TPM_AI_IPV6_US_NH_SES,
	TPM_AI_IPV6_DS_5T_SES,
	TPM_AI_IPV6_US_5T_SES
} tpm_ai_ses_type_t;

typedef enum {
	SES_IN,
	SES_OUT
} tpm_ai_con_type_t;

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/

/* Amber related */
#define  TPM_AMBER_MH_EN_MASK              (0x800)
/* End Amber related */

/* Following are derivatives of the TPM_PARSE_FLAGs in tpm_types.h */
#define TPM_PARSE_FLAG_TAG1_MASK	(TPM_PARSE_FLAG_TAG1_TRUE | TPM_PARSE_FLAG_TAG1_FALSE)
#define TPM_PARSE_FLAG_TAG2_MASK	(TPM_PARSE_FLAG_TAG2_TRUE | TPM_PARSE_FLAG_TAG2_FALSE)
#define TPM_PARSE_FLAG_MTM_MASK		(TPM_PARSE_FLAG_MTM_TRUE | TPM_PARSE_FLAG_MTM_FALSE)
#define TPM_PARSE_FLAG_TO_CPU_MASK	(TPM_PARSE_FLAG_TO_CPU_TRUE | TPM_PARSE_FLAG_TO_CPU_FALSE)
#define TPM_PARSE_FLAG_L4P_MASK		(TPM_PARSE_FLAG_L4_TCP | TPM_PARSE_FLAG_L4_UDP)
#define TPM_PARSE_FLAG_PPPOE_MASK	(TPM_PARSE_FLAG_PPPOE_TRUE | TPM_PARSE_FLAG_PPPOE_FALSE)
#define TPM_PARSE_FLAG_PPPOE_ADD_MASK	(TPM_PARSE_FLAG_PPPOE_ADD_TRUE | TPM_PARSE_FLAG_PPPOE_ADD_FALSE)
#define TPM_PARSE_FLAG_NO_PROTO_MASK	(TPM_PARSE_FLAG_NO_PROTO_TRUE | TPM_PARSE_FLAG_NO_PROTO_FALSE)
#define TPM_PARSE_FLAG_NO_FRAG_MASK	(TPM_PARSE_FLAG_NO_FRAG_TRUE | TPM_PARSE_FLAG_NO_FRAG_FALSE)
#define TPM_PARSE_FLAG_MC_VID_MASK	(TPM_PARSE_FLAG_MC_VID_TRUE | TPM_PARSE_FLAG_MC_VID_FALSE)
#define TPM_PARSE_FLAG_NH2_ITER_MASK	(TPM_PARSE_FLAG_NH2_ITER_TRUE | TPM_PARSE_FLAG_NH2_ITER_FALSE)
#define TPM_PARSE_FLAG_CNM_IPV4_MASK	(TPM_PARSE_FLAG_CNM_IPV4_TRUE | TPM_PARSE_FLAG_CNM_IPV4_FALSE)
#define TPM_PARSE_FLAG_SPLIT_MOD_MASK	(TPM_PARSE_FLGA_SPLIT_MOD_TRUE | TPM_PARSE_FLGA_SPLIT_MOD_FALSE)

#define TPM_DS_LOAD_BALNC_PARSE_BM_MASK		\
		(TPM_L2_PARSE_MAC_DA | TPM_L2_PARSE_MAC_SA | TPM_L2_PARSE_ONE_VLAN_TAG | TPM_L2_PARSE_TWO_VLAN_TAG |\
		TPM_L2_PARSE_ETYPE | TPM_L2_PARSE_PPPOE_SES | TPM_L2_PARSE_PPP_PROT | TPM_L2_PARSE_GEMPORT)
#define TPM_L2_PARSE_BM_MASK		\
		(TPM_L2_PARSE_MAC_DA | TPM_L2_PARSE_MAC_SA | TPM_L2_PARSE_ONE_VLAN_TAG | TPM_L2_PARSE_TWO_VLAN_TAG |\
		TPM_L2_PARSE_ETYPE | TPM_L2_PARSE_PPPOE_SES | TPM_L2_PARSE_PPP_PROT | TPM_L2_PARSE_GEMPORT)
#define TPM_L3_PARSE_BM_MASK		(TPM_L2_PARSE_ETYPE | TPM_L2_PARSE_PPPOE_SES | TPM_L2_PARSE_PPP_PROT)
#define TPM_IPV4_PARSE_BM_MASK		\
		(TPM_IPv4_PARSE_SIP | TPM_IPv4_PARSE_DIP | TPM_IPv4_PARSE_DSCP | TPM_IPv4_PARSE_PROTO |\
		TPM_PARSE_L4_SRC|TPM_PARSE_L4_DST)
#define TPM_IPV6_GEN_BM_MASK		(TPM_IPv6_PARSE_SIP | TPM_IPv6_PARSE_DSCP | TPM_IPv6_PARSE_HOPL)
#define TPM_IPV6_L4_BM_MASK			(TPM_PARSE_L4_SRC | TPM_PARSE_L4_DST)

/* in PARSE_FLAG_BM - bit#0 and bit#1 - TRUE and FALSE - should not be set simultaneously */
#define TPM_PARSE_FLAG_CHECK_TRUE_FALSE		(0x0003)
/* number of pairs of TRUE/FALSE bits present in the  PARSE_FLAG_BM:
   TAG1 / TAG2 / MTM / TO_CPU / L2                                   */
#define  TPM_PARSE_FLAG_CHECK_FIELD_NUMBER	(6)

/* Following additional TPM_PARSE_FLAGs  for internal use, they must co-exist (not overlap)
   with existing TPM_PARSE_FLAGs in types.h */
#define TPM_PARSE_FLAG_UNI_PORT_PARSE		(0x00000001LL)
#define TPM_PARSE_FLAG_PPPOE_ADD_TRUE		(0x00000002LL)
#define TPM_PARSE_FLAG_PPPOE_ADD_FALSE		(0x00000004LL)
#define TPM_PARSE_FLAG_NO_PROTO_TRUE		(0x00000008LL)
#define TPM_PARSE_FLAG_NO_PROTO_FALSE		(0x00000010LL)
#define TPM_PARSE_FLAG_NO_FRAG_TRUE			(0x00000020LL)
#define TPM_PARSE_FLAG_NO_FRAG_FALSE		(0x00000040LL)
#define TPM_PARSE_FLAG_MC_VID_TRUE			(0x00000080LL)
#define TPM_PARSE_FLAG_MC_VID_FALSE			(0x00000100LL)
#define TPM_PARSE_FLAG_MC_VID_PARSE			(0x00000200LL)
#define TPM_PARSE_FLAG_NH2_ITER_TRUE		(0x00000400LL)
#define TPM_PARSE_FLAG_NH2_ITER_FALSE		(0x00000800LL)
#define TPM_PARSE_FLAG_IPV6_SUBFLOW_PARSE	(0x00001000LL)
#define TPM_PARSE_FLAG_CNM_IPV4_TRUE        (0x00002000LL)
#define TPM_PARSE_FLAG_CNM_IPV4_FALSE       (0x00004000LL)
#define TPM_PARSE_FLGA_SPLIT_MOD_TRUE       (0x00008000LL)
#define TPM_PARSE_FLGA_SPLIT_MOD_FALSE      (0x00010000LL)
#define TPM_PARSE_FLAG_IPV4_PRE_KEY_PARSE   (0x00020000LL)
#define TPM_PARSE_FLAG_CNM_PREC_PARSE       (0x00040000LL)
#define TPM_PARSE_FLAG_IPV6_MC_SIP_PARSE    (0x00080000LL)
#define TPM_PARSE_FLAG_DNRT_DS_TRUNK        (0x00100000LL)	/* Set DNRT_DS_TRUNK bit */

/* Following additional TPM_ACTIONs for internal use, they must co-exist (not overlap)
   with existing TPM_ACTIONs in types.h */
#define TPM_ACTION_SET_UNI_PORT			(0x00000001LL)	/* Set UNI Port */
#define TPM_ACTION_SET_TAG1				(0x00000002LL)	/* Set TAG1 */
#define TPM_ACTION_SET_TAG2				(0x00000004LL)	/* Set TAG2 */
#define TPM_ACTION_SET_L4P_TOG_UDP		(0x00000008LL)	/* Set L4 proto toggle bit to '1' (=UDP) */
#define TPM_ACTION_SET_L4P_TOG_TCP		(0x00000010LL)	/* Set L4 proto toggle bit to '0' (=TCP) */
#define TPM_ACTION_UNSET_DNRT			(0x00000020LL)	/* Unset DNRT bit */
#define TPM_ACTION_UNSET_MC_VID_VALID	(0x00000040LL)	/* Unset MC_VID valid bit */
#define TPM_ACTION_SET_PPPOE			(0x00000080LL)	/* Set PPPOE packet */
#define TPM_ACTION_UNSET_PPPOE			(0x00000100LL)	/* Set not PPPOE packet */
#define TPM_ACTION_SET_NO_PROTO_CHECK	(0x00000200LL)	/* Set no IPV4 proto check */
#define TPM_ACTION_SET_NO_FRAG_CHECK	(0x00000400LL)	/* Set no IPV4 fragment check */
#define TPM_ACTION_SET_ADD_PPPOE		(0x00000800LL)	/* Set PPPOE header is added */
#define TPM_ACTION_UNSET_UNI_PORT		(0x00001000LL)	/* UnSet UNI Port */
#define TPM_ACTION_SET_NH2_ITER			(0x00002000LL)	/* Set NH2 iteration */
#define TPM_ACTION_UNSET_NH2_ITER		(0x00004000LL)	/* Unset NH2 iteration */
#define TPM_ACTION_SET_IPV6_SUBFLOW     (0x00008000LL)	/* Set IPV6 subflow */
#define TPM_ACTION_UNSET_IPV6_SUBFLOW   (0x00010000LL)	/* Unset IPV6 subflow */
#define TPM_ACTION_SET_CNM_IPV4         (0x00020000LL)
#define TPM_ACTION_UNSET_CNM_IPV4       (0x00040000LL)
#define TPM_ACTION_SET_SPLIT_MOD        (0x00080000LL)
#define TPM_ACTION_UNSET_SPLIT_MOD      (0x00100000LL)
#define TPM_ACTION_SET_IPV4_PRE_KEY     (0x00200000LL)
#define TPM_ACTION_UNSET_IPV4_PRE_KEY   (0x00400000LL)
#define TPM_ACTION_SET_CNM_PREC         (0x00800000LL)
#define TPM_ACTION_UNSET_CNM_PREC       (0x01000000LL)
#define TPM_ACTION_UNSET_DNRT_DS_TRUNK  (0x02000000LL)	/* Unset DNRT_DS_TRUNK bit */

#define     TPM_MAX_WRR_WEIGHT             (255)

#define      TPM_MH_LEN                    (2)
#define      TPM_DA_LEN                    (6)
#define      TPM_SA_LEN                    (6)
#define      TPM_VLAN_LEN                  (4)
#define      TPM_ETYPE_LEN                 (2)
#define      TPM_PPPOE_HDR_LEN             (8)
#define      TPM_PPPOE_SES_LEN             (2)
#define      TPM_PPP_PROTO_LEN             (2)
#define      TPM_IPV4_VER_IHL_LEN          (1)
#define      TPM_IPV4_DSCP_LEN             (1)
#define      TPM_IPV4_TOTLEN_LEN           (2)
#define      TPM_IPV4_IDENT_LEN            (2)
#define      TPM_IPV4_FLAG_OFF_LEN         (2)
#define      TPM_IPV4_TTL_LEN              (1)
#define      TPM_IPV4_PROTO_LEN            (1)
#define      TPM_IPV4_SIP_LEN              (4)
#define      TPM_IPV4_DIP_LEN              (4)
#define      TPM_IPV4_L4_PORT_LEN          (2)

#define      TPM_TCP_DUMMY_LEN             (13)
#define      TPM_TCP_FLAGS_LEN             (1)

#define      TPM_IPV6_VER_DSCP_LEN         (1)
#define      TPM_IPV6_DSCP_FLOW_LEN        (1)
#define      TPM_IPV6_FLOW_LEN             (2)
#define      TPM_IPV6_PLLEN_LEN			   (2)
#define      TPM_IPV6_NH_LEN               (1)
#define      TPM_IPV6_HOPLIM_LEN           (1)
#define      TPM_IPV6_SIP_LEN			   (16)
#define      TPM_IPV6_DIP_LEN			   (16)
#define      TPM_L4_SPORT_LEN              (2)
#define      TPM_L4_DPORT_LEN              (2)
#define      TPM_UDP_LEN_LEN               (2)
#define      TPM_L4_CHECKSUM_LEN           (2)
#define      TPM_TCP_SEQ_NUM_LEN	       (4)
#define      TPM_TCP_ACK_NUM_LEN	       (4)
#define      TPM_TCP_OFFSET_LEN            (1)
#define      TPM_TCP_FLAGS_LEN             (1)
#define      TPM_TCP_WIN_LEN               (2)
#define      TPM_TCP_URGTPTR_LEN           (2)

#define      TPM_VLAN_CFI_OFFSET           (12)
#define      TPM_VLAN_PBIT_OFFSET          (13)
#define      TPM_VLAN_VID_OFFSET           (0)

#define      TPM_IPV4_IHL_OFFSET           (0)	/* Offset in the VER_IHL byte */
#define      TPM_IPV4_IHL_MASK             (0x0F)	/* Mask in the  IHL byte */

#define      TPM_IPV4_VER_OFFSET           (4)	/* Offset in the VER_IHL byte */
#define      TPM_IPV4_VER_MASK             (0x0F)	/* Mask in the VER byte */

#define      TPM_IPV4_FRAG_OFFSET          (0)	/* Offset in the FLAGS_FRAGMENT 2_byte */
#define      TPM_IPV4_FRAG_MASK            (0x1FFF)	/* Mask in the FRAGMENT byte */

#define      TPM_IPV4_MF_OFFSET            (13)	/* Offset in the FLAGS_FRAGMENT 2_byte */
#define      TPM_IPV4_MF_MASK              (0x01)	/* Mask in the  FLAGS byte */

#define      TPM_IPV4_TOT_LEN_MASK         (0xFFFF)
#define      TPM_IPV4_IDENT_MASK           (0xFFFF)
#define      TPM_IPV4_TTL_MASK             (0xFF)

#define     MH_UNI_PORT_MASK                (0x000F)
#define     MH_UNI_PORT_OFFSET              (0)
#define     MH_GEM_PORT_MASK                (0x0FFF)
#define     MH_GEM_PORT_OFFSET              (0)

/* Maximum number of pnc ranges a single API can be spanned upon.
 * Max. is currently two, to support IPV6 session DIP + SIP Routing + DSCP API call */
#define     TPM_MAX_API_MULTI_RANGES        (3)

#define     TPM_MAX_AI_REFS                 (4)

#define     TPM_BM_GMAC_0                   (0x04)
#define     TPM_BM_GMAC_1                   (0x10)
#define     TPM_BM_PMAC                     (0x01)

/* Bitmap of the GMACs */
typedef uint32_t tpm_gmac_bm_t;

/* TODO - add Amber to part of dynamc Init */
#define TPM_AMBER                           (1)

/* Direction Bitmap*/
#define TPM_BM_DIR_DS                       (1<<TPM_DIR_DS)
#define TPM_BM_DIR_US                       (1<<TPM_DIR_US)
#define TPM_BM_DIR_BIDIR                    (TPM_BM_DIR_DS|TPM_BM_DIR_US)

#define TPM_RESERVED_WINDOW_SIZE_DEFAULT     0

typedef enum {
	TPM_IP_VER_4,
	TPM_IP_VER_6,
	TPM_IP_VER_MAX,
} tpm_ip_ver_t;

/* TPM flowid update values */
#define     TPM_FLOW_ID_31_16            (0x02)
#define     TPM_FLOW_ID_15_00            (0x01)

#define     TPM_TCAM_PKT_WIDTH           (24)

#define     TPM_SWITCH_NUM_PORTS         (6)

#define     TPM_PNC_LU_REG0              (0)
#define     TPM_PNC_LU_REG1              (1)
#define     TPM_PNC_LU_REG2              (2)
#define     TPM_PNC_ETY_REG              (3)

#define     TPM_PNC_CNM_L2_REG           (5)  /* KEEP IT ZERO, DON'T TOUCH IT!! THIS IS FOR CNM L2 LOOKUP. */
#define     TPM_PNC_IPV6_DIP_REG         (6)
#define     TPM_PNC_NOSHIFT_UPDATE_REG   (7)

#define     TPM_MAX_NUM_TCONT            (8)
#define     TPM_MAX_NUM_GIG_MAC          (2)
#define     TPM_MAX_MOD_RANGES           (1)
#define     TPM_MOD_TABL_SIZE            (256)

#define     TPM_PNC_SIZE                 (512)

#define     TPM_TX_MAX_MH_REGS           (15)

/* TPM modification (z2) definitions */
#define     TPM_MOD2_TABL_SIZE              (4095)
#define     TPM_MOD2_MAC_TRANSLATIONS_NUM   (100)
#define     TPM_MOD2_IP_TRANSLATIONS_NUM    (200)
#define     TPM_MAX_MOD2_FLOWS              ((TPM_MOD2_MAC_TRANSLATIONS_NUM) + (TPM_MOD2_IP_TRANSLATIONS_NUM))

#define     TPM_MOD2_DSCP_DATA_MASK         (0x00FC)	/* IPv4 DSCP */
#define     TPM_MOD2_DSCP_DATA_BIT          (2)

#define     TPM_MOD2_TC_DATA_MASK           (0x0FC0)	/* IPv6 DSCP */
#define     TPM_MOD2_TC_DATA_BIT            (6)

#define     TPM_PNC_ANY_LUID             (0xFFFF)

#define     TPM_TRUE                     (1)
#define     TPM_FALSE                    (0)

#define     TPM_OK                       (0)
#define     TPM_FAIL                     (1)
#define     TPM_NOT_FOUND                (2)

#define     TPM_MC_MAX_STREAM_NUM        (256)
#define     TPM_MC_MAX_MAC_NUM           (256)
#define     TPM_MAX_CPU_LOOPBACK_NUM     (4096)
#define     TPM_MAX_CPU_LOOPBACK_ENTRY   (64)
#define     TPM_MC_MAX_LPBK_ENTRIES_NUM  (128)

#define     TPM_MAX_LU_THRESH_VAL         (0x3ffffff)
#define     TPM_MC_MAX_MVLAN_XITS_NUM    (16)

typedef enum {
	TPM_MOD_MH_SET,	/* 0 - Update the Marvell Header */
	TPM_MOD_MAC_DA_SET,	/* 1 - Update the MAC DA Address */
	TPM_MOD_MAC_SA_SET,	/* 2 - Update the MAC SA Address */
	TPM_MOD_VLAN_MOD,	/* 3 - Update the VLAN Tags (add/del/update) */
	TPM_MOD_PPPOE_DEL,	/* 4 - Delete a PPPoE encapsulation */
	TPM_MOD_PPPOE_ADD,	/* 5 - Add a PPPoE encapsulation */
	TPM_MOD_DSCP_SET,	/* 6 - Set the DSCP value */
	TPM_MOD_TTL_DEC,	/* 7 - Decrease the TTL value */
	TPM_MOD_IPV4_UPDATE,	/* 8 - Update the IPv4 Header */
	TPM_MOD_IPV4_SRC_SET,	/* 9 - Set the IPV4 Source Address */
	TPM_MOD_IPV4_DST_SET,	/* 10 - Set the IPV4 Destination Address */
	TPM_MOD_IPV6_UPDATE,	/* 11 - Update the IPv6 Header */
	TPM_MOD_HOPLIM_DEC,	/* 12 - Decrease the Hop Limit value */
	TPM_MOD_IPV6_SRC_SET,	/* 13 - Set the IPV6 Source Address */
	TPM_MOD_IPV6_DST_SET,	/* 14 - Set the IPV6 Destination Address */
	TPM_MOD_L4_SRC_SET,	/* 15 - Set the L4 Source Port (UDP or TCP) */
	TPM_MOD_L4_DST_SET,	/* 16 - Set the L4 Destination Port (UDP or TCP) */

	TPM_MAX_PKT_MOD_FLAGS
} tpm_pkt_mod_flags_t;

typedef enum {
	TPM_AREA_TYPE_JUMP = 0,
	TPM_AREA_TYPE_CHAIN,
	TPM_AREA_TYPE_MAX,
} tpm_area_type_t;

typedef enum {
	TPM_CHAIN_TYPE_NONE = 0,
	TPM_CHAIN_TYPE_MH,
	TPM_CHAIN_TYPE_MAC,
	TPM_CHAIN_TYPE_VLAN,
	TPM_CHAIN_TYPE_PPPOE,
	TPM_CHAIN_TYPE_IPV6_PPPOE,
	TPM_CHAIN_TYPE_L2,
	TPM_CHAIN_TYPE_IPV4_NAPT,
	TPM_CHAIN_TYPE_IPV4_MC,
	TPM_CHAIN_TYPE_ROUTE,
	TPM_CHAIN_TYPE_IPV6_MC,
	TPM_CHAIN_TYPE_MAX,
} tpm_chain_type_t;

/******************* Internal modification flags *****************************/
typedef enum {
	TPM_MOD_INT_L4_TCP,	/* Update TCP checksum */
	TPM_MOD_INT_L4_UDP,	/* Update UDP checksum */
	TPM_MOD_INT_SPLIT_MOD,	/* First part of split modification */
	TPM_MOD_INT_UDP_CHECKSUM,	/* Update UDP checksum - user control */
	TPM_MAX_INT_MOD_FLAGS
} tpm_pkt_int_mod_flags_t;

/* CNM related */

typedef enum {
	TPM_CNM_IPV4_PRE_FILTER_KEY_1 = 0,
	TPM_CNM_IPV4_PRE_FILTER_KEY_2,
	TPM_CNM_IPV4_PRE_FILTER_KEY_3,
	TPM_CNM_IPV4_PRE_FILTER_KEY_MAX,
} tpm_cnm_ipv4_pre_filter_key_id_t;

#define TPM_CNM_IPV4_PRE_FILTER_KEY_1_MASK    (1 << TPM_CNM_IPV4_PRE_FILTER_KEY_1)
#define TPM_CNM_IPV4_PRE_FILTER_KEY_2_MASK    (1 << TPM_CNM_IPV4_PRE_FILTER_KEY_2)
#define TPM_CNM_IPV4_PRE_FILTER_KEY_3_MASK    (1 << TPM_CNM_IPV4_PRE_FILTER_KEY_3)

#define TPM_CNM_INVALID_IPV4_PRE_FILTER_KEY_ID    (0xFF)

#define TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_KEY     (4)
#define TPM_CNM_MAX_IPV4_PRE_FILTER_RULE_PER_PORT    (7)


/*************************************************************/
/*               STRUCTURES                                  */
/*************************************************************/

/* Structure for mapping tx_mod to HAL port and txp  */
typedef struct {
	uint32_t hal_port;
	uint32_t hal_txp;
} tpm_tx_mod_hal_map_t;

/* Structure for mapping pnc_range to lookup_id */
typedef struct {
	tpm_pnc_ranges_t pnc_range;
	uint32_t lookup_id;
	uint32_t last_lookup_range;
	uint32_t valid;
} tpm_pnc_range_lookup_map_t;

typedef struct {
	uint32_t            enum_in;
	uint8_t str_out[50];
} tpm_str_map_t;

/*****************************/
/* PNC  related structures   */
/*****************************/

/* Structure represents 24byte packet data in TCAM */
typedef struct {
	uint8_t pkt_byte[TPM_TCAM_PKT_WIDTH];
} tpm_pkt_data_t;

/* Structure represents 24byte mask on packet data in TCAM */
typedef tpm_pkt_data_t tpm_pkt_mask_t;

/* Structure represents complete TCAM key */
typedef struct {
	uint32_t lu_id;	/*  TPM_PNC_ANY_LUID , for unmasking LU_ID */
	tpm_gmac_bm_t port_ids;
	tpm_pkt_data_t pkt_data;
	tpm_pkt_mask_t pkt_mask;
	uint32_t add_info_data;
	uint32_t add_info_mask;
} tpm_tcam_all_t;

/* Structure represents complete SRAM entry */
typedef struct {
	uint32_t next_lu_id;
	uint32_t lookup_done;
	uint32_t pnc_queue;	/* 0xFFFF, don't update queue */
	uint32_t res_info_15_0_data;
	uint32_t res_info_15_0_mask;
	uint32_t res_info_23_16_data;
	uint32_t res_info_23_16_mask;
	uint32_t add_info_data;
	uint32_t add_info_mask;
	uint32_t flowid_val;
	uint32_t flowid_updt_mask;
	uint32_t shift_updt_reg;
	uint32_t shift_updt_val;
	uint32_t next_lu_off_reg;
} tpm_sram_all_t;

/* Structure represents PnC entry, (TCAM, SRAM) without Index */
typedef struct {
	tpm_tcam_all_t tcam_entry;
	tpm_sram_all_t sram_entry;
} tpm_pnc_all_t;

#define TPM_MOD2_MAX_PATTERN_ENTRIES    (20)

/* Max number of modification pattern sets:
	1  - jump entry
	7 - main chains
*/
#define TPM_MOD2_MAX_PATTERN_SETS       (8)

/* Structure for fields that may need to be parsed for internal functions (e.g. creating hardcoded pnc entries),
   these fields are not exposed to the API's */
typedef struct {
	uint8_t ipv4_ver;
	uint8_t ipv4_ver_mask;	/* 0-MASK ,  1-PARSE */
	uint8_t ipv4_ihl;
	uint8_t ipv4_ihl_mask;	/* 0-MASK ,  1-PARSE */
	uint16_t ipv4_totlen;
	uint16_t ipv4_totlen_mask;	/* 16-bits */
	uint16_t ipv4_ident;
	uint16_t ipv4_flags;	/* MF Flag at bit 0 */
	uint16_t ipv4_flags_mask;	/* 0-MASK ,  1-PARSE */
	uint8_t ipv4_frag_offset;
	uint8_t ipv4_frag_offset_mask;	/* 0-MASK ,  1-PARSE */
	uint8_t ipv4_ttl;
} tpm_ipv4_add_key_t;

typedef struct {
	uint8_t ipv6_hopl;
	uint16_t ipv6_totlen;
	uint16_t ipv6_totlen_mask;	/* 16-bits */
} tpm_ipv6_add_key_t;

typedef struct {
	uint8_t tcp_flags;
	uint8_t tcp_flags_mask;	/* Bit7-0 - signal each of the TCP flags */
} tpm_tcp_key_t;

#ifdef __cplusplus
}
#endif
#endif				/* _TPM_INT_TYPES_H_ */
