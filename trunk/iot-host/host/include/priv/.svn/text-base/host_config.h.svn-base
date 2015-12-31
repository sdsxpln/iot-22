#ifndef _SIM_CONFIG_H_
#define _SIM_CONFIG_H_

#include <types.h>
#include <rtos.h>

#include <ssv_ex_lib.h>
// ------------------------- debug ---------------------------

#define CONFIG_STATUS_CHECK         1
#define AP_MODE_BEACON_VIRT_BMAP_0XFF         1		// 1: FIX VIRT BMAP 0XFF, 0:DYNAMIC VIRT BMAP

//-----------------------------------------------------------
#define Sleep                       OS_MsDelay
#define AUTO_INIT_STATION           1


#define CONFIG_USE_LWIP_PBUF        1



#define CONFIG_CHECKSUM_DCHECK      0
#define CONFIG_LOG_ENABLE			1				// turn on/off ssv log module
#define TARGET						prj1_config




// ------------------------- log -------------------------------
#define SSV_LOG_DEBUG           1


/** lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */
#define LOG_LEVEL_ALL     0x00
#define LOG_LEVEL_WARNING 0x01 	/* bad checksums, dropped packets, ... */
#define LOG_LEVEL_SERIOUS 0x02 	/* memory allocation failures, ... */
#define LOG_LEVEL_SEVERE  0x03
#define LOG_LEVEL_MASK_LEVEL 0x03


#define LOG_NONE	       			   (0)
#define LOG_MEM	       			       (1<<16)
#define LOG_L3_SOCKET				   (1<<17)
#define LOG_L3_API				   	   (1<<18)
#define LOG_L3_TCP					   (1<<19)
#define LOG_L3_UDP					   (1<<20)
#define LOG_L3_IP					   (1<<21)
#define LOG_L3_OTHER_PROTO			   (1<<22)
#define LOG_L2_DATA			           (1<<23)
#define LOG_L2_AP			           (1<<24)
#define LOG_L2_STA			           (1<<25)
#define LOG_CMDENG			           (1<<26)
#define LOG_TXRX			           (1<<27)

#define LOG_L4_HTTPD                   (1<<29)
#define LOG_L4_NETMGR   	           (1<<30)
#define LOG_L4_DHCPD			       (1<<31)
#define LOG_L4_IPERF                   (1<<32)




#define LOG_ALL_MODULES                (0xffffffff)

//***********************************************//
//Modify default log config
#define CONFIG_LOG_MODULE				(LOG_TXRX)
#define CONFIG_LOG_LEVEL				(LOG_LEVEL_SERIOUS)

// ------------------------- chip -------------------------------
#define SSV6051Q    0
#define SSV6051Z    1
#define SSV6060P    2

#define CONFIG_CHIP_ID     SSV6051Q

// ------------------------- bsp -------------------------------
#define HZ							1000            // 1000Hz => 1000 ticks per second
#define UART_BAUD_RATE				115200          // UART Baud Rate
#define IRQ_STACK_SIZE				256             // IRQ stack size
#define MAX_MAIN_IRQ_ENTRY			10
#define MAX_PERI_IRQ_ENTRY			10

// ------------------------- rtos -------------------------------
#define OS_MUTEX_SIZE				10              // Mutex
#define OS_COND_SIZE				5               // Conditional variable
#define OS_TIMER_SIZE				5               // OS Timer
#define OS_MSGQ_SIZE				5               // Message Queue

// ------------------------- cli -------------------------------
#define CLI_ENABLE					1               // Enable/Disable CLI
#define CLI_HISTORY_ENABLE			1               // Enable/Disable CLI history log. only work in SIM platofrm for now.
#define CLI_HISTORY_NUM				10


#define CLI_BUFFER_SIZE				80              // CLI Buffer size
#define CLI_ARG_SIZE				20             // The number of arguments
#define CLI_PROMPT					"wifi-host> "

// ------------------------- misc ------------------------------
/* Message Queue Length: */
#define MBOX_MAX_MSG_EVENT			32              // Max message event in the system

// ------------------------- mlme ------------------------------
/*Regular iw scan or regulare ap list update*/
#define CONFIG_AUTO_SCAN            0               // 1=auto scan, 0=auto flush ap_list table
#define MLME_TASK                   0               // MLME function 0=close,1=open. PS:If MLME_TASK=1, please re-set the stack size to 64 in porting.h

#if (MLME_TASK == 0)
#define TIMEOUT_TASK                MBOX_CMD_ENGINE               // MLME function 0=close,1=open
#else
#define TIMEOUT_TASK                MBOX_MLME_TASK               // MLME function 0=close,1=open
#endif

// ------------------------- txrx ------------------------------
#define RXFLT_ENABLE                1               // Rx filter for data frames
#define ONE_TR_THREAD               0               // one txrx thread. PS: If ONE_TR_THREAD=1, please re-set the value of WIFI_RX_STACK_SIZE to 0 in porting.h

// ------------------------- iperf3 ---------------------------
/* TCP/IP Configuration: */
#define IPERF3_ENABLE               1

// ------------------------- network ---------------------------
/* TCP/IP Configuration: */
#define __LWIP__						1
#define CONFIG_MAX_NETIF				1
#define TCPIP_ENABLE        			0
#define AP_MODE_ENABLE      			0
#define WLAN_MAX_STA					2
#define CONFIG_PLATFORM_CHECK           1
#define CONFIG_MEMP_DEBUG               1
#define LWIP_PARAM_SET                  2           // 0 for minimal resources, 1 for default, 2 for maximal resources
// ------------------------- mac address  ------------------------------
#define CONFIG_EFUSE_MAC            1
#define	CONFIG_RANDOM_MAC           1

// ------------------------- STA/SCONFIG mode channel mask  ------------------------------
//This macro is for STA mode and SmartConfig mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 0, bit[1] ->cahnnel 1.
//If user assign 0 to channel mask to netmgr_wifi_scan or netmgr_wifi_sconfig, we use the default value.
//If youe set ch0 and ch15, we will filter it automatically
#define DEFAULT_STA_CHANNEL_MASK 0x3FFE //from 1~14

// ------------------------- auto channel selection ---------------------------
//The ACS_CHANNEL_MASK is for auto channel selection in AP  mode, evey bit corrsebonds to a channel, for example: bit[0] -> channel 0, bit[1] ->cahnnel 1.
//Now, we set 0xFFE, this value means we do auto channel selection from channel 1 to channel 11.
//If you just want to choose a channel from 1, 6,11, you must set ACS_CHANNEL_MASK  to 0x842
//If youe set ch0, ch12, ch13, ch14, or ch15, we will filter it automatically
#define ACS_CHANNEL_MASK 0xFFE

//The g_acs_channel_scanning_interval is for auto channel secltion in AP mode, if you set 10, it means AP will stay in one channel for 10x10ms,
//and then change to the next channel.
#define ACS_SCANNING_INTERVAL 10 //The unit is 10ms

//This macro is used to set the number of times that you want to do the channel scanning.
//If this macro is 1, we do channel scanning one times, if this macro is 2, we do channel scanning two times, and then we choose a channel by all datas
#define ACS_SCANNING_LOOP 2

// ------------------------- Smart Config ---------------------------
#define ENABLE_SMART_CONFIG 1
#define CONFIG_USER_MODE_SMART_CONFIG 0 // 1: user mode (do smart configuration on host). 0:non-user mode  (do smart configuration on fw, fw only support airkiss now)
#ifndef __SSV_UNIX_SIM__
#undef CONFIG_USER_MODE_SMART_CONFIG
#define CONFIG_USER_MODE_SMART_CONFIG 0
#endif

#if (AP_MODE_ENABLE == 1)

	#define __TEST__
	//#define __TEST_DATA__  //Test data flow
	//#define __TEST_BEACON__

#else
	#define __STA__

	//#define __TCPIP_TEST__
#endif


//#define __AP_DEBUG__

// ------------------------- rate control ---------------------------

#define RC_DEFAULT_RATE_MSK 0x0EE0
#define RC_ARITH_SHIFT 8
#define RC_TARGET_PF 16

//==============EDCA===============
//#define EDCA_PATTERN_TEST
#ifdef EDCA_PATTERN_TEST
#define EDCA_DBG						1		//Enable to test edca function
#define MACTXRX_CONTROL_ENABLE			1		//To simulate MAC TX operation. It's also enable ASIC queue empty interrupt.
#define MACTXRX_CONTROL_DURATION_SIM	1		//TX control use softmain edca handler to test MAC TX EDCA function
//#define __EDCA_INT_TEST__						//
//#define __EDCA_NOTIFY_HOST__					//When TX done send an event to nofity host to know
#define BEACON_DBG						1
#else
#define EDCA_DBG						0		//Enable to test edca function
#define MACTXRX_CONTROL_ENABLE			0		//To simulate MAC TX operation. It's also enable ASIC queue empty interrupt.
#define MACTXRX_CONTROL_DURATION_SIM	0		//TX control use softmain edca handler to test MAC TX EDCA function
//#define __EDCA_INT_TEST__						//
//#define __EDCA_NOTIFY_HOST__					//When TX done send an event to nofity host to know
#define BEACON_DBG						0
#endif


//=================================

//#define PACKED

/* default ip */
#define DEFAULT_IPADDR   "192.168.25.1"
#define DEFAULT_SUBNET   "255.255.255.0"
#define DEFAULT_GATEWAY  "192.168.25.1"
#define DEFAULT_DNS      "0.0.0.0"

/* default dhcp server info */
#define DEFAULT_DHCP_START_IP    "192.168.25.101"
#define DEFAULT_DHCP_END_IP      "192.168.25.110"
#define DEFAULT_DHCP_MAX_LEASES  10

#define DEFAULT_DHCP_AUTO_TIME       7200
#define DEFAULT_DHCP_DECLINE_TIME    3600
#define DEFAULT_DHCP_CONFLICT_TIME   3600
#define DEFAULT_DHCP_OFFER_TIME      60
#define DEFAULT_DHCP_MIN_LEASE_SEC   60

/* test app */
#define APP_MJS							0

/* watchdog */
#define ENABLE_WATCHDOG					1

// ------------------------- Power Saving ---------------------------
#define BUS_IDLE_TIME 0 //5000 //ms

#endif /* _SIM_CONFIG_H_ */
