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
* mv_mac_learn_sysfsc.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include "tpm_common.h"
#include "tpm_header.h"

#include "mv_mac_learn_header.h"

/*******************************************************************************
* sys_mac_learn_parse_addr
*
* DESCRIPTION:
*           This function parses MAC address
* INPUTS:
*       buf - buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       bool
*******************************************************************************/
static bool sys_mac_learn_parse_addr (char *buf, uint32_t *macaddr_parts)
{
	if (6 == sscanf(buf, "%x:%x:%x:%x:%x:%x", &macaddr_parts[0], &macaddr_parts[1], &macaddr_parts[2], &macaddr_parts[3],
	    &macaddr_parts[4], &macaddr_parts[5])) {
		if (macaddr_parts[0] <= 255 && macaddr_parts[1] <= 255 && macaddr_parts[2] <= 255 && macaddr_parts[3] <= 255 &&
		    macaddr_parts[4] <= 255 && macaddr_parts[5] <= 255)
			return true;

	}
	return false;
}

/*******************************************************************************
* sys_mac_learn_count_para
*
* DESCRIPTION:
*           This function get the number of input parameters
* INPUTS:
*       buf - buffer with parameter string
*
* OUTPUTS:
        NONE.
*
* RETURNS:
*       int, number of input para
*******************************************************************************/
static int sys_mac_learn_count_para (const char *buf)
{
	char bufcopy[513];
	char **ap, *argv[20];
	char *inputstring = bufcopy;
	int numparms = 0;

	strcpy(bufcopy, buf);

	for (ap = argv; ap < &argv[15] && (*ap = strsep(&inputstring, " \t")) != NULL;) {
		if (**ap != '\0') {
			ap++;
			numparms++;
		}
	}
	return numparms;
}

/***********************************************************************************
**
**  sfs_help_mac_learn_cfg_all - help for mac learn sysfs commands under cfg folder
**
************************************************************************************/
int sfs_help_mac_learn_cfg_all(char *buf)
{
	int off = 0;

	off += sprintf(buf+off, "\necho [src_mac] > mac_learn_static_entry_add   - add one static MAC learn entry\n");
	off += sprintf(buf+off, "\necho [src_mac] > mac_learn_static_entry_del   - del one static MAC learn entry\n");
	off += sprintf(buf+off, "\tsrc_mac         (str)aa:bb:cc:dd:ee:ff  where each part is hexadecimal in range 0..FF\n");

	off += sprintf(buf+off, "\n");

	off += sprintf(buf+off, "\necho [enable] > mac_learn_enable   - enable/disable MAC learning\n");
	off += sprintf(buf+off, "\necho [enable] > mac_learn_overwrite_enable   - enable/disable non-static MAC entry overwrite when FDB full\n");
	off += sprintf(buf+off, "\tenable          (dec)0: disable, 1: enable\n");

	off += sprintf(buf+off, "\n");

	off += sprintf(buf+off, "\necho [max_count] > mac_learn_max_count_set   - Set max count of MAC learning \n");
	off += sprintf(buf+off, "\tmax_count       (dec)Should not greater than PNC range size\n");

	off += sprintf(buf+off, "\n");

	off += sprintf(buf+off, "\necho [expired_time] > mac_learn_expired_time_set   - Set expired time for non-static MAC learning entry\n");
	off += sprintf(buf+off, "\texpired_time    (dec)unit: second\n");

	off += sprintf(buf+off, "\n");

	off += sprintf(buf+off, "\necho [trace_level] > mac_learn_trace_level   - Set trace level for print info\n");
	off += sprintf(buf+off, "\ttrace_level     (dec)0: no trace, 1: debug level, 2: warn level\n");

	off += sprintf(buf+off, "\n");

	return(off);
}

/*******************************************************************************
* sfs_mac_learn_static_entry_add
*
* DESCRIPTION:
*           This function adds static MAC address as MAC learn entry
*
*
*******************************************************************************/
void sfs_mac_learn_static_entry_add(const char *buf, size_t len)
{
	typedef enum {
		mac_learn_static_mac, mac_learn_static_max
	} l2_mac_rule_parm_indx_t;

	uint32_t sa[6];
	char temp_sa[30];
	int numparms;
	char src_mac[6];
	int parsedargs;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_learn_static_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%s", temp_sa);

		if (sys_mac_learn_parse_addr(temp_sa, sa) == false) {
			MVMACLEARN_ERR_PRINT("Invalid SA [%s]\n", temp_sa);
		} else {
			src_mac[0] = (char)sa[0];
			src_mac[1] = (char)sa[1];
			src_mac[2] = (char)sa[2];
			src_mac[3] = (char)sa[3];
			src_mac[4] = (char)sa[4];
			src_mac[5] = (char)sa[5];
			if (mv_mac_learn_static_entry_add(src_mac))
				MVMACLEARN_ERR_PRINT("Faild to add static MAC entry");
			else
				printk("OK.\n");
		}
	}
}

/*******************************************************************************
* sfs_mac_learn_static_entry_del
*
* DESCRIPTION:
*           This function deletes static MAC address as MAC learn entry
*
*
*******************************************************************************/
void sfs_mac_learn_static_entry_del(const char *buf, size_t len)
{
	typedef enum {
		mac_learn_static_mac, mac_learn_static_max
	} l2_mac_rule_parm_indx_t;

	uint32_t sa[6];
	char temp_sa[30];
	int numparms;
	char src_mac[6];
	int parsedargs;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_learn_static_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%s", temp_sa);

		if (sys_mac_learn_parse_addr(temp_sa, sa) == false) {
			MVMACLEARN_ERR_PRINT("Invalid SA [%s]\n", temp_sa);
		} else {
			src_mac[0] = (char)sa[0];
			src_mac[1] = (char)sa[1];
			src_mac[2] = (char)sa[2];
			src_mac[3] = (char)sa[3];
			src_mac[4] = (char)sa[4];
			src_mac[5] = (char)sa[5];
			if (mv_mac_learn_static_entry_del(src_mac))
				MVMACLEARN_ERR_PRINT("Faild to del static MAC entry");
			else
				printk("OK.\n");
		}
	}
}

/*******************************************************************************
* sfs_mac_learn_enable_cfg
*
* DESCRIPTION:
*           This function enable or disable MAC learning
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_mac_learn_enable_cfg(const char *buf, size_t len)
{
	typedef enum
	{
		mac_leanr_enable_bool, mac_leanr_enable_max
	} mac_learn_enable_t;

	uint32_t mac_learn_enable;
	int parsedargs;
	int numparms;

	bool enable_para;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_leanr_enable_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &mac_learn_enable);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else {
			if (mac_learn_enable)
				enable_para = true;
			else
				enable_para = false;
			if (mv_mac_learn_enable_set(enable_para))
				MVMACLEARN_ERR_PRINT("Faild to enable/diable MAC learning\n");
		}
	}
}

/*******************************************************************************
* sfs_mac_learn_overwrite_enable_cfg
*
* DESCRIPTION:
*           This function enable or disable MAC learning overwrite when FDB full
* INPUTS:
*       buf	- Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_mac_learn_overwrite_enable_cfg(const char *buf, size_t len)
{
	typedef enum
	{
		mac_learn_overwrite_bool, mac_learn_overwrite_max
	} mac_learn_overwrite_t;

	uint32_t mac_learn_overwrite;
	int parsedargs;
	int numparms;

	bool enable_para;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_learn_overwrite_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &mac_learn_overwrite);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else {
			if (mac_learn_overwrite)
				enable_para = true;
			else
				enable_para = false;
			if (mv_mac_learn_overwrite_enable_set(enable_para))
				MVMACLEARN_ERR_PRINT("Faild to enable/diable MAC learning overwrite function\n");
		}
	}
}

/*******************************************************************************
* sfs_mac_learn_max_count_cfg
*
* DESCRIPTION:
*           This function set the max count of mac leanr entry
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_mac_learn_max_count_cfg(const char *buf, size_t len)
{
	typedef enum
	{
		mac_learn_max_count, mac_learn_max_count_max
	} mac_learn_max_count_t;

	uint32_t max_count;
	int parsedargs;
	int numparms;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_learn_max_count_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &max_count);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else {
			if (mv_mac_learn_max_count_set(max_count))
				MVMACLEARN_ERR_PRINT("Faild to cfg MAC learning max count\n");
		}
	}
}

/*******************************************************************************
* sfs_mac_learn_expire_time_cfg
*
* DESCRIPTION:
*           This function set the expired time of non-static mac leanr entry
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_mac_learn_expired_time_cfg(const char *buf, size_t len)
{
	typedef enum
	{
		mac_learn_expire_time, mac_learn_expire_time_max
	} mac_learn_expire_time_t;

	uint32_t expire_time;
	int parsedargs;
	int numparms;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_learn_expire_time_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &expire_time);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else {
			if (mv_mac_learn_expire_time_set(expire_time))
				MVMACLEARN_ERR_PRINT("Faild to cfg MAC learning expired time\n");
		}
	}
}

/*******************************************************************************
* sfs_mac_learn_trace_level_cfg
*
* DESCRIPTION:
*           This function set the trace level of print info
* INPUTS:
*       buf - Shell parameters as char buffer
*       len - Number of characters in buffer
*
*******************************************************************************/
void sfs_mac_learn_trace_level_cfg(const char *buf, size_t len)
{
	typedef enum
	{
		mac_learn_trace_level, mac_learn_trace_level_max
	} mac_learn_trace_level_t;

	uint32_t trace_level;
	int parsedargs;
	int numparms;

	numparms = sys_mac_learn_count_para(buf);
	if (numparms != mac_learn_trace_level_max) {
		MVMACLEARN_ERR_PRINT("Parameter number not right");
	} else {
		/* Get parameters */
		parsedargs = sscanf(buf, "%d", &trace_level);

		if (parsedargs != numparms) {
			printk(KERN_INFO "Parse failure - %d/%d parameters were parsed\n", parsedargs, numparms);
		} else {
			if (mv_mac_learn_op_trace_level_set(trace_level))
				MVMACLEARN_ERR_PRINT("Faild to cfg MAC learning expired time\n");
		}
	}
}

/***********************************************************************************
**
**  sfs_help_mac_learn_show_all - help for mac learn sysfs commands under show folder
**
************************************************************************************/
int sfs_help_mac_learn_show_all(char *buf)
{
	int off = 0;

	off += sprintf(buf+off, "\ncat show_mac_learn_enable             - show mac learn enable state\n");
	off += sprintf(buf+off, "\ncat show_mac_learn_overwrite_enable   - show mac learn non-static entry overwrite enable state when FDB full\n");
	off += sprintf(buf+off, "\ncat show_mac_learn_max_count          - show mac learn max entry count\n");
	off += sprintf(buf+off, "\ncat show_mac_learn_expire_time        - show mac learn non-static entry expired time\n");
	off += sprintf(buf+off, "\ncat show_mac_learn_fdb_entry          - show mac learn entries in FDB\n");

	off += sprintf(buf+off, "\n");

	return(off);
}


/*******************************************************************************
* sfs_mac_learn_enable_get
*
* DESCRIPTION:
*           This function show mac learn enable state
*******************************************************************************/
int sfs_mac_learn_enable_get(char *buf)
{
	int off = 0;
	char result[8];
	bool mac_learn_enable;

	if (mv_mac_learn_enable_get(&mac_learn_enable)) {
		memcpy(result, "failed", 8);
	} else {
		if (true == mac_learn_enable)
			memcpy(result, "enable", 8);
		else
			memcpy(result, "disable", 8);
	}
	off += sprintf(buf+off, "\nmac learn enable get: %s\n", result);

	return off;
}

/*******************************************************************************
* sfs_mac_learn_overwrite_enable_get
*
* DESCRIPTION:
*           This function show mac learn overwrite enable state when FDB full
*******************************************************************************/
int sfs_mac_learn_overwrite_enable_get(char *buf)
{
	int off = 0;
	char result[8];
	bool mac_learn_enable;

	if (mv_mac_learn_overwrite_enable_get(&mac_learn_enable)) {
		memcpy(result, "failed", 8);
	} else {
		if (true == mac_learn_enable)
			memcpy(result, "enable", 8);
		else
			memcpy(result, "disable", 8);
	}
	off += sprintf(buf+off, "\nmac learn overwrite enable get: %s\n", result);

	return off;
}

/*******************************************************************************
* sfs_mac_learn_max_count_get
*
* DESCRIPTION:
*           This function show mac learn MAX entry count configured
*******************************************************************************/
int sfs_mac_learn_max_count_get(char *buf)
{
	int off = 0;
	char result[8];
	uint32_t max_count;

	if (mv_mac_learn_max_count_get(&max_count)) {
		memcpy(result, "failed", 8);
		off += sprintf(buf+off, "\nmac learn max count get: %s\n", result);
	} else {
		off += sprintf(buf+off, "\nmac learn max count get: %d\n", max_count);
	}

	return off;
}

/**************************************************************************************
* sfs_mac_learn_expired_time_get
*
* DESCRIPTION:
*           This function show mac learn expired time configured for non-static entry
***************************************************************************************/
int sfs_mac_learn_expired_time_get(char *buf)
{
	int off = 0;
	char result[8];
	uint32_t expire_time;

	if (mv_mac_learn_expire_time_get(&expire_time)) {
		memcpy(result, "failed", 8);
		off += sprintf(buf+off, "\nmac learn expire time get: %s\n", result);
	} else {
		off += sprintf(buf+off, "\nmac learn expire time get: %d(s)\n", expire_time);
	}

	return off;
}

/**************************************************************************************
* sfs_mac_learn_entry_print
*
* DESCRIPTION:
*           This function print the MAC learning entries learned
***************************************************************************************/
int sfs_mac_learn_entry_print(char *buf)
{
	int off = 0;

	mac_learn_db_valid_print();

	return off;
}



static mac_learn_store_sysfs_func_t mac_learn_sysfs_func_ara[] =
{
	{"mac_learn_static_entry_add", sfs_mac_learn_static_entry_add},
	{"mac_learn_static_entry_del", sfs_mac_learn_static_entry_del},
	{"mac_learn_enable",           sfs_mac_learn_enable_cfg},
	{"mac_learn_overwrite_enable", sfs_mac_learn_overwrite_enable_cfg},
	{"mac_learn_max_count_set",    sfs_mac_learn_max_count_cfg},
	{"mac_learn_expired_time_set", sfs_mac_learn_expired_time_cfg},
	{"mac_learn_trace_level",      sfs_mac_learn_trace_level_cfg},
};

static int mac_learn_num_store = sizeof(mac_learn_sysfs_func_ara)/sizeof(mac_learn_sysfs_func_ara[0]);

static ssize_t mac_learn_cfg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t len)
{
	const char *name = attr->attr.name;
	mac_learn_store_sysfs_func_t *pnamefunc;
	int indx;
	bool found = false;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	for (indx = 0; indx < mac_learn_num_store; indx++){
		pnamefunc = &mac_learn_sysfs_func_ara[indx];
		if (!strcmp(name, pnamefunc->sysfs_name)) {
			found = true;
			(pnamefunc->sysfs_func)(buf, len);
			break;
		}
	}

	if (found == false){
		printk("%s: operation <%s> not found\n", __FUNCTION__, attr->attr.name);
	}

	return(len);
}

static mac_learn_show_sysfs_func_t mac_learn_show_sysfs_func_ara[] =
{
	{"help_mac_learn_cfg", sfs_help_mac_learn_cfg_all},
	{"help_mac_learn_show", sfs_help_mac_learn_show_all},
	{"show_mac_learn_enable", sfs_mac_learn_enable_get},
	{"show_mac_learn_overwrite_enable", sfs_mac_learn_overwrite_enable_get},
	{"show_mac_learn_max_count", sfs_mac_learn_max_count_get},
	{"show_mac_learn_expire_time", sfs_mac_learn_expired_time_get},
	{"show_mac_learn_fdb_entry", sfs_mac_learn_entry_print},

};

static int mac_learn_num_show = sizeof(mac_learn_show_sysfs_func_ara)/sizeof(mac_learn_show_sysfs_func_ara[0]);


static ssize_t mac_learn_cfg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	const char *name = attr->attr.name;
	mac_learn_show_sysfs_func_t *pnamefunc;
	int indx;
	bool found = false;
	int buflen = 0;

	if (!capable(CAP_NET_ADMIN))
		return -EPERM;

	for (indx = 0; indx < mac_learn_num_show; indx++){
		pnamefunc = &mac_learn_show_sysfs_func_ara[indx];
		if (!strcmp(name, pnamefunc->sysfs_name)) {
			found = true;
			buflen = (pnamefunc->sysfs_func)(buf);
			break;
		}
	}
	if (found == false){
		printk("%s: operation <%s> not found\n", __FUNCTION__, attr->attr.name);
	}

	return buflen;
}

/*store attr*/
static DEVICE_ATTR(mac_learn_static_entry_add, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(mac_learn_static_entry_del, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(mac_learn_enable, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(mac_learn_overwrite_enable, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(mac_learn_max_count_set, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(mac_learn_expired_time_set, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(mac_learn_trace_level, S_IWUSR, mac_learn_cfg_show, mac_learn_cfg_store);

/*show attr*/
static DEVICE_ATTR(help_mac_learn_cfg, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(help_mac_learn_show, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(show_mac_learn_enable, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(show_mac_learn_overwrite_enable, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(show_mac_learn_max_count, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(show_mac_learn_expire_time, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);
static DEVICE_ATTR(show_mac_learn_fdb_entry, S_IRUSR, mac_learn_cfg_show, mac_learn_cfg_store);



static struct attribute *mac_learn_cfg_attrs[] =
{
	&dev_attr_help_mac_learn_cfg.attr,
	&dev_attr_mac_learn_static_entry_add.attr,
	&dev_attr_mac_learn_static_entry_del.attr,
	&dev_attr_mac_learn_enable.attr,
	&dev_attr_mac_learn_overwrite_enable.attr,
	&dev_attr_mac_learn_max_count_set.attr,
	&dev_attr_mac_learn_expired_time_set.attr,
	&dev_attr_mac_learn_trace_level.attr,

	NULL
};

static struct attribute *mac_learn_show_attrs[] =
{
	&dev_attr_help_mac_learn_show.attr,
	&dev_attr_show_mac_learn_enable.attr,
	&dev_attr_show_mac_learn_overwrite_enable.attr,
	&dev_attr_show_mac_learn_max_count.attr,
	&dev_attr_show_mac_learn_expire_time.attr,
	&dev_attr_show_mac_learn_fdb_entry.attr,
	NULL
};


static struct attribute_group mac_learn_cfg_group =
{
	.name = "mac_learn_cfg",
	.attrs = mac_learn_cfg_attrs
};

static struct attribute_group mac_learn_show_group =
{
	.name = "mac_learn_show",
	.attrs = mac_learn_show_attrs
};

static mv_attr_group_pair_t mac_learn_attr_group_pair[] =
{
	{"mac_learn_cfg",          &mac_learn_cfg_group},
	{"mac_learn_show",         &mac_learn_show_group},
};
static int num_attr_group_pairs = sizeof(mac_learn_attr_group_pair)/sizeof(mac_learn_attr_group_pair[0]);

/*******************************************************************************
* mv_mac_learn_sysfs_init()
*
* DESCRIPTION:    Finish all initialization for sysfs.
*
* INPUTS:
* None
*
* OUTPUTS:
* None
*
* RETURNS:
* int, success--0; failed--non-zero
*******************************************************************************/
int mv_mac_learn_sysfs_init(void)
{
	int err;
	struct device *pd;
	int indx;

	pd = bus_find_device_by_name(&platform_bus_type, NULL, "mc_mac_learn");
	if (!pd) {
		platform_device_register_simple("mc_mac_learn", -1, NULL, 0);
		pd = bus_find_device_by_name(&platform_bus_type, NULL, "mc_mac_learn");
	}

	if (!pd) {
		printk(KERN_ERR"%s: cannot find mc_mac_learn device\n", __FUNCTION__);
		pd = &platform_bus;
	}

	for (indx = 0; indx < num_attr_group_pairs; indx++) {
		mv_attr_group_pair_t *ppair;

		ppair = &mac_learn_attr_group_pair[indx];

		err = sysfs_create_group(&pd->kobj, ppair->pattrgroup);
		if (err) {
			printk(KERN_INFO "sysfs_create_group failed for %s, err = %d\n", ppair->folder_name, err);
			goto out;
		}
	}
	printk(KERN_INFO "= MC MAC LEARN Module SYS FS Init ended successfully =\n");

out:
	return err;
}

/*******************************************************************************
* mv_mac_learn_sysfs_exit()
*
* DESCRIPTION:    unregister sysfs if exit
*
* INPUTS:
* None
*
* OUTPUTS:
* None
*
* RETURNS:
* None
*******************************************************************************/
void mv_mac_learn_sysfs_exit(void)
{
	struct device *pd;
	int indx;

	pd = bus_find_device_by_name(&platform_bus_type, NULL, "mc_mac_learn");
	if (!pd) {
		printk(KERN_ERR"%s: cannot find mc_mac_learn device\n", __FUNCTION__);
		return;
	}

	for (indx = 0; indx < num_attr_group_pairs; indx++) {
		mv_attr_group_pair_t *ppair;
		ppair = &mac_learn_attr_group_pair[indx];
		sysfs_remove_group(&pd->kobj, ppair->pattrgroup);
	}

	printk(KERN_INFO "= MC MAC LEARN Module SYS FS Remove ended successfully =\n");
}
