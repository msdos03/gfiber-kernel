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
* mv_cust_mod.c
*
* DESCRIPTION:
*       
* 
*******************************************************************************/
#include <linux/module.h>
#include <linux/moduleparam.h>
#include "mv_cust_dev.h"


#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif

static void __exit mv_cust_mod_exit(void)
{
    mvcust_dev_shutdown();
}
module_exit(mv_cust_mod_exit);


static int __init mv_cust_mod_init(void)
{
    if (mvcust_dev_init() != 0)
    {
        //mv_cust_mod_exit();
        printk(KERN_ERR "\nMV_CUST module initialization failed \n\n");
        return -1;
    }

    printk(KERN_INFO "\nMV_CUST module inserted - %s\n\n",MV_CUST_VERSION);

    return 0;
}

//module_init(mv_cust_mod_init);
device_initcall_sync(mv_cust_mod_init);
