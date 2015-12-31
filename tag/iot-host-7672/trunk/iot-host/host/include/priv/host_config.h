#ifndef _SIM_CONFIG_H_
#define _SIM_CONFIG_H_

#include <types.h>
#include <rtos.h>

#include <ssv_ex_lib.h>
// ------------------------- debug ---------------------------

#define CONFIG_STATUS_CHECK         1


//-----------------------------------------------------------
#define Sleep                       OS_MsDelay
#define AUTO_INIT_STATION           1


#define CONFIG_USE_LWIP_PBUF        1



#define CONFIG_CHECKSUM_DCHECK      0
#define CONFIG_LOG_ENABLE			1				// turn on/off ssv log module
#define TARGET						prj1_config
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
#define CLI_ARG_SIZE				100             // The number of arguments
#define CLI_PROMPT					"wifi-host> "

// ------------------------- misc ------------------------------
/* Message Queue Length: */
#define MBOX_MAX_MSG_EVENT			48              // Max message event in the system

// ------------------------- mlme ------------------------------
/*Regular iw scan or regulare ap list update*/
#define CONFIG_REGULAR_IW_SCAN      1               // MLME sta mode aplist handle with regular iw scan or not 
#define MLME_SETTING                0               // MLME function 0=close,1=open 

// ------------------------- txrx ------------------------------
#define OLDCMDENG_TRX               1               // Macro for old architecture
#if (OLDCMDENG_TRX == 0)
#define RXFLT_ENABLE                1               // Rx filter for data frames
#define TXRXTASK_ENABLE             1               // Use txrxtask instead of TX_DRV_Task and RX_DRV_Task
#define ONE_TR_THREAD               0               // one txrx thread
#endif

// ------------------------- network ---------------------------
/* TCP/IP Configuration: */
#define __LWIP__						1
#define CONFIG_MAX_NETIF				1
#define TCPIP_ENABLE        			0
#define AP_MODE_ENABLE      			0
#define WLAN_MAX_STA					2
#define CONFIG_PLATFORM_CHECK           1
#define CONFIG_MEMP_DEBUG               1


#if (AP_MODE_ENABLE == 1)
	
	#define __TEST__
	//#define __TEST_DATA__  //Test data flow
	//#define __TEST_BEACON__  

#else
	#define __STA__

	//#define __TCPIP_TEST__
#endif


//#define __AP_DEBUG__



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

#endif /* _SIM_CONFIG_H_ */

