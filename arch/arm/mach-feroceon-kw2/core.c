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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/sysdev.h>
#include <asm/mach/time.h>
#include <linux/clocksource.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/setup.h>
#include <asm/mach-types.h>

#include <asm/mach/arch.h>
#include <asm/mach/flash.h>
#include <asm/mach/irq.h>
#include <asm/mach/map.h>
#include <mach/system.h>

#include <linux/tty.h>
#include <linux/platform_device.h>
#include <linux/serial_core.h>
#include <linux/serial.h>
#include <linux/serial_8250.h>
#include <linux/serial_reg.h>
#include <asm/serial.h>
#include <plat/cache-feroceon-l2.h>
#include <linux/proc_fs.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>

#include <mach/serial.h>

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/mvCtrlEthCompLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "cpu/mvCpu.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "gpp/mvGpp.h"
#include "plat/gpio.h"
#ifdef CONFIG_MTD_NAND_NFC
#include "mv_mtd/nand_nfc.h"
#endif
#if defined(CONFIG_MV_INCLUDE_SDIO)
#include "sdmmc/mvSdmmc.h"
#include <plat/mvsdio.h>
#endif
#if defined(CONFIG_MV_INCLUDE_CESA)
#include "cesa/mvCesa.h"
#endif

/* WD */
#include <plat/orion_wdt.h>

/* I2C */
#include <linux/i2c.h>	
#include <linux/mv643xx_i2c.h>
#include "ctrlEnv/mvCtrlEnvSpec.h"

/* SPI */
#include "mvSysSpiApi.h"


/* Eth Phy */
#include "mvSysEthPhyApi.h"

extern unsigned int irq_int_type[];

/* for debug putstr */
#include <mach/uncompress.h> 
static char arr[256];

#ifdef MV_INCLUDE_EARLY_PRINTK
extern void putstr(const char *ptr);
void mv_early_printk(char *fmt,...)
{
	va_list args;
	va_start(args, fmt);
	vsprintf(arr,fmt,args);
	va_end(args);
	putstr(arr);
}
#endif


extern void __init mv_map_io(void);
extern void __init mv_init_irq(void);
extern struct sys_timer mv_timer;
extern MV_CPU_DEC_WIN* mv_sys_map(void);
#if defined(CONFIG_MV_INCLUDE_CESA)
extern u32 mv_crypto_base_get(void);
#endif
unsigned int support_wait_for_interrupt = 0x1;

#ifdef CONFIG_MV_PMU_PROC
struct proc_dir_entry *mv_pm_proc_entry;
#endif /* CONFIG_MV_PMU_PROC */

u32 mvTclk = 166666667;
u32 mvSysclk = 200000000;
u32 mvIsUsbHost = 1;
u32 mod_config = 0;

#ifdef CONFIG_MV_INCLUDE_GIG_ETH
u8	mvMacAddr[CONFIG_MV_ETH_PORTS_NUM][6];
u16	mvMtu[CONFIG_MV_ETH_PORTS_NUM] = {0};
#endif
extern MV_U32 gBoardId;
extern unsigned int elf_hwcap;

#ifdef CONFIG_MTD_NAND_LNC
extern unsigned int mv_nand_ecc;
#endif

static int noL2 = 0;
static int __init noL2_setup(char *__unused)
{
	noL2 = 1;
	return 1;
}
__setup("noL2", noL2_setup);


static int __init parse_tag_mv_uboot(const struct tag *tag)
{
	unsigned int mvUbootVer = 0;
	int i = 0;

	mvUbootVer = tag->u.mv_uboot.uboot_version;
	mvIsUsbHost = tag->u.mv_uboot.isUsbHost;

	printk("Using UBoot passing parameters structure\n");

	gBoardId =  (mvUbootVer & 0xff);
#ifdef CONFIG_MV_INCLUDE_GIG_ETH
	for (i = 0; i < CONFIG_MV_ETH_PORTS_NUM; i++) {
#if defined (CONFIG_OVERRIDE_ETH_CMDLINE)
		memset(mvMacAddr[i], 0, 6);
		mvMtu[i] = 0;
#else
		memcpy(mvMacAddr[i], tag->u.mv_uboot.macAddr[i], 6);
		mvMtu[i] = tag->u.mv_uboot.mtu[i];
#endif
	}
#endif
#ifdef CONFIG_MTD_NAND_LNC
	/* get NAND ECC type(1-bit or 4-bit) */
	if((mvUbootVer >> 8) >= 0x3040c)
		mv_nand_ecc = tag->u.mv_uboot.nand_ecc;
	else
		mv_nand_ecc = 1; /* fallback to 1-bit ECC */
#endif

	mod_config = tag->u.mv_uboot.mod_bitmask;

	return 0;
}

__tagtable(ATAG_MV_UBOOT, parse_tag_mv_uboot);

#ifdef CONFIG_MV_INCLUDE_CESA
unsigned char*  mv_sram_usage_get(int* sram_size_ptr)
{
	int used_size = 0;

#if defined(CONFIG_MV_CESA)
	used_size = sizeof(MV_CESA_SRAM_MAP);
#endif

	if(sram_size_ptr != NULL)
		*sram_size_ptr = _8K - used_size;

	return (char *)(mv_crypto_base_get() + used_size);
}
#endif


void print_board_info(void)
{
	char name_buff[50];
	printk("\n  Marvell Development Board (LSP Version %s_%s)", LSP_VERSION, UPON_SDK_VERSION);

	mvBoardNameGet(name_buff);
	printk("-- %s ",name_buff);

	mvCtrlModelRevNameGet(name_buff);
	printk(" Soc: %s",  name_buff);
#if defined(MV_CPU_LE)
	printk(" LE");
#else
	printk(" BE");
#endif
	printk("\n\n");
	printk(" Detected Tclk %d and SysClk %d \n",mvTclk, mvSysclk);
}

/*****************************************************************************
 * I2C(TWSI)
 ****************************************************************************/

/* Platform devices list */

static struct mv64xxx_i2c_pdata kw_i2c_pdata = {
       .freq_m         = 8, /* assumes 166 MHz TCLK */
       .freq_n         = 3,
       .timeout        = 1000, /* Default timeout of 1 second */
};

static struct resource kw_i2c_resources[] = {
       {
               .name   = "i2c base",
               .start  = INTER_REGS_BASE + MV_TWSI_SLAVE_REGS_OFFSET(0),
               .end    = INTER_REGS_BASE + MV_TWSI_SLAVE_REGS_OFFSET(0) + 0x20 -1,
               .flags  = IORESOURCE_MEM,
       },
       {
               .name   = "i2c irq",
               .start  = TWSI_IRQ_NUM(0),
               .end    = TWSI_IRQ_NUM(0),
               .flags  = IORESOURCE_IRQ,
       },
};

static struct platform_device kw_i2c = {
       .name           = MV64XXX_I2C_CTLR_NAME,
       .id             = 0,
       .num_resources  = ARRAY_SIZE(kw_i2c_resources),
       .resource       = kw_i2c_resources,
       .dev            = {
               .platform_data = &kw_i2c_pdata,
       },
};

#ifdef CONFIG_MTD_NAND_NFC
/*****************************************************************************
 * NAND controller
 ****************************************************************************/
static struct resource kw2_nfc_resources[] = {
       {
               .start  = INTER_REGS_BASE + MV_NFC_REGS_OFFSET,
               .end    = INTER_REGS_BASE + MV_NFC_REGS_OFFSET + 0x400 -1,
               .flags  = IORESOURCE_MEM,
       }
};
#endif

#if 0
struct mtd_partition nand_parts_info[] = {
	{ .name = "uboot",
	  .offset = 0,
	  .size = 2 * 1024 * 1024 },
	{ .name = "uimageU",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = 4 * 1024 * 1024 },
	{ .name = "rootfsU",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = MTDPART_SIZ_FULL },
};
int nand_parts_num = 3;
#endif

struct mtd_partition nand_parts_info[] = {
	{ .name = "uboot",
	  .offset = 0,
	  .size = 2 * 1024 * 1024 },
	{ .name = "uimageU",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = 4 * 1024 * 1024 },
	{ .name = "rootfsU",
	  .offset = MTDPART_OFS_NXTBLK,
        .size = 94 * 1024 * 1024 },
	{ .name = "spacer",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = 2 * 1024 * 1024 },
	{ .name = "uimageB",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = 4 * 1024 * 1024 },
	{ .name = "rootfsB",
	  .offset = MTDPART_OFS_NXTBLK,
	  .size = MTDPART_SIZ_FULL },
};
int nand_parts_num = 6;


#ifdef CONFIG_MTD_NAND_NFC
static struct nfc_platform_data kw2_nfc_data = {
	.nfc_width	= 8,
	.num_devs	= 1,
	.num_cs		= 1,
	.use_dma	= 0,
	.ecc_type	= MV_NFC_ECC_BCH_2K,
	.parts		= nand_parts_info,
	.nr_parts	= 3,
};

static struct platform_device kw2_nfc = {
       .name           = "orion-nfc-hal",
       .id             = 0,
       .num_resources  = ARRAY_SIZE(kw2_nfc_resources),
       .resource       = kw2_nfc_resources,
       .dev            = {
               .platform_data = &kw2_nfc_data,
       },
};
#endif

/*****************************************************************************
 * UART
 ****************************************************************************/
static struct resource mv_uart0_resources[] = {
	{
		.start		= PORT0_BASE,
		.end		= PORT0_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
#ifdef CONFIG_MV_UART_POLLING_MODE
		.start          = 0,
		.end            = 0,
#else
		.start          = UART_IRQ_NUM(0),
		.end            = UART_IRQ_NUM(0),
#endif
		.flags          = IORESOURCE_IRQ,
	},
};

#if 0
static struct resource mv_uart1_resources[] = {
	{
		.start		= PORT1_BASE,
		.end		= PORT1_BASE + 0xff,
		.flags		= IORESOURCE_MEM,
	},
	{
		.start          = UART_IRQ_NUM(1),
		.end            = UART_IRQ_NUM(1),
		.flags          = IORESOURCE_IRQ,
	},
};
#endif

static struct plat_serial8250_port mv_uart0_data[] = {
	{
		.mapbase	= PORT0_BASE,
		.membase	= (char *)PORT0_BASE,
#ifdef CONFIG_MV_UART_POLLING_MODE
		.irq		= 0, //UART_IRQ_NUM(0),
#else
		.irq		= UART_IRQ_NUM(0),
#endif
		.flags		= UPF_FIXED_TYPE | UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_DWAPB,
		.private_data	= (void *)0xf101207c,
		.type           = PORT_16550A,
		.regshift	= 2,
	},
	{ },
};

static struct plat_serial8250_port mv_uart1_data[] = {
	{
		.mapbase	= PORT1_BASE,
		.membase	= (char *)PORT1_BASE,
		.irq		= UART_IRQ_NUM(1),
		.flags		= UPF_FIXED_TYPE | UPF_SKIP_TEST | UPF_BOOT_AUTOCONF,
		.iotype		= UPIO_DWAPB,
		.private_data	= (void *)0xf101217c,
		.type           = PORT_16550A,
		.regshift	= 2,
	},
	{ },
};

static struct platform_device mv_uart = {
	.name			= "serial8250",
	.id			= PLAT8250_DEV_PLATFORM,
	.dev			= {
		.platform_data	= mv_uart0_data,
	},
	.num_resources		= 2, /*ARRAY_SIZE(mv_uart_resources),*/
	.resource		= mv_uart0_resources,
};


static void serial_initialize(void)
{
	mv_uart0_data[0].uartclk = mv_uart1_data[0].uartclk = mvTclk;
	platform_device_register(&mv_uart);
}

#if defined(CONFIG_MV_INCLUDE_SDIO)

static struct resource mvsdio_resources[] = {
	[0] = {
		.start	= INTER_REGS_BASE + MV_SDMMC_REGS_OFFSET,
		.end	= INTER_REGS_BASE + MV_SDMMC_REGS_OFFSET + SZ_1K -1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= SDIO_IRQ_NUM,
		.end	= SDIO_IRQ_NUM,
		.flags	= IORESOURCE_IRQ,
	},

};

static u64 mvsdio_dmamask = 0xffffffffUL;

static struct mvsdio_platform_data mvsdio_data = {
	.gpio_write_protect	= 0,
	.gpio_card_detect	= 0,
	.dram			= NULL,
};

static struct platform_device mv_sdio_plat = {
	.name		= "mvsdio",
	.id		= -1,
	.dev		= {
		.dma_mask = &mvsdio_dmamask,
		.coherent_dma_mask = 0xffffffff,
		.platform_data	= &mvsdio_data,
	},
	.num_resources	= ARRAY_SIZE(mvsdio_resources),
	.resource	= mvsdio_resources,
};


#endif /* #if defined(CONFIG_MV_INCLUDE_SDIO) */

#ifdef CONFIG_MV_ETHERNET
/*****************************************************************************
 * Ethernet
 ****************************************************************************/
#if defined(CONFIG_MV_ETH_LEGACY)
static struct platform_device mv88fx_eth = {
	.name		= "mv88fx_eth",
	.id		= 0,
	.num_resources	= 0,
};
#elif defined(CONFIG_MV_ETH_NETA)
static struct platform_device mv88fx_neta = {
	.name		= "mv88fx_neta",
	.id		= 0,
	.num_resources	= 0,
};
#else

#error "Ethernet Mode is not defined (should be Legacy or NETA)"

#endif /* Ethernet mode: legacy or NETA */

#endif /* CONFIG_MV_ETHERNET */

/*****************************************************************************
 * WATCHDOG
 ****************************************************************************/

/* the orion watchdog device data structure */
static struct orion_wdt_platform_data mv_wdt_data = {
       .tclk           = 0,
};

/* the watchdog device structure */
static struct platform_device mv_wdt_device = {
       .name           = "orion_wdt",
       .id             = -1,
       .dev            = {
               .platform_data  = &mv_wdt_data,
       },
       .num_resources  = 0,
};

/* init the watchdog device */
static void __init mv_wdt_init(void)
{
       mv_wdt_data.tclk = mvTclk;
       platform_device_register(&mv_wdt_device);
}


static void __init kirkwood_l2_init(void)
{
#ifdef CONFIG_CACHE_FEROCEON_L2_WRITETHROUGH
	MV_REG_BIT_SET(CPU_L2_CONFIG_REG, 0x10);
	feroceon_l2_init(1);
#else
	MV_REG_BIT_RESET(CPU_L2_CONFIG_REG, 0x10);
	feroceon_l2_init(0);
#endif
}

static void __init mv_init(void)
{
	MV_U32 boardId;

#ifndef CONFIG_MV_DRAM_DEFAULT_ACCESS_CFG
        /* Support DRAM access configuration for Avanta-MC only */
        if (gBoardId == DB_88F6601_BP_ID || gBoardId == RD_88F6601_MC_ID ||
	    gBoardId == RD_88F6601_MC2L_ID) {
                printk("DRAM access: ");
#ifdef CONFIG_MV_DRAM_XBAR_ACCESS_CFG
                printk("XBAR\n");
#else
                printk("fast-path\n");
#endif
        }
#endif

#ifdef CONFIG_CACHE_FEROCEON_L2
	if ((noL2 == 0) && (mvCpuL2Exists() == MV_TRUE))
		kirkwood_l2_init();
	else
		printk("No L2-Cache.\n");
#endif

	/* init the Board environment */
	mvBoardEnvInit();

	/* init the controller environment */
	if (mvCtrlEnvInit()) {
		printk("Controller env initialization failed.\n");
		return;
	}

	/* Init the CPU windows setting and the access protection windows. */
	if (mvCpuIfInit(mv_sys_map())) {
		printk("Cpu Interface initialization failed.\n");
		return;
	}

	/* Init Tclk & SysClk */
	mvTclk = mvBoardTclkGet();
	mvSysclk = mvBoardSysClkGet();

	support_wait_for_interrupt = 1;

#ifdef CONFIG_JTAG_DEBUG
	support_wait_for_interrupt = 0; /*  for Lauterbach */
#endif

	elf_hwcap &= ~HWCAP_JAVA;

	serial_initialize();

	/* At this point, the CPU windows are configured according to default definitions in mvSysHwConfig.h */
	/* and cpuAddrWinMap table in mvCpuIf.c. Now it's time to change defaults for each platform.         */
	mvCpuIfAddDecShow();

	print_board_info();

	mv_gpio_init();

#ifdef CONFIG_MV_INCLUDE_SPI
	mvSysSpiInit(0, _16M);
#endif

	/* ETH-PHY */
	mvSysEthPhyInit();

	/* I2C */
	platform_device_register(&kw_i2c);

#ifdef CONFIG_MV_PMU_PROC
	mv_pm_proc_entry = proc_mkdir("mv_pm", NULL);
#endif

#if defined(CONFIG_MV_INCLUDE_SDIO)
	if (mvCtrlSdioSupport()) {
		if (MV_TRUE == mvCtrlPwrClckGet(SDIO_UNIT_ID, 0)) {
			int irq_detect = mvBoardSDIOGpioPinGet(BOARD_GPP_SDIO_DETECT);
			MV_UNIT_WIN_INFO addrWinMap[MAX_TARGETS + 1];
	
			if (irq_detect != MV_ERROR) {
				mvsdio_data.gpio_card_detect = mvBoardSDIOGpioPinGet(BOARD_GPP_SDIO_DETECT);
				irq_int_type[mvBoardSDIOGpioPinGet(BOARD_GPP_SDIO_DETECT)+IRQ_GPP_START] = GPP_IRQ_TYPE_CHANGE_LEVEL;
			}
	
			if (mvBoardSDIOGpioPinGet(BOARD_GPP_SDIO_WP) != MV_ERROR)
				mvsdio_data.gpio_write_protect = mvBoardSDIOGpioPinGet(BOARD_GPP_SDIO_WP);
	
			if (MV_OK == mvCtrlAddrWinMapBuild(addrWinMap, MAX_TARGETS + 1))
				if (MV_OK == mvSdmmcWinInit(addrWinMap))
					mvsdio_data.clock = 200000000; //mvBoardTclkGet();
			platform_device_register(&mv_sdio_plat);
		}
	}
#endif

#ifdef CONFIG_MV_ETHERNET
#if defined(CONFIG_MV_ETH_LEGACY)
	platform_device_register(&mv88fx_eth);
#elif defined(CONFIG_MV_ETH_NETA)
	mv88fx_neta.dev.platform_data = NULL;
	platform_device_register(&mv88fx_neta);
#endif
#endif

#ifdef CONFIG_THERMAL_SENSOR_KW2
	platform_device_register_simple("KW2Thermal", 0, NULL, 0);
#endif

#ifdef CONFIG_MTD_NAND_NFC
	kw2_nfc_data.tclk = mvTclk;
	platform_device_register(&kw2_nfc);
#endif

	/* Watchdog */
	mv_wdt_init();

	/* CPU idle driver */
	boardId = mvBoardIdGet();
	if (boardId == DB_88F6535_BP_ID || boardId == RD_88F6560_GW_ID)
		platform_device_register_simple("kw_cpuidle", 0, NULL, 0);
}

MACHINE_START(FEROCEON_KW2 ,"Feroceon-KW2")
	/* MAINTAINER("MARVELL") */
	.phys_io = 0xf1000000,
	.io_pg_offst = ((0xf1000000) >> 18) & 0xfffc,
	.boot_params = 0x00000100,
	.map_io = mv_map_io,
	.init_irq = mv_init_irq,
	.timer = &mv_timer,
	.init_machine = mv_init,
MACHINE_END
