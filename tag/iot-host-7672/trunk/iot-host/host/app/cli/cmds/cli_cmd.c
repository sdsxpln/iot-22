
#include <config.h>
#include <msgevt.h>
#include <cli/cli.h>
#include <regs/ssv6200_reg.h>
#include <drv/ssv_drv_config.h>
#include <drv/ssv_drv.h>
#include "cli_cmd.h"
#include "cli_cmd_soc.h"
#include "cli_cmd_net.h"
#include "cli_cmd_wifi.h"
#include "cli_cmd_sim.h"
#include "cli_cmd_test.h"
#include "cli_cmd_sdio_test.h"
#include "cli_cmd_throughput.h"
#include "cli_cmd_sys.h"
#include <log.h>
#include <ssv_lib.h>
//#include <regs.h>
#include <cmd_def.h>
#include <host_apis.h>

#if (defined _WIN32)
#include <drv/sdio/sdio.h>
#elif (defined __linux__)
#include <drv/sdio_linux/sdio.h>
#endif
//#include <ssv6200_uart_bin.h>

#include <os_wrapper.h>



/*---------------------------------- CMDs -----------------------------------*/

static void cmd_abort( s32 argc, s8 *argv[] )
{
    //char *prt = NULL;
	//printf("%d\n", *prt);	
    abort();
}

#ifdef __SSV_UNIX_SIM__
extern char SysProfBuf[256];
extern s8 SysProfSt;
#endif

static void cmd_exit( s32 argc, s8 *argv[] )
{
	//ssv6xxx_drv_module_release();
#ifdef __SSV_UNIX_SIM__
    if(SysProfSt == 1)
    {
        s16 i;

        LOG_PRINTF("*************System profiling result*************\n");
        for(i=0; i<256; i++)
            LOG_PRINTF("%c",SysProfBuf[i]);

        LOG_PRINTF("*************End of profiling result*************\n");
    }
    LOG_PRINTF("cmd_exit");
    reset_term();
#endif
    ssv6xxx_wifi_deinit();
    OS_Terminate(); 
}

static void cmd_sdrv(s32 argc, s8 *argv[])
{
    // Usage : sdrv {list} | {select [sim|uart|usb|sdio|spi]}
	if (argc == 2 && strcmp(argv[1], "list") == 0)
	{
		ssv6xxx_drv_list();
		return;
	}
	else if ((argc == 2) && ((strcmp(argv[1], DRV_NAME_SIM)  == 0) ||
							 (strcmp(argv[1], DRV_NAME_UART) == 0) ||
							 (strcmp(argv[1], DRV_NAME_USB)  == 0) ||
							 (strcmp(argv[1], DRV_NAME_SDIO) == 0) ||
							 (strcmp(argv[1], DRV_NAME_SPI)  == 0)))
	{
		ssv6xxx_drv_select(argv[1]);
		return;
	}
    log_printf("Usage : sdrv list\n"  \
			   "desc  : list all available drivers in ssv driver module.\n\n" \
			   "Usage : sdrv [sim|uart|usb|sdio|spi]\n" \
		       "desc  : select [sim|uart|usb|sdio|spi] driver to use.\n");
}


static void cmd_dump( s32 argc, s8 *argv[] )
{
    u8 dump_all;

    if (argc != 2) {
        LOG_PRINTF("Invalid Parameter !\n");
        return;
    }

    if (strcmp(argv[1], "all") == 0)
        dump_all = true;
    else dump_all = false;

    /* Dump PHY-Info: */
    if (strcmp(argv[1], "phyinfo")==0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_PHY_INFO_TBL, NULL, 0);
       
    }

    /* Dump Decision Table: */
    if (strcmp(argv[1], "decision") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_DECI_TBL, NULL, 0);

    }
    
    /* Dump STA MAC Address: */
    if (strcmp(argv[1], "sta-mac") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_STA_MAC, NULL, 0);
    }    

    
    /* Dump STA BSSID: */
    if (strcmp(argv[1], "bssid") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_BSSID, NULL, 0);

    }    

    /* Dump Ether Trap Table: */
    if (strcmp(argv[1], "ether-trap") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_ETHER_TRAP, NULL, 0);

    }    

    /* Dump Flow Command Table: */
    if (strcmp(argv[1], "fcmd") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_FCMDS, NULL, 0);

    }    

    /* Dump WSID Table: */
    if (strcmp(argv[1], "wsid") == 0 || dump_all==true) {
        if (dump_all == false)
            ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_WSID_TBL, NULL, 0);

    }    

}


static void cmd_phy_config( s32 argc, s8 *argv[] )
{
	ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_PHY_ON, NULL, 0);
}

static void cmd_cal( s32 argc, s8 *argv[] )
{
	 u32 channel_index= 0;
	 channel_index=(u32)ssv6xxx_atoi(argv[1]);
	 ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_CAL, &channel_index, sizeof(u32));
}


#if CONFIG_STATUS_CHECK

extern void stats_display(void);
extern void stats_p(void);
extern void stats_m(void);
extern void SSV_PBUF_Status(void);

extern u32 g_l2_tx_packets;
extern u32 g_l2_tx_copy;
extern u32 g_l2_tx_late;
extern u32 g_notpool;

extern u32 g_heap_used;
extern u32 g_heap_max;


void dump_protocol(void){
    stats_p();
    LOG_PRINTF("l2 tx pakets[%d]\n\tl2_tx_copy[%d]\n\tl2_tx_late[%d]\n\tg_notpool[%d]\n\t", 
        g_l2_tx_packets, g_l2_tx_copy, g_l2_tx_late, g_notpool);
}



void dump_mem(void){
    
    stats_m();
#if (CONFIG_USE_LWIP_PBUF == 0)
    SSV_PBUF_Status();
#endif
    LOG_PRINTF("g_heap_used[%lu] g_heap_max[%lu]\n", g_heap_used, g_heap_max);

}


#if CONFIG_MEMP_DEBUG
#include "lwip/memp.h"
extern void dump_mem_pool(memp_t type);
extern void dump_mem_pool_pbuf();
#endif

void cmd_tcpip_status (s32 argc, s8 *argv[]){

    if(argc >=2){
        
        if (strcmp(argv[1], "p") == 0) {
            dump_protocol();
            return;
        }
        else if (strcmp(argv[1], "m") == 0) {
            dump_mem();        
            return;
        }
#if CONFIG_MEMP_DEBUG        
        else if(strcmp(argv[1], "mp") == 0){

            dump_mem_pool(ssv6xxx_atoi(argv[2]));
            return;
        }
        else if(strcmp(argv[1], "mpp") == 0){
            dump_mem_pool_pbuf();

             LOG_PRINTF("l2 tx pakets[%d]\n\tl2_tx_copy[%d]\n\tl2_tx_late[%d]\n\tg_notpool[%d]\n\t", 
                    g_l2_tx_packets, g_l2_tx_copy, g_l2_tx_late, g_notpool);
            return;
        }
        
#endif//#if CONFIG_MEMP_DEBUG
        

        
    }

    dump_protocol();
    dump_mem();           
}
#endif//CONFIG_STATUS_CHECK


CLICmds gCliCmdTable[] = 
{
    { "exit",       cmd_exit,           "Terminate uWifi system."           },
    { "abort",       cmd_abort,           "force abort"           },
    
	{ "sdrv",       cmd_sdrv,           "SSV driver module commands."       },
    { "r",          cmd_read,           "Read SoC"                          },
    { "w",          cmd_write,          "Write SoC"                         },

    { "ifconfig",   cmd_ifconfig,       "Network interface configuration"   },
    { "route",      cmd_route,          "Routing table manipulation"        },
    { "ping",       cmd_ping,           "ping"                              },
    { "ttcp",       cmd_ttcp,           "ttcp"                              },
    //{ "dhcpd",       cmd_dhcpd,           "dhcpd"                           },
    //{ "dhcpc",       cmd_dhcpc,           "dhcpc"                          },
    { "iperf3",      cmd_iperf3,         "throughput testing via tcp or udp"},
    { "netapp",     cmd_net_app,        "Network application utilities"     },
    { "netmgr",     cmd_net_mgr,        "Network Management utilities"     },

    { "iw",         cmd_iw,             "Wireless utility"                  },
    
   // { "pattern",    cmd_pattern,        "Pattern mode"                      },

//	{ "setnetif",   cmd_setnetif,		"SetNetIf Ip Mask Gateway Mac"		},
	//{ "socket-c",   cmd_createsocketclient, "Create Socket client and set server addreass" },
	{ "52w",        cmd_write_52,          "Write 52 dommand"               },

#ifdef __AP_DEBUG__
   // { "ap"		,   cmd_ap				, "list ap mode info." 				},
   // { "test"		,cmd_test			, "send test frame"					},
#endif

    { "dump",       cmd_dump,           "Do dump."                          },
	{ "phy-on",		cmd_phy_config,		"Load PHY configuration"			},
    { "cal",        cmd_cal,			"Calibration Process"				},
#ifdef THROUGHPUT_TEST
    { "sdio",		cmd_sdio_test,		"SDIO throughput test(from HOST)"	},
    { "dut",  		cmd_throughput,		"DUT throughput test(from DUT)"		},
#endif    
	
//    { "phy",        Cmd_Phy,            "phy" },
    { "delba",  	cmd_wifi_delba,		"DELBA"		                        },

#if CONFIG_STATUS_CHECK
    { "s",     cmd_tcpip_status,    "dump tcp/ip status"                },
#endif
    { "ctl",       cmd_ctl,           "wi-fi interface control (AP/station on or off)"       },
    { "sys",       cmd_sys,           "Components info"       },

    { NULL, NULL, NULL },
};

