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
/*******************************************************************************
* tpm_ctc_cm.h
*
* DESCRIPTION:
*               header file for tpm_cpu_port_fc.c
*
* DEPENDENCIES:
*               None
*
* CREATED BY:   KostaP
*
* DATE CREATED: 6 June 2012
*
* FILE REVISION NUMBER:
*               Revision: 1.1
*
*
*******************************************************************************/
#ifndef _TPM_CPU_PORT_FC_H_
#define _TPM_CPU_PORT_FC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <linux/hrtimer.h>
#include <linux/ktime.h>

/*************************************************************/
/*               ENUMERATIONS                                */
/*************************************************************/

/*************************************************************/
/*               DEFINITIONS                                 */
/*************************************************************/
#define TPM_FC_MAGIC_NUMBER		0xFCFACED
#define TPM_FC_DEBUG

typedef struct tpm_fc_cfg_t_ {
	MV_U32   thresh_high;
	MV_U32   thresh_low;
	MV_U32   port;
	MV_U32   tgt_port;
	MV_BOOL  tgt_port_changed;
	MV_U32   tx_port;
	MV_U32   tx_queue;
	ktime_t  hrt_hit_time;
	ktime_t  hrt_hit_time_delay;
	MV_U32   oneshot_count;

} tpm_fc_cfg_t;

#ifdef  TPM_FC_DEBUG
typedef struct tpm_fc_stat_t_ {
	MV_U32   hrt_hits_num;
	MV_U32   hrt_lost_num;
	MV_U32   hrt_wraparound;
	s64      hrt_lost_max_ns;
	s64      hrt_lost_200_up;  /* timer slips 200% and more than set time */
	s64      hrt_lost_150_200; /* timer slips 150%-200% out of set time */
	s64      hrt_lost_100_150; /* timer slips 100%-150% out of set time */
	s64      hrt_lost_50_100;  /* timer slips 50%-100% out of set time */
	s64      hrt_lost_25_50;   /* timer slips 25%-50% out of set time */
	s64      hrt_lost_25_less; /* timer slips less than 25% out of set time */
} tpm_fc_stat_t;
#endif

typedef struct tpm_fc_info_t_ {
	tpm_fc_cfg_t    cfg;
	MV_U32          magic;
	struct hrtimer  hr_timer;
	ktime_t  	hrt_last_hit;
#ifdef  TPM_FC_DEBUG
	tpm_fc_stat_t   stat;
	tpm_fc_stat_t   oneshot_stat;
	ktime_t  	oneshot_hrt_last_hit;
	MV_U32      	pre_stat_cnt;
#endif	
} tpm_fc_info_t;

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
MV_STATUS tpm_fc_set_period(MV_U32 us_period);

/*******************************************************************************
**
**  tpm_fc_set_config
**  ____________________________________________________________________________
**
**  DESCRIPTION: The function configure Rate limiting parameters
**
**  PARAMETERS:  tpm_fc_cfg_t  *cf
**
**  OUTPUTS:     none
**
**  RETURNS:     void
**
*******************************************************************************/
void tpm_fc_set_config(tpm_fc_cfg_t *cfg);

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
MV_STATUS tpm_fc_set_oneshot(MV_U32 oneshot);

/*******************************************************************************
**
**  tpm_fc_enable
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
MV_STATUS tpm_fc_enable(MV_BOOL enable);


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
void tpm_fc_get_info(tpm_fc_info_t **info);

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
void tpm_fc_clear_stat(void);

/*******************************************************************************
**
**  tpm_fc_engine_init
**  ____________________________________________________________________________
**
**  DESCRIPTION: Init fc engine
**
**  PARAMETERS:  none
**
**  OUTPUTS:     none
**
**  RETURNS:     MV_OK/MV_ERROR
**
*******************************************************************************/
MV_STATUS tpm_fc_engine_init(void);

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
MV_BOOL tpm_fc_is_running(void);


#ifdef __cplusplus
}
#endif
#endif    /* _TPM_CPU_PORT_FC_H_ */

