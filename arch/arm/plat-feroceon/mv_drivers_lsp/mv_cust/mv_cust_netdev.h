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
	__be16	udp_port;
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



/******************************************************
 * Function prototypes --                             *
*******************************************************/
void        mv_cust_omci_hw_cmd_set(unsigned int hw_cmd);
int         mv_cust_omci_tx_set(int tcont, int txq);
//int         mv_cust_eoam_tx_set(int txp, int txq);
MV_STATUS   mv_cust_omci_enable(int enable);
MV_STATUS   mv_cust_eoam_enable(int enable);
//MV_STATUS   mv_cust_eoam_tx_func_set(void (*tx_func) (uint8_t *data, int size, struct mv_eth_tx_spec *tx_spec));
#ifdef  CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
MV_STATUS   mv_cust_udp_src_spec_set(int tx_port, uint16_t udp_src_port, uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd);
MV_STATUS   mv_cust_udp_dest_spec_set(int tx_port, uint16_t udp_dest_port, uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd);
#endif
void        mv_cust_omci_type_set(unsigned short int type);
void        mv_cust_xpon_oam_rx_gh_set(int val);
void        mv_cust_epon_oam_type_set(unsigned short int type);
#ifdef CONFIG_MV_CUST_IGMP_HANDLE
void        mv_cust_igmp_type_set(unsigned short int type);
#endif
void        mv_cust_omci_gemport_set(int gemport);
void        mv_cust_loopdet_type_set(unsigned short int type);
int         mv_cust_omci_set(int tcont, int txq, int gemport, int keep_rx_mh);
int         mv_cust_eoam_llid_set(int llid, uint8_t *llid_mac, int txq);





#endif /* __mv_cust_netdev_h__ */
