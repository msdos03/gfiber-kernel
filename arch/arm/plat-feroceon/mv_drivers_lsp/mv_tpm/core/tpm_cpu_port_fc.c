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

/******************************************************************************
* tpm_cpu_port_fc.c
*
* DESCRIPTION:
*               Traffic Processor Manager CPU port flow control module
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   KostaP
*
* DATE CREATED:
*
* FILE REVISION NUMBER:
*               Revision: 1.1
*
*
*******************************************************************************/
#include <linux/version.h>
#include "tpm_common.h"
#include "tpm_header.h"
#include "tpm_cpu_port_fc.h"
#include "dbg-trace.h"

static tpm_fc_info_t fc_info;

extern int mv_enable_fc_events(void*, unsigned long);
extern void mv_disable_fc_events(void);
void tpm_fc_hr_timer_handler(void);

/*******************************************************************************
**
**  tpm_fc_rate_limit
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function periodically switches FC on/off on specific port
**               if specific queue reaches pre-configured threshold
**
**  PARAMETERS:  None
**
**  OUTPUTS:     None
**
**  RETURNS:     None
**
*******************************************************************************/
void tpm_fc_rate_limit(void)
{
	MV_U32    queue_desc_number;
	MV_U32    reg_val;

	if (fc_info.cfg.tgt_port_changed == MV_TRUE) {
		/* Enable FC for target port */
		MV_U32   phy_addr = mvBoardPhyAddrGet(fc_info.cfg.tgt_port);

		reg_val  = MV_REG_READ(NETA_GMAC_AN_CTRL_REG(fc_info.cfg.tgt_port));
		reg_val |= NETA_ENABLE_FLOW_CONTROL_AUTO_NEG_MASK;
		reg_val |= NETA_SET_FLOW_CONTROL_MASK;
		MV_REG_WRITE(NETA_GMAC_AN_CTRL_REG(fc_info.cfg.tgt_port), reg_val);

		/* Reatart AN on target port PHY */
		mvEthPhyRestartAN(phy_addr, 0);

		fc_info.cfg.tgt_port_changed = MV_FALSE;
	}

	/* Manage FC mode on target port if needed */
	reg_val  = MV_REG_READ(ETH_PORT_SERIAL_CTRL_REG(fc_info.cfg.tgt_port));
	reg_val &= ~ETH_TX_FC_MODE_MASK;

	queue_desc_number = mvNetaTxqPendDescNumGet(fc_info.cfg.port, fc_info.cfg.tx_port, fc_info.cfg.tx_queue);
	if (queue_desc_number >= fc_info.cfg.thresh_high)
		reg_val |= ETH_TX_FC_SEND_PAUSE;
	else if (queue_desc_number <= fc_info.cfg.thresh_low)
		reg_val |= ETH_TX_FC_NO_PAUSE;
	else
		return;

	MV_REG_WRITE(ETH_PORT_SERIAL_CTRL_REG(fc_info.cfg.tgt_port), reg_val);
}

/*******************************************************************************
**
**  tpm_fc_get_info
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function returns FC module configuration and statistics
**
**  PARAMETERS:  tpm_fc_info_t  **info
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void tpm_fc_get_info(tpm_fc_info_t **info)
{
	*info = &fc_info;
}

/*******************************************************************************
**
**  tpm_fc_clear_stat
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function clears FC module  statistics
**
**  PARAMETERS:  none
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void tpm_fc_clear_stat(void)
{
#ifdef TPM_FC_DEBUG
	memset(&fc_info.stat, 0, sizeof(tpm_fc_stat_t));
#endif
}

/*******************************************************************************
**
**  tpm_fc_set_config
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure Rate limiting parameters
**
**  PARAMETERS:  tpm_fc_cfg_t  *cfg
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void tpm_fc_set_config(tpm_fc_cfg_t *cfg)
{
	tpm_init_fc_params_t db_fc_conf;

	fc_info.cfg.thresh_high = cfg->thresh_high;
	fc_info.cfg.thresh_low  = cfg->thresh_low;
	fc_info.cfg.port        = cfg->port;

	if (fc_info.cfg.tgt_port != cfg->tgt_port) {
		fc_info.cfg.tgt_port         = cfg->tgt_port;
		fc_info.cfg.tgt_port_changed = MV_TRUE;
	}

	fc_info.cfg.tx_port     = cfg->tx_port;
	fc_info.cfg.tx_queue    = cfg->tx_queue;

	/* update DB with new settings */
	tpm_db_fc_conf_get(&db_fc_conf);

	db_fc_conf.port		= cfg->tx_port;
	db_fc_conf.tgt_port 	= cfg->tgt_port;
	db_fc_conf.thresh_high 	= cfg->thresh_high;
	db_fc_conf.thresh_low 	= cfg->thresh_low;
	db_fc_conf.tx_port	= cfg->tx_port;
	db_fc_conf.tx_queue	= cfg->tx_queue;

	tpm_db_fc_conf_set(&db_fc_conf);
}

/*******************************************************************************
**
**  tpm_fc_set_period
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure Rate limiting parameters
**
**  PARAMETERS:  MV_U32 us_period
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
MV_STATUS tpm_fc_set_period(MV_U32 us_period)
{
	MV_U32 ns_period = us_period * 1000;
	tpm_init_fc_params_t db_fc_conf;

	if (ns_period < us_period)
		return (MV_ERROR);

	/* update DB entry */
	tpm_db_fc_conf_get(&db_fc_conf);
       db_fc_conf.queue_sample_freq = us_period;
	tpm_db_fc_conf_set(&db_fc_conf);

	fc_info.cfg.hrt_hit_time = ktime_set(0, ns_period);
	mv_enable_fc_events(&tpm_fc_hr_timer_handler, ns_period);

	return (MV_OK);
}

#ifdef TPM_FC_DEBUG
/*******************************************************************************
**
**  tpm_fc_set_oneshot
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure oneshot counter value
**
**  PARAMETERS:  MV_U32 oneshot
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
MV_STATUS tpm_fc_set_oneshot(MV_U32 oneshot)
{
	fc_info.cfg.oneshot_count = oneshot;

	return (MV_OK);
}
#endif

/*******************************************************************************
**
**  tpm_fc_hr_timer_handler
**  ____________________________________________________________________________
**
**  DESCRIPTION: PON periodic high resolution timer handler
**
**  PARAMETERS:  none
**
**  OUTPUTS:     none
**
**  RETURNS:     hr_timer_RESTART
**
*******************************************************************************/
#if 0
static enum hrtimer_restart tpm_fc_hr_timer_handler(struct hrtimer *timer)
{
	ktime_t current_time    = timer->base->get_time();
#else
void tpm_fc_hr_timer_handler(void)
{
	ktime_t current_time    = ktime_get_real();
#endif

	s64     current_time_ns = ktime_to_ns(current_time);
	s64     last_hit_ns     = ktime_to_ns(fc_info.hrt_last_hit);
	s64     time_slip_ns;
	s64     hit_time_ns     = ktime_to_ns(fc_info.cfg.hrt_hit_time);

#ifdef TPM_FC_DEBUG
	fc_info.stat.hrt_hits_num++;
	if (fc_info.stat.hrt_hits_num < 100)
		TRC_REC("Hit %ld\n", fc_info.stat.hrt_hits_num);

	if ((fc_info.cfg.oneshot_count != 0) && (fc_info.oneshot_stat.hrt_hits_num < fc_info.cfg.oneshot_count))
		fc_info.oneshot_stat.hrt_hits_num++;
#endif
#if 0
	/* Update timer for the next hit at the beginning for better precision */
	hrtimer_forward(timer, current_time, fc_info.cfg.hrt_hit_time);
#endif
	/* First compute the real timer period */
	time_slip_ns = current_time_ns - last_hit_ns;

	if (time_slip_ns < 0) {
#ifdef TPM_FC_DEBUG
		fc_info.stat.hrt_wraparound++;
		if ((fc_info.cfg.oneshot_count != 0) && (fc_info.oneshot_stat.hrt_hits_num < fc_info.cfg.oneshot_count))
			fc_info.oneshot_stat.hrt_wraparound++;
#endif
		time_slip_ns = KTIME_MAX - last_hit_ns + current_time_ns;
	}

	/* Update last hit with the current time */
	fc_info.hrt_last_hit = current_time;

#ifdef TPM_FC_DEBUG
	if (fc_info.pre_stat_cnt) {
		fc_info.pre_stat_cnt--;

		/* Flow Control the port */
		tpm_fc_rate_limit();
#if 0
		return (HRTIMER_RESTART);
#else
		return;
#endif
	}
#endif

	/* Calculate the slip from the requested time period */
	time_slip_ns -= hit_time_ns;

#ifdef TPM_FC_DEBUG
	/* Collect more statistics */
	if (time_slip_ns != 0) {

		/* Total counters */
		fc_info.stat.hrt_lost_num++;

		if (time_slip_ns > fc_info.stat.hrt_lost_max_ns)
			fc_info.stat.hrt_lost_max_ns = time_slip_ns;

		if (time_slip_ns >= (hit_time_ns << 1))
			fc_info.stat.hrt_lost_200_up++;
		else if (time_slip_ns >= (hit_time_ns + (hit_time_ns >> 1)))
			fc_info.stat.hrt_lost_150_200++;
		else if (time_slip_ns >= hit_time_ns)
			fc_info.stat.hrt_lost_100_150++;
		else if (time_slip_ns >= (hit_time_ns >> 1))
			fc_info.stat.hrt_lost_50_100++;
		else if (time_slip_ns >= (hit_time_ns >> 2))
			fc_info.stat.hrt_lost_25_50++;
		else
			fc_info.stat.hrt_lost_25_less++;

		/* One-shot counters */
		if ((fc_info.cfg.oneshot_count != 0) && (fc_info.oneshot_stat.hrt_hits_num < fc_info.cfg.oneshot_count)) {
			fc_info.oneshot_stat.hrt_lost_num++;

			if (time_slip_ns > fc_info.oneshot_stat.hrt_lost_max_ns)
				fc_info.oneshot_stat.hrt_lost_max_ns = time_slip_ns;

			if (time_slip_ns >= (hit_time_ns << 1))
				fc_info.oneshot_stat.hrt_lost_200_up++;
			else if (time_slip_ns >= (hit_time_ns + (hit_time_ns >> 1)))
				fc_info.oneshot_stat.hrt_lost_150_200++;
			else if (time_slip_ns >= hit_time_ns)
				fc_info.oneshot_stat.hrt_lost_100_150++;
			else if (time_slip_ns >= (hit_time_ns >> 1))
				fc_info.oneshot_stat.hrt_lost_50_100++;
			else if (time_slip_ns >= (hit_time_ns >> 2))
				fc_info.oneshot_stat.hrt_lost_25_50++;
			else
				fc_info.oneshot_stat.hrt_lost_25_less++;
		}
	}
#endif

	/* Flow Control the port */
	tpm_fc_rate_limit();
#if 0
	return (HRTIMER_RESTART);
#endif
}


/*******************************************************************************
**
**  tpm_fc_enable
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function enable the SW Flow-Control
**
**  PARAMETERS:  MV_BOOL enable
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
MV_STATUS tpm_fc_enable(MV_BOOL enable)
{
	MV_BOOL fc_is_running = tpm_fc_is_running();
	tpm_init_fc_params_t db_fc_conf;
	MV_BOOL config_change = false;

	if (MV_FALSE == enable)
	{
		/* Disable FC for target port if mode changed */
		if (true == fc_is_running) {
			MV_U32  phy_addr = mvBoardPhyAddrGet(fc_info.cfg.tgt_port);
			MV_U32	reg_val;

#if 0
			hrtimer_cancel(&fc_info.hr_timer);
#else
			mv_disable_fc_events();
#endif
			reg_val  = MV_REG_READ(NETA_GMAC_AN_CTRL_REG(fc_info.cfg.tgt_port));
			reg_val |= NETA_ENABLE_FLOW_CONTROL_AUTO_NEG_MASK;
			reg_val &= ~NETA_SET_FLOW_CONTROL_MASK;
			MV_REG_WRITE(NETA_GMAC_AN_CTRL_REG(fc_info.cfg.tgt_port), reg_val);

			/* Reatart AN on target port PHY */
			mvEthPhyRestartAN(phy_addr, 0);

			/* set FC not running indication */
			fc_info.magic = 0;
			config_change = true;
		}

#ifdef TPM_FC_DEBUG
		fc_info.cfg.oneshot_count = 0;
#endif
		TRC_RELEASE();
	}
	else
	{
		if (false == fc_is_running){
#ifdef TPM_FC_DEBUG
			/* clear up statistics */
			memset(&fc_info.oneshot_stat, 0, sizeof(tpm_fc_stat_t));
			memset(&fc_info.stat, 0, sizeof(tpm_fc_stat_t));
			fc_info.pre_stat_cnt = 64;
#endif
			/* force port setting */
			fc_info.cfg.tgt_port_changed = MV_TRUE;

			/* set FC running indication */
			fc_info.magic = TPM_FC_MAGIC_NUMBER;

			/* save the current time in the last hit */
			fc_info.hrt_last_hit = ktime_get_real();
#if 0
			hrtimer_init(&fc_info.hr_timer, CLOCK_REALTIME, HRTIMER_MODE_REL);
			fc_info.hr_timer.function = &tpm_fc_hr_timer_handler;
			hrtimer_start(&fc_info.hr_timer, fc_info.cfg.hrt_hit_time, HRTIMER_MODE_REL);
#else
			//TRC_INIT();
			mv_enable_fc_events(&tpm_fc_hr_timer_handler, ktime_to_ns(fc_info.cfg.hrt_hit_time));
#endif
			config_change = true;
		}
	}

	if (config_change) {
		/* update current DB settings */
		tpm_db_fc_conf_get(&db_fc_conf);
		db_fc_conf.enabled = (enable == MV_TRUE) ? 1 : 0;
		tpm_db_fc_conf_set(&db_fc_conf);
	}


	return (MV_OK);
}

/*******************************************************************************
**
**  tpm_fc_is_running
**  ____________________________________________________________________________
**
**  DESCRIPTION: Return the running status of FC engine
**
**  PARAMETERS:  None
**
**  OUTPUTS:     none
**
**  RETURNS:     MV_TRUE/MV_FALSE
**
*******************************************************************************/
MV_BOOL tpm_fc_is_running(void)
{
	if (fc_info.magic == TPM_FC_MAGIC_NUMBER)
		return(MV_TRUE);
	else
		return(MV_FALSE);
}

/*******************************************************************************
**
**  tpm_fc_engine_init
**  ____________________________________________________________________________
**
**  DESCRIPTION: Init fc engine
**
**  PARAMETERS:  MV_U32           us_period
**
**  OUTPUTS:     none
**
**  RETURNS:     MV_OK/MV_ERROR
**
*******************************************************************************/
int32_t tpm_fc_engine_init(void)
{
	tpm_init_fc_params_t 	db_fc_conf;
	MV_U32 			ns_q_sample_freq;
	MV_BOOL			fc_is_running;

	/* save the current state, for runtime MIB reset */
	fc_is_running = tpm_fc_is_running();

	/* get current DB settings */
	tpm_db_fc_conf_get(&db_fc_conf);

	/* FC was enabled before MIB reset */
	if (fc_is_running) {
		TPM_OS_DEBUG(TPM_MTU_MOD, "SW port FC disabled!\n");
		tpm_fc_enable(MV_FALSE);
	}

	memset(&fc_info, 0, sizeof(tpm_fc_info_t));

	ns_q_sample_freq = db_fc_conf.queue_sample_freq * 1000;

	if (ns_q_sample_freq < db_fc_conf.queue_sample_freq)
		return (MV_ERROR);

	fc_info.cfg.hrt_hit_time	= ktime_set(0, ns_q_sample_freq);
	fc_info.cfg.thresh_high		= db_fc_conf.thresh_high;
	fc_info.cfg.thresh_low		= db_fc_conf.thresh_low;
	fc_info.cfg.port		    = db_fc_conf.port;
	fc_info.cfg.tgt_port		= db_fc_conf.tgt_port;
	fc_info.cfg.tgt_port_changed	= MV_TRUE;

#ifdef TPM_FC_DEBUG
	fc_info.cfg.oneshot_count = 0;
#endif

	if (db_fc_conf.enabled){
		TPM_OS_DEBUG(TPM_MTU_MOD, "SW port FC enabled!\n");
		tpm_fc_enable(MV_TRUE);
	}

	return MV_OK;
}

