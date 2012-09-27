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
* tpm_print.c
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

#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_sysfs_utils.h"

/*TODO - currently some printing funtions are directly accessing DB */
extern tpm_db_t tpm_db;

#define DWORD_LEN       32

/* start offset of api_dump_ipv4_parse_bm, should be bigger than l2_parse_bm
   right now the biggest l2_parse_bm is TPM_L2_PARSE_GEMPORT with offset 8
 */
#define API_DUMP_IPV4_PARSE_BM_START       12

/* offset of api_dump_ipv6_parse_bm, should be bigger than ipv4_parse_bm + API_DUMP_IPV4_PARSE_BM_START
   right now the biggest ipv4_parse_bm is TPM_IPv4_PARSE_PROTO with offset 4
 */
#define API_DUMP_IPV6_PARSE_BM_START       24

#define TPM_DBVAL_CON(dbval)	((dbval == TPM_DB_VALID) ? TPM_TRUE : TPM_FALSE)
#define DIR2STR(dir)		((dir == TPM_DIR_DS) ? "DS" : "US")
#define IF_ERROR(ret)		\
		if (ret != TPM_DB_OK) {\
			TPM_OS_ERROR(TPM_CLI_MOD, "%s(%d):  recev'd error code (%d)\n", __func__, __LINE__, ret);\
			return;\
		}

db_enum_string_t tpm_db_chip_conn_str[] = {
	BuildEnumString(TPM_CONN_DISC),
	BuildEnumString(TPM_CONN_QSGMII),
	BuildEnumString(TPM_CONN_FE_PHY),
	BuildEnumString(TPM_CONN_GE_PHY),
	BuildEnumString(TPM_CONN_RGMII1),
	BuildEnumString(TPM_CONN_RGMII2),
};

db_enum_string_t tpm_db_int_conn_str[] = {
	BuildEnumString(TPM_INTCON_GMAC0),
	BuildEnumString(TPM_INTCON_GMAC1),
	BuildEnumString(TPM_INTCON_SWITCH),
};

db_enum_string_t tpm_db_sched_str[] = {
	BuildEnumString(TPM_SCHED_SP),
	BuildEnumString(TPM_SCHED_WRR),
};

db_enum_string_t tpm_db_txq_owner_str[] = {
	BuildEnumString(TPM_Q_OWNER_CPU),
	BuildEnumString(TPM_Q_OWNER_GMAC0),
	BuildEnumString(TPM_Q_OWNER_GMAC1),
	BuildEnumString(TPM_Q_OWNER_PMAC),
};

db_enum_string_t tpm_db_gmac_conn_str[] = {
	BuildEnumString(TPM_GMAC_CON_DISC),
	BuildEnumString(TPM_GMAC_CON_QSGMII),
	BuildEnumString(TPM_GMAC_CON_SWITCH_4),
	BuildEnumString(TPM_GMAC_CON_SWITCH_5),
	BuildEnumString(TPM_GMAC_CON_SWITCH_6),
	BuildEnumString(TPM_GMAC_CON_RGMII1),
	BuildEnumString(TPM_GMAC_CON_RGMII2),
	BuildEnumString(TPM_GMAC_CON_GE_PHY),
};

db_enum_string_t tpm_db_gmac_func_str[] = {
	BuildEnumString(TPM_GMAC_FUNC_NONE),
	BuildEnumString(TPM_GMAC_FUNC_LAN),
	BuildEnumString(TPM_GMAC_FUNC_WAN),
	BuildEnumString(TPM_GMAC_FUNC_LAN_AND_WAN),
	BuildEnumString(TPM_GMAC_FUNC_VIRT_UNI),
	BuildEnumString(TPM_GMAC_FUNC_LAN_UNI),
};

db_enum_string_t tpm_pnc_ranges_str[] = {
	BuildEnumString(TPM_PNC_MNGMT_DS),
	BuildEnumString(TPM_PNC_MAC_LEARN),
	BuildEnumString(TPM_PNC_CPU_WAN_LPBK_US),
	BuildEnumString(TPM_PNC_NUM_VLAN_TAGS),
	BuildEnumString(TPM_PNC_VIRT_UNI),
	BuildEnumString(TPM_PNC_MULTI_LPBK),
	BuildEnumString(TPM_PNC_LOOP_DET_US),
	BuildEnumString(TPM_PNC_L2_MAIN),
	BuildEnumString(TPM_PNC_ETH_TYPE),
	BuildEnumString(TPM_PNC_IGMP),
	BuildEnumString(TPM_PNC_IPV4_MC_DS),
	BuildEnumString(TPM_PNC_IPV4_MAIN),
	BuildEnumString(TPM_PNC_TCP_FLAG),
	BuildEnumString(TPM_PNC_TTL),
	BuildEnumString(TPM_PNC_IPV4_PROTO),
	BuildEnumString(TPM_PNC_IPV4_FRAG),
	BuildEnumString(TPM_PNC_IPV4_LEN),
	BuildEnumString(TPM_PNC_IPV6_HOPL),
	BuildEnumString(TPM_PNC_IPV6_GEN),
	BuildEnumString(TPM_PNC_IPV6_MC_SIP),
	BuildEnumString(TPM_PNC_IPV6_DIP),
	BuildEnumString(TPM_PNC_IPV6_MC_DS),
	BuildEnumString(TPM_PNC_IPV6_NH),
	BuildEnumString(TPM_PNC_IPV6_L4_MC_DS),
	BuildEnumString(TPM_PNC_IPV6_L4),
	BuildEnumString(TPM_PNC_CNM_IPV4_PRE),
	BuildEnumString(TPM_PNC_CNM_MAIN),
	BuildEnumString(TPM_PNC_CATCH_ALL),
};

db_enum_string_t tpm_api_range_type_str[] = {
	BuildEnumString(TPM_RANGE_TYPE_ACL),
	BuildEnumString(TPM_RANGE_TYPE_TABLE),
};

db_enum_string_t tpm_db_pnc_last_init_str[] = {
	BuildEnumString(TPM_PNC_RNG_LAST_INIT_DEF),
	BuildEnumString(TPM_PNC_RNG_LAST_INIT_DROP),
	BuildEnumString(TPM_PNC_RNG_LAST_INIT_TRAP),
};

static tpm_str_map_t api_section_str[] = {
	{TPM_L2_PRIM_ACL, "TPM_L2_PRIM_ACL"},
	{TPM_L3_TYPE_ACL, "TPM_L3_TYPE_ACL"},
	{TPM_IPV4_ACL, "TPM_IPV4_ACL"},
	{TPM_IPV4_MC, "TPM_IPV4_MC"},
	{TPM_IPV6_GEN_ACL, "TPM_IPV6_GEN_ACL"},
	{TPM_IPV6_DIP_ACL, "TPM_IPV6_DIP_ACL"},
	{TPM_IPV6_NH_ACL, "TPM_IPV6_NH_ACL"},
	{TPM_L4_ACL, "TPM_L4_ACL"},
};

static tpm_str_map_t api_type_str[] = {
	{TPM_API_MGMT, "MNGMT"},
	{TPM_API_CPU_LOOPBACK, "CPU_LOOPBACK"},
	{TPM_API_L2_PRIM, "L2      "},
	{TPM_API_L3_TYPE, "L3      "},
	{TPM_API_IPV4, "IPV4    "},
	{TPM_API_IPV4_MC, "IPV4_MC "},
	{TPM_API_IPV6_GEN, "IPV6_GEN"},
	{TPM_API_IPV6_DIP, "IPV6_DIP"},
	{TPM_API_IPV6_NH, "IPV6_NH "},
	{TPM_API_IPV6_L4, "L4      "},
};


/********************************************************************************/
/*                                Print Utils                                   */
/********************************************************************************/
char *db_mac_to_str(uint8_t * addr, char *str)
{
	if ((str != NULL) && (addr != NULL)) {
		str[0] = '\0';
		sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
		return str;
	}

	return NULL;
}

char *db_ipv4_to_str(uint8_t * ipaddr, char *str)
{
	if ((str != NULL) && (ipaddr != NULL)) {
		str[0] = '\0';
		sprintf(str, "%u.%u.%u.%u", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
		return str;
	}

	return NULL;
}

char *db_ipv6_to_str(uint8_t * ipaddr, char *str)
{
	int i, j;
	uint16_t addr[DB_IPV6_ADDR_LEN / 2];

	if ((str != NULL) && (ipaddr != NULL)) {
		for (i = 0, j = 0; i < DB_IPV6_ADDR_LEN; j++) {
			addr[j] = (ipaddr[i] << 8) | ipaddr[i + 1];
			i += 2;
		}

		str[0] = '\0';
		sprintf(str,
			"%x:%x:%x:%x:%x:%x:%x:%x",
			addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7]);
		return str;
	}

	return NULL;
}

/*******************************************************************************
* api_sec_to_str()
*
* DESCRIPTION:      Convert api section to string
*
* INPUTS:
* api_section      - API Section to retrieve configuration for
*
* RETURNS:
* On success, return pointer to entry in api_section_str[], on error return NULL.
*
* COMMENTS:
*
*******************************************************************************/
static uint8_t *api_sec_to_str(tpm_api_sections_t api_section)
{
	uint32_t i;

	for (i = 0; i < (sizeof(api_section_str) / sizeof(tpm_str_map_t)); i++) {
		if (api_section_str[i].enum_in == api_section)
			return (&(api_section_str[i].str_out[0]));
	}
	return (NULL);
}

/*******************************************************************************
* api_type_to_str()
*
* DESCRIPTION:      Convert api section to string
*
* INPUTS:
* tpm_api_type_t    - API Type to retrieve configuration for
*
* RETURNS:
* On success, return pointer to entry in api_type_str[], on error return NULL.
*
* COMMENTS:
*
*******************************************************************************/
static uint8_t *api_type_to_str(tpm_api_type_t api_type)
{
	uint32_t i;

	for (i = 0; i < (sizeof(api_type_str) / sizeof(tpm_str_map_t)); i++) {
		if (api_type_str[i].enum_in == api_type)
			return (&(api_type_str[i].str_out[0]));
	}
	return (NULL);
}

/*******************************************************************************
* pnc_rng_to_str()
*
* DESCRIPTION:      Convert Pnc Range  to string
*
* INPUTS:
* api_section      - API Section to retrieve configuration for
*
* RETURNS:
* On success, return pointer to entry in api_section_str[], on error return NULL.
*
* COMMENTS:
*
*******************************************************************************/
static uint8_t *pnc_rng_to_str(tpm_pnc_ranges_t range)
{
	uint32_t i;

	for (i = 0; i < (sizeof(tpm_pnc_ranges_str) / sizeof(db_enum_string_t)); i++) {
		if (tpm_pnc_ranges_str[i].enumPar == range)
			return (&(tpm_pnc_ranges_str[i].enumString[0]));
	}
	return (NULL);
}

/*******************************************************************************
* tpm_print_mac_key()
*
* DESCRIPTION:
*
* INPUTS:
* mac_key       - Print mac key
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_mac_key(tpm_mac_key_t *mac_key)
{
	char smac_str[DB_MAC_STR_LEN] = { '\0' };
	char dmac_str[DB_MAC_STR_LEN] = { '\0' };

	printk("=========================================\n");
	printk("          DA                SA           \n");
	printk("        DA mask           SA mask        \n");
	printk("=========================================\n");
	printk("  %s   %s\n", db_mac_to_str(mac_key->mac_da, dmac_str),
	       db_mac_to_str(mac_key->mac_sa, smac_str));
	printk("  %s   %s\n", db_mac_to_str(mac_key->mac_da_mask, dmac_str),
	       db_mac_to_str(mac_key->mac_sa_mask, smac_str));
	printk("=========================================\n");
	return;
}

/*******************************************************************************
* tpm_print_cpu_lpbk_entry()
*
* DESCRIPTION:
*
* INPUTS:
*   None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_cpu_lpbk_entry(void)
{
	uint16_t flow_id;
	tpm_cpu_loopback_t *lpbk_entry = NULL;

	print_horizontal_line(60);
	printk("  Index  trg_port  trg_queue  gem_port  rule_idx  mod_idx \n");
	print_horizontal_line(60);

	for (flow_id = 0; flow_id < TPM_MAX_CPU_LOOPBACK_NUM; flow_id++) {
		lpbk_entry = tpm_proc_get_loopback_entry(flow_id);

		if ((lpbk_entry != NULL) && (lpbk_entry->in_use == TPM_TRUE)) {
			printk("  %4.4d   0x%4.4x    %1.1d          %4.4d      %4.4d      %4.4d \n",
			       flow_id, lpbk_entry->trg_port, lpbk_entry->trg_queue,
			       lpbk_entry->gem_port, lpbk_entry->rule_idx, lpbk_entry->mod_idx);
		}

	}

	print_horizontal_line(60);
}

/*******************************************************************************
* tpm_print_tcam_lu_entry()
*
* DESCRIPTION:
*
* INPUTS:
*   None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_tcam_lu_entry(uint32_t owner_id, uint32_t api_group, uint32_t lu_num, uint32_t lu_reset)
{
	uint16_t rule_num;
	tpm_error_code_t ret_code;
	uint16_t valid_num;
	tpm_api_entry_count_t count_array[TPM_MAX_LU_ENTRY_NUM];
	uint16_t unrelated_num;

	ret_code = tpm_get_pnc_lu_entry(owner_id, api_group, (uint16_t) lu_num, lu_reset, &valid_num,
					count_array, &unrelated_num);

	if (ret_code != TPM_RC_OK) {
		printk(KERN_ERR "Failed to call tpm_count_get_pnc_lu_entry, ret_code[%d] \n", ret_code);
		return;
	}

	printk("\n");
	print_horizontal_line(60);
	printk("  Input owner_id[%d], api_group[%d], lu_num[%d] lu_reset[%d]\n",
	       owner_id, api_group, lu_num, lu_reset);
	printk("  output valid_num[%d], unrelated_num[%d]\n", valid_num, unrelated_num);
	print_horizontal_line(60);
	printk("  Index    rule_idx    hit_counter \n");
	print_horizontal_line(60);

	for (rule_num = 0; rule_num < valid_num; rule_num++) {
		printk("  %4.4d     %4.4d        %6.6d\n",
		       rule_num, count_array[rule_num].rule_idx, count_array[rule_num].hit_count);
	}

	print_horizontal_line(60);
}
/*******************************************************************************
* tpm_print_pnc_all_hit_counters()
*
* DESCRIPTION:
*
* INPUTS:
*   None
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_pnc_all_hit_counters(uint32_t                owner_id,
				    tpm_api_type_t          api_type,
				    uint32_t                high_thresh_pkts,
				    uint8_t                 counters_reset,
				    uint16_t                valid_counters,
				    tpm_api_entry_count_t  *count_array)
{
	uint16_t              rule_num;

	printk("\n");
	print_horizontal_line(60);
	printk("  Input owner_id[%d], api_type[%d], high_thresh_pkts[%d] counters_reset[%d]\n",
	       owner_id, api_type, high_thresh_pkts, counters_reset);
	printk("  Output valid_counters[%d]\n", valid_counters);
	print_horizontal_line(60);
	printk("  Index    rule_idx    hit_counter \n");
	print_horizontal_line(60);

	for (rule_num = 0; rule_num < valid_counters; rule_num++) {
		printk("  %4.4d     %4.4d        %6.6d\n",
		       rule_num, count_array[rule_num].rule_idx, count_array[rule_num].hit_count);
	}

	print_horizontal_line(60);
}

/*******************************************************************************
* tpm_print_vlan_key()
*
* DESCRIPTION:
*
* INPUTS:
* vlan_key      - Print CPU WAN loopback entry
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_vlan_key(tpm_vlan_key_t *vlan_key)
{
	print_horizontal_line(49);
	printk("  TPID  VID  VID mask CFI CFI mask PBIT PBIT mask\n");
	print_horizontal_line(49);
	printk("  %4.4x  %4.4x  %4d   %4.4x    %1.1x     %1.1x      %2.2x     %2.2x\n",
	       vlan_key->tpid, vlan_key->tpid_mask, vlan_key->vid, vlan_key->vid_mask,
	       vlan_key->cfi, vlan_key->cfi_mask, vlan_key->pbit, vlan_key->pbit_mask);
	print_horizontal_line(49);
}

/*******************************************************************************
* tpm_print_ipv4_key()
*
* DESCRIPTION:
*
* INPUTS:
* ipv4_key      - Print ipv4 key
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_ipv4_key(tpm_ipv4_acl_key_t *ipv4_key, tpm_ipv4_add_key_t *ipv4_add_key)
{
	printk("IPV4:\n");
	printk("=======================================================\n");
	printk("    DST_IP       SRC_IP     DSCP PROT L4_DST L4_SRC\n");
	printk("=======================================================\n");
	printk("  %02x.%02x.%02x.%02x  %02x.%02x.%02x.%02x   %02x   %02x   %02x     %02x\n"
	       "  %02x.%02x.%02x.%02x  %02x.%02x.%02x.%02x   %02x \n",
	       ipv4_key->ipv4_dst_ip_add[0], ipv4_key->ipv4_dst_ip_add[1], ipv4_key->ipv4_dst_ip_add[2],
	       ipv4_key->ipv4_dst_ip_add[3], ipv4_key->ipv4_src_ip_add[0], ipv4_key->ipv4_src_ip_add[1],
	       ipv4_key->ipv4_src_ip_add[2], ipv4_key->ipv4_src_ip_add[3], ipv4_key->ipv4_dscp, ipv4_key->ipv4_proto,
	       ipv4_key->l4_dst_port, ipv4_key->l4_src_port, ipv4_key->ipv4_dst_ip_add_mask[0],
	       ipv4_key->ipv4_dst_ip_add_mask[1], ipv4_key->ipv4_dst_ip_add_mask[2], ipv4_key->ipv4_dst_ip_add_mask[3],
	       ipv4_key->ipv4_src_ip_add_mask[0], ipv4_key->ipv4_src_ip_add_mask[1], ipv4_key->ipv4_src_ip_add_mask[2],
	       ipv4_key->ipv4_src_ip_add_mask[3], ipv4_key->ipv4_dscp_mask);
	if (ipv4_add_key != NULL) {
		printk("=======================================================\n");
		printk("    IP_VER  IP_IHL  IP_LEN  IP_FLAG  IP_FRAG IP_TTL \n");
		printk("=======================================================\n");
		printk("     %01x        %01x     %04x      %02x      %04x   %02x   \n"
		       "     %01x        %01x      %01x        %01x        %01x \n",
		       ipv4_add_key->ipv4_ver, ipv4_add_key->ipv4_ihl, ipv4_add_key->ipv4_totlen,
		       ipv4_add_key->ipv4_flags, ipv4_add_key->ipv4_frag_offset, ipv4_add_key->ipv4_ttl,
		       ipv4_add_key->ipv4_ver_mask, ipv4_add_key->ipv4_ihl_mask, ipv4_add_key->ipv4_totlen_mask,
		       ipv4_add_key->ipv4_flags_mask, ipv4_add_key->ipv4_frag_offset_mask);
		printk("=======================================================\n");
	}
}

/*******************************************************************************
* tpm_print_l2_key()
*
* DESCRIPTION:
*
* INPUTS:
* l2_key      - Print l2 key
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_l2_key(tpm_l2_acl_key_t *l2_key)
{
	printk("\nL2 KEY:\n");

	tpm_print_mac_key(&(l2_key->mac));
	TPM_OS_DEB_WAIT();
	printk("\nVLAN 1:\n");
	tpm_print_vlan_key(&(l2_key->vlan1));
	printk("\nVLAN 2:\n");
	tpm_print_vlan_key(&(l2_key->vlan2));
	TPM_OS_DEB_WAIT();
	printk("L2 ethertype(%04x), pppoe_ses(%d), pppoe_proto(%x)\n",
	       l2_key->ether_type, l2_key->pppoe_hdr.ppp_session, l2_key->pppoe_hdr.ppp_proto);
	printk("gem_port(%x)\n", l2_key->gem_port);
	printk("=======================================================\n");
}

/*******************************************************************************
* tpm_print_l3_key()
*
* DESCRIPTION:
*
* INPUTS:
* l3_key      - Print l2 key
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_l3_key(tpm_l3_type_key_t *l3_key)
{
	printk("L3 KEY:\n");
	printk("L3 ethertype(%04x), pppoe_ses(%d), pppoe_proto(%x)\n",
	       l3_key->ether_type_key, l3_key->pppoe_key.ppp_session, l3_key->pppoe_key.ppp_proto);
	printk("=======================================================\n");
}

/*******************************************************************************
* tpm_print_etherports()
*
* DESCRIPTION:
*
* INPUTS:
* vlan_key      - Print vlan key
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_etherports(void)
{
	uint32_t i;

	printk("======================================================\n");
	printk(" TPM ethernet ports:                                  \n");
	printk("======================================================\n");
	printk("     ext   switch       chip             internal     \n");
	printk("                     connection         connection    \n");
	printk("======================================================\n");
	for (i = 0; i < TPM_MAX_NUM_ETH_PORTS; i++) {
		if (tpm_db.eth_ports[i].valid == TPM_DB_VALID) {
			printk(" %1d   %3d   ", i, tpm_db.eth_ports[i].port_src);

			if (tpm_db.eth_ports[i].int_connect == TPM_INTCON_SWITCH)
				printk(" %3d    ", tpm_db.eth_ports[i].switch_port);
			else
				printk("%8s", " ");

			printk("%15s  %17s \n",
			       tpm_db_chip_conn_str[tpm_db.eth_ports[i].chip_connect].enumString,
			       tpm_db_int_conn_str[tpm_db.eth_ports[i].int_connect].enumString);
		}
	}
	printk("======================================================\n");
	return;
}

/*******************************************************************************
* tpm_print_rx_modules()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_rx_modules(void)
{
	uint32_t i, j, q_val, q_size;

	printk("============================================================\n");
	printk(" TPM GMAC RX Queues:                                            \n");
	printk("============================================================\n");
	printk(" Queue  size\n");
	printk("============================================================\n");

	for (i = 0; i < TPM_MAX_NUM_GMACS; i++) {
		if (tpm_db_gmac_rx_val_get(i)) {
			printk("GMAC - %d\n", i);

			for (j = 0; j < TPM_MAX_NUM_RX_QUEUE; j++) {

				tpm_db_gmac_rx_q_conf_get(i, j, &q_val, &q_size);
				if (q_val)
					printk("q%1d  rxq_size %4d\n", j, q_size);
			}
		}
	}
	printk("======================================================\n");
	return;
}

/*******************************************************************************
* tpm_print_tx_modules()
*
* DESCRIPTION:
*
* INPUTS:
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
void tpm_print_tx_modules(void)
{
	uint32_t i, j;

	printk("============================================================\n");
	printk(" TPM TX modules:                                            \n");
	printk("============================================================\n");
	printk(" Queue      Sched          Q-owner      owner   size  weight\n");
	printk("                                        q-num               \n");
	printk("============================================================\n");

	for (i = 0; i < TPM_MAX_NUM_TX_PORTS; i++) {
		if (tpm_db.gmac_tx[i].valid == TPM_DB_VALID) {
			printk("TX Device - %d\n", i);

			for (j = 0; j < TPM_MAX_NUM_TX_QUEUE; j++) {
				if (tpm_db.gmac_tx[i].tx_queue[j].valid == TPM_DB_VALID) {
					printk("   %1d   %13s  %17s   %1d     %4d  %5d\n",
					       j,
					       tpm_db_sched_str[tpm_db.gmac_tx[i].tx_queue[j].sched_method].enumString,
					       tpm_db_txq_owner_str[tpm_db.gmac_tx[i].tx_queue[j].queue_owner].
					       enumString, tpm_db.gmac_tx[i].tx_queue[j].owner_queue_num,
					       tpm_db.gmac_tx[i].tx_queue[j].queue_size,
					       tpm_db.gmac_tx[i].tx_queue[j].queue_weight);
				}
			}

		}
	}
	printk("======================================================\n");
	return;
}

void tpm_print_gmac_config(void)
{
	printk("=========================================================\n");
	printk(" TPM GMAC configuration:                                 \n");
	printk("=========================================================\n");
	printk("    Num                     Connection                   \n");
	printk(" tcont/llid         G0                      G1           \n");
	printk("    %4.4x    %18s  %18s\n",
	       tpm_db.num_valid_tcont_llid,
	       tpm_db_gmac_conn_str[tpm_db.gmac_port_conf[0].conn].enumString,
	       tpm_db_gmac_conn_str[tpm_db.gmac_port_conf[1].conn].enumString);
	printk("                    mh_en                  mh_en    \n");
	printk("                      %1d                      %1d\n",
	       tpm_db.gmac_mh_en[1], tpm_db.gmac_mh_en[0]);

	printk("====================================================\n");
	return;
}

void tpm_print_gmac_func(void)
{
	printk("=============================================================\n");
	printk(" TPM GMAC functions:                                         \n");
	printk("=============================================================\n");
	printk("        PMAC                 G0                   G1         \n");
	printk("=============================================================\n");
	printk(" %18s  %18s  %18s\n",
	       tpm_db_gmac_func_str[tpm_db.gmac_func[2]].enumString,
	       tpm_db_gmac_func_str[tpm_db.gmac_func[0]].enumString,
	       tpm_db_gmac_func_str[tpm_db.gmac_func[1]].enumString);
	printk("====================================================\n");
	return;
}

void tpm_print_bm_buffers(void)
{
	uint32_t i, valid, large_pkt_buf, small_pkt_buf;

	printk("=============================================================\n");
	printk(" Buffer Mngmt Pools:                                         \n");
	printk("=============================================================\n");
	printk("        Pool   cfg_val  large_bufs   small_bufs   \n");
	printk("=============================================================\n");
	for (i = 0; i < TPM_MAX_NUM_GMACS; i++) {
		tpm_db_gmac_bm_bufs_conf_get(i, &valid, &large_pkt_buf, &small_pkt_buf);
		printk("          %d     %d          %d          %d\n",
		       i, valid, large_pkt_buf, small_pkt_buf);
	}
	printk("====================================================\n");
	return;
}

void tpm_print_igmp(void)
{
#if 0
	printk("===================\n");
	printk(" IGMP:             \n");
	printk("===================\n");
	printk("Valid  Snoop  CPU-q\n");
	printk("===================\n");
	printk(" %3s    %3d    %2d \n",
	       (tpm_db.igmp_def.valid == TPM_DB_VALID ? "yes" : "no"),
	       tpm_db.igmp_def.igmp_snoop, tpm_db.igmp_def.igmp_cpu_queue);
	printk("========================\n");
	return;
#endif

	printk("===================\n");
	printk(" Multicast:   value\n");
	printk("===================\n");
	printk("Filter mode        %d\n", tpm_db.igmp_def.filter_mode);
	printk("Per UNI Vlan Xlate %d\n", tpm_db.igmp_def.per_uni_vlan_xlat);
	printk("MC PPPoE enable    %d\n", tpm_db.igmp_def.mc_pppoe_enable);
	printk("MC hwf queue       %d\n", tpm_db.igmp_def.mc_hwf_queue);
	printk("MC cpu queue       %d\n", tpm_db.igmp_def.mc_cpu_queue);
	printk("===================\n");
	return;
}

void tpm_print_misc(void)
{
	char *gmac_name[] = {"GMAC_0","GMAC_1","PMAC"};
	char *pon_type[] = {"EPON","GPON","P2P","NONE"};

	printk("==========================\n");
	printk(" TPM misc:          value \n");
	printk("==========================\n");
	printk("OMCI ETY            %4.4x\n", tpm_db.init_misc.omci_etype);
	printk("Debug port          %4d\n", tpm_db.init_misc.pnc_init_debug_port);
	printk("PON type            %s\n", pon_type[tpm_db.init_misc.pon_type]);

	if ((tpm_db.init_misc.backup_wan < TPM_MAX_GMAC) && (tpm_db.init_misc.backup_wan >= TPM_ENUM_GMAC_0))
		printk("Backup WAN          %s\n", gmac_name[tpm_db.init_misc.backup_wan]);
	else
		printk("Backup WAN          N/A");
	printk("DS MH select source %4d\n", tpm_db.init_misc.ds_mh_set_conf);
	printk("CFG PNC parse       %4d\n", tpm_db.init_misc.cfg_pnc_parse);
	printk("CPU loopback        %4d\n", tpm_db.init_misc.cpu_loopback);
	printk("Double Tag support  %4d\n", tpm_db.init_misc.dbl_tag);
	printk("IPV6 5T enable      %4d\n", tpm_db.init_misc.ipv6_5t_enable);
	printk("Virtual UNI-en:%4d uni-src %4d switch port %4d\n",
	       tpm_db.func_profile.virt_uni_info.enabled,
	       tpm_db.func_profile.virt_uni_info.uni_port,
	       tpm_db.func_profile.virt_uni_info.switch_port);
	printk("===============================\n");
	printk("TRACE DEBUG info    0x%08x\n", tpm_glob_trace);
	printk("===============================\n");

	return;
}

void tpm_print_owners(void)
{

	printk("\ntpm_print_owners : owners tables NOT USED YET \n");

	return;
}

void tpm_print_vlan_etype(void)
{
	uint32_t i;

	printk("========================\n");
	printk(" VLAN ETYs:             \n");
	printk("========================\n");
	for (i = 0; i < TPM_NUM_VLAN_ETYPE_REGS; i++) {
		if (tpm_db.vlan_etype[i].valid == TPM_DB_VALID)
			printk(" %2d   %4.4x\n", i, tpm_db.vlan_etype[i].tpid_ether_type);
	}

	return;
}

void tpm_print_valid_api_sections(void)
{
	tpm_api_sections_t cur_section = -1, next_section;
	int32_t last_valid;
	int32_t ret_code;
	uint32_t api_rng_size, num_valid_entries, tbl_start_ind;

	printk("==================================================\n");
	printk(" Valid API ranges:                         \n");
	printk("==================================================\n");
	printk("      Name       Size    Num     Last   Tbl\n");
	printk("                         entries  valid  Start\n");
	printk("==================================================\n");

	/* Loop through valid API ranges and print them */
	ret_code = tpm_db_api_section_val_get_next(cur_section, &next_section,
						   &api_rng_size, &num_valid_entries, &last_valid, &tbl_start_ind);
	IF_ERROR(ret_code);

	while (next_section != TPM_INVALID_SECTION) {
		printk("%15s  %4d    %4d    %4d    %4d\n",
		       api_sec_to_str(next_section), api_rng_size, num_valid_entries, last_valid, tbl_start_ind);

		cur_section = next_section;
		ret_code = tpm_db_api_section_val_get_next(cur_section, &next_section,
							   &api_rng_size, &num_valid_entries, &last_valid,
							   &tbl_start_ind);
		IF_ERROR(ret_code);
	}
	printk("===========================================\n");
	return;
}

void tpm_print_full_api_section(tpm_api_sections_t api_section)
{
	uint32_t rule_idx, bi_dir, i;
	int32_t ret_code;
	int32_t cur_rule, next_rule, last_valid;
	uint32_t api_rng_size, num_valid_entries, tbl_start_ind;
	tpm_pnc_ranges_t prim_pnc_range;
	tpm_rule_entry_t api_data;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_con;

	/* Print API Range Header */
	printk("====================================================\n");
	printk(" Full API range:                          \n");
	printk("====================================================\n");
	printk("      Name       Size    Num     Last    Start \n");
	printk("                         entries  valid     Ind  \n");
	printk("====================================================\n");

	ret_code =
	    tpm_db_api_section_get(api_section, &api_rng_size, &num_valid_entries, &prim_pnc_range, &last_valid,
				   &tbl_start_ind);
	if (ret_code != TPM_OK) {
		printk("%15s - invalid \n", api_sec_to_str(api_section));
		return;
	}

	printk("%15s %4d    %4d    %4d    %4d\n",
	       api_sec_to_str(api_section), api_rng_size, num_valid_entries, last_valid, tbl_start_ind);

	cur_rule = -1;

	tpm_db_api_entry_val_get_next(api_section, cur_rule, &next_rule, &rule_idx, &bi_dir,
				      &api_data, &mod_con, &pnc_con);

	printk("=========================================================\n");
	printk("  Rule  Rule  Bi-dir  Mod  Mod |  Ranges    PnC    PnC   \n");
	printk("  num   idx           cmd  bm  |  total    range  index  \n");
	printk("=========================================================\n");

	while (next_rule != -1) {
		printk("  %3d  %5d   %3s    %3d  %2d    %4d",
		       next_rule, rule_idx,
		       (bi_dir ? "yes" : "no"), mod_con.mod_cmd_ind, mod_con.mod_cmd_mac, pnc_con.num_pnc_ranges);

		for (i = 0; i < pnc_con.num_pnc_ranges; i++) {
			printk("   %02d     %03d\n",
			       pnc_con.pnc_conn_tbl[i].pnc_range, pnc_con.pnc_conn_tbl[i].pnc_index);
		}

		cur_rule = next_rule;
		tpm_db_api_entry_val_get_next(api_section, cur_rule, &next_rule, &rule_idx, &bi_dir,
					      &api_data, &mod_con, &pnc_con);
	}
	printk("=================================================================\n");
	return;
}

void tpm_print_valid_pnc_ranges(void)
{
	int32_t ret_code;
	tpm_db_pnc_range_t range_data;
	tpm_pnc_ranges_t cur_range, next_range;

	/* Print PNC Range Header */
	printk("==============================================================================================\n");
	printk(" Valid PnC ranges:                                                                            \n");
	printk("==============================================================================================\n");
	printk(" Range                        LU   Res Size Aging LU      Range         API      RSRV  Free     Num         Last    \n");
	printk("                                   Lvl      group skip  Start  End   Start  End  entr  entr     rsts        init        \n");
	printk("==============================================================================================\n");

	cur_range = -1;

	ret_code = tpm_db_pnc_rng_val_get_next(cur_range, &next_range, &range_data);
	IF_ERROR(ret_code);

	while (next_range != -1) {
		printk(" %24s    %2d   %2d %4d  %4d  %2d    %4d   %4d  %4d  %4d  %4d  %4d  %s\n",
		       pnc_rng_to_str(next_range),
		       range_data.pnc_range_conf.base_lu_id,
		       range_data.pnc_range_conf.min_reset_level,
		       range_data.pnc_range_conf.range_size,
		       range_data.pnc_range_conf.cntr_grp,
		       range_data.pnc_range_conf.lu_mask,
		       range_data.pnc_range_conf.range_start,
		       range_data.pnc_range_conf.range_end,
		       range_data.pnc_range_conf.api_start,
		       range_data.pnc_range_conf.api_end,
		       range_data.pnc_range_oper.free_entries,
		       range_data.pnc_range_oper.num_resets,
		       tpm_db_pnc_last_init_str[range_data.pnc_range_conf.init_last_entry].enumString);
		cur_range = next_range;
		ret_code = tpm_db_pnc_rng_val_get_next(cur_range, &next_range, &range_data);
		IF_ERROR(ret_code);
	}
	printk("================================================================================\n");
	return;
}

void tpm_print_pnc_shadow_range(uint32_t valid_only, uint32_t start, uint32_t end)
{
	uint32_t i;
	uint32_t valid_entry;
	tpm_pnc_all_t pnc_data;

	printk("======================================================================\n");
	printk(" Valid PnC shadow range:                                              \n");
	printk("======================================================================\n");

	if ((end >= (TPM_PNC_SIZE)) || (start > end)) {
		printk("Invalid params: start(%d) end(%d)\n", start, end);
		return;
	}

	for (i = start; i <= end; i++) {
		tpm_db_pnc_shdw_ent_get(i, &valid_entry, &pnc_data);
		if ((valid_only == TPM_FALSE) || (valid_entry == TPM_TRUE)) {
			printk("PNC Shadow Entry (%d)\n", i);
			tpm_pnc_print_sw_entry(&pnc_data);
		}
	}
	return;
}

void tpm_print_mh_port_vector_table(void)
{
	uint32_t i;

	printk("======================================================================\n");
	printk(" MH port vector table:                                                \n");
	printk("======================================================================\n");
	printk("=  REG#     UNI_VEC                 PNC_VEC               AMBER_VEC  =\n");
	printk("======================================================================\n");

	for (i = 0; i < TPM_TX_MAX_MH_REGS; i++) {
		printk("= %02d      0x%05x                0x%08x                 0x%04x   ==\n",
		       i, tpm_db.tpm_mh_port_vector_tbl[i].uni_vector,
		       tpm_db.tpm_mh_port_vector_tbl[i].pnc_vector, tpm_db.tpm_mh_port_vector_tbl[i].amber_port_vector);
	}
	printk("======================================================================\n");
	return;
}

void tpm_print_init_tables(void)
{
	tpm_print_etherports();
	tpm_print_rx_modules();
	tpm_print_tx_modules();
	tpm_print_gmac_config();
	tpm_print_gmac_func();
	tpm_print_bm_buffers();
	tpm_print_igmp();
	tpm_print_misc();
	tpm_print_owners();
	tpm_print_vlan_etype();
	tpm_print_mh_port_vector_table();
}

void tpm_print_api_dump_head(void)
{
	printk("==========================================================================================================================================================\n");
	printk("=  Section    Rule   Rule   PNC     Src_Port     Parse_BM      Parse_Flag_BM     Next       Pkt_Action          Target_Port         Mod_BM        Mod.   =\n");
	printk("=             No.    Ind.   Entry                                                Phase                       (Gem_Port)(Queue)                    Entry  =\n");
	printk("==========================================================================================================================================================\n");
}

char *tpm_db_params_illegal_str = "??";

char *tpm_db_api_type_str[TPM_MAX_API_TYPES] = {
	"MGNT",
	"MAC_LEARN",
	"CPU_LPBK",
	"L2",
	"L3",
	"IPV4",
	"IPV4_MC",
	"IPV6_GEN",
	"IPV6_DIP",
	"IPV6_MC",
	"IPV6_NH",
	"IPV6_L4",
	"CTC_CNM"
};

char *tpm_db_src_port_str[] = {
       "UNI_0",
	"UNI_1",
	"UNI_2",
	"UNI_3",
	"UNI_4",
	"UNI_5",
	"UNI_6",
	"UNI_7",
	"UNI_VIRT",
	"WAN",
	"UNI_ANY",
	"PORT_ANY",
};

char *tpm_db_next_phase_str[] = {
	"L2",
	"L3",
	"IPv4",
	"IPv6_GEN",
	"IPv6_DIP",
	"IPv6_NH",
	"IPV6_L4",
	"CTC_CM",
	"DONE",
};

tpm_str_map_t tpm_db_none_parse_type_str[] = {
	{0, ""}
};

tpm_str_map_t tpm_db_l2_parse_type[] = {
	{TPM_L2_PARSE_MAC_DA, "DA"},
	{TPM_L2_PARSE_MAC_SA, "SA"},
	{TPM_L2_PARSE_ONE_VLAN_TAG, "ONE_TAG"},
	{TPM_L2_PARSE_TWO_VLAN_TAG, "TWO_TAGS"},
	{TPM_L2_PARSE_ETYPE, "ETYPE"},
	{TPM_L2_PARSE_PPPOE_SES, "PPPOE_SES"},
	{TPM_L2_PARSE_PPP_PROT, "PPP_PROT"},
	{TPM_L2_PARSE_GEMPORT, "GEMPORT"},
	{0, ""},
};

tpm_str_map_t tpm_db_l3_parse_type[] = {
	{TPM_L2_PARSE_ETYPE, "ETYPE"},
	{TPM_L2_PARSE_PPPOE_SES, "PPPOE_SES"},
	{TPM_L2_PARSE_PPP_PROT, "PPP_PROT"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv4_parse_type[] = {
	{TPM_IPv4_PARSE_SIP,        "IPV4_SRC"},
	{TPM_IPv4_PARSE_DIP,        "IPV4_DST"},
	{TPM_IPv4_PARSE_DSCP, "DSCP"},
	{TPM_IPv4_PARSE_PROTO,      "L4_PROTO"},
	{TPM_PARSE_L4_SRC, "L4_SRC"},
	{TPM_PARSE_L4_DST, "L4_DST"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv4_mc_parse_type[] = {
	{TPM_IPv4_PARSE_SIP, "SIP"},
	{TPM_IPv4_PARSE_DIP, "DIP"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv6_gen_parse_type[] = {
	{TPM_IPv6_PARSE_SIP, "SIP"},
	{TPM_IPv6_PARSE_DSCP, "DSCP"},
	{TPM_IPv6_PARSE_HOPL, "HOPL"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv6_gen_5t_parse_type[] = {
	{TPM_IPv6_PARSE_SIP, "SIP"},
	{TPM_PARSE_L4_SRC, "L4_SRC"},
	{TPM_PARSE_L4_DST, "L4_DST"},
	{0, ""},
};


tpm_str_map_t tpm_db_ipv6_dip_parse_type[] = {
	{TPM_IPv6_PARSE_DIP, "DIP"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv6_dip_5t_parse_type[] = {
	{TPM_IPv6_PARSE_SIP, "SIP"},
	{TPM_IPv6_PARSE_DIP, "DIP"},
	{TPM_PARSE_L4_SRC, "L4_SRC"},
	{TPM_PARSE_L4_DST, "L4_DST"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv6_mc_parse_type[] = {
	{TPM_IPv4_PARSE_DIP, "DIP"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv6_nh_parse_type[] = {
	{TPM_IPv6_PARSE_NH, "NH"},
	{0, ""},
};

tpm_str_map_t tpm_db_ipv6_l4_parse_type[] = {
	{TPM_PARSE_L4_SRC, "L4_SRC"},
	{TPM_PARSE_L4_DST, "L4_DST"},
	{0, ""},
};
tpm_str_map_t tpm_db_ctc_cm_parse_type[] = {
	{TPM_L2_PARSE_MAC_DA, "DA"},
	{TPM_L2_PARSE_MAC_SA, "SA"},
	{TPM_L2_PARSE_ONE_VLAN_TAG, "ONE_TAG"},
	{TPM_L2_PARSE_TWO_VLAN_TAG, "TWO_TAGS"},
	{TPM_L2_PARSE_ETYPE, "ETYPE"},
	{TPM_L2_PARSE_PPPOE_SES, "PPPOE_SES"},
	{TPM_L2_PARSE_PPP_PROT, "PPP_PROT"},

	{TPM_IPv4_PARSE_SIP << API_DUMP_IPV4_PARSE_BM_START,        "IPV4_SIP"},
	{TPM_IPv4_PARSE_DIP << API_DUMP_IPV4_PARSE_BM_START,        "IPV4_DIP"},
	{TPM_IPv4_PARSE_DSCP << API_DUMP_IPV4_PARSE_BM_START,       "IPV4_DSCP"},
	{TPM_IPv4_PARSE_PROTO << API_DUMP_IPV4_PARSE_BM_START,      "IPV4_PROTO"},
	{TPM_PARSE_L4_SRC << API_DUMP_IPV4_PARSE_BM_START,          "L4_SRC"},
	{TPM_PARSE_L4_DST << API_DUMP_IPV4_PARSE_BM_START,          "L4_DST"},

	{TPM_IPv6_PARSE_SIP << API_DUMP_IPV6_PARSE_BM_START,        "IPV6_SIP"},
	{TPM_IPv6_PARSE_DSCP << API_DUMP_IPV6_PARSE_BM_START,       "IPV6_DSCP"},
	{TPM_IPv6_PARSE_HOPL << API_DUMP_IPV6_PARSE_BM_START,       "IPV6_HOPL"},
	{TPM_IPv6_PARSE_DIP << API_DUMP_IPV6_PARSE_BM_START,        "IPV6_DIP"},
	{TPM_IPv6_PARSE_NH << API_DUMP_IPV6_PARSE_BM_START,         "IPV6_NH"},
	{TPM_PARSE_L4_SRC << API_DUMP_IPV6_PARSE_BM_START,          "L4_SRC"},
	{TPM_PARSE_L4_DST << API_DUMP_IPV6_PARSE_BM_START,          "L4_DST"},
	{0, ""},
};

tpm_str_map_t *tpm_db_parse_type_str[TPM_MAX_API_TYPES] = {
	/* TPM_API_MGMT */
	tpm_db_none_parse_type_str,
	/* TPM_API_MAC_LEARN */
	tpm_db_none_parse_type_str,
	/* TPM_API_CPU_LOOPBACK */
	tpm_db_none_parse_type_str,
	/* TPM_API_L2_PRIM */
	tpm_db_l2_parse_type,
	/* TPM_API_L3_TYPE */
	tpm_db_l3_parse_type,
	/* TPM_API_IPV4 */
	tpm_db_ipv4_parse_type,
	/* TPM_API_IPV4_MC */
	tpm_db_ipv4_mc_parse_type,
	/* TPM_API_IPV6_GEN */
	tpm_db_ipv6_gen_parse_type,
	/* TPM_API_IPV6_DIP */
	tpm_db_ipv6_dip_parse_type,
	/* TPM_API_IPV6_MC */
	tpm_db_ipv6_mc_parse_type,
	/* TPM_API_IPV6_NH */
	tpm_db_ipv6_nh_parse_type,
	/* TPM_API_IPV6_L4 */
	tpm_db_ipv6_l4_parse_type,
	/* TPM_API_CTC_CM */
	tpm_db_ctc_cm_parse_type,
};

tpm_str_map_t tpm_db_parse_flag_type_str[] = {
	{TPM_PARSE_FLAG_TAG1_TRUE, "TAG1_TRUE"},
	{TPM_PARSE_FLAG_TAG1_FALSE, "TAG1_FALSE"},
	{TPM_PARSE_FLAG_TAG2_TRUE, "TAG2_TRUE"},
	{TPM_PARSE_FLAG_TAG2_FALSE, "TAG2_FALSE"},
	{TPM_PARSE_FLAG_MTM_TRUE, "MTM_TRUE"},
	{TPM_PARSE_FLAG_MTM_FALSE, "MTM_FALSE"},
	{TPM_PARSE_FLAG_TO_CPU_TRUE, "TO_CPU_TRUE"},
	{TPM_PARSE_FLAG_TO_CPU_FALSE, "TO_CPU_FALSE"},
	{TPM_PARSE_FLAG_L4_UDP, "L4_UDP"},
	{TPM_PARSE_FLAG_L4_TCP, "L4_TCP"},
	{TPM_PARSE_FLAG_PPPOE_TRUE, "PPPOE_TRUE"},
	{TPM_PARSE_FLAG_PPPOE_FALSE, "PPPOE_FALSE"},
};

tpm_str_map_t tpm_db_action_type_str[] = {
	{TPM_ACTION_DROP_PK, "DROP_PK"},
	{TPM_ACTION_SET_TARGET_PORT, "SET_TRG_PORT"},
	{TPM_ACTION_SET_TARGET_QUEUE, "SET_TRG_Q"},
	{TPM_ACTION_SET_PKT_MOD, "SET_PKT_MOD"},
	{TPM_ACTION_TO_CPU, "TO_CPU"},
	{TPM_ACTION_MTM, "MTM"},
	{TPM_ACTION_CUST_CPU_PKT_PARSE, "CUST_PKT_PARSE"},
	{TPM_ACTION_SPEC_MC_VID, "SPEC_MC_VID"},
	{TPM_ACTION_UDP_CHKSUM_CALC, "UDP_CHKSUM_CALC"},
};

tpm_str_map_t tpm_db_target_type_str[] =
{
	{TPM_TRG_PORT_WAN,           "WAN_PORT"},
	{TPM_TRG_TCONT_0,            "TCONT_0"},
	{TPM_TRG_TCONT_1,            "TCONT_1"},
	{TPM_TRG_TCONT_2,            "TCONT_2"},
	{TPM_TRG_TCONT_3,            "TCONT_3"},
	{TPM_TRG_TCONT_4,            "TCONT_4"},
	{TPM_TRG_TCONT_5,            "TCONT_5"},
	{TPM_TRG_TCONT_6,            "TCONT_6"},
	{TPM_TRG_TCONT_7,            "TCONT_7"},
	{TPM_TRG_LLID_0,             "LLID_0"},
	{TPM_TRG_LLID_1,             "LLID_1"},
	{TPM_TRG_LLID_2,             "LLID_2"},
	{TPM_TRG_LLID_3,             "LLID_3"},
	{TPM_TRG_LLID_4,             "LLID_4"},
	{TPM_TRG_LLID_5,             "LLID_5"},
	{TPM_TRG_LLID_6,             "LLID_6"},
	{TPM_TRG_LLID_7,             "LLID_7"},
	{TPM_TRG_UNI_0,              "UNI_0"},
	{TPM_TRG_UNI_1,              "UNI_1"},
	{TPM_TRG_UNI_2,              "UNI_2"},
	{TPM_TRG_UNI_3,              "UNI_3"},
	{TPM_TRG_UNI_4,              "UNI_4"},
	{TPM_TRG_UNI_5,              "UNI_5"},
	{TPM_TRG_UNI_6,              "UNI_6"},
	{TPM_TRG_UNI_7,              "UNI_7"},
	{TPM_TRG_UNI_VIRT,           "UNI_VIRT"},
	{TPM_TRG_PORT_CPU,           "CPU"},
	{TPM_TRG_PORT_UNI_ANY,       "UNI_ANY"},
	{TPM_TRG_PORT_UNI_CPU_LOOP,  "UNI_CPU_LOOP"}
};
tpm_str_map_t tpm_db_mod_type_str[] = {
	{TPM_MH_SET, "MH_SET"},
	{TPM_MAC_DA_SET, "DA_SET"},
	{TPM_MAC_SA_SET, "SA_SET"},
	{TPM_VLAN_MOD, "VLAN_MOD"},
	{TPM_PPPOE_DEL, "PPPOE_DEL"},
	{TPM_PPPOE_ADD, "PPPOE_ADD"},
	{TPM_DSCP_SET, "DSCP_SET"},
	{TPM_TTL_DEC, "TTL_DEC"},
	{TPM_IPV4_UPDATE, "IPV4_UPDATE"},
	{TPM_IPV4_SRC_SET, "IPV4_SRC_SET"},
	{TPM_IPV4_DST_SET, "IPV4_DST_SET"},
	{TPM_IPV6_UPDATE, "IPV6_UPDATE"},
	{TPM_HOPLIM_DEC, "HOPLIM_DEC"},
	{TPM_IPV6_SRC_SET, "IPV6_SRC_SET"},
	{TPM_IPV6_DST_SET, "IPV6_DST_SET"},
	{TPM_L4_SRC_SET, "L4_SRC_SET"},
	{TPM_L4_DST_SET, "L4_DST_SET"},
};

char *tpm_db_mc_uni_mode_str[] = {
	"EXCLUDE",
	"TRANSPARENT",
	"STRIP",
	"TRANSLATE",
};

char *db_parse_type_to_str(uint32_t api_type, uint32_t parse_type)
{
	uint32_t i;
	tpm_init_ipv6_5t_enable_t ipv6_5t_enable;

	if (parse_type == 0)
		return "";

	if (api_type >= TPM_MAX_API_TYPES)
		return tpm_db_params_illegal_str;

	tpm_db_ipv6_5t_enable_get(&ipv6_5t_enable);
	if (ipv6_5t_enable == TPM_IPV6_5T_ENABLED) {
		tpm_db_parse_type_str[TPM_API_IPV6_GEN] = tpm_db_ipv6_gen_5t_parse_type;
		tpm_db_parse_type_str[TPM_API_IPV6_DIP] = tpm_db_ipv6_dip_5t_parse_type;
	}

	for (i = 0; tpm_db_parse_type_str[api_type][i].enum_in != 0; i++) {
		if (tpm_db_parse_type_str[api_type][i].enum_in == parse_type)
			return (&(tpm_db_parse_type_str[api_type][i].str_out[0]));
	}

	return tpm_db_params_illegal_str;
}

char *db_parse_flag_type_to_str(uint32_t parse_flag_type)
{
	uint32_t i;

	if (parse_flag_type == 0)
		return "";

	for (i = 0; i < (sizeof(tpm_db_parse_flag_type_str) / sizeof(tpm_str_map_t)); i++) {
		if (tpm_db_parse_flag_type_str[i].enum_in == parse_flag_type)
			return (&(tpm_db_parse_flag_type_str[i].str_out[0]));
	}

	return tpm_db_params_illegal_str;
}

char *db_target_type_to_str(uint32_t action_type, tpm_pkt_frwd_t  *pkt_frwd)
{
	static char target_str[50];
	char *port_str = "";
	char gem_str[10] = "";
	char q_str[10] = "";
	uint32_t i;
	tpm_dir_t            dir;
	tpm_db_pon_type_t    pon_type;
	tpm_gmacs_enum_t     act_wan = tpm_db_active_wan_get();

	memset(target_str, 0, sizeof(target_str));

	if (SET_TARGET_PORT(action_type)) {
		for (i = 0; i < (sizeof(tpm_db_target_type_str)/sizeof(tpm_str_map_t)); i++) {
			if (tpm_db_target_type_str[i].enum_in == pkt_frwd->trg_port) {
				port_str = &(tpm_db_target_type_str[i].str_out[0]);
				break;
			}
		}

		dir = TPM_DIR_US;
	/* Get pon_type */
		tpm_db_pon_type_get(&pon_type);

		if (TO_GPON(dir, pkt_frwd->trg_port, pon_type, act_wan)) {
			sprintf(gem_str, "%03d",  pkt_frwd->gem_port);
		}
	}

	if (SET_TARGET_QUEUE(action_type))
		sprintf(q_str, "Q%d",  pkt_frwd->trg_queue);


	sprintf(target_str, " %-9s %3s | %2s ", port_str, gem_str, q_str);

	return target_str;
}
char *db_action_type_to_str(uint32_t action_type)
{
	uint32_t i;

	if (action_type == 0)
		return "";

	for (i = 0; i < (sizeof(tpm_db_action_type_str) / sizeof(tpm_str_map_t)); i++) {
		if (tpm_db_action_type_str[i].enum_in == action_type)
			return (&(tpm_db_action_type_str[i].str_out[0]));
	}

	return tpm_db_params_illegal_str;
}

char *db_mod_type_to_str(uint32_t mod_type)
{
	uint32_t i;

	if (mod_type == 0)
		return "";

	for (i = 0; i < (sizeof(tpm_db_mod_type_str) / sizeof(tpm_str_map_t)); i++) {
		if (tpm_db_mod_type_str[i].enum_in == mod_type)
			return (&(tpm_db_mod_type_str[i].str_out[0]));
	}

	return tpm_db_params_illegal_str;
}

void tpm_print_api_dump_line(uint8_t first_rule,
			     uint8_t first_line,
			     uint32_t api_type,
			     uint32_t rule_num,
			     uint32_t rule_idx,
			     uint32_t pnc_entry,
			     uint32_t src_port,
			     uint32_t parse_type,
			     uint32_t parse_flag_type,
			     uint32_t next_phase,
			     uint32_t action_type,
			     uint32_t mod_type,
			     uint32_t mod_entry,
			     uint32_t set_port_q,
			     tpm_pkt_frwd_t *pkt_frwd)
{
	char rule_num_str[8] = "";
	char rule_idx_str[8] = "";
	char pnc_entry_str[8] = "";
	char mod_entry_str[8] = "";

	char *empty_str = "";

	char *api_type_str = empty_str;
	char *src_port_str = empty_str;
	char *next_phase_str = empty_str;

	char *parse_type_str = empty_str;
	char *parse_flag_type_str = empty_str;
	char *action_type_str = empty_str;
	char *mod_type_str = empty_str;
	char *target_port_q_str   = empty_str;

	if (first_line) {
		sprintf(rule_num_str, "%03d", rule_num);
		sprintf(rule_idx_str, "%03d", rule_idx);
		sprintf(pnc_entry_str, "%03d", pnc_entry);
		if (mod_entry != 0)
			sprintf(mod_entry_str, "%04d", mod_entry);

		if (first_rule) {
			if (api_type < TPM_MAX_API_TYPES)
				api_type_str = tpm_db_api_type_str[api_type];
			else
				api_type_str = tpm_db_params_illegal_str;
		}

		if (src_port <= TPM_SRC_PORT_WAN_OR_LAN)
			src_port_str = tpm_db_src_port_str[src_port];
		else
			src_port_str = tpm_db_params_illegal_str;

		if (next_phase <= STAGE_DONE)
			next_phase_str = tpm_db_next_phase_str[next_phase];
		else
			next_phase_str = tpm_db_params_illegal_str;

	}

	if (parse_type != 0)
		parse_type_str = db_parse_type_to_str(api_type, parse_type);

	if (parse_flag_type != 0)
		parse_flag_type_str = db_parse_flag_type_to_str(parse_flag_type);

	if (action_type != 0) {
		action_type_str = db_action_type_to_str(action_type);
	}

	if (set_port_q != 0)
		target_port_q_str = db_target_type_to_str(set_port_q, pkt_frwd);

	if (mod_type != 0)
		mod_type_str = db_mod_type_to_str(mod_type);

    printk("=  %8s | %4s | %4s | %5s | %8s | %13s | %13s | %8s | %16s |%20s| %12s | %6s  =\n",
		api_type_str, rule_num_str, rule_idx_str, pnc_entry_str,
		src_port_str, parse_type_str, parse_flag_type_str,
	       next_phase_str, action_type_str, target_port_q_str, mod_type_str, mod_entry_str);
}

void tpm_print_api_dump_rule(uint8_t first_rule,
				tpm_api_type_t api_type,
				uint32_t rule_num,
				uint32_t rule_idx, uint32_t pnc_entry, uint32_t mod_entry, tpm_rule_entry_t *rule)
{
	uint8_t first_line = TPM_TRUE;
	uint32_t src_port = TPM_SRC_PORT_ILLEGAL;
	uint32_t next_phase = STAGE_DONE;
	uint32_t parse_bm = 0;
	uint32_t parse_flag_bm = 0;
	uint32_t action_bm = 0;
	uint32_t mod_bm = 0;
	uint32_t tgt_port_bm = 0;
	uint32_t parse_type = 0x1;
	uint32_t parse_flag_type = 0x1;
	uint32_t action_type = 0x1;
	uint32_t mod_type = 0x1;
	uint32_t tgt_port = 0x1;
	uint32_t bit_cnt;
	uint32_t set_port_q = 0;
	tpm_pkt_frwd_t *pkt_frwd = NULL;
	tpm_pkt_frwd_t mc_frwd;

	if (rule == NULL)
		return;

	switch (api_type) {
	case TPM_API_L2_PRIM:
		src_port = rule->l2_prim_key.src_port;
		next_phase = rule->l2_prim_key.rule_action.next_phase;
		parse_bm = rule->l2_prim_key.parse_rule_bm;
		parse_flag_bm = rule->l2_prim_key.parse_flags_bm;
		action_bm = rule->l2_prim_key.rule_action.pkt_act;
		mod_bm = rule->l2_prim_key.pkt_mod_bm;
		pkt_frwd = &rule->l2_prim_key.pkt_frwd;
		break;

	case TPM_API_L3_TYPE:
		src_port = rule->l3_type_key.src_port;
		next_phase = rule->l3_type_key.rule_action.next_phase;
		parse_bm = rule->l3_type_key.parse_rule_bm;
		parse_flag_bm = rule->l3_type_key.parse_flags_bm;
		action_bm = rule->l3_type_key.rule_action.pkt_act;
		pkt_frwd  = &rule->l3_type_key.pkt_frwd;
		break;

	case TPM_API_IPV4:
		src_port = rule->ipv4_key.src_port;
		next_phase = rule->ipv4_key.rule_action.next_phase;
		parse_bm = rule->ipv4_key.parse_rule_bm;
		parse_flag_bm = rule->ipv4_key.parse_flags_bm;
		action_bm = rule->ipv4_key.rule_action.pkt_act;
		mod_bm = rule->ipv4_key.pkt_mod_bm;
		pkt_frwd = &rule->ipv4_key.pkt_frwd;
		break;

	case TPM_API_IPV4_MC:
		src_port = TPM_SRC_PORT_WAN;
		parse_bm = TPM_IPv4_PARSE_DIP;
		if (rule->ipv4_mc_key.ignore_ipv4_src == 0)
			parse_bm |= TPM_IPv4_PARSE_SIP;
		mod_bm = 0;	/* ?? */
		/*mc_frwd is only used to print target port and target queue for api_dump*/
		action_bm = TPM_ACTION_SET_TARGET_PORT | TPM_ACTION_SET_TARGET_QUEUE;
		mc_frwd.trg_port = rule->ipv4_mc_key.dest_port_bm;
		mc_frwd.trg_queue = tpm_db.igmp_def.mc_hwf_queue;
		pkt_frwd = &mc_frwd;
		break;

	case TPM_API_IPV6_GEN:
		src_port = rule->ipv6_gen_key.src_port;
		next_phase = rule->ipv6_gen_key.rule_action.next_phase;
		parse_bm = rule->ipv6_gen_key.parse_rule_bm;
		parse_flag_bm = rule->ipv6_gen_key.parse_flags_bm;
		action_bm = rule->ipv6_gen_key.rule_action.pkt_act;
		mod_bm = rule->ipv6_gen_key.pkt_mod_bm;
		pkt_frwd = &rule->ipv6_gen_key.pkt_frwd;
		break;

	case TPM_API_IPV6_DIP:
		src_port = rule->ipv6_dip_key.src_port;
		next_phase = rule->ipv6_dip_key.rule_action.next_phase;
		parse_bm = rule->ipv6_dip_key.parse_rule_bm;
		parse_flag_bm = rule->ipv6_dip_key.parse_flags_bm;
		action_bm = rule->ipv6_dip_key.rule_action.pkt_act;
		mod_bm = rule->ipv6_dip_key.pkt_mod_bm;
		pkt_frwd = &rule->ipv6_dip_key.pkt_frwd;
		break;

	case TPM_API_IPV6_NH:
		src_port = TPM_SRC_PORT_WAN_OR_LAN;
		next_phase = rule->ipv6_nh_key.rule_action.next_phase;
		parse_bm = rule->ipv6_nh_key.parse_rule_bm;
		parse_flag_bm = rule->ipv6_nh_key.parse_flags_bm;
		action_bm = rule->ipv6_nh_key.rule_action.pkt_act;
		pkt_frwd = &rule->ipv6_nh_key.pkt_frwd;
		break;

	case TPM_API_IPV6_L4:
		src_port = rule->ipv6_l4_key.src_port;
		next_phase = rule->ipv6_l4_key.rule_action.next_phase;
		parse_bm = rule->ipv6_l4_key.parse_rule_bm;
		parse_flag_bm = rule->ipv6_l4_key.parse_flags_bm;
		action_bm = rule->ipv6_l4_key.rule_action.pkt_act;
		mod_bm = rule->ipv6_l4_key.pkt_mod_bm;
		pkt_frwd = &rule->ipv6_l4_key.pkt_frwd;
		break;

	case TPM_API_IPV6_MC:
		src_port = TPM_SRC_PORT_WAN;
		parse_bm = TPM_IPv6_PARSE_DIP;
		mod_bm = 0;	/* ?? */
		/*mc_frwd is only used to print target port and target queue for api_dump*/
		action_bm = TPM_ACTION_SET_TARGET_PORT | TPM_ACTION_SET_TARGET_QUEUE;
		mc_frwd.trg_port = rule->ipv6_mc_key.dest_port_bm;
		mc_frwd.trg_queue = tpm_db.igmp_def.mc_hwf_queue;
		pkt_frwd = &mc_frwd;
		break;

	case TPM_API_CNM:
		src_port = rule->cnm_key.src_port;
		parse_bm = rule->cnm_key.l2_parse_rule_bm;
		parse_bm |= (rule->cnm_key.ipv4_parse_rule_bm << API_DUMP_IPV4_PARSE_BM_START);
		parse_bm |= (rule->cnm_key.ipv6_parse_rule_bm << API_DUMP_IPV6_PARSE_BM_START);
		action_bm = rule->cnm_key.pkt_act;
		pkt_frwd = &rule->cnm_key.pkt_frwd;
		break;
	default:
		return;
	}

	action_bm &= 0xFFFF;	/* DON'T show the internal actions! */
	tgt_port_bm = pkt_frwd->trg_port;

	set_port_q = action_bm & (TPM_ACTION_SET_TARGET_PORT | TPM_ACTION_SET_TARGET_QUEUE);

	do {
		if (parse_bm != 0) {
			for (bit_cnt = 0; (parse_bm & 0x1) == 0; parse_bm = parse_bm >> 1, bit_cnt++)
				;
			parse_type = parse_type << bit_cnt;
		} else
			parse_type = 0;

		if (parse_flag_bm != 0) {
			for (bit_cnt = 0; (parse_flag_bm & 0x1) == 0; parse_flag_bm = parse_flag_bm >> 1, bit_cnt++)
				;
			parse_flag_type = parse_flag_type << bit_cnt;
		} else
			parse_flag_type = 0;

		if (action_bm != 0) {
			for (bit_cnt = 0; (action_bm & 0x1) == 0; action_bm = action_bm >> 1, bit_cnt++)
				;
			action_type = action_type << bit_cnt;
		} else
			action_type = 0;

		if (mod_bm != 0) {
			for (bit_cnt = 0; (mod_bm & 0x1) == 0; mod_bm = mod_bm >> 1, bit_cnt++)
				;
			mod_type = mod_type << bit_cnt;
		} else
			mod_type = 0;

		if (tgt_port_bm != 0) {
			for (bit_cnt = 0; (tgt_port_bm & 0x1) == 0; tgt_port_bm = tgt_port_bm >> 1, bit_cnt++)
				;
			tgt_port = tgt_port << bit_cnt;
			pkt_frwd->trg_port = tgt_port;
		} else
			pkt_frwd->trg_port = 0;


		tpm_print_api_dump_line(first_rule, first_line, api_type, rule_num, rule_idx,
					pnc_entry, src_port, parse_type, parse_flag_type, next_phase,
					action_type, mod_type, mod_entry, set_port_q, pkt_frwd);

		first_line = TPM_FALSE;
		set_port_q &= ~TPM_ACTION_SET_TARGET_QUEUE;

		parse_bm &= ~0x1;
		parse_flag_bm &= ~0x1;
		action_bm &= ~0x1;
		mod_bm &= ~0x1;
		tgt_port_bm &= ~0x1;

		if ((parse_bm == 0) && (parse_flag_bm == 0) && (action_bm == 0) && (mod_bm == 0) && (tgt_port_bm == 0))
			break;

	} while (TPM_TRUE);

}

void tpm_print_api_dump(tpm_api_type_t api_type)
{
	int8_t first_rule = 1;
	int32_t current_rule, rcode;
	int32_t rule_index;
	int32_t bi_dir;
	int32_t next_rule;
	int32_t rule_num;
	tpm_rule_entry_t tpm_rule;
	tpm_error_code_t tpm_ret;
	tpm_db_mod_conn_t mod_con;
	tpm_db_pnc_conn_t pnc_con;
	tpm_api_sections_t api_section;
	char *api_type_str = "";

	tpm_print_api_dump_head();

	tpm_db_api_section_get_from_api_type(api_type, &api_section);
	current_rule = -1;

	tpm_ret = tpm_db_api_entry_val_get_next(api_section, current_rule, &next_rule, &rule_index,
						&bi_dir, &tpm_rule, &mod_con, &pnc_con);

	if (-1 == next_rule) {
		if (api_type < TPM_MAX_API_TYPES)
			api_type_str = tpm_db_api_type_str[api_type];
		else
			api_type_str = tpm_db_params_illegal_str;

		printk("=  %8s | %4s | %4s | %5s | %8s | %13s | %13s | %8s | %16s |%20s| %12s | %5s   =\n",
		       api_type_str, "", "", "", "", "", "", "", "", "", "", "");
	}

	while (-1 != next_rule) {
		if (first_rule == 0)
			printk("=           |------+------+-------+----------+---------------+---------------+----------+------------------+--------------------+--------------+---------=\n");
		/* Get the rule_num */
		rcode = tpm_db_api_rulenum_get(api_section, rule_index, &rule_num);
		if (rcode != TPM_DB_OK) {
			printk("get DB failed: %d\n", rcode);
			return;
		}

		tpm_print_api_dump_rule(first_rule, api_type, rule_num, rule_index, pnc_con.pnc_conn_tbl[0].pnc_index,
					mod_con.mod_cmd_ind, &tpm_rule);

		tpm_ret = tpm_db_api_entry_val_get_next(api_section, next_rule, &next_rule, &rule_index,
							&bi_dir, &tpm_rule, &mod_con, &pnc_con);
		first_rule = 0;
	}

	printk("==========================================================================================================================================================\n");
	return;
}

void tpm_print_api_dump_all(void)
{
	tpm_api_type_t api_type;

	tpm_api_sections_t api_section;
	uint32_t api_rng_size;
	uint32_t num_valid_entries;
	tpm_pnc_ranges_t prim_pnc_range;
	int32_t last_valid_entry;
	uint32_t tbl_start;
	tpm_db_pnc_range_conf_t range_conf;

	printk("\n");
	for (api_type = TPM_API_L2_PRIM; api_type < TPM_MAX_API_TYPES; api_type++) {
		tpm_db_api_section_get_from_api_type(api_type, &api_section);
		tpm_db_api_section_get(api_section, &api_rng_size, &num_valid_entries,
					&prim_pnc_range, &last_valid_entry, &tbl_start);

		/* Get Range Conf */
		tpm_db_pnc_rng_conf_get(prim_pnc_range, &range_conf);

		tpm_print_api_dump(api_type);
		printk("\n");
	}

	return;
}

int tpm_print_sram_next_lookup(struct tcam_entry *te, char *buf)
{
	unsigned int word;
	unsigned int lookup;

	word = LU_DONE_OFFS / DWORD_LEN;
	lookup = te->sram.word[word] >> (LU_DONE_OFFS % DWORD_LEN);
	lookup &= 0x1;

	if (lookup)
		return sprintf(buf, " D");

	word = LU_ID_OFFS / DWORD_LEN;
	lookup = te->sram.word[word] >> (LU_ID_OFFS % DWORD_LEN);
	lookup &= LU_MASK;

	return sprintf(buf, " %d", lookup);
}

int tpm_print_sram_next_lookup_shift(struct tcam_entry *te, char *buf)
{
	unsigned int word, value;

	word = NEXT_LU_SHIFT_OFFS / DWORD_LEN;
	value = te->sram.word[word] >> (NEXT_LU_SHIFT_OFFS % DWORD_LEN);
	value &= SHIFT_IDX_MASK;

	if (value)
		return sprintf(buf, " %1d", value);

	return sprintf(buf, " -");
}

int tpm_print_sram_shift_update(struct tcam_entry *te, char *buf)
{
	unsigned int word;
	unsigned int index;
	unsigned int value;

	word = SHIFT_VAL_OFFS / DWORD_LEN;
	value = te->sram.word[word] >> (SHIFT_VAL_OFFS % DWORD_LEN);
	value &= SHIFT_VAL_MASK;

	word = SHIFT_IDX_OFFS / DWORD_LEN;
	index = te->sram.word[word] >> (SHIFT_IDX_OFFS % DWORD_LEN);
	index &= SHIFT_IDX_MASK;

	if (value)
		return sprintf(buf, " [%2.2d]=%3.3d", index, value);

	return sprintf(buf, " [ 0]= 0");
}

int tpm_print_sram_rxq(struct tcam_entry *te, char *buf)
{
	unsigned int rxq, force;

	rxq = sram_sw_get_rxq(te, &force);
	if (rxq)
		return sprintf(buf, " %1.1sQ%1.1d", force ? "f" : " ", rxq);

	return sprintf(buf, " ---");
}

int tpm_print_sram_ainfo(struct tcam_entry *te, char *buf)
{
	unsigned int word, shift, data, mask;
	int i, off = 0;

	word = AI_VALUE_OFFS / DWORD_LEN;
	shift = AI_VALUE_OFFS % DWORD_LEN;
	data = ((te->sram.word[word] >> shift) & AI_MASK);
	shift = AI_MASK_OFFS % DWORD_LEN;
	mask = ((te->sram.word[word] >> shift) & AI_MASK);

	if (mask) {
		for (i = 0; i < AI_BITS; i++) {
			if (mask & (1 << i))
				off += sprintf(buf + off, "%d", ((data & (1 << i)) != 0));
			else
				off += sprintf(buf + off, "x");
		}
	} else
		off += sprintf(buf + off, "-------");

	return off;
}

int tpm_print_tcam_ainfo(struct tcam_entry *te, char *buf)
{
	int i, data, mask;
	int off = 0;

	mask = ((te->mask.u.word[AI_WORD] >> AI_OFFS) & AI_MASK);
	if (mask == 0)
		off += sprintf(buf + off, "-------");
	else {
		data = ((te->data.u.word[AI_WORD] >> AI_OFFS) & AI_MASK);
		for (i = 0; i < AI_BITS; i++)
			if (mask & (1 << i))
				off += sprintf(buf + off, "%d", ((data & (1 << i)) != 0));
			else
				off += sprintf(buf + off, "x");
	}

	return off;
}

int tpm_print_tcam_port_mask(unsigned int port_mask, char *buf)
{
	int off = 0;

	if (port_mask & 1)
		off += sprintf(buf + off, "--,");
	else
		off += sprintf(buf + off, "P ,");

	if (port_mask & 4)
		off += sprintf(buf + off, "--,");
	else
		off += sprintf(buf + off, "G0,");

	if (port_mask & 0x10)
		off += sprintf(buf + off, "--");
	else
		off += sprintf(buf + off, "G1");

	return off;
}

int tpm_print_sram_rinfo(struct tcam_entry *te, char *buf, unsigned int buf_len)
{
	unsigned int word, shift, rinfo_val, rinfo_mask;
	int off = 0;
	int i;

	word = RI_VALUE_OFFS / DWORD_LEN;
	shift = RI_VALUE_OFFS % DWORD_LEN;
	rinfo_val = ((te->sram.word[word] >> shift) & RI_MASK);

	word = RI_MASK_OFFS / DWORD_LEN;
	shift = RI_MASK_OFFS % DWORD_LEN;
	rinfo_mask = ((te->sram.word[word] >> shift) & RI_MASK);
	word++;
	rinfo_mask |= (te->sram.word[word] & 0x3FFFF) << 6;

	if (rinfo_val & 1)
		/* discard */
		off += sprintf(buf + off, "DIS ");

	/* L4 */
	if ((rinfo_val & 0x6) == 0x2)
		/* UDP */
		off += sprintf(buf + off, "UDP ");
	else if ((rinfo_val & 0x6) == 0x6)
		/* other */
		off += sprintf(buf + off, "L4-OTH ");
#if 0
    this code snippet is ifdefed till mask bits are supported in LSP
    to refrain from flooding with TCP
	else if ((rinfo_val & 0x6) == 0x0)
		/* TCP */
		off += sprintf(buf + off, "TCP ");
#endif

	/* L3 */
	if ((rinfo_val & 0x18) == 0x18)
		/* IPv4 not fragmented */
		off += sprintf(buf + off, "IPV4_NF ");
	else if ((rinfo_val & 0x18) == 0x8)
		/* IPv6 */
		off += sprintf(buf + off, "IPV6 ");
	else if ((rinfo_val & 0x18) == 0x10)
		/* IPv4 fragmented */
		off += sprintf(buf + off, "IPV4_FR ");
#if 0
    this code snippet is ifdefed till mask bits are supported in LSP
    to refrain from flooding with L3_OTH
	else if ((rinfo_val & 0x18) == 0x0)
		/* other */
		off += sprintf(buf + off, "L3_OTH ");
#endif

#if 0
	/* First fragmented */
	if (rinfo_val & 0x20)
		off += sprintf(buf + off, "ff ");
#endif

	/* Filtering method */
	if (rinfo_val & 0x1c0)
		off += sprintf(buf + off, "ERR FM-%d ", (rinfo_val & 0x1c0) >> 6);

	/* packet discard decision */
	if (rinfo_val & 0x200)
		off += sprintf(buf + off, "COL ");

#if 0
	/* TX port */
	if (rinfo_val & 0x3c00)
		off += sprintf(buf + off, "txp-%d ", (rinfo_val & 0x3c00) >> 10);
#endif

	/* Marvell Header */
	if (rinfo_val & 0x3c000)
		off += sprintf(buf + off, "MH_REG-%d ", (rinfo_val & 0x3c000) >> 14);

#if 0
	if (rinfo_val & 0x3c0000)
		off += sprintf(buf + off, "gen-%d ", (rinfo_val & 0x3c0000) >> 14);

	if (rinfo_val & 0xc00000)
		off += sprintf(buf + off, "prof-%d ", (rinfo_val & 0xc00000) >> 18);
#endif

	for (i = off; i < buf_len; i++)
		off += sprintf(buf + off, " ");

	return off;
}

int tpm_print_tcam(unsigned int print_index, struct tcam_entry *te, char *buf)
{
	unsigned int    *p_data = (unsigned int *)&te->data;
	unsigned int    *p_sram = (unsigned int *)&te->sram;
	unsigned int    *p_mask = (unsigned int *)&te->mask;
	unsigned int    value;
	unsigned int    mask;
	unsigned int    off = 0;
	int             i;
	unsigned int    port_val, port_mask, lookup_val, lookup_mask;
	unsigned int    bitmap_mask;
	unsigned int    shift;

	/* hw entry id */
	if (print_index)
		off += sprintf(buf + off, "[%4d]\n", te->ctrl.index);

	/* get LU value/mask */
	tcam_sw_get_lookup(te, &lookup_val, &lookup_mask);

	/* print the LU value */
	off += sprintf(buf + off, "    %1.1x ", lookup_val);

	/* print the port */
	tcam_sw_get_port(te, &port_val, &port_mask);
	off += tpm_print_tcam_port_mask(port_mask, buf + off);
	off += sprintf(buf+off, "  ");

	/* print tcam data bits */
	i = 0;
	while (i < TCAM_LEN - 1) {
		value = MV_BYTE_SWAP_32BIT(MV_32BIT_LE_FAST(p_data[i]));
		mask  = MV_BYTE_SWAP_32BIT(MV_32BIT_LE_FAST(p_mask[i]));

		for (shift = 28, bitmap_mask = 0xf0000000; bitmap_mask; shift -= 4) {
			if (bitmap_mask & mask)
				off += sprintf(buf + off, "%1.1x", (bitmap_mask & value) >> shift);
			else
				off += sprintf(buf + off, "-");
			bitmap_mask = bitmap_mask >> 4;
		}
		off += sprintf(buf + off, " ");
		i++;
	}

	/* print sram next LU */
	off += tpm_print_sram_next_lookup(te, buf + off);

	/* print sram next LU shift */
	off += tpm_print_sram_next_lookup_shift(te, buf + off);

	/* print sram next shift update */
	off += tpm_print_sram_shift_update(te, buf + off);

	/* print sram next RX queue */
	off += tpm_print_sram_rxq(te, buf + off);

	/* print entry name */
	if (!strncmp("empty", te->ctrl.text, 5))
		off += sprintf(buf + off, "                 ");
	else
		off += sprintf(buf + off, " %-16s", te->ctrl.text);
	off += sprintf(buf+off, " ");

	/* print sram aditional info (AI) */
	off += tpm_print_sram_ainfo(te, buf + off);

	/* print sram flow ID value */
	off += sprintf(buf + off, " %8.8x", p_sram[0]);

	off += sprintf(buf + off, "\n");

	/* print the LU mask */
	off += sprintf(buf + off, "    %1.1x ", lookup_mask);

	/* print tcam aditional info (AI) */
	off += tpm_print_tcam_ainfo(te, buf + off);
	off += sprintf(buf + off, "   ");

	/* print the tcam mask bits */
	i = 0;
	while (i < TCAM_LEN - 1) {
		mask  = MV_BYTE_SWAP_32BIT(MV_32BIT_LE_FAST(p_mask[i]));

		for (shift = 28, bitmap_mask = 0xf0000000; bitmap_mask; shift -= 4) {
			if (bitmap_mask & mask)
				off += sprintf(buf + off, "%1.1x", (bitmap_mask & mask) >> shift);
			else
				off += sprintf(buf + off, "-");
			bitmap_mask = bitmap_mask >> 4;
		}
		off += sprintf(buf + off, " ");
		i++;
	}
	off += sprintf(buf + off, " ");

	/* print sram result info */
	off += tpm_print_sram_rinfo(te, buf + off, 42);

	/* print sram flow ID mask */
	value = p_sram[1];
	for (bitmap_mask = 0x80; bitmap_mask; shift--) {
		if (bitmap_mask & value)
			off += sprintf(buf + off, "f");
		else
			off += sprintf(buf + off, "0");
		bitmap_mask = bitmap_mask >> 1;
	}

	off += sprintf(buf + off, "\n");

	return off;
}

void tpm_print_pnc_field_desc(void)
{
#ifdef CONFIG_MV_TPM_SYSFS_HELP
    printk("    ------------------------------ T C A M -------------------------  --------------- S R A M ----------------------------\n");
    printk("    +--- LU ID                                            Shift Update ---+    Entry name ---+    modL [3:0]   GEM [23:12]\n");
    printk("    | +--- Port/s                                  Next LU Shift reg ---+ |                  |    modM [7:4]   txp [27:24]\n");
    printk("    | |        +--- words value                             Next LU --+ | |  RxQ -+   +------+    modH [9:8]    +-- FlowID\n");
    printk("    | |        |                                                      | | |       |   |                +--AI    |\n");
    printk("    . [.,..,.] [......0 .......1 .......2 .......3 .......4 ......5]  . . [.....] [.] [..............] [.....]  [........]\n");
    printk("    . [.....]  [......0 .......1 .......2 .......3 .......4 ......5]  [.......................................] [........]\n");
    printk("    | |                                                            |  |                                        |\n");
    printk("    | +--- AI                                                      |  |                         FlowID mask ---+\n");
    printk("    +--- LU ID mask                                  words mask ---+  +--- SRAM RI\n");
#endif
}

int tpm_print_pnc(void)
{
	int i;
	struct tcam_entry te;
	char buff[1024];

	tpm_print_pnc_field_desc();
	for (i = 0; i < CONFIG_MV_PNC_TCAM_LINES; i++) {
		tcam_sw_clear(&te);
		tcam_hw_read(&te, i);
		if (te.ctrl.flags & TCAM_F_INV)
			continue;
		tpm_print_tcam(1, &te, buff);
		printk(buff);
	}

	return 0;
}
unsigned int tpm_tcam_hw_hits_hist_print(unsigned int   seq,
					 unsigned int   shift,
					 unsigned int   last_val,
					 unsigned int   new_val,
					 unsigned int   print_pnc)
{
	unsigned int off = 0;
	unsigned int pnc_new_entry  = (new_val >> shift) & 0x3FF;
	unsigned int pnc_last_entry = (last_val >> shift) & 0x3FF;
	char         buff[1024];

	off += sprintf(buff + off, "%d - %3.3d", seq, pnc_new_entry);
	if (pnc_new_entry != 0) {
		if (pnc_new_entry == pnc_last_entry)
			off += sprintf(buff + off, " *\n");
		else
			off += sprintf(buff + off, "\n");

		if (print_pnc) {
			struct tcam_entry te;

			tcam_sw_clear(&te);
			tcam_hw_read(&te, pnc_new_entry);
			if (!(te.ctrl.flags & TCAM_F_INV))
				off += tpm_print_tcam(0, &te, buff + off);
		}
		if (pnc_new_entry == pnc_last_entry) {
			printk(buff);
			return 0;
		}
	}
	off += sprintf(buff + off, "\n");
	printk(buff);

	return 0;
}


void tpm_tcam_hw_record(int port)
{
	MV_REG_WRITE(MV_PNC_HIT_SEQ0_REG, (port << 1) | 1);
}


int tpm_tcam_hw_hits(unsigned int print_pnc)
{
	unsigned int            new_val;
	static unsigned int     last_read[3] = {0, 0, 0};

	printk("seq hit recurring\n");
	printk("--- --- ---------\n");

	new_val = MV_REG_READ(MV_PNC_HIT_SEQ0_REG);
	tpm_tcam_hw_hits_hist_print(0, 10, last_read[0], new_val, print_pnc);
	tpm_tcam_hw_hits_hist_print(1, 20, last_read[0], new_val, print_pnc);
	last_read[0] = new_val;

	new_val = MV_REG_READ(MV_PNC_HIT_SEQ1_REG);
	tpm_tcam_hw_hits_hist_print(2,  0, last_read[1], new_val, print_pnc);
	tpm_tcam_hw_hits_hist_print(3, 10, last_read[1], new_val, print_pnc);
	tpm_tcam_hw_hits_hist_print(4, 20, last_read[1], new_val, print_pnc);
	last_read[1] = new_val;

	new_val = MV_REG_READ(MV_PNC_HIT_SEQ2_REG);
	tpm_tcam_hw_hits_hist_print(5,  0, last_read[2], new_val, print_pnc);
	tpm_tcam_hw_hits_hist_print(6, 10, last_read[2], new_val, print_pnc);
	tpm_tcam_hw_hits_hist_print(7, 20, last_read[2], new_val, print_pnc);
	last_read[2] = new_val;

	return 0;
}

int tpm_age_pnc_dump(void)
{
	unsigned int        tid, reg_val;
	char                buff[1024];
	struct tcam_entry   te;
	unsigned int        off;

	for (tid = 0; tid < CONFIG_MV_PNC_TCAM_LINES; tid++) {
		reg_val = mvPncAgingCntrRead(tid);

		if (!(reg_val & PNC_AGING_CNTR_MASK))
			continue;

		off = 0;
		off += sprintf(buff + off, "[%4d]: gr=%d - %10u", tid,
			       ((reg_val & PNC_AGING_GROUP_ALL_MASK) >> PNC_AGING_GROUP_OFFS),
			       ((reg_val & PNC_AGING_CNTR_MASK) >> PNC_AGING_CNTR_OFFS));

		if (reg_val & PNC_AGING_READ_LU_LOG_MASK)
			off += sprintf(buff + off, ", LU_READ");

		if (reg_val & PNC_AGING_READ_MU_LOG_MASK)
			off += sprintf(buff + off, ", MU_READ");

		if (reg_val & PNC_AGING_SKIP_LU_SCAN_MASK)
			off += sprintf(buff + off, ", LU_SKIP");

		if (reg_val & PNC_AGING_SKIP_MU_SCAN_MASK)
			off += sprintf(buff + off, ", MU_SKIP");

		off += sprintf(buff + off, "\n");
		printk(buff);

		tcam_sw_clear(&te);
		tcam_hw_read(&te, tid);

		if (!(te.ctrl.flags & TCAM_F_INV)) {
			tpm_print_tcam(0, &te, buff);
			printk(buff);
		}
	}

	return 0;
}

int tpm_age_pnc_dump_live(void)
{
	unsigned int        tid, reg_val;
	unsigned int        off;
	struct tcam_entry   te;
	char                buff[1024];
	unsigned int        tpm_age_first[CONFIG_MV_PNC_TCAM_LINES];

	for (tid = 0; tid < CONFIG_MV_PNC_TCAM_LINES; tid++)
		tpm_age_first[tid] = mvPncAgingCntrRead(tid);

	msleep(1000);

	for (tid = 0; tid < CONFIG_MV_PNC_TCAM_LINES; tid++) {
		reg_val = mvPncAgingCntrRead(tid);

		if (!(reg_val & PNC_AGING_CNTR_MASK))
			continue;

		if (tpm_age_first[tid] == reg_val)
			continue;

		off = 0;
		off += sprintf(buff + off, "[%4d]: gr=%d - %10u", tid,
			       ((reg_val & PNC_AGING_GROUP_ALL_MASK) >> PNC_AGING_GROUP_OFFS),
			       ((reg_val & PNC_AGING_CNTR_MASK) >> PNC_AGING_CNTR_OFFS));

		if (reg_val & PNC_AGING_READ_LU_LOG_MASK)
			off += sprintf(buff + off, ", LU_READ");

		if (reg_val & PNC_AGING_READ_MU_LOG_MASK)
			off += sprintf(buff + off, ", MU_READ");

		if (reg_val & PNC_AGING_SKIP_LU_SCAN_MASK)
			off += sprintf(buff + off, ", LU_SKIP");

		if (reg_val & PNC_AGING_SKIP_MU_SCAN_MASK)
			off += sprintf(buff + off, ", MU_SKIP");

		off += sprintf(buff + off, "\n");
		printk(buff);

		tcam_sw_clear(&te);
		tcam_hw_read(&te, tid);

		if (!(te.ctrl.flags & TCAM_F_INV)) {
			tpm_print_tcam(0, &te, buff);
			printk(buff);
		}
	}
	return 0;
}

void tpm_print_mc_vlan_cfg_head(void)
{
	print_horizontal_line(65);
	printk("=    MC VLAN      MC AI       Port        Mode      UNI MC VLAN =\n");
	print_horizontal_line(65);
}

void tpm_print_mc_vlan_cfg(tpm_mc_vid_cfg_t *mc_vid_cfg)
{
	int32_t src_port;
	char *src_port_str = "";
	char *mc_mode_str  = "";

	for (src_port = 0; src_port < TPM_MAX_NUM_UNI_PORTS; src_port++) {
		src_port_str = tpm_db_src_port_str[mc_vid_cfg->mc_vid_port_vids[src_port].tpm_src_port];
		mc_mode_str  = tpm_db_mc_uni_mode_str[mc_vid_cfg->mc_vid_port_vids[src_port].mc_uni_port_mode];
		printk("=  %11s | %8s |   %4s  |  %11s |", "", "", src_port_str, mc_mode_str);

		if (TPM_MC_UNI_MODE_TRANSLATE == mc_vid_cfg->mc_vid_port_vids[src_port].mc_uni_port_mode)
			printk("  %4d %4s=\n", mc_vid_cfg->mc_vid_port_vids[src_port].uni_port_vid, "");
		else
			printk("%11s=\n", "");
	}
	return;
}

void tpm_print_mc_vlan_cfg_all(void)
{
	uint32_t i, rcode;
	uint32_t ai_bit;
	uint32_t first_rule = 1;

	tpm_print_mc_vlan_cfg_head();
	for (i = 0; i < TPM_MC_VID_NUM_MAX; i++) {
		if (true == tpm_db.mc_vid_port_cfg[i].valid) {
			if (!first_rule)
				printk("=--------------|----------|----------|--------------|-----------=\n");

			rcode = tpm_db_mc_vlan_get_ai_bit(tpm_db.mc_vid_port_cfg[i].mc_vid, &ai_bit);
			if (rcode != TPM_DB_OK) {
				printk("get DB failed: %d\n", rcode);
				return;
			}
			printk("=     %4d     |    %2x    |%10s|%14s|%11s=\n",
				tpm_db.mc_vid_port_cfg[i].mc_vid, ai_bit, "", "", "");

			tpm_print_mc_vlan_cfg(&tpm_db.mc_vid_port_cfg[i]);
			first_rule = 0;
		}
	}
	print_horizontal_line(65);

	return;
}

void tpm_print_section_free_szie(tpm_api_type_t api_type)
{
	int32_t section_free_szie = 0;
	tpm_error_code_t rcode;

	rcode = tpm_get_section_free_size(api_type, &section_free_szie);
	if (rcode == TPM_OK)
		printk("Section free size: %4d\n", section_free_szie);

	return;
}

void tpm_print_gpon_omci_channel(void)
{
	uint32_t is_valid;
	tpm_gem_port_key_t gem_port = 0;
	uint32_t cpu_rx_queue;
	uint32_t cpu_tx_queue = 0;
	tpm_trg_port_type_t tcont_num;
	tpm_error_code_t rcode;

	rcode = tpm_omci_get_channel(&is_valid, &gem_port, &cpu_rx_queue, &tcont_num, &cpu_tx_queue);
	if (rcode == TPM_OK)
		printk("Valid = %d GEM = %d CPU_RX queue = %d CPU_TX queue = %d Tcont = 0x%4.4x\n",
		is_valid, gem_port, cpu_rx_queue, cpu_tx_queue, tcont_num);

	return;
}

void tpm_print_epon_oam_channel(void)
{
	uint32_t is_valid;
	uint32_t cpu_rx_queue;
	tpm_trg_port_type_t llid_num;
	tpm_error_code_t rcode;

	rcode = tpm_oam_epon_get_channel(&is_valid, &cpu_rx_queue, &llid_num);
	if (rcode == TPM_OK)
		printk("Valid = %d RX queue = %d LLID = 0x%4.4x\n", is_valid, cpu_rx_queue, llid_num);

	return;
}

void tpm_print_busy_apis(void)
{
	tpm_api_type_t i;
	uint32_t temp_api_busy_rulenums[TPM_MAX_PARALLEL_API_CALLS];
	uint32_t num_busy, j;

	for (i=0;i<TPM_MAX_API_TYPES;i++) {
		memset(&temp_api_busy_rulenums[0], 0, sizeof(temp_api_busy_rulenums));
		tpm_db_get_api_all_busy(i, &num_busy, &temp_api_busy_rulenums[0]);
		if (num_busy > 0) {
			printk("%s:\t",api_type_to_str(i));
			for (j=0;j<num_busy;j++) {
				printk("%d\t",temp_api_busy_rulenums[j]);
			}
			printk("\n");
		}
	}
}

void tpm_print_fc(unsigned int print_only)
{
	tpm_fc_info_t   *p_fc_inf = NULL;

	if (print_only == 0) {
		printk(KERN_INFO "Resetting Flow Control engine statistics\n");
		tpm_fc_clear_stat();
	} else if (print_only != 1) {
		printk(KERN_INFO "UNEXPECTED PARAMETER (%d)\n", print_only);
		return;
	}

	tpm_fc_get_info(&p_fc_inf);
	if (p_fc_inf == NULL) {
		printk(KERN_INFO "UNEXPECTED ERROR\n");
		return;
	}

	printk(KERN_INFO "---------------------------------------------------------------------------------\n");
	printk(KERN_INFO "FC is %s\n", (tpm_fc_is_running() == MV_TRUE) ? "ENABLED" : "DISABLED");
	printk(KERN_INFO "The Flow Control engine settings:\n");
	printk(KERN_INFO "\t Threshold: High = %ld, Low = %ld\n",
	       (long)p_fc_inf->cfg.thresh_high,
	       (long)p_fc_inf->cfg.thresh_low);
	printk(KERN_INFO "\t Port %d, Target port = %d, TX port = %d, TX Queue = %d\n",
	       p_fc_inf->cfg.port,
	       p_fc_inf->cfg.tgt_port,
	       p_fc_inf->cfg.tx_port,
	       p_fc_inf->cfg.tx_queue);
	printk(KERN_INFO "\t Queue status is checked every %lld ns\n", ktime_to_ns(p_fc_inf->cfg.hrt_hit_time));

#ifdef TPM_FC_DEBUG
	printk(KERN_INFO "\n");
	printk(KERN_INFO "Latched statistics for oneshot counter configured to %d hits\n", p_fc_inf->cfg.oneshot_count);
	printk(KERN_INFO "\t Total timer hits = %ld, Total late hits = %ld, timer wraprarounds = %ld \n",
		   (long)p_fc_inf->oneshot_stat.hrt_hits_num,
		   (long)p_fc_inf->oneshot_stat.hrt_lost_num,
		   (long)p_fc_inf->oneshot_stat.hrt_wraparound);
	printk(KERN_INFO "\t Highest \"late than expected\" period = %lld ns\n", p_fc_inf->oneshot_stat.hrt_lost_max_ns);
	printk(KERN_INFO "\t Number of late hits of \"higher than expected\" period distribution in %%:\n");
	printk(KERN_INFO "\t >200%%    - %lld\n", p_fc_inf->oneshot_stat.hrt_lost_200_up);
	printk(KERN_INFO "\t 150-200%% - %lld\n", p_fc_inf->oneshot_stat.hrt_lost_150_200);
	printk(KERN_INFO "\t 100-150%% - %lld\n", p_fc_inf->oneshot_stat.hrt_lost_100_150);
	printk(KERN_INFO "\t 50-100%%  - %lld\n", p_fc_inf->oneshot_stat.hrt_lost_50_100);
	printk(KERN_INFO "\t 25-50%%   - %lld\n", p_fc_inf->oneshot_stat.hrt_lost_25_50);
	printk(KERN_INFO "\t <25%%     - %lld\n", p_fc_inf->oneshot_stat.hrt_lost_25_less);
	printk(KERN_INFO "\n");

	printk(KERN_INFO "Full statistics since start/reset\n");
	printk(KERN_INFO "\t Total timer hits = %ld, Total late hits = %ld, timer wraprarounds = %ld \n",
	       (long)p_fc_inf->stat.hrt_hits_num,
	       (long)p_fc_inf->stat.hrt_lost_num,
	       (long)p_fc_inf->stat.hrt_wraparound);
	printk(KERN_INFO "\t Highest \"late than expected\" period = %lld ns\n", p_fc_inf->stat.hrt_lost_max_ns);
	printk(KERN_INFO "\t Number of late hits of \"higher than expected\" period distribution in %%:\n");
	printk(KERN_INFO "\t >200%%    - %lld\n", p_fc_inf->stat.hrt_lost_200_up);
	printk(KERN_INFO "\t 150-200%% - %lld\n", p_fc_inf->stat.hrt_lost_150_200);
	printk(KERN_INFO "\t 100-150%% - %lld\n", p_fc_inf->stat.hrt_lost_100_150);
	printk(KERN_INFO "\t 50-100%%  - %lld\n", p_fc_inf->stat.hrt_lost_50_100);
	printk(KERN_INFO "\t 25-50%%   - %lld\n", p_fc_inf->stat.hrt_lost_25_50);
	printk(KERN_INFO "\t <25%%     - %lld\n", p_fc_inf->stat.hrt_lost_25_less);
	printk(KERN_INFO "---------------------------------------------------------------------------------\n");
#endif
}

void tpm_print_mac_learn_entry_count(void)
{
	uint32_t entry_count = 0;
	tpm_error_code_t rcode;

	rcode = tpm_mac_learn_entry_num_get(&entry_count);
	if (rcode == TPM_OK)
		printk("MAC learn entry count: %d\n", entry_count);

	return;
}


