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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>

#include "tpm_common.h"
#include "tpm_header.h"

static ssize_t tpm_sysfs_print_help(char *buf)
{
	int off = 0;
#ifdef CONFIG_MV_TPM_SYSFS_HELP
	off += sprintf(buf + off,
		"cat help                                          - show this help\n");
	off += sprintf(buf + off,
		"cat etherports                                    - show TPM DB ethernet port configuration\n");
	off += sprintf(buf + off, "cat rx_modules                                    - show TPM DB  rx configuration \n");
	off += sprintf(buf + off,
		"cat tx_modules                                    - show TPM DB tx modules configuration\n");
	off += sprintf(buf + off, "cat gmac_cfg                                      - show TPM DB GMAC configuration\n");
	off += sprintf(buf + off,
		"cat gmac_func                                     - show TPM DB GMAC function configuration \n");
	off += sprintf(buf + off, "cat igmp_cfg                                      - show TPM DB IGMP configuration \n");
	off += sprintf(buf + off,
		"cat misc_cfg                                      - show TPM DB Misc. configuration \n");
	off += sprintf(buf + off,
		"cat vlan_etype                                    - show TPM DB VLAN ethertypes configuration \n");
	off += sprintf(buf + off,
		"cat valid_api_section                             - show TPM DB API ranges configuration \n");
	off += sprintf(buf + off,
		"cat valid_pnc_range                               - show TPM DB PNC Ranges configuration \n");
	off += sprintf(buf + off,
		"cat init_tables                                   - show TPM DB Initialization configuration \n");
	off += sprintf(buf + off,
		"cat mtu_cfg                                       - show MTU running configuration \n");
	off += sprintf(buf + off,
		"cat tcam_hw_hits                                  - show TCAM HW entries hit table\n");
	off += sprintf(buf + off,
		"cat show_omci_channel                             - show gpon OMCI channel info\n");
	off += sprintf(buf + off,
		"cat show_oam_channel                              - show epon OAM channel info\n");
	off += sprintf(buf + off,
		"cat tcam_hw_hits_dump                             "\
		"- show TCAM HW entries hit table including per hit TCAM/SRAM data\n");
	off += sprintf(buf+off,
		"cat api_dump                                      "
		"- show TPM DB all API types (l2/l3/ipv4/ipv4mc/ipv6 api's)\n");
	off += sprintf(buf+off,
		"cat busy_apis                                      "
		"- debug info. Show TPM API's that are in an active API call. Should always be empty. \n");
	off += sprintf(buf + off,
		"cat show_mac_learn_entry_count                     "
		"- show MAC learn entry count. \n");
	off += sprintf(buf + off,
		 "cat show_fc_config                                - show FC engine configuration/statistics\n");
	off += sprintf(buf + off,
		 "echo 0 > show_fc_config                           - clear FC engine statistics\n");
	off += sprintf(buf + off,
		"echo section                 > api_section        "
		"- show TPM DB specific API Section full configuration \n");
	off += sprintf(buf + off,
		"echo valid_only st end       > pnc_shadow_range   "
		"- show TPM DB shadow PNC tables (start_entry, end_entry)\n");
	off += sprintf(buf + off,
		"echo <port>                  > tcam_hw_hits       "
		"- start recording for port (GMAC0-%d GMAC1-%d PONMAC-%d)\n", TPM_ENUM_GMAC_0, TPM_ENUM_GMAC_1, TPM_ENUM_PMAC);
#else
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __func__);
#endif
	return off;
}

static ssize_t tpm_sysfs_print_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int off = 0;
	const char *name = attr->attr.name;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "help"))
		off = tpm_sysfs_print_help(buf);
	else if (!strcmp(name, "etherports"))
		tpm_print_etherports();
	else if (!strcmp(name, "rx_modules"))
		tpm_print_rx_modules();
	else if (!strcmp(name, "tx_modules"))
		tpm_print_tx_modules();
	else if (!strcmp(name, "gmac_cfg"))
		tpm_print_gmac_config();
	else if (!strcmp(name, "gmac_func"))
		tpm_print_gmac_func();
	else if (!strcmp(name, "igmp_cfg"))
		tpm_print_igmp();
	else if (!strcmp(name, "misc_cfg"))
		tpm_print_misc();
	else if (!strcmp(name, "vlan_etype"))
		tpm_print_vlan_etype();
	else if (!strcmp(name, "valid_api_section"))
		tpm_print_valid_api_sections();
	else if (!strcmp(name, "valid_pnc_range"))
		tpm_print_valid_pnc_ranges();
	else if (!strcmp(name, "init_tables"))
		tpm_print_init_tables();
	else if (!strcmp(name, "mtu_cfg"))
		tpm_db_show_mtu_cfg();
	else if (!strcmp(name, "length_check_range"))
		tpm_mtu_print_len_chk_range();
	else if (!strcmp(name, "api_dump"))
		tpm_print_api_dump_all();
	else if (!strcmp(name, "pnc_dump"))
	      tpm_print_pnc();
	else if (!strcmp(name, "tcam_hw_hits"))
		return tpm_tcam_hw_hits(DONT_PRINT_PNC_ENTRY);
	else if (!strcmp(name, "tcam_hw_hits_dump"))
		return tpm_tcam_hw_hits(PRINT_PNC_ENTRY);
	else if (!strcmp(name, "age_pnc_dump"))
		return tpm_age_pnc_dump();
	else if (!strcmp(name, "age_pnc_dump_live"))
		return tpm_age_pnc_dump_live();
	else if (!strcmp(name, "age_pnc_dump_hdrs"))
		tpm_print_pnc_field_desc();
	else if (!strcmp(name, "age_dump"))
		mvPncAgingDump(0);
	else if (!strcmp(name, "age_dump_all"))
		mvPncAgingDump(1);
	else if (!strcmp(name, "mc_vlan_dump"))
		tpm_print_mc_vlan_cfg_all();
	else if (!strcmp(name, "show_omci_channel"))
		tpm_print_gpon_omci_channel();
	else if (!strcmp(name, "show_oam_channel"))
		tpm_print_epon_oam_channel();
	else if (!strcmp(name, "busy_apis"))
		tpm_print_busy_apis();
	else if (!strcmp(name, "show_mac_learn_entry_count"))
		tpm_print_mac_learn_entry_count();
	else if (!strcmp(name, "show_fc_config"))
		tpm_print_fc(1);
	else
		off = tpm_sysfs_print_help(buf);

	return off;
}

#if 0				/*Keep as placeholder*/
static ssize_t tpm_sysfs_print_1_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	unsigned int v;
	unsigned long flags;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read input */
	v = 0;

	sscanf(buf, "%x", &v);

	raw_local_irq_save(flags);

	if (!strcmp(name, "place_holder"))
		XXXXX(v);
	else
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);

	raw_local_irq_restore(flags);

	return len;
}
#endif

static ssize_t tpm_sysfs_print_2_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	unsigned int p;
	/*unsigned long flags;*/

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read input */
	sscanf(buf, "%d", &p);

	/*raw_local_irq_save(flags);*/

	if (!strcmp(name, "api_section"))
		tpm_print_full_api_section(p);
	else if (!strcmp(name, "tcam_hw_hits"))
		tpm_tcam_hw_record(p);
	else if (!strcmp(name, "show_fc_config"))
		tpm_print_fc(p);
	else
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);

	/*raw_local_irq_restore(flags);*/

	return len;
}

static ssize_t tpm_sysfs_print_6_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	unsigned int p, v1 = 0, v2 = 0, v3 = 0, v4 = 0, v5 = 0;
	/*unsigned long flags;*/

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read input */
	sscanf(buf, "%d %d %d %d %x %x", &p, &v1, &v2, &v3, &v4, &v5);

	/*raw_local_irq_save(flags);*/

	if (!strcmp(name, "pnc_shadow_range"))
		tpm_print_pnc_shadow_range(p, v1, v2);
	else
		printk(KERN_ERR "%s: illegal operation <%s>\n", __func__, attr->attr.name);

	/*raw_local_irq_restore(flags);*/

	return len;
}

#ifdef CONFIG_MV_TPM_SYSFS_HELP
static ssize_t tpm_sysfs_mod2_help(char *buf)
{
	int off = 0;

	off += sprintf(buf + off,
		"cat help_mod2_print                                           - show this help\n");
	off += sprintf(buf + off,
		"cat show_jump_cfg                                             - show jump area configuration\n");
	off += sprintf(buf + off,
		"echo chain_type                    > show_chain_cfg           - show chain configuration\n");
	off += sprintf(buf + off,
		"     chain_type:                   "
		"[1-mh|2-mac|3-vlan|4-pppoe|5-l2_main|6-ipv4_napt|7-ipv4_mc|8-route]\n");
	off += sprintf(buf + off,
		"echo gmac_port                     > show_jump_pmt_entries    - show the pmt entries of gmac port\n");
	off += sprintf(buf + off, "     gmac_port :                   [0-2]\n");
	off += sprintf(buf + off,
		"echo gmac_port chain_type          > show_chain_pmt_entries   - show the pmt entries of gmac port\n");
	off += sprintf(buf + off, "     gmac_port :                   [0-2]\n");
	off += sprintf(buf + off,
		"     chain_type:                   "
		"[0-all|1-mh|2-mac|3-vlan|4-pppoe|5-l2_main|6-ipv4_napt|7-ipv4_mc|8-route]\n");
	off += sprintf(buf + off,
		"echo gmac_port chain_type chain_id > show_chain_db_entry      - show one chain data base entry\n");
	off += sprintf(buf + off, "     gmac_port :                   [0-2]\n");
	off += sprintf(buf + off,
		"     chain_type:                   "
		"[1-mh|2-mac|3-vlan|4-pppoe|5-l2_main|6-ipv4_napt|7-ipv4_mc|8-route]\n");
	off += sprintf(buf + off, "     chain_id  :                   chain index\n");
	off += sprintf(buf + off,
		"echo gmac_port chain_type          > show_chain_db            "
		"- show all the chain database entries of one chain type\n");
	off += sprintf(buf + off, "     gmac_port :                   [0-2]\n");
	off += sprintf(buf + off,
		"     chain_type:                   "
		"[1-mh|2-mac|3-vlan|4-pppoe|5-l2_main|6-ipv4_napt|7-ipv4_mc|8-route]\n");
	off += sprintf(buf + off,
		"echo gmac_port                     > show_all_chain           "
		"- show all the chain database entries\n");
	off += sprintf(buf + off, "     gmac_port :                   [0-2]\n");
	off += sprintf(buf + off,
		"echo api_type                      > show_section_free_size   "
		"- show specific API Section free size \n");
	off += sprintf(buf + off,
		"     api_type:                   "
		"  [0-mgmt|1-cpu_lpk|2-l2|3-l3|4-ipv4|5-ipv4_mc|6-ipv6_gen|7-ipv6_dip|8-ipv6_nh|9-ipv6_l4]\n");

	return off;
}

static ssize_t tpm_sysfs_mod2_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int off = 0;
	const char *name = attr->attr.name;

	if (!strcmp(name, "show_jump_cfg"))
		tpm_db_mod2_show_jump_cfg_data();
	else
		off = tpm_sysfs_mod2_help(buf);

	return off;
}
#else
static ssize_t tpm_sysfs_mod2_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __func__);
	return 0;
}
#endif /* CONFIG_MV_TPM_SYSFS_HELP */

static ssize_t tpm_sysfs_mod2_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	uint8_t arg_num;
	uint32_t v1, v2, v3;

	if (!strcmp(name, "show_chain_cfg")) {
		arg_num = sscanf(buf, "%d", &v1);
		if (arg_num == 1)
			tpm_db_mod2_show_chain_cfg_data(v1);
	} else if (!strcmp(name, "show_jump_pmt_entries")) {
		arg_num = sscanf(buf, "%d", &v1);
		if (arg_num == 1)
			tpm_db_mod2_show_jump_pmt_entries(v1);
	} else if (!strcmp(name, "show_chain_pmt_entries")) {
		arg_num = sscanf(buf, "%d %d", &v1, &v2);
		if (arg_num == 2)
			tpm_db_mod2_show_chain_pmt_entries(v1, v2);
	} else if (!strcmp(name, "show_chain_db_entry")) {
		arg_num = sscanf(buf, "%d %d %d", &v1, &v2, &v3);
		if (arg_num == 3)
			tpm_db_mod2_show_chain_info_entries(v1, v2, v3);
	} else if (!strcmp(name, "show_chain_db")) {
		arg_num = sscanf(buf, "%d %d", &v1, &v2);
		if (arg_num == 2)
			tpm_db_mod2_show_chain_info_entries(v1, v2, 0xffff);
	} else if (!strcmp(name, "show_all_chain")) {
		arg_num = sscanf(buf, "%d", &v1);
		if (arg_num == 1)
			tpm_db_mod2_show_all_chain(v1);
	} else if (!strcmp(name, "show_section_free_size")) {
		arg_num = sscanf(buf, "%d", &v1);
		if (arg_num == 1)
			tpm_print_section_free_szie(v1);
	}

	return len;
}

#ifdef CONFIG_MV_TPM_SYSFS_HELP
static DEVICE_ATTR(help, S_IRUSR, tpm_sysfs_print_show, NULL);
#endif
static DEVICE_ATTR(etherports, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(rx_modules, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(tx_modules, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(gmac_cfg, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(gmac_func, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(igmp_cfg, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(misc_cfg, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(vlan_etype, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(valid_api_section, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(valid_pnc_range, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(init_tables, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(mtu_cfg, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(api_dump, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(show_omci_channel, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(show_oam_channel, S_IRUSR, tpm_sysfs_print_show, NULL);


static DEVICE_ATTR(length_check_range, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(pnc_dump, S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(tcam_hw_hits,           S_IRUSR | S_IWUSR, tpm_sysfs_print_show, tpm_sysfs_print_2_store);
static DEVICE_ATTR(tcam_hw_hits_dump,      S_IRUSR | S_IWUSR, tpm_sysfs_print_show, tpm_sysfs_print_2_store);
static DEVICE_ATTR(age_pnc_dump,           S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(age_pnc_dump_live,      S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(age_pnc_dump_hdrs,      S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(age_dump,               S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(age_dump_all,           S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(mc_vlan_dump,           S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(busy_apis,              S_IRUSR, tpm_sysfs_print_show, NULL);
static DEVICE_ATTR(show_mac_learn_entry_count, S_IRUSR, tpm_sysfs_print_show, NULL);

static DEVICE_ATTR(api_section, S_IWUSR, tpm_sysfs_print_show, tpm_sysfs_print_2_store);
static DEVICE_ATTR(pnc_shadow_range, S_IWUSR, tpm_sysfs_print_show, tpm_sysfs_print_6_store);

#ifdef CONFIG_MV_TPM_SYSFS_HELP
static DEVICE_ATTR(help_mod2_print, S_IRUSR, tpm_sysfs_mod2_show, NULL);
#endif
static DEVICE_ATTR(show_jump_cfg, S_IRUSR, tpm_sysfs_mod2_show, NULL);
static DEVICE_ATTR(show_chain_cfg, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);
static DEVICE_ATTR(show_jump_pmt_entries, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);
static DEVICE_ATTR(show_chain_pmt_entries, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);
static DEVICE_ATTR(show_chain_db_entry, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);
static DEVICE_ATTR(show_chain_db, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);
static DEVICE_ATTR(show_all_chain, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);
static DEVICE_ATTR(show_section_free_size, S_IWUSR, tpm_sysfs_mod2_show, tpm_sysfs_mod2_store);

static DEVICE_ATTR(show_fc_config,          S_IRUSR | S_IWUSR, tpm_sysfs_print_show, tpm_sysfs_print_2_store);

static struct attribute *tpm_print_attrs[] = {
#ifdef CONFIG_MV_TPM_SYSFS_HELP
	&dev_attr_help.attr,
#endif
	&dev_attr_etherports.attr,
	&dev_attr_rx_modules.attr,
	&dev_attr_tx_modules.attr,
	&dev_attr_gmac_cfg.attr,
	&dev_attr_gmac_func.attr,
	&dev_attr_igmp_cfg.attr,
	&dev_attr_misc_cfg.attr,
	&dev_attr_vlan_etype.attr,
	&dev_attr_valid_api_section.attr,
	&dev_attr_valid_pnc_range.attr,
	&dev_attr_init_tables.attr,
	&dev_attr_api_section.attr,
	&dev_attr_pnc_shadow_range.attr,
	&dev_attr_mtu_cfg.attr,
	&dev_attr_length_check_range.attr,
	&dev_attr_api_dump.attr,
	&dev_attr_pnc_dump.attr,
	&dev_attr_tcam_hw_hits.attr,
	&dev_attr_tcam_hw_hits_dump.attr,
	&dev_attr_age_pnc_dump.attr,
	&dev_attr_age_pnc_dump_live.attr,
	&dev_attr_age_pnc_dump_hdrs.attr,
	&dev_attr_age_dump.attr,
	&dev_attr_age_dump_all.attr,
	&dev_attr_mc_vlan_dump.attr,
	&dev_attr_show_omci_channel.attr,
	&dev_attr_show_oam_channel.attr,
	&dev_attr_busy_apis.attr,
	&dev_attr_show_mac_learn_entry_count.attr,
	&dev_attr_show_fc_config.attr,

#ifdef CONFIG_MV_TPM_SYSFS_HELP
	&dev_attr_help_mod2_print.attr,
#endif
	&dev_attr_show_jump_cfg.attr,
	&dev_attr_show_chain_cfg.attr,
	&dev_attr_show_jump_pmt_entries.attr,
	&dev_attr_show_chain_pmt_entries.attr,
	&dev_attr_show_chain_db_entry.attr,
	&dev_attr_show_chain_db.attr,
	&dev_attr_show_all_chain.attr,
	&dev_attr_show_section_free_size.attr,

	NULL
};

static struct attribute_group tpm_print_group = {
	.name = "print",
	.attrs = tpm_print_attrs,
};

int tpm_sysfs_print_init(void)
{
	int err;
	struct device *pd;

	pd = bus_find_device_by_name(&platform_bus_type, NULL, "tpm");
	if (!pd) {
		platform_device_register_simple("tpm", -1, NULL, 0);
		pd = bus_find_device_by_name(&platform_bus_type, NULL, "tpm");
	}

	if (!pd) {
		printk(KERN_ERR "%s: cannot find tpm device\n", __func__);
		pd = &platform_bus;
	}

	err = sysfs_create_group(&pd->kobj, &tpm_print_group);
	if (err) {
		printk(KERN_ERR "sysfs group failed %d\n", err);
		goto out;
	}
out:
	return err;
}

/*module_init(xxx); */

MODULE_AUTHOR("Yuval Caduri");
MODULE_DESCRIPTION("sysfs for TPM Print");
MODULE_LICENSE("GPL");
