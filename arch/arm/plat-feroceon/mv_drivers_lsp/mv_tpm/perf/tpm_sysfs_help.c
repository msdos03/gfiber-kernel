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
#ifdef CONFIG_MV_TPM_SYSFS_HELP

#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_sysfs_help.h"



/**********************************************************************/
/* ===================================================================*/
/*             TPM SYSFS CFG HELP FUNCTION                            */
/* ================================================================== */


/*******************************************************************************
**
**  sfs_tpm_cfg_index
**
**  DESCRIPTION: The function print cfg help
**
**  PARAMETERS:  char* buf
**
**  OUTPUTS:     char* buf
**
**  RETURNS:     message length
**
*******************************************************************************/
int sfs_tpm_cfg_index(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "==================================================================================================\n");
    off += sprintf(buf+off, "cat help_tpm_cfg_index                        - show this help\n");
    off += sprintf(buf+off, "cat help_frwd_rule_cfg                        - show frwd rule help\n");
    off += sprintf(buf+off, "cat help_igmp_cfg                             - show IGMP configuration help\n");
    off += sprintf(buf+off, "cat help_ipv4_key_cfg                         - show IPV4 key ACL help\n");
    off += sprintf(buf+off, "cat help_ipv4_rule_add                        - show add IPV4 rule help\n");

    off += sprintf(buf+off, "cat help_ipv6_dip_key_cfg                     - show IPV6 DIP key ACL help\n");
    off += sprintf(buf+off, "cat help_ipv6_ctc_cm_key_cfg                  - show IPV6 CTC key ACL help\n");
    off += sprintf(buf+off, "cat help_ipv6_dip_rule_add                    - show add IPV6 DIP rule help\n");
    off += sprintf(buf+off, "cat help_ipv6_l4_ports_rule_add               - show add IPV6 L4 ports rule help\n");
    off += sprintf(buf+off, "cat help_ipv6_nh_rule_add                     - show add IPV6 NH rule help\n");
    off += sprintf(buf+off, "cat help_ipv6_gen_key_cfg                     - show IPV6 key ACL help\n");
    off += sprintf(buf+off, "cat help_ipv6_gen_rule_add                    - show add IPV6 rule help\n");

    off += sprintf(buf+off, "cat help_ctc_cm_rule_add                      - show add CTC CnM rule help\n");

    off += sprintf(buf+off, "cat help_key_rule_delete                      - show key delete from DB table (mod/frwd/vlan/l2/l3/ipv4) help\n");
    off += sprintf(buf+off, "cat help_l2_key_cfg                           - show L2 key ACL help\n");
    off += sprintf(buf+off, "cat help_l2_rule_add                          - show add L2 rule help\n");
    off += sprintf(buf+off, "cat help_l3_key_cfg                           - show L3 key ACL help\n");
    off += sprintf(buf+off, "cat help_l3_rule_add                          - show add L3 rule help\n");
    off += sprintf(buf+off, "cat help_mc_ipvx_stream_add                   - show add multicast IPV4 stream help\n");
    off += sprintf(buf+off, "cat help_mib_reset                            - show MIB reset help\n");
    off += sprintf(buf+off, "cat help_mod_rule_cfg                         - show mod rule help\n");
    off += sprintf(buf+off, "cat help_no_mc_stream_add                     - show delete multicast stream help\n");
    off += sprintf(buf+off, "cat help_no_oam_omci_channel                  - show delete OAM/OMCI channel help\n");
    off += sprintf(buf+off, "cat help_no_rule_add                          - show delete L2/L3/IPV4/IPV6_xxx/DSCP IPV4/DSCP IPV6/CTC CnM rule help\n");
    off += sprintf(buf+off, "cat help_oam_omci_channel                     - show OAM/OMCI channel configuration help\n");
    off += sprintf(buf+off, "cat help_pkt_mod_add                          - show packet modification add help\n");
    off += sprintf(buf+off, "cat help_pkt_mod_get_del_purge                - show packet modification display, delete. purge help\n");
    off += sprintf(buf+off, "cat help_rate_limit                           - show queue rate limit/scheduling, entity rate limit help\n");
    off += sprintf(buf+off, "cat help_rule_table_display                   - show rule table display help\n");
    off += sprintf(buf+off, "cat help_vlan_rule_cfg                        - show vlan rule help\n");
    off += sprintf(buf+off, "cat help_delete_entry_rule_table              - show delete rule entry help\n");
    off += sprintf(buf+off, "cat help_tpm_self_check                       - show TPM check help\n");
    off += sprintf(buf+off, "cat help_mac_learn_rule_add                   - show add MAC learn rule help\n");
    off += sprintf(buf+off, "cat help_mac_learn_def_act_set                - show set action of mac learn default rule help\n");

    off += sprintf(buf+off, "==================================================================================================\n");
    return(off);
}

/*******************************************************************************
**
**  sfs_help_frwd_rule_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_frwd_rule_cfg(char* buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [rule_name] [port_bitmap] [queue] [gem_port]  > frwd_rule_set    - set a forwarding rule\n");
	off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
	off += sprintf(buf+off, "\tport_bitmap         (hex) see below\n");
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_WAN           %#.5X     TPM_TRG_TCONT_0      %#.5X\n",TPM_TRG_PORT_WAN, TPM_TRG_TCONT_0);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_1            %#.5X     TPM_TRG_TCONT_2      %#.5X\n",TPM_TRG_TCONT_1, TPM_TRG_TCONT_2);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_3            %#.5X     TPM_TRG_TCONT_4      %#.5X\n",TPM_TRG_TCONT_3, TPM_TRG_TCONT_4);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_5            %#.5X     TPM_TRG_TCONT_6      %#.5X\n",TPM_TRG_TCONT_5, TPM_TRG_TCONT_6);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_7            %#.5X     TPM_TRG_LLID_0       %#.5X\n",TPM_TRG_TCONT_7, TPM_TRG_LLID_0);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_1             %#.5X     TPM_TRG_LLID_2       %#.5X\n",TPM_TRG_LLID_1, TPM_TRG_LLID_2);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_3             %#.5X     TPM_TRG_LLID_4       %#.5X\n",TPM_TRG_LLID_3, TPM_TRG_LLID_4);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_5             %#.5X     TPM_TRG_LLID_6       %#.5X\n",TPM_TRG_LLID_5, TPM_TRG_LLID_6);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_7             %#.5X     TPM_TRG_UNI_0        %#.5X\n",TPM_TRG_LLID_7, TPM_TRG_UNI_0);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_1              %#.5X     TPM_TRG_UNI_2        %#.5X\n",TPM_TRG_UNI_1, TPM_TRG_UNI_2);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_3              %#.5X     TPM_TRG_UNI_4        %#.5X\n",TPM_TRG_UNI_3, TPM_TRG_UNI_4);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_5              %#.5X     TPM_TRG_UNI_6        %#.5X\n",TPM_TRG_UNI_5, TPM_TRG_UNI_6);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_7              %#.5X     TPM_TRG_UNI_VIRT     %#.5X\n",TPM_TRG_UNI_7, TPM_TRG_UNI_VIRT);
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_CPU           %#.5X     TPM_TRG_PORT_UNI_ANY %#.5X\n",TPM_TRG_PORT_CPU, TPM_TRG_PORT_UNI_ANY);
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_UNI_CPU_LOOP  %#.5X\n", TPM_TRG_PORT_UNI_CPU_LOOP);
	off += sprintf(buf+off, "\tqueue               (dec)Queue number\n");
	off += sprintf(buf+off, "\tgem_port            (dec)GEM port\n");

	return(off);
}

/*******************************************************************************
**
**  sfs_help_vlan_rule_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_vlan_rule_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [rule_name] [tpid] [tpid_mask] [vid] [vid_mask] [cfi] [cfi_mask] [pbit] [pbit_mask] > vlan_rule_set        - set a VLAN rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\ttpid                (hex)TPID VLAN Ethertype\n");
    off += sprintf(buf+off, "\ttpid_mask            (hex)TPID mask\n");
    off += sprintf(buf+off, "\tvid                 (dec)VID\n");
    off += sprintf(buf+off, "\tvid_mask            (hex)VID mask\n");
    off += sprintf(buf+off, "\tcfi                 (dec)CFI\n");
    off += sprintf(buf+off, "\tcfi_mask            (hex)CFI mask\n");
    off += sprintf(buf+off, "\tpbit                (dec)P-bits\n");
    off += sprintf(buf+off, "\tpbit_mask           (hex)P-bits mask\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_mod_rule_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_mod_rule_cfg(char* buf)
{
    int off = 0;
#if 0
    off += sprintf(buf+off, "echo [rule_name] [proto]                                    > mod_proto_rule_set   - set protocol part of mod rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tprotocol_type       (hex)Protocol\n");
#endif
    off += sprintf(buf+off, "echo [rule_name] [mh2b]                                     > mod_mh_rule_set   - set protocol part of mod rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\t                     'dbg_entry' is reserved for PMT debugging\n");
    off += sprintf(buf+off, "\tmh2b                (hex)2 Bytes Marvell Header\n");

    off += sprintf(buf+off, "\necho [rule_name]   [operation] [vlan_rule_1] [vlan_rule_2] > mod_vlan_rule_set    - set VLAN part of mod rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tvlan_rule_1         (str)VLAN rule (Use vlan_empty for absent rule)\n");
    off += sprintf(buf+off, "\tvlan_rule_2         (str)VLAN rule (Use vlan_empty for absent rule)\n");
    off += sprintf(buf+off, "\toperation           (str)VLAN operation\n");
    off += sprintf(buf+off, "\t\tnone      ext_tag_mod  ext_tag_del  ext_tag_ins  ext_tag_mod_ins  ins_2tag\n");
    off += sprintf(buf+off, "\t\tmod_2tag  swap_tags    del_2tags    int_tag_mod  split_mod_pbit   ext_tag_del_int_mod\n");
#if 0
    off += sprintf(buf+off, "\necho [rule_name] [dscp] [dscpmask]                         > mod_dscp_rule_set      - set dscp part of mod rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tdscp                (dec)DSCP value\n");
    off += sprintf(buf+off, "\tdscp_mask           (hex)DSCP mask\n");
#endif
    off += sprintf(buf+off, "\necho [rule_name] [session] [protocol_type]                 > mod_pppoe_rule_set     - set PPPoE part of mod rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tsession             (dec)Session number\n");
    off += sprintf(buf+off, "\tprotocol_type       (hex)Protocol type\n");

    off += sprintf(buf+off, "\necho [rule_name] [src_port] [dst_port]                     > mod_ipv4_port_rule_set - set IPV4 port part of mod rule\n");
    off += sprintf(buf+off, "echo [rule_name] [src_port] [dst_port]                       > mod_ipv6_port_rule_set - set IPV6 port part of mod rule\n");
    off += sprintf(buf+off, "\trule_name           (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tsrc_port            (dec)Source port\n");
    off += sprintf(buf+off, "\tdst_port            (dec)Destination port\n");

    off += sprintf(buf+off, "\necho [rule_name] [srcip] [srcip_mask] [dstip] [dstip_mask] > mod_ipv6_addr_rule_set - set IPV6 address part of mod rule\n");
    off += sprintf(buf+off, "echo [rule_name] [srcip] [srcip_mask] [dstip] [dstip_mask] > mod_ipv4_addr_rule_set - set IPV4 address part of mod rule\n");
    off += sprintf(buf+off, "echo [rule_name] [SA] [SA_mask] [DA] [DA_mask]             > mod_mac_addr_rule_set  - set MAC address part of mod rule\n");
    off += sprintf(buf+off, "\trule_name            (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tMAC  address/mask    aa:bb:cc:dd:ee:ff  where each part is hexadecimal in range 0..FF\n");
    off += sprintf(buf+off, "\tIPV4 address/mask    w.x.y.z  where each part is decimal value in range 0..255\n");
    off += sprintf(buf+off, "\tIPV6 address/mask    aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh  where each part is hexadecimal in range 0..FFFF\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_l2_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_l2_key_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [rule_name] [ether_type]                     > l2_key_ethertype_rule_set  - set ethertype part of L2 ACL key\n");
    off += sprintf(buf+off, "\trule_name         (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tether_type        (hex)Ethernet type\n");

    off += sprintf(buf+off, "\necho [rule_name] [gem_port]                     > l2_key_gemport_rule_set    - set gemport part of L2 ACL key\n");
    off += sprintf(buf+off, "\trule_name         (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tgem_port          (dec)Ethernet type\n");

    off += sprintf(buf+off, "\necho [rule_name] [SA] [SA_mask] [DA] [DA_mask]  > l2_key_mac_addr_rule_set   - set MAC address part of L2 ACL rule\n");
    off += sprintf(buf+off, "\trule_name         (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tMAC address       aa:bb:cc:dd:ee:ff  where each part is hexadecimal in range 0..FF\n");

    off += sprintf(buf+off, "\necho [rule_name] [vlan_rule_1] [vlan_rule_2]    > l2_key_vlan_rule_set       - set VLAN part of L2 ACL rule\n");
    off += sprintf(buf+off, "\trule_name         (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tvlan_rule_1       (str)VLAN rule name (Use vlan_empty for absent rule)\n");
    off += sprintf(buf+off, "\tvlan_rule_2       (str)VLAN rule name (Use vlan_empty for absent rule)\n");

    off += sprintf(buf+off, "\necho [rule_name] [session] [protocol_type]      > l2_key_pppoe_rule_set      - set PPPoE part of L2 ACL rule\n");
    off += sprintf(buf+off, "\trule_name         (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tsession           (dec)Session number\n");
    off += sprintf(buf+off, "\tprotocol_type     (hex)Protocol type\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_l3_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_l3_key_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [rule_name] [ether_type]              > l3_key_ethertype_rule_set  - set ethertype part of L3 ACL key\n");
    off += sprintf(buf+off, "\trule_name       (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tether_type      (hex)Ethernet type\n");

    off += sprintf(buf+off, "echo [rule name] [session] [protocol_type] > l3_key_pppoe_rule_set      - set PPPoE part of L3 ACL rule\n");
    off += sprintf(buf+off, "\trule_name       (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tsession         (dec)Session number\n");
    off += sprintf(buf+off, "\tprotocol_type   (hex)Protocol type\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv4_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv4_key_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [rule_name] [proto]                           > ipv4_key_proto_rule_set - set protocol part of IPV4 ACL rule\n");
    off += sprintf(buf+off, "\trule_name        (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tproto            (dec)Protocol 0-255\n");

    off += sprintf(buf+off, "\necho [rule_name] [dscp] [dscpmask]               > ipv4_key_dscp_rule_set  - set dscp part of IPV4 ACL rule\n");
    off += sprintf(buf+off, "\trule_name      (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tdscp            (dec)DSCP value 0-63\n");
    off += sprintf(buf+off, "\tdscp_mask       (hex)DSCP mask  0x0-0xFF\n");

    off += sprintf(buf+off, "\necho [rule_name] [src_port] [dst_port]           > ipv4_key_port_rule_set  - set IPV4 port part of IPV4 ACL rule\n");
    off += sprintf(buf+off, "\trule_name       (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tsrc_port        (dec)Source port\n");
    off += sprintf(buf+off, "\tdst_port        (dec)Destination port\n");

    off += sprintf(buf+off, "\necho [rule_name] [srcip] [srcip_mask] [dstip] [dstip_mask] > ipv4_key_addr_rule_set  - set IPV4 address part of IPV4 ACL rule\n");
    off += sprintf(buf+off, "\trule_name       (max 16 chars)Rule name\n");
    off += sprintf(buf+off, "\tIPV4 address     w.x.y.z  where each part is decimal value in range 0..255\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_mtu_cfg - see
**
*******************************************************************************/
int sfs_help_mtu_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [enable]                                    > enable_mtu  - enable mtu checking\n");
    off += sprintf(buf+off, "\tenable                      (dec)0 disable, 1 enable\n");
    off += sprintf(buf+off, "echo [ipv4_or_ipv6] [direction] [mtu_value]      > set_mtu  - set mtu\n");
    off += sprintf(buf+off, "\tipv4_or_ipv6                (dec)0 for IPv4, 1 for IPv6\n");
    off += sprintf(buf+off, "\tdirection                   (dec)0 for DS, 1 for US\n");
    off += sprintf(buf+off, "\tmtu_value                   (dec)mtu value\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [ipv4_or_ipv6] [direction] [pppoe_mtu_value] > set_pppoe_mtu  - set pppoe mtu\n");
    off += sprintf(buf+off, "\tipv4_or_ipv6                (dec)0 for IPv4, 1 for IPv6\n");
    off += sprintf(buf+off, "\tdirection                   (dec)0 for DS, 1 for US\n");
    off += sprintf(buf+off, "\tpppoe_mtu_value             (dec)pppoe mtu value\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_gen_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_gen_key_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [key_name] [DSCP] [DSCP Mask] > ipv6_gen_key_dscp_rule_set  - set dscp key\n");
    off += sprintf(buf+off, "\tkey_name           (max 16 chars)Key name\n");
    off += sprintf(buf+off, "\tDSCP               (dec)DSCP\n");
    off += sprintf(buf+off, "\tDSCP Mask          (hex)DSCP Mask\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [Key_name] [srcip] [srcip_mask] > ipv6_gen_key_sip_rule_set      - set sip key\n");
    off += sprintf(buf+off, "\tKey_name           (max 16 chars)Key name\n");
    off += sprintf(buf+off, "\tIPV6 address/mask  aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh  where each part is hexadecimal in range 0..FFFF\n");

    return(off);
}
/*******************************************************************************
**
**  sfs_help_ipv6_dip_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_dip_key_cfg(char* buf)
{
    int off = 0;
    off += sprintf(buf+off, "echo [Key_name] [dip] [dip_mask] > ipv6_dip_key_rule_set      - set dip key\n");
    off += sprintf(buf+off, "\tKey_name                (max 16 chars)Key name\n");
    off += sprintf(buf+off, "\tIPV6 dip address        aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh  where each part is hexadecimal in range 0..FFFF\n");
    off += sprintf(buf+off, "\tIPV6 dip address mask   aaaa:bbbb:cccc:dddd:eeee:ffff:gggg:hhhh  where each part is hexadecimal in range 0..FFFF\n");

    return(off);
}
/*******************************************************************************
**
**  sfs_help_ipv6_ctc_cm_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_ctc_cm_key_cfg(char* buf)
{
    int off = 0;
    off += sprintf(buf+off, "echo [ipv6_ctc_Key_name] [ipv6_gen_Key_name] [ipv6_dip_Key_name] [ipv6_l4_Key_name] "
    "[ipv6_nh] [ipv6_hoplimit] > ipv6_ctc_cm_key_rule_set      - set IPv6 CTC CnM key\n");
    off += sprintf(buf+off, "\tIPV6 ctc Key name                (max 32 chars)Key name\n");
    off += sprintf(buf+off, "\tIPV6 gen Key name                (max 32 chars)Key name\n");
    off += sprintf(buf+off, "\tIPV6 dip Key name                (max 32 chars)Key name\n");
    off += sprintf(buf+off, "\tIPV6 l4 Key name                 (max 32 chars)Key name\n");
    off += sprintf(buf+off, "\tIPV6 ipv6 nh                     (dec)Next Header\n");
    off += sprintf(buf+off, "\tIPV6 ipv6 hoplimit               (dec)Hop Limit\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_l4_key_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_l4_key_cfg(char* buf)
{
    int off = 0;
    off += sprintf(buf+off, "echo [Key_name] [src_port] [dst_port] > ipv6_l4_key_rule_set  - set l4 key\n");
    off += sprintf(buf+off, "\tKey_name                (max 16 chars)Key name\n");
    off += sprintf(buf+off, "\tsrc_port                (dec)Source port\n");
    off += sprintf(buf+off, "\tdst_port                (dec)Destination port\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_rule_table_display - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_rule_table_display(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "\necho [table_name]         > rule_table_display  - display table rules\n");
    off += sprintf(buf+off, "\ttable_name    (str)frwd/vlan/mod/l2/l3/ipv4/ipv6_dip/ipv6_gen/ipv6_l4\n");
    return(off);
}

/*******************************************************************************
**
**  sfs_help_key_rule_delete - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_key_rule_delete(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [table_name] [rule_name]   > no_rule_set         - clear rule from table\n");
    off += sprintf(buf+off, "\ttable_name        (str)frwd/vlan/mod/l2key/l3key/ipv4key/ipv6_dip/ipv6_gen/ipv6_l4\n");

    off += sprintf(buf+off, "\trule_name         (str)Name of entry in table\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_l2_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_l2_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [key_name] [frwd_name] [mod_name] [pkt_mod_bm] > l2_rule_add\n");
    off += sprintf(buf+off, "Creates a new primary L2 processing ACL\n");

    off += sprintf(buf+off, "\towner_id        (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port        The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT/UNI_ANY)\n");
    off += sprintf(buf+off, "\trule_num        (dec)Entry number to be added to the current ACL\n");
    off += sprintf(buf+off, "\tparse_rule_bm   (hex)Bitmap containing the significant flags for parsing fields of the packet:\n");
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_MAC_DA             %#.4X\n", TPM_L2_PARSE_MAC_DA);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_MAC_SA             %#.4X\n", TPM_L2_PARSE_MAC_SA);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_ONE_VLAN_TAG       %#.4X\n", TPM_L2_PARSE_ONE_VLAN_TAG);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_TWO_VLAN_TAG       %#.4X\n", TPM_L2_PARSE_TWO_VLAN_TAG);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_ETYPE              %#.4X\n", TPM_L2_PARSE_ETYPE);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_PPPOE_SES          %#.4X\n", TPM_L2_PARSE_PPPOE_SES);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_PPP_PROT           %#.4X\n", TPM_L2_PARSE_PPP_PROT);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_GEMPORT            %#.4X\n", TPM_L2_PARSE_GEMPORT);
    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG1_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG1_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG2_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG2_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG2_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG2_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);
    off += sprintf(buf+off, "\taction          (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.4X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.4X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.4X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.4X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.4X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_MTM                  %#.4X\n", TPM_ACTION_MTM);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.4X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SPEC_MC_VID          %#.4X\n", TPM_ACTION_SPEC_MC_VID);
    off += sprintf(buf+off, "\tnext_phase      (str)Parse stage (l2/l3/ipv4/ipv6_gen/ipv6_dip/ipv6_nh/ipv6_l4/done)\n");
    off += sprintf(buf+off, "\tkey_name        (str)Name of L2 key data which has been defined by user [or l2_key_empty]\n");
    off += sprintf(buf+off, "\tfrwd_name       (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name        (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");
    off += sprintf(buf+off, "\tpkt_mod_bm      (str)Bit map of pkt modification data \n");
    off += sprintf(buf+off, "\t\tTPM_MAC_DA_SET                  %#.6X\n", TPM_MAC_DA_SET);
    off += sprintf(buf+off, "\t\tTPM_MAC_SA_SET                  %#.6X\n", TPM_MAC_SA_SET);
    off += sprintf(buf+off, "\t\tTPM_VLAN_MOD                    %#.6X\n", TPM_VLAN_MOD);
    off += sprintf(buf+off, "\t\tTPM_PPPOE_DEL                   %#.6X\n", TPM_PPPOE_DEL);
    off += sprintf(buf+off, "\t\tTPM_PPPOE_ADD                   %#.6X\n", TPM_PPPOE_ADD);
    off += sprintf(buf+off, "\t\tTPM_DSCP_SET                    %#.6X\n", TPM_DSCP_SET);
    off += sprintf(buf+off, "\t\tTPM_TTL_DEC                     %#.6X\n", TPM_TTL_DEC);
    off += sprintf(buf+off, "\t\tTPM_IPV4_UPDATE                 %#.6X\n", TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\tTPM_IPV4_SRC_SET                %#.6X\n", TPM_IPV4_SRC_SET);
    off += sprintf(buf+off, "\t\tTPM_IPV4_DST_SET                %#.6X\n", TPM_IPV4_DST_SET);
    off += sprintf(buf+off, "\t\tTPM_IPV6_UPDATE                 %#.6X\n", TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\tTPM_HOPLIM_DEC                  %#.6X\n", TPM_HOPLIM_DEC);
    off += sprintf(buf+off, "\t\tTPM_IPV6_SRC_SET                %#.6X\n", TPM_IPV6_SRC_SET);
    off += sprintf(buf+off, "\t\tTPM_IPV6_DST_SET                %#.6X\n", TPM_IPV6_DST_SET);
    off += sprintf(buf+off, "\t\tTPM_L4_SRC_SET                  %#.6X\n", TPM_L4_SRC_SET);
    off += sprintf(buf+off, "\t\tTPM_L4_DST_SET                  %#.6X\n", TPM_L4_DST_SET);

	return(off);
}

/*******************************************************************************
**
**  sfs_help_l3_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_l3_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [key_name] [frwd_name] > l3_rule_add\n");

    off += sprintf(buf+off, "Creates a new L3 type processing ACL\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT/UNI_ANY)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");
    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_ETYPE              %#.4X\n", TPM_L2_PARSE_ETYPE);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_PPPOE_SES          %#.4X\n", TPM_L2_PARSE_PPPOE_SES);
    off += sprintf(buf+off, "\t\tTPM_L2_PARSE_PPP_PROT           %#.4X\n", TPM_L2_PARSE_PPP_PROT);
    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG1_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG1_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG2_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG2_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG2_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG2_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);
    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.2X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.2X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.2X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);
    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (l2/l3/ipv4/ipv6_gen/ipv6_dip/ipv6_nh/ipv6_l4/ctc_cm/done)\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tkey_name         (str)Name of L3 key data which has been defined by user [or l3_key_empty]\n");

	return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv4_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv4_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [mod_bm] [key_name] [frwd_name] [mod_name] > ipv4_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv4 processing ACL\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         (str)The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT/UNI_ANY)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");
    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_SIP              %#.4X\n", TPM_IPv4_PARSE_SIP);
    off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_DIP              %#.4X\n", TPM_IPv4_PARSE_DIP);
    off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_DSCP             %#.4X\n", TPM_IPv4_PARSE_DSCP);
    off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_PROTO            %#.4X\n", TPM_IPv4_PARSE_PROTO);
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                %#.4X\n", TPM_PARSE_L4_SRC);
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                %#.4X\n", TPM_PARSE_L4_DST);
    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG1_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG1_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);
    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.4X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.4X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.4X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.4X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.4X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.4X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_UDP_CHKSUM_CALC      %#.4X\n", TPM_ACTION_UDP_CHKSUM_CALC);
    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (l2/l3/ipv4/ipv6_gen/ipv6_dip/ipv6_nh/ipv6_l4/ctc_cm/done)\n");
    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\t\tTPM_VLAN_MOD                    %#.4X\n", TPM_VLAN_MOD);
    off += sprintf(buf+off, "\t\tThis is the only packet modification supported in this version. \n");
    off += sprintf(buf+off, "\tkey_name         (str)Name of IPv4 key data which has been defined by user [or ipv4_key_empty]\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_gen_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_gen_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [mod_bm] [key_name] [frwd_name] [mod_name] > ipv6_gen_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 gen processing ACL\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         (str)The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT/UNI_ANY)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_SIP              %#.4X\n", TPM_IPv6_PARSE_SIP);
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_DSCP             %#.4X\n", TPM_IPv6_PARSE_DSCP);
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_HOPL             %#.4X\n", TPM_IPv6_PARSE_HOPL);

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG1_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG1_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);

    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.2X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.2X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.2X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);

    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (l2/l3/ipv4/ipv6_gen/ipv6_dip/ipv6_nh/ipv6_l4/done)\n");

    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\t\tTPM_VLAN_MOD                    %#.4X\n", TPM_VLAN_MOD);
    off += sprintf(buf+off, "\t\tThis is the only packet modification supported in this version. \n");
    off += sprintf(buf+off, "\tkey_name         (str)Name of IPv6 gen key data which has been defined by user [or ipv6_gen_key_empty]\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");


	return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_nh_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_nh_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [rule_num] [parse_flags_bm] [action] [next_phase] [nh_iter] [nh] [frwd_name] > ipv6_nh_acl_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 NH processing ACL\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);



    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.2X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.2X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.2X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);

    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (ipv6_nh/ipv6_l4/ipv6_gen/ctc_cm/done)\n");

    off += sprintf(buf+off, "\tnh_iter          (dec)0:First Next Header 1:Extension Header\n");
    off += sprintf(buf+off, "\tnh               (hex)Code of Next Header\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    return(off);
}


/*******************************************************************************
**
**  sfs_help_mc_ipvx_stream_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_mc_ipvx_stream_add(char* buf)
{
	int off = 0;

	off += sprintf(buf+off, "IPV4: echo [owner_id] [stream] [igmp_mode] [mc_stream_pppoe] [vid] [src_ip] [dst_ip] [ignore] [target_ports] > mc_ipv4_stream_add\n");
	off += sprintf(buf+off, "Creates a new multicast stream\n");

	off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tstream           (dec)Stream number\n");
	off += sprintf(buf+off, "\tigmp_mode        (dec)0-snooping/1-proxy\n");
	off += sprintf(buf+off, "\tmc_pppoe         (str)true/false.\n");
	off += sprintf(buf+off, "\tvid              (dec)Vlan Id. When Id is 65535, MC packet is untagged or do not need MC VID check\n");
	off += sprintf(buf+off, "\tsrc_ip           Source IP address\n");
	off += sprintf(buf+off, "\tdst_ip           Destination IP address\n");
	off += sprintf(buf+off, "\tignore           (str)true/false. When true, the source IP address is not part of the key\n");
	off += sprintf(buf+off, "\ttarget_ports     (hex)Bitmap of port targets \n");
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_WAN            %#.4X\n", TPM_TRG_PORT_WAN);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_0             %#.4X\n", TPM_TRG_TCONT_0);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_1             %#.4X\n", TPM_TRG_TCONT_1);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_2             %#.4X\n", TPM_TRG_TCONT_2);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_3             %#.4X\n", TPM_TRG_TCONT_3);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_4             %#.4X\n", TPM_TRG_TCONT_4);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_5             %#.4X\n", TPM_TRG_TCONT_5);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_6             %#.4X\n", TPM_TRG_TCONT_6);
	off += sprintf(buf+off, "\t\tTPM_TRG_TCONT_7             %#.4X\n", TPM_TRG_TCONT_7);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_0              %#.4X\n", TPM_TRG_LLID_0);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_1              %#.4X\n", TPM_TRG_LLID_1);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_2              %#.4X\n", TPM_TRG_LLID_2);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_3              %#.4X\n", TPM_TRG_LLID_3);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_4              %#.4X\n", TPM_TRG_LLID_4);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_5              %#.4X\n", TPM_TRG_LLID_5);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_6              %#.4X\n", TPM_TRG_LLID_6);
	off += sprintf(buf+off, "\t\tTPM_TRG_LLID_7              %#.4X\n", TPM_TRG_LLID_7);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_0               %#.5X\n", TPM_TRG_UNI_0);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_1               %#.5X\n", TPM_TRG_UNI_1);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_2               %#.5X\n", TPM_TRG_UNI_2);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_3               %#.5X\n", TPM_TRG_UNI_3);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_4               %#.5X\n", TPM_TRG_UNI_4);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_5               %#.5X\n", TPM_TRG_UNI_5);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_6               %#.5X\n", TPM_TRG_UNI_6);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_7               %#.5X\n", TPM_TRG_UNI_7);
	off += sprintf(buf+off, "\t\tTPM_TRG_UNI_VIRT            %#.5X\n", TPM_TRG_UNI_VIRT);
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_CPU            %#.5X\n", TPM_TRG_PORT_CPU);
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_UNI_ANY        %#.5X\n", TPM_TRG_PORT_UNI_ANY);
	off += sprintf(buf+off, "\t\tTPM_TRG_PORT_UNI_CPU_LOOP   %#.5X\n", TPM_TRG_PORT_UNI_CPU_LOOP);

	off += sprintf(buf+off, "\n\tIPV4 address: dst_ip 224.x.y.z  - 239.x.y.z.  Source address w.x.y.z. Each part is decimal value in range 0..255.\n");
	off += sprintf(buf+off, "\n\n");
	off += sprintf(buf+off, "echo [owner_id] [stream] [target_ports] > mc_ipv4_stream_update\n");
	off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tstream           (dec)Stream number\n");
	off += sprintf(buf+off, "\ttarget_ports     (hex)Bitmap of port targets - SEE ABOVE\n");

	off += sprintf(buf+off, "\nIPV6: echo [owner_id] [stream] [igmp_mode] [mc_stream_pppoe] [vid] [src_ip] [dst_ip] [ignore] [target_ports] > mc_ipv6_stream_add\n");
	off += sprintf(buf+off, "Creates a new multicast stream, params refer to IPv4 MC stream add interface, except dst_ip.\n");
	off += sprintf(buf+off, "\tIPV6 address: ff00:aabb:bbcc:ccdd:ddee:eeff:1111:2222. Each part is Hex value in range 0..ffff.\n");
	off += sprintf(buf+off, "\n\n");
	off += sprintf(buf+off, "echo [owner_id] [stream] [target_ports] > mc_ipv6_stream_update\n");
	off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tstream           (dec)Stream number\n");
	off += sprintf(buf+off, "\ttarget_ports     (hex)Bitmap of port targets - SEE ABOVE\n");

	return(off);
}

/*******************************************************************************
**
**  sfs_help_igmp_cfg - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_igmp_cfg(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [frwd_mode] > igmp_port_forward_mode_cfg\n");
    off += sprintf(buf+off, "Configures port forward mode\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         (str)Source port - WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_ANY\n");
    off += sprintf(buf+off, "\tfrwd_mode        (str)Forwarding mode - drop/frwd/snoop\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [queue] > igmp_cpu_queue_cfg\n");
    off += sprintf(buf+off, "Configures CPU queue for IGMP\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tqueue            (dec)CPU queue 0-7\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [sa_mac] > igmp_proxy_sa_mac\n");
    off += sprintf(buf+off, "Configures the sa_mac for IGMP proxy\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsa_mac           (hex)Mac address in format, aa:bb:cc:dd:ee:ff\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] > mc_vid_key_reset\n");
    off += sprintf(buf+off, "reset the mc_uni_xlate configuration for all the UNI ports\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [src_port] [mc_uni_xlate_mode] [mc_uni_xlate_vid] > mc_vid_key_set\n");
    off += sprintf(buf+off, "Configures mc_uni_xlate_mode\n");
    off += sprintf(buf+off, "\tsrc_port                    (str)Source port - UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT\n");
    off += sprintf(buf+off, "\tmc_uni_xlate_mode           (dec)uni_xlate_mode\n");
    off += sprintf(buf+off, "\t\texclude             0\n");
    off += sprintf(buf+off, "\t\ttransparent         1\n");
    off += sprintf(buf+off, "\t\tstrip               2\n");
    off += sprintf(buf+off, "\t\ttranslate           3\n");
    off += sprintf(buf+off, "\tuni_vid                      (dec)mc uni vid\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [mc_vid] > mc_vid_cfg_set\n");
    off += sprintf(buf+off, "Configures mc vid, first use mc_vid_key_reset, then mc_vid_key_set to set per uni xlate, at last use mc_vid_cfg_set\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tmc_vid           (dec)mc_vid\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_no_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_no_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [rule_num]            > no_rule_add_l2                - delete L2 rule from hardware\n");
    off += sprintf(buf+off, "echo [owner_id] [rule_idx]            > no_rule_add_l3                - delete L3 rule from hardware\n");
    off += sprintf(buf+off, "echo [owner_id] [rule_idx]            > no_rule_add_ipv4              - delete IPv4 rule from hardware\n");
    off += sprintf(buf+off, "echo [owner_id] [rule_idx]            > no_rule_add_ipv6_dip_acl      - delete a IPv6 DIP ACL\n");
    off += sprintf(buf+off, "echo [owner_id] [rule_idx]            > no_rule_add_ipv6_gen_acl      - delete a IPv6 gen ACL\n");
    off += sprintf(buf+off, "echo [owner_id] [rule_idx]            > no_rule_add_ipv6_l4_ports_acl - delete a IPv6 L4 ports ACL\n");
    off += sprintf(buf+off, "echo [owner_id] [rule_idx]            > no_rule_add_ipv6_nh_acl       - delete a IPv6 NH ACL\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\trule_idx         (dec)Rule Id returned from the create call\n");

    off += sprintf(buf+off, "\necho [owner_id] [src_port] [precedence]    > no_rule_add_ctc_cm_acl  - delete a CTC cm rule from hardware\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         (str)The packet originating source port (UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT)\n");
    off += sprintf(buf+off, "\tprecedence       (dec)Precedence - 0..7\n");

    off += sprintf(buf+off, "echo [owner_id] [l2key_name]          > no_rule_add_mac_learn         - delete MAC learn rule from hardware\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tl2key_name       (str)Name of L2 key data which define src MAC addr, empty is not allowed\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_no_mc_stream_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_no_mc_stream_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [stream]   > no_mc_stream_add_ipv4   - delete IPV4 multicast stream from hardware\n");
    off += sprintf(buf+off, "\towner_id   (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tstream     (dec)Stream Id used in the mc create call\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_oam_omci_channel - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_oam_omci_channel(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [rx_q] [llid]             > oam_channel - create OAM management channel\n");
    off += sprintf(buf+off, "\towner_id   (dec)Application owner ID\n");
    off += sprintf(buf+off, "\trx_q       (dec)Receive queue\n");
    off += sprintf(buf+off, "\tllid       (hex)LLID for management\n");
    off += sprintf(buf+off, "\t\tLLID_0  0x01     LLID_1  0x02    LLID_2  0x04     LLID_3  0x08\n");
    off += sprintf(buf+off, "\t\tLLID_4  0x10     LLID_5  0x20    LLID_6  0x40     LLID_7  0x80\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [gem_port] [rx_q] [tx_q] [tcont] > omci_channel - create OMCI management channel\n");
    off += sprintf(buf+off, "\towner_id   (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tgem_port   (dec)OMCI gem port\n");
    off += sprintf(buf+off, "\trx_q       (dec)Receive queue\n");
    off += sprintf(buf+off, "\ttx_q       (dec)Transmit queue\n");
    off += sprintf(buf+off, "\ttcont      (hex)T-CONT for management\n");
    off += sprintf(buf+off, "\t\tTCONT_0  0x01     TCONT_1  0x02    TCONT_2  0x04     TCONT_3  0x08\n");
    off += sprintf(buf+off, "\t\tTCONT_4  0x10     TCONT_5  0x20    TCONT_6  0x40     TCONT_7  0x80\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_no_oam_omci_channel - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_no_oam_omci_channel(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] > no_oam_channel   - delete OAM management channel from hardware\n");
    off += sprintf(buf+off, "echo [owner_id] > no_omci_channel  - delete OMCI management channel from hardware\n");
    off += sprintf(buf+off, "\towner_id   (dec)Application owner ID\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_setup - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_setup(char* buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [owner_id] [wan_type] [filename] > setup   - tpm setup \n");
	off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
	off += sprintf(buf+off, "\t[wan_type]       (dec)EPON 0 or GPON 1\n");
	off += sprintf(buf+off, "\t[filename]       FILENAME/default\n");

	return(off);
}

/*******************************************************************************
**
**  sfs_help_mib_reset - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_mib_reset(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [reset_level] > mib_reset   - MIB reset\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[reset_level]    (dec)Reset level 0 or 1\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_erase_section - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_erase_section(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [api_type] > erase_section   - erase spceified API section\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_type]       (dec)the API PnC section to be deleted\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_flush_vtu - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_flush_vtu(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] > flush_vtu   - flush VTU on Switch\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_flush_atu - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_flush_atu(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] > flush_atu   - flush ATU on Switch\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tflush_type       (dec)0-flush all/1-flush dynamic\n");
    off += sprintf(buf+off, "\tdb_num           (dec)only 0 is valid since there is only one ATU db\n");

    return(off);
}


/*******************************************************************************
**
**  sfs_help_cfg_cpu_lpbk - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_cfg_cpu_lpbk(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [trg_port] [trg_queue] [gem port]  > add_cpu_lpbk   - Add CPU WAN loopback\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[trg_port ]       (hex)tcont/llid bitmap, 0x01:tcont0/llid0,..., 0x80: tcont7/llid7\n");
    off += sprintf(buf+off, "\t[trg_queue]       (dec)Target queue number, 0~7\n");
    off += sprintf(buf+off, "\t[gem port ]       (dec)GEM port number, 0<gem port<4096, fill 0 in EPON mode\n");
    off += sprintf(buf+off, "echo [owner_id] [trg_port] [trg_queue] [gem port]  > del_cpu_lpbk   - Del CPU WAN loopback\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[trg_port ]       (hex)tcont/llid bitmap, 0x01:tcont0/llid0,..., 0x80: tcont7/llid7\n");
    off += sprintf(buf+off, "\t[trg_queue]       (dec)Target queue number, 0~7\n");
    off += sprintf(buf+off, "\t[gem port ]       (dec)GEM port number, 0<gem port<4096, fill 0 in EPON mode\n");
    off += sprintf(buf+off, "echo [owner_id]                                    > dump_cpu_lpbk  - Dump all valid loopback entry from DB \n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_cfg_age_count - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_cfg_age_count(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [api_group] [lu_num] [lu_reset]        > get_lu_entry   - Get least used PnC entires\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_group]       (dec)PnC API group, supported TPM API group,l2[%d], L3[%d], L4[%d] \n", TPM_API_L2_PRIM, TPM_API_L3_TYPE, TPM_API_IPV4);
    off += sprintf(buf+off, "\t[lu_num   ]       (dec)required LU number queue number, 1~100\n");
    off += sprintf(buf+off, "\t[lu_reset ]       (dec)Whether need to reset PnC aging counter after read it.1: reset after read, 0: do not reset\n");
    off += sprintf(buf+off, "echo [owner_id] [api_group] [rule_idx] [lu_rule_mask]  > set_count_mask  - Set least used mask of specific PnC entry\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_group]       (dec)PnC API group, supported TPM API group,l2[%d], L3[%d], L4[%d] \n", TPM_API_L2_PRIM, TPM_API_L3_TYPE, TPM_API_IPV4);
    off += sprintf(buf+off, "\t[rule_idx ]       (dec)rule index returned when add L2/L3/L4 PnC rules\n");
    off += sprintf(buf+off, "\t[lu_rule_mask]    (dec)Whether need to mask least used loopup for specific PnC rule.1: mask, 0: enable\n");
    off += sprintf(buf+off, "echo [owner_id] [api_group] [rule_idx] [hit_reset]     > get_hit_count   - Get hitted counter for specific PnC rules\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_group]       (dec)PnC API group, supported TPM API group,l2[%d], L3[%d], L4[%d] \n", TPM_API_L2_PRIM, TPM_API_L3_TYPE, TPM_API_IPV4);
    off += sprintf(buf+off, "\t[rule_idx ]       (dec)rule index returned when add L2/L3/L4 PnC rules\n");
    off += sprintf(buf+off, "echo [owner_id] [api_group] [lu_thresh_pkts]           > set_lu_thresh   - Set least used threshold for specific counter group\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_group]       (dec)PnC API group, supported TPM API group,l2[%d], L3[%d], L4[%d] \n", TPM_API_L2_PRIM, TPM_API_L3_TYPE, TPM_API_IPV4);
    off += sprintf(buf+off, "\t[lu_thresh_pkts ] (dec)Least used lookup threshold in number of packets, 0~67108863\n");
    off += sprintf(buf+off, "echo [owner_id] [api_group]                            > reset_age_group - Reset aging counter for all the PnC rules in one couter group\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_group]       (dec)PnC API group, supported TPM API group,l2[%d], L3[%d], L4[%d] \n", TPM_API_L2_PRIM, TPM_API_L3_TYPE, TPM_API_IPV4);

    off += sprintf(buf+off, "echo [owner_id] [api_group] [high_thresh_pkts] [counters_reset] [valid_counters] > get_all_hit_counters - Get all PnC  entires for the API group lower than threshold\n");
    off += sprintf(buf+off, "\t[owner_id ]       (dec)Application owner ID\n");
    off += sprintf(buf+off, "\t[api_group]       (dec)PnC API group, supported TPM API group,l2[%d], L3[%d], L4[%d] \n", TPM_API_L2_PRIM, TPM_API_L3_TYPE, TPM_API_IPV4);
    off += sprintf(buf+off, "\t[high_thresh_pkts](dec)counter high threshold watermark\n");
    off += sprintf(buf+off, "\t[counters_reset]  (dec)Whether need to reset PnC aging counter after read it.1: reset after read, 0: do not reset\n");
    off += sprintf(buf+off, "\t[valid_counters]  (dec)The number of entries to read, returns the actually read counters\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_rate_limit - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_rate_limit(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [direction] [sched_entity] [que] [rate_limit] [bucket_size]  > rate_limit_queue_set -  set queue rate limit\n");
    off += sprintf(buf+off, "\towner_id        (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tdirection       Upstream or downstream (us/ds)\n");
    off += sprintf(buf+off, "\tsched_entity    WAN/TCONT_0/TCONT_1/../TCONT_7/UNI_ANY\n");
    off += sprintf(buf+off, "\tque             (dec)Queue id - 0..7\n");
    off += sprintf(buf+off, "\trate_limit      (dec)> 64\n");
    off += sprintf(buf+off, "\tbucket_size     (dec)TBD\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [direction] [sched_entity] [rate_limit] [bucket_size]  > rate_limit_if_set   - set interface rate limit\n");
    off += sprintf(buf+off, "\towner_id        Application owner ID\n");
    off += sprintf(buf+off, "\tdirection       Upstream or downstream (us/ds)\n");
    off += sprintf(buf+off, "\tsched_entity    WAN/TCONT_0/TCONT_1/../TCONT_7/UNI_ANY\n");
    off += sprintf(buf+off, "\trate_limit      (dec)> 64\n");
    off += sprintf(buf+off, "\tbucket_size     (dec)TBD\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [direction] [sched_entity] [mode] [que] [wrr_weight] > scheduling_mode_queue_set   - configure queue scheduling mode\n");
    off += sprintf(buf+off, "\towner_id        Application owner ID\n");
    off += sprintf(buf+off, "\tdirection       Upstream or downstream (us/ds)\n");
    off += sprintf(buf+off, "\tsched_entity    WAN/TCONT_0/TCONT_1/../TCONT_7/UNI_ANY\n");
    off += sprintf(buf+off, "\tmode            Strict or weighted round robin - strict/wrr \n");
    off += sprintf(buf+off, "\tque             (dec)Queue id - 0..7\n");
    off += sprintf(buf+off, "\twrr_weight      (dec)Weight for wrr\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [owner_id] [rate_limit] [bucket_size] > gmac0_ingr_rate_limit_set - set ingress rate limit for GMAC0 only when no switch init\n");
    off += sprintf(buf+off, "\towner_id        (dec)Application owner ID\n");
    off += sprintf(buf+off, "\trate_limit      (dec)> 64\n");
    off += sprintf(buf+off, "\tbucket_size     (dec)TBD\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [thresh_high] [thresh_low] [port] [tx_port] [tx_queue] [tgt_port] > "
     					"fc_config_set   - configure CPU port flow control\n");
    off += sprintf(buf+off, "\tthresh_high     (dec)High threshold (decriptors amount)\n");
    off += sprintf(buf+off, "\tthresh_low      (dec)Low threshold (decriptors amount)\n");
    off += sprintf(buf+off, "\tport            (dec)Port to monitor \n");
    off += sprintf(buf+off, "\ttx_port         (dec)TX port\n");
    off += sprintf(buf+off, "\ttx_queue        (dec)TX queue\n");
    off += sprintf(buf+off, "\ttgt_port        (dec)Port to apply FC\n");

    off += sprintf(buf+off, "\n");
    off += sprintf(buf+off, "echo [us_period] > fc_us_period_set - configure flow control period\n");
    off += sprintf(buf+off, "\tus_period       (dec)Flow control engine monitoring priod (us)\n");
    off += sprintf(buf+off, "\n");
#ifdef TPM_FC_DEBUG
    off += sprintf(buf+off, "echo [oneshot_hits] > fc_oneshot_set - configure oneshot statistic count\n");
    off += sprintf(buf+off, "\toneshot_hits    (dec)Flow control engine monitoring oneshot statistics hits count\n");
    off += sprintf(buf+off, "\t                Setting this value to 0 resets oneshot statistics counters\n");
    off += sprintf(buf+off, "\n");
#endif

    return(off);
}

/*******************************************************************************
**
**  sfs_help_pkt_mod_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_pkt_mod_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo  [trg_port] [mod_bm] [flag] [mod_name] > pkt_mod_eng_entry_add   - add packet modification to mod rule\n");
    off += sprintf(buf+off, "\ttrg_port   Target port - WAN/TCONT_0/TCONT_1/TCONT_2/TCONT_3/TCONT_4/TCONT_5/TCONT_6/TCONT_7/UNI_ANY\n");
    off += sprintf(buf+off, "\tmod_bm     (hex)Modification bitmap:\n");

    /* VLAN Operation only */
    off += sprintf(buf+off, "\t\t0x%05x - VLAN\n", TPM_VLAN_MOD);

    /* IPv4 Napt */
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | TTL | SIP | L4_SRC\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | TTL | DIP | L4_DST\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | SIP | L4_SRC | TTL\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | DIP | L4_DST | TTL\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | PPPoE ADD | SIP | L4_SRC | TTL\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | PPPoE DEL | DIP | L4_DST | TTL\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | PPPoE ADD | SIP | L4_SRC | TTL\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | PPPoE DEL | DIP | L4_DST | TTL\n",
        TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET);

    /* IPv6 Route */
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | VLAN | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | PPPoE ADD | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_ADD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | PPPoE DEL | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | VLAN | PPPoE ADD | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_ADD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | VLAN | PPPoE DEL | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);

    /* IPv4 Route */
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | PPPoE ADD | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | PPPoE DEL | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | PPPoE ADD | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | PPPoE DEL | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE);

    /* (IPv4) Multicast */
    off += sprintf(buf+off, "\t\t0x%05x - MH\n", TPM_MH_SET);
    off += sprintf(buf+off, "\t\t0x%05x - MH | VLAN\n", TPM_MH_SET | TPM_VLAN_MOD);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | MH | SA | TTL\n", TPM_MH_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | MH | SA | VLAN | TTL\n", TPM_MH_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | MH | DA | SA | PPPoE DEL | TTL\n", TPM_MH_SET | TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | MH | DA | SA | VLAN | PPPoE DEL | TTL\n", TPM_MH_SET | TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | SA | TTL\n", TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | SA | VLAN | TTL\n", TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | PPPoE DEL | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv4 | DA | SA | VLAN | PPPoE DEL | TTL\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE);

	/* (IPv6) Multicast */
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | MH | SA | HOPLIM\n", TPM_MH_SET | TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | MH | SA | VLAN | HOPLIM\n", TPM_MH_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | MH | DA | SA | PPPoE DEL | HOPLIM\n", TPM_MH_SET | TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | MH | DA | SA | VLAN | PPPoE DEL | HOPLIM\n", TPM_MH_SET | TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | SA | HOPLIM\n", TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | SA | VLAN | HOPLIM\n", TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | PPPoE DEL | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);
    off += sprintf(buf+off, "\t\t0x%05x - IPv6 | DA | SA | VLAN | PPPoE DEL | HOPLIM\n", TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE);

    off += sprintf(buf+off, "\tflag       (hex)Additional packet data:\n");
    off += sprintf(buf+off, "\t\t0x%03x - TCP\n", TPM_INT_L4_TCP);
    off += sprintf(buf+off, "\t\t0x%03x - UDP\n", TPM_INT_L4_UDP);
    off += sprintf(buf+off, "\t\t0x%03x - Split modification flag\n", TPM_INT_SPLIT_MOD);
    off += sprintf(buf+off, "\t\t0x%03x - Update UDP checksum\n", TPM_INT_UDP_CHECKSUM);
    off += sprintf(buf+off, "\t\t0x%03x - Multicast modification\n", TPM_INT_MC_MOD);

    off += sprintf(buf+off, "\tmod_name   Name of pkt modification data which has been defined by user\n");
    return(off);
}

/*******************************************************************************
**
**  sfs_help_pkt_mod_eng - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_pkt_mod_get_del_purge(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo  [trg_port] [mod_idx]       > pkt_mod_eng_entry_del   - delete packet modification from mod rule\n");
    off += sprintf(buf+off, "echo  [trg_port] [mod_idx]       > pkt_mod_eng_entry_show  - show packet modification\n");
    off += sprintf(buf+off, "echo  [trg_port]                 > pkt_mod_eng_purge       - delete all packet modifications for given port\n");
    off += sprintf(buf+off, "\ttrg_port   Target port - WAN/TCONT_0/TCONT_1/TCONT_2/TCONT_3/TCONT_4/TCONT_5/TCONT_6/TCONT_7/UNI_ANY\n");
    off += sprintf(buf+off, "\tmod_idx    (dec)Modification entry returned from pkt_mod_eng_entry_add\n");

    return(off);
}

/* jinghuaxxxxxx */
/*******************************************************************************
**
**  sys_help_ipv6_dip_acl_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sys_help_ipv6_dip_acl_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [mod_bm] [key_name] [frwd_name] [mod_name] > ipv6_dip_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 processing ACL\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         (str)The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT/UNI_ANY)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_DIP              %#.4X\n", TPM_IPv6_PARSE_DIP);

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_TRUE        %#.4X\n", TPM_PARSE_FLAG_TAG1_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TAG1_FALSE       %#.4X\n", TPM_PARSE_FLAG_TAG1_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);


    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.2X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.2X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.2X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);

    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (l2/l3/ipv4/ipv6_gen/ipv6_dip/ipv6_nh/ipv6_l4/done)\n");

    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\t\tTPM_VLAN_MOD                    %#.4X\n", TPM_VLAN_MOD);
    off += sprintf(buf+off, "\t\tThis is the only packet modification supported in this version. \n");

    off += sprintf(buf+off, "\tkey_name         (str)Name of IPv6 key data which has been defined by user [or ipv6_dip_key_empty]\n");

    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");

    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");

    return(off);
}

#if 0
 int sys_help_del_ipv6_dip_acl_rule          (char *buf)
{
     int off = 0;

     off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_idx] [parse_rule_bm] [l2_key_name]    > no_rule_add_l2   - delete L2 rule from hardware\n");
     off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_idx] [parse_rule_bm] [l3_key_name]    > no_rule_add_l3   - delete L3 rule from hardware\n");
     off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_idx] [parse_rule_bm] [ipv4_key_name]  > no_rule_add_ipv4 - delete IPV4 rule from hardware\n");
     off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_idx] [parse_rule_bm] [ipv6_key_name]  > no_rule_add_ipv6 - delete IPV6 rule from hardware\n");
     off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
     off += sprintf(buf+off, "\tsrc_port         WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_ANY\n");
     off += sprintf(buf+off, "\trule_idx         (dec)Rule Id returned from the create call\n");
     off += sprintf(buf+off, "\tparse_rule_bm    (hex)Parsing fields bitmap used when the rule was created\n");
     off += sprintf(buf+off, "\tkey_name         The key name used when the rule was created\n");
     off += sprintf(buf+off, "\necho [owner_id] [rule_idx]  > no_rule_add_dscp_ipv4 - delete DSCP IPV4 rule from hardware\n");
     off += sprintf(buf+off, "echo [owner_id] [rule_idx]  > no_rule_add_dscp_ipv6 - delete DSCP IPV6 rule from hardware\n");
     off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
     off += sprintf(buf+off, "\trule_idx         (dec)Rule Id returned from the create call\n");


     off += sprintf(buf+off, "rule del ipv6 <owner_id> <src_port> <rule_index> \n");
     off += sprintf(buf+off, "delete an existing IPv6 processing ACL\n");

     off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
     off += sprintf(buf+off, "\tsrc_port          (str)The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_ANY)\n");
     off += sprintf(buf+off, "\rule_index         (dec)Entry index was given when adding the ACL\n");

    return(off);
}
int sys_help_del_ipv6_l4_ports_acl_rule     (char *buf)
 {
      int off = 0;

      off += sprintf(buf+off, "rule del ipv6 <owner_id>  <rule_index> \n");
      off += sprintf(buf+off, "delete an existing IPv6 l4 processing ACL\n");

      off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
      off += sprintf(buf+off, "\tsrc_port          (str)The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_ANY)\n");
      off += sprintf(buf+off, "\rule_index         (dec)Entry index was given when adding the ACL\n");

     return(off);
 }

#endif
/*******************************************************************************
**
**  sfs_help_pkt_mod_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_l4_ports_rule_add     (char *buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_port] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [l4_src_port] [l4_dst_port] [mod_bm] [frwd_name] [mod_name] > ipv6_l4_ports_acl_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 l4 processing ACL\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_port         (str)The packet originating source port (WAN/UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT/UNI_ANY)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                %#.4X\n", TPM_PARSE_L4_SRC);
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                %#.4X\n", TPM_PARSE_L4_DST);

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE         %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE        %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_TRUE      %#.4X\n", TPM_PARSE_FLAG_TO_CPU_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_TO_CPU_FALSE     %#.4X\n", TPM_PARSE_FLAG_TO_CPU_FALSE);

    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK              %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD          %#.2X\n", TPM_ACTION_SET_PKT_MOD);
    off += sprintf(buf+off, "\t\tTPM_ACTION_TO_CPU               %#.2X\n", TPM_ACTION_TO_CPU);
    off += sprintf(buf+off, "\t\tTPM_ACTION_CUST_CPU_PKT_PARSE   %#.2X\n", TPM_ACTION_CUST_CPU_PKT_PARSE);
    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (ipv6_gen/ipv6_dip/ctc_cm/done)\n");
    off += sprintf(buf+off, "\tl4_src_port      (dec)L4 source port\n");
    off += sprintf(buf+off, "\tl4_dst_port      (dec)L4 destination port\n");



    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\t\tTPM_VLAN_MOD                    %#.4X\n", TPM_VLAN_MOD);
    off += sprintf(buf+off, "\t\tThis is the only packet modification supported in this version. \n");

    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");


    return(off);
 }

/*******************************************************************************
**
**  sfs_help_delete_entry_rule_table - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_delete_entry_rule_table(char *buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [rule_name]     > delete_entry_frwd_rule_table                - delete a forwarding rule entry\n");
    off += sprintf(buf+off, "echo [rule_name]     > delete_entry_mod_rule_table                 - delete a modification rule entry\n");
    off += sprintf(buf+off, "echo [rule_name]     > delete_entry_vlan_rule_table                - delete a VLAN rule entry\n");
    off += sprintf(buf+off, "echo [l2key_name]    > delete_entry_l2_rule_table                  - delete a L2 key entry\n");
    off += sprintf(buf+off, "echo [l3key_name]    > delete_entry_l3_rule_table                  - delete a L3 key entry\n");
    off += sprintf(buf+off, "echo [ipv4key_name]  > delete_entry_ipv4_rule_table                - delete a ipv4 key entry\n");
    off += sprintf(buf+off, "echo [ipv6_gen]      > delete_entry_ipv6_gen_rule_table            - delete a ipv6_gen key entry\n");
    off += sprintf(buf+off, "echo [ipv6_dip]      > delete_entry_ipv6_dip_rule_table            - delete a ipv6_dip key entry\n");
    off += sprintf(buf+off, "echo [ipv6_l4]       > delete_entry_ipv6_l4_rule_table             - delete a ipv6_l4 key entry\n");

    return(off);
}


int sfs_help_send_genquery_to_uni(char *buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [target_ports]	[pkt_num]  > send_genquery_to_uni\n");
	off += sprintf(buf+off, "send general query to uni ports\n");
    off += sprintf(buf+off, "\ttarget_ports                  (hex)Bitmap of port targets \n");
    off += sprintf(buf+off, "\t\tTPM_TRG_UNI_0               %#.4X\n", TPM_TRG_UNI_0);
    off += sprintf(buf+off, "\t\tTPM_TRG_UNI_1               %#.4X\n", TPM_TRG_UNI_1);
    off += sprintf(buf+off, "\t\tTPM_TRG_UNI_2               %#.4X\n", TPM_TRG_UNI_2);
    off += sprintf(buf+off, "\t\tTPM_TRG_UNI_3               %#.4X\n", TPM_TRG_UNI_3);
    off += sprintf(buf+off, "\t\tTPM_TRG_UNI_4               %#.4X\n", TPM_TRG_UNI_4);
    off += sprintf(buf+off, "\t\tTPM_TRG_PORT_UNI_ANY        %#.4X\n", TPM_TRG_PORT_UNI_ANY);
	off += sprintf(buf+off, "\tpkt_num                       (dec)general query packet number send to each UNI port\n");

	return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_gen_5t_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_gen_5t_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_dir] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [mod_bm] [l4_key_name] [gen_key_name] [frwd_name] [mod_name] > ipv6_gen_5t_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 SIP processing rule of 5-tuple\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_dir          (str)The packet direction (US/DS)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                 %#.4X\n", TPM_PARSE_L4_SRC);
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                 %#.4X\n", TPM_PARSE_L4_DST);
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_SIP               %#.4X\n", TPM_IPv6_PARSE_SIP);

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_L4_UDP            %#.4X\n", TPM_PARSE_FLAG_L4_UDP);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_L4_TCP            %#.4X\n", TPM_PARSE_FLAG_L4_TCP);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE          %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE         %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_PPPOE_TRUE        %#.4X\n", TPM_PARSE_FLAG_PPPOE_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_PPPOE_FALSE       %#.4X\n", TPM_PARSE_FLAG_PPPOE_FALSE);

    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK               %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT       %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE      %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD           %#.2X\n", TPM_ACTION_SET_PKT_MOD);

    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (ipv6_dip/done)\n");

    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\tl4_key_name      (str)Name of IPv6 l4 ports key data which has been defined by user [or ipv6_l4_key_empty]\n");
    off += sprintf(buf+off, "\tgen_key_name     (str)Name of IPv6 gen key data which has been defined by user [or ipv6_gen_key_empty]\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_dip_5t_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_dip_5t_rule_add(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_dir] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [mod_bm] [l4_key_name] [gen_key_name] [dip_key_name] [frwd_name] [mod_name] > ipv6_dip_5t_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 DIP processing rule of 5-tuple\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_dir          (str)The packet direction (US/DS)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                 %#.4X\n", TPM_PARSE_L4_SRC);
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                 %#.4X\n", TPM_PARSE_L4_DST);
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_SIP               %#.4X\n", TPM_IPv6_PARSE_SIP);
    off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_DIP               %#.4X\n", TPM_IPv6_PARSE_DIP);

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_L4_UDP            %#.4X\n", TPM_PARSE_FLAG_L4_UDP);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_L4_TCP            %#.4X\n", TPM_PARSE_FLAG_L4_TCP);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE          %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE         %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_PPPOE_TRUE        %#.4X\n", TPM_PARSE_FLAG_PPPOE_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_PPPOE_FALSE       %#.4X\n", TPM_PARSE_FLAG_PPPOE_FALSE);

    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK               %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT       %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE      %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD           %#.2X\n", TPM_ACTION_SET_PKT_MOD);

    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (done)\n");
    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\tl4_key_name      (str)Name of IPv6 key data which has been defined by user [or ipv6_l4_key_empty]\n");
    off += sprintf(buf+off, "\tgen_key_name     (str)Name of IPv6 key data which has been defined by user [or ipv6_gen_key_empty]\n");
    off += sprintf(buf+off, "\tdip_key_name     (str)Name of IPv6 key data which has been defined by user [or ipv6_dip_key_empty]\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");

    return(off);
}

/*******************************************************************************
**
**  sfs_help_ipv6_l4_ports_5t_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ipv6_l4_ports_5t_rule_add(char *buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [src_dir] [rule_num] [parse_rule_bm] [parse_flags_bm] [action] [next_phase] [mod_bm] [l4_key_name] [frwd_name] [mod_name] > ipv6_l4_ports_5t_rule_add\n");
    off += sprintf(buf+off, "Creates a new IPv6 L4 ports processing rule of 5-tuple\n");

    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tsrc_dir          (str)The packet direction (US/DS)\n");
    off += sprintf(buf+off, "\trule_num         (dec)Entry number to be added to the current ACL\n");

    off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                 %#.4X\n", TPM_PARSE_L4_SRC);
    off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                 %#.4X\n", TPM_PARSE_L4_DST);

    off += sprintf(buf+off, "\tparse_flags_bm   (hex)Bitmap containing the significant flags result of the primary ACL filtering\n");
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_L4_UDP            %#.4X\n", TPM_PARSE_FLAG_L4_UDP);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_L4_TCP            %#.4X\n", TPM_PARSE_FLAG_L4_TCP);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_TRUE          %#.4X\n", TPM_PARSE_FLAG_MTM_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_MTM_FALSE         %#.4X\n", TPM_PARSE_FLAG_MTM_FALSE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_PPPOE_TRUE        %#.4X\n", TPM_PARSE_FLAG_PPPOE_TRUE);
    off += sprintf(buf+off, "\t\tTPM_PARSE_FLAG_PPPOE_FALSE       %#.4X\n", TPM_PARSE_FLAG_PPPOE_FALSE);

    off += sprintf(buf+off, "\taction           (hex)Action associated with the rule:\n");
    off += sprintf(buf+off, "\t\tTPM_ACTION_DROP_PK               %#.2X\n", TPM_ACTION_DROP_PK);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT       %#.2X\n", TPM_ACTION_SET_TARGET_PORT);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE      %#.2X\n", TPM_ACTION_SET_TARGET_QUEUE);
    off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD           %#.2X\n", TPM_ACTION_SET_PKT_MOD);

    off += sprintf(buf+off, "\tnext_phase       (str)Parse stage (ipv6_gen/ctc_cm/done)\n");
    off += sprintf(buf+off, "\tmod_bm           (hex)Packet fields modification command bitmap:\n");
    off += sprintf(buf+off, "\tl4_key_name      (str)Name of IPv6 key data which has been defined by user [or ipv6_l4_key_empty]\n");
    off += sprintf(buf+off, "\tfrwd_name        (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
    off += sprintf(buf+off, "\tmod_name         (str)Name of pkt modification data which has been defined by user [or mod_empty]\n");

    return(off);
 }

/*******************************************************************************
**
**  sfs_help_ctc_cm_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ctc_cm_rule_add(char *buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [owner_id] [src_port] [precedence] [l2_parse_rule_bm] [ipv4_parse_rule_bm] [action] [l2_key_name] [ipv4_key_name] [frwd_name] [p_bits] > ctc_cm_acl_rule_add\n");
	off += sprintf(buf+off, "Creates a CTC CnM rule\n");

	off += sprintf(buf+off, "\towner_id           (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tsrc_port           (str)The packet originating source port (UNI_0/UNI_1/UNI_2/UNI_3/UNI_4/UNI_5/UNI_6/UNI_7/UNI_VIRT)\n");
	off += sprintf(buf+off, "\tprecedence         (dec)Precedence - 0..7\n");

	off += sprintf(buf+off, "\tl2_parse_rule_bm   (hex)Bitmap containing the significant flags for parsing fields of the packet:\n");
	off += sprintf(buf+off, "\t\tTPM_L2_PARSE_MAC_DA             %#.4X\n", TPM_L2_PARSE_MAC_DA);
	off += sprintf(buf+off, "\t\tTPM_L2_PARSE_MAC_SA             %#.4X\n", TPM_L2_PARSE_MAC_SA);
	off += sprintf(buf+off, "\t\tTPM_L2_PARSE_ONE_VLAN_TAG       %#.4X\n", TPM_L2_PARSE_ONE_VLAN_TAG);
	off += sprintf(buf+off, "\t\tTPM_L2_PARSE_ETYPE              %#.4X\n", TPM_L2_PARSE_ETYPE);

	off += sprintf(buf+off, "\tipv4_parse_rule_bm (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
	off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_SIP                 %#.4X\n", TPM_IPv4_PARSE_SIP);
	off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_DIP                 %#.4X\n", TPM_IPv4_PARSE_DIP);
	off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_DSCP                %#.4X\n", TPM_IPv4_PARSE_DSCP);
	off += sprintf(buf+off, "\t\tTPM_IPv4_PARSE_PROTO               %#.4X\n", TPM_IPv4_PARSE_PROTO);
	off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                   %#.4X\n", TPM_PARSE_L4_SRC);
	off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                   %#.4X\n", TPM_PARSE_L4_DST);

	off += sprintf(buf+off, "\taction	           (hex)Action associated with the rule:\n");
	off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_PORT      %#.4X\n", TPM_ACTION_SET_TARGET_PORT);
	off += sprintf(buf+off, "\t\tTPM_ACTION_SET_TARGET_QUEUE     %#.4X\n", TPM_ACTION_SET_TARGET_QUEUE);
	off += sprintf(buf+off, "\t\tTPM_ACTION_SET_PKT_MOD	        %#.4X\n", TPM_ACTION_SET_PKT_MOD);

	off += sprintf(buf+off, "\tl2_key_name        (str)Name of L2 key data which has been defined by user [or l2_key_empty]\n");
	off += sprintf(buf+off, "\tipv4_key_name      (str)Name of IPV4 key data which has been defined by user [or ipv4_key_empty]\n");
	off += sprintf(buf+off, "\tfrwd_name          (str)Name of pkt forwarding data which has been defined by user [or frwd_empty]\n");
	off += sprintf(buf+off, "\tp_bits             (dec)P_bit  - 0..7\n");

	off += sprintf(buf+off, "\necho [owner_id] [src_port] [precedence] [ipv6_parse_rule_bm] [action] [ipv6_key_name] [frwd_name] [p_bits] > ctc_cm_ipv6_acl_rule_add\n");
	off += sprintf(buf+off, "Creates a CTC CnM rule\n");

	off += sprintf(buf+off, "\towner_id           (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tparse_rule_bm    (hex)Bitmap containing the significant flags for parsing fields of the packet\n");
	off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_SIP              %#.4X\n", TPM_IPv6_PARSE_SIP);
	off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_DIP              %#.4X\n", TPM_IPv6_PARSE_DIP);
	off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_DSCP             %#.4X\n", TPM_IPv6_PARSE_DSCP);
	off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_NH               %#.4X\n", TPM_IPv6_PARSE_NH);
	off += sprintf(buf+off, "\t\tTPM_IPv6_PARSE_HOPL             %#.4X\n", TPM_IPv6_PARSE_HOPL);
	off += sprintf(buf+off, "\t\tTPM_PARSE_L4_SRC                %#.4X\n", TPM_PARSE_L4_SRC);
	off += sprintf(buf+off, "\t\tTPM_PARSE_L4_DST                %#.4X\n", TPM_PARSE_L4_DST);

	return(off);
}

/*******************************************************************************
**
**  sfs_help_ctc_cm_set_ipv6_parse_win - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_ctc_cm_set_ipv6_parse_win(char* buf)
{
    int off = 0;

    off += sprintf(buf+off, "echo [owner_id] [ipv6_parse_win] > ctc_cm_set_ipv6_parse_win   - set ipv6 parse window\n");
    off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
    off += sprintf(buf+off, "\tipv6_parse_win   (dec)ipv6 parse window\n");
    off += sprintf(buf+off, "\t\tTPM_CTC_CM_IPv6_FIRST_24B	 %d\n", TPM_CTC_CM_IPv6_FIRST_24B);
    off += sprintf(buf+off, "\t\tTPM_CTC_CM_IPv6_SECOND_24B	 %d\n", TPM_CTC_CM_IPv6_SECOND_24B);

    return(off);
}

/*******************************************************************************
**
**  sfs_help_tpm_self_check - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_tpm_self_check(char* buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [owner_id] [check_level] > tpm_self_check   - tpm rules check\n");
	off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
	off += sprintf(buf+off, "\t[check_level]    (dec)0: check without correct; 1: check with correct\n");

	return(off);
}


/*******************************************************************************
**
**  sfs_help_mac_learn_rule_add - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_mac_learn_rule_add(char* buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [owner_id] [l2_key_name] > mac_learn_rule_add\n");
	off += sprintf(buf+off, "Creates a new L2 MAC learn rule ACL\n");
	off += sprintf(buf+off, "\towner_id         (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tl2_key_name      (str)Name of L2 key data which define src MAC addr, empty is not allowed \n");

	return(off);

}

/*******************************************************************************
**
**  sfs_help_mac_learn_def_act_set - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_help_mac_learn_def_act_set(char* buf)
{
	int off = 0;

	off += sprintf(buf+off, "echo [owner_id] [mac_conf] > mac_learn_default_rule_act_set\n");
	off += sprintf(buf+off, "Set the action of mac learn default rule\n");
	off += sprintf(buf+off, "\towner_id        (dec)Application owner ID\n");
	off += sprintf(buf+off, "\tmac_conf        (hex)Action associated with mac learn default rule: \n");
	off += sprintf(buf+off, "\t\tTPM_UNK_MAC_TRAP,     (trap packet to CPU)     %#.2X\n", TPM_UNK_MAC_TRAP);
	off += sprintf(buf+off, "\t\tTPM_UNK_MAC_DROP,     (drop the packet)        %#.2X\n", TPM_UNK_MAC_DROP);
	off += sprintf(buf+off, "\t\tTPM_UNK_MAC_CONTINUE, (frwd packet to GMAC1)   %#.2X\n", TPM_UNK_MAC_CONTINUE);

	return(off);

}


#endif /* CONFIG_MV_TPM_SYSFS_HELP */

