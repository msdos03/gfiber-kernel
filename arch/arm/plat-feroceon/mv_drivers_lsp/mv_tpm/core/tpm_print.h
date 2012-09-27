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
* tpm_print.h
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
*               Revision: 1.1.1.1
*
*
*******************************************************************************/

#ifndef _TPM_PRINT_H_
#define _TPM_PRINT_H_

/********************************************************************************/
/*                                Print Utils                                   */
/********************************************************************************/
#define DB_MAC_STR_LEN      25
#define DB_IPV4_STR_LEN     17
#define DB_IPV6_STR_LEN     40
#define DB_IPV6_ADDR_LEN    16
#define PRINT_PNC_ENTRY         1
#define DONT_PRINT_PNC_ENTRY    0

#define BuildEnumString(param) { param, #param"\0" }

typedef struct {
	int enumPar;
	char enumString[128];
} db_enum_string_t;

char *db_mac_to_str(uint8_t *addr, char *str);
char *db_ipv4_to_str(uint8_t *ipaddr, char *str);
char *db_ipv6_to_str(uint8_t *ipaddr, char *str);

/* TODO - following functions should use db functions, instead of accessing DB directly */
void tpm_print_etherports(void);
void tpm_print_tx_modules(void);
void tpm_print_rx_modules(void);
void tpm_print_gmac_config(void);
void tpm_print_gmac_func(void);
void tpm_print_igmp(void);
void tpm_print_misc(void);
void tpm_print_owners(void);
void tpm_print_vlan_etype(void);
void tpm_print_mac_key(tpm_mac_key_t *mac_key);
void tpm_print_cpu_lpbk_entry(void);
void tpm_print_tcam_lu_entry(uint32_t owner_id, uint32_t api_group, uint32_t lu_num, uint32_t lu_reset);
void tpm_print_pnc_all_hit_counters(uint32_t owner_id, tpm_api_type_t api_type, uint32_t high_thresh_pkts,
				    uint8_t counters_reset, uint16_t valid_counters,
				    tpm_api_entry_count_t *count_array);

void tpm_print_vlan_key(tpm_vlan_key_t *vlan_key);
void tpm_print_ipv4_key(tpm_ipv4_acl_key_t *ipv4_key, tpm_ipv4_add_key_t *ipv4_add_key);
void tpm_print_l2_key(tpm_l2_acl_key_t *l2_key);
void tpm_print_l3_key(tpm_l3_type_key_t *l3_key);

void tpm_print_valid_api_sections(void);
void tpm_print_full_api_section(tpm_api_sections_t api_section);
void tpm_print_pnc_shadow_range(uint32_t valid_only, uint32_t start, uint32_t end);
void tpm_print_valid_pnc_ranges(void);
void tpm_print_init_tables(void);
void tpm_print_api_dump_all(void);
int tpm_print_pnc(void);
int  tpm_tcam_hw_hits(unsigned int print_pnc);
void tpm_tcam_hw_record(int port);
int  tpm_age_pnc_dump(void);
int  tpm_age_pnc_dump_live(void);
void tpm_print_pnc_field_desc(void);
void tpm_print_mc_vlan_cfg_all(void);
void tpm_print_section_free_szie(tpm_api_type_t api_type);
void tpm_print_gpon_omci_channel(void);
void tpm_print_epon_oam_channel(void);
void tpm_print_busy_apis(void);
void tpm_print_fc(unsigned int print_only);
void tpm_init_eth_cmplx_setup_error_print(uint32_t hwEthCmplx, bool sysfs_call);
void tpm_print_mac_learn_entry_count(void);


#endif /* _TPM_PRINT_H_ */
