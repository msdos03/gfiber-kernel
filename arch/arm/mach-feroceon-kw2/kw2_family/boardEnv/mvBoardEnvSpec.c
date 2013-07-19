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
#include "mvCommon.h"
#include "mvBoardEnvLib.h"
#include "mvBoardEnvSpec.h"
#include "gpp/mvGpp.h"
#include "twsi/mvTwsi.h"

/***************************************************************************
** RD-88F6510-SFU
****************************************************************************/

#define RD_88F6510_BOARD_NAND_READ_PARAMS	0x000C0282
#define RD_88F6510_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define RD_88F6510_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO rd88f6510InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6510InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x0}
	,
	{BOARD_MAC_SPEED_1000M, 0x9},
	{N_A,N_A}

/* 	{BOARD_MAC_SPEED_AUTO, 0x9} */

};

MV_BOARD_MPP_TYPE_INFO rd88f6510InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH,
	 .ethSataComplexOpt = ESC_OPT_MAC0_2_SW_P4 | ESC_OPT_MAC1_2_SW_P5 | ESC_OPT_FE3PHY | ESC_OPT_GEPHY_SW_P0,
	 .ethPortsMode = 0x0
	 }
};

MV_BOARD_GPP_INFO rd88f6510InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_SYS_LED, 44},
	{BOARD_GPP_PON_LED, 45},
	{BOARD_GPP_PEX_RESET, 67},
	{BOARD_GPP_PON_XVR_TX, 68}
};

MV_DEV_CS_INFO rd88f6510InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#if defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6510InfoBoardMppConfigValue[] = {
	{{
		RD_88F6510_MPP0_7,
		RD_88F6510_MPP8_15,
		RD_88F6510_MPP16_23,
		RD_88F6510_MPP24_31,
		RD_88F6510_MPP32_39,
		RD_88F6510_MPP40_47,
		RD_88F6510_MPP48_55,
		RD_88F6510_MPP56_63,
		RD_88F6510_MPP64_71,
		RD_88F6510_MPP72_79,
		RD_88F6510_MPP80_87,
		RD_88F6510_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO rd88f6510InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,	/* set to -1 for timer operation */
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, 5, -1},
	 .smiScanMode = 1,
	 .quadPhyAddr = 0,
	 .forceLinkMask = 0x0
	 }
};

MV_BOARD_TDM_INFO rd88f6510Tdm880[] = { {1} };

MV_BOARD_TDM_SPI_INFO rd88f6510TdmSpiInfo[] = { {0} };

MV_BOARD_INFO rd88f6510Info = {
	.boardName = "RD-88F6510-SFU",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6510InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6510InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6510InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6510InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6510InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6510InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6510InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6510InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6510InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6510InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6510InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6510InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6510_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6510_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = RD_88F6510_GPP_OUT_ENA_HIGH,
	.gppOutValLow = RD_88F6510_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6510_GPP_OUT_VAL_MID,
	.gppOutValHigh = RD_88F6510_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = RD_88F6510_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6510_GPP_POL_MID,
	.gppPolarityValHigh = RD_88F6510_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = rd88f6510InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(rd88f6510InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_AUTO,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {1}
	,
	.pBoardTdmInt2CsInfo = {rd88f6510Tdm880,
				}
	,
	.boardTdmInfoIndex = 0,

	.deepIdlePwrUpDelay = 0x80000,

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = RD_88F6510_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = RD_88F6510_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = RD_88F6510_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = rd88f6510TdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** RD-88F6560-GW
****************************************************************************/

#define RD_88F6560_BOARD_NAND_READ_PARAMS	0x000C0282
#define RD_88F6560_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define RD_88F6560_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO rd88f6560InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6560InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x8}
	,
	{BOARD_MAC_SPEED_AUTO, 0x9},
	{N_A,N_A}
};

MV_BOARD_MPP_TYPE_INFO rd88f6560InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH,
	 .ethSataComplexOpt = ESC_OPT_QSGMII | ESC_OPT_GEPHY_MAC1 | ESC_OPT_MAC0_2_SW_P4,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO rd88f6560InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_SDIO_WP, 31},
	{BOARD_GPP_HDD_POWER, 37},
    {BOARD_GPP_PON_XVR_TX, 37},
	{BOARD_GPP_FAN_POWER, 40},
	{BOARD_GPP_WPS_BUTTON, 47},
	{BOARD_GPP_SDIO_DETECT, 48},
	{BOARD_GPP_PON_LED, 49},
	{BOARD_GPP_USB_VBUS, 58},
	{BOARD_GPP_USB_OC, 59},
	{BOARD_GPP_CONF, 60},
	{BOARD_GPP_CONF, 61},
	{BOARD_GPP_CONF, 62},
	{BOARD_GPP_SYS_LED, 68},
	{BOARD_GPP_PEX_RESET, 77}
};

MV_DEV_CS_INFO rd88f6560InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#if defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},	/* SPI DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6560InfoBoardMppConfigValue[] = {
	{{
	  RD_88F6560_MPP0_7,
	  RD_88F6560_MPP8_15,
	  RD_88F6560_MPP16_23,
	  RD_88F6560_MPP24_31,
	  RD_88F6560_MPP32_39,
	  RD_88F6560_MPP40_47,
	  RD_88F6560_MPP48_55,
	  RD_88F6560_MPP56_63,
	  RD_88F6560_MPP64_71,
	  RD_88F6560_MPP72_79,
	  RD_88F6560_MPP80_87,
	  RD_88F6560_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO rd88f6560InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,	/* set to -1 for timer operation */
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, -1, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0,
	 .forceLinkMask = 0x0
	 }
};

MV_U8 rd88f6560LedGppPin[] = { 68 };

MV_BOARD_TDM_INFO rd88f6560Tdm880[] = { {0}, {1} };

MV_BOARD_TDM_SPI_INFO rd88f6560TdmSpiInfo[] = { {1} };

MV_BOARD_SPEC_INIT rd88f6560BoardSpecInit[] = {
	{
		.reg = PMU_POWER_IF_POLARITY_REG,
		.mask = (BIT1),
		.val = 0
	},
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};

MV_BOARD_INFO rd88f6560Info = {
	.boardName = "RD-88F6560-GW",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6560InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6560InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6560InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6560InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6560InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6560InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6560InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6560InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6560InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6560InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6560InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6560InfoBoardGppInfo,
	.activeLedsNumber = MV_ARRAY_SIZE(rd88f6560LedGppPin),
	.pLedGppPin = rd88f6560LedGppPin,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6560_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6560_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = RD_88F6560_GPP_OUT_ENA_HIGH,
	.gppOutValLow = RD_88F6560_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6560_GPP_OUT_VAL_MID,
	.gppOutValHigh = RD_88F6560_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = RD_88F6560_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6560_GPP_POL_MID,
	.gppPolarityValHigh = RD_88F6560_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = rd88f6560InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(rd88f6560InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {2},
	.pBoardTdmInt2CsInfo = { rd88f6560Tdm880 },
	.boardTdmInfoIndex = 0,

	.pBoardSpecInit = rd88f6560BoardSpecInit,

	.deepIdlePwrUpDelay = 2400, /* 12 uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = RD_88F6560_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = RD_88F6560_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = RD_88F6560_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = rd88f6560TdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** RD-88F6530-MDU
****************************************************************************/
#define RD_88F6530_BOARD_NAND_READ_PARAMS	0x000C0282
#define RD_88F6530_BOARD_NAND_WRITE_PARAMS	0x00010305
#define RD_88F6530_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO rd88f6530InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6530InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_1000M, 0x8}
	,			/* This connected to SGMII to xcat ,right now forced SGMII 1G */
	{BOARD_MAC_SPEED_AUTO, 0x9},
	{N_A,N_A}

};

MV_BOARD_MPP_TYPE_INFO rd88f6530InfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_TDM_2CH,
	 .ethSataComplexOpt = ESC_OPT_SGMII | ESC_OPT_GEPHY_MAC1,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO rd88f6530InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
};

MV_DEV_CS_INFO rd88f6530InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#if defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},	/* SPI DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6530InfoBoardMppConfigValue[] = {
	{{
	  RD_88F6530_MPP0_7,
	  RD_88F6530_MPP8_15,
	  RD_88F6530_MPP16_23,
	  RD_88F6530_MPP24_31,
	  RD_88F6530_MPP32_39,
	  RD_88F6530_MPP40_47,
	  RD_88F6530_MPP48_55,
	  RD_88F6530_MPP56_63,
	  RD_88F6530_MPP64_69,
	  RD_88F6530_MPP72_79,
	  RD_88F6530_MPP80_87,
	  RD_88F6530_MPP88_88
	  }
	 }
};

MV_BOARD_TDM_INFO rd88f6530Tdm880[] = { {1}
};

MV_BOARD_TDM_SPI_INFO rd88f6530TdmSpiInfo[] = { {0} };

MV_BOARD_INFO rd88f6530Info = {
	.boardName = "RD-88F6530-MDU",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6530InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6530InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6530InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6530InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6530InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6530InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6530InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6530InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6530InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6530InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6530InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6530InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6530_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6530_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = RD_88F6530_GPP_OUT_ENA_HIGH,
	.gppOutValLow = RD_88F6530_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6530_GPP_OUT_VAL_MID,
	.gppOutValHigh = RD_88F6530_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = RD_88F6530_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6530_GPP_POL_MID,
	.gppPolarityValHigh = RD_88F6530_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {1}
	,
	.pBoardTdmInt2CsInfo = {rd88f6530Tdm880,
				}
	,
	.boardTdmInfoIndex = 0,

	.deepIdlePwrUpDelay = 0x80000,

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = RD_88F6530_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = RD_88F6530_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = RD_88F6530_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = rd88f6530TdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** DB-88F6560-PCAC
****************************************************************************/

#define DB_88F6560_PCAC_BOARD_NAND_READ_PARAMS	0x000C0282
#define DB_88F6560_PCAC_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define DB_88F6560_PCAC_BOARD_NAND_CONTROL	0x01c00543

MV_BOARD_TWSI_INFO db88f6560PcacInfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO db88f6560PcacInfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
	{N_A,N_A}
};

MV_BOARD_MPP_TYPE_INFO db88f6560PcacInfoBoardMppTypeInfo[] = {
	{
	 .boardMppTdm = MV_BOARD_OTHER,
	 .ethSataComplexOpt = ESC_OPT_GEPHY_MAC1,
	 .ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO db88f6560PcacInfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_USB_VBUS, 58}
};

MV_DEV_CS_INFO db88f6560PcacInfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#if defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},	/* SPI DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO db88f6560PcacInfoBoardMppConfigValue[] = {
	{ {
		 DB_88F6560_PCAC_MPP0_7,
		 DB_88F6560_PCAC_MPP8_15,
		 DB_88F6560_PCAC_MPP16_23,
		 DB_88F6560_PCAC_MPP24_31,
		 DB_88F6560_PCAC_MPP32_39,
		 DB_88F6560_PCAC_MPP40_47,
		 DB_88F6560_PCAC_MPP48_55,
		 DB_88F6560_PCAC_MPP56_63,
		 DB_88F6560_PCAC_MPP64_71,
		 DB_88F6560_PCAC_MPP72_79,
		 DB_88F6560_PCAC_MPP80_87,
		 DB_88F6560_PCAC_MPP88_88
	 } }
};

MV_U8 db88f6560PcacLedGppPin[] = { 68, 69, 70, 71};

MV_BOARD_TDM_SPI_INFO db88f6560PcacTdmSpiInfo[] = { {0} };

MV_BOARD_SPEC_INIT db88f6560PcacBoardSpecInit[] = {
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};

MV_BOARD_INFO db88f6560PCACPInfo = {
	.boardName = "DB-88F6560-PCAC",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(db88f6560PcacInfoBoardMppTypeInfo),
	.pBoardMppTypeValue = db88f6560PcacInfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(db88f6560PcacInfoBoardMppConfigValue),
	.pBoardMppConfigValue = db88f6560PcacInfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(db88f6560PcacInfoBoardDeCsInfo),
	.pDevCsInfo = db88f6560PcacInfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(db88f6560PcacInfoBoardTwsiDev),
	.pBoardTwsiDev = db88f6560PcacInfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(db88f6560PcacInfoBoardMacInfo),
	.pBoardMacInfo = db88f6560PcacInfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(db88f6560PcacInfoBoardGppInfo),
	.pBoardGppInfo = db88f6560PcacInfoBoardGppInfo,
	.activeLedsNumber = MV_ARRAY_SIZE(db88f6560PcacLedGppPin),
	.pLedGppPin = db88f6560PcacLedGppPin,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = DB_88F6560_PCAC_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = DB_88F6560_PCAC_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = DB_88F6560_PCAC_GPP_OUT_ENA_HIGH,
	.gppOutValLow = DB_88F6560_PCAC_GPP_OUT_VAL_LOW,
	.gppOutValMid = DB_88F6560_PCAC_GPP_OUT_VAL_MID,
	.gppOutValHigh = DB_88F6560_PCAC_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = DB_88F6560_PCAC_GPP_POL_LOW,
	.gppPolarityValMid = DB_88F6560_PCAC_GPP_POL_MID,
	.gppPolarityValHigh = DB_88F6560_PCAC_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_NONE,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {0},
	.pBoardTdmInt2CsInfo = {NULL},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = db88f6560PcacBoardSpecInit,

	.deepIdlePwrUpDelay = 2400, /* 12 uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = DB_88F6560_PCAC_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = DB_88F6560_PCAC_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = DB_88F6560_PCAC_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = db88f6560PcacTdmSpiInfo,

	/* Disable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** DB-88F6560-BP
****************************************************************************/
#define DB_88F6535_BOARD_NAND_READ_PARAMS	0x000C0282
#define DB_88F6535_BOARD_NAND_WRITE_PARAMS	0x00010305
/* NAND care support for small page chips */
#define DB_88F6535_BOARD_NAND_CONTROL		0x01c00543

MV_BOARD_TWSI_INFO db88f6535InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4D, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4E, ADDR7_BIT},
	{BOARD_DEV_TWSI_SATR, 0x4F, ADDR7_BIT},
	{BOARD_TWSI_MUX, 0x70, ADDR7_BIT}
};

MV_BOARD_MAC_INFO db88f6535InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x8},
	{BOARD_MAC_SPEED_AUTO, 0x9},
	{N_A,N_A}
};

MV_BOARD_MPP_TYPE_INFO db88f6535InfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_AUTO,
		.ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO db88f6535InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PON_XVR_TX, 17},
	{BOARD_GPP_USB_VBUS, 31},
	{BOARD_GPP_PEX_RESET, 67},
	{BOARD_GPP_SDIO_DETECT, 68},
	{BOARD_GPP_SDIO_WP, 69},
};

MV_DEV_CS_INFO db88f6535InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#ifdef MTD_NAND_LNC
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#ifdef MV_NOR
	{NAND_NOR_CS, N_A, BOARD_DEV_NOR_FLASH, 16},	/* NOR DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI) && !defined(MV_NOR)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO db88f6535InfoBoardMppConfigValue[] = {
	{{
	  DB_88F6535_MPP0_7,
	  DB_88F6535_MPP8_15,
	  DB_88F6535_MPP16_23,
	  DB_88F6535_MPP24_31,
	  DB_88F6535_MPP32_39,
	  DB_88F6535_MPP40_47,
	  DB_88F6535_MPP48_55,
	  DB_88F6535_MPP56_63,
	  DB_88F6535_MPP64_71,
	  DB_88F6535_MPP72_79,
	  DB_88F6535_MPP80_87,
	  DB_88F6535_MPP88_88
	  }
	 }
};

MV_BOARD_SWITCH_INFO db88f6535InfoBoardSwitchValue[] = {
	{
	 .switchIrq = 29,	/* set to -1 for timer operation */
	 .switchPort = {1, 2, 3, 0, -1, -1, -1, -1},
	 .cpuPort = 4,
	 .connectedPort = {4, 5, -1},
	 .smiScanMode = 2,
	 .quadPhyAddr = 0,
	 .forceLinkMask = 0x0
	 }
};

MV_BOARD_TDM_INFO db88f6535Tdm880[] = {
	{1},
	{2}
};

MV_BOARD_TDM_INFO db88f6535Tdm890[] = {
	{2}
};

MV_BOARD_TDM_INFO db88f6535Tdm792[] = {
	{1},
	{2},
	{3},
	{4},
	{6},
	{7}
};

MV_BOARD_TDM_INFO db88f6535Tdm3215[] = { {1} };

MV_BOARD_TDM_SPI_INFO db88f6535TdmSpiInfo[] = { {0} };

MV_BOARD_SPEC_INIT db88f6535BoardSpecInit[] = {
	{
		.reg = PMU_POWER_IF_POLARITY_REG,
		.mask = (BIT1),
		.val = 0
	},
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};

MV_BOARD_INFO db88f6535Info = {
	.boardName = "DB-88F6560-BP",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(db88f6535InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = db88f6535InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(db88f6535InfoBoardMppConfigValue),
	.pBoardMppConfigValue = db88f6535InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(db88f6535InfoBoardDeCsInfo),
	.pDevCsInfo = db88f6535InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(db88f6535InfoBoardTwsiDev),
	.pBoardTwsiDev = db88f6535InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(db88f6535InfoBoardMacInfo),
	.pBoardMacInfo = db88f6535InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(db88f6535InfoBoardGppInfo),
	.pBoardGppInfo = db88f6535InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = DB_88F6535_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = DB_88F6535_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = DB_88F6535_GPP_OUT_ENA_HIGH,
	.gppOutValLow = DB_88F6535_GPP_OUT_VAL_LOW,
	.gppOutValMid = DB_88F6535_GPP_OUT_VAL_MID,
	.gppOutValHigh = DB_88F6535_GPP_OUT_VAL_HIGH,
	.gppPolarityValLow = DB_88F6535_GPP_POL_LOW,
	.gppPolarityValMid = DB_88F6535_GPP_POL_MID,
	.gppPolarityValHigh = DB_88F6535_GPP_POL_HIGH,

	/* External Switch Configuration */
	.pSwitchInfo = db88f6535InfoBoardSwitchValue,
	.switchInfoNum = MV_ARRAY_SIZE(db88f6535InfoBoardSwitchValue),

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_AUTO,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {2, 6, 1, 1},
	.pBoardTdmInt2CsInfo = {db88f6535Tdm880,
				db88f6535Tdm792,
				db88f6535Tdm3215,
				db88f6535Tdm890},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = db88f6535BoardSpecInit,

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_TRUE,
	.nandFlashReadParams = DB_88F6535_BOARD_NAND_READ_PARAMS,
	.nandFlashWriteParams = DB_88F6535_BOARD_NAND_WRITE_PARAMS,
	.nandFlashControl = DB_88F6535_BOARD_NAND_CONTROL,
	.pBoardTdmSpiInfo = db88f6535TdmSpiInfo,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_TRUE
};

/***************************************************************************
** RD-88F6601 Avanta
****************************************************************************/
/* NAND not supported  */

MV_BOARD_TWSI_INFO rd88f6601InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6601InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x0},
	{BOARD_MAC_SPEED_AUTO, 0x1},
	{N_A,N_A}
};

MV_BOARD_MPP_TYPE_INFO rd88f6601InfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_GEPHY_MAC0,
		.ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO rd88f6601InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PON_XVR_TX, 17},
	{BOARD_GPP_PON_XVR_TX_POWER, 37},
	{BOARD_GPP_PON_XVR_TX_IND, 24},
};

MV_DEV_CS_INFO rd88f6601InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#if !defined(MV_SPI) 
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6601InfoBoardMppConfigValue[] = {
	{{
	  RD_88F6601_MPP0_7,
	  RD_88F6601_MPP8_15,
	  RD_88F6601_MPP16_23,
	  RD_88F6601_MPP24_31,
	  RD_88F6601_MPP32_37
	  }
	 }
};

MV_BOARD_INFO rd88f6601Info = {
	.boardName = "RD-88F6601-BP",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6601InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6601InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6601InfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6601InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6601InfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6601InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6601InfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6601InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6601InfoBoardMacInfo),
	.pBoardMacInfo = rd88f6601InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6601InfoBoardGppInfo),
	.pBoardGppInfo = rd88f6601InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = RD_88F6601_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6601_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = 0,
	.gppOutValLow = RD_88F6601_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6601_GPP_OUT_VAL_MID,
	.gppOutValHigh = 0,
	.gppPolarityValLow = RD_88F6601_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6601_GPP_POL_MID,
	.gppPolarityValHigh = 0,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {0},
	.pBoardTdmInt2CsInfo = {NULL},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = NULL,

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_FALSE,
	.nandFlashReadParams = 0,
	.nandFlashWriteParams = 0,
	.nandFlashControl = 0,
	.pBoardTdmSpiInfo = NULL,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** RD-88F6601MC2L Avanta
****************************************************************************/
/* NAND not supported  */

MV_BOARD_TWSI_INFO rd88f6601MC2LInfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO rd88f6601MC2LInfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x0},
	{N_A, N_A}
};

MV_BOARD_MPP_TYPE_INFO rd88f6601MC2LInfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_GEPHY_MAC0,
		.ethPortsMode = 0x0,
	}
};

MV_BOARD_GPP_INFO rd88f6601MC2LInfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PON_XVR_TX, 17},
	{BOARD_GPP_PON_XVR_TX_POWER, 37},
	{BOARD_GPP_PON_XVR_TX_IND, 24},	
};

MV_DEV_CS_INFO rd88f6601MC2LInfoBoardDeCsInfo[] = {
	/* {deviceCS, params, devType, devWidth} */
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},	/* SPI DEV */
#endif
#if !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO rd88f6601MC2LInfoBoardMppConfigValue[] = {
	{
	{
	  RD_88F6601MC2L_MPP0_7,
	  RD_88F6601MC2L_MPP8_15,
	  RD_88F6601MC2L_MPP16_23,
	  RD_88F6601MC2L_MPP24_31,
	  RD_88F6601MC2L_MPP32_37,
	}
	}
};

MV_BOARD_INFO rd88f6601MC2LInfo = {
	.boardName = "RD-88F6601-MC2L",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(rd88f6601MC2LInfoBoardMppTypeInfo),
	.pBoardMppTypeValue = rd88f6601MC2LInfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(rd88f6601MC2LInfoBoardMppConfigValue),
	.pBoardMppConfigValue = rd88f6601MC2LInfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(rd88f6601MC2LInfoBoardDeCsInfo),
	.pDevCsInfo = rd88f6601MC2LInfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(rd88f6601MC2LInfoBoardTwsiDev),
	.pBoardTwsiDev = rd88f6601MC2LInfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(rd88f6601MC2LInfoBoardMacInfo),
	.pBoardMacInfo = rd88f6601MC2LInfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(rd88f6601MC2LInfoBoardGppInfo),
	.pBoardGppInfo = rd88f6601MC2LInfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,
 
	/* GPP values */
	.gppOutEnValLow = RD_88F6601MC2L_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = RD_88F6601MC2L_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = 0,
	.gppOutValLow = RD_88F6601MC2L_GPP_OUT_VAL_LOW,
	.gppOutValMid = RD_88F6601MC2L_GPP_OUT_VAL_MID,
	.gppOutValHigh = 0,
	.gppPolarityValLow = RD_88F6601MC2L_GPP_POL_LOW,
	.gppPolarityValMid = RD_88F6601MC2L_GPP_POL_MID,
	.gppPolarityValHigh = 0,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {0},
	.pBoardTdmInt2CsInfo = {NULL},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = NULL,

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_FALSE,
	.nandFlashReadParams = 0,
	.nandFlashWriteParams = 0,
	.nandFlashControl = 0,
	.pBoardTdmSpiInfo = NULL,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** DB-88F6601-BP Avanta
****************************************************************************/
/* NAND not supported  */

MV_BOARD_TWSI_INFO db88f6601InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
	{BOARD_DEV_TWSI_SATR, 0x4C, ADDR7_BIT},
	{BOARD_TWSI_MUX, 0x70, ADDR7_BIT}
};

MV_BOARD_MAC_INFO db88f6601InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x0},
	{BOARD_MAC_SPEED_AUTO, 0x1},
	{N_A, N_A}
};

MV_BOARD_MPP_TYPE_INFO db88f6601InfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_AUTO,
		.ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO db88f6601InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PON_XVR_TX, 17},
	{BOARD_GPP_PON_XVR_TX_POWER, 37},
	{BOARD_GPP_PON_XVR_TX_IND, 19},

};

MV_DEV_CS_INFO db88f6601InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#if !defined(MV_SPI) 
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO db88f6601InfoBoardMppConfigValue[] = {
	{{
	  DB_88F6601_MPP0_7,
	  DB_88F6601_MPP8_15,
	  DB_88F6601_MPP16_23,
	  DB_88F6601_MPP24_31,
	  DB_88F6601_MPP32_37
	  }
	 }
};


MV_BOARD_TDM_INFO db88f6601Tdm880[] = { {0} };

MV_BOARD_TDM_SPI_INFO db88f6601TdmSpiInfo[] = { {1} };

MV_BOARD_INFO db88f6601Info = {
	.boardName = "DB-88F6601-BP",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(db88f6601InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = db88f6601InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(db88f6601InfoBoardMppConfigValue),
	.pBoardMppConfigValue = db88f6601InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(db88f6601InfoBoardDeCsInfo),
	.pDevCsInfo = db88f6601InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(db88f6601InfoBoardTwsiDev),
	.pBoardTwsiDev = db88f6601InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(db88f6601InfoBoardMacInfo),
	.pBoardMacInfo = db88f6601InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(db88f6601InfoBoardGppInfo),
	.pBoardGppInfo = db88f6601InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = DB_88F6601_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = DB_88F6601_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = 0,	
	.gppOutValLow = DB_88F6601_GPP_OUT_VAL_LOW,
	.gppOutValMid = DB_88F6601_GPP_OUT_VAL_MID,
	.gppOutValHigh = 0,
	.gppPolarityValLow = DB_88F6601_GPP_POL_LOW,
	.gppPolarityValMid = DB_88F6601_GPP_POL_MID,
	.gppPolarityValHigh = 0,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {1},
	.pBoardTdmInt2CsInfo = {db88f6601Tdm880},
	.boardTdmInfoIndex = 0,

	.pBoardSpecInit = NULL,

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_FALSE,
	.nandFlashReadParams = 0,
	.nandFlashWriteParams = 0,
	.nandFlashControl = 0,
	.pBoardTdmSpiInfo = db88f6601TdmSpiInfo,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_TRUE
};

/***************************************************************************
** Customer board place holder
****************************************************************************/

#define DB_CUSTOMER_BOARD_PCI_IF_NUM		        0x0
#define DB_CUSTOMER_BOARD_TWSI_DEF_NUM		        0x0
#define DB_CUSTOMER_BOARD_MAC_INFO_NUM		        0x0
#define DB_CUSTOMER_BOARD_GPP_INFO_NUM		        0x0
#define DB_CUSTOMER_BOARD_MPP_GROUP_TYPE_NUN        0x0
#define DB_CUSTOMER_BOARD_MPP_CONFIG_NUM		    0x0
#define DB_CUSTOMER_BOARD_DEVICE_CONFIG_NUM	    	0x0
#define DB_CUSTOMER_BOARD_DEBUG_LED_NUM				0x0
#define DB_CUSTOMER_BOARD_NAND_READ_PARAMS		    0x000E02C2
#define DB_CUSTOMER_BOARD_NAND_WRITE_PARAMS		    0x00010305
#define DB_CUSTOMER_BOARD_NAND_CONTROL		        0x01c00543

MV_U8 dbCustomerInfoBoardDebugLedIf[] = { 0 };

MV_BOARD_MAC_INFO dbCustomerInfoBoardMacInfo[] =
    /* {{MV_BOARD_MAC_SPEED     boardMacSpeed,  MV_U8   boardEthSmiAddr}} */
{ {BOARD_MAC_SPEED_AUTO, 0x0}
};

MV_BOARD_TWSI_INFO dbCustomerInfoBoardTwsiDev[] =
    /* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
{ {BOARD_TWSI_OTHER, 0x0, ADDR7_BIT}
};

MV_BOARD_MPP_TYPE_INFO dbCustomerInfoBoardMppTypeInfo[] = { {MV_BOARD_OTHER, MV_BOARD_OTHER, 0x0}
};

MV_DEV_CS_INFO dbCustomerInfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#if defined(MTD_NAND_LNC)
	{NAND_NOR_CS, N_A, BOARD_DEV_NAND_FLASH, 8},	/* NAND DEV */
#endif
#if defined(MV_SPI)
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},	/* SPI DEV */
#endif
#if !defined(MTD_NAND_LNC) && !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO dbCustomerInfoBoardMppConfigValue[] = {
	{
		{
			DB_CUSTOMER_MPP0_7,
			DB_CUSTOMER_MPP8_15,
			DB_CUSTOMER_MPP16_23,
			DB_CUSTOMER_MPP24_31,
			DB_CUSTOMER_MPP32_39,
			DB_CUSTOMER_MPP40_47,
			DB_CUSTOMER_MPP48_55
		}
	}
};

MV_BOARD_INFO dbCustomerInfo = { };

/***************************************************************************
** GFLT200 Prism
****************************************************************************/
/* NAND not supported  */

MV_BOARD_TWSI_INFO gflt200InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO gflt200InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x0},
	{BOARD_MAC_SPEED_AUTO, 0x1},
	{N_A,N_A}
};

MV_BOARD_MPP_TYPE_INFO gflt200InfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_GEPHY_MAC0,
		.ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO gflt200InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	/*{BOARD_GPP_PON_XVR_TX, 17, 0},*/
	/*{BOARD_GPP_PON_XVR_TX_IND, 24, 0},*/
	{BOARD_GPP_PON_XVR_TX, 21, 1},
	{BOARD_GPP_PON_XVR_TX_POWER, 37, 1},
};

MV_DEV_CS_INFO gflt200InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#if !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO gflt200Evt1InfoBoardMppConfigValue[] = {
	{{
	  GFLT200_EVT1_MPP0_7,
	  GFLT200_EVT1_MPP8_15,
	  GFLT200_EVT1_MPP16_23,
	  GFLT200_EVT1_MPP24_31,
	  GFLT200_EVT1_MPP32_37
	  }
	 }
};

MV_BOARD_MPP_INFO gflt200Evt2InfoBoardMppConfigValue[] = {
	{{
	  GFLT200_EVT2_MPP0_7,
	  GFLT200_EVT2_MPP8_15,
	  GFLT200_EVT2_MPP16_23,
	  GFLT200_EVT2_MPP24_31,
	  GFLT200_EVT2_MPP32_37
	  }
	 }
};

/*
MV_BOARD_SPEC_INIT gflt200BoardSpecInit[] = {
	{
		.reg = PMU_POWER_IF_POLARITY_REG,
		.mask = (BIT1),
		.val = 0
	},
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};
*/

#define GFLT200_GPP_BOARD_VER_MASK	((1 << 18) | (1 << 15) | (1 << 13))
#define GFLT200_EVT1_BOARD_VER		(0)
#define GFLT200_EVT2_BOARD_VER		(1 << 13)

static MV_VOID gflt200BoardInit(MV_BOARD_INFO *pBoardInfo)
{
	MV_U32 board_ver;

	mvGppTypeSet(0, GFLT200_GPP_BOARD_VER_MASK, GFLT200_GPP_BOARD_VER_MASK);

	switch ((board_ver = mvGppValueGet(0, GFLT200_GPP_BOARD_VER_MASK))) {
	case GFLT200_EVT1_BOARD_VER:
		pBoardInfo->numBoardMppConfigValue
			= MV_ARRAY_SIZE(gflt200Evt1InfoBoardMppConfigValue);
		pBoardInfo->pBoardMppConfigValue
			= gflt200Evt1InfoBoardMppConfigValue;
		pBoardInfo->gppOutEnValLow = GFLT200_EVT1_GPP_OUT_ENA_LOW;
		pBoardInfo->gppOutEnValMid = GFLT200_EVT1_GPP_OUT_ENA_MID;
		pBoardInfo->gppOutValLow = GFLT200_EVT1_GPP_OUT_VAL_LOW;
		pBoardInfo->gppOutValMid = GFLT200_EVT1_GPP_OUT_VAL_MID;
		pBoardInfo->gppPolarityValLow = GFLT200_EVT1_GPP_POL_LOW;
		pBoardInfo->gppPolarityValMid = GFLT200_EVT1_GPP_POL_MID;
		break;

	default:
		pr_err("GFLT200: unknown board version '%x'\n", board_ver);
		/* fallthrough */
	case GFLT200_EVT2_BOARD_VER:
		pBoardInfo->numBoardMppConfigValue
			= MV_ARRAY_SIZE(gflt200Evt2InfoBoardMppConfigValue);
		pBoardInfo->pBoardMppConfigValue
			= gflt200Evt2InfoBoardMppConfigValue;
		pBoardInfo->gppOutEnValLow = GFLT200_EVT2_GPP_OUT_ENA_LOW;
		pBoardInfo->gppOutEnValMid = GFLT200_EVT2_GPP_OUT_ENA_MID;
		pBoardInfo->gppOutValLow = GFLT200_EVT2_GPP_OUT_VAL_LOW;
		pBoardInfo->gppOutValMid = GFLT200_EVT2_GPP_OUT_VAL_MID;
		pBoardInfo->gppPolarityValLow = GFLT200_EVT2_GPP_POL_LOW;
		pBoardInfo->gppPolarityValMid = GFLT200_EVT2_GPP_POL_MID;
		break;
	}
}

MV_BOARD_INFO gflt200Info = {
	.boardName = "GFLT200",
	.pBoardInit = gflt200BoardInit,
	.numBoardMppTypeValue = MV_ARRAY_SIZE(gflt200InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = gflt200InfoBoardMppTypeInfo,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(gflt200InfoBoardDeCsInfo),
	.pDevCsInfo = gflt200InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(gflt200InfoBoardTwsiDev),
	.pBoardTwsiDev = gflt200InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(gflt200InfoBoardMacInfo),
	.pBoardMacInfo = gflt200InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(gflt200InfoBoardGppInfo),
	.pBoardGppInfo = gflt200InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValHigh = 0,
	.gppOutValHigh = 0,
	.gppPolarityValHigh = 0,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_PON_NONE,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {0},
	.pBoardTdmInt2CsInfo = {NULL},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = NULL,			/* gflt200BoardSpecInit, */

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_FALSE,
	.nandFlashReadParams = 0,
	.nandFlashWriteParams = 0,
	.nandFlashControl = 0,
	.pBoardTdmSpiInfo = NULL,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

/***************************************************************************
** GFLT110
****************************************************************************/
/* NAND not supported  */

MV_BOARD_TWSI_INFO gflt110InfoBoardTwsiDev[] = {
	/* {{MV_BOARD_DEV_CLASS devClass, MV_U8 twsiDevAddr, MV_U8 twsiDevAddrType}} */
};

MV_BOARD_MAC_INFO gflt110InfoBoardMacInfo[] = {
	/* {{MV_BOARD_MAC_SPEED boardMacSpeed, MV_U8 boardEthSmiAddr}} */
	{BOARD_MAC_SPEED_AUTO, 0x0},
	{BOARD_MAC_SPEED_AUTO, 0x1},
	{N_A,N_A}
};

MV_BOARD_MPP_TYPE_INFO gflt110InfoBoardMppTypeInfo[] = {
	{
		.boardMppTdm = MV_BOARD_AUTO,
		.ethSataComplexOpt = ESC_OPT_GEPHY_MAC0,
		.ethPortsMode = 0x0
	}
};

MV_BOARD_GPP_INFO gflt110InfoBoardGppInfo[] = {
	/* {{MV_BOARD_GPP_CLASS devClass, MV_U8 gppPinNum}} */
	{BOARD_GPP_PON_XVR_TX, 17},
	{BOARD_GPP_PON_XVR_TX_POWER, 37},
	{BOARD_GPP_PON_XVR_TX_IND, 24},
};

MV_DEV_CS_INFO gflt110InfoBoardDeCsInfo[] = {
	/*{deviceCS, params, devType, devWidth} */
#ifdef MV_SPI
	{SPI_CS0, N_A, BOARD_DEV_SPI_FLASH, 8},		/* SPI DEV */
#endif
#if !defined(MV_SPI)
	{N_A, N_A, N_A, N_A}			/* No device */
#endif
};

MV_BOARD_MPP_INFO gflt110InfoBoardMppConfigValue[] = {
	{{
	  GFLT110_MPP0_7,
	  GFLT110_MPP8_15,
	  GFLT110_MPP16_23,
	  GFLT110_MPP24_31,
	  GFLT110_MPP32_37
	  }
	 }
};

/*
MV_BOARD_SPEC_INIT gflt110BoardSpecInit[] = {
	{
		.reg = PMU_POWER_IF_POLARITY_REG,
		.mask = (BIT1),
		.val = 0
	},
	{
		.reg = TBL_TERM,
		.val = TBL_TERM
	}
};
*/
MV_BOARD_INFO gflt110Info = {
	.boardName = "GFLT110",
	.numBoardMppTypeValue = MV_ARRAY_SIZE(gflt110InfoBoardMppTypeInfo),
	.pBoardMppTypeValue = gflt110InfoBoardMppTypeInfo,
	.numBoardMppConfigValue = MV_ARRAY_SIZE(gflt110InfoBoardMppConfigValue),
	.pBoardMppConfigValue = gflt110InfoBoardMppConfigValue,
	.intsGppMaskLow = 0,
	.intsGppMaskMid = 0,
	.intsGppMaskHigh = 0,
	.numBoardDeviceIf = MV_ARRAY_SIZE(gflt110InfoBoardDeCsInfo),
	.pDevCsInfo = gflt110InfoBoardDeCsInfo,
	.numBoardTwsiDev = MV_ARRAY_SIZE(gflt110InfoBoardTwsiDev),
	.pBoardTwsiDev = gflt110InfoBoardTwsiDev,
	.numBoardMacInfo = MV_ARRAY_SIZE(gflt110InfoBoardMacInfo),
	.pBoardMacInfo = gflt110InfoBoardMacInfo,
	.numBoardGppInfo = MV_ARRAY_SIZE(gflt110InfoBoardGppInfo),
	.pBoardGppInfo = gflt110InfoBoardGppInfo,
	.activeLedsNumber = 0,
	.pLedGppPin = NULL,
	.ledsPolarity = 0,

	/* GPP values */
	.gppOutEnValLow = GFLT110_GPP_OUT_ENA_LOW,
	.gppOutEnValMid = GFLT110_GPP_OUT_ENA_MID,
	.gppOutEnValHigh = 0,
	.gppOutValLow = GFLT110_GPP_OUT_VAL_LOW,
	.gppOutValMid = GFLT110_GPP_OUT_VAL_MID,
	.gppOutValHigh = 0,
	.gppPolarityValLow = GFLT110_GPP_POL_LOW,
	.gppPolarityValMid = GFLT110_GPP_POL_MID,
	.gppPolarityValHigh = 0,

	/* External Switch Configuration */
	.pSwitchInfo = NULL,
	.switchInfoNum = 0,

	/* PON configuration. */
	.ponConfigValue = BOARD_GPON_CONFIG,

	/* TDM configuration */
	/* We hold a different configuration array for each possible slic that
	 ** can be connected to board.
	 ** When modules are scanned, then we select the index of the relevant
	 ** slic's information array.
	 ** For RD and Customers boards we only need to initialize a single
	 ** entry of the arrays below, and set the boardTdmInfoIndex to 0.
	 */
	.numBoardTdmInfo = {0},
	.pBoardTdmInt2CsInfo = {NULL},
	.boardTdmInfoIndex = -1,

	.pBoardSpecInit = NULL,			/* gflt110BoardSpecInit, */

	.deepIdlePwrUpDelay = 2400,	/* 12uS */

	/* NAND init params */
	.nandFlashParamsValid = MV_FALSE,
	.nandFlashReadParams = 0,
	.nandFlashWriteParams = 0,
	.nandFlashControl = 0,
	.pBoardTdmSpiInfo = NULL,

	/* Enable modules auto-detection. */
	.moduleAutoDetect = MV_FALSE
};

MV_BOARD_INFO *boardInfoTbl[] = {
	&db88f6535Info,
	&rd88f6510Info,
	&rd88f6560Info,
	&rd88f6530Info,
	&db88f6560PCACPInfo,
	&db88f6601Info,
	&rd88f6601Info,
	&gflt200Info,
	&gflt110Info,
	&rd88f6601MC2LInfo,
};
