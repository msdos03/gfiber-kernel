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

#include <linux/ctype.h>
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_sysfs_utils.h"
#include "tpm_sysfs_help.h"
#include "tpm_sysfs_hwcall.h"

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
extern struct semaphore    tpm_sfs_2_ioctl_sem;
extern tpm_ioctl_mng_t     tpm_sfs_2_ioctl_command;
 #define PR_RULE_IDX(rule_idx)  {}
 #define PR_HIT_COUNT(hit_count){}
 #define PR_RESULT              {}
 #define PR_MAC_COUNT           {}
#else
 #define PR_RULE_IDX(rule_idx)  {printk(KERN_INFO "OK. rule_idx = %d\n", rule_idx);}
 #define PR_HIT_COUNT(hit_count){printk(KERN_INFO "OK. hit_count = %d\n", hit_count);}
 #define PR_RESULT              {printk(KERN_INFO "OK.\n");}
 #define PR_MAC_COUNT(count)    {printk(KERN_INFO "OK. mac_learn_entry_count = %d\n", count);}
#endif

static char *l2_key_empty_name   = "l2_key_empty";
static char *l3_key_empty_name   = "l3_key_empty";
static char *ipv4_key_empty_name = "ipv4_key_empty";
static char *ipv6_key_empty_name = "ipv6_key_empty";
static char *frwd_empty_name     = "frwd_empty";
static char *mod_empty_name      = "mod_empty";
char *ipv6_gen_key_empty_name = "ipv6_gen_key_empty";
char *ipv6_dip_key_empty_name = "ipv6_dip_key_empty";
char *ipv6_l4_key_empty_name = "ipv6_l4_key_empty";

extern int32_t module_tpmSetup(tpm_setup_t *tpm_initp);


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_l2_rule_bounce(uint32_t                owner_id,
                                        tpm_src_port_type_t     src_port,
                                        uint32_t                rule_num,
                                        uint32_t               *rule_idx,
                                        tpm_parse_fields_t      parse_rule_bm,
                                        tpm_parse_flags_t       parse_flags_bm,
                                        tpm_l2_acl_key_t       *l2_key,
                                        tpm_pkt_frwd_t         *pkt_frwd,
                                        tpm_pkt_mod_t          *pkt_mod,
                                        tpm_pkt_mod_bm_t        pkt_mod_bm,
                                        tpm_rule_action_t      *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_ADD_L2_ACL_RULE;
    tpm_add_acl_rule->owner_id                      = owner_id;
    tpm_add_acl_rule->src_port                      = src_port;
    tpm_add_acl_rule->rule_num                      = rule_num;
    tpm_add_acl_rule->parse_rule_bm                 = parse_rule_bm;
    tpm_add_acl_rule->l2_acl_rule.parse_flags_bm    = parse_flags_bm;
    tpm_add_acl_rule->l2_acl_rule.pkt_mod_bm        = pkt_mod_bm;
    memcpy(&(tpm_add_acl_rule->l2_acl_rule.l2_key),         (void*)l2_key,      sizeof(tpm_l2_acl_key_t));
    memcpy(&(tpm_add_acl_rule->l2_acl_rule.pkt_frwd),       (void*)pkt_frwd,    sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->l2_acl_rule.pkt_mod),        (void*)pkt_mod,     sizeof(tpm_pkt_mod_t));
    memcpy(&(tpm_add_acl_rule->l2_acl_rule.rule_action),    (void*)rule_action, sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_l2_rule tpm_add_l2_rule_bounce
#else
 #define _tpm_add_l2_rule tpm_add_l2_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_l2_rule_add
*
* DESCRIPTION:
*           This function creates a L2 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_l2_rule_add  (const char *buf, size_t len)
{
    typedef enum
    {
        l2ruleadd_owner=0, l2ruleadd_srcport, l2ruleadd_rulenum, l2ruleadd_parserulebm, l2ruleadd_parseflagbm, l2ruleadd_action,  l2ruleadd_nextphase,
        l2ruleadd_l2keyname, l2ruleadd_frwdname, l2ruleadd_modname, l2ruleadd_pktmodbm, l2ruleadd_max
    } l2ruleadd_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    char                     srcport_str[20];
    char                     uc_srcport_str[20];
    int                      indx;
    uint32_t                 src_port;
    uint32_t                 rulenum;
    uint32_t                 parserulebm;
    uint32_t                 parseflagbm;
    uint32_t                 pkt_mod_bm;
    uint32_t                 action;
    char                     nextphase_str[20];
    uint32_t                 nextphase;
    char                     l2keyname[20];
    char                     frwdname[20];
    char                     modname[20];
    int                      parsedargs;
    int                      numparms;
    // DB
    tpmcfg_frwd_entry_t      *pdbfrwdentry  = 0;
    tpmcfg_l2_key_entry_t    *pdbl2keyentry = 0;
    tpmcfg_mod_entry_t       *pdbmodentry = 0;
    //Used in API call
    tpm_rule_action_t        rule_action;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_l2_acl_key_t         l2_acl;
    tpm_pkt_mod_t            pkt_mod;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != l2ruleadd_max)
    {
        parm_error_completion(numparms, l2ruleadd_max, buf, sfs_help_l2_rule_add);
//        if (numparms == 1 && buf[0] == '?')
//        {
//            char helpbuf[3000];
//            int  helpbuf_len;
//
//            helpbuf_len = sfs_help_l2_rule_add(helpbuf);
//            print_help_buffer(helpbuf, helpbuf_len);
//        }
//        else
//        {
//            printk(KERN_INFO "Parse problem: expected %d parameters, found %d\n", l2ruleadd_max, numparms);
//        }
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s %s %s %s 0x%x",  &ownerid, srcport_str, &rulenum, &parserulebm,
                                       &parseflagbm, &action, nextphase_str, l2keyname, frwdname, modname, &pkt_mod_bm);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], action[0x%x], nextphase_str[%s], l2keyname[%s], frwdname[%s], modname[%s], pkt_mod_bm[0x%x]\n",
               len, parsedargs, ownerid, srcport_str, rulenum, parserulebm, action, nextphase_str, l2keyname, frwdname, modname, pkt_mod_bm);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else if (((pdbl2keyentry = find_tpm_l2_key_entry_by_name(l2keyname)) == 0) && strcmp(l2keyname, l2_key_empty_name) != 0)
        {
            printk(KERN_INFO "L2 key entry [%s] not found\n", l2keyname);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && strcmp(modname, mod_empty_name) != 0)
        {
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbl2keyentry != 0)  memcpy(&l2_acl, &pdbl2keyentry->l2_acl, sizeof(tpm_l2_acl_key_t));
            else                     memset(&l2_acl, 0,                      sizeof(tpm_l2_acl_key_t));

            if (pdbfrwdentry != 0)   memcpy(&pkt_frwd, &pdbfrwdentry->frwd,  sizeof(tpm_pkt_frwd_t));
            else                     memset(&pkt_frwd, 0,                    sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)    memcpy(&pkt_mod, &pdbmodentry->mod,     sizeof(tpm_pkt_mod_t));
            else                     memset(&pkt_mod, 0,                     sizeof(tpm_pkt_mod_t));

            if ((rc = _tpm_add_l2_rule(ownerid,
                                       src_port,
                                       rulenum,
                                       &rule_idx,
                                       parserulebm,
                                       parseflagbm,
                                       &l2_acl,
                                       &pkt_frwd,
                                       &pkt_mod,
                                       pkt_mod_bm,
                                       &rule_action)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_l2_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_l3_type_rule_bounce(uint32_t               owner_id,
                                             tpm_src_port_type_t     src_port,
                                             uint32_t                rule_num,
                                             uint32_t               *rule_idx,
                                             tpm_parse_fields_t      parse_rule_bm,
                                             tpm_parse_flags_t       parse_flags_bm,
                                             tpm_l3_type_key_t      *l3_key,
                                             tpm_pkt_frwd_t         *pkt_frwd,
                                             tpm_rule_action_t      *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_ADD_L3_ACL_RULE;
    tpm_add_acl_rule->owner_id                      = owner_id;
    tpm_add_acl_rule->src_port                      = src_port;
    tpm_add_acl_rule->rule_num                      = rule_num;
    tpm_add_acl_rule->parse_rule_bm                 = parse_rule_bm;
    tpm_add_acl_rule->l3_acl_rule.parse_flags_bm    = parse_flags_bm;
    memcpy(&(tpm_add_acl_rule->l3_acl_rule.l3_key),         (void*)l3_key,      sizeof(tpm_l3_type_key_t));
    memcpy(&(tpm_add_acl_rule->l3_acl_rule.pkt_frwd),       (void*)pkt_frwd,    sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->l3_acl_rule.rule_action),    (void*)rule_action, sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_l3_type_rule tpm_add_l3_type_rule_bounce
#else
 #define _tpm_add_l3_type_rule tpm_add_l3_type_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_l3_rule_add
*
* DESCRIPTION:
*           This function creates a L3 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_l3_rule_add  (const char *buf, size_t len)
{
    typedef enum
    {
        l3ruleadd_owner=0, l3ruleadd_srcport, l3ruleadd_rulenum, l3ruleadd_parserulebm, l3ruleadd_parseflagsbm, l3ruleadd_action, l3ruleadd_nextphase,
        l2ruleadd_frwdname, l3ruleadd_l3keyname, l3ruleadd_max
    } l3ruleadd_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    char                     srcport_str[20];
    char                     uc_srcport_str[20];
    int                      indx;
    uint32_t                 src_port;
    uint32_t                 rulenum;
    uint32_t                 parserulebm;
    uint32_t                 parseflagbm;
    uint32_t                 action;
    char                     nextphase_str[20];
    uint32_t                 nextphase;
    char                     frwdname[20];
    char                     l3keyname[20];
    int                      parsedargs;
    int                      numparms;
    // DB
    tpmcfg_l3_key_entry_t    *pdbl3keyentry = 0;
    tpmcfg_frwd_entry_t      *pdbfrwdentry  = 0;
    // Used in API call
    tpm_rule_action_t        rule_action;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_l3_type_key_t        l3_acl;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != l3ruleadd_max)
    {
        parm_error_completion(numparms, l3ruleadd_max, buf, sfs_help_l3_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s %s %s",
                            &ownerid, srcport_str, &rulenum, &parserulebm, &parseflagbm, &action, nextphase_str, l3keyname, frwdname);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], parseflagbm[0x%x], action[0x%x], nextphase_str[%s], l3keyname[%s], frwdname[%s]\n",
               len, parsedargs, ownerid, srcport_str, rulenum, parserulebm, parseflagbm, action, nextphase_str, l3keyname, frwdname);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port[%s]\n", uc_srcport_str);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (((pdbl3keyentry = find_tpm_l3_key_entry_by_name(l3keyname)) == 0) && strcmp(l3keyname, l3_key_empty_name) != 0)
        {
            printk(KERN_INFO "L3 key entry [%s] not found\n", l3keyname);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbl3keyentry != 0)  memcpy(&l3_acl, &pdbl3keyentry->l3_acl, sizeof(tpm_l3_type_key_t));
            else                     memset(&l3_acl, 0,                      sizeof(tpm_l3_type_key_t));

            if (pdbfrwdentry != 0)   memcpy(&pkt_frwd, &pdbfrwdentry->frwd,  sizeof(tpm_pkt_frwd_t));
            else                     memset(&pkt_frwd, 0,                    sizeof(tpm_pkt_frwd_t));

            if ((rc = _tpm_add_l3_type_rule(ownerid,
                                            src_port,
                                            rulenum,
                                            &rule_idx,
                                            parserulebm,
                                            parseflagbm,
                                            &l3_acl,
                                            &pkt_frwd,
                                            &rule_action)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_l3_type_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv4_rule_bounce(uint32_t                owner_id,
                                          tpm_src_port_type_t     src_port,
                                          uint32_t                rule_num,
                                          uint32_t               *rule_idx,
                                          tpm_parse_fields_t      parse_rule_bm,
                                          tpm_parse_flags_t       parse_flags_bm,
                                          tpm_ipv4_acl_key_t     *ipv4_key,
                                          tpm_pkt_frwd_t         *pkt_frwd,
                                          tpm_pkt_mod_t          *pkt_mod,
                                          tpm_pkt_mod_bm_t        pkt_mod_bm,
                                          tpm_rule_action_t      *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_ADD_IPv4_ACL_RULE;
    tpm_add_acl_rule->owner_id                      = owner_id;
    tpm_add_acl_rule->src_port                      = src_port;
    tpm_add_acl_rule->rule_num                      = rule_num;
    tpm_add_acl_rule->parse_rule_bm                 = parse_rule_bm;
    tpm_add_acl_rule->ipv4_acl_rule.parse_flags_bm  = parse_flags_bm;
    tpm_add_acl_rule->ipv4_acl_rule.pkt_mod_bm      = pkt_mod_bm;
    memcpy(&(tpm_add_acl_rule->ipv4_acl_rule.ipv4_key),     (void*)ipv4_key,    sizeof(tpm_ipv4_acl_key_t));
    memcpy(&(tpm_add_acl_rule->ipv4_acl_rule.pkt_frwd),     (void*)pkt_frwd,    sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ipv4_acl_rule.pkt_mod),      (void*)pkt_mod,     sizeof(tpm_pkt_mod_t));
    memcpy(&(tpm_add_acl_rule->ipv4_acl_rule.rule_action),  (void*)rule_action, sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv4_rule tpm_add_ipv4_rule_bounce
#else
 #define _tpm_add_ipv4_rule tpm_add_ipv4_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_ipv4_rule_add
*
* DESCRIPTION:
*           This function creates a IPV4 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv4_rule_add  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv4ruleadd_owner=0, ipv4ruleadd_srcport,   ipv4ruleadd_rulenum,  ipv4ruleadd_parserulebm, ipv4ruleadd_parseflagsbm,
        ipv4ruleadd_action,  ipv4ruleadd_nextphase, ipv4ruleadd_frwdname, ipv4ruleadd_ipv4keyname,   ipv4ruleadd_modbm,
        ipv4ruleadd_modname, ipv4ruleadd_max
    } ipv4ruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    char                     srcport_str[20];
    char                     uc_srcport_str[20];
    int                      indx;
    uint32_t                 src_port;
    uint32_t                 rulenum;
    uint32_t                 parserulebm;
    uint32_t                 parseflagsbm;
    uint32_t                 action;
    uint32_t                 nextphase;
    char                     nextphase_str[20];
    uint32_t                 modbm;
    char                     ipv4keyname[20];
    char                     frwdname[20];
    char                     modname[20];
    int                      parsedargs;
    int                      numparms;
    // DB
    tpmcfg_ipv4_key_entry_t  *pdbipv4keyentry = 0;
    tpmcfg_frwd_entry_t      *pdbfrwdentry    = 0;
    tpmcfg_mod_entry_t       *pdbmodentry     = 0;
    // Used in API call
    tpm_rule_action_t        rule_action;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    tpm_ipv4_acl_key_t       ipv4_acl;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ipv4ruleadd_max)
    {
        parm_error_completion(numparms, ipv4ruleadd_max, buf, sfs_help_ipv4_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s 0x%x %s %s %s", &ownerid, srcport_str, &rulenum, &parserulebm, &parseflagsbm,
                            &action, nextphase_str, &modbm, ipv4keyname, frwdname, modname);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], modbm[0x%x], ipv4keyname[%s], frwdname[%s], modname[%s]\n",
//               len, parsedargs, ownerid, srcport_str, rulenum, parserulebm, parseflagsbm, action, nextphase_str, modbm, ipv4keyname, frwdname, modname);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else if (((pdbipv4keyentry = find_tpm_ipv4_key_entry_by_name(ipv4keyname)) == 0) && strcmp(ipv4keyname, ipv4_key_empty_name) != 0)
        {
            printk(KERN_INFO "IPV4 key entry [%s] not found\n", ipv4keyname);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && strcmp(modname, mod_empty_name) != 0)
        {
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbipv4keyentry != 0)  memcpy(&ipv4_acl, &pdbipv4keyentry->ipv4_acl, sizeof(tpm_ipv4_acl_key_t));
            else                       memset(&ipv4_acl, 0,                          sizeof(tpm_ipv4_acl_key_t));

            if (pdbfrwdentry != 0)     memcpy(&pkt_frwd, &pdbfrwdentry->frwd,        sizeof(tpm_pkt_frwd_t));
            else                       memset(&pkt_frwd, 0,                          sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)      memcpy(&pkt_mod, &pdbmodentry->mod,           sizeof(tpm_pkt_mod_t));
            else                       memset(&pkt_mod, 0,                           sizeof(tpm_pkt_mod_t));

            if ((rc = _tpm_add_ipv4_rule(ownerid,
                                         src_port,
                                         rulenum,
                                         &rule_idx,
                                         parserulebm,
                                         parseflagsbm,
                                         &ipv4_acl,
                                         &pkt_frwd,
                                         &pkt_mod,
                                         modbm,
                                         &rule_action)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv4_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv4_mc_stream_bounce(uint32_t                 owner_id,
                                              uint32_t                  stream_num,
                                              tpm_mc_igmp_mode_t        igmp_mode,
                                              uint8_t                   mc_stream_pppoe,
                                              uint16_t                  vid,
                                              uint8_t                   ipv4_src_add[4],
                                              uint8_t                   ipv4_dst_add[4],
                                              uint8_t                   ignore_ipv4_src,
                                              tpm_trg_port_type_t       dest_port_bm)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd             = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd                     = MV_TPM_IOCTL_ADD_IPv4_MC_STREAM;
    tpm_mc_rule->stream_num                 = stream_num;
    tpm_mc_rule->igmp_mode                  = igmp_mode;
    tpm_mc_rule->mc_stream_pppoe            = mc_stream_pppoe;
    tpm_mc_rule->vid                        = vid;
    tpm_mc_rule->ipv4_mc.ignore_ipv4_src    = ignore_ipv4_src;
    tpm_mc_rule->dest_port_bm               = dest_port_bm;
    memcpy(&(tpm_mc_rule->ipv4_mc.ipv4_src_add[0]), ipv4_src_add, sizeof(uint8_t)*4);
    memcpy(&(tpm_mc_rule->ipv4_mc.ipv4_dst_add[0]), ipv4_dst_add, sizeof(uint8_t)*4);

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv4_mc_stream tpm_add_ipv4_mc_stream_bounce
#else
 #define _tpm_add_ipv4_mc_stream tpm_add_ipv4_mc_stream
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_mc_ipv4_stream_add
*
* DESCRIPTION:
*           This function creates a multicast IPV4 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mc_ipv4_stream_add  (const char *buf, size_t len)
{
    typedef enum
    {
        mcipv4add_owner=0, mcipv4add_stream, mcipv4add_mode,   mcipv4add_pppoe,        mcipv4add_vid,
        mcipv4add_src_ip,  mcipv4add_dst_ip, mcipv4add_ignore, mcipv4add_target_ports, mcipv4add_max
    } mcipv4add_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 stream;
    uint32_t                 mode;
    char                     mc_pppoe_str[20];
    uint32_t                 mc_stream_pppoe;
    uint32_t                 vid;
    char                     srcip_str[30];
    char                     dstip_str[30];
    uint32_t                 temp_srcip[4];
    uint32_t                 temp_dstip[4];
    uint8_t                  srcip[4];
    uint8_t                  dstip[4];
    char                     ignore_str[20];
    uint32_t                 ignore;
    uint32_t                 target_ports;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != mcipv4add_max)
    {
        parm_error_completion(numparms, mcipv4add_max, buf, sfs_help_mc_ipvx_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d %s %d %s %s %s 0x%x",
                            &ownerid, &stream, &mode, mc_pppoe_str, &vid, srcip_str, dstip_str, ignore_str, &target_ports);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream[%d], vid[%d], srcip_str[%s], dstip_str[%s], ignore_str[%s], target_ports[0x%x]\n",
//               len, parsedargs, ownerid, stream, vid, srcip_str, dstip_str, ignore_str, target_ports);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_bool_value(mc_pppoe_str, &mc_stream_pppoe) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid ignore[%s]\n", ignore_str);
        }
        else if (parse_ipv4_address(srcip_str, temp_srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_ip [%s]\n", srcip_str);
        }
        else if (parse_ipv4_address(dstip_str, temp_dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dst_ip [%s]\n", dstip_str);
        }
        else if (get_bool_value(ignore_str, &ignore) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid ignore[%s]\n", ignore_str);
        }
        else
        {
            int indx;

            for (indx = 0; indx < sizeof(srcip); indx++)
            {
                srcip[indx] = (uint8_t)temp_srcip[indx];
                dstip[indx] = (uint8_t)temp_dstip[indx];
            }

            if ((rc = _tpm_add_ipv4_mc_stream(ownerid,
                                              stream,
                                              mode,
                                              (uint8_t)mc_stream_pppoe,
                                              vid,
                                              srcip,
                                              dstip,
                                              (uint8_t)ignore,
                                              target_ports)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv4_mc_stream failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv4_mc_stream_set_queue_bounce(
					      uint32_t                 owner_id,
                                              uint32_t                  stream_num,
                                              tpm_mc_igmp_mode_t        igmp_mode,
                                              uint8_t                   mc_stream_pppoe,
                                              uint16_t                  vid,
                                              uint8_t                   ipv4_src_add[4],
                                              uint8_t                   ipv4_dst_add[4],
                                              uint8_t                   ignore_ipv4_src,
                                              uint16_t                  dest_queue,
                                              tpm_trg_port_type_t       dest_port_bm)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd             = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd                     = MV_TPM_IOCTL_ADD_IPv4_MC_STREAM_SET_QUEUE;
    tpm_mc_rule->stream_num                 = stream_num;
    tpm_mc_rule->igmp_mode                  = igmp_mode;
    tpm_mc_rule->mc_stream_pppoe            = mc_stream_pppoe;
    tpm_mc_rule->vid                        = vid;
    tpm_mc_rule->ipv4_mc.ignore_ipv4_src    = ignore_ipv4_src;
    tpm_mc_rule->dest_queue                 = dest_queue;
    tpm_mc_rule->dest_port_bm               = dest_port_bm;
    memcpy(&(tpm_mc_rule->ipv4_mc.ipv4_src_add[0]), ipv4_src_add, sizeof(uint8_t)*4);
    memcpy(&(tpm_mc_rule->ipv4_mc.ipv4_dst_add[0]), ipv4_dst_add, sizeof(uint8_t)*4);

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv4_mc_stream_set_queue tpm_add_ipv4_mc_stream_set_queue_bounce
#else
 #define _tpm_add_ipv4_mc_stream_set_queue tpm_add_ipv4_mc_stream_set_queue
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_mc_ipv4_stream_set_queue_add
*
* DESCRIPTION:
*           This function creates a multicast IPV4 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mc_ipv4_stream_set_queue_add  (const char *buf, size_t len)
{
    typedef enum
    {
        mcipv4add_owner=0, mcipv4add_stream, mcipv4add_mode,   mcipv4add_pppoe,        mcipv4add_vid,
        mcipv4add_src_ip,  mcipv4add_dst_ip, mcipv4add_ignore, mcipv4add_target_ports, mcipv4add_target_queue,
        mcipv4add_max
    } mcipv4add_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 stream;
    uint32_t                 mode;
    char                     mc_pppoe_str[20];
    uint32_t                 mc_stream_pppoe;
    uint32_t                 vid;
    char                     srcip_str[30];
    char                     dstip_str[30];
    uint32_t                 temp_srcip[4];
    uint32_t                 temp_dstip[4];
    uint8_t                  srcip[4];
    uint8_t                  dstip[4];
    char                     ignore_str[20];
    uint32_t                 ignore;
    uint32_t                 target_queue;
    uint32_t                 target_ports;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != mcipv4add_max)
    {
        parm_error_completion(numparms, mcipv4add_max, buf, sfs_help_mc_ipvx_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d %s %d %s %s %s %d 0x%x",
                            &ownerid, &stream, &mode, mc_pppoe_str, &vid, srcip_str, dstip_str, ignore_str, &target_queue, &target_ports);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream[%d], vid[%d], srcip_str[%s], dstip_str[%s], ignore_str[%s], target_ports[0x%x]\n",
//               len, parsedargs, ownerid, stream, vid, srcip_str, dstip_str, ignore_str, target_ports);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_bool_value(mc_pppoe_str, &mc_stream_pppoe) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid ignore[%s]\n", ignore_str);
        }
        else if (parse_ipv4_address(srcip_str, temp_srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_ip [%s]\n", srcip_str);
        }
        else if (parse_ipv4_address(dstip_str, temp_dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dst_ip [%s]\n", dstip_str);
        }
        else if (get_bool_value(ignore_str, &ignore) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid ignore[%s]\n", ignore_str);
        }
        else
        {
            int indx;

            for (indx = 0; indx < sizeof(srcip); indx++)
            {
                srcip[indx] = (uint8_t)temp_srcip[indx];
                dstip[indx] = (uint8_t)temp_dstip[indx];
            }

            if ((rc = _tpm_add_ipv4_mc_stream_set_queue(ownerid,
                                              stream,
                                              mode,
                                              (uint8_t)mc_stream_pppoe,
                                              vid,
                                              srcip,
                                              dstip,
                                              (uint8_t)ignore,
                                              target_queue,
                                              target_ports)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv4_mc_stream_set_queue failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_mc_stream_set_queue_bounce(uint32_t owner_id,
						uint32_t stream_num,
						tpm_mc_igmp_mode_t igmp_mode,
						uint8_t mc_stream_pppoe,
						uint16_t vid,
						uint8_t ipv6_src_add[16],
						uint8_t ipv6_dst_add[16],
						uint8_t ignore_ipv6_src,
						uint16_t dest_queue,
						tpm_trg_port_type_t dest_port_bm)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd             = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd                     = MV_TPM_IOCTL_ADD_IPv6_MC_STREAM_SET_QUEUE;
    tpm_mc_rule->stream_num                 = stream_num;
    tpm_mc_rule->igmp_mode                  = igmp_mode;
    tpm_mc_rule->mc_stream_pppoe            = mc_stream_pppoe;
    tpm_mc_rule->vid                        = vid;
    tpm_mc_rule->dest_port_bm               = dest_port_bm;
    tpm_mc_rule->dest_queue                 = dest_queue;
    memcpy(&(tpm_mc_rule->ipv6_mc.ipv6_dst_add[0]), ipv6_dst_add, sizeof(uint8_t) * 16);
    memcpy(&(tpm_mc_rule->ipv6_mc.ipv6_src_add[0]), ipv6_src_add, sizeof(uint8_t) * 16);
    tpm_mc_rule->ipv6_mc.ignore_ipv6_src = ignore_ipv6_src;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_mc_stream_set_queue tpm_add_ipv6_mc_stream_set_queue_bounce
#else
 #define _tpm_add_ipv6_mc_stream_set_queue tpm_add_ipv6_mc_stream_set_queue
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_mc_ipv6_stream_add_set_queue
*
* DESCRIPTION:
*           This function creates a multicast ipv6 rule with destination queue
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mc_ipv6_stream_set_queue_add  (const char *buf, size_t len)
{
    typedef enum
    {
        mcipv6add_owner=0, mcipv6add_stream, mcipv6add_mode,   mcipv6add_pppoe,        mcipv6add_vid,
        mcipv6add_src_ip, mcipv6add_dst_ip, mcipv6add_ignor_src_ip, mcipv6add_target_queue, mcipv6add_target_ports, mcipv6add_max
    } mcipv6add_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 stream;
    uint32_t                 mode;
    char                     mc_pppoe_str[20];
    uint32_t                 mc_stream_pppoe;
    uint32_t                 vid;
    char                     dstip_str[60];
    uint32_t                 temp_dstip[16];
    char                     srcip_str[60];
    uint32_t                 temp_srcip[16];
    uint8_t                  dstip[16];
    uint8_t                  srcip[16];
    uint32_t                 ignor_srcip;
    char                     ignore_str[20];
    uint32_t                 target_ports;
    uint32_t                 target_queue;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != mcipv6add_max)
    {
        parm_error_completion(numparms, mcipv6add_max, buf, sfs_help_mc_ipvx_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d %s %d %s %s %s %d 0x%x",
                            &ownerid, &stream, &mode, mc_pppoe_str, &vid, srcip_str, dstip_str, ignore_str, &target_queue, &target_ports);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream[%d], vid[%d], srcip_str[%s],  "
		"dstip_str[%s],  ignor_srcip[%s], target_queue[%d], target_ports[0x%x]\n",
               len, parsedargs, ownerid, stream, vid, srcip_str, dstip_str, ignore_str, target_queue, target_ports);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_bool_value(mc_pppoe_str, &mc_stream_pppoe) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid mc_pppoe_str[%s]\n", mc_pppoe_str);
        }
        else if (parse_ipv6_address(dstip_str, temp_dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dst_ip [%s]\n", dstip_str);
        }
        else if (parse_ipv6_address(srcip_str, temp_srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip_str [%s]\n", srcip_str);
	}
	else if (get_bool_value(ignore_str, &ignor_srcip) == GT_FALSE)
	{
	    printk(KERN_INFO "Invalid ignore[%s]\n", ignore_str);
        }
        else
        {
            int indx;

            for (indx = 0; indx < sizeof(dstip); indx++)
            {
                dstip[indx] = (uint8_t)temp_dstip[indx];
                srcip[indx] = (uint8_t)temp_srcip[indx];
            }

            if ((rc = _tpm_add_ipv6_mc_stream_set_queue(ownerid,
                                              stream,
                                              mode,
                                              (uint8_t)mc_stream_pppoe,
                                              vid,
                                              srcip,
                                              dstip,
                                              ignor_srcip,
                                              target_queue,
                                              target_ports)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv6_mc_stream_set_queue failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_mc_stream_bounce(uint32_t owner_id,
						uint32_t stream_num,
						tpm_mc_igmp_mode_t igmp_mode,
						uint8_t mc_stream_pppoe,
						uint16_t vid,
						uint8_t ipv6_src_add[16],
						uint8_t ipv6_dst_add[16],
						uint8_t ignore_ipv6_src,
						tpm_trg_port_type_t dest_port_bm)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd             = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd                     = MV_TPM_IOCTL_ADD_IPv6_MC_STREAM;
    tpm_mc_rule->stream_num                 = stream_num;
    tpm_mc_rule->igmp_mode                  = igmp_mode;
    tpm_mc_rule->mc_stream_pppoe            = mc_stream_pppoe;
    tpm_mc_rule->vid                        = vid;
    tpm_mc_rule->dest_port_bm               = dest_port_bm;
    memcpy(&(tpm_mc_rule->ipv6_mc.ipv6_dst_add[0]), ipv6_dst_add, sizeof(uint8_t) * 16);
    memcpy(&(tpm_mc_rule->ipv6_mc.ipv6_src_add[0]), ipv6_src_add, sizeof(uint8_t) * 16);
    tpm_mc_rule->ipv6_mc.ignore_ipv6_src = ignore_ipv6_src;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_mc_stream tpm_add_ipv6_mc_stream_bounce
#else
 #define _tpm_add_ipv6_mc_stream tpm_add_ipv6_mc_stream
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_mc_ipv6_stream_add
*
* DESCRIPTION:
*           This function creates a multicast ipv6 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mc_ipv6_stream_add  (const char *buf, size_t len)
{
    typedef enum
    {
        mcipv6add_owner=0, mcipv6add_stream, mcipv6add_mode,   mcipv6add_pppoe,        mcipv6add_vid,
        mcipv6add_src_ip, mcipv6add_dst_ip, mcipv6add_ignor_src_ip, mcipv6add_target_ports, mcipv6add_max
    } mcipv6add_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 stream;
    uint32_t                 mode;
    char                     mc_pppoe_str[20];
    uint32_t                 mc_stream_pppoe;
    uint32_t                 vid;
    char                     dstip_str[60];
    uint32_t                 temp_dstip[16];
    char                     srcip_str[60];
    uint32_t                 temp_srcip[16];
    uint8_t                  dstip[16];
    uint8_t                  srcip[16];
    uint32_t                 ignor_srcip;
    char                     ignore_str[20];
    uint32_t                 target_ports;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != mcipv6add_max)
    {
        parm_error_completion(numparms, mcipv6add_max, buf, sfs_help_mc_ipvx_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d %s %d %s %s %s 0x%x",
                            &ownerid, &stream, &mode, mc_pppoe_str, &vid, srcip_str, dstip_str, ignore_str, &target_ports);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream[%d], vid[%d], srcip_str[%s],  "
		"dstip_str[%s],  ignor_srcip[%s], target_ports[0x%x]\n",
               len, parsedargs, ownerid, stream, vid, srcip_str, dstip_str, ignore_str, target_ports);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_bool_value(mc_pppoe_str, &mc_stream_pppoe) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid mc_pppoe_str[%s]\n", mc_pppoe_str);
        }
        else if (parse_ipv6_address(dstip_str, temp_dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dst_ip [%s]\n", dstip_str);
        }
        else if (parse_ipv6_address(srcip_str, temp_srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip_str [%s]\n", srcip_str);
	}
	else if (get_bool_value(ignore_str, &ignor_srcip) == GT_FALSE)
	{
	    printk(KERN_INFO "Invalid ignore[%s]\n", ignore_str);
        }
        else
        {
            int indx;

            for (indx = 0; indx < sizeof(dstip); indx++)
            {
                dstip[indx] = (uint8_t)temp_dstip[indx];
                srcip[indx] = (uint8_t)temp_srcip[indx];
            }

            if ((rc = _tpm_add_ipv6_mc_stream(ownerid,
                                              stream,
                                              mode,
                                              (uint8_t)mc_stream_pppoe,
                                              vid,
                                              srcip,
                                              dstip,
                                              ignor_srcip,
                                              target_ports)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv6_mc_stream failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_port_igmp_frwd_mode_bounce(tpm_src_port_type_t  src_port,
                                                    tpm_igmp_frwd_mode_t mode)
{
    tpm_ioctl_igmp_t *tpm_igmp = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_igmp;

    tpm_sfs_2_ioctl_command.cmd   = MV_TPM_IOCTL_IGMP_SECTION;
    tpm_igmp->igmp_cmd          = MV_TPM_IOCTL_IGMP_SET_PORT_FRWD_MODE;
    tpm_igmp->src_port          = src_port;
    tpm_igmp->frwd_mode         = mode;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_set_port_igmp_frwd_mode tpm_set_port_igmp_frwd_mode_bounce
#else
 #define _tpm_set_port_igmp_frwd_mode tpm_set_port_igmp_frwd_mode
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_igmp_port_forward_mode_cfg
*
* DESCRIPTION:
*           This function creates a multicast IPV4 rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void  sfs_tpm_cfg_set_igmp_port_forward_mode_cfg (const char *buf, size_t len)
{
    typedef enum
    {
        igmpfrwd_owner=0, igmpfrwd_src_port, igmpfrwd_mode, igmpfrwd_max
    } igmpfrwd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;          // NOTE: not used in API!!
    char                     srcport_str[30];
    char                     uc_srcport_str[30];
    int                      indx;
    uint32_t                 src_port;
    char                     mode_str[30];
    uint32_t                 mode;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != igmpfrwd_max)
    {
        parm_error_completion(numparms, igmpfrwd_max, buf, sfs_help_igmp_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %s", &ownerid, srcport_str, mode_str);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], mode_str[%s]\n",
//               len, parsedargs, ownerid, srcport_str, mode_str);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_igmp_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port [%s]\n", srcport_str);
        }
        else if (get_igmp_mode_value(mode_str, &mode) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid mode [%s]\n", mode_str);
        }
        else
        {
            if ((rc = _tpm_set_port_igmp_frwd_mode(src_port,
                                                   mode)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_set_port_igmp_frwd_mode failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_igmp_cpu_rx_queue_bounce(uint32_t  queue)
{
    tpm_ioctl_igmp_t *tpm_igmp = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_igmp;

    tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_IGMP_SECTION;
    tpm_igmp->igmp_cmd          = MV_TPM_IOCTL_IGMP_SET_CPU_RX_Q;
    tpm_igmp->cpu_queue         = queue;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_set_igmp_cpu_rx_queue tpm_set_igmp_cpu_rx_queue_bounce
#else
 #define _tpm_set_igmp_cpu_rx_queue tpm_set_igmp_cpu_rx_queue
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_igmp_cpu_queue_cfg
*
* DESCRIPTION:
*           This function configres the CPU queue for IGMP
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_igmp_cpu_queue_cfg (const char *buf, size_t len)
{
    typedef enum
    {
        igmpcpuq_owner=0, igmpcpuq_queue, igmpcpuq_max
    } igmpcpuq_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;             // NOTE: not used in API!!
    uint32_t                 que;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != igmpcpuq_max)
    {
        parm_error_completion(numparms, igmpcpuq_max, buf, sfs_help_igmp_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &que);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], que[%d]\n",
//               len, parsedargs, ownerid, que);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (que >= TPM_MAX_NUM_RX_QUEUE)  // Hope this a good choice
        {
            printk(KERN_INFO "Invalid queue [%d]\n", que);
        }
        else
        {
            if ((rc = _tpm_set_igmp_cpu_rx_queue(que)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_set_igmp_cpu_rx_queue failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_igmp_proxy_sa_mac_bounce(uint8_t *sa_mac)
{
	tpm_ioctl_igmp_t *tpm_igmp_set_sa_mac = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_igmp;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_IGMP_SECTION;
	tpm_igmp_set_sa_mac->igmp_cmd = MV_TPM_IOCTL_SET_IGMP_PROXY_SA_MAC;
	memcpy(tpm_igmp_set_sa_mac->sa_mac, (void *)sa_mac, 6);

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_set_igmp_proxy_sa_mac tpm_set_igmp_proxy_sa_mac_bounce
#else
 #define _tpm_set_igmp_proxy_sa_mac tpm_set_igmp_proxy_sa_mac
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_igmp_proxy_sa_mac
*
* DESCRIPTION:
*           This function configures the sa_mac for IGMP proxy
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_igmp_proxy_sa_mac (const char *buf, size_t len)
{
    typedef enum
    {
        igmpproxysa_owner=0, igmpproxysa_mac, igmpproxysa_max
    } igmpproxysa_parm_indx_t;

    // shell line parsing
    unsigned int             ownerid, i;
    uint8_t                  mac[6];
    uint32_t                 temp_mac[6];
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;


    numparms = count_parameters(buf);
    if (numparms != igmpproxysa_max)
    {
        parm_error_completion(numparms, igmpproxysa_max, buf, sfs_help_igmp_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %02x:%02x:%02x:%02x:%02x:%02x",
                            &ownerid, &temp_mac[0], &temp_mac[1], &temp_mac[2], &temp_mac[3], &temp_mac[4], &temp_mac[5]);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], sa_mac[%02x:%02x:%02x:%02x:%02x:%02x]\n",
//               len, parsedargs, ownerid, temp_mac[0], temp_mac[1], temp_mac[2], temp_mac[3], temp_mac[4], temp_mac[5]);

        if (parsedargs != 7)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, 7);
        }
        else
        {
            for (i=0;i<6;i++)
            {
                mac[i] = temp_mac[i];
            }
            if ((rc = _tpm_set_igmp_proxy_sa_mac (mac)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_set_igmp_proxy_sa_mac failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_l2_rule_bounce(uint32_t        owner_id,
                                        uint32_t        rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_L2_ACL_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_l2_rule tpm_del_l2_rule_bounce
#else
 #define _tpm_del_l2_rule tpm_del_l2_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_l2
*
* DESCRIPTION:
*           This function deletes a L2 HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_l2  (const char *buf, size_t len)
{
    typedef enum
    {
        noruleaddl2_owner=0,  noruleaddl2_ruleidx,  noruleaddl2_max
    } noruleaddl2_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noruleaddl2_max)
    {
        parm_error_completion(numparms, noruleaddl2_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        //printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
        //       len, parsedargs, ownerid, rule_idx);


        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_del_l2_rule(ownerid,
                                       rule_idx)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_del_l2_prim_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_l3_type_rule_bounce(uint32_t           owner_id,
                                             uint32_t           rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_L3_ACL_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_l3_type_rule tpm_del_l3_type_rule_bounce
#else
 #define _tpm_del_l3_type_rule tpm_del_l3_type_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_l3
*
* DESCRIPTION:
*           This function deletes a L3 HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_l3  (const char *buf, size_t len)
{
    typedef enum
    {
        noruleaddl3_owner=0,  noruleaddl3_ruleidx,  noruleaddl3_max
    } noruleaddl3_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noruleaddl3_max)
    {
        parm_error_completion(numparms, noruleaddl3_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rule_idx[%d], parserulebm[0x%x], keyname[%s]\n",
//               len, parsedargs, ownerid, srcport_str, rule_idx, parserulebm, keyname);


        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_del_l3_type_rule(ownerid,
                                            rule_idx)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_del_l3_type_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv4_rule_bounce(uint32_t          owner_id,
                                          uint32_t          rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_IPv4_ACL_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv4_rule tpm_del_ipv4_rule_bounce
#else
 #define _tpm_del_ipv4_rule tpm_del_ipv4_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv4
*
* DESCRIPTION:
*           This function deletes a IPV4 HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv4  (const char *buf, size_t len)
{
    typedef enum
    {
        noruleaddipv4_owner=0,  noruleaddipv4_ruleidx, noruleaddipv4_max
    } noruleaddipv4_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noruleaddipv4_max)
    {
        parm_error_completion(numparms, noruleaddipv4_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d",
                            &ownerid, &rule_idx);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rule_idx[%d], parserulebm[0x%x], keyname[%s]\n",
//               len, parsedargs, ownerid, srcport_str, rule_idx, parserulebm, keyname);


        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_del_ipv4_rule(ownerid,
                                         rule_idx)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_del_ipv4_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv4_mc_stream_bounce(uint32_t     owner_id,
                                              uint32_t      stream_num)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd         = MV_TPM_IOCTL_DEL_IPv4_MC_STREAM;
    tpm_mc_rule->owner_id       = owner_id;
    tpm_mc_rule->stream_num     = stream_num;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv4_mc_stream tpm_del_ipv4_mc_stream_bounce
#else
 #define _tpm_del_ipv4_mc_stream tpm_del_ipv4_mc_stream
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_mc_stream_add_ipv4
*
* DESCRIPTION:
*           This function deletes a DSCP IPV6 HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_mc_stream_add_ipv4  (const char *buf, size_t len)
{
    typedef enum
    {
        nostreamaddipv4_owner=0, nostreamaddipv4_ruleidx,  nostreamaddipv4_max
    } nostreamaddipv4_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 stream;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != nostreamaddipv4_max)
    {
        parm_error_completion(numparms, nostreamaddipv4_max, buf, sfs_help_no_mc_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &stream);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream[%d]\n", len, parsedargs, ownerid, stream);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if ((rc = _tpm_del_ipv4_mc_stream(ownerid,
                                               stream)) == TPM_RC_OK)
        {
            printk(KERN_INFO "OK\n");
        }
        else
        {
            printk(KERN_INFO "%s: tpm_del_ipv4_mc_stream failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_mc_stream_bounce(uint32_t     owner_id,
                                              uint32_t      stream_num)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd         = MV_TPM_IOCTL_DEL_IPv6_MC_STREAM;
    tpm_mc_rule->owner_id       = owner_id;
    tpm_mc_rule->stream_num     = stream_num;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_mc_stream tpm_del_ipv6_mc_stream_bounce
#else
 #define _tpm_del_ipv6_mc_stream tpm_del_ipv6_mc_stream
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_mc_stream_add_ipv6
*
* DESCRIPTION:
*           This function deletes a DSCP IPV6 HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_mc_stream_add_ipv6  (const char *buf, size_t len)
{
    typedef enum
    {
        nostreamaddipv6_owner=0, nostreamaddipv6_ruleidx,  nostreamaddipv6_max
    } nostreamaddipv6_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 stream;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != nostreamaddipv6_max)
    {
        parm_error_completion(numparms, nostreamaddipv6_max, buf, sfs_help_no_mc_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &stream);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream[%d]\n", len, parsedargs, ownerid, stream);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if ((rc = _tpm_del_ipv6_mc_stream(ownerid,
                                               stream)) == TPM_RC_OK)
        {
            printk(KERN_INFO "OK\n");
        }
        else
        {
            printk(KERN_INFO "%s: tpm_del_ipv6_mc_stream failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_proc_set_enable_mtu_bounce(tpm_init_mtu_setting_enable_t enable)
{
	tpm_ioctl_mtu_t *tpm_mtu_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_mtu_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MTU_SECTION;
	tpm_mtu_set->mtu_setting_cmd = MV_TPM_IOCTL_SET_MTU_ADMIN;
	tpm_mtu_set->enable = enable;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_proc_set_enable_mtu tpm_proc_set_enable_mtu_bounce
#else
 #define _tpm_proc_set_enable_mtu tpm_db_set_mtu_enable
#endif

void sfs_tpm_cfg_set_mtu_enable (const char *buf, size_t len)
{
	typedef enum
	{
		mtu_enable=0, mtu_enable_max
	} mtu_enable_indx_t;
	/*shell line parsing*/
	int enable;

	int parsedargs;
	int numparms;

	/*used in API call*/
	tpm_error_code_t rc;

	numparms = count_parameters(buf);
	if (numparms != mtu_enable_max)
		parm_error_completion(numparms, mtu_enable_max, buf, sfs_help_setup);
	else {
		/*get parameters*/
		parsedargs = sscanf(buf, "%d", &enable);
		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else if ((enable != 1) && (enable != 0))
			printk(KERN_INFO "Invalid enable[%d]\n", enable);
		else {
			if ((rc = _tpm_proc_set_enable_mtu(enable)) == TPM_RC_OK)
				printk(KERN_INFO "OK\n");
			else
				printk(KERN_INFO "%s: tpm_proc_set_enable_mtu failed, rc[%d] - %s\n",
				__FUNCTION__, rc, get_tpm_err_str(rc));
		}
	}
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_proc_set_mtu_bounce(uint32_t ipv4_or_ipv6,
					uint32_t direction,
					uint32_t mtu)
{
	tpm_ioctl_mtu_t *tpm_mtu_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_mtu_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MTU_SECTION;
	tpm_mtu_set->mtu_setting_cmd = MV_TPM_IOCTL_SET_MTU;
	tpm_mtu_set->ethertype = ipv4_or_ipv6;
	tpm_mtu_set->direction = direction;
	tpm_mtu_set->mtu = mtu;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_proc_set_mtu tpm_proc_set_mtu_bounce
#else
 #define _tpm_proc_set_mtu tpm_proc_set_mtu
#endif

void sfs_tpm_cfg_set_mtu (const char *buf, size_t len)
{
    typedef enum
    {
        mtu_ipv4_or_ipv6=0,  mtu_direction, mtu_value,  mtu_max
    } mtu_indx_t;
    // shell line parsing
    uint32_t                 ipv4_or_ipv6;
    uint32_t                 direction;
    uint32_t                 mtu;

    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;
    tpm_setup_t              setup;

    memset(&setup, 0, sizeof(tpm_setup_t));

    numparms = count_parameters(buf);
    if (numparms != mtu_max)
    {
        parm_error_completion(numparms, mtu_max, buf, sfs_help_setup);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d", &ipv4_or_ipv6, &direction, &mtu);
        printk(KERN_INFO "ipv4_or_ipv6=%d, direction=%d. mtu[%d],\n", ipv4_or_ipv6, direction, mtu);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (ipv4_or_ipv6 >= 2)
        {
            printk(KERN_INFO "Invalid ipv4_or_ipv6[%d]\n", ipv4_or_ipv6);
        }
        else if (direction >= 2)
        {
            printk(KERN_INFO "Invalid direction[%d]\n", direction);
        }
        else
        {
            if ((rc = _tpm_proc_set_mtu(ipv4_or_ipv6, direction, mtu)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_proc_set_mtu failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }

}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_proc_set_pppoe_mtu_bounce(uint32_t ipv4_or_ipv6,
					uint32_t direction,
					uint32_t pppoe_mtu)
{
	tpm_ioctl_mtu_t *tpm_mtu_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_mtu_set;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MTU_SECTION;
	tpm_mtu_set->mtu_setting_cmd = MV_TPM_IOCTL_SET_PPPOE_MTU;
	tpm_mtu_set->ethertype = ipv4_or_ipv6;
	tpm_mtu_set->direction = direction;
	tpm_mtu_set->pppoe_mtu= pppoe_mtu;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_proc_set_pppoe_mtu tpm_proc_set_pppoe_mtu_bounce
#else
 #define _tpm_proc_set_pppoe_mtu tpm_proc_set_pppoe_mtu
#endif


void sfs_tpm_cfg_set_pppoe_mtu (const char *buf, size_t len)
{
    typedef enum
    {
        pppoe_mtu_ipv4_or_ipv6=0,  pppoe_mtu_direction, pppoe_mtu_value,  pppoe_mtu_max
    } pppoe_mtu_indx_t;
    // shell line parsing
    uint32_t                 ipv4_or_ipv6;
    uint32_t                 direction;
    uint32_t                 pppoe_mtu;

    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;
    tpm_setup_t              setup;

    memset(&setup, 0, sizeof(tpm_setup_t));

    numparms = count_parameters(buf);
    if (numparms != pppoe_mtu_max)
    {
        parm_error_completion(numparms, pppoe_mtu_max, buf, sfs_help_setup);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d", &ipv4_or_ipv6, &direction, &pppoe_mtu);
        printk(KERN_INFO "ipv4_or_ipv6=%d, direction=%d. pppoe_mtu[%d],\n", ipv4_or_ipv6, direction, pppoe_mtu);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (ipv4_or_ipv6 >= 2)
        {
            printk(KERN_INFO "Invalid ipv4_or_ipv6[%d]\n", ipv4_or_ipv6);
        }
        else if (direction >= 2)
        {
            printk(KERN_INFO "Invalid direction[%d]\n", direction);
        }
        else
        {
            if ((rc = _tpm_proc_set_pppoe_mtu(ipv4_or_ipv6, direction, pppoe_mtu)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_proc_set_pppoe_mtu failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }

}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_gen_rule_bounce(uint32_t            owner_id,
                                                uint32_t            rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd                   = MV_TPM_IOCTL_DEL_IPV6_GEN_ACL_RULE;
    tpm_del_acl_rule->owner_id                      = owner_id;
    tpm_del_acl_rule->rule_idx                      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_gen_rule tpm_del_ipv6_gen_rule_bounce
#else
 #define _tpm_del_ipv6_gen_rule tpm_del_ipv6_gen_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_gen_acl
*
* DESCRIPTION:
*           This function deletes a IPV6 HW gen ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_gen_acl (const char *buf, size_t len)
{
    typedef enum
    {
        noipv6gen_owner=0, noipv6gen_ruleidx, noipv6gen_max
    } noipv6genruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noipv6gen_max)
    {
        parm_error_completion(numparms, noipv6gen_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
               len, parsedargs, ownerid, rule_idx);


        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_del_ipv6_gen_rule(ownerid,
                                               rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
            {
                printk(KERN_INFO "%s: tpm_del_ipv6_gen_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_gen_rule_bounce(uint32_t                    owner_id,
                                                tpm_src_port_type_t         src_port,
                                                uint32_t                    rule_num,
                                                uint32_t                   *rule_idx,
                                                tpm_parse_fields_t          parse_rule_bm,
                                                tpm_parse_flags_t           parse_flags_bm,
                                                tpm_ipv6_gen_acl_key_t   *ipv6_gen_key,
                                                tpm_pkt_frwd_t             *pkt_frwd,
                                                tpm_pkt_mod_t              *pkt_mod,
                                                tpm_pkt_mod_bm_t            pkt_mod_bm,
                                                tpm_rule_action_t          *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                             = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                           = MV_TPM_IOCTL_ADD_IPV6_GEN_ACL_RULE;
    tpm_add_acl_rule->owner_id                              = owner_id;
    tpm_add_acl_rule->src_port                              = src_port;
    tpm_add_acl_rule->rule_num                              = rule_num;
    tpm_add_acl_rule->parse_rule_bm                         = parse_rule_bm;
    tpm_add_acl_rule->ipv6_gen_acl_rule.parse_flags_bm    = parse_flags_bm;
    tpm_add_acl_rule->ipv6_gen_acl_rule.pkt_mod_bm        = pkt_mod_bm;

    memcpy(&(tpm_add_acl_rule->ipv6_gen_acl_rule.ipv6_gen_key), ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));
    memcpy(&(tpm_add_acl_rule->ipv6_gen_acl_rule.pkt_frwd),       pkt_frwd,       sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ipv6_gen_acl_rule.pkt_mod),        pkt_mod,        sizeof(tpm_pkt_mod_t));
    memcpy(&(tpm_add_acl_rule->ipv6_gen_acl_rule.rule_action),    rule_action,    sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_gen_rule tpm_add_ipv6_gen_rule_bounce
#else
 #define _tpm_add_ipv6_gen_rule tpm_add_ipv6_gen_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_gen_rule_add
*
* DESCRIPTION:
*           This function create IPv6 general rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_gen_rule_add  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6ruleadd_owner=0, ipv6ruleadd_srcport,   ipv6ruleadd_rulenum,  ipv6ruleadd_parserulebm,      ipv6ruleadd_parseflagsbm,
        ipv6ruleadd_action,  ipv6ruleadd_nextphase, ipv6ruleadd_modbm,    ipv6ruleadd_ipv6genkeyname, ipv6ruleadd_frwdname,
        ipv6ruleadd_modname, ipv6ruleadd_max
    } ipv4ruleadd_parm_indx_t;

    uint32_t            owner_id;
    tpm_src_port_type_t src_port;
    uint32_t            rule_num;
    uint32_t            rule_idx;
    tpm_parse_fields_t  parse_rule_bm;
    tpm_parse_flags_t   parse_flags_bm;
    tpm_ipv6_gen_acl_key_t ipv6_gen_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_pkt_mod_t       pkt_mod;
    tpm_pkt_mod_bm_t    pkt_mod_bm;
    tpm_rule_action_t   rule_action;

    tpmcfg_ipv6_gen_key_entry_t *pdbipv6genkeyentry = 0;
    tpmcfg_frwd_entry_t           *pdbfrwdentry         = 0;
    tpmcfg_mod_entry_t            *pdbmodentry          = 0;

    tpm_error_code_t    rc;
    int                 indx;
    int                 parsedargs;
    int                 numparms;
    uint32_t            action;
    uint32_t            nextphase;
    char                srcport_str[20];
    char                uc_srcport_str[20];
    char                nextphase_str[20];
    char                ipv6genkeyname[30];
    char                frwdname[20];
    char                modname[20];


    numparms = count_parameters(buf);
    if (numparms != ipv6ruleadd_max)
    {
        parm_error_completion(numparms, ipv6ruleadd_max, buf, sfs_help_ipv6_gen_rule_add);
    }
    else
    {
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x  0x%x %s 0x%x %s %s %s",
                            &owner_id, srcport_str, &rule_num, &parse_rule_bm, &parse_flags_bm,
                            &action, nextphase_str, &pkt_mod_bm, ipv6genkeyname, frwdname, modname);
                printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], modbm[0x%x], ipv6keyname[%s], frwdname[%s], modname[%s]\n",
                       len, parsedargs, owner_id, srcport_str, rule_num, parse_rule_bm, parse_flags_bm, action, nextphase_str, pkt_mod_bm, ipv6genkeyname, frwdname, modname);

        for (indx = 0; indx < strlen(srcport_str); indx++)
        {
            uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        }
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else if (((pdbipv6genkeyentry = find_tpm_ipv6_gen_key_entry_by_name(ipv6genkeyname)) == 0) && \
                    strcmp(ipv6genkeyname, ipv6_gen_key_empty_name) != 0)
        {
            printk(KERN_INFO "IPV4 key entry [%s] not found\n", ipv6genkeyname);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && \
                    strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && \
                    strcmp(modname, mod_empty_name) != 0)
        {
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbipv6genkeyentry != 0)
                memcpy(&ipv6_gen_key, &pdbipv6genkeyentry->ipv6gen_acl, sizeof(tpm_ipv6_gen_acl_key_t));
            else
                memset(&ipv6_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));

            if (pdbfrwdentry != 0)
                memcpy(&pkt_frwd, &pdbfrwdentry->frwd, sizeof(tpm_pkt_frwd_t));
            else
                memset(&pkt_frwd, 0, sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)
                memcpy(&pkt_mod, &pdbmodentry->mod, sizeof(tpm_pkt_mod_t));
            else
                memset(&pkt_mod, 0, sizeof(tpm_pkt_mod_t));

            if (TPM_RC_OK == (rc = _tpm_add_ipv6_gen_rule(owner_id,
                                                            src_port,
                                                            rule_num,
                                                            &rule_idx,
                                                            parse_rule_bm,
                                                            parse_flags_bm,
                                                            &ipv6_gen_key,
                                                            &pkt_frwd,
                                                            &pkt_mod,
                                                            pkt_mod_bm,
                                                            &rule_action)) )
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv6_gen_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_oam_epon_add_channel_bounce(uint32_t owner_id,
						uint32_t cpu_rx_queue,
						tpm_trg_port_type_t llid_num)
{
	tpm_ioctl_mng_ch_t *tpm_oam_epon_chan = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mng_channel;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MNG_SECTION;
	tpm_oam_epon_chan->mng_cmd = MV_TPM_IOCTL_ADD_OAM_CHNL;
	tpm_oam_epon_chan->tpm_ioctl_oam_ch.owner_id = owner_id;
	tpm_oam_epon_chan->tpm_ioctl_oam_ch.cpu_rx_queue = cpu_rx_queue;
	tpm_oam_epon_chan->tpm_ioctl_oam_ch.llid_num = llid_num;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_oam_epon_add_channel tpm_oam_epon_add_channel_bounce
#else
 #define _tpm_oam_epon_add_channel tpm_oam_epon_add_channel
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_oam_channel
*
* DESCRIPTION:
*           This function creates OAM channel HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_oam_channel  (const char *buf, size_t len)
{
    typedef enum
    {
        oam_owner=0, oam_rxq,  oam_llid,  oam_max
    } oam_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 rx_q;
    uint32_t                 llid;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != oam_max)
    {
        parm_error_completion(numparms, oam_max, buf, sfs_help_oam_omci_channel);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d 0x%x", &ownerid, &rx_q, &llid);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rxq[%d], llid[0x%x]\n", len, parsedargs, ownerid, rx_q, llid);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if ((rc = _tpm_oam_epon_add_channel (ownerid,
                                                 rx_q,
                                                 llid)) == TPM_RC_OK)
        {
            printk(KERN_INFO "OK\n");
        }
        else
        {
            printk(KERN_INFO "%s: tpm_oam_epon_add_channel failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_omci_add_channel_bounce(uint32_t owner_id,
						tpm_gem_port_key_t gem_port,
						uint32_t cpu_rx_queue,
						tpm_trg_port_type_t tcont_num,
						uint32_t cpu_tx_queue)
{
	tpm_ioctl_mng_ch_t *tpm_omci_gpon_chan = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mng_channel;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MNG_SECTION;
	tpm_omci_gpon_chan->mng_cmd = MV_TPM_IOCTL_ADD_OMCI_CHNL;
	tpm_omci_gpon_chan->tpm_ioctl_omci_ch.owner_id = owner_id;
	tpm_omci_gpon_chan->tpm_ioctl_omci_ch.gem_port = gem_port;
	tpm_omci_gpon_chan->tpm_ioctl_omci_ch.cpu_rx_queue = cpu_rx_queue;
	tpm_omci_gpon_chan->tpm_ioctl_omci_ch.tcont_num = tcont_num;
	tpm_omci_gpon_chan->tpm_ioctl_omci_ch.cpu_tx_queue = cpu_tx_queue;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_omci_add_channel tpm_omci_add_channel_bounce
#else
 #define _tpm_omci_add_channel tpm_omci_add_channel
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_omci_channel
*
* DESCRIPTION:
*           This function creates creates an OMCI channel HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_omci_channel  (const char *buf, size_t len)
{
    typedef enum
    {
        omci_owner=0, omci_gemport,  omci_rxq,  omci_txq,  omci_tcont,  omci_max
    } omci_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 gemport;
    uint32_t                 rx_q;
    uint32_t                 tx_q;
    uint32_t                 tcont;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != omci_max)
    {
        parm_error_completion(numparms, omci_max, buf, sfs_help_oam_omci_channel);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d %d 0x%x", &ownerid, &gemport, &rx_q, &tx_q, &tcont);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], gemport[%d], rx_q[%d], tx_q[%d], tcont[0x%x]\n",
//               len, parsedargs, ownerid, gemport, rx_q, tx_q, tcont);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (gemport >= 255)
        {
            printk(KERN_INFO "invalid gemport[%d]\n", gemport);
        }
        else
        {
            if ((rc = _tpm_omci_add_channel (ownerid,
                                            (uint16_t)gemport,
                                            rx_q,
                                            tcont,
                                            tx_q)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_omci_add_channel failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_oam_epon_del_channel_bounce(uint32_t owner_id)
{
	tpm_ioctl_mng_ch_t *tpm_oam_epon_chan = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mng_channel;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MNG_SECTION;
	tpm_oam_epon_chan->mng_cmd = MV_TPM_IOCTL_DEL_OAM_CHNL;
	tpm_oam_epon_chan->tpm_ioctl_oam_ch.owner_id = owner_id;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_oam_epon_del_channel tpm_oam_epon_del_channel_bounce
#else
 #define _tpm_oam_epon_del_channel tpm_oam_epon_del_channel
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_no_oam_channel
*
* DESCRIPTION:
*           This function deletes OAM channel HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_oam_channel  (const char *buf, size_t len)
{
    typedef enum
    {
        nooam_owner=0, nooam_max
    } oam_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);

    if (numparms == 1 && buf[0] == '?')
    {
#ifdef CONFIG_MV_TPM_SYSFS_HELP
        char helpbuf[1000];
        int  helpbuf_len;

        helpbuf_len = sfs_help_no_oam_omci_channel(helpbuf);
        print_help_buffer(helpbuf, helpbuf_len);
#else
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", nooam_max, numparms);
#endif
    }
    else if (numparms != 1)
    {
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", nooam_max, numparms);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d", &ownerid);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d]\n", len, parsedargs, ownerid);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if ((rc = _tpm_oam_epon_del_channel (ownerid)) == TPM_RC_OK)
        {
            printk(KERN_INFO "OK\n");
        }
        else
        {
            printk(KERN_INFO "%s: tpm_oam_epon_del_channel failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_omci_del_channel_bounce(uint32_t owner_id)
{
	tpm_ioctl_mng_ch_t *tpm_omci_gpon_chan = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mng_channel;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MNG_SECTION;
	tpm_omci_gpon_chan->mng_cmd = MV_TPM_IOCTL_DEL_OMCI_CHNL;
	tpm_omci_gpon_chan->tpm_ioctl_omci_ch.owner_id = owner_id;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}
 #define _tpm_omci_del_channel tpm_omci_del_channel_bounce
#else
 #define _tpm_omci_del_channel tpm_omci_del_channel
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_no_omci_channel
*
* DESCRIPTION:
*           This function creates creates an OMCI channel HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_omci_channel  (const char *buf, size_t len)
{
    typedef enum
    {
        noomci_owner=0, noomci_max
    } noomci__parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);

    if (numparms == 1 && buf[0] == '?')
    {
#ifdef CONFIG_MV_TPM_SYSFS_HELP
        char helpbuf[1000];
        int  helpbuf_len;

        helpbuf_len = sfs_help_no_oam_omci_channel(helpbuf);
        print_help_buffer(helpbuf, helpbuf_len);
#else
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", noomci_max, numparms);
#endif
    }
    else if (numparms != 1)
    {
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", noomci_max, numparms);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d ", &ownerid);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d]\n", len, parsedargs, ownerid);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if ((rc = _tpm_omci_del_channel (ownerid)) == TPM_RC_OK)
        {
            printk(KERN_INFO "OK\n");
        }
        else
        {
            printk(KERN_INFO "%s: tpm_omci_del_channel failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_setup
*
* DESCRIPTION:
*           This function initializes tpm module
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_setup  (const char *buf, size_t len)
{
    typedef enum
    {
        /*setup_ponype=0,  */setup_filename=0, setup_max
    } setup_parm_indx_t;
    // shell line parsing
    /*uint32_t                 pon_type;*/
    char                     filename[XML_FILE_PATH_LENGTH];
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;
    tpm_setup_t              setup;

    memset(&setup, 0, sizeof(tpm_setup_t));

    numparms = count_parameters(buf);
    if (numparms != setup_max)
    {
        parm_error_completion(numparms, setup_max, buf, sfs_help_setup);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s", filename);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], level[%d]\n", len, parsedargs, ownerid, level);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        /*else if (pon_type >= TPM_NONE)
        {
            printk(KERN_INFO "Invalid pon_type[%d]\n", pon_type);
        }*/
        else
        {
            /*setup.pon_type = pon_type;*/
            if (strcmp(filename, "default"))
            {
                strcpy(setup.xml_file_path, filename);
            }
            if ((rc = module_tpmSetup(&setup)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: module_tpmSetup failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_mib_reset_bounce(uint32_t     owner_id,
                                      uint32_t     level)
{
    tpm_ioctl_mib_reset_t *tpm_mib_reset_param = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mib_reset_param;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_RESET_SECTION;
    tpm_mib_reset_param->owner_id       = owner_id;
    tpm_mib_reset_param->reset_level    = level;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_mib_reset tpm_mib_reset_bounce
#else
 #define _tpm_mib_reset tpm_mib_reset
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_mib_reset
*
* DESCRIPTION:
*           This function creates does a MIB reset
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mib_reset  (const char *buf, size_t len)
{
    typedef enum
    {
        mibreset_owner=0, mibreset_gemport,  mibreset_max
    } mibreset_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 level;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != mibreset_max)
    {
        parm_error_completion(numparms, mibreset_max, buf, sfs_help_mib_reset);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &level);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], level[%d]\n", len, parsedargs, ownerid, level);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (level > TPM_ENUM_MAX_RESET_LEVEL)
        {
            printk(KERN_INFO "Invalid level[%d]\n", level);
        }
        else
        {
            if ((rc = _tpm_mib_reset(ownerid,
                                     level)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_mib_reset failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_active_wan_bounce(uint32_t     owner_id,
                                           tpm_gmacs_enum_t active_wan)
{
    tpm_ioctl_set_active_wan_t *tpm_set_active_wan_param = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_set_active_wan_param;

    tpm_sfs_2_ioctl_command.cmd              = MV_TPM_IOCTL_SET_ACTIVE_WAN_SECTION;
    tpm_set_active_wan_param->owner_id       = owner_id;
    tpm_set_active_wan_param->active_wan     = active_wan;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_set_active_wan tpm_set_active_wan_bounce
#else
 #define _tpm_set_active_wan tpm_set_active_wan
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_set_active_wan
*
* DESCRIPTION:
*           This function sets active wan port
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_active_wan  (const char *buf, size_t len)
{
    typedef enum
    {
	activewan_owner=0, activewan_active_wan,  activewan_max
    } activewan_parm_indx_t;
    // shell line parsing
    uint32_t		     ownerid;
    int 		     parsedargs;
    int 		     numparms;
    //Used in API call
    tpm_error_code_t	     rc;
    tpm_gmacs_enum_t	     active_wan;

    numparms = count_parameters(buf);
    if (numparms != activewan_max)
    {
	parm_error_completion(numparms, activewan_max, buf, sfs_help_set_active_wan);
    }
    else
    {
	// Get parameters
	parsedargs = sscanf(buf, "%d %d", &ownerid, &active_wan);
//	  printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], active_wan_int[%d]\n",
//			len, parsedargs, ownerid, active_wan);

	if (parsedargs != numparms)
	{
	    printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
	}
	else if (active_wan >= TPM_MAX_NUM_GMACS)
	{
	    printk(KERN_INFO "Invalid active_wan[%d]\n", active_wan);
	}
	else
	{
	    if ((rc = _tpm_set_active_wan(ownerid,
					  active_wan)) == TPM_RC_OK)
	    {
		printk(KERN_INFO "OK\n");
	    }
	    else
	    {
		printk(KERN_INFO "%s: tpm_set_active_wan failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
	    }
	}
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_hot_swap_profile_bounce(uint32_t     owner_id,
                                           tpm_eth_complex_profile_t profile_id)
{
    tpm_ioctl_hot_swap_profile_t *tpm_hot_swap_profile_param = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_hot_swap_profile_param;

    tpm_sfs_2_ioctl_command.cmd              = MV_TPM_IOCTL_HOT_SWAP_PROFILE_SECTION;
    tpm_hot_swap_profile_param->owner_id       = owner_id;
    tpm_hot_swap_profile_param->profile_id     = profile_id;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_hot_swap_profile tpm_hot_swap_profile_bounce
#else
 #define _tpm_hot_swap_profile tpm_hot_swap_profile
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_hot_swap_profile
*
* DESCRIPTION:
*           This function swap profile in run time
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_hot_swap_profile  (const char *buf, size_t len)
{
    typedef enum
    {
	hotswap_owner=0, hotswap_profile_id,  hotswap_max
    } hotswap_parm_indx_t;
    // shell line parsing
    uint32_t		     ownerid;
    int 		     parsedargs;
    int 		     numparms;
    //Used in API call
    tpm_error_code_t	     rc;
    uint32_t	     profile_id;

    numparms = count_parameters(buf);
    if (numparms != hotswap_max)
    {
	parm_error_completion(numparms, hotswap_max, buf, sfs_help_hot_swap_profile);
    }
    else
    {
	// Get parameters
	parsedargs = sscanf(buf, "%d %d", &ownerid, &profile_id);
//	  printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], profile_id_int[%d]\n",
//			len, parsedargs, ownerid, profile_id);

	if (parsedargs != numparms)
	{
	    printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
	}
	else
	{
	    if ((rc = _tpm_hot_swap_profile(ownerid,
					  profile_id)) == TPM_RC_OK)
	    {
		printk(KERN_INFO "OK\n");
	    }
	    else
	    {
		printk(KERN_INFO "%s: tpm_hot_swap_profile failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
	    }
	}
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_port_hwf_admin_bounce(uint32_t	   owner_id,
							  tpm_gmacs_enum_t port,
							  uint8_t	   txp,
							  uint8_t	   enable)
{
    tpm_ioctl_set_port_hwf_admin_t *tpm_set_port_hwf_admin_param = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_set_port_hwf_admin_param;

    tpm_sfs_2_ioctl_command.cmd              = MV_TPM_IOCTL_SET_PORT_HWF_ADMIN_SECTION;
    tpm_set_port_hwf_admin_param->owner_id   = owner_id;
    tpm_set_port_hwf_admin_param->port       = port;
    tpm_set_port_hwf_admin_param->txp        = txp;
    tpm_set_port_hwf_admin_param->enable     = enable;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_set_port_hwf_admin tpm_set_port_hwf_admin_bounce
#else
 #define _tpm_set_port_hwf_admin tpm_proc_hwf_admin_set
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_set_port_hwf_admin
*
* DESCRIPTION:
*           This function set port hwf enable/disable
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_port_hwf_admin  (const char *buf, size_t len)
{
    typedef enum
    {
	setporthwfadmin_owner=0, setporthwfadmin_port, setporthwfadmin_txp,
	setporthwfadmin_enable, setporthwfadmin_max
    } setporthwfadmin_parm_indx_t;
    // shell line parsing
    int 		     parsedargs;
    int 		     numparms;
    //Used in API call
    tpm_error_code_t	   rc;
    uint32_t               ownerid;
    tpm_gmacs_enum_t       port;
    uint32_t               txp;
    uint32_t               enable;

    numparms = count_parameters(buf);
    if (numparms != setporthwfadmin_max)
    {
	parm_error_completion(numparms, setporthwfadmin_max, buf, sfs_help_set_port_hwf_admin);
    }
    else
    {
	// Get parameters
	parsedargs = sscanf(buf, "%d %d %d %d", &ownerid, &port, &txp, &enable);
//	  printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], port[%d], txp[%d], enable[%d]\n",
//			len, parsedargs, ownerid, port, txp, enable);

	if (parsedargs != numparms)
	{
	    printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
	}
	else
	{
	    if ((rc = _tpm_set_port_hwf_admin(port, txp, enable)) == TPM_RC_OK)
	    {
		printk(KERN_INFO "OK\n");
	    }
	    else
	    {
		printk(KERN_INFO "%s: tpm_set_port_hwf_admin failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
	    }
	}
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_erase_section
*
* DESCRIPTION:
*           This function erases a section per an application group.
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_erase_section(const char *buf, size_t len)
{
	typedef enum
	{
		section_owner_id=0, section_api_type, erase_section_max
	} erase_section_parm_indx_t;
	/*shell line parsing*/
	uint32_t                 ownerid;
	uint32_t                 api_type;
	int                      parsedargs;
	int                      numparms;
	tpm_error_code_t         rc;

	numparms = count_parameters(buf);
	if (numparms != erase_section_max)
		parm_error_completion(numparms, erase_section_max, buf, sfs_help_erase_section);
	else {
		/*get parameters*/
		parsedargs = sscanf(buf, "%d %d", &ownerid, &api_type);
		if(parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else if (api_type > TPM_MAX_API_TYPES)
			printk(KERN_INFO "Invalid api type[%d]\n", api_type);
		else {
			if ((rc = tpm_erase_section(ownerid, api_type)) == TPM_RC_OK)
				printk(KERN_INFO "OK\n");
			else
				printk(KERN_INFO "%s: tpm_erase_section failed, rc[%d] - %s\n",
				__FUNCTION__, rc, get_tpm_err_str(rc));
		}
	}
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_cpu_wan_loopback_bounce(uint32_t         owner_id,
                                                 tpm_pkt_frwd_t  *pkt_frwd,
                                                 uint32_t        *mod_idx)
{
    tpm_ioctl_cpu_lpbk_t *tpm_ioctl_cpu_lpbk = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_cpu_lpbk;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_CPU_LPBK_SECTION;
    tpm_ioctl_cpu_lpbk->cpu_lpbk_cmd    = MV_TPM_IOCTL_ADD_CPU_WAN_LPBK;
    tpm_ioctl_cpu_lpbk->owner_id        = owner_id;
    memcpy(&(tpm_ioctl_cpu_lpbk->pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_cpu_wan_loopback tpm_add_cpu_wan_loopback_bounce
#else
 #define _tpm_add_cpu_wan_loopback tpm_add_cpu_wan_loopback
#endif


/*******************************************************************************
* sfs_tpm_cfg_add_cpu_lpbk
*
* DESCRIPTION:
*           This function adds CPU WAN loopback
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_add_cpu_lpbk  (const char *buf, size_t len)
{
    typedef enum
    {
        lpbk_owner=0, lpbk_port, lpbk_que, lpbk_gem, lpbk_max
    } lpbk_parm_indx_t;
    uint32_t            ownerid;
    uint32_t            temp_port;
    uint32_t            temp_que;
    uint32_t            temp_gem;
    int                 parsedargs;
    int                 numparms;
    tpm_error_code_t    rc;
    tpm_pkt_frwd_t      frwd_entry;
    uint32_t            mod_idx;

    numparms = count_parameters(buf);
    if (numparms != lpbk_max)
    {
        parm_error_completion(numparms, lpbk_max, buf, sfs_help_cfg_cpu_lpbk);
    }
    else
    {
        /* Get parameters */
        parsedargs = sscanf(buf, "%d 0x%x %d %d", &ownerid, &temp_port, &temp_que, &temp_gem);
/*
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%s] temp_port[0x%x],temp_que[%d],temp_gem[%d]\n",
               len, parsedargs, ownerid, temp_port, temp_que, temp_gem);
*/
        if (temp_que > 7)
        {
            printk(KERN_INFO "Invalid queue [%d]\n", temp_que);
        }
        else if (temp_gem > 4095)
        {
            printk(KERN_INFO "Invalid gem port [%d]\n", temp_gem);
        }
        else
        {
            frwd_entry.trg_port  = (tpm_trg_port_type_t)temp_port;
            frwd_entry.trg_queue = (uint8_t)temp_que;
            frwd_entry.gem_port  = (uint16_t)temp_gem;

            rc = _tpm_add_cpu_wan_loopback(TPM_MOD_OWNER_TPM, &frwd_entry, &mod_idx);
            if(TPM_RC_OK != rc)
            {
                printk(KERN_ERR "Failed to call tpm_add_cpu_wan_loopback, ret_code[%d]\n", rc);
            }
            else
            {
                printk(KERN_ERR "Call tpm_add_cpu_wan_loopback successfully, mod_idx[%d]\n", mod_idx);
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_cpu_wan_loopback_bounce(uint32_t         owner_id,
                                                 tpm_pkt_frwd_t  *pkt_frwd)
{
    tpm_ioctl_cpu_lpbk_t *tpm_ioctl_cpu_lpbk = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_cpu_lpbk;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_CPU_LPBK_SECTION;
    tpm_ioctl_cpu_lpbk->cpu_lpbk_cmd    = MV_TPM_IOCTL_DEL_CPU_WAN_LPBK;
    tpm_ioctl_cpu_lpbk->owner_id        = owner_id;
    memcpy(&(tpm_ioctl_cpu_lpbk->pkt_frwd), pkt_frwd, sizeof(tpm_pkt_frwd_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_cpu_wan_loopback tpm_del_cpu_wan_loopback_bounce
#else
 #define _tpm_del_cpu_wan_loopback tpm_del_cpu_wan_loopback
#endif


/*******************************************************************************
* sfs_tpm_cfg_del_cpu_lpbk
*
* DESCRIPTION:
*           This function delete CPU WAN loopback
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_del_cpu_lpbk  (const char *buf, size_t len)
{
    typedef enum
    {
        lpbk_owner=0, lpbk_port, lpbk_que, lpbk_gem, lpbk_max
    } lpbk_parm_indx_t;
    uint32_t            ownerid;
    uint32_t            temp_port;
    uint32_t            temp_que;
    uint32_t            temp_gem;
    int                 parsedargs;
    int                 numparms;
    tpm_error_code_t    rc;
    tpm_pkt_frwd_t      frwd_entry;

    numparms = count_parameters(buf);
    if (numparms != lpbk_max)
    {
        parm_error_completion(numparms, lpbk_max, buf, sfs_help_cfg_cpu_lpbk);
    }
    else
    {
        /* Get parameters */
        parsedargs = sscanf(buf, "%d 0x%x %d %d", &ownerid, &temp_port, &temp_que, &temp_gem);
/*
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%s] temp_port[0x%x],temp_que[%d],temp_gem[%d]\n",
               len, parsedargs, ownerid, temp_port, temp_que, temp_gem);
*/
        if (temp_que > 7)
        {
            printk(KERN_INFO "Invalid queue [%d]\n", temp_que);
        }
        else if (temp_gem > 4095)
        {
            printk(KERN_INFO "Invalid gem port [%d]\n", temp_gem);
        }
        else
        {
            frwd_entry.trg_port  = (tpm_trg_port_type_t)temp_port;
            frwd_entry.trg_queue = (uint8_t)temp_que;
            frwd_entry.gem_port  = (uint16_t)temp_gem;

            rc = _tpm_del_cpu_wan_loopback(TPM_MOD_OWNER_TPM, &frwd_entry);
            if(TPM_RC_OK != rc)
            {
                printk(KERN_ERR "Failed to call tpm_del_cpu_wan_loopback, ret_code[%d]\n", rc);
            }
            else
            {
                printk(KERN_ERR "Call tpm_del_cpu_wan_loopback successfully\n");
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_dump_cpu_lpbk
*
* DESCRIPTION:
*           This function dumps all valid CPU WAN loopback entries from DB
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_dump_cpu_lpbk  (const char *buf, size_t len)
{
    typedef enum
    {
        lpbk_owner=0, lpbk_max
    } lpbk_parm_indx_t;
    uint32_t            ownerid;
    int                 parsedargs;
    int                 numparms;

    numparms = count_parameters(buf);
    if (numparms != lpbk_max)
    {
        parm_error_completion(numparms, lpbk_max, buf, sfs_help_cfg_cpu_lpbk);
    }
    else
    {
        /* Get parameters */
        parsedargs = sscanf(buf, "%d", &ownerid);
        /*
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%s]\n",
               len, parsedargs, ownerid);
        */

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            tpm_print_cpu_lpbk_entry();
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_get_lu_entry
*
* DESCRIPTION:
*           This function dumps least used PnC entries
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_get_lu_entry (const char *buf, size_t len)
{
    typedef enum
    {
        age_owner_id=0, age_api_group, age_lu_num, age_lu_reset, age_max
    } age_parm_indx_t;
    uint32_t            owner_id;
    uint32_t            api_group;
    uint32_t            lu_num;
    uint32_t            lu_reset;
    int                 parsed_args_num;
    int                 param_num;

    param_num = count_parameters(buf);
    if (param_num != age_max)
    {
        parm_error_completion(param_num, age_max, buf, sfs_help_cfg_age_count);
    }
    else
    {
        /* Get parameters */
        parsed_args_num = sscanf(buf, "%d %d %d %d", &owner_id, &api_group, &lu_num, &lu_reset);
        /*
        printk(KERN_INFO "len=%d, parsed_args_num[%d], ownerid[%d], api_group[%d], lu_num[%d], lu_reset[%d]\n",
               len, parsed_args_num, owner_id, api_group, lu_num, lu_reset);
        */

        if (parsed_args_num != param_num)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsed_args_num, param_num);
        }
        else
        {
            if((lu_num <= 0) || ( lu_num > TPM_MAX_LU_ENTRY_NUM))
            {
                printk(KERN_INFO "lu_num[%d] is illegal, allowed lu_num[1~%d]\n", lu_num, TPM_MAX_LU_ENTRY_NUM);
                return;
            }

            tpm_print_tcam_lu_entry(owner_id, api_group, lu_num, lu_reset);
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_get_pnc_all_hit_counters_bounce(uint32_t               owner_id,
                                                    tpm_api_type_t          api_type,
                                                    uint32_t                high_thresh_pkts,
                                                    uint8_t                 counters_reset,
                                                    uint16_t               *valid_counters,
                                                    tpm_api_entry_count_t  *count_array)
{
    tpm_ioctl_pnc_hit_cnt_t *tpm_ioctl_pnc_hit_cnt = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_pnc_hit_cnt;

    tpm_sfs_2_ioctl_command.cmd             = MV_TPM_IOCTL_AGE_COUNT_SECTION;
    tpm_ioctl_pnc_hit_cnt->age_count_cmd    = MV_TPM_IOCTL_GET_ALL_HIT_COUNTERS;
    tpm_ioctl_pnc_hit_cnt->owner_id         = owner_id;
    tpm_ioctl_pnc_hit_cnt->api_type         = api_type;
    tpm_ioctl_pnc_hit_cnt->high_thresh_pkts = high_thresh_pkts;
    tpm_ioctl_pnc_hit_cnt->counter_reset    = counters_reset;
    tpm_ioctl_pnc_hit_cnt->valid_counters   = *valid_counters;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_get_pnc_all_hit_counters tpm_get_pnc_all_hit_counters_bounce
 #define _tpm_print_pnc_all_hit_counters(a,b,c,d,e,f)
#else
 #define _tpm_get_pnc_all_hit_counters tpm_get_pnc_all_hit_counters
 #define _tpm_print_pnc_all_hit_counters(a,b,c,d,e,f) tpm_print_pnc_all_hit_counters(a,b,c,d,e,f)
#endif


/*******************************************************************************
* sfs_tpm_cfg_get_pnc_all_hit_counters
*
* DESCRIPTION:
*           This function prints all pnc hit counters per API type masking out
*           high counter values according to high_thresh_pkts
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_get_pnc_all_hit_counters (const char *buf, size_t len)
{
    typedef enum
    {
        age_owner_id=0, age_api_type, age_high_thresh_pkts, age_counters_reset, age_valid_counters, age_max
    } age_parm_indx_t;
    uint32_t                owner_id;
    uint32_t                api_type;
    uint32_t                high_thresh_pkts;
    uint32_t                counters_reset;
    uint32_t                valid_counters;
    int                     parsed_args_num;
    int                     param_num;
    tpm_error_code_t        ret_code;
	tpm_api_entry_count_t   count_array[CONFIG_MV_PNC_TCAM_LINES];

    param_num = count_parameters(buf);
    if (param_num != age_max)
    {
        parm_error_completion(param_num, age_max, buf, sfs_help_cfg_age_count);
    }
    else
    {
        /* Get parameters */
        parsed_args_num = sscanf(buf, "%d %d %d %d %d", &owner_id, &api_type, &high_thresh_pkts,
                                 &counters_reset, &valid_counters);

        if (parsed_args_num != param_num)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsed_args_num, param_num);
        }
        else
        {
            ret_code = _tpm_get_pnc_all_hit_counters(owner_id,
                                                     api_type,
                                                     high_thresh_pkts,
                                                     counters_reset,
                                                    (uint16_t*)&valid_counters,
                                                    &(count_array[0]));

            if (TPM_RC_OK == ret_code)
            {
                _tpm_print_pnc_all_hit_counters(owner_id,
                                                api_type, high_thresh_pkts,
                                                (uint8_t)counters_reset,
                                                valid_counters,
                                                &(count_array[0]));
            }
            else
            {
                printk(KERN_INFO "%s: _tpm_get_pnc_all_hit_counters failed, rc[%d] - %s\n",
                       __FUNCTION__, ret_code, get_tpm_err_str(ret_code));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_pnc_counter_mask_bounce(uint32_t           owner_id,
                                                 tpm_api_type_t     api_type,
                                                 uint32_t           rule_idx,
                                                 uint32_t           lu_rule_mask)
{
    tpm_ioctl_age_count_t *tpm_ioctl_age_count = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_age_count;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_AGE_COUNT_SECTION;
    tpm_ioctl_age_count->age_count_cmd  = MV_TPM_IOCTL_SET_LU_COUNT_MASK;
    tpm_ioctl_age_count->owner_id       = owner_id;
    tpm_ioctl_age_count->api_type       = api_type;
    tpm_ioctl_age_count->rule_idx       = rule_idx;
    tpm_ioctl_age_count->lu_rule_mask   = lu_rule_mask;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_set_pnc_counter_mask tpm_set_pnc_counter_mask_bounce
#else
 #define _tpm_set_pnc_counter_mask tpm_set_pnc_counter_mask
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_count_mask
*
* DESCRIPTION:
*           This function sets count mask
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_count_mask (const char *buf, size_t len)
{
    typedef enum
    {
        age_owner_id=0, age_api_group, age_rule_idx, age_lu_rule_mask, age_max
    } age_parm_indx_t;
    uint32_t            owner_id;
    uint32_t            api_group;
    uint32_t            rule_idx;
    uint32_t            lu_rule_mask;
    int                 parsed_args_num;
    int                 param_num;
    tpm_error_code_t    ret_code;

    param_num = count_parameters(buf);
    if (param_num != age_max)
    {
        parm_error_completion(param_num, age_max, buf, sfs_help_cfg_age_count);
    }
    else
    {
        /* Get parameters */
        parsed_args_num = sscanf(buf, "%d %d %d %d", &owner_id, &api_group, &rule_idx, &lu_rule_mask);
        /*
        printk(KERN_INFO "len=%d, parsed_args_num[%d], ownerid[%d], api_group[%d], rule_idx[%d], lu_rule_mask[%d]\n",
               len, parsed_args_num, owner_id, api_group, rule_idx, lu_rule_mask);
        */

        if (parsed_args_num != param_num)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsed_args_num, param_num);
        }
        else
        {
            if ((ret_code = _tpm_set_pnc_counter_mask(owner_id,
                                                      api_group,
                                                      rule_idx,
                                                      lu_rule_mask)) == TPM_RC_OK)
                PR_RESULT
            else
                printk(KERN_INFO "%s: tpm_set_pnc_counter_mask failed, rc[%d] - %s\n", __FUNCTION__,
                       ret_code, get_tpm_err_str(ret_code));
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_get_pnc_hit_count_bounce(uint32_t             owner_id,
                                              tpm_api_type_t       api_type,
                                              uint32_t             rule_idx,
                                              uint8_t              hit_reset,
                                              uint32_t            *hit_count)
{
    tpm_ioctl_age_count_t *tpm_ioctl_age_count = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_age_count;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_AGE_COUNT_SECTION;
    tpm_ioctl_age_count->age_count_cmd  = MV_TPM_IOCTL_GET_HIT_COUNT;
    tpm_ioctl_age_count->owner_id       = owner_id;
    tpm_ioctl_age_count->api_type       = api_type;
    tpm_ioctl_age_count->rule_idx       = rule_idx;
    tpm_ioctl_age_count->hit_reset      = hit_reset;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_get_pnc_hit_count tpm_get_pnc_hit_count_bounce
#else
 #define _tpm_get_pnc_hit_count tpm_get_pnc_hit_count
#endif


/*******************************************************************************
* sfs_tpm_cfg_get_hit_count
*
* DESCRIPTION:
*           This function sets count mask
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_get_hit_count (const char *buf, size_t len)
{
    typedef enum
    {
        age_owner_id=0, age_api_group, age_rule_idx, age_hit_reset, age_max
    } age_parm_indx_t;
    uint32_t            owner_id;
    uint32_t            api_group;
    uint32_t            rule_idx;
    int                 parsed_args_num;
    int                 param_num;
    uint32_t            hit_count = 0;
    uint32_t            hit_reset;
    tpm_error_code_t    ret_code;

    param_num = count_parameters(buf);
    if (param_num != age_max)
    {
        parm_error_completion(param_num, age_max, buf, sfs_help_cfg_age_count);
    }
    else
    {
        /* Get parameters */
        parsed_args_num = sscanf(buf, "%d %d %d %d", &owner_id, &api_group, &rule_idx, &hit_reset);
        /*
        printk(KERN_INFO "len=%d, parsed_args_num[%d], ownerid[%d], range_id[%d], rule_idx[%d]\n",
               len, parsed_args_num, owner_id, range_id, rule_idx);
        */
        if (parsed_args_num != param_num)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsed_args_num, param_num);
        }
        else
        {
            if ((ret_code = _tpm_get_pnc_hit_count(owner_id,
                                                   api_group,
                                                   rule_idx,
                                                   (uint8_t)hit_reset,
                                                   &hit_count)) == TPM_RC_OK)
                PR_HIT_COUNT(hit_count)
            else
                printk(KERN_INFO "%s: tpm_get_pnc_hit_count failed, rc[%d] - %s\n", __FUNCTION__,
                       ret_code, get_tpm_err_str(ret_code));
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_set_pnc_lu_threshold_bounce(uint32_t             owner_id,
                                                 tpm_api_type_t       api_type,
                                                 uint32_t             lu_thresh_pkts)
{
    tpm_ioctl_age_count_t *tpm_ioctl_age_count = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_age_count;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_AGE_COUNT_SECTION;
    tpm_ioctl_age_count->age_count_cmd  = MV_TPM_IOCTL_SET_LU_THESHOLD;
    tpm_ioctl_age_count->owner_id       = owner_id;
    tpm_ioctl_age_count->api_type       = api_type;
    tpm_ioctl_age_count->lu_thresh_pkts = lu_thresh_pkts;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_set_pnc_lu_threshold tpm_set_pnc_lu_threshold_bounce
#else
 #define _tpm_set_pnc_lu_threshold tpm_set_pnc_lu_threshold
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_lu_thresh
*
* DESCRIPTION:
*           This function sets count mask
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_lu_thresh (const char *buf, size_t len)
{
    typedef enum
    {
        age_owner_id=0, age_api_group, age_lu_thresh_pkts, age_max
    } age_parm_indx_t;
    uint32_t            owner_id;
    uint32_t            api_group;
    uint32_t            lu_thresh_pkts;
    int                 parsed_args_num;
    int                 param_num;
    tpm_error_code_t    ret_code;

    param_num = count_parameters(buf);
    if (param_num != age_max)
    {
        parm_error_completion(param_num, age_max, buf, sfs_help_cfg_age_count);
    }
    else
    {
        /* Get parameters */
        parsed_args_num = sscanf(buf, "%d %d %d", &owner_id, &api_group, &lu_thresh_pkts);
        /*
        printk(KERN_INFO "len=%d, parsed_args_num[%d], ownerid[%d], api_group[%d], rule_idx[%d]\n",
               len, parsed_args_num, owner_id, api_group, rule_idx);
        */

        if (parsed_args_num != param_num)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsed_args_num, param_num);
        }
        else
        {
            if ((ret_code = _tpm_set_pnc_lu_threshold(owner_id,
                                                     api_group,
                                                     lu_thresh_pkts)) == TPM_RC_OK)
                PR_RESULT
            else
                printk(KERN_INFO "%s: tpm_set_pnc_lu_threshold failed, rc[%d] - %s\n", __FUNCTION__,
                       ret_code, get_tpm_err_str(ret_code));
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_reset_pnc_age_group_bounce(uint32_t                owner_id,
                                                tpm_api_type_t          api_type)
{
    tpm_ioctl_age_count_t *tpm_ioctl_age_count = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_age_count;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_AGE_COUNT_SECTION;
    tpm_ioctl_age_count->age_count_cmd  = MV_TPM_IOCTL_RESET_COUNT_GROUP;
    tpm_ioctl_age_count->owner_id       = owner_id;
    tpm_ioctl_age_count->api_type       = api_type;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_reset_pnc_age_group tpm_reset_pnc_age_group_bounce
#else
 #define _tpm_reset_pnc_age_group tpm_reset_pnc_age_group
#endif


/*******************************************************************************
* sfs_tpm_cfg_reset_age_group
*
* DESCRIPTION:
*           This function sets count mask
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_reset_age_group (const char *buf, size_t len)
{
    typedef enum
    {
        age_owner_id=0, age_api_group,  age_max
    } age_parm_indx_t;
    uint32_t            owner_id;
    uint32_t            api_group;
    int                 parsed_args_num;
    int                 param_num;
    tpm_error_code_t    ret_code;

    param_num = count_parameters(buf);
    if (param_num != age_max)
    {
        parm_error_completion(param_num, age_max, buf, sfs_help_cfg_age_count);
    }
    else
    {
        /* Get parameters */
        parsed_args_num = sscanf(buf, "%d %d", &owner_id, &api_group);
        /*
        printk(KERN_INFO "len=%d, parsed_args_num[%d], ownerid[%d], api_group[%d]\n",
               len, parsed_args_num, owner_id, api_group);
        */

        if (parsed_args_num != param_num)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsed_args_num, param_num);
        }
        else
        {
            if ((ret_code = _tpm_reset_pnc_age_group(owner_id,
                                                     api_group)) == TPM_RC_OK)
                PR_RESULT
            else
                printk(KERN_INFO "%s: tpm_reset_pnc_age_group failed, rc[%d] - %s\n", __FUNCTION__,
                       ret_code, get_tpm_err_str(ret_code));
        }
    }
}


/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_tm_set_wan_q_ingr_rate_lim_bounce(uint32_t owner_id,
							uint32_t queue_id,
							uint32_t rate_limit_val,
							uint32_t bucket_size)
{
	tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_WAN_QUE_INGR_RATE_LIMIT;
	tpm_tm_tm_set->owner_id = owner_id;
	tpm_tm_tm_set->queue_id = queue_id;
	tpm_tm_tm_set->rate_limit_val = rate_limit_val;
	tpm_tm_tm_set->bucket_size = bucket_size;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}

 tpm_error_code_t tpm_tm_set_wan_queue_egr_rate_lim_bounce(uint32_t owner_id,
 							tpm_trg_port_type_t sched_ent,
							uint32_t queue_id,
							uint32_t rate_limit_val,
							uint32_t bucket_size)
 {
	 tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	 tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	 tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_WAN_QUE_EGR_RATE_LIMIT;
	 tpm_tm_tm_set->owner_id = owner_id;
	 tpm_tm_tm_set->sched_ent = sched_ent;
	 tpm_tm_tm_set->queue_id = queue_id;
	 tpm_tm_tm_set->rate_limit_val = rate_limit_val;
	 tpm_tm_tm_set->bucket_size = bucket_size;

	 up(&tpm_sfs_2_ioctl_sem);

	 return TPM_RC_OK;
 }

 #define _tpm_tm_set_wan_q_ingr_rate_lim tpm_tm_set_wan_q_ingr_rate_lim_bounce
 #define _tpm_tm_set_wan_queue_egr_rate_lim tpm_tm_set_wan_queue_egr_rate_lim_bounce
#else
 #define _tpm_tm_set_wan_q_ingr_rate_lim tpm_tm_set_wan_q_ingr_rate_lim
 #define _tpm_tm_set_wan_queue_egr_rate_lim tpm_tm_set_wan_queue_egr_rate_lim
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_rate_limit_queue_set
*
* DESCRIPTION:
*           This function sets rate limit on queue
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_rate_limit_queue_set       (const char *buf, size_t len)
{
    typedef enum
    {
        qratelim_owner=0, qratelim_direction, qratelim_if, qratelim_que, qratelim_rate_limit, qratelim_bucket_size, qratelim_max
    } qratelim_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    char                     direction_str[20];
    uint32_t                 direction;
    char                     entity_str[20];
    char                     uc_entity_str[20];
    int                      indx;
    uint32_t                 entity;
    uint32_t                 que;
    uint32_t                 rate_limit;
    uint32_t                 bucket_size;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != qratelim_max)
    {
        parm_error_completion(numparms, qratelim_max, buf, sfs_help_rate_limit);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %s %d %d %d", &ownerid, direction_str, entity_str, &que, &rate_limit, &bucket_size);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], direction_str[%s], entity_str[%s], que[%d], rate_limit[%d], bucket_size[%d]\n",
//               len, parsedargs, ownerid, direction_str, entity_str, que, rate_limit, bucket_size);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(entity_str); indx++) uc_entity_str[indx] = (char)toupper(entity_str[indx]);
        uc_entity_str[strlen(entity_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_direction_value(direction_str, &direction) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid direction[%s]\n", direction_str);
        }
        else if (get_sched_entity_value(uc_entity_str, &entity) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid sched_entity[%s] %s\n", entity_str, uc_entity_str);
        }
        else if ((direction == TPM_UPSTREAM   && que >= TPM_MAX_NUM_TX_QUEUE) ||
                 (direction == TPM_DOWNSTREAM && que >= TPM_MAX_NUM_RX_QUEUE))
        {
            printk(KERN_INFO "Invalid queue[%d]\n", que);
        }
        else
        {
            if (direction == TPM_DOWNSTREAM)
            {
                if ((rc = _tpm_tm_set_wan_q_ingr_rate_lim (ownerid,
                                                          // entity,
                                                          que,
                                                          rate_limit,
                                                          bucket_size)) == TPM_RC_OK)
                {
                    printk(KERN_INFO "OK\n");
                }
                else
                {
                    printk(KERN_INFO "%s: tpm_tm_set_wan_q_ingr_rate_lim failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
                }
            }
            else
            {
                if ((rc = _tpm_tm_set_wan_queue_egr_rate_lim (ownerid,
                                                             entity,
                                                             que,
                                                             rate_limit,
                                                             bucket_size)) == TPM_RC_OK)
                {
                    printk(KERN_INFO "OK\n");
                }
                else
                {
                    printk(KERN_INFO "%s: tpm_tm_set_wan_queue_egr_rate_lim failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
                }
            }
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_tm_set_wan_ingr_queue_sched_bounce(uint32_t owner_id,
							tpm_pp_sched_type_t sched_mode,
							uint8_t queue_id,
							uint16_t wrr_weight)
{
	tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_WAN_INGR_QUE_SCHED;
	tpm_tm_tm_set->owner_id = owner_id;
	tpm_tm_tm_set->pp_sched_mode = sched_mode;
	tpm_tm_tm_set->queue_id = queue_id;
	tpm_tm_tm_set->wrr_weight = wrr_weight;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}

 tpm_error_code_t tpm_tm_set_wan_egr_queue_sched_bounce(uint32_t owner_id,
							tpm_trg_port_type_t sched_ent,
							tpm_pp_sched_type_t sched_mode,
							uint8_t queue_id,
							uint16_t wrr_weight)
 {
	 tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	 tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	 tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_WAN_EGR_QUE_SCHED;
	 tpm_tm_tm_set->owner_id = owner_id;
	 tpm_tm_tm_set->sched_ent = sched_ent;
	 tpm_tm_tm_set->pp_sched_mode = sched_mode;
	 tpm_tm_tm_set->queue_id = queue_id;
	 tpm_tm_tm_set->wrr_weight = wrr_weight;

	 up(&tpm_sfs_2_ioctl_sem);

	 return TPM_RC_OK;
 }

 #define _tpm_tm_set_wan_ingr_queue_sched tpm_tm_set_wan_ingr_queue_sched_bounce
 #define _tpm_tm_set_wan_egr_queue_sched tpm_tm_set_wan_egr_queue_sched_bounce
#else
 #define _tpm_tm_set_wan_ingr_queue_sched tpm_tm_set_wan_ingr_queue_sched
 #define _tpm_tm_set_wan_egr_queue_sched tpm_tm_set_wan_egr_queue_sched
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_scheduling_mode_queue_set
*
* DESCRIPTION:
*           This function configures queue operation mode (per IF, per direction)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_scheduling_mode_queue_set       (const char *buf, size_t len)
{
    typedef enum
    {
        sched_owner=0, sched_direction, sched_if, sched_que, sched_rate_limit, sched_bucket_size, sched_max
    } sched_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    char                     direction_str[20];
    uint32_t                 direction;
    char                     entity_str[20];
    char                     uc_entity_str[20];
    int                      indx;
    uint32_t                 entity;
    char                     mode_str[20];
    uint32_t                 mode;
    uint32_t                 que;
    uint32_t                 wrr_weight;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != sched_max)
    {
        parm_error_completion(numparms, sched_max, buf, sfs_help_rate_limit);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %s %s %d %d", &ownerid, direction_str, entity_str, mode_str, &que, &wrr_weight);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], direction_str[%s], entity_str[%s], mode[%s], que[%d], wrr_weight[%d]\n",
//               len, parsedargs, ownerid, direction_str, entity_str, mode_str, que, wrr_weight);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(entity_str); indx++) uc_entity_str[indx] = (char)toupper(entity_str[indx]);
        uc_entity_str[strlen(entity_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_direction_value(direction_str, &direction) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid direction[%s]\n", direction_str);
        }
        else if (get_sched_entity_value(uc_entity_str, &entity) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid sched_entity[%s] %s\n", entity_str, uc_entity_str);
        }
        else if (get_scheduling_mode_value(mode_str, &mode) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid mode[%s]\n", mode_str);
        }
        else if ((direction == TPM_UPSTREAM   && que >= TPM_MAX_NUM_TX_QUEUE) ||
                 (direction == TPM_DOWNSTREAM && que >= TPM_MAX_NUM_RX_QUEUE))
        {
            printk(KERN_INFO "Invalid queue[%d]\n", que);
        }
        else
        {
            if (direction == TPM_DOWNSTREAM)
            {
                if ((rc = _tpm_tm_set_wan_ingr_queue_sched (ownerid,
                                                           mode,
                                                           que,
                                                           wrr_weight)) == TPM_RC_OK)
                {
                    printk(KERN_INFO "OK\n");
                }
                else
                {
                    printk(KERN_INFO "%s: tpm_tm_set_wan_ingr_queue_sched failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
                }
            }
            else
            {
                if ((rc = _tpm_tm_set_wan_egr_queue_sched (ownerid,
                                                          entity,
                                                          mode,
                                                          que,
                                                          wrr_weight)) == TPM_RC_OK)
                {
                    printk(KERN_INFO "OK\n");
                }
                else
                {
                    printk(KERN_INFO "%s: tpm_tm_set_wan_egr_queue_sched failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
                }
            }
        }
    }
}

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_tm_set_wan_ingr_rate_lim_bounce(uint32_t owner_id,
							uint32_t rate_limit_val,
							uint32_t bucket_size)
{
	tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_WAN_INGR_RATE_LIMIT;
	tpm_tm_tm_set->owner_id = owner_id;
	tpm_tm_tm_set->rate_limit_val = rate_limit_val;
	tpm_tm_tm_set->bucket_size = bucket_size;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}

 tpm_error_code_t tpm_tm_set_wan_sched_egr_rate_lim_bounce(uint32_t owner_id,
 							tpm_trg_port_type_t sched_ent,
							uint32_t rate_limit_val,
							uint32_t bucket_size)
 {
	 tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	 tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	 tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_WAN_SCHED_EGR_RATE_LIMIT;
	 tpm_tm_tm_set->owner_id = owner_id;
	 tpm_tm_tm_set->sched_ent = sched_ent;
	 tpm_tm_tm_set->rate_limit_val = rate_limit_val;
	 tpm_tm_tm_set->bucket_size = bucket_size;

	 up(&tpm_sfs_2_ioctl_sem);

	 return TPM_RC_OK;
 }

 #define _tpm_tm_set_wan_ingr_rate_lim tpm_tm_set_wan_ingr_rate_lim_bounce
 #define _tpm_tm_set_wan_sched_egr_rate_lim tpm_tm_set_wan_sched_egr_rate_lim_bounce
#else
 #define _tpm_tm_set_wan_ingr_rate_lim tpm_tm_set_wan_ingr_rate_lim
 #define _tpm_tm_set_wan_sched_egr_rate_lim tpm_tm_set_wan_sched_egr_rate_lim
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_rate_limit_if_set
*
* DESCRIPTION:
*           This function sets rate limit on interface
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_rate_limit_if_set          (const char *buf, size_t len)
{
    typedef enum
    {
        entratelim_owner=0, entratelim_direction, entratelim_if, entratelim_rate_limit, entratelim_bucket_size, entratelim_max
    } entratelim_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    char                     direction_str[20];
    uint32_t                 direction;
    char                     entity_str[20];
    char                     uc_entity_str[20];
    int                      indx;
    uint32_t                 entity;
    uint32_t                 rate_limit;
    uint32_t                 bucket_size;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != entratelim_max)
    {
        parm_error_completion(numparms, entratelim_max, buf, sfs_help_rate_limit);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %s %d %d", &ownerid, direction_str, entity_str, &rate_limit, &bucket_size);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], direction_str[%s], entity_str[%s], rate_limit[%d], bucket_size[%d]\n",
//               len, parsedargs, ownerid, direction_str, entity_str, rate_limit, bucket_size);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(entity_str); indx++) uc_entity_str[indx] = (char)toupper(entity_str[indx]);
        uc_entity_str[strlen(entity_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_direction_value(direction_str, &direction) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid direction[%s]\n", direction_str);
        }
        else if (get_sched_entity_value(uc_entity_str, &entity) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid sched_entity[%s]\n", entity_str);
        }
        else
        {
            if (direction == TPM_DOWNSTREAM)
            {
                if ((rc = _tpm_tm_set_wan_ingr_rate_lim (ownerid,
                                                        rate_limit,
                                                        bucket_size)) == TPM_RC_OK)
                {
                    printk(KERN_INFO "OK\n");
                }
                else
                {
                    printk(KERN_INFO "%s: tpm_tm_set_wan_ingr_rate_lim failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
                }
            }
            else
            {
                if ((rc = _tpm_tm_set_wan_sched_egr_rate_lim (ownerid,
                                                             entity,
                                                             rate_limit,
                                                             bucket_size)) == TPM_RC_OK)
                {
                    printk(KERN_INFO "OK\n");
                }
                else
                {
                    printk(KERN_INFO "%s: tpm_tm_set_wan_sched_egr_rate_lim failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
                }
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_tm_set_gmac0_ingr_rate_lim_bounce(uint32_t owner_id,
						       uint32_t rate_limit_val,
						       uint32_t bucket_size)
{
	tpm_ioctl_tm_tm_t *tpm_tm_tm_set = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_tm_tm;

	tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_PP_TM_SECTION;
	tpm_tm_tm_set->pp_tm_cmd = MV_TPM_IOCTL_TM_SET_GMAC0_INGR_RATE_LIMIT;
	tpm_tm_tm_set->owner_id = owner_id;
	tpm_tm_tm_set->rate_limit_val = rate_limit_val;
	tpm_tm_tm_set->bucket_size = bucket_size;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;
}

 #define _tpm_tm_set_gmac0_ingr_rate_lim tpm_tm_set_gmac0_ingr_rate_lim_bounce
#else
 #define _tpm_tm_set_gmac0_ingr_rate_lim tpm_tm_set_gmac0_ingr_rate_lim
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_gmac0_ingr_rate_limit
*
* DESCRIPTION:
*           This function sets rate limit on queue
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_gmac0_ingr_rate_limit(const char *buf, size_t len)
{
    typedef enum
    {
        qratelim_owner=0, qratelim_rate_limit, qratelim_bucket_size, qratelim_max
    } qratelim_parm_indx_t;
    /* shell line parsing */
    uint32_t                 ownerid;
    uint32_t                 rate_limit;
    uint32_t                 bucket_size;
    int                      parsedargs;
    int                      numparms;
    /* Used in API call */
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != qratelim_max) {
        parm_error_completion(numparms, qratelim_max, buf, sfs_help_rate_limit);
    } else {
        /* Get parameters */
        parsedargs = sscanf(buf, "%d %d %d", &ownerid, &rate_limit, &bucket_size);
        if (parsedargs != numparms) {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        } else {
            if ((rc = _tpm_tm_set_gmac0_ingr_rate_lim(ownerid,
                                                      rate_limit,
                                                      bucket_size)) == TPM_RC_OK)
                printk(KERN_INFO "OK\n");
            else
                printk(KERN_INFO "%s: tpm_tm_set_gmac0_ingr_rate_lim failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_dip_rule_bounce(uint32_t              owner_id,
                                              tpm_src_port_type_t   src_port,
                                              uint32_t              rule_num,
                                              uint32_t             *rule_idx,
                                              tpm_parse_fields_t    parse_rule_bm,
                                              tpm_parse_flags_t     parse_flags_bm,
                                              tpm_ipv6_addr_key_t  *ipv6_dip_key,
                                              tpm_pkt_frwd_t       *pkt_frwd,
                                              tpm_pkt_mod_t        *pkt_mod,
                                              tpm_pkt_mod_bm_t      pkt_mod_bm,
                                              tpm_rule_action_t    *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                             = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                           = MV_TPM_IOCTL_ADD_IPV6_DIP_ACL_RULE;
    tpm_add_acl_rule->owner_id                              = owner_id;
    tpm_add_acl_rule->src_port                              = src_port;
    tpm_add_acl_rule->rule_num                              = rule_num;
    tpm_add_acl_rule->parse_rule_bm                         = parse_rule_bm;
    tpm_add_acl_rule->ipv6_dip_acl_rule.parse_flags_bm      = parse_flags_bm;
    tpm_add_acl_rule->ipv6_dip_acl_rule.pkt_mod_bm          = pkt_mod_bm;

    memcpy(&(tpm_add_acl_rule->ipv6_dip_acl_rule.ipv6_dip_key), ipv6_dip_key,   sizeof(tpm_ipv6_addr_key_t));
    memcpy(&(tpm_add_acl_rule->ipv6_dip_acl_rule.pkt_frwd),     pkt_frwd,       sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ipv6_dip_acl_rule.pkt_mod),      pkt_mod,        sizeof(tpm_pkt_mod_t));
    memcpy(&(tpm_add_acl_rule->ipv6_dip_acl_rule.rule_action),  rule_action,    sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_dip_rule tpm_add_ipv6_dip_rule_bounce
#else
 #define _tpm_add_ipv6_dip_rule tpm_add_ipv6_dip_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_dip_acl_rule_add
*
* DESCRIPTION:
*           This function creates a IPV6 DIP rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_dip_acl_rule_add  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6ruleadd_owner=0, ipv6ruleadd_srcport,   ipv6ruleadd_rulenum,  ipv6ruleadd_parserulebm, ipv6ruleadd_parseflagsbm,
        ipv6ruleadd_action,  ipv6ruleadd_nextphase, ipv6ruleadd_modbm,    ipv6ruleadd_ipv6dipkeyname, ipv6ruleadd_frwdname,
        ipv6ruleadd_modname, ipv6dipruleadd_max
    } ipv6dipruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    char                     srcport_str[20];
    char                     uc_srcport_str[20];
    int                      indx;
    uint32_t                 src_port;
    unsigned int             rulenum;
    unsigned int             parserulebm;
    unsigned int             parseflagsbm;
    unsigned int             action;
    unsigned int             nextphase;
    char                     nextphase_str[20];
    unsigned int             modbm;
    char                     ipv6keyname[30];
    char                     frwdname[20];
    char                     modname[20];
    int                      parsedargs;
    int                      numparms;
    // DB
    tpmcfg_ipv6_dip_key_entry_t  *pdbipv6keyentry = 0;
    tpm_ipv6_addr_key_t    ipv6AddrKey;
    tpmcfg_frwd_entry_t      *pdbfrwdentry    = 0;
    tpmcfg_mod_entry_t       *pdbmodentry     = 0;
    // Used in API call
    tpm_rule_action_t        rule_action;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    //tpm_ipv6_acl_key_t       ipv6_acl;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ipv6dipruleadd_max)
    {
        parm_error_completion(numparms, ipv6dipruleadd_max, buf, sys_help_ipv6_dip_acl_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s 0x%x %s %s %s",
                            &ownerid, srcport_str, &rulenum, &parserulebm, &parseflagsbm, &action,
                            nextphase_str, &modbm, ipv6keyname, frwdname, modname);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], modbm[0x%x], ipv6keyname[%s], frwdname[%s], modname[%s]\n",
               len, parsedargs, ownerid, srcport_str, rulenum, parserulebm, parseflagsbm, action, nextphase_str, modbm, ipv6keyname, frwdname, modname);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else if (((pdbipv6keyentry = find_tpm_ipv6_dip_key_entry_by_name(ipv6keyname)) == 0) && strcmp(ipv6keyname, ipv6_dip_key_empty_name) != 0)
        {
            printk(KERN_INFO "IPV6 key entry [%s] not found\n", ipv6keyname);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && strcmp(modname, mod_empty_name) != 0)
        {
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbipv6keyentry != 0)
            {
                memcpy(&(ipv6AddrKey.ipv6_ip_add), &(pdbipv6keyentry->ipv6_addr.ipv6_ip_add), sizeof(pdbipv6keyentry->ipv6_addr.ipv6_ip_add));
                memcpy(&(ipv6AddrKey.ipv6_ip_add_mask), &(pdbipv6keyentry->ipv6_addr.ipv6_ip_add_mask), sizeof(pdbipv6keyentry->ipv6_addr.ipv6_ip_add_mask));
            }
            else                       memset(&ipv6AddrKey, 0,                          sizeof(ipv6AddrKey));

            if (pdbfrwdentry != 0)     memcpy(&pkt_frwd, &pdbfrwdentry->frwd,    sizeof(tpm_pkt_frwd_t));
            else                       memset(&pkt_frwd, 0,                      sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)      memcpy(&pkt_mod, &pdbmodentry->mod,      sizeof(tpm_pkt_mod_t));
            else                       memset(&pkt_mod, 0,                      sizeof(tpm_pkt_mod_t));

            if ((rc = _tpm_add_ipv6_dip_rule(ownerid,
                                             src_port,
                                             rulenum,
                                             &rule_idx,
                                             parserulebm,
                                             parseflagsbm,
                                             &ipv6AddrKey,
                                             &pkt_frwd,
                                             &pkt_mod,
                                             modbm,
                                             &rule_action)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv6_dip_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_dip_rule_bounce(uint32_t              owner_id,
                                              uint32_t              rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd                   = MV_TPM_IOCTL_DEL_IPV6_DIP_ACL_RULE;
    tpm_del_acl_rule->owner_id                      = owner_id;
    tpm_del_acl_rule->rule_idx                      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_dip_rule tpm_del_ipv6_dip_rule_bounce
#else
 #define _tpm_del_ipv6_dip_rule tpm_del_ipv6_dip_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_dip
*
* DESCRIPTION:
*           This function deletes a IPV6 HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_dip  (const char *buf, size_t len)
{
    typedef enum
    {
        noruleaddipv6_owner=0,  noruleaddipv6_ruleidx, noruleaddipv6_max
    } noruleaddipv6_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 rule_idx;
//    uint32_t                 parserulebm;
//    char                     keyname[20];
    int                      parsedargs;
    int                      numparms;
    // DB
//    tpmcfg_ipv6_dip_key_entry_t  *pdbipv6keyentry = 0;
    //Used in API call
    //tpm_ipv6_acl_key_t       ipv6_acl;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noruleaddipv6_max)
    {
        parm_error_completion(numparms, noruleaddipv6_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        // Zeev
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
//        parsedargs = sscanf(buf, "%d %s %d 0x%x %s",
//                            &ownerid, srcport_str, &rule_idx, &parserulebm, keyname);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rule_idx[%d], parserulebm[0x%x], keyname[%s]\n",
//               len, parsedargs, ownerid, srcport_str, rule_idx, parserulebm, keyname);


        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
//        else if ((pdbipv6keyentry = find_tpm_ipv6_dip_key_entry_by_name(keyname)) == 0)
//        {
//            printk(KERN_INFO "key entry [%s] not found\n", keyname);
//        }
        else
        {
            //memcpy(&ipv6_acl, &pdbipv6keyentry->ipv6_acl, sizeof(tpm_ipv6_acl_key_t));

            if ((rc = _tpm_del_ipv6_dip_rule(ownerid,
                                             rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
            {
                printk(KERN_INFO "%s: tpm_del_ipv6_dip_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_l4_ports_rule_bounce(uint32_t                owner_id,
                                                   tpm_src_port_type_t     src_port,
                                                   uint32_t                rule_num,
                                                   uint32_t               *rule_idx,
                                                   tpm_parse_fields_t      parse_rule_bm,
                                                   tpm_parse_flags_t       parse_flags_bm,
                                                   tpm_l4_ports_key_t     *l4_key,
                                                   tpm_pkt_frwd_t         *pkt_frwd,
                                                   tpm_pkt_mod_t          *pkt_mod,
                                                   tpm_pkt_mod_bm_t        pkt_mod_bm,
                                                   tpm_rule_action_t      *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                             = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                           = MV_TPM_IOCTL_ADD_IPV6_L4_PORTS_ACL_RULE;
    tpm_add_acl_rule->owner_id                              = owner_id;
    tpm_add_acl_rule->src_port                              = src_port;
    tpm_add_acl_rule->rule_num                              = rule_num;
    tpm_add_acl_rule->parse_rule_bm                         = parse_rule_bm;
    tpm_add_acl_rule->ipv6_l4_ports_acl_rule.parse_flags_bm = parse_flags_bm;
    tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_mod_bm     = pkt_mod_bm;

    memcpy(&(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.l4_key),      l4_key,         sizeof(tpm_l4_ports_key_t));
    memcpy(&(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_frwd),    pkt_frwd,       sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.pkt_mod),     pkt_mod,        sizeof(tpm_pkt_mod_t));
    memcpy(&(tpm_add_acl_rule->ipv6_l4_ports_acl_rule.rule_action), rule_action,    sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_l4_ports_rule tpm_add_ipv6_l4_ports_rule_bounce
#else
 #define _tpm_add_ipv6_l4_ports_rule tpm_add_ipv6_l4_ports_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_l4_ports_acl_rule_add
*
* DESCRIPTION:
*           This function adds a IPV6 HW L4 ports ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_l4_ports_acl_rule_add (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6l4ports_owner=0, ipv6l4ports_srcport,   ipv6l4ports_rulenum,  ipv6l4ports_parserulebm, ipv6l4ports_parseflagsbm,
        ipv6l4ports_action,   ipv6l4ports_next_phase, ipv6l4ports_src_port, ipv6l4ports_dst_port,    ipv6l4ports_modbm,
        ipv6l4ports_frwdname, ipv6l4ports_modname,    ipv6l4ports_max
    } ipv6l4ports_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    char                     srcport_str[20];
    char                     uc_srcport_str[20];
    int                      indx;
    uint32_t                 src_port;
    unsigned int             rulenum;
    unsigned int             parserulebm;
    unsigned int             parseflagsbm;

    uint32_t                 temp_l4_src_port;
    uint32_t                 temp_l4_dst_port;

    char                     frwdname[20];

    char                     modname[20];
    unsigned int             modbm;

    unsigned int             action;
    unsigned int             nextphase;
    char                     nextphase_str[20];

    int                      parsedargs;
    int                      numparms;
    // DB
    tpmcfg_frwd_entry_t      *pdbfrwdentry    = 0;
    tpmcfg_mod_entry_t       *pdbmodentry     = 0;
    // Used in API call
    tpm_l4_ports_key_t       l4_key;
    tpm_rule_action_t        rule_action;
    tpm_pkt_frwd_t           pkt_frwd;
    tpm_pkt_mod_t            pkt_mod;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ipv6l4ports_max)
    {
        parm_error_completion(numparms, ipv6l4ports_max, buf, sfs_help_ipv6_l4_ports_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x  0x%x %s %d %d  0x%x %s %s",
                            &ownerid, srcport_str, &rulenum, &parserulebm, &parseflagsbm, &action, nextphase_str,
                            &temp_l4_src_port, &temp_l4_dst_port, &modbm, frwdname, modname);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], l4_src_port[%d], l4_dst_port[%d], modbm[0x%x], frwdname[%s], modname[%s]\n",
               len, parsedargs, ownerid, srcport_str, rulenum, parserulebm, parseflagsbm, action, nextphase_str, temp_l4_src_port, temp_l4_dst_port, modbm, frwdname, modname);

        // Minimal help for src_port upper/lower case support
        for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && strcmp(modname, mod_empty_name) != 0)
        {
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            l4_key.l4_dst_port = (uint16_t)temp_l4_dst_port;
            l4_key.l4_src_port = (uint16_t)temp_l4_src_port;

            if (pdbfrwdentry != 0)     memcpy(&pkt_frwd, &pdbfrwdentry->frwd,    sizeof(tpm_pkt_frwd_t));
            else                       memset(&pkt_frwd, 0,                      sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)      memcpy(&pkt_mod, &pdbmodentry->mod,      sizeof(tpm_pkt_mod_t));
            else                       memset(&pkt_mod, 0,                      sizeof(tpm_pkt_mod_t));

            if ((rc = _tpm_add_ipv6_l4_ports_rule(ownerid,
                                                  src_port,
                                                  rulenum,
                                                  &rule_idx,
                                                  parserulebm,
                                                  parseflagsbm,
                                                  &l4_key,
                                                  &pkt_frwd,
                                                  &pkt_mod,
                                                  modbm,
                                                  &rule_action)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv6_l4_ports_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_l4_ports_rule_bounce(uint32_t              owner_id,
                                                   uint32_t              rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd                   = MV_TPM_IOCTL_DEL_IPV6_L4_PORTS_ACL_RULE;
    tpm_del_acl_rule->owner_id                      = owner_id;
    tpm_del_acl_rule->rule_idx                      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_l4_ports_rule tpm_del_ipv6_l4_ports_rule_bounce
#else
 #define _tpm_del_ipv6_l4_ports_rule tpm_del_ipv6_l4_ports_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_acl
*
* DESCRIPTION:
*           This function deletes a IPV6 HW L4 ports ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_acl (const char *buf, size_t len)
{
    typedef enum
    {
        noipv6l4ports_owner=0, noipv6l4ports_ruleidx, noipv6l4ports_max
    } noipv6l4portsruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noipv6l4ports_max)
    {
        parm_error_completion(numparms, noipv6l4ports_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
               len, parsedargs, ownerid, rule_idx);


        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_del_ipv6_l4_ports_rule(ownerid,
                                                  rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
            {
                printk(KERN_INFO "%s: tpm_del_ipv6_l4_ports_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_nh_rule_bounce(uint32_t           owner_id,
                                            uint32_t            rule_num,
                                            uint32_t           *rule_idx,
                                            tpm_parse_flags_t   parse_flags_bm,
                                            tpm_nh_iter_t       nh_iter,
                                            uint32_t            nh,
                                            tpm_pkt_frwd_t     *pkt_frwd,
                                            tpm_rule_action_t  *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_ADD_IPV6_NH_ACL_RULE;
    tpm_add_acl_rule->owner_id                      = owner_id;
    tpm_add_acl_rule->rule_num                      = rule_num;
    tpm_add_acl_rule->parse_rule_bm                 = parse_flags_bm;
    tpm_add_acl_rule->ipv6_nh_acl_rule.nh_iter      = nh_iter;
    tpm_add_acl_rule->ipv6_nh_acl_rule.nh           = nh;
    memcpy(&(tpm_add_acl_rule->ipv6_nh_acl_rule.pkt_frwd),      pkt_frwd,       sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ipv6_nh_acl_rule.rule_action),   rule_action,    sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_nh_rule tpm_add_ipv6_nh_rule_bounce
#else
 #define _tpm_add_ipv6_nh_rule tpm_add_ipv6_nh_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_nh_acl_rule_add
*
* DESCRIPTION:
*           This function adds a IPV6 HW NH (Next Header) ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_nh_acl_rule_add    (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6nh_owner=0,   ipv6nh_rulenum,   ipv6nh_parseflagsbm, ipv6nh_action, ipv6nh_nextphase, ipv6nh_nhiter,
         ipv6nh_nh,       ipv6nh_frwdname,  ipv6nh_max
    } ipv6nhruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    unsigned int             rulenum;
    unsigned int             parseflagsbm;
    unsigned int             nh_iter;
    unsigned int             nh;
    unsigned int             action;
    unsigned int             nextphase;
    char                     nextphase_str[20];
    int                      parsedargs;
    int                      numparms;
    // DB
    char                     frwdname[20];
    tpmcfg_frwd_entry_t      *pdbfrwdentry    = 0;
    // Used in API call
    tpm_rule_action_t        rule_action;
    tpm_pkt_frwd_t           pkt_frwd;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ipv6nh_max)
    {
        parm_error_completion(numparms, ipv6nh_max, buf, sfs_help_ipv6_nh_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d 0x%x 0x%x %s %d 0x%x %s",
                            &ownerid, &rulenum, &parseflagsbm, &action, nextphase_str, &nh_iter, &nh, frwdname);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rulenum[%d], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], nh_iter[%d], nh[0x%x], frwdname[%s]\n",
               len, parsedargs, ownerid, rulenum, parseflagsbm, action, nextphase_str, nh_iter, nh, frwdname);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name) != 0)
        {
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        }
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        }
        else
        {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbfrwdentry != 0)     memcpy(&pkt_frwd, &pdbfrwdentry->frwd,    sizeof(tpm_pkt_frwd_t));
            else                       memset(&pkt_frwd, 0,                      sizeof(tpm_pkt_frwd_t));

            if ((rc = _tpm_add_ipv6_nh_rule(ownerid,
                                            rulenum,
                                            &rule_idx,
                                            parseflagsbm,
                                            nh_iter,
                                            nh,
                                            &pkt_frwd,
                                            &rule_action)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_ipv6_nh_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_nh_rule_bounce(uint32_t           owner_id,
                                            uint32_t            rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd                   = MV_TPM_IOCTL_DEL_IPV6_NH_ACL_RULE;
    tpm_del_acl_rule->owner_id                      = owner_id;
    tpm_del_acl_rule->rule_idx                      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_nh_rule tpm_del_ipv6_nh_rule_bounce
#else
 #define _tpm_del_ipv6_nh_rule tpm_del_ipv6_nh_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_nh_acl
*
* DESCRIPTION:
*           This function deletes a IPV6 HW nh_acl rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_nh_acl    (const char *buf, size_t len)
{
    typedef enum
    {
        noipv6nh_owner=0, noipv6nh_ruleidx, noipv6nh_max
    } noipv6nhruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noipv6nh_max)
    {
        parm_error_completion(numparms, noipv6nh_max, buf, sfs_help_no_rule_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",len, parsedargs, ownerid, rule_idx);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_del_ipv6_nh_rule(ownerid,
                                            rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
            {
                printk(KERN_INFO "%s: tpm_del_ipv6_nh_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ctc_cm_acl_rule_bounce(uint32_t owner_id,
						tpm_src_port_type_t src_port,
						uint32_t precedence,
						tpm_parse_fields_t l2_parse_rule_bm,
						tpm_parse_fields_t ipv4_parse_rule_bm,
						tpm_l2_acl_key_t *l2_key,
						tpm_ipv4_acl_key_t *ipv4_key,
						tpm_pkt_frwd_t *pkt_frwd,
						tpm_pkt_action_t *pkt_act,
						uint32_t pbits)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                          = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                        = MV_TPM_IOCTL_ADD_CTC_CM_ACL_RULE;
    tpm_add_acl_rule->owner_id                           = owner_id;
    tpm_add_acl_rule->src_port                           = src_port;
    tpm_add_acl_rule->ctc_cm_acl_rule.precedence         = precedence;
    tpm_add_acl_rule->ctc_cm_acl_rule.l2_parse_rule_bm   = l2_parse_rule_bm;
    tpm_add_acl_rule->ctc_cm_acl_rule.ipv4_parse_rule_bm = ipv4_parse_rule_bm;
    tpm_add_acl_rule->ctc_cm_acl_rule.p_bits             = pbits;

    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.l2_key),        l2_key,       sizeof(tpm_l2_acl_key_t));
    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.ipv4_key),      ipv4_key,     sizeof(tpm_ipv4_acl_key_t));
    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.pkt_frwd),      pkt_frwd,     sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.pkt_act),       pkt_act,      sizeof(tpm_pkt_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ctc_cm_acl_rule tpm_add_ctc_cm_acl_rule_bounce
#else
 #define _tpm_add_ctc_cm_acl_rule tpm_add_ctc_cm_acl_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_ctc_cm_rule_add
*
* DESCRIPTION:
*           This function adds a CnM ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ctc_cm_rule_add (const char *buf, size_t len)
{
	typedef enum
	{
	    cnmruleadd_owner=0, cnmruleadd_srcport, cnmruleadd_precedence, cnmruleadd_l2parserulebm,
	    cnmruleadd_ipv4parserulebm, cnmruleadd_action, cnmruleadd_l2keyname, cnmruleadd_ipv4keyname,
	    cnmruleadd_frwdname, cnmruleadd_pbits, cnmruleadd_max
	} cnmruleadd_parm_indx_t;
	/*shell line parsing*/
	uint32_t             	  owner_id;
	char                      srcport_str[20];
	char                      uc_srcport_str[20];
	int                       indx;
	tpm_src_port_type_t 	  src_port;
	uint32_t            	  precedence;
	tpm_parse_fields_t  	  l2_parse_rule_bm;
	tpm_parse_fields_t  	  ipv4_parse_rule_bm;
	char                      l2keyname[20];
	tpm_l2_acl_key_t   	  l2_key;
	char                      ipv4keyname[20];
	tpm_ipv4_acl_key_t 	  ipv4_key;
	char                      frwdname[20];
	tpm_pkt_frwd_t     	  pkt_frwd;
	tpm_pkt_action_t 	  action;
	uint32_t            	  pbits;
	int                       parsedargs;
	int                       numparms;
	/*DB*/
	tpmcfg_l2_key_entry_t    *pdbl2keyentry   = NULL;
	tpmcfg_ipv4_key_entry_t  *pdbipv4keyentry = NULL;
	tpmcfg_frwd_entry_t      *pdbfrwdentry    = NULL;

	tpm_error_code_t         rc;

	numparms = count_parameters(buf);
	if (numparms != cnmruleadd_max) {
	    parm_error_completion(numparms, cnmruleadd_max, buf, sfs_help_ctc_cm_rule_add);
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s %s %s %d",  &owner_id, srcport_str, &precedence,
			&l2_parse_rule_bm, &ipv4_parse_rule_bm, &action, l2keyname, ipv4keyname, frwdname, &pbits);

		for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
		uc_srcport_str[strlen(srcport_str)] = 0;

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE) {
			printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
		} else if (((pdbl2keyentry = find_tpm_l2_key_entry_by_name(l2keyname)) == 0) && strcmp(l2keyname, l2_key_empty_name) != 0) {
			printk(KERN_INFO "L2 key entry [%s] not found\n", l2keyname);
		} else if (((pdbipv4keyentry = find_tpm_ipv4_key_entry_by_name(ipv4keyname)) == 0) && strcmp(ipv4keyname, ipv4_key_empty_name) != 0) {
			printk(KERN_INFO "IPV4 key entry [%s] not found\n", ipv4keyname);
		} else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name)!= 0) {
			printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
		} else {
			if (pdbl2keyentry != NULL)
				memcpy(&l2_key, &(pdbl2keyentry->l2_acl), sizeof(tpm_l2_acl_key_t));
			else
				memset(&l2_key, 0, sizeof(tpm_l2_acl_key_t));

			if (pdbipv4keyentry != NULL)
				memcpy(&ipv4_key, &(pdbipv4keyentry->ipv4_acl), sizeof(tpm_ipv4_acl_key_t));
			else
				memset(&ipv4_key, 0, sizeof(tpm_ipv4_acl_key_t));

			if (pdbfrwdentry != NULL)
				memcpy(&pkt_frwd, &(pdbfrwdentry->frwd), sizeof(tpm_pkt_frwd_t));
			else
				memset(&pkt_frwd, 0, sizeof(tpm_pkt_frwd_t));

			if ((rc = _tpm_add_ctc_cm_acl_rule(owner_id,
						src_port,
						precedence,
						l2_parse_rule_bm,
						ipv4_parse_rule_bm,
						&l2_key,
						&ipv4_key,
						&pkt_frwd,
						action,
						pbits)) == TPM_RC_OK)
				printk(KERN_INFO "OK. precedence = %d\n", precedence);
			else
				printk(KERN_INFO "%s: tpm_add_ctc_cm_acl_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));

		}
	}

}
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ctc_cm_ipv6_acl_rule_bounce(uint32_t owner_id,
						     tpm_src_port_type_t src_port,
						     uint32_t precedence,
						     tpm_parse_fields_t ipv6_parse_rule_bm,
						     tpm_ipv4_acl_key_t *ipv6_key,
						     tpm_pkt_frwd_t *pkt_frwd,
						     tpm_pkt_action_t *pkt_act,
						     uint32_t pbits)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                          = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                        = MV_TPM_IOCTL_ADD_CTC_CM_ACL_RULE;
    tpm_add_acl_rule->owner_id                           = owner_id;
    tpm_add_acl_rule->src_port                           = src_port;
    tpm_add_acl_rule->ctc_cm_acl_rule.precedence         = precedence;
    tpm_add_acl_rule->ctc_cm_acl_rule.ipv6_parse_rule_bm = ipv6_parse_rule_bm;
    tpm_add_acl_rule->ctc_cm_acl_rule.p_bits             = pbits;

    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.ipv6_key),      ipv6_key,     sizeof(tpm_ipv6_acl_key_t));
    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.pkt_frwd),      pkt_frwd,     sizeof(tpm_pkt_frwd_t));
    memcpy(&(tpm_add_acl_rule->ctc_cm_acl_rule.pkt_act),       pkt_act,      sizeof(tpm_pkt_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ctc_cm_ipv6_acl_rule tpm_add_ctc_cm_ipv6_acl_rule_bounce
#else
 #define _tpm_add_ctc_cm_ipv6_acl_rule tpm_add_ctc_cm_ipv6_acl_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_ctc_cm_ipv6_rule_add
*
* DESCRIPTION:
*           This function adds a CnM IPv6 ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ctc_cm_ipv6_rule_add (const char *buf, size_t len)
{
	typedef enum
	{
	    cnmruleadd_owner=0, cnmruleadd_srcport, cnmruleadd_precedence,
	    cnmruleadd_ipv6parserulebm, cnmruleadd_action, cnmruleadd_ipv6keyname,
	    cnmruleadd_frwdname, cnmruleadd_pbits, cnmruleipv6add_max
	} cnmruleadd_parm_indx_t;
	/*shell line parsing*/
	uint32_t		  owner_id;
	char			  srcport_str[20];
	char			  uc_srcport_str[20];
	int			  indx;
	tpm_src_port_type_t	  src_port;
	uint32_t		  precedence;
	tpm_parse_fields_t	  ipv6_parse_rule_bm;
	char			  ipv6keyname[20];
	tpm_ipv6_acl_key_t	  ipv6_key;
	char			  frwdname[20];
	tpm_pkt_frwd_t		  pkt_frwd;
	tpm_pkt_action_t	  action;
	uint32_t		  pbits;
	int			  parsedargs;
	int			  numparms;
	/*DB*/
	tpmcfg_ipv6_key_entry_t  *pdbipv6keyentry = NULL;
	tpmcfg_frwd_entry_t	 *pdbfrwdentry	  = NULL;

	tpm_error_code_t	 rc;

	numparms = count_parameters(buf);
	if (numparms != cnmruleipv6add_max) {
	    parm_error_completion(numparms, cnmruleipv6add_max, buf, sfs_help_ctc_cm_rule_add);
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x %s %s %d",  &owner_id, srcport_str, &precedence,
			&ipv6_parse_rule_bm, &action, ipv6keyname, frwdname, &pbits);

		for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
		uc_srcport_str[strlen(srcport_str)] = 0;

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE) {
			printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
		} else if (((pdbipv6keyentry = find_tpm_ipv6_key_entry_by_name(ipv6keyname)) == 0) && strcmp(ipv6keyname, ipv6_key_empty_name) != 0) {
			printk(KERN_INFO "ipv6 key entry [%s] not found\n", ipv6keyname);
		} else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && strcmp(frwdname, frwd_empty_name)!= 0) {
			printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
		} else {
			if (pdbipv6keyentry != NULL)
				memcpy(&ipv6_key, &(pdbipv6keyentry->ipv6_acl), sizeof(tpm_ipv6_acl_key_t));
			else
				memset(&ipv6_key, 0, sizeof(tpm_ipv6_acl_key_t));

			if (pdbfrwdentry != NULL)
				memcpy(&pkt_frwd, &(pdbfrwdentry->frwd), sizeof(tpm_pkt_frwd_t));
			else
				memset(&pkt_frwd, 0, sizeof(tpm_pkt_frwd_t));

			if ((rc = _tpm_add_ctc_cm_ipv6_acl_rule(owner_id,
						src_port,
						precedence,
						ipv6_parse_rule_bm,
						&ipv6_key,
						&pkt_frwd,
						action,
						pbits)) == TPM_RC_OK)
				printk(KERN_INFO "OK. precedence = %d\n", precedence);
			else
				printk(KERN_INFO "%s: tpm_add_ctc_cm_acl_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));

		}
	}

}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ctc_cm_acl_rule_bounce(uint32_t owner_id,
						tpm_src_port_type_t src_port,
						uint32_t precedence)
{
	tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

	tpm_sfs_2_ioctl_command.cmd			= MV_TPM_IOCTL_DEL_ACL_SECTION;
	tpm_del_acl_rule->del_acl_cmd			= MV_TPM_IOCTL_DEL_CTC_CM_ACL_RULE;
	tpm_del_acl_rule->owner_id			= owner_id;
	tpm_del_acl_rule->src_port			= src_port;
	tpm_del_acl_rule->precedence			= precedence;

	up(&tpm_sfs_2_ioctl_sem);

	return TPM_RC_OK;

}
 #define _tpm_del_ctc_cm_acl_rule tpm_del_ctc_cm_acl_rule_bounce
#else
 #define _tpm_del_ctc_cm_acl_rule tpm_del_ctc_cm_acl_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ctc_cm_acl
*
* DESCRIPTION:
*           This function deletes a ctc CnM HW acl rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ctc_cm_acl(const char *buf, size_t len)
{
	typedef enum
	{
		noctccm_owner=0, noctccm_srcport, noctccm_precedence, noctccm_max
	} noctccmruleadd_parm_indx_t;
	/*shell line parsing*/
	uint32_t             	  owner_id;
	char                      srcport_str[20];
	char                      uc_srcport_str[20];
	tpm_src_port_type_t 	  src_port;
	uint32_t            	  precedence;
	int                       parsedargs;
	int                       numparms;
	int                       indx;

	tpm_error_code_t         rc;

	numparms = count_parameters(buf);
	if (numparms != noctccm_max) {
	    parm_error_completion(numparms, noctccm_max, buf, sfs_help_no_rule_add);
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %s %d",  &owner_id, srcport_str, &precedence);

		for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
		uc_srcport_str[strlen(srcport_str)] = 0;

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE) {
			printk(KERN_INFO "Invalid src_port[%s]\n", srcport_str);
		} else {
			if ((rc = _tpm_del_ctc_cm_acl_rule(owner_id,
							src_port,
							precedence)) == TPM_RC_OK)
                		PR_RESULT
			else
		                printk(KERN_INFO "%s: tpm_del_ctc_cm_acl_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
		}
	}

}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_updt_ipv4_mc_stream_bounce(uint32_t            owner_id,
                                                uint32_t            stream_num,
                                                tpm_trg_port_type_t dest_port_bm)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd         = MV_TPM_IOCTL_MOD_IPv4_MC_STREAM;
    tpm_mc_rule->stream_num     = stream_num;
    tpm_mc_rule->dest_port_bm   = dest_port_bm;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_updt_ipv4_mc_stream tpm_updt_ipv4_mc_stream_bounce
#else
 #define _tpm_updt_ipv4_mc_stream tpm_updt_ipv4_mc_stream
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_mc_ipv4_stream_update
*
* DESCRIPTION:
*           This function updates a IPv4 multicast stream
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mc_ipv4_stream_update    (const char *buf, size_t len)
{
    typedef enum
    {
        ipv4strupdt_owner=0, ipv4strupdt_stream_num, ipv4strupdt_dest_port_bm, ipv4strupdt_max
    } noipv6nhruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 stream_num;
    uint32_t                 dest_port_bm;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ipv4strupdt_max)
    {
        parm_error_completion(numparms, ipv4strupdt_max, buf, sfs_help_mc_ipvx_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d 0x%x", &ownerid, &stream_num, &dest_port_bm);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream_num[%d], dest_port_bm[0x%x]\n",
               len, parsedargs, ownerid, stream_num, dest_port_bm);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_updt_ipv4_mc_stream(ownerid,
                                               stream_num,
                                               dest_port_bm)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK.\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_updt_ipv4_mc_stream failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_updt_ipv6_mc_stream_bounce(uint32_t            owner_id,
                                                uint32_t            stream_num,
                                                tpm_trg_port_type_t dest_port_bm)
{
    tpm_ioctl_mc_rule_t *tpm_mc_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_mc_rule;

    tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_MC_STREAM_SECTION;
    tpm_mc_rule->mc_cmd         = MV_TPM_IOCTL_MOD_IPv6_MC_STREAM;
    tpm_mc_rule->stream_num     = stream_num;
    tpm_mc_rule->dest_port_bm   = dest_port_bm;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_updt_ipv6_mc_stream tpm_updt_ipv6_mc_stream_bounce
#else
 #define _tpm_updt_ipv6_mc_stream tpm_updt_ipv6_mc_stream
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_mc_ipv6_stream_update
*
* DESCRIPTION:
*           This function updates a ipv6 multicast stream
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mc_ipv6_stream_update    (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6strupdt_owner=0, ipv6strupdt_stream_num, ipv6strupdt_dest_port_bm, ipv6strupdt_max
    } noipv6nhruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 stream_num;
    uint32_t                 dest_port_bm;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ipv6strupdt_max)
    {
        parm_error_completion(numparms, ipv6strupdt_max, buf, sfs_help_mc_ipvx_stream_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d 0x%x", &ownerid, &stream_num, &dest_port_bm);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], stream_num[%d], dest_port_bm[0x%x]\n",
               len, parsedargs, ownerid, stream_num, dest_port_bm);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_updt_ipv6_mc_stream(ownerid,
                                               stream_num,
                                               dest_port_bm)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK.\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_updt_ipv6_mc_stream failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

void sfs_tpm_cfg_set_mc_vid_key_reset    (const char *buf, size_t len)
{
    typedef enum
    {
        mcvidkeyreset_owner=0,  mcvidkeyreset_max
    } mcvidkeyreset_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    int                      parsedargs;
    int                      numparms;

    numparms = count_parameters(buf);
    if (numparms != mcvidkeyreset_max)
    {
        parm_error_completion(numparms, mcvidkeyreset_max, buf, sfs_help_igmp_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d", &ownerid);
        printk(KERN_INFO "parsedargs=%d. ownerid[%d]\n", parsedargs, ownerid);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            tpm_reset_mc_vid_key ();
            printk(KERN_INFO "OK.\n");
        }
    }
}

void sfs_tpm_cfg_set_mc_vid_key_set    (const char *buf, size_t len)
{
    typedef enum
    {
        mcvidkeyset_src_port=0, mcvidkeyset_mc_uni_xlate_mode, mcvidkeyset_mc_uni_xlate_vid, mcvidkeyset_max
    } mcvidkeyset_parm_indx_t;
    // shell line parsing
    char                     srcport_str[30];
    char                     uc_srcport_str[30];
    uint32_t                 src_port;
    uint32_t                 mc_uni_xlate_mode;
    uint32_t                 mc_uni_xlate_vid;
    int                      parsedargs;
    int                      numparms;
    int                      indx;

    numparms = count_parameters(buf);
    if (numparms != mcvidkeyset_max)
    {
        parm_error_completion(numparms, mcvidkeyset_max, buf, sfs_help_igmp_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d %d", srcport_str, &mc_uni_xlate_mode, &mc_uni_xlate_vid);
        printk(KERN_INFO "len=%d, parsedargs=%d. src_port[%s], mc_uni_xlate_mode[%d], mc_uni_xlate_vid[%d]\n",
               len, parsedargs, srcport_str, mc_uni_xlate_mode, mc_uni_xlate_vid);

         for (indx = 0; indx < strlen(srcport_str); indx++) uc_srcport_str[indx] = (char)toupper(srcport_str[indx]);
        uc_srcport_str[strlen(srcport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_srcport_value(uc_srcport_str, &src_port) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid src_port [%s]\n", srcport_str);
        }
        else
        {
            if ( GT_TRUE == tpm_set_mc_vid_key (src_port,
                                               mc_uni_xlate_mode,
                                               mc_uni_xlate_vid) )
            {
                printk(KERN_INFO "OK.\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_set_mc_vid_key failed\n", __FUNCTION__);
            }
        }
    }
}
void sfs_tpm_cfg_set_mc_vid_cfg_set    (const char *buf, size_t len)
{
    typedef enum
    {
        mcvidcfgset_owner=0, mcvidcfgset_mc_vid, mcvidcfgset_max
    } mcvidcfgset_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 mc_vid;
    int                      parsedargs;
    int                      numparms;

    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != mcvidcfgset_max)
    {
        parm_error_completion(numparms, mcvidcfgset_max, buf, sfs_help_igmp_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &mc_vid);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], mc_vid[%d]\n",
               len, parsedargs, ownerid, mc_vid);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = tpm_set_mc_vid_cfg (mc_vid)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK.\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_set_mc_vid_cfg failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

void sfs_tpm_cfg_send_genquery_to_uni  (const char *buf, size_t len)
{
    typedef enum
    {
        sendgenquerytouni_target_ports=0, sendgenquerytouni_packet_num, sendgenquerytouni_max
    } sendgenquerytouni_parm_indx_t;
    // shell line parsing
    uint32_t                 packet_num;
    uint32_t                 target_ports;
    int                      parsedargs;
    int                      numparms;
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != sendgenquerytouni_max)
    {
        parm_error_completion(numparms, sendgenquerytouni_max, buf, sfs_help_send_genquery_to_uni);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "0x%x %d ", &target_ports, &packet_num);
        printk(KERN_INFO "len=%d, parsedargs=%d. target_ports[0x%x]. packet_num[%d]\n",
               len, parsedargs, target_ports, packet_num);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
        	rc = tpm_proc_send_genquery_to_uni(target_ports, packet_num);
            if (TPM_RC_OK == rc)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_send_genquery_to_uni failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_gen_5t_rule_bounce(uint32_t owner_id,
										tpm_dir_t src_dir,
										uint32_t rule_num,
										uint32_t *rule_idx,
										tpm_parse_fields_t parse_rule_bm,
										tpm_parse_flags_t parse_flags_bm,
										tpm_l4_ports_key_t *l4_key,
										tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
										tpm_pkt_frwd_t *pkt_frwd,
										tpm_pkt_mod_t *pkt_mod,
										tpm_pkt_mod_bm_t pkt_mod_bm,
										tpm_rule_action_t *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd      = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd    = MV_TPM_IOCTL_ADD_IPV6_GEN_5T_RULE;
    tpm_add_acl_rule->owner_id       = owner_id;
    tpm_add_acl_rule->src_dir        = src_dir;
    tpm_add_acl_rule->rule_num       = rule_num;
    tpm_add_acl_rule->parse_rule_bm  = parse_rule_bm;
    tpm_add_acl_rule->ipv6_gen_5t_rule.parse_flags_bm = parse_flags_bm;
    tpm_add_acl_rule->ipv6_gen_5t_rule.pkt_mod_bm     = pkt_mod_bm;

	memcpy(&tpm_add_acl_rule->ipv6_gen_5t_rule.l4_key, l4_key, sizeof(tpm_l4_ports_key_t));
	memcpy(&tpm_add_acl_rule->ipv6_gen_5t_rule.ipv6_gen_key, ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));
	memcpy(&tpm_add_acl_rule->ipv6_gen_5t_rule.pkt_frwd, pkt_frwd, sizeof(tpm_pkt_frwd_t));
	memcpy(&tpm_add_acl_rule->ipv6_gen_5t_rule.pkt_mod, pkt_mod, sizeof(tpm_pkt_mod_t));
	memcpy(&tpm_add_acl_rule->ipv6_gen_5t_rule.rule_action, rule_action, sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_gen_5t_rule tpm_add_ipv6_gen_5t_rule_bounce
#else
 #define _tpm_add_ipv6_gen_5t_rule tpm_add_ipv6_gen_5t_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_gen_rule_add
*
* DESCRIPTION:
*           This function create IPv6 general rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_gen_5t_rule_add(const char *buf, size_t len)
{
    typedef enum {
        ipv6ruleadd_owner=0, ipv6ruleadd_srcdir,    ipv6ruleadd_rulenum,  ipv6ruleadd_parserulebm,   ipv6ruleadd_parseflagsbm,
        ipv6ruleadd_action,  ipv6ruleadd_nextphase, ipv6ruleadd_modbm,    ipv6ruleadd_ipv6l4keyname, ipv6ruleadd_ipv6genkeyname,
        ipv6ruleadd_frwdname,ipv6ruleadd_modname,   ipv6ruleadd_max
    } ipv4ruleadd_parm_indx_t;

    uint32_t            owner_id;
    tpm_dir_t           src_dir;
    uint32_t            rule_num;
    uint32_t            rule_idx;
    tpm_parse_fields_t  parse_rule_bm;
    tpm_parse_flags_t   parse_flags_bm;
    tpm_l4_ports_key_t     l4_key;
    tpm_ipv6_gen_acl_key_t ipv6_gen_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_pkt_mod_t       pkt_mod;
    tpm_pkt_mod_bm_t    pkt_mod_bm;
    tpm_rule_action_t   rule_action;

    tpmcfg_ipv6_l4_ports_key_entry_t *pdbipv6l4keyentry  = 0;
    tpmcfg_ipv6_gen_key_entry_t      *pdbipv6genkeyentry = 0;
    tpmcfg_frwd_entry_t              *pdbfrwdentry       = 0;
    tpmcfg_mod_entry_t               *pdbmodentry        = 0;

    tpm_error_code_t    rc;
    int                 indx;
    int                 parsedargs;
    int                 numparms;
    uint32_t            action;
    uint32_t            nextphase;
    char                srcdir_str[20];
    char                uc_srcdir_str[20];
    char                nextphase_str[20];
    char                ipv6l4keyname[30];
    char                ipv6genkeyname[30];
    char                frwdname[20];
    char                modname[20];


    numparms = count_parameters(buf);
    if (numparms != ipv6ruleadd_max)
        parm_error_completion(numparms, ipv6ruleadd_max, buf, sfs_help_ipv6_gen_5t_rule_add);
    else {
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s 0x%x %s %s %s %s",
                            &owner_id, srcdir_str, &rule_num, &parse_rule_bm, &parse_flags_bm,
                            &action, nextphase_str, &pkt_mod_bm, ipv6l4keyname, ipv6genkeyname, frwdname, modname);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], modbm[0x%x], ipv6l4keyname[%s], ipv6genkeyname[%s], frwdname[%s], modname[%s]\n",
               len, parsedargs, owner_id, srcdir_str, rule_num, parse_rule_bm, parse_flags_bm, action, nextphase_str, pkt_mod_bm, ipv6l4keyname, ipv6genkeyname, frwdname, modname);

        for (indx = 0; indx < strlen(srcdir_str); indx++)
			uc_srcdir_str[indx] = (char)toupper(srcdir_str[indx]);

        uc_srcdir_str[strlen(srcdir_str)] = 0;

        if (parsedargs != numparms)
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        else if (get_srcdir_value(uc_srcdir_str, &src_dir) == GT_FALSE)
            printk(KERN_INFO "Invalid src_port[%s]\n", srcdir_str);
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        else if (((pdbipv6l4keyentry = find_tpm_ipv6_l4_ports_key_entry_by_name(ipv6l4keyname)) == 0) && \
                    strcmp(ipv6l4keyname, ipv6_l4_key_empty_name) != 0)
            printk(KERN_INFO "L4 ports key entry [%s] not found\n", ipv6genkeyname);
        else if (((pdbipv6genkeyentry = find_tpm_ipv6_gen_key_entry_by_name(ipv6genkeyname)) == 0) && \
                    strcmp(ipv6genkeyname, ipv6_gen_key_empty_name) != 0)
            printk(KERN_INFO "IPV6 gen key entry [%s] not found\n", ipv6genkeyname);
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && \
                    strcmp(frwdname, frwd_empty_name) != 0)
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && \
                    strcmp(modname, mod_empty_name) != 0)
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        else {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbipv6l4keyentry != 0)
                memcpy(&l4_key, &pdbipv6l4keyentry->l4_ports, sizeof(tpm_l4_ports_key_t));
            else
                memset(&l4_key, 0, sizeof(tpm_l4_ports_key_t));

            if (pdbipv6genkeyentry != 0)
                memcpy(&ipv6_gen_key, &pdbipv6genkeyentry->ipv6gen_acl, sizeof(tpm_ipv6_gen_acl_key_t));
            else
                memset(&ipv6_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));

            if (pdbfrwdentry != 0)
                memcpy(&pkt_frwd, &pdbfrwdentry->frwd, sizeof(tpm_pkt_frwd_t));
            else
                memset(&pkt_frwd, 0, sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)
                memcpy(&pkt_mod, &pdbmodentry->mod, sizeof(tpm_pkt_mod_t));
            else
                memset(&pkt_mod, 0, sizeof(tpm_pkt_mod_t));

            if (TPM_RC_OK == (rc = _tpm_add_ipv6_gen_5t_rule(owner_id,
                                                            src_dir,
                                                            rule_num,
                                                            &rule_idx,
                                                            parse_rule_bm,
                                                            parse_flags_bm,
                                                            &l4_key,
                                                            &ipv6_gen_key,
                                                            &pkt_frwd,
                                                            &pkt_mod,
                                                            pkt_mod_bm,
                                                            &rule_action)) )
                PR_RULE_IDX(rule_idx)
            else
                printk(KERN_INFO "%s: tpm_add_ipv6_gen_5t_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_gen_5t_rule_bounce(uint32_t        owner_id,
                                        		 uint32_t        rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_IPV6_GEN_5T_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_gen_5t_rule tpm_del_ipv6_gen_5t_rule_bounce
#else
 #define _tpm_del_ipv6_gen_5t_rule tpm_del_ipv6_gen_5t_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_gen_5t
*
* DESCRIPTION:
*           This function deletes a IPV6 HW gen ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_gen_5t(const char *buf, size_t len)
{
    typedef enum {
        noipv6gen_owner=0, noipv6gen_ruleidx, noipv6gen_max
    } noipv6genruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noipv6gen_max)
        parm_error_completion(numparms, noipv6gen_max, buf, sfs_help_no_rule_add);
    else {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
               len, parsedargs, ownerid, rule_idx);


        if (parsedargs != numparms)
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        else {
            if ((rc = _tpm_del_ipv6_gen_5t_rule(ownerid, rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
                printk(KERN_INFO "%s: tpm_del_ipv6_gen_5t_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_dip_5t_rule_bounce(uint32_t owner_id,
												tpm_dir_t src_dir,
												uint32_t rule_num,
												uint32_t *rule_idx,
												tpm_parse_fields_t parse_rule_bm,
												tpm_parse_flags_t parse_flags_bm,
												tpm_l4_ports_key_t *l4_key,
												tpm_ipv6_gen_acl_key_t *ipv6_gen_key,
												tpm_ipv6_addr_key_t *ipv6_dip_key,
												tpm_pkt_frwd_t *pkt_frwd,
												tpm_pkt_mod_t *pkt_mod,
												tpm_pkt_mod_bm_t pkt_mod_bm,
												tpm_rule_action_t *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd      = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd    = MV_TPM_IOCTL_ADD_IPV6_DIP_5T_RULE;
    tpm_add_acl_rule->owner_id       = owner_id;
    tpm_add_acl_rule->src_dir        = src_dir;
    tpm_add_acl_rule->rule_num       = rule_num;
    tpm_add_acl_rule->parse_rule_bm  = parse_rule_bm;
    tpm_add_acl_rule->ipv6_dip_5t_rule.parse_flags_bm = parse_flags_bm;
    tpm_add_acl_rule->ipv6_dip_5t_rule.pkt_mod_bm     = pkt_mod_bm;

	memcpy(&tpm_add_acl_rule->ipv6_dip_5t_rule.l4_key, l4_key, sizeof(tpm_l4_ports_key_t));
	memcpy(&tpm_add_acl_rule->ipv6_dip_5t_rule.ipv6_gen_key, ipv6_gen_key, sizeof(tpm_ipv6_gen_acl_key_t));
	memcpy(&tpm_add_acl_rule->ipv6_dip_5t_rule.ipv6_dip_key, ipv6_dip_key, sizeof(tpm_ipv6_addr_key_t));
	memcpy(&tpm_add_acl_rule->ipv6_dip_5t_rule.pkt_frwd, pkt_frwd, sizeof(tpm_pkt_frwd_t));
	memcpy(&tpm_add_acl_rule->ipv6_dip_5t_rule.pkt_mod, pkt_mod, sizeof(tpm_pkt_mod_t));
	memcpy(&tpm_add_acl_rule->ipv6_dip_5t_rule.rule_action, rule_action, sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_dip_5t_rule tpm_add_ipv6_dip_5t_rule_bounce
#else
 #define _tpm_add_ipv6_dip_5t_rule tpm_add_ipv6_dip_5t_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_dip_5t_rule_add
*
* DESCRIPTION:
*           This function create IPv6 general rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_dip_5t_rule_add(const char *buf, size_t len)
{
    typedef enum {
        ipv6ruleadd_owner=0,        ipv6ruleadd_srcdir,    ipv6ruleadd_rulenum,  ipv6ruleadd_parserulebm,   ipv6ruleadd_parseflagsbm,
        ipv6ruleadd_action,         ipv6ruleadd_nextphase, ipv6ruleadd_modbm,    ipv6ruleadd_ipv6l4keyname, ipv6ruleadd_ipv6genkeyname,
        ipv6ruleadd_ipv6dipkeyname, ipv6ruleadd_frwdname,ipv6ruleadd_modname,   ipv6ruleadd_max
    } ipv4ruleadd_parm_indx_t;

    uint32_t            owner_id;
    tpm_dir_t           src_dir;
    uint32_t            rule_num;
    uint32_t            rule_idx;
    tpm_parse_fields_t  parse_rule_bm;
    tpm_parse_flags_t   parse_flags_bm;
    tpm_l4_ports_key_t     l4_key;
    tpm_ipv6_gen_acl_key_t ipv6_gen_key;
    tpm_ipv6_addr_key_t    ipv6_dip_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_pkt_mod_t       pkt_mod;
    tpm_pkt_mod_bm_t    pkt_mod_bm;
    tpm_rule_action_t   rule_action;

    tpmcfg_ipv6_l4_ports_key_entry_t *pdbipv6l4keyentry  = 0;
    tpmcfg_ipv6_gen_key_entry_t      *pdbipv6genkeyentry = 0;
    tpmcfg_ipv6_dip_key_entry_t      *pdbipv6dipkeyentry = 0;
    tpmcfg_frwd_entry_t              *pdbfrwdentry       = 0;
    tpmcfg_mod_entry_t               *pdbmodentry        = 0;

    tpm_error_code_t    rc;
    int                 indx;
    int                 parsedargs;
    int                 numparms;
    uint32_t            action;
    uint32_t            nextphase;
    char                srcdir_str[20];
    char                uc_srcdir_str[20];
    char                nextphase_str[20];
    char                ipv6l4keyname[30];
    char                ipv6genkeyname[30];
    char                ipv6dipkeyname[30];
    char                frwdname[20];
    char                modname[20];


    numparms = count_parameters(buf);
    if (numparms != ipv6ruleadd_max)
        parm_error_completion(numparms, ipv6ruleadd_max, buf, sfs_help_ipv6_dip_5t_rule_add);
    else {
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s 0x%x %s %s %s %s %s",
                            &owner_id, srcdir_str, &rule_num, &parse_rule_bm, &parse_flags_bm,
                            &action, nextphase_str, &pkt_mod_bm, ipv6l4keyname, ipv6genkeyname,
                            ipv6dipkeyname, frwdname, modname);
        printk("len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], "\
			"parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], modbm[0x%x], "\
			"ipv6l4keyname[%s], ipv6genkeyname[%s], ipv6dipkeyname[%s], frwdname[%s], modname[%s]\n",
            len, parsedargs, owner_id, srcdir_str, rule_num,
            parse_rule_bm, parse_flags_bm, action, nextphase_str, pkt_mod_bm,
            ipv6l4keyname, ipv6genkeyname, ipv6dipkeyname, frwdname, modname);

        for (indx = 0; indx < strlen(srcdir_str); indx++)
			uc_srcdir_str[indx] = (char)toupper(srcdir_str[indx]);

        uc_srcdir_str[strlen(srcdir_str)] = 0;

        if (parsedargs != numparms)
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        else if (get_srcdir_value(uc_srcdir_str, &src_dir) == GT_FALSE)
            printk(KERN_INFO "Invalid src_port[%s]\n", srcdir_str);
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        else if (((pdbipv6l4keyentry = find_tpm_ipv6_l4_ports_key_entry_by_name(ipv6l4keyname)) == 0) && \
                    strcmp(ipv6l4keyname, ipv6_l4_key_empty_name) != 0)
            printk(KERN_INFO "L4 ports key entry [%s] not found\n", ipv6l4keyname);
        else if (((pdbipv6genkeyentry = find_tpm_ipv6_gen_key_entry_by_name(ipv6genkeyname)) == 0) && \
                    strcmp(ipv6genkeyname, ipv6_gen_key_empty_name) != 0)
            printk(KERN_INFO "IPV6 gen key entry [%s] not found\n", ipv6genkeyname);
        else if (((pdbipv6dipkeyentry = find_tpm_ipv6_dip_key_entry_by_name(ipv6dipkeyname)) == 0) && \
                    strcmp(ipv6dipkeyname, ipv6_dip_key_empty_name) != 0)
            printk(KERN_INFO "IPV6 dip key entry [%s] not found\n", ipv6dipkeyname);
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && \
                    strcmp(frwdname, frwd_empty_name) != 0)
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && \
                    strcmp(modname, mod_empty_name) != 0)
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        else {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbipv6l4keyentry != 0)
                memcpy(&l4_key, &pdbipv6l4keyentry->l4_ports, sizeof(tpm_l4_ports_key_t));
            else
                memset(&l4_key, 0, sizeof(tpm_l4_ports_key_t));

            if (pdbipv6genkeyentry != 0)
                memcpy(&ipv6_gen_key, &pdbipv6genkeyentry->ipv6gen_acl, sizeof(tpm_ipv6_gen_acl_key_t));
            else
                memset(&ipv6_gen_key, 0, sizeof(tpm_ipv6_gen_acl_key_t));

            if (pdbipv6dipkeyentry != 0)
                memcpy(&ipv6_dip_key, &pdbipv6dipkeyentry->ipv6_addr, sizeof(tpm_ipv6_addr_key_t));
            else
                memset(&ipv6_dip_key, 0, sizeof(tpm_ipv6_addr_key_t));

            if (pdbfrwdentry != 0)
                memcpy(&pkt_frwd, &pdbfrwdentry->frwd, sizeof(tpm_pkt_frwd_t));
            else
                memset(&pkt_frwd, 0, sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)
                memcpy(&pkt_mod, &pdbmodentry->mod, sizeof(tpm_pkt_mod_t));
            else
                memset(&pkt_mod, 0, sizeof(tpm_pkt_mod_t));

            if (TPM_RC_OK == (rc = _tpm_add_ipv6_dip_5t_rule(owner_id,
                                                            src_dir,
                                                            rule_num,
                                                            &rule_idx,
                                                            parse_rule_bm,
                                                            parse_flags_bm,
                                                            &l4_key,
                                                            &ipv6_gen_key,
                                                            &ipv6_dip_key,
                                                            &pkt_frwd,
                                                            &pkt_mod,
                                                            pkt_mod_bm,
                                                            &rule_action)) )
                PR_RULE_IDX(rule_idx)
            else
                printk(KERN_INFO "%s: tpm_add_ipv6_dip_5t_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_dip_5t_rule_bounce(uint32_t        owner_id,
                                        		 uint32_t        rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_IPV6_DIP_5T_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_dip_5t_rule tpm_del_ipv6_dip_5t_rule_bounce
#else
 #define _tpm_del_ipv6_dip_5t_rule tpm_del_ipv6_dip_5t_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_dip_5t
*
* DESCRIPTION:
*           This function deletes a IPV6 HW gen ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_dip_5t(const char *buf, size_t len)
{
    typedef enum {
        noipv6dip_owner=0, noipv6dip_ruleidx, noipv6dip_max
    } noipv6dipruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noipv6dip_max)
        parm_error_completion(numparms, noipv6dip_max, buf, sfs_help_no_rule_add);
    else {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
               len, parsedargs, ownerid, rule_idx);


        if (parsedargs != numparms)
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        else {
            if ((rc = _tpm_del_ipv6_dip_5t_rule(ownerid, rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
                printk(KERN_INFO "%s: tpm_del_ipv6_dip_5t_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ipv6_l4_ports_5t_rule_bounce(uint32_t owner_id,
													tpm_dir_t src_dir,
													uint32_t rule_num,
													uint32_t *rule_idx,
													tpm_parse_fields_t parse_rule_bm,
													tpm_parse_flags_t parse_flags_bm,
													tpm_l4_ports_key_t *l4_key,
													tpm_pkt_frwd_t *pkt_frwd,
													tpm_pkt_mod_t *pkt_mod,
													tpm_pkt_mod_bm_t pkt_mod_bm,
													tpm_rule_action_t *rule_action)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd      = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd    = MV_TPM_IOCTL_ADD_IPV6_L4_PORTS_5T_RULE;
    tpm_add_acl_rule->owner_id       = owner_id;
    tpm_add_acl_rule->src_dir        = src_dir;
    tpm_add_acl_rule->rule_num       = rule_num;
    tpm_add_acl_rule->parse_rule_bm  = parse_rule_bm;
    tpm_add_acl_rule->ipv6_l4_ports_5t_rule.parse_flags_bm = parse_flags_bm;
    tpm_add_acl_rule->ipv6_l4_ports_5t_rule.pkt_mod_bm     = pkt_mod_bm;

	memcpy(&tpm_add_acl_rule->ipv6_l4_ports_5t_rule.l4_key, l4_key, sizeof(tpm_l4_ports_key_t));
	memcpy(&tpm_add_acl_rule->ipv6_l4_ports_5t_rule.pkt_frwd, pkt_frwd, sizeof(tpm_pkt_frwd_t));
	memcpy(&tpm_add_acl_rule->ipv6_l4_ports_5t_rule.pkt_mod, pkt_mod, sizeof(tpm_pkt_mod_t));
	memcpy(&tpm_add_acl_rule->ipv6_l4_ports_5t_rule.rule_action, rule_action, sizeof(tpm_rule_action_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ipv6_l4_ports_5t_rule tpm_add_ipv6_l4_ports_5t_rule_bounce
#else
 #define _tpm_add_ipv6_l4_ports_5t_rule tpm_add_ipv6_l4_ports_5t_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_l4_ports_5t_rule_add
*
* DESCRIPTION:
*           This function create IPv6 general rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_l4_ports_5t_rule_add(const char *buf, size_t len)
{
    typedef enum {
        ipv6ruleadd_owner=0, ipv6ruleadd_srcdir,    ipv6ruleadd_rulenum,  ipv6ruleadd_parserulebm,   ipv6ruleadd_parseflagsbm,
        ipv6ruleadd_action,  ipv6ruleadd_nextphase, ipv6ruleadd_modbm,    ipv6ruleadd_ipv6l4keyname, ipv6ruleadd_frwdname,
        ipv6ruleadd_modname, ipv6ruleadd_max
    } ipv4ruleadd_parm_indx_t;

    uint32_t            owner_id;
    tpm_dir_t           src_dir;
    uint32_t            rule_num;
    uint32_t            rule_idx;
    tpm_parse_fields_t  parse_rule_bm;
    tpm_parse_flags_t   parse_flags_bm;
    tpm_l4_ports_key_t  l4_key;
    tpm_pkt_frwd_t      pkt_frwd;
    tpm_pkt_mod_t       pkt_mod;
    tpm_pkt_mod_bm_t    pkt_mod_bm;
    tpm_rule_action_t   rule_action;

    tpmcfg_ipv6_l4_ports_key_entry_t *pdbipv6l4keyentry  = 0;
    tpmcfg_frwd_entry_t              *pdbfrwdentry       = 0;
    tpmcfg_mod_entry_t               *pdbmodentry        = 0;

    tpm_error_code_t    rc;
    int                 indx;
    int                 parsedargs;
    int                 numparms;
    uint32_t            action;
    uint32_t            nextphase;
    char                srcdir_str[20];
    char                uc_srcdir_str[20];
    char                nextphase_str[20];
    char                ipv6l4keyname[30];
    char                frwdname[20];
    char                modname[20];


    numparms = count_parameters(buf);
    if (numparms != ipv6ruleadd_max)
        parm_error_completion(numparms, ipv6ruleadd_max, buf, sfs_help_ipv6_l4_ports_5t_rule_add);
    else {
        parsedargs = sscanf(buf, "%d %s %d 0x%x 0x%x 0x%x %s 0x%x %s %s %s",
                            &owner_id, srcdir_str, &rule_num, &parse_rule_bm, &parse_flags_bm,
                            &action, nextphase_str, &pkt_mod_bm, ipv6l4keyname, frwdname, modname);
        printk("len=%d, parsedargs=%d. ownerid[%d], srcport_str[%s], rulenum[%d], "\
			"parserulebm[0x%x], parseflagsbm[0x%x], action[0x%x], nextphase_str[%s], "\
			"modbm[0x%x], ipv6l4keyname[%s], frwdname[%s], modname[%s]\n",
            len, parsedargs, owner_id, srcdir_str, rule_num,
            parse_rule_bm, parse_flags_bm, action, nextphase_str,
            pkt_mod_bm, ipv6l4keyname, frwdname, modname);

        for (indx = 0; indx < strlen(srcdir_str); indx++)
			uc_srcdir_str[indx] = (char)toupper(srcdir_str[indx]);

        uc_srcdir_str[strlen(srcdir_str)] = 0;

        if (parsedargs != numparms)
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        else if (get_srcdir_value(uc_srcdir_str, &src_dir) == GT_FALSE)
            printk(KERN_INFO "Invalid src_port[%s]\n", srcdir_str);
        else if (get_phase_value(nextphase_str, &nextphase) == GT_FALSE)
            printk(KERN_INFO "Invalid next_phase[%s]\n", nextphase_str);
        else if (((pdbipv6l4keyentry = find_tpm_ipv6_l4_ports_key_entry_by_name(ipv6l4keyname)) == 0) && \
                    strcmp(ipv6l4keyname, ipv6_l4_key_empty_name) != 0)
            printk(KERN_INFO "L4 ports key entry [%s] not found\n", ipv6l4keyname);
        else if (((pdbfrwdentry = find_tpm_pkt_frwd_entry_by_name(frwdname)) == 0) && \
                    strcmp(frwdname, frwd_empty_name) != 0)
            printk(KERN_INFO "frwd entry [%s] not found\n", frwdname);
        else if (((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0) && \
                    strcmp(modname, mod_empty_name) != 0)
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        else {
            rule_action.next_phase = nextphase;
            rule_action.pkt_act    = action;

            if (pdbipv6l4keyentry != 0)
                memcpy(&l4_key, &pdbipv6l4keyentry->l4_ports, sizeof(tpm_l4_ports_key_t));
            else
                memset(&l4_key, 0, sizeof(tpm_l4_ports_key_t));

            if (pdbfrwdentry != 0)
                memcpy(&pkt_frwd, &pdbfrwdentry->frwd, sizeof(tpm_pkt_frwd_t));
            else
                memset(&pkt_frwd, 0, sizeof(tpm_pkt_frwd_t));

            if (pdbmodentry != 0)
                memcpy(&pkt_mod, &pdbmodentry->mod, sizeof(tpm_pkt_mod_t));
            else
                memset(&pkt_mod, 0, sizeof(tpm_pkt_mod_t));

            if (TPM_RC_OK == (rc = _tpm_add_ipv6_l4_ports_5t_rule(owner_id,
	                                                            src_dir,
	                                                            rule_num,
	                                                            &rule_idx,
	                                                            parse_rule_bm,
	                                                            parse_flags_bm,
	                                                            &l4_key,
	                                                            &pkt_frwd,
	                                                            &pkt_mod,
	                                                            pkt_mod_bm,
	                                                            &rule_action)) )
                PR_RULE_IDX(rule_idx)
            else
                printk(KERN_INFO "%s: tpm_add_ipv6_l4_ports_5t_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ipv6_l4_ports_5t_rule_bounce(uint32_t        owner_id,
                                        		      uint32_t        rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_IPV6_DIP_5T_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ipv6_l4_ports_5t_rule tpm_del_ipv6_l4_ports_5t_rule_bounce
#else
 #define _tpm_del_ipv6_l4_ports_5t_rule tpm_del_ipv6_l4_ports_5t_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_5t
*
* DESCRIPTION:
*           This function deletes a IPV6 HW gen ACL rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ipv6_l4_ports_5t(const char *buf, size_t len)
{
    typedef enum {
        noipv6l4_owner=0, noipv6l4_ruleidx, noipv6l4_max
    } noipv6l4ruleadd_parm_indx_t;
    // shell line parsing
    unsigned int             ownerid;
    uint32_t                 rule_idx;
    int                      parsedargs;
    int                      numparms;
    // DB
    // Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != noipv6l4_max)
        parm_error_completion(numparms, noipv6l4_max, buf, sfs_help_no_rule_add);
    else {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
               len, parsedargs, ownerid, rule_idx);


        if (parsedargs != numparms)
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        else {
            if ((rc = _tpm_del_ipv6_l4_ports_5t_rule(ownerid, rule_idx)) == TPM_RC_OK)
                PR_RESULT
            else
                printk(KERN_INFO "%s: tpm_del_ipv6_l4_ports_5t_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
        }
    }
}

/*tpm self check*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_rule_self_check_bounce(uint32_t owner_id,uint32_t level)
{
    tpm_ioctl_tpm_check_t *tpm_check_param = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_check_param;

    tpm_sfs_2_ioctl_command.cmd = MV_TPM_IOCTL_TPM_CHECK_SECTION;
    tpm_check_param->owner_id = owner_id;
    tpm_check_param->check_level = level;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_rule_self_check tpm_rule_self_check_bounce
#else
 #define _tpm_rule_self_check tpm_rule_self_check
#endif


/*******************************************************************************
* sfs_tpm_rule_self_check
*
* DESCRIPTION:
*           This function creates does a tpm rule check
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_rule_self_check (const char *buf, size_t len)
{
	typedef enum
	{
		tpm_self_check_owner=0, tpm_self_check_level, tpm_self_check_max
	} tpm_check_parm_indx_t;
	// shell line parsing
	uint32_t                 ownerid;
	uint32_t                 level;
	int                      parsedargs;
	int                      numparms;
	//Used in API call
	tpm_error_code_t         rc;

	numparms = count_parameters(buf);
	if (numparms != tpm_self_check_max) {
		parm_error_completion(numparms, tpm_self_check_max, buf, sfs_help_mib_reset);
	} else {
		// Get parameters
		parsedargs = sscanf(buf, "%d %d", &ownerid, &level);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else if (level > TPM_ENUM_MAX_CHECK_LEVEL) {
			printk(KERN_INFO "Invalid level[%d]\n", level);
		} else {
			if ((rc = _tpm_rule_self_check(ownerid, level)) != TPM_RC_OK) {
				printk(KERN_INFO "%s: tpm_rule_self_check failed, rc[%d] - %s\n",
				       __FUNCTION__, rc, get_tpm_err_str(rc));
			}
		}
	}
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_flush_vtu_bounce(uint32_t     owner_id)
{
    tpm_ioctl_flush_vtu_t *tpm_flush_vtu = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_flush_vtu;

    tpm_sfs_2_ioctl_command.cmd   = MV_TPM_IOCTL_FLUSH_VTU_SECTION;
    tpm_flush_vtu->owner_id       = owner_id;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_flush_vtu tpm_flush_vtu_bounce
#else
 #define _tpm_flush_vtu tpm_flush_vtu
#endif


/*******************************************************************************
* sfs_tpm_cfg_flush_vtu
*
* DESCRIPTION:
*           This function flush vtu of the switch
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_flush_vtu  (const char *buf, size_t len)
{
    typedef enum
    {
        flush_vtu_owner=0,  flush_vtu_max
    } flush_vtu_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != flush_vtu_max)
    {
        parm_error_completion(numparms, flush_vtu_max, buf, sfs_help_flush_vtu);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d", &ownerid);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d]\n", len, parsedargs, ownerid);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else
        {
            if ((rc = _tpm_flush_vtu(ownerid)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_flush_vtu failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_flush_atu_bounce(uint32_t owner_id, tpm_flush_atu_type_t flush_type, uint16_t db_num)
{
    tpm_ioctl_flush_atu_t *tpm_flush_atu = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_flush_atu;

    tpm_sfs_2_ioctl_command.cmd         = MV_TPM_IOCTL_FLUSH_ATU_SECTION;
    tpm_flush_atu->owner_id       = owner_id;
    tpm_flush_atu->flush_type     = flush_type;
    tpm_flush_atu->db_num         = db_num;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_flush_atu tpm_flush_atu_bounce
#else
 #define _tpm_flush_atu tpm_flush_atu
#endif


/*******************************************************************************
* sfs_tpm_cfg_flush_atu
*
* DESCRIPTION:
*           This function flush atu of the switch
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_flush_atu  (const char *buf, size_t len)
{
    typedef enum
    {
        flush_atu_owner=0, flush_atu_type, flush_atu_dbnum, flush_atu_max
    } flush_atu_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 flush_type;
    uint32_t                 db_num;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != flush_atu_max)
    {
        parm_error_completion(numparms, flush_atu_max, buf, sfs_help_flush_atu);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d %d", &ownerid, &flush_type, &db_num);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], flush_type[%d], db_num[%d]\n",
//                          len, parsedargs, ownerid, flush_type, db_num);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (flush_type > TPM_FLUSH_ATU_DYNAMIC)
        {
            printk(KERN_INFO "Invalid flush_type[%d]\n", flush_type);
        }
        else
        {
            if ((rc = _tpm_flush_atu(ownerid, flush_type, db_num)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_flush_atu failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_ctc_cm_set_ipv6_parse_win_bounce(uint32_t owner_id, tpm_ctc_cm_ipv6_parse_win_t ipv6_parse_window)
{
    tpm_ioctl_ipv6_parse_window_t *ipv6_parse_window = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ipv6_parse_window;

    tpm_sfs_2_ioctl_command.cmd           = MV_TPM_IOCTL_SET_IPV6_CM_PARSE_WIN_SECTION;
    ipv6_parse_window->owner_id           = owner_id;
    ipv6_parse_window->ipv6_parse_window  = ipv6_parse_window;
    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_ctc_cm_set_ipv6_parse_win tpm_ctc_cm_set_ipv6_parse_win_bounce
#else
 #define _tpm_ctc_cm_set_ipv6_parse_win tpm_ctc_cm_set_ipv6_parse_win
#endif
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_mac_learn_rule_bounce(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_ADD_MAC_LEARN_ACL_RULE;
    tpm_add_acl_rule->owner_id                      = owner_id;
    memcpy(&(tpm_add_acl_rule->l2_acl_rule.l2_key), (void*)src_mac_addr, sizeof(tpm_l2_acl_key_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_mac_learn_rule tpm_add_mac_learn_rule_bounce
#else
 #define _tpm_add_mac_learn_rule tpm_add_mac_learn_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_ctc_cm_set_ipv6_parse_win
*
* DESCRIPTION:
*           This function set_ipv6_parse_win
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_ctc_cm_set_ipv6_parse_win  (const char *buf, size_t len)
{
    typedef enum
    {
        ctc_cm_set_ipv6_parse_win_owner=0, ctc_cm_set_ipv6_parse_win, ctc_cm_set_ipv6_parse_win_max
    } ctc_cm_set_ipv6_parse_win_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 ipv6_parse_win;
    int                      parsedargs;
    int                      numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ctc_cm_set_ipv6_parse_win_max)
    {
        parm_error_completion(numparms, ctc_cm_set_ipv6_parse_win_max, buf, sfs_help_ctc_cm_set_ipv6_parse_win);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d", &ownerid, &ipv6_parse_win);
//        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], ipv6_parse_win[%d]\n",
//                          len, parsedargs, ownerid, db_num);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (ipv6_parse_win > TPM_CTC_CM_IPv6_SECOND_24B)
        {
            printk(KERN_INFO "Invalid ipv6_parse_win[%d]\n", ipv6_parse_win);
        }
        else
        {
            if ((rc = _tpm_ctc_cm_set_ipv6_parse_win(ownerid, (tpm_ctc_cm_ipv6_parse_win_t)ipv6_parse_win)) == TPM_RC_OK)
            {
                printk(KERN_INFO "OK\n");
            }
            else
            {
                printk(KERN_INFO "%s: tpm_ctc_cm_set_ipv6_parse_win failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}
/*******************************************************************************
* sfs_tpm_cfg_set_mac_learn_rule_add
*
* DESCRIPTION:
*           This function creates a L2 mac learn rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mac_learn_rule_add(const char *buf, size_t len)
{
	typedef enum
	{
		mac_learn_add_owner=0, mac_learn_add_l2keyname, mac_learn_add_max
	} mac_learn_add_parm_indx_t;
	/* shell line parsing */
	uint32_t ownerid;
	char l2keyname[20];
	int parsedargs;
	int numparms;
	/* DB */
	tpmcfg_l2_key_entry_t *pdbl2keyentry = 0;

	/* Used in API call */
	tpm_l2_acl_key_t l2_acl;
	tpm_error_code_t rc;

	numparms = count_parameters(buf);
	if (numparms != mac_learn_add_max) {
		parm_error_completion(numparms, mac_learn_add_max, buf, sfs_help_mac_learn_rule_add);
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %s", &ownerid, l2keyname);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else if (((pdbl2keyentry = find_tpm_l2_key_entry_by_name(l2keyname)) == 0)) {
			printk(KERN_INFO "L2 MAC addr key entry [%s] not found\n", l2keyname);
			return;
		} else {
			memcpy(&l2_acl, &pdbl2keyentry->l2_acl, sizeof(tpm_l2_acl_key_t));
			if ((rc = _tpm_add_mac_learn_rule(ownerid, &l2_acl)) == TPM_RC_OK)
				printk("OK\n");
			else
				printk(KERN_INFO "%s: tpm_add_mac_learn_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
		}
	}
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_mac_learn_rule_bounce(uint32_t owner_id, tpm_l2_acl_key_t *src_mac_addr)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_MAC_LEARN_ACL_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    memcpy(&(tpm_del_acl_rule->l2_key), (void*)src_mac_addr, sizeof(tpm_l2_acl_key_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
#define _tpm_del_mac_learn_rule tpm_del_mac_learn_rule_bounce
#else
#define _tpm_del_mac_learn_rule tpm_del_mac_learn_rule
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_mac_learn
*
* DESCRIPTION:
*           This function deletes a L2 mac learn rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_mac_learn(const char *buf, size_t len)
{
	typedef enum
	{
		noruleadd_maclearn_owner=0, noruleadd_maclearn_l2keyname, noruleadd_maclearn_max
	} noruleadd_maclearn_parm_indx_t;
	/* shell line parsing */
	uint32_t ownerid;
	int parsedargs;
	int numparms;
	char l2keyname[20];
	tpmcfg_l2_key_entry_t *pdbl2keyentry = 0;
	/* Used in API call */
	tpm_l2_acl_key_t l2_acl;
	tpm_error_code_t rc;

	numparms = count_parameters(buf);
	if (numparms != noruleadd_maclearn_max) {
		parm_error_completion(numparms, noruleadd_maclearn_max, buf, sfs_help_no_rule_add);
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %s", &ownerid, l2keyname);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else if (((pdbl2keyentry = find_tpm_l2_key_entry_by_name(l2keyname)) == 0)) {
			printk(KERN_INFO "L2 MAC addr key entry [%s] not found\n", l2keyname);
			return;
		} else {
			memcpy(&l2_acl, &pdbl2keyentry->l2_acl, sizeof(tpm_l2_acl_key_t));
			if ((rc = _tpm_del_mac_learn_rule(ownerid, &l2_acl)) == TPM_RC_OK)
				printk(KERN_INFO "OK\n");
			else
				printk(KERN_INFO "%s: tpm_del_mac_learn_rule failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
		}
	}
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_mac_learn_default_rule_act_set_bounce(uint32_t owner_id, tpm_unknown_mac_conf_t mac_conf)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd   = MV_TPM_IOCTL_SET_MAC_LEARN_DEFAULT_ACTION;
    tpm_add_acl_rule->owner_id      = owner_id;
    tpm_add_acl_rule->l2_acl_rule.mac_conf = mac_conf;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
#define _tpm_mac_learn_default_rule_act_set tpm_mac_learn_default_rule_act_set_bounce
#else
#define _tpm_mac_learn_default_rule_act_set tpm_mac_learn_default_rule_act_set
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_mac_learn_default_rule_action
*
* DESCRIPTION:
*           This function set the action for mac learn default rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_mac_learn_default_rule_action(const char *buf, size_t len)
{
	typedef enum
	{
		default_rule_action_owner=0, default_rule_action_act, default_rule_action_max
	} default_rule_action_parm_indx_t;
	/* shell line parsing */
	uint32_t ownerid;
	int parsedargs;
	int numparms;
	tpm_unknown_mac_conf_t mac_conf;
	/* Used in API call */
	tpm_error_code_t rc;

	numparms = count_parameters(buf);
	if (numparms != default_rule_action_max) {
		parm_error_completion(numparms, default_rule_action_max, buf, sfs_help_mac_learn_def_act_set);
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %x", &ownerid, &mac_conf);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else {
			if ((rc = _tpm_mac_learn_default_rule_act_set(ownerid, mac_conf)) == TPM_RC_OK)
				printk(KERN_INFO "OK\n");
			else
				printk(KERN_INFO "%s: tpm_mac_learn_default_rule_act_set failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
		}
	}
}

/*******************************************************************************
* sfs_tpm_cfg_set_fc
*
* DESCRIPTION:
*           This function configures SW flow control
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_fc(const char *buf, size_t len)
{
	typedef enum {
		fc_thresh_high = 0,
		fc_thresh_low,
		fc_port,
		fc_tgt_port,
		fc_tx_port,
		fc_tx_queue,
		fc_max
	} fc_parm_indx_t;
	/* shell line parsing */
	tpm_fc_cfg_t cfg;
	int parsedargs;
	int numparms;

	numparms = count_parameters(buf);
	if (numparms != fc_max)
		parm_error_completion(numparms, fc_max, buf, sfs_help_rate_limit);
	else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d %d %d %d %d %d",
				    &cfg.thresh_high, &cfg.thresh_low,
				    &cfg.port, &cfg.tx_port, &cfg.tx_queue, &cfg.tgt_port);

		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else {
			tpm_fc_set_config(&cfg);
			printk(KERN_INFO "OK\n");

		}
	}
}

/*******************************************************************************
* sfs_tpm_cfg_set_fc_period
*
* DESCRIPTION:
*           This function configures SW flow control engine minitoring period
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_fc_period(const char *buf, size_t len)
{
	typedef enum {
		fc_period = 0,
		fc_max
	} fc_parm_indx_t;
	/* shell line parsing */
	uint32_t period;
	int parsedargs;
	int numparms;
	MV_STATUS ret_code;

	numparms = count_parameters(buf);
	if (numparms != fc_max)
		parm_error_completion(numparms, fc_max, buf, sfs_help_rate_limit);
	else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &period);

		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else {
			ret_code = tpm_fc_set_period(period);
			if (ret_code != MV_OK)
				printk(KERN_INFO "%s: tpm_fc_set_period() failed!\n", __func__);
			else
				printk(KERN_INFO "OK\n");
		}
	}
}

#ifdef TPM_FC_DEBUG
/*******************************************************************************
* sfs_tpm_cfg_set_fc_oneshot
*
* DESCRIPTION:
*   		This function configures SW flow control engine minitoring
*   		oneshot count for statistics
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_fc_oneshot(const char *buf, size_t len)
{
	typedef enum {
		fc_oneshot = 0,
		fc_max
	} fc_parm_indx_t;
	/* shell line parsing */
	uint32_t oneshot;
	int parsedargs;
	int numparms;
	MV_STATUS ret_code;

	numparms = count_parameters(buf);
	if (numparms != fc_max)
		parm_error_completion(numparms, fc_max, buf, sfs_help_rate_limit);
	else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &oneshot);

		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else {
			ret_code = tpm_fc_set_oneshot(oneshot);
			if (ret_code != MV_OK)
				printk(KERN_INFO "%s: tpm_fc_set_oneshot() failed!\n", __func__);
			else
				printk(KERN_INFO "OK\n");
		}
	}
}
#endif

/*******************************************************************************
* sfs_tpm_cfg_fc_enable
*
* DESCRIPTION:
*   		This function configures SW flow control engine minitoring
*   		oneshot count for statistics
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len     - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_fc_enable(const char *buf, size_t len)
{
	typedef enum {
		_fc_enable = 0,
		fc_max
	} fc_parm_indx_t;
	/* shell line parsing */
	uint32_t fc_enable;
	int parsedargs;
	int numparms;
	MV_STATUS ret_code;

	numparms = count_parameters(buf);
	if (numparms != fc_max)
		parm_error_completion(numparms, fc_max, buf, sfs_help_rate_limit);
	else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &fc_enable);

		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else {
			ret_code = tpm_fc_enable((fc_enable == 0) ? MV_FALSE : MV_TRUE);
			if (ret_code != MV_OK)
				printk(KERN_INFO "%s: tpm_fc_enable() failed!\n", __func__);
			else
				printk(KERN_INFO "OK\n");
		}
	}
}

/*******************************************************************************
* sfs_tpm_drop_pre_set
*
* DESCRIPTION:
*		This function configures drop precedence mode
* INPUTS:
*	buf	- Shell parameters as char buffer
*	len	- Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_drop_pre_set(const char *buf, size_t len)
{
	typedef enum {
		mode = 0,
		mode_max
	} drop_pre_parm_indx_t;
	/* shell line parsing */
	uint32_t drop_pre;
	int parsedargs;
	int numparms;
	MV_STATUS ret_code;

	numparms = count_parameters(buf);
	if (numparms != mode_max)
		parm_error_completion(numparms, mode_max, buf, sfs_help_drop_pre);
	else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &drop_pre);

		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else {
			ret_code = tpm_set_drop_precedence_mode(0, drop_pre);
			if (ret_code != MV_OK)
				printk(KERN_INFO "%s: failed!\n", __func__);
			else
				printk(KERN_INFO "OK\n");
		}
	}
}

/*******************************************************************************
* sfs_tpm_drop_pre_show
*
* DESCRIPTION:
*		This function show drop precedence mode
* INPUTS:
*	buf	- Shell parameters as char buffer
*	len	- Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_drop_pre_show(const char *buf, size_t len)
{
	typedef enum {
		mode = 0,
		mode_max
	} drop_pre_parm_indx_t;
	/* shell line parsing */
	uint32_t drop_pre;
	int parsedargs;
	int numparms;
	MV_STATUS ret_code;
	tpm_drop_precedence_t drop_pre_mode;

	numparms = count_parameters(buf);
	if (numparms != mode_max)
		parm_error_completion(numparms, mode_max, buf, sfs_help_drop_pre);
	else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &drop_pre);

		if (parsedargs != numparms)
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		else {
			ret_code = tpm_get_drop_precedence_mode(0, &drop_pre_mode);
			if (ret_code != MV_OK)
				printk(KERN_INFO "%s: failed!\n", __func__);
			else
				printk(KERN_INFO "OK Drop precedence mode %d\n", drop_pre_mode);
		}
	}
}



#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_add_ds_load_balance_rule_bounce(uint32_t                owner_id,
                                        uint32_t                rule_num,
                                        uint32_t               *rule_idx,
                                        tpm_parse_fields_t      parse_rule_bm,
					tpm_parse_flags_t	parse_flags_bm,
                                        tpm_l2_acl_key_t       *l2_key,
                                        tpm_ds_load_balance_tgrt_t tgrt_port)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_ADD_DS_LOAD_BALANCE_RULE;
    tpm_add_acl_rule->owner_id                      = owner_id;
    tpm_add_acl_rule->rule_num                      = rule_num;
    tpm_add_acl_rule->parse_rule_bm                 = parse_rule_bm;
    tpm_add_acl_rule->ds_load_balance_acl_rule.parse_flags_bm = parse_flags_bm;
    tpm_add_acl_rule->ds_load_balance_acl_rule.tgrt = tgrt_port;
    memcpy(&(tpm_add_acl_rule->ds_load_balance_acl_rule.l2_key),         (void*)l2_key,      sizeof(tpm_l2_acl_key_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_add_ds_load_balance_rule tpm_add_ds_load_balance_rule_bounce
#else
 #define _tpm_add_ds_load_balance_rule tpm_add_ds_load_balance_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_ds_load_balance_rule_add
*
* DESCRIPTION:
*           This function creates a ds_load_balance rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_ds_load_balance_rule_add  (const char *buf, size_t len)
{
    typedef enum
    {
        ds_load_blnc_ruleadd_owner=0, ds_load_blnc_ruleadd_rulenum,
	ds_load_blnc_ruleadd_parserulebm, ds_load_blnc_ruleadd_parseflagsbm,
	ds_load_blnc_ruleadd_l2keyname,
        ds_load_blnc_ruleadd_trgt, ds_load_blnc_ruleadd_max
    } ds_load_blnc_ruleadd_parm_indx_t;
    // shell line parsing
    uint32_t                 ownerid;
    uint32_t                 rulenum;
    uint32_t                 parserulebm;
    uint32_t                 parseflagsbm;
    uint32_t                 trgt;
    char                     l2keyname[20];
    int                      parsedargs;
    int                      numparms;
    // DB
    tpmcfg_l2_key_entry_t    *pdbl2keyentry = 0;
    //Used in API call
    tpm_l2_acl_key_t         l2_acl;
    uint32_t                 rule_idx;
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != ds_load_blnc_ruleadd_max)
    {
        parm_error_completion(numparms, ds_load_blnc_ruleadd_max, buf, sfs_help_ds_load_balance_rule);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%d %d 0x%x 0x%x %s %d",  &ownerid, &rulenum, &parserulebm,
                                 &parseflagsbm, l2keyname, &trgt);
        printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rulenum[%d], parserulebm[0x%x], parseflagsbm[0x%x], "
			"l2keyname[%s], trgt[%d]\n",
               len, parsedargs, ownerid, rulenum, parserulebm, parseflagsbm, l2keyname, trgt);

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (((pdbl2keyentry = find_tpm_l2_key_entry_by_name(l2keyname)) == 0) && strcmp(l2keyname, l2_key_empty_name) != 0)
        {
            printk(KERN_INFO "L2 key entry [%s] not found\n", l2keyname);
        }
        else
        {

            if (pdbl2keyentry != 0)  memcpy(&l2_acl, &pdbl2keyentry->l2_acl, sizeof(tpm_l2_acl_key_t));
            else                     memset(&l2_acl, 0,                      sizeof(tpm_l2_acl_key_t));

            if ((rc = _tpm_add_ds_load_balance_rule(ownerid,
                                       rulenum,
                                       &rule_idx,
                                       parserulebm,
                                       parseflagsbm,
                                       &l2_acl,
                                       trgt)) == TPM_RC_OK)
                PR_RULE_IDX(rule_idx)
            else
            {
                printk(KERN_INFO "%s: tpm_add_ds_load_balance_rule failed, rc[%d] - %s\n",
			__FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_del_ds_load_balance_rule_bounce(uint32_t        owner_id,
                                        uint32_t        rule_idx)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd   = MV_TPM_IOCTL_DEL_DS_LOAD_BALANCE_RULE;
    tpm_del_acl_rule->owner_id      = owner_id;
    tpm_del_acl_rule->rule_idx      = rule_idx;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_del_ds_load_balance_rule tpm_del_ds_load_balance_rule_bounce
#else
 #define _tpm_del_ds_load_balance_rule tpm_del_ds_load_balance_rule
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_no_rule_add_ds_load_balance
*
* DESCRIPTION:
*           This function deletes a ds_load_balance HW rule (PNC)
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_no_rule_add_ds_load_balance  (const char *buf, size_t len)
{
    typedef enum
    {
	noruleadd_owner=0,  noruleadd_ruleidx,	noruleadd_max
    } noruleadd_parm_indx_t;
    // shell line parsing
    uint32_t		     ownerid;
    uint32_t		     rule_idx;
    int 		     parsedargs;
    int 		     numparms;
    //Used in API call
    tpm_error_code_t	     rc;

    numparms = count_parameters(buf);
    if (numparms != noruleadd_max)
    {
	parm_error_completion(numparms, noruleadd_max, buf, sfs_help_no_rule_add);
    }
    else
    {
	// Get parameters
	parsedargs = sscanf(buf, "%d %d", &ownerid, &rule_idx);
	//printk(KERN_INFO "len=%d, parsedargs=%d. ownerid[%d], rule_idx[%d]\n",
	//	 len, parsedargs, ownerid, rule_idx);


	if (parsedargs != numparms)
	{
	    printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
	}
	else
	{
	    if ((rc = _tpm_del_ds_load_balance_rule(ownerid, rule_idx)) == TPM_RC_OK)
	    {
		printk(KERN_INFO "OK\n");
	    }
	    else
	    {
		printk(KERN_INFO "%s: tpm_del_ds_load_balance_rule failed, rc[%d] - %s\n",
			__FUNCTION__, rc, get_tpm_err_str(rc));
	    }
	}
    }
}


