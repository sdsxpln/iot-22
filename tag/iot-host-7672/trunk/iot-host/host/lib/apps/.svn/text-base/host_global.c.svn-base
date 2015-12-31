#include <config.h>
#include "dev.h"

/* cli: */
s8 gCmdBuffer[CLI_BUFFER_SIZE+1];



#if (CLI_HISTORY_ENABLE==1)

s8 gCmdHistoryBuffer[CLI_HISTORY_NUM][CLI_BUFFER_SIZE+1];
s8 gCmdHistoryIdx;
s8 gCmdHistoryCnt;

#endif//CLI_HISTORY_ENABLE


/*  Global Variables Declaration: */
ETHER_ADDR WILDCARD_ADDR = { {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };


u32 gTxFlowDataReason;
u32 gRxFlowDataReason;
u16 gTxFlowMgmtReason;
u16 gRxFlowMgmtReason;
u16 gRxFlowCtrlReason;


u32 g_free_msgevt_cnt;


/* wsimp_config: */
u8 g_soc_cmd_buffer[1024];


/* Task Count to indicate if all tasks are running. */
u32 g_RunTaskCount;
u32 g_MaxTaskCount;

u8 g_sta_mac[6];


//#include <sim_regs.h>
//#include <security/drv_security.h>


//extern SRAM_KEY			*pGSRAM_KEY;

//OsMutex g_hcmd_blocking_mutex;
OsMutex			g_wsimp_mutex;
OsMutex			g_beacon_mutex;





/* For cli_cmd_soc commands: */
s32 g_soc_cmd_rx_ready = 0;
s8 g_soc_cmd_rx_buffer[1024];
s8 g_soc_cmd_prepend[50];


u32 g_sim_net_up = 0;
u32 g_sim_link_up = 0;

DeviceInfo_st *gDeviceInfo;

//debug
#if CONFIG_STATUS_CHECK
u32 g_l2_tx_packets;
u32 g_l2_tx_copy;
u32 g_l2_tx_late;
u32 g_notpool;
u32 g_heap_used;
u32 g_heap_max;
#endif

void host_global_init(void)
{

#if CONFIG_STATUS_CHECK    
    g_l2_tx_packets = 0;
    g_l2_tx_copy=0;
    g_l2_tx_late=0;
	g_notpool=0;
    g_heap_used=0;
    g_heap_max=0;
#endif 


#if (CLI_HISTORY_ENABLE==1)
	{
		int i;
		gCmdHistoryIdx = 0;
		gCmdHistoryCnt = 0;

		for(i=0;i<CLI_HISTORY_NUM; i++)
			gCmdHistoryBuffer[i][0]=0x00;		
	}
	
#endif

    ssv6xxx_memset((void *)gCmdBuffer, 0, CLI_BUFFER_SIZE+1);
    ssv6xxx_memset((void *)g_soc_cmd_buffer, 0, 1024);
    ssv6xxx_memset((void *)g_soc_cmd_prepend, 0, 50);
//    OS_MutexInit(&g_hcmd_blocking_mutex);
    g_RunTaskCount = 0;
    g_MaxTaskCount = 0;

	OS_MutexInit(&g_wsimp_mutex);
	OS_MutexInit(&g_beacon_mutex);
	
}




void sim_global_bcn_lock()
{
	OS_MutexLock(g_beacon_mutex);
}

void sim_global_bcn_unlock()
{
	OS_MutexUnLock(g_beacon_mutex);

}




