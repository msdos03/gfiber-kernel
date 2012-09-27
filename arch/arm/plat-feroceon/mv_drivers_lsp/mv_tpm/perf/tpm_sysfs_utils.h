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
* tpm_usr_tpmsysfs_db.h
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
#ifndef _TPM_USR_TPM_SYSFS_DB_H_
#define _TPM_USR_TPM_SYSFS_DB_H_

#define DB_TPMCFG_MAX_NAME         32
#define DB_TPMCFG_MAX_ENTRIES      512


extern void tpm_sysfs_rule_db_init(void);


//
// Miscellaneous
//
// Internal rule type classification
typedef enum
{
    rule_type_l2, rule_type_l3, rule_type_ipv4, rule_type_ipv6
} rule_type_t;

typedef struct
{
    int  value;
    char *str;
} string_enum_pair_t;

typedef struct
{
    string_enum_pair_t *string_enum_pair;
    int                num_entries;
} string_enum_pair_db_t;
extern GT_BOOL get_srcport_value        (char *str, unsigned int *value);
extern GT_BOOL get_srcdir_value         (char *str, unsigned int *value);
extern GT_BOOL get_phase_value          (char *str, unsigned int *value);
extern GT_BOOL get_bool_value           (char *str, unsigned int *value);
extern GT_BOOL get_igmp_mode_value      (char *str, unsigned int *value);
extern GT_BOOL get_igmp_srcport_value   (char *str, unsigned int *value);
extern GT_BOOL get_lx_ipvy_value        (char *str, unsigned int *value);
extern GT_BOOL get_direction_value      (char *str, unsigned int *value);
extern GT_BOOL get_scheduling_mode_value(char *str, unsigned int *value);
extern GT_BOOL get_sched_entity_value   (char *str, unsigned int *value);
extern GT_BOOL get_vlan_op_value        (char *str, unsigned int *value);

extern char *get_tpm_err_str(int value);


extern GT_BOOL parse_ipv4_address (char *buf, uint32_t *ipv4addr_parts);
extern GT_BOOL parse_ipv6_address (char *buf, uint32_t *ipv6addr_parts);
extern GT_BOOL parse_mac_address  (char *buf, uint32_t *macaddr_parts);



extern int  count_parameters     (const char *buf);
extern void print_horizontal_line(unsigned int char_count);
extern void print_help_buffer    (char *helpbuf, int len);
extern void parm_error_completion(int numparms, int reqdparms, const char *buf, int (*help_text_routine)(char *));


//
// Common DB structure for entries
//
typedef struct
{
    int  max_num_entries;
    int  num_entries;
    int  size_entry;
    void *entryAra;
} tpm_generic_rule_db_t;


// Frwd packet entry and DB API
typedef struct
{
    char                name[DB_TPMCFG_MAX_NAME+1];
    tpm_pkt_frwd_t      frwd;
} tpmcfg_frwd_entry_t;

extern tpmcfg_frwd_entry_t *find_tpm_pkt_frwd_entry_by_name(char *name);
extern tpmcfg_frwd_entry_t *find_free_tpm_pkt_frwd_entry   (void);
extern GT_BOOL             del_tpm_pkt_frwd_entry_by_name  (char *name);
extern void                show_tpm_pkt_frwd_db(void);



// VLAN entry and DB API
typedef struct
{
    char                name[DB_TPMCFG_MAX_NAME+1];
    tpm_vlan_key_t      vlan;
} tpmcfg_vlan_entry_t;

extern tpmcfg_vlan_entry_t *find_tpm_vlan_entry_by_name(char *name);
extern tpmcfg_vlan_entry_t *find_free_tpm_vlan_entry   (void);
extern GT_BOOL             del_tpm_vlan_entry_by_name  (char *name);
extern void                show_tpm_vlan_db(void);


// Mod (packet modification) entry and DB API
//
// Flags to indicate which fields are defined
#define MOD_ENTRY_FLAG_IPV4        0x1
#define MOD_ENTRY_FLAG_IPV6        0x2
#define MOD_ENTRY_FLAG_PPPOE       0x4
#define MOD_ENTRY_FLAG_MAC         0x8
#define MOD_ENTRY_FLAG_VLAN        0x10
#define MOD_ENTRY_FLAG_MH          0x20

typedef struct
{
    char                name[DB_TPMCFG_MAX_NAME+1];
    tpm_pkt_mod_t       mod;
    uint32_t            flags;
} tpmcfg_mod_entry_t;

extern tpmcfg_mod_entry_t  *find_tpm_mod_entry_by_name(char *name);
extern tpmcfg_mod_entry_t  *find_free_tpm_mod_entry   (void);
extern GT_BOOL             del_tpm_mod_entry_by_name  (char *name);
extern void                show_tpm_mod_db(void);

// L2 ACL entry and DB API
typedef struct
{
    char                   name[DB_TPMCFG_MAX_NAME+1];
    tpm_l2_acl_key_t       l2_acl;
} tpmcfg_l2_key_entry_t;

extern tpmcfg_l2_key_entry_t  *find_tpm_l2_key_entry_by_name(char *name);
extern tpmcfg_l2_key_entry_t  *find_free_tpm_l2_key_entry  (void);
extern GT_BOOL                del_tpm_l2_key_entry_by_name  (char *name);
extern void                   show_tpm_l2_key_db(void);

// L3 ACL entry and DB API
typedef struct
{
    char                   name[DB_TPMCFG_MAX_NAME+1];
    tpm_l3_type_key_t      l3_acl;
} tpmcfg_l3_key_entry_t;

extern tpmcfg_l3_key_entry_t  *find_tpm_l3_key_entry_by_name(char *name);
extern tpmcfg_l3_key_entry_t  *find_free_tpm_l3_key_entry  (void);
extern GT_BOOL                del_tpm_l3_key_entry_by_name  (char *name);
extern void                   show_tpm_l3_key_db(void);

// IPV4 ACL entry and DB API
typedef struct
{
    char                   name[DB_TPMCFG_MAX_NAME+1];
    tpm_ipv4_acl_key_t     ipv4_acl;
} tpmcfg_ipv4_key_entry_t;

extern tpmcfg_ipv4_key_entry_t  *find_tpm_ipv4_key_entry_by_name(char *name);
extern tpmcfg_ipv4_key_entry_t  *find_free_tpm_ipv4_key_entry  (void);
extern GT_BOOL                  del_tpm_ipv4_key_entry_by_name  (char *name);
extern void                     show_tpm_ipv4_key_db(void);

// IPV6 ACL entry and DB API
typedef struct
{
    char               name[DB_TPMCFG_MAX_NAME+1];
    tpm_ipv6_acl_key_t ipv6_acl;
} tpmcfg_ipv6_key_entry_t;

extern tpmcfg_ipv6_key_entry_t  *find_tpm_ipv6_key_entry_by_name(char *name);
extern tpmcfg_ipv6_key_entry_t  *find_free_tpm_ipv6_key_entry(void);
extern GT_BOOL                   del_tpm_ipv6_key_entry_by_name(char *name);
extern void                      show_tpm_ipv6_key_db(void);

// IPV6 gen ACL entry and DB API
typedef struct
{
    char                   name[DB_TPMCFG_MAX_NAME+1];
    tpm_ipv6_gen_acl_key_t ipv6gen_acl;
} tpmcfg_ipv6_gen_key_entry_t;

typedef struct
{
    char                name[DB_TPMCFG_MAX_NAME+1];
    tpm_ipv6_addr_key_t ipv6_addr;
} tpmcfg_ipv6_dip_key_entry_t;

typedef struct
{
    char                name[DB_TPMCFG_MAX_NAME+1];
    tpm_l4_ports_key_t  l4_ports;
} tpmcfg_ipv6_l4_ports_key_entry_t;

extern tpmcfg_ipv6_gen_key_entry_t       *find_tpm_ipv6_gen_key_entry_by_name(char *name);
extern tpmcfg_ipv6_gen_key_entry_t       *find_free_tpm_ipv6_gen_key_entry(void);
extern GT_BOOL                            del_tpm_ipv6_gen_key_entry_by_name(char *name);
extern void                               show_tpm_ipv6_gen_key_db(void);
extern tpmcfg_ipv6_dip_key_entry_t       *find_tpm_ipv6_dip_key_entry_by_name(char *name);
extern tpmcfg_ipv6_dip_key_entry_t       *find_free_tpm_ipv6_dip_key_entry(void);
extern GT_BOOL                            del_tpm_ipv6_dip_key_entry_by_name(char *name);
extern void                               show_tpm_ipv6_dip_key_db(void);
extern tpmcfg_ipv6_l4_ports_key_entry_t  *find_tpm_ipv6_l4_ports_key_entry_by_name(char *name);
extern tpmcfg_ipv6_l4_ports_key_entry_t  *find_free_tpm_ipv6_l4_ports_key_entry(void);
extern GT_BOOL                            del_tpm_ipv6_l4_ports_key_entry_by_name(char *name);
extern void                               show_tpm_ipv6_l4_ports_key_db(void);


extern void tpm_reset_mc_vid_key(void);

extern GT_BOOL tpm_set_mc_vid_key(
    uint32_t                 src_port,
    uint32_t                 mc_uni_xlate_mode,
    uint32_t                 mc_uni_xlate_vid
);
extern tpm_error_code_t tpm_set_mc_vid_cfg(
    uint32_t                 mc_vid
);
tpm_error_code_t tpm_proc_send_genquery_to_uni(tpm_trg_port_type_t dest_port_bm, uint32_t packet_num);


#endif  /* _TPM_USR_TPM_SYSFS_DB_H_*/
