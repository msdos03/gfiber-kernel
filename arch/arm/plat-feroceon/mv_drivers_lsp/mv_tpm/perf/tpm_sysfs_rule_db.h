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

/******************************************************************************
* tpm_usr_tpmsysfs_if.h
*
* DESCRIPTION:
*               Traffic Processor Manager = TPM
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   Zeev
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               $Revision: 1.1.1.1 $
*
*
*******************************************************************************/
#ifndef _TPM_USR_TPM_SYSFS_RULE_DB_H_
#define _TPM_USR_TPM_SYSFS_RULE_DB_H_

extern void sfs_tpm_cfg_set_frwd_rule                   (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_vlan_rule                   (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_proto_rule              (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_mh_rule                 (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_vlan_rule               (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_dscp_rule               (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_ipv4_addr_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_ipv4_port_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_ipv6_addr_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_ipv6_port_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_mac_addr_rule           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_mod_pppoe_rule              (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_pkt_mod_eng_entry_add       (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_pkt_mod_eng_entry_show      (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_pkt_mod_eng_entry_del       (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_pkt_mod_eng_purge           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l2_key_ethertype_rule       (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l2_key_gemport_rule         (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l2_key_mac_addr_rule        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l2_key_pppoe_rule           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l2_key_vlan_rule            (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l3_key_ethertype_rule       (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_l3_key_pppoe_rule           (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv4_key_addr_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv4_key_port_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv4_key_protocol_rule      (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv4_key_dscp_rule          (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_gen_key_sip_rule       (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_ctc_cm_key_rule        (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_ipv6_gen_key_dscp_rule      (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_key_rule_delete             (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_rule_table_display          (const char *buf, size_t len);

extern void sfs_tpm_cfg_set_delete_entry_l2_rule_table  (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_l3_rule_table  (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_ipv4_rule_table(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_ipv6_rule_table(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_ipv6_dip_rule_table(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_ipv6_gen_rule_table(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_ipv6_l4_rule_table (const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_frwd_rule_table(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_vlan_rule_table(const char *buf, size_t len);
extern void sfs_tpm_cfg_set_delete_entry_mod_rule_table (const char *buf, size_t len);



#if 0
extern int sfs_show_l2_rule_table                     (char *buf);
extern int sfs_show_l3_rule_table                     (char *buf);
extern int sfs_show_ipv4_rule_table                   (char *buf);
extern int sfs_show_ipv6_rule_table                   (char *buf);
extern int sfs_show_frwd_rule_table                   (char *buf);
extern int sfs_show_vlan_rule_table                   (char *buf);
extern int sfs_show_mod_rule_table                    (char *buf);
#endif

#endif  /* _sfs_tpm_cfg_setTPM_USR_TPM_SYSFS_RULE_DB_H_ */
