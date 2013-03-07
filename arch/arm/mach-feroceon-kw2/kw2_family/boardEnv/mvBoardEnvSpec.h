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

    * 	Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

    *	Redistributions in binary form must reproduce the above copyright
	notice, this list of conditions and the following disclaimer in the
	documentation and/or other materials provided with the distribution.

    *	Neither the name of Marvell nor the names of its contributors may be
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

*******************************************************************************/

#ifndef __INCmvBoardEnvSpech
#define __INCmvBoardEnvSpech

#include "mvSysHwConfig.h"

/* For future use */
#define BD_ID_DATA_START_OFFS			0x0
#define BD_DETECT_SEQ_OFFS			0x0
#define BD_SYS_NUM_OFFS				0x4
#define BD_NAME_OFFS				0x8

/* I2C bus addresses */
#define MV_BOARD_CTRL_I2C_ADDR			0x0	/* Controller slave addr */
#define MV_BOARD_CTRL_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM0_I2C_ADDR			0x56
#define MV_BOARD_DIMM0_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_DIMM1_I2C_ADDR			0x54
#define MV_BOARD_DIMM1_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_EEPROM_I2C_ADDR	    	0x51
#define MV_BOARD_EEPROM_I2C_ADDR_TYPE 		ADDR7_BIT
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR	   	0x50
#define MV_BOARD_MAIN_EEPROM_I2C_ADDR_TYPE 	ADDR7_BIT
#define MV_BOARD_MUX_I2C_ADDR_ENTRY		0x2
#define MV_BOARD_DIMM_I2C_CHANNEL		0x0

#define BOOT_FLASH_INDEX		0
#define MAIN_FLASH_INDEX		1

#define BOARD_ETH_START_PORT_NUM	0

/* Board specific configuration */
/* ============================ */

/* boards ID numbers */

#define BOARD_ID_BASE			0x20

/* New board ID numbers */
#define DB_88F6535_BP_ID		(BOARD_ID_BASE)
#define RD_88F6510_SFU_ID		(BOARD_ID_BASE+0x1)
#define RD_88F6560_GW_ID		(BOARD_ID_BASE+0x2)
#define RD_88F6530_MDU_ID		(BOARD_ID_BASE+0x3)
#define DB_88F6560_PCAC_ID		(BOARD_ID_BASE+0x4)
#define DB_88F6601_BP_ID		(BOARD_ID_BASE+0x5)
#define RD_88F6601_MC_ID		(BOARD_ID_BASE+0x6)
#define DB_CUSTOMER_ID			(BOARD_ID_BASE+0x7)
#define GFLT200_ID			DB_CUSTOMER_ID
#define MV_MAX_BOARD_ID			(DB_CUSTOMER_ID + 1)

/***************************************************************************
** RD-88F6510-SFU
****************************************************************************/
#define RD_88F6510_MPP0_7		0x11111111
#define RD_88F6510_MPP8_15		0x31111111
#define RD_88F6510_MPP16_23		0x20000003
#define RD_88F6510_MPP24_31		0x06622222
#define RD_88F6510_MPP32_39		0x00044444
#define RD_88F6510_MPP40_47		0x00006660
#define RD_88F6510_MPP48_55		0x00000460
#define RD_88F6510_MPP56_63		0x00000000
#define RD_88F6510_MPP64_71		0x00500000
#define RD_88F6510_MPP72_79		0x00000000
#define RD_88F6510_MPP80_87		0x00000000
#define RD_88F6510_MPP88_88		0x00000000

/* GPPs
MPP#	NAME			IN/OUT
--------------------------------------
17		WiFi_Host2WLAN	OUT
18		free		free
19		WiFi_WLAN2Host	IN
20		WiFi_PDn	OUT
21		WiFi_PWR	IN/OUT?
22		WiFi_RSTn	OUT
31		free		free
37		NF&SPI_WP	OUT
39		free (ground)	free
40		free (3.3V)	free
44		LED_SYS		OUT
45		LED_PON		OUT
46		LED_Phone	OUT
47		LED_PWR		OUT
48		free		free
51		free		free
52		free		free
53		free		free
54		free		free
55		free		free
56		free (ground)	free
57		free		free
58		free		free
59		free		free
60		free		free
61		free		free
62		free		free
63		free		free
64		free		free
65		free		free
66		PB		IN
67		free		free
68		TXfault_RST	IN/OUT?
*/
#define RD_88F6510_GPP_OUT_ENA_LOW	0x0
#define RD_88F6510_GPP_OUT_ENA_MID	0x0
#define RD_88F6510_GPP_OUT_ENA_HIGH	(BIT2 | BIT4)

#define RD_88F6510_GPP_OUT_VAL_LOW	0x0
#define RD_88F6510_GPP_OUT_VAL_MID	(BIT5)
#define RD_88F6510_GPP_OUT_VAL_HIGH	0

#define RD_88F6510_GPP_POL_LOW		0x0
#define RD_88F6510_GPP_POL_MID		0x0
#define RD_88F6510_GPP_POL_HIGH		0x0

/***************************************************************************
** RD-88F6560-GW
****************************************************************************/
#define RD_88F6560_MPP0_7		0x11111111
#define RD_88F6560_MPP8_15		0x31111111
#define RD_88F6560_MPP16_23		0x11111113
#define RD_88F6560_MPP24_31		0x04411001
#define RD_88F6560_MPP32_39		0x60004444
#define RD_88F6560_MPP40_47		0x04006060
#define RD_88F6560_MPP48_55		0x00000460
#define RD_88F6560_MPP56_63		0x00000000
#define RD_88F6560_MPP64_71		0x90500000
#define RD_88F6560_MPP72_79		0x00044444
#define RD_88F6560_MPP80_87		0x10000000
#define RD_88F6560_MPP88_88		0x00000000

/* GPPs
MPP#	NAME			IN/OUT
--------------------------------------
24		TDM_Reset	OUT
25		TDM_INTm0	IN
31		SD_WP		IN
37		HDD_PWR_CTRL	OUT
38		free		free
40		Fan_PWR_CTRL	OUT
44		Phy_INTn	IN
45		FXO_CTRL	OUT
47		WPS_Switch	IN
48		SD_Status	IN
51		TDM_INTn1	IN
52		Phone_Led	OUT
53		free		free
54		free		free
55		free		free
56		free		free
57		free		free
58		USB_Dev_Vbus	IN
59		USB_OC		IN
60		GP0		IN
61		GP1		IN
62		GP2		IN
63		free		free
64		free		free
65		free		free
66		free		free
68		SYS_LED		OUT
77		RST_PEX#	OUT
*/
#define RD_88F6560_GPP_OUT_ENA_LOW	BIT31
#define RD_88F6560_GPP_OUT_ENA_MID	(BIT5 | BIT12 | BIT15 | BIT16 | BIT19 | BIT26 | BIT27 | BIT28 | BIT29 | BIT30)
#define RD_88F6560_GPP_OUT_ENA_HIGH	0x0

#define RD_88F6560_GPP_OUT_VAL_LOW	0x0
#define RD_88F6560_GPP_OUT_VAL_MID	(BIT20)
#define RD_88F6560_GPP_OUT_VAL_HIGH	(BIT13)

#define RD_88F6560_GPP_POL_LOW		0x0
#define RD_88F6560_GPP_POL_MID		(BIT19)
#define RD_88F6560_GPP_POL_HIGH		0x0

/***************************************************************************
** RD-88F6530-MDU
****************************************************************************/
#define RD_88F6530_MPP0_7		0x11111111
#define RD_88F6530_MPP8_15		0x31111111
#define RD_88F6530_MPP16_23		0x00005003
#define RD_88F6530_MPP24_31		0x04422222
#define RD_88F6530_MPP32_39		0x00444444
#define RD_88F6530_MPP40_47		0x00044400
#define RD_88F6530_MPP48_55		0x00000000
#define RD_88F6530_MPP56_63		0x00000000
#define RD_88F6530_MPP64_69		0x00000000
#define RD_88F6530_MPP72_79		0x00000000
#define RD_88F6530_MPP80_87		0x00000000
#define RD_88F6530_MPP88_88		0x00000000

/* GPPs
MPP#	NAME			IN/OUT
-------------------------------
*/
/*the output GPIO is 17, 21, 23, 59, 60 */
/*61,62,63 as INPUT pin */

#define RD_88F6530_GPP_OUT_ENA_LOW	0xff5dffff
#define RD_88F6530_GPP_OUT_ENA_MID	0xffffffff
#define RD_88F6530_GPP_OUT_ENA_HIGH	0xffffffff

/*the output default value = 1 */
#define RD_88F6530_GPP_OUT_VAL_LOW	0x00A20000
#define RD_88F6530_GPP_OUT_VAL_MID	0x00000000
#define RD_88F6530_GPP_OUT_VAL_HIGH	(BIT4)

#define RD_88F6530_GPP_POL_LOW		0x0
#define RD_88F6530_GPP_POL_MID		0x0
#define RD_88F6530_GPP_POL_HIGH		0x0

/***************************************************************************
** RD-88F6560-PCAC
****************************************************************************/
#define DB_88F6560_PCAC_MPP0_7		0x99999999
#define DB_88F6560_PCAC_MPP8_15		0x31199999
#define DB_88F6560_PCAC_MPP16_23	0x00000003
#define DB_88F6560_PCAC_MPP24_31	0x00000000
#define DB_88F6560_PCAC_MPP32_39	0x66004444
#define DB_88F6560_PCAC_MPP40_47	0x00006666
#define DB_88F6560_PCAC_MPP48_55	0x00000300
#define DB_88F6560_PCAC_MPP56_63	0x00000000
#define DB_88F6560_PCAC_MPP64_71	0x00000000
#define DB_88F6560_PCAC_MPP72_79	0x00000000
#define DB_88F6560_PCAC_MPP80_87	0x10000000
#define DB_88F6560_PCAC_MPP88_88	0x00000000

/* GPPs
MPP#	NAME			IN/OUT
--------------------------------------
58		USB_Dev_Vbus	IN
68		7-Seg		OUT
69		7-Seg		OUT
70		7-Seg		OUT
71		7-Seg		OUT
*/
#define DB_88F6560_PCAC_GPP_OUT_ENA_LOW		0x0
#define DB_88F6560_PCAC_GPP_OUT_ENA_MID		(BIT26)
#define DB_88F6560_PCAC_GPP_OUT_ENA_HIGH	0x0

#define DB_88F6560_PCAC_GPP_OUT_VAL_LOW		0x0
#define DB_88F6560_PCAC_GPP_OUT_VAL_MID		0x0
#define DB_88F6560_PCAC_GPP_OUT_VAL_HIGH	0x0

#define DB_88F6560_PCAC_GPP_POL_LOW		0x0
#define DB_88F6560_PCAC_GPP_POL_MID		0x0
#define DB_88F6560_PCAC_GPP_POL_HIGH		0x0

/***************************************************************************
** DB-88F6560-BP
****************************************************************************/
#define DB_88F6535_MPP0_7		0x11111111
#define DB_88F6535_MPP8_15		0x31111111
#define DB_88F6535_MPP16_23		0x03555603
#define DB_88F6535_MPP24_31		0x04400000
#define DB_88F6535_MPP32_39		0x00444444
#define DB_88F6535_MPP40_47		0x00000000
#define DB_88F6535_MPP48_55		0x00000000
#define DB_88F6535_MPP56_63		0x00000000
#define DB_88F6535_MPP64_71		0x90000000
#define DB_88F6535_MPP72_79		0x00000000
#define DB_88F6535_MPP80_87		0x90000000
#define DB_88F6535_MPP88_88		0x00000000

/* GPPs
MPP#	NAME			IN/OUT
--------------------------------------
17		XVR_TXfault_RST	IN/OUT?
31		UsbDevice_Vbus	IN
68		SD_Status	IN
67		PEX_VCC_OFF	OUT
69		SD_WP		IN
*/
#define DB_88F6535_GPP_OUT_ENA_LOW	(BIT31 | BIT23 | BIT17)
#define DB_88F6535_GPP_OUT_ENA_MID	0x0
#define DB_88F6535_GPP_OUT_ENA_HIGH	(BIT5 | BIT23)

#define DB_88F6535_GPP_OUT_VAL_LOW	0x0
#define DB_88F6535_GPP_OUT_VAL_MID	0x0
#define DB_88F6535_GPP_OUT_VAL_HIGH	(BIT3)

#define DB_88F6535_GPP_POL_LOW		(BIT23)
#define DB_88F6535_GPP_POL_MID		0x0
#define DB_88F6535_GPP_POL_HIGH		0x0


/***************************************************************************
** DB-CUSTOMER
****************************************************************************/
#define DB_CUSTOMER_MPP0_7		0x21111111
#define DB_CUSTOMER_MPP8_15		0x00003311
#define DB_CUSTOMER_MPP16_23		0x00001100
#define DB_CUSTOMER_MPP24_31		0x00000000
#define DB_CUSTOMER_MPP32_39		0x00000000
#define DB_CUSTOMER_MPP40_47		0x00000000
#define DB_CUSTOMER_MPP48_55		0x00000000
#define DB_CUSTOMER_OE_LOW		0x0
#define DB_CUSTOMER_OE_HIGH		(~((BIT6) | (BIT7) | (BIT8) | (BIT9)))
#define DB_CUSTOMER_OE_VAL_LOW		0x0
#define DB_CUSTOMER_OE_VAL_HIGH		0x0

/***************************************************************************
** DB-88F6601-BP
****************************************************************************/
#define DB_88F6601_MPP0_7		0x22222220
#define DB_88F6601_MPP8_15		0x52222222
#define DB_88F6601_MPP16_23		0x55520555
#define DB_88F6601_MPP24_31		0x04445555
#define DB_88F6601_MPP32_37		0x00050040

/* GPPs
 0 GPIO[0] (inout) P_INT_S_TX_Fault
 1 SPI0_MOSI (out)
 2 SPI0_SCK (out)
 3 SPI0_CSn[0] (out)
 4 SPI0_MISO (in)
 5 I2C0_SDA (inout)
 6 I2C0_SCK (inout)
 7 UA0_TXD (out)
 8 UA0_RXD (in)
 9 TDM2C_CODEC_INTn (in)
10 TDM2C_CODEC_RSTn (out)
11 TDM2C_PCLK (inout)
12 TDM2C_FS (inout)
13 TDM2C_DRX (in)
14 TDM2C_DTX (out)
15 SPI1_CSn[0] (out)
16 SPI1_MOSI (out)
17 SPI1_SCK (out)
18 SPI1_MISO (in)
19 GPIO[19] (inout) SFP_LOS
20 DYING_GASP (in)
21 P3_LED (out)
22 P0_LED (out)
23 P1_LED (out)
24 P2_LED (out)
25 C0_LED (out)
26 C1_LED (out)
27 C2_LED (out)
28 GE_MDC (out)
29 GE_MDIO (inout)
30 XVR_SD (in)
31  GPIO[31] (inout) SFP_Dis
32  GPIO[32] (inout) SFP_Present
33  PON_BEN (out)
34  GPIO[34] (inout) XVR_TX_Ind
35  GPIO[35] (inout) XVR_TXfault_RST
36  PON_BEN (out)
37  TX_PD (out)
*/
#define DB_88F6601_GPP_OUT_ENA_LOW	(BIT0 | BIT19)
#define DB_88F6601_GPP_OUT_ENA_MID	(BIT0 | BIT2  | BIT3 )

#define DB_88F6601_GPP_OUT_VAL_LOW	0x0
#define DB_88F6601_GPP_OUT_VAL_MID	0x0

#define DB_88F6601_GPP_POL_LOW		0x0
#define DB_88F6601_GPP_POL_MID		0x0




/***************************************************************************
** RD-88F6601
****************************************************************************/
#define RD_88F6601_MPP0_7		0x22222220
#define RD_88F6601_MPP8_15		0x00000002
#define RD_88F6601_MPP16_23		0x00400000
#define RD_88F6601_MPP24_31		0x00200650
#define RD_88F6601_MPP32_37		0x00000000

/* GPPs
 1 SPI0_MOSI (out)
 2 SPI0_SCK (out)	
 3 SPI0_CSn[0] (out)
 4 SPI0_MISO (in)
 5 I2C0_SDA (inout)
 6 I2C0_SCK (inout)
 7 UA0_TXD (out)
 8 UA0_RXD (in)
20 LED_PON
21 PON_BEN (out)
24 XVR_Tx_IND
25 LED_G
26 LED_Y 
28 NF&SPI_WP
29 XVR_SD (in)
33 TX_Fault/TX_indication
37 TX_PD


*/
#define RD_88F6601_GPP_OUT_ENA_LOW	(BIT0 | BIT13 | BIT14 | BIT16 | BIT17 | BIT18 | BIT19 | BIT22 | BIT23 | BIT24 | BIT27| BIT30 | BIT31) 
#define RD_88F6601_GPP_OUT_ENA_MID	(BIT0 | BIT3 | BIT4)

#define RD_88F6601_GPP_OUT_VAL_LOW	0x0
#define RD_88F6601_GPP_OUT_VAL_MID	0x0

#define RD_88F6601_GPP_POL_LOW		(BIT23)
#define RD_88F6601_GPP_POL_MID		0x0

/***************************************************************************
** GFLT200
****************************************************************************/
#define GFLT200_MPP0_7			0x22222220
#define GFLT200_MPP8_15			0x00000002
#define GFLT200_MPP16_23		0x00000000
#define GFLT200_MPP24_31		0x40000000
#define GFLT200_MPP32_37		0x00000004

/* GPPs
 1 SPI_MOSI (out)
 2 SPI_SCK (out)
 3 SPI_CS_L (out)
 4 SPI_MISO (in)
 5 I2C_SDA (inout)
 6 I2C_SCLK (inout)
 7 UART0_TX (out)
 8 UART0_RX (in)
 9 VDD_MARGIN_EN (out)
10 VDD_MARGIN_CTRL (out)
11 PON_LINK_LED (out)
12 PON_ERROR_LED (out)
13 BOARD_VER[0] (in)
15 BOARD_VER[1] (in)
17 SW_RESET (out)
18 BOARD_VER[2] (in)
21 PON_TX_DIS (out)
23 GE_DATA_LED (out)
24 GE_LINK_LED (out)
26 PON_C2_DATA (out)
27 PON_C2_CLK (out)
28 SPI_WP_L (out)
29 PON_RX_LOS (in)
31 UART1_RX (out)
32 UART2_TX (in)
36 PON_RX_PMON (in)
37 PON_PWR_EN_L (out)
*/

#define GFLT200_GPP_OUT_ENA_LOW		(BIT13 | BIT15 | BIT18 | BIT29)
#define GFLT200_GPP_OUT_ENA_MID		(BIT4)

#define GFLT200_GPP_OUT_VAL_LOW		(BIT9 | BIT10 | BIT21 | BIT26 | BIT27 | BIT28)
#define GFLT200_GPP_OUT_VAL_MID		0x0

#define GFLT200_GPP_POL_LOW		0x0
#define GFLT200_GPP_POL_MID		0x0

#endif /* __INCmvBoardEnvSpech */
