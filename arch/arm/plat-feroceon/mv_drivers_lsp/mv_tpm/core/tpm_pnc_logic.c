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
* tpm_pnc_logic.c
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

#include "tpm_common.h"
#include "tpm_header.h"

typedef int32_t(*tpm_parse_func_t) (uint32_t, tpm_pkt_data_t *, tpm_pkt_mask_t *, tpm_pncl_tcam_data_t *);

/* Local Structures */

/* Structure for parsing tables */
typedef struct {
	uint32_t mandatory;
	uint32_t parse_bm;
	uint32_t field_len;
	uint32_t shift_len;
	tpm_parse_func_t parse_func;
	char field_name[30];
} tpm_parse_tbl_t;

/* Local definitions */

#define IF_ERROR(ret)		\
		if (ret != TPM_OK) {\
			TPM_OS_ERROR(TPM_PNCL_MOD, " recvd ret_code(%d)\n", ret);\
			return(ret);\
		}
#define IF_ERROR_I(ret, i)	\
		if (ret != TPM_OK) {\
			TPM_OS_ERROR(TPM_PNCL_MOD, " recvd ret_code(%d), ind(%d)\n", ret, i);\
			return(ret);\
		}

/* Function Declarations */

int32_t tpm_pncl_mh_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_da_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_sa_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_vlan_parse(uint32_t offset, uint32_t sec_vlan_tag, tpm_pkt_data_t *pkt_data,
			    tpm_pkt_mask_t *pkt_mask, tpm_pncl_tcam_data_t * tcam_in);
int32_t tpm_pncl_vlan1_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_vlan2_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_etype_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_pppoe_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_l3_etype_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_l3_pppoe_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_ver_ihl_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				    tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_tot_len_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				    tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_ident_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_flag_ttl_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_flag_off_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_dscp_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				 tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_proto_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_sip_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_dip_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_l4_src_port_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv4_l4_dst_port_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_l4_src_port_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_l4_dst_port_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_l4_src_port_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_l4_dst_port_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_tcp_flags_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				 tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_ver_dscp_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				     tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_dscp_flow_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				      tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_flow_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				 tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_pllen_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_nh_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_hoplim_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_sip_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_dip_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_udp_len_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_l4_checksum_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_tcp_seq_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_tcp_ack_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_tcp_offset_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_ipv6_tcp_flags_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				      tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_tcp_win_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_tcp_urgtptr_parse(uint32_t offset, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in);

int32_t tpm_pncl_pkt_data_parse(tpm_pncl_tcam_data_t *tcam_in, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask);

int32_t tpm_pncl_log_offset_get(tpm_pncl_offset_t *logic_offset, uint32_t *parse_tbl_ind);
int32_t tpm_pncl_shift_calc(tpm_pncl_offset_t *start_offset, tpm_pncl_offset_t *stop_offset,
			    tpm_pncl_parse_bm_t l2_parse_bm, uint32_t *calc_shift);
int32_t tpm_pncl_tcam_map(tpm_pncl_tcam_data_t *tcam_in, tpm_tcam_all_t *tcam_out);
int32_t tpm_pncl_flowid_map(tpm_pncl_flowid_t *flowid_in, tpm_pncl_sram_updt_bm_t sram_bm_in, uint32_t *flowid_val,
			    uint32_t *flowid_updt_mask);
int32_t tpm_pncl_res_info_map(tpm_pncl_sram_data_t *sram_in, tpm_sram_all_t *sram_out);
int32_t tpm_pncl_sram_map(tpm_pncl_sram_data_t *sram_in, tpm_sram_all_t *sram_out, tpm_pncl_tcam_data_t *tcam_in);
int32_t tpm_pncl_set_ri_bit(tpm_sram_all_t *sram_out, uint32_t ri_bit, uint32_t ri_val);
int32_t tpm_pncl_set_l4(tpm_pncl_l4_type_t l4_type, tpm_sram_all_t *sram_out);
int32_t tpm_pncl_set_l3(tpm_pncl_l3_type_t l3_type, tpm_sram_all_t *sram_out);

/* L2 Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN     SHIFT TO NEXT     PARSING FUNCTION     TEXT */
static tpm_parse_tbl_t pncl_l2_parse_tbl[] = {
	{TPM_TRUE, TPM_L2_PARSE_MH, TPM_MH_LEN, TPM_MH_LEN, &tpm_pncl_mh_parse, "MH"},
	{TPM_TRUE, TPM_L2_PARSE_MAC_DA, TPM_DA_LEN, TPM_DA_LEN, &tpm_pncl_da_parse, "DA"},
	{TPM_TRUE, TPM_L2_PARSE_MAC_SA, TPM_SA_LEN, TPM_SA_LEN, &tpm_pncl_sa_parse, "SA"},
	{TPM_FALSE, TPM_PNCL_ONE_TWO_VLAN_TAG, TPM_VLAN_LEN, TPM_VLAN_LEN, &tpm_pncl_vlan1_parse, "VLAN1"},
	{TPM_FALSE, TPM_L2_PARSE_TWO_VLAN_TAG, TPM_VLAN_LEN, TPM_VLAN_LEN, &tpm_pncl_vlan2_parse, "VLAN2"},
	{TPM_TRUE, TPM_L2_PARSE_ETYPE, TPM_ETYPE_LEN, TPM_ETYPE_LEN, &tpm_pncl_etype_parse, "ETHER_TYPE"},
	{TPM_FALSE, TPM_L2_PARSE_PPPOE_HDR, TPM_PPPOE_HDR_LEN, TPM_PPPOE_HDR_LEN, &tpm_pncl_pppoe_parse, "PPPOE"}
};

/* L3 Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN     SHIFT TO NEXT     PARSING FUNCTION     TEXT */
static tpm_parse_tbl_t pncl_l3_parse_tbl[] = {
	{TPM_TRUE, TPM_L2_PARSE_ETYPE, TPM_ETYPE_LEN, TPM_ETYPE_LEN, &tpm_pncl_l3_etype_parse, "ETHER_TYPE"},
	{TPM_FALSE, TPM_L2_PARSE_PPPOE_HDR, TPM_PPPOE_HDR_LEN, TPM_PPPOE_HDR_LEN, &tpm_pncl_l3_pppoe_parse,
		"PPPOE HDR"},
};

/* IPv4 Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN            SHIFT TO NEXT     PARSING FUNCTION     TEXT */
static tpm_parse_tbl_t pncl_ipv4_parse_tbl[] = {
	{TPM_TRUE, TPM_IPv4_PARSE_VER_OR_IHL, TPM_IPV4_VER_IHL_LEN, TPM_IPV4_VER_IHL_LEN, &tpm_pncl_ipv4_ver_ihl_parse,
		"IPv4 Ver&IHL"},
	{TPM_TRUE, TPM_IPv4_PARSE_DSCP, TPM_IPV4_DSCP_LEN, TPM_IPV4_DSCP_LEN, &tpm_pncl_ipv4_dscp_parse, "IPv4 DSCP"},
	{TPM_TRUE, TPM_IPv4_PARSE_TOTLEN, TPM_IPV4_TOTLEN_LEN, TPM_IPV4_TOTLEN_LEN, &tpm_pncl_ipv4_tot_len_parse,
		"IPv4 TotalLength"},
	{TPM_TRUE, TPM_IPv4_PARSE_IDENT, TPM_IPV4_IDENT_LEN, TPM_IPV4_IDENT_LEN, &tpm_pncl_ipv4_ident_parse,
		"IPv4 Idenitification"},
	{TPM_TRUE, TPM_IPv4_PARSE_MF_OR_FRAG, TPM_IPV4_FLAG_OFF_LEN, TPM_IPV4_FLAG_OFF_LEN, &tpm_pncl_flag_off_parse,
		"IPv4 MF Flag and Offset"},
	{TPM_TRUE, TPM_IPv4_PARSE_TTL, TPM_IPV4_TTL_LEN, TPM_IPV4_TTL_LEN, &tpm_pncl_flag_ttl_parse, "IPv4 TTL"},
	{TPM_TRUE, TPM_IPv4_PARSE_PROTO, TPM_IPV4_PROTO_LEN, TPM_IPV4_PROTO_LEN + 2, &tpm_pncl_ipv4_proto_parse,
		"IPv4 PROTO"},
	{TPM_TRUE, TPM_IPv4_PARSE_SIP, TPM_IPV4_SIP_LEN, TPM_IPV4_SIP_LEN, &tpm_pncl_ipv4_sip_parse, "IPv4 SIP"},
	{TPM_TRUE, TPM_IPv4_PARSE_DIP, TPM_IPV4_DIP_LEN, TPM_IPV4_DIP_LEN, &tpm_pncl_ipv4_dip_parse, "IPv4 DIP"},
	{TPM_TRUE, TPM_PARSE_L4_SRC, TPM_IPV4_L4_PORT_LEN, TPM_IPV4_L4_PORT_LEN, &tpm_pncl_ipv4_l4_src_port_parse,
		"IPV4_L4_SRC"},
	{TPM_TRUE, TPM_PARSE_L4_DST, TPM_IPV4_L4_PORT_LEN, TPM_IPV4_L4_PORT_LEN, &tpm_pncl_ipv4_l4_dst_port_parse,
		"IPV4_L4_DST"}
};

/* IPv6 Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN     SHIFT TO NEXT
		PARSING FUNCTION     		TEXT */
static tpm_parse_tbl_t pncl_ipv6_parse_tbl[] = {
	{TPM_TRUE, TPM_IPv6_PARSE_VER_OR_DSCP, TPM_IPV6_VER_DSCP_LEN, TPM_IPV6_VER_DSCP_LEN,
		&tpm_pncl_ipv6_ver_dscp_parse,  "IPv6 Ver&DSCP"},
	{TPM_TRUE, TPM_IPv6_PARSE_DSCP_OR_FLOW, TPM_IPV6_DSCP_FLOW_LEN, TPM_IPV6_DSCP_FLOW_LEN,
		&tpm_pncl_ipv6_dscp_flow_parse, "IPv6 DSCP&Flow"},
	{TPM_TRUE, TPM_IPv6_PARSE_FLOW, TPM_IPV6_FLOW_LEN, TPM_IPV6_FLOW_LEN,
		&tpm_pncl_ipv6_flow_parse,      "IPv6 Flow"},
	{TPM_TRUE, TPM_IPv6_PARSE_PLLEN, TPM_IPV6_PLLEN_LEN, TPM_IPV6_PLLEN_LEN,
		&tpm_pncl_ipv6_pllen_parse,     "IPv6 Payload len"},
	{TPM_TRUE, TPM_IPv6_PARSE_NH, TPM_IPV6_NH_LEN, TPM_IPV6_NH_LEN,
		&tpm_pncl_ipv6_nh_parse,        "IPv6 Next header"},
	{TPM_TRUE, TPM_IPv6_PARSE_HOPL, TPM_IPV6_HOPLIM_LEN, TPM_IPV6_HOPLIM_LEN,
		&tpm_pncl_ipv6_hoplim_parse,    "IPv6 hop limit"},
	{TPM_TRUE, TPM_IPv6_PARSE_SIP, TPM_IPV6_SIP_LEN, TPM_IPV6_SIP_LEN,
		&tpm_pncl_ipv6_sip_parse,       "IPv6 SIP"},
	{TPM_TRUE, TPM_IPv6_PARSE_DIP, TPM_IPV6_DIP_LEN, TPM_IPV6_DIP_LEN,
		&tpm_pncl_ipv6_dip_parse,       "IPv6 DIP"},
};

/* IPv6 Ext Hdr Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN     SHIFT TO NEXT
		PARSING FUNCTION     		TEXT */
static tpm_parse_tbl_t pncl_ipv6_ext_hdr_parse_tbl[] = {
	{TPM_TRUE, TPM_IPv6_PARSE_NH, TPM_IPV6_NH_LEN, TPM_IPV6_NH_LEN,
		&tpm_pncl_ipv6_nh_parse,        "IPv6 Next header"},
};

static tpm_parse_tbl_t pncl_ipv6_dip_parse_tbl[] = {
	{TPM_TRUE, TPM_IPv6_PARSE_DIP, TPM_IPV6_DIP_LEN, TPM_IPV6_DIP_LEN,
		&tpm_pncl_ipv6_dip_parse,       "IPv6 DIP"},
	{TPM_TRUE, TPM_PARSE_L4_SRC, TPM_L4_SPORT_LEN, TPM_L4_SPORT_LEN,
		&tpm_pncl_l4_src_port_parse,	"L4 source port"},
	{TPM_TRUE, TPM_PARSE_L4_DST, TPM_L4_DPORT_LEN, TPM_L4_DPORT_LEN,
		&tpm_pncl_l4_dst_port_parse,	"L4 dest port"},
};

/* TCP Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN     SHIFT TO NEXT
		PARSING FUNCTION     		TEXT */
static tpm_parse_tbl_t pncl_tcp_parse_tbl[] = {
	{TPM_TRUE, TPM_PARSE_L4_SRC, TPM_L4_SPORT_LEN, TPM_L4_SPORT_LEN,
		&tpm_pncl_l4_src_port_parse,    "L4 source port"},
	{TPM_TRUE, TPM_PARSE_L4_DST, TPM_L4_DPORT_LEN, TPM_L4_DPORT_LEN,
		&tpm_pncl_l4_dst_port_parse,    "L4 dest port"},
	{TPM_TRUE, TPM_PARSE_TCPSEQ_NUM, TPM_TCP_SEQ_NUM_LEN, TPM_TCP_SEQ_NUM_LEN,
		&tpm_pncl_tcp_seq_parse,        "TCP Seq Num"},
	{TPM_TRUE, TPM_PARSE_TCPACK_NUM, TPM_TCP_ACK_NUM_LEN, TPM_TCP_ACK_NUM_LEN,
		&tpm_pncl_tcp_ack_parse,        "TCP Ack Num"},
	{TPM_TRUE, TPM_PARSE_TCPOFFSET, TPM_TCP_OFFSET_LEN, TPM_TCP_OFFSET_LEN,
		&tpm_pncl_tcp_offset_parse,     "TCP Offset"},
	{TPM_TRUE, TPM_PARSE_TCPFLAGS, TPM_TCP_FLAGS_LEN, TPM_TCP_FLAGS_LEN,
		&tpm_pncl_tcp_flags_parse,      "TCP Flags"},
	{TPM_TRUE, TPM_PARSE_TCPWIN, TPM_TCP_WIN_LEN, TPM_TCP_WIN_LEN,
		&tpm_pncl_tcp_win_parse,        "TCP Window"},
	{TPM_TRUE, TPM_PARSE_L4_CHECKSUM, TPM_L4_CHECKSUM_LEN, TPM_L4_CHECKSUM_LEN,
		&tpm_pncl_l4_checksum_parse,    "L4 checksum"},
	{TPM_TRUE, TPM_PARSE_TCPURGTPTR, TPM_TCP_URGTPTR_LEN, TPM_TCP_URGTPTR_LEN,
		&tpm_pncl_tcp_urgtptr_parse,    "TCP Urgent Ptr"},
};

/* L4 Ports Parsing Table */
/*    MAND.           PARSE_BM                FIELD_LEN     SHIFT TO NEXT     PARSING FUNCTION     TEXT */
static tpm_parse_tbl_t pncl_l4_ports_parse_tbl[] = {
	{TPM_TRUE, TPM_PARSE_L4_SRC, TPM_L4_SPORT_LEN, TPM_L4_SPORT_LEN, &tpm_pncl_l4_src_port_parse, "L4 source port"},
	{TPM_TRUE, TPM_PARSE_L4_DST, TPM_L4_DPORT_LEN, TPM_L4_DPORT_LEN, &tpm_pncl_l4_dst_port_parse, "L4 dest port"},
};

/*******************************************************************************
* tpm_pncl_print_pncl()
*
* DESCRIPTION:     The function prints
*
* INPUTS:
* pnc_data            -  PNC entry in logical format
*
* OUTPUTS:
*
* RETURNS:
*
*******************************************************************************/
void tpm_pncl_print_pncl(tpm_pncl_pnc_full_t *pnc_data)
{
	tpm_pncl_pkt_key_t *l_pkt_key = &(pnc_data->pncl_tcam.pkt_key);

	/* TODO - Organize printout to Table */
	printk(KERN_INFO "TCAM Logical:\n");
	printk("lu_id(%02d), port_ids(%02x), start_offset_base(%02d), start_offset_sub(%x)\n",
	       pnc_data->pncl_tcam.lu_id,
	       pnc_data->pncl_tcam.port_ids,
	       pnc_data->pncl_tcam.start_offset.offset_base, pnc_data->pncl_tcam.start_offset.offset_sub.l2_subf);
	TPM_OS_DEB_WAIT();

	if (pnc_data->pncl_tcam.add_info_mask != 0)
		printk("ai_data(%x),  ai_mask(%x)\n",
		       pnc_data->pncl_tcam.add_info_data, pnc_data->pncl_tcam.add_info_mask);

	printk("l2_bm(0x%x), l3_bm(0x%x), ipv4bm(0x%x), ipv6_bm(0x%x)\n",
	       pnc_data->pncl_tcam.l2_parse_bm,
	       pnc_data->pncl_tcam.l3_parse_bm, pnc_data->pncl_tcam.ipv4_parse_bm, pnc_data->pncl_tcam.ipv6_parse_bm);

	TPM_OS_DEB_WAIT();

	printk("src_port(%d), gem_port(%x)\n", l_pkt_key->src_port, l_pkt_key->l2_key.gem_port);
	printk("L2 ethertype(%04x), pppoe_ses(%d), pppoe_proto(%x)\n",
	       l_pkt_key->l2_key.ether_type,
	       l_pkt_key->l2_key.pppoe_hdr.ppp_session, l_pkt_key->l2_key.pppoe_hdr.ppp_proto);

	TPM_OS_DEB_WAIT();
	printk("L3 ethertype(%04x), pppoe_ses(%d), pppoe_proto(%x)\n",
	       l_pkt_key->l3_key.ether_type_key,
	       l_pkt_key->l3_key.pppoe_key.ppp_session, l_pkt_key->l3_key.pppoe_key.ppp_proto);

	TPM_OS_DEB_WAIT();
	tpm_print_mac_key(&(l_pkt_key->l2_key.mac));

	TPM_OS_DEB_WAIT();
	printk("\nVLAN 1:\n");
	tpm_print_vlan_key(&(l_pkt_key->l2_key.vlan1));
	printk("\nVLAN 2:\n");
	tpm_print_vlan_key(&(l_pkt_key->l2_key.vlan2));
	TPM_OS_DEB_WAIT();

	tpm_print_ipv4_key(&(l_pkt_key->ipv4_key), &(l_pkt_key->ipv4_add_key));
	TPM_OS_DEB_WAIT();

	printk("ipv6 L4 Src Port(%d), pv6 L4 Dst Port(%d)\n",
	       pnc_data->pncl_tcam.pkt_key.l4_ports_key.l4_src_port, pnc_data->pncl_tcam.pkt_key.l4_ports_key.l4_dst_port);

	printk("\nSRAM\n");
	printk("next_lu_id(%d), next_lu_off_reg(%d), shift_updt_reg(%d)\n",
	       pnc_data->pncl_sram.next_lu_id, pnc_data->pncl_sram.next_lu_off_reg, pnc_data->pncl_sram.shift_updt_reg);
	printk("next_off_base(%d), lx_subf(%X)\n",
	       pnc_data->pncl_sram.next_offset.offset_base, pnc_data->pncl_sram.next_offset.offset_sub.l2_subf);
	TPM_OS_DEB_WAIT();
	printk("sram_updt_bm(0x%x), l3_type(%d), l4_type(%d), mh_reg(%d,%d), pkt_col(%d)\n",
	       pnc_data->pncl_sram.sram_updt_bm,
	       pnc_data->pncl_sram.l3_type,
	       pnc_data->pncl_sram.l4_type,
	       pnc_data->pncl_sram.mh_reg.mh_set, pnc_data->pncl_sram.mh_reg.mh_reg, pnc_data->pncl_sram.pkt_col);

	TPM_OS_DEB_WAIT();
	printk("pnc_queue(%d), flow_id_gem(%d), flow_id_mod(%d), flow_id_txp(%d), \n",
	       pnc_data->pncl_sram.pnc_queue,
	       pnc_data->pncl_sram.flow_id_sub.gem_port,
	       pnc_data->pncl_sram.flow_id_sub.mod_cmd, pnc_data->pncl_sram.flow_id_sub.pnc_target);

	TPM_OS_DEB_WAIT();
	if (pnc_data->pncl_sram.add_info_mask != 0)
		printk("ai_data(%x),  ai_mask(%x)\n",
		       pnc_data->pncl_sram.add_info_data, pnc_data->pncl_sram.add_info_mask);
}

/*******************************************************************************
* tpm_pncl_mh_parse()
*
* DESCRIPTION:     The function creates the TCAM Marvell Header parsing format,
*                  according to the information that must be parsed (Amber MH, or GPON Header)
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function (should always be zero)
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physicalTCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_mh_parse(uint32_t offset,
			  tpm_pkt_data_t *pkt_data,
			  tpm_pkt_mask_t *pkt_mask,
			  tpm_pncl_tcam_data_t *tcam_in)
{
	tpm_db_int_conn_t int_con;
	uint32_t switch_port;
	int32_t ret_code;
	uint16_t mh = 0;
	tpm_db_pon_type_t pon_type;
	uint32_t switch_init;

	/* Parse MH, depending on field parse request */

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* US, specific UNI Port */
	if (FROM_SPEC_UNI(tcam_in->pkt_key.src_port)) {
		/* Get Amber Switch Port */
		ret_code = tpm_db_intport_int_con_get(tcam_in->pkt_key.src_port, &int_con, &switch_port);
		IF_ERROR(ret_code);

		TPM_OS_DEBUG(TPM_PNCL_MOD, "MH set by UNI_Port_%d -> switch_port(%d)\n",
			     (tcam_in->pkt_key.src_port - TPM_SRC_PORT_UNI_0), switch_port);

		/* Translate switch_port to Marvell Header */
		if (int_con == TPM_INTCON_SWITCH)
			mh = (switch_port << MH_UNI_PORT_OFFSET);
		else {		/* UNI Port, not connected to Switch */
			mh = 0;
		}

		/* Update TCAM Data and Mask */
		*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(mh);
		*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(MH_UNI_PORT_MASK);

	}
	/* DS, set MH according to Gemport */
	else if (tcam_in->l2_parse_bm & TPM_L2_PARSE_GEMPORT) {	/* Specific GPON gemport */
		/* Double-check WAN technology */
		ret_code = tpm_db_pon_type_get(&pon_type);
		IF_ERROR(ret_code);
		if (pon_type != TPM_GPON) {
			TPM_OS_ERROR(TPM_PNCL_MOD, " MH set requested for non-GPON WAN technology \n");
			return(TPM_FAIL);
		}
		mh = ((tcam_in->pkt_key.l2_key.gem_port) << MH_GEM_PORT_OFFSET);

		/* Update TCAM Data and Mask */
		*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(mh);
		*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(MH_GEM_PORT_MASK);

		TPM_OS_DEBUG(TPM_PNCL_MOD, "MH from GemPort\n");
	} else {
		if (tcam_in->pkt_key.src_port == TPM_SRC_PORT_WAN) {
			/* it is for multicast per uni vlan feature
			   in the first loop, pkt goes to G1 from G0 through Switch.
			 */
			ret_code = tpm_db_switch_init_get(&switch_init);
			IF_ERROR(ret_code);
			if (switch_init) {
				mh = tpm_db_eth_port_switch_port_get(TPM_SRC_PORT_WAN);
				if (TPM_DB_ERR_PORT_NUM == mh) {
					TPM_OS_ERROR(TPM_PNCL_MOD, "tpm_db_eth_port_switch_port_get returned %d\n", mh);
					return(TPM_FAIL);
				}
			} else
				mh = 0;

			/* Update TCAM Data and Mask */
			*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(mh);
			*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(MH_UNI_PORT_MASK);
		} else {
			TPM_OS_ERROR(TPM_PNCL_MOD, "Unrecognized MH parsing request received\n");
			return(TPM_FAIL);
		}
	}

	TPM_OS_DEBUG(TPM_PNCL_MOD, "MH(%02x%02x) MH_mask(%02x%02x) \n",
		     pkt_data->pkt_byte[offset],
		     pkt_data->pkt_byte[offset + 1], pkt_mask->pkt_byte[offset], pkt_mask->pkt_byte[offset + 1]);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_da_parse()
*
* DESCRIPTION:     The function creates the TCAM MAC DA parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_da_parse(uint32_t offset,
			  tpm_pkt_data_t *pkt_data,
			  tpm_pkt_mask_t *pkt_mask,
			  tpm_pncl_tcam_data_t *tcam_in)
{
	uint32_t i;
	for (i = 0; i < TPM_DA_LEN; i++) {
		pkt_data->pkt_byte[offset] = tcam_in->pkt_key.l2_key.mac.mac_da[i];
		pkt_mask->pkt_byte[offset] = tcam_in->pkt_key.l2_key.mac.mac_da_mask[i];
		offset++;
	}
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_sa_parse()
*
* DESCRIPTION:     The function creates the TCAM MAC SA parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_sa_parse(uint32_t offset,
			  tpm_pkt_data_t *pkt_data,
			  tpm_pkt_mask_t *pkt_mask,
			  tpm_pncl_tcam_data_t *tcam_in)
{
	uint32_t i;
	for (i = 0; i < TPM_DA_LEN; i++) {
		pkt_data->pkt_byte[offset] = tcam_in->pkt_key.l2_key.mac.mac_sa[i];
		pkt_mask->pkt_byte[offset] = tcam_in->pkt_key.l2_key.mac.mac_sa_mask[i];
		offset++;
	}

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_vlan_parse()
*
* DESCRIPTION:     The function creates a TCAM Vlan Tag parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
* sec_vlan_tag     - States if this is second Vlan Tag
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS: Used by tpm_pncl_vlan1_parse, tpm_pncl_vlan2_parse
*
*******************************************************************************/
int32_t tpm_pncl_vlan_parse(uint32_t offset,
			    uint32_t sec_vlan_tag,
			    tpm_pkt_data_t *pkt_data,
			    tpm_pkt_mask_t *pkt_mask,
			    tpm_pncl_tcam_data_t *tcam_in)
{
	tpm_vlan_key_t *vlan_key;
	uint16_t temp_data;
	uint16_t temp_mask;

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d), sec_vlan_tag(%d)\n", offset, sec_vlan_tag);

	if (sec_vlan_tag == 0)
		vlan_key = &(tcam_in->pkt_key.l2_key.vlan1);
	else
		vlan_key = &(tcam_in->pkt_key.l2_key.vlan2);

	/* Vlan Ethertype */
#if 0
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(vlan_key->tpid);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);
#endif
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(((vlan_key->tpid) & (vlan_key->tpid_mask)));
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons((0xFFFF & (vlan_key->tpid_mask)));

	offset += 2;

	/* VID, pbits, cfi */
	temp_data = (((vlan_key->cfi) & (vlan_key->cfi_mask)) << TPM_VLAN_CFI_OFFSET) +
		    (((vlan_key->pbit) & (vlan_key->pbit_mask)) << TPM_VLAN_PBIT_OFFSET) +
		    (((vlan_key->vid) & (vlan_key->vid_mask)) << TPM_VLAN_VID_OFFSET);

	temp_mask = ((0x01 & vlan_key->cfi_mask) << TPM_VLAN_CFI_OFFSET) +
		    ((0x07 & vlan_key->pbit_mask) << TPM_VLAN_PBIT_OFFSET) +
		    ((0x0FFF & vlan_key->vid_mask) << TPM_VLAN_VID_OFFSET);

	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(temp_data);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(temp_mask);

	return(TPM_OK);

}

/*******************************************************************************
* tpm_pncl_vlan1_parse()
*
* DESCRIPTION:     The function creates a TCAM first Vlan Tag parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_vlan1_parse(uint32_t offset,
			     tpm_pkt_data_t *pkt_data,
			     tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);
	tpm_pncl_vlan_parse(offset, 0, pkt_data, pkt_mask, tcam_in);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_vlan2_parse()
*
* DESCRIPTION:     The function creates a TCAM second Vlan Tag parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_vlan2_parse(uint32_t offset,
			     tpm_pkt_data_t *pkt_data,
			     tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in)
{
	tpm_pncl_vlan_parse(offset, 1, pkt_data, pkt_mask, tcam_in);
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_etype_parse()
*
* DESCRIPTION:     The function creates a TCAM Ethertype parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_etype_parse(uint32_t offset,
			     tpm_pkt_data_t *pkt_data,
			     tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.l2_key.ether_type);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_pppoe_parse()
*
* DESCRIPTION:     The function creates a TCAM PPPoE header parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_pppoe_parse(uint32_t offset,
			     tpm_pkt_data_t *pkt_data,
			     tpm_pkt_mask_t *pkt_mask,
			     tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	if (tcam_in->l2_parse_bm & TPM_L2_PARSE_PPPOE_SES) {
		/* Update TCAM Data and Mask */
		*(uint16_t *) (&(pkt_data->pkt_byte[offset + 2])) =
		htons(tcam_in->pkt_key.l2_key.pppoe_hdr.ppp_session);
		*(uint16_t *) (&(pkt_mask->pkt_byte[offset + 2])) = htons(0xFFFF);
	}
	if (tcam_in->l2_parse_bm & TPM_L2_PARSE_PPP_PROT) {
		/* Update TCAM Data and Mask */
		*(uint16_t *) (&(pkt_data->pkt_byte[offset + 6])) = htons(tcam_in->pkt_key.l2_key.pppoe_hdr.ppp_proto);
		*(uint16_t *) (&(pkt_mask->pkt_byte[offset + 6])) = htons(0xFFFF);
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_l3_etype_parse()
*
* DESCRIPTION:     The function creates a TCAM L3 Ethertype parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_l3_etype_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.l3_key.ether_type_key);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_l3_pppoe_parse()
*
* DESCRIPTION:     The function creates a TCAM PPPoE header parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_l3_pppoe_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	if (tcam_in->l3_parse_bm & TPM_L2_PARSE_PPPOE_SES) {
		/* Update TCAM Data and Mask */
		*(uint16_t *) (&(pkt_data->pkt_byte[offset + 2])) =
		htons(tcam_in->pkt_key.l3_key.pppoe_key.ppp_session);
		*(uint16_t *) (&(pkt_mask->pkt_byte[offset + 2])) = htons(0xFFFF);
	}
	if (tcam_in->l3_parse_bm & TPM_L2_PARSE_PPP_PROT) {
		/* Update TCAM Data and Mask */
		*(uint16_t *) (&(pkt_data->pkt_byte[offset + 6])) = htons(tcam_in->pkt_key.l3_key.pppoe_key.ppp_proto);
		*(uint16_t *) (&(pkt_mask->pkt_byte[offset + 6])) = htons(0xFFFF);
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_ver_ihl_parse()
*
* DESCRIPTION:     The function creates a TCAM ipv4 version and IHL parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_ver_ihl_parse(uint32_t offset,
				    tpm_pkt_data_t *pkt_data,
				    tpm_pkt_mask_t *pkt_mask,
				    tpm_pncl_tcam_data_t *tcam_in)
{
	uint16_t temp_data = 0;
	uint16_t temp_mask = 0;
	tpm_ipv4_add_key_t *ipv4_add_key;

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	ipv4_add_key = &(tcam_in->pkt_key.ipv4_add_key);

	/* Add IHL if requested */
	temp_data |=
	(ipv4_add_key->ipv4_ihl_mask) * ((ipv4_add_key->ipv4_ihl & TPM_IPV4_IHL_MASK) << TPM_IPV4_IHL_OFFSET);
	temp_mask |= (ipv4_add_key->ipv4_ihl_mask) * (TPM_IPV4_IHL_MASK << TPM_IPV4_IHL_OFFSET);

	/* Add IP Version if requested */
	temp_data |=
	(ipv4_add_key->ipv4_ver_mask) * ((ipv4_add_key->ipv4_ver & TPM_IPV4_VER_MASK) << TPM_IPV4_VER_OFFSET);
	temp_mask |= (ipv4_add_key->ipv4_ver_mask) * (TPM_IPV4_VER_MASK << TPM_IPV4_VER_OFFSET);

	/* Update TCAM Data and Mask */
	*(uint8_t *) (&(pkt_data->pkt_byte[offset])) = temp_data;
	*(uint8_t *) (&(pkt_mask->pkt_byte[offset])) = temp_mask;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_tot_len_parse()
*
* DESCRIPTION:     The function creates a TCAM ipv4 version and IHL parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_tot_len_parse(uint32_t offset,
				    tpm_pkt_data_t *pkt_data,
				    tpm_pkt_mask_t *pkt_mask,
				    tpm_pncl_tcam_data_t *tcam_in)
{

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv4_add_key.ipv4_totlen);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv4_add_key.ipv4_totlen_mask);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_flag_ttl_parse()
*
* DESCRIPTION:     The function creates a TCAM ipv4 version and IHL parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_flag_ttl_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint8_t *) (&(pkt_data->pkt_byte[offset])) = tcam_in->pkt_key.ipv4_add_key.ipv4_ttl;
	*(uint8_t *) (&(pkt_mask->pkt_byte[offset])) = TPM_IPV4_TTL_MASK;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_ident_parse()
*
* DESCRIPTION:     The function creates a TCAM ipv4 version and IHL parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_ident_parse(uint32_t offset,
				  tpm_pkt_data_t *pkt_data,
				  tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in)
{

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv4_add_key.ipv4_ident);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = TPM_IPV4_IDENT_MASK;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_flag_off_parse()
*
* DESCRIPTION:     The function creates a TCAM ipv4 version and IHL parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_flag_off_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	uint16_t temp_data = 0;
	uint16_t temp_mask = 0;
	tpm_ipv4_add_key_t *ipv4_add_key;

	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	ipv4_add_key = &(tcam_in->pkt_key.ipv4_add_key);

	/* Add Fragment Offset if requested */
	temp_data |= ((ipv4_add_key->ipv4_frag_offset_mask) *
		      ((ipv4_add_key->ipv4_frag_offset) & TPM_IPV4_FRAG_MASK)) << TPM_IPV4_FRAG_OFFSET;

	temp_mask |= ((ipv4_add_key->ipv4_frag_offset_mask) * TPM_IPV4_FRAG_MASK) << TPM_IPV4_FRAG_OFFSET;

	/* Add MH Flag if requested */
	temp_data |= ((ipv4_add_key->ipv4_flags_mask) *
		      ((ipv4_add_key->ipv4_flags) & TPM_IPV4_MF_MASK)) << TPM_IPV4_MF_OFFSET;

	temp_mask |= ((ipv4_add_key->ipv4_flags_mask) * TPM_IPV4_MF_MASK) << TPM_IPV4_MF_OFFSET;

	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(temp_data);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(temp_mask);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_dscp_parse()
*
* DESCRIPTION:     The function creates a TCAM ipv4 DSCP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_dscp_parse(uint32_t offset,
				 tpm_pkt_data_t *pkt_data,
				 tpm_pkt_mask_t *pkt_mask,
				 tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint8_t *) (&(pkt_data->pkt_byte[offset])) = ((tcam_in->pkt_key.ipv4_key.ipv4_dscp) & 0x3F) << 2;
	*(uint8_t *) (&(pkt_mask->pkt_byte[offset])) = ((tcam_in->pkt_key.ipv4_key.ipv4_dscp_mask) & 0x3F) << 2;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_proto_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv4 protocol parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_proto_parse(uint32_t offset,
				  tpm_pkt_data_t *pkt_data,
				  tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint8_t *) (&(pkt_data->pkt_byte[offset])) = (tcam_in->pkt_key.ipv4_key.ipv4_proto);
	*(uint8_t *) (&(pkt_mask->pkt_byte[offset])) = (0xFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_sip_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv4 source IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_sip_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	memcpy(&(pkt_data->pkt_byte[offset]), tcam_in->pkt_key.ipv4_key.ipv4_src_ip_add, 4);
	memcpy(&(pkt_mask->pkt_byte[offset]), tcam_in->pkt_key.ipv4_key.ipv4_src_ip_add_mask, 4);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_dip_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv4 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_dip_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	memcpy(&(pkt_data->pkt_byte[offset]), tcam_in->pkt_key.ipv4_key.ipv4_dst_ip_add, 4);
	memcpy(&(pkt_mask->pkt_byte[offset]), tcam_in->pkt_key.ipv4_key.ipv4_dst_ip_add_mask, 4);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_l4_src_port_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv4 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_l4_src_port_parse(uint32_t offset,
					tpm_pkt_data_t *pkt_data,
					tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv4_key.l4_src_port);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv4_l4_dst_port_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv4 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv4_l4_dst_port_parse(uint32_t offset,
					tpm_pkt_data_t *pkt_data,
					tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv4_key.l4_dst_port);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_l4_src_port_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_l4_src_port_parse(uint32_t offset,
					tpm_pkt_data_t *pkt_data,
					tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv6_key.l4_src_port);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_l4_dst_port_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_l4_dst_port_parse(uint32_t offset,
					tpm_pkt_data_t *pkt_data,
					tpm_pkt_mask_t *pkt_mask,
					tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv6_key.l4_dst_port);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_l4_src_port_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_l4_src_port_parse(uint32_t offset,
				   tpm_pkt_data_t *pkt_data,
				   tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.l4_ports_key.l4_src_port);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_l4_dst_port_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_l4_dst_port_parse(uint32_t offset,
				   tpm_pkt_data_t *pkt_data,
				   tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.l4_ports_key.l4_dst_port);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(0xFFFF);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_tcp_flags_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv4 destination IP parsing format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_tcp_flags_parse(uint32_t offset,
				 tpm_pkt_data_t *pkt_data,
				 tpm_pkt_mask_t *pkt_mask,
				 tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	pkt_data->pkt_byte[offset] = tcam_in->pkt_key.tcp_key.tcp_flags;
	pkt_mask->pkt_byte[offset] = tcam_in->pkt_key.tcp_key.tcp_flags_mask;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_ver_dscp_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 Version and DSCP parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_ver_dscp_parse(uint32_t offset,
				     tpm_pkt_data_t *pkt_data,
				     tpm_pkt_mask_t *pkt_mask,
				     tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	TPM_OS_DEBUG(TPM_PNCL_MOD, "ipv6 key, dscp: %d, mask: %d\n",
		     tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp, tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp_mask);

	/* Update TCAM Data and Mask */
	/* We don't care version */
	pkt_data->pkt_byte[offset] |= (tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp & 0x3c) >> 2;
	pkt_mask->pkt_byte[offset] |= (tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp_mask & 0x3c) >> 2;
	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_dscp_flow_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 DSCP and FLOW parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_dscp_flow_parse(uint32_t offset,
				      tpm_pkt_data_t *pkt_data,
				      tpm_pkt_mask_t *pkt_mask,
				      tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	TPM_OS_DEBUG(TPM_PNCL_MOD, "ipv6 key, dscp: %d, mask: %d\n",
		     tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp, tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp_mask);
	/* Update TCAM Data and Mask */
	/* We don't care flow */
	pkt_data->pkt_byte[offset] |= (tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp & 0x03) << 6;
	pkt_mask->pkt_byte[offset] |= (tcam_in->pkt_key.ipv6_gen_key.ipv6_dscp_mask & 0x03) << 6;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_flow_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 FLOW parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_flow_parse(uint32_t offset,
				 tpm_pkt_data_t *pkt_data,
				 tpm_pkt_mask_t *pkt_mask,
				 tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care flow */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_IPV6_FLOW_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_IPV6_FLOW_LEN);
	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_pllen_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 Payload len parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_pllen_parse(uint32_t offset,
				  tpm_pkt_data_t *pkt_data,
				  tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	*(uint16_t *) (&(pkt_data->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv6_add_key.ipv6_totlen);
	*(uint16_t *) (&(pkt_mask->pkt_byte[offset])) = htons(tcam_in->pkt_key.ipv6_add_key.ipv6_totlen_mask);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_nh_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 Next Header parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_nh_parse(uint32_t offset,
			       tpm_pkt_data_t *pkt_data,
			       tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	pkt_data->pkt_byte[offset] = tcam_in->pkt_key.ipv6_key.ipv6_next_header;
	pkt_mask->pkt_byte[offset] = 0xFF;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_hoplim_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 Hop Limit parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_hoplim_parse(uint32_t offset,
				   tpm_pkt_data_t *pkt_data,
				   tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */

	/*memset(&pkt_data->pkt_byte[offset], 0x0, TPM_IPV6_HOPLIM_LEN);*/
	/*memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_IPV6_HOPLIM_LEN);*/
	pkt_data->pkt_byte[offset] = tcam_in->pkt_key.ipv6_gen_key.hop_limit;
	pkt_mask->pkt_byte[offset] = 0xff;
	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_ipv6_sip_parse()
*
* DESCRIPTION:     The function creates a TCAM IPv6 SIP parse format.
*
* INPUTS:
* offset           - Offset in PNC TCAM when starting the function
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_ipv6_sip_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	memcpy(&(pkt_data->pkt_byte[offset]), tcam_in->pkt_key.ipv6_gen_key.ipv6_src_ip_add, 16);
	memcpy(&(pkt_mask->pkt_byte[offset]), tcam_in->pkt_key.ipv6_gen_key.ipv6_src_ip_add_mask, 16);

	return(TPM_OK);
}

int32_t tpm_pncl_ipv6_dip_parse(uint32_t offset,
				tpm_pkt_data_t *pkt_data,
				tpm_pkt_mask_t *pkt_mask,
				tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	memcpy(&(pkt_data->pkt_byte[offset]), tcam_in->pkt_key.ipv6_dip_key.ipv6_ip_add, 16);
	memcpy(&(pkt_mask->pkt_byte[offset]), tcam_in->pkt_key.ipv6_dip_key.ipv6_ip_add_mask, 16);
	return(TPM_OK);
}

int32_t tpm_pncl_udp_len_parse(uint32_t offset,
			       tpm_pkt_data_t *pkt_data,
			       tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_UDP_LEN_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_UDP_LEN_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_l4_checksum_parse(uint32_t offset,
				   tpm_pkt_data_t *pkt_data,
				   tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_L4_CHECKSUM_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_L4_CHECKSUM_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_tcp_seq_parse(uint32_t offset,
			       tpm_pkt_data_t *pkt_data,
			       tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_TCP_SEQ_NUM_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_TCP_SEQ_NUM_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_tcp_ack_parse(uint32_t offset,
			       tpm_pkt_data_t *pkt_data,
			       tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_TCP_ACK_NUM_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_TCP_ACK_NUM_LEN);
	return(TPM_OK);
}

int32_t tpm_pncl_tcp_offset_parse(uint32_t offset,
				  tpm_pkt_data_t *pkt_data,
				  tpm_pkt_mask_t *pkt_mask,
				  tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_TCP_OFFSET_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_TCP_OFFSET_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_ipv6_tcp_flags_parse(uint32_t offset,
				      tpm_pkt_data_t *pkt_data,
				      tpm_pkt_mask_t *pkt_mask,
				      tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_TCP_FLAGS_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_TCP_FLAGS_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_tcp_win_parse(uint32_t offset,
			       tpm_pkt_data_t *pkt_data,
			       tpm_pkt_mask_t *pkt_mask,
			       tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_TCP_WIN_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_TCP_WIN_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_tcp_urgtptr_parse(uint32_t offset,
				   tpm_pkt_data_t *pkt_data,
				   tpm_pkt_mask_t *pkt_mask,
				   tpm_pncl_tcam_data_t *tcam_in)
{
	TPM_OS_DEBUG(TPM_PNCL_MOD, " offset_in(%d)\n", offset);

	/* Update TCAM Data and Mask */
	/* We don't care */
	memset(&pkt_data->pkt_byte[offset], 0x0, TPM_TCP_URGTPTR_LEN);
	memset(&pkt_mask->pkt_byte[offset], 0x0, TPM_TCP_URGTPTR_LEN);

	return(TPM_OK);
}

int32_t tpm_pncl_get_parse_tbl(tpm_pncl_offset_t *logic_offset,
			       tpm_parse_tbl_t **tbl, uint32_t *tbl_size)
{
	if (logic_offset->offset_base == TPM_PNCL_ZERO_OFFSET) {
		*tbl = pncl_l2_parse_tbl;
		*tbl_size = sizeof(pncl_l2_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_L3_OFFSET) {
		*tbl = pncl_l3_parse_tbl;
		*tbl_size = sizeof(pncl_l3_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_IPV4_OFFSET) {
		*tbl = pncl_ipv4_parse_tbl;
		*tbl_size = sizeof(pncl_ipv4_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_IPV6_OFFSET) {
		*tbl = pncl_ipv6_parse_tbl;
		*tbl_size = sizeof(pncl_ipv6_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_IPV6_DIP_OFFSET) {
		*tbl = pncl_ipv6_dip_parse_tbl;
		*tbl_size = sizeof(pncl_ipv6_dip_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_IPV6_EXT_OFFSET) {
		*tbl = pncl_ipv6_ext_hdr_parse_tbl;
		*tbl_size = sizeof(pncl_ipv6_ext_hdr_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_TCP_OFFSET) {
		*tbl = pncl_tcp_parse_tbl;
		*tbl_size = sizeof(pncl_tcp_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else if (logic_offset->offset_base == TPM_PNCL_L4_OFFSET) {
		*tbl = pncl_l4_ports_parse_tbl;
		*tbl_size = sizeof(pncl_l4_ports_parse_tbl) / sizeof(tpm_parse_tbl_t);
	} else {
		TPM_OS_ERROR(TPM_PNCL_MOD, "Unsupported offset_base %d \n", logic_offset->offset_base);
		return TPM_FAIL;
	}

	return TPM_OK;
}

/*******************************************************************************
* tpm_pncl_log_offset_get()
*
* DESCRIPTION:     This function takes a TCAM logical Offset field and logical Offset sub-field,
*                  (See tpm_pncl_offset_t) and returns the matching entry in the parsing function table
*                  pncl_l2_parse_tbl.
* INPUTS:
* logic_offset     - Pointer to offset bas & offset sub-field
*
* OUTPUTS:
* parse_tbl_ind   - pointer to physical TCAM packet data (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_log_offset_get(tpm_pncl_offset_t *logic_offset, uint32_t *parse_tbl_ind)
{
	uint32_t i;
	int32_t ret_code;
	tpm_parse_tbl_t *tbl;
	uint32_t tbl_size;

	/* Reset output */
	*parse_tbl_ind = 0;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "log_offset_base(%d) , log_offset_sub(%x)\n",
		     logic_offset->offset_base, logic_offset->offset_sub.subf);

	ret_code = tpm_pncl_get_parse_tbl(logic_offset, &tbl, &tbl_size);
	IF_ERROR(ret_code);

	for (i = 0; i < tbl_size; i++) {
		if (tbl[i].parse_bm & logic_offset->offset_sub.subf) {
			*parse_tbl_ind = i;
			break;
		}
	}

	if (i == tbl_size) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "No start offset sub-field found \n");
		return(TPM_FAIL);
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_pkt_data_parse()
*
* DESCRIPTION: Function translates Logical TCAM structure to physical TCAM packet data and mask
*
* INPUTS:
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* pkt_data         - pointer to physical TCAM packet data (24 bytes)
* pkt_mask         - pointer to physical TCAM packet mask (24 bytes)
*
* RETURNS:
* On success, the function returns TPM_DB_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_pkt_data_parse(tpm_pncl_tcam_data_t *tcam_in, tpm_pkt_data_t *pkt_data, tpm_pkt_mask_t *pkt_mask)
{
	int32_t ret_code;
	int32_t white_space;
	uint32_t start_ind, i;
	uint32_t tcam_offset = 0, field_len, shift_len, size;
	tpm_parse_tbl_t *parse_table;
	tpm_pncl_parse_bm_t parse_bm;

	tpm_pkt_data_t l_pkt_data;
	tpm_pkt_mask_t l_pkt_mask;

	memset(&l_pkt_data, 0, sizeof(tpm_pkt_data_t));
	memset(&l_pkt_mask, 0, sizeof(tpm_pkt_mask_t));

	TPM_OS_DEBUG(TPM_PNCL_MOD, "\n");

	/* Loop over L2 fields, starting from the start_offset sub_field */
	/* 1 - Find start Offset */
	ret_code = tpm_pncl_log_offset_get(&(tcam_in->start_offset), &start_ind);
	TPM_OS_DEBUG(TPM_PNCL_MOD, "start_offset_bm(%x), start_index(%d) \n",
		     tcam_in->start_offset.offset_sub.subf, start_ind);
	IF_ERROR(ret_code);

	ret_code = tpm_pncl_get_parse_tbl(&tcam_in->start_offset, &parse_table, &size);
	IF_ERROR(ret_code);

	if (tcam_in->start_offset.offset_base == TPM_PNCL_ZERO_OFFSET) {
		parse_bm = tcam_in->l2_parse_bm;
	} else if (tcam_in->start_offset.offset_base == TPM_PNCL_L3_OFFSET) {
		parse_bm = tcam_in->l3_parse_bm;
	} else if (tcam_in->start_offset.offset_base == TPM_PNCL_IPV4_OFFSET) {
		parse_bm = tcam_in->ipv4_parse_bm;
	} else if ((tcam_in->start_offset.offset_base == TPM_PNCL_IPV6_OFFSET)
		   || (tcam_in->start_offset.offset_base == TPM_PNCL_IPV6_EXT_OFFSET)
		   || (tcam_in->start_offset.offset_base == TPM_PNCL_IPV6_DIP_OFFSET)
		  ) {
		parse_bm = tcam_in->ipv6_parse_bm;
	} else if (tcam_in->start_offset.offset_base == TPM_PNCL_TCP_OFFSET) {
		parse_bm = tcam_in->tcp_parse_bm;
	} else if (tcam_in->start_offset.offset_base == TPM_PNCL_L4_OFFSET) {
		parse_bm = tcam_in->l4_parse_bm;
	}

	else {
		TPM_OS_ERROR(TPM_PNCL_MOD, "Invalid base offset %d\n", tcam_in->start_offset.offset_base);
		return(TPM_FAIL);
	}

	/* 2 - Perform Parsing Loop */
	for (i = start_ind; i < size; i++) {
		field_len = parse_table[i].field_len;
		shift_len = parse_table[i].shift_len;

		/* 1 - Field must be parsed */
		if (parse_table[i].parse_bm & parse_bm) {
			/* Cannot parse pass TCAM length */
			if ((tcam_offset + field_len) > TPM_TCAM_PKT_WIDTH) {
				TPM_OS_ERROR(TPM_PNCL_MOD, "TCAM Width passed \n");
				return(TPM_FAIL);
			}
			/* Parse the field, by func */
			(*(parse_table[i].parse_func)) (tcam_offset, &l_pkt_data, &l_pkt_mask, tcam_in);
			tcam_offset += field_len;

			/* Must skip untill next field, "skip" means to write masked bytes to TCAM packet data and TCAM mask */
			white_space = shift_len - field_len;
			/* TODO : Check shift_len - field_len > 0 */

		} else if (parse_table[i].mandatory)	/* 2 - Must skip entire current field untill next field */
			white_space = shift_len - 0;
		else	/* 3 - Field doesn't exist in packet, no parsing or skipping */
			white_space = 0;

		/* 4 - Perform generic skipping for all three cases above (no skipping for case 3) */
		if ((tcam_offset + white_space) > TPM_TCAM_PKT_WIDTH)
			white_space = TPM_TCAM_PKT_WIDTH - tcam_offset;

		memset(&(l_pkt_data.pkt_byte[tcam_offset]), 0, white_space);
		memset(&(l_pkt_mask.pkt_byte[tcam_offset]), 0, white_space);
		tcam_offset += white_space;
	}

	/* Set TCAM Data & Mask */
	memcpy(pkt_data, &l_pkt_data, TPM_TCAM_PKT_WIDTH);
	memcpy(pkt_mask, &l_pkt_mask, TPM_TCAM_PKT_WIDTH);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_shift_calc()
*
* DESCRIPTION: Function receives a logical start_offset, stop_offset and parsing bitmap,
*              and calculates the shift_length from start_offset up untill stop_offset
*              The parse bitmap is important, because it indicates which optional fields exist,
*              e.g. Vlan Tags, PPPoE etc.
* INPUTS:
* start_offset     - Start offset structure
* stop_offset      - Stop offset structure
* l2_parse_bm      - Parse bitmap field.
*
* OUTPUTS:
* calc_shift       - Calculated Shift in bytes
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_shift_calc(tpm_pncl_offset_t *start_offset,
			    tpm_pncl_offset_t *stop_offset,
			    tpm_pncl_parse_bm_t parse_bm,
			    uint32_t *calc_shift)
{
	uint32_t start_ind, stop_ind, t_calc_shift = 0;
	uint32_t i;
	uint32_t ret_code, size;
	tpm_parse_tbl_t *parse_table;
	tpm_pncl_parse_bm_t stop_offset_bm;

	if (start_offset->offset_base != stop_offset->offset_base) {
		TPM_OS_ERROR(TPM_PNCL_MOD, "start base offset %d != stop base offset %d\n",
			     start_offset->offset_base, stop_offset->offset_base);
		return(TPM_FAIL);
	}

	ret_code = tpm_pncl_get_parse_tbl(start_offset, &parse_table, &size);
	IF_ERROR(ret_code);

	stop_offset_bm = stop_offset->offset_sub.subf;

	/* Get Start Index */
	ret_code = tpm_pncl_log_offset_get(start_offset, &start_ind);
	IF_ERROR(ret_code);

	/* Get Stop Index */
	if (stop_offset_bm == 0)
		stop_ind = 0;
	else if (stop_offset_bm == 0xFFFF) {
		stop_ind = size;

		if (stop_offset->offset_base == TPM_PNCL_IPV4_OFFSET) {
			*calc_shift = 126;
			return(TPM_OK);
		}

		if (stop_offset->offset_base == TPM_PNCL_IPV6_EXT_OFFSET) {
			*calc_shift = 127;
			return(TPM_OK);
		}
	} else {
		ret_code = tpm_pncl_log_offset_get(stop_offset, &stop_ind);
		IF_ERROR(ret_code);
	}
	if (stop_ind < start_ind)
		TPM_OS_DEBUG(TPM_PNCL_MOD, " stop field < start field, shift=0 \n");

	/* Calc Shift until end (not including last!) */
	for (i = start_ind; i < stop_ind; i++) {
		/* Shift is added to sum if (field is mandatory) or
		 * (field is optional, but it was parsed in TCAM, so it exists) */
		if (parse_table[i].mandatory == TPM_TRUE || ((parse_table[i].parse_bm & parse_bm) > 0))
			t_calc_shift += parse_table[i].shift_len;
	}

	TPM_OS_DEBUG(TPM_PNCL_MOD, " Calculated PNC shift =(%d)\n", t_calc_shift);

	*calc_shift = t_calc_shift;

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_tcam_map()
*
* DESCRIPTION:    Function maps a Logical TCAM structure to a pseudo-physical TCAM Entry
*
*
*
* INPUTS:
* tcam_in          - Logical TCAM structure
*
* OUTPUTS:
* tcam_out         - Physical TCAM Entry
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_tcam_map(tpm_pncl_tcam_data_t *tcam_in, tpm_tcam_all_t *tcam_out)
{
	int32_t ret_code;
	tpm_tcam_all_t temp_tcam_out;
	TPM_OS_DEBUG(TPM_PNCL_MOD, "\n");

	/* AI bits are unchanged */
	temp_tcam_out.add_info_data = tcam_in->add_info_data;
	temp_tcam_out.add_info_mask = tcam_in->add_info_mask;

	/* Lookup is unchanged */
	temp_tcam_out.lu_id = tcam_in->lu_id;

	/* Port Id mask is unchanged */
	temp_tcam_out.port_ids = tcam_in->port_ids;

	/**** Packet Data parsing ****/
	ret_code = tpm_pncl_pkt_data_parse(tcam_in, &(temp_tcam_out.pkt_data), &(temp_tcam_out.pkt_mask));
	IF_ERROR(ret_code);

	/* Set Output */
	memcpy(tcam_out, &temp_tcam_out, sizeof(tpm_tcam_all_t));

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_flowid_map()
*
* DESCRIPTION:    Function maps a Logical Flowid and SRAM bitmap to a pseudo-physical FlowId value and update mask
*
*
* INPUTS:
* flowid_in       - Flowid Data
* sram_bm_in      - Logical SRAM bitmap
*
* OUTPUTS:
* flowid_val      - Physical SRAM flowid value
* flowid_updt_mask- pseudo-physical flowid update mask (mapping to physical maps performed in tpm_pnc_mgr)
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_flowid_map(tpm_pncl_flowid_t *flowid_in,
			    tpm_pncl_sram_updt_bm_t sram_bm_in,
			    uint32_t *flowid_val, uint32_t *flowid_updt_mask)
{

	*flowid_val = 0;
	*flowid_updt_mask = 0;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "\n");
	if (sram_bm_in & TPM_PNCL_SET_GEM) {
		*flowid_val |= (flowid_in->gem_port << TPM_GEM_FL_SHIFT);
		TPM_OS_DEBUG(TPM_PNCL_MOD, "flow_id after gem_addition(%x)\n", *flowid_val);
		*flowid_updt_mask |= TPM_GEM_FL_UPDT_MASK;
	}
	if (sram_bm_in & TPM_PNCL_SET_TXP) {
		*flowid_val |= (flowid_in->pnc_target << TPM_TXP_FL_SHIFT);
		*flowid_updt_mask |= TPM_TXP_FL_UPDT_MASK;
		TPM_OS_DEBUG(TPM_PNCL_MOD, "flow_id after txp_addition(%x)\n", *flowid_val);
	}
	if (sram_bm_in & TPM_PNCL_SET_MOD) {
		*flowid_val |= (flowid_in->mod_cmd << TPM_MOD_FL_SHIFT);
		*flowid_updt_mask |= TPM_MOD_FL_UPDT_MASK;
		TPM_OS_DEBUG(TPM_PNCL_MOD, "flow_id after pkt_mod_addition(%x)\n", *flowid_val);
	}
	if (sram_bm_in & TPM_PNCL_SET_SPLIT_MOD) {
		*flowid_val |= (flowid_in->mod_cmd << TPM_MOD_FL_SHIFT);
		*flowid_updt_mask |= TPM_MOD_FL_SPLIT_MOD_UPDT_MASK;
		TPM_OS_DEBUG(TPM_PNCL_MOD, "flow_id after pkt_mod_addition(%x)\n", *flowid_val);
	}

	TPM_OS_DEBUG(TPM_PNCL_MOD, "flow_id log_mask(%x)\n", *flowid_updt_mask);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_res_info_map()
*
* DESCRIPTION:   The function maps info from the SRAM Logical format
*                into the Result Info bits in the SRAM pseudo-physical format
*
* INPUTS:
* sram_in        - Sram logical structure
*
* OUTPUTS:
* sram_out       - SRAM pseudo-physical structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_res_info_map(tpm_pncl_sram_data_t *sram_in, tpm_sram_all_t *sram_out)
{

	sram_out->res_info_23_16_data = 0;
	sram_out->res_info_23_16_mask = 0;
	sram_out->res_info_15_0_data = 0;
	sram_out->res_info_15_0_mask = 0;

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_DISC)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_DISC_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_IGMP)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_RX_SPECIAL_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_RX_SPECIAL)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_RX_SPECIAL_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_L3)
		tpm_pncl_set_l3(sram_in->l3_type, sram_out);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_L4)
		tpm_pncl_set_l4(sram_in->l4_type, sram_out);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_COL)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_COLOR_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_BC)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_BC_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_MC)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_MC_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_UC)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_UC_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_MH_RI) {
		TPM_OS_DEBUG(TPM_PNCL_MOD, "mapping <TPM_PNCL_SET_MH_RI>: mh_set(%d), mh_reg(0x%x)\r\n",
			     sram_in->mh_reg.mh_set, sram_in->mh_reg.mh_reg);
		if (sram_in->mh_reg.mh_set) {
			if (sram_in->mh_reg.mh_reg & TPM_MH_RI_BIT14)
				tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_MH_BIT_14, 1);

			if (sram_in->mh_reg.mh_reg & TPM_MH_RI_BIT15)
				tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_MH_BIT_15, 1);

			if (sram_in->mh_reg.mh_reg & TPM_MH_RI_BIT16)
				tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_MH_BIT_16, 1);

			if (sram_in->mh_reg.mh_reg & TPM_MH_RI_BIT17)
				tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_MH_BIT_17, 1);
		}
	}

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_PPPOE)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_PPPOE_BIT, 1);

	if (sram_in->sram_updt_bm & TPM_PNCL_SET_TAGGED)
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_VLAN_BIT, 1);

	TPM_OS_DEBUG(TPM_PNCL_MOD, "sram_out: res_info_23_16_data(0x%x) res_info_15_0_data(0x%x)\r\n"
		     "          res_info_23_16_mask(0x%x) res_info_15_0_mask(0x%x)\r\n",
		     sram_out->res_info_23_16_data, sram_out->res_info_15_0_data,
		     sram_out->res_info_23_16_mask, sram_out->res_info_15_0_mask);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_sram_map()
*
* DESCRIPTION:   The function maps the Logical SRAM format into a pseudo-physical SRAM format
*                To perform the mapping, certain TCAM fields are needed as well
*
* INPUTS:
* tcam_in        - Logical TCAM structure
* sram_in        - SRAM logical structure
*
* OUTPUTS:
* sram_out       - SRAM pseudo-physical structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_sram_map(tpm_pncl_sram_data_t *sram_in, tpm_sram_all_t *sram_out, tpm_pncl_tcam_data_t *tcam_in)
{
	uint32_t calc_shift;
	uint32_t ret_code;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "\n");

	/* Next LU */
	sram_out->next_lu_id = sram_in->next_lu_id;
	sram_out->next_lu_off_reg = sram_in->next_lu_off_reg;
	sram_out->lookup_done = (((sram_in->sram_updt_bm & TPM_PNCL_SET_LUD) > 0) ? 1 : 0);

	/* AI */
	sram_out->add_info_data = sram_in->add_info_data;
	sram_out->add_info_mask = sram_in->add_info_mask;

	/* Queue */
	sram_out->pnc_queue = sram_in->pnc_queue;

	/* Shift Reg */
	sram_out->shift_updt_reg = sram_in->shift_updt_reg;

	/* Calculate Shift Value, if a shift_register must be updated */
	if ((sram_out->shift_updt_reg != TPM_PNC_NOSHIFT_UPDATE_REG) && (!(sram_in->sram_updt_bm & TPM_PNCL_SET_DISC))) {
		if (tcam_in->start_offset.offset_base == TPM_PNCL_ZERO_OFFSET) {
			ret_code =
			tpm_pncl_shift_calc(&(tcam_in->start_offset), &(sram_in->next_offset), tcam_in->l2_parse_bm,
					    &calc_shift);
			IF_ERROR(ret_code);
		} else if (tcam_in->start_offset.offset_base == TPM_PNCL_L3_OFFSET) {
			ret_code =
			tpm_pncl_shift_calc(&(tcam_in->start_offset), &(sram_in->next_offset), tcam_in->l3_parse_bm,
					    &calc_shift);
			IF_ERROR(ret_code);
		} else if (tcam_in->start_offset.offset_base == TPM_PNCL_IPV4_OFFSET) {
			ret_code =
			tpm_pncl_shift_calc(&(tcam_in->start_offset), &(sram_in->next_offset),
					    tcam_in->ipv4_parse_bm, &calc_shift);
			IF_ERROR(ret_code);
		} else if ((tcam_in->start_offset.offset_base == TPM_PNCL_IPV6_OFFSET) ||
					(tcam_in->start_offset.offset_base == TPM_PNCL_IPV6_EXT_OFFSET)/*||
			(tcam_in->start_offset.offset_base == TPM_PNCL_IPV6_DIP_OFFSET) */) {
			ret_code =
			tpm_pncl_shift_calc(&(tcam_in->start_offset), &(sram_in->next_offset),
					    tcam_in->ipv6_parse_bm, &calc_shift);
			IF_ERROR(ret_code);
		} else if (tcam_in->start_offset.offset_base == TPM_PNCL_TCP_OFFSET) {
			ret_code =
			tpm_pncl_shift_calc(&(tcam_in->start_offset), &(sram_in->next_offset),
					    tcam_in->tcp_parse_bm, &calc_shift);
			IF_ERROR(ret_code);
		} else {
			TPM_OS_ERROR(TPM_PNCL_MOD, "invalid offset base %d\n", tcam_in->start_offset.offset_base);
			IF_ERROR(TPM_FAIL);
		}

		sram_out->shift_updt_val = calc_shift;
	} else
		sram_out->shift_updt_val = 0;

	/* FlowId */
	tpm_pncl_flowid_map(&(sram_in->flow_id_sub), sram_in->sram_updt_bm, &(sram_out->flowid_val),
			    &(sram_out->flowid_updt_mask));

	/* Result Info */
	tpm_pncl_res_info_map(sram_in, sram_out);

	TPM_OS_DEBUG(TPM_PNCL_MOD, "ai_data(%x) ai_mask(%x) ri_15_0_d(%x) ri_15_0_m(%x) ri_23_16_d(%x) ri_23_16_m(%x)\n"
		     "flowid_updt_mask(%x) flowid_val(%x) pnc_queue(%x)\n"
		     "next_lu_reg(%d) next_lu_id(%d) shift_updt_reg(%d), shift_updt_val(%d)\n",
		     sram_out->add_info_data,
		     sram_out->add_info_mask,
		     sram_out->res_info_15_0_data,
		     sram_out->res_info_15_0_mask,
		     sram_out->res_info_23_16_data,
		     sram_out->res_info_23_16_mask,
		     sram_out->flowid_updt_mask,
		     sram_out->flowid_val,
		     sram_out->pnc_queue,
		     sram_out->next_lu_off_reg,
		     sram_out->next_lu_id,
		     sram_out->shift_updt_reg,
		     sram_out->shift_updt_val);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_set_ri_bit()
*
* DESCRIPTION:   The function sets a single Result Info bit in the pseudo-phsical SRAM structure.
*
* INPUTS:
* ri_bit         - Result Info bit number
* ri_val         - Result Info bit value
*
* OUTPUTS:
* sram_out       - SRAM pseudo-physical structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_set_ri_bit(tpm_sram_all_t *sram_out, uint32_t ri_bit, uint32_t ri_val)
{
	/*TODO - check data */
	if (ri_bit < 16) {
		sram_out->res_info_15_0_data |= (ri_val << ri_bit);
		sram_out->res_info_15_0_mask |= (1 << ri_bit);
	} else {
		sram_out->res_info_23_16_data |= (ri_val << (ri_bit - 16));
		sram_out->res_info_23_16_mask |= (1 << (ri_bit - 16));
	}

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_set_l3()
*
* DESCRIPTION:   The function sets a single Result Info bit in the pseudo-phsical SRAM structure.
*
* INPUTS:
* l3_type         - Logical L3_type
*
* OUTPUTS:
* sram_out       - SRAM pseudo-physical structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_set_l3(tpm_pncl_l3_type_t l3_type, tpm_sram_all_t *sram_out)
{
	switch (l3_type) {
	case TPM_PNCL_L3_OTHER:
		break;
	case TPM_PNCL_L3_IPV6:
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_L3_OFF_BIT, 1);
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l3_type(%d)\n", l3_type);
		break;
	case TPM_PNCL_L3_IPV4_NFRAG:
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l3_type(%d)\n", l3_type);
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_L3_OFF_BIT, 1);
		tpm_pncl_set_ri_bit(sram_out, (TPM_PNC_RI_L3_OFF_BIT + 1), 1);
		break;
	case TPM_PNCL_L3_IPV4_FRAG:
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l3_type(%d)\n", l3_type);
		tpm_pncl_set_ri_bit(sram_out, (TPM_PNC_RI_L3_OFF_BIT + 1), 1);
		break;
	case TPM_PNCL_L3_IPV4_FFRAG:
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l3_type(%d)\n", l3_type);
		tpm_pncl_set_ri_bit(sram_out, (TPM_PNC_RI_L3_OFF_BIT + 1), 1);
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_L3_FF_BIT, 1);
		break;
	default:
		return(TPM_FAIL);
	}
	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_set_l4()
*
* DESCRIPTION:   The function sets a single Result Info bit in the pseudo-phsical SRAM structure.
*
* INPUTS:
* l4_type         - Logical L4_type
*
* OUTPUTS:
* sram_out       - SRAM pseudo-physical structure
*
* RETURNS:
* On success, the function returns TPM_OK. On error different types are returned
* according to the case - see tpm_db_err_t.
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_pncl_set_l4(tpm_pncl_l4_type_t l4_type, tpm_sram_all_t *sram_out)
{
	switch (l4_type) {
	case TPM_PNCL_L4_OTHER:
		tpm_pncl_set_ri_bit(sram_out, (TPM_PNC_RI_L4_OFF_BIT + 1), 1);
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l4_type(%d)\n", l4_type);
		break;
	case TPM_PNCL_L4_TCP:
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l4_type(%d)\n", l4_type);
		break;
	case TPM_PNCL_L4_UDP:
		tpm_pncl_set_ri_bit(sram_out, TPM_PNC_RI_L4_OFF_BIT, 1);
		TPM_OS_DEBUG(TPM_PNCL_MOD, "l4_type(%d)\n", l4_type);
		break;
	default:
		TPM_OS_ERROR(TPM_PNCL_MOD, "l4_type(%d)\n", l4_type);
		return(TPM_FAIL);
	}
	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_update_sram()
*
* DESCRIPTION:   The function sets a PNC Entry.
*                It maps TCAM/SRAM logical to physical structures and updates HW and PNC Shadow Tables.
* INPUTS:
* pnc_entry      - Pnc Entry number (0-511)
* pnc_data       - PNC pseudo-physical data (TCAM+SRAM)
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
int32_t tpm_pncl_update_sram(uint32_t pnc_entry, tpm_pncl_pnc_full_t *pnc_data)
{
	tpm_pnc_all_t pnc_out;
	int32_t ret_code;

	memset(&pnc_out, 0, sizeof(tpm_pnc_all_t));

	TPM_OS_DEBUG(TPM_PNCL_MOD, "pnc_entry(%d)\n", pnc_entry);
	TPM_OS_COND_DEBUG(TPM_PNCL_MOD) {
		tpm_pncl_print_pncl(pnc_data);
	}

	/* TODO : check pnc_entry value */
	/* TODO : check pointer */

	/* TCAM Translate */
	ret_code = tpm_pncl_tcam_map(&(pnc_data->pncl_tcam), &(pnc_out.tcam_entry));
	IF_ERROR(ret_code);

	/* SRAM  Translate */
	ret_code = tpm_pncl_sram_map(&(pnc_data->pncl_sram), &(pnc_out.sram_entry), &(pnc_data->pncl_tcam));
	IF_ERROR(ret_code);

	/* TODO: validate */

	/* Call HW */
	tpm_pnc_set(pnc_entry, 1 /*sram_update */ , &pnc_out);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(pnc_entry, &pnc_out);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_entry_set()
*
* DESCRIPTION:   The function sets a PNC Entry.
*                It maps TCAM/SRAM logical to physical structures and updates HW and PNC Shadow Tables.
* INPUTS:
* pnc_entry      - Pnc Entry number (0-511)
* pnc_data       - PNC pseudo-physical data (TCAM+SRAM)
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
int32_t tpm_pncl_entry_set(uint32_t pnc_entry, tpm_pncl_pnc_full_t *pnc_data)
{
	tpm_pnc_all_t pnc_out;
	int32_t ret_code;
	memset(&pnc_out, 0, sizeof(tpm_pnc_all_t));

	TPM_OS_DEBUG(TPM_PNCL_MOD, "pnc_entry(%d)\n", pnc_entry);
	TPM_OS_COND_DEBUG(TPM_PNCL_MOD) {
		tpm_pncl_print_pncl(pnc_data);
	}

	/* TODO : check pnc_entry value */
	/* TODO : check pointer */

	/* TCAM Translate */
	ret_code = tpm_pncl_tcam_map(&(pnc_data->pncl_tcam), &(pnc_out.tcam_entry));
	IF_ERROR(ret_code);

	/* SRAM  Translate */
	ret_code = tpm_pncl_sram_map(&(pnc_data->pncl_sram), &(pnc_out.sram_entry), &(pnc_data->pncl_tcam));
	IF_ERROR(ret_code);

	/* TODO: validate */

	/* Call HW */
	tpm_pnc_set(pnc_entry, 0 /*sram_update */ , &pnc_out);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(pnc_entry, &pnc_out);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_entry_insert()
*
* DESCRIPTION:   The function inserts a PNC Entry.
*                It inserts an empty entry into a range and then sets the entry.
* INPUTS:
* pnc_ins_entry  - Pnc Entry first entry to "move down", also the entry to set
* pnc_stop_entry - Pnc last entry to "move"down"
* pnc_data       - PNC pseudo-physical data (TCAM+SRAM)
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
int32_t tpm_pncl_entry_insert(uint32_t pnc_ins_entry, uint32_t pnc_stop_entry, tpm_pncl_pnc_full_t *pnc_data)
{

	int32_t ret_code;

	/* TODO : check pnc_entry value */
	/* TODO : check pointer */

	TPM_OS_DEBUG(TPM_PNCL_MOD, "pnc_ins_entry(%d) pnc_stop_entry(%d)\n", pnc_ins_entry, pnc_stop_entry);

	/* Insert empty entry in HW */
	tpm_pnc_entry_insert(pnc_ins_entry, pnc_stop_entry);

	/* Update PNC Shadow  */
	ret_code = tpm_db_pnc_shdw_ent_ins(pnc_ins_entry, pnc_stop_entry);
	IF_ERROR(ret_code);

	/* Set new PNC entry */
	ret_code = tpm_pncl_entry_set(pnc_ins_entry, pnc_data);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_entry_reset()
*
* DESCRIPTION:   The function resets a PNC Entry.
*                It updates HW and PNC Shadow Tables.
* INPUTS:
* pnc_entry      - Pnc Entry number (0-511)
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
int32_t tpm_pncl_entry_reset(uint32_t pnc_entry)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "pnc_entry(%d)\n", pnc_entry);

	/* TODO : check pnc_entry value */

	/* Call HW */
	ret_code = tpm_pnc_entry_inv(pnc_entry);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_inv(pnc_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_entry_delete()
*
* DESCRIPTION:   The function deletes a PNC Entry. It "pushes up" a range of PNC entries
*                following the entry to be deleted. It updates HW and PNC Shadow Tables.
* INPUTS:
* pnc_del_entry  - Pnc Entry to delete, the following entry is the frist entry to "push Up"
* pnc_stop_entry - Pnc last entry to "push Up"
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
int32_t tpm_pncl_entry_delete(uint32_t pnc_del_entry, uint32_t pnc_stop_entry)
{
	int32_t ret_code;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "pnc_del_entry(%d) pnc_stop_entry(%d)\n", pnc_del_entry, pnc_stop_entry);

	if ((pnc_del_entry > pnc_stop_entry) || (pnc_stop_entry > TPM_PNC_SIZE))
		TPM_OS_ERROR(TPM_PNCL_MOD, "pnc_del_entry(%d) pnc_stop_entry(%d)\n", pnc_del_entry, pnc_stop_entry);

	/* Call HW */
	ret_code = tpm_pnc_entry_delete(pnc_del_entry, pnc_stop_entry);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_del(pnc_del_entry, pnc_stop_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_pkt_trap()
*
* DESCRIPTION:   The function sets an entry to "CPU trap"
*                The function purpose is to catch  packets that for unknown reason have not been caught
*                by previous rules in a lookup_id
*
* INPUTS:
* entry          - Pnc Entry number
* lu_id          - lu_id to set in TCAM. The function only traps packets with  this lu_id
* gmac_bm        - gmac bitmap to set in TCAM port_id field. The function only traps packets from GMACs in the bitmap.
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
int32_t tpm_pncl_pkt_trap(uint32_t entry, uint32_t lu_id, tpm_gmac_bm_t gmac_bm)
{
	uint32_t ret_code;

	tpm_pnc_all_t pnc_entry;

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	TPM_OS_DEBUG(TPM_PNCL_MOD, " pnc_entry(%d) lu_id(%d) gmac_bm(%x)\n", entry, lu_id, gmac_bm);

	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = 1;

	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = lu_id;

	/* Set port_ids */
	pnc_entry.tcam_entry.port_ids = gmac_bm;

	/* Set high FlowId to Pnc Entry, as indication to CPU, which PnC entry performed a trap */
	pnc_entry.sram_entry.flowid_val = (entry << TPM_PKT_TRAP_FL_SHIFT);
	pnc_entry.sram_entry.flowid_updt_mask = TPM_PKT_TRAP_UPDT_MASK;

	/* Set Target to CPU */
	pnc_entry.sram_entry.flowid_val |= (TPM_PNC_TRG_CPU << TPM_TXP_FL_SHIFT);
	pnc_entry.sram_entry.flowid_updt_mask |= TPM_TXP_FL_UPDT_MASK;

	/* Set Result Info Ind. to CPU */
	tpm_pncl_set_ri_bit(&(pnc_entry.sram_entry), TPM_PNC_RI_TRAP_BIT, 1);

	/* If debug, then print entry */
	TPM_OS_COND_DEBUG(TPM_PNCL_MOD) {
		tpm_pnc_print_sw_entry(&pnc_entry);
	}
	/* Write to PNC */
	tpm_pnc_set(entry, 0, &pnc_entry);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry, &pnc_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_pkt_drop()
*
* DESCRIPTION:   The function sets an entry to drop. It sets only lu_id, port_id in TCAM, empty
*                and leaves the packet part (24 bytes). The function purpose is to drop packets t
*                hat for unknown reason have not been caught by previous rules in a lookup_id
*
* INPUTS:
* entry          - Pnc Entry number
* lu_id          - lookup_id to set in TCAM. The function only drops packets with this lu_id
* gmac_bm        - gmac bitmap to set in TCAM port_id field. The function only drops packets from GMACs in the bitmap.
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
int32_t tpm_pncl_pkt_drop(uint32_t entry, uint32_t lu_id, tpm_gmac_bm_t gmac_bm)
{
	uint32_t ret_code;
	tpm_pnc_all_t pnc_entry;

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	TPM_OS_DEBUG(TPM_PNCL_MOD, " pnc_entry(%d) lu_id(%d) gmac_bm(%x)\n", entry, lu_id, gmac_bm);

	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = lu_id;

	/* Set port_ids */
	pnc_entry.tcam_entry.port_ids = gmac_bm;

	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = 1;

	/* Set Discard */
	tpm_pncl_set_ri_bit(&(pnc_entry.sram_entry), TPM_PNC_RI_DISC_BIT, 1);

	/* Write to PNC */
	tpm_pnc_set(entry, 0, &pnc_entry);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry, &pnc_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

int32_t tpm_pncl_init_ipv6_last_entry(uint32_t entry, uint32_t lu_id, uint32_t next_lu_id)
{
	uint32_t ret_code;
	tpm_pnc_all_t pnc_entry;
	tpm_gmac_bm_t l_gmac_bm;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "entry(%d) lu_id(%d), next_lu_id(%d)\n", entry, lu_id, next_lu_id);

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = lu_id;

	/* Set port_ids, both dir */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN, &l_gmac_bm);
	pnc_entry.tcam_entry.port_ids |= l_gmac_bm;
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_UNI_ANY, &l_gmac_bm);
	pnc_entry.tcam_entry.port_ids |= l_gmac_bm;

	/* Set LU Done */
	pnc_entry.sram_entry.next_lu_id = next_lu_id;

	/* Write to PNC */
	tpm_pnc_set(entry, 0, &pnc_entry);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry, &pnc_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

int32_t tpm_pncl_ipv6_shift2dip(tpm_sram_all_t *sram_entry)
{
	/*TODO  check pointer */

	/* in ipv6, use the 3rd shift offset table for now, need further discussion.
	 * it is like in excel column "shift update" says "[2] = 24",
	 * "Nxt LU offset Ind" says " 2 ".
	 */
	sram_entry->next_lu_off_reg = 2;
	sram_entry->shift_updt_reg = 2;
	sram_entry->shift_updt_val = 24;

	return TPM_OK;
}

int32_t tpm_pncl_init_ipv4_mc_range(void)
{
	int32_t ret_code = TPM_OK;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_pnc_all_t pnc_entry;
	tpm_gmac_bm_t l_gmac_bm;
	tpm_mc_filter_mode_t filter_mode;
	uint32_t cpu_rx_queue;

	/* Get DB Info, check PnC config */
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV4_MC_DS, &range_conf);
	IF_ERROR(ret_code);

	filter_mode = tpm_db_get_mc_filter_mode();
	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_conf.base_lu_id;

	/* Set port_ids, both dir */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN_OR_LAN, &l_gmac_bm);
	pnc_entry.tcam_entry.port_ids |= l_gmac_bm;

	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = 1;

	pnc_entry.sram_entry.flowid_updt_mask = TPM_TXP_FL_UPDT_MASK;
	if (TPM_MC_ALL_CPU_FRWD == filter_mode)
		pnc_entry.sram_entry.flowid_val = (TPM_PNC_TRG_CPU << TPM_TXP_FL_SHIFT);
	else if (TPM_MC_MAC_ONLY_FILTER == filter_mode) {
		pnc_entry.sram_entry.flowid_val = (TPM_PNC_TRG_GMAC0 << TPM_TXP_FL_SHIFT);
		pnc_entry.sram_entry.pnc_queue = 1;

		tpm_pncl_set_ri_bit(&pnc_entry.sram_entry, TPM_PNC_RI_MH_BIT_14, 1);
		tpm_pncl_set_ri_bit(&pnc_entry.sram_entry, TPM_PNC_RI_MH_BIT_15, 1);
		tpm_pncl_set_ri_bit(&pnc_entry.sram_entry, TPM_PNC_RI_MH_BIT_16, 1);
		tpm_pncl_set_ri_bit(&pnc_entry.sram_entry, TPM_PNC_RI_MH_BIT_17, 1);
	} else
		/* Set Discard */
		tpm_pncl_set_ri_bit(&(pnc_entry.sram_entry), TPM_PNC_RI_DISC_BIT, 1);

	/* the first byte of DIP is 224 to 239, high 3 bits are 1s */
	pnc_entry.tcam_entry.pkt_data.pkt_byte[16] = 0xE0;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[16] = 0xE0;

	/* Write to PNC */
	tpm_pnc_set(range_conf.range_end, 0, &pnc_entry);

	/* fix bug for IPv4MC - default last entry in range is DISCARDING all MC
	   - but num of free entries / api_end - is not decremented */
	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_IPV4_MC_DS);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(range_conf.range_end, &pnc_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

/*******************************************************************************
* tpm_pncl_init_ipv4_ranges()
*
* DESCRIPTION:  Functions resets a IPv4 PnC range, in HW and in DB.
*
*
* INPUTS:
* range_id       - range_id number
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
int32_t tpm_pncl_init_ipv4_ranges(tpm_pnc_ranges_t range_id)
{
	int32_t ret_code = TPM_OK;

	if (range_id == TPM_PNC_IPV4_MC_DS)
		ret_code = tpm_pncl_init_ipv4_mc_range();

	/* No other cases yet */
	return ret_code;
}

/*******************************************************************************
* tpm_pncl_init_ipv6_mc_range()
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
tpm_error_code_t tpm_pncl_init_ipv6_mc_range(void)
{
	int32_t ret_code = TPM_OK;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_pnc_all_t pnc_entry;
	tpm_gmac_bm_t l_gmac_bm;

	/*
		In TPM_PNC_IPV6_MC, add a default PNC entry at the end
		to discard all the DS un-match IPv6 MC packets
	*/

	/* Get DB Info, check PnC config */
	ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_IPV6_MC_DS, &range_conf);
	IF_ERROR(ret_code);

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_conf.base_lu_id;

	/* Set port_ids, both dir */
	tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_WAN_OR_LAN, &l_gmac_bm);
	pnc_entry.tcam_entry.port_ids |= l_gmac_bm;

	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = 1;

	pnc_entry.sram_entry.flowid_updt_mask = TPM_TXP_FL_UPDT_MASK;

	tpm_pncl_set_ri_bit(&(pnc_entry.sram_entry), TPM_PNC_RI_DISC_BIT, 1);

	/* the first byte of DIP is 0xff */
	pnc_entry.tcam_entry.pkt_data.pkt_byte[0] = 0xff;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[0] = 0xff;

	/* Write to PNC */
	tpm_pnc_set(range_conf.range_end, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_IPV6_MC_DS);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(range_conf.range_end, &pnc_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

/*******************************************************************************
* tpm_pncl_init_ipv6_ranges()
*
* DESCRIPTION:  Functions resets a IPv4 PnC range, in HW and in DB.
*
*
* INPUTS:
* range_id       - range_id number
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
int32_t tpm_pncl_init_ipv6_ranges(tpm_pnc_ranges_t range_id)
{
	int32_t ret_code = TPM_OK;

	if (range_id == TPM_PNC_IPV6_MC_DS)
		ret_code = tpm_pncl_init_ipv6_mc_range();

	/* No other cases yet */
	return ret_code;
}


int32_t tpm_pncl_init_cnm_ipv4_pre_range(void)
{
	int32_t ret_code = TPM_OK;
	tpm_db_pnc_range_conf_t range_conf;
	tpm_pnc_all_t pnc_entry;
	uint32_t entry_id;
	tpm_gmac_bm_t gmac_bm;

	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;

	/* Set Structs to zero */
	memset(&range_conf, 0, sizeof(tpm_db_pnc_range_conf_t));
	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	/* default rule:
	 *	create default PNC rules in CnM_IPv4_Pre, go to CnM_MAIN
	 */
	if (ctc_cm_enable != TPM_CTC_CM_DISABLED) {
		/* Get DB Info, check PnC config */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_IPV4_PRE, &range_conf);
		IF_ERROR(ret_code);

		entry_id = range_conf.range_end;

		/* Set Lookup Id */
		pnc_entry.tcam_entry.lu_id = range_conf.base_lu_id;

		/* Set port_ids, both dir */
		ret_code = tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_UNI_ANY, &gmac_bm);
		IF_ERROR(ret_code);
		pnc_entry.tcam_entry.port_ids = gmac_bm;

		/* Get DB Info, check PnC config */
		ret_code = tpm_db_pnc_rng_conf_get(TPM_PNC_CNM_MAIN, &range_conf);
		IF_ERROR(ret_code);

		/* Set Next LUID  */
		pnc_entry.sram_entry.next_lu_id = range_conf.base_lu_id + TPM_CNM_L2_MAIN_LUID_OFFSET;
		pnc_entry.sram_entry.next_lu_off_reg = TPM_PNC_CNM_L2_REG;

		pnc_entry.sram_entry.add_info_data &= ~TPM_AI_CNM_IPV4_PRE_KEY_MASK;
		pnc_entry.sram_entry.add_info_mask = TPM_AI_CNM_IPV4_PRE_KEY_MASK;

		pnc_entry.sram_entry.pnc_queue = 0xFFFF;
		pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

		/* Write to PNC */
		tpm_pnc_set(entry_id, 0, &pnc_entry);

		ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_IPV4_PRE);
		IF_ERROR(ret_code);

		/* Write to Shadow */
		ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
		IF_ERROR(ret_code);
	}

	return(TPM_OK);
}
int32_t tpm_pncl_init_cnm_main_ety_ipv6(uint32_t entry_id, tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win)
{
	int32_t ret_code = TPM_OK;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_all_t pnc_entry;
	tpm_gmac_bm_t gmac_bm;

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	/* Set Structs to zero */
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	/* Get DB Info, check PnC config */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_CNM_MAIN, &range_data);
	IF_ERROR(ret_code);

	/* Set port_ids, US dir */
	ret_code = tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_UNI_ANY, &gmac_bm);
	IF_ERROR(ret_code);
	pnc_entry.tcam_entry.port_ids = gmac_bm;

	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_ETY_MAIN_LUID_OFFSET;

	/* Set Next LUID  */
	pnc_entry.sram_entry.next_lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_IPV6_MAIN_LUID_OFFSET;

	pnc_entry.sram_entry.pnc_queue = 0xFFFF;

	/* depends on ipv6 CnM parse window */
	if(TPM_CTC_CM_IPv6_FIRST_24B == ctc_cm_ipv6_parse_win) {
		pnc_entry.sram_entry.next_lu_off_reg = TPM_PNC_LU_REG0;
		pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;
	} else {
		pnc_entry.sram_entry.next_lu_off_reg = TPM_PNC_IPV6_DIP_REG;
		pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_IPV6_DIP_REG;
		pnc_entry.sram_entry.shift_updt_val = 26;
	}

	/* ETY = 0x86dd */
	pnc_entry.tcam_entry.pkt_data.pkt_byte[0] = 0x86;
	pnc_entry.tcam_entry.pkt_data.pkt_byte[1] = 0xDD;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[0] = 0xFF;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[1] = 0xFF;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	return TPM_OK;
}

int32_t tpm_pncl_init_cnm_main_range(void)
{
	int32_t ret_code = TPM_OK;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_all_t pnc_entry;
	uint32_t free_entry = 0, entry_id;
	tpm_db_ctc_cm_ipv6_parse_win_t ctc_cm_ipv6_parse_win;
	tpm_gmac_bm_t gmac_bm;

	tpm_db_ctc_cm_enable_t ctc_cm_enable = TPM_CTC_CM_DISABLED;

	/* Set Structs to zero */
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));

	tpm_db_ctc_cm_enable_get(&ctc_cm_enable);

	if (ctc_cm_enable == TPM_CTC_CM_DISABLED)
		return TPM_OK;

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	/* Get DB Info, check PnC config */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_CNM_MAIN, &range_data);
	IF_ERROR(ret_code);

	free_entry = range_data.pnc_range_oper.free_entries;
	entry_id = range_data.pnc_range_conf.range_end;

	/* default rule #1:
	 *	create three default PNC rules in CnM_MAIN, CnM_IPv4 DONE
	 */

	if (free_entry == 0) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "No free entries\n");
		return(TPM_FAIL);
	}

	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_IPV4_MAIN_LUID_OFFSET;
	/* Set port_ids, US dir */
	ret_code = tpm_proc_src_port_gmac_bm_map(TPM_SRC_PORT_UNI_ANY, &gmac_bm);
	IF_ERROR(ret_code);
	pnc_entry.tcam_entry.port_ids = gmac_bm;

	/* Set Next LUID  */
	pnc_entry.sram_entry.lookup_done = TPM_TRUE;

	pnc_entry.sram_entry.pnc_queue = 0xFFFF;
	pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_MAIN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	free_entry--;
	entry_id--;

	/* default rule #2:
	 *	create three default PNC rules in CnM_MAIN, CnM_IPv6 DONE
	 */
	if (free_entry == 0) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "No free entries\n");
		return(TPM_FAIL);
	}
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_IPV6_MAIN_LUID_OFFSET;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_MAIN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	free_entry--;
	entry_id--;

	/* default rule #3:
	 *	create default PNC rules in CnM_MAIN, CnM_ETY L2 done
	 */
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_ETY_MAIN_LUID_OFFSET;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_MAIN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	free_entry--;
	entry_id--;

	/* default rule #4:
	 *	create default PNC rules in CnM_MAIN, CnM_ETY IPv4 goto CnM_IPv4
	 */

	pnc_entry.sram_entry.lookup_done = TPM_FALSE;

	/* Set Next LUID  */
	pnc_entry.sram_entry.next_lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_IPV4_MAIN_LUID_OFFSET;
	pnc_entry.sram_entry.next_lu_off_reg = TPM_PNC_LU_REG0;

	pnc_entry.sram_entry.pnc_queue = 0xFFFF;
	pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* ETY = 0x0800 */
	pnc_entry.tcam_entry.pkt_data.pkt_byte[0] = 0x08;
	pnc_entry.tcam_entry.pkt_data.pkt_byte[1] = 0x00;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[0] = 0xFF;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[1] = 0xFF;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_MAIN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	free_entry--;
	entry_id--;
	/* default rule #5:
	 *	create default PNC rules in CnM_MAIN, CnM_ETY IPv6 goto CnM_IPv6
	 */
	tpm_db_ctc_cm_ipv6_parse_win_get(&ctc_cm_ipv6_parse_win);
	ret_code = tpm_pncl_init_cnm_main_ety_ipv6(entry_id, ctc_cm_ipv6_parse_win);
	IF_ERROR(ret_code);

	/* record ipv6_ety_rule_num */
	tpm_db_ctc_cm_ipv6_ety_rule_num_set(entry_id);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_MAIN);
	IF_ERROR(ret_code);

	free_entry--;
	entry_id--;

	/* default rule #6:
	 *	create default PNC rules in CnM_MAIN, CnM_l2 go to CnM_ETY
	 */

	if (free_entry == 0) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "No free entries\n");
		return(TPM_FAIL);
	}

	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_L2_MAIN_LUID_OFFSET;

	/* Set port_ids, US dir */
	pnc_entry.tcam_entry.port_ids = gmac_bm;

	/* Set Next LUID  */
	pnc_entry.sram_entry.next_lu_id = range_data.pnc_range_conf.base_lu_id + TPM_CNM_ETY_MAIN_LUID_OFFSET;
	pnc_entry.sram_entry.next_lu_off_reg = TPM_PNC_ETY_REG;

	pnc_entry.sram_entry.add_info_data = TPM_AI_CNM_PREC_MASK;
	pnc_entry.sram_entry.add_info_mask = TPM_AI_CNM_PREC_MASK;

	pnc_entry.sram_entry.pnc_queue = 0xFFFF;
	pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_CNM_MAIN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

int32_t tpm_pncl_init_cnm_ranges(tpm_pnc_ranges_t range_id)
{
	int32_t ret_code = TPM_OK;

	if (range_id == TPM_PNC_CNM_IPV4_PRE)
		ret_code = tpm_pncl_init_cnm_ipv4_pre_range();
	else if (range_id == TPM_PNC_CNM_MAIN)
		ret_code = tpm_pncl_init_cnm_main_range();

	/* No other cases yet */
	return ret_code;
}

/*******************************************************************************
* tpm_pncl_init_mac_learn_range()
*
* DESCRIPTION:  Functions init MAC learn range default rule
*
*
* INPUTS:
* range_id       - range_id number
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
int32_t tpm_pncl_init_mac_learn_range(void)
{
	int32_t ret_code = TPM_OK;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_all_t pnc_entry;
	uint32_t free_entry = 0, entry_id;
	uint32_t queue_id;
	tpm_gmacs_enum_t lpk_gmac;
	int32_t cpu_rx_queue;

	/* Set Structs to zero */
	memset(&range_data, 0, sizeof(tpm_db_pnc_range_t));
	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));

	/* check gmac1 loopback en */
	if (!tpm_db_gmac1_lpbk_en_get())
		return(TPM_OK);

	/* Get DB Info, check PnC config */
	ret_code = tpm_db_pnc_rng_get(TPM_PNC_MAC_LEARN, &range_data);
	IF_ERROR(ret_code);

	free_entry = range_data.pnc_range_oper.free_entries;
	/* check the free size in the range for default rule*/
	if (free_entry < 2) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "No enough entries(%d) for MAC learn default rules\n",
			     free_entry);
		return(TPM_FAIL);
	}

	/* construct default rule on GMAC0 forward packet to GMAC1*/
	entry_id = range_data.pnc_range_conf.range_end;
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id;
	/* Set port_id */
	pnc_entry.tcam_entry.port_ids = TPM_BM_GMAC_0;
	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = TPM_TRUE;

	pnc_entry.sram_entry.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	pnc_entry.sram_entry.flowid_updt_mask = TPM_TXP_FL_UPDT_MASK;

	/*forward packet to GMAC1 as default*/
	if (TPM_DB_OK != tpm_db_gmac_lpk_queue_get(&lpk_gmac,
						   &queue_id,
						   TPM_GMAC1_QUEUE_DATA_TRAFFIC)) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "loopback gmac queue get failed \n");
		return TPM_FAIL;
	}
	/* Set target GMAC1, queue id changed to cpu_rate_limit queue*/
	pnc_entry.sram_entry.flowid_val = (TPM_PNC_TRG_GMAC1 << TPM_TXP_FL_SHIFT);
	pnc_entry.sram_entry.pnc_queue = queue_id;

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_MAC_LEARN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	/*CPU rate limit*/
	memset(&pnc_entry, 0, sizeof(tpm_pnc_all_t));
	entry_id = range_data.pnc_range_conf.range_end - 1;
	/* Set Lookup Id */
	pnc_entry.tcam_entry.lu_id = range_data.pnc_range_conf.base_lu_id;
	/* Set port_id */
	pnc_entry.tcam_entry.port_ids = TPM_BM_GMAC_1;
	/* check MH, 0xAACC indicates MAC learning packets */
	pnc_entry.tcam_entry.pkt_data.pkt_byte[0] = TPM_MOD2_MAC_LEARN_MH >> 8;
	pnc_entry.tcam_entry.pkt_data.pkt_byte[1] = TPM_MOD2_MAC_LEARN_MH & 0xFF;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[0] = 0xFF;
	pnc_entry.tcam_entry.pkt_mask.pkt_byte[1] = 0xFF;
	/* Set LU Done */
	pnc_entry.sram_entry.lookup_done = TPM_TRUE;

	pnc_entry.sram_entry.pnc_queue = TPM_PNCL_NO_QUEUE_UPDATE;
	pnc_entry.sram_entry.shift_updt_reg = TPM_PNC_NOSHIFT_UPDATE_REG;

	pnc_entry.sram_entry.flowid_updt_mask = TPM_TXP_FL_UPDT_MASK;

	/* Get default CPU queue */
	tpm_db_get_cpu_rx_queue(&cpu_rx_queue);
	/* Trap to CPU */
	pnc_entry.sram_entry.flowid_val = (TPM_PNC_TRG_CPU << TPM_TXP_FL_SHIFT);
	pnc_entry.sram_entry.pnc_queue = cpu_rx_queue;
	/* Set result info */
	pnc_entry.sram_entry.res_info_15_0_data |= (1 << TPM_PNC_RI_MAC_LEARN_BIT);
	pnc_entry.sram_entry.res_info_15_0_mask |= (1 << TPM_PNC_RI_MAC_LEARN_BIT);

	/* Write to PNC */
	tpm_pnc_set(entry_id, 0, &pnc_entry);

	ret_code = tpm_db_pnc_rng_api_end_dec(TPM_PNC_MAC_LEARN);
	IF_ERROR(ret_code);

	/* Write to Shadow */
	ret_code = tpm_db_pnc_shdw_ent_set(entry_id, &pnc_entry);
	IF_ERROR(ret_code);

	return(TPM_OK);
}

/*******************************************************************************
* tpm_pncl_init_mac_learn_ranges()
*
* DESCRIPTION:  Functions init MAC learn range
*
*
* INPUTS:
* range_id       - range_id number
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
int32_t tpm_pncl_init_mac_learn_ranges(tpm_pnc_ranges_t range_id)
{
	int32_t ret_code = TPM_OK;

	if (range_id == TPM_PNC_MAC_LEARN)
		ret_code = tpm_pncl_init_mac_learn_range();

	return ret_code;
}

/*******************************************************************************
* tpm_pncl_range_reset()
*
* DESCRIPTION:  Functions resets a PnC range, in HW and in DB.
*
*
* INPUTS:
* range_id       - range_id number
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
int32_t tpm_pncl_range_reset(tpm_pnc_ranges_t range_id)
{
	int32_t ret_code;
	tpm_db_pnc_range_conf_t range_conf;
	uint32_t switch_init;

	TPM_OS_DEBUG(TPM_PNCL_MOD, "range_id(%d)\n", range_id);

	/* Get DB Info, check PnC config */
	ret_code = tpm_db_pnc_rng_conf_get(range_id, &range_conf);
	IF_ERROR(ret_code);

	/* Update HW */
	if (range_conf.range_size > 1)	/* Invalidate all entries, except last */
		tpm_pnc_range_inv(range_conf.range_start, (range_conf.range_end - 1));

	/* get switch_init */
	ret_code = tpm_db_switch_init_get(&switch_init);
	IF_ERROR(ret_code);

	/* Handle Range's last Pnc Entry */
	if (range_id == TPM_PNC_IPV4_MC_DS) {
		ret_code = tpm_pncl_init_ipv4_ranges(range_id);
		IF_ERROR(ret_code);
	} else if (range_id == TPM_PNC_IPV6_MC_DS) {
		ret_code = tpm_pncl_init_ipv6_ranges(range_id);
		IF_ERROR(ret_code);
	} else if (range_id == TPM_PNC_CNM_IPV4_PRE ||
			   range_id == TPM_PNC_CNM_MAIN) {
		ret_code = tpm_pncl_init_cnm_ranges(range_id);
		IF_ERROR(ret_code);
	} else if (range_id == TPM_PNC_MAC_LEARN) {
		ret_code = tpm_pncl_init_mac_learn_ranges(range_id);
		IF_ERROR(ret_code);
	} else {
		if (range_conf.init_last_entry == TPM_PNC_RNG_LAST_INIT_TRAP) {
			ret_code = tpm_pncl_pkt_trap(range_conf.range_end,
						     range_conf.base_lu_id, range_conf.last_ent_portid);
			IF_ERROR(ret_code);
		} else if (range_conf.init_last_entry == TPM_PNC_RNG_LAST_INIT_DROP) {
			ret_code = tpm_pncl_pkt_drop(range_conf.range_end,
						     range_conf.base_lu_id, range_conf.last_ent_portid);
			IF_ERROR(ret_code);
		} else
			tpm_pnc_entry_inv(range_conf.range_end);
	}
	/* Update DB */
	ret_code = tpm_db_pnc_rng_reset(range_id);
	IF_ERROR(ret_code);

	return(TPM_OK);
}
