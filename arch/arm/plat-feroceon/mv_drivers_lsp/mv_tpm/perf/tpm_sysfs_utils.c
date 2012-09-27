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
#include "tpm_sysfs_utils.h"

tpm_mc_vid_port_vid_set_t mc_vid_key;


uint8_t g_gen_query_pkt[] =
{
	0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
	0x54, 0xE6, 0xFC, 0x63, 0x36, 0x74,
	0xda, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* reserve for Ethertype DSA tag */
	0x81, 0x00, 0x00, 0x11,
	0x08, 0x00, 0x46, 0xEE, 0x00, 0x20,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x02,
	0x82, 0x3F, 0xC0, 0xA8, 0x01, 0x01,
	0xE0, 0x00, 0x00, 0x01, 0x94, 0x04,
	0x00, 0x00, 0x11, 0x64, 0xEE, 0x9B,
	0x00, 0x00, 0x00, 0x00, 0x9E, 0xA0,
	0x01, 0x00, 0x5E, 0x00, 0x00, 0x01,
	0x00, 0x00, 0x54, 0xE6, 0xFC, 0x63,
	0x70, 0xF7, 0xDC, 0x59
};

/********************************************************************************/
/*                          Miscellaneous                   */
/********************************************************************************/

/*******************************************************************************
* print_help_buffer
*
* DESCRIPTION:
*           This function prints help buffer
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void print_help_buffer(char *helpbuf, int len)
{
#if 1
    int  indx;
    char savech;

    for (indx = 0; indx < len; indx += 1000)
    {
        if (indx + 1000 < len)
        {
            savech = helpbuf[indx + 1000];
            helpbuf[indx + 1000] = 0;
            printk(KERN_INFO "%s", &helpbuf[indx]);
            helpbuf[indx + 1000] = savech;
        }
        else
        {
            printk(KERN_INFO "%s", &helpbuf[indx]);
        }
    }
#else
    printk(KERN_INFO "%s", helpbuf);
#endif
}

/*******************************************************************************
* parm_error_completion
*
* DESCRIPTION:
*           This function prints out help message if buffer holds 1 parameter that
*           starts '?'; else it prints out incorrect number of parameters message
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void parm_error_completion(int numparms, int reqdparms, const char *buf, int (*help_text_routine)(char *))
{
    if (numparms == 1 && buf[0] == '?' && help_text_routine != NULL)
    {
        char helpbuf[4000];
        int  helpbuf_len;

        helpbuf_len = help_text_routine(helpbuf);
        print_help_buffer(helpbuf, helpbuf_len);
    }
    else
    {
        printk(KERN_INFO "Parse problem: parameters expected/found %d/%d\n", reqdparms, numparms);
    }
}

/*******************************************************************************
* count_parameters
*
* DESCRIPTION:
*           This function counts the number of parameters provided
* INPUTS:
*       buf	- buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       int
*
* COMMENTS:
*       None.
*
*******************************************************************************/
int count_parameters (const char *buf)
{
    char                bufcopy[513];
    char                **ap, *argv[20];
    char                *inputstring = bufcopy;
    int                 numparms = 0;

    strcpy(bufcopy, buf);

    for (ap = argv; ap < &argv[15] && (*ap = strsep(&inputstring, " \t")) != NULL;)
    {
        if (**ap != '\0')
        {
            ap++;
            numparms++;
        }
    }
    return numparms;
}

/*******************************************************************************
* print_horizontal_line
*
* DESCRIPTION:
*           This function prints a horizontal line using the "=" sign and places CRLF
* INPUTS:
*       char_count  - the horizontal line length
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       int
*
* COMMENTS:
*       None.
*
*******************************************************************************/
void print_horizontal_line(unsigned int char_count)
{
    unsigned int cnt;
    for (cnt = 0; cnt < char_count; cnt++)
        printk("=");
    printk("\n");
}

/********************************************************************************/
/*                          String-int map lookups                              */
/********************************************************************************/


/*******************************************************************************
* map_string_to_value
*
* DESCRIPTION:
*           This function maps a string in a given map table to its value
* INPUTS:
*       buf	- buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       int
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static GT_BOOL map_string_to_value (string_enum_pair_db_t *db, char *str, unsigned int *value)
{
    int                indx;
    string_enum_pair_t *pentry;

    for (indx = 0; indx < db->num_entries; indx++)
    {
        pentry = &db->string_enum_pair[indx];
        if (strcmp(str, pentry->str) == 0)
        {
            *value = pentry->value;
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/*******************************************************************************
* map_value_to_string
*
* DESCRIPTION:
*           This function maps a value to string in a given map table
* INPUTS:
*       value	- int enumaeration
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       char *. The matching string or "<unknown>"
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static char *map_value_to_string (string_enum_pair_db_t *db, int value)
{
    int                indx;
    string_enum_pair_t *pentry;
    static char *unknown_str = "<unknown>";

    for (indx = 0; indx < db->num_entries; indx++)
    {
        pentry = &db->string_enum_pair[indx];
        if (value == pentry->value)
        {
            return pentry->str;
        }
    }
    return unknown_str;
}

//--------------------------------------------------------------------------

static string_enum_pair_t phase_string_enum_pair_ara[] =
{
    {STAGE_L2_PRIM,  "l2"   },
    {STAGE_L3_TYPE,  "l3"   },
    {STAGE_IPv4,     "ipv4" },
    {STAGE_IPv6_GEN,   "ipv6_gen" },
    {STAGE_IPv6_DIP,   "ipv6_dip" },
    {STAGE_IPv6_NH,    "ipv6_nh"  },
	{STAGE_IPV6_L4,    "ipv6_l4"  },
    {STAGE_CTC_CM,    "ctc_cm"  },
    {STAGE_DONE,     "done" },
};

static string_enum_pair_db_t  phase_string_enum_pair_db =
{
    phase_string_enum_pair_ara,
    sizeof(phase_string_enum_pair_ara)/sizeof(phase_string_enum_pair_ara[0])
};

GT_BOOL get_phase_value(char *str, unsigned int *value)
{
    return map_string_to_value(&phase_string_enum_pair_db, str, value);
}

//---------------------------------------------------------------------------

static string_enum_pair_t srcport_string_enum_pair_ara[] =
{
	{TPM_SRC_PORT_UNI_0,    "UNI_0"   },
	{TPM_SRC_PORT_UNI_1,    "UNI_1"   },
	{TPM_SRC_PORT_UNI_2,    "UNI_2"   },
	{TPM_SRC_PORT_UNI_3,    "UNI_3"   },
	{TPM_SRC_PORT_UNI_4,    "UNI_4"   },
	{TPM_SRC_PORT_UNI_5,    "UNI_5"   },
	{TPM_SRC_PORT_UNI_6,    "UNI_6"   },
	{TPM_SRC_PORT_UNI_7,    "UNI_7"   },
	{TPM_SRC_PORT_UNI_VIRT, "UNI_VIRT" },
	{TPM_SRC_PORT_WAN,      "WAN"     },
	{TPM_SRC_PORT_UNI_ANY,  "UNI_ANY" },
};

static string_enum_pair_db_t  srcport_string_enum_pair_db =
{
    srcport_string_enum_pair_ara,
    sizeof(srcport_string_enum_pair_ara)/sizeof(srcport_string_enum_pair_ara[0])
};

GT_BOOL get_srcport_value(char *str, unsigned int *value)
{
    return map_string_to_value(&srcport_string_enum_pair_db, str, value);
}

//---------------------------------------------------------------------------

static string_enum_pair_t srcdir_string_enum_pair_ara[] =
{
    {TPM_DIR_DS,            "DS" },
    {TPM_DIR_US,            "US" },
};

static string_enum_pair_db_t srcdir_string_enum_pair_db =
{
    srcdir_string_enum_pair_ara,
    sizeof(srcdir_string_enum_pair_ara)/sizeof(srcdir_string_enum_pair_ara[0])
};

GT_BOOL get_srcdir_value(char *str, unsigned int *value)
{
    return map_string_to_value(&srcdir_string_enum_pair_db, str, value);
}

//-----------------------------------------------------------------------------

static string_enum_pair_t bool_string_enum_pair_ara[] =
{
    {GT_FALSE,  "false"},
    {GT_TRUE,   "true" },
};

static string_enum_pair_db_t  bool_string_enum_pair_db =
{
    bool_string_enum_pair_ara,
    sizeof(bool_string_enum_pair_ara)/sizeof(bool_string_enum_pair_ara[0])
};

GT_BOOL get_bool_value(char *str, unsigned int *value)
{
    return map_string_to_value(&bool_string_enum_pair_db, str, value);
}

//---------------------------------------------------------------------------

static string_enum_pair_t igmp_mode_string_enum_pair_ara[] =
{
    {TPM_IGMP_FRWD_MODE_DROP,       "drop" },
    {TPM_IGMP_FRWD_MODE_FORWARD,    "frwd" },
    {TPM_IGMP_FRWD_MODE_SNOOPING,   "snoop"},
};

static string_enum_pair_db_t  igmp_mode_string_enum_pair_db =
{
    igmp_mode_string_enum_pair_ara,
    sizeof(igmp_mode_string_enum_pair_ara)/sizeof(igmp_mode_string_enum_pair_ara[0])
};

GT_BOOL get_igmp_mode_value(char *str, unsigned int *value)
{
    return map_string_to_value(&igmp_mode_string_enum_pair_db, str, value);
}

//---------------------------------------------------------------------------

static string_enum_pair_t igmp_srcport_string_enum_pair_ara[] =
{
    {TPM_SRC_PORT_WAN,      "WAN"     },
    {TPM_SRC_PORT_UNI_0,    "UNI_0"   },
    {TPM_SRC_PORT_UNI_1,    "UNI_1"   },
    {TPM_SRC_PORT_UNI_2,    "UNI_2"   },
    {TPM_SRC_PORT_UNI_3,    "UNI_3"   },
};

static string_enum_pair_db_t  igmp_srcport_string_enum_pair_db =
{
    igmp_srcport_string_enum_pair_ara,
    sizeof(igmp_srcport_string_enum_pair_ara)/sizeof(igmp_srcport_string_enum_pair_ara[0])
};

GT_BOOL get_igmp_srcport_value(char *str, unsigned int *value)
{
    return map_string_to_value(&igmp_srcport_string_enum_pair_db, str, value);
}

//---------------------------------------------------------------------------

static string_enum_pair_t lx_ipvy_string_enum_pair_ara[] =
{
    {TPM_SRC_PORT_WAN,      "l2"     },
    {TPM_SRC_PORT_UNI_0,    "l3"     },
    {TPM_SRC_PORT_UNI_1,    "ipv4"   },
    {TPM_SRC_PORT_UNI_2,    "ipv6"   },
};

static string_enum_pair_db_t  lx_ipvy_string_enum_pair_db =
{
    lx_ipvy_string_enum_pair_ara,
    sizeof(lx_ipvy_string_enum_pair_ara)/sizeof(lx_ipvy_string_enum_pair_ara[0])
};

GT_BOOL get_lx_ipvy_value(char *str, unsigned int *value)
{
    return map_string_to_value(&lx_ipvy_string_enum_pair_db, str, value);
}

//---------------------------------------------------------------------------
static string_enum_pair_t direction_string_enum_pair_ara[] =
{
    {TPM_DOWNSTREAM,      "ds"  },
    {TPM_UPSTREAM,        "us"  },
};

static string_enum_pair_db_t  direction_string_enum_pair_db =
{
    direction_string_enum_pair_ara,
    sizeof(direction_string_enum_pair_ara)/sizeof(direction_string_enum_pair_ara[0])
};

GT_BOOL get_direction_value(char *str, unsigned int *value)
{
    return map_string_to_value(&direction_string_enum_pair_db, str, value);
}

//-----------------------------------------------------------------------------
static string_enum_pair_t scheduling_mode_string_enum_pair_ara[] =
{
    {TPM_PP_SCHED_STRICT,      "strict"  },
    {TPM_PP_SCHED_WRR,         "wrr"     },
};

static string_enum_pair_db_t  scheduling_mode_string_enum_pair_db =
{
    scheduling_mode_string_enum_pair_ara,
    sizeof(scheduling_mode_string_enum_pair_ara)/sizeof(scheduling_mode_string_enum_pair_ara[0])
};

GT_BOOL get_scheduling_mode_value(char *str, unsigned int *value)
{
    return map_string_to_value(&scheduling_mode_string_enum_pair_db, str, value);
}

//-----------------------------------------------------------------------------
static string_enum_pair_t sched_entity_string_enum_pair_ara[] =
{
    {TPM_TRG_PORT_WAN,       "WAN"      },
    {TPM_TRG_TCONT_0,        "TCONT_0"  },
    {TPM_TRG_TCONT_1,        "TCONT_1"  },
    {TPM_TRG_TCONT_2,        "TCONT_2"  },
    {TPM_TRG_TCONT_3,        "TCONT_3"  },
    {TPM_TRG_TCONT_4,        "TCONT_4"  },
    {TPM_TRG_TCONT_5,        "TCONT_5"  },
    {TPM_TRG_TCONT_6,        "TCONT_6"  },
    {TPM_TRG_TCONT_7,        "TCONT_7"  },
    {TPM_TRG_PORT_UNI_ANY,   "UNI_ANY"  },
};

static string_enum_pair_db_t  sched_entity_string_enum_pair_db =
{
    sched_entity_string_enum_pair_ara,
    sizeof(sched_entity_string_enum_pair_ara)/sizeof(sched_entity_string_enum_pair_ara[0])
};

GT_BOOL get_sched_entity_value(char *str, unsigned int *value)
{
    return map_string_to_value(&sched_entity_string_enum_pair_db, str, value);
}

//-----------------------------------------------------------------------------
static string_enum_pair_t vlan_op_string_enum_pair_ara[] =
{
    {0,    "none"                },
    {1,    "ext_tag_mod"         },
    {2,    "ext_tag_del"         },
    {3,    "ext_tag_ins"         },
    {4,    "ext_tag_mod_ins"     },
    {5,    "ins_2tag"            },
    {6,    "mod_2tag"            },
    {7,    "swap_tags"           },
    {8,    "del_2tags"           },
    {9,    "int_tag_mod"         },
    {10,   "ext_tag_del_int_mod" },
	{11,   "split_mod_pbit"      },
};

static string_enum_pair_db_t  vlan_op_string_enum_pair_db =
{
    vlan_op_string_enum_pair_ara,
    sizeof(vlan_op_string_enum_pair_ara)/sizeof(vlan_op_string_enum_pair_ara[0])
};

GT_BOOL get_vlan_op_value(char *str, unsigned int *value)
{
    return map_string_to_value(&vlan_op_string_enum_pair_db, str, value);
}

char *get_vlan_op_str(int value)
{
    return map_value_to_string(&vlan_op_string_enum_pair_db, value);
}

//-----------------------------------------------------------------------------

static string_enum_pair_t tpm_err_string_enum_pair_ara[] =
{
    {TPM_RC_OK,                         "TPM_RC_OK"},
    {ERR_GENERAL,                       "ERR_GENERAL"},
    {ERR_OWNER_INVALID,                 "ERR_OWNER_INVALID"},
    {ERR_SRC_PORT_INVALID,              "ERR_SRC_PORT_INVALID"},
    {ERR_RULE_NUM_INVALID,              "ERR_RULE_NUM_INVALID"},
    {ERR_RULE_IDX_INVALID,              "ERR_RULE_IDX_INVALID"},
    {ERR_PARSE_MAP_INVALID,             "ERR_PARSE_MAP_INVALID"},
    {ERR_VLAN_OP_INVALID,               "ERR_VLAN_OP_INVALID"},
    {ERR_L2_KEY_INVALID,                "ERR_L2_KEY_INVALID"},
    {ERR_FRWD_INVALID,                  "ERR_FRWD_INVALID"},
    {ERR_MOD_INVALID,                   "ERR_MOD_INVALID"},
    {ERR_ACTION_INVALID,                "ERR_ACTION_INVALID"},
    {ERR_NEXT_PHASE_INVALID,            "ERR_NEXT_PHASE_INVALID"},
    {ERR_RULE_KEY_MISMATCH,             "ERR_RULE_KEY_MISMATCH"},
    {ERR_IPV4_NO_CONT_L4,               "ERR_IPV4_NO_CONT_L4"},
    {ERR_IPV6_ADD_FAIL,                 "ERR_IPV6_ADD_FAIL"},
    {ERR_MC_STREAM_INVALID,             "ERR_MC_STREAM_INVALID"},
    {ERR_MC_STREAM_EXISTS ,             "ERR_MC_STREAM_EXISTS "},
    {ERR_MC_DST_PORT_INVALID,           "ERR_MC_DST_PORT_INVALID"},
    {ERR_IPV4_MC_DST_IP_INVALID,        "ERR_IPV4_MC_DST_IP_INVALID"},
    {ERR_IPV6_MC_DST_IP_INVALID,        "ERR_IPV6_MC_DST_IP_INVALID"},
    {ERR_OMCI_TCONT_INVALID,            "ERR_OMCI_TCONT_INVALID"},
    {ERR_MNGT_TX_Q_INVALID,             "ERR_MNGT_TX_Q_INVALID"},
    {ERR_MNGT_CREATE_DUPLICATE_CHANNEL, "ERR_MNGT_CREATE_DUPLICATE_CHANNEL"},
    {ERR_MNGT_DEL_CHANNEL_INVALID,      "ERR_MNGT_DEL_CHANNEL_INVALID"},
    {ERR_OAM_LLID_INVALID,              "ERR_OAM_LLID_INVALID"},
    {ERR_CFG_GETNEXT_INDEX_INVALID,     "ERR_CFG_GETNEXT_INDEX_INVALID"},
    {ERR_CFG_GETNEXT_DIRECTION_INVALID, "ERR_CFG_GETNEXT_DIRECTION_INVALID"},
    {ERR_CFG_GET_CHANNEL_INVALID,       "ERR_CFG_GET_CHANNEL_INVALID"},
    {ERR_SW_MAC_INVALID,                "ERR_SW_MAC_INVALID"},
    {ERR_SW_MAC_STATIC_NOT_FOUND,       "ERR_SW_MAC_STATIC_NOT_FOUND"},
    {ERR_SW_MAC_PER_PORT_INVALID,       "ERR_SW_MAC_PER_PORT_INVALID"},
    {ERR_SW_NUM_OF_MAC_PER_PORT_INVALID,"ERR_SW_NUM_OF_MAC_PER_PORT_INVALID"},
    {ERR_SW_VID_INVALID,                "ERR_SW_VID_INVALID"},
    {ERR_SW_TM_QUEUE_INVALID,           "ERR_SW_TM_QUEUE_INVALID"},
    {ERR_SW_TM_WEIGHT_INVALID,          "ERR_SW_TM_WEIGHT_INVALID"},
    {ERR_SW_TM_WRR_MODE_INVALID,        "ERR_SW_TM_WRR_MODE_INVALID"},
    {ERR_API_TYPE_INVALID,              "ERR_API_TYPE_INVALID"},
    {ERR_DIR_INVALID,                   "ERR_DIR_INVALID"},
    {ERR_NULL_POINTER,                  "ERR_NULL_POINTER"},
    {ERR_IGMP_NOT_ENABLED,              "ERR_IGMP_NOT_ENABLED"},
    {ERR_OUT_OF_RESOURCES,              "ERR_OUT_OF_RESOURCES"},
    {ERR_AI_SESS_EXISTS,                "ERR_AI_SESS_EXISTS"},
    {ERR_AI_SESS_DELETING,              "ERR_AI_SESS_DELETING"},
    {ERR_SW_TM_BUCKET_SIZE_INVALID,     "ERR_SW_TM_BUCKET_SIZE_INVALID"},
    {ERR_SW_TM_RATE_LIMIT_INVALID,      "ERR_SW_TM_RATE_LIMIT_INVALID"},
    {ERR_DELETE_KEY_INVALID,            "ERR_DELETE_KEY_INVALID"},
    {ERR_L3_KEY_INVALID,                "ERR_L3_KEY_INVALID"},
    {ERR_IPV4_KEY_INVALID,              "ERR_IPV4_KEY_INVALID"},
    {ERR_IPV6_KEY_INVALID,              "ERR_IPV6_KEY_INVALID"},
    {ERR_PORT_IGMP_FRWD_MODE_INVALID,   "ERR_PORT_IGMP_FRWD_MODE_INVALID"},
    {ERR_IPV6_API_ILLEGAL_CALL,         "ERR_IPV6_API_ILLEGAL_CALL"},
    {ERR_IPV6_5T_RULE_EXISTS,           "ERR_IPV6_5T_RULE_EXISTS"},
    {ERR_IPV6_5T_FLOW_PARTIAL_MATCHED,  "ERR_IPV6_5T_FLOW_PARTIAL_MATCHED"},
    {ERR_IPV6_5T_RULE_IN_USE,           "ERR_IPV6_5T_RULE_IN_USE"},
    {ERR_IPV6_5T_FLOW_AI_BITS_EXHAUSED, "ERR_IPV6_5T_FLOW_AI_BITS_EXHAUSED"},
    {ERR_SW_NOT_INIT,                   "ERR_SW_NOT_INIT"},
};


static string_enum_pair_db_t  tpm_err_string_enum_pair_db =
{
    tpm_err_string_enum_pair_ara,
    sizeof(tpm_err_string_enum_pair_ara)/sizeof(tpm_err_string_enum_pair_ara[0])
};

char *get_tpm_err_str(int value)
{
    return map_value_to_string(&tpm_err_string_enum_pair_db, value);
}


/*******************************************************************************

* parse_ipv4_address
*
* DESCRIPTION:
*           This function parses IPV4 address
* INPUTS:
*       buf	- buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       GT_BOOL
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL parse_ipv4_address (char *buf, uint32_t *ipv4addr_parts)
{
    if (4 == sscanf(buf, "%d.%d.%d.%d", &ipv4addr_parts[0], &ipv4addr_parts[1], &ipv4addr_parts[2], &ipv4addr_parts[3]))
    {
        if (ipv4addr_parts[0] <= 255 && ipv4addr_parts[1] <= 255 && ipv4addr_parts[2] <= 255 && ipv4addr_parts[3] <= 255)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/*******************************************************************************
* parse_ipv6_address
*
* DESCRIPTION:
*           This function parses IPV4 address
* INPUTS:
*       buf	- buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       GT_BOOL
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL parse_ipv6_address (char *buf, uint32_t *ipv6addr_parts)
{
    if (8 == sscanf(buf, "%x:%x:%x:%x:%x:%x:%x:%x",
                    &ipv6addr_parts[0], &ipv6addr_parts[2],  &ipv6addr_parts[4],  &ipv6addr_parts[6],
                    &ipv6addr_parts[8], &ipv6addr_parts[10], &ipv6addr_parts[12], &ipv6addr_parts[14]));
    {
        if (ipv6addr_parts[0] <= 0xFFFF && ipv6addr_parts[2] <= 0xFFFF  && ipv6addr_parts[4] <= 0xFFFF  && ipv6addr_parts[6] <= 0xFFFF &&
            ipv6addr_parts[8] <= 0xFFFF && ipv6addr_parts[10] <= 0xFFFF && ipv6addr_parts[12] <= 0xFFFF && ipv6addr_parts[14] <= 0xFFFF)
        {
            // Note the order
            ipv6addr_parts[1]  = ipv6addr_parts[0]  & 0xFF;   ipv6addr_parts[0]  = ipv6addr_parts[0]  >> 8;
            ipv6addr_parts[3]  = ipv6addr_parts[2]  & 0xFF;   ipv6addr_parts[2]  = ipv6addr_parts[2]  >> 8;
            ipv6addr_parts[5]  = ipv6addr_parts[4]  & 0xFF;   ipv6addr_parts[4]  = ipv6addr_parts[4]  >> 8;
            ipv6addr_parts[7]  = ipv6addr_parts[6]  & 0xFF;   ipv6addr_parts[6]  = ipv6addr_parts[6]  >> 8;
            ipv6addr_parts[9]  = ipv6addr_parts[8]  & 0xFF;   ipv6addr_parts[8]  = ipv6addr_parts[8]  >> 8;
            ipv6addr_parts[11] = ipv6addr_parts[10] & 0xFF;   ipv6addr_parts[10] = ipv6addr_parts[10] >> 8;
            ipv6addr_parts[13] = ipv6addr_parts[12] & 0xFF;   ipv6addr_parts[12] = ipv6addr_parts[12] >> 8;
            ipv6addr_parts[15] = ipv6addr_parts[14] & 0xFF;   ipv6addr_parts[14] = ipv6addr_parts[14] >> 8;

            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

/*******************************************************************************
* parse_mac_address
*
* DESCRIPTION:
*           This function parses MAC address
* INPUTS:
*       buf	- buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       GT_BOOL
*
* COMMENTS:
*       None.
*
*******************************************************************************/
GT_BOOL parse_mac_address (char *buf, uint32_t *macaddr_parts)
{
    if (6 == sscanf(buf, "%x:%x:%x:%x:%x:%x", &macaddr_parts[0], &macaddr_parts[1], &macaddr_parts[2], &macaddr_parts[3],
                    &macaddr_parts[4], &macaddr_parts[5]))
    {
        if (macaddr_parts[0] <= 255 && macaddr_parts[1] <= 255 && macaddr_parts[2] <= 255 && macaddr_parts[3] <= 255 &&
            macaddr_parts[4] <= 255 && macaddr_parts[5] <= 255)
        {
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}



/********************************************************************************/
/*                          Packet Forward rule table and API                   */
/********************************************************************************/

static tpmcfg_frwd_entry_t  tpm_sysfs_frwd_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_frwd_rule_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_frwd_entry_t),
    .entryAra           = tpm_sysfs_frwd_table
};


static void init_tpm_pkt_frwd_db(void)
{
    tpmcfg_frwd_entry_t *pentry = (tpmcfg_frwd_entry_t *)tpm_frwd_rule_db.entryAra;
    int                 indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_frwd_rule_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}


static void format_ipv4_addr(uint8_t *ipaddr, char *str)
{
    sprintf(str, "%d.%d.%d.%d", ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
}

static void format_ipv6_addr(uint8_t *ipaddr, char *str)
{
    uint16_t p1 = (ipaddr[0] << 8)  | ipaddr[1];
    uint16_t p2 = (ipaddr[2] << 8)  | ipaddr[3];
    uint16_t p3 = (ipaddr[4] << 8)  | ipaddr[5];
    uint16_t p4 = (ipaddr[6] << 8)  | ipaddr[7];
    uint16_t p5 = (ipaddr[8] << 8)  | ipaddr[9];
    uint16_t p6 = (ipaddr[10] << 8) | ipaddr[11];
    uint16_t p7 = (ipaddr[12] << 8) | ipaddr[13];
    uint16_t p8 = (ipaddr[14] << 8) | ipaddr[15];

    sprintf(str, "%04x:%04x:%04x:%04x:%04x:%04x:%04x:%04x", p1, p2, p3, p4, p5, p6, p7, p8);
}

static void format_mac_addr(uint8_t *macaddr, char *str)
{
    sprintf(str, "%02x:%02x:%02x:%02x:%02x:%02x", macaddr[0], macaddr[1], macaddr[2], macaddr[3], macaddr[4], macaddr[5]);
}

tpmcfg_frwd_entry_t *find_tpm_pkt_frwd_entry_by_name(char *name)
{
    tpmcfg_frwd_entry_t *pentry = (tpmcfg_frwd_entry_t *)tpm_frwd_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_frwd_rule_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_frwd_entry_t *find_free_tpm_pkt_frwd_entry   (void)
{
    tpmcfg_frwd_entry_t *pentry = (tpmcfg_frwd_entry_t *)tpm_frwd_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_frwd_rule_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_pkt_frwd_entry_by_name  (char *name)
{
    tpmcfg_frwd_entry_t *pentry = (tpmcfg_frwd_entry_t *)tpm_frwd_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_frwd_rule_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            memset(&pentry->frwd, 0, sizeof(pentry->frwd));
            pentry->name[0] = 0;
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_pkt_frwd_db   (void)
{
    tpmcfg_frwd_entry_t *pentry = (tpmcfg_frwd_entry_t *)tpm_frwd_rule_db.entryAra;
    int                 indx;

    printk(KERN_INFO "%16s  target-port  queue gem-port\n", "name");
    for (indx = 0; indx < tpm_frwd_rule_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            printk(KERN_INFO "%16s  0x%04x      %-3d    %d\n", pentry->name, pentry->frwd.trg_port, pentry->frwd.trg_queue, pentry->frwd.gem_port);
        }
    }
}


/********************************************************************************/
/*                          VLAN rule table and API                             */
/********************************************************************************/

static tpmcfg_vlan_entry_t  tpm_sysfs_vlan_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_vlan_rule_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_vlan_entry_t),
    .entryAra           = tpm_sysfs_vlan_table
};


static void init_tpm_vlan_db(void)
{
    tpmcfg_vlan_entry_t *pentry = (tpmcfg_vlan_entry_t *)tpm_vlan_rule_db.entryAra;
    int                 indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_vlan_rule_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_vlan_entry_t *find_tpm_vlan_entry_by_name(char *name)
{
    tpmcfg_vlan_entry_t *pentry = (tpmcfg_vlan_entry_t *)tpm_vlan_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_vlan_rule_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_vlan_entry_t *find_free_tpm_vlan_entry   (void)
{
    tpmcfg_vlan_entry_t *pentry = (tpmcfg_vlan_entry_t *)tpm_vlan_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_vlan_rule_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_vlan_entry_by_name  (char *name)
{
    tpmcfg_vlan_entry_t *pentry = (tpmcfg_vlan_entry_t *)tpm_vlan_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_vlan_rule_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->vlan, 0, sizeof(pentry->vlan));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_vlan_db   (void)
{
    tpmcfg_vlan_entry_t *pentry = (tpmcfg_vlan_entry_t *)tpm_vlan_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_vlan_rule_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            printk(KERN_INFO "%s: tpid 0x%x, tpid_mask 0x%x, VID %d/0x%x, CFI %d/0x%x, pbit %d/0x%x\n", pentry->name,
                   pentry->vlan.tpid, pentry->vlan.tpid_mask, pentry->vlan.vid, pentry->vlan.vid_mask,
                   pentry->vlan.cfi, pentry->vlan.cfi_mask, pentry->vlan.pbit, pentry->vlan.pbit_mask);
        }
    }
}

/********************************************************************************/
/*                          Mod (packet modification) rule table and API        */
/********************************************************************************/

/*
 *  Wilson:
 *   DB_TPMCFG_MAX_ENTRIES are for ACL rules while one entry,
 *   which is named as 'dbg_entry', is reserved for mod_eng debug only.
 *   The 'dbg_entry' is the last entry in 'tpm_sysfs_mod_table'.
 */

static tpmcfg_mod_entry_t  tpm_sysfs_mod_table[DB_TPMCFG_MAX_ENTRIES+1];

static tpm_generic_rule_db_t tpm_mod_rule_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_mod_entry_t),
    .entryAra           = tpm_sysfs_mod_table
};


static void init_tpm_mod_db(void)
{
    tpmcfg_mod_entry_t *pentry = (tpmcfg_mod_entry_t *)tpm_mod_rule_db.entryAra;
    int                 indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_mod_rule_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
        pentry->flags   = 0;
    }

    strcpy(pentry->name, "dbg_entry");
    pentry->flags = 0;
}



tpmcfg_mod_entry_t *find_tpm_mod_entry_by_name(char *name)
{
    tpmcfg_mod_entry_t *pentry = (tpmcfg_mod_entry_t *)tpm_mod_rule_db.entryAra;
    int                 indx;

    if (strcmp(name, "dbg_entry") == 0)
        return &(tpm_sysfs_mod_table[DB_TPMCFG_MAX_ENTRIES]);

    for (indx = 0; indx < tpm_mod_rule_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_mod_entry_t *find_free_tpm_mod_entry   (void)
{
    tpmcfg_mod_entry_t *pentry = (tpmcfg_mod_entry_t *)tpm_mod_rule_db.entryAra;
    int                 indx;

    for (indx = 0; indx < tpm_mod_rule_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_mod_entry_by_name  (char *name)
{
    tpmcfg_mod_entry_t *pentry = (tpmcfg_mod_entry_t *)tpm_mod_rule_db.entryAra;
    int                 indx;

    if (strcmp(name, "dbg_entry") == 0)
    {
        memset(&pentry->mod, 0, sizeof(pentry->mod));
        pentry->flags = 0;
        return GT_TRUE;
    }

    for (indx = 0; indx < tpm_mod_rule_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->mod, 0, sizeof(pentry->mod));
            pentry->flags = 0;
            return GT_TRUE;
        }
    }
    return GT_FALSE;
}

void show_tpm_mod_db   (void)
{
    tpmcfg_mod_entry_t  *pentry = (tpmcfg_mod_entry_t *)tpm_mod_rule_db.entryAra;
    int                 indx;
    int                 off = 0;
    char                buf[800];
    char                d1[60];
    char                d2[60];
    char                d1_mask[60];
    char                d2_mask[60];

    for (indx = 0; indx < tpm_mod_rule_db.max_num_entries+1; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: flags 0x%x\n",
                           pentry->name, pentry->flags);

            if (pentry->flags & MOD_ENTRY_FLAG_MH)
            {
                off += sprintf(buf+off, "\tMH 0x%x\n", pentry->mod.mh_mod);
            }

            if (pentry->flags & MOD_ENTRY_FLAG_MAC)
            {
                format_mac_addr(pentry->mod.mac_mod.mac_sa, d1);
                format_mac_addr(pentry->mod.mac_mod.mac_sa_mask, d1_mask);
                format_mac_addr(pentry->mod.mac_mod.mac_da, d2);
                format_mac_addr(pentry->mod.mac_mod.mac_da_mask, d2_mask);

                off += sprintf(buf+off, "\tMAC SA %s/%s, DA %s/%s\n", d1, d1_mask, d2, d2_mask);
            }

            if (pentry->flags & MOD_ENTRY_FLAG_VLAN)
            {
                off += sprintf(buf+off, "\t%-19s  VLAN_1 - tpid 0x%x, VID %d/0x%x, CFI %d/0x%x, pbit %d/0x%x\n",
                               get_vlan_op_str(pentry->mod.vlan_mod.vlan_op), pentry->mod.vlan_mod.vlan1_out.tpid,
                               pentry->mod.vlan_mod.vlan1_out.vid, pentry->mod.vlan_mod.vlan1_out.vid_mask,
                               pentry->mod.vlan_mod.vlan1_out.cfi, pentry->mod.vlan_mod.vlan1_out.cfi_mask,
                               pentry->mod.vlan_mod.vlan1_out.pbit, pentry->mod.vlan_mod.vlan1_out.pbit_mask);

                off += sprintf(buf+off, "\t                     VLAN_2 - tpid 0x%x, VID %d/0x%x, CFI %d/0x%x, pbit %d/0x%x\n",
                               pentry->mod.vlan_mod.vlan2_out.tpid,
                               pentry->mod.vlan_mod.vlan2_out.vid, pentry->mod.vlan_mod.vlan2_out.vid_mask,
                               pentry->mod.vlan_mod.vlan2_out.cfi, pentry->mod.vlan_mod.vlan2_out.cfi_mask,
                               pentry->mod.vlan_mod.vlan2_out.pbit, pentry->mod.vlan_mod.vlan2_out.pbit_mask);
            }
            if (pentry->flags & MOD_ENTRY_FLAG_IPV4)
            {
                format_ipv4_addr(pentry->mod.l3.ipv4_mod.ipv4_src_ip_add, d1);
                format_ipv4_addr(pentry->mod.l3.ipv4_mod.ipv4_src_ip_add_mask, d1_mask);
                format_ipv4_addr(pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add, d2);
                format_ipv4_addr(pentry->mod.l3.ipv4_mod.ipv4_dst_ip_add_mask, d2_mask);

                off += sprintf(buf+off, "\tIPV4: src_ip_add %s/%s, dst_ip_add %s/%s\n", d1, d1_mask, d2, d2_mask);

                off += sprintf(buf+off, "\tl3.l4_src_port %d, l3.l4_dst_port %d\n",
                               pentry->mod.l3.ipv4_mod.l4_src_port, pentry->mod.l3.ipv4_mod.l4_dst_port);
            }
            else if (pentry->flags & MOD_ENTRY_FLAG_IPV6)
            {
                format_ipv6_addr(pentry->mod.l3.ipv6_mod.ipv6_src_ip_add, d1);
                format_ipv6_addr(pentry->mod.l3.ipv6_mod.ipv6_src_ip_add_mask, d1_mask);
                format_ipv6_addr(pentry->mod.l3.ipv6_mod.ipv6_dst_ip_add, d2);
                format_ipv6_addr(pentry->mod.l3.ipv6_mod.ipv6_dst_ip_add_mask, d2_mask);

                off += sprintf(buf+off, "\tIPV6: src_ip_add %s/%s\n\t      dst_ip_add %s/%s\n", d1, d1_mask, d2, d2_mask);

                off += sprintf(buf+off, "\tl4_src_port %d, l4_dst_port %d\n",
                               pentry->mod.l3.ipv6_mod.l4_src_port, pentry->mod.l3.ipv6_mod.l4_dst_port);
            }


            if (pentry->flags & MOD_ENTRY_FLAG_PPPOE)
            {
                off += sprintf(buf+off, "\tPPPoE session %d, protocol type 0x%x\n",
                               pentry->mod.pppoe_mod.ppp_session, pentry->mod.pppoe_mod.ppp_proto);
            }

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}

/********************************************************************************/
/*                          L2 ACL table and API                                */
/********************************************************************************/

static tpmcfg_l2_key_entry_t  tpm_sysfs_l2_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_l2_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_l2_key_entry_t),
    .entryAra           = tpm_sysfs_l2_key_table
};


static void init_tpm_l2_key_db(void)
{
    tpmcfg_l2_key_entry_t *pentry = (tpmcfg_l2_key_entry_t *)tpm_l2_key_db.entryAra;
    int                   indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_l2_key_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_l2_key_entry_t *find_tpm_l2_key_entry_by_name(char *name)
{
    tpmcfg_l2_key_entry_t *pentry = (tpmcfg_l2_key_entry_t *)tpm_l2_key_db.entryAra;
    int                   indx;

    for (indx = 0; indx < tpm_l2_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_l2_key_entry_t *find_free_tpm_l2_key_entry   (void)
{
    tpmcfg_l2_key_entry_t *pentry = (tpmcfg_l2_key_entry_t *)tpm_l2_key_db.entryAra;
    int                   indx;

    for (indx = 0; indx < tpm_l2_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_l2_key_entry_by_name  (char *name)
{
    tpmcfg_l2_key_entry_t *pentry = (tpmcfg_l2_key_entry_t *)tpm_l2_key_db.entryAra;
    int                   indx;

    for (indx = 0; indx < tpm_l2_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->l2_acl, 0, sizeof(pentry->l2_acl));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_l2_key_db   (void)
{
    tpmcfg_l2_key_entry_t *pentry = (tpmcfg_l2_key_entry_t *)tpm_l2_key_db.entryAra;
    char                  buf[800];
    int                   indx;
    int                   off = 0;
    char                  d1[60];
    char                  d2[60];
    char                  d1_mask[60];
    char                  d2_mask[60];

    for (indx = 0; indx < tpm_l2_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: \n", pentry->name);

            off += sprintf(buf+off, "\tVLAN_1 - tpid 0x%x/0x%x, VID %d/0x%x, CFI %d/0x%x, pbit %d/0x%x\n",
                           pentry->l2_acl.vlan1.tpid, pentry->l2_acl.vlan1.tpid_mask,
                           pentry->l2_acl.vlan1.vid,  pentry->l2_acl.vlan1.vid_mask,
                           pentry->l2_acl.vlan1.cfi,  pentry->l2_acl.vlan1.cfi_mask,
                           pentry->l2_acl.vlan1.pbit, pentry->l2_acl.vlan1.pbit_mask);

            off += sprintf(buf+off, "\tVLAN_2 - tpid 0x%x/0x%x, VID %d/0x%x, CFI %d/0x%x, pbit %d/0x%x\n",
                           pentry->l2_acl.vlan2.tpid, pentry->l2_acl.vlan2.tpid_mask,
                           pentry->l2_acl.vlan2.vid,  pentry->l2_acl.vlan2.vid_mask,
                           pentry->l2_acl.vlan2.cfi,  pentry->l2_acl.vlan2.cfi_mask,
                           pentry->l2_acl.vlan2.pbit, pentry->l2_acl.vlan2.pbit_mask);

            off += sprintf(buf+off, "\tethertype 0x%x, gemport %d, PPPoE: session %d, protocol type 0x%x\n",
                           pentry->l2_acl.ether_type, pentry->l2_acl.gem_port,
                           pentry->l2_acl.pppoe_hdr.ppp_session, pentry->l2_acl.pppoe_hdr.ppp_proto);

            format_mac_addr(pentry->l2_acl.mac.mac_sa, d1);
            format_mac_addr(pentry->l2_acl.mac.mac_sa_mask, d1_mask);
            format_mac_addr(pentry->l2_acl.mac.mac_da, d2);
            format_mac_addr(pentry->l2_acl.mac.mac_da_mask, d2_mask);

            off += sprintf(buf+off, "\tMAC SA %s/%s, DA %s/%s\n", d1, d1_mask, d2, d2_mask);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}

/********************************************************************************/
/*                          L3 ACL table and API                                */
/********************************************************************************/

static tpmcfg_l3_key_entry_t  tpm_sysfs_l3_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_l3_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_l3_key_entry_t),
    .entryAra           = tpm_sysfs_l3_key_table
};


static void init_tpm_l3_key_db(void)
{
    tpmcfg_l3_key_entry_t *pentry = (tpmcfg_l3_key_entry_t *)tpm_l3_key_db.entryAra;
    int                   indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_l3_key_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_l3_key_entry_t *find_tpm_l3_key_entry_by_name(char *name)
{
    tpmcfg_l3_key_entry_t *pentry = (tpmcfg_l3_key_entry_t *)tpm_l3_key_db.entryAra;
    int                   indx;

    for (indx = 0; indx < tpm_l3_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_l3_key_entry_t *find_free_tpm_l3_key_entry   (void)
{
    tpmcfg_l3_key_entry_t *pentry = (tpmcfg_l3_key_entry_t *)tpm_l3_key_db.entryAra;
    int                   indx;

    for (indx = 0; indx < tpm_l3_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_l3_key_entry_by_name  (char *name)
{
    tpmcfg_l3_key_entry_t *pentry = (tpmcfg_l3_key_entry_t *)tpm_l3_key_db.entryAra;
    int                   indx;

    for (indx = 0; indx < tpm_l3_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->l3_acl, 0, sizeof(pentry->l3_acl));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_l3_key_db   (void)
{
    tpmcfg_l3_key_entry_t *pentry = (tpmcfg_l3_key_entry_t *)tpm_l3_key_db.entryAra;
    char                  buf[800];
    int                   indx;
    int                   off = 0;

    for (indx = 0; indx < tpm_l3_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: \n", pentry->name);

            off += sprintf(buf+off, "\tethertype 0x%x, PPPoE: session %d, protocol type 0x%x\n",
                           pentry->l3_acl.ether_type_key, pentry->l3_acl.pppoe_key.ppp_session, pentry->l3_acl.pppoe_key.ppp_proto);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}

/********************************************************************************/
/*                          IPV4 ACL table and API                              */
/********************************************************************************/

static tpmcfg_ipv4_key_entry_t  tpm_sysfs_ipv4_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_ipv4_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_ipv4_key_entry_t),
    .entryAra           = tpm_sysfs_ipv4_key_table
};


static void init_tpm_ipv4_key_db(void)
{
    tpmcfg_ipv4_key_entry_t *pentry = (tpmcfg_ipv4_key_entry_t *)tpm_ipv4_key_db.entryAra;
    int                     indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_ipv4_key_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_ipv4_key_entry_t *find_tpm_ipv4_key_entry_by_name(char *name)
{
    tpmcfg_ipv4_key_entry_t *pentry = (tpmcfg_ipv4_key_entry_t *)tpm_ipv4_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv4_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_ipv4_key_entry_t *find_free_tpm_ipv4_key_entry   (void)
{
    tpmcfg_ipv4_key_entry_t *pentry = (tpmcfg_ipv4_key_entry_t *)tpm_ipv4_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv4_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_ipv4_key_entry_by_name  (char *name)
{
    tpmcfg_ipv4_key_entry_t *pentry = (tpmcfg_ipv4_key_entry_t *)tpm_ipv4_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv4_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->ipv4_acl, 0, sizeof(pentry->ipv4_acl));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_ipv4_key_db   (void)
{
    tpmcfg_ipv4_key_entry_t *pentry = (tpmcfg_ipv4_key_entry_t *)tpm_ipv4_key_db.entryAra;
    char                    buf[800];
    int                     indx;
    int                     off = 0;
    char                    d1[60];
    char                    d2[60];
    char                    d1_mask[60];
    char                    d2_mask[60];

    for (indx = 0; indx < tpm_ipv4_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: \n",
                           pentry->name);

            off += sprintf(buf+off, "\tdscp %d/0x%x, protocol %d, src_port %d, dst_port %d\n",
                           pentry->ipv4_acl.ipv4_dscp, pentry->ipv4_acl.ipv4_dscp_mask,
                           pentry->ipv4_acl.ipv4_proto, pentry->ipv4_acl.l4_src_port, pentry->ipv4_acl.l4_dst_port);

            format_ipv4_addr(pentry->ipv4_acl.ipv4_src_ip_add, d1);
            format_ipv4_addr(pentry->ipv4_acl.ipv4_src_ip_add_mask, d1_mask);
            format_ipv4_addr(pentry->ipv4_acl.ipv4_dst_ip_add, d2);
            format_ipv4_addr(pentry->ipv4_acl.ipv4_dst_ip_add_mask, d2_mask);

            off += sprintf(buf+off, "\tsrc_ip_add %s/%s, dst_ip_add %s/%s\n", d1, d1_mask, d2, d2_mask);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}

/********************************************************************************/
/*                          IPV6 ACL table and API        */
/********************************************************************************/

static tpmcfg_ipv6_key_entry_t  tpm_sysfs_ipv6_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_ipv6_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_ipv6_key_entry_t),
    .entryAra           = tpm_sysfs_ipv6_key_table
};

static void init_tpm_ipv6_key_db(void)
{
    tpmcfg_ipv6_key_entry_t *pentry = (tpmcfg_ipv6_key_entry_t *)tpm_ipv6_key_db.entryAra;
    int                     indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_ipv6_key_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_ipv6_key_entry_t *find_tpm_ipv6_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_key_entry_t *pentry = (tpmcfg_ipv6_key_entry_t *)tpm_ipv6_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_ipv6_key_entry_t *find_free_tpm_ipv6_key_entry   (void)
{
    tpmcfg_ipv6_key_entry_t *pentry = (tpmcfg_ipv6_key_entry_t *)tpm_ipv6_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL        del_tpm_ipv6_key_entry_by_name  (char *name)
{
    tpmcfg_ipv6_key_entry_t *pentry = (tpmcfg_ipv6_key_entry_t *)tpm_ipv6_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->ipv6_acl, 0, sizeof(pentry->ipv6_acl));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_ipv6_key_db   (void)
{
    tpmcfg_ipv6_key_entry_t *pentry = (tpmcfg_ipv6_key_entry_t *)tpm_ipv6_key_db.entryAra;
    char                    buf[800];
    int                     indx;
    int                     off = 0;
    char                    d1[60];
    char                    d2[60];
    char                    d1_mask[60];
    char                    d2_mask[60];

    for (indx = 0; indx < tpm_ipv6_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: \n", pentry->name);

            off += sprintf(buf+off, "\tdscp %d/0x%x, next_header %d, src_port %d, dst_port %d\n",
                           pentry->ipv6_acl.ipv6_dscp, pentry->ipv6_acl.ipv6_dscp_mask,
                           pentry->ipv6_acl.ipv6_next_header, pentry->ipv6_acl.l4_src_port, pentry->ipv6_acl.l4_dst_port);

            format_ipv6_addr(pentry->ipv6_acl.ipv6_src_ip_add, d1);
            format_ipv6_addr(pentry->ipv6_acl.ipv6_src_ip_add_mask, d1_mask);
            format_ipv6_addr(pentry->ipv6_acl.ipv6_dst_ip_add, d2);
            format_ipv6_addr(pentry->ipv6_acl.ipv6_dst_ip_add_mask, d2_mask);

            off += sprintf(buf+off, "\tsrc_ip_add %s/%s, dst_ip_add %s/%s\n", d1, d1_mask, d2, d2_mask);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}


/********************************************************************************/
/*                          IPV6 gen ACL table and API        */
/********************************************************************************/

static tpmcfg_ipv6_key_entry_t tpm_sysfs_ipv6_gen_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_ipv6_gen_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_ipv6_key_entry_t),
    .entryAra           = tpm_sysfs_ipv6_gen_key_table
};


static void init_tpm_ipv6_gen_key_db(void)
{
    tpmcfg_ipv6_gen_key_entry_t *pentry = (tpmcfg_ipv6_gen_key_entry_t *)tpm_ipv6_gen_key_db.entryAra;
    int                     indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_ipv6_gen_key_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_ipv6_gen_key_entry_t *find_tpm_ipv6_gen_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_gen_key_entry_t *pentry = (tpmcfg_ipv6_gen_key_entry_t *)tpm_ipv6_gen_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_gen_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_ipv6_gen_key_entry_t *find_free_tpm_ipv6_gen_key_entry(void)
{
    tpmcfg_ipv6_gen_key_entry_t *pentry = (tpmcfg_ipv6_gen_key_entry_t *)tpm_ipv6_gen_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_gen_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL del_tpm_ipv6_gen_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_gen_key_entry_t *pentry = (tpmcfg_ipv6_gen_key_entry_t *)tpm_ipv6_gen_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_gen_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->ipv6gen_acl, 0, sizeof(pentry->ipv6gen_acl));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_ipv6_gen_key_db(void)
{
    tpmcfg_ipv6_gen_key_entry_t *pentry = (tpmcfg_ipv6_gen_key_entry_t *)tpm_ipv6_gen_key_db.entryAra;
    char                    buf[800];
    int                     indx;
    int                     off = 0;
    char                    d1[60];
    char                    d1_mask[60];

    for (indx = 0; indx < tpm_ipv6_gen_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: \n", pentry->name);

            off += sprintf(buf+off, "\tdscp %d/0x%x, hop_limit %d\n",
                           pentry->ipv6gen_acl.ipv6_dscp, pentry->ipv6gen_acl.ipv6_dscp_mask,
                           pentry->ipv6gen_acl.hop_limit);

            format_ipv6_addr(pentry->ipv6gen_acl.ipv6_src_ip_add, d1);
            format_ipv6_addr(pentry->ipv6gen_acl.ipv6_src_ip_add_mask, d1_mask);

            off += sprintf(buf+off, "\tsrc_ip_add %s/%s\n", d1, d1_mask);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}

static tpmcfg_ipv6_dip_key_entry_t tpm_sysfs_ipv6_dip_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_ipv6_dip_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_ipv6_dip_key_entry_t),
    .entryAra           = tpm_sysfs_ipv6_dip_key_table
};


static void init_tpm_ipv6_dip_key_db(void)
{
    tpmcfg_ipv6_dip_key_entry_t *pentry = (tpmcfg_ipv6_dip_key_entry_t *)tpm_ipv6_dip_key_db.entryAra;
    int                     indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_ipv6_dip_key_db.max_num_entries; indx++, pentry++)
    {
        pentry->name[0] = 0;
    }
}

tpmcfg_ipv6_dip_key_entry_t *find_tpm_ipv6_dip_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_dip_key_entry_t *pentry = (tpmcfg_ipv6_dip_key_entry_t *)tpm_ipv6_dip_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_dip_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0) return pentry;
    }
    return 0;
}

tpmcfg_ipv6_dip_key_entry_t *find_free_tpm_ipv6_dip_key_entry(void)
{
    tpmcfg_ipv6_dip_key_entry_t *pentry = (tpmcfg_ipv6_dip_key_entry_t *)tpm_ipv6_dip_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_dip_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] == 0) return pentry;
    }
    return 0;
}

GT_BOOL del_tpm_ipv6_dip_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_dip_key_entry_t *pentry = (tpmcfg_ipv6_dip_key_entry_t *)tpm_ipv6_dip_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_dip_key_db.max_num_entries; indx++, pentry++)
    {
        if (strcmp(pentry->name, name) == 0)
        {
            pentry->name[0] = 0;
            memset(&pentry->ipv6_addr, 0, sizeof(pentry->ipv6_addr));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_ipv6_dip_key_db(void)
{
    tpmcfg_ipv6_dip_key_entry_t *pentry = (tpmcfg_ipv6_dip_key_entry_t *)tpm_ipv6_dip_key_db.entryAra;
    char                    buf[800];
    int                     indx;
    int                     off = 0;
    char                    d1[60];
//    char                    d2[60];
    char                    d1_mask[60];
//    char                    d2_mask[60];

    for (indx = 0; indx < tpm_ipv6_dip_key_db.max_num_entries; indx++, pentry++)
    {
        if (pentry->name[0] != 0)
        {
            off += sprintf(buf+off, "%s: \n", pentry->name);

            format_ipv6_addr(pentry->ipv6_addr.ipv6_ip_add, d1);
            format_ipv6_addr(pentry->ipv6_addr.ipv6_ip_add_mask, d1_mask);

            off += sprintf(buf+off, "\tdst_ip_add %s/%s\n", d1, d1_mask);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}

static tpmcfg_ipv6_l4_ports_key_entry_t  tpm_sysfs_ipv6_l4_ports_key_table[DB_TPMCFG_MAX_ENTRIES];

static tpm_generic_rule_db_t tpm_ipv6_l4_ports_key_db =
{
    .max_num_entries    = DB_TPMCFG_MAX_ENTRIES,
    .num_entries        = 0,
    .size_entry         = sizeof(tpmcfg_ipv6_l4_ports_key_entry_t),
    .entryAra           = tpm_sysfs_ipv6_l4_ports_key_table
};

static void init_tpm_ipv6_l4_ports_key_db(void)
{
    tpmcfg_ipv6_l4_ports_key_entry_t *pentry = (tpmcfg_ipv6_l4_ports_key_entry_t *)tpm_ipv6_l4_ports_key_db.entryAra;
    int indx;

    printk(KERN_INFO "%s: Clearing DB\n", __FUNCTION__);
    for (indx = 0; indx < tpm_ipv6_l4_ports_key_db.max_num_entries; indx++, pentry++)
        pentry->name[0] = 0;
}

tpmcfg_ipv6_l4_ports_key_entry_t *find_tpm_ipv6_l4_ports_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_l4_ports_key_entry_t *pentry = (tpmcfg_ipv6_l4_ports_key_entry_t *)tpm_ipv6_l4_ports_key_db.entryAra;
    int indx;

    for (indx = 0; indx < tpm_ipv6_l4_ports_key_db.max_num_entries; indx++, pentry++) {
        if (strcmp(pentry->name, name) == 0) 
			return pentry;
    }
    return 0;
}

tpmcfg_ipv6_l4_ports_key_entry_t *find_free_tpm_ipv6_l4_ports_key_entry(void)
{
    tpmcfg_ipv6_l4_ports_key_entry_t *pentry = (tpmcfg_ipv6_l4_ports_key_entry_t *)tpm_ipv6_l4_ports_key_db.entryAra;
    int indx;

    for (indx = 0; indx < tpm_ipv6_l4_ports_key_db.max_num_entries; indx++, pentry++) {
        if (pentry->name[0] == 0) 
			return pentry;
    }
    return 0;
}

GT_BOOL del_tpm_ipv6_l4_ports_key_entry_by_name(char *name)
{
    tpmcfg_ipv6_l4_ports_key_entry_t *pentry = (tpmcfg_ipv6_l4_ports_key_entry_t *)tpm_ipv6_l4_ports_key_db.entryAra;
    int                     indx;

    for (indx = 0; indx < tpm_ipv6_l4_ports_key_db.max_num_entries; indx++, pentry++) {
        if (strcmp(pentry->name, name) == 0) {
            pentry->name[0] = 0;
            memset(&pentry->l4_ports, 0, sizeof(pentry->l4_ports));
            return GT_TRUE;
        };
    }
    return GT_FALSE;
}

void show_tpm_ipv6_l4_ports_key_db(void)
{
    tpmcfg_ipv6_l4_ports_key_entry_t *pentry = (tpmcfg_ipv6_l4_ports_key_entry_t *)tpm_ipv6_l4_ports_key_db.entryAra;
    char                    buf[800];
    int                     indx;
    int                     off = 0;

    for (indx = 0; indx < tpm_ipv6_l4_ports_key_db.max_num_entries; indx++, pentry++) {
        if (pentry->name[0] != 0) {
            off += sprintf(buf+off, "%s: \n", pentry->name);

            off += sprintf(buf+off, "\tsrc_port %d, dst_port %d\n", 
						pentry->l4_ports.l4_src_port, pentry->l4_ports.l4_dst_port);

            printk(KERN_INFO "%s\n", buf);
            off = 0;
        }
    }
}


void tpm_reset_mc_vid_key(void)
{
    tpm_src_port_type_t                 src_port;
    
    memset(&mc_vid_key, 0, sizeof(mc_vid_key));
    
    for ( src_port = TPM_SRC_PORT_UNI_0; src_port <= TPM_SRC_PORT_UNI_VIRT; src_port++)
    {
        mc_vid_key.mc_vid_port_vids[src_port - TPM_SRC_PORT_UNI_0].tpm_src_port = src_port;
    }
    
    return;
}

GT_BOOL tpm_set_mc_vid_key(
    uint32_t                 src_port,
    uint32_t                 mc_uni_xlate_mode,
    uint32_t                 mc_uni_xlate_vid
)
{
    if (src_port < TPM_SRC_PORT_UNI_0 || src_port > TPM_SRC_PORT_UNI_VIRT)
    {
        printk(KERN_INFO "illegal input src port(%d)\n", src_port);
        return GT_FALSE;
    }
    
    mc_vid_key.mc_vid_port_vids[src_port - TPM_SRC_PORT_UNI_0].mc_uni_port_mode = mc_uni_xlate_mode;
    mc_vid_key.mc_vid_port_vids[src_port - TPM_SRC_PORT_UNI_0].tpm_src_port = src_port;
    mc_vid_key.mc_vid_port_vids[src_port - TPM_SRC_PORT_UNI_0].uni_port_vid = mc_uni_xlate_vid;
    
    return GT_TRUE;
}

tpm_error_code_t tpm_set_mc_vid_cfg(
    uint32_t                 mc_vid
)
{
    tpm_error_code_t ret;
    
    ret = tpm_proc_set_mc_vid_port_vids (0, mc_vid, &mc_vid_key);
    return ret;
}

// Intialization of database
void tpm_sysfs_rule_db_init(void)
{
    init_tpm_pkt_frwd_db();
    init_tpm_vlan_db();
    init_tpm_mod_db();
    init_tpm_l2_key_db();
    init_tpm_l3_key_db();
    init_tpm_ipv4_key_db();
    init_tpm_ipv6_key_db();
    init_tpm_ipv6_dip_key_db();
    init_tpm_ipv6_gen_key_db();
	init_tpm_ipv6_l4_ports_key_db();
}
tpm_error_code_t tpm_proc_send_genquery_to_uni(tpm_trg_port_type_t dest_port_bm, uint32_t packet_num)
{
	int32_t switch_dev_num;
	uint8_t loop;
	uint8_t loop_in;
	uint32_t trg_port;
	uint32_t switch_port_bm;
	tpm_error_code_t ret_code;
	tpm_db_ety_dsa_enable_t ety_dsa_enable;

	tpm_db_get_ety_dsa_enable(&ety_dsa_enable);
	if (TPM_ETY_DSA_DISABLE == ety_dsa_enable) {
		TPM_OS_ERROR(TPM_TPM_LOG_MOD, "ety_dsa is not enabled!\n");
		return TPM_OK;
	}
	/* fill in the switch dev_num */
	tpm_db_get_switch_dev_num(&switch_dev_num);
	switch_dev_num &= 0x1f;
	g_gen_query_pkt[16] = (0x40 | switch_dev_num);

	switch_port_bm = tpm_db_trg_port_switch_port_get(dest_port_bm);
	for (loop = 0, trg_port = 1; loop <= 7; loop++, trg_port = trg_port << 1) {
		if (0 == (trg_port & switch_port_bm))
			continue;

		g_gen_query_pkt[17] = (loop << 3);

		for (loop_in = 0; loop_in < packet_num; loop_in++) {
			g_gen_query_pkt[23] = (loop_in + 1);
			ret_code = tpm_tx_igmp_frame(0, TPM_TRG_PORT_UNI_ANY, 7, 0, g_gen_query_pkt, sizeof(g_gen_query_pkt));
			if (ret_code != TPM_OK) {\
				TPM_OS_ERROR(TPM_CLI_MOD, " recvd ret_code(%d)\n", ret_code);\
				return(ret_code);\
			}
		}
	}


	return(TPM_RC_OK);
}

