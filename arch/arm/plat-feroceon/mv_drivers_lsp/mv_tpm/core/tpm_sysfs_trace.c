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

static ssize_t tpm_sysfs_trace_help(char *buf)
{
	int off = 0;
#ifdef CONFIG_MV_TPM_SYSFS_HELP
	off += sprintf(buf + off, "cat help                   - show this help\n");
	off += sprintf(buf + off, "cat trace                  - show TPM Trace configuration\n");
	off += sprintf(buf + off,
		"echo level  > trace_level  - "
		"Set TPM Global Trace level (0-none, 1-debug, 2-info, 3-warn, 4-err, 5-fatal)\n");
	off += sprintf(buf + off,
		"echo mod en > trace_module - Trace en/dis TPM module"
		"(0-db, 1-pnc_logic, 2-init, 5-proc_logic, 6-cli, 7-pnc_hw, 9-mod)\n");
#else
	printk(KERN_WARNING "%s: sysfs help is not compiled (CONFIG_MV_TPM_SYSFS_HELP)\n", __func__);
#endif
	return off;
}

static ssize_t tpm_sysfs_trace_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int off = 0;
	const char *name = attr->attr.name;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	if (!strcmp(name, "trace"))
		off = tpm_trace_status_print();
	else
		off = tpm_sysfs_trace_help(buf);

	return off;
}

static ssize_t tpm_sysfs_trace_1_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	unsigned int v;
	/*unsigned long flags;*/

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read input */
	v = 0;

	sscanf(buf, "%x", &v);

	/*raw_local_irq_save(flags);*/

	if (!strcmp(name, "trace_level"))
		tpm_trace_set(v);
	else
		printk(KERN_INFO "%s: illegal operation <%s>\n", __func__, attr->attr.name);

	/*raw_local_irq_restore(flags);*/

	return len;
}

static ssize_t tpm_sysfs_trace_2_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	unsigned int p, v;
	/*unsigned long flags;*/

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	/* Read input */
	v = 0;
	sscanf(buf, "%d %x", &p, &v);

	/*raw_local_irq_save(flags);*/

	if (!strcmp(name, "trace_module"))
		tpm_trace_module_set(p, v);
	else
		printk(KERN_INFO "%s: illegal operation <%s>\n", __func__, attr->attr.name);

	/*raw_local_irq_restore(flags);*/

	return len;
}

static DEVICE_ATTR(trace_level, S_IWUSR, tpm_sysfs_trace_show, tpm_sysfs_trace_1_store);
static DEVICE_ATTR(trace_module, S_IWUSR, tpm_sysfs_trace_show, tpm_sysfs_trace_2_store);
static DEVICE_ATTR(trace, S_IRUSR, tpm_sysfs_trace_show, NULL);
#ifdef CONFIG_MV_TPM_SYSFS_HELP
static DEVICE_ATTR(help, S_IRUSR, tpm_sysfs_trace_show, NULL);
#endif
static struct attribute *tpm_trace_attrs[] = {
	&dev_attr_trace_level.attr,
	&dev_attr_trace_module.attr,
	&dev_attr_trace.attr,
#ifdef CONFIG_MV_TPM_SYSFS_HELP
	&dev_attr_help.attr,
#endif
	NULL
};

static struct attribute_group tpm_trace_group = {
	.name = "trace",
	.attrs = tpm_trace_attrs,
};

int tpm_sysfs_trace_init(void)
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

	err = sysfs_create_group(&pd->kobj, &tpm_trace_group);
	if (err) {
		printk(KERN_INFO "sysfs group failed %d\n", err);
		goto out;
	}
out:
	return err;
}

/*module_init(xxx); */

MODULE_AUTHOR("Yuval Caduri");
MODULE_DESCRIPTION("sysfs for TPM Trace");
MODULE_LICENSE("GPL");
