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

#include <stdio.h>
#include <stdlib.h>
#include "mv_eth_proc.h"

extern char **environ; /* all environments */

static unsigned int port = 0, txp=0, q = 0, status = 0;
static unsigned char mac[6] = {0,};
static unsigned int command = 0;
static unsigned int value = 0;
static unsigned int inport, outport, dip, sip, da[6] = {0, }, sa[6] = {0, };
static unsigned int db_type = 0;
static char* name = NULL;

void show_usage(int badarg)
{
    fprintf(stderr,
    "Usage:                                                                                         \n"
    " mv_neta_tool -h                               Display this help                               \n"
    "                                                       	                                    \n"
    " --- Global commands ---                                                                       \n"
    " mv_neta_tool -txdone <quota>                  Set threshold to start tx_done operations       \n"
    " mv_neta_tool -skb <0 | 1>                     SKB recycle support: 0 - disabled, 1 - enabled  \n"
    " mv_neta_tool -nfp <0 | 1>                     NFP support: 0 - disabled, 1 - enabled          \n"
    "                                                                                       	    \n"
    " --- NetDev commands ---                                                                       \n"
    " mv_neta_tool -netdev_sts <name>               Print network device status                     \n"
    " mv_neta_tool -port_add <name> <port>          Map switch port to one of network devices       \n"
    " mv_neta_tool -port_del <name> <port>          Unmap switch port from network device           \n"
    " mv_neta_tool -tx_noq <name> <0 | 1>           Set queuing discipline mode               	    \n"
    "                                                                                       	    \n"
    " --- Port commands ---                                                                         \n"
    " mv_neta_tool -ejp     <p> <txp> <0 | 1>       Set EJP mode: 0 - Disable, 1 - Enable           \n"
    " mv_neta_tool -txp_bw  <p> <txp> <bw>          Set TX Port Bandwidth limitation                \n"
    "                                                                                       	    \n"
    " --- RXQ commands ---                                                                          \n"
    " mv_neta_tool -rxq_time_coal <p> <q> <usec>    Set RXQ interrupt coalescing value in [usec]    \n"
    " mv_neta_tool -rxq_pkts_coal <p> <q> <pkts>    Set RXQ interrupt coalescing value in [pkts]    \n"
    " mv_neta_tool -rxq_tos  <p> <q> <tos>          Map TOS field to RX queue number                \n" 
    "                                                                                       	    \n"
    " mv_neta_tool -rxq_type <p> <q> <type>         Set RXQ for different packet types	            \n"
    "                                               Supported types: bpdu | arp | tcp | udp. 	    \n"
    "                                                                                       	    \n"
    " mv_neta_tool -rxq_mc   <p> <q> <mac>          Set RXQ for a Multicast MAC                     \n"
    "                                               rxq==8 indicates delete entry.                  \n"
    "                                                                                               \n"
    " --- TXQ commands ---                                                                          \n"
    " mv_neta_tool -txq_coal <p> <txp> <q> <pkts>   Set TXQ interrupt coalescing value       	    \n"
    " mv_neta_tool -txq_tos  <p> <txp> <q> <tos>    Map TOS field to TXQ number. <txp> ignored      \n" 
    " mv_neta_tool -txq_wrr  <p> <txp> <q> <weight> Set TXQ weight: 0 - FIXED, other - WRR          \n"
    " mv_neta_tool -txq_bw   <p> <txp> <txq> <bw>   Set TX Queue Bandwidth limitation               \n"
    "                                                                                               \n"
    " mv_neta_tool -St <options>                                                                    \n"
    "   Display different status information of the port through the kernel printk.                 \n"
    "   OPTIONS:                                                                                    \n"
    "   p      <port>                       Display General port information                        \n"
    "   mac    <port>                       Display MAC addresses information                       \n"
    "   rxq    <port> <q> <m>               Display specific RXQ information                        \n"
    "   txq    <port> <txp> <q> <m>         Display specific TXQ information                        \n"
    "   cntrs  <port>                       Display the HW MIB counters	                            \n"
    "   regs   <port>                       Display a dump of the giga registers                    \n"
    "   stats  <port>                       Display port statistics information                     \n"
    "   tos    <port>                       Display port TOS to RXQ mapping                 	    \n"
    "   switch <port>                       Display switch statistics                 	            \n"
    "   wrr    <port> <txp>                 Display TX Policy information                           \n"
    "   txp    <port> <txp>                 Display TX registers of specific TCONT                  \n"
    "   neta   <port>                       Display Neta registers                                  \n"
    "   gmac   <port>                       Display GMAC registers                                  \n"
    "   rxq_r  <port>                       Display RXQ registers                                   \n"
    "   txq_r  <port>                       Display TXQ registers                                   \n"
    "   hwf    <port>                       Display HWF configuration registers                     \n"
    "   bm                                  Display BM registers                                    \n"
    "   pnc                                 Display PNC registers                                   \n"
    "   pmt    <port> <txp>                 Display PNC registers                                   \n"
    "                                                                                               \n"	
    );
    exit(badarg);
}

static void parse_pt(char *src, int* value_ptr)
{
	packet_t packet;

        if (!strcmp(src, "bpdu"))
        	packet = PT_BPDU;
	else if(!strcmp(src, "arp"))
        	packet = PT_ARP;
	else if(!strcmp(src, "tcp"))
        	packet = PT_TCP;
	else if(!strcmp(src, "udp"))
        	packet = PT_UDP;
	else {
		fprintf(stderr, "illegal packet type, packet type should be bpdu/arp/tcp/udp. \n");	
     		*value_ptr = (int)PT_NONE;
		exit(-1);
        }
	*value_ptr = (int)packet;
        return;
}

static void parse_port(char *src)
{
	int count;

        count = sscanf(src, "%x",&port);

        if (count != 1)  {
		fprintf(stderr, "Port parsing error: count=%d\n", count);	
                exit(-1);
        }
        return;
}


static void parse_q(char *src)
{
	int count;

        count = sscanf(src, "%x",&q);

        if (count != 1) {
		fprintf(stderr, "Queue parsing error: count=%d\n", count);	
                exit(-1);
        }
        return;
}


static void parse_status(char *src)
{
    	if (!strcmp(src, "p")) {
      		status = STS_PORT;
      	}
        else if (!strcmp(src, "txp")) {
                status = STS_TXP_REGS;
        }
        else if (!strcmp(src, "mac")) {
                status = STS_PORT_MAC;
        }
     	else if(!strcmp(src, "rxq")) {
       		status = STS_PORT_RXQ;
     	}
        else if(!strcmp(src, "txq")) {
                status = STS_PORT_TXQ;
        }
     	else if(!strcmp(src, "wrr")) {
           	status = STS_TXP_WRR;
      	}
        else if(!strcmp(src, "cntrs")) {
                status = STS_PORT_MIB;
        }
        else if(!strcmp(src, "regs")) {
                status = STS_PORT_REGS;
        }
      	else if(!strcmp(src, "stats")) {
             	status = STS_PORT_STATS;
        }
        else if(!strcmp(src, "tos")) {
                status = STS_PORT_TOS_MAP;
        }
        else if(!strcmp(src, "switch")) {
                status = STS_SWITCH_STATS;
        }
        else if(!strcmp(src, "neta")) {
                status = STS_NETA_REGS;
        }
        else if(!strcmp(src, "gmac")) {
                status = STS_GMAC_REGS;
        }
        else if(!strcmp(src, "rxq_r")) {
                status = STS_RXQ_REGS;
        }
        else if(!strcmp(src, "txq_r")) {
                status = STS_TXQ_REGS;
        }
        else if(!strcmp(src, "hwf")) {
                status = STS_HWF_REGS;
        }
        else if(!strcmp(src, "bm")) {
                status = STS_BM_REGS;
        }
        else if(!strcmp(src, "pnc")) {
                status = STS_PNC_REGS;
        } 
        else if(!strcmp(src, "pmt")) {
                status = STS_PMT_REGS;
        } 
        else {
                fprintf(stderr, "illegal status %d.\n");
                exit(-1);
        }
        return;
}
static void parse_dec_val(char *src, unsigned int* val_ptr)
{
    int count;

    count = sscanf(src, "%d", val_ptr);
    if(count != 1) {
        fprintf(stderr, "illegal value - should be decimal.\n");
        exit(-1);
    }
    return;
}

static void parse_hex_val(char *src, unsigned int* hex_val_ptr)
{
    int count;

    count = sscanf(src, "%x", hex_val_ptr);
    if(count != 1) {
        fprintf(stderr, "illegal value - should be hexadecimal\n");
        exit(-1);
    }
    return;
}

static int parse_mac(char *src, unsigned char macaddr[])
{
        int count;
        int i;
        int buf[6];

        count = sscanf(src, "%2x:%2x:%2x:%2x:%2x:%2x",
                &buf[0], &buf[1], &buf[2], &buf[3], &buf[4], &buf[5]);

        if (count != 6) {
		fprintf(stderr, "MAC parsing error: Expected %%2x:%%2x:%%2x:%%2x:%%2x:%%2x.\n");
                exit(-1);
        }

        for (i = 0; i < count; i++) {
                macaddr[i] = buf[i] & 0xFF;
        }
        return 0;
}

static int parse_ip(char *src, unsigned int* ip)
{
    int count, i;
    int buf[4];

    count = sscanf(src, "%d.%d.%d.%d",
                &buf[0], &buf[1], &buf[2], &buf[3]);

    if (count != 4) {
        fprintf(stderr, "illegal IP address (should be %%d.%%d.%%d.%%d)\n");
        exit(-1);
    }
    *ip = (((buf[0] & 0xFF) << 24) | ((buf[1] & 0xFF) << 16) |
           ((buf[2] & 0xFF) << 8) | ((buf[3] & 0xFF) << 0));
    return 0;
}

static void parse_cmdline(int argc, char **argp)
{
	unsigned int i = 1;

	if(argc < 2) {
		show_usage(1);
	}

	if (!strcmp(argp[i], "-h")) {
		show_usage(0);
	}
	else if (!strcmp(argp[i], "-rxq_type")) {
		command = COM_RXQ_TYPE;
		i++;	
		if(argc != 5)
			show_usage(1); 
		parse_dec_val(argp[i++], &port);
		parse_dec_val(argp[i++], &q);
		parse_pt(argp[i++], &value);
	}
	else if (!strcmp(argp[i], "-rxq_mc")) {
		command = COM_RXQ_MC;
		i++;
		if(argc != 6)
			show_usage(1); 
		parse_dec_val(argp[i++], &port);
		parse_dec_val(argp[i++], &q);
		parse_mac(argp[i++], mac);
	}
    	else if (!strcmp(argp[i], "-txdone")) {
        	command = COM_TXDONE_Q;
        	i++;
		if(argc != 3)
			show_usage(1);
        	parse_dec_val(argp[i++], &value);
    	}
        else if (!strcmp(argp[i], "-skb")) {
                command = COM_SKB_RECYCLE;
                i++;
		if(argc != 3)
			show_usage(1);
                parse_dec_val(argp[i++], &value);
        }
        else if (!strcmp(argp[i], "-netdev_sts")) {
                command = COM_NETDEV_STS;
                i++;
                if(argc != 3)
                        show_usage(1);
                name = argp[i++];
        } 
		else if (!strcmp(argp[i], "-port_add")) {
                command = COM_NETDEV_PORT_ADD;
                i++;
                if(argc != 4)
                        show_usage(1);
                name = argp[i++];
                parse_dec_val(argp[i++], &value);
        }
        else if (!strcmp(argp[i], "-port_del")) {
                command = COM_NETDEV_PORT_DEL;
                i++;
                if(argc != 4)
                        show_usage(1);
                name = argp[i++];
                parse_dec_val(argp[i++], &value);
        }
       	else if (!strcmp(argp[i], "-nfp")) {
		 if (argc == 3) {
		 	command = COM_NFP_ENABLE;
			i++;
                	parse_dec_val(argp[i++], &value);
		 }
		 else 
		    	show_usage(1);
        }
	else if (!strcmp(argp[i], "-rxq_time_coal")) {
        	command = COM_RXQ_TIME_COAL;
        	i++;
		if(argc != 5)
			show_usage(1);
        	parse_port(argp[i++]);
		parse_dec_val(argp[i++], &q);
        	parse_dec_val(argp[i++], &value);
    	}
        else if (!strcmp(argp[i], "-txp_bw")) {
                command = COM_TXP_BW;
                i++;
                if(argc != 5)
                        show_usage(1);
		parse_dec_val(argp[i++], &port);
		parse_dec_val(argp[i++], &txp);
                parse_dec_val(argp[i++], &value);
        }
        else if (!strcmp(argp[i], "-rxq_pkts_coal")) {
                command = COM_RXQ_PKTS_COAL;
                i++;
                if(argc != 5)
                        show_usage(1);
                parse_dec_val(argp[i++], &port);
                parse_dec_val(argp[i++], &q);
                parse_dec_val(argp[i++], &value);
        }
    	else if (!strcmp(argp[i], "-txq_coal")) {
        	command = COM_TXQ_COAL;
        	i++;
		if(argc != 6)
			show_usage(1);
                parse_dec_val(argp[i++], &port);
                parse_dec_val(argp[i++], &txp);
                parse_dec_val(argp[i++], &q);
        	parse_dec_val(argp[i++], &value);
    	}
        else if (!strcmp(argp[i], "-txq_wrr")) {
                command = COM_TXQ_WRR;
                i++;
                if(argc != 6)
                        show_usage(1);
                parse_dec_val(argp[i++], &port);
                parse_dec_val(argp[i++], &txp);
                parse_dec_val(argp[i++], &q);
                parse_dec_val(argp[i++], &value);
        }
	else if (!strcmp(argp[i], "-txq_bw")) {
                command = COM_TXQ_BW;
                i++;
                if(argc != 6)
                        show_usage(1);
                parse_dec_val(argp[i++], &port);
                parse_dec_val(argp[i++], &txp);
                parse_dec_val(argp[i++], &q);
                parse_dec_val(argp[i++], &value);
        }
        else if (!strcmp(argp[i], "-ejp")) {
                command = COM_EJP_MODE;
                i++;
                if(argc != 5)
                        show_usage(1);
				parse_dec_val(argp[i++], &port);
				parse_dec_val(argp[i++], &txp);
                parse_dec_val(argp[i++], &value);
	    }
        else if (!strcmp(argp[i], "-rxq_tos")) {
                command = COM_RXQ_TOS_MAP;
                i++;
                if(argc != 5)
                    show_usage(1);

                parse_port(argp[i++]);
		        parse_q(argp[i++]);
                parse_hex_val(argp[i++], &value);
        }
        else if (!strcmp(argp[i], "-txq_tos")) {
                command = COM_TXQ_TOS;
                i++;
                if(argc != 6)
                    show_usage(1);

                parse_port(argp[i++]);
		        parse_dec_val(argp[i++], &txp);
                parse_q(argp[i++]);
                parse_hex_val(argp[i++], &value);
        }
	    else if (!strcmp(argp[i], "-tx_noq")) {
		    command = COM_TX_NOQUEUE;
		    i++;
		    if(argc != 4)
			    show_usage(1);
		    name = argp[i++];
		    parse_dec_val(argp[i++], &value);
	    }
    	else if (!strcmp(argp[i], "-St")) {
        	command = COM_STS;
        	i++;
		if(argc < 3)
			show_usage(1);
	
		parse_status(argp[i++]);
		switch(status)
		{
		    case STS_PNC_REGS:
		    case STS_BM_REGS:
                	if(argc != 3)
                       		show_usage(1);
				break;

                    case STS_TXP_REGS:
		            case STS_TXP_WRR:
                    case STS_PMT_REGS:
                        if(argc != 5)
                            show_usage(1);

                        parse_dec_val(argp[i++], &port);
                        parse_dec_val(argp[i++], &txp);
                        break;
			
		    case STS_PORT_RXQ:
		    case STS_RXQ_REGS:
                        if(argc < 5)
                             show_usage(1);

                        parse_dec_val(argp[i++], &port);
                        parse_dec_val(argp[i++], &q);
		        value = 0;
			if(argc > 5)
				parse_dec_val(argp[i++], &value);
                        break;

		    case STS_PORT_TXQ:
		    case STS_TXQ_REGS:
            	        if(argc < 6)
                	    show_usage(1);

			parse_dec_val(argp[i++], &port);
       			parse_dec_val(argp[i++], &txp);
		        parse_dec_val(argp[i++], &q);
                        value = 0;
                        if(argc > 6)
                            parse_dec_val(argp[i++], &value);
			break;

		    default:
            		if(argc != 4)
                		show_usage(1);
			parse_dec_val(argp[i++], &port);
		}
	}
}

static int procit(void)
{
  	FILE *mvethproc;
  	mvethproc = fopen(FILE_PATH FILE_NAME, "w");
  	if (!mvethproc) {
    		printf ("Eror opening file %s/%s\n",FILE_PATH,FILE_NAME);
    		exit(-1);
  	}

	switch (command) {
		case COM_TXDONE_Q:
		case COM_SKB_RECYCLE:
		case COM_NFP_ENABLE:
			fprintf (mvethproc, ETH_CMD_STRING, ETH_PRINTF_LIST);
			break;

		case COM_TX_NOQUEUE:
		case COM_NETDEV_STS:
		case COM_NETDEV_PORT_ADD:
		case COM_NETDEV_PORT_DEL:
			fprintf(mvethproc, NETDEV_CMD_STRING, NETDEV_PRINTF_LIST);
			break; 

		case COM_EJP_MODE:
		case COM_TXP_BW:
			fprintf (mvethproc, PORT_CMD_STRING, PORT_PRINTF_LIST);
			break;

		case COM_RXQ_TOS_MAP:
		case COM_RXQ_PKTS_COAL:
		case COM_RXQ_TIME_COAL:
		case COM_RXQ_TYPE:
			fprintf (mvethproc, RXQ_CMD_STRING, RXQ_PRINTF_LIST);
			break;

        case COM_TXQ_COAL:
		case COM_TXQ_TOS:
		case COM_TXQ_WRR:
		case COM_TXQ_BW:
                        fprintf (mvethproc, TXQ_CMD_STRING, TXQ_PRINTF_LIST);
                        break;

		case COM_STS:
			fprintf (mvethproc, STATUS_CMD_STRING, STATUS_PRINTF_LIST);
			break;

		default:
 			fprintf(stderr,"Unexpected command\n");
			show_usage(2);
			break;
	}

	fclose (mvethproc);
	return 0;
}

int main(int argc, char **argp, char **envp)
{
        parse_cmdline(argc, argp);
        return procit();
}

