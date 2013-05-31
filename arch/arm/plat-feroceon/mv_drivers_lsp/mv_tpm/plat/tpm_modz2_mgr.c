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

#include "tpm_common.h"
#include "tpm_header.h"

/* Local variables */
//spinlock_t tpmModLock;

spinlock_t tpmMod2JumpEntryLock;
spinlock_t tpmMod2ChainEntryLock;
spinlock_t tpmMod2PmtHwLock;



/*******************************************************************************
                               Global definitions
*******************************************************************************/
#define TPM_IPV6_ADDR_LEN    (16)

/* Null MAC address */
uint8_t null_mac_address[TPM_SA_LEN] = {0,0,0,0,0,0};
/* Null IPv4 address */
uint8_t null_ipv4_address[TPM_IPV4_SIP_LEN] = {0,0,0,0};
/* Null IPv6 address */
uint8_t null_ipv6_address[TPM_IPV6_ADDR_LEN] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
/* NULL VLAN key */
tpm_vlan_key_t null_vlan_key = {0,0,0,0,0,0,0};


/*******************************************************************************
                        PROTOTYPES for internal using
*******************************************************************************/
int32_t tpm_mod2_process_mh_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_da_set (tpm_gmacs_enum_t      gmac_port,
                                 uint32_t                bm,
                                 uint16_t                cmd_idx,
                                 uint8_t                *skip,
                                 tpm_pkt_mod_t          *mod_data,
                                 tpm_mod_pattern_data_t *pattern_data,
                                 tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_sa_set (tpm_gmacs_enum_t      gmac_port,
                                 uint32_t                bm,
                                 uint16_t                cmd_idx,
                                 uint8_t                *skip,
                                 tpm_pkt_mod_t          *mod_data,
                                 tpm_mod_pattern_data_t *pattern_data,
                                 tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_vlan_mod (tpm_gmacs_enum_t      gmac_port,
                                   uint32_t                bm,
                                   uint16_t                cmd_idx,
                                   uint8_t                *skip,
                                   tpm_pkt_mod_t          *mod_data,
                                   tpm_mod_pattern_data_t *pattern_data,
                                   tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_pppoe_del (tpm_gmacs_enum_t      gmac_port,
                                    uint32_t                bm,
                                    uint16_t                cmd_idx,
                                    uint8_t                *skip,
                                    tpm_pkt_mod_t          *mod_data,
                                    tpm_mod_pattern_data_t *pattern_data,
                                    tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_pppoe_add (tpm_gmacs_enum_t      gmac_port,
                                    uint32_t                bm,
                                    uint16_t                cmd_idx,
                                    uint8_t                *skip,
                                    tpm_pkt_mod_t          *mod_data,
                                    tpm_mod_pattern_data_t *pattern_data,
                                    tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_ipv4_update (tpm_gmacs_enum_t      gmac_port,
                                      uint32_t                bm,
                                      uint16_t                cmd_idx,
                                      uint8_t                *skip,
                                      tpm_pkt_mod_t          *mod_data,
                                      tpm_mod_pattern_data_t *pattern_data,
                                      tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_dscp_set (tpm_gmacs_enum_t      gmac_port,
                                   uint32_t                bm,
                                   uint16_t                cmd_idx,
                                   uint8_t                *skip,
                                   tpm_pkt_mod_t          *mod_data,
                                   tpm_mod_pattern_data_t *pattern_data,
                                   tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_ttl_dec (tpm_gmacs_enum_t      gmac_port,
                                  uint32_t                bm,
                                  uint16_t                cmd_idx,
                                  uint8_t                *skip,
                                  tpm_pkt_mod_t          *mod_data,
                                  tpm_mod_pattern_data_t *pattern_data,
                                  tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_v4_sip_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_v4_dip_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_hoplim_dec (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_l4_src_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_l4_dst_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check);
int32_t tpm_mod2_process_l4_checksum_set(tpm_gmacs_enum_t      gmac_port,
                                         uint32_t                bm,
                                         uint16_t                cmd_idx,
                                         uint8_t                *skip,
                                         tpm_pkt_mod_t          *mod_data,
                                         tpm_mod_pattern_data_t *pattern_data,
                                         tpm_self_check_enable_t is_tpm_check);



tpm_mod_flag_bm_entry_t tpm_mod2_jump_pattern =
/*=============================================================================================*/
/*  <skip_before><skip_no_parse><mask><process_function>    <opcode>                <mod_data><last><ip4updt><L4updt>*/
{        0,             0,        0,         NULL,     {{TPM_MOD2_OPCODE_JUMP,          0xFFFF,    0,     0,      0 }}};

tpm_mod_flag_bm_entry_t tpm_mod2_jump_skip2_pattern =
/*=============================================================================================*/
/*  <skip_before><skip_no_parse><mask><process_function>    <opcode>                <mod_data><last><ip4updt><L4updt>*/
{        0,             0,        0,         NULL,     {{TPM_MOD2_OPCODE_JUMP_SKIP2,    0xFFFF,    0,     0,      0 }}};

tpm_mod_flag_bm_entry_t tpm_mod2_jump_subr_pattern =
/*=============================================================================================*/
/*  <skip_before><skip_no_parse><mask><process_function>    <opcode>                <mod_data><last><ip4updt><L4updt>*/
{        0,             0,        0,         NULL,     {{TPM_MOD2_OPCODE_JUMP_SUBR,     0xFFFF,    0,     0,      0 }}};

tpm_mod_flag_bm_entry_t tpm_mod2_skip_pattern =
/*=============================================================================================*/
/*  <skip_before><skip_no_parse><mask><process_function>    <opcode>                <mod_data><last><ip4updt><L4updt>*/
{        0,             0,        0,         NULL,     {{TPM_MOD2_OPCODE_SKIP,          0xFFFF,    0,     0,      0 }}};

tpm_mod_flag_bm_entry_t tpm_mod2_ipv6_tc_pattern =
/*=============================================================================================*/
/*  <skip_before><skip_no_parse><mask>            <process_function>*/
{        2,             4,       TPM_DSCP_SET,    tpm_mod2_process_dscp_set,
/*         <opcode>                <mod_data><last><ip4updt><L4updt>*/
    {{TPM_MOD2_OPCODE_REP2,          0x6000,    0,     0,      0 },}
};

tpm_mod_flag_bm_entry_t tpm_mod2_ipv6_pppoe_pattern =
/*=============================================================================================*/
/*  <skip_before><skip_no_parse><mask>            <process_function>*/
{        2,             4,       TPM_DSCP_SET,    tpm_mod2_process_pppoe_add,
/*         <opcode>                <mod_data><last><ip4updt><L4updt>*/
    {{TPM_MOD2_OPCODE_REP2,          0x8864,    0,     0,      0 },
     {TPM_MOD2_OPCODE_ADD,           0x1100,    0,     0,      0 },
     {TPM_MOD2_OPCODE_ADD,           0x0000,    0,     0,      0 },
     {TPM_MOD2_OPCODE_ADD_CALC_LEN,  0x7000,    0,     0,      0 },
     {TPM_MOD2_OPCODE_ADD,           0x0057,    0,     0,      0 }}
};


tpm_mod_flag_bm_entry_t tpm_mod2_pattern_array[TPM_MAX_PKT_MOD_FLAGS] =
/*=============================================================================================*/
{
    /* TPM_MH_SET            -   Update the Marvell Header*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           0/*skip w/ jump*/, TPM_MH_SET,         tpm_mod2_process_mh_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_MAC_DA_SET     -   Update the MAC DA Address*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           6,        TPM_MAC_DA_SET,     tpm_mod2_process_da_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 },
         {TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 },
         {TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_MAC_SA_SET     -   Update the MAC SA Address*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           6,        TPM_MAC_SA_SET,     tpm_mod2_process_sa_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 },
         {TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 },
         {TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_VLAN_MOD       -   Update the VLAN Tags (add/del/update)*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           0,        TPM_VLAN_MOD,       tpm_mod2_process_vlan_mod,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_PPPOE_DEL      -   Delete a PPPoE encapsulation*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           0,        TPM_PPPOE_DEL,      tpm_mod2_process_pppoe_del,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,          0x0800,    0,     0,      0 },
         {TPM_MOD2_OPCODE_DEL,           0x0004,    0,     0,      0 }},
    },

    /* TPM_MOD_PPPOE_ADD      -   Add a PPPoE encapsulation*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           0,        TPM_PPPOE_ADD,      tpm_mod2_process_pppoe_add,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_PPPOE,          0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_DSCP_SET       -   Set the DSCP value */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0/* Don't care ety */,           2/* Don't care ety */,        TPM_DSCP_SET,       tpm_mod2_process_dscp_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP_LSB,        0x0300,    0,     1,      0 }},
    },

    /* TPM_MOD_TTL_DEC        -   Decrease the TTL value */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           8,        TPM_TTL_DEC,        tpm_mod2_process_ttl_dec,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_DEC_TTL_MSB,    0x0300,    0,     1,      0 }},
    },

    /* TPM_MOD_IPV4_UPDATE    -   Update the IPv4 Header*/
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           2,        TPM_IPV4_UPDATE,    tpm_mod2_process_ipv4_update,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP_IP_CHKSUM,  0x0000,    0,     0,      0 }},
    },

    /* TPM_MOD_IPV4_SRC_SET   -   Set the IPV4 Source Address */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           4,        TPM_IPV4_SRC_SET,   tpm_mod2_process_v4_sip_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     1,      1 },
         {TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     1,      1 }},
    },

    /* TPM_MOD_IPV4_DST_SET   -   Set the IPV4 Destination Address */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           4,         TPM_IPV4_DST_SET,   tpm_mod2_process_v4_dip_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     1,      1 },
         {TPM_MOD2_OPCODE_REP2,          0xFFFF,    0,     1,      1 }},
    },

    /* TPM_MOD_IPV6_UPDATE    -   Update the IPv6 Header */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           0,         TPM_IPV6_UPDATE,   NULL,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_HOPLIM_DEC     -   Decrease the Hop Limit value */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           6,        TPM_HOPLIM_DEC,     tpm_mod2_process_hoplim_dec,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_DEC_TTL_LSB,    0x0200,    0,     0,      0 }},
    },

    /* TPM_MOD_IPV6_SRC_SET   -   Set the IPV6 Source Address */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,          16,        TPM_IPV6_SRC_SET,   NULL,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_IPV6_DST_SET   -   Set the IPV6 Destination Address */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,          16,        TPM_IPV6_DST_SET,   NULL,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},
    },

    /* TPM_MOD_L4_SRC_SET     -   Set the L4 Source Port (UDP or TCP) */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           2,        TPM_L4_SRC_SET,     tpm_mod2_process_l4_src_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,           0xFFFF,    0,     0,      1 }},
    },

    /* TPM_MOD_L4_DST_SET     -   Set the L4 Destination Port (UDP or TCP) */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        0,           2,        TPM_L4_DST_SET,     tpm_mod2_process_l4_dst_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP2,           0xFFFF,    0,     0,      1 }},
    },

};

tpm_mod_flag_bm_entry_t tpm_mod2_int_pattern_array[TPM_MAX_INT_MOD_FLAGS] =
/*=============================================================================================*/
{
    /* TPM_MOD_INT_L4_TCP     -   Update TCP checksum */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {       12,          14,        TPM_INT_L4_TCP,     tpm_mod2_process_l4_checksum_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP_L4_CHKSUM,  0x0000,    0,     0,      0 }},
    },

    /* TPM_MOD_INT_L4_UDP     -   Update UDP checksum */
    /*  <skip_before><skip_no_parse><mask>              <process_function>         */
    {        2,           4,        TPM_INT_L4_UDP,     tpm_mod2_process_l4_checksum_set,
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_REP_L4_CHKSUM,  0x0000,    0,     0,      0 }},
    },
};

tpm_mod_vlan_entry_t tpm_mod2_vlan_pattern_array[VLANOP_MAX_NUM] =
/*=============================================================================================*/
{
    /* VLANOP_NOOP                 -        no VLAN operation performed */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
        {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},  /* VLAN 1 */

        {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},  /* VLAN 2 */
    },

    /* VLANOP_EXT_TAG_MOD          -        modify external tag */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 1 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_EXT_TAG_DEL          -        delete external tag */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_DEL,            0x0002,    0,     0,      0 }},   /* VLAN 1 */
    },

    /* VLANOP_EXT_TAG_INS          -        insert(prepend) external tag */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 1 */
        {TPM_MOD2_OPCODE_ADD_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_EXT_TAG_MOD_INS      -        modify existing external tag and insert(prepend) new tag */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 1 */
        {TPM_MOD2_OPCODE_ADD_VLAN,       0xFFFF,    0,     0,      0 }},

       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 2 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_INS_2TAG             -        insert(prepend) 2 new tags */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 1 */
        {TPM_MOD2_OPCODE_ADD_VLAN,       0xFFFF,    0,     0,      0 }},

       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 2 */
        {TPM_MOD2_OPCODE_ADD_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_MOD_2TAG             -        modify 2 tags */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 1 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},

       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 2 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_SWAP_TAGS            -        swap internal and external tags */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 1 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},

       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },    /* VLAN 2 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_DEL_2TAG             -        delete 2 existing tags */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_DEL,            0x0002,    0,     0,      0 }},   /* VLAN 1 */

       {{TPM_MOD2_OPCODE_DEL,            0x0002,    0,     0,      0 }},   /* VLAN 2 */
    },

    /* VLANOP_INT_TAG_MOD          -        modify existing internal tag */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_SKIP,           0x0004,    0,     0,      0 }},  /* VLAN 1 */

       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },   /* VLAN 2 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_EXT_TAG_DEL_INT_MOD  -        delete existing external tag and modify internal tag */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_DEL,            0x0002,    0,     0,      0 }},  /* VLAN 1 */

       {{TPM_MOD2_OPCODE_CONF_VLAN,      0xFFFF,    0,     0,      0 },   /* VLAN 2 */
        {TPM_MOD2_OPCODE_REP_VLAN,       0xFFFF,    0,     0,      0 }},
    },

    /* VLANOP_ILLEGAL              -        illegal VLAN operation */
    {
        /*    <opcode>                 <mod_data><last><ip4updt><L4updt>           */
       {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},  /* VLAN 1 */

       {{TPM_MOD2_OPCODE_NOOP,           0xFFFF,    0,     0,      0 }},  /* VLAN 2 */
    },
};

tpm_chain_map_t tpm_mod2_chain_map[] =
{
    {TPM_CHAIN_TYPE_L2,         TPM_VLAN_MOD,                                                                                                                       TPM_MOD2_DONT_CARE_INT_BM},

    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET,                                TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET,                                TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET,                 TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET,                 TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET,                TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET,                TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_SRC_SET | TPM_L4_SRC_SET, TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_NAPT,  TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE | TPM_IPV4_DST_SET | TPM_L4_DST_SET, TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                                 TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                  TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_ADD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                 TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                 TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_ADD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                  TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                  TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE,								       TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE,						       TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE,						       TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE,						       TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_ADD | TPM_TTL_DEC | TPM_IPV4_UPDATE,				       TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_ROUTE, TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE,				       TPM_MOD2_DONT_CARE_INT_BM},

    {TPM_CHAIN_TYPE_MH,         TPM_MH_SET,                                                                                                                         TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_L2,         TPM_MH_SET | TPM_VLAN_MOD,                                                                                                          TPM_MOD2_DONT_CARE_INT_BM},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MH_SET | TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                                                        TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MH_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                                         TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MAC_DA_SET | TPM_MH_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                       TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MAC_DA_SET | TPM_MH_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE,                        TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MAC_SA_SET | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                                                                     TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                                                      TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                                    TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV4_MC,    TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_TTL_DEC | TPM_IPV4_UPDATE,                                     TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                                                  TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                                   TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                 TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                  TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MH_SET | TPM_MAC_SA_SET | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                                     TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MH_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                                      TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MH_SET | TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                                    TPM_INT_MC_MOD},
    {TPM_CHAIN_TYPE_IPV6_MC,    TPM_MH_SET | TPM_MAC_DA_SET | TPM_MAC_SA_SET | TPM_VLAN_MOD | TPM_PPPOE_DEL | TPM_HOPLIM_DEC | TPM_IPV6_UPDATE,                     TPM_INT_MC_MOD},
};

#define TPM_CHAIN_MAP_SIZE    (sizeof(tpm_mod2_chain_map)/sizeof(tpm_chain_map_t))

/*******************************************************************************
                               Internal definitions
*******************************************************************************/

/*******************************************************************************
                               Internal functions
*******************************************************************************/

uint16_t tpm_mod2_get_jump_to(tpm_mod2_entry_t *pattern)
{
    if (pattern == NULL)
    {
        return TPM_MOD2_NULL_ENT_IDX;
    }

    if ((pattern->opcode == TPM_MOD2_OPCODE_JUMP) ||
        (pattern->opcode == TPM_MOD2_OPCODE_JUMP_SKIP2) ||
        (pattern->opcode == TPM_MOD2_OPCODE_JUMP_SUBR))
    {
        return pattern->data;
    }

    return TPM_MOD2_NULL_ENT_IDX;
}

tpm_pattern_entry_t * tpm_mod2_get_chain_from_pattern_set(tpm_pattern_entry_t *pattern_set, tpm_chain_type_t chain_type, uint16_t chain_idx)
{
    uint8_t set_idx, fst_free = TPM_MOD2_MAX_PATTERN_SETS;

    for (set_idx = 1; set_idx < TPM_MOD2_MAX_PATTERN_SETS; set_idx++)
    {
        if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
        {
            if (pattern_set[set_idx].is_valid == 0)
            {
                fst_free = set_idx;
                break;
            }
        }
        else if (pattern_set[set_idx].is_valid)
        {
            if ((TPM_AREA_TYPE_CHAIN == pattern_set[set_idx].area_type) &&
                (chain_type == pattern_set[set_idx].chain_type) &&
                (chain_idx ==  pattern_set[set_idx].index))
            {
                return &pattern_set[set_idx];
            }
        }
        else if (fst_free == TPM_MOD2_MAX_PATTERN_SETS)
        {
            fst_free = set_idx;
        }
    }

    if (chain_idx != TPM_MOD2_INVALID_CHAIN_ID)
    {
        return NULL;
    }

    if (fst_free != TPM_MOD2_MAX_PATTERN_SETS)
    {
        pattern_set[fst_free].is_valid   = 1;
        pattern_set[fst_free].area_type  = TPM_AREA_TYPE_CHAIN;
        pattern_set[fst_free].chain_type = chain_type;
    }

    return &pattern_set[fst_free];
}

/*******************************************************************************
* tpm_mod2_parse_chain_type()
*
* DESCRIPTION: The API is mapping mod bitmap into chain type - BM vs chain_type
*
* INPUTS:   mod_bm      - set of flags described which fields in the packet
*                         to be changed
*
* OUTPUTS:
*
* RETURNS:
*           chain_type
*
* COMMENTS:
*           None
*
*******************************************************************************/
tpm_chain_type_t tpm_mod2_parse_chain_type(tpm_pkt_mod_bm_t mod_bm, tpm_pkt_mod_int_bm_t int_mod_bm)
{
    uint32_t map_idx;

    for (map_idx = 0; map_idx < TPM_CHAIN_MAP_SIZE; map_idx++) {
        if (mod_bm == tpm_mod2_chain_map[map_idx].mod_bm) {
        	if (tpm_mod2_chain_map[map_idx].int_mod_bm == TPM_MOD2_DONT_CARE_INT_BM ||
				int_mod_bm == tpm_mod2_chain_map[map_idx].int_mod_bm) {
	            return tpm_mod2_chain_map[map_idx].chain_type;
        	}
        }
    }

    return TPM_CHAIN_TYPE_NONE;
}

/*******************************************************************************
* tpm_mod2_validate_logical_params()
*
* DESCRIPTION: The API is validating the params - BM vs mod_data
*
* INPUTS:   gmac_port   - GMAC port
*           mod_bm      - set of flags described which fields in the packet
*                         to be changed
*           mod_data    - modification entry data
*
* OUTPUTS:
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_validate_logical_params   (tpm_gmacs_enum_t   gmac_port,
                                               tpm_pkt_mod_bm_t   mod_bm,
                                               tpm_pkt_mod_int_bm_t int_mod_bm,
                                               tpm_pkt_mod_t     *mod_data,
                                               uint32_t          *mod_entry)
{
    tpm_chain_type_t chain_type;
    tpm_mac_key_t   *mac_data = &mod_data->mac_mod;
    tpm_pppoe_key_t *pppoe_data = &mod_data->pppoe_mod;
    uint8_t          forbid_dbl_tag_oper = 0;

    if (gmac_port >= TPM_MAX_NUM_GMACS)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: port %d \n", gmac_port);
        return TPM_FAIL;
    }

    if (mod_data == NULL ||
        mod_entry == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL pointer\n");
        return TPM_FAIL;
    }

    if (mod_bm == 0)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL bitmap\n");
        return TPM_FAIL;
    }

    chain_type = tpm_mod2_parse_chain_type(mod_bm, int_mod_bm);
    if (chain_type == TPM_CHAIN_TYPE_NONE)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: unsupported bitmap 0x%05x\n", mod_bm);
        return TPM_FAIL;
    }

    if (chain_type == TPM_CHAIN_TYPE_IPV4_NAPT ||
        chain_type == TPM_CHAIN_TYPE_IPV4_MC   ||
        chain_type == TPM_CHAIN_TYPE_ROUTE)
    {
        forbid_dbl_tag_oper = 1;
    }

    /* for every bit set in the BM - check that the mod_data has the values ok - not NULL pointers */
    if (mod_bm & TPM_MH_SET)
    {
        if (!tpm_db_mod2_get_multicast_mh_state())
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: MH disabled\n");
            return TPM_FAIL;
        }
        if (mod_data->mh_mod == 0)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: Zero MH_2B\n");
            return TPM_FAIL;
        }
    }

    if (mod_bm & TPM_MAC_DA_SET)
    {
        if (memcmp(mac_data->mac_da, null_mac_address, TPM_DA_LEN) == 0 ||
            memcmp(mac_data->mac_da_mask, null_mac_address, TPM_DA_LEN) == 0)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL DST MAC\n");
            return TPM_FAIL;
        }
    }

    if (mod_bm & TPM_MAC_SA_SET)
    {
        if (memcmp(mac_data->mac_sa, null_mac_address, TPM_SA_LEN) == 0 ||
            memcmp(mac_data->mac_sa_mask, null_mac_address, TPM_SA_LEN) == 0)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL SRC MAC\n");
            return TPM_FAIL;
        }
    }

    if (mod_bm & TPM_VLAN_MOD)
    {
        tpm_vlan_oper_t vlan_oper = mod_data->vlan_mod.vlan_op;
        tpm_vlan_key_t *v1_out = &mod_data->vlan_mod.vlan1_out, *v2_out = &mod_data->vlan_mod.vlan2_out;
#if 0
        /* Currently VLAN modification request cannot include other modifications */
        if (mod_bm != TPM_VLAN_MOD)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN modification BM (0x%x)\n", mod_bm);
            return TPM_FAIL;
        }
#endif
        if (vlan_oper >= VLANOP_ILLEGAL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN op. %d \n", vlan_oper);
            return TPM_FAIL;
        }

        if (!tpm_db_mod2_get_double_tag_state() || forbid_dbl_tag_oper)
        {
            if (vlan_oper == VLANOP_EXT_TAG_MOD_INS ||
                vlan_oper == VLANOP_INS_2TAG ||
                vlan_oper == VLANOP_MOD_2TAG ||
                vlan_oper == VLANOP_SWAP_TAGS ||
                vlan_oper == VLANOP_DEL_2TAG ||
                vlan_oper == VLANOP_INT_TAG_MOD ||
                vlan_oper == VLANOP_EXT_TAG_DEL_INT_MOD)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Unsupport double VLAN Operation: VLAN op. %d \n", vlan_oper);
                return TPM_FAIL;
            }
        }

		switch (vlan_oper) {

		case VLANOP_INS_2TAG:
			if ((v1_out->tpid == 0) || (v1_out->tpid_mask == 0)) {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: v2 TPID is NULL for add action\n");
                return TPM_FAIL;
			}
		case VLANOP_EXT_TAG_INS:
			if ((v1_out->tpid == 0) || (v1_out->tpid_mask == 0)) {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: v1 TPID is NULL for add action\n");
                return TPM_FAIL;
			}
			break;

		case VLANOP_EXT_TAG_MOD_INS:
		case VLANOP_MOD_2TAG:
		case VLANOP_SWAP_TAGS:
			if ((v2_out->tpid == 0) && (v2_out->tpid_mask == 0xffff)) {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: v2 TPID is NULL for modify new action\n");
                return TPM_FAIL;
			}
		case VLANOP_EXT_TAG_MOD:
			if ((v1_out->tpid == 0) && (v1_out->tpid_mask == 0xffff)) {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: v1 TPID is NULL for modify new action\n");
                return TPM_FAIL;
			}
			break;

		case VLANOP_INT_TAG_MOD:
		case VLANOP_EXT_TAG_DEL_INT_MOD:
			if ((v2_out->tpid == 0) && (v2_out->tpid_mask == 0xffff)) {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: v2 TPID is NULL for modify new action\n");
                return TPM_FAIL;
			}
			break;

		default:
			break;
		}

        if ( v1_out->vid_mask != TPM_MOD2_ORIGINAL_VID &&
             v1_out->vid_mask != TPM_MOD2_NEW_VID )
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN1 mask - %x\n", v1_out->vid_mask);
            return TPM_FAIL;
        }

        if ( v2_out->vid_mask != TPM_MOD2_ORIGINAL_VID &&
             v2_out->vid_mask != TPM_MOD2_NEW_VID )
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN2 mask - %x\n", v2_out->vid_mask);
            return TPM_FAIL;
        }

        if ( v1_out->cfi_mask != TPM_MOD2_ORIGINAL_CFI &&
             v1_out->cfi_mask != TPM_MOD2_NEW_CFI )
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN1 CFI mask - %x\n", v1_out->cfi_mask);
            return TPM_FAIL;
        }

        if ( v2_out->cfi_mask != TPM_MOD2_ORIGINAL_CFI &&
             v2_out->cfi_mask != TPM_MOD2_NEW_CFI )
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN2 CFI mask - %x\n", v2_out->cfi_mask);
            return TPM_FAIL;
        }

        if ( v1_out->pbit_mask != TPM_MOD2_ORIGINAL_PBIT &&
             v1_out->pbit_mask != TPM_MOD2_NEW_PBIT )
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN1 PBIT mask - %x\n", v1_out->pbit_mask);
            return TPM_FAIL;
        }

        if ( v2_out->pbit_mask != TPM_MOD2_ORIGINAL_PBIT &&
             v2_out->pbit_mask != TPM_MOD2_NEW_PBIT )
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: VLAN2 PBIT mask - %x\n", v2_out->pbit_mask);
            return TPM_FAIL;
        }
    }

    if ((mod_bm & TPM_PPPOE_DEL || mod_bm & TPM_PPPOE_ADD) &&
        !tpm_db_mod2_get_pppoe_add_mod_state())
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: PPPoE disabled\n");
        return TPM_FAIL;
    }

    if (mod_bm & TPM_PPPOE_DEL &&
        mod_bm & TPM_PPPOE_ADD)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: BM includes PPPoE add and del flags\n");
        return TPM_FAIL;
    }

    if (mod_bm & TPM_PPPOE_ADD)
    {
        if (pppoe_data->ppp_proto == 0 ||
            pppoe_data->ppp_session == 0)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL PPPoE data\n");
            return TPM_FAIL;
        }
    }

    if (mod_bm & TPM_IPV4_UPDATE)
    {
        tpm_ipv4_acl_key_t *ipv4_data = &(mod_data->l3.ipv4_mod);
        if (mod_bm & TPM_IPV4_DST_SET)
        {
            if (memcmp(ipv4_data->ipv4_dst_ip_add, null_ipv4_address, TPM_IPV4_SIP_LEN) == 0 ||
                memcmp(ipv4_data->ipv4_dst_ip_add_mask, null_ipv4_address, TPM_IPV4_SIP_LEN) == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL DST IPv4 data\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_IPV4_SRC_SET)
        {
            if (memcmp(ipv4_data->ipv4_src_ip_add, null_ipv4_address, TPM_IPV4_SIP_LEN) == 0 ||
                memcmp(ipv4_data->ipv4_src_ip_add_mask, null_ipv4_address, TPM_IPV4_SIP_LEN) == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL SRC IPv4 data\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_DSCP_SET)
        {
            if (ipv4_data->ipv4_dscp == 0 ||
                ipv4_data->ipv4_dscp_mask == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL IPv4 DSCP data\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_L4_DST_SET)
        {
            if (ipv4_data->l4_dst_port == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL L4 DST port\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_L4_SRC_SET)
        {
            if (ipv4_data->l4_src_port == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL L4 SRC port\n");
                return TPM_FAIL;
            }
        }
    }
    else
    {
        if (mod_bm & TPM_IPV4_DST_SET ||
            mod_bm & TPM_IPV4_SRC_SET ||
            mod_bm & TPM_TTL_DEC)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid flags: TPM_IPV4_UPDATE should be set\n");
            return TPM_FAIL;
        }
    }

    if (mod_bm & TPM_IPV6_UPDATE)
    {
        tpm_ipv6_acl_key_t *ipv6_data = &mod_data->l3.ipv6_mod;
        if (mod_bm & TPM_IPV6_DST_SET)
        {
            if (memcmp(ipv6_data->ipv6_dst_ip_add, null_ipv6_address, TPM_IPV6_ADDR_LEN) == 0 ||
                memcmp(ipv6_data->ipv6_dst_ip_add_mask, null_ipv6_address, TPM_IPV6_ADDR_LEN) == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL DST IPv6 data\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_IPV6_SRC_SET)
        {
            if (memcmp(ipv6_data->ipv6_src_ip_add, null_ipv6_address, TPM_IPV6_ADDR_LEN) == 0 ||
                memcmp(ipv6_data->ipv6_src_ip_add_mask, null_ipv6_address, TPM_IPV6_ADDR_LEN) == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL SRC IPv6 data\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_DSCP_SET)
        {
            if (ipv6_data->ipv6_dscp == 0 ||
                ipv6_data->ipv6_dscp_mask == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL IPv6 DSCP data\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_L4_DST_SET)
        {
            if (ipv6_data->l4_dst_port == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL L6 DST port\n");
                return TPM_FAIL;
            }
        }

        if (mod_bm & TPM_L4_SRC_SET)
        {
            if (ipv6_data->l4_src_port == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL L6 SRC port\n");
                return TPM_FAIL;
            }
        }
    }
    else
    {
        if (mod_bm & TPM_IPV6_DST_SET ||
            mod_bm & TPM_IPV6_SRC_SET ||
            mod_bm & TPM_HOPLIM_DEC)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid flags: TPM_IPV6_UPDATE should be set\n");
            return TPM_FAIL;
        }
    }

    if (mod_bm & TPM_DSCP_SET     &&
        !(mod_bm & TPM_IPV6_UPDATE) &&
        !(mod_bm & TPM_IPV4_UPDATE))
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid flags: TPM_IPV4_UPDATE or TPM_IPV6_UPDATE should be set\n");
        return TPM_FAIL;
    }

    if (mod_bm & TPM_L4_SRC_SET   ||
        mod_bm & TPM_L4_DST_SET   ||
        mod_bm & TPM_IPV6_SRC_SET ||
        mod_bm & TPM_IPV6_DST_SET ||
        mod_bm & TPM_IPV4_SRC_SET ||
        mod_bm & TPM_IPV4_DST_SET)
    {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Internal BM verification (0x%x)\n", int_mod_bm);

        if (int_mod_bm == 0)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL internal modification BM \n");
            return TPM_FAIL;
        }
#if 0
        else if (int_mod_bm & TPM_INT_2_VLANS)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: TPM_INT_2_VLANS should NOT be set \n");
            return TPM_FAIL;
        }
#endif
    }

    if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) && (int_mod_bm & TPM_INT_SPLIT_MOD))
    {
        if (mod_bm == TPM_VLAN_MOD)
        {
            if ((mod_data->vlan_mod.vlan_op != VLANOP_EXT_TAG_MOD) &&
                (mod_data->vlan_mod.vlan_op != VLANOP_EXT_TAG_INS) &&
                (mod_data->vlan_mod.vlan_op != VLANOP_EXT_TAG_MOD_INS) &&
                (mod_data->vlan_mod.vlan_op != VLANOP_NOOP))
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: vlan_op should be one of VLANOP_EXT_TAG_MOD, VLANOP_EXT_TAG_INS and VLANOP_EXT_TAG_MOD_INS. \n");
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: TPM_VLAN_MOD should be set. \n");
            return TPM_FAIL;
        }
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_single_command_add()
*
*******************************************************************************/
static int32_t tpm_mod2_single_command_add(tpm_mod_flag_bm_entry_t *cmd_pattern,
                                           uint16_t                data,
                                           tpm_pattern_entry_t    *entry_p)
{
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), &(cmd_pattern->entry), sizeof(tpm_mod2_entry_t));
        entry_p->line[entry_p->line_num++].data = data;

        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added command 0x%x (data=0x%x pattern_line=%d)\n",
                    cmd_pattern->entry[0].opcode, data, entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_skip_command_add()
*
*******************************************************************************/
static int32_t  tpm_mod2_skip_command_add (uint8_t *skip,
                                           uint8_t              skip_before,
                                           tpm_pattern_entry_t *entry_p)
{
    if (*skip || skip_before)
    {
        /* Add SKIP */
        if (tpm_mod2_single_command_add(&tpm_mod2_skip_pattern, (*skip + skip_before), entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }

        *skip = 0;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_skip_to_l3_off_command_add()
*
*******************************************************************************/
static int32_t  tpm_mod2_skip_to_l3_offset_command_add (uint8_t *skip,
                                           uint8_t              mod_data,
                                           tpm_pattern_entry_t *entry_p)
{
    /* Add SKIP */
    if (tpm_mod2_single_command_add(&tpm_mod2_skip_pattern,
                                    ((0x2 << 12)/*add l3_offset*/ | (0x2 << 8)/*sub mod_data*/ | mod_data),
                                    entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    *skip = 0;

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_fill_in_vlan_data()
*
*******************************************************************************/
uint32_t tpm_mod2_fill_in_vlan_data (tpm_vlan_key_t      *vlan_data)
{
    uint32_t tpm_data = 0;

    tpm_data  = (vlan_data->vid << TPM_MOD2_ENTRY_VID_OFFS) & TPM_MOD2_ENTRY_VID_MASK;
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "tpm_data 1: 0x%4.4x (vid 0x%x mask 0x%x)\n",
                 tpm_data, vlan_data->vid, vlan_data->vid_mask);

    tpm_data |= (vlan_data->cfi << TPM_MOD2_ENTRY_CFI_OFFS) & TPM_MOD2_ENTRY_CFI_MASK;
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "tpm_data 2: 0x%4.4x (cfi 0x%x mask 0x%x)\n",
                 tpm_data, vlan_data->cfi, vlan_data->cfi_mask);

    tpm_data |= (vlan_data->pbit << TPM_MOD2_ENTRY_PBIT_OFFS) & TPM_MOD2_ENTRY_PBIT_MASK;
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "tpm_data 3: 0x%4.4x (pbit 0x%x mask 0x%x)\n",
                 tpm_data, vlan_data->pbit, vlan_data->pbit_mask);

    return (tpm_data & TPM_MOD2_ENTRY_MOD_DATA_MASK);
}

/*******************************************************************************
* tpm_mod2_process_mh_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_mh_set (tpm_gmacs_enum_t      gmac_port,
                                 uint32_t                bm,
                                 uint16_t                cmd_idx,
                                 uint8_t                *skip,
                                 tpm_pkt_mod_t          *mod_data,
                                 tpm_mod_pattern_data_t *pattern_data,
                                 tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t    tmp_pattern[TPM_MOD2_MH_CMD_ENTRIES];
    uint16_t            chain_idx = TPM_MOD2_INVALID_CHAIN_ID, main_chain_id = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t            jump_to;

    memset(tmp_pattern, 0, sizeof(tmp_pattern));

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    /* Build pattern entries */
    memcpy(tmp_pattern, tpm_mod2_pattern_array[cmd_idx].entry, sizeof(tmp_pattern));
    tmp_pattern[0].data = mod_data->mh_mod;
    tmp_pattern[0].last = 1;

    spin_lock_bh(&tpmMod2ChainEntryLock);
    /* Scan mh db to check if the MH subchain exist or not. */
    chain_idx = tpm_db_mod2_get_chain_entry(gmac_port, TPM_CHAIN_TYPE_MH, TPM_MOD2_MH_CMD_ENTRIES, tmp_pattern);
    if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
    {
        if(is_tpm_check) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "TPM check sub chain-%d failed\n", TPM_CHAIN_TYPE_MH);
            return TPM_FAIL;
        }

        if (pattern_data->main_chain_type == TPM_CHAIN_TYPE_MH)
        {
            chain_idx = pattern_data->main_chain_id;
        }
        else
        {
            /* Get next free chain index of MH subchain area */
            chain_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, TPM_CHAIN_TYPE_MH);
            if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
            {
                spin_unlock_bh(&tpmMod2ChainEntryLock);
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the mac subchain \n");
                return TPM_FAIL;
            }
        }
    }
    else
    {
        if (pattern_data->main_chain_type == TPM_CHAIN_TYPE_MH)
        {
            main_chain_id = chain_idx;
        }
    }
    /* Write to mh db */
    if(!is_tpm_check) {
        if (tpm_db_mod2_set_chain_entry(gmac_port, TPM_CHAIN_TYPE_MH, chain_idx,
                                        TPM_MOD2_MH_CMD_ENTRIES, tmp_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set mh subr chain entry\n");
            return TPM_FAIL;
        }
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    /* Fill MH entries in local pattern set */
    if (pattern_data->main_chain_type == TPM_CHAIN_TYPE_MH)
    {
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
        if (main_chain_id != TPM_MOD2_INVALID_CHAIN_ID)
        {
            tpm_db_mod2_rollback_chain_entry(gmac_port, TPM_CHAIN_TYPE_MH, pattern_data->main_chain_id, false);
            pattern_data->main_chain_id = main_chain_id;
        }
    }
    else
    {
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, TPM_CHAIN_TYPE_MH, TPM_MOD2_INVALID_CHAIN_ID);
    }
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set. \n");
        return TPM_FAIL;
    }
    entry_p->index = chain_idx;

    if ((entry_p->line_num + TPM_MOD2_MH_CMD_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tmp_pattern,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_MH_CMD_ENTRIES);
        entry_p->line_num += TPM_MOD2_MH_CMD_ENTRIES;
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* Update main chain */
    if (pattern_data->main_chain_type != TPM_CHAIN_TYPE_MH)
    {
        /* Convert Chain Index to PMT Index */
        jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(TPM_CHAIN_TYPE_MH, chain_idx);
        if (jump_to == TPM_MOD2_NULL_ENT_IDX)
        {
            TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
            return TPM_FAIL;
        }
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
        if (entry_p == NULL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
            return TPM_FAIL;
        }
        if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
        {
            if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
            {
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
            return TPM_FAIL;
        }
    }

    return TPM_OK;
}


/*******************************************************************************
* tpm_mod2_process_da_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_da_set (tpm_gmacs_enum_t      gmac_port,
                                 uint32_t                bm,
                                 uint16_t                cmd_idx,
                                 uint8_t                *skip,
                                 tpm_pkt_mod_t          *mod_data,
                                 tpm_mod_pattern_data_t *pattern_data,
                                 tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t    tmp_pattern[TPM_MOD2_MAC_CMD_ENTRIES];
    uint16_t            chain_idx = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t            jump_to;
    uint8_t             *mac_array = NULL;

    memset(tmp_pattern, 0, sizeof(tmp_pattern));

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    /* Skip to DA field */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* Build pattern entries */
    memcpy(tmp_pattern, tpm_mod2_pattern_array[cmd_idx].entry, sizeof(tmp_pattern));

    mac_array = mod_data->mac_mod.mac_da;

    tmp_pattern[0].data = (mac_array[0] << 8) | mac_array[1];
    tmp_pattern[1].data = (mac_array[2] << 8) | mac_array[3];
    tmp_pattern[2].data = (mac_array[4] << 8) | mac_array[5];
    tmp_pattern[2].last = 1;

    spin_lock_bh(&tpmMod2ChainEntryLock);
    /* Scan mac db to check if the mac subchain exist or not. */
    chain_idx = tpm_db_mod2_get_chain_entry(gmac_port, TPM_CHAIN_TYPE_MAC, TPM_MOD2_MAC_CMD_ENTRIES, tmp_pattern);
    if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
    {
        if(is_tpm_check) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "TPM check sub chain-%d failed\n", TPM_CHAIN_TYPE_MAC);
            return TPM_FAIL;
        }
        /* Get next free chain index of mac subchain area */
        chain_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, TPM_CHAIN_TYPE_MAC);
        if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the mac subchain \n");
            return TPM_FAIL;
        }
    }
    /* Write to mac db */
    if(!is_tpm_check) {
        if (tpm_db_mod2_set_chain_entry(gmac_port, TPM_CHAIN_TYPE_MAC, chain_idx,
	                                TPM_MOD2_MAC_CMD_ENTRIES, tmp_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set mac subr chain entry\n");
            return TPM_FAIL;
        }
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    /* Fill DA entries in local pattern set */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, TPM_CHAIN_TYPE_MAC, TPM_MOD2_INVALID_CHAIN_ID);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set. \n");
        return TPM_FAIL;
    }
    entry_p->index = chain_idx;

    if ((entry_p->line_num + TPM_MOD2_MAC_CMD_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tmp_pattern,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_MAC_CMD_ENTRIES);
        entry_p->line_num += TPM_MOD2_MAC_CMD_ENTRIES;
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the mac.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* Add jump/jump_subr command in main_chain */
    /* Convert Chain Index to PMT Index */
    jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(TPM_CHAIN_TYPE_MAC, chain_idx);
    if (jump_to == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
        return TPM_FAIL;
    }
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }
    if (pattern_data->main_chain_type == TPM_CHAIN_TYPE_L2)
    {
        if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
        {
            if (tpm_mod2_single_command_add(&tpm_mod2_jump_pattern, jump_to, entry_p) != TPM_OK)
            {
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
            return TPM_FAIL;
        }
    }
    else
    {
        if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
        {
            if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
            {
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
            return TPM_FAIL;
        }
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_sa_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_sa_set (tpm_gmacs_enum_t      gmac_port,
                                 uint32_t                bm,
                                 uint16_t                cmd_idx,
                                 uint8_t                *skip,
                                 tpm_pkt_mod_t          *mod_data,
                                 tpm_mod_pattern_data_t *pattern_data,
                                 tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t    tmp_pattern[TPM_MOD2_MAC_CMD_ENTRIES];
    uint16_t            chain_idx = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t            jump_to;
    uint8_t             *mac_array = NULL;


    memset(tmp_pattern, 0, sizeof(tmp_pattern));

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    /* Skip to SA field */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }
    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* Build pattern entries */
    memcpy(tmp_pattern, tpm_mod2_pattern_array[cmd_idx].entry, sizeof(tmp_pattern));

    mac_array = mod_data->mac_mod.mac_sa;

    tmp_pattern[0].data = (mac_array[0] << 8) | mac_array[1];
    tmp_pattern[1].data = (mac_array[2] << 8) | mac_array[3];
    tmp_pattern[2].data = (mac_array[4] << 8) | mac_array[5];
    tmp_pattern[2].last = 1;


    spin_lock_bh(&tpmMod2ChainEntryLock);
    /* Scan mac db to check if the mac subchain exist or not. */
    chain_idx = tpm_db_mod2_get_chain_entry(gmac_port, TPM_CHAIN_TYPE_MAC, TPM_MOD2_MAC_CMD_ENTRIES, tmp_pattern);
    if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
    {
        if(is_tpm_check) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "TPM check sub chain-%d failed\n", TPM_CHAIN_TYPE_MAC);
            return TPM_FAIL;
        }
        /* Get next free chain index of mac subchain area */
        chain_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, TPM_CHAIN_TYPE_MAC);
        if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the mac subchain \n");
            return TPM_FAIL;
        }
    }
    /* Write to mac db */
    if(!is_tpm_check) {
        if (tpm_db_mod2_set_chain_entry(gmac_port, TPM_CHAIN_TYPE_MAC, chain_idx,
	                                TPM_MOD2_MAC_CMD_ENTRIES, tmp_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set mac subr chain entry\n");
            return TPM_FAIL;
        }
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    /* Fill SA entries in local pattern set */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, TPM_CHAIN_TYPE_MAC, TPM_MOD2_INVALID_CHAIN_ID);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set. \n");
        return TPM_FAIL;
    }
    entry_p->index = chain_idx;

    if ((entry_p->line_num + TPM_MOD2_MAC_CMD_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tmp_pattern,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_MAC_CMD_ENTRIES);
        entry_p->line_num += TPM_MOD2_MAC_CMD_ENTRIES;
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the mac.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* add jump/jump_subr command in main_chain */
    /* Convert Chain Index to PMT Index */
    jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(TPM_CHAIN_TYPE_MAC, chain_idx);
    if (jump_to == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
        return TPM_FAIL;
    }
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }
    if (pattern_data->main_chain_type == TPM_CHAIN_TYPE_L2)
    {
        if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
        {
            if (tpm_mod2_single_command_add(&tpm_mod2_jump_pattern, jump_to, entry_p) != TPM_OK)
            {
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
            return TPM_FAIL;
        }
    }
    else
    {
        if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
        {
            if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
            {
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
            return TPM_FAIL;
        }
    }

    return TPM_OK;
}


/*******************************************************************************
* tpm_mod2_configured_vlan_oper_set()
*
*******************************************************************************/
uint16_t tpm_mod2_configured_vlan_oper_set (tpm_gmacs_enum_t      gmac_port,
                                           tpm_mod2_entry_t       *patt_entry,
                                           tpm_vlan_key_t         *vlan_data,
                                           uint16_t               *conf_data,
                                           tpm_mod_pattern_data_t *pattern_data,
                                           tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t    tmp_pattern[TPM_MOD2_MAX_VLAN_ENTRIES];
    uint16_t            chain_idx = TPM_MOD2_INVALID_CHAIN_ID;

    memset(tmp_pattern, 0, sizeof(tmp_pattern));

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    /* Build pattern entries */
    memcpy(tmp_pattern, patt_entry, sizeof(tmp_pattern));
    if (conf_data)
    {
        tmp_pattern[0].data = *conf_data;
    }
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d: 0x%4.4x, 0x%4.4x\n",
                 1, tmp_pattern[0].opcode, tmp_pattern[0].data);
    if (vlan_data)
    {
        tmp_pattern[1].data = tpm_mod2_fill_in_vlan_data(vlan_data);
    }
    if (conf_data == NULL || patt_entry->opcode == TPM_MOD2_OPCODE_SKIP)
    {
        tmp_pattern[0].last = 1;
    }
    else
    {
        tmp_pattern[1].last = 1;
    }
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d: 0x%4.4x, 0x%4.4x\n",
                 1, tmp_pattern[0].opcode, tmp_pattern[0].data);
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d: 0x%4.4x, 0x%4.4x\n",
                 2, tmp_pattern[1].opcode, tmp_pattern[1].data);

    spin_lock_bh(&tpmMod2ChainEntryLock);
    /* Scan vlan db to check if the vlan subchain exist or not. */
    chain_idx = tpm_db_mod2_get_chain_entry(gmac_port, TPM_CHAIN_TYPE_VLAN, TPM_MOD2_MAX_VLAN_ENTRIES, tmp_pattern);
    if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
    {
        if(is_tpm_check) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "sub chain-%d check failed\n",TPM_CHAIN_TYPE_VLAN);
            return TPM_MOD2_INVALID_CHAIN_ID;
        }
        /* Get next free chain index of vlan subchain area */
        chain_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, TPM_CHAIN_TYPE_VLAN);
        if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the vlan subchain \n");
            return TPM_MOD2_INVALID_CHAIN_ID;
        }
    }
    /* Write to vlan db */
    if(!is_tpm_check) {
        if (tpm_db_mod2_set_chain_entry(gmac_port, TPM_CHAIN_TYPE_VLAN, chain_idx,
	                                TPM_MOD2_MAX_VLAN_ENTRIES, tmp_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set vlan subr chain entry\n");
            return TPM_FAIL;
        }
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    /* Fill VLAN entries in local pattern set */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, TPM_CHAIN_TYPE_VLAN, TPM_MOD2_INVALID_CHAIN_ID);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set. \n");
        return TPM_FAIL;
    }
    entry_p->index = chain_idx;

    if ((entry_p->line_num + TPM_MOD2_MAX_VLAN_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tmp_pattern,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_MAX_VLAN_ENTRIES);
        entry_p->line_num += TPM_MOD2_MAX_VLAN_ENTRIES;
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the vlan.pattern (%d)\n", entry_p->line_num);
        return TPM_MOD2_INVALID_CHAIN_ID;
    }

    return chain_idx;
}

/*******************************************************************************
* tpm_mod2_config_vlan1_data_get()
*
*******************************************************************************/
int32_t tpm_mod2_config_tpid (uint8_t is_vlan1, uint16_t tpid, uint16_t tpid_mask, uint16_t *conf_data)
{
	int32_t     rc;
	uint32_t    eth_type_sel;

	if (tpid_mask == TPM_MOD2_NEW_TPID) {
		rc = tpm_mod2_tpid_get(tpid, &eth_type_sel);

		if (rc != TPM_OK) {
			TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get TPID selector (0x%x)\n", tpid);
			return rc;
		}
	} else {
		if (is_vlan1 != 0)
			eth_type_sel = TPM_MOD2_TP_FROM_VLAN_1;
		else
			eth_type_sel = TPM_MOD2_TP_FROM_VLAN_2;
	}

	*conf_data = eth_type_sel;
	return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_config_vlan1_data_get()
*
*******************************************************************************/
int32_t tpm_mod2_config_vlan1_data_get (tpm_vlan_key_t *vlan_data, uint16_t *cdata)
{
    uint16_t conf_data;
    int32_t  rc;

    *cdata = 0;

    rc = tpm_mod2_config_tpid(1, vlan_data->tpid, vlan_data->tpid_mask, &conf_data);

    if (rc != TPM_OK)
        return rc;

    if (vlan_data->vid_mask == TPM_MOD2_NEW_VID)
        conf_data |= TPM_MOD2_VID_NEW;
    else
        conf_data |= TPM_MOD2_VID_FROM_VLAN_1;

    if (vlan_data->cfi_mask == TPM_MOD2_NEW_CFI)
        conf_data |= TPM_MOD2_CFI_NEW;
    else
        conf_data |= TPM_MOD2_CFI_FROM_VLAN_1;

    if (vlan_data->pbit_mask == TPM_MOD2_NEW_PBIT)
        conf_data |= TPM_MOD2_PBIT_NEW;
    else
        conf_data |= TPM_MOD2_PBIT_FROM_VLAN_1;

    *cdata = conf_data;
    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_config_vlan2_data_get()
*
*******************************************************************************/
int32_t tpm_mod2_config_vlan2_data_get (tpm_vlan_key_t *vlan_data, uint16_t *cdata)
{
    uint16_t conf_data;
    int32_t  rc;

    *cdata = 0;

    rc = tpm_mod2_config_tpid(0, vlan_data->tpid, vlan_data->tpid_mask, &conf_data);

    if (rc != TPM_OK)
        return rc;

    if (vlan_data->vid_mask == TPM_MOD2_NEW_VID)
        conf_data |= TPM_MOD2_VID_NEW;
    else
        conf_data |= TPM_MOD2_VID_FROM_VLAN_2;

    if (vlan_data->cfi_mask == TPM_MOD2_NEW_CFI)
        conf_data |= TPM_MOD2_CFI_NEW;
    else
        conf_data |= TPM_MOD2_CFI_FROM_VLAN_2;

    if (vlan_data->pbit_mask == TPM_MOD2_NEW_PBIT)
        conf_data |= TPM_MOD2_PBIT_NEW;
    else
        conf_data |= TPM_MOD2_PBIT_FROM_VLAN_2;

    *cdata = conf_data;
    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_vlan_mod()
*
*******************************************************************************/
int32_t tpm_mod2_process_vlan_mod     (tpm_gmacs_enum_t      gmac_port,
                                       uint32_t                bm,
                                       uint16_t                cmd_idx,
                                       uint8_t                *skip,
                                       tpm_pkt_mod_t          *mod_data,
                                       tpm_mod_pattern_data_t *pattern_data,
                                       tpm_self_check_enable_t is_tpm_check)
{
    uint16_t    config_data;
    uint16_t    v1_chain_id = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t    v2_chain_id = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t    jump_to;
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    /* Skip to VLAN field */
    if (mod_data->vlan_mod.vlan_op != VLANOP_NOOP)
    {
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
        if (entry_p == NULL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
            return TPM_FAIL;
        }
        if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
        {
            if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
            {
                return TPM_FAIL;
            }
        }
        else
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
            return TPM_FAIL;
        }
    } else {
    	return TPM_OK;
    }

    switch(mod_data->vlan_mod.vlan_op)
    {
    case VLANOP_EXT_TAG_MOD:
    /***********************/
        /* Add VLAN modification commands for VLAN1 */
        if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan1_out), &config_data) == TPM_OK)
        {
            v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_MOD].vlan1,
                                                            &(mod_data->vlan_mod.vlan1_out),
                                                            &config_data, pattern_data, is_tpm_check);
        }
        break;

    case VLANOP_EXT_TAG_DEL:
    /***********************/
        /* Add VLAN delete command for VLAN1 */
        v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port, tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_DEL].vlan1,
                                                        NULL, NULL, pattern_data, is_tpm_check);
        break;

    case VLANOP_EXT_TAG_INS:
    /***********************/
        /* Add VLAN add commands for VLAN1 */
        if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan1_out), &config_data) == TPM_OK)
        {
            v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_INS].vlan1,
                                                           &(mod_data->vlan_mod.vlan1_out),
                                                           &config_data, pattern_data, is_tpm_check);
        }
        break;

    case VLANOP_EXT_TAG_MOD_INS:
    /***********************/
        /* Add VLAN add commands for VLAN1 */
        if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan1_out), &config_data) == TPM_OK)
        {
            v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_MOD_INS].vlan1,
                                                           &(mod_data->vlan_mod.vlan1_out),
                                                           &config_data, pattern_data, is_tpm_check);

            /* Add VLAN modification commands for VLAN2 */
            if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan2_out), &config_data) == TPM_OK)
            {
                v2_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                                tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_MOD_INS].vlan2,
                                                                &(mod_data->vlan_mod.vlan2_out),
                                                                &config_data, pattern_data, is_tpm_check);
            }
            else
            {
                return TPM_FAIL;
            }
        }
        break;

    case VLANOP_INS_2TAG:
    /***********************/
        /* Add VLAN add commands for VLAN1 */
        if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan1_out), &config_data) == TPM_OK)
        {
            v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_INS_2TAG].vlan1,
                                                            &(mod_data->vlan_mod.vlan1_out),
                                                            &config_data, pattern_data, is_tpm_check);

            /* Add VLAN add commands for VLAN2 */
            if (tpm_mod2_config_vlan2_data_get(&(mod_data->vlan_mod.vlan2_out), &config_data) == TPM_OK)
            {
                v2_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                                tpm_mod2_vlan_pattern_array[VLANOP_INS_2TAG].vlan2,
                                                                &(mod_data->vlan_mod.vlan2_out),
                                                                &config_data, pattern_data, is_tpm_check);
            }
            else
            {
                return TPM_FAIL;
            }
        }
        break;

    case VLANOP_MOD_2TAG:
    /***********************/
        /* Add VLAN modification commands for VLAN1 */
        if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan1_out), &config_data) == TPM_OK)
        {
            v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_MOD_2TAG].vlan1,
                                                            &(mod_data->vlan_mod.vlan1_out),
                                                            &config_data, pattern_data, is_tpm_check);

            /* Add VLAN modification commands for VLAN2 */
            if (tpm_mod2_config_vlan2_data_get(&(mod_data->vlan_mod.vlan2_out), &config_data) == TPM_OK)
            {
                v2_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                                tpm_mod2_vlan_pattern_array[VLANOP_MOD_2TAG].vlan2,
                                                                &(mod_data->vlan_mod.vlan2_out),
                                                                &config_data, pattern_data, is_tpm_check);
            }
            else
            {
                return TPM_FAIL;
            }
        }
        break;

    case VLANOP_SWAP_TAGS:
    /***********************/
        /* Add VLAN modification commands for VLAN1 */
        if (tpm_mod2_config_vlan2_data_get(&(mod_data->vlan_mod.vlan1_out), &config_data) == TPM_OK)
        {
        	if (mod_data->vlan_mod.vlan1_out.tpid_mask != TPM_MOD2_NEW_TPID)
			{
	        	config_data &= ~(0x7);
				config_data |= TPM_MOD2_TP_FROM_VLAN_1;
			}
            v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_SWAP_TAGS].vlan1,
                                                            &(mod_data->vlan_mod.vlan1_out),
                                                            &config_data, pattern_data, is_tpm_check);

            /* Add VLAN modification commands for VLAN2 */
            if (tpm_mod2_config_vlan1_data_get(&(mod_data->vlan_mod.vlan2_out), &config_data) == TPM_OK)
            {
	        	if (mod_data->vlan_mod.vlan2_out.tpid_mask != TPM_MOD2_NEW_TPID)
				{
		        	config_data &= ~(0x7);
					config_data |= TPM_MOD2_TP_FROM_VLAN_2;
				}
                v2_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                                tpm_mod2_vlan_pattern_array[VLANOP_SWAP_TAGS].vlan2,
                                                                &(mod_data->vlan_mod.vlan2_out),
                                                                &config_data, pattern_data, is_tpm_check);
            }
            else
            {
                return TPM_FAIL;
            }
        }
        break;

    case VLANOP_DEL_2TAG:
    /***********************/
        v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port, tpm_mod2_vlan_pattern_array[VLANOP_DEL_2TAG].vlan1,
                                                        NULL, NULL, pattern_data, is_tpm_check);
        v2_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port, tpm_mod2_vlan_pattern_array[VLANOP_DEL_2TAG].vlan2,
                                                        NULL, NULL, pattern_data, is_tpm_check);
        break;

    case VLANOP_INT_TAG_MOD:
    /***********************/
        /* SKIP to the inner VLAN tag */
        if (*skip)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "skip is not zero (%d)\n", *skip);
            return TPM_FAIL;
        }

        config_data = tpm_mod2_vlan_pattern_array[VLANOP_INT_TAG_MOD].vlan1[0].data;
        v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port, tpm_mod2_vlan_pattern_array[VLANOP_INT_TAG_MOD].vlan1,
                                                        NULL, &config_data, pattern_data, is_tpm_check);

        /* Add VLAN modification commands for VLAN2 */
        if (tpm_mod2_config_vlan2_data_get(&(mod_data->vlan_mod.vlan2_out), &config_data) == TPM_OK)
        {
            v2_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_INT_TAG_MOD].vlan2,
                                                           &(mod_data->vlan_mod.vlan2_out),
                                                           &config_data, pattern_data, is_tpm_check);
        }
        break;

    case VLANOP_EXT_TAG_DEL_INT_MOD:
    /***********************/
        /* Add VLAN delete command for VLAN1 */
        v1_chain_id = tpm_mod2_configured_vlan_oper_set(gmac_port, &(tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_DEL_INT_MOD].vlan1[0]),
                                                        NULL, NULL, pattern_data, is_tpm_check);

        /* Add VLAN modification commands for VLAN2 */
        if (tpm_mod2_config_vlan2_data_get(&(mod_data->vlan_mod.vlan2_out), &config_data) == TPM_OK)
        {
            v2_chain_id = tpm_mod2_configured_vlan_oper_set (gmac_port,
                                                            tpm_mod2_vlan_pattern_array[VLANOP_EXT_TAG_DEL_INT_MOD].vlan2,
                                                            &(mod_data->vlan_mod.vlan2_out),
                                                            &config_data, pattern_data, is_tpm_check);
        }
        break;

    default:
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Do nothing for %d operation \n", mod_data->vlan_mod.vlan_op);
        return TPM_OK;
    }

    if ((v1_chain_id == TPM_MOD2_INVALID_CHAIN_ID && v2_chain_id == TPM_MOD2_INVALID_CHAIN_ID) ||
        v1_chain_id == TPM_MOD2_INVALID_CHAIN_ID)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No VLAN chain is added \n");
        return TPM_FAIL;
    }

    /* Update main chain */
    if (v1_chain_id != TPM_MOD2_INVALID_CHAIN_ID)
    {
        /* Add jump/jump_subr command in main_chain */
        /* Convert Chain Index to PMT Index */
        jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(TPM_CHAIN_TYPE_VLAN, v1_chain_id);
        if (jump_to == TPM_MOD2_NULL_ENT_IDX)
        {
            TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
            return TPM_FAIL;
        }
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
        if (entry_p == NULL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
            return TPM_FAIL;
        }
        if (pattern_data->main_chain_type != TPM_CHAIN_TYPE_L2 || v2_chain_id != TPM_MOD2_INVALID_CHAIN_ID)
        {
            if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
            {
                if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
                {
                    return TPM_FAIL;
                }
            }
            else
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
                return TPM_FAIL;
            }
        }
        else
        {
            if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
            {
                if (tpm_mod2_single_command_add(&tpm_mod2_jump_pattern, jump_to, entry_p) != TPM_OK)
                {
                    return TPM_FAIL;
                }
            }
            else
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
                return TPM_FAIL;
            }
        }
    }

    if (v2_chain_id != TPM_MOD2_INVALID_CHAIN_ID)
    {
        //tpm_pattern_entry_t *v1_entry, *v2_entry;

        //v1_entry = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, TPM_CHAIN_TYPE_VLAN, v1_chain_id);
        //v2_entry = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, TPM_CHAIN_TYPE_VLAN, v2_chain_id);
        //if (v1_entry == NULL || v2_entry == NULL)
        //{
        //    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get v1 or v2 pattern set. \n");
        //    return TPM_FAIL;
        //}

        //if (v1_entry->line_num == v2_entry->line_num &&
        //    !memcmp(v1_entry->line, v2_entry->line,
        //            v1_entry->line_num * sizeof(tpm_mod2_entry_t)))
        //{
        //    v2_entry->index = v1_chain_id;
        //    jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(TPM_CHAIN_TYPE_VLAN, v1_chain_id);
        //    if (jump_to == TPM_MOD2_NULL_ENT_IDX)
        //    {
        //        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
        //        return TPM_FAIL;
        //    }
        //}
        //else
        //{
            /* Convert Chain Index to PMT Index */
            jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(TPM_CHAIN_TYPE_VLAN, v2_chain_id);
            if (jump_to == TPM_MOD2_NULL_ENT_IDX)
            {
                TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
                return TPM_FAIL;
            }
        //}

        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
        if (entry_p == NULL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
            return TPM_FAIL;
        }
        /* Add jump/jump_subr command in main_chain */
        if (pattern_data->main_chain_type == TPM_CHAIN_TYPE_L2)
        {
            if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
            {
                if (tpm_mod2_single_command_add(&tpm_mod2_jump_pattern, jump_to, entry_p) != TPM_OK)
                {
                    return TPM_FAIL;
                }
            }
            else
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
                return TPM_FAIL;
            }
        }
        else
        {
            if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
            {
                if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
                {
                    return TPM_FAIL;
                }
            }
            else
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
                return TPM_FAIL;
            }
        }
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_pppoe_del()
*
*******************************************************************************/
int32_t tpm_mod2_process_pppoe_del (tpm_gmacs_enum_t      gmac_port,
                                    uint32_t                bm,
                                    uint16_t                cmd_idx,
                                    uint8_t                *skip,
                                    tpm_pkt_mod_t          *mod_data,
                                    tpm_mod_pattern_data_t *pattern_data,
                                    tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t     tmp_pattern[TPM_MOD2_PPPOE_DEL_CMD_ENTRIES];
    uint16_t             chain_idx = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t             jump_to;
	tpm_chain_type_t     subr_chain = TPM_CHAIN_TYPE_PPPOE;

    memset(tmp_pattern, 0, sizeof(tmp_pattern));

	if (bm & TPM_IPV6_UPDATE)
		subr_chain = TPM_CHAIN_TYPE_IPV6_PPPOE;

    /* Build pattern entries */
    memcpy(tmp_pattern, tpm_mod2_pattern_array[cmd_idx].entry,
           sizeof(tpm_mod2_entry_t)*TPM_MOD2_PPPOE_DEL_CMD_ENTRIES);
    tmp_pattern[TPM_MOD2_PPPOE_DEL_CMD_ENTRIES-1].last = 1;

    spin_lock_bh(&tpmMod2ChainEntryLock);
    /*update IPV6 in pattern*/
    if (bm & TPM_IPV6_UPDATE)
        tmp_pattern[0].data = 0x86dd;

    /* Scan pppoe db to check if the pppoe subchain exist or not.  */
    chain_idx = tpm_db_mod2_get_chain_entry(gmac_port, subr_chain, TPM_MOD2_PPPOE_DEL_CMD_ENTRIES, tmp_pattern);
    if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
    {
    	/*If it is tpm check, subchain check failed, return*/
        if (is_tpm_check) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "TPM check sub chain-%d failed\n", subr_chain);
            return TPM_FAIL;
        }
        /* Get next free chain index of pppoe subchain area */
        chain_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, subr_chain);
        if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the pppoe subchain \n");
            return TPM_FAIL;
        }
    }
    /* Write to pppoe db */
    if(!is_tpm_check) {
        if (tpm_db_mod2_set_chain_entry(gmac_port, subr_chain, chain_idx,
	                                TPM_MOD2_PPPOE_DEL_CMD_ENTRIES, tmp_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set pppoe subr chain entry\n");
	        return TPM_FAIL;
	    }
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    /* Fill PPPoE DEL entries in local pattern set */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, subr_chain, TPM_MOD2_INVALID_CHAIN_ID);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set. \n");
        return TPM_FAIL;
    }
    entry_p->index = chain_idx;

    if ((entry_p->line_num + TPM_MOD2_PPPOE_DEL_CMD_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tmp_pattern,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_PPPOE_DEL_CMD_ENTRIES);
        entry_p->line_num += TPM_MOD2_PPPOE_DEL_CMD_ENTRIES;
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the pppoe.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* Add jump/jump_subr command in main_chain */
    /* Convert Chain Index to PMT Index */
    jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(subr_chain, chain_idx);
    if (jump_to == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
        return TPM_FAIL;
    }
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }
    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_pppoe_add()
*
*******************************************************************************/
int32_t tpm_mod2_process_pppoe_add (tpm_gmacs_enum_t      gmac_port,
                                    uint32_t                bm,
                                    uint16_t                cmd_idx,
                                    uint8_t                *skip,
                                    tpm_pkt_mod_t          *mod_data,
                                    tpm_mod_pattern_data_t *pattern_data,
                                    tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t    ipv4_pattern[TPM_MOD2_PPPOE_ADD_CMD_ENTRIES];
    tpm_mod2_entry_t    ipv6_pattern[TPM_MOD2_IPV6_PPPOE_ADD_CMD_ENTRIES];
    uint16_t            chain_idx = TPM_MOD2_INVALID_CHAIN_ID;
    uint16_t            jump_to;
	tpm_chain_type_t    subr_chain = TPM_CHAIN_TYPE_PPPOE;
    tpm_mod2_entry_t    *tmp_pattern = ipv4_pattern;
	uint32_t            entry_num = TPM_MOD2_PPPOE_ADD_CMD_ENTRIES;
    uint32_t  gmac_mh_en;


    memset(ipv4_pattern, 0, sizeof(ipv4_pattern));
    memset(ipv6_pattern, 0, sizeof(ipv6_pattern));

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

	if (bm & TPM_IPV6_UPDATE) {
		subr_chain = TPM_CHAIN_TYPE_IPV6_PPPOE;
		tmp_pattern = ipv6_pattern;
		entry_num = TPM_MOD2_IPV6_PPPOE_ADD_CMD_ENTRIES;
	}

    /* Build pattern entries */
	if (bm & TPM_IPV6_UPDATE) {
	    memcpy(tmp_pattern, tpm_mod2_ipv6_pppoe_pattern.entry,
	           entry_num * sizeof(tpm_mod2_entry_t));
	    tmp_pattern[2].data = mod_data->pppoe_mod.ppp_session;
	    tpm_db_gmac_mh_en_conf_get(gmac_port, &gmac_mh_en);
	    if (!gmac_mh_en)
	        tmp_pattern[3].data = TPM_MOD2_DEFAULT_PPPOE_LEN_MH_DIS;
	} else {
	    memcpy(tmp_pattern, tpm_mod2_pattern_array[cmd_idx].entry,
	           entry_num * sizeof(tpm_mod2_entry_t));
	    tmp_pattern[0].data = mod_data->pppoe_mod.ppp_session;
	}
    tmp_pattern[entry_num - 1].last = 1;

    spin_lock_bh(&tpmMod2ChainEntryLock);
    /* Scan pppoe db to check if the pppoe subchain exist or not. */
    chain_idx = tpm_db_mod2_get_chain_entry(gmac_port, subr_chain, entry_num, tmp_pattern);
    if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
    {
        if(is_tpm_check) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "TPM check sub chain-%d failed\n", subr_chain);
            return TPM_FAIL;
        }
        /* Get next free chain index of pppoe subchain area */
        chain_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, subr_chain);
        if (chain_idx == TPM_MOD2_INVALID_CHAIN_ID)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the pppoe subchain \n");
            return TPM_FAIL;
        }
    }
    /* Write to pppoe db */
    if(!is_tpm_check) {
        if (tpm_db_mod2_set_chain_entry(gmac_port, subr_chain, chain_idx,
	                                entry_num, tmp_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set pppoe subr chain entry\n");
            return TPM_FAIL;
        }
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    /* Fill PPPoE ADD entries in local pattern set */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, subr_chain, TPM_MOD2_INVALID_CHAIN_ID);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set. \n");
        return TPM_FAIL;
    }
    entry_p->index = chain_idx;

    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tmp_pattern,
               entry_num * sizeof(tpm_mod2_entry_t));
        entry_p->line_num += entry_num;
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the pppoe.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    /* Add jump/jump_subr command in main_chain */
    /* Convert Chain Index to PMT Index */
    jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(subr_chain, chain_idx);
    if (jump_to == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
        return TPM_FAIL;
    }
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }
    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        if (tpm_mod2_single_command_add(&tpm_mod2_jump_subr_pattern, jump_to, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_ipv4_update()
*
*******************************************************************************/
int32_t tpm_mod2_process_ipv4_update (tpm_gmacs_enum_t      gmac_port,
                                      uint32_t                bm,
                                      uint16_t                cmd_idx,
                                      uint8_t                *skip,
                                      tpm_pkt_mod_t          *mod_data,
                                      tpm_mod_pattern_data_t *pattern_data,
                                      tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if ((bm & TPM_DSCP_SET)     ||
        (bm & TPM_TTL_DEC)      ||
        (bm & TPM_IPV4_SRC_SET) ||
        (bm & TPM_IPV4_DST_SET))
    {
        if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
            return TPM_FAIL;

        /* Copy IP checksum pattern */
        if (tpm_mod2_single_command_add(&tpm_mod2_pattern_array[cmd_idx], 0, entry_p) != TPM_OK)
            return TPM_FAIL;
    }
    else
    {
        *skip = tpm_mod2_pattern_array[cmd_idx].skip_no_parse;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_dscp_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_dscp_set (tpm_gmacs_enum_t      gmac_port,
                                   uint32_t                bm,
                                   uint16_t                cmd_idx,
                                   uint8_t                *skip,
                                   tpm_pkt_mod_t          *mod_data,
                                   tpm_mod_pattern_data_t *pattern_data,
                                   tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    uint16_t    data = 0;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (bm & TPM_IPV4_UPDATE)
    {
        if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }

        /* Copy IPv4 DSCP pattern */
        data = tpm_mod2_pattern_array[cmd_idx].entry[0].data |
               ((mod_data->l3.ipv4_mod.ipv4_dscp << TPM_MOD2_DSCP_DATA_BIT) & TPM_MOD2_DSCP_DATA_MASK);

        if (tpm_mod2_single_command_add(&tpm_mod2_pattern_array[cmd_idx], data, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }
    else
    {
        if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }

        /* Copy IPv6 TC pattern */
        data = tpm_mod2_ipv6_tc_pattern.entry[0].data |
               ((mod_data->l3.ipv6_mod.ipv6_dscp << TPM_MOD2_TC_DATA_BIT) & TPM_MOD2_TC_DATA_MASK);

        if (tpm_mod2_single_command_add(&tpm_mod2_ipv6_tc_pattern, data, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_ttl_dec()
*
*******************************************************************************/
int32_t tpm_mod2_process_ttl_dec (tpm_gmacs_enum_t      gmac_port,
                                  uint32_t                bm,
                                  uint16_t                cmd_idx,
                                  uint8_t                *skip,
                                  tpm_pkt_mod_t          *mod_data,
                                  tpm_mod_pattern_data_t *pattern_data,
                                  tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;
    uint16_t extra_word_skip = 0;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    /* Save skip command by TTL DEC skip_before */
    if (*skip % 2)
    {
        if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
    }
    else
    {
        extra_word_skip = (*skip / 2) << 8;
        *skip = 0;
    }

    /* Copy TTL DEC pattern w/o changing data*/
    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tpm_mod2_pattern_array[cmd_idx].entry, sizeof(tpm_mod2_entry_t));
        entry_p->line[entry_p->line_num].data += extra_word_skip;
        entry_p->line_num++;

        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added command 0x%x pattern_line=%d\n",
                    tpm_mod2_pattern_array[cmd_idx].entry[0].opcode, entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_v4_sip_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_v4_sip_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    /* Copy IPv4 SIP pattern */
    if ((entry_p->line_num + TPM_MOD2_IP_CMD_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tpm_mod2_pattern_array[cmd_idx].entry,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_IP_CMD_ENTRIES);

        entry_p->line[entry_p->line_num++].data = ((mod_data->l3.ipv4_mod.ipv4_src_ip_add[0] << 8) | mod_data->l3.ipv4_mod.ipv4_src_ip_add[1]);
        entry_p->line[entry_p->line_num++].data = ((mod_data->l3.ipv4_mod.ipv4_src_ip_add[2] << 8) | mod_data->l3.ipv4_mod.ipv4_src_ip_add[3]);

        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}


/*******************************************************************************
* tpm_mod2_process_v4_dip_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_v4_dip_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    /* Copy IPv4 DIP pattern */
    if ((entry_p->line_num + TPM_MOD2_IP_CMD_ENTRIES) <= TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tpm_mod2_pattern_array[cmd_idx].entry,
               sizeof(tpm_mod2_entry_t)*TPM_MOD2_IP_CMD_ENTRIES);

        entry_p->line[entry_p->line_num++].data = ((mod_data->l3.ipv4_mod.ipv4_dst_ip_add[0] << 8) | mod_data->l3.ipv4_mod.ipv4_dst_ip_add[1]);
        entry_p->line[entry_p->line_num++].data = ((mod_data->l3.ipv4_mod.ipv4_dst_ip_add[2] << 8) | mod_data->l3.ipv4_mod.ipv4_dst_ip_add[3]);

        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "pattern_line=%d \n", entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_hoplim_dec()
*
*******************************************************************************/
int32_t tpm_mod2_process_hoplim_dec (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    /* Copy Hop limit DEC pattern w/o changing data*/
    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tpm_mod2_pattern_array[cmd_idx].entry, sizeof(tpm_mod2_entry_t));
        entry_p->line_num++;

        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added command 0x%x pattern_line=%d\n",
                    tpm_mod2_pattern_array[cmd_idx].entry[0].opcode, entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_l4_src_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_l4_src_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    /* Copy L4 SRC pattern */
    if (tpm_mod2_single_command_add(&tpm_mod2_pattern_array[cmd_idx],
                                    mod_data->l3.ipv4_mod.l4_src_port, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    return TPM_OK;
}


/*******************************************************************************
* tpm_mod2_process_l4_dst_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_l4_dst_set (tpm_gmacs_enum_t      gmac_port,
                                     uint32_t                bm,
                                     uint16_t                cmd_idx,
                                     uint8_t                *skip,
                                     tpm_pkt_mod_t          *mod_data,
                                     tpm_mod_pattern_data_t *pattern_data,
                                     tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (tpm_mod2_skip_command_add(skip, tpm_mod2_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    /* Copy L4 DST pattern */
    if (tpm_mod2_single_command_add(&tpm_mod2_pattern_array[cmd_idx],
                                    mod_data->l3.ipv4_mod.l4_dst_port, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_l4_checksum_set()
*
*******************************************************************************/
int32_t tpm_mod2_process_l4_checksum_set(tpm_gmacs_enum_t      gmac_port,
                                         uint32_t                bm,
                                         uint16_t                cmd_idx,
                                         uint8_t                *skip,
                                         tpm_pkt_mod_t          *mod_data,
                                         tpm_mod_pattern_data_t *pattern_data,
                                         tpm_self_check_enable_t is_tpm_check)
{
    tpm_pattern_entry_t *entry_p = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "gmac(%d), bitmap(%x)\n",gmac_port, bm);

    if (bm & TPM_INT_L4_UDP)
    {
        /* If udp_checksum_calc is taken from Init, take init value */
        if (tpm_db_mod2_get_udp_checksum_init_bm_state())
        {
            if (!tpm_db_mod2_get_udp_checksum_state())
                return TPM_OK;
        }
        else if ((bm & TPM_INT_UDP_CHECKSUM) == 0) /* else take from API flag */
        {
            return TPM_OK;
        }
    }

    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, pattern_data->main_chain_type, pattern_data->main_chain_id);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
        return TPM_FAIL;
    }

    if (tpm_mod2_skip_command_add(skip, tpm_mod2_int_pattern_array[cmd_idx].skip_before, entry_p) != TPM_OK)
    {
        return TPM_FAIL;
    }

    /* Copy L4 checksum pattern w/o changing data*/
    if (entry_p->line_num < TPM_MOD2_MAX_PATTERN_ENTRIES)
    {
        memcpy(&(entry_p->line[entry_p->line_num]), tpm_mod2_int_pattern_array[cmd_idx].entry, sizeof(tpm_mod2_entry_t));
        entry_p->line_num++;

        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added command 0x%x pattern_line=%d\n",
                    tpm_mod2_pattern_array[cmd_idx].entry[0].opcode, entry_p->line_num);
    }
    else
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the tmp.pattern (%d)\n", entry_p->line_num);
        return TPM_FAIL;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_process_l4_checksum_set()
*
*******************************************************************************/
static uint8_t  tpm_mod2_skip_get (tpm_pkt_mod_bm_t mod_bm,
                                   uint16_t         mod_flag_idx)
{
    uint8_t skip_val = 0;

    switch(mod_flag_idx)
    {
    case TPM_MOD_MAC_DA_SET:     /* 0 - Update the MAC DA Address*/
    case TPM_MOD_MAC_SA_SET:     /* 1 - Update the MAC SA Address*/
    case TPM_MOD_VLAN_MOD:       /* 2 - Update the VLAN Tags (add/del/update)*/
    case TPM_MOD_PPPOE_DEL:      /* 3 - Delete a PPPoE encapsulation*/
    case TPM_MOD_PPPOE_ADD:      /* 4 - Add a PPPoE encapsulation*/
        skip_val = tpm_mod2_pattern_array[mod_flag_idx].skip_no_parse;
        break;

    case TPM_MOD_DSCP_SET:       /* 5 - Set the DSCP value */
        skip_val = tpm_mod2_pattern_array[mod_flag_idx].skip_no_parse;

        break;

    case TPM_MOD_TTL_DEC:        /* 6 - Decrease the TTL value */
    case TPM_MOD_IPV4_SRC_SET:   /* 8 - Set the IPV4 Source Address */
    case TPM_MOD_IPV4_DST_SET:   /* 9 - Set the IPV4 Destination Address */
        if (mod_bm & TPM_IPV4_UPDATE)
        {
            skip_val = tpm_mod2_pattern_array[mod_flag_idx].skip_no_parse;
        }
        break;

    case TPM_MOD_HOPLIM_DEC:     /* 11 - Decrease the Hop Limit value */
    case TPM_MOD_IPV6_SRC_SET:   /* 12 - Set the IPV6 Source Address */
    case TPM_MOD_IPV6_DST_SET:   /* 13 - Set the IPV6 Destination Address */
        if (mod_bm & TPM_IPV6_UPDATE)
        {
            skip_val = tpm_mod2_pattern_array[mod_flag_idx].skip_no_parse;
        }
        break;

    case TPM_MOD_L4_SRC_SET:     /* 14 - Set the L4 Source Port (UDP or TCP) */
    case TPM_MOD_L4_DST_SET:     /* 15 - Set the L4 Destination Port (UDP or TCP) */
        if ((mod_bm & TPM_IPV6_UPDATE) ||
            (mod_bm & TPM_IPV4_UPDATE))
        {
            skip_val = tpm_mod2_pattern_array[mod_flag_idx].skip_no_parse;
        }
        break;

    case TPM_MOD_IPV4_UPDATE:    /* 7 - Update the IPv4 Header*/
    case TPM_MOD_IPV6_UPDATE:    /* 10 - Update the IPv6 Header */
    default:
        break;
    }

    return skip_val;
}

/*******************************************************************************
* tpm_mod2_parse_get()
*
*******************************************************************************/
static uint8_t  tpm_mod2_l2_parse_len_get (uint16_t         mod_flag_idx,
                                           uint32_t         mh_en,
                                           tpm_pkt_mod_t   *mod_data)
{
    uint8_t  parse_len = 0;

    if (mod_flag_idx < TPM_MOD_PPPOE_DEL)
    {
        if (mod_flag_idx >= TPM_MOD_MH_SET && mh_en)
        {
            parse_len += 2; // add mh len
        }

        if (mod_flag_idx >= TPM_MOD_MAC_DA_SET)
        {
            parse_len += 6; // add da len
        }

        if (mod_flag_idx >= TPM_MOD_MAC_SA_SET)
        {
            parse_len += 6; // add sa len
        }

        if (mod_flag_idx == TPM_MOD_VLAN_MOD)
        {
            switch (mod_data->vlan_mod.vlan_op)
            {
            case VLANOP_EXT_TAG_MOD:
            case VLANOP_EXT_TAG_DEL:
            case VLANOP_EXT_TAG_MOD_INS:
                parse_len += 4; // one tag is processed.
                break;

            case VLANOP_INS_2TAG:
            case VLANOP_MOD_2TAG:
            case VLANOP_SWAP_TAGS:
            case VLANOP_DEL_2TAG:
            case VLANOP_INT_TAG_MOD:
            case VLANOP_EXT_TAG_DEL_INT_MOD:
                parse_len += 8; // two tags are processed.
                break;

            case VLANOP_EXT_TAG_INS:
            default:
                break;
            }
        }
    }

    return parse_len;
}

/*******************************************************************************
* tpm_mod2_fill_in_pattern()
*
* DESCRIPTION: The API sets the temporary pattern which will be written to HW.
*
* INPUTS:   mod_bm      - set of flags described which fields in the packet
*                         to be changed
*           int_mod_bm  - set of internal flags
*           mod_data    - modification entry data
*	    is_tpm_check -fill pattern for tpm check or not
*
* OUTPUTS:
*           mod_entry   - start entry number which has been set for this
*                         modification <j>
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_fill_in_pattern   (tpm_gmacs_enum_t     gmac_port,
                                       tpm_pkt_mod_bm_t        mod_bm,
                                       tpm_pkt_mod_int_bm_t    int_mod_bm,
                                       tpm_pkt_mod_t          *mod_data,
                                       uint32_t               *mod_entry,
                                       tpm_mod_pattern_data_t *pattern_data,
                                       tpm_self_check_enable_t is_tpm_check)
{
    uint16_t    jump_idx, main_idx, flag_idx, jump_to;
    uint16_t    last_mod_flag = TPM_MOD_MH_SET;
    uint8_t     skip = 0;
    uint8_t     minus_len = 0;
    uint32_t    mh_en;
    tpm_chain_type_t     main_chain_type, db_main_chain_type;
    tpm_pattern_entry_t *entry_p = NULL;
    uint32_t    i;
    uint16_t    max_sz;
    uint16_t    pmt_entry;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }
    jump_idx = 0;

    /* Init jump pattern entry */
    entry_p = &pattern_data->pattern_set[0];
    entry_p->is_valid   = 1;
    entry_p->area_type  = TPM_AREA_TYPE_JUMP;
    entry_p->chain_type = TPM_CHAIN_TYPE_NONE;

    if (TPM_CHECK_DISABLE == is_tpm_check) {
        /* Get next_free index => jump_idx */
        spin_lock_bh(&tpmMod2JumpEntryLock);
        if ((TPM_SPLIT_MOD_ENABLED == tpm_db_split_mod_get_enable()) && (int_mod_bm & TPM_INT_SPLIT_MOD))
        {
            TPM_OS_INFO(TPM_MODZ2_HM_MOD, "Need split_mod JUMP entry\n");
            jump_idx = tpm_db_mod2_get_next_split_mod_free_jump_entry(gmac_port, mod_data);
        }
	else {
            TPM_OS_INFO(TPM_MODZ2_HM_MOD, "Need common JUMP entry\n");
            jump_idx = tpm_db_mod2_get_next_free_jump_entry(gmac_port);
	}
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        if (jump_idx == TPM_MOD2_NULL_ENT_IDX)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free JUMP entry\n");
            return TPM_FAIL;
        }

        /* Ensure there is no write to index 0 in modification table */
        if (jump_idx == 0)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Illegal write to index 0 in modification table: (jump_idx=%d)\n", jump_idx);
            return TPM_FAIL;
        }
	entry_p->index = jump_idx;
        /* Return mod_entry index */
        *mod_entry = jump_idx;
    } else {
        entry_p->index = *mod_entry;
        jump_idx = *mod_entry;
        /*get main chain index*/
        if (tpm_mod2_main_chain_index_get(gmac_port, jump_idx, &pmt_entry,is_tpm_check)) {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain index for tpm check\n");
            return TPM_FAIL;
        }
        /*get chain ID by PMT entry index*/
        if (tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port,pmt_entry,&db_main_chain_type,&main_idx)) {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Get main chain id failed for tpm check\n");
            return TPM_FAIL;
        }
    }

    /* Get main chain type from mod_bm */
    main_chain_type = tpm_mod2_parse_chain_type(mod_bm, int_mod_bm);
    if (TPM_CHECK_DISABLE == is_tpm_check) {
        spin_lock_bh(&tpmMod2ChainEntryLock);
        /* Get next_free index by chain_type => main_idx */
        main_idx = tpm_db_mod2_get_next_free_chain_entry(gmac_port, main_chain_type);
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        if (main_idx == TPM_MOD2_INVALID_CHAIN_ID) {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "No free space in the main area (chain_type=%d)\n", main_chain_type);
            return TPM_FAIL;
        }
    }

    /* Get port MH mode */
    if (tpm_db_gmac_mh_en_conf_get(gmac_port, &mh_en)) {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get port MH mode\n");
        return TPM_FAIL;
    }

    /* Set main chain for mod_rules */
    pattern_data->main_chain_type = main_chain_type;
    pattern_data->main_chain_id   = main_idx;

    /* Get free pattern entry for main_chain */
    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, main_chain_type, TPM_MOD2_INVALID_CHAIN_ID);
    if (entry_p == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get free pattern set for main_chain\n");
        return TPM_FAIL;
    }
    entry_p->index = pattern_data->main_chain_id;

    for (flag_idx = 0; flag_idx < TPM_MAX_PKT_MOD_FLAGS; flag_idx++)
    {
        if (mod_bm & tpm_mod2_pattern_array[flag_idx].mask)
        {
            /* next pattern found */
            if (tpm_mod2_pattern_array[flag_idx].process_pattern != NULL)
            {
                entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, main_chain_type, main_idx);
                if (entry_p == NULL)
                {
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set\n");
                    return TPM_FAIL;
                }

                //if ((last_mod_flag < TPM_MOD_PPPOE_DEL && flag_idx == TPM_MOD_PPPOE_DEL) ||
                //    (last_mod_flag < TPM_MOD_PPPOE_ADD && flag_idx == TPM_MOD_PPPOE_ADD))
                //{
                //    /* skip to L3_offset */
                //    minus_len = tpm_mod2_l2_parse_len_get(last_mod_flag, mod_data);
                //    if (tpm_mod2_skip_to_l3_offset_command_add(&skip, (minus_len + 2/*eth_type*/), entry_p) != TPM_OK)
                //    {
                //        return TPM_FAIL;
                //    }
                //    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added SKIP to ipv4 header\n");
                //}

                if (last_mod_flag < TPM_MOD_PPPOE_DEL && flag_idx == TPM_MOD_PPPOE_DEL)
                {
                    /* skip to L3_offset */
                    minus_len = tpm_mod2_l2_parse_len_get(last_mod_flag, mh_en, mod_data);
                    minus_len+= (2/*eth_type*/ + 8/*pppoe_hdr*/);
                    if (tpm_mod2_skip_to_l3_offset_command_add(&skip, minus_len, entry_p) != TPM_OK)
                    {
                        return TPM_FAIL;
                    }
                    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added SKIP to ipv4 header\n");
                }
                else if (last_mod_flag < TPM_MOD_PPPOE_ADD && flag_idx == TPM_MOD_PPPOE_ADD)
                {
                    /* skip to L3_offset */
                    minus_len = tpm_mod2_l2_parse_len_get(last_mod_flag, mh_en, mod_data);
                    minus_len+= 2/*eth_type*/;
                    if (tpm_mod2_skip_to_l3_offset_command_add(&skip, minus_len, entry_p) != TPM_OK)
                    {
                        return TPM_FAIL;
                    }
                    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added SKIP to ipv4 header\n");
                }

                if (tpm_mod2_pattern_array[flag_idx].process_pattern(gmac_port, mod_bm, flag_idx, &skip, mod_data, pattern_data, is_tpm_check) != TPM_OK)
                {
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to add cmd pattern (%d)\n", flag_idx);
                    return TPM_FAIL;
                }
                last_mod_flag = flag_idx;
                TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added patterns for %d \n", flag_idx);
            }
            else
            {
                //if ((last_mod_flag < TPM_MOD_PPPOE_ADD && flag_idx == TPM_MOD_PPPOE_ADD) ||
                //    (last_mod_flag < TPM_MOD_PPPOE_DEL && flag_idx == TPM_MOD_PPPOE_DEL))
                if (last_mod_flag < TPM_MOD_PPPOE_DEL && flag_idx == TPM_MOD_PPPOE_ADD)
                {
                    /* Skip to L3 offset if L3_mod is requested */
                    if (mod_bm & (TPM_MOD2_IPV4_MOD_MASK | TPM_MOD2_IPV6_MOD_MASK | TPM_MOD2_L4_MOD_MASK))
                    {
                        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, main_chain_type, main_idx);
                        if (entry_p == NULL)
                        {
                            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
                            return TPM_FAIL;
                        }

                        /* Skip to L3_offset */
                        minus_len = tpm_mod2_l2_parse_len_get(last_mod_flag, mh_en, mod_data);
                        if (tpm_mod2_skip_to_l3_offset_command_add(&skip, minus_len, entry_p) != TPM_OK)
                        {
                            return TPM_FAIL;
                        }
                        last_mod_flag = TPM_MOD_PPPOE_ADD;
                        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added SKIP to ipv4 header\n");
                    }
                    else
                    {
                        /* Finish parsing mod_flag */
                        break;
                    }
                }
                else
                {
                    skip += tpm_mod2_skip_get(mod_bm, flag_idx);
                }
            }
        }
        else
        {
            /* If not match any mod flag before L3 offset, skip the gap between VLAN Tag and L3 header. */
            if (last_mod_flag < TPM_MOD_PPPOE_DEL && flag_idx == TPM_MOD_PPPOE_ADD)
            {
                if (mod_bm & (TPM_MOD2_IPV4_MOD_MASK | TPM_MOD2_IPV6_MOD_MASK | TPM_MOD2_L4_MOD_MASK))
                {
                    entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, main_chain_type, main_idx);
                    if (entry_p == NULL)
                    {
                        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
                        return TPM_FAIL;
                    }

                    /* skip to L3_offset */
                    minus_len = tpm_mod2_l2_parse_len_get(last_mod_flag, mh_en, mod_data);
                    if (tpm_mod2_skip_to_l3_offset_command_add(&skip, minus_len, entry_p) != TPM_OK)
                    {
                        return TPM_FAIL;
                    }
                    last_mod_flag = TPM_MOD_PPPOE_ADD;
                }
                else
                {
                    /* Finish parsing mod_flag */
                    break;
                }
            }
            else
            {
                skip += tpm_mod2_skip_get(mod_bm, flag_idx);
            }
        }
    }

    if (mod_bm & TPM_L4_SRC_SET ||
        mod_bm & TPM_L4_DST_SET ||
        mod_bm & TPM_IPV6_SRC_SET ||
        mod_bm & TPM_IPV6_DST_SET ||
        mod_bm & TPM_IPV4_SRC_SET ||
        mod_bm & TPM_IPV4_DST_SET)
    {
        /* Update L4 checksum */
        for (flag_idx = 0; flag_idx < TPM_MAX_INT_MOD_FLAGS; flag_idx++)
        {
            if (int_mod_bm & tpm_mod2_int_pattern_array[flag_idx].mask)
            {
                if (tpm_mod2_int_pattern_array[flag_idx].process_pattern != NULL)
                {
                    if (tpm_mod2_int_pattern_array[flag_idx].process_pattern(gmac_port, int_mod_bm, flag_idx, &skip, mod_data, pattern_data, is_tpm_check) != TPM_OK)
                    {
                        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to add cmd pattern (%d)\n", flag_idx);
                        return TPM_FAIL;
                    }

                    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added patterns for %d \n", flag_idx);
                }
            }
        }
    }

    if (main_chain_type != TPM_CHAIN_TYPE_MH)
    {
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, main_chain_type, main_idx);
        if (entry_p == NULL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
            return TPM_FAIL;
        }
        /* Set "last" bit to the last modification entry */
        entry_p->line[entry_p->line_num-1].last = 1;
        /* Last command should never be jump_subr, replace it with jump */
        if (entry_p->line[entry_p->line_num-1].opcode == TPM_MOD2_OPCODE_JUMP_SUBR)
        {
            entry_p->line[entry_p->line_num-1].opcode = TPM_MOD2_OPCODE_JUMP;
        }
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Set last bit to pattern_line=%d\n", entry_p->line_num-1);

    /* Write jump entry to db */
    jump_to = tpm_db_mod2_convert_chain_to_pmt_entry(main_chain_type, pattern_data->main_chain_id);
    if (jump_to == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
        return TPM_FAIL;
    }
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "[GMAC-%d] Jump from 0x%04x to chain-%d[%d] 0x%04x\n",
                 gmac_port, jump_idx, main_chain_type, pattern_data->main_chain_id, jump_to);

    /* Verify whether the Marvell tag has been defined -
       in this case the first command in the pattern should be JUMP_SKIP2 */
    /* get the MH - if enabled or not */
    entry_p = &pattern_data->pattern_set[0];
    if (mh_en == TPM_FALSE || mod_bm & TPM_MH_SET)
    {
        if (tpm_mod2_single_command_add(&tpm_mod2_jump_pattern, jump_to, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added JUMP pattern\n");
    }
    else
    {
        if (tpm_mod2_single_command_add(&tpm_mod2_jump_skip2_pattern, jump_to, entry_p) != TPM_OK)
        {
            return TPM_FAIL;
        }
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Added JUMP-SKIP2 pattern\n");
    }

    /*if function for tpm check, return from here*/
    if (is_tpm_check)
        return TPM_OK;

    spin_lock_bh(&tpmMod2JumpEntryLock);
    if (tpm_db_mod2_set_jump_entry(gmac_port, entry_p->index, entry_p->line) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set jump entry\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2JumpEntryLock);

    if (main_chain_type != TPM_CHAIN_TYPE_MH)
    {
        /* Write main chain entry to db */
        entry_p = tpm_mod2_get_chain_from_pattern_set(pattern_data->pattern_set, main_chain_type, main_idx);
        if (entry_p == NULL)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Fail to get main pattern set. \n");
            return TPM_FAIL;
        }
        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_set_chain_entry(gmac_port, main_chain_type, main_idx, entry_p->line_num, entry_p->line) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set main chain entry\n");
            return TPM_FAIL;
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);
    }

    /* Check if number of chain PMT entry exceeds the maximum chain size */
    for (i=1; i < TPM_MOD2_MAX_PATTERN_SETS; i++)
    {
        entry_p = &pattern_data->pattern_set[i];

        if (entry_p->is_valid)
        {
            max_sz = tpm_db_mod2_get_chain_max_size(entry_p->chain_type);
            if (entry_p->line_num > max_sz)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "chain(%d) PMT entry number(%d) exceeds the maximum chain size(%d)\n",
                             entry_p->chain_type, entry_p->line_num, max_sz);
                return TPM_FAIL;
            }
        }
    }

    return (TPM_OK);
}

/*******************************************************************************
* tpm_mod2_hw_add_entry()
*
* DESCRIPTION: The API writes a complete Modification table entry to HW table
*
* INPUTS:   gmac_port   - GMAC port
*           pattern_set - pattern array with data to be written
*
* OUTPUTS:
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_hw_add_entry   (tpm_gmacs_enum_t       gmac_port,
                                    tpm_mod_pattern_data_t *pattern_data)
{
    uint16_t    line_id, set_id, pmt_id, /*chain_id, */valid_set_num;
    uint32_t    cfg_pnc_parse_flag = 0;
    uint8_t     is_exist;
    tpm_pattern_entry_t *entry_p = NULL;
    tpm_mod2_entry_t    *line_p  = NULL;

    MV_NETA_PMT entry;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "\n");

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " TPM is NOT configuring the PNC / MOD - MV driver is responsible \n");
        return (TPM_OK);
    }
    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " TPM is configuring the PNC / MOD \n");

    valid_set_num = 0;
    /* Set sub chain and main chain first */
    for (set_id = 1; set_id < TPM_MOD2_MAX_PATTERN_SETS; set_id++)
    {
        entry_p = &pattern_data->pattern_set[set_id];
        if (entry_p->is_valid)
        {
            TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " Write pattern set %d to HW: chain_type %d, chain_index %d, line_num %d\n",
                         set_id, entry_p->chain_type, entry_p->index, entry_p->line_num);

            /* Validate pattern data */
            if (entry_p->line_num == 0)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Empty pattern\n");
                return TPM_FAIL;
            }

            /* Convert chain index to PMT entry index */
            pmt_id = tpm_db_mod2_convert_chain_to_pmt_entry(entry_p->chain_type, entry_p->index);
            if (pmt_id == TPM_MOD2_NULL_ENT_IDX)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid pmt entry\n");
                return TPM_FAIL;
            }

            valid_set_num++;

            /* If chain_type is one of the subchains,
               check if the entry has already existed in database. */
            switch (entry_p->chain_type)
            {
            case TPM_CHAIN_TYPE_MH:
            case TPM_CHAIN_TYPE_MAC:
            case TPM_CHAIN_TYPE_PPPOE:
			case TPM_CHAIN_TYPE_IPV6_PPPOE:
            case TPM_CHAIN_TYPE_VLAN:
                spin_lock_bh(&tpmMod2ChainEntryLock);
                if (tpm_db_mod2_if_chain_pmt_entries_exist(gmac_port, entry_p->chain_type, entry_p->index, &is_exist) != TPM_OK)
                {
                    spin_unlock_bh(&tpmMod2ChainEntryLock);
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get hw entries status\n");
                    return TPM_FAIL;
                }
                if (is_exist)
                {
                    spin_unlock_bh(&tpmMod2ChainEntryLock);
                    continue;
                }
                if (tpm_db_mod2_activate_chain_pmt_entry(gmac_port, entry_p->chain_type, entry_p->index) != TPM_OK)
                {
                    spin_unlock_bh(&tpmMod2ChainEntryLock);
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to activate hw entries\n");
                    return TPM_FAIL;
                }
                spin_unlock_bh(&tpmMod2ChainEntryLock);
                break;
            case TPM_CHAIN_TYPE_L2:
            case TPM_CHAIN_TYPE_IPV4_NAPT:
            case TPM_CHAIN_TYPE_IPV4_MC:
            case TPM_CHAIN_TYPE_ROUTE:
            case TPM_CHAIN_TYPE_IPV6_MC:
                spin_lock_bh(&tpmMod2ChainEntryLock);
                if (tpm_db_mod2_activate_chain_pmt_entry(gmac_port, entry_p->chain_type, entry_p->index) != TPM_OK)
                {
                    spin_unlock_bh(&tpmMod2ChainEntryLock);
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to activate hw entries\n");
                    return TPM_FAIL;
                }
                spin_unlock_bh(&tpmMod2ChainEntryLock);
                break;
            default:
                break;
            }

            for (line_id = 0; line_id < TPM_MOD2_MAX_PATTERN_ENTRIES && line_id < entry_p->line_num; line_id++)
            {
                line_p = &entry_p->line[line_id];
                /* Fill in modData */
                entry.word = line_p->data & MV_NETA_PMT_DATA_MASK;
                /* Fill in opCode */
                entry.word |= ((line_p->opcode << MV_NETA_PMT_CMD_OFFS) & MV_NETA_PMT_CMD_ALL_MASK);
                /* Fill in L3 checksum flag */
                entry.word |= ((line_p->updt_ipv4 << MV_NETA_PMT_IP4_CSUM_BIT) & MV_NETA_PMT_IP4_CSUM_MASK);
                /* Fill in L4 checksum flag */
                entry.word |= ((line_p->updt_tcp << MV_NETA_PMT_L4_CSUM_BIT) & MV_NETA_PMT_L4_CSUM_MASK);
                /* Fill in last instruction flag */
                entry.word |= ((line_p->last << MV_NETA_PMT_LAST_BIT) & MV_NETA_PMT_LAST_MASK);

                spin_lock_bh(&tpmMod2PmtHwLock);
                mvNetaPmtWrite(gmac_port, pmt_id++, &entry);
                spin_unlock_bh(&tpmMod2PmtHwLock);
            }
        }
    }

    /* Set jump entry */
    line_p = &pattern_data->pattern_set[0].line[0];
    /* Fill in modData */
    entry.word = line_p->data & MV_NETA_PMT_DATA_MASK;
    /* Fill in opCode */
    entry.word |= ((line_p->opcode << MV_NETA_PMT_CMD_OFFS) & MV_NETA_PMT_CMD_ALL_MASK);
    /* Fill in L3 checksum flag */
    entry.word |= ((line_p->updt_ipv4 << MV_NETA_PMT_IP4_CSUM_BIT) & MV_NETA_PMT_IP4_CSUM_MASK);
    /* Fill in L4 checksum flag */
    entry.word |= ((line_p->updt_tcp << MV_NETA_PMT_L4_CSUM_BIT) & MV_NETA_PMT_L4_CSUM_MASK);
    /* Fill in last instruction flag */
    entry.word |= ((line_p->last << MV_NETA_PMT_LAST_BIT) & MV_NETA_PMT_LAST_MASK);

    spin_lock_bh(&tpmMod2PmtHwLock);
    mvNetaPmtWrite(gmac_port, pattern_data->pattern_set[0].index, &entry);
    spin_unlock_bh(&tpmMod2PmtHwLock);

    return TPM_OK;
}

int32_t    tpm_mod2_flush_all   (tpm_gmacs_enum_t       gmac_port,
                            tpm_mod_pattern_data_t *pattern_data,
                            uint8_t                 updt_next_free,
                            uint8_t                 on_failure)
{
    uint16_t set_id;
    tpm_pattern_entry_t *pattern_set = NULL;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    pattern_set = pattern_data->pattern_set;

	if (pattern_set[0].index != 0 && pattern_set[0].index != TPM_MOD2_NULL_ENT_IDX) {
	    spin_lock_bh(&tpmMod2JumpEntryLock);
	    if (tpm_db_mod2_flush_jump_area(gmac_port, pattern_set[0].index, updt_next_free, on_failure) != TPM_OK)
	    {
	        spin_unlock_bh(&tpmMod2JumpEntryLock);
	        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to flush jump area\n");
	        return TPM_FAIL;
	    }
	    spin_unlock_bh(&tpmMod2JumpEntryLock);
	}

	if (pattern_data->main_chain_type != TPM_CHAIN_TYPE_NONE) {
	    for (set_id = 1; set_id < TPM_MOD2_MAX_PATTERN_SETS; set_id++)
	    {
	        if (pattern_set[set_id].is_valid)
	        {
	            spin_lock_bh(&tpmMod2ChainEntryLock);
	            if (tpm_db_mod2_flush_chain_entry(gmac_port, pattern_set[set_id].chain_type,
	                                        pattern_set[set_id].index, updt_next_free, on_failure) != TPM_OK)
	            {
	                spin_unlock_bh(&tpmMod2ChainEntryLock);
	                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to flush jump area\n");
	                return TPM_FAIL;
	            }
	            spin_unlock_bh(&tpmMod2ChainEntryLock);
	        }
	    }
	}

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_hw_add_entry()
*
* DESCRIPTION: The API writes NOOP entry to HW Modification table
*
* INPUTS:   gmac_port   - GMAC port
*           mod_entry   - modification entry number
*
* OUTPUTS:
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_hw_inv_entry   (tpm_gmacs_enum_t    gmac_port,
                                    uint16_t            mod_entry)
{
    MV_NETA_PMT entry;
    uint32_t    cfg_pnc_parse_flag = 0;

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " TPM is NOT invalidating the MOD - MV driver is responsible \n");
        return (TPM_OK);
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " gmac_port = %d mod_entry = %d\n", gmac_port, mod_entry);

    entry.word = 0;
    /* Fill in opCode */
    entry.word |= ((TPM_MOD2_OPCODE_NOOP << MV_NETA_PMT_CMD_OFFS) & MV_NETA_PMT_CMD_ALL_MASK);
    /* Fill in last instruction flag */
    entry.word |= ((1 << MV_NETA_PMT_LAST_BIT) & MV_NETA_PMT_LAST_MASK);

    spin_lock_bh(&tpmMod2PmtHwLock);
    mvNetaPmtWrite(gmac_port, mod_entry, &entry);
    spin_unlock_bh(&tpmMod2PmtHwLock);

    return (TPM_OK);
}

/*******************************************************************************
* tpm_mod2_sw_add_entry()
*
* DESCRIPTION: The API writes a complete Modification table entry to Shadow table
*
* INPUTS:   gmac_port   - GMAC port
*
* OUTPUTS:  None
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_sw_add_entry   (tpm_mod_owner_t     owner,
                                    tpm_gmacs_enum_t        gmac_port,
                                    tpm_mod_pattern_data_t *pattern_data)
{
    uint16_t             set_id;
    tpm_pattern_entry_t *entry_p;

    if (pattern_data == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }

    for (set_id = 0; set_id < TPM_MOD2_MAX_PATTERN_SETS; set_id++)
    {
        entry_p = &pattern_data->pattern_set[set_id];
        if (entry_p->is_valid)
        {
            if (entry_p->area_type == TPM_AREA_TYPE_JUMP)
            {
                spin_lock_bh(&tpmMod2JumpEntryLock);
                if (tpm_db_mod2_activate_jump_entry(gmac_port, entry_p->index) != TPM_OK)
                {
                    spin_unlock_bh(&tpmMod2JumpEntryLock);
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set jump entry\n");
                    return TPM_FAIL;
                }
                spin_unlock_bh(&tpmMod2JumpEntryLock);
            }
            else
            {
                spin_lock_bh(&tpmMod2ChainEntryLock);
                if (tpm_db_mod2_activate_chain_entry(gmac_port, entry_p->chain_type, entry_p->index) != TPM_OK)
                {
                    spin_unlock_bh(&tpmMod2ChainEntryLock);
                    TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to set chain-%d entry\n", entry_p->chain_type);
                    return TPM_FAIL;
                }
                spin_unlock_bh(&tpmMod2ChainEntryLock);
            }
        }
    }

    return (TPM_OK);
}

/*******************************************************************************
* tpm_mod2_print_set_params()
*
* DESCRIPTION: Prints set parameters
*
* INPUTS:   gmac_port   - GMAC port
*           mod_bm      - set of flags described which fields in the packet
*                         to be changed
*           mod_data    - modification entry data
*
* OUTPUTS:  None
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
void    tpm_mod2_print_set_params   (tpm_gmacs_enum_t   gmac_port,
                                     tpm_pkt_mod_bm_t   mod_bm,
                                     tpm_pkt_mod_t      *mod_data)
{
    char    mac_str[DB_MAC_STR_LEN] = {'\0'};
    char    mac_mask_str[DB_MAC_STR_LEN] = {'\0'};
    char    ipv4_str[DB_IPV4_STR_LEN];
    char    ipv4_mask_str[DB_IPV4_STR_LEN];
    char    ipv6_str[DB_IPV6_STR_LEN];

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "port = %d, BM = 0x%x\n", gmac_port, mod_bm);
    printk("VLAN data: op = %d\n", mod_data->vlan_mod.vlan_op);
    printk("   VLAN1 tpid=0x%4.4x, vid=%d(%d), cfi=%d(%d), pbit=%d(%d)\n", mod_data->vlan_mod.vlan1_out.tpid,
           mod_data->vlan_mod.vlan1_out.vid, mod_data->vlan_mod.vlan1_out.vid_mask,
           mod_data->vlan_mod.vlan1_out.cfi, mod_data->vlan_mod.vlan1_out.cfi_mask,
           mod_data->vlan_mod.vlan1_out.pbit, mod_data->vlan_mod.vlan1_out.pbit_mask);
    printk("   VLAN2 tpid=0x%4.4x, vid=%d(%d), cfi=%d(%d), pbit=%d(%d)\n", mod_data->vlan_mod.vlan2_out.tpid,
           mod_data->vlan_mod.vlan2_out.vid, mod_data->vlan_mod.vlan2_out.vid_mask,
           mod_data->vlan_mod.vlan2_out.cfi, mod_data->vlan_mod.vlan2_out.cfi_mask,
           mod_data->vlan_mod.vlan2_out.pbit, mod_data->vlan_mod.vlan2_out.pbit_mask);
    printk("MH data:   0x%02x\n", mod_data->mh_mod);
    printk("MAC data:  SRC %s  MASK %s\n",
           db_mac_to_str(mod_data->mac_mod.mac_sa, mac_str),
           db_mac_to_str(mod_data->mac_mod.mac_sa_mask, mac_mask_str));
    printk("           DST %s  MASK %s\n",
           db_mac_to_str(mod_data->mac_mod.mac_da, mac_str),
           db_mac_to_str(mod_data->mac_mod.mac_da_mask, mac_mask_str));
    printk("PPPoE data: session 0x%x  proto 0x%x\n",
           mod_data->pppoe_mod.ppp_session, mod_data->pppoe_mod.ppp_proto);

    if (mod_bm & TPM_IPV4_UPDATE)
    {
        printk("IPv4 data: proto = %d\n", mod_data->l3.ipv4_mod.ipv4_proto);
        printk("           SRC %s  MASK %s\n",
               db_ipv4_to_str(mod_data->l3.ipv4_mod.ipv4_src_ip_add, ipv4_str),
               db_ipv4_to_str(mod_data->l3.ipv4_mod.ipv4_src_ip_add_mask, ipv4_mask_str));
        printk("           DST %s  MASK %s\n",
               db_ipv4_to_str(mod_data->l3.ipv4_mod.ipv4_dst_ip_add, ipv4_str),
               db_ipv4_to_str(mod_data->l3.ipv4_mod.ipv4_dst_ip_add_mask, ipv4_mask_str));
        printk("           DSCP %d  MASK 0x%x\n",
               mod_data->l3.ipv4_mod.ipv4_dscp, mod_data->l3.ipv4_mod.ipv4_dscp_mask);
        printk("L4 data:   SRC port = %d DST port = %d\n",
               mod_data->l3.ipv4_mod.l4_src_port, mod_data->l3.ipv4_mod.l4_dst_port);
    }

    if (mod_bm & TPM_IPV6_UPDATE)
    {
        printk("IPv6 data: proto = %d\n", mod_data->l3.ipv6_mod.ipv6_next_header);
        printk("           SRC %s \n",
               db_ipv4_to_str(mod_data->l3.ipv6_mod.ipv6_src_ip_add, ipv6_str));
        printk("           MASK %s\n",
               db_ipv4_to_str(mod_data->l3.ipv6_mod.ipv6_src_ip_add_mask, ipv6_str));
        printk("           DST %s\n",
               db_ipv4_to_str(mod_data->l3.ipv6_mod.ipv6_dst_ip_add, ipv6_str));
        printk("           MASK %s\n",
               db_ipv4_to_str(mod_data->l3.ipv6_mod.ipv6_dst_ip_add_mask, ipv6_str));
        printk("           DSCP %d  MASK 0x%x\n",
               mod_data->l3.ipv6_mod.ipv6_dscp, mod_data->l3.ipv6_mod.ipv6_dscp_mask);
        printk("L4 data:   SRC port = %d DST port = %d\n",
               mod_data->l3.ipv6_mod.l4_src_port, mod_data->l3.ipv6_mod.l4_dst_port);
    }
}

/*******************************************************************************
                               Modification table APIs
*******************************************************************************/

int32_t    tpm_mod2_inlock_entry_set(tpm_mod_owner_t      owner,
                                     tpm_gmacs_enum_t     gmac_port,
                                     tpm_pkt_mod_bm_t     mod_bm,
                                     tpm_pkt_mod_int_bm_t int_mod_bm,
                                     tpm_pkt_mod_t        *mod_data,
                                     uint32_t             *mod_entry)
{
    int32_t     rc;
    tpm_mod_pattern_data_t pattern_data;

    /* PH1 - clear the global pattern for reuse */
    memset(&pattern_data, 0, sizeof(tpm_mod_pattern_data_t));

    *mod_entry = TPM_MOD2_NULL_ENT_IDX;

    /* validate logical parameters*/
    rc = tpm_mod2_validate_logical_params(gmac_port, mod_bm, int_mod_bm, mod_data, mod_entry);

    if (rc != TPM_OK) {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, " -- tpm_mod2_validate_logical_params FAILED. \n");
        return (TPM_FAIL);
    }

    /* print debug - all function params */
    TPM_OS_COND_DEBUG(TPM_MODZ2_HM_MOD)
    {
        tpm_mod2_print_set_params (gmac_port, mod_bm, mod_data);
    }
    /*if only VLAN mod, and op is NO OP, return from here*/
    if ((mod_bm & TPM_VLAN_MOD) && ((mod_bm & ~TPM_VLAN_MOD) == 0)) {
        if(mod_data->vlan_mod.vlan_op == VLANOP_NOOP) {
            *mod_entry = 0;
            return (TPM_OK);
        }
    }

    /* update the pattern of new modification with real data */
    rc = tpm_mod2_fill_in_pattern(gmac_port, mod_bm, int_mod_bm, mod_data, mod_entry, &pattern_data, TPM_CHECK_DISABLE);

    if (rc != TPM_OK) {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, " -- tpm_mod2_fill_in_pattern FAILED. \n");
        if (tpm_mod2_flush_all(gmac_port, &pattern_data, 0, 1) != TPM_OK)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to flush pmt entry status\n");
            return TPM_FAIL;
        }
        return (TPM_FAIL);
    }


    TPM_OS_COND_DEBUG(TPM_MODZ2_HM_MOD)
    {
        tpm_mod2_print_tmp_pattern(&pattern_data);
    }

    if (*mod_entry != 0)
    {
        /* WR entries to the HW table */
        rc = tpm_mod2_hw_add_entry(gmac_port, &pattern_data);

        if (rc != TPM_OK) {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, " -- tpm_mod2_hw_add_entry FAILED. \n");
            if (tpm_mod2_flush_all(gmac_port, &pattern_data, 0, 1) != TPM_OK)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to flush pmt entry status\n");
                return TPM_FAIL;
            }
            return (TPM_FAIL);
        }


        /* WR entries to the SW table */
        rc = tpm_mod2_sw_add_entry(owner, gmac_port, &pattern_data);

        if (rc != TPM_OK) {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, " -- tpm_mod2_sw_add_entry FAILED. \n");
            if (tpm_mod2_flush_all(gmac_port, &pattern_data, 0, 1) != TPM_OK)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to flush pmt entry status\n");
                return TPM_FAIL;
            }
            return (TPM_FAIL);
        }
    }

    if (tpm_mod2_flush_all(gmac_port, &pattern_data, 0, 0) != TPM_OK)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to flush pmt entry status\n");
        return TPM_FAIL;
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Modification entry start index = %d\n", *mod_entry);

    return (TPM_OK);
}

/*******************************************************************************
* tpm_mod2_entry_set()
*
* DESCRIPTION: The API sets a complete Modification table entry
*
* INPUTS:   gmac_port   - GMAC port
*           mod_bm      - set of flags described which fields in the packet
*                         to be changed
*           int_mod_bm  - set of internal flags
*           mod_data    - modification entry data
*
* OUTPUTS:
*           mod_entry   - start entry number which has been set for this
*                         modification
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_entry_set   (tpm_mod_owner_t      owner,
                                 tpm_gmacs_enum_t     gmac_port,
                                 tpm_pkt_mod_bm_t     mod_bm,
                                 tpm_pkt_mod_int_bm_t int_mod_bm,
                                 tpm_pkt_mod_t        *mod_data,
                                 uint32_t             *mod_entry)
{
    int32_t           rc;

    rc = tpm_mod2_inlock_entry_set(owner, gmac_port, mod_bm, int_mod_bm, mod_data, mod_entry);

    return(rc);
}


/*******************************************************************************
* tpm_mod2_entry_get()
*
* DESCRIPTION: The API gets Modification table entry
*
* INPUTS:   gmac_port   - GMAC port
*           mod_entry   - start entry number (in the "jump" area)
*
* OUTPUTS:
*           valid_entries   - number of valid entries in the chunk
*                             0 - there is no requested data
*           pnc_ref         - Number of references from Pnc entries to this modification rule
* RETURNS:
*           pointer to the tpm_mod2_rule_entries[TPM_MOD2_MAIN_CHAIN_SIZE]
*           included requested data or NULL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_entry_get   (tpm_gmacs_enum_t   gmac_port,
                                        uint32_t           mod_entry,
                                        uint16_t           *entries_num,
                                        tpm_mod_rule_t     *pnc_entries)
{
    uint16_t pmt_entry, main_chain_id, subr_chain_id, entry_num, entry_id, subr_entry_num, subr_entry_id;
    uint16_t main_entry, subr_entry;
    uint16_t pattern_num = 0;
    tpm_mod2_entry_t  pattern_data;
    tpm_chain_type_t  main_chain_type, subr_chain_type;


    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Get Mod. entry for port %d with start index = %d\n",
                 gmac_port, mod_entry);

    if (gmac_port >= TPM_MAX_NUM_GMACS)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: port %d\n", gmac_port);
        return TPM_FAIL;
    }

    if (entries_num == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL pointer\n");
        return TPM_FAIL;
    }

    if (pnc_entries == NULL)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: NULL pointer\n");
        return TPM_FAIL;
    }

    *entries_num = 0;

    /* Get jump entry pattern data */
    spin_lock_bh(&tpmMod2JumpEntryLock);
    if (tpm_db_mod2_get_jump_entry_pattern_data(gmac_port, mod_entry, &pattern_data) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to delete jump entry\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2JumpEntryLock);

    if (pattern_num < TPM_MAX_MOD_RULE_NUM)
    {
        pnc_entries[pattern_num].entry_id = mod_entry;
        memcpy(&(pnc_entries[pattern_num].entry_data), &pattern_data, sizeof(tpm_mod2_entry_t));
        pattern_num++;
    }


    /* Get main chain PMT entry index */
    pmt_entry = tpm_mod2_get_jump_to(&pattern_data);
    if (pmt_entry == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain entry\n");
        return TPM_FAIL;
    }

    main_entry = pmt_entry;

    /* Get main chain type and chain index */
    spin_lock_bh(&tpmMod2ChainEntryLock);
    if (tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port, pmt_entry, &main_chain_type, &main_chain_id) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain entry\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    spin_lock_bh(&tpmMod2ChainEntryLock);
    if (tpm_db_mod2_get_chain_entry_num(gmac_port, main_chain_type, main_chain_id, &entry_num) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain size\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    for (entry_id = 0; entry_id < entry_num; entry_id++)
    {
        /* Get main chain entry pattern data */
        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_get_chain_entry_pattern_data(gmac_port, main_chain_type, main_chain_id,
                                                        entry_id, &pattern_data) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain data\n");
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);

        if (pattern_num < TPM_MAX_MOD_RULE_NUM)
        {
            pnc_entries[pattern_num].entry_id = main_entry + entry_id;
            memcpy(&(pnc_entries[pattern_num].entry_data), &pattern_data, sizeof(tpm_mod2_entry_t));
            pattern_num++;
        }

        /* Check if calling a subr chain */
        pmt_entry = tpm_mod2_get_jump_to(&pattern_data);
        if (pmt_entry == TPM_MOD2_NULL_ENT_IDX)
        {
            continue;
        }

        subr_entry = pmt_entry;

        /* Get subr chain type and chain index */
        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port, pmt_entry, &subr_chain_type, &subr_chain_id) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain entry\n");
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);

        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_get_chain_entry_num(gmac_port, subr_chain_type, subr_chain_id, &subr_entry_num) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get subr chain size\n");
            return TPM_FAIL;
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);

        for (subr_entry_id = 0; subr_entry_id < subr_entry_num; subr_entry_id++)
        {
            /* Get subr chain entry pattern data */
            spin_lock_bh(&tpmMod2ChainEntryLock);
            if (tpm_db_mod2_get_chain_entry_pattern_data(gmac_port, subr_chain_type, subr_chain_id,
                                                            subr_entry_id, &pattern_data) != TPM_OK)
            {
                spin_unlock_bh(&tpmMod2ChainEntryLock);
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get subr chain data\n");
            }
            spin_unlock_bh(&tpmMod2ChainEntryLock);

            if (pattern_num < TPM_MAX_MOD_RULE_NUM)
            {
                pnc_entries[pattern_num].entry_id = subr_entry + subr_entry_id;
                memcpy(&(pnc_entries[pattern_num].entry_data), &pattern_data, sizeof(tpm_mod2_entry_t));
                pattern_num++;
            }
        }
    }

    *entries_num = pattern_num;

    return TPM_OK;
}


/*******************************************************************************
* tpm_mod2_entry_del()
*
* DESCRIPTION: The API invalidates a modification entry chunk - given by PNC.
*
* INPUTS:   gmac_port   - GMAC port
*           mod_entry   - start entry number (in the "jump" area)
*
* OUTPUTS:
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_mod2_inlock_entry_del(tpm_mod_owner_t      owner,
                                  tpm_gmacs_enum_t     gmac_port,
                                  uint32_t             mod_entry)
{
    uint16_t pmt_entry, main_chain_id, subr_chain_id, entry_num, entry_id;
    tpm_mod2_entry_t  pattern_data;
    tpm_chain_type_t  main_chain_type, subr_chain_type;

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "for gmac_port= (%d) mod_entry (%d)\n", gmac_port, mod_entry);
    if (gmac_port >= TPM_MAX_NUM_GMACS)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: port %d\n", gmac_port);
        return TPM_FAIL;
    }

    memset(&pattern_data, 0, sizeof(tpm_mod2_entry_t));

    /* Invalidate HW entry */
    tpm_mod2_hw_inv_entry(gmac_port, mod_entry);

    /* Get jump entry pattern data */
    spin_lock_bh(&tpmMod2JumpEntryLock);
    if (tpm_db_mod2_get_jump_entry_pattern_data(gmac_port, mod_entry, &pattern_data) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to delete jump entry\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2JumpEntryLock);

    /* Delete jump entry */
    spin_lock_bh(&tpmMod2JumpEntryLock);
    if (tpm_db_mod2_del_jump_entry(gmac_port, mod_entry) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to delete jump entry\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2JumpEntryLock);

    /* Get main chain PMT entry index */
    pmt_entry = tpm_mod2_get_jump_to(&pattern_data);
    if (pmt_entry == TPM_MOD2_NULL_ENT_IDX)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain entry\n");
        return TPM_FAIL;
    }
    /* Get main chain type and chain index */
    spin_lock_bh(&tpmMod2ChainEntryLock);
    if (tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port, pmt_entry, &main_chain_type, &main_chain_id) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain entry\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    spin_lock_bh(&tpmMod2ChainEntryLock);
    if (tpm_db_mod2_get_chain_entry_num(gmac_port, main_chain_type, main_chain_id, &entry_num) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain size\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    for (entry_id = 0; entry_id < entry_num; entry_id++)
    {
        /* Get main chain entry pattern data */
        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_get_chain_entry_pattern_data(gmac_port, main_chain_type, main_chain_id,
                                                        entry_id, &pattern_data) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain size\n");
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);

        /* Check if calling a subr chain */
        pmt_entry = tpm_mod2_get_jump_to(&pattern_data);
        if (pmt_entry == TPM_MOD2_NULL_ENT_IDX)
        {
            continue;
        }

        /* Get subr chain type and chain index */
        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_get_chain_id_by_pmt_entry(gmac_port, pmt_entry, &subr_chain_type, &subr_chain_id) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get main chain entry\n");
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);

        /* Delete subr chain */
        spin_lock_bh(&tpmMod2ChainEntryLock);
        if (tpm_db_mod2_del_chain_entry(gmac_port, subr_chain_type, subr_chain_id) != TPM_OK)
        {
            spin_unlock_bh(&tpmMod2ChainEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to delete chain entries\n");
        }
        spin_unlock_bh(&tpmMod2ChainEntryLock);
    }

    /* Delete main chain */
    spin_lock_bh(&tpmMod2ChainEntryLock);
    if (tpm_db_mod2_del_chain_entry(gmac_port, main_chain_type, main_chain_id) != TPM_OK)
    {
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to delete chain entries\n");
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_entry_del()
*
* DESCRIPTION: The API invalidates a modification entry chunk - given by PNC.
*
* INPUTS:   gmac_port   - GMAC port
*           mod_entry   - start entry number (in the "jump" area)
*
* OUTPUTS:
*
* RETURNS:
*           TPM_OK, TPM_NOT_FOUND or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t tpm_mod2_entry_del(tpm_mod_owner_t      owner,
                           tpm_gmacs_enum_t     gmac_port,
                           uint32_t             mod_entry)
{
    int32_t            rc;

    //if (in_interrupt())
    //{
    //    return ERR_IN_INTERRUPT;
    //}

    /* Lock the Mod */
    //spin_lock(&tpmModLock);

    rc = tpm_mod2_inlock_entry_del(owner, gmac_port, mod_entry);

    /* Unlock Mod */
    //spin_lock(&tpmModLock);
    return (rc);
}


/*******************************************************************************
* tpm_mod2_mac_inv()
*
* DESCRIPTION: The API resets Modification table for a specified port
*
* INPUTS:   gmac_port     - GMAC port
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_mac_inv  (tpm_gmacs_enum_t   gmac_port)
{
    uint32_t    cfg_pnc_parse_flag = 0;

	cfg_pnc_parse_flag = mv_eth_ctrl_pnc_get();

    if (cfg_pnc_parse_flag == 1) {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " TPM is NOT invalidating the MOD - MV driver is responsible \n");
        return (TPM_OK);
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, " gmac_port = %d \n", gmac_port);

    if (gmac_port >= TPM_MAX_NUM_GMACS)
    {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid parameter: port %d \n", gmac_port);
        return TPM_FAIL;
    }

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "gmac port = %d\n", gmac_port);

    /* Invalidate PMT */
    mvNetaPmtClear(gmac_port);

    return TPM_OK;
}



/*******************************************************************************
* tpm_mod2_registers_init()
*
* DESCRIPTION: The API sets init values to the HW registers used by modification
*
* INPUTS:   None
*
* OUTPUTS:
*           None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*           None
*
*******************************************************************************/
int32_t    tpm_mod2_registers_init  (tpm_gmacs_enum_t   gmac_port, uint16_t  txp)
{
    uint32_t gmac_mh_en;

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "\n");

    /* Init PPPOE registers*/
    MV_REG_WRITE (NETA_TX_PMT_PPPOE_TYPE_REG(gmac_port, txp), TPM_MOD2_DEFAULT_PPPOE_ETY);
    MV_REG_WRITE (NETA_TX_PMT_PPPOE_DATA_REG(gmac_port, txp), TPM_MOD2_DEFAULT_PPPOE_DATA);
    MV_REG_WRITE (NETA_TX_PMT_PPPOE_PROTO_REG(gmac_port, txp), TPM_MOD2_DEFAULT_PPPOE_PROTO);

    if ((TPM_ENUM_GMAC_0 == gmac_port) || (TPM_ENUM_GMAC_1 == gmac_port))
        tpm_db_gmac_mh_en_conf_get(gmac_port, &gmac_mh_en);
    else
        gmac_mh_en = 1;

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "GMAC: (%d), mh_en: (%d)\n", gmac_port, gmac_mh_en);

    if (gmac_mh_en)
        MV_REG_WRITE (NETA_TX_PMT_PPPOE_LEN_REG(gmac_port, txp), TPM_MOD2_DEFAULT_PPPOE_LEN_MH_EN);
    else
        MV_REG_WRITE (NETA_TX_PMT_PPPOE_LEN_REG(gmac_port, txp), TPM_MOD2_DEFAULT_PPPOE_LEN_MH_DIS);

    return TPM_OK;
}

/*******************************************************************************
* tpm_modz2_tpid_get()
*
* DESCRIPTION:     The function receives a tpid, and returns the selector that matches it.
*                  - If the tpid has a match, it is returned.
*                  - If there is no match, a new selector  is created in hardware and in DB.
*                  - If there is no free selector,  an error is returned
*
* INPUTS:
* tpid            - Requested Vlan tpid
*
* OUTPUTS:
* eth_type_sel    - selector that matches the tpid
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_mod2_tpid_get(uint32_t tpid,
                          uint32_t *eth_type_sel)
{
    uint32_t    ret_code;
    uint32_t    l_etype_sel;
    int         port, txp;

    /* Check if vlan_ethertype exists */
    ret_code = tpm_db_mod_etype_sel_get(tpid, &l_etype_sel);

    /*Create new entry for ethertype */
    if (ret_code != TPM_OK)
    {
        TPM_OS_DEBUG(TPM_MODZ2_HM_MOD,  "New tpid(0x%4x)", tpid);

        /* Get free Etype Select */
        ret_code = tpm_db_mod_etype_sel_free_get(&l_etype_sel);
        if (ret_code != TPM_OK)
        {
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Can't create additional Vlan Ethertype, tpid(0x%x)\n", tpid);
            return(TPM_FAIL);
        }
        else
        {
            /* Create tpid in HW  */
            for (port=TPM_ENUM_GMAC_0; port < TPM_MAX_NUM_GMACS; port++)
            {
                TPM_OS_DEBUG(TPM_MODZ2_HM_MOD,  " port(%d) sel(%d)\n", port, l_etype_sel);

                if (port != TPM_ENUM_PMAC)
                {
                    mvNetaVlanEtySet(port, 0, l_etype_sel, tpid);
                }
                else
                {
                    for (txp=0; txp < TPM_MAX_NUM_TCONT; txp++)
                    {
                        mvNetaVlanEtySet(port, txp, l_etype_sel, tpid);
                    }
                }
            }

            /* Set tpid in db */
            ret_code = tpm_db_mod_etype_sel_set(tpid, l_etype_sel);
            if (ret_code != TPM_OK)
            {
                TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Can't create additional Vlan Ethertype, tpid(0x%x) l_etype_sel(%d)\n", tpid, l_etype_sel);
                return(TPM_FAIL);
            }
        }
    }

    *eth_type_sel = l_etype_sel;

    TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "etype_sel(%d)\n", l_etype_sel);
    return(TPM_OK);
}


int32_t tpm_mod2_init(void)
{
    spin_lock_init(&tpmMod2JumpEntryLock);
    spin_lock_init(&tpmMod2ChainEntryLock);
    spin_lock_init(&tpmMod2PmtHwLock);

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_split_mod_create_l2_pmts()
*
* DESCRIPTION:     The function create split mod entries
*                  - If the tpid has a match, it is returned.
*                  - If there is no match, a new selector  is created in hardware and in DB.
*                  - If there is no free selector,  an error is returned
*
* INPUTS:
* port            - target gmac port
* mod_data        - split mod entries info
* init_flag       -flag indicate whether it invoked by init or not
*
* OUTPUTS:
* None
*
* RETURNS:
*           TPM_OK or TPM_FAIL
*
* COMMENTS:
*
*******************************************************************************/
int32_t tpm_mod2_split_mod_create_l2_pmts(tpm_gmacs_enum_t port, tpm_pkt_mod_t *mod_data, bool init_flag)
{
	uint32_t p_bits_nums;
	uint32_t loop;
	uint32_t mod_entry;
	int32_t db_ret, tpm_ret;
	uint32_t vlan_index;
	tpm_vlan_key_t *vlan_out = NULL;
	tpm_pkt_mod_t mod_data_tmp;

	/*check param*/
	if (mod_data == NULL) {
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid pointer\n");
		return(TPM_FAIL);
	}

	if (TPM_SPLIT_MOD_DISABLED == tpm_db_split_mod_get_enable())
	{
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "SPLIT_MOD_DISABLED\n");
		return(TPM_RC_OK);
	}

	/* find vlan */
	db_ret = tpm_db_mod2_split_mod_get_vlan_index(port, mod_data, &vlan_index);
	if (TPM_DB_OK == db_ret)
	{
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "this vlan already exists\n");
		/* increase user number of this vlan */
		tpm_db_mod2_split_mod_increase_vlan_user_num(port, mod_data);
		return(TPM_OK);
	}

	db_ret = tpm_db_mod2_split_mod_insert_vlan(port, mod_data);
	if (TPM_DB_OK != db_ret)
	{
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "insert_vlan failed\n");
		return(ERR_GENERAL);
	}

	p_bits_nums = TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX;

	memcpy(&mod_data_tmp, mod_data, sizeof(tpm_pkt_mod_t));
	/* VLANOP_EXT_TAG_MOD_INS mod insert VLAN p_bit*/
	if (VLANOP_EXT_TAG_MOD_INS == mod_data->vlan_mod.vlan_op)
		vlan_out = &mod_data_tmp.vlan_mod.vlan2_out;
	else
		vlan_out = &mod_data_tmp.vlan_mod.vlan1_out;

	if (!init_flag) {
		vlan_out->pbit_mask = 0;
		/* add the first pmt to set p-bit AS-IS */
		tpm_ret = tpm_mod2_entry_set(TPM_MOD_OWNER_TPM, port, TPM_VLAN_MOD, TPM_INT_SPLIT_MOD, &mod_data_tmp, &mod_entry);
		if (TPM_OK != tpm_ret)
		{
			TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "set pbit init PMT failed: port %d\n", port);
			return TPM_FAIL;
		}
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "add the first pmt to set p-bit AS-IS, mod_entry: %d \n", mod_entry);
	}

	vlan_out->pbit_mask = 0xff;

	/* build pmt entry for each pbit from 0 to 7 */
	for (loop = 0; loop < p_bits_nums; loop++) {
		vlan_out->pbit = loop;
		tpm_ret = tpm_mod2_entry_set(TPM_MOD_OWNER_TPM, port, TPM_VLAN_MOD, TPM_INT_SPLIT_MOD, &mod_data_tmp, &mod_entry);
		if (TPM_OK != tpm_ret) {
			TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "set pbit init PMT failed: port %d, pbit %d \n", port, loop);
			return TPM_FAIL;
		}
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "add pmt for p-bit: %d, mod_entry: %d \n", loop, mod_entry);
	}

	return TPM_OK;
}

int32_t tpm_mod2_split_mod_init(tpm_gmacs_enum_t port)
{
	int32_t	    tpm_ret;
	tpm_pkt_mod_t mod_data;

	if (TPM_SPLIT_MOD_DISABLED == tpm_db_split_mod_get_enable())
	{
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "SPLIT_MOD_DISABLED\n");
		return(TPM_OK);
	}

	/* add default pmt from 1 to p_bits_nums */
	memset(&mod_data, 0, sizeof(mod_data));
	mod_data.vlan_mod.vlan_op = VLANOP_EXT_TAG_MOD;
	mod_data.vlan_mod.vlan1_out.pbit_mask = 0xff;

	tpm_ret = tpm_mod2_split_mod_create_l2_pmts(port, &mod_data, true);
	if (TPM_RC_OK != tpm_ret)
	{
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "failed to add default pmt from 1 to p_bits_nums\n");
		return(TPM_FAIL);
	}

	/* add default pmt from untag NOOP to p_bits_nums */
	memset(&mod_data, 0, sizeof(mod_data));
	mod_data.vlan_mod.vlan_op = VLANOP_NOOP;
	//mod_data.vlan_mod.vlan1_out.pbit_mask = 0xff;

	tpm_ret = tpm_mod2_split_mod_create_l2_pmts(port, &mod_data, true);
	if (TPM_RC_OK != tpm_ret)
	{
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "failed to add default pmt from 1 to p_bits_nums\n");
		return(TPM_FAIL);
	}

	return TPM_OK;
}

void tpm_mod2_split_mod_entry_del(tpm_gmacs_enum_t     gmac_port,
                                  uint32_t             mod_entry,
                                  uint32_t             vlan_index)
{
	int32_t  tpm_ret;
	uint32_t i = 0;
	uint32_t num_pbits;
	uint32_t user_num = 0;

	/* decrease user number of this vlan */
	tpm_ret = tpm_db_mod2_split_mod_decrease_vlan_user_num(gmac_port, vlan_index, &user_num);
	if (TPM_DB_OK != tpm_ret)
	{
		TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "failed to decrease vlan user num, index: %d\n", vlan_index);
		return;
	}

	/* still other usr, do not remove */
	if (user_num != 0)
	{
		TPM_OS_INFO(TPM_MODZ2_HM_MOD, "still other usr, do not remove, usr num: %d\n", user_num);
		return;
	}

	/* remove all the PMT entry for this VLAN */
	num_pbits = TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX;
	for(i = 0; i <= num_pbits; i++) {
		tpm_ret = tpm_mod2_entry_del(TPM_MOD_OWNER_TPM, gmac_port, (vlan_index * 16 + i));
		if (TPM_DB_OK != tpm_ret)
		{
			TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "failed to del split mod, index: %d\n", (vlan_index * 16 + i));
			return;
		}
	}

	return;
}
int32_t tpm_mod2_split_mod_try_pmt_entry_del(tpm_api_sections_t api_section,
                                  tpm_gmacs_enum_t     gmac_port,
                                  uint32_t             mod_entry)
{
	uint32_t num_vlans;
	uint32_t vlan_index;
	tpm_gmacs_enum_t duplicate_gmac;
	tpm_db_ds_mac_based_trunk_enable_t ds_mac_based_trunk_enable;

	if (!mod_entry) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "mod_entry is zero, PMT do not need to be removed\n");
		return(TPM_OK);
	}

	if (TPM_SPLIT_MOD_DISABLED == tpm_db_split_mod_get_enable()) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "SPLIT_MOD_DISABLED, PMT need to be removed\n");
		return(TPM_FAIL);
	}

	if (	(    (TPM_IPV4_ACL == api_section)
		  || (TPM_L4_ACL == api_section)
		  || (TPM_IPV6_GEN_ACL == api_section)
		  || (TPM_IPV6_DIP_ACL == api_section))
	     && (mod_entry <= TPM_DB_SPLIT_MOD_P_BIT_NUM_MAX)) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "IPv4/6 split mod, PMT do not need to be removed\n");
		return(TPM_OK);
	}

	if (TPM_L2_PRIM_ACL != api_section) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Not L2 ACL, PMT need to be removed\n");
		return(TPM_FAIL);
	}

	if ((mod_entry % 16) > 8) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Not L2 split mod: %d, PMT need to be removed\n", mod_entry);
		return(TPM_FAIL);
	}

	vlan_index = mod_entry / 16;
	num_vlans = tpm_db_split_mod_get_num_vlans();
	if (vlan_index > num_vlans) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "Not L2 split mod: %d, PMT need to be removed\n", mod_entry);
		return(TPM_FAIL);
	}

	if (vlan_index <= 1) {
		TPM_OS_DEBUG(TPM_MODZ2_HM_MOD, "mod entry: %d, PMT smaller than 32 is reserved, "
			"can not be removed\n", mod_entry);
		return(TPM_OK);
	}

	tpm_mod2_split_mod_entry_del(gmac_port, mod_entry, vlan_index);

	/* when ds load balance on G0 and G1 is enabled, need to duplicate DS PMT on G0/1 */
	tpm_db_ds_mac_based_trunk_enable_get(&ds_mac_based_trunk_enable);
	if (	(TPM_DS_MAC_BASED_TRUNK_ENABLED == ds_mac_based_trunk_enable)
	     && (TPM_ENUM_GMAC_0 == gmac_port || TPM_ENUM_GMAC_1 == gmac_port)) {

		/* if this is DS and DS_MAC_BASED_TRUNK is ENABLED */
		if (gmac_port == TPM_ENUM_GMAC_0)
			duplicate_gmac = TPM_ENUM_GMAC_1;
		else
			duplicate_gmac = TPM_ENUM_GMAC_0;

		tpm_mod2_split_mod_entry_del(duplicate_gmac, mod_entry, vlan_index);
	}
	return TPM_OK;
}


/*******************************************************************************
                          Modification table print functions
*******************************************************************************/
char *opCodeOperationStr[30] =
{
    "NOOP",
    "ADD",
    "CONF_VLAN",
    "ADD_VLAN",
    "CONF_DSA1",
    "CONF_DSA2",
    "ADD_DSA",
    "DEL",
    "REP2",
    "REP_LSB",
    "REP_MSB",
    "REP_VLAN",
    "DEC_TTL_LSB",
    "DEC_TTL_MSB",
    "ADD_CALC_LEN",
    "REP_LEN",
    "REP_IP_CHKSUM",
    "REP_L4_CHKSUM",
    "SKIP",
    "JUMP",
    "JUMP_SKIP2",
    "JUMP_SUBR",
    "PPPOE",
    "STORE",
    "ADD_IP_CHKSUM",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED",
    "RESERVED"
};

static void tpm_mod2_print_header (void)
{
    printk("==========================================================\n");
    printk(" Index  OpCode    Operation      Data  Last  IPv4    L4   \n");
    printk("                                            update update \n");
    printk("==========================================================\n");
}

int32_t tpm_mod2_print_tmp_pattern (tpm_mod_pattern_data_t *pattern_data)
{
    uint32_t set_id, line_id;
    tpm_pattern_entry_t *entry_p = NULL;
    uint16_t pmt_entry;

    if (NULL == pattern_data)
    {
        return TPM_FAIL;
    }

    tpm_mod2_print_header();

    for (set_id = 0; set_id < TPM_MOD2_MAX_PATTERN_SETS; set_id++)
    {
        if (pattern_data->pattern_set[set_id].is_valid)
        {
            entry_p = &(pattern_data->pattern_set[set_id]);

            if (set_id == 0)
            {
                pmt_entry = entry_p->index;
            }
            else
            {
                pmt_entry = tpm_db_mod2_convert_chain_to_pmt_entry(entry_p->chain_type, entry_p->index);
            }

            printk("[ set %d ] area_type %d, chain_type %d, chain_index %d (pmt_entry 0x%04x), line_num %d \r\n",
                    set_id, entry_p->area_type, entry_p->chain_type, entry_p->index, pmt_entry, entry_p->line_num);
            for (line_id = 0; line_id < TPM_MOD2_MAX_PATTERN_ENTRIES && line_id < entry_p->line_num; line_id++)
            {
                printk(" %03d      0x%2x  %15s  0x%4.4x  %1d     %1d      %1d\n",
                       line_id,
                       entry_p->line[line_id].opcode,
                       opCodeOperationStr[entry_p->line[line_id].opcode],
                       entry_p->line[line_id].data, entry_p->line[line_id].last,
                       entry_p->line[line_id].updt_ipv4, entry_p->line[line_id].updt_tcp);
            }
        }
    }
    return TPM_OK;
}

int32_t tpm_mod2_print_rule (tpm_gmacs_enum_t   gmac_port,
                             uint32_t           mod_entry)
{
    tpm_mod_rule_t mod_rule[TPM_MAX_MOD_RULE_NUM];
    uint16_t       rule_num, rule_idx;

    if (tpm_mod2_entry_get(gmac_port, mod_entry, &rule_num, mod_rule) != TPM_OK)
    {
        printk("Modification entry %d is not found\n", mod_entry);
        return TPM_NOT_FOUND;
    }

    tpm_mod2_print_header();

    for (rule_idx = 0; rule_idx < rule_num; rule_idx++)
    {
        printk(" %03d      0x%2x  %15s  0x%4.4x  %1d     %1d      %1d\n",
               mod_rule[rule_idx].entry_id, mod_rule[rule_idx].entry_data.opcode,
               opCodeOperationStr[mod_rule[rule_idx].entry_data.opcode],
               mod_rule[rule_idx].entry_data.data, mod_rule[rule_idx].entry_data.last,
               mod_rule[rule_idx].entry_data.updt_ipv4, mod_rule[rule_idx].entry_data.updt_tcp);
    }


    return TPM_OK;
}

int32_t tpm_mod2_print_jump_all (tpm_gmacs_enum_t   gmac_port)
{
    tpm_db_mod2_show_jump_pmt_entries(gmac_port);
    return TPM_OK;
}

int32_t tpm_mod2_print_main_all (tpm_gmacs_enum_t   gmac_port)
{
    tpm_db_mod2_show_chain_pmt_entries(gmac_port, TPM_CHAIN_TYPE_NONE);
    return TPM_OK;
}

int32_t tpm_mod2_print_config (tpm_gmacs_enum_t   gmac_port)
{
    uint32_t chain_type;

    tpm_db_mod2_show_jump_cfg_data();
    for (chain_type = TPM_CHAIN_TYPE_MH; chain_type < TPM_CHAIN_TYPE_MAX; chain_type++)
    {
        tpm_db_mod2_show_chain_cfg_data(chain_type);
    }

    return TPM_OK;
}

/*tpm check*/
/*******************************************************************************
* tpm_mod2_main_chain_index_get - Get main chain index from HW or DB
*
* INPUT:
*       gmac_port - target gmac port
*	jump_idx - jump entry index
*	from - get main chain index from DB or HW
*
* OUTPUT:
*	main_entry - main chain index
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t	tpm_mod2_main_chain_index_get(tpm_gmacs_enum_t gmac_port, uint16_t jump_idx,
				      uint16_t *main_entry, tpm_self_check_enable_t from)
{
    int err;
    tpm_mod2_entry_t db_jump_pattern;
    MV_NETA_PMT pEntry;

    /*params check*/
    if (main_entry == NULL) {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "NULL pointer\n");
        return TPM_FAIL;
    }
    if (from != TPM_CHECK_MAIN_CHAIN_IDX_DB && from != TPM_CHECK_MAIN_CHAIN_IDX_HW) {
        TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Invalid input\n");
        return TPM_FAIL;
    }

    if (from == TPM_CHECK_MAIN_CHAIN_IDX_DB) {
        /*get main chain index from DB*/
        spin_lock_bh(&tpmMod2JumpEntryLock);
        if (tpm_db_mod2_get_jump_entry_pattern_data(gmac_port, jump_idx, &db_jump_pattern) != TPM_OK) {
            spin_unlock_bh(&tpmMod2JumpEntryLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get db jump entry pattern_data\n");
            return TPM_FAIL;
        }
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        *main_entry = db_jump_pattern.data;
    } else if (from == TPM_CHECK_MAIN_CHAIN_IDX_HW) {
        /*get main chain index from HW*/
        spin_lock_bh(&tpmMod2PmtHwLock);
        err = mvNetaPmtRead(gmac_port, jump_idx, &pEntry);
        if (err) {
            spin_unlock_bh(&tpmMod2PmtHwLock);
            TPM_OS_ERROR(TPM_MODZ2_HM_MOD, "Failed to get hw jump data\n");
            return TPM_FAIL;
        }
        spin_unlock_bh(&tpmMod2PmtHwLock);
        *main_entry = pEntry.word & 0xFFFF;
    }

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_neta_pmt_get - read HW PMT entry from target port
*
* INPUT:
*       gmac_port - target gmac port
*	idx - entry index
*
* OUTPUT:
*	pEntry - pmt entry wanted
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t tpm_mod2_neta_pmt_get(tpm_gmacs_enum_t gmac_port, int idx, MV_NETA_PMT *pEntry)
{
    spin_lock_bh(&tpmMod2PmtHwLock);
    if (mvNetaPmtRead(gmac_port, idx, pEntry)) {
        spin_unlock_bh(&tpmMod2PmtHwLock);
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2PmtHwLock);

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_db_jump_pattern_data_get - read jump entry pattern data from tpm db
*
* INPUT:
*       gmac_port - target gmac port
*	entry_id - jump entry index
*
* OUTPUT:
*	pattern - pattern data
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t tpm_mod2_db_jump_pattern_data_get(tpm_gmacs_enum_t gmac_port, uint16_t entry_id, tpm_mod2_entry_t *pattern)
{
    spin_lock_bh(&tpmMod2JumpEntryLock);
    if (tpm_db_mod2_get_jump_entry_pattern_data(gmac_port, entry_id, pattern) != TPM_OK) {
        spin_unlock_bh(&tpmMod2JumpEntryLock);
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2JumpEntryLock);

    return TPM_OK;
}

/*******************************************************************************
* tpm_mod2_db_jump_pattern_data_get - read chain entry pattern data from tpm db
*
* INPUT:
*       gmac_port - target gmac port
*	chain_type - chain type
*	chain_id - chain index
*	pattern_id - pattern index
*
* OUTPUT:
*	pattern - pattern data
*
*RETURN:
*
*COMMENTS:
*
*******************************************************************************/
int32_t tpm_mod2_db_chain_pattern_data_get(tpm_gmacs_enum_t gmac_port, tpm_chain_type_t chain_type,
					   uint16_t chain_id, uint16_t pattern_id, tpm_mod2_entry_t *pattern)
{
    spin_lock_bh(&tpmMod2ChainEntryLock);
    if (tpm_db_mod2_get_chain_entry_pattern_data(gmac_port, chain_type, chain_id, pattern_id, pattern)) {
        spin_unlock_bh(&tpmMod2ChainEntryLock);
        return TPM_FAIL;
    }
    spin_unlock_bh(&tpmMod2ChainEntryLock);

    return TPM_OK;
}

