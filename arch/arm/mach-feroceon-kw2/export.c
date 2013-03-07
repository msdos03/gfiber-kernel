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

#include "ctrlEnv/mvCtrlEnvLib.h"
#include "ctrlEnv/sys/mvCpuIf.h"
#include "boardEnv/mvBoardEnvLib.h"
#include "mvDebug.h"
#include "mvSysHwConfig.h"
#include "pex/mvPexRegs.h"
#include "cntmr/mvCntmr.h"
#include "ctrlEnv/mvCtrlEnvLib.h"
#include "pnc/mvPnc.h"
#include "mvOs.h"

#include "mv_neta/net_dev/mv_netdev.h"

/*************************************************************************************************************
 * Environment
 *************************************************************************************************************/
extern u32 mvTclk;
extern u32 mvSysclk;
extern u8  mvMacAddr[CONFIG_MV_ETH_PORTS_NUM][6];

#include "mv_neta/net_dev/mv_netdev.h"

EXPORT_SYMBOL(pnc_mh_omci);
EXPORT_SYMBOL(pnc_eoam);
EXPORT_SYMBOL(pnc_default_init);
EXPORT_SYMBOL(pnc_def_entry_init);
EXPORT_SYMBOL(mv_eth_dev_name_by_portid);
EXPORT_SYMBOL(mv_eth_port_ifname_print);
EXPORT_SYMBOL(mv_eth_pon_ifname_dump);
EXPORT_SYMBOL(mvNetaMaxCheck);
EXPORT_SYMBOL(mv_eth_rx_special_proc_func);
EXPORT_SYMBOL(mvNetaTxpCheck);
EXPORT_SYMBOL(mv_eth_port_by_id);
EXPORT_SYMBOL(mv_eth_tx_special_check_func);

EXPORT_SYMBOL(mv_early_printk);
EXPORT_SYMBOL(mvCtrlPwrClckGet);
EXPORT_SYMBOL(mvCtrlModelRevGet);
EXPORT_SYMBOL(mvTclk);
EXPORT_SYMBOL(mvSysclk);
EXPORT_SYMBOL(mvCtrlModelGet);
EXPORT_SYMBOL(mvCtrlRevGet);
EXPORT_SYMBOL(mvOsIoUncachedMalloc);
EXPORT_SYMBOL(mvOsIoUncachedFree);
EXPORT_SYMBOL(mvOsIoCachedMalloc);
EXPORT_SYMBOL(mvOsIoCachedFree);
EXPORT_SYMBOL(mvDebugMemDump);
EXPORT_SYMBOL(mvHexToBin);
EXPORT_SYMBOL(mvBinToHex);
EXPORT_SYMBOL(mvSizePrint);
EXPORT_SYMBOL(mvDebugPrintMacAddr);
EXPORT_SYMBOL(mvCtrlEthMaxPortGet);
EXPORT_SYMBOL(mvCtrlTargetNameGet);
EXPORT_SYMBOL(mvBoardIdGet);
EXPORT_SYMBOL(mvBoardPhyAddrGet);
EXPORT_SYMBOL(mvCpuIfTargetWinGet);
EXPORT_SYMBOL(mvMacStrToHex);
EXPORT_SYMBOL(mvBoardTclkGet);
EXPORT_SYMBOL(mvBoardMacSpeedGet);
#ifdef CONFIG_MV_INCLUDE_TDM
EXPORT_SYMBOL(mvCtrlTdmClkCtrlConfig);
#if defined(MV_TDM_USE_DCO)
EXPORT_SYMBOL(mvCtrlTdmClkCtrlGet);
EXPORT_SYMBOL(mvCtrlTdmClkCtrlSet);
#endif
#endif
EXPORT_SYMBOL(mvCtrlTdmUnitIrqGet);
EXPORT_SYMBOL(mvWinOverlapTest);
EXPORT_SYMBOL(mvCtrlTdmUnitTypeGet);
EXPORT_SYMBOL(mvCtrlAddrWinMapBuild);
EXPORT_SYMBOL(mvBoardTdmSpiModeGet);
EXPORT_SYMBOL(mvBoardTdmSpiCsGet);
EXPORT_SYMBOL(mvBoardTdmDevicesCountGet);
EXPORT_SYMBOL(mvMacAddr);
#ifdef CONFIG_MV_INCLUDE_SPI
#include "spi/mvSpiCmnd.h"
EXPORT_SYMBOL(mvSpiWriteThenWrite);
EXPORT_SYMBOL(mvSpiWriteThenRead);
#include "spi/mvSpi.h"
EXPORT_SYMBOL(mvSpiParamsSet);
#endif
#include "gpp/mvGpp.h"
EXPORT_SYMBOL(mvGppValueSet);
/*************************************************************************************************************
 * Audio
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_AUDIO
#include "audio/mvAudio.h"
#include "mvSysAudioApi.h"
EXPORT_SYMBOL(mvSPDIFRecordTclockSet);
EXPORT_SYMBOL(mvSPDIFPlaybackCtrlSet);
EXPORT_SYMBOL(mvI2SPlaybackCtrlSet);
EXPORT_SYMBOL(mvAudioPlaybackControlSet);
EXPORT_SYMBOL(mvAudioDCOCtrlSet);
EXPORT_SYMBOL(mvI2SRecordCntrlSet);
EXPORT_SYMBOL(mvAudioRecordControlSet);
EXPORT_SYMBOL(mvSysAudioInit);
EXPORT_SYMBOL(mvBoardA2DTwsiAddrGet);
EXPORT_SYMBOL(mvBoardA2DTwsiAddrTypeGet);
#endif

/*************************************************************************************************************
 * USB
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_USB
extern u32 mvIsUsbHost;

#include "usb/mvUsb.h"
EXPORT_SYMBOL(mvIsUsbHost);
EXPORT_SYMBOL(mvCtrlUsbMaxGet);
EXPORT_SYMBOL(mvUsbGetCapRegAddr);
#ifdef MV_USB_VOLTAGE_FIX
EXPORT_SYMBOL(mvUsbGppInit);
EXPORT_SYMBOL(mvUsbBackVoltageUpdate);
#endif
#endif /* CONFIG_MV_INCLUDE_USB */

/*************************************************************************************************************
 * CESA
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_CESA
#include "mvSysCesaApi.h"
#include "cesa/mvCesa.h"
#include "cesa/mvMD5.h"
#include "cesa/mvSHA1.h"
extern unsigned char *mv_sram_usage_get(int *sram_size_ptr);

EXPORT_SYMBOL(mvSysCesaInit);
EXPORT_SYMBOL(mvCesaSessionOpen);
EXPORT_SYMBOL(mvCesaSessionClose);
EXPORT_SYMBOL(mvCesaAction);
EXPORT_SYMBOL(mvCesaReadyGet);
EXPORT_SYMBOL(mvCesaCopyFromMbuf);
EXPORT_SYMBOL(mvCesaCopyToMbuf);
EXPORT_SYMBOL(mvCesaMbufCopy);
EXPORT_SYMBOL(mvCesaCryptoIvSet);
EXPORT_SYMBOL(mvMD5);
EXPORT_SYMBOL(mvSHA1);

EXPORT_SYMBOL(mvCesaDebugQueue);
EXPORT_SYMBOL(mvCesaDebugSram);
EXPORT_SYMBOL(mvCesaDebugSAD);
EXPORT_SYMBOL(mvCesaDebugStatus);
EXPORT_SYMBOL(mvCesaDebugMbuf);
EXPORT_SYMBOL(mvCesaDebugSA);
EXPORT_SYMBOL(mv_sram_usage_get);

extern u32 mv_crypto_base_get(void);
EXPORT_SYMBOL(mv_crypto_base_get);
EXPORT_SYMBOL(cesaReqResources);
EXPORT_SYMBOL(mvCesaFinish);

#endif

/*************************************************************************************************************
 * Flashes
 *************************************************************************************************************/
#if defined (CONFIG_MV_INCLUDE_SPI)
#include <sflash/mvSFlash.h>
#include <sflash/mvSFlashSpec.h>
EXPORT_SYMBOL(mvSFlashInit);
EXPORT_SYMBOL(mvSFlashSectorErase);
EXPORT_SYMBOL(mvSFlashChipErase);
EXPORT_SYMBOL(mvSFlashBlockRd);
EXPORT_SYMBOL(mvSFlashBlockWr);
EXPORT_SYMBOL(mvSFlashIdGet);
EXPORT_SYMBOL(mvSFlashWpRegionSet);
EXPORT_SYMBOL(mvSFlashWpRegionGet);
EXPORT_SYMBOL(mvSFlashStatRegLock);
EXPORT_SYMBOL(mvSFlashSizeGet);
EXPORT_SYMBOL(mvSFlashPowerSaveEnter);
EXPORT_SYMBOL(mvSFlashPowerSaveExit);
EXPORT_SYMBOL(mvSFlashModelGet);
#endif


/*************************************************************************************************************
 * SATA
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_INTEG_SATA
#include <sata/CoreDriver/mvSata.h>
EXPORT_SYMBOL(mvSataWinInit);
#endif

/*************************************************************************************************************
 * DMA/XOR
 *************************************************************************************************************/
#if defined(CONFIG_MV_XOR_MEMCOPY) || defined(CONFIG_MV_IDMA_MEMCOPY)
EXPORT_SYMBOL(asm_memcpy);
#endif

#ifdef CONFIG_MV_SP_I_FTCH_DB_INV
EXPORT_SYMBOL(mv_l2_inv_range);
#endif

/*************************************************************************************************************
 * PON
 *************************************************************************************************************/
#ifdef CONFIG_MV_PON
EXPORT_SYMBOL(mv_pon_link_state_register);
EXPORT_SYMBOL(mvGppBlinkEn);
EXPORT_SYMBOL(mvGppTypeSet);
EXPORT_SYMBOL(mvBoarGpioPinNumGet);
EXPORT_SYMBOL(mvBoardGpioPolarityGet);
#endif /* CONFIG_MV_PON */

/*************************************************************************************************************
 * Marvell TRACE
 *************************************************************************************************************/
#ifdef CONFIG_MV_DBG_TRACE
#include "dbg-trace.h"
EXPORT_SYMBOL(TRC_INIT);
EXPORT_SYMBOL(TRC_REC);
EXPORT_SYMBOL(TRC_OUTPUT);
EXPORT_SYMBOL(TRC_START);
EXPORT_SYMBOL(TRC_RELEASE);
#endif

#ifdef CONFIG_MV_ETH_NFP_MODULE
#ifdef CONFIG_MV_ETH_BM
#include "bm/mvBm.h"
EXPORT_SYMBOL(mvBmVirtBase);
#endif
#include "mvList.h"
EXPORT_SYMBOL(mvListCreate);
EXPORT_SYMBOL(mvListDestroy);
#endif


/*************************************************************************************************************
 * TPM
 *************************************************************************************************************/
#ifdef CONFIG_MV_INCLUDE_TPM
#include "net_dev/mv_netdev.h"

#include "pnc/mvTcam.h"
#include "pmt/mvPmt.h"
#include "gbe/mvNeta.h"
#include "msApiTypes.h"
#include "msApiDefs.h"
#include "mv_switch.h"
#include "mv_switch_wrap.h"
#include "boardEnv/mvBoardEnvLib.h"


EXPORT_SYMBOL(tcam_hw_inv);
EXPORT_SYMBOL(tcam_hw_read);
EXPORT_SYMBOL(tcam_hw_write);
EXPORT_SYMBOL(tcam_sw_set_lookup);
EXPORT_SYMBOL(tcam_sw_set_port);
EXPORT_SYMBOL(tcam_sw_set_byte);
EXPORT_SYMBOL(tcam_sw_set_ainfo);
EXPORT_SYMBOL(tcam_sw_set_mask);
EXPORT_SYMBOL(tcam_sw_clear);
EXPORT_SYMBOL(tcam_hw_init);

EXPORT_SYMBOL(sram_sw_set_lookup_done);
EXPORT_SYMBOL(sram_sw_set_next_lookup_shift);
EXPORT_SYMBOL(sram_sw_set_next_lookup);

#ifdef CONFIG_MV_TPM_Z1_VER
EXPORT_SYMBOL(sram_sw_set_flowid_hi);
EXPORT_SYMBOL(sram_sw_set_flowid_lo);
#elif CONFIG_MV_TPM_Z2_VER
EXPORT_SYMBOL(sram_sw_set_flowid);
#endif
EXPORT_SYMBOL(sram_sw_set_rxq);
EXPORT_SYMBOL(sram_sw_set_rinfo);
EXPORT_SYMBOL(sram_sw_set_ainfo);
EXPORT_SYMBOL(sram_sw_set_shift_update);

EXPORT_SYMBOL(mvNetaTxMhRegSet);
EXPORT_SYMBOL(mvNetaPmtInit);
EXPORT_SYMBOL(mvNetaPmtClear);
EXPORT_SYMBOL(mvNetaPmtRead);
EXPORT_SYMBOL(mvNetaPmtWrite);
EXPORT_SYMBOL(mvNetaTxpRateSet);
EXPORT_SYMBOL(mvNetaTxqRateSet);
EXPORT_SYMBOL(mvNetaTxqWrrPrioSet);
EXPORT_SYMBOL(mvNetaTxqFixPrioSet);
EXPORT_SYMBOL(mvNetaHwfMhSelSet);
EXPORT_SYMBOL(mvNetaHwfMhSrcSet);
EXPORT_SYMBOL(mv_eth_set_mtu);
EXPORT_SYMBOL(mv_eth_get_mtu);
#ifdef CONFIG_MV_TPM_Z2_VER
EXPORT_SYMBOL(mvNetaTxpBurstSet);
EXPORT_SYMBOL(mvNetaTxqBurstSet);
#endif
EXPORT_SYMBOL(mvNetaHwfTxqEnable);
EXPORT_SYMBOL(mvNetaVlanEtySet);

EXPORT_SYMBOL(mv_eth_ctrl_port_buf_num_set);
EXPORT_SYMBOL(mv_eth_ctrl_txq_hwf_own);
EXPORT_SYMBOL(mv_eth_ctrl_txq_size_set);
EXPORT_SYMBOL(mv_eth_ctrl_rxq_size_set);
EXPORT_SYMBOL(mv_eth_ctrl_port_started_get);
EXPORT_SYMBOL(mv_eth_ctrl_flag);
EXPORT_SYMBOL(mv_eth_ctrl_tx_mh);
EXPORT_SYMBOL(mv_switch_reg_write);
EXPORT_SYMBOL(mv_switch_reg_read);

EXPORT_SYMBOL(mv_switch_mac_addr_set);
EXPORT_SYMBOL(mv_switch_add_static_mac);
EXPORT_SYMBOL(mv_switch_del_static_mac);
EXPORT_SYMBOL(mv_switch_set_port_max_macs);
EXPORT_SYMBOL(mv_switch_get_port_max_macs);
EXPORT_SYMBOL(mv_switch_clear_dynamic_mac);
EXPORT_SYMBOL(mv_switch_set_mirror);
EXPORT_SYMBOL(mv_switch_get_mirror);
EXPORT_SYMBOL(mv_switch_set_mtu);
EXPORT_SYMBOL(mv_switch_get_mtu);
//EXPORT_SYMBOL(mv_switch_set_port_flooding);
//EXPORT_SYMBOL(mv_switch_get_port_flooding);
EXPORT_SYMBOL(mv_switch_set_port_tagged);
EXPORT_SYMBOL(mv_switch_get_port_tagged);
EXPORT_SYMBOL(mv_switch_set_port_untagged);
EXPORT_SYMBOL(mv_switch_get_port_untagged);
EXPORT_SYMBOL(mv_switch_set_port_def_vlan);
EXPORT_SYMBOL(mv_switch_get_port_def_vlan);
EXPORT_SYMBOL(mv_switch_set_port_def_pri);
EXPORT_SYMBOL(mv_switch_get_port_def_pri);
EXPORT_SYMBOL(mv_switch_port_add_vid);
EXPORT_SYMBOL(mv_switch_prv_port_add_vid);
EXPORT_SYMBOL(mv_switch_del_vid_per_port);
EXPORT_SYMBOL(mv_switch_prv_del_vid_per_port);
EXPORT_SYMBOL(mv_switch_set_port_vid_egress_mode);
EXPORT_SYMBOL(mv_switch_prv_set_secure_mode);
EXPORT_SYMBOL(mv_switch_prv_set_fallback_mode);
EXPORT_SYMBOL(mv_switch_set_vid_filter_per_port);
EXPORT_SYMBOL(mv_switch_get_vid_filter_per_port);
EXPORT_SYMBOL(mv_switch_set_port_sched_mode);
EXPORT_SYMBOL(mv_switch_set_uni_q_weight);
EXPORT_SYMBOL(mv_switch_set_uni_ingr_police_rate);
EXPORT_SYMBOL(mv_switch_get_uni_ingr_police_rate);
EXPORT_SYMBOL(mv_switch_set_uni_tc_ingr_police_rate);
EXPORT_SYMBOL(mv_switch_get_uni_tc_ingr_police_rate);
EXPORT_SYMBOL(mv_switch_set_uni_egr_rate_limit);
EXPORT_SYMBOL(mv_switch_get_uni_egr_rate_limit);
EXPORT_SYMBOL(mv_switch_set_age_time);
EXPORT_SYMBOL(mv_switch_get_age_time);
EXPORT_SYMBOL(mv_switch_set_mac_learn);
EXPORT_SYMBOL(mv_switch_get_mac_learn);
EXPORT_SYMBOL(mv_switch_set_port_vlan_ports);
EXPORT_SYMBOL(mv_switch_get_port_vlan_ports);
EXPORT_SYMBOL(mv_switch_set_ingress_limit_mode);
EXPORT_SYMBOL(mv_switch_get_ingress_limit_mode);
EXPORT_SYMBOL(mv_switch_get_port_los_alarm);
EXPORT_SYMBOL(mv_switch_set_priority_selection);
EXPORT_SYMBOL(mv_switch_get_priority_selection);
EXPORT_SYMBOL(mv_switch_set_tag_pri_mapping);
EXPORT_SYMBOL(mv_switch_get_tag_pri_mapping);
EXPORT_SYMBOL(mv_switch_set_ip_pri_mapping);
EXPORT_SYMBOL(mv_switch_get_ip_pri_mapping);
EXPORT_SYMBOL(mv_switch_set_unknown_unicast_flood);
EXPORT_SYMBOL(mv_switch_get_unknown_unicast_flood);
EXPORT_SYMBOL(mv_switch_set_unknown_multicast_flood);
EXPORT_SYMBOL(mv_switch_get_unknown_multicast_flood);
EXPORT_SYMBOL(mv_switch_set_broadcast_flood);
EXPORT_SYMBOL(mv_switch_get_broadcast_flood);
EXPORT_SYMBOL(mv_switch_set_marvell_header_mode);
EXPORT_SYMBOL(mv_switch_get_marvell_header_mode);
EXPORT_SYMBOL(mv_switch_set_port_autoneg_mode);
EXPORT_SYMBOL(mv_switch_get_port_autoneg_mode);
EXPORT_SYMBOL(mv_switch_restart_port_autoneg);
EXPORT_SYMBOL(mv_switch_set_phy_port_state);
EXPORT_SYMBOL(mv_switch_get_phy_port_state);
EXPORT_SYMBOL(mv_switch_get_port_link_status);
EXPORT_SYMBOL(mv_switch_get_port_duplex_status);
EXPORT_SYMBOL(mv_switch_get_port_speed_mode);
EXPORT_SYMBOL(mv_switch_set_port_pause);
EXPORT_SYMBOL(mv_switch_get_port_pause);
EXPORT_SYMBOL(mv_switch_get_port_pause_state);
EXPORT_SYMBOL(mv_switch_set_port_loopback);
EXPORT_SYMBOL(mv_switch_get_port_loopback);
EXPORT_SYMBOL(mv_switch_set_port_line_loopback);
EXPORT_SYMBOL(mv_switch_get_port_line_loopback);
EXPORT_SYMBOL(mv_switch_set_port_duplex_mode);
EXPORT_SYMBOL(mv_switch_get_port_duplex_mode);
EXPORT_SYMBOL(mv_switch_set_port_speed);
EXPORT_SYMBOL(mv_switch_get_port_speed);
/*EXPORT_SYMBOL(mv_switch_set_port_state);
EXPORT_SYMBOL(mv_switch_get_port_state);*/
EXPORT_SYMBOL(mv_switch_set_global_reg);
EXPORT_SYMBOL(mv_switch_get_global_reg);
EXPORT_SYMBOL(mv_switch_set_port_reg);
EXPORT_SYMBOL(mv_switch_get_port_reg);
EXPORT_SYMBOL(mv_switch_port_print_vid);
EXPORT_SYMBOL(mv_switch_print_fdb);
EXPORT_SYMBOL(mv_switch_drv_init);
EXPORT_SYMBOL(mv_switch_clear_port_counters);
EXPORT_SYMBOL(mv_switch_print_port_counters);
//EXPORT_SYMBOL(mv_switch_get_port_counters_sh);
EXPORT_SYMBOL(mv_switch_get_port_counters);
EXPORT_SYMBOL(mv_switch_get_port_drop_counters);
EXPORT_SYMBOL(mv_switch_clean_port_counters);
EXPORT_SYMBOL(mvBoardMppModuleTypePrint);
EXPORT_SYMBOL(mvBoardIsInternalSwitchConnected);
EXPORT_SYMBOL(mvBoardEthComplexConfigGet);
#endif /* CONFIG_MV_INCLUDE_TPM */

