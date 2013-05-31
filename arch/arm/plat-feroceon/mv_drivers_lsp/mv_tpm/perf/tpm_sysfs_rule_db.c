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
#include "tpm_sysfs_rule_db.h"

#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
extern struct semaphore    tpm_sfs_2_ioctl_sem;
extern tpm_ioctl_mng_t     tpm_sfs_2_ioctl_command;
 #define PR_RULE_IDX(rule_idx)  {}
 #define PR_RESULT              {}
#else
 #define PR_RULE_IDX(rule_idx)  {printk(KERN_INFO "OK. rule_idx = %d\n", rule_idx);}
 #define PR_RESULT              {printk(KERN_INFO "OK.\n");}
#endif

const char *vlan_empty_name = "vlan_empty";

/******************************************************************************/
/* ========================================================================== */
/*                            TPM SYS FS                                      */
/* ========================================================================== */
/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG FORWARDING FUNCTIONS                                                 */
/* ======================================================================================== */

/*******************************************************************************
* sfs_tpm_cfg_set_frwd_rule
*
* DESCRIPTION:
*           This function adds/modifies forwarding rule
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       None
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void sfs_tpm_cfg_set_frwd_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        frpi_name=0, frpi_port, frpi_que, frpi_gem, frpi_max
    } frwd_rule_parm_indx_t;
    unsigned int        temp_port;
    unsigned int        temp_que;
    unsigned int        temp_gem;
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_frwd_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != frpi_max)
    {
        parm_error_completion(numparms, frpi_max, buf, sfs_help_frwd_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s 0x%x %d %d", name, &temp_port, &temp_que,&temp_gem);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_port[0x%x],temp_que[%d],temp_gem[%d]\n",
//               len, parsedargs, name, temp_port, temp_que, temp_gem);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_que > 255)
        {
            printk(KERN_INFO "Invalid queue [%d]\n", temp_que);
        }
        else if (temp_gem > 4095)
        {
            printk(KERN_INFO "Invalid gem port [%d]\n", temp_gem);
        }
        else
        {
            if ((pentry = find_tpm_pkt_frwd_entry_by_name(name)) == 0)
                pentry = find_free_tpm_pkt_frwd_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Packet Forward DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->frwd.trg_port  = (tpm_trg_port_type_t)temp_port;
                pentry->frwd.trg_queue = (uint8_t)temp_que;
                pentry->frwd.gem_port  = (uint16_t)temp_gem;
            }
        }
    }
}

/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG VLAN FUNCTIONS                                                       */
/* ======================================================================================== */

/*******************************************************************************
* sfs_tpm_cfg_set_vlan_rule
*
* DESCRIPTION:
*           This function adds/modifies VLAN rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_vlan_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        vrpi_name=0, vrpi_tpid, vrpi_tpid_mask, vrpi_vid, vrpi_vid_mask, vrpi_cfi, vrpi_cfi_mask, vrpi_pbit,
		vrpi_pbit_mask, vrpi_max
    } vlan_rule_parm_indx_t;
    unsigned int        temp_tpid;
    unsigned int        temp_tpid_mask;
    unsigned int        temp_vid;
    unsigned int        temp_vid_mask;
    unsigned int        temp_cfi;
    unsigned int        temp_cfi_mask;
    unsigned int        temp_pbit;
    unsigned int        temp_pbit_mask;
    char                name[100];
    int                 parsedargs;
    int                 numparms = 0;
    tpmcfg_vlan_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != vrpi_max)
    {
        parm_error_completion(numparms, vrpi_max, buf, sfs_help_vlan_rule_cfg);
//        if (numparms == 1 && buf[0] == '?')
//        {
//            char helpbuf[3000];
//            int  helpbuf_len;
//
//            helpbuf_len = sfs_help_vlan_rule_cfg(helpbuf);
//            print_help_buffer(helpbuf, helpbuf_len);
//        }
//        else
//        {
//            printk(KERN_INFO "Parse problem: expected %d parameters, found %d\n", vrpi_max, numparms);
//        }
    }
    else
    {
        parsedargs = sscanf(buf, "%s 0x%x 0x%x %d 0x%x %d 0x%x %d 0x%x", name, &temp_tpid, &temp_tpid_mask, &temp_vid,
							&temp_vid_mask, &temp_cfi, &temp_cfi_mask, &temp_pbit, &temp_pbit_mask);
//        printk(KERN_INFO "len=%d. pardeargs=%d. rulename[%s] temp_tpid[0x%x], temp_tpid_mask[0x%x], vid[%d/0x%x], cfi[%d/0x%x],"
//						"pbit[%d/0x%x]\n", len, parsedargs, name, temp_tpid, temp_vid, temp_vid_mask,
//               				 temp_cfi, temp_cfi_mask, temp_pbit, temp_pbit_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_tpid > 0xFFFF)
        {
            printk(KERN_INFO "Invalid TPID [0x%x]\n", temp_tpid);
        }
        else if (temp_tpid_mask > 0xFFFF)
        {
            printk(KERN_INFO "Invalid TPID mask [0x%x]\n", temp_tpid_mask);
        }
        else if (temp_vid > 4095)
        {
            printk(KERN_INFO "Invalid VID [%d]\n", temp_vid);
        }
        else if (temp_vid_mask > 0xFFFF)
        {
            printk(KERN_INFO "Invalid VID mask [0x%x]\n", temp_vid_mask);
        }
        else if (temp_cfi > 1)
        {
            printk(KERN_INFO "Invalid CFI [%d]\n", temp_cfi);
        }
        else if (temp_cfi_mask > 0xFF)
        {
            printk(KERN_INFO "Invalid CFI mask [0x%x]\n", temp_cfi_mask);
        }
        else if (temp_pbit > 7)
        {
            printk(KERN_INFO "Invalid pbit [%d]\n", temp_pbit);
        }
        else if (temp_pbit_mask > 0xFF)
        {
            printk(KERN_INFO "Invalid pbit mask [0x%x]\n", temp_pbit_mask);
        }
        else
        {
            if ((pentry = find_tpm_vlan_entry_by_name(name)) == 0)
                pentry = find_free_tpm_vlan_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "VLAN DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->vlan.tpid      = (uint16_t)temp_tpid;
                pentry->vlan.tpid_mask = (uint16_t)temp_tpid_mask;
                pentry->vlan.vid       = (uint16_t)temp_vid;
                pentry->vlan.vid_mask  = (uint16_t)temp_vid_mask;
                pentry->vlan.cfi       = (uint8_t) temp_cfi;
                pentry->vlan.cfi_mask  = (uint8_t) temp_cfi_mask;
                pentry->vlan.pbit      = (uint8_t) temp_pbit;
                pentry->vlan.pbit_mask = (uint8_t) temp_pbit_mask;
            }
        }
    }
}

/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG MOD  FUNCTIONS                                                       */
/* ======================================================================================== */
#if 0
/*******************************************************************************
* sfs_tpm_cfg_set_mod_proto_rule
*
* DESCRIPTION:
*           This function adds/modifies protocol part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_proto_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mprpi_name=0, mprpi_protocol, mprpi_max
    } mod_proto_rule_parm_indx_t;
    unsigned int        temp_protocol;
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mprpi_max)
    {
        parm_error_completion(numparms, mprpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d", name, &temp_protocol);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_protocol[0x%x]\n",
//               len, parsedargs, name, temp_protocol);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_protocol > 255)
        {
            printk(KERN_INFO "Invalid protocol [%d]\n", temp_protocol);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_IP_PROTO;
                pentry->mod.l3.ipv4_mod.ipv4_proto  = (uint8_t)temp_protocol;
            }
        }
    }
}
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_mod_vlan_rule
*
* DESCRIPTION:
*           This function adds/modifies vlan part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_vlan_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mvrpi_name=0, mvrpi_oper, mvrpi_vlan1, mvrpi_vlan2, mvrpi_max
    } mod_vlan_rule_parm_indx_t;
    char                name[100];
    uint32_t            vlan_op;
    char                vlan_op_str[30];
    char                vlan1_name[100];
    char                vlan2_name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;
    tpmcfg_vlan_entry_t *pvlan1entry = 0;
    tpmcfg_vlan_entry_t *pvlan2entry = 0;

    numparms = count_parameters(buf);
    if (numparms != mvrpi_max)
    {
        parm_error_completion(numparms, mvrpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s %s", name, vlan_op_str, vlan1_name, vlan2_name);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] vlan_op_str[%s] vlan1_name[%s] vlan2_name[%s]\n",
//               len, parsedargs, name, vlan_op_str, vlan1_name, vlan2_name);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (get_vlan_op_value(vlan_op_str, &vlan_op) == GT_FALSE)     // TBD-ZG make symbolic
        {
            printk(KERN_INFO "Invalid vlan_op[%s]\n", vlan_op_str);
        }
        else if (((pvlan1entry = find_tpm_vlan_entry_by_name(vlan1_name)) == 0) && strcmp(vlan1_name, vlan_empty_name) != 0)
        {
            printk(KERN_INFO "vlan1 entry [%s] not found\n", vlan1_name);
        }
        else if (((pvlan2entry = find_tpm_vlan_entry_by_name(vlan2_name)) == 0) && strcmp(vlan2_name, vlan_empty_name) != 0)
        {
            printk(KERN_INFO "vlan2 entry [%s] not found\n", vlan2_name);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_VLAN;

                pentry->mod.vlan_mod.vlan_op = vlan_op;

                if (pvlan1entry != 0)
                {
                    memcpy(&pentry->mod.vlan_mod.vlan1_out, &pvlan1entry->vlan, sizeof(pentry->mod.vlan_mod.vlan1_out));
                }
                else
                {
                    memset(&pentry->mod.vlan_mod.vlan1_out, 0, sizeof(pentry->mod.vlan_mod.vlan1_out));
                }

                if (pvlan2entry != 0)
                {
                    memcpy(&pentry->mod.vlan_mod.vlan2_out, &pvlan2entry->vlan, sizeof(pentry->mod.vlan_mod.vlan2_out));
                }
                else
                {
                    memset(&pentry->mod.vlan_mod.vlan2_out, 0, sizeof(pentry->mod.vlan_mod.vlan2_out));
                }
            }
        }
    }
}

#if 0
/*******************************************************************************
* sfs_tpm_cfg_set_mod_dscp_rule
*
* DESCRIPTION:
*           This function adds/modifies DSCP part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_dscp_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mdspi_name=0, mdspi_dscp, mdspi_dscpmask, mdspi_max
    } mod_dscp_rule_parm_indx_t;
    unsigned int        temp_dscp;
    unsigned int        temp_dscp_mask;
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mdspi_max)
    {
        parm_error_completion(numparms, mdspi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d 0x%x", name, &temp_dscp, &temp_dscp_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_dscp[%d] temp_dscp_mask[0x%x]\n",
//               len, parsedargs, name, temp_dscp, temp_dscp_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_dscp > 63)
        {
            printk(KERN_INFO "Invalid dscp [%d]\n", temp_dscp);
        }
        else if (temp_dscp_mask > 0xFF)
        {
            printk(KERN_INFO "Invalid dscp mask [0x%x]\n", temp_dscp_mask);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_DSCP;
                pentry->mod.l3.ipv4_mod.ipv4_dscp      = (uint8_t)temp_dscp;
                pentry->mod.l3.ipv4_mod.ipv4_dscp_mask = (uint8_t)temp_dscp_mask;
            }
        }
    }
}
#endif

/*******************************************************************************
* sfs_tpm_cfg_set_mod_ipv4_addr_rule
*
* DESCRIPTION:
*           This function adds/modifies IPV4 address part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_ipv4_addr_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mipv4addpi_name=0, mipv4addpi_srcip, mipv4addpi_srcipmask, mipv4addpi_dstip, mipv4addpi_dstipmask, mipv4addpi_max
    } mod_ipv4add_rule_parm_indx_t;
    char                temp_srcip[30];
    char                temp_srcip_mask[30];
    char                temp_dstip[30];
    char                temp_dstip_mask[30];
    uint32_t            srcip[4];
    uint32_t            srcip_mask[4];
    uint32_t            dstip[4];
    uint32_t            dstip_mask[4];
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mipv4addpi_max)
    {
        parm_error_completion(numparms, mipv4addpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s %s %s", name, temp_srcip, temp_srcip_mask, temp_dstip, temp_dstip_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_srcip[%s] temp_srcip_mask[%s] temp_dstip[%s] temp_dstip_mask[%s]\n",
//               len, parsedargs, name, temp_srcip, temp_srcip_mask, temp_dstip, temp_dstip_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_ipv4_address(temp_srcip, srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip [%s]\n", temp_srcip);
        }
        else if (parse_ipv4_address(temp_srcip_mask, srcip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip mask [%s]\n", temp_srcip_mask);
        }
        else if (parse_ipv4_address(temp_dstip, dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dstip [%s]\n", temp_dstip);
        }
        else if (parse_ipv4_address(temp_dstip_mask, dstip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dstip mask [%s]\n", temp_dstip_mask);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else if (pentry->flags & MOD_ENTRY_FLAG_IPV6)
            {
                printk(KERN_INFO "IPV6 already defined for this entry!\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_IPV4;
                pentry->mod.l3.ipv4_mod.ipv4_src_ip_add[0] = (uint8_t)srcip[0]; pentry->mod.l3.ipv4_mod.ipv4_src_ip_add[1] = (uint8_t)srcip[1];
                pentry->mod.l3.ipv4_mod.ipv4_src_ip_add[2] = (uint8_t)srcip[2]; pentry->mod.l3.ipv4_mod.ipv4_src_ip_add[3] = (uint8_t)srcip[3];
                pentry->mod.l3.ipv4_mod.ipv4_src_ip_add_mask[0] = (uint8_t)srcip_mask[0]; pentry->mod.l3.ipv4_mod.ipv4_src_ip_add_mask[1] = (uint8_t)srcip_mask[1];
                pentry->mod.l3.ipv4_mod.ipv4_src_ip_add_mask[2] = (uint8_t)srcip_mask[2]; pentry->mod.l3.ipv4_mod.ipv4_src_ip_add_mask[3] = (uint8_t)srcip_mask[3];
                pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add[0] = (uint8_t)dstip[0]; pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add[1] = (uint8_t)dstip[1];
                pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add[2] = (uint8_t)dstip[2]; pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add[3] = (uint8_t)dstip[3];
                pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add_mask[0] = (uint8_t)dstip_mask[0]; pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add_mask[1] = (uint8_t)dstip_mask[1];
                pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add_mask[2] = (uint8_t)dstip_mask[2]; pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add_mask[3] = (uint8_t)dstip_mask[3];
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_mod_ipv4_port_rule
*
* DESCRIPTION:
*           This function adds/modifies IPV4 port part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_ipv4_port_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mipv4portpi_name=0, mipv4portpi_srcport, mipv4portpi_dstport, mipv4portpi_max
    } mod_ipv4port_rule_parm_indx_t;
    unsigned int        temp_src_port;
    unsigned int        temp_dst_port;
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mipv4portpi_max)
    {
        parm_error_completion(numparms, mipv4portpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d %d", name, &temp_src_port, &temp_dst_port);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_src_port[%d] temp_dst_port[%d]\n",
//               len, parsedargs, name, temp_src_port, temp_dst_port);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_src_port > 0xFFFF)
        {
            printk(KERN_INFO "Invalid srcport [%d]\n", temp_src_port);
        }
        else if (temp_dst_port > 0xFFFF)
        {
            printk(KERN_INFO "Invalid dstport [%d]\n", temp_dst_port);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else if (pentry->flags & MOD_ENTRY_FLAG_IPV6)
            {
                printk(KERN_INFO "IPV6 already defined for this entry!\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_IPV4;
                pentry->mod.l3.ipv4_mod.l4_src_port = (uint16_t)temp_src_port;
                pentry->mod.l3.ipv4_mod.l4_dst_port = (uint16_t)temp_dst_port;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_mod_ipv6_addr_rule
*
* DESCRIPTION:
*           This function adds/modifies IPV6 address part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_ipv6_addr_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mipv6addpi_name=0, mipv6addpi_srcip, mipv6addpi_srcipmask, mipv6addpi_dstip, mipv6addpi_dstipmask, mipv6addpi_max
    } mod_ipv6add_rule_parm_indx_t;
    char                     temp_srcip[60];
    char                     temp_srcip_mask[60];
    char                     temp_dstip[60];
    char                     temp_dstip_mask[60];
    uint32_t                 srcip[16];
    uint32_t                 srcip_mask[16];
    uint32_t                 dstip[16];
    uint32_t                 dstip_mask[16];
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    int                      indx;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mipv6addpi_max)
    {
        parm_error_completion(numparms, mipv6addpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s %s %s", name, temp_srcip, temp_srcip_mask, temp_dstip, temp_dstip_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_srcip[%s] temp_srcip_mask[%s] temp_dstip[%s] temp_dstip_mask[%s]\n",
//               len, parsedargs, name, temp_srcip, temp_srcip_mask, temp_dstip, temp_dstip_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_ipv6_address(temp_srcip, srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip [%s]\n", temp_srcip);
        }
        else if (parse_ipv6_address(temp_srcip_mask, srcip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip mask [%s]\n", temp_srcip_mask);
        }
        else if (parse_ipv6_address(temp_dstip, dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dstip [%s]\n", temp_dstip);
        }
        else if (parse_ipv6_address(temp_dstip_mask, dstip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dstip mask [%s]\n", temp_dstip_mask);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else if (pentry->flags & MOD_ENTRY_FLAG_IPV4)
            {
                printk(KERN_INFO "IPV4 already defined for this entry!\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_IPV6;
                for (indx = 0; indx < 16; indx++)
                {
                    pentry->mod.l3.ipv6_mod.ipv6_src_ip_add[indx] = (uint8_t)srcip[indx];
                    pentry->mod.l3.ipv6_mod.ipv6_src_ip_add_mask[indx] = (uint8_t)srcip_mask[indx];
                    pentry->mod.l3.ipv6_mod.ipv6_dst_ip_add[indx] = (uint8_t)dstip[indx];
                    pentry->mod.l3.ipv6_mod.ipv6_dst_ip_add_mask[indx] = (uint8_t)dstip_mask[indx];
                }
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_mod_ipv6_port_rule
*
* DESCRIPTION:
*           This function adds/modifies IPV6 port part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_ipv6_port_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mipv6portpi_name=0, mipv6portpi_srcport, mipv6portpi_dstport, mipv6portpi_max
    } mod_ipv6port_rule_parm_indx_t;
    unsigned int             temp_src_port;
    unsigned int             temp_dst_port;
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mipv6portpi_max)
    {
        parm_error_completion(numparms, mipv6portpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d %d", name, &temp_src_port, &temp_dst_port);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_src_port[%d] temp_dst_port[%d]\n",
//               len, parsedargs, name, temp_src_port, temp_dst_port);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_src_port > 0xFFFF)
        {
            printk(KERN_INFO "Invalid srcport [%d]\n", temp_src_port);
        }
        else if (temp_dst_port > 0xFFFF)
        {
            printk(KERN_INFO "Invalid dstport [%d]\n", temp_dst_port);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else if (pentry->flags & MOD_ENTRY_FLAG_IPV4)
            {
                printk(KERN_INFO "IPV4 already defined for this entry!\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_IPV6;
                pentry->mod.l3.ipv6_mod.l4_src_port = (uint16_t)temp_src_port;
                pentry->mod.l3.ipv6_mod.l4_dst_port = (uint16_t)temp_dst_port;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_mod_mac_addr_rule
*
* DESCRIPTION:
*           This function adds/modifies MAC address part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_mac_addr_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mmacpi_name=0, mmacpi_srcip, mmacpi_srcipmask, mmacpi_dstip, mmacpi_dstipmask, mmacpi_max
    } mod_mac_rule_parm_indx_t;
    char                temp_sa[30];
    char                temp_sa_mask[30];
    char                temp_da[30];
    char                temp_da_mask[30];
    uint32_t            sa[6];
    uint32_t            sa_mask[6];
    uint32_t            da[6];
    uint32_t            da_mask[6];
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mmacpi_max)
    {
        parm_error_completion(numparms, mmacpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s %s %s", name, temp_sa, temp_sa_mask, temp_da, temp_da_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_sa[%s] temp_sa_mask[%s] temp_da[%s] temp_da_mask[%s]\n",
//               len, parsedargs, name, temp_sa, temp_sa_mask, temp_da, temp_da_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_mac_address(temp_sa, sa) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid SA [%s]\n", temp_sa);
        }
        else if (parse_mac_address(temp_sa_mask, sa_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid SA mask [%s]\n", temp_sa_mask);
        }
        else if (parse_mac_address(temp_da, da) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid DA [%s]\n", temp_da);
        }
        else if (parse_mac_address(temp_da_mask, da_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid DA mask [%s]\n", temp_da_mask);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_MAC;
                pentry->mod.mac_mod.mac_sa[0] = (uint8_t)sa[0]; pentry->mod.mac_mod.mac_sa[1] = (uint8_t)sa[1]; pentry->mod.mac_mod.mac_sa[2] = (uint8_t)sa[2];
                pentry->mod.mac_mod.mac_sa[3] = (uint8_t)sa[3]; pentry->mod.mac_mod.mac_sa[4] = (uint8_t)sa[4]; pentry->mod.mac_mod.mac_sa[5] = (uint8_t)sa[5];

                pentry->mod.mac_mod.mac_sa_mask[0] = (uint8_t)sa_mask[0]; pentry->mod.mac_mod.mac_sa_mask[1] = (uint8_t)sa_mask[1]; pentry->mod.mac_mod.mac_sa_mask[2] = (uint8_t)sa_mask[2];
                pentry->mod.mac_mod.mac_sa_mask[3] = (uint8_t)sa_mask[3]; pentry->mod.mac_mod.mac_sa_mask[4] = (uint8_t)sa_mask[4]; pentry->mod.mac_mod.mac_sa_mask[5] = (uint8_t)sa_mask[5];

                pentry->mod.mac_mod.mac_da[0] = (uint8_t)da[0]; pentry->mod.mac_mod.mac_da[1] = (uint8_t)da[1]; pentry->mod.mac_mod.mac_da[2] = (uint8_t)da[2];
                pentry->mod.mac_mod.mac_da[3] = (uint8_t)da[3]; pentry->mod.mac_mod.mac_da[4] = (uint8_t)da[4]; pentry->mod.mac_mod.mac_da[5] = (uint8_t)da[5];

                pentry->mod.mac_mod.mac_da_mask[0] = (uint8_t)da_mask[0]; pentry->mod.mac_mod.mac_da_mask[1] = (uint8_t)da_mask[1]; pentry->mod.mac_mod.mac_da_mask[2] = (uint8_t)da_mask[2];
                pentry->mod.mac_mod.mac_da_mask[3] = (uint8_t)da_mask[3]; pentry->mod.mac_mod.mac_da_mask[4] = (uint8_t)da_mask[4]; pentry->mod.mac_mod.mac_da_mask[5] = (uint8_t)da_mask[5];
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_mod_mh_addr_rule
*
* DESCRIPTION:
*           This function adds/modifies marvell header part of modification rule
*
* See sfs_tpm_cfg_set_mod_mh_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_mh_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mmacpi_name=0, mmacpi_mh2b, mmacpi_max
    } mod_mac_rule_parm_indx_t;
    uint32_t            mh2b = 0;
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mmacpi_max)
    {
        parm_error_completion(numparms, mmacpi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s 0x%2x", name, &mh2b);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] mh2b[0x%x]\n"
//               len, parsedargs, name, mh2b);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (mh2b == 0)
        {
            printk(KERN_INFO "Invalid mh2b [0x%2x]\n", mh2b);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_MH;
                pentry->mod.mh_mod = mh2b;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_mod_pppoe_rule
*
* DESCRIPTION:
*           This function adds/modifies PPPoE part of modification rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_mod_pppoe_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        mpppoepi_name=0, mpppoepi_session, mpppoepi_prototype, mpppoepi_max
    } mod_pppoe_rule_parm_indx_t;
    unsigned int        temp_session;
    unsigned int        temp_prototype;
    char                name[100];
    int                 parsedargs;
    int                 numparms;
    tpmcfg_mod_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != mpppoepi_max)
    {
        parm_error_completion(numparms, mpppoepi_max, buf, sfs_help_mod_rule_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d 0x%x", name, &temp_session, &temp_prototype);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_session[%d] temp_prototype[0x%x]\n",
//               len, parsedargs, name, temp_session, temp_prototype);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_session > 0xFFFF)
        {
            printk(KERN_INFO "Invalid session [%d]\n", temp_session);
        }
        else if (temp_prototype > 0xFFFF)
        {
            printk(KERN_INFO "Invalid protocol type [0x%x]\n", temp_prototype);
        }
        else
        {
            if ((pentry = find_tpm_mod_entry_by_name(name)) == 0)
                pentry = find_free_tpm_mod_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "Mod DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->flags |= MOD_ENTRY_FLAG_PPPOE;
                pentry->mod.pppoe_mod.ppp_session = (uint16_t)temp_session;
                pentry->mod.pppoe_mod.ppp_proto   = (uint16_t)temp_prototype;
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_mod_entry_set_bounce(tpm_trg_port_type_t   trg_port,
                                          tpm_pkt_mod_bm_t      mod_bm,
                                          tpm_pkt_mod_int_bm_t  int_mod_bm,
                                          tpm_pkt_mod_t        *mod_data,
                                          uint32_t             *mod_entry)
{
    tpm_ioctl_add_acl_rule_t *tpm_add_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_add_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_ADD_ACL_SECTION;
    tpm_add_acl_rule->add_acl_cmd                   = MV_TPM_IOCTL_SET_MOD_RULE;
    tpm_add_acl_rule->mod_rule.trg_port             = trg_port;
    tpm_add_acl_rule->mod_rule.mod_bm               = mod_bm;
    tpm_add_acl_rule->mod_rule.int_bm               = int_mod_bm;
    memcpy(&(tpm_add_acl_rule->mod_rule.mod_data), (void*)mod_data, sizeof(tpm_pkt_mod_t));

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_mod_entry_set tpm_mod_entry_set_bounce
#else
 #define _tpm_mod_entry_set tpm_mod_entry_set
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_pkt_mod_eng_entry_add
*
* DESCRIPTION:
*           This function adds packet modification to specified port
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_pkt_mod_eng_entry_add      (const char *buf, size_t len)
{
    typedef enum
    {
        pktmodadd_trgport=0, pktmodadd_modbm, pktmodadd_int_modbm, pktmodadd_modname, pktmodadd_max
    } pktmodadd_parm_indx_t;
    // shell line parsing
    char                trgport_str[20];
    char                uc_trgport_str[20];
    int                 indx;
    uint32_t            trgport;
    uint32_t            mod_bm;
    uint32_t            int_mod_bm;
    char                modname[20];
    tpm_pkt_mod_t       mod_data;
    uint32_t            mod_entry = 0;
    int                 parsedargs;
    int                 numparms;
    // DB
    tpmcfg_mod_entry_t  *pdbmodentry = 0;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != pktmodadd_max)
    {
        parm_error_completion(numparms, pktmodadd_max, buf, sfs_help_pkt_mod_add);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s 0x%x 0x%x %s", trgport_str, &mod_bm, &int_mod_bm, modname);
//        printk(KERN_INFO "len=%d, parsedargs=%d. trgport_str[%s], mod_bm[0x%x], flag[0x%x], modname[%s]\n",
//               len, parsedargs, trgport_str, mod_bm, int_mod_bm, modname);

        // Minimal help for trgport upper/lower case support
        for (indx = 0; indx < strlen(trgport_str); indx++) uc_trgport_str[indx] = (char)toupper(trgport_str[indx]);
        uc_trgport_str[strlen(trgport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_sched_entity_value(uc_trgport_str, &trgport) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid trg_port[%s]\n", trgport_str);
        }
        else if ((pdbmodentry = find_tpm_mod_entry_by_name(modname)) == 0)
        {
            printk(KERN_INFO "mod entry [%s] not found\n", modname);
        }
        else
        {
            memcpy(&mod_data, &pdbmodentry->mod, sizeof(mod_data));

            if ((rc = _tpm_mod_entry_set(trgport,
                                         mod_bm,
                                         int_mod_bm,
                                         &mod_data,
                                         &mod_entry)) == TPM_RC_OK)
            {
                PR_RULE_IDX(mod_entry)
            }
            else
            {
                printk(KERN_INFO "%s: tpm_mod_entry_set failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

static char* opCodeOperationStr[30] =
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

/*Evan*/
#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_mod_entry_get_bounce(tpm_trg_port_type_t trg_port,
					uint32_t mod_entry,
					uint16_t *valid_cmds,
					uint16_t *pnc_ref,
					tpm_mod_rule_t *rule)
{
	tpm_ioctl_get_mod_rule_t *tpm_get_mod_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_ioctl_get_mod;

	tpm_sfs_2_ioctl_command.cmd   = MV_TPM_IOCTL_GET_MOD_SECTION;
	tpm_get_mod_rule->get_mod_cmd = MV_TPM_IOCTL_GET_MOD_RULE;
	tpm_get_mod_rule->trg_port    = trg_port;
	tpm_get_mod_rule->rule_idx    = mod_entry;
	//tpm_get_mod_rule->valid_num   = *valid_cmds;
	//tpm_get_mod_rule->pnc_ref     = *pnc_ref;
	up(&tpm_sfs_2_ioctl_sem);
	//pnc_ref = &tpm_get_mod_rule->pnc_ref;//     = pnc_ref;
	//rule = tpm_get_mod_rule->rule;// = rule;
	//up(&tpm_sfs_2_ioctl_sem);
	return TPM_RC_OK;
}
 #define _tpm_mod_entry_get tpm_mod_entry_get_bounce
#else
 #define _tpm_mod_entry_get tpm_mod_entry_get
#endif
/*******************************************************************************
* sfs_tpm_cfg_set_pkt_mod_eng_entry_show
*
* DESCRIPTION:
*           This function displays specific packet modification
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_pkt_mod_eng_entry_show     (const char *buf, size_t len)
{
    typedef enum
    {
        pktmodshow_trgport=0, pktmodshow_modentry, pktmodshow_max
    } pktmoddel_parm_indx_t;
    // shell line parsing
    char                trgport_str[20];
    char                uc_trgport_str[20];
    int                 indx;
    uint32_t            trgport;
    uint32_t            mod_entry;
    uint16_t            valid_cmds;
    uint16_t            pnc_ref;
    tpm_mod_rule_t      rule[TPM_MAX_MOD_RULE_NUM];
    int                 parsedargs;
    int                 numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != pktmodshow_max)
    {
        parm_error_completion(numparms, pktmodshow_max, buf, sfs_help_pkt_mod_get_del_purge);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d", trgport_str, &mod_entry);
//        printk(KERN_INFO "len=%d, parsedargs=%d. trgport_str[%s], mod_entry[%d]\n", len, parsedargs, trgport_str, mod_entry);

        // Minimal help for trgport upper/lower case support
        for (indx = 0; indx < strlen(trgport_str); indx++) uc_trgport_str[indx] = (char)toupper(trgport_str[indx]);
        uc_trgport_str[strlen(trgport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_sched_entity_value(uc_trgport_str, &trgport) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid trg_port[%s]\n", trgport_str);
        }
        else
        {
            if ((rc = _tpm_mod_entry_get(trgport,
                                         mod_entry,
                                         &valid_cmds,
                                         &pnc_ref,
                                         rule)) == TPM_RC_OK)
            {
#ifndef CONFIG_MV_TPM_SFS_2_IOCTL
                uint16_t         rule_idx;

                printk("==========================================================\n");
                printk(" Index  OpCode    Operation      Data  Last  IPv4    L4   \n");
                printk("                                            update update \n");
                printk("==========================================================\n");

                for (rule_idx = 0; rule_idx < valid_cmds; rule_idx++)
                {
                    printk(" %03d      0x%2x  %15s  0x%4.4x  %1d     %1d      %1d\n",
                           rule[rule_idx].entry_id, rule[rule_idx].entry_data.opcode,
                           opCodeOperationStr[rule[rule_idx].entry_data.opcode],
                           rule[rule_idx].entry_data.data, rule[rule_idx].entry_data.last,
                           rule[rule_idx].entry_data.updt_ipv4, rule[rule_idx].entry_data.updt_tcp);
                }
#endif
            }
            else
            {
                printk(KERN_INFO "%s: tpm_mod_entry_get failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }

}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_mod_entry_del_bounce(tpm_trg_port_type_t  trg_port,
                                          uint32_t             mod_entry)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd                   = MV_TPM_IOCTL_DEL_MOD_RULE;
    tpm_del_acl_rule->trg_port                      = trg_port;
    tpm_del_acl_rule->rule_idx                      = mod_entry;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_mod_entry_del tpm_mod_entry_del_bounce
#else
 #define _tpm_mod_entry_del tpm_mod_entry_del
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_pkt_mod_eng_entry_del
*
* DESCRIPTION:
*           This function deletes single packet modification from supplied port
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_pkt_mod_eng_entry_del      (const char *buf, size_t len)
{
    typedef enum
    {
        pktmoddel_trgport=0, pktmoddel_modentry, pktmoddel_max
    } pktmoddel_parm_indx_t;
    // shell line parsing
    char                trgport_str[20];
    char                uc_trgport_str[20];
    int                 indx;
    uint32_t            trgport;
    uint32_t            mod_entry;
    int                 parsedargs;
    int                 numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms != pktmoddel_max)
    {
        parm_error_completion(numparms, pktmoddel_max, buf, sfs_help_pkt_mod_get_del_purge);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d", trgport_str, &mod_entry);
//        printk(KERN_INFO "len=%d, parsedargs=%d. trgport_str[%s], mod_entry[%d]\n", len, parsedargs, trgport_str, mod_entry);

        // Minimal help for trgport upper/lower case support
        for (indx = 0; indx < strlen(trgport_str); indx++) uc_trgport_str[indx] = (char)toupper(trgport_str[indx]);
        uc_trgport_str[strlen(trgport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_sched_entity_value(uc_trgport_str, &trgport) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid trg_port[%s]\n", trgport_str);
        }
        else
        {
            if ((rc = _tpm_mod_entry_del(trgport,
                                         mod_entry)) == TPM_RC_OK)
                PR_RESULT
            else
            {
                printk(KERN_INFO "%s: tpm_mod_entry_del failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}


#ifdef CONFIG_MV_TPM_SFS_2_IOCTL
tpm_error_code_t tpm_mod_mac_inv_bounce(tpm_trg_port_type_t  trg_port)
{
    tpm_ioctl_del_acl_rule_t *tpm_del_acl_rule = &tpm_sfs_2_ioctl_command.tpm_cmd_data.tpm_del_acl_rule;

    tpm_sfs_2_ioctl_command.cmd                     = MV_TPM_IOCTL_DEL_ACL_SECTION;
    tpm_del_acl_rule->del_acl_cmd                   = MV_TPM_IOCTL_INV_MOD_RULES;
    tpm_del_acl_rule->trg_port                      = trg_port;

    up(&tpm_sfs_2_ioctl_sem);

    return TPM_RC_OK;
}
 #define _tpm_mod_mac_inv tpm_mod_mac_inv_bounce
#else
 #define _tpm_mod_mac_inv tpm_mod_mac_inv
#endif


/*******************************************************************************
* sfs_tpm_cfg_set_pkt_mod_eng_purge
*
* DESCRIPTION:
*           This function removes all packet modification from supplied port
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_tpm_cfg_set_pkt_mod_eng_purge          (const char *buf, size_t len)
{
    typedef enum
    {
        pktmodadd_trgport, pktmodpurge_max
    } pktmodpurge_parm_indx_t;
    // shell line parsing
    char                trgport_str[20];
    char                uc_trgport_str[20];
    int                 indx;
    uint32_t            trgport;
    int                 parsedargs;
    int                 numparms;
    //Used in API call
    tpm_error_code_t         rc;

    numparms = count_parameters(buf);
    if (numparms == 1 && buf[0] == '?')
    {
#ifdef CONFIG_MV_TPM_SYSFS_HELP
        char helpbuf[1000];
        int  helpbuf_len;

        helpbuf_len = sfs_help_pkt_mod_get_del_purge(helpbuf);
        print_help_buffer(helpbuf, helpbuf_len);
#else
	printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", pktmodpurge_max, numparms);
#endif
    }
    else if (numparms != 1)
    {
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", pktmodpurge_max, numparms);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s", trgport_str);
//        printk(KERN_INFO "len=%d, parsedargs=%d. trgport_str[%s]\n", len, parsedargs, trgport_str);

        // Minimal help for trgport upper/lower case support
        for (indx = 0; indx < strlen(trgport_str); indx++) uc_trgport_str[indx] = (char)toupper(trgport_str[indx]);
        uc_trgport_str[strlen(trgport_str)] = 0;

        if (parsedargs != numparms)
        {
            printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
        }
        else if (get_sched_entity_value(uc_trgport_str, &trgport) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid trg_port[%s]\n", trgport_str);
        }
        else
        {
            if ((rc = _tpm_mod_mac_inv(trgport)) == TPM_RC_OK)
                PR_RESULT
            else
            {
                printk(KERN_INFO "%s: tpm_mod_mac_inv failed, rc[%d] - %s\n", __FUNCTION__, rc, get_tpm_err_str(rc));
            }
        }
    }
}

/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG L2 KEY FUNCTIONS                                                   */
/* ======================================================================================== */

/*******************************************************************************
* sfs_tpm_cfg_set_l2_key_ethertype_rule
*
* DESCRIPTION:
*           This function adds/modifies ethertype part part of L2 ACL key
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_l2_key_ethertype_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l2etypi_name=0, l2etypi_ety, l2etypi_max
    } l2_ety_rule_parm_indx_t;
    unsigned int          temp_ety;
    char                  name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l2_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != l2etypi_max)
    {
        parm_error_completion(numparms, l2etypi_max, buf, sfs_help_l2_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s 0x%x", name, &temp_ety);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_ety[0x%x]\n",
//               len, parsedargs, name, temp_ety);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_ety > 0xFFFF)
        {
            printk(KERN_INFO "Invalid ethertype [0x%x]\n", temp_ety);
        }
        else
        {
            if ((pentry = find_tpm_l2_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l2_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L2 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->l2_key_type = l2_key_type_ethertype;
                pentry->l2_acl.ether_type  = (uint16_t)temp_ety;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_l2_key_gemport_rule
*
* DESCRIPTION:
*           This function adds/modifies gemport part part of L2 ACL key
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_l2_key_gemport_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l2gempi_name=0, l2gempi_ety, l2gempi_max
    } l2_gemport_rule_parm_indx_t;
    unsigned int          temp_gemport;
    char                  name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l2_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != l2gempi_max)
    {
        parm_error_completion(numparms, l2gempi_max, buf, sfs_help_l2_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d", name, &temp_gemport);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_gemport[%d]\n",
//               len, parsedargs, name, temp_gemport);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_gemport > 4095)
        {
            printk(KERN_INFO "Invalid gemport [%d]\n", temp_gemport);
        }
        else
        {
            if ((pentry = find_tpm_l2_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l2_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L2 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->l2_key_type = l2_key_type_gemport;
                pentry->l2_acl.gem_port  = (uint16_t)temp_gemport;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_l2_key_mac_addr_rule
*
* DESCRIPTION:
*           This function adds/modifies MAC address part of L2 ACL key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_l2_key_mac_addr_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l2macpi_name=0, l2macpi_srcip, l2macpi_srcipmask, l2macpi_dstip, l2macpi_dstipmask, l2macpi_max
    } l2_mac_rule_parm_indx_t;
    char                  temp_sa[30];
    char                  temp_sa_mask[30];
    char                  temp_da[30];
    char                  temp_da_mask[30];
    uint32_t              sa[6];
    uint32_t              sa_mask[6];
    uint32_t              da[6];
    uint32_t              da_mask[6];
    char                  name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l2_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != l2macpi_max)
    {
        parm_error_completion(numparms, l2macpi_max, buf, sfs_help_l2_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s %s %s", name, temp_sa, temp_sa_mask, temp_da, temp_da_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_sa[%s] temp_sa_mask[%s] temp_da[%s] temp_da_mask[%s]\n",
//               len, parsedargs, name, temp_sa, temp_sa_mask, temp_da, temp_da_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_mac_address(temp_sa, sa) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid SA [%s]\n", temp_sa);
        }
        else if (parse_mac_address(temp_sa_mask, sa_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid SA mask [%s]\n", temp_sa_mask);
        }
        else if (parse_mac_address(temp_da, da) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid DA [%s]\n", temp_da);
        }
        else if (parse_mac_address(temp_da_mask, da_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid DA mask [%s]\n", temp_da_mask);
        }
        else
        {
            if ((pentry = find_tpm_l2_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l2_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L2 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->l2_key_type = l2_key_type_macaddr;
                pentry->l2_acl.mac.mac_sa[0] = (uint8_t)sa[0]; pentry->l2_acl.mac.mac_sa[1] = (uint8_t)sa[1]; pentry->l2_acl.mac.mac_sa[2] = (uint8_t)sa[2];
                pentry->l2_acl.mac.mac_sa[3] = (uint8_t)sa[3]; pentry->l2_acl.mac.mac_sa[4] = (uint8_t)sa[4]; pentry->l2_acl.mac.mac_sa[5] = (uint8_t)sa[5];

                pentry->l2_acl.mac.mac_sa_mask[0] = (uint8_t)sa_mask[0]; pentry->l2_acl.mac.mac_sa_mask[1] = (uint8_t)sa_mask[1]; pentry->l2_acl.mac.mac_sa_mask[2] = (uint8_t)sa_mask[2];
                pentry->l2_acl.mac.mac_sa_mask[3] = (uint8_t)sa_mask[3]; pentry->l2_acl.mac.mac_sa_mask[4] = (uint8_t)sa_mask[4]; pentry->l2_acl.mac.mac_sa_mask[5] = (uint8_t)sa_mask[5];

                pentry->l2_acl.mac.mac_da[0] = (uint8_t)da[0]; pentry->l2_acl.mac.mac_da[1] = (uint8_t)da[1]; pentry->l2_acl.mac.mac_da[2] = (uint8_t)da[2];
                pentry->l2_acl.mac.mac_da[3] = (uint8_t)da[3]; pentry->l2_acl.mac.mac_da[4] = (uint8_t)da[4]; pentry->l2_acl.mac.mac_da[5] = (uint8_t)da[5];

                pentry->l2_acl.mac.mac_da_mask[0] = (uint8_t)da_mask[0]; pentry->l2_acl.mac.mac_da_mask[1] = (uint8_t)da_mask[1]; pentry->l2_acl.mac.mac_da_mask[2] = (uint8_t)da_mask[2];
                pentry->l2_acl.mac.mac_da_mask[3] = (uint8_t)da_mask[3]; pentry->l2_acl.mac.mac_da_mask[4] = (uint8_t)da_mask[4]; pentry->l2_acl.mac.mac_da_mask[5] = (uint8_t)da_mask[5];
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_l2_key_pppoe_rule
*
* DESCRIPTION:
*           This function adds/modifies PPPoE part of L2 ACL key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_l2_key_pppoe_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l2pppoepi_name=0, l2pppoepi_session, l2pppoepi_prototype, l2pppoepi_max
    } l2_pppoe_rule_parm_indx_t;
    unsigned int          temp_session;
    unsigned int          temp_prototype;
    char                  name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l2_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != l2pppoepi_max)
    {
        parm_error_completion(numparms, l2pppoepi_max, buf, sfs_help_l2_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d 0x%x", name, &temp_session, &temp_prototype);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_session[%d] temp_prototype[0x%x]\n",
//               len, parsedargs, name, temp_session, temp_prototype);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_session > 0xFFFF)
        {
            printk(KERN_INFO "Invalid session [%d]\n", temp_session);
        }
        else if (temp_prototype > 0xFFFF)
        {
            printk(KERN_INFO "Invalid protocol type [0x%x]\n", temp_prototype);
        }
        else
        {
            if ((pentry = find_tpm_l2_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l2_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L2 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->l2_key_type = l2_key_type_pppoe;
                pentry->l2_acl.pppoe_hdr.ppp_session = (uint16_t)temp_session;
                pentry->l2_acl.pppoe_hdr.ppp_proto   = (uint16_t)temp_prototype;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_l2_key_vlan_rule
*
* DESCRIPTION:
*           This function adds/modifies vlan part of L2 ACL key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_l2_key_vlan_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l2vrpi_name=0, l2vrpi_vlan1, l2vrpi_vlan2, l2vrpi_max
    } l2_vlan_rule_parm_indx_t;
    char                  name[100];
    char                  vlan1_name[100];
    char                  vlan2_name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l2_key_entry_t *pentry;
    tpmcfg_vlan_entry_t   *pvlan1entry = 0;
    tpmcfg_vlan_entry_t   *pvlan2entry = 0;

    numparms = count_parameters(buf);
    if (numparms != l2vrpi_max)
    {
        parm_error_completion(numparms, l2vrpi_max, buf, sfs_help_l2_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s", name, vlan1_name, vlan2_name);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] vlan1_name[%s] vlan2_name[%s]\n",
//               len, parsedargs, name, vlan1_name, vlan2_name);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (((pvlan1entry = find_tpm_vlan_entry_by_name(vlan1_name)) == 0) && strcmp(vlan1_name, vlan_empty_name) != 0)
        {
            printk(KERN_INFO "vlan1 entry [%s] not found\n", vlan1_name);
        }
        else if (((pvlan2entry = find_tpm_vlan_entry_by_name(vlan2_name)) == 0) && strcmp(vlan2_name, vlan_empty_name) != 0)
        {
            printk(KERN_INFO "vlan2 entry [%s] not found\n", vlan2_name);
        }
        else
        {
            if ((pentry = find_tpm_l2_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l2_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L2 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                strcpy(pentry->vlan_rule1, vlan1_name);
                strcpy(pentry->vlan_rule2, vlan2_name);
                pentry->l2_key_type = l2_key_type_vlan;

                if (pvlan1entry != 0)
                {
                    memcpy(&pentry->l2_acl.vlan1, &pvlan1entry->vlan, sizeof(pentry->l2_acl.vlan1));
                }
                else
                {
                    memset(&pentry->l2_acl.vlan1, 0, sizeof(pentry->l2_acl.vlan1));
                }

                if (pvlan2entry != 0)
                {
                    memcpy(&pentry->l2_acl.vlan2, &pvlan2entry->vlan, sizeof(pentry->l2_acl.vlan2));
                }
                else
                {
                    memset(&pentry->l2_acl.vlan2, 0, sizeof(pentry->l2_acl.vlan2));
                }
            }
        }
    }
}


/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG L3 KEY FUNCTIONS                                                   */
/* ======================================================================================== */

/*******************************************************************************
* sfs_tpm_cfg_set_l3_key_ethertype_rule
*
* DESCRIPTION:
*           This function adds/modifies ethertype part part of L3 ACL key
*
*******************************************************************************/
void sfs_tpm_cfg_set_l3_key_ethertype_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l3etypi_name=0, l3etypi_ety, l3etypi_max
    } l3_ety_rule_parm_indx_t;
    unsigned int          temp_ety;
    char                  name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l3_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != l3etypi_max)
    {
        parm_error_completion(numparms, l3etypi_max, buf, sfs_help_l3_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s 0x%x", name, &temp_ety);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_ety[0x%x]\n", len, parsedargs, name, temp_ety);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_ety > 0xFFFF)
        {
            printk(KERN_INFO "Invalid ethertype [0x%x]\n", temp_ety);
        }
        else
        {
            if ((pentry = find_tpm_l3_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l3_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L3 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->l3_acl.ether_type_key  = (uint16_t)temp_ety;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_l3_key_pppoe_rule
*
* DESCRIPTION:
*           This function adds/modifies PPPoE part of L3 ACL key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_l3_key_pppoe_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        l3pppoepi_name=0, l3pppoepi_session, l3pppoepi_prototype, l3pppoepi_max
    } l3_pppoe_rule_parm_indx_t;
    unsigned int          temp_session;
    unsigned int          temp_prototype;
    char                  name[100];
    int                   parsedargs;
    int                   numparms;
    tpmcfg_l3_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != l3pppoepi_max)
    {
        parm_error_completion(numparms, l3pppoepi_max, buf, sfs_help_l3_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d 0x%x", name, &temp_session, &temp_prototype);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_session[%d] temp_prototype[0x%x]\n",
//               len, parsedargs, name, temp_session, temp_prototype);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_session > 0xFFFF)
        {
            printk(KERN_INFO "Invalid session [%d]\n", temp_session);
        }
        else if (temp_prototype > 0xFFFF)
        {
            printk(KERN_INFO "Invalid protocol type [0x%x]\n", temp_prototype);
        }
        else
        {
            if ((pentry = find_tpm_l3_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_l3_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "L3 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->l3_acl.pppoe_key.ppp_session = (uint16_t)temp_session;
                pentry->l3_acl.pppoe_key.ppp_proto   = (uint16_t)temp_prototype;
            }
        }
    }
}

/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG IPV4 KEY FUNCTIONS                                                   */
/* ======================================================================================== */

/*******************************************************************************
* sfs_tpm_cfg_set_ipv4_key_addr_rule
*
* DESCRIPTION:
*           This function adds/modifies address part of IPV4 key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv4_key_addr_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv4addpi_name=0, ipv4addpi_srcip, ipv4addpi_srcipmask, ipv4addpi_dstip, ipv4addpi_dstipmask, ipv4addpi_max
    } ipv4_add_rule_parm_indx_t;
    char                     temp_srcip[30];
    char                     temp_srcip_mask[30];
    char                     temp_dstip[30];
    char                     temp_dstip_mask[30];
    uint32_t                 srcip[4];
    uint32_t                 srcip_mask[4];
    uint32_t                 dstip[4];
    uint32_t                 dstip_mask[4];
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    tpmcfg_ipv4_key_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv4addpi_max)
    {
        parm_error_completion(numparms, ipv4addpi_max, buf, sfs_help_ipv4_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s %s %s %s", name, temp_srcip, temp_srcip_mask, temp_dstip, temp_dstip_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_srcip[%s] temp_srcip_mask[%s] temp_dstip[%s] temp_dstip_mask[%s]\n",
//               len, parsedargs, name, temp_srcip, temp_srcip_mask, temp_dstip, temp_dstip_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_ipv4_address(temp_srcip, srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip [%s]\n", temp_srcip);
        }
        else if (parse_ipv4_address(temp_srcip_mask, srcip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip mask [%s]\n", temp_srcip_mask);
        }
        else if (parse_ipv4_address(temp_dstip, dstip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dstip [%s]\n", temp_dstip);
        }
        else if (parse_ipv4_address(temp_dstip_mask, dstip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid dstip mask [%s]\n", temp_dstip_mask);
        }
        else
        {
            if ((pentry = find_tpm_ipv4_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_ipv4_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "IPV4 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->ipv4_acl.ipv4_src_ip_add[0] = (uint8_t)srcip[0]; pentry->ipv4_acl.ipv4_src_ip_add[1] = (uint8_t)srcip[1];
                pentry->ipv4_acl.ipv4_src_ip_add[2] = (uint8_t)srcip[2]; pentry->ipv4_acl.ipv4_src_ip_add[3] = (uint8_t)srcip[3];
                pentry->ipv4_acl.ipv4_src_ip_add_mask[0] = (uint8_t)srcip_mask[0]; pentry->ipv4_acl.ipv4_src_ip_add_mask[1] = (uint8_t)srcip_mask[1];
                pentry->ipv4_acl.ipv4_src_ip_add_mask[2] = (uint8_t)srcip_mask[2]; pentry->ipv4_acl.ipv4_src_ip_add_mask[3] = (uint8_t)srcip_mask[3];
                pentry->ipv4_acl.ipv4_dst_ip_add[0] = (uint8_t)dstip[0]; pentry->ipv4_acl.ipv4_dst_ip_add[1] = (uint8_t)dstip[1];
                pentry->ipv4_acl.ipv4_dst_ip_add[2] = (uint8_t)dstip[2]; pentry->ipv4_acl.ipv4_dst_ip_add[3] = (uint8_t)dstip[3];
                pentry->ipv4_acl.ipv4_dst_ip_add_mask[0] = (uint8_t)dstip_mask[0]; pentry->ipv4_acl.ipv4_dst_ip_add_mask[1] = (uint8_t)dstip_mask[1];
                pentry->ipv4_acl.ipv4_dst_ip_add_mask[2] = (uint8_t)dstip_mask[2]; pentry->ipv4_acl.ipv4_dst_ip_add_mask[3] = (uint8_t)dstip_mask[3];
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_ipv4_key_port_rule
*
* DESCRIPTION:
*           This function adds/modifies port part of IPV4 key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv4_key_port_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv4portpi_name=0, ipv4portpi_srcport, ipv4portpi_dstport, ipv4portpi_max
    } ipv4port_rule_parm_indx_t;
    unsigned int             temp_src_port;
    unsigned int             temp_dst_port;
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    tpmcfg_ipv4_key_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv4portpi_max)
    {
        parm_error_completion(numparms, ipv4portpi_max, buf, sfs_help_ipv4_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d %d", name, &temp_src_port, &temp_dst_port);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_src_port[%d] temp_dst_port[%d]\n",
//               len, parsedargs, name, temp_src_port, temp_dst_port);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_src_port > 0xFFFF)
        {
            printk(KERN_INFO "Invalid srcport [%d]\n", temp_src_port);
        }
        else if (temp_dst_port > 0xFFFF)
        {
            printk(KERN_INFO "Invalid dstport [%d]\n", temp_dst_port);
        }
        else
        {
            if ((pentry = find_tpm_ipv4_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_ipv4_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "IPV4 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->ipv4_acl.l4_src_port = (uint16_t)temp_src_port;
                pentry->ipv4_acl.l4_dst_port = (uint16_t)temp_dst_port;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_ipv4_key_protocol_rule
*
* DESCRIPTION:
*           This function adds/modifies protocol part of IPV4 ACL rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv4_key_protocol_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv4protopi_name=0, ipv4protopi_protocol, ipv4protopi_max
    } ipv4proto_rule_parm_indx_t;
    unsigned int             temp_protocol;
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    tpmcfg_ipv4_key_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv4protopi_max)
    {
        parm_error_completion(numparms, ipv4protopi_max, buf, sfs_help_ipv4_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d", name, &temp_protocol);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_protocol[0x%x]\n",
//               len, parsedargs, name, temp_protocol);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_protocol > 255)
        {
            printk(KERN_INFO "Invalid protocol [%d]\n", temp_protocol);
        }
        else
        {
            if ((pentry = find_tpm_ipv4_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_ipv4_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "IPV4 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->ipv4_acl.ipv4_proto  = (uint8_t)temp_protocol;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_ipv4_key_dscp_rule
*
* DESCRIPTION:
*           This function adds/modifies DSCP part of IPV4 ACL rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv4_key_dscp_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv4dscppi_name=0, ipv4dscppi_dscp, ipv4dscppi_dscpmask, ipv4dscppi_max
    } ipv4dscp_rule_parm_indx_t;
    unsigned int             temp_dscp;
    unsigned int             temp_dscp_mask;
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    tpmcfg_ipv4_key_entry_t  *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv4dscppi_max)
    {
        parm_error_completion(numparms, ipv4dscppi_max, buf, sfs_help_ipv4_key_cfg);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %d 0x%x", name, &temp_dscp, &temp_dscp_mask);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s] temp_dscp[%d] temp_dscp_mask[0x%x]\n",
//               len, parsedargs, name, temp_dscp, temp_dscp_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_dscp > 63)
        {
            printk(KERN_INFO "Invalid dscp [%d]\n", temp_dscp);
        }
        else if (temp_dscp_mask > 0xFF)
        {
            printk(KERN_INFO "Invalid dscp mask [0x%x]\n", temp_dscp_mask);
        }
        else
        {
            if ((pentry = find_tpm_ipv4_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_ipv4_key_entry();

            if (pentry == 0)
            {
                printk(KERN_INFO "IPV4 key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->ipv4_acl.ipv4_dscp      = (uint8_t)temp_dscp;
                pentry->ipv4_acl.ipv4_dscp_mask = (uint8_t)temp_dscp_mask;
            }
        }
    }
}


/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG IPV6 KEY FUNCTIONS                                                   */
/* ======================================================================================== */



/********************************************************************************************/
/* =========================================================================================*/
/*             TPM CFG IPV6 GEN KEY FUNCTIONS                                             */
/* ======================================================================================== */

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_gen_key_sip_rule
*
* DESCRIPTION:
*           This function adds/modifies source ip address part of IPV6 gen key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_gen_key_sip_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6gen_name=0, ipv6gen_srcip, ipv6gen_srcipmask, ipv6gen_max
    } ipv6gensip_rule_parm_indx_t;
    char     temp_srcip[60];
    char     temp_srcip_mask[60];
    uint32_t srcip[16];
    uint32_t srcip_mask[16];
    char     name[100];
    int      parsedargs;
    int      numparms;
    int      indx;
    tpmcfg_ipv6_gen_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv6gen_max)
    {
        parm_error_completion(numparms, ipv6gen_max, buf, sfs_help_ipv6_gen_key_cfg);
    }
    else
    {
        parsedargs = sscanf(buf, "%s %s %s", name, temp_srcip, temp_srcip_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_ipv6_address(temp_srcip, srcip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip [%s]\n", temp_srcip);
        }
        else if (parse_ipv6_address(temp_srcip_mask, srcip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid srcip mask [%s]\n", temp_srcip_mask);
        }
        else
        {
            if ((pentry = find_tpm_ipv6_gen_key_entry_by_name(name)) == 0)
            {
                pentry = find_free_tpm_ipv6_gen_key_entry();
            }
            if (pentry == 0)
            {
                printk(KERN_INFO "IPV6 gen key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                for (indx = 0; indx < 16; indx++)
                {
                    pentry->ipv6gen_acl.ipv6_src_ip_add[indx]      = (uint8_t)srcip[indx];
                    pentry->ipv6gen_acl.ipv6_src_ip_add_mask[indx] = (uint8_t)srcip_mask[indx];
                }
            }
        }
    }
}
/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_ctc_cm_key_rule
*
* DESCRIPTION:
*           This function adds/modifies CTC CnM of IPV6 gen key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_ctc_cm_key_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6ctc_name=0, ipv6gen_name, ipv6dip_name, ipv6l4_name, ipv6nh, ipv6hoplimit, ipv6ctc_max
    } ipv6ctc_rule_parm_indx_t;
    uint32_t ipv6_nh;
    uint32_t ipv6_hoplimit;
    char     ctc_key_name[40];
    char     gen_key_name[40];
    char     dip_key_name[40];
    char     l4_key_name[40];
    int      parsedargs;
    int      numparms;
    int      indx;
    tpmcfg_ipv6_key_entry_t *pctc_entry;
    tpmcfg_ipv6_gen_key_entry_t *pgen_entry;
    tpmcfg_ipv6_dip_key_entry_t *pdip_entry;
    tpmcfg_ipv6_l4_ports_key_entry_t  *pl4_entry;

    numparms = count_parameters(buf);
    if (numparms != ipv6ctc_max)
    {
        parm_error_completion(numparms, ipv6ctc_max, buf, sfs_help_ipv6_ctc_cm_key_cfg);
    }
    else
    {
        parsedargs = sscanf(buf, "%s %s %s %s %d %d", ctc_key_name, gen_key_name, dip_key_name,
                            l4_key_name, &ipv6_nh, &ipv6_hoplimit);

        if (strlen(ctc_key_name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", ctc_key_name);
        }
        else if (strlen(gen_key_name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", gen_key_name);
        }
        else if (strlen(dip_key_name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", dip_key_name);
        }
        else if (strlen(l4_key_name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", l4_key_name);
        }
        else if (ipv6_nh > 0xff)
        {
            printk(KERN_INFO "Invalid ipv6_nh [%d]\n", ipv6_nh);
        }
        else if (ipv6_hoplimit > 0xff)
        {
            printk(KERN_INFO "Invalid ipv6_hoplimit [%d]\n", ipv6_hoplimit);
        }
        else
        {
            if (    ((pgen_entry = find_tpm_ipv6_gen_key_entry_by_name(gen_key_name)) == 0)
                  && (strcmp(gen_key_name, ipv6_gen_key_empty_name) != 0))
            {
		printk(KERN_INFO "ipv6 gen key entry [%s] not found\n", gen_key_name);
		return;
            }
            if (    ((pdip_entry = find_tpm_ipv6_dip_key_entry_by_name(dip_key_name)) == 0)
                  && (strcmp(dip_key_name, ipv6_dip_key_empty_name) != 0))
            {
		printk(KERN_INFO "ipv6 dip key entry [%s] not found\n", dip_key_name);
		return;
            }
            if (    ((pl4_entry = find_tpm_ipv6_l4_ports_key_entry_by_name(l4_key_name)) == 0)
                  && (strcmp(l4_key_name, ipv6_l4_key_empty_name) != 0))
            {
		printk(KERN_INFO "ipv6 l4 key entry [%s] not found\n", l4_key_name);
		return;
            }

            if ((pctc_entry = find_tpm_ipv6_key_entry_by_name(ctc_key_name)) == 0)
            {
                pctc_entry = find_free_tpm_ipv6_key_entry();
            }
            if (pctc_entry == 0)
            {
                printk(KERN_INFO "IPV6 key DB full\n");
                return;
            }
            else
            {
                strcpy(pctc_entry->name, ctc_key_name);
                for (indx = 0; indx < 16; indx++)
                {
                    if (pdip_entry) {
		        pctc_entry->ipv6_acl.ipv6_dst_ip_add[indx]	= (uint8_t)pdip_entry->ipv6_addr.ipv6_ip_add[indx];
		        pctc_entry->ipv6_acl.ipv6_dst_ip_add_mask[indx] = (uint8_t)pdip_entry->ipv6_addr.ipv6_ip_add_mask[indx];
		    }
                    if (pgen_entry) {
			pctc_entry->ipv6_acl.ipv6_src_ip_add[indx]      = (uint8_t)pgen_entry->ipv6gen_acl.ipv6_src_ip_add[indx];
			pctc_entry->ipv6_acl.ipv6_src_ip_add_mask[indx] = (uint8_t)pgen_entry->ipv6gen_acl.ipv6_src_ip_add_mask[indx];
		    }
                }
		if (pgen_entry) {
		    pctc_entry->ipv6_acl.ipv6_dscp_mask = (uint8_t)pgen_entry->ipv6gen_acl.ipv6_dscp_mask;
		    pctc_entry->ipv6_acl.ipv6_dscp = (uint8_t)pgen_entry->ipv6gen_acl.ipv6_dscp;
		}
		if (pl4_entry) {
		    pctc_entry->ipv6_acl.l4_src_port = (uint16_t)pl4_entry->l4_ports.l4_src_port;
		    pctc_entry->ipv6_acl.l4_dst_port = (uint16_t)pl4_entry->l4_ports.l4_dst_port;
		}
		pctc_entry->ipv6_acl.ipv6_next_header= (uint8_t)ipv6_nh;
		pctc_entry->ipv6_acl.ipv6_hoplimit = (uint8_t)ipv6_hoplimit;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_dip_key_dip_rule
*
* DESCRIPTION:
*           This function adds/modifies source ip address part of IPV6 dip key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_dip_key_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6dip_name=0, ipv6dip_ip, ipv6dip_ipmask, ipv6dip_max
    } ipv6dip_rule_parm_indx_t;
    char     temp_ip[60];
    char     temp_ip_mask[60];
    uint32_t ip[16];
    uint32_t ip_mask[16];
    char     name[100];
    int      parsedargs;
    int      numparms;
    int      indx;
    tpmcfg_ipv6_dip_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv6dip_max)
    {
        parm_error_completion(numparms, ipv6dip_max, buf, sfs_help_ipv6_dip_key_cfg);
    }
    else
    {
        parsedargs = sscanf(buf, "%s %s %s", name, temp_ip, temp_ip_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (parse_ipv6_address(temp_ip, ip) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid ip [%s]\n", temp_ip);
        }
        else if (parse_ipv6_address(temp_ip_mask, ip_mask) == GT_FALSE)
        {
            printk(KERN_INFO "Invalid ip mask [%s]\n", temp_ip_mask);
        }
        else
        {
            if ((pentry = find_tpm_ipv6_dip_key_entry_by_name(name)) == 0)
            {
                pentry = find_free_tpm_ipv6_dip_key_entry();
            }
            if (pentry == 0)
            {
                printk(KERN_INFO "IPV6 dip key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                for (indx = 0; indx < 16; indx++)
                {
                    pentry->ipv6_addr.ipv6_ip_add[indx]      = (uint8_t)ip[indx];
                    pentry->ipv6_addr.ipv6_ip_add_mask[indx] = (uint8_t)ip_mask[indx];
                }
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_l4_key_rule
*
* DESCRIPTION:
*           This function adds/modifies source ip address part of IPV6 dip key rule
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_l4_key_rule(const char *buf, size_t len)
{
    typedef enum {
        ipv6l4key_name=0, ipv6l4key_srcport, ipv6l4key_dstport, ipv6l4key_max
    } ipv6l4_key_parm_indx_t;
    uint32_t src_port, dst_port;
    char     name[100];
    int      parsedargs;
    int      numparms;
    tpmcfg_ipv6_l4_ports_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv6l4key_max)
        parm_error_completion(numparms, ipv6l4key_max, buf, sfs_help_ipv6_l4_key_cfg);
    else {
        parsedargs = sscanf(buf, "%s %d %d", name, &src_port, &dst_port);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
            printk(KERN_INFO "Name too long [%s]\n", name);
        else {
            if ((pentry = find_tpm_ipv6_l4_ports_key_entry_by_name(name)) == 0)
                pentry = find_free_tpm_ipv6_l4_ports_key_entry();

            if (pentry == 0)
                printk(KERN_INFO "IPV6 L4 ports key DB full\n");
            else {
                strcpy(pentry->name, name);
				pentry->l4_ports.l4_dst_port = dst_port;
				pentry->l4_ports.l4_src_port = src_port;
            }
        }
    }
}

/*******************************************************************************
* sfs_tpm_cfg_set_ipv6_gen_key_dscp_rule
*
* DESCRIPTION:
*           This function adds/modifies DSCP part of IPV6 ACL rule
*
*******************************************************************************/
void sfs_tpm_cfg_set_ipv6_gen_key_dscp_rule  (const char *buf, size_t len)
{
    typedef enum
    {
        ipv6gendscp_name=0, ipv6gendscp_dscp, ipv6gendscp_dscpmask, ipv6gendscp_max
    } ipv6gendscp_rule_parm_indx_t;
    unsigned int             temp_dscp;
    unsigned int             temp_dscp_mask;
    char                     name[100];
    int                      parsedargs;
    int                      numparms;
    tpmcfg_ipv6_gen_key_entry_t *pentry;

    numparms = count_parameters(buf);
    if (numparms != ipv6gendscp_max)
    {
        parm_error_completion(numparms, ipv6gendscp_max, buf, sfs_help_ipv6_gen_key_cfg);
    }
    else
    {
        parsedargs = sscanf(buf, "%s %d 0x%x", name, &temp_dscp, &temp_dscp_mask);

        if (strlen(name) > DB_TPMCFG_MAX_NAME)
        {
            printk(KERN_INFO "Name too long [%s]\n", name);
        }
        else if (temp_dscp > 63)
        {
            printk(KERN_INFO "Invalid dscp [%d]\n", temp_dscp);
        }
        else if (temp_dscp_mask > 0xFF)
        {
            printk(KERN_INFO "Invalid dscp mask [0x%x]\n", temp_dscp_mask);
        }
        else
        {
            if ((pentry = find_tpm_ipv6_gen_key_entry_by_name(name)) == 0)
            {
                pentry = find_free_tpm_ipv6_gen_key_entry();
            }
            if (pentry == 0)
            {
                printk(KERN_INFO "IPV6 gen key DB full\n");
            }
            else
            {
                strcpy(pentry->name, name);
                pentry->ipv6gen_acl.ipv6_dscp      = (uint8_t)temp_dscp;
                pentry->ipv6gen_acl.ipv6_dscp_mask = (uint8_t)temp_dscp_mask;
            }
        }
    }
}

typedef struct
{
    char    *table_name;
    GT_BOOL (*clear_rule)(char *buf);
    void    (*display_table)(void);
} table_rule_operation_t;

static table_rule_operation_t table_rule_operation_ara[] =
{
    {"frwd",         del_tpm_pkt_frwd_entry_by_name,          show_tpm_pkt_frwd_db},
    {"vlan",         del_tpm_vlan_entry_by_name,              show_tpm_vlan_db},
    {"mod",          del_tpm_mod_entry_by_name,               show_tpm_mod_db},
    {"l2key",        del_tpm_l2_key_entry_by_name,            show_tpm_l2_key_db},
    {"l3key",        del_tpm_l3_key_entry_by_name,            show_tpm_l3_key_db},
    {"ipv4key",      del_tpm_ipv4_key_entry_by_name,          show_tpm_ipv4_key_db},
    // The following "duplicate" entries for alternate access to mdelete entry functionality
    {"l2",           del_tpm_l2_key_entry_by_name,            show_tpm_l2_key_db},
    {"l3",           del_tpm_l3_key_entry_by_name,            show_tpm_l3_key_db},
    {"ipv4",         del_tpm_ipv4_key_entry_by_name,          show_tpm_ipv4_key_db},
    {"ipv6_dip",     del_tpm_ipv6_dip_key_entry_by_name,      show_tpm_ipv6_dip_key_db},
    {"ipv6_gen",     del_tpm_ipv6_gen_key_entry_by_name,      show_tpm_ipv6_gen_key_db},
    {"ipv6_l4",      del_tpm_ipv6_l4_ports_key_entry_by_name, show_tpm_ipv6_l4_ports_key_db},
    {"ipv6_ctc",     del_tpm_ipv6_key_entry_by_name,          show_tpm_ipv6_key_db},
};
static int num_table_operations = sizeof(table_rule_operation_ara)/sizeof(table_rule_operation_ara[0]);

/*******************************************************************************
* sfs_tpm_cfg_set_key_rule_delete
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_key_rule_delete  (const char *buf, size_t len)
{
    typedef enum
    {
        ruleclearpi_table=0, ruleclearpi_rule, ruleclearpi_max
    } ruleclear_rule_parm_indx_t;
    char                     table[100];
    char                     rulename[100];
    int                      parsedargs;
    int                      numparms;
    int                      indx;
    GT_BOOL                  found = GT_FALSE;
    GT_BOOL                  rc;
    table_rule_operation_t   *ptableop;

    numparms = count_parameters(buf);
    if (numparms != ruleclearpi_max)
    {
        parm_error_completion(numparms, ruleclearpi_max, buf, sfs_help_key_rule_delete);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s %s", table, rulename);
//        printk(KERN_INFO "len=%d, parsedargs=%d. table[%s] rulename[%s]\n",
//               len, parsedargs, table, rulename);

        for (indx = 0; indx < num_table_operations; indx++)
        {
            ptableop = &table_rule_operation_ara[indx];
            if (!strcmp(table, ptableop->table_name))
            {
                found = GT_TRUE;
                rc = (ptableop->clear_rule)(rulename);
                break;
            }
        }

        if (found == GT_FALSE)
        {
            printk("%s: Table %s not found\n", __FUNCTION__, table);
        }
        else if (rc == GT_FALSE)
        {
            printk("%s: Table %s does not have entry named %s\n", __FUNCTION__, table, rulename);
        }
//        else
//        {
//            (ptableop->display_table)();
//        }
    }
}


/*******************************************************************************
* sfs_tpm_cfg_set_rule_table_display
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_rule_table_display  (const char *buf, size_t len)
{
    typedef enum
    {
        ruledisppi_table=0, ruledisppi_table_max
    } ruledisp_rule_parm_indx_t;
    char                     table[100];
    int                      parsedargs;
    int                      numparms;
    int                      indx;
    GT_BOOL                  found = GT_FALSE;
    table_rule_operation_t   *ptableop;

    numparms = count_parameters(buf);

    if (numparms == 1 && buf[0] == '?')
    {
#ifdef CONFIG_MV_TPM_SYSFS_HELP
        char helpbuf[1000];
        int  helpbuf_len;

        helpbuf_len = sfs_help_rule_table_display(helpbuf);
        print_help_buffer(helpbuf, helpbuf_len);
#else
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", ruledisppi_table_max, numparms);
#endif
    }
    else if (numparms != 1)
    {
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", ruledisppi_table_max, numparms);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s", table);
//        printk(KERN_INFO "len=%d, parsedargs=%d. table[%s]\n", len, parsedargs, table);

        for (indx = 0; indx < num_table_operations; indx++)
        {
            ptableop = &table_rule_operation_ara[indx];
            if (!strcmp(table, ptableop->table_name))
            {
                found = GT_TRUE;
                (ptableop->display_table)();
                break;
            }
        }

        if (found == GT_FALSE)
        {
            printk("%s: Table %s not found\n", __FUNCTION__, table);
        }
    }
}

/*******************************************************************************
* delete_rule_from_table
*
* DESCRIPTION:
*           This function deletes entry in rule table
*
*
*******************************************************************************/
static void delete_rule_from_table(char *table, const char *buf, size_t len)
{
    typedef enum
    {
        ruledelpi_rule=0, ruledelpi_max
    } ruledel_rule_parm_indx_t;
    char                     rulename[100];
    int                      parsedargs;
    int                      numparms;
    int                      indx;
    GT_BOOL                  found = GT_FALSE;
    GT_BOOL                  rc;
    table_rule_operation_t   *ptableop;

    numparms = count_parameters(buf);
    if (numparms == 1 && buf[0] == '?')
    {
        printk(KERN_INFO "echo [rule_name] > delete_entry_%s_rule_table\n", table);
    }
    else if (numparms != 1)
    {
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", ruledelpi_max, numparms);
    }
    else
    {
        // Get parameters
        parsedargs = sscanf(buf, "%s", rulename);
//        printk(KERN_INFO "len=%d, parsedargs=%d. rulename[%s]\n", len, parsedargs, rulename);

        for (indx = 0; indx < num_table_operations; indx++)
        {
            ptableop = &table_rule_operation_ara[indx];
            if (!strcmp(table, ptableop->table_name))
            {
                found = GT_TRUE;
                rc = (ptableop->clear_rule)(rulename);
                break;
            }
        }

        if (found == GT_FALSE)
        {
            printk("%s: Table %s not found\n", __FUNCTION__, table);
        }
        else if (rc == GT_FALSE)
        {
            printk("%s: Table %s does not have entry named %s\n", __FUNCTION__, table, rulename);
        }
    }

}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_l2_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_l2_rule_table  (const char *buf, size_t len)
{
    delete_rule_from_table("l2key", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_l3_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_l3_rule_table  (const char *buf, size_t len)
{
    delete_rule_from_table("l3key", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_ipv4_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_ipv4_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("ipv4key", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_ipv6_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_ipv6_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("ipv6key", buf, len);
}
/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_ipv6_dip_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_ipv6_dip_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("ipv6_dip", buf, len);
}


/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_ipv6_gen_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_ipv6_gen_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("ipv6_gen", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_ipv6_gen_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_ipv6_l4_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("ipv6_l4", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_frwd_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_frwd_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("frwd", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_vlan_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_vlan_rule_table(const char *buf, size_t len)
{
    delete_rule_from_table("vlan", buf, len);
}

/*******************************************************************************
* sfs_tpm_cfg_set_delete_entry_mod_rule_table
*
* DESCRIPTION:
*           This function removes named rule from specified table
*
* See sfs_tpm_cfg_set_frwd_rule() for INPUTS, OUTPUTS, RETURNS and COMMENTS details
*
*******************************************************************************/
void sfs_tpm_cfg_set_delete_entry_mod_rule_table (const char *buf, size_t len)
{
    delete_rule_from_table("mod", buf, len);
}




#if 0
// The show_tpm_XYZ_key_db() routines must be made more flexible - take char *buf parameter
// and return number of characters to print.
// If called from the functions below, the buf argument is passed
// If called from a store function, the argument wll be zero and the routine uses a local buffer
// Messy, this way or that - since the file may be too big for sysfs store invocation
/*******************************************************************************
**
**  sfs_show_l2_rule_table
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
int sfs_show_l2_rule_table   (char *buf)
{
    show_tpm_l2_key_db();
}

/*******************************************************************************
**
**  sfs_show_l3_rule_table - see header of sfs_show_l2_rule_table
**
*******************************************************************************/
int sfs_show_l3_rule_table   (char *buf)
{
    show_tpm_l3_key_db();
}

/*******************************************************************************
**
**  sfs_show_ipv4_rule_table - see header of sfs_show_l2_rule_table
**
*******************************************************************************/
int sfs_show_ipv4_rule_table (char *buf)
{
    show_tpm_ipv4_key_db();
}

/*******************************************************************************
**
**  sfs_show_ipv6_rule_table - see header of sfs_tpm_cfg_index
**
*******************************************************************************/
int sfs_show_ipv6_rule_table (char *buf)
{
    show_tpm_ipv6_key_db();
}

/*******************************************************************************
**
**  sfs_show_frwd_rule_table - see header of sfs_show_l2_rule_table
**
*******************************************************************************/
int sfs_show_frwd_rule_table (char *buf)
{
    show_tpm_pkt_frwd_db()
}

/*******************************************************************************
**
**  sfs_show_vlan_rule_table - see header of sfs_show_l2_rule_table
**
*******************************************************************************/
int sfs_show_vlan_rule_table (char *buf)
{
    show_tpm_vlan_db();
}

/*******************************************************************************
**
**  sfs_show_mod_rule_table - see header of sfs_show_l2_rule_table
**
*******************************************************************************/
int sfs_show_mod_rule_table  (char *buf)
{
    show_tpm_mod_db()
}

#endif
