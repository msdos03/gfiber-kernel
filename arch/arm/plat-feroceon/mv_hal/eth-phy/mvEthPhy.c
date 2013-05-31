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

*******************************************************************************/

#include "mvCommon.h"
#include "mvOs.h"
#include "ctrlEnv/mvCtrlEnvSpec.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "mvSysEthPhyConfig.h"
#include "mvEthPhyRegs.h"
#include "mvEthPhy.h"
#include "gbe/mvNeta.h"
#include "boardEnv/mvBoardEnvLib.h"

static 	MV_VOID	mvEthPhyPower(MV_U32 ethPortNum, MV_BOOL enable);

static MV_ETHPHY_HAL_DATA ethphyHalData;


/*******************************************************************************
* mvEthPhyRegWriteExtSmi -
* mvEthPhyRegReadExtSmi -
*
* DESCRIPTION:
*       Wrapper functions for accessing PHY registers internally, in case the
*	external PHY access functions are not provided (In HAL Data).
*	This is used for initializing the Quad PHY through a switch SMI bus
*	and not the MAC SMI.
*
* INPUT:
*	phyAddr		- The PHY address to be accessed.
*	regAddr		- The register address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
*******************************************************************************/
static MV_STATUS mvEthPhyRegWriteExtSmi(MV_U32 phyAddr, MV_U32 regAddr, MV_U16 val)
{
	if (ethphyHalData.mvExtPhyWriteFunc != NULL)
		return ethphyHalData.mvExtPhyWriteFunc(phyAddr, regAddr, val);
	else
		return mvEthPhyRegWrite(phyAddr, regAddr, val);
}

static MV_STATUS mvEthPhyRegReadExtSmi(MV_U32 phyAddr, MV_U32 regAddr, MV_U16 *data)
{
	if (ethphyHalData.mvExtPhyReadFunc != NULL)
		return ethphyHalData.mvExtPhyReadFunc(phyAddr, regAddr, data);
	else
		return mvEthPhyRegRead(phyAddr, regAddr, data);
}


/*******************************************************************************
* mvEthPhyHalInit -
*
* DESCRIPTION:
*       Initialize the ethernet phy unit HAL.
*
* INPUT:
*       halData	- Ethernet PHY HAL data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
*******************************************************************************/
MV_STATUS mvEthPhyHalInit(MV_ETHPHY_HAL_DATA *halData)
{
	mvOsMemcpy(&ethphyHalData, halData, sizeof(MV_ETHPHY_HAL_DATA));

	return MV_OK;
}


/*******************************************************************************
* mvEthPhygetId -
*
* DESCRIPTION:
*       Get Phy ID.
*
* INPUT:
*       ethPortNum - The port number to get the PHY ID for.
*
* OUTPUT:
*       devId - The PHY ID.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
*******************************************************************************/
static MV_STATUS mvEthPhyGetId(MV_U32 ethPortNum, MV_U16 *devId)
{
	MV_U32     phyAddr;
	MV_U16     id1, id2;

	if (ethPortNum != ((MV_U32) -1)) {
		phyAddr = ethphyHalData.phyAddr[ethPortNum];
		/* Set page as 0 */
		if (mvEthPhyRegWrite(phyAddr, 22, 0) != MV_OK)
			return MV_ERROR;

		/* Reads ID1 */
		if (mvEthPhyRegRead(phyAddr, 2, &id1) != MV_OK)
			return MV_ERROR;

		/* Reads ID2 */
		if (mvEthPhyRegRead(phyAddr, 3, &id2) != MV_OK)
			return MV_ERROR;
	} else {
		phyAddr = ethphyHalData.quadPhyStartAddr;
		/* Set page as 0 */
		if (mvEthPhyRegWriteExtSmi(phyAddr, 22, 0) != MV_OK)
			return MV_ERROR;

		/* Reads ID1 */
		if (mvEthPhyRegReadExtSmi(phyAddr, 2, &id1) != MV_OK)
			return MV_ERROR;

		/* Reads ID2 */
		if (mvEthPhyRegReadExtSmi(phyAddr, 3, &id2) != MV_OK)
			return MV_ERROR;
	}

	if (!MV_IS_MARVELL_OUI(id1, id2)) {
		mvOsPrintf("Cannot find Marvell Device id1 %x id2 %x\n", id1, id2);
		return MV_ERROR;
	}

	*devId = (id2 & 0x3F0) >> 4;
	return MV_OK;
}

/*******************************************************************************
* mvEthPhyInit -
*
* DESCRIPTION:
*       Initialize the ethernet phy unit.
*
* INPUT:
*       ethPortNum - The port number on which to initialize the PHY.
*	eeeEnable  - Whether to enable EEE or not.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK on success, MV_ERROR otherwise.
*
*******************************************************************************/
MV_STATUS mvEthPhyInit(MV_U32 ethPortNum, MV_BOOL eeeEnable)
{
	MV_U32     phyAddr;
	MV_U16     deviceId;

	if (mvEthPhyGetId(ethPortNum, &deviceId) != MV_OK) {
		mvOsPrintf("Cannot Get Marvell PHY Device ID.\n");
		return MV_ERROR;
	}

	if (ethPortNum != ((MV_U32) -1))
		phyAddr = ethphyHalData.phyAddr[ethPortNum];
	else
		phyAddr = ethphyHalData.quadPhyStartAddr;

	switch (deviceId) {
	case MV_PHY_88E1116:
	case MV_PHY_88E1116R:
		mvEthE1116PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E3061:
		mvEthE3016PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E1111:
		mvEthE1111PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E1112:
		mvEthE1112PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E1011:
		mvEthE1011PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E1121:
		mvEth1121PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E114X:
		mvEth1145PhyBasicInit(phyAddr);
		break;
	case MV_PHY_88F6601_INTERNAL:
	case MV_PHY_88E154X:
	case MV_PHY_88E154X_Z1:
		/* case MV_PHY_KW2_INTERNAL_GE: */
		if (ethPortNum != ((MV_U32) -1))
			mvEthInternalGEPhyBasicInit(phyAddr, eeeEnable);
		else
			mvEth1540PhyBasicInit(eeeEnable);
		break;
	case MV_PHY_88E1340S:
	case MV_PHY_88E1340:
	case MV_PHY_88E1340M:
		mvEth1340PhyBasicInit();
		break;
	case MV_PHY_88E1310:
		mvEth131xPhyBasicInit(phyAddr);
		break;
	case MV_PHY_88E1512:
		mvEthE1512PhyBasicInit(ethPortNum, eeeEnable);
		break;

	case MV_PHY_88E104X:
	case MV_PHY_88E10X0:
	case MV_PHY_88E10X0S:
	case MV_PHY_88E3082:
	case MV_PHY_88E1149:
	case MV_PHY_88E1181:
	case MV_PHY_88E3016_88E3019:
	case MV_PHY_88E1240:
	case MV_PHY_88E1149R:
	case MV_PHY_88E1119R:
	default:
			mvOsPrintf("Unknown Device(%#x). Initialization failed\n", deviceId);
			return MV_ERROR;
	}
	return MV_OK;
}

void    rdPhy(MV_U32 phyAddr, MV_U32 regOffs)
{
	MV_U16      data;
	MV_STATUS   status;

	status = mvEthPhyRegRead(phyAddr, regOffs, &data);
	if (status == MV_OK)
		mvOsPrintf("reg=%d: 0x%04x\n", regOffs, data);
	else
		mvOsPrintf("Read failed\n");
}


/*******************************************************************************
* mvEthPhyRegRead - Read from ethernet phy register.
*
* DESCRIPTION:
*       This function reads ethernet phy register.
*
* INPUT:
*       phyAddr - Phy address.
*       regOffs - Phy register offset.
*
* OUTPUT:
*       None.
*
* RETURN:
*       16bit phy register value, or 0xffff on error
*
*******************************************************************************/
MV_STATUS mvEthPhyRegRead(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 *data)
{
	MV_U32 			smiReg;
	volatile MV_U32 timeout;

	/* check parameters */
	if ((phyAddr << ETH_PHY_SMI_DEV_ADDR_OFFS) & ~ETH_PHY_SMI_DEV_ADDR_MASK) {
		mvOsPrintf("mvEthPhyRegRead: Err. Illegal PHY device address %d\n",
				phyAddr);
		return MV_FAIL;
	}
	if ((regOffs <<  ETH_PHY_SMI_REG_ADDR_OFFS) & ~ETH_PHY_SMI_REG_ADDR_MASK) {
		mvOsPrintf("mvEthPhyRegRead: Err. Illegal PHY register offset %d\n",
				regOffs);
		return MV_FAIL;
	}

	timeout = ETH_PHY_TIMEOUT;
	/* wait till the SMI is not busy*/
	do {
		/* read smi register */
		smiReg = MV_REG_READ(ethphyHalData.ethPhySmiReg);
		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegRead: SMI busy timeout\n");
			return MV_FAIL;
		}
	} while (smiReg & ETH_PHY_SMI_BUSY_MASK);

	/* fill the phy address and regiser offset and read opcode */
	smiReg = (phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) | (regOffs << ETH_PHY_SMI_REG_ADDR_OFFS)|
			   ETH_PHY_SMI_OPCODE_READ;

	/* write the smi register */
	MV_REG_WRITE(ethphyHalData.ethPhySmiReg, smiReg);

	timeout = ETH_PHY_TIMEOUT;

	/* wait till readed value is ready */
	do {
		/* read smi register */
		smiReg = MV_REG_READ(ethphyHalData.ethPhySmiReg);

		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegRead: SMI read-valid timeout\n");
			return MV_FAIL;
		}
	} while (!(smiReg & ETH_PHY_SMI_READ_VALID_MASK));

	/* Wait for the data to update in the SMI register */
	for (timeout = 0; timeout < ETH_PHY_TIMEOUT; timeout++)
		;

	*data = (MV_U16)(MV_REG_READ(ethphyHalData.ethPhySmiReg) & ETH_PHY_SMI_DATA_MASK);

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyRegWrite - Write to ethernet phy register.
*
* DESCRIPTION:
*       This function write to ethernet phy register.
*
* INPUT:
*       phyAddr - Phy address.
*       regOffs - Phy register offset.
*       data    - 16bit data.
*
* OUTPUT:
*       None.
*
* RETURN:
*       MV_OK if write succeed, MV_BAD_PARAM on bad parameters , MV_ERROR on error .
*		MV_TIMEOUT on timeout
*
*******************************************************************************/
MV_STATUS mvEthPhyRegWrite(MV_U32 phyAddr, MV_U32 regOffs, MV_U16 data)
{
	MV_U32 			smiReg;
	volatile MV_U32 timeout;

	/* check parameters */
	if ((phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) & ~ETH_PHY_SMI_DEV_ADDR_MASK) {
		mvOsPrintf("mvEthPhyRegWrite: Err. Illegal phy address \n");
		return MV_BAD_PARAM;
	}
	if ((regOffs <<  ETH_PHY_SMI_REG_ADDR_OFFS) & ~ETH_PHY_SMI_REG_ADDR_MASK) {
		mvOsPrintf("mvEthPhyRegWrite: Err. Illegal register offset \n");
		return MV_BAD_PARAM;
	}

	timeout = ETH_PHY_TIMEOUT;

	/* wait till the SMI is not busy*/
	do {
		/* read smi register */
		smiReg = MV_REG_READ(ethphyHalData.ethPhySmiReg);
		if (timeout-- == 0) {
			mvOsPrintf("mvEthPhyRegWrite: SMI busy timeout\n");
		return MV_TIMEOUT;
		}
	} while (smiReg & ETH_PHY_SMI_BUSY_MASK);

	/* fill the phy address and regiser offset and write opcode and data*/
	smiReg = (data << ETH_PHY_SMI_DATA_OFFS);
	smiReg |= (phyAddr <<  ETH_PHY_SMI_DEV_ADDR_OFFS) | (regOffs << ETH_PHY_SMI_REG_ADDR_OFFS);
	smiReg &= ~ETH_PHY_SMI_OPCODE_READ;

	/* write the smi register */
	MV_REG_WRITE(ethphyHalData.ethPhySmiReg, smiReg);

	return MV_OK;
}


/*******************************************************************************
* mvEthPhyReset - Reset ethernet Phy.
*
* DESCRIPTION:
*       This function resets a given ethernet Phy.
*
* INPUT:
*       phyAddr - Phy address.
*	data    - PHY control register value want to be, if 0xFFFF, use original value
*       timeout - in millisec
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK       - Success
*           MV_TIMEOUT  - Timeout
*
*******************************************************************************/
MV_STATUS mvEthPhyReset(MV_U32 phyAddr, MV_U16 data, int timeout)
{
	MV_U16  phyRegData, temp;
	MV_BOOL power_down = 0;

	/* Reset the PHY */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &temp) != MV_OK)
		return MV_FAIL;

	if (temp & ETH_PHY_CTRL_POWER_DOWN_MASK)
		power_down = 1;

	if (data != 0xFFFF)
		temp = data;

	if (power_down)
		temp |= ETH_PHY_CTRL_POWER_DOWN_MASK;
	else
		temp |= ETH_PHY_CTRL_RESET_MASK;

	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, temp);

	if (power_down)
		return MV_OK;

	/* Wait untill Reset completed */
	while (timeout > 0) {
		mvOsSleep(100);
		timeout -= 100;

		if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
			return MV_FAIL;

		if ((phyRegData & ETH_PHY_CTRL_RESET_MASK) == 0)
			return MV_OK;
	}
	return MV_TIMEOUT;
}


/*******************************************************************************
* mvEthPhyRestartAN - Restart ethernet Phy Auto-Negotiation.
*
* DESCRIPTION:
*       This function resets a given ethernet Phy.
*	If AutoNegotiation is not enabled, it'll enable it.
*	Loopback and Power Down will be disabled by this routine
*
* INPUT:
*       phyAddr - Phy address.
*       timeout - in millisec; 0 - no timeout (don't wait)
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK       - Success
*           MV_TIMEOUT  - Timeout
*
*******************************************************************************/
MV_STATUS mvEthPhyRestartAN(MV_U32 phyAddr, int timeout)
{
	MV_U16  phyRegData;

	/* Disable internal loopback first, in order to restore value of related register */
	if (mvEthPhyLoopbackSet(phyAddr, MV_FALSE) != MV_OK)
		return MV_FAIL;

	/* Read Control reg of the PHY */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

	/* disable loopback and power down */
	phyRegData &= (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK | ETH_PHY_CTRL_SPEED_MSB_MASK);

	/* Set bit 12 to Enable autonegotiation of the PHY */
	phyRegData |= ETH_PHY_CTRL_AN_ENABLE_MASK;

	/* Set bit 9 to Restart autonegotiation of the PHY */
	phyRegData |= ETH_PHY_CTRL_AN_RESTART_MASK;
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);

	if (timeout == 0)
		return MV_OK;

	/* Wait untill Auotonegotiation completed */
	while (timeout > 0) {
		mvOsSleep(100);
		timeout -= 100;

		if (mvEthPhyRegRead(phyAddr, ETH_PHY_STATUS_REG, &phyRegData) != MV_OK)
			return MV_FAIL;

		if (phyRegData & ETH_PHY_STATUS_AN_DONE_MASK)
			return MV_OK;
	}
	return MV_TIMEOUT;
}


/*******************************************************************************
* mvEthPhyDisableAN - Disable Phy Auto-Negotiation and set forced Speed and Duplex
*
* DESCRIPTION:
*       This function disable AN and set duplex and speed.
*
* INPUT:
*       phyAddr - Phy address.
*       speed   - port speed. 0 - 10 Mbps, 1-100 Mbps, 2 - 1000 Mbps
*       duplex  - port duplex. 0 - Half duplex, 1 - Full duplex
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*
*******************************************************************************/
MV_STATUS mvEthPhyDisableAN(MV_U32 phyAddr, int speed, int duplex)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

	switch (speed) {
	case 0: /* 10 Mbps */
			phyRegData &= ~ETH_PHY_CTRL_SPEED_LSB_MASK;
			phyRegData &= ~ETH_PHY_CTRL_SPEED_MSB_MASK;
			break;

	case 1: /* 100 Mbps */
			phyRegData |= ETH_PHY_CTRL_SPEED_LSB_MASK;
			phyRegData &= ~ETH_PHY_CTRL_SPEED_MSB_MASK;
			break;

	case 2: /* 1000 Mbps */
			phyRegData &= ~ETH_PHY_CTRL_SPEED_LSB_MASK;
			phyRegData |= ETH_PHY_CTRL_SPEED_MSB_MASK;
			break;

	default:
			mvOsOutput("Unexpected speed = %d\n", speed);
			return MV_FAIL;
	}

	switch (duplex) {
	case 0: /* half duplex */
			phyRegData &= ~ETH_PHY_CTRL_DUPLEX_MASK;
			break;

	case 1: /* full duplex */
			phyRegData |= ETH_PHY_CTRL_DUPLEX_MASK;
			break;

	default:
			mvOsOutput("Unexpected duplex = %d\n", duplex);
	}
	/* Clear bit 12 to Disable autonegotiation of the PHY */
	phyRegData &= ~ETH_PHY_CTRL_AN_ENABLE_MASK;

	/* Clear bit 9 to DISABLE, Restart autonegotiation of the PHY */
	phyRegData &= ~ETH_PHY_CTRL_AN_RESTART_MASK;
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData);
	return MV_OK;
}

MV_STATUS   mvEthPhyLoopback(MV_U32 phyAddr, MV_BOOL isEnable)
{
	MV_U16      regVal, ctrlVal;
	MV_STATUS   status;

	/* Set loopback speed and duplex accordingly with current */
	/* Bits: 6, 8, 13 */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &ctrlVal) != MV_OK)
		return MV_FAIL;

	if (isEnable) {
		/* Select page 2 */
		mvEthPhyRegWrite(phyAddr, 22, 2);

		mvEthPhyRegRead(phyAddr, 21, &regVal);
		regVal &= ~(ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK |
				ETH_PHY_CTRL_SPEED_MSB_MASK | ETH_PHY_CTRL_AN_ENABLE_MASK);
		regVal |= (ctrlVal & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK |
					ETH_PHY_CTRL_SPEED_MSB_MASK | ETH_PHY_CTRL_AN_ENABLE_MASK));
		mvEthPhyRegWrite(phyAddr, 21, regVal);

		/* Select page 0 */
		mvEthPhyRegWrite(phyAddr, 22, 0);

		/* Disable Energy detection   R16[9:8] = 00 */
		/* Disable MDI/MDIX crossover R16[6:5] = 00 */
		mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_CTRL_REG, &regVal);
		regVal &= ~(BIT5 | BIT6 | BIT8 | BIT9);
		mvEthPhyRegWrite(phyAddr, ETH_PHY_SPEC_CTRL_REG, regVal);

		status = mvEthPhyReset(phyAddr, 0xFFFF, MV_PHY_RESET_EXPIRE_COUNT);
		if (status != MV_OK) {
			mvOsPrintf("mvEthPhyReset failed: status=0x%x\n", status);
			return status;
		}

		/* Set loopback */
		ctrlVal |= ETH_PHY_CTRL_LOOPBACK_MASK;
		mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, ctrlVal);
	} else {
		/* Cancel Loopback */
		ctrlVal &= ~ETH_PHY_CTRL_LOOPBACK_MASK;

		status = mvEthPhyReset(phyAddr, ctrlVal, MV_PHY_RESET_EXPIRE_COUNT);
		if (status != MV_OK) {
			mvOsPrintf("mvEthPhyReset failed: status=0x%x\n", status);
			return status;
		}

		/* Enable Energy detection   R16[9:8] = 11 */
		/* Enable MDI/MDIX crossover R16[6:5] = 11 */
		mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_CTRL_REG, &regVal);
		regVal |= (BIT5 | BIT6 | BIT8 | BIT9);
		mvEthPhyRegWrite(phyAddr, ETH_PHY_SPEC_CTRL_REG, regVal);

		status = mvEthPhyReset(phyAddr, 0xFFFF, MV_PHY_RESET_EXPIRE_COUNT);
		if (status != MV_OK) {
			mvOsPrintf("mvEthPhyReset failed: status=0x%x\n", status);
			return status;
		}
	}

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyCheckLink -
*
* DESCRIPTION:
*	check link in phy port
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:   MV_TRUE if link is up, MV_FALSE if down
*
*******************************************************************************/
MV_BOOL mvEthPhyCheckLink(MV_U32 phyAddr)
{
	MV_U16 val_st, val_ctrl, val_spec_st;

	/* read status reg */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_STATUS_REG, &val_st) != MV_OK)
		return MV_FALSE;

	/* read control reg */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &val_ctrl) != MV_OK)
		return MV_FALSE;

	/* read special status reg */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_STATUS_REG, &val_spec_st) != MV_OK)
		return MV_FALSE;

	/* Check for PHY exist */
	if ((val_ctrl == ETH_PHY_SMI_DATA_MASK) && (val_st & ETH_PHY_SMI_DATA_MASK))
		return MV_FALSE;


	if (val_ctrl & ETH_PHY_CTRL_AN_ENABLE_MASK) {
		if (val_st & ETH_PHY_STATUS_AN_DONE_MASK)
			return MV_TRUE;
		else
			return MV_FALSE;
	} else {
		if (val_spec_st & ETH_PHY_SPEC_STATUS_LINK_MASK)
			return MV_TRUE;
	}
	return MV_FALSE;
}

/*******************************************************************************
* mvEthPhyPrintStatus -
*
* DESCRIPTION:
*	print port Speed, Duplex, Auto-negotiation, Link.
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:   16bit phy register value, or 0xffff on error
*
*******************************************************************************/
MV_STATUS	mvEthPhyPrintStatus(MV_U32 phyAddr)
{
	MV_U16 val;

	/* read control reg */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &val) != MV_OK)
		return MV_ERROR;

	if (val & ETH_PHY_CTRL_AN_ENABLE_MASK)
		mvOsOutput("Auto negotiation: Enabled\n");
	else
		mvOsOutput("Auto negotiation: Disabled\n");


	/* read specific status reg */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_STATUS_REG, &val) != MV_OK)
		return MV_ERROR;

	switch (val & ETH_PHY_SPEC_STATUS_SPEED_MASK) {
	case ETH_PHY_SPEC_STATUS_SPEED_1000MBPS:
			mvOsOutput("Speed: 1000 Mbps\n");
			break;
	case ETH_PHY_SPEC_STATUS_SPEED_100MBPS:
			mvOsOutput("Speed: 100 Mbps\n");
			break;
	case ETH_PHY_SPEC_STATUS_SPEED_10MBPS:
			mvOsOutput("Speed: 10 Mbps\n");
	default:
			mvOsOutput("Speed: Uknown\n");
			break;

	}

	if (val & ETH_PHY_SPEC_STATUS_DUPLEX_MASK)
		mvOsOutput("Duplex: Full\n");
	else
		mvOsOutput("Duplex: Half\n");


	if (val & ETH_PHY_SPEC_STATUS_LINK_MASK)
		mvOsOutput("Link: up\n");
	else
		mvOsOutput("Link: down\n");

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyAdvertiseSet -
*
* DESCRIPTION:
*	Set advertisement mode
*
* INPUT:
*       phyAddr - Phy address.
*	advertise - 	0x1: 10 half
*			0x2: 10 full
*			0x4: 100 half
*			0x8: 100 full
*			0x10: 1000 half
*			0x20: 1000 full
*
* OUTPUT:
*       None.
*
* RETURN:
*
*******************************************************************************/
MV_STATUS mvEthPhyAdvertiseSet(MV_U32 phyAddr, MV_U16 advertise)
{
	MV_U16 regVal, tmp;
	/* 10 - 100 */
	mvEthPhyRegRead(phyAddr, ETH_PHY_AUTONEGO_AD_REG, &regVal);

	regVal = regVal & (~ETH_PHY_10_100_BASE_ADVERTISE_MASK);
	tmp = (advertise & 0xf);
	regVal = regVal | (tmp << ETH_PHY_10_100_BASE_ADVERTISE_OFFSET);
	mvEthPhyRegWrite(phyAddr, ETH_PHY_AUTONEGO_AD_REG, regVal);
	/* 1000 */
	mvEthPhyRegRead(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, &regVal);
	/* keep only bits 4-5 + shift to the right */
	tmp = ((advertise >> 4) & 0x3);
	regVal = regVal & (~ETH_PHY_1000BASE_ADVERTISE_MASK);
	regVal = regVal | (tmp << ETH_PHY_1000BASE_ADVERTISE_OFFSET);
	mvEthPhyRegWrite(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, regVal);

	/* Reset PHY */
	mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &tmp);
	mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, tmp | ETH_PHY_CTRL_RESET_MASK);
	mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &tmp);
	while(tmp & ETH_PHY_CTRL_RESET_MASK)
		mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &tmp);

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyAdvertiseGet -
*
* DESCRIPTION:
*	Get advertisement mode
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       advertise - 	0x1: 10 half
*			0x2: 10 full
*			0x4: 100 half
*			0x8: 100 full
*			0x10: 1000 half
*			0x20: 1000 full
*
* RETURN:
*
*******************************************************************************/
MV_STATUS mvEthPhyAdvertiseGet(MV_U32 phyAddr, MV_U16 *advertise)
{
	MV_U16 regVal, tmp;
	if (advertise == NULL)
		return MV_BAD_PARAM;

	/* 10 - 100 */
	mvEthPhyRegRead(phyAddr, ETH_PHY_AUTONEGO_AD_REG, &regVal);
	tmp = ((regVal & ETH_PHY_10_100_BASE_ADVERTISE_MASK) >> ETH_PHY_10_100_BASE_ADVERTISE_OFFSET);
	/* 1000 */
	mvEthPhyRegRead(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, &regVal);
	tmp |= (((regVal & ETH_PHY_1000BASE_ADVERTISE_MASK) >> ETH_PHY_1000BASE_ADVERTISE_OFFSET) << 4);
	*advertise = tmp;
	return MV_OK;

}

/*******************************************************************************
* mvEthE1111PhyBasicInit -
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*
* INPUT:
*       phyAddr - PHY address.
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1111PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 reg;

	/* Phy recv and tx delay */
	mvEthPhyRegRead(phyAddr, 20, &reg);
	reg |= BIT1 | BIT7;
	mvEthPhyRegWrite(phyAddr, 20, reg);

	/* Leds link and activity*/
	mvEthPhyRegWrite(phyAddr, 24, 0x4111);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, reg);

	if (ethphyHalData.boardSpecInit == MV_TRUE)
		mvEthPhyRegWrite(phyAddr,
				ethphyHalData.specRegOff , ethphyHalData.specData);

}

/*******************************************************************************
* mvEthE1112PhyBasicInit -
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*
* INPUT:
*	phyAddr - PHY address.
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1112PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 reg;

	/* Set phy address */
	/*MV_REG_WRITE(ETH_PHY_ADDR_REG(ethPortNum), mvBoardPhyAddrGet(ethPortNum));*/

	/* Implement PHY errata */
	mvEthPhyRegWrite(phyAddr, 22, 2);
	mvEthPhyRegWrite(phyAddr, 0, 0x140);
	mvEthPhyRegWrite(phyAddr, 0, 0x8140);
	mvEthPhyRegWrite(phyAddr, 22, 0);

	mvEthPhyRegWrite(phyAddr, 22, 3);
	mvEthPhyRegWrite(phyAddr, 16, 0x103);
	mvEthPhyRegWrite(phyAddr, 22, 0);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, reg);

}

/*******************************************************************************
* mvEthE1116PhyBasicInit -
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*
* INPUT:
*       phyAddr - PHY address.
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1116PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 reg;

	/* Leds link and activity*/
	mvEthPhyRegWrite(phyAddr, 22, 0x3);
	mvEthPhyRegRead(phyAddr, 16, &reg);
	reg &= ~0xf;
	reg	|= 0x1;
	mvEthPhyRegWrite(phyAddr, 16, reg);
	mvEthPhyRegWrite(phyAddr, 22, 0x0);

	/* Set RGMII delay */
	mvEthPhyRegWrite(phyAddr, 22, 2);
	mvEthPhyRegRead(phyAddr, 21, &reg);
	reg	|= (BIT5 | BIT4);
	mvEthPhyRegWrite(phyAddr, 21, reg);
	mvEthPhyRegWrite(phyAddr, 22, 0);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, reg);
}


/*******************************************************************************
* mvEthE3016PhyBasicInit -
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*
* INPUT:
*       phyAddr - PHY address.
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE3016PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 reg;

	/* Leds link and activity*/
	mvEthPhyRegRead(phyAddr, 22, &reg);
	reg &= ~0xf;
	reg	|= 0xa;
	mvEthPhyRegWrite(phyAddr, 22, reg);

	/* Set RGMII (RX) delay and copper mode */
	mvEthPhyRegRead(phyAddr, 28, &reg);
	reg &= ~(BIT3 | BIT10 | BIT11);
	reg	|= (BIT10);
	mvEthPhyRegWrite(phyAddr, 28, reg);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, reg);
}


/*******************************************************************************
* mvEthE1011PhyBasicInit -
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*
* INPUT:
*       phyAddr - PHY address.
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID		mvEthE1011PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 reg;

	/* Phy recv and tx delay */
	mvEthPhyRegRead(phyAddr, 20, &reg);
	reg &= ~(BIT1 | BIT7);
	mvEthPhyRegWrite(phyAddr, 20, reg);

	/* Leds link and activity*/
	mvEthPhyRegWrite(phyAddr, 24, 0x4111);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &reg);
	reg |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, reg);

}

/*******************************************************************************
* mvEthE1112PhyPowerDown -
*
* DESCRIPTION:
*	Power down the Phy , including reset
*
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID	mvEthE1112PhyPowerDown(MV_U32 ethPortNum)
{
	mvEthPhyPower(ethPortNum, MV_FALSE);
}

/*******************************************************************************
* mvEthE1112PhyPowerUp -
*
* DESCRIPTION:
*	Power up the Phy , including reset
*
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID	mvEthE1112PhyPowerUp(MV_U32 ethPortNum)
{
	mvEthPhyPower(ethPortNum, MV_TRUE);
}

/*******************************************************************************
* mvEthPhyPower -
*
* DESCRIPTION:
*	Do a basic power down/up to the Phy , including reset
*
* INPUT:
*       ethPortNum - Ethernet port number
*	enable - MV_TRUE - power up
*		 MV_FALSE - power down
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
static MV_VOID	mvEthPhyPower(MV_U32 ethPortNum, MV_BOOL enable)
{
	MV_U16 reg;
	if (enable == MV_FALSE) {
	/* Power down command */
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 2); 		/* select page 2 */
		mvEthPhyRegRead(ethphyHalData.phyAddr[ethPortNum], 16, &reg);
		reg |= BIT3;
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 16, reg);		/* select to disable the SERDES */
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 0); 		/* select page 0 */

		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 3);		/* Power off LED's */
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 16, 0x88);
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 0);

		mvEthPhyRegRead(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, &reg);
		reg |= ETH_PHY_CTRL_RESET_MASK;
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, reg);	/* software reset */
		mvEthPhyRegRead(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, &reg);
		reg |= ETH_PHY_CTRL_POWER_DOWN_MASK;
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, reg);	/* power down the PHY */
	} else {
	/* Power up command */
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 2); 		/* select page 2 */
		mvEthPhyRegRead(ethphyHalData.phyAddr[ethPortNum], 16, &reg);
		reg &= ~BIT3;
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 16, reg);		/* select to enable the SERDES */
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 0); 		/* select page 0 */

		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 3);		/* Power on LED's */
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 16, 0x03);
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], 22, 0);

		mvEthPhyRegRead(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, &reg);
		reg |= ETH_PHY_CTRL_RESET_MASK;
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, reg);	/* software reset */
		mvEthPhyRegRead(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, &reg);
		reg &= ~ETH_PHY_CTRL_POWER_DOWN_MASK;
		mvEthPhyRegWrite(ethphyHalData.phyAddr[ethPortNum], ETH_PHY_CTRL_REG, reg);	/* power up the PHY */
	}
}


/*******************************************************************************
* mvEth1145PhyInit - Initialize MARVELL 1145 Phy
*
* DESCRIPTION:
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvEth1145PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 value;

	/* Set Link1000 output pin to be link indication, set Tx output pin to be activity */
	mvEthPhyRegWrite(phyAddr, 0x18, ETH_PHY_LED_ACT_LNK_DV);
	mvOsDelay(10);

	/* Add delay to RGMII Tx and Rx */
	mvEthPhyRegRead(phyAddr, 0x14, &value);
	mvEthPhyRegWrite(phyAddr, 0x14, (value | BIT1 | BIT7));
	mvOsDelay(10);

	/* Set Phy TPVL to 0 */
	mvEthPhyRegWrite(phyAddr, 0x10, 0x60);
	mvOsDelay(10);

	/* Reset Phy */
	mvEthPhyRegRead(phyAddr, 0x00, &value);
	mvEthPhyRegWrite(phyAddr, 0x00, (value | BIT15));
	mvOsDelay(10);

	return;
}


/*******************************************************************************
* mvEthSgmiiToCopperPhyInit - Initialize Test board 1112 Phy
*
* DESCRIPTION:
*
* INPUT:
*       phyAddr - Phy address.
*
* OUTPUT:
*       None.
*
* RETURN:
*       None.
*
*******************************************************************************/
MV_VOID mvEthSgmiiToCopperPhyBasicInit(MV_U32 port)
{
    MV_U16 value;
    MV_U16 phyAddr = 0xC;

   /* Port 0 phyAdd c */
   /* Port 1 phyAdd d */
    mvEthPhyRegWrite(phyAddr + port, 22, 3);
    mvEthPhyRegWrite(phyAddr + port, 16, 0x103);
    mvEthPhyRegWrite(phyAddr + port, 22, 0);

		/* reset the phy */
    mvEthPhyRegRead(phyAddr + port, 0, &value);
    value |= BIT15;
    mvEthPhyRegWrite(phyAddr + port, 0, value);
}


MV_VOID mvEth1121PhyBasicInit(MV_U32 phyAddr)
{
	MV_U16 value;

	/* Change page select to 2 */
	value = 2;
	mvEthPhyRegWrite(phyAddr, 22, value);
	mvOsDelay(10);

	/* Set RGMII rx delay */
	mvEthPhyRegRead(phyAddr, 21, &value);
	value |= BIT5;
	mvEthPhyRegWrite(phyAddr, 21, value);
	mvOsDelay(10);

	/* Change page select to 0 */
	value = 0;
	mvEthPhyRegWrite(phyAddr, 22, value);
	mvOsDelay(10);

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &value);
	value |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, value);
	mvOsDelay(10);
}
MV_VOID mvEthInternalGEPhyBasicInit(MV_U32 phyAddr, MV_BOOL eeeEnable)
{
	MV_U16 value;

	if ((MV_6601_DEV_ID != mvCtrlModelGet()) && 
	    (ethphyHalData.ctrlRevId < 2)) {
		mvEthPhyRegWrite(phyAddr, 0x16, 0x00FF);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x0FD0);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x214C);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x0000);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2000);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x0F16);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2146);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0);
	}

	/* reset the phy */
	mvEthPhyRegRead(phyAddr, 0, &value);
	value |= BIT15;
	mvEthPhyRegWrite(phyAddr, 0, value);
	mvOsDelay(10);

	/* EEE support on Avanta-MC internel GbE PHY */
	if ((MV_6601_DEV_ID == mvCtrlModelGet()) && (eeeEnable == MV_TRUE)) {
		mvEthPhyRegWrite(phyAddr, 0x16, 0x00FF);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x214B);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2144);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x0C28);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2146);
		mvEthPhyRegWrite(phyAddr, 0x11, 0xB233);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x214D);
		mvEthPhyRegWrite(phyAddr, 0x11, 0xCC0C);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2159);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x00FB);
		mvEthPhyRegWrite(phyAddr, 0x11, 0xC00D);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0000);
		mvEthPhyRegWrite(phyAddr, 0xD, 0x0007);
		mvEthPhyRegWrite(phyAddr, 0xE, 0x003C);
		mvEthPhyRegWrite(phyAddr, 0xD, 0x4007);
		mvEthPhyRegWrite(phyAddr, 0xE, 0x0006);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0012);
		mvEthPhyRegWrite(phyAddr, 0x0, 0x0C00);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0000);
		mvEthPhyRegWrite(phyAddr, 0x0, 0x9140);
		mvOsDelay(10);
	}

	return;
}

MV_VOID mvEth1540Y0PhyBasicInit(MV_BOOL eeeEnable)
{
	int i;
	MV_U16 reg;
	int startAddr, endAddr;

	startAddr = ethphyHalData.quadPhyStartAddr;
	endAddr = startAddr + 4;

	for (i = startAddr; i < endAddr; i++) {
		/* Enable QSGMII AN */
		/* Set page to 4. */
		mvEthPhyRegWriteExtSmi(i, 0x16, 4);
		/* Enable AN */
		mvEthPhyRegWriteExtSmi(i, 0x0, 0x1140);
		/* Set page to 0. */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0);

		/* Power up the phy */
		mvEthPhyRegReadExtSmi(i, ETH_PHY_CTRL_REG, &reg);
		reg &= ~(ETH_PHY_CTRL_POWER_DOWN_MASK);
		mvEthPhyRegWriteExtSmi(i, ETH_PHY_CTRL_REG, reg);
		mvOsDelay(100);
		if (eeeEnable) {
			/* set ELU#0 default match */
			/* RW U1 P0 R22 H0010 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x10);
			/* RW U1 P0 R1 H0104 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0104);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H4000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x4000);
			/* RW U1 P0 R1 H0904 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0904);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H4000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x4000);
			/* RW U1 P0 R1 H1104 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1104);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H4000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x4000);
			/* RW U1 P0 R1 H1904 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1904);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H4000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x4000);
			/* RW U1 P0 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

			/* set ILU#0 default match */
			/* RW U1 P0 R22 H0010 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
			/* RW U1 P0 R1 H0207 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0207);
			/* RW U1 P0 R2 h4000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x4000);
			/* RW U1 P0 R3 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0000);
			/* RW U1 P0 R1 H0A07 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0a07);
			/* RW U1 P0 R2 h4000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x4000);
			/* RW U1 P0 R3 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0000);
			/* RW U1 P0 R1 H1207 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1207);
			/* RW U1 P0 R2 h4000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x4000);
			/* RW U1 P0 R3 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0000);
			/* RW U1 P0 R1 H1A07 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1a07);
			/* RW U1 P0 R2 h4000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x4000);
			/* RW U1 P0 R3 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0000);
			/* RW U1 P0 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

			/* change the wiremac ipg from 12 to 11 */
			/* RW U1 P0 R22 H0010 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
			/* RW U1 P0 R1 H0041 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0041);
			/* RW U1 P0 R2 h00b1 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x00b1);
			/* RW U1 P0 R3 H0002 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0002);
			/* RW U1 P0 R1 H0841 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0841);
			/* RW U1 P0 R2 h00b1 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x00b1);
			/* RW U1 P0 R3 H0002 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0002);
			/* RW U1 P0 R1 H1041 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1041);
			/* RW U1 P0 R2 h00b1 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x00b1);
			/* RW U1 P0 R3 H0002 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0002);
			/* RW U1 P0 R1 H1841 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1841);
			/* RW U1 P0 R2 h00b1 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x00b1);
			/* RW U1 P0 R3 H0002 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0002);
			/* RW U1 P0 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);


			/* change the setting to not drop badtag */
			/* RW U1 P0 R22 H0010 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
			/* RW U1 P0 R1 H000b */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x000b);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H0fb4 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0fb4);
			/* RW U1 P0 R1 H080b */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x080b);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H0fb4 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0fb4);
			/* RW U1 P0 R1 H100b */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x100b);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H0fb4 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0fb4);
			/* RW U1 P0 R1 H180b */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x180b);
			/* RW U1 P0 R2 h0000 */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x0000);
			/* RW U1 P0 R3 H0fb4 */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x0fb4);
			/* RW U1 P0 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

			/* set MACSec EEE Entry/Exit Timer */
			/* RW U1 P0 R22 H0010 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
			/* RW U1 P0 R1 H03C0 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x03c0);
			/* RW U1 P0 R2 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x111e);
			/* RW U1 P0 R3 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x111e);
			/* RW U1 P0 R1 H0BC0 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x0bc0);
			/* RW U1 P0 R2 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x111e);
			/* RW U1 P0 R3 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x111e);
			/* RW U1 P0 R1 H13C0 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x13c0);
			/* RW U1 P0 R2 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x111e);
			/* RW U1 P0 R3 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x111e);
			/* RW U1 P0 R1 H1BC0 */
			mvEthPhyRegWriteExtSmi(i, 0x1, 0x1bc0);
			/* RW U1 P0 R2 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x2, 0x111e);
			/* RW U1 P0 R3 H111E */
			mvEthPhyRegWriteExtSmi(i, 0x3, 0x111e);
			/* RW U1 P0 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

			/* Start of EEE Workaround */
			/* RW U1 P0-3 R22 H00FB */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x00FB);
			/* RW U1 P0-3 R11 H1120 */
			mvEthPhyRegWriteExtSmi(i, 0xB , 0x1120);
			/* RW U1 P0-3 R8  H3666 */
			mvEthPhyRegWriteExtSmi(i, 0x8 , 0x3666);
			/* RW U1 P0-3 R22 H00FF */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x00FF);
			/* RW U1 P0-3 R17 H0F0C */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x0F0C);
			/* RW U1 P0-3 R16 H2146 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2146);
			/* RW U1 P0-3 R17 Hc090 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0xC090);
			/* RW U1 P0-3 R16 H2147 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2147);
			/* RW U1 P0-3 R17 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x0000);
			/* RW U1 P0-3 R16 H2000 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2000);
			/* RW U1 P0-3 R17 H6000 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x6000);
			/* RW U1 P0-3 R16 H2143 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2143);
			/* RW U1 P0-3 R17 HC004 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0xC004);
			/* RW U1 P0-3 R16 H2100 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2100);
			/* RW U1 P0-3 R17 H49E8 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x49E8);
			/* RW U1 P0-3 R16 H2144 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2144);
			/* RW U1 P0-3 R17 H3180 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x3180);
			/* RW U1 P0-3 R16 H2148 */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x2148);
			/* RW U1 P0-3 R17 HFC44 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0xFC44);
			/* RW U1 P0-3 R16 H214B */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x214B);
			/* RW U1 P0-3 R17 H7FD2 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x7FD2);
			/* RW U1 P0-3 R16 H214C */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x214C);
			/* RW U1 P0-3 R17 H2240 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x2240);
			/* RW U1 P0-3 R16 H214D */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x214D);
			/* RW U1 P0-3 R17 H3008 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x3008);
			/* RW U1 P0-3 R16 H214E */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x214E);
			/* RW U1 P0-3 R17 H3DF0 */
			mvEthPhyRegWriteExtSmi(i, 0x11, 0x3DF0);
			/* RW U1 P0-3 R16 H214F */
			mvEthPhyRegWriteExtSmi(i, 0x10, 0x214F);
			/* RW U1 P0-3 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0);

			/* Enable EEE Auto-neg advertisement on P0-P7 ports */
			/* RW U1 P0-3 R13 H0007 */
			mvEthPhyRegWriteExtSmi(i, 0xD , 0x0007);
			/* RW U1 P0-3 R14 H003C */
			mvEthPhyRegWriteExtSmi(i, 0xE , 0x003C);
			/* RW U1 P0-3 R13 H4007 */
			mvEthPhyRegWriteExtSmi(i, 0xD , 0x4007);
			/* RW U1 P0-3 R14 H0006 */
			mvEthPhyRegWriteExtSmi(i, 0xE , 0x0006);

			/* Soft-Reset on P0-P7 ports */
			/* RW U1 P0-3 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0);
			/* RW U1 P0-3 R0  H9140 */
			mvEthPhyRegWriteExtSmi(i, 0x0 , 0x9140);

			/* Enable MACsec EEE Master Mode on P0-3 ports */
			/* RW U1 P0 R22 H0010 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
			/* RW U1 P0 R1  H03C1 */
			mvEthPhyRegWriteExtSmi(i, 0x1 , 0x03C1);
			/* RW U1 P0 R2  H0001 */
			mvEthPhyRegWriteExtSmi(i, 0x2 , 0x0001);
			/* RW U1 P0 R3  H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3 , 0x0000);
			/* RW U1 P0 R1  H0BC1 */
			mvEthPhyRegWriteExtSmi(i, 0x1 , 0x0BC1);
			/* RW U1 P0 R2  H0001 */
			mvEthPhyRegWriteExtSmi(i, 0x2 , 0x0001);
			/* RW U1 P0 R3  H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3 , 0x0000);
			/* RW U1 P0 R1  H13C1 */
			mvEthPhyRegWriteExtSmi(i, 0x1 , 0x13C1);
			/* RW U1 P0 R2  H0001 */
			mvEthPhyRegWriteExtSmi(i, 0x2 , 0x0001);
			/* RW U1 P0 R3  H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3 , 0x0000);
			/* RW U1 P0 R1  H1BC1 */
			mvEthPhyRegWriteExtSmi(i, 0x1 , 0x1BC1);
			/* RW U1 P0 R2  H0001 */
			mvEthPhyRegWriteExtSmi(i, 0x2 , 0x0001);
			/* RW U1 P0 R3  H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x3 , 0x0000);
			/* RW U1 P0 R22 H0000 */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);
		}
		/*  set ELU#0 default match   */
		/*  RW U1 P0 R22 H0010  */
		/*  RW U1 P0 R1 H0104 */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H4000 */
		/*  RW U1 P0 R1 H0904 */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H4000 */
		/*  RW U1 P0 R1 H1104 */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H4000 */
		/*  RW U1 P0 R1 H1904 */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H4000 */
		/*  RW U1 P0 R22 H0000 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

		/*  set ILU#0 default match  */
		/*  RW U1 P0 R22 H0010 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
		/*  RW U1 P0 R1 H0207 */
		/*  RW U1 P0 R2 h4000 */
		/*  RW U1 P0 R3 H0000 */
		/*  RW U1 P0 R1 H0A07 */
		/*  RW U1 P0 R2 h4000 */
		/*  RW U1 P0 R3 H0000 */
		/*  RW U1 P0 R1 H1207 */
		/*  RW U1 P0 R2 h4000 */
		/*  RW U1 P0 R3 H0000 */
		/*  RW U1 P0 R1 H1A07 */
		/*  RW U1 P0 R2 h4000 */
		/*  RW U1 P0 R3 H0000 */
		/*  RW U1 P0 R22 H0000 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

		/*  change the wiremac ipg from 12 to 11   */
		/*  RW U1 P0 R22 H0010 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
		/*  RW U1 P0 R1 H0041 */
		/*  RW U1 P0 R2 h00b1 */
		/*  RW U1 P0 R3 H0002 */
		/*  RW U1 P0 R1 H0841 */
		/*  RW U1 P0 R2 h00b1 */
		/*  RW U1 P0 R3 H0002 */
		/*  RW U1 P0 R1 H1041 */
		/*  RW U1 P0 R2 h00b1 */
		/*  RW U1 P0 R3 H0002 */
		/*  RW U1 P0 R1 H1841 */
		/*  RW U1 P0 R2 h00b1 */
		/*  RW U1 P0 R3 H0002 */
		/*  RW U1 P0 R22 H0000 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);


		/*  change the setting to not drop badtag   */
		/*  RW U1 P0 R22 H0010 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
		/*  RW U1 P0 R1 H000b */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H0fb4 */
		/*  RW U1 P0 R1 H080b */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H0fb4 */
		/*  RW U1 P0 R1 H100b */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H0fb4 */
		/*  RW U1 P0 R1 H180b */
		/*  RW U1 P0 R2 h0000 */
		/*  RW U1 P0 R3 H0fb4 */
		/*  RW U1 P0 R22 H0000 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

		/*  set MACSec EEE Entry/Exit Timer  */
		/*  RW U1 P0 R22 H0010 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0010);
		/*  RW U1 P0 R1 H03C0 */
		/*  RW U1 P0 R2 H111E */
		/*  RW U1 P0 R3 H111E */
		/*  RW U1 P0 R1 H0BC0 */
		/*  RW U1 P0 R2 H111E */
		/*  RW U1 P0 R3 H111E */
		/*  RW U1 P0 R1 H13C0 */
		/*  RW U1 P0 R2 H111E */
		/*  RW U1 P0 R3 H111E */
		/*  RW U1 P0 R1 H1BC0 */
		/*  RW U1 P0 R2 H111E */
		/*  RW U1 P0 R3 H111E */
		/*  RW U1 P0 R22 H0000 */
		/*  RW U1 P0-3 R22 H00FB */
		/*  RW U1 P0-3 R11 H1120 */
		/*  RW U1 P0-3 R8  H3666 */
		/*  RW U1 P0-3 R22 H00FF */
		/*  RW U1 P0-3 R17 H0F0C */
		/*  RW U1 P0-3 R16 H2146 */
		/*  RW U1 P0-3 R17 Hc090 */
		/*  RW U1 P0-3 R16 H2147 */
		/*  RW U1 P0-3 R17 H0000 */
		/*  RW U1 P0-3 R16 H2000 */
		/*  RW U1 P0-3 R17 H6000 */
		/*  RW U1 P0-3 R16 H2143 */
		/*  RW U1 P0-3 R17 HC004 */
		/*  RW U1 P0-3 R16 H2100 */
		/*  RW U1 P0-3 R17 H49E8 */
		/*  RW U1 P0-3 R16 H2144 */
		/*  RW U1 P0-3 R17 H3180 */
		/*  RW U1 P0-3 R16 H2148 */
		/*  RW U1 P0-3 R17 HFC44 */
		/*  RW U1 P0-3 R16 H214B */
		/*  RW U1 P0-3 R17 H7FD2 */
		/*  RW U1 P0-3 R16 H214C */
		/*  RW U1 P0-3 R17 H2240 */
		/*  RW U1 P0-3 R16 H214D */
		/*  RW U1 P0-3 R17 H3008 */
		/*  RW U1 P0-3 R16 H214E */
		/*  RW U1 P0-3 R17 H3DF0 */
		/*  RW U1 P0-3 R16 H214F */
		/*  RW U1 P0-3 R22 H0000 */
		/*  RW U1 P0-3 R13 H0007 */
		/*  RW U1 P0-3 R14 H003C */
		/*  RW U1 P0-3 R13 H4007 */
		/*  RW U1 P0-3 R14 H0006 */
		/*  RW U1 P0-3 R22 H0000 */
		/*  RW U1 P0-3 R0  H9140 */
		/*  RW U1 P0 R22 H0010 */
		/*  RW U1 P0 R1  H03C1 */
		/*  RW U1 P0 R2  H0001 */
		/*  RW U1 P0 R3  H0000 */
		/*  RW U1 P0 R1  H0BC1 */
		/*  RW U1 P0 R2  H0001 */
		/*  RW U1 P0 R3  H0000 */
		/*  RW U1 P0 R1  H13C1 */
		/*  RW U1 P0 R2  H0001 */
		/*  RW U1 P0 R3  H0000 */
		/*  RW U1 P0 R1  H1BC1 */
		/*  RW U1 P0 R2  H0001 */
		/*  RW U1 P0 R3  H0000 */
		/*  RW U1 P0 R22 H0000 */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);

		if (eeeEnable == MV_FALSE) {
			/* Disable MACSec */
			mvEthPhyRegWriteExtSmi(i, 0x16, 0x12);
			mvEthPhyRegReadExtSmi(i, 27, &reg);
			reg &= ~(1 << 13);
			mvEthPhyRegWriteExtSmi(i, 27, reg);
		}
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);
	}
	/*  Sleep 3000 */
	mvOsDelay(300);

}

MV_VOID mvEth1540A0PhyBasicInit(MV_BOOL eeeEnable)
{
	int i;
	MV_U16 reg;
	int startAddr, endAddr;

	startAddr = ethphyHalData.quadPhyStartAddr;
	endAddr = startAddr + 4;

	for (i = startAddr; i < endAddr; i++) {
		/* Enable QSGMII AN */
		/* Set page to 4. */
		mvEthPhyRegWriteExtSmi(i, 0x16, 4);
		/* Enable AN */
		mvEthPhyRegWriteExtSmi(i, 0x0, 0x1140);
		/* Set page to 0. */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0);

		/* Power up the phy */
		mvEthPhyRegReadExtSmi(i, ETH_PHY_CTRL_REG, &reg);
		reg &= ~(ETH_PHY_CTRL_POWER_DOWN_MASK);
		mvEthPhyRegWriteExtSmi(i, ETH_PHY_CTRL_REG, reg);

		/* Disable Drop BadTag */
		mvEthPhyRegWriteExtSmi(i, 22, 0x0010);
		mvEthPhyRegWriteExtSmi(i, 1, 0x000B);
		mvEthPhyRegWriteExtSmi(i, 2, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 3, 0x0FB4);
		mvEthPhyRegWriteExtSmi(i, 1, 0x080B);
		mvEthPhyRegWriteExtSmi(i, 2, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 3, 0x0FB4);
		mvEthPhyRegWriteExtSmi(i, 1, 0x100B);
		mvEthPhyRegWriteExtSmi(i, 2, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 3, 0x0FB4);
		mvEthPhyRegWriteExtSmi(i, 1, 0x180B);
		mvEthPhyRegWriteExtSmi(i, 2, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 3, 0x0FB4);
		mvEthPhyRegWriteExtSmi(i, 22, 0x0000);

		mvEthPhyRegWriteExtSmi(i, 22, 0x00FA);
		mvEthPhyRegWriteExtSmi(i, 8, 0x0010);

		mvEthPhyRegWriteExtSmi(i, 22, 0x00FB);
		mvEthPhyRegWriteExtSmi(i, 1, 0x4099);
		mvEthPhyRegWriteExtSmi(i, 3, 0x1120);
		mvEthPhyRegWriteExtSmi(i, 11, 0x113C);
		mvEthPhyRegWriteExtSmi(i, 14, 0x8100);
		mvEthPhyRegWriteExtSmi(i, 15, 0x112A);

		mvEthPhyRegWriteExtSmi(i, 22, 0x00FC);
		mvEthPhyRegWriteExtSmi(i, 1, 0x20B0);

		mvEthPhyRegWriteExtSmi(i, 22, 0x00FF);
		mvEthPhyRegWriteExtSmi(i, 17, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2000);
		mvEthPhyRegWriteExtSmi(i, 17, 0x4444);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2140);
		mvEthPhyRegWriteExtSmi(i, 17, 0x8064);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2141);
		mvEthPhyRegWriteExtSmi(i, 17, 0x0108);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2144);
		mvEthPhyRegWriteExtSmi(i, 17, 0x0F16);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2146);
		mvEthPhyRegWriteExtSmi(i, 17, 0x8C44);
		mvEthPhyRegWriteExtSmi(i, 16, 0x214B);

		mvEthPhyRegWriteExtSmi(i, 17, 0x0F90);
		mvEthPhyRegWriteExtSmi(i, 16, 0x214C);
		mvEthPhyRegWriteExtSmi(i, 17, 0xBA33);
		mvEthPhyRegWriteExtSmi(i, 16, 0x214D);
		mvEthPhyRegWriteExtSmi(i, 17, 0x39AA);
		mvEthPhyRegWriteExtSmi(i, 16, 0x214F);
		mvEthPhyRegWriteExtSmi(i, 17, 0x8433);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2151);
		mvEthPhyRegWriteExtSmi(i, 17, 0x2010);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2152);
		mvEthPhyRegWriteExtSmi(i, 17, 0x99EB);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2153);
		mvEthPhyRegWriteExtSmi(i, 17, 0x2F3B);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2154);
		mvEthPhyRegWriteExtSmi(i, 17, 0x584E);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2156);
		mvEthPhyRegWriteExtSmi(i, 17, 0x1223);
		mvEthPhyRegWriteExtSmi(i, 16, 0x2158);
		mvEthPhyRegWriteExtSmi(i, 22, 0x0000);

		/* Enable EEE_Auto-neg for 1000BASE-T and 100BASE-TX */
		mvEthPhyRegWriteExtSmi(i, 22, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 13, 0x0007);
		mvEthPhyRegWriteExtSmi(i, 14, 0x003C);
		mvEthPhyRegWriteExtSmi(i, 13, 0x4007);
		mvEthPhyRegWriteExtSmi(i, 14, 0x0006);

		/* Enable MACSec (Reg 27_2.13 = '1') */
		mvEthPhyRegWriteExtSmi(i, 22, 0x0012);
		mvEthPhyRegReadExtSmi(i, 27, &reg);
		reg |= (1 << 13);
		mvEthPhyRegWriteExtSmi(i, 27, reg);

		if (eeeEnable == MV_TRUE) {
			/* Enable EEE Master (Legacy) Mode */
			mvEthPhyRegWriteExtSmi(i, 22, 0x0010);
			mvEthPhyRegWriteExtSmi(i, 1, 0x03c1);
			mvEthPhyRegWriteExtSmi(i, 2, 0x0001);
			mvEthPhyRegWriteExtSmi(i, 3, 0x0000);
			mvEthPhyRegWriteExtSmi(i, 1, 0x0bc1);
			mvEthPhyRegWriteExtSmi(i, 2, 0x0001);
			mvEthPhyRegWriteExtSmi(i, 3, 0x0000);
			mvEthPhyRegWriteExtSmi(i, 1, 0x13c1);
			mvEthPhyRegWriteExtSmi(i, 2, 0x0001);
			mvEthPhyRegWriteExtSmi(i, 3, 0x0000);
			mvEthPhyRegWriteExtSmi(i, 1, 0x1bc1);
			mvEthPhyRegWriteExtSmi(i, 2, 0x0001);
			mvEthPhyRegWriteExtSmi(i, 3, 0x0000);
			mvEthPhyRegWriteExtSmi(i, 22, 0x0000);
		}

		mvEthPhyRegWriteExtSmi(i, 22, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 0, 0x9140);

		mvOsDelay(100);
	}
	/*  Sleep 3000 */
	/* mvOsDelay(300); */

}

MV_VOID mvEth1540Z1PhyBasicInit(MV_BOOL eeeEnable)
{
	MV_U32 i;
	MV_U32 startAddr, endAddr;

	startAddr = ethphyHalData.quadPhyStartAddr;
	endAddr = startAddr + 4;

	for (i = startAddr; i < endAddr; i++) {
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0002);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x6008);
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0012);
		mvEthPhyRegWriteExtSmi(i, 0x1, 0x111E);
		mvEthPhyRegWriteExtSmi(i, 0x2, 0x111E);
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x00FA);
		mvEthPhyRegWriteExtSmi(i, 0x8, 0x0010);
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x00FB);
		mvEthPhyRegWriteExtSmi(i, 0x1, 0x4099);
		mvEthPhyRegWriteExtSmi(i, 0x3, 0x1120);
		mvEthPhyRegWriteExtSmi(i, 0xb, 0x113C);
		mvEthPhyRegWriteExtSmi(i, 0xe, 0x8100);
		mvEthPhyRegWriteExtSmi(i, 0xf, 0x112A);
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x00FC);
		mvEthPhyRegWriteExtSmi(i, 0x1, 0x20B0);
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x00FF);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2000);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x4444);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2140);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x8064);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2141);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x0108);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2144);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x0F16);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2146);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x8C44);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x214B);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x0F90);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x214C);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0xBA33);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x214D);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x39AA);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x214F);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x8433);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2151);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x2010);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2152);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x99EB);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2153);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x2F3B);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2154);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x584E);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2156);
		mvEthPhyRegWriteExtSmi(i, 0x11, 0x1223);
		mvEthPhyRegWriteExtSmi(i, 0x10, 0x2158);
		mvEthPhyRegWriteExtSmi(i, 0x16, 0x0000);
		mvEthPhyRegWriteExtSmi(i, 0x0 , 0x9140);
	}
}


MV_VOID mvEth1540PhyBasicInit(MV_BOOL eeeEnable)
{
	MV_U16 reg;
	int startAddr;

	startAddr = ethphyHalData.quadPhyStartAddr;

	/* Reads ID2 */
	if (mvEthPhyRegReadExtSmi(startAddr, 3, &reg) != MV_OK)
		return;

	/* Check if this is 1545-Z1 */
	if (((reg & 0x3F0) >> 4) == MV_PHY_88E154X_Z1) {
		mvEth1540Z1PhyBasicInit(eeeEnable);
		return;
	}

	/* Identify PHY Revision. */
	mvEthPhyRegWriteExtSmi(startAddr, 0x16, 0xFB);
	mvEthPhyRegReadExtSmi(startAddr, 0x1F, &reg);
	mvEthPhyRegWriteExtSmi(startAddr, 0x16, 0x0);

	if (reg == 0x100)
		mvEth1540Y0PhyBasicInit(eeeEnable);
	else
		mvEth1540A0PhyBasicInit(eeeEnable);

	return;
}

MV_VOID mvEth1340PhyBasicInit(void)
{
	int i;
	MV_U16 reg;
	int startAddr, endAddr;

	startAddr = ethphyHalData.quadPhyStartAddr;
	endAddr = startAddr + 4;

	for (i = startAddr; i < endAddr; i++) {
		mvEthPhyRegWriteExtSmi(i, 0x16, 0);
		mvEthPhyRegWriteExtSmi(i, 0x1d, 3);
		mvEthPhyRegWriteExtSmi(i, 0x1e, 2);
		mvEthPhyRegWriteExtSmi(i, 0x1d, 0);

		/* Power up the phy */
		/* mvEthPhyRegReadExtSmi(i,ETH_PHY_CTRL_REG, &reg); */
		/* reg |= ETH_PHY_CTRL_RESET_MASK; */
		/* mvEthPhyRegWriteExtSmi(i,ETH_PHY_CTRL_REG, reg);   software reset */

		/* Enable QSGMII AN */
		/* Set page to 4. */
		mvEthPhyRegWriteExtSmi(i, 0x16, 4);
		/* Enable AN */
		mvEthPhyRegWriteExtSmi(i, 0x0, 0x1140);
		/* Set page to 0. */
		mvEthPhyRegWriteExtSmi(i, 0x16, 0);

		mvEthPhyRegReadExtSmi(i, ETH_PHY_CTRL_REG, &reg);
		reg &= ~(ETH_PHY_CTRL_POWER_DOWN_MASK);
		mvEthPhyRegWriteExtSmi(i, ETH_PHY_CTRL_REG, reg);
	}
}

MV_VOID mvEth131xPhyBasicInit(MV_U32 phyAddr)
{
	MV_U16	val;

	mvEthPhyRegWrite(phyAddr, 0x16, 0x2);
	mvEthPhyRegRead(phyAddr, 0x18, &val);
	/* Force PMOS, NMOS values. */
	val |= (1 << 6);
	/* Set NMOS value. */
	val &= ~0x7;
	val |= 0x1;

	/* Set PMOS value. */
	val &= ~(0x7 << 8);
	val |= (1 << 8);
	mvEthPhyRegWrite(phyAddr, 0x18, val);
	mvEthPhyRegWrite(phyAddr, 0x16, 0x0);

	return;
}
/*******************************************************************************
* mvEthE1512PhyBasicInit -
*
* DESCRIPTION:
*	Do a basic Init to the Phy , including reset
*
* INPUT:
*       ethPortNum - Ethernet port number
*
* OUTPUT:
*       None.
*
* RETURN:   None
*
*******************************************************************************/
MV_VOID mvEthE1512PhyBasicInit(MV_U32 ethPortNum, MV_BOOL eeeEnable)
{
	MV_U32 phyAddr;

	phyAddr = ethphyHalData.phyAddr[ethPortNum];

	mvEthPhyRegWrite(phyAddr, 0x16, 0x12);
	mvEthPhyRegWrite(phyAddr, 0x14, 0x201);
	mvEthPhyRegWrite(phyAddr, 0x14, 0x8201);
	mvEthPhyRegWrite(phyAddr, 0x16, 0);

	/* EEE support on Avanta-MC 1512 PHY(on-board) */
	if ((MV_6601_DEV_ID == mvCtrlModelGet()) && (eeeEnable == MV_TRUE)) {
		mvEthPhyRegWrite(phyAddr, 0x16, 0x00FF);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x214B);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2144);
		mvEthPhyRegWrite(phyAddr, 0x11, 0x0C28);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2146);
		mvEthPhyRegWrite(phyAddr, 0x11, 0xB233);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x214D);
		mvEthPhyRegWrite(phyAddr, 0x11, 0xCC0C);
		mvEthPhyRegWrite(phyAddr, 0x10, 0x2159);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x00FB);
		mvEthPhyRegWrite(phyAddr, 0x11, 0xC00D);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0000);
		mvEthPhyRegWrite(phyAddr, 0xD, 0x0007);
		mvEthPhyRegWrite(phyAddr, 0xE, 0x003C);
		mvEthPhyRegWrite(phyAddr, 0xD, 0x4007);
		mvEthPhyRegWrite(phyAddr, 0xE, 0x0006);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0012);
		mvEthPhyRegWrite(phyAddr, 0x0, 0x0C00);
		mvEthPhyRegWrite(phyAddr, 0x16, 0x0000);
		mvEthPhyRegWrite(phyAddr, 0x0, 0x9140);
		mvOsDelay(10);
	}
}

/* Check GEPHY or FEPHY through Reg1.8 */
static MV_STATUS mvEthPhyIsFe(MV_U32 phyAddr, MV_BOOL *Is_Fe_PHY)
{
	MV_U16  phyRegData;
	MV_BOOL isfe = 0;

	/* Check PHY exist or not */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_ID2_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	if (phyRegData == ETH_PHY_SMI_DATA_MASK)
		return MV_FAIL;

	/* Judge PHY is FE or GE through reg1.8 */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_STATUS_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	if (ETH_PHY_STATUS_EXTEND_MASK & phyRegData)
		isfe = 0;
	else
		isfe = 1;

	*Is_Fe_PHY = isfe;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhySetAdminState - Set phy power down state.
*
* DESCRIPTION:
*       The API Configures the PHY state on the eth port.
* INPUTS:
*       phyAddr - Phy address.
*       phy_state  - PHY port  state to set.
*                         0:power down
*                         1:power up
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/

MV_STATUS mvEthPhySetAdminState(MV_U32 phyAddr, MV_BOOL phy_state)
{
	MV_U16  phyRegData;
	MV_BOOL pre_state;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	pre_state = (phyRegData & ETH_PHY_CTRL_POWER_DOWN_MASK) ? 0 : 1;

	if (pre_state != phy_state) {
		if (!phy_state) {
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData | ETH_PHY_CTRL_POWER_DOWN_MASK) != MV_OK)
				return MV_FAIL;
		} else {
			/* from down to normal, 0.11 and 16.2 must be set 0 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData & (~ETH_PHY_CTRL_POWER_DOWN_MASK)) != MV_OK)
				return MV_FAIL;
			if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_CTRL_REG, &phyRegData) != MV_OK)
				return MV_FAIL;
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_SPEC_CTRL_REG, phyRegData & (~ETH_PHY_SPEC_CTRL_POWER_DOWN_MASK)) != MV_OK)
				return MV_FAIL;
		}
	}

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyGetAdminState - Get PHY power down dtate.
*
* DESCRIPTION:
*       The API get the PHY state on the eth port.
* INPUTS:
*       phyAddr - Phy address.
*       phy_state  - PHY port  state to set.
*                         0:power down
*                         1:power up
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyGetAdminState(MV_U32 phyAddr, MV_BOOL *phy_state)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	*phy_state = (phyRegData & ETH_PHY_CTRL_POWER_DOWN_MASK) ? 0 : 1;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyGetLinkStatus - Get PHY link state.
*
* DESCRIPTION:
*       The API get the PHY  link state on the eth port.
* INPUTS:
*       phyAddr - Phy address.
*       link_state  - PHY link state to set.
*                         0:link down
*                         1:link up
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyGetLinkStatus(MV_U32 phyAddr, MV_BOOL *link_state)
{
	MV_U16  phyRegData;

	/* Check if the PHY exist */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_ID2_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	if (phyRegData == ETH_PHY_SMI_DATA_MASK)
		return MV_FAIL;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_STATUS_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	/* Check status valid or not */
	if (!(phyRegData & ETH_PHY_SPEC_STATUS_RESOLVE_MASK))
		return MV_FAIL;

	*link_state = (phyRegData & ETH_PHY_SPEC_STATUS_LINK_MASK) ? 1 : 0;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyDuplexOperGet - Get PHYduplex info.
*
* DESCRIPTION:
*       The API get the PHY duplex status on the eth port. It reflects the real
*       configuration happened in PHY.
* INPUTS:
*       phyAddr       - Phy address.
*       state_valid   - indicates PHY state is valid or not
*                         0: invalid
*                         1: valid
*       duplex_state  - PHY link state to set.
*                         0:half duplex
*                         1:full duplex
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
* COMMENT: Reg17.13
*******************************************************************************/
MV_STATUS mvEthPhyDuplexOperGet(MV_U32 phyAddr, MV_BOOL *state_valid, MV_BOOL *duplex_state)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_STATUS_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	/* Check status valid or not */
	if (!(phyRegData & ETH_PHY_SPEC_STATUS_RESOLVE_MASK)) {
		*state_valid = MV_FALSE;
		return MV_OK;
	}
	*state_valid = MV_TRUE;

	*duplex_state = (phyRegData & ETH_PHY_SPEC_STATUS_DUPLEX_MASK) ? 1 : 0;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyPauseSet - Set pause of PHY.
*
* DESCRIPTION:
*       The API set pause state.
* INPUTS:
*       phyAddr       - Phy address.
*       pause_state   - pause state set to
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyPauseSet(MV_U32 phyAddr, MV_U32 pause_state)
{
	MV_U16  phyRegData;
	MV_U32 pre_state = MV_ETHPHY_NO_PAUSE;

	/* read current pause state */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_AUTONEGO_AD_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	if (phyRegData & ETH_PHY_SPEC_CTRL_PAUSE_MASK)
		pre_state |= MV_ETHPHY_PAUSE;
	if (phyRegData & ETH_PHY_SPEC_CTRL_ASY_PAUSE_MASK)
		pre_state |= MV_ETHPHY_ASYMMETRIC_PAUSE;

	if (pause_state == pre_state)
		return MV_OK;

	/*set expected PAUSE state*/
	phyRegData &= ((~ETH_PHY_SPEC_CTRL_PAUSE_MASK) & (~ETH_PHY_SPEC_CTRL_ASY_PAUSE_MASK));
	switch (pause_state) {
	case MV_ETHPHY_NO_PAUSE:
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_AUTONEGO_AD_REG, phyRegData) != MV_OK)
			return MV_FAIL;
		break;
	case MV_ETHPHY_PAUSE:
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_AUTONEGO_AD_REG, phyRegData |
								       ETH_PHY_SPEC_CTRL_PAUSE_MASK) != MV_OK)
			return MV_FAIL;
		break;
	case MV_ETHPHY_ASYMMETRIC_PAUSE:
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_AUTONEGO_AD_REG, phyRegData |
								       ETH_PHY_SPEC_CTRL_ASY_PAUSE_MASK) != MV_OK)
			return MV_FAIL;
		break;
	case MV_ETHPHY_BOTH_PAUSE:
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_AUTONEGO_AD_REG, phyRegData |
								       ETH_PHY_SPEC_CTRL_ASY_PAUSE_MASK |
								       ETH_PHY_SPEC_CTRL_PAUSE_MASK) != MV_OK)
			return MV_FAIL;
		break;
	default:
		return MV_FAIL;
	}

	/*restart auto-nego, let set above take effect*/
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	phyRegData = phyRegData | ETH_PHY_CTRL_AN_RESTART_MASK;

	if (mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, phyRegData) != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyPauseAdminGet - Get pause state of PHY.
*
* DESCRIPTION:
*       The API get pause state.
* INPUTS:
*       phyAddr       - Phy address.
*       pause_state   - pause state set to
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
* COMMENT: Reg4.10 4.11
*******************************************************************************/
MV_STATUS mvEthPhyPauseAdminGet(MV_U32 phyAddr, MV_U32 *pause_state)
{
	MV_U16  phyRegData;
	MV_U32 pre_state = MV_ETHPHY_NO_PAUSE;

	/* read current pause state */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_AUTONEGO_AD_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	if (phyRegData & ETH_PHY_SPEC_CTRL_PAUSE_MASK)
		pre_state |= MV_ETHPHY_PAUSE;
	if (phyRegData & ETH_PHY_SPEC_CTRL_ASY_PAUSE_MASK)
		pre_state |= MV_ETHPHY_ASYMMETRIC_PAUSE;

	*pause_state = pre_state;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyLoopbackSet - Set loopback state of PHY.
*
* DESCRIPTION:
*       The API set loopback state.
* INPUTS:
*       phyAddr       - Phy address.
*       isEnable      - loopback state.
*                     0--disable
*                     1--enabled
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyLoopbackSet(MV_U32 phyAddr, MV_BOOL isEnable)
{
	MV_U16	phyRegData, ctrlVal;
	MV_BOOL isfe = 0;

	if (mvEthPhyIsFe(phyAddr, &isfe))
		return MV_FAIL;

	/* Set loopback speed and duplex accordingly with current */
	/* Bits: 6, 8, 13 */
	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &ctrlVal) != MV_OK)
		return MV_FAIL;

	if (isEnable) {
		if (!isfe) {
			/* select page 2 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 2) != MV_OK)
				return MV_FAIL;

			/* Set register 21_2 */
			if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_CTRL2_PAGE2, &phyRegData) != MV_OK)
				return MV_FAIL;
			phyRegData &= ~(ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK |
					ETH_PHY_CTRL_SPEED_MSB_MASK | ETH_PHY_CTRL_AN_ENABLE_MASK);
			phyRegData |= (ctrlVal & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_SPEED_LSB_MASK |
						  ETH_PHY_CTRL_SPEED_MSB_MASK | ETH_PHY_CTRL_AN_ENABLE_MASK));
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_SPEC_CTRL2_PAGE2, phyRegData) != MV_OK)
				return MV_FAIL;

			/* restore page0 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0) != MV_OK)
				return MV_FAIL;

			/* Force master */
			if (mvEthPhyRegRead(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, &phyRegData) != MV_OK)
				return MV_FAIL;
			phyRegData |= ETH_PHY_1000BASE_MASTER_ENABLE_MASK;
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, phyRegData) != MV_OK)
				return MV_FAIL;

			/*reset PHY */
			if (mvEthPhyReset(phyAddr, 0xFFFF, MV_PHY_RESET_EXPIRE_COUNT) != MV_OK)
				return MV_FAIL;
			/* When auto-nego is disabled, 2 PHY reset is needed */
			if (mvEthPhyReset(phyAddr, 0xFFFF, MV_PHY_RESET_EXPIRE_COUNT) != MV_OK)
				return MV_FAIL;

			/* Select page 0x00FA, there is no description in reference */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0x00FA) != MV_OK)
				return MV_FAIL;

			/* write register 7, force 1000M link, dfault is 0x200*/
			if (mvEthPhyRegWrite(phyAddr, 7, 0x020c) != MV_OK)
				return MV_FAIL;

			/* restore page0 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0) != MV_OK)
				return MV_FAIL;
		}

		/* Set loopback */
		ctrlVal |= ETH_PHY_CTRL_LOOPBACK_MASK;
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_CTRL_REG, ctrlVal) != MV_OK)
			return MV_FAIL;
	} else {
		if (!isfe) {
			/*restore original setting*/
			/* Select page 0x00FA */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0x00FA) != MV_OK)
				return MV_FAIL;

			/* write register 7, restore default is 0x200*/
			if (mvEthPhyRegWrite(phyAddr, 7, 0x0200) != MV_OK)
				return MV_FAIL;

			/* restore page0 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0) != MV_OK)
				return MV_FAIL;

			/* Cancel  Force master */
			if (mvEthPhyRegRead(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, &phyRegData) != MV_OK)
				return MV_FAIL;
			phyRegData &= (~ETH_PHY_1000BASE_MASTER_ENABLE_MASK);
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_1000BASE_T_CTRL_REG, phyRegData) != MV_OK)
				return MV_FAIL;
		}
		/*disable loopback*/
		ctrlVal &= (~ETH_PHY_CTRL_LOOPBACK_MASK);
		if (mvEthPhyReset(phyAddr, ctrlVal, MV_PHY_RESET_EXPIRE_COUNT) !=MV_OK)
			return MV_FAIL;
	}

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyLoopbackGet - Get loopback state of PHY.
*
* DESCRIPTION:
*       The API get loopback state.
* INPUTS:
*       phyAddr       - Phy address.
*
* OUTPUT:
*       isEnable      - loopback state.
*                     0--disable
*                     1--enabled
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyLoopbackGet(MV_U32 phyAddr, MV_BOOL *isEnable)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

	*isEnable = (phyRegData & ETH_PHY_CTRL_LOOPBACK_MASK) ? 1 : 0;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyLineLoopbackSet - Set line loopback state of PHY.
*
* DESCRIPTION:
*       The API set loopback state.
* INPUTS:
*       phyAddr       - Phy address.
*       isEnable      - loopback state.
*                     0--disable
*                     1--enabled
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyLineLoopbackSet(MV_U32 phyAddr, MV_BOOL isEnable)
{
	MV_U16  phyRegData;
	MV_BOOL isfe = 0;

	if (mvEthPhyIsFe(phyAddr, &isfe))
		return MV_FAIL;

	if (isEnable) {
		if (!isfe) {
			/* select page 2 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 2) != MV_OK)
				return MV_FAIL;
			/* set speed to 1000Mbps */
			if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_CTRL2_PAGE2, &phyRegData) != MV_OK)
				return MV_FAIL;
			phyRegData &= 0xFFFFFFF8;
			phyRegData |= ETH_PHY_SPEC_CTRL2_PAGE2_MAC_SPEED_1000M;
			phyRegData |= ETH_PHY_SPEC_CTRL2_PAGE2_LINE_LPK_MASK;
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_SPEC_CTRL2_PAGE2, phyRegData) != MV_OK)
				return MV_FAIL;
			/* restore page0 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0) != MV_OK)
				return MV_FAIL;
		} else {

			if (mvEthPhyRegRead(phyAddr, ETH_PHY_FE_SPEC_CTRL2, &phyRegData) != MV_OK)
				return MV_FAIL;

			phyRegData |= ETH_PHY_SPEC_CTRL2_FE_LINE_LPK_MASK;

			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_FE_SPEC_CTRL2, phyRegData) != MV_OK)
				return MV_FAIL;
		}
	} else {
		if (!isfe) {
			/* select page 2 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 2) != MV_OK)
				return MV_FAIL;
			phyRegData &= (~ETH_PHY_SPEC_CTRL2_PAGE2_LINE_LPK_MASK);
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_SPEC_CTRL2_PAGE2, phyRegData) != MV_OK)
				return MV_FAIL;
			/* restore page0 */
			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0) != MV_OK)
				return MV_FAIL;
		} else {

			if (mvEthPhyRegRead(phyAddr, ETH_PHY_FE_SPEC_CTRL2, &phyRegData) != MV_OK)
				return MV_FAIL;

			phyRegData &= (~ETH_PHY_SPEC_CTRL2_FE_LINE_LPK_MASK);

			if (mvEthPhyRegWrite(phyAddr, ETH_PHY_FE_SPEC_CTRL2, phyRegData) != MV_OK)
				return MV_FAIL;
		}
	}

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyLineLoopbackGet - Get line loopback state of PHY, external loopback
*
* DESCRIPTION:
*       The API get loopback state.
* INPUTS:
*       phyAddr       - Phy address.
*
* OUTPUT:
*       isEnable      - loopback state.
*                     0--disable
*                     1--enabled
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyLineLoopbackGet(MV_U32 phyAddr, MV_BOOL *isEnable)
{
	MV_U16  phyRegData;
	MV_BOOL isfe = 0;

	if (mvEthPhyIsFe(phyAddr, &isfe))
		return MV_FAIL;

	if (!isfe) {
		/* select page 2 */
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 2) != MV_OK)
			return MV_FAIL;

		if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_CTRL2_PAGE2, &phyRegData) != MV_OK)
			return MV_FAIL;
		/* restore page0 */
		if (mvEthPhyRegWrite(phyAddr, ETH_PHY_PAGE_ADDR, 0) != MV_OK)
			return MV_FAIL;

		*isEnable = (phyRegData & ETH_PHY_SPEC_CTRL2_PAGE2_LINE_LPK_MASK) ? 1 : 0;
	} else {
		if (mvEthPhyRegRead(phyAddr, ETH_PHY_FE_SPEC_CTRL2, &phyRegData) != MV_OK)
			return MV_FAIL;
		*isEnable = (phyRegData & ETH_PHY_SPEC_CTRL2_FE_LINE_LPK_MASK) ? 1 : 0;
	}

	return MV_OK;
}


/*******************************************************************************
* mvEthPhyDuplexModeSet - Set duplex mode of PHY.
*
* DESCRIPTION:
*		This function will keep the speed and loopback mode to the
*		previous value, but disable others, such as Autonegotiation.
* INPUTS:
*       phyAddr       - Phy address.
*       isEnable      - duplex state
*                     0--half duplex
*                     1--full duplex
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyDuplexModeSet(MV_U32 phyAddr, MV_BOOL isEnable)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

	if (isEnable)
		phyRegData = phyRegData | ETH_PHY_CTRL_DUPLEX_MASK;
	else
		phyRegData = phyRegData & (~ETH_PHY_CTRL_DUPLEX_MASK);

	phyRegData = phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK |
				   ETH_PHY_CTRL_LOOPBACK_MASK |
				   ETH_PHY_CTRL_SPEED_LSB_MASK |
				   ETH_PHY_CTRL_SPEED_MSB_MASK);
	/* Write Control reg and reset PHY */
	if (mvEthPhyReset(phyAddr, phyRegData, MV_PHY_RESET_EXPIRE_COUNT) != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyDuplexModeAdminGet - Get configured duplex mode of PHY.
*
* DESCRIPTION:
*       The API get duplex mode.
* INPUTS:
*       phyAddr       - Phy address.
*
* OUTPUT:
*       isEnable      - duplex state
*                     0--half duplex
*                     1--full duplex
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
* Comment: reg0.8
*******************************************************************************/
MV_STATUS mvEthPhyDuplexModeAdminGet(MV_U32 phyAddr, MV_BOOL *isEnable)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	*isEnable = (phyRegData & ETH_PHY_CTRL_DUPLEX_MASK) ? 1 : 0;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhySpeedSet - Set speed of PHY.
*
* DESCRIPTION:
*		This function will keep the duplex mode and loopback mode to the
*		previous value, but disable others, such as Autonegotiation.
* INPUTS:
*       phyAddr       - Phy address.
*       speed         - PHY speed
*                     0--10 Mbps
*                     1--100 Mbps
*                     2--1000 Mbps
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhySpeedSet(MV_U32 phyAddr, MV_U32 speed)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

	phyRegData = phyRegData & ((~ETH_PHY_CTRL_SPEED_LSB_MASK) & (~ETH_PHY_CTRL_SPEED_MSB_MASK));

	switch (speed) {
	case MV_ETHPHY_SPEED_10_MBPS:
		phyRegData = phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_LOOPBACK_MASK);
		break;
	case MV_ETHPHY_SPEED_100_MBPS:
		phyRegData = (phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_LOOPBACK_MASK)) |
			     ETH_PHY_CTRL_SPEED_LSB_MASK;
		break;
	case MV_ETHPHY_SPEED_1000_MBPS:
		phyRegData = (phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_LOOPBACK_MASK)) |
			     ETH_PHY_CTRL_SPEED_MSB_MASK;
		break;
	default:
		return MV_FAIL;
	}

	if (mvEthPhyReset(phyAddr, phyRegData, MV_PHY_RESET_EXPIRE_COUNT) != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhySpeedOperGet - Get PHYduplex info.
*
* DESCRIPTION:
*       The API get the PHY speed status on the eth port. It reflects the real
*       configuration happened in PHY.
* INPUTS:
*       phyAddr       - Phy address.
*       speed         - PHY connection speed.
*                         0--10 Mbps
*                         1--100 Mbps
*                         2--1000 Mbps
*                         3--Unknown
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
* COMMENT: Reg17.14 and 17.15
*******************************************************************************/
MV_STATUS mvEthPhySpeedOperGet(MV_U32 phyAddr, MV_U32 *speed)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_SPEC_STATUS_REG, &phyRegData) != MV_OK)
		return MV_ERROR;
	/* Check status valid or not */
	if (!(phyRegData & ETH_PHY_SPEC_STATUS_RESOLVE_MASK)) {
		*speed = MV_ETHPHY_SPEED_UNKNOWN;
		return MV_OK;
	}

	switch (phyRegData & ETH_PHY_SPEC_STATUS_SPEED_MASK) {
	case ETH_PHY_SPEC_STATUS_SPEED_1000MBPS:
		*speed = MV_ETHPHY_SPEED_1000_MBPS;
		break;
	case ETH_PHY_SPEC_STATUS_SPEED_100MBPS:
		*speed = MV_ETHPHY_SPEED_100_MBPS;
		break;
	case ETH_PHY_SPEC_STATUS_SPEED_10MBPS:
		*speed = MV_ETHPHY_SPEED_10_MBPS;
		break;
	default:
		return MV_FAIL;
	}

	return MV_OK;
}

/*******************************************************************************
* mvEthPhySpeedAdminGet - Get Configured speed of PHY.
*
* DESCRIPTION:
*       The API get speed.
* INPUTS:
*       phyAddr       - Phy address.
*
* OUTPUT:
*       speed         - duplex state
*                     0--10 Mbps
*                     1--100 Mbps
*                     2--1000 Mbps
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
* Comment: Reg0.6, 0.13
*******************************************************************************/
MV_STATUS mvEthPhySpeedAdminGet(MV_U32 phyAddr, MV_U32 *speed)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_ERROR;

	/* MSB = 1, LSB = 0, 1000M */
	if ((phyRegData & ETH_PHY_CTRL_SPEED_MSB_MASK) && (!(phyRegData & ETH_PHY_CTRL_SPEED_LSB_MASK)))
		*speed = MV_ETHPHY_SPEED_1000_MBPS;
	/* MSB = 0, LSB = 1, 100M */
	else if ((phyRegData & ETH_PHY_CTRL_SPEED_LSB_MASK) && (!(phyRegData & ETH_PHY_CTRL_SPEED_MSB_MASK)))
		*speed = MV_ETHPHY_SPEED_100_MBPS;
	/* MSB = 0, LSB = 0, 10M */
	else if ((!(phyRegData & ETH_PHY_CTRL_SPEED_LSB_MASK)) && (!(phyRegData & ETH_PHY_CTRL_SPEED_MSB_MASK)))
		*speed = MV_ETHPHY_SPEED_10_MBPS;
	else
		return MV_FAIL;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhySpeedDuplexModeSet - Set Speed and duplex mode of PHY.
*
* DESCRIPTION:
*		This function will keep the loopback mode to the
*		previous value, but disable others, such as Autonegotiation.
* INPUTS:
*       phyAddr       - Phy address.
*       speed         - PHY speed
*                     0--10 Mbps
*                     1--100 Mbps
*                     2--1000 Mbps
*       isEnable      - duplex state
*                     0--half duplex
*                     1--full duplex
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhySpeedDuplexModeSet(MV_U32 phyAddr, MV_U32 speed, MV_BOOL isEnable)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;

	if (isEnable)
		phyRegData = phyRegData | ETH_PHY_CTRL_DUPLEX_MASK;
	else
		phyRegData = phyRegData & (~ETH_PHY_CTRL_DUPLEX_MASK);

	phyRegData = phyRegData & ((~ETH_PHY_CTRL_SPEED_LSB_MASK) & (~ETH_PHY_CTRL_SPEED_MSB_MASK));

	switch (speed) {
	case MV_ETHPHY_SPEED_10_MBPS:
		phyRegData = phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_LOOPBACK_MASK);
		break;
	case MV_ETHPHY_SPEED_100_MBPS:
		phyRegData = (phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_LOOPBACK_MASK)) |
			     ETH_PHY_CTRL_SPEED_LSB_MASK;
		break;
	case MV_ETHPHY_SPEED_1000_MBPS:
		phyRegData = (phyRegData & (ETH_PHY_CTRL_DUPLEX_MASK | ETH_PHY_CTRL_LOOPBACK_MASK)) |
			     ETH_PHY_CTRL_SPEED_MSB_MASK;
		break;
	default:
		return MV_FAIL;
	}

	if (mvEthPhyReset(phyAddr, phyRegData, MV_PHY_RESET_EXPIRE_COUNT) != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyAutoNegoSet - Set auto nego of PHY.
*
* DESCRIPTION:
*       The API set auto nego.
* INPUTS:
*       phyAddr       - Phy address.
*       isEnable      - auto nego state
*                     0--disable auto nego
*                     1--enable auto nego
*
* OUTPUT:
*       None.
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyAutoNegoSet(MV_U32 phyAddr, MV_BOOL isEnable)
{
	MV_U16  phyRegData;
	MV_BOOL pre_state;

	/* Disable internal loopback first if enable auto-nego, in order to restore value of related register */
	if (isEnable) {
		if (mvEthPhyLoopbackSet(phyAddr, MV_FALSE) != MV_OK)
			return MV_FAIL;
	}

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	pre_state = (phyRegData & ETH_PHY_CTRL_AN_ENABLE_MASK) ? 1 : 0;
	if (isEnable == pre_state)
		return MV_OK;
	if (isEnable)
		phyRegData |= ETH_PHY_CTRL_AN_ENABLE_MASK;
	else
		phyRegData &= (~ETH_PHY_CTRL_AN_ENABLE_MASK);

	/*reset PHY, let set above take effect*/
	if (mvEthPhyReset(phyAddr, phyRegData, MV_PHY_RESET_EXPIRE_COUNT) != MV_OK)
		return MV_FAIL;

	return MV_OK;
}

/*******************************************************************************
* mvEthPhyAutoNegoGet - Get auto nego of PHY.
*
* DESCRIPTION:
*       The API get duplex mode.
* INPUTS:
*       phyAddr       - Phy address.
*
* OUTPUT:
*       isEnable      - auto nego state
*                     0--disable
*                     1--enable
*
* RETURN:   MV_OK   - Success
*           MV_FAIL - Failure
*******************************************************************************/
MV_STATUS mvEthPhyAutoNegoGet(MV_U32 phyAddr, MV_BOOL *isEnable)
{
	MV_U16  phyRegData;

	if (mvEthPhyRegRead(phyAddr, ETH_PHY_CTRL_REG, &phyRegData) != MV_OK)
		return MV_FAIL;
	*isEnable = (phyRegData & ETH_PHY_CTRL_AN_ENABLE_MASK) ? 1 : 0;

	return MV_OK;
}


