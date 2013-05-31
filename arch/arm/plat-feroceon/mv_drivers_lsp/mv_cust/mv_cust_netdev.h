/************************************************************************
* Copyright (C) 2010, Marvell Technology Group Ltd.
* All Rights Reserved.
*
* This is UNPUBLISHED PROPRIETARY SOURCE CODE of Marvell Technology Group;
* the contents of this file may not be disclosed to third parties, copied
* or duplicated in any form, in whole or in part, without the prior
* written permission of Marvell Technology Group.
*
*********************************************************************************
* Marvell GPL License Option
*
* If you received this File from Marvell, you may opt to use, redistribute and/or
* modify this File in accordance with the terms and conditions of the General
* Public License Version 2, June 1991 (the "GPL License"), a copy of which is
* available along with the File in the license.txt file or by writing to the Free
* Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
* on the worldwide web at http://www.gnu.org/licenses/gpl.txt.
*
* THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
* WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
* DISCLAIMED.  The GPL License provides additional details about this warranty
* disclaimer.
*
*********************************************************************************
* mv_cust_netdev.h
*
* DESCRIPTION:
*
*
*******************************************************************************/
#ifndef __mv_cust_netdev_h__
#define __mv_cust_netdev_h__

#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <net/ip.h>

#include <mvCommon.h>
#include <mvOs.h>
#include <mv_neta/net_dev/mv_netdev.h>


#define MV_CUST_MAS_UDP_SRC_PORT          8
#define MV_CUST_MAS_UDP_DST_PORT          8
#define MV_CUST_NUM_LLID                  8


struct mv_udp_port_tx_spec {
    __be16    udp_port;
    struct mv_eth_tx_spec tx_spec;
};

struct mv_port_tx_spec {
    struct mv_udp_port_tx_spec udp_src[MV_CUST_MAS_UDP_SRC_PORT];
    struct mv_udp_port_tx_spec udp_dst[MV_CUST_MAS_UDP_DST_PORT];
};


struct mv_eoam_llid_spec {
    uint8_t llid_mac_address[6];
    struct  mv_eth_tx_spec tx_spec;
};

typedef enum
{
    MV_CUST_APP_TYPE_IGMP = 0, /* For IGMP application               */
    MV_CUST_APP_TYPE_MLD,      /* For MLD application                */
    MV_CUST_APP_TYPE_LPBK,     /* For loopback detection application */
    MV_CUST_APP_TYPE_OAM,      /* For eOAM application               */
    MV_CUST_APP_TYPE_OMCI,     /* For OMCI application               */
    MV_CUST_APP_TYPE_MAX       /* Max number of application          */ 
} mv_cust_app_type_e;

/*  Enum: enable or disable application*/
typedef enum
{
    MV_CUST_APP_DISABLE  = 0,
    MV_CUST_APP_ENABLE   = 1,
} mv_cust_app_flag_e;

typedef struct
{
    mv_cust_app_type_e app_type; /* Application type, such as IGMP, MLD                */
    uint16_t           enable;   /* Flag indicates whether to enable application Rx/Tx */         
    uint16_t           eth_type; /* Application Ethernet type used for socket/skb      */
    char              *name;     /* The readable name of the application               */
} mv_cust_app_config_t;


/******************************************************
 * Function prototypes --                             *
*******************************************************/
void        mv_cust_omci_hw_cmd_set(unsigned int hw_cmd);
int         mv_cust_omci_tx_set(int tcont, int txq);

#ifdef  CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
MV_STATUS   mv_cust_udp_src_spec_set(int tx_port, uint16_t udp_src_port, uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd);
MV_STATUS   mv_cust_udp_dest_spec_set(int tx_port, uint16_t udp_dest_port, uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd);
#endif

void        mv_cust_oam_rx_gh_set(int val);
int         mv_cust_omci_set(int tcont, int txq, int gemport, int keep_rx_mh);
int         mv_cust_eoam_llid_set(int llid, uint8_t *llid_mac, int txq);

void        mv_cust_app_flag_set(mv_cust_app_type_e app_type, uint16_t enable);
void        mv_cust_app_etype_set(mv_cust_app_type_e app_type, uint16_t eth_type);
void        mv_cust_print(int type);


#endif /* __mv_cust_netdev_h__ */
