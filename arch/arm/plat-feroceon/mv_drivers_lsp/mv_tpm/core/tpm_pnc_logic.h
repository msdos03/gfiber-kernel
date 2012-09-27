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
* tpm_pnc_logic.h
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
*               Revision: 1.2
*
*******************************************************************************/
#ifndef _TPM_PNCL_H_
#define _TPM_PNCL_H_

#ifdef __cplusplus
extern "C" {
#endif

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/
typedef enum {
	TPM_PNCL_ZERO_OFFSET,	/* Offset 0 */
	TPM_PNCL_L3_OFFSET,	/* L3 Header offset */
	TPM_PNCL_IPV4_OFFSET,	/* IPV4 Header offset */
	TPM_PNCL_IPV6_OFFSET,	/* IPV6 Header offset */
	TPM_PNCL_IPV6_EXT_OFFSET,	/* IPV6 Ext Header offset */
	TPM_PNCL_IPV6_DIP_OFFSET,	/* IPV6 DIP offset */
	TPM_PNCL_TCP_OFFSET,	/* TCP Header offset */
	TPM_PNCL_L4_OFFSET	/* UDP/TCP Header offset, used for L4 ports only */
} tpm_pncl_offset_base_t;

typedef enum {
	TPM_PNCL_L3_OTHER,
	TPM_PNCL_L3_IPV6,
	TPM_PNCL_L3_IPV4_FRAG,
	TPM_PNCL_L3_IPV4_FFRAG,
	TPM_PNCL_L3_IPV4_NFRAG
} tpm_pncl_l3_type_t;

typedef enum {
	TPM_PNCL_L4_OTHER,
	TPM_PNCL_L4_TCP,
	TPM_PNCL_L4_UDP,
	TPM_PNCL_L4_IGMP,
	TPM_PNCL_L4_ICMP
} tpm_pncl_l4_type_t;

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/

#define     TPM_PNCL_NO_QUEUE_UPDATE        0xFFFF

/* SRAM Update sub-fields update mask */
#define     TPM_PNCL_SET_GEM             0x1
#define     TPM_PNCL_SET_TXP             0x2
#define     TPM_PNCL_SET_MOD             0x4
#define     TPM_PNCL_SET_DISC            0x8
#define     TPM_PNCL_SET_RX_SPECIAL      0x10
#define     TPM_PNCL_SET_LUD             0x20
#define     TPM_PNCL_SET_L3              0x40
#define     TPM_PNCL_SET_L4              0x80
#define     TPM_PNCL_SET_COL             0x100
#define     TPM_PNCL_SET_IGMP            0x200
#define     TPM_PNCL_SET_MH_RI           0x400
#define     TPM_PNCL_SET_BC              0x800
#define     TPM_PNCL_SET_MC              0x1000
#define     TPM_PNCL_SET_UC              0x2000
#define     TPM_PNCL_SET_PPPOE           0x4000
#define     TPM_PNCL_SET_TAGGED          0x8000
#define     TPM_PNCL_SET_SPLIT_MOD       0x10000

typedef uint32_t tpm_pncl_sram_updt_bm_t;

#define     TPM_MH_RI_BIT14       0x00004000
#define     TPM_MH_RI_BIT15       0x00008000
#define     TPM_MH_RI_BIT16       0x00010000
#define     TPM_MH_RI_BIT17       0x00020000
#define     TPM_MH_RI_OFFSET      14

/* TPM flowid shift values */
#define     TPM_GEM_FL_SHIFT            (12)
#define     TPM_TXP_FL_SHIFT            (24)
#define     TPM_MOD_FL_SHIFT            (00)
#define     TPM_PKT_TRAP_FL_SHIFT       (00)

/* TPM flowid update values */
#define     TPM_FLOW_NIB_0              (0x01)
#define     TPM_FLOW_NIB_1              (0x02)
#define     TPM_FLOW_NIB_2              (0x04)
#define     TPM_FLOW_NIB_3              (0x08)
#define     TPM_FLOW_NIB_4              (0x10)
#define     TPM_FLOW_NIB_5              (0x20)
#define     TPM_FLOW_NIB_6              (0x40)
#define     TPM_FLOW_NIB_7              (0x80)

#define     TPM_TXP_FL_UPDT_MASK        (TPM_FLOW_NIB_6)
#define     TPM_GEM_FL_UPDT_MASK        (TPM_FLOW_NIB_5|TPM_FLOW_NIB_4|TPM_FLOW_NIB_3)
#define     TPM_MOD_FL_UPDT_MASK        (TPM_FLOW_NIB_2|TPM_FLOW_NIB_1|TPM_FLOW_NIB_0)
#define     TPM_PKT_TRAP_UPDT_MASK      (TPM_FLOW_NIB_3|TPM_FLOW_NIB_2|TPM_FLOW_NIB_1|TPM_FLOW_NIB_0)
#define     TPM_MOD_FL_SPLIT_MOD_UPDT_MASK        (TPM_FLOW_NIB_0)

/* PNC - Parse fields - bitmap ****************/
/* These bitmap values are internal, they are complimentary to the values in tpm_types.h
 * must have different values */
#define                 TPM_L2_PARSE_MH                 (0x8000)
#define                 TPM_L2_PARSE_PPPOE_CTRL         (0x4000)
#define                 TPM_L2_PARSE_PPPOE_HDR		(TPM_L2_PARSE_PPPOE_CTRL | \
								TPM_L2_PARSE_PPPOE_SES | TPM_L2_PARSE_PPP_PROT)
#define                 TPM_PNCL_ONE_TWO_VLAN_TAG       (TPM_L2_PARSE_ONE_VLAN_TAG|TPM_L2_PARSE_TWO_VLAN_TAG)

/* These bitmap values are internal, they are complimentary to the values in tpm_types.h
 * MUST!! have different values, than the values there */
#define                 TPM_IPv4_PARSE_VER               (0x8000)	/* parse IPv4 Version */
#define                 TPM_IPv4_PARSE_IHL               (0x4000)	/* parse IPv4 IHL */
#define                 TPM_IPv4_PARSE_VER_OR_IHL        (TPM_IPv4_PARSE_VER|TPM_IPv4_PARSE_IHL)
#define                 TPM_IPv4_PARSE_TOTLEN            (0x2000)	/* parse IPv4 Total Len */
#define                 TPM_IPv4_PARSE_IDENT             (0x1000)	/* parse IPv4  Identification field */
#define                 TPM_IPv4_PARSE_FLAG_MF           (0x0800)	/* parse IPv4  MF Flag */
#define                 TPM_IPv4_PARSE_FRAG_OFFSET       (0x0400)	/* parse IPv4  Offset */
#define                 TPM_IPv4_PARSE_MF_OR_FRAG        (TPM_IPv4_PARSE_FLAG_MF|TPM_IPv4_PARSE_FRAG_OFFSET)
#define                 TPM_IPv4_PARSE_TTL               (0x0200)	/* parse IPv4  TTL */

#define                 TPM_TCP_PARSE_DUMMY              (0x8000)
#define                 TPM_TCP_PARSE_FLAGS              (0x4000)

/* These bitmap values are internal, they are complimentary to the values in tpm_types.h
 * must have different values, than the values there */
#define                 TPM_IPv6_PARSE_VER               (0x10000)	/* parse IPv6 Version */
#define                 TPM_IPv6_PARSE_VER_OR_DSCP       (TPM_IPv6_PARSE_VER|TPM_IPv6_PARSE_DSCP)
#define 				TPM_IPv6_PARSE_FLOW              (0x20000)	/* parse IPv6 Flow Label */
#define                 TPM_IPv6_PARSE_DSCP_OR_FLOW      (TPM_IPv6_PARSE_DSCP|TPM_IPv6_PARSE_FLOW)
#define                 TPM_IPv6_PARSE_PLLEN             (0x40000)	/* parse IPv6 Payload Len */
#define                 TPM_IPv6_PARSE_HOPLIM            (0x80000)	/* parse IPv6  Hop Limit */

#define                 TPM_PARSE_L4_CHECKSUM       (0x10000)	/* parse L4 Checksum */

#define                 TPM_PARSE_UDP_LEN           (0x20000)	/* parse UDP Len */

#define                 TPM_PARSE_TCPSEQ_NUM       (0x020000)	/* parse TCP Seq num */
#define                 TPM_PARSE_TCPACK_NUM       (0x040000)	/* parse TCP Ack num */
#define                 TPM_PARSE_TCPOFFSET        (0x080000)	/* parse TCP Offset */
#define                 TPM_PARSE_TCPFLAGS         (0x100000)	/* parse TCP Flags */
#define                 TPM_PARSE_TCPWIN           (0x200000)	/* parse TCP window */
#define                 TPM_PARSE_TCPURGTPTR       (0x400000)	/* parse TCP urgent pointer */

#define                 TPM_IPv6_PARSE_ICMP_TYPE         (0x20000)	/* parse IPv6 ICMP Type */
#define                 TPM_IPv6_PARSE_ICMP_CODE         (0x40000)	/* parse IPv6 ICMP Code */

typedef uint32_t tpm_pncl_parse_bm_t;

#define                 TPM_PNC_RI_DISC_BIT             (0)
#define                 TPM_PNC_RI_L4_OFF_BIT           (1)
#define                 TPM_PNC_RI_L3_OFF_BIT           (3)
#define                 TPM_PNC_RI_L3_FF_BIT            (5)
#define                 TPM_PNC_RI_MAC_LEARN_BIT        (9)/*Just for media convert MAC learning*/
#define                 TPM_PNC_RI_MC_BIT               (10)
#define                 TPM_PNC_RI_BC_BIT               (11)
#define                 TPM_PNC_RI_UC_BIT               (12)
#define                 TPM_PNC_RI_PPPOE_BIT            (13)
#define                 TPM_PNC_RI_MH_BIT_14            (14)
#define                 TPM_PNC_RI_MH_BIT_15            (15)
#define                 TPM_PNC_RI_MH_BIT_16            (16)
#define                 TPM_PNC_RI_MH_BIT_17            (17)
/* BIT18 is used by NFP */
#define                 TPM_PNC_RI_VLAN_BIT             (19)
#define                 TPM_PNC_RI_RX_SPECIAL_BIT       (20)
#define                 TPM_PNC_RI_TRAP_BIT             (21) /* Internal to TPM, driver is not aware of this */

typedef struct {
	uint32_t mh_set;
	uint32_t mh_reg;
} tpm_pncl_mh_reg_t;

typedef struct {
	tpm_src_port_type_t src_port;	/* Specific UNI port in MH/DSA Tag */
	tpm_l2_acl_key_t l2_key;	/* L2 fields to parse */
	tpm_l3_type_key_t l3_key;	/* L3 fields to parse */
	tpm_ipv4_acl_key_t ipv4_key;	/* IPv4 fields to parse */
	tpm_ipv4_add_key_t ipv4_add_key;	/* Additional IPv4 fields to parse, that are not exposed to the API */
	tpm_ipv6_acl_key_t ipv6_key;	/* IPv6 fields to parse */
	tpm_ipv6_add_key_t ipv6_add_key;	/* Additional IPv6 fields to parse, that are not exposed to the API */
	tpm_tcp_key_t tcp_key;	/* TCP fields to parse */
	tpm_l4_ports_key_t l4_ports_key;	/* L4 Ports to parse */
	tpm_ipv6_gen_acl_key_t ipv6_gen_key;
	tpm_ipv6_addr_key_t ipv6_dip_key;
} tpm_pncl_pkt_key_t;

/* Structure indicates next logical offset.  There are few type of offset,
 * Either one of the L1/L2/L3 Header Offsets, or a specific sub-field in a header.
 * Specific sub-field
 */
typedef struct {
	tpm_pnc_ranges_t range_id;	/* Determine which pnc range it is working on */
	tpm_pncl_offset_base_t offset_base;	/* Determine if one of L1/L2/L3 Header offset, or specific sub-field offset */
	union {
		tpm_pncl_parse_bm_t l2_subf;	/* Specific sub-field to offset to, if empty, start at offset_base */
		tpm_pncl_parse_bm_t l3_subf;	/* Specific sub-field to offset to, if empty, start at offset_base */
		tpm_pncl_parse_bm_t ipv4_subf;	/* Specific sub-field to offset to, if empty, start at offset_base */
		tpm_pncl_parse_bm_t ipv6_subf;	/* Specific sub-field to offset to, if empty, start at offset_base */
		tpm_pncl_parse_bm_t subf;	/* Easy for coding, we can use it in any case */
		tpm_pncl_parse_bm_t tcp_subf;	/* Specific sub-field to offset to, if empty, start at offset_base */
	} offset_sub;
} tpm_pncl_offset_t;

typedef struct {
	uint32_t lu_id;
	tpm_gmac_bm_t port_ids;	/* GMAC(s) relevant to this entry */
	tpm_pncl_offset_t start_offset;	/* Single bit raised to indicate start of packet */
	tpm_pncl_pkt_key_t pkt_key;
	tpm_pncl_parse_bm_t l2_parse_bm;	/* L2 Parsing bitmap for TCAM entry */
	tpm_pncl_parse_bm_t l3_parse_bm;	/* L2 Parsing bitmap for TCAM entry */
	tpm_pncl_parse_bm_t ipv4_parse_bm;	/* IPV4 Parsing bitmap for TCAM entry */
	tpm_pncl_parse_bm_t ipv6_parse_bm;	/* IPV6 Parsing bitmap for TCAM entry */
	tpm_pncl_parse_bm_t tcp_parse_bm;	/* TCP Parsing bitmap for TCAM entry */
	tpm_pncl_parse_bm_t l4_parse_bm;	/* UDP/TCP L4 ports Parsing bitmap for TCAM entry */
	uint32_t add_info_data;
	uint32_t add_info_mask;
} tpm_pncl_tcam_data_t;

typedef struct {
	uint32_t gem_port;
	uint32_t mod_cmd;
	tpm_pnc_trg_t pnc_target;
} tpm_pncl_flowid_t;

typedef struct {
	/* Next LU */
	uint32_t next_lu_id;
	uint32_t next_lu_off_reg;

	/* Next Shift Offset */
	tpm_pncl_offset_t next_offset;	/* Next Offset Sub-field */
	uint32_t shift_updt_reg;
	/* Queue Update */
	uint32_t pnc_queue;	/* pnc_queue=TPM_PNCL_NO_QUEUE_UPDATE -  don't set queue */

	/* Sram ResultInfo&FlowId update Bitmap */
	tpm_pncl_sram_updt_bm_t sram_updt_bm;
	tpm_pncl_mh_reg_t mh_reg;
	tpm_pncl_l3_type_t l3_type;
	tpm_pncl_l4_type_t l4_type;
	uint32_t pkt_col;
	/* Add Info Update */
	uint32_t add_info_data;
	uint32_t add_info_mask;
	/* flowid values */
	tpm_pncl_flowid_t flow_id_sub;
} tpm_pncl_sram_data_t;

typedef struct {
	tpm_pncl_tcam_data_t pncl_tcam;
	tpm_pncl_sram_data_t pncl_sram;
} tpm_pncl_pnc_full_t;

/*************************************************************/
/*               APIs                                        */
/*************************************************************/

int32_t tpm_pncl_pkt_trap(uint32_t entry, uint32_t lu_id, tpm_gmac_bm_t gmac_bm);
int32_t tpm_pncl_pkt_drop(uint32_t entry, uint32_t lu_id, tpm_gmac_bm_t gmac_bm);
int32_t tpm_pncl_range_reset(tpm_pnc_ranges_t range_id);
int32_t tpm_pncl_entry_set(uint32_t pnc_entry, tpm_pncl_pnc_full_t *pnc_data);
int32_t tpm_pncl_entry_insert(uint32_t pnc_ins_entry, uint32_t pnc_stop_entry, tpm_pncl_pnc_full_t *pnc_data);
int32_t tpm_pncl_update_sram(uint32_t pnc_entry, tpm_pncl_pnc_full_t *pnc_data);
int32_t tpm_pncl_init_cnm_main_ety_ipv6(uint32_t entry_id, tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win);

int32_t tpm_pncl_entry_delete(uint32_t pnc_del_entry, uint32_t pnc_stop_entry);
int32_t tpm_pncl_entry_reset(uint32_t pnc_entry);

int32_t tpm_proc_trg_port_gmac_map(tpm_trg_port_type_t trg_port, tpm_gmacs_enum_t *gmac_port);

int32_t tpm_pncl_tcam_map(tpm_pncl_tcam_data_t *tcam_in, tpm_tcam_all_t *tcam_out);

int32_t tpm_pncl_sram_map(tpm_pncl_sram_data_t *sram_in, tpm_sram_all_t *sram_out, tpm_pncl_tcam_data_t *tcam_in);

#ifdef __cplusplus
}
#endif
#endif				/* _TPM_PNCL_H_ */
