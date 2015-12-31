#include <cmd_def.h>
#include "os_wrapper.h"
#include "txrx_task.h"

#define NUM_PRI_Q 5
#define ONE_TR_THREAD

OsMutex txrx_mtx;
ModeType  curr_mode;


#ifdef ONE_TR_THREAD
void TXRX_Task(void *args);
#else
void TXRX_TxTask(void *args);
void TXRX_RxTask(void *args);
#endif

struct task_info_st g_txrx_task_info[] =
{
#ifdef ONE_TR_THREAD    
	{ "txrx_task",   (OsMsgQ)0, 0, 	OS_TX_TASK_PRIO, 64, NULL, TXRX_Task },
#else
    { "txrx_txtask",   (OsMsgQ)0, 0, 	OS_TX_TASK_PRIO, 64, NULL, TXRX_TxTask },
    { "txrx_rxtask",   (OsMsgQ)0, 0, 	OS_RX_TASK_PRIO, 64, NULL, TXRX_RxTask },
#endif
};

void TXRX_FlushFrames()
{
    
}

//TXRX_RxFrameProc Process rx frame
s32 TXRX_RxFrameProc(void* frame)
{
    return 0;
}

//TXRX_Task TXRX main thread
#ifdef ONE_TR_THREAD
void TXRX_Task(void *args)
{
    while(1)
    {
        
    }
    
}
#else
void TXRX_TxTask(void *args)
{
    while(0)
    {
        
    }
    
}

void TXRX_RxTask(void *args)
{
    while(0)
    {
        
    }
    
}
#endif


//TXRX_Init Initialize 
s32 TXRX_Init()
{    
    u32 i, size, res=OS_SUCCESS;

	OS_MutexInit(&txrx_mtx);
    curr_mode = MT_STOP;
    
	size = sizeof(g_txrx_task_info)/sizeof(struct task_info_st);
	for (i = 0; i < size; i++) 
    {		
		/* Create Registered Task: */
		OS_TaskCreate(g_txrx_task_info[i].task_func, 
			g_txrx_task_info[i].task_name,
			g_txrx_task_info[i].stack_size<<4, 
			g_txrx_task_info[i].args, 
			g_txrx_task_info[i].prio, 
			NULL); 
	}
    
    return res;
}

//TXRX_FrameEnqueue Enqueue tx frames 
s32 TXRX_FrameEnqueue(void* frame, u32 priority)
{
    return 0;
}

//TXRX_SetOpMode Set the operation mode
ssv6xxx_result TXRX_SetOpMode(ModeType mode)
{
    return SSV6XXX_SUCCESS;
}