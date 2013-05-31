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
* mv_mac_learn_netdev.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include <ctrlEnv/mvCtrlEnvLib.h>
#include "tpm_common.h"
#include "tpm_header.h"

#include "mv_mac_learn_header.h"

/*******************************************************************************
* mv_mac_learn_eth_rx_func()
*
* DESCRIPTION:    Special RX function of mv_eth_rx in mv_netdev.c
*
* INPUTS:
*  port: gmac port the packet from
*  rxq: interrupt for rx
*  dev: net device
*  skb: buffer to receive packet
*  rx_desc: RX descriptor
*
* RETURNS:
* void
* COMMENTS:
*
*******************************************************************************/
void mv_mac_learn_eth_rx_func(int port, int rxq, struct net_device *dev, struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
	struct ethhdr *eth_hdr1;
	bool mac_learn_enable = false;

	/*check mac learn enable*/
	if (mv_mac_learn_op_enable_get(&mac_learn_enable))
		return;
	if (false == mac_learn_enable)
		return;

	/* get source MAC address */
	eth_hdr1 = eth_hdr(skb);
	mv_mac_learn_queue_write(eth_hdr1->h_source);
}

/*******************************************************************************
* mv_mac_learn_netdev_init()
*
* DESCRIPTION:    register special RX function netdev
*
* INPUTS:
* None
*
* OUTPUTS:
* None
*
* RETURNS:
* Init success--0
* COMMENTS:
*
*******************************************************************************/
int mv_mac_learn_netdev_init(void)
{
	uint32_t port_i;
	int mv_eth_ports_num = 0;

	/* Retrieve num_ports, as in mv_eth_init_module*/
	mv_eth_ports_num = mvCtrlEthMaxPortGet();

	if (mv_eth_ports_num > CONFIG_MV_ETH_PORTS_NUM)
		mv_eth_ports_num = CONFIG_MV_ETH_PORTS_NUM;

	/* Register mac learn parse function */
	for (port_i=0;port_i< mv_eth_ports_num;port_i++) {
		if (port_i == 0 || port_i == 1)
			mv_eth_rx_mac_learn_func(port_i, mv_mac_learn_eth_rx_func);
		else
			mv_eth_rx_mac_learn_func(port_i, NULL);
	}

	return 0;
}