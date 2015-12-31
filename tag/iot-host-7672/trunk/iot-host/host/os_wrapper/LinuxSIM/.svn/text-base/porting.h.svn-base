
#ifndef _OS_PORTING_H_
#define _OS_PORTING_H_
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <timers.h>

/*============OS parameter setting===================*/
typedef pdTASK_CODE				    OsTask;
typedef xTaskHandle				    OsTaskHandle;
typedef xSemaphoreHandle			OsMutex;
typedef xSemaphoreHandle			OsSemaphore;
typedef xQueueHandle				OsMsgQ;
typedef xTimerHandle                OsTimer;
typedef tmrTIMER_CALLBACK           OsTimerHandler;

#define OS_TASK_ENTER_CRITICAL()        taskENTER_CRITICAL()
#define OS_TASK_EXIT_CRITICAL()         taskEXIT_CRITICAL()

#define RX_ISR_PRIORITY    3
#define CMD_ENG_PRIORITY        3
#define WIFI_TX_PRIORITY        3
#define WIFI_RX_PRIORITY        3
#define TCPIP_PRIORITY          2
#define NETAPP_PRIORITY         2
#define NETAPP_PRIORITY_1         2
#define NETAPP_PRIORITY_2         2
#define DHCPD_PRIORITY            2
#define NETMGR_PRIORITY        2

#define TMR_TASK_PRIORITY         1
#define MLME_TASK_PRIORITY      2 
#define TASK_END_PRIO          2

#define TICK_RATE_MS portTICK_RATE_MS
#define TASK_IDLE_STK_SIZE configMINIMAL_STACK_SIZE 


/*============Console setting===================*/
#define PRINTF LOG_PRINTF
#define FFLUSH(x) fflush(x)
 //Changed __linux__ to __SSV_UNIX_SIM__ for CYGWIN compatibility
#ifndef __SSV_UNIX_SIM__
#include <conio.h>
#define hal_getchar   _getch
#else
#define hal_getchar   getch
#endif
#define hal_print printf
#define hal_putchar  putchar


/*============Compiler setting===================*/
//#ifdef __GNUC__
#define PACK( __Declaration__ ) __Declaration__ __attribute__((__packed__));
//#else
//#define PACK( __Declaration__ ) __pragma( pack(push, 1) ) __Declaration__; __pragma( pack(pop) )
//#endif
#undef STRUCT_PACKED
#define STRUCT_PACKED __attribute__ ((packed))
#define UNION_PACKED


/*============SSV-DRV setting===================*/
#define INTERFACE "sdio"
//#if (defined _WIN32)
//#define	CONFIG_RX_POLL              1
//#define	CONFIG_RX_AUTO_ACK_INT      0
//#define snprintf                    _snprintf
//#else
#define	CONFIG_RX_POLL      1
#define SDRV_INCLUDE_SDIO   1
//#endif

#endif
