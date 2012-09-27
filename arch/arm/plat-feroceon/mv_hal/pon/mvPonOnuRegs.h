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

#ifndef _ONU_GPON_REG_H
#define _ONU_GPON_REG_H

/* Include Files
------------------------------------------------------------------------------*/

/* Definitions
------------------------------------------------------------------------------*/
#define MV_ASIC_ONT_BASE (0)

//#define KW2_ASIC
/* Enums
------------------------------------------------------------------------------*/

/* ========================== */
/* = New ASIC Register Enum = */
/* ========================== */
typedef enum
{
/* Enumeration                         	                Description                        */
/* ======================================================================================= */
  mvAsicReg_Start                               = 0,

/******************************************************************************/
/******************************************************************************/
/* ========================================================================== */
/* ========================================================================== */
/* ==                                                                      == */
/* ==           =========   =========   =========   ===       ==           == */
/* ==           =========   =========   =========   ====      ==           == */
/* ==           ==          ==     ==   ==     ==   == ==     ==           == */
/* ==           ==          ==     ==   ==     ==   ==  ==    ==           == */
/* ==           =========   =========   ==     ==   ==   ==   ==           == */
/* ==           =========   =========   ==     ==   ==    ==  ==           == */
/* ==           ==     ==   ==          ==     ==   ==     == ==           == */
/* ==           ==     ==   ==          ==     ==   ==      ====           == */
/* ==           =========   ==          =========   ==       ===           == */
/* ==           =========   ==          =========   ==        ==           == */
/* ==                                                                      == */
/* ========================================================================== */
/* ========================================================================== */
/******************************************************************************/
/******************************************************************************/

  /* Interrupt Registers */
  /* =================== */
  mvAsicReg_GPON_INTERRUPT_PON                  = 1,    /* GponMainInterrupt               */
  mvAsicReg_GPON_INTERRUPT_PON_MASK             = 2,    /* GponMainInterruptMask           */
  mvAsicReg_BIP_INTR_INTERVAL                   = 3,    /* GponBipInterruptCfgInterval     */
  mvAsicReg_RX_BIP_STATUS_FOR_INTERRUPT         = 4,    /* GponBipInterruptStatAccumulator */
  mvAsicReg_RAM_TEST_CONFIG                     = 5,    /* RamTestCfg                      */

  /* General Registers */
  /* ================= */
  mvAsicReg_GPON_GEN_MAC_VERSION                = 6,    /* GenMACVersion                   */
  mvAsicReg_GPON_GEN_MAC_VERSION_ID             = 7,
  mvAsicReg_GPON_GEN_MAC_VERSION_MAC_ID         = 8,
  mvAsicReg_GPON_GEN_MAC_SCRATCH                = 9,    /* GenMACScratch                   */
  mvAsicReg_GPON_ONU_STATE                      = 10,   /* GenONUState                     */
  mvAsicReg_GPON_GEN_MICRO_SEC_CNT              = 11,   /* GenMicroSecCnt                  */
  mvAsicReg_GPON_TX_SERIAL_NUMBER               = 12,   /* GenSerialNumber                 */
  mvAsicReg_GPON_ONU_ID                         = 13,   /* GenONUId                        */
  mvAsicReg_GPON_ONU_ID_OID                     = 14,
  mvAsicReg_GPON_ONU_ID_V                       = 15,

  /* RX Registers */
  /* ============ */
  mvAsicReg_GPON_RX_CONFIG                      = 18,   /* GrxCfg                          */
  mvAsicReg_GPON_RX_CONFIG_EN                   = 19,
  mvAsicReg_GPON_RX_CONFIG_BIT_ORDER            = 20,
  mvAsicReg_GPON_RX_PSA_CONFIG                  = 21,   /* GrxCfgPsa                       */
  mvAsicReg_GPON_RX_PSA_CONFIG_SFM3             = 22,
  mvAsicReg_GPON_RX_PSA_CONFIG_SFM2             = 23,
  mvAsicReg_GPON_RX_PSA_CONFIG_SFM1             = 24,
  mvAsicReg_GPON_RX_PSA_CONFIG_FHM1             = 25,
  mvAsicReg_GPON_RX_FEC_CONFIG                  = 26,   /* GrxCfgFec                       */
  mvAsicReg_GPON_RX_FEC_CONFIG_SWVAL            = 27,
  mvAsicReg_GPON_RX_FEC_CONFIG_FSW              = 28,
  mvAsicReg_GPON_RX_FEC_CONFIG_IGNP             = 29,
  mvAsicReg_GPON_RX_FEC_CONFIG_IND              = 30,
  mvAsicReg_GPON_RX_FEC_STAT0                   = 31,   /* GrxStatFec0                     */
  mvAsicReg_GPON_RX_FEC_STAT1                   = 32,   /* GrxStatFec1                     */
  mvAsicReg_GPON_RX_FEC_STAT2                   = 33,   /* GrxStatFec2                     */
  mvAsicReg_GPON_RX_SFRAME_COUNTER              = 34,   /* GrxStatSuperFrameCnt            */
  mvAsicReg_GPON_RX_PLOAMD_DATA_READ            = 35,   /* GrxDataPloamRead                */
  mvAsicReg_GPON_RX_PLOAMD_DATA_USED            = 36,   /* GrxDataPloamUsed                */
  mvAsicReg_GPON_RX_PLOAMD_CONFIG               = 37,   /* GrxCfgPrm                       */
  mvAsicReg_GPON_RX_PLOAMD_CONFIG_IGNC          = 38,
  mvAsicReg_GPON_RX_PLOAMD_CONFIG_IGNB          = 39,
  mvAsicReg_GPON_RX_PLOAMD_CONFIG_FALL          = 40,
  mvAsicReg_GPON_RX_PLOAMD_LOST_CRC_STATUS      = 41,   /* GrxStatPrmLostCrcCnt            */
  mvAsicReg_GPON_RX_PLOAMD_LOST_FULL_STATUS     = 42,   /* GrxStatPrmLostFullCnt           */
  mvAsicReg_GPON_RX_PLOAMD_RCVD_IDLE_STATUS     = 43,   /* GrxStatPrmRcvdIdleCnt           */
  mvAsicReg_GPON_RX_PLOAMD_RCVD_BROAD_STATUS    = 44,   /* GrxStatPrmRcvdBroadCnt          */
  mvAsicReg_GPON_RX_PLOAMD_RCVD_MYID_STATUS     = 45,   /* GrxStatPrmRcvdMyIdCnt           */
  mvAsicReg_GPON_RX_BIP_STATUS                  = 46,   /* GrxStatBip                      */
  mvAsicReg_GPON_RX_PLEND_CONFIG                = 47,   /* GrxCfgPdb                       */
  mvAsicReg_GPON_RX_PLEND_CONFIG_FN             = 48,
  mvAsicReg_GPON_RX_PLEND_CONFIG_UF             = 49,
  mvAsicReg_GPON_RX_PLEND_CONFIG_IGND           = 50,
  mvAsicReg_GPON_RX_PLEND_CONFIG_DFIX           = 51,
  mvAsicReg_GPON_RX_PLEND_CONFIG_IGNC           = 52,
  mvAsicReg_GPON_RX_PLEND_STATUS                = 53,   /* GrxStatPdb                      */
  mvAsicReg_GPON_RX_BWMAP_CONFIG                = 54,   /* GrxCfgBmd                       */
  mvAsicReg_GPON_RX_BWMAP_CONFIG_MSD            = 55,
  mvAsicReg_GPON_RX_BWMAP_CONFIG_DFIX           = 56,
  mvAsicReg_GPON_RX_BWMAP_CONFIG_IGNC           = 57,
  mvAsicReg_GPON_RX_BWMAP_STATUS0               = 58,   /* GrxStatBmd0                     */
  mvAsicReg_GPON_RX_BWMAP_STATUS1               = 59,   /* GrxStatBmd1                     */
  mvAsicReg_GPON_RX_AES_CONFIG                  = 60,   /* GrxCfgAesKeySwitch              */
  mvAsicReg_GPON_RX_AES_CONFIG_TIME             = 61,   /* GrxStatAesKeySwitch             */
  mvAsicReg_GPON_RX_AES_CONFIG_SWS              = 62,
  mvAsicReg_GPON_RX_AES_STATUS                  = 63,
  mvAsicReg_GPON_RX_AES_STATUS_HST              = 64,
  mvAsicReg_GPON_RX_AES_STATUS_HWS              = 65,
  mvAsicReg_GPON_RX_EQULIZATION_DELAY           = 66,   /* GrxCfgEqDelay                   */
  mvAsicReg_GPON_RX_INTERNAL_DELAY              = 67,   /* GrxCfgInterDelay                */
  mvAsicReg_GPON_RX_BW_MAP                      = 68,   /* GrxCfgBmdMapTcont               */
  mvAsicReg_GPON_RX_BW_MAP_ALID                 = 60,
  mvAsicReg_GPON_RX_BW_MAP_TCN                  = 70,
  mvAsicReg_GPON_RX_BW_MAP_EN                   = 71,

  /* GEM Registers */
  /* ============= */
  mvAsicReg_GPON_GEM_STAT_IDLE_GEM_CNT          = 72,   /* GemStatIdleGEMCnt               */
  mvAsicReg_GPON_GEM_STAT_VALID_GEM_CNT         = 73,   /* GemStatValidGEMCnt              */
  mvAsicReg_GPON_GEM_STAT_UNDEF_GEM_CNT         = 74,   /* GemStatUndefGEMCnt              */
  mvAsicReg_GPON_GEM_STAT_VALID_OMCI_CNT        = 75,   /* GemStatValidOMCICnt             */
  mvAsicReg_GPON_GEM_STAT_DROPPED_GEM_CNT       = 76,   /* GemStatDroppedGEMCnt            */
  mvAsicReg_GPON_GEM_STAT_DROPPED_OMCI_CNT      = 77,   /* GemStatDroppedOMCICnt           */
  mvAsicReg_GPON_GEM_STAT_ERROR_GEM_CNT         = 78,   /* GemStatErrorGEMCnt              */
  mvAsicReg_GPON_GEM_STAT_ONE_FIXED_HEC_ERR_CNT = 79,   /* GemStatOneFixedHecErrCnt        */
  mvAsicReg_GPON_GEM_STAT_TWO_FIXED_HEC_ERR_CNT = 80,   /* GemStatTwoFixedHecErrCnt        */
  mvAsicReg_GPON_GEM_STAT_VALID_GEM_PAYLOAD_CNT = 81,   /* GemStatValidGEMPayload          */
  mvAsicReg_GPON_GEM_STAT_UNDEF_GEM_PAYLOAD_CNT = 82,   /* GemStatUndefGEMPayload          */
  mvAsicReg_GPON_GEM_STAT_RSMBL_MEM_FLUSH_CNT   = 83,   /* GemStatRsmblMemFlushCnt         */
  mvAsicReg_GPON_GEM_STAT_LCDGi_CNT             = 84,   /* GemStatLCDGiCnt                 */
  mvAsicReg_GPON_GEM_STAT_CORR_ETH_FRAME_CNT    = 85,   /* GemStatCorrEthFrameCnt          */
  mvAsicReg_GPON_GEM_STAT_ERR_ETH_FRAME_CNT     = 86,   /* GemStatErrEthFrameCnt           */
  mvAsicReg_GPON_GEM_STAT_CORR_OMCI_FRAME_CNT   = 87,   /* GemStatCorrOMCIFrameCnt         */
  mvAsicReg_GPON_GEM_STAT_ERR_OMCI_FRAME_CNT    = 88,   /* GemStatErrOMCIFrameCnt          */
  mvAsicReg_GPON_GEM_CONFIG_PLI                 = 89,   /* GemCfgPLI                       */
  mvAsicReg_GPON_GEM_CONFIG_ETH_FRAME_LEN       = 90,   /* GemCfgEthLength                 */
  mvAsicReg_GPON_GEM_CONFIG_ETH_FRAME_LEN_MAX   = 91,
  mvAsicReg_GPON_GEM_CONFIG_ETH_FRAME_LEN_MIN   = 92,
  mvAsicReg_GPON_GEM_CONFIG_OMCI_FRAME_LEN      = 93,   /* GemCfgOMCILength                */
  mvAsicReg_GPON_GEM_CONFIG_OMCI_FRAME_LEN_MAX  = 94,
  mvAsicReg_GPON_GEM_CONFIG_OMCI_FRAME_LEN_MIN  = 95,
  mvAsicReg_GPON_GEM_CONFIG_OMCI_PORT           = 96,   /* GemCfgOMCIPort                  */
  mvAsicReg_GPON_GEM_CONFIG_OMCI_PORT_ID        = 97,
  mvAsicReg_GPON_GEM_CONFIG_OMCI_PORT_VALID     = 98,
  mvAsicReg_GPON_GEM_CONFIG_AES                 = 99,   /* GemCfgAES                       */
  mvAsicReg_GPON_GEM_AES_SHADOW_KEY             = 100,   /* GemCfgAESShadowKey0/1/2/3       */
  mvAsicReg_GPON_GEM_CFG_IPG_VALUE              = 101,   /* GemCfgIPGValue                  */
  mvAsicReg_GPON_GEM_CFG_IPG_VALID              = 102,  /* GemCfgIPGValid                  */

  /* TX Registers */
  /* ============ */
  mvAsicReg_GPON_TX_PREAMBLE                    = 103,   /* GtxCfgPreamble0/1/2             */
  mvAsicReg_GPON_TX_PREAMBLE_PATT               = 104,
  mvAsicReg_GPON_TX_PREAMBLE_SIZE               = 105,
  mvAsicReg_GPON_TX_DELIMITER                   = 106,  /* GtxCfgDelimiter                 */
  mvAsicReg_GPON_TX_DELIMITER_PAT               = 107,
  mvAsicReg_GPON_TX_DELIMITER_SIZE              = 108,
  mvAsicReg_GPON_TX_BURST_EN_PARAM              = 109,  /* GtxCfgBurstEnable               */
  mvAsicReg_GPON_TX_BURST_EN_PARAM_STR          = 110,
  mvAsicReg_GPON_TX_BURST_EN_PARAM_STP          = 111,
  mvAsicReg_GPON_TX_BURST_EN_PARAM_DLY          = 112,
  mvAsicReg_GPON_TX_BURST_EN_PARAM_P            = 113,
  mvAsicReg_GPON_TX_BURST_EN_PARAM_MASK         = 114,
  mvAsicReg_GPON_TX_FDELAY                      = 115,  /* GtxCfgFinelDelay                */
  mvAsicReg_GPON_TX_PLOAM_DATA_WRITE            = 116,  /* GtxDataPloamWrite               */
  mvAsicReg_GPON_TX_PLOAM_DATA_WRITE_FREE       = 117,  /* GtxDataPloamStatus              */
  mvAsicReg_GPON_TX_PLS_CONSTANT                = 118,  /* GtxCfgPls                       */
  mvAsicReg_GPON_TX_DBR_REPORT_BLOCK_IDX        = 119,  /* GtxCfgDbr                       */
  mvAsicReg_GPON_TX_FE_RDI_INDICATION           = 120,  /* GtxCfgRdi                       */
  mvAsicReg_GPON_TX_STAT_GEM_PTI1               = 121,  /* GtxStatGemPti1                  */
  mvAsicReg_GPON_TX_STAT_GEM_PTI0               = 122,  /* GtxStatGemPti0                  */
  mvAsicReg_GPON_TX_STAT_GEM_IDLE               = 123,  /* GtxStatGemIdle                  */
  mvAsicReg_GPON_TX_STAT_TX_EN_CNT              = 124,  /* GtxStatTxEnableCounter          */
  mvAsicReg_GPON_TX_CONFIG_EN_THRESHOLD         = 125,  /* GtxCfgTxEnableThreshold         */
  mvAsicReg_GPON_TX_GSE_TRANS_THRESHOLD         = 126,  /* GtxCfgGseTransThreshold         */
  mvAsicReg_GPON_TX_CFG_AC_COUPLING             = 127,  /* GtxCfgAcCoupling                */
  mvAsicReg_GPON_TX_STAT_TCONT_i_ETH_FRAMES     = 129,  /* GtxStatTcontiEtherFrames        */
  mvAsicReg_GPON_TX_STAT_TCONT_i_ETH_BYTES      = 130,  /* GtxStatTcontiEtherBytes         */
  mvAsicReg_GPON_TX_STAT_TCONT_i_GEM_FRAMES     = 131,  /* GtxStatTcontiGemFrames          */
  mvAsicReg_GPON_TX_STAT_TCONT_i_IDLE_GEM       = 132,  /* GtxStatTcontiIdleGem            */

  /* UTM Registers */
  /* ============= */
  mvAsicReg_GPON_UTM_CONFIG_OMCI_PORT_ID        = 133,  /* UtmCfgOmciPnum                  */
  mvAsicReg_GPON_UTM_CONFIG_OMCI_PORT_VALID     = 134,  /* UtmCfgOmciPvalid                */
  mvAsicReg_GPON_UTM_CONFIG_TC_PERIOD           = 135,  /* UtmCfgTcPeriod                  */
  mvAsicReg_GPON_UTM_CONFIG_TC_PERIOD_VALID     = 136,  /* UtmCfgTcPeriodValid             */

  /* SGL Registers */
  /* ============= */
  mvAsicReg_GPON_SGL_SW_RESET                   = 139,  /* SglSwResetReg                   */
  mvAsicReg_GPON_SGL_CONFIG_REG                 = 140,  /* SglCfgReg                       */
  mvAsicReg_GPON_SGL_STATS_REG                  = 141,  /* SglStatReg                      */

  /*  Memory Registers           */
  /* =========================== */
  mvAsicReg_GPON_TX_CONST_DATA_RAM              = 142,
  mvAsicReg_GPON_TX_CONST_DATA_RAM_IDLE         = 143,
  mvAsicReg_GPON_TX_CONST_DATA_RAM_SN           = 144,
  mvAsicReg_GPON_GEM_AES_PID_TABLE              = 145,
  mvAsicReg_GPON_GEM_VALID_PID_TABLE            = 146,
  mvAsicReg_GPON_TEST                           = 147,

  /*  GPON MIB Counter Set       */
  /* =========================== */
  mvAsicReg_PON_MAC_MIB_COUNTERS_0              = 148,
  mvAsicReg_PON_MAC_MIB_COUNTERS_1              = 149,
  mvAsicReg_PON_MAC_MIB_COUNTERS_2              = 150,
  mvAsicReg_PON_MAC_MIB_COUNTERS_3              = 151,
  mvAsicReg_PON_MAC_MIB_COUNTERS_4              = 152,
  mvAsicReg_PON_MAC_MIB_COUNTERS_5              = 153,
  mvAsicReg_PON_MAC_MIB_COUNTERS_6              = 154,
  mvAsicReg_PON_MAC_MIB_COUNTERS_7              = 155,

  /*  GPON MIB Counter Control Registers */
  /* =================================== */
  mvAsicReg_RX_MIB_CTRL                         = 156,
  mvAsicReg_RX_MIB_DEFAULT                      = 157,


/******************************************************************************/
/******************************************************************************/
/* ========================================================================== */
/* ========================================================================== */
/* ==                                                                      == */
/* ==           =========   =========   =========   ===       ==           == */
/* ==           =========   =========   =========   ====      ==           == */
/* ==           ==          ==     ==   ==     ==   == ==     ==           == */
/* ==           ==          ==     ==   ==     ==   ==  ==    ==           == */
/* ==           =========   =========   ==     ==   ==   ==   ==           == */
/* ==           =========   =========   ==     ==   ==    ==  ==           == */
/* ==           ==          ==          ==     ==   ==     == ==           == */
/* ==           ==          ==          ==     ==   ==      ====           == */
/* ==           =========   ==          =========   ==       ===           == */
/* ==           =========   ==          =========   ==        ==           == */
/* ==                                                                      == */
/* ========================================================================== */
/* ========================================================================== */
/******************************************************************************/
/******************************************************************************/

  /*  EPON Interrupt Registers   */
  /* =========================== */
  mvAsicReg_EPON_INTR_REG                            = 180,
  mvAsicReg_EPON_INTR_MASK                           = 181,

  /*  EPON General Registers     */
  /* =========================== */
  mvAsicReg_EPON_GEN_MAC_VERSION_ADDR                = 184,
  mvAsicReg_EPON_GEN_ONT_ENABLE                      = 185,
  mvAsicReg_EPON_GEN_ONT_RX_ENABLE                   = 186,
  mvAsicReg_EPON_GEN_ONT_TX_ENABLE                   = 187,
  mvAsicReg_EPON_GEN_ONT_STATE                       = 188,
  mvAsicReg_EPON_GEN_ONT_STATE_REG_AUTO_EN           = 189,
  mvAsicReg_EPON_GEN_ONT_STATE_REG                   = 190,
  mvAsicReg_EPON_GEN_ONT_STATE_REREG_AUTO_EN         = 191,
  mvAsicReg_EPON_GEN_ONT_STATE_DEREG_AUTO_EN         = 192,
  mvAsicReg_EPON_GEN_ONT_STATE_NACK_AUTO_EN          = 193,
  mvAsicReg_EPON_GEN_TQ_SIZE                         = 194,
  mvAsicReg_EPON_GEN_LASER_PARAM                     = 195,
  mvAsicReg_EPON_GEN_TAIL_GUARD                      = 196,
  mvAsicReg_EPON_GEN_SYNC_TIME                       = 197,
  mvAsicReg_EPON_GEN_BROADCAST_ADDR_LOW              = 198,
  mvAsicReg_EPON_GEN_BROADCAST_ADDR_HIGH             = 199,
  mvAsicReg_EPON_GEN_CONFIGURATION                   = 200,
  mvAsicReg_EPON_GEN_CONFIGURATION_AUTO_REPORT       = 201,
  mvAsicReg_EPON_GEN_CONFIGURATION_AUTO_ACK          = 202,
  mvAsicReg_EPON_GEN_CONFIGURATION_AUTO_REQUEST      = 203,
  mvAsicReg_EPON_GEN_SLD                             = 204,
  mvAsicReg_EPON_GEN_MAC_CONTROL_TYPE                = 205,
  mvAsicReg_EPON_GEN_LOCAL_TIMESTAMP                 = 206,
  mvAsicReg_EPON_GEN_NUM_OF_PENDING_GRANTS           = 207,
  mvAsicReg_EPON_GEN_SGL_STATUS                      = 208,
  mvAsicReg_EPON_GEN_PON_PHY_DEBUG                   = 209,
  mvAsicReg_EPON_GEN_SGL_SW_RESET                    = 210,
  mvAsicReg_EPON_GEN_SGL_CONFIG                      = 211,
  mvAsicReg_EPON_GEN_SGL_DEBOUNCE_CONFIG             = 212,
  mvAsicReg_EPON_GEN_SGL_DEBOUNCE_ENABLE             = 213,
  mvAsicReg_EPON_GEN_TCH_CHURNING_KEY                = 214,
  mvAsicReg_EPON_GEN_ADDITIONAL_OPCODE1              = 215,
  mvAsicReg_EPON_GEN_ADDITIONAL_OPCODE2              = 216,
  mvAsicReg_EPON_GEN_DRIFT_THRESHOLD                 = 217,
  mvAsicReg_EPON_GEN_TIMESTAMP_VAL_FOR_INTR          = 218,
  mvAsicReg_EPON_GEN_ADDITIONAL_OPCODE3              = 219,
#ifdef PON_Z2
  mvAsicReg_EPON_GEN_LLID_CRC_CONFIG                 = 220,
  mvAsicReg_EPON_GEN_TIMESTAMP_CONFIG                = 221,
  mvAsicReg_EPON_GEN_ONT_RX_TIMESTAMP_CONFIG         = 222,
  mvAsicReg_EPON_GEN_ONT_REGISTER_PACKET_PADDING     = 223,
#endif /* PON_Z2 */
  mvAsicReg_EPON_GEN_MAC_ADDR_LOW                    = 224,
  mvAsicReg_EPON_GEN_MAC_ADDR_HIGH                   = 225,
  mvAsicReg_EPON_GEN_UTM_TC_PERIOD                   = 226,
  mvAsicReg_EPON_GEN_UTM_TC_PERIOD_VALID             = 227,
  mvAsicReg_EPON_GEN_GEM_IPG_VAL                     = 228,
  mvAsicReg_EPON_GEN_GEM_IPG_VAL_VALID               = 229,

  /*  EPON RXP Registers         */
  /* =========================== */
  mvAsicReg_EPON_RXP_PACKET_SIZE_LIMIT               = 233,
  mvAsicReg_EPON_RXP_PACKET_FILTER                   = 234,
  mvAsicReg_EPON_RXP_CTRL_FRAME_FORWARD              = 235,
  mvAsicReg_EPON_RXP_LLT_LLID_DATA                   = 236,
  mvAsicReg_EPON_RXP_LLT_LLID_DATA_VALUE             = 237,
  mvAsicReg_EPON_RXP_LLT_LLID_DATA_INDEX             = 238,
  mvAsicReg_EPON_RXP_LLT_LLID_DATA_VALID             = 239,
  mvAsicReg_EPON_RXP_ENCRYPTION_CONFIG               = 240,
  mvAsicReg_EPON_RXP_ENCRYPTION_KEY0                 = 241,
  mvAsicReg_EPON_RXP_ENCRYPTION_KEY1                 = 242,
  mvAsicReg_EPON_RXP_DATA_FIFO_THRESH                = 243,

  /*  EPON GPM Registers         */
  /* =========================== */
  mvAsicReg_EPON_GPM_MAX_FUTURE_GRANT_TIME           = 250,
  mvAsicReg_EPON_GPM_MIN_PROCESSING_TIME             = 251,
  mvAsicReg_EPON_GPM_DISCOVERY_GRANT_LENGTH          = 252,
  mvAsicReg_EPON_GPM_RX_SYNC_TIME                    = 253,
  mvAsicReg_EPON_GPM_GRANT_VALID                     = 254,
  mvAsicReg_EPON_GPM_GRANT_MAX_FUTURE_TIME_ERR       = 255,
  mvAsicReg_EPON_GPM_MIN_PROC_TIME_ERR               = 256,
  mvAsicReg_EPON_GPM_LENGTH_ERR                      = 257,
  mvAsicReg_EPON_GPM_DISCOVERY_AND_REGISTERED_ERR    = 258,
  mvAsicReg_EPON_GPM_FIFO_FULL_ERR                   = 259,
  mvAsicReg_EPON_GPM_OPC_DISC_NOT_REG_BCAST          = 260,
  mvAsicReg_EPON_GPM_OPC_REG_NOT_DISC                = 261,
  mvAsicReg_EPON_GPM_OPC_DISC_NOT_REG_NOT_BCAST      = 262,
  mvAsicReg_EPON_GPM_OPC_DROPED_GRANT                = 263,
  mvAsicReg_EPON_GPM_OPC_HIDDEN_GRANT                = 264,
  mvAsicReg_EPON_GPM_OPC_BACK_TO_BACK_GRANT          = 265,

  /*  EPON TXM Registers         */
  /* =========================== */
  mvAsicReg_EPON_TXM_DEFAULT_OVERHEAD                = 266,
  mvAsicReg_EPON_TXM_CONFIGURATION                   = 267,
  mvAsicReg_EPON_TXM_TX_LLID                         = 269,
  mvAsicReg_EPON_TXM_CPP_RPRT_CONFIG                 = 270,
  mvAsicReg_EPON_TXM_CPP_RPRT_BIT_MAP                = 271,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_SET              = 272,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_0                = 273,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_1                = 274,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_2                = 275,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_3                = 276,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_4                = 277,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_5                = 278,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_6                = 279,
  mvAsicReg_EPON_TXM_CPP_RPRT_QUEUE_7                = 280,
  mvAsicReg_EPON_TXM_CPP_RPRT_FEC_CONFIG             = 281,

  /*  EPON PCS Registers         */
  /* =========================== */
  mvAsicReg_EPON_PCS_CONFIGURATION                   = 284,
  mvAsicReg_EPON_PCS_DELAY_CONFIG                    = 285,
  mvAsicReg_EPON_PCS_STATS_FEC_0                     = 286,
  mvAsicReg_EPON_PCS_STATS_FEC_1                     = 287,
  mvAsicReg_EPON_PCS_STATS_FEC_2                     = 288,
  mvAsicReg_EPON_PCS_STATS_0                         = 290,
  mvAsicReg_EPON_PCS_STATS_1                         = 291,
  mvAsicReg_EPON_PCS_STATS_2                         = 292,
  mvAsicReg_EPON_PCS_STATS_3                         = 293,
  mvAsicReg_EPON_PCS_STATS_4                         = 294,
  mvAsicReg_EPON_PCS_FRAME_SZ_LIMITS                 = 295,

  /*  EPON DDM Registers         */
  /* =========================== */
  mvAsicReg_EPON_DDM_DELAY_CONFIG                    = 296,
  mvAsicReg_EPON_DDM_TX_POLARITY                     = 297,
  /* statistics */
  mvAsicReg_EPON_STAT_RXP_FCS_ERROR_CNT              = 298,
  mvAsicReg_EPON_STAT_RXP_SHORT_ERROR_CNT            = 299,
  mvAsicReg_EPON_STAT_RXP_LONG_ERROR_CNT             = 300,
  mvAsicReg_EPON_STAT_RXP_DATA_FRAMES_CNT            = 301,
  mvAsicReg_EPON_STAT_RXP_CTRL_FRAMES_CNT            = 302,
  mvAsicReg_EPON_STAT_RXP_REPORT_FRAMES_CNT          = 303,
  mvAsicReg_EPON_STAT_RXP_GATE_FRAMES_CNT            = 304,
  mvAsicReg_EPON_STAT_TXP_CTRL_REG_REQ_FRAMES_CNT    = 305,
  mvAsicReg_EPON_STAT_TXP_CTRL_REG_ACK_FRAMES_CNT    = 306,
  mvAsicReg_EPON_STAT_TXP_CTRL_REPORT_FRAMES_CNT     = 307,
  mvAsicReg_EPON_STAT_TXP_DATA_FRAMES_CNT            = 308,
  mvAsicReg_EPON_STAT_TXP_TX_ALLOWED_BYTE_CNT        = 309,

  /*  EPON Control Packet queue Registers   */
  /* =====================================  */
  mvAsicReg_EPON_CPQ_RX_CTRL_Q_READ                  = 310,
  mvAsicReg_EPON_CPQ_RX_CTRL_Q_USED                  = 311,
  mvAsicReg_EPON_CPQ_RX_RPRT_Q_READ                  = 312,
  mvAsicReg_EPON_CPQ_RX_RPRT_Q_USED                  = 313,
  mvAsicReg_EPON_CPQ_RX_CTRL_HQ_READ_L               = 314,
  mvAsicReg_EPON_CPQ_RX_CTRL_HQ_READ_H               = 315,
  mvAsicReg_EPON_CPQ_RX_CTRL_HQ_USED                 = 316,
  mvAsicReg_EPON_CPQ_RX_RPRT_HQ_READ_L               = 317,
  mvAsicReg_EPON_CPQ_RX_RPRT_HQ_READ_H               = 318,
  mvAsicReg_EPON_CPQ_RX_RPRT_HQ_USED                 = 319,
  mvAsicReg_EPON_CPQ_TX_CTRL_Q_WRITE                 = 320,
  mvAsicReg_EPON_CPQ_TX_CTRL_Q_FREE                  = 321,
  mvAsicReg_EPON_CPQ_TX_CTRL_HQ_WRITE                = 322,
  mvAsicReg_EPON_CPQ_TX_CTRL_HQ_FREE                 = 323,


  /*  P2P Registers                         */
  /* =====================================  */
  mvAsicReg_P2P_GEN_ONT_MODE                         = 330,
  mvAsicReg_P2P_PCS_CONFIGURATION                    = 331,
  mvAsicReg_P2P_TXM_CFG_MODE                         = 332,

#ifndef PON_FPGA
  mvAsicReg_PON_SERDES_PHY_CTRL_0                    = 333,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_Pll             = 334,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_RX              = 335,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_PU_TX              = 336,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_RST                = 337,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_RST_TX_DOUT        = 338,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_INIT            = 339,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_RX_RATE            = 340,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_SEL_GEPON          = 341,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_SEL_REF_CLK        = 342,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_CID_REV            = 343,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_TX_RATE            = 344,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_REF_CLK_25M        = 345,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_INIT_DONE          = 346,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_READY_TX           = 347,
  mvAsicReg_PON_SERDES_PHY_CTRL_0_READY_RX           = 348,
  mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_IO_EN          = 349,
  mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_EN    = 350,
  mvAsicReg_PON_SERDES_PHY_CTRL_1_FORCE_BEN_IO_VAL   = 351,
  mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_HW_SELECT   = 352,
  mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_FORCE       = 353,
  mvAsicReg_PON_SERDES_PHY_CTRL_1_BEN_SW_VALUE       = 354,
  mvAsicReg_PON_SERDES_CLK_SEL                       = 355,
  mvAsicReg_PON_SERDES_CLK_EN                        = 356,
  mvAsicReg_PON_SERDES_XPON_CTRL                     = 357,
  mvAsicReg_PON_SERDES_INTERNAL_PASSWORD             = 358,
  mvAsicReg_PON_SERDES_INTERNAL_EN_LOOP_TIMING       = 359,
  mvAsicReg_PON_SERDES_INTERNAL_PON_SELECT           = 360,
  mvAsicReg_PON_SERDES_INTERNAL_OPEN_TX_DOOR         = 361,
  mvAsicReg_PON_MAC_GPON_CLK_EN                      = 362,
  mvAsicReg_PON_MAC_SW_RESET_CTRL                    = 363,
  mvAsicReg_PON_MPP_00                               = 364,
  mvAsicReg_PON_MPP_01                               = 365,
  mvAsicReg_PON_MPP_02                               = 366,
  mvAsicReg_PON_MPP_03                               = 367,
  mvAsicReg_PON_MPP_04                               = 368,
  mvAsicReg_PON_MPP_05                               = 369,
  mvAsicReg_PON_MPP_06                               = 370,
  mvAsicReg_PON_MPP_07                               = 371,
  mvAsicReg_PON_MPP_08                               = 372,
  mvAsicReg_PON_MPP_09                               = 373,
  mvAsicReg_PON_MPP_10                               = 374,
  mvAsicReg_PON_MPP_11                               = 375,
  mvAsicReg_PON_MPP_12                               = 376,
  mvAsicReg_PON_MPP_13                               = 377,
  mvAsicReg_PON_MPP_14                               = 378,
  mvAsicReg_PON_MPP_15                               = 379,
  mvAsicReg_PON_MPP_16                               = 380,
  mvAsicReg_PON_MPP_17                               = 381,
  mvAsicReg_PON_MPP_18                               = 382,
  mvAsicReg_PON_MPP_19                               = 383,
  mvAsicReg_PON_MPP_20                               = 384,
  mvAsicReg_PON_MPP_21                               = 385,
  mvAsicReg_PON_MPP_22                               = 386,
  mvAsicReg_PON_MPP_23                               = 387,
  mvAsicReg_PON_MPP_24                               = 388,
  mvAsicReg_PON_MPP_25                               = 389,
  mvAsicReg_PON_MPP_26                               = 390,
  mvAsicReg_PON_MPP_27                               = 391,
  mvAsicReg_PON_MPP_28                               = 392,
  mvAsicReg_PON_MPP_29                               = 393,
  mvAsicReg_PON_MPP_30                               = 394,
  mvAsicReg_PON_MPP_31                               = 395,
  mvAsicReg_PON_MPP_32                               = 396,
  mvAsicReg_PON_MPP_33                               = 397,
  mvAsicReg_PON_MPP_34                               = 398,
  mvAsicReg_PON_MPP_35                               = 399,
  mvAsicReg_PON_MPP_36                               = 400,
  mvAsicReg_PON_MPP_37                               = 401,
  mvAsicReg_PON_MPP_38                               = 402,
  mvAsicReg_PON_MPP_39                               = 403,
  mvAsicReg_PON_MPP_40                               = 404,
  mvAsicReg_PON_MPP_41                               = 405,
  mvAsicReg_PON_MPP_42                               = 406,
  mvAsicReg_PON_MPP_43                               = 407,
  mvAsicReg_PON_MPP_44                               = 408,
  mvAsicReg_PON_MPP_45                               = 409,
  mvAsicReg_PON_MPP_46                               = 410,
  mvAsicReg_PON_MPP_47                               = 411,
  mvAsicReg_PON_MPP_48                               = 412,
  mvAsicReg_PON_MPP_49                               = 413,
  mvAsicReg_PON_MPP_50                               = 414,
  mvAsicReg_PON_MPP_51                               = 415,
  mvAsicReg_PON_MPP_52                               = 416,
  mvAsicReg_PON_MPP_53                               = 417,
  mvAsicReg_PON_MPP_54                               = 418,
  mvAsicReg_PON_MPP_55                               = 419,
  mvAsicReg_PON_MPP_56                               = 420,
  mvAsicReg_PON_MPP_57                               = 421,
  mvAsicReg_PON_MPP_58                               = 422,
  mvAsicReg_PON_MPP_59                               = 423,
  mvAsicReg_PON_MPP_60                               = 424,
  mvAsicReg_PON_MPP_61                               = 425,
  mvAsicReg_PON_MPP_62                               = 426,
  mvAsicReg_PON_MPP_63                               = 427,
  mvAsicReg_PON_MPP_64                               = 428,
  mvAsicReg_PON_MPP_65                               = 429,
  mvAsicReg_PON_MPP_66                               = 430,
  mvAsicReg_PON_MPP_67                               = 431,
  mvAsicReg_PON_MPP_68                               = 432,
  mvAsicReg_PON_MPP_69                               = 433,
  mvAsicReg_PON_XVR_TX_DATA_OUT_17                   = 434,
  mvAsicReg_PON_XVR_TX_DATA_OUT_37                   = 435,
  mvAsicReg_PON_XVR_TX_DATA_OUT_68                   = 436,
  mvAsicReg_PON_XVR_TX_OUTPUT_ENABLE_17              = 437,
  mvAsicReg_PON_XVR_TX_OUTPUT_ENABLE_37              = 438,
  mvAsicReg_PON_XVR_TX_OUTPUT_ENABLE_68              = 439,
  mvAsicReg_PON_SERDES_INTERNAL_OPEN_TX_DOOR_15      = 440,
  mvAsicReg_PON_SERDES_POWER_PHY_MODE                = 441,
  mvAsicReg_PON_SYS_LED_ENABLE                       = 442,
  mvAsicReg_PON_SYS_LED_STATE                        = 443,
  mvAsicReg_PON_SYS_LED_BLINK                        = 444,
  mvAsicReg_PON_SYS_LED_BLINK_FREQ                   = 445,
  mvAsicReg_PON_SYNC_LED_ENABLE                      = 446,
  mvAsicReg_PON_SYNC_LED_STATE                       = 447,
  mvAsicReg_PON_SYNC_LED_BLINK                       = 448,
  mvAsicReg_PON_SYNC_LED_BLINK_FREQ                  = 449,
  mvAsicReg_PON_LED_BLINK_FREQ_A_ON                  = 450,
  mvAsicReg_PON_LED_BLINK_FREQ_A_OFF                 = 451,
  mvAsicReg_PON_LED_BLINK_FREQ_B_ON                  = 452,
  mvAsicReg_PON_LED_BLINK_FREQ_B_OFF                 = 453,
  mvAsicReg_PON_DG_CTRL_EN                           = 454,
  mvAsicReg_PON_DG_CTRL_POLARITY                     = 455,
  mvAsicReg_PON_DG_THRESHOLD                         = 456,
  mvAsicReg_PT_PATTERN_SELECT                        = 457,
  mvAsicReg_PT_PATTERN_ENABLED                       = 458,
  mvAsicReg_PT_PATTERN_DATA                          = 459,

  mvAsicReg_GUNIT_TX_0_QUEUES                        = 460,
  mvAsicReg_GUNIT_TX_1_QUEUES                        = 461,
  mvAsicReg_GUNIT_TX_2_QUEUES                        = 462,
  mvAsicReg_GUNIT_TX_3_QUEUES                        = 463,
  mvAsicReg_GUNIT_TX_4_QUEUES                        = 464,
  mvAsicReg_GUNIT_TX_5_QUEUES                        = 465,
  mvAsicReg_GUNIT_TX_6_QUEUES                        = 466,
  mvAsicReg_GUNIT_TX_7_QUEUES                        = 467,

  mvAsicReg_GUNIT_TX_0_PKT_MOD_MAX_HEAD_SIZE_CFG     = 470,
  mvAsicReg_GUNIT_TX_1_PKT_MOD_MAX_HEAD_SIZE_CFG     = 471,
  mvAsicReg_GUNIT_TX_2_PKT_MOD_MAX_HEAD_SIZE_CFG     = 472,
  mvAsicReg_GUNIT_TX_3_PKT_MOD_MAX_HEAD_SIZE_CFG     = 473,
  mvAsicReg_GUNIT_TX_4_PKT_MOD_MAX_HEAD_SIZE_CFG     = 474,
  mvAsicReg_GUNIT_TX_5_PKT_MOD_MAX_HEAD_SIZE_CFG     = 475,
  mvAsicReg_GUNIT_TX_6_PKT_MOD_MAX_HEAD_SIZE_CFG     = 476,
  mvAsicReg_GUNIT_TX_7_PKT_MOD_MAX_HEAD_SIZE_CFG     = 477,

  mvAsicReg_GUNIT_TX_0_PKT_MOD_STATS_PKT_COUNT       = 480,
  mvAsicReg_GUNIT_TX_1_PKT_MOD_STATS_PKT_COUNT       = 481,
  mvAsicReg_GUNIT_TX_2_PKT_MOD_STATS_PKT_COUNT       = 482,
  mvAsicReg_GUNIT_TX_3_PKT_MOD_STATS_PKT_COUNT       = 483,
  mvAsicReg_GUNIT_TX_4_PKT_MOD_STATS_PKT_COUNT       = 484,
  mvAsicReg_GUNIT_TX_5_PKT_MOD_STATS_PKT_COUNT       = 485,
  mvAsicReg_GUNIT_TX_6_PKT_MOD_STATS_PKT_COUNT       = 486,
  mvAsicReg_GUNIT_TX_7_PKT_MOD_STATS_PKT_COUNT       = 487,

#endif /* PON_FPGA */
  mvAsicReg_MAX_NUM_OF_REGS

} E_asicGlobalRegs;

/* Register access enumeration */
typedef enum
{
  asicRO = 0x01,
  asicWO = 0x02,
  asicRW = 0x03
} E_asicAccessType;

typedef enum
{
  funcRegR = 0x00,
  funcRegW = 0x01
} E_asicRegFuncType;

/* Typedefs
------------------------------------------------------------------------------*/
typedef struct
{
  E_asicGlobalRegs enumVal;          /* The enumeration value of the Register */
  MV_U32           address;          /* The absolute address of the Register */
  MV_U32           offset;           /* The relative address of the Register */
  E_asicAccessType accessType;       /* Access Type: Read Only, Write Only, Read/Write */
  MV_U32           mask;             /* Mask for sub-fields Register */
  MV_U32           shift;            /* Shift field location for sub-fields Register */
  MV_U32           tblLength;        /* Number of entries in the table, Only relevant for tables */
  MV_U32           tblEntrySize;     /* The length in interval of 32bits of a table entry, Only relevant for tables */
  MV_U32           accessCount;      /* Number accesses to the specific Register */
  MV_U8            description[64];  /* Description */
} S_asicGlobalRegDb;

/* Global variables
------------------------------------------------------------------------------*/

/* Global functions
------------------------------------------------------------------------------*/
MV_STATUS asicOntGlbRegRead(E_asicGlobalRegs reg, MV_U32 *pvalue_p, MV_U32 entry);
MV_STATUS asicOntGlbRegWrite(E_asicGlobalRegs reg, MV_U32 value, MV_U32 entry);
MV_STATUS asicOntGlbRegReadNoCheck(E_asicGlobalRegs reg, MV_U32 *pvalue, MV_U32 entry);
MV_STATUS asicOntGlbRegWriteNoCheck (E_asicGlobalRegs reg, MV_U32 value, MV_U32 entry);
MV_STATUS asicOntMiscRegWrite(E_asicGlobalRegs reg, MV_U32 value, MV_U32 entry);
MV_STATUS asicOntMiscRegRead(E_asicGlobalRegs reg, MV_U32 *pvalue, MV_U32 entry);

/* Macros
------------------------------------------------------------------------------*/
#define PON_GPIO_NOT_USED (0xFFFF)
#define PON_GPIO_GET(gppClass, gpioGroup, gpioMask)\
{\
  MV_32 gpioNum;\
  gpioNum = mvBoarGpioPinNumGet(gppClass, 0);\
  if (gpioNum < 0)\
  {\
    gpioGroup = PON_GPIO_NOT_USED;\
    gpioMask  = PON_GPIO_NOT_USED;\
  }\
  else\
  {\
    gpioGroup = gpioNum >> 5;\
    gpioMask  = (1 << (gpioNum & 0x1F));\
  }\
}

#endif /* _ONU_GPON_REG_H */
