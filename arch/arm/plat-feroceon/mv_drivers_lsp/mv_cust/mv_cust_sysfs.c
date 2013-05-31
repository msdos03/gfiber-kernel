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
* mv_cust_sysfs.c
*
* DESCRIPTION:
*
*
*******************************************************************************/
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/netdevice.h>

#include <gbe/mvNeta.h>
#include <pnc/mvPnc.h>

#include "mv_cust_dev.h"
#include "mv_cust_netdev.h"
#include "mv_cust_flow_map.h"
#include "mv_cust_mng_if.h"

static int      eoam_txq = 0;
static uint8_t  eoam_mac[6] = {0};


static ssize_t mv_cust_spec_proc_help(char *buf)
{
    int off = 0;
    off += sprintf(buf+off, "cat  help                              - show this help\n");
#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE    
    off += sprintf(buf+off, "cat  udp_ports                         - show special udp source and dest. port configuration\n");
#endif
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
    off += sprintf(buf+off, "cat  flow_map                          - show flow mapping configuration\n");        
    off += sprintf(buf+off, "cat  flow_map_show                     - show flow mapping rules \n");    
    off += sprintf(buf+off, "cat  flow_map_clear                    - clear all flow mapping rules\n");
    off += sprintf(buf+off, "cat  dscp_map_del                      - delete DSCP to P-bits mapping rules\n");    
#endif
    off += sprintf(buf+off, "cat  tcont_show                        - Show all T-CONT state\n");
    off += sprintf(buf+off, "echo tcont state        > tcont_set    - Set T-CONT tatus, tcont(0~7), state(1:enable, 0:disable) \n");
    off += sprintf(buf+off, "echo hex                > debug        - Set Customized module debug information \n");
    off += sprintf(buf+off, "echo app_type           > app_show     - Show application configuration. 0:igmp, 1:mld, 2:lpbk, 3:eoam, 4:omci\n");
    off += sprintf(buf+off, "echo app_type enable    > app_flag_set - Enable/disable application. 0:igmp, 1:mld, 2:lpbk, 3:eoam, 4:omci, 1:enable, 0:disable\n");
    off += sprintf(buf+off, "echo app_type hex       > app_etype_set- Set application Eth type. 0:igmp, 1:mld, 2:lpbk, 3:eoam, 4:omci\n");
    off += sprintf(buf+off, "echo tc txq gem keep_mh > omci_set     - set udp source port special Tx behavior\n");
    off += sprintf(buf+off, "echo keep_mh            > eoam_gh_keep - Keep or not keep GH for eOAM packets, 1:keep, 0:do not keep\n");    
    off += sprintf(buf+off, "echo txq                > eoam_txq     - set TX queue into sw_buffer for EOAM llid command\n");
    off += sprintf(buf+off, "echo mac[0]-mac[5]      > eoam_mac     - set LLID mac into sw_buffer for EOAM llid command\n");
    off += sprintf(buf+off, "echo llid               > eoam_write   - Write EOAM txq and mac address from sw_buffer into LLID\n");
#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE    
    off += sprintf(buf+off, "echo p udp_src(dec) txp txq flags hw_cmd  > udp_src - set udp source port special Tx behavior\n");
    off += sprintf(buf+off, "echo p udp_dst(dec) txp txq flags hw_cmd  > udp_dst - set udp dest.  port special Tx behavior\n");
#endif    
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE
    off += sprintf(buf+off, "echo hex                                  > flow_map_debug  - Set flow mapping debug flag, 1:enable, 0:disable \n");    
    off += sprintf(buf+off, "echo vid pbits mod_vid mod_pbits trg_port trg_queue trg_hwf_queue gem_port  > flow_map_us_set - set U/S flow mapping rule\n");
    off += sprintf(buf+off, "echo vid pbits mod_vid mod_pbits          > flow_map_ds_set - set D/S flow mapping rule\n");    
    off += sprintf(buf+off, "echo pbits0 pbits1 ... pbits62 pbits63    > dscp_map_set    - set DSCP to P-bits mapping rules\n");    
    off += sprintf(buf+off, "echo vlan pbits dir(0:U/S, 1:D/S)         > flow_map_del    - delete flow mapping rule\n");
    off += sprintf(buf+off, "echo vlan pbits dir(0:U/S, 1:D/S)         > tag_flow_get    - get tagged flow mapping rule\n");
    off += sprintf(buf+off, "echo dscp dir(0:U/S, 1:D/S)               > untag_flow_get  - get untagged flow mapping rule\n");
#endif        

    return off;
}


static ssize_t mv_cust_spec_proc_show(struct device *dev,
                                      struct device_attribute *attr, char *buf)
{
    int off   = 0;
    int index = 0;
    bool state;
    const char* name = attr->attr.name;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    if (!strcmp(name, "help") ) {
        off = mv_cust_spec_proc_help(buf);
    }
#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE       
    else if (!strcmp(name, "udp_ports")) {
        mv_cust_udp_spec_print_all();
    }
#endif    
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE       
    else if (!strcmp(name, "flow_map")) { 
        mv_cust_flow_map_print();
    }     
    else if (!strcmp(name, "flow_map_show")) {
        mv_cust_map_table_print();
    } 
    else if (!strcmp(name, "flow_map_clear")) {
        mv_cust_map_rule_clear();
    }  
    else if (!strcmp(name, "dscp_map_del")) {
        mv_cust_dscp_map_del();
    }     
#endif
    else if (!strcmp(name, "tcont_show")) {
        printk(KERN_INFO "MV_CUST T-CONT state table\n");
        printk(KERN_INFO "Default invalid Txq:%d \n", CPH_INVALID_TRGT_QUEUE);
        printk(KERN_INFO "--------------------------\n");
        for (index = 0; index < CPH_MAX_TCONT_NUM; index++)
        {
            state = mv_cust_get_tcont_state(index);
            printk(KERN_INFO "T-CONT%1.1d: %s\n", index, (state==true)?"enabled":"disabled");
        }
    } 
    else
        off = mv_cust_spec_proc_help(buf);

    return off;
}


static ssize_t mv_cust_spec_proc_1_store(struct device *dev,
                                         struct device_attribute *attr,
                                         const char *buf, size_t len)
{
    const char*     name = attr->attr.name;
    unsigned int    v;
    unsigned long   flags;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    /* Read input */
    v = 0;

    sscanf(buf, "%x", &v);
    
    raw_local_irq_save(flags);

    if (!strcmp(name, "debug")) {
        mv_cust_debug_info_set(v);
    }
    else if (!strcmp(name, "app_show")) {
        mv_cust_print(v);
    }
    else if (!strcmp(name, "eoam_gh_keep")) {
        mv_cust_oam_rx_gh_set(v);
    }   
    else if (!strcmp(name, "eoam_txq")) {
        eoam_txq = v;
    }
    else if (!strcmp(name, "eoam_write")) {
        mv_cust_eoam_llid_set((int)v, &eoam_mac[0], eoam_txq);
    }
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

    raw_local_irq_restore(flags);

    return len;
}


static ssize_t mv_cust_spec_proc_2_store(struct device *dev,
                                         struct device_attribute *attr,
                                         const char *buf, size_t len)
{
    const char*     name = attr->attr.name;
    unsigned int    p, v;
    unsigned long   flags;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    /* Read input */
    p = 0;
    v = 0;
    sscanf(buf, "%d %x", &p, &v);

    raw_local_irq_save(flags);

    if (!strcmp(name, "app_flag_set")) {
        mv_cust_app_flag_set(p, v);
    }
    else if (!strcmp(name, "app_etype_set")) {
        mv_cust_app_etype_set(p, v);
    }
    else if (!strcmp(name, "tcont_set")) {
        if (v)
            mv_cust_set_tcont_state(p, true);
        else
            mv_cust_set_tcont_state(p, false);
    }    
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

    raw_local_irq_restore(flags);

    return len;
}


static ssize_t mv_cust_spec_proc_6_store(struct device *dev,
                                         struct device_attribute *attr,
                                         const char *buf, size_t len)
{
    const char*     name = attr->attr.name;
    unsigned int    p, v1=0, v2=0, v3=0, v4=0, v5=0;
    unsigned long   flags;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    /* Read input */
    sscanf(buf, "%d %d %x %x %x %x", &p, &v1, &v2, &v3, &v4, &v5);

    raw_local_irq_save(flags);

    if (!strcmp(name, "udp_src")) {
#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
        mv_cust_udp_src_spec_set(p, v1, v2, v3, v4, v5);
#else
        printk("mv_cust module was not compiled with UDP SAMPLE Config Support\n");
#endif
    }
    else if (!strcmp(name, "udp_dst")) {
#ifdef CONFIG_MV_CUST_UDP_SAMPLE_HANDLE
        mv_cust_udp_dest_spec_set(p, v1, v2, v3, v4, v5);
#else
        printk("mv_cust module was not compiled with UDP SAMPLE Config Support\n");
#endif
    }
    else if (!strcmp(name, "omci_set")) {
        mv_cust_omci_set(p, v1, v2, v3);
    }
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

    raw_local_irq_restore(flags);

    return len;
}

static ssize_t mv_cust_spec_proc_flow_store(struct device *dev,
                                            struct device_attribute *attr,
                                            const char *buf, size_t len)
{
    const char*        name = attr->attr.name;
    unsigned int       v1=0, v2=0, v3=0, v4=0, v5=0, v6=0, v7=0, v8=0;
    unsigned long      flags;
    mv_cust_ioctl_flow_map_t cust_flow;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    /* Read input */
    sscanf(buf, "%d %d %d %d %d %d %d %d", &v1, &v2, &v3, &v4, &v5, &v6, &v7, &v8);

    raw_local_irq_save(flags);

#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE   

    if (!strcmp(name, "flow_map_debug")) {
        mv_cust_set_trace_flag((uint32_t)v1);
    }
    else if (!strcmp(name, "flow_map_us_set")) {
        memset(&cust_flow, 0, sizeof(cust_flow));
        cust_flow.dir       = MV_CUST_FLOW_DIR_US;
        cust_flow.vid       = v1;
        cust_flow.pbits     = v2;
        cust_flow.mod_vid   = v3;
        cust_flow.mod_pbits = v4;
        
        cust_flow.pkt_frwd.in_use        = 1;
        cust_flow.pkt_frwd.trg_port      = (mv_cust_trg_port_type_t)v5;
        cust_flow.pkt_frwd.trg_queue     = (uint32_t)v6;
        cust_flow.pkt_frwd.trg_hwf_queue = (uint32_t)v7;        
        cust_flow.pkt_frwd.gem_port      = (mv_cust_gem_port_key_t)v8;
        
        mv_cust_map_rule_set(&cust_flow);
    }
    else if (!strcmp(name, "flow_map_ds_set")) {
        memset(&cust_flow, 0, sizeof(cust_flow));
        cust_flow.dir       = MV_CUST_FLOW_DIR_DS;
        cust_flow.vid       = v1;
        cust_flow.pbits     = v2;
        cust_flow.mod_vid   = v3;
        cust_flow.mod_pbits = v4;
        cust_flow.pkt_frwd.in_use    = 1;
        
        mv_cust_map_rule_set(&cust_flow);
    }    
    else if (!strcmp(name, "flow_map_del")) {
        mv_cust_map_rule_del((uint16_t)v1, (uint8_t)v2, (mv_cust_flow_dir_e)v3);
    }
    else if (!strcmp(name, "tag_flow_get")) {
        memset(&cust_flow, 0, sizeof(cust_flow));
        cust_flow.vid   = v1;
        cust_flow.pbits = v2;   
        cust_flow.dir   = (mv_cust_flow_dir_e)v3;        
        mv_cust_tag_map_rule_get(&cust_flow);
        printk("in_use[%d], mod_vid[%d] mod_pbits[%d] trg_port[%d], trg_queue[%d], gem_port[%d]\r\n",
                cust_flow.pkt_frwd.in_use, cust_flow.mod_vid, 
                cust_flow.mod_pbits, cust_flow.pkt_frwd.trg_port, 
                cust_flow.pkt_frwd.trg_queue, cust_flow.pkt_frwd.gem_port);
    }    
    else if (!strcmp(name, "untag_flow_get")) {
        memset(&cust_flow, 0, sizeof(cust_flow));
        cust_flow.dscp  =  v1;
        cust_flow.dir   = (mv_cust_flow_dir_e)v2;        
        mv_cust_untag_map_rule_get(&cust_flow);    
        printk("in_use[%d], mod_vid[%d] mod_pbits[%d] trg_port[%d], trg_queue[%d], gem_port[%d]\r\n", 
                cust_flow.pkt_frwd.in_use, cust_flow.mod_vid, 
                cust_flow.mod_pbits, cust_flow.pkt_frwd.trg_port, 
                cust_flow.pkt_frwd.trg_queue, cust_flow.pkt_frwd.gem_port);        
    }      
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);
#endif

    raw_local_irq_restore(flags);

    return len;
}

static ssize_t mv_cust_spec_proc_6h_store(struct device *dev,
                                          struct device_attribute *attr,
                                          const char *buf, size_t len)
{
    const char*     name = attr->attr.name;
    unsigned int    v1=0, v2=0, v3=0, v4=0, v5=0, v6=0;
    unsigned long   flags;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    /* Read input */
    sscanf(buf, "%x %x %x %x %x %x", &v1, &v2, &v3, &v4, &v5, &v6);

    raw_local_irq_save(flags);

    if (!strcmp(name, "eoam_mac")) {
        eoam_mac[0] = (uint8_t)v1;
        eoam_mac[1] = (uint8_t)v2;
        eoam_mac[2] = (uint8_t)v3;
        eoam_mac[3] = (uint8_t)v4;
        eoam_mac[4] = (uint8_t)v5;
        eoam_mac[5] = (uint8_t)v6;
    }
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

    raw_local_irq_restore(flags);

    return len;
}

static ssize_t mv_cust_spec_proc_64_store(struct device *dev,
                                          struct device_attribute *attr,
                                          const char *buf, size_t len)
{
    const char*          name = attr->attr.name;
    unsigned int         v[64];
    unsigned int         index = 0;
    unsigned long        flags;
    mv_cust_dscp_pbits_t dscp_map;

    if (!capable(CAP_NET_ADMIN))
        return -EPERM;

    /* Read input */
    for (index=0; index<64; index++) {
        sscanf(buf, "%d", &v[index]);
        dscp_map.pbits[index] = (uint8_t)v[index];
    }
    dscp_map.in_use = 1;

    raw_local_irq_save(flags);

    if (!strcmp(name, "dscp_map_set")) {
#ifdef CONFIG_MV_CUST_FLOW_MAP_HANDLE       
        mv_cust_dscp_map_set(&dscp_map);
#endif        
    }
    else
        printk("%s: illegal operation <%s>\n", __FUNCTION__, attr->attr.name);

    raw_local_irq_restore(flags);

    return len;
}

static DEVICE_ATTR(help,           S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(debug,          S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_1_store);

static DEVICE_ATTR(omci_set,       S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_6_store);
static DEVICE_ATTR(eoam_gh_keep,   S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_1_store);
static DEVICE_ATTR(eoam_txq,       S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_1_store);
static DEVICE_ATTR(eoam_mac,       S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_6h_store);
static DEVICE_ATTR(eoam_write,     S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_1_store);

static DEVICE_ATTR(tcont_show,     S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(tcont_set,      S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_2_store);

static DEVICE_ATTR(app_show,       S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_1_store);
static DEVICE_ATTR(app_flag_set,   S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_2_store);
static DEVICE_ATTR(app_etype_set,  S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_2_store);

static DEVICE_ATTR(udp_ports,      S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(udp_src,        S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_6_store);
static DEVICE_ATTR(udp_dst,        S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_6_store);

static DEVICE_ATTR(flow_map,       S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(flow_map_show,  S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(flow_map_clear, S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(dscp_map_del,   S_IRUSR, mv_cust_spec_proc_show, NULL);
static DEVICE_ATTR(flow_map_debug, S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_flow_store);
static DEVICE_ATTR(flow_map_us_set,S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_flow_store);
static DEVICE_ATTR(flow_map_ds_set,S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_flow_store);
static DEVICE_ATTR(flow_map_del,   S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_flow_store);
static DEVICE_ATTR(tag_flow_get,   S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_flow_store);
static DEVICE_ATTR(untag_flow_get, S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_flow_store);
static DEVICE_ATTR(dscp_map_set,   S_IWUSR, mv_cust_spec_proc_show, mv_cust_spec_proc_64_store);


static struct attribute *mv_cust_spec_proc_attrs[] = {
    &dev_attr_help.attr,    
    &dev_attr_debug.attr,
    &dev_attr_omci_set.attr,
    &dev_attr_eoam_gh_keep.attr,
    &dev_attr_eoam_txq.attr,
    &dev_attr_eoam_mac.attr,
    &dev_attr_eoam_write.attr,
    &dev_attr_tcont_show.attr,
    &dev_attr_tcont_set.attr,    
    &dev_attr_app_show.attr,
    &dev_attr_app_flag_set.attr,
    &dev_attr_app_etype_set.attr,   
    &dev_attr_udp_ports.attr,    
    &dev_attr_udp_src.attr,
    &dev_attr_udp_dst.attr,
    &dev_attr_flow_map_debug.attr,
    &dev_attr_flow_map_us_set.attr,
    &dev_attr_flow_map_ds_set.attr,    
    &dev_attr_dscp_map_set.attr,
    &dev_attr_flow_map_del.attr,
    &dev_attr_tag_flow_get.attr,
    &dev_attr_untag_flow_get.attr,
    &dev_attr_flow_map.attr,    
    &dev_attr_flow_map_show.attr,     
    &dev_attr_flow_map_clear.attr,   
    &dev_attr_dscp_map_del.attr,       

    NULL
};

static struct attribute_group mv_cust_spec_proc_group = {
    .name = "proto",
    .attrs = mv_cust_spec_proc_attrs,
};

int mvcust_sysfs_init(void)
{
    int err;
    struct device *pd;

    pd = bus_find_device_by_name(&platform_bus_type, NULL, "cust");
    if (!pd) {
        platform_device_register_simple("cust", -1, NULL, 0);
        pd = bus_find_device_by_name(&platform_bus_type, NULL, "cust");
    }

    if (!pd) {
        printk(KERN_ERR"%s: cannot find cust device\n", __FUNCTION__);
        pd = &platform_bus;
    }

    err = sysfs_create_group(&pd->kobj, &mv_cust_spec_proc_group);
    if (err) {
        printk(KERN_INFO "sysfs group failed %d\n", err);
        goto out;
    }

    printk(KERN_INFO "= CUST Module SYS FS Init ended successfully =\n");
    out:
    return err;
}

void mvcust_sysfs_delete(void)
{
    struct device *pd;

    pd = bus_find_device_by_name(&platform_bus_type, NULL, "cust");
    if (!pd)
    {
        printk(KERN_ERR"%s: cannot find CUST device\n", __FUNCTION__);
        return;
    }

    sysfs_remove_group(&pd->kobj, &mv_cust_spec_proc_group);

    printk(KERN_INFO "= CUST Module SYS FS Remove ended successfully =\n");
}
