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
********************************************************************************
* mv_cph_infra.h
*
* DESCRIPTION: Include user space infrastructure modules definitions
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   VictorGu
*
* DATE CREATED: 22Jan2013
*
* FILE REVISION NUMBER:
*               Revision: 1.0
*
*
*******************************************************************************/
#ifndef _MV_CPH_INFRA_H_
#define _MV_CPH_INFRA_H_

#ifdef __cplusplus
extern "C" {
#endif


/******************************************************************************
 * Data Type Definition
 ******************************************************************************/
typedef          char      INT8;
typedef unsigned char      UINT8;
typedef          short     INT16;
typedef unsigned short     UINT16;
typedef          int       INT32;
typedef unsigned int       UINT32;
typedef          long long INT64;
typedef unsigned long long UINT64;
typedef          char      CHAR;
typedef unsigned char      UCHAR;
typedef          long      LONG;
typedef unsigned long      ULONG;

typedef void               VOID;
typedef int                STATUS;

typedef bool               BOOL;
#define TRUE               true
#define FALSE              false


/******************************************************************************
 * Port Index Definition
 ******************************************************************************/
/* Ethernet port index */
#define MV_APP_ETH_PORT_INDEX_MIN 1
#define MV_APP_ETH_PORT_INDEX_MAX 4
#define MV_APP_ETH_PORT_NUM       (MV_APP_ETH_PORT_INDEX_MAX-MV_APP_ETH_PORT_INDEX_MIN+1)

typedef enum {
    MV_APP_ETH_PORT_UNI_0   = 1,
    MV_APP_ETH_PORT_UNI_1,
    MV_APP_ETH_PORT_UNI_2,
    MV_APP_ETH_PORT_UNI_3,
    MV_APP_ETH_PORT_UNI_4,
    MV_APP_ETH_PORT_UNI_5,
    MV_APP_ETH_PORT_UNI_6,
    MV_APP_ETH_PORT_UNI_7,
    MV_APP_ETH_PORT_INVALID   
}  MV_APP_ETH_PORT_UNI_E;

/* VoIP port index */
#define MV_APP_VOIP_PORT_INDEX_MIN 1
#define MV_APP_VOIP_PORT_INDEX_MAX 2
#define MV_APP_VOIP_PORT_NUM       (MV_APP_VOIP_PORT_INDEX_MAX-MV_APP_VOIP_PORT_INDEX_MIN+1)

typedef enum {
    MV_APP_VOIP_PORT_0 = 1,
    MV_APP_VOIP_PORT_1
}  MV_APP_VOIP_PORT_E;

/* WIFI SSID port index */
#define MV_APP_SSID_INDEX_MIN 1
#define MV_APP_SSID_INDEX_MAX 4
typedef enum {
    MV_APP_SSID_0 = 1,
    MV_APP_SSID_1,
    MV_APP_SSID_2,
    MV_APP_SSID_3,
    MV_APP_SSID_4,
    MV_APP_SSID_5,
    MV_APP_SSID_6,
    MV_APP_SSID_7
}  MV_APP_SSID_E;

/* USB port index */
#define MV_APP_USB_PORT_INDEX_MIN 1
#define MV_APP_USB_PORT_INDEX_MAX 2
#define MV_APP_USB_PORT_NUM       (MV_APP_USB_PORT_INDEX_MAX-MV_APP_USB_PORT_INDEX_MIN+1)

typedef enum {
    MV_APP_USB_PORT_0 = 1,
    MV_APP_USB_PORT_1
}  MV_APP_USB_PORT_E;

/******************************************************************************
 * GE MAC Port Index Definition
 ******************************************************************************/
typedef enum {
    MV_APP_GMAC_PORT_0       = 0,
    MV_APP_GMAC_PORT_1,
    MV_APP_PON_MAC_PORT,
    MV_APP_GMAC_PORT_NUM     = 3,
    MV_APP_GMAC_PORT_INVALID = 3,
}  MV_APP_GMAC_PORT_E;

/******************************************************************************
 * T-CONT/LLID Index Definition
 ******************************************************************************/
#define MV_TCONT_LLID_MIN 0
#define MV_TCONT_LLID_MAX 7
#define MV_TCONT_LLID_NUM (MV_TCONT_LLID_MAX-MV_TCONT_LLID_MIN+1)

typedef enum {
    MV_TCONT_LLID_0 = 0,
    MV_TCONT_LLID_1,
    MV_TCONT_LLID_2,
    MV_TCONT_LLID_3,
    MV_TCONT_LLID_4,
    MV_TCONT_LLID_5,
    MV_TCONT_LLID_6 ,
    MV_TCONT_LLID_7, 
    MV_TCONT_LLID_8,
    MV_TCONT_LLID_9,
    MV_TCONT_LLID_10,
    MV_TCONT_LLID_11,
    MV_TCONT_LLID_12,
    MV_TCONT_LLID_13,
    MV_TCONT_LLID_14 ,
    MV_TCONT_LLID_15, 
    MV_TCONT_LLID_BROADCAST,
    MV_TCONT_LLID_INVALID
} MV_TCONT_LLID_E;

/******************************************************************************
 * WAN and LAN Index Definition
 ******************************************************************************/
typedef enum {
    MV_APP_PORT_WAN = 0,
    MV_APP_PORT_LAN,
    MV_APP_PORT_INVALID,
}  MV_APP_PORT_TYPE_E;

/******************************************************************************
 * GEM Port Index Definition
 ******************************************************************************/
#define MV_GEM_PORT_MIN  0
#define MV_GEM_PORT_MAX  4095
#define MV_GEM_PORT_NUM  (MV_GEM_PORT_MAX-MV_GEM_PORT_MIN+1)
#define MV_GEM_PORT_MASK 0x0FFF

/******************************************************************************
 * Queue Index Definition
 ******************************************************************************/
#define MV_QUEUE_MIN  0
#define MV_QUEUE_MAX  7
#define MV_QUEUE_NUM  (MV_QUEUE_MAX-MV_QUEUE_MIN+1)
#define MV_INVALID_QUEUE_NUM  (0xFF) /* Invalid queue number  */

typedef enum {
    MV_QUEUE_0 = 0,
    MV_QUEUE_1,
    MV_QUEUE_2,
    MV_QUEUE_3,
    MV_QUEUE_4,
    MV_QUEUE_5,
    MV_QUEUE_6,
    MV_QUEUE_7
}  MV_QUEUE_E;

/******************************************************************************
 * VLAN ID/P-bits Index Definition
 ******************************************************************************/
#define MV_VLAN_ID_MIN  0
#define MV_VLAN_ID_MAX 4095
#define MV_VLAN_ID_NUM (MV_VLAN_ID_MAX-MV_VLAN_ID_MIN+1)
#define MV_VLAN_ID_MASK 0x0FFF
#define MV_VLAN_ID_INVALID_VALUE 0xFFFF

#define MV_PBITS_MIN   0
#define MV_PBITS_MAX   7
#define MV_PBITS_NUM  (MV_PBITS_MAX-MV_PBITS_MIN+1)
#define MV_PBITS_SHIFT 13
#define MV_PBITS_MASK 0x07
#define MV_PBITS_INVALID_VALUE 0xFF


/******************************************************************************
 * WAN Port State Definition
 ******************************************************************************/
typedef enum {
    MV_GE_PORT_INACTIVE = 0,
    MV_GE_PORT_ACTIVE,
    MV_GE_PORT_INVALID,
}  MV_GE_PORT_STATE_E;

/* Enum for well known TPID
------------------------------------------------------------------------------*/
#define MV_TPID_8100 0x8100
#define MV_TPID_88A8 0x88A8
#define MV_TPID_9100 0x9100
#define MV_CPH_TPID_LEN      (2)
#define MV_CPH_VLAN_TAG_LEN  (2)
#define MV_CPH_ETH_TYPE_LEN  (2)
#define MV_IPV4_PROTO_OFFSET (9)
#define MV_ICMPV6_TYPE_MLD   (1)
#define MV_CPH_ETH_TYPE_IPV4 (0x0800)
#define MV_CPH_ETH_TYPE_IPV6 (0x86DD)


/******************************************************************************
 * TPM Source port - used to specify through which port the packet entered the processor
 ******************************************************************************/
typedef enum {
    TPM_SRC_PORT_UNI_0,        /* upstream */
    TPM_SRC_PORT_UNI_1,        /* upstream */
    TPM_SRC_PORT_UNI_2,        /* upstream */
    TPM_SRC_PORT_UNI_3,        /* upstream */
    TPM_SRC_PORT_UNI_4,        /* upstream */
    TPM_SRC_PORT_UNI_5,        /* upstream */
    TPM_SRC_PORT_UNI_6,        /* upstream */
    TPM_SRC_PORT_UNI_7,        /* upstream */
    TPM_SRC_PORT_UNI_VIRT,     /* upstream */
    TPM_SRC_PORT_WAN,          /* downstram */
    TPM_SRC_PORT_UNI_ANY,      /* upstream - all UNI ports */
    TPM_SRC_PORT_WAN_OR_LAN,   /* Any Port, currently not supported */
    TPM_SRC_PORT_ILLEGAL = 0xFF
} tpm_src_port_type_t;

/******************************************************************************
 * Check API return value
 ******************************************************************************/
#define CHECK_API_RETURN_AND_LOG_ERROR(ret, log) \
{\
    INT32 rc = 0 ;\
    if((rc = (ret)) != 0)\
    {\
        printk(KERN_ERR "%s(%d) error:%s , ret(%d)\n", __FUNCTION__, __LINE__, (log), rc);\
        return (rc) ;\
    }\
}

#define CPH_TBL_ENTRY_NUM(a) (sizeof(a)/sizeof(a[0]))

/******************************************************************************
 * Enum for display
 ******************************************************************************/
typedef struct
{
    INT32  enum_value;
    CHAR  *enum_str;
} MV_ENUM_ENTRY_T;

typedef struct
{
    INT32            enum_num;
    MV_ENUM_ENTRY_T *enum_array;
} MV_ENUM_ARRAY_T;

/******************************************************************************
 * Function Declaration
 ******************************************************************************/
/******************************************************************************
* mindex_tpm_src_to_app_port()
*
* DESCRIPTION:Convert TPM source port to application UNI port 
*
* INPUTS:
*       src_port    - TPM source port
*
* OUTPUTS:
*       Application UNI port index
*
* RETURNS:
*       On success, the function returns application UNI port index. 
*       On error return invalid application UNI port index.
*******************************************************************************/
MV_APP_ETH_PORT_UNI_E mindex_tpm_src_to_app_port(tpm_src_port_type_t src_port);

/******************************************************************************
* mindex_mh_to_app_llid()
*
* DESCRIPTION:Convert Marvell header to application LLID 
*
* INPUTS:
*       mh  - Marvell header
*
* OUTPUTS:
*       Application LLID
*
* RETURNS:
*       On success, the function returns application LLID. 
*       On error return invalid application LLID.
*******************************************************************************/
MV_TCONT_LLID_E mindex_mh_to_app_llid(UINT16 mh);

/******************************************************************************
* mtype_get_digit_num()
*
* DESCRIPTION:Convert character string to digital number 
*
* INPUTS:
*       str   - Character string
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Digital numbe
*******************************************************************************/
UINT32 mtype_get_digit_num(const CHAR  *str);

/******************************************************************************
* mtype_lookup_enum_str()
* _____________________________________________________________________________
*
* DESCRIPTION:lookup enum string according to enum value
*
* INPUTS:
*       p_enum_array   - Pointer to enum array
*       enum_value     - The enum value to be matched
*
* OUTPUTS:
*       None
*
* RETURNS:
*       Enum string
*******************************************************************************/
CHAR *mtype_lookup_enum_str(MV_ENUM_ARRAY_T *p_enum_array, INT32 enum_value);

/******************************************************************************
* mutils_is_frwd_broadcast_packet()
* _____________________________________________________________________________
*
* DESCRIPTION:Check whether packet is directly forwarded broadcast one
*
* INPUTS:
*       data   - packet data
*
* OUTPUTS:
*       None
*
* RETURNS:
*       TRUE: broadcast packet, FALSE:none broadcast packet
*******************************************************************************/
BOOL mutils_is_frwd_broadcast_packet(char *data);
#ifdef __cplusplus
}
#endif

#endif /* _MV_CPH_INFRA_H_ */
