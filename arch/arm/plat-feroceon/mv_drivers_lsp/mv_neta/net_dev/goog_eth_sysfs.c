/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) Google, 2013
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>

#include "gbe/mvNeta.h"
#include "mv_netdev.h"

struct goog_net_attribute {
	struct device_attribute dev_attr;
	uint64_t val;
	uint32_t mib;
	uint32_t port;
};

#define GOOG_ATTR(_name, _portname, _show, _mib, _port)                 \
		struct goog_net_attribute goog_attr_##_name##_##_portname = {      \
			.dev_attr = __ATTR(_name, S_IRUSR, _show, NULL), \
			.mib= _mib,                             \
			.port = _port,                                   \
		}

static ssize_t goog_get_stat(struct device *dev,
							 struct device_attribute *attr, char *buf)
{
	MV_U32 reg_low, reg_high = 0;
	uint64_t reg_val;
	struct goog_net_attribute *goog_attr = container_of(attr, struct goog_net_attribute, dev_attr);
        // These mib registers are clear on read.
	reg_low = mvNetaMibCounterRead(goog_attr->port, 0, goog_attr->mib, &reg_high);
	reg_val = ((uint64_t)reg_high) << 32 | (uint64_t)reg_low;
	goog_attr->val += reg_val;
	return sprintf(buf, "%llu\n", goog_attr->val);
}

// Stats for the UNI port.
static GOOG_ATTR(rx_good_octets, uni, goog_get_stat, ETH_MIB_GOOD_OCTETS_RECEIVED_LOW, 0);
static GOOG_ATTR(rx_bad_octets, uni, goog_get_stat, ETH_MIB_BAD_OCTETS_RECEIVED, 0);
static GOOG_ATTR(rx_good_packets, uni, goog_get_stat, ETH_MIB_GOOD_FRAMES_RECEIVED, 0);
static GOOG_ATTR(rx_bad_packets, uni, goog_get_stat, ETH_MIB_BAD_FRAMES_RECEIVED, 0);
static GOOG_ATTR(rx_broadcast_packets, uni, goog_get_stat, ETH_MIB_BROADCAST_FRAMES_RECEIVED, 0);
static GOOG_ATTR(rx_multicast_packets, uni, goog_get_stat, ETH_MIB_MULTICAST_FRAMES_RECEIVED, 0);
static GOOG_ATTR(rx_bad_fc, uni, goog_get_stat, ETH_MIB_BAD_FC_RECEIVED, 0);
static GOOG_ATTR(rx_undersized, uni, goog_get_stat, ETH_MIB_UNDERSIZE_RECEIVED, 0);
static GOOG_ATTR(rx_fragments, uni, goog_get_stat, ETH_MIB_FRAGMENTS_RECEIVED, 0);
static GOOG_ATTR(rx_oversized, uni, goog_get_stat, ETH_MIB_OVERSIZE_RECEIVED, 0);
static GOOG_ATTR(rx_jabber, uni, goog_get_stat, ETH_MIB_JABBER_RECEIVED, 0);
static GOOG_ATTR(rx_mac_error, uni, goog_get_stat, ETH_MIB_MAC_RECEIVE_ERROR, 0);
static GOOG_ATTR(rx_crc_error, uni, goog_get_stat, ETH_MIB_BAD_CRC_EVENT, 0);
static GOOG_ATTR(tx_good_octets, uni, goog_get_stat, ETH_MIB_GOOD_OCTETS_SENT_LOW, 0);
static GOOG_ATTR(tx_good_packets, uni, goog_get_stat, ETH_MIB_GOOD_FRAMES_SENT, 0);
static GOOG_ATTR(tx_broadcast_packets, uni, goog_get_stat, ETH_MIB_BROADCAST_FRAMES_SENT, 0);
static GOOG_ATTR(tx_multicast_packets, uni, goog_get_stat, ETH_MIB_MULTICAST_FRAMES_SENT, 0);

static struct attribute *goog_uni_attrs[] = {
	&goog_attr_rx_good_octets_uni.dev_attr.attr,
	&goog_attr_rx_bad_octets_uni.dev_attr.attr,
	&goog_attr_rx_good_packets_uni.dev_attr.attr,
	&goog_attr_rx_bad_packets_uni.dev_attr.attr,
	&goog_attr_rx_multicast_packets_uni.dev_attr.attr,
	&goog_attr_rx_broadcast_packets_uni.dev_attr.attr,
	&goog_attr_rx_bad_fc_uni.dev_attr.attr,
	&goog_attr_rx_undersized_uni.dev_attr.attr,
	&goog_attr_rx_fragments_uni.dev_attr.attr,
	&goog_attr_rx_oversized_uni.dev_attr.attr,
	&goog_attr_rx_jabber_uni.dev_attr.attr,
	&goog_attr_rx_mac_error_uni.dev_attr.attr,
	&goog_attr_rx_crc_error_uni.dev_attr.attr,
	&goog_attr_tx_good_octets_uni.dev_attr.attr,
	&goog_attr_tx_good_packets_uni.dev_attr.attr,
	&goog_attr_tx_broadcast_packets_uni.dev_attr.attr,
	&goog_attr_tx_multicast_packets_uni.dev_attr.attr,
	NULL
};


static struct attribute_group goog_uni_stats = {
	.name = "unistats",
	.attrs = goog_uni_attrs,
};


// Stats for the ANI port.
static GOOG_ATTR(rx_good_octets, ani, goog_get_stat, ETH_MIB_GOOD_OCTETS_RECEIVED_LOW, MV_PON_PORT_ID);
static GOOG_ATTR(rx_bad_octets, ani, goog_get_stat, ETH_MIB_BAD_OCTETS_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_good_packets, ani, goog_get_stat, ETH_MIB_GOOD_FRAMES_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_bad_packets, ani, goog_get_stat, ETH_MIB_BAD_FRAMES_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_broadcast_packets, ani, goog_get_stat, ETH_MIB_BROADCAST_FRAMES_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_multicast_packets, ani, goog_get_stat, ETH_MIB_MULTICAST_FRAMES_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_bad_fc, ani, goog_get_stat, ETH_MIB_BAD_FC_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_undersized, ani, goog_get_stat, ETH_MIB_UNDERSIZE_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_fragments, ani, goog_get_stat, ETH_MIB_FRAGMENTS_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_oversized, ani, goog_get_stat, ETH_MIB_OVERSIZE_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_jabber, ani, goog_get_stat, ETH_MIB_JABBER_RECEIVED, MV_PON_PORT_ID);
static GOOG_ATTR(rx_mac_error, ani, goog_get_stat, ETH_MIB_MAC_RECEIVE_ERROR, MV_PON_PORT_ID);
static GOOG_ATTR(rx_crc_error, ani, goog_get_stat, ETH_MIB_BAD_CRC_EVENT, MV_PON_PORT_ID);
static GOOG_ATTR(tx_good_octets, ani, goog_get_stat, ETH_MIB_GOOD_OCTETS_SENT_LOW, MV_PON_PORT_ID);
static GOOG_ATTR(tx_good_packets, ani, goog_get_stat, ETH_MIB_GOOD_FRAMES_SENT, MV_PON_PORT_ID);
static GOOG_ATTR(tx_broadcast_packets, ani, goog_get_stat, ETH_MIB_BROADCAST_FRAMES_SENT, MV_PON_PORT_ID);
static GOOG_ATTR(tx_multicast_packets, ani, goog_get_stat, ETH_MIB_MULTICAST_FRAMES_SENT, MV_PON_PORT_ID);

static struct attribute *goog_ani_attrs[] = {
	&goog_attr_rx_good_octets_ani.dev_attr.attr,
	&goog_attr_rx_bad_octets_ani.dev_attr.attr,
	&goog_attr_rx_good_packets_ani.dev_attr.attr,
	&goog_attr_rx_bad_packets_ani.dev_attr.attr,
	&goog_attr_rx_multicast_packets_ani.dev_attr.attr,
	&goog_attr_rx_broadcast_packets_ani.dev_attr.attr,
	&goog_attr_rx_bad_fc_ani.dev_attr.attr,
	&goog_attr_rx_undersized_ani.dev_attr.attr,
	&goog_attr_rx_fragments_ani.dev_attr.attr,
	&goog_attr_rx_oversized_ani.dev_attr.attr,
	&goog_attr_rx_jabber_ani.dev_attr.attr,
	&goog_attr_rx_mac_error_ani.dev_attr.attr,
	&goog_attr_rx_crc_error_ani.dev_attr.attr,
	&goog_attr_tx_good_octets_ani.dev_attr.attr,
	&goog_attr_tx_good_packets_ani.dev_attr.attr,
	&goog_attr_tx_broadcast_packets_ani.dev_attr.attr,
	&goog_attr_tx_multicast_packets_ani.dev_attr.attr,
	NULL
};

static struct attribute_group goog_ani_stats = {
	.name = "anistats",
	.attrs = goog_ani_attrs,
};

int __devinit goog_eth_sysfs_init(void)
{
	int err = 0;
	struct device *pd = NULL;

	pd = bus_find_device_by_name(&platform_bus_type, NULL, "neta");
	if (!pd) {
		platform_device_register_simple("neta", -1, NULL, 0);
		pd = bus_find_device_by_name(&platform_bus_type, NULL, "neta");
	}

	if (!pd) {
		printk(KERN_ERR"%s: cannot find neta device\n", __func__);
		pd = &platform_bus;
	}

	err = sysfs_create_group(&pd->kobj, &goog_uni_stats);
	if (err) {
		printk(KERN_INFO "sysfs group failed %d\n", err);
		goto out;
	}

	err = sysfs_create_group(&pd->kobj, &goog_ani_stats);
	if (err) {
		printk(KERN_INFO "sysfs group failed %d\n", err);
		goto out;
	}
out:
	return err;
}



module_init(goog_eth_sysfs_init);

MODULE_AUTHOR("jnewlin@google.com");
MODULE_DESCRIPTION("Readable netstats for Marvell 6601.");
MODULE_LICENSE("GPL");
