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
* mv_cust_netdev.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include <mvCommon.h>

#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <net/ip.h>
#include <net/ipv6.h>

#include <mvOs.h>
#include <ctrlEnv/mvCtrlEnvLib.h>

#include "mv_cust_dev.h"
#include "mv_cust_netdev.h"
#include "mv_cust_flow_map.h"
#include "mv_cust_mng_if.h"

/* YUVAL - update to pnc define */
#define     MH_GEM_PORT_MASK                (0x0FFF)
#define     MH_EPON_OAM_TYPE                (0x8809)
#define     ETY_IPV4                        (0x0800)
#define     IPV4_PROTO_OFFSET               (9)

#define     CUST_TBL_NUM_ENTRIES(a)         (sizeof(a)/sizeof(a[0]))

/*Static Declarations */
static int    mv_eth_ports_num = 0;
static int    mv_cust_omci_gemport = 0;

static int    mv_cust_omci_valid = 0;
static int    mv_cust_eoam_valid = 0;
#ifdef CONFIG_MV_CUST_IGMP_HANDLE
static int    mv_cust_igmp_detect = 1;
static uint16_t mv_cust_igmp_type;
static struct mv_eth_tx_spec     igmp_tx_spec = {0, MV_ETH_F_MH, 0, 0};
#endif
static int    mv_cust_port_loopback_detect = 1;
static int    mv_cust_debug_code = 0;
static int    mv_cust_xpon_oam_rx_gh = 0;

static uint16_t mv_cust_xpon_oam_type;
static uint16_t mv_cust_loopdet_type;

/* Protocol definitions */
static struct mv_eth_tx_spec     omci_mgmt_tx_spec = {0};
static struct mv_eoam_llid_spec  epon_mgmt_tx_spec[MV_CUST_NUM_LLID];

#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
static struct mv_port_tx_spec    port_spec_cfg[CONFIG_MV_ETH_PORTS_NUM];
#endif

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
static int    mv_cust_flow_map = 0;
static int    mv_cust_flow_map_parse(uint8_t *data, uint16_t *vlan, uint8_t *pbits, uint8_t dir);
static int    mv_cust_flow_map_mod(uint8_t *data, uint16_t vid, uint8_t pbits, uint8_t dir);
#endif


#define     PORT_ENTRIES                    CUST_TBL_NUM_ENTRIES(port_spec_cfg)
#define     EPON_MGMT_ENTRIES               CUST_TBL_NUM_ENTRIES(epon_mgmt_tx_spec)


void mv_cust_omci_print(void)
{
    printk("************* OMCI Configuration *****************\n\n");
    printk("OMCI: valid = %d, gemport = %d, ethtype = 0x%04x, gh_keep = %d\n",
           mv_cust_omci_valid, mv_cust_omci_gemport, ntohs(mv_cust_xpon_oam_type), mv_cust_xpon_oam_rx_gh);
    printk("OMCI: txp   = %d, txq = %d, hw_cmd = 0x%08x, flags = 0x%04x on TX \n",
           omci_mgmt_tx_spec.txp, omci_mgmt_tx_spec.txq, omci_mgmt_tx_spec.hw_cmd, omci_mgmt_tx_spec.flags);
    printk("\n");
}


void mv_cust_eoam_print(void)
{
    int i;
    printk("************* eOAM Configuration *****************\n\n");
    printk("EOAM: valid = %d, ethtype = 0x%04x, gh_keep = %d\n",
           mv_cust_eoam_valid, ntohs(mv_cust_xpon_oam_type), mv_cust_xpon_oam_rx_gh);
    for (i=0;i <(EPON_MGMT_ENTRIES);i++) {
        printk("llid%d: mac=%02x:%02x:%02x:%02x:%02x:%02x, txp=%d, txq=%d, hw_cmd=0x%08x, flags = 0x%04x\n",
               i,
               epon_mgmt_tx_spec[i].llid_mac_address[0],epon_mgmt_tx_spec[i].llid_mac_address[1],
               epon_mgmt_tx_spec[i].llid_mac_address[2],epon_mgmt_tx_spec[i].llid_mac_address[3],
               epon_mgmt_tx_spec[i].llid_mac_address[4],epon_mgmt_tx_spec[i].llid_mac_address[5],
               epon_mgmt_tx_spec[i].tx_spec.txp, epon_mgmt_tx_spec[i].tx_spec.txq,
               epon_mgmt_tx_spec[i].tx_spec.hw_cmd, epon_mgmt_tx_spec[i].tx_spec.flags);
        printk("\n");
    }
}

#ifdef CONFIG_MV_CUST_IGMP_HANDLE
void mv_cust_igmp_print(void)
{
    printk("************* IGMP Configuration *****************\n\n");
    printk("IGMP valid = %d,  ethtype = 0x%04x, on RX\n",
           mv_cust_igmp_detect, ntohs(mv_cust_igmp_type));
    printk("IGMP txp = %d, txq = %d, , hw_cmd = 0x%08x on TX\n",
           igmp_tx_spec.txp, igmp_tx_spec.txq, igmp_tx_spec.hw_cmd);
    printk("\n");
}
#endif

void mv_cust_loopdet_print(void)
{
    printk("************* Port Loopback Configuration *****************\n\n");
    printk("Port Loopback valid = %d,  ethtype = 0x%04x, on RX\n",
           mv_cust_port_loopback_detect, ntohs(mv_cust_loopdet_type));
}


#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
int mv_cust_udp_spec_print(int port)
{
    int i;
    struct eth_port *pp = mv_eth_port_by_id(port);
    struct mv_udp_port_tx_spec *udp_spec;

    if (!pp)
        return -ENODEV;

    udp_spec = &(port_spec_cfg[port].udp_dst[0]);

    printk("\n**** port #%d - TX UDP Dest Port configuration *****\n", port);
    printk("----------------------------------------------------\n");
    printk("ID udp_dst   txp    txq    flags    hw_cmd     func_add\n");
    for (i = 0; i < sizeof(port_spec_cfg[port].udp_dst)/sizeof(port_spec_cfg[port].udp_dst[0]); i++) {
        if (udp_spec[i].tx_spec.txq != MV_ETH_TXQ_INVALID)
            printk("%2d   %04d      %d      %d     0x%04x   0x%08x   0x%p\n",
                   i, ntohs(udp_spec[i].udp_port),
                   udp_spec[i].tx_spec.txp, udp_spec[i].tx_spec.txq,
                   udp_spec[i].tx_spec.flags, udp_spec[i].tx_spec.hw_cmd,
                   udp_spec[i].tx_spec.tx_func);
    }
    printk("-----------------------------------------------------\n");

    udp_spec = &(port_spec_cfg[port].udp_src[0]);

    printk("**** port #%d - TX UDP Source Port configuration *****\n", port);
    printk("-----------------------------------------------------\n");
    printk("ID udp_src   txp    txq     flags    hw_cmd     func_add\n");
    for (i = 0; i < sizeof(port_spec_cfg[port].udp_src)/sizeof(port_spec_cfg[port].udp_src[0]); i++) {
        if (udp_spec[i].tx_spec.txq != MV_ETH_TXQ_INVALID)
            printk("%2d   %04d      %d      %d     0x%04x   0x%08x   0x%p\n",
                   i, ntohs(udp_spec[i].udp_port),
                   udp_spec[i].tx_spec.txp, udp_spec[i].tx_spec.txq,
                   udp_spec[i].tx_spec.flags, udp_spec[i].tx_spec.hw_cmd,
                   udp_spec[i].tx_spec.tx_func);
    }
    printk("**************************************************************\n");


    return 0;
}

void mv_cust_udp_spec_print_all(void)
{
    int port;

    for (port=0;port < CONFIG_MV_ETH_PORTS_NUM ;port++) {
        mv_cust_udp_spec_print(port);
    }
}
#endif

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
void mv_cust_flow_map_print(void)
{
    printk("************* Flow Mapping Configuration *****************\n\n");
    printk("FLow mapping valid = %d\n", mv_cust_flow_map);
}
#endif

void mv_cust_omci_hw_cmd_set(uint32_t hw_cmd)
{
    omci_mgmt_tx_spec.hw_cmd = hw_cmd;
}
EXPORT_SYMBOL(mv_cust_omci_hw_cmd_set);

int mv_cust_omci_tx_set(int tcont, int txq)
{
    if (mvNetaTxpCheck(MV_PON_PORT_ID, tcont)) {
        return -EINVAL;
    }

    if (txq<0 || txq>CONFIG_MV_ETH_TXQ) {
        return -EINVAL;
    }

    omci_mgmt_tx_spec.txp = tcont;
    omci_mgmt_tx_spec.txq = txq;

    return 0;
}


int mv_cust_omci_set(int tcont, int txq, int gem_port, int keep_rx_mh)
{
    int ret, hw_cmd;

    omci_mgmt_tx_spec.tx_func = NULL;
    omci_mgmt_tx_spec.flags = MV_ETH_F_NO_PAD | MV_ETH_F_MH;

    ret = mv_cust_omci_tx_set(tcont, txq);
    if (ret) {
        MVCUST_ERR_PRINT("mv_cust_omci_tx_set - ret(%d)  \n", ret);
        return ret;
    }

    mv_cust_omci_gemport_set(gem_port);
    hw_cmd = ((gem_port << 8) | 0x0010);
    mv_cust_omci_hw_cmd_set(hw_cmd);
    mv_cust_xpon_oam_rx_gh_set(keep_rx_mh);

    mv_cust_omci_enable(1);

    return 0;
}
EXPORT_SYMBOL(mv_cust_omci_set);


int mv_cust_eoam_llid_set(int llid, uint8_t *llid_mac, int txq)
{

    if (mvNetaTxpCheck(MV_PON_PORT_ID, llid))
        return -EINVAL;

    if (txq<0 || txq>CONFIG_MV_ETH_TXQ) {
            return -EINVAL;
    }
    if (!llid_mac) {
        return -EINVAL;
    }
    memcpy((void *) &(epon_mgmt_tx_spec[llid].llid_mac_address), (void *) llid_mac, ETH_ALEN);

    epon_mgmt_tx_spec[llid].tx_spec.txq = txq;

    return 0;
}
EXPORT_SYMBOL(mv_cust_eoam_llid_set);


MV_STATUS mv_cust_omci_enable(int enable)
{
    if (enable) {

        if (mv_cust_eoam_valid) {
            MVCUST_ERR_PRINT("EPON is already valid\n");
            return MV_ERROR;
        }
        mv_cust_omci_valid = 1;
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
        mv_cust_flow_map   = 1;
#endif
    }
    else {
        mv_cust_omci_valid = 0;
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
        mv_cust_flow_map   = 0;
#endif
    }

    return MV_OK;
}


void mv_cust_eoam_init(void)
{
    int i;

    for (i=0;i <(EPON_MGMT_ENTRIES);i++) {
        memset(&epon_mgmt_tx_spec[i],0, sizeof(struct mv_eoam_llid_spec));

        /* Each table entry (i) a-priori corresponds to LLID (i) */
        epon_mgmt_tx_spec[i].tx_spec.txp = i;

        /* The mv_neta driver will add the default MH. The MH value has no significance in EPON Upstream */
        epon_mgmt_tx_spec[i].tx_spec.flags = MV_ETH_F_MH;
    }

    /* In Rx, keep the MH for EOAM */
    mv_cust_xpon_oam_rx_gh_set(1);
    return ;
}
EXPORT_SYMBOL(mv_cust_eoam_init);


MV_STATUS mv_cust_eoam_enable(int enable)
{

    if(mv_cust_debug_code)
        printk("%s: enable = %d\n", __func__, enable);

    if (enable) {
        if (mv_cust_omci_valid) {
            MVCUST_ERR_PRINT("OMCI is already valid\n");
            return MV_ERROR;
        }
        mv_cust_eoam_valid = 1;
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
        mv_cust_flow_map   = 0;
#endif
    }
    else {
        mv_cust_eoam_valid = 0;
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
        mv_cust_flow_map   = 0;
#endif
    }

    return MV_OK;
}
EXPORT_SYMBOL(mv_cust_eoam_enable);

#if 0
MV_STATUS mv_cust_eoam_tx_func_set(void (*tx_func) (uint8_t *data, int size, struct mv_eth_tx_spec *tx_spec))
{

    if (mv_cust_debug_code)
        printk("%s: \n", __func__);

    /* System cannot be OMCI_valid */
    if (mv_cust_omci_valid) {
        MVCUST_ERR_PRINT("EPON is valid\n");
        return MV_ERROR;
    }
    /*Pointer cannot be null */
    if (!tx_func) {
        MVCUST_ERR_PRINT("NULL pointer\n");
    }

    xpon_mgmt_tx_spec.tx_func = tx_func;

    return MV_OK;
}
EXPORT_SYMBOL(mv_cust_eoam_tx_func_set);
#endif

static inline void mv_cust_copy_tx_spec(struct mv_eth_tx_spec * tx_spec,
                                        uint8_t txp, uint8_t txq,
                                        uint16_t flags, uint32_t hw_cmd)
{
    tx_spec->txp = txp;
    tx_spec->txq = txq;
    tx_spec->hw_cmd = hw_cmd;
    tx_spec->flags = flags;
}



#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
MV_STATUS  mv_cust_udp_int_spec_set(struct mv_udp_port_tx_spec *udp_spec, uint16_t udp_port, int table_size,
                                    uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd)
{
    int i;

    /* Check if already exists */
    for (i=0; i < table_size;i++) {
        if (udp_spec[i].udp_port == htons(udp_port) &&
            udp_spec[i].tx_spec.txq != MV_ETH_TXQ_INVALID) {
            mv_cust_copy_tx_spec(&(udp_spec[i].tx_spec), txp, txq, flags, hw_cmd);
            return MV_OK;
        }
    }
    /* Check empty */
    for (i=0; i < table_size;i++) {
        if (udp_spec[i].tx_spec.txq == MV_ETH_TXQ_INVALID) {
            udp_spec[i].udp_port = htons(udp_port);
            mv_cust_copy_tx_spec(&(udp_spec[i].tx_spec), txp, txq, flags, hw_cmd);
            return MV_OK;
        }
    }

    return(MV_FULL);
}


MV_STATUS  mv_cust_udp_src_spec_set(int tx_port, uint16_t udp_src_port, uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd)
{
    struct eth_port *pp = mv_eth_port_by_id(tx_port);
    struct mv_udp_port_tx_spec *udp_src_spec = port_spec_cfg[tx_port].udp_src;
    MV_STATUS mv_status;

    if (!pp)
        return -ENODEV;

    mv_status = mv_cust_udp_int_spec_set(udp_src_spec, udp_src_port,
                                         sizeof(port_spec_cfg[tx_port].udp_src)/sizeof(port_spec_cfg[tx_port].udp_src[0]),
                                         txp, txq, flags, hw_cmd);

    if (mv_status != MV_OK)
        printk("%s: UDP Special Source Port Table is full\n", __func__);

    return(mv_status);
}
EXPORT_SYMBOL(mv_cust_udp_src_spec_set);


MV_STATUS  mv_cust_udp_dest_spec_set(int tx_port, uint16_t udp_dest_port, uint8_t txp, uint8_t txq, uint16_t flags, uint32_t hw_cmd)
{
    struct eth_port *pp = mv_eth_port_by_id(tx_port);
    struct mv_udp_port_tx_spec *udp_dst_spec = port_spec_cfg[tx_port].udp_dst;
    MV_STATUS mv_status;

    if (!pp)
        return -ENODEV;

    mv_status = mv_cust_udp_int_spec_set(udp_dst_spec, udp_dest_port,
                                         sizeof(port_spec_cfg[tx_port].udp_dst)/sizeof(port_spec_cfg[tx_port].udp_dst[0]),
                                         txp, txq, flags, hw_cmd);

    if (mv_status != MV_OK)
        printk("%s: UDP Special Dest. Port Table is full\n", __func__);

    return(mv_status);
}
EXPORT_SYMBOL(mv_cust_udp_dest_spec_set);


void  mv_cust_udp_table_del(void)
{
    int num_ports = PORT_ENTRIES;
    int tx_port, i;

    if (num_ports > mv_eth_ports_num)
        num_ports = mv_eth_ports_num;


    for (tx_port=0; tx_port<num_ports;tx_port++) {

        /* Invalidate UDP Dest ports, set txq=invalid  */
        for (i=0;i<(sizeof(port_spec_cfg[tx_port].udp_dst)/sizeof(port_spec_cfg[tx_port].udp_dst[0]));i++) {
            memset(&(port_spec_cfg[tx_port].udp_dst[i]), 0, sizeof(struct mv_udp_port_tx_spec));
            port_spec_cfg[tx_port].udp_dst[i].tx_spec.txq = MV_ETH_TXQ_INVALID;
        }

        /* Invalidate UDP Source ports, , set txq=invalid */
        for (i=0;i<(sizeof(port_spec_cfg[tx_port].udp_src)/sizeof(port_spec_cfg[tx_port].udp_src[0]));i++) {
            memset(&(port_spec_cfg[tx_port].udp_src[i]), 0, sizeof(struct mv_udp_port_tx_spec));
            port_spec_cfg[tx_port].udp_src[i].tx_spec.txq = MV_ETH_TXQ_INVALID;
        }

    }
    return;
}
#endif

int mv_cust_omci_tx(int port, struct net_device *dev, struct sk_buff *skb,
                   struct mv_eth_tx_spec *tx_spec_out)
{
    if ( (skb->protocol == mv_cust_xpon_oam_type)
        && mv_cust_omci_valid
        && port == MV_PON_PORT_ID) {
        memcpy (tx_spec_out, &omci_mgmt_tx_spec, sizeof(struct mv_eth_tx_spec));
        if(mv_cust_debug_code)
            printk("%s", __func__);
        return 1;
    }
    return 0;
}


int mv_cust_eoam_tx(int port, struct net_device *dev, struct sk_buff *skb,
                    struct mv_eth_tx_spec *tx_spec_out)
{
    int mac_match, i;

    if (skb->protocol == mv_cust_xpon_oam_type
        && mv_cust_eoam_valid
        && port == MV_PON_PORT_ID) {
        /* Lookup MAC Address */
        for (i=0; i<(EPON_MGMT_ENTRIES);i++) {
            mac_match = memcmp((void *) &(epon_mgmt_tx_spec[i].llid_mac_address[0]),
                               (void *)(skb->data + /*MV_ETH_MH_SIZE +*/ ETH_ALEN),
                               ETH_ALEN);
            if (!mac_match) {
                memcpy (tx_spec_out, &epon_mgmt_tx_spec[i].tx_spec, sizeof(struct mv_eth_tx_spec));
                if(mv_cust_debug_code)
                    printk("%s, llid = %d", __func__, i);
                return 1;
            }
        }
        /* Source MAC Address not found */
        if(mv_cust_debug_code) {
            printk("(%s)Input Packet first bytes:\n", __func__);
            for (i=0;i<24;i++) {
                if (i%8== 0)
                    printk("\n");
                printk ("%02x ", *(skb->data + i));
            }
        }
    }
    return 0;
}


#ifdef CONFIG_MV_CUST_IGMP_HANDLE
int mv_cust_igmp_tx(int port, struct net_device *dev, struct sk_buff *skb,
                    struct mv_eth_tx_spec *tx_spec_out)
{
    if (mv_cust_igmp_detect) {
        /* Check Tx XPON Type */
        if (skb->protocol == mv_cust_igmp_type) {
            memcpy (tx_spec_out, &igmp_tx_spec, sizeof(struct mv_eth_tx_spec));

            /* MH should not be set */
            tx_spec_out->flags = 0;
            if (2 == port)
            {
                tx_spec_out->txp = skb->data[0];
                tx_spec_out->hw_cmd = ((skb->data[2] << 8) | skb->data[3]) << 8;
                skb->data[2] |= 0x80;
            }
            tx_spec_out->txq = skb->data[1];

            skb_pull(skb, 2);

            return 1;
        }
    }
    return 0;
}
#endif


#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
int mv_cust_udp_port_tx(int port, struct net_device *dev, struct sk_buff *skb,
                        struct mv_eth_tx_spec *tx_spec_out)
{
    struct iphdr  * iphdrp   = NULL;
    struct udphdr * udphdrp  = NULL;
    int i;

    if (port > CONFIG_MV_ETH_PORTS_NUM) {
        printk("Port Error\n");
        return(0);
    }

    if (skb->protocol == ETY_IPV4) {
        /* Get UDP Port */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	iphdrp = skb->nh.iph;
#else
	iphdrp = ip_hdr(skb);
#endif

        if ((iphdrp) && (iphdrp->protocol == IPPROTO_UDP)) {
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
	    udphdrp = skb->h.uh;
#else
        udphdrp = udp_hdr(skb);
#endif
            if (udphdrp) {
                if (udphdrp == (struct udphdr *)iphdrp) {
                    udphdrp = (struct udphdr *)((char *)udphdrp + (4*(iphdrp->ihl)));
                }
                /* Find configured UDP Source Port*/
                for (i=0; i < sizeof(port_spec_cfg[port].udp_src)/sizeof(port_spec_cfg[port].udp_src[0]);i++) {
                    if ((udphdrp->source == port_spec_cfg[port].udp_src[i].udp_port) &&
                        (port_spec_cfg[port].udp_src[i].tx_spec.txq != MV_ETH_TXQ_INVALID)) {
                        memcpy (tx_spec_out, &(port_spec_cfg[port].udp_src[i].tx_spec), sizeof(struct mv_eth_tx_spec));
                        if (mv_cust_debug_code)
                            printk("%s: found udp_src 0x(%04x)\n", __func__, ntohs(udphdrp->source));
                        return 1;
                    }
                }
                /* Find configured UDP Dest. Port*/
                for (i=0; i < sizeof(port_spec_cfg[port].udp_dst)/sizeof(port_spec_cfg[port].udp_dst[0]);i++) {
                    if ((udphdrp->dest == port_spec_cfg[port].udp_dst[i].udp_port) &&
                        (port_spec_cfg[port].udp_src[i].tx_spec.txq != MV_ETH_TXQ_INVALID)) {
                        memcpy (tx_spec_out, &(port_spec_cfg[port].udp_dst[i].tx_spec), sizeof(struct mv_eth_tx_spec));
                        if (mv_cust_debug_code)
                            printk("%s: found udp_dst 0x(%04x)\n", __func__, ntohs(udphdrp->dest));
                        return 1;
                    }
                }

        if ( (port == MV_PON_PORT_ID) && mv_cust_debug_code) {
            printk("%s:DEBUG : Packet UDP, udp source or dest port not found udp_src(%x)x udp_dst(%x)x\n",
                   __func__,ntohs(udphdrp->source), ntohs(udphdrp->dest));
        }

            }
        }
        else if ( (port == MV_PON_PORT_ID) && mv_cust_debug_code )
            printk("%s:DEBUG : NOT UDP, ip_proto(%d) \n", __func__, iphdrp->protocol);
    }
    else if ( (port == MV_PON_PORT_ID) && mv_cust_debug_code )
        printk("%s:DEBUG : NOT IP, proto(%d) \n", __func__, skb->protocol);

    return 0;

}
#endif

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
int mv_cust_flow_map_tx(int port, struct net_device *dev, struct sk_buff *skb,
                        struct mv_eth_tx_spec *tx_spec_out)
{
    struct iphdr *iphdrp = NULL;
    uint16_t vlan        = 0;
    uint8_t  pbits       = 0;
    int      btag        = 0;
    int      ret         = 0;
    mv_cust_ioctl_flow_map_t cust_flow;

    if (MV_PON_PORT_ID != port)
        return 0;

    if (mv_cust_flow_map) {

        cust_flow.dir  = MV_CUST_FLOW_DIR_US;

        /* Parse packets to check whether it is tagged or untagged, and get vlan and pbits in tagged mode */
        btag = mv_cust_flow_map_parse(skb->data, &vlan, &pbits, MV_CUST_FLOW_DIR_US);

        //printk(KERN_ERR " %s TX packet 1 btag[%d] vlan[%d]  pbits[%d]\n", __func__, btag, vlan, pbits);

        /* The frame is tagged */
        if (btag == 1) {
            cust_flow.vid    = vlan;
            cust_flow.pbits  = pbits;

            ret = mv_cust_tag_map_rule_get(&cust_flow);
            //printk(KERN_ERR " %s TX packet 2 trg_port[%d] trg_queue[%d]  gem_port[%d]\n", __func__, pkt_fwd.trg_port, pkt_fwd.trg_queue, pkt_fwd.gem_port);

            /* Set GEM port, target port and queue */
            if (ret == MV_CUST_OK) {
                    tx_spec_out->txp     = (u8)cust_flow.pkt_frwd.trg_port;
                    tx_spec_out->txq     = (u8)cust_flow.pkt_frwd.trg_queue;
                    tx_spec_out->hw_cmd  = ((cust_flow.pkt_frwd.gem_port << 8)|0x0010);
                    tx_spec_out->tx_func = NULL;
                    tx_spec_out->flags   = MV_ETH_F_MH;

                    /* modify VID and P-bits if needed */
                    ret = mv_cust_flow_map_mod(skb->data, cust_flow.mod_vid, cust_flow.mod_pbits, MV_CUST_FLOW_DIR_US);
                    return 1;
            }
            else /* Look for rules that does not care VLAN */
            {
                cust_flow.vid   = MV_CUST_DEFAULT_SINGLE_TAG_RULE;
                ret = mv_cust_tag_map_rule_get(&cust_flow);

                /* Set GEM port, target port and queue */
                if (ret == MV_CUST_OK) {
                        tx_spec_out->txp     = (u8)cust_flow.pkt_frwd.trg_port;
                        tx_spec_out->txq     = (u8)cust_flow.pkt_frwd.trg_queue;
                        tx_spec_out->hw_cmd  = ((cust_flow.pkt_frwd.gem_port << 8)|0x0010);
                        tx_spec_out->tx_func = NULL;
                        tx_spec_out->flags   = MV_ETH_F_MH;

                        /* modify VID and P-bits if needed */
                        ret = mv_cust_flow_map_mod(skb->data, cust_flow.mod_vid, cust_flow.mod_pbits, MV_CUST_FLOW_DIR_US);
                        return 1;
                }
            }
        }
        /* The frame is untagged, try to get DSCP value */
        else {
            if (skb->protocol == ETY_IPV4) {
                /* Get UDP Port */
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,22)
                iphdrp = skb->nh.iph;
#else
                iphdrp = ip_hdr(skb);
#endif
                cust_flow.dscp  = (iphdrp->tos >> 2) & 0x3f;
                cust_flow.dir   = MV_CUST_FLOW_DIR_US;

                ret = mv_cust_untag_map_rule_get(&cust_flow);
                /* Set GEM port, target port and queue */
                if (ret == MV_CUST_OK) {
                        tx_spec_out->txp     = cust_flow.pkt_frwd.trg_port;
                        tx_spec_out->txq     = cust_flow.pkt_frwd.trg_queue;
                        tx_spec_out->hw_cmd  = ((cust_flow.pkt_frwd.gem_port << 8)|0x0010);
                        tx_spec_out->tx_func = NULL;
                        tx_spec_out->flags   = MV_ETH_F_MH;;

                        return 1;
                }
            }
            else
            {
                cust_flow.dscp  = 0;
                cust_flow.dir   = MV_CUST_FLOW_DIR_US;

                ret = mv_cust_untag_map_rule_get(&cust_flow);
                /* Set GEM port, target port and queue */
                if (ret == MV_CUST_OK) {
                        tx_spec_out->txp     = cust_flow.pkt_frwd.trg_port;
                        tx_spec_out->txq     = cust_flow.pkt_frwd.trg_queue;
                        tx_spec_out->hw_cmd  = ((cust_flow.pkt_frwd.gem_port << 8)|0x0010);
                        tx_spec_out->tx_func = NULL;
                        tx_spec_out->flags   = MV_ETH_F_MH;;

                        return 1;
                }
            }
        }
    }
    return 0;
}
#endif

void mv_cust_debug_info_set(int val)
{
    mv_cust_debug_code = val;
    return;
}

void mv_cust_omci_type_set(uint16_t type)
{
    mv_cust_xpon_oam_type = htons(type);
    return;
}
EXPORT_SYMBOL(mv_cust_omci_type_set);

void mv_cust_epon_oam_type_set(uint16_t type)
{
    mv_cust_xpon_oam_type = htons(type);
    return;
}
EXPORT_SYMBOL(mv_cust_epon_oam_type_set);


void mv_cust_xpon_oam_rx_gh_set(int val)
{
    mv_cust_xpon_oam_rx_gh = val;
    return;
}
EXPORT_SYMBOL(mv_cust_xpon_oam_rx_gh_set);


#ifdef CONFIG_MV_CUST_IGMP_HANDLE
void mv_cust_igmp_type_set(uint16_t type)
{
    mv_cust_igmp_type = htons(type);
    return;
}
EXPORT_SYMBOL(mv_cust_igmp_type_set);
#endif

void mv_cust_loopdet_type_set(uint16_t type)
{
    mv_cust_loopdet_type = htons(type);
    return;
}
EXPORT_SYMBOL(mv_cust_loopdet_type_set);


void mv_cust_omci_gemport_set(int gemport)
{
    mv_cust_omci_gemport = gemport;
    return;
}
EXPORT_SYMBOL(mv_cust_omci_gemport_set);


static int mv_cust_omci_gem_parse(uint8_t *data)
{
    uint16_t gh;

    gh = ntohs(*(uint16_t *)data);

    if(mv_cust_debug_code)
        printk("%s:gh= 0x(%04x) - mv_cust_omci_gemport= 0x(%04x)  \n", __func__, gh, mv_cust_omci_gemport);

    /* Compare GH for omci_gemport */
    if ( (gh & MH_GEM_PORT_MASK) != mv_cust_omci_gemport ) {
        if(mv_cust_debug_code)
            printk("%s: compare GH for OMCI_gemport failed: gh= 0x(%04x) - mv_cust_omci_gemport= 0x(%04x)  \n", __func__, gh, mv_cust_omci_gemport);
        return(0);
    }

    return(1);
}

static int mv_cust_eoam_type_parse(uint8_t *data)
{
    uint16_t ety;

    ety = ntohs(*(uint16_t *)(data + MV_ETH_MH_SIZE + ETH_ALEN + ETH_ALEN));

    if(mv_cust_debug_code)
        printk("%s: ety 0x(%04x)\n", __func__, ety);

    /* Compare EPON OAM ether_type */
    if (ety == MH_EPON_OAM_TYPE)
        return(1);

    return(0);
}


#ifdef CONFIG_MV_CUST_IGMP_HANDLE
static int mv_cust_igmp_parse(uint8_t *data)
{
    uint16_t ety;
    uint8_t  proto;
    uint8_t *fieldp = data + MV_ETH_MH_SIZE + ETH_ALEN + ETH_ALEN;

    /* Loop through VLAN tags */
    ety = ntohs(*(uint16_t *)fieldp);
    while (ety == 0x8100 || ety == 0x88A8) {
        fieldp+= VLAN_HLEN;
        ety = ntohs(*(uint16_t *)fieldp);
    }

    if(mv_cust_debug_code)
        printk("%s:ety 0x(%04x)\n", __func__, ety);

    if (ety == ETY_IPV4) {
	    fieldp+= 2;
        fieldp+= IPV4_PROTO_OFFSET;
        proto = *fieldp;
        if (mv_cust_debug_code)
            printk("%s:proto 0x(%02x)\n", __func__, proto);

        if (proto == IPPROTO_IGMP)
            return(1);
    }

    return(0);
}
#endif

static int mv_cust_loopdet_parse(uint8_t *data)
{
    uint16_t ety;
    uint8_t *fieldp = data + MV_ETH_MH_SIZE + ETH_ALEN + ETH_ALEN;

    /* Loop through VLAN tags */
    ety = ntohs(*(uint16_t *)fieldp);
    while (ety == 0x8100 || ety == 0x88A8) {
        fieldp+= VLAN_HLEN;
        ety = ntohs(*(uint16_t *)fieldp);
    }
    if(mv_cust_debug_code)
        printk("%s: ety 0x(%04x)\n", __func__, ety);

    /* Compare EPON OAM ether_type */
    if (ety == mv_cust_loopdet_type)
        return(1);

    return(0);
}

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
static int mv_cust_flow_map_parse(uint8_t *data, uint16_t *vlan, uint8_t *pbits, uint8_t dir)
{
    uint16_t ety;
    uint8_t *fieldp;

    if (MV_CUST_FLOW_DIR_US == dir)
        fieldp = data + ETH_ALEN + ETH_ALEN;
    else if (MV_CUST_FLOW_DIR_DS == dir)
        fieldp = data + ETH_ALEN + ETH_ALEN + MV_ETH_MH_SIZE;

    /* Loop through VLAN tags */
    ety = ntohs(*(uint16_t *)fieldp);
    if (ety == 0x8100 || ety == 0x88A8 || ety == 0x9100) {
        fieldp += 2;
        *vlan  = ntohs(*(uint16_t *)fieldp);
        *pbits = (*vlan >> 13 ) & 0x7;
        *vlan  = (*vlan) & 0xfff;
        return(1);
    }
    else {
        return(0);
    }

    return(0);
}

static int mv_cust_flow_map_mod(uint8_t *data, uint16_t vid, uint8_t pbits, uint8_t dir)
{
    uint16_t ety  = 0;
    uint16_t vlan = 0;
    uint8_t *fieldp;

    if (MV_CUST_FLOW_DIR_US == dir)
        fieldp = data + ETH_ALEN + ETH_ALEN;
    else if (MV_CUST_FLOW_DIR_DS == dir)
        fieldp = data + ETH_ALEN + ETH_ALEN + MV_ETH_MH_SIZE;

    /* If not need to modify VID or P-bits */
    if((vid == MV_CUST_VID_NOT_CARE_VALUE) &&
       (pbits == MV_CUST_PBITS_NOT_CARE_VALUE))
        return (1);

    /* Loop through VLAN tags */
    ety = ntohs(*(uint16_t *)fieldp);
    if (ety == 0x8100 || ety == 0x88A8 || ety == 0x9100) {
        fieldp += 2;

        vlan = ntohs(*(uint16_t *)fieldp);

        if (vid < MV_CUST_VID_NOT_CARE_VALUE)
            vlan = (vlan & 0xf000) | (vid & 0xfff);
        if (pbits < MV_CUST_PBITS_NOT_CARE_VALUE)
            vlan = (vlan & 0x0fff) | ((pbits & 0x7) << 13);

        *(uint16_t *)fieldp = htons(vlan);
        return(1);
    }
    else {
        return(0);
    }

    return(0);
}

#endif

void mv_cust_rec_skb(int port, struct sk_buff *skb)
{
    uint32_t rx_status;
    struct eth_port *pp;

    rx_status = netif_receive_skb(skb);
    pp = mv_eth_port_by_id(port);
    STAT_DBG(if (rx_status) (pp->stats.rx_drop_sw++));
}


static int mv_cust_omci_rx(int port, struct net_device *dev, struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
    uint32_t rx_bytes;

    if (!mv_cust_omci_gem_parse(skb->data))
        return 0;
    if (mv_cust_xpon_oam_rx_gh) {
        rx_bytes = rx_desc->dataSize;
    }
    else {
        skb->data += MV_ETH_MH_SIZE;
        rx_bytes = rx_desc->dataSize - MV_ETH_MH_SIZE;
    }
    skb->tail += rx_bytes;
    skb->len = rx_bytes;
    skb->protocol = eth_type_trans(skb, dev);
    skb->protocol = mv_cust_xpon_oam_type;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    skb->dev = dev;
#endif
    mv_cust_rec_skb(port, skb);

    return 1;
}

static int mv_cust_epon_oam_rx(int port, struct net_device *dev, struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
    uint32_t rx_bytes;

    if (!mv_cust_eoam_type_parse(skb->data))
        return 0;

    if (mv_cust_xpon_oam_rx_gh) {
        rx_bytes = rx_desc->dataSize;
    }
    else {
        skb->data += MV_ETH_MH_SIZE;
        rx_bytes = rx_desc->dataSize - MV_ETH_MH_SIZE;
    }

    skb->tail += rx_bytes;
    skb->len = rx_bytes;
    skb->protocol = eth_type_trans(skb, dev);
    skb->protocol = mv_cust_xpon_oam_type;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    skb->dev = dev;
#endif
    mv_cust_rec_skb(port, skb);

    return 1;
}

#ifdef CONFIG_MV_CUST_IGMP_HANDLE
static int mv_cust_igmp_rx(int port, struct net_device *dev, struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
    uint32_t rx_bytes;

    if (!mv_cust_igmp_parse(skb->data))
        return 0;

    /* To Indicate the source GMAC */
    skb->data[0] = port;

    rx_bytes = rx_desc->dataSize;

    skb->tail += rx_bytes;
    skb->len = rx_bytes;
    skb->protocol = eth_type_trans(skb, dev);
    skb->protocol = mv_cust_igmp_type;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    skb->dev = dev;
#endif
    mv_cust_rec_skb(port, skb);

    return 1;
}
#endif

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
static int mv_cust_flow_map_rx(int port, struct net_device *dev, struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
    uint32_t rx_bytes;
    uint16_t vlan        = 0;
    uint8_t  pbits       = 0;
    int      btag        = 0;
    int      ret         = 0;
    mv_cust_ioctl_flow_map_t cust_flow;

    if (MV_PON_PORT_ID != port)
        return 0;

    if (mv_cust_flow_map) {

        /* Parse packets to check whether it is tagged or untagged, and get vlan and pbits in tagged mode */
        btag = mv_cust_flow_map_parse(skb->data, &vlan, &pbits, MV_CUST_FLOW_DIR_DS);
        //printk(KERN_ERR " %s TX packet 1 btag[%d] vlan[%d]  pbits[%d]\n", __func__, btag, vlan, pbits);

        /* The frame is tagged */
        if (btag == 1) {
            cust_flow.vid    = vlan;
            cust_flow.pbits  = pbits;
            cust_flow.dir    = MV_CUST_FLOW_DIR_DS;

            ret = mv_cust_tag_map_rule_get(&cust_flow);
            //printk(KERN_ERR " %s TX packet 2 trg_port[%d] trg_queue[%d]  gem_port[%d]\n", __func__, pkt_fwd.trg_port, pkt_fwd.trg_queue, pkt_fwd.gem_port);

            /* Modify VID and P-bits if needed */
            if (ret == MV_CUST_OK) {

                /* modify VID and P-bits if needed */
                ret = mv_cust_flow_map_mod(skb->data, cust_flow.mod_vid, cust_flow.mod_pbits, MV_CUST_FLOW_DIR_DS);
            }
        }
    }

    return 1;
}
#endif

static int mv_cust_loopdet_rx(int port, struct net_device *dev, struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
    uint32_t rx_bytes;

    if (!mv_cust_loopdet_parse(skb->data))
        return 0;

    rx_bytes = rx_desc->dataSize;

    skb->tail += rx_bytes;
    skb->len = rx_bytes;
    skb->protocol = eth_type_trans(skb, dev);
    skb->protocol = mv_cust_loopdet_type;
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
    skb->dev = dev;
#endif
    mv_cust_rec_skb(port, skb);

    return 1;
}

void mv_cust_rx_func(int port, int rxq, struct net_device *dev,
                     struct sk_buff *skb, struct neta_rx_desc *rx_desc)
{
    uint32_t i;

    if(mv_cust_debug_code)
        printk("%s\n", __func__);

    if (dev == NULL || skb == NULL || rx_desc == NULL) {
        printk("%s: NULL Pointer dev(%p) skb(%p) rx_desc(%p)\n",
               __func__, dev, skb, rx_desc);
        return;
    }

    if (rx_desc->pncInfo & NETA_PNC_RX_SPECIAL)
    {
        if (mv_cust_omci_valid) {
            if (mv_cust_omci_rx(port, dev, skb, rx_desc)) {
                if(mv_cust_debug_code)
                    printk("%s omci_packet\n", __func__);
                return;
            }
        }
        else{
            if (mv_cust_eoam_valid) {
                if (mv_cust_epon_oam_rx(port, dev, skb, rx_desc)) {
                    if (mv_cust_debug_code)
                        printk("%s eoam_packet\n", __func__);
                    return;
                }
            }
        }

#ifdef CONFIG_MV_CUST_IGMP_HANDLE
        if (mv_cust_igmp_detect) {
            if (mv_cust_igmp_rx(port, dev, skb, rx_desc)) {
                if(mv_cust_debug_code)
                    printk("%s igmp_packet\n", __func__);
                return;
            }
        }
#endif

        if (mv_cust_port_loopback_detect) {
            if (mv_cust_loopdet_rx(port, dev, skb, rx_desc)) {
                if(mv_cust_debug_code)
                    printk("%s loop_det_packet\n", __func__);
                return;
            }
        }

        MVCUST_ERR_PRINT("Special pkt arrived from port(%d), was not handled. \n", port);
        dev_kfree_skb_any(skb);
        if(mv_cust_debug_code) {
            printk("Input Packet first bytes:\n");
            for (i=0;i<24;i++) {
                if (i%8== 0)
                    printk("\n");
                printk ("%02x ", *(skb->data + i));
            }
        }
        
    }
    else
    {
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
        if (mv_cust_flow_map) {
            if (mv_cust_flow_map_rx(port, dev, skb, rx_desc)) {
                if(mv_cust_debug_code)
                    printk("%s flow map\n", __func__);
            }
        }
        return;
#endif
    }

    return;
}

int mv_cust_tx_func(int port, struct net_device *dev, struct sk_buff *skb,
                    struct mv_eth_tx_spec *tx_spec_out)
{
    if (mv_cust_omci_tx(port, dev, skb, tx_spec_out))
        return 1;

    if (mv_cust_eoam_tx(port, dev, skb, tx_spec_out))
        return 1;

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
    if (mv_cust_flow_map_tx(port, dev, skb, tx_spec_out))
        return 1;
#endif

#ifdef CONFIG_MV_CUST_IGMP_HANDLE
    if (mv_cust_igmp_tx(port, dev, skb, tx_spec_out))
        return 1;
#endif

#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
    /* Simple example for UDP Source Port or Dest Port Parsing */
    /* Per UDP source or dest port configuration */
    if (mv_cust_udp_port_tx(port, dev, skb, tx_spec_out))
        return 1;
#endif

    return 0;
}

int mvcust_netdev_init(void)
{
    uint32_t port_i;

    /* Retrieve num_ports, as in mv_eth_init_module*/
    mv_eth_ports_num = mvCtrlEthMaxPortGet();

    if (mv_eth_ports_num > CONFIG_MV_ETH_PORTS_NUM)
        mv_eth_ports_num = CONFIG_MV_ETH_PORTS_NUM;

    mv_cust_eoam_init();

/* Initialize flow mapping data structure */
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
    mv_cust_flow_map_init();
#endif

#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
    mv_cust_udp_table_del();
#endif

    /* Register special receive check function */
#ifdef CONFIG_MV_ETH_RX_SPECIAL
    for (port_i=0;port_i< mv_eth_ports_num;port_i++) {
        mv_eth_rx_special_proc_func(port_i, mv_cust_rx_func);
    }
#endif /* CONFIG_MV_ETH_RX_SPECIAL */


    /* Register special transmit check function */
#ifdef CONFIG_MV_ETH_TX_SPECIAL
    for (port_i=0;port_i< mv_eth_ports_num;port_i++) {
        mv_eth_tx_special_check_func(port_i, mv_cust_tx_func);
    }
#endif /* CONFIG_MV_ETH_TX_SPECIAL */

    /* Set global constants */
    mv_cust_xpon_oam_type = htons(0xBABA);

#ifdef CONFIG_MV_CUST_IGMP_HANDLE
    mv_cust_igmp_type     = htons(0xA000);
#endif
    mv_cust_loopdet_type  = htons(0xA0A0);

    return 0;
}
