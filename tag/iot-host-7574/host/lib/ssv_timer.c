#include "rtos.h"
#include "log.h"
#include "common.h"
#include "ssv_lib.h"
#include "ssv_timer.h"

#define SSV_TMR_MAX 30

OsMutex g_tmr_mutex;
struct list_q tmr_hd;
struct list_q free_tmr_hd;
extern struct task_info_st g_host_task_info[];

void SSV_Timer_Task( void *args );

struct task_info_st g_timer_task_info[] =
{
    { "SSV_TMR",    (OsMsgQ)0, 8,   OS_TMR_TASK_PRIO,   64, NULL, SSV_Timer_Task},
};
#define MBOX_TMR_TASK        g_timer_task_info[0].qevt

void os_timer_init(void)
{
    int i;
    struct os_timer *pOSTimer;

    OS_MutexInit( &g_tmr_mutex );
    list_q_init((struct list_q*)&tmr_hd);
    list_q_init((struct list_q*)&free_tmr_hd);

    if (g_timer_task_info[0].qlength> 0) {
        ASSERT(OS_MsgQCreate(&g_timer_task_info[0].qevt,
        (u32)g_timer_task_info[0].qlength)==OS_SUCCESS);
    }

    /* Create Registered Task: */
    OS_TaskCreate(g_timer_task_info[0].task_func,
    g_timer_task_info[0].task_name,
    g_timer_task_info[0].stack_size<<4,
    g_timer_task_info[0].args,
    g_timer_task_info[0].prio,
    NULL);

    for(i=0;i<SSV_TMR_MAX;i++)
    {
        pOSTimer = (struct os_timer *)OS_MemAlloc(sizeof(struct os_timer));
        list_q_qtail(&free_tmr_hd,(struct list_q*)pOSTimer);
    }
}
void _create_timer(struct os_timer *pOSTimer, u32 xElapsed)
{
    struct list_q* qhd = &tmr_hd;
    OS_MutexLock(g_tmr_mutex);
    
    if(qhd->qlen > 0)
    {
        struct list_q* next = qhd->next;
        struct os_timer* tmr_ptr;

        while(next != qhd)
        {
            tmr_ptr = (struct os_timer*)next;
            if(tmr_ptr->msRemian > pOSTimer->msRemian)
            {
                //tmr_ptr->msRemian -= pOSTimer->msRemian;
                //pOSTimer->msRemian -= xElapsed;
                list_q_insert(qhd,next->prev,(struct list_q*)pOSTimer);
                goto DONE;
            }
            else
            {
                pOSTimer->msRemian -= tmr_ptr->msRemian;
            }
            next = next->next;
        }

        list_q_qtail(qhd,(struct list_q*)pOSTimer);
    }
    else
    {
        list_q_qtail(qhd,(struct list_q*)pOSTimer);
    }
DONE:
    //LOG_PRINTF("add TMR(%x):%d,%d,%d,tick=%d\r\n",(u32)pOSTimer,pOSTimer->msRemian,pOSTimer->msTimeout,xElapsed,OS_MS2TICK(pOSTimer->msRemian));
    OS_MutexUnLock(g_tmr_mutex);

}

void _update_all_timer(u32 xElapsed)
{
    struct list_q* qhd = &tmr_hd;
    struct list_q* next = NULL;
    struct os_timer* tmr_ptr;
    MsgEvent *pMsgEv=NULL;

    OS_MutexLock(g_tmr_mutex);

    //LOG_PRINTF("\r\nTMR update :%d\r\n",xElapsed);

    next = qhd->next;

    while(next != qhd)
    {
        tmr_ptr = (struct os_timer*)next;
        if(tmr_ptr->msRemian > xElapsed)
        {
            tmr_ptr->msRemian -= xElapsed;
            //LOG_PRINTF("next TMR:%d,%x\r\n",tmr_ptr->msRemian,tmr_ptr->nMTData0);
            next = next->next;
        }
        else
        {
            //LOG_PRINTF("Time's up:%,%d,%d,%x\r\n",tmr_ptr->msTimeout,tmr_ptr->msRemian,tmr_ptr->nMTData0); 
            tmr_ptr = (struct os_timer*)list_q_deq(qhd);
            //LOG_PRINTF("Time's up:%x\r\n",(u32)tmr_ptr);
            if(tmr_ptr->handler)
            {
                pMsgEv=msg_evt_alloc();
                if(pMsgEv)
                {
                    pMsgEv->MsgType=tmr_ptr->nMsgType;
                    pMsgEv->MsgData=(u32)tmr_ptr->handler;
                    pMsgEv->MsgData1=tmr_ptr->nMTData0;
                    pMsgEv->MsgData2=tmr_ptr->nMTData1;
                    pMsgEv->MsgData3=0;
                    if(tmr_ptr->infombx)
                    {
                        msg_evt_post((OsMsgQ)tmr_ptr->infombx, pMsgEv);
                    }
                    else
                    {
                        //OS_MemFree((void*)tmr_ptr);
                        //list_q_qtail(free_tmr_hd,(struct list_q*)tmr_ptr);
                        LOG_PRINTF("infombx error\r\n");
                    }
                }
                else
                {
                    //OS_MemFree((void*)tmr_ptr);                    
                    //list_q_qtail(free_tmr_hd,(struct list_q*)tmr_ptr);
                    LOG_PRINTF("tmr alloc evt fail\r\n");
                }
            }
            else
            {
                LOG_PRINTF("invalid tmr\r\n");
                //OS_MemFree((void*)tmr_ptr);
            }
            list_q_qtail(&free_tmr_hd,(struct list_q*)tmr_ptr);
            next = qhd->next;
        }
    }
    
    OS_MutexUnLock(g_tmr_mutex);
}

void _cancel_timer(timer_handler handler, u32 data1, u32 data2)
{
    struct list_q* qhd = &tmr_hd;
    OS_MutexLock(g_tmr_mutex);

    if(qhd->qlen > 0)
    {
        struct list_q* next = qhd->next;
        struct os_timer* tmr_ptr;

        while(next != qhd)
        {
            tmr_ptr = (struct os_timer*)next;
            if (handler == tmr_ptr->handler && 
                tmr_ptr->nMTData0 == data1 &&
                tmr_ptr->nMTData1 == data2 )
            {
                tmr_ptr->handler = NULL;
                list_q_remove(qhd,(struct list_q*)tmr_ptr);
                list_q_qtail(&free_tmr_hd,(struct list_q*)tmr_ptr);
                //LOG_PRINTF("cancel_timer:%x\r\n",(u32)tmr_ptr);
                break;
            }
            next = next->next;
        }
    }
    OS_MutexUnLock(g_tmr_mutex);
}

void SSV_Timer_Task( void *args )
{
    u32 xStartTime, xEndTime, xElapsed;
    MsgEvent *MsgEv = NULL;
    struct os_timer* cur_tmr = NULL;

    LOG_PRINTF("SSV_Timer_Task Start,hd=%x\r\n",(u32)&tmr_hd);
    while(1)
    {
        xElapsed = 0;
        xStartTime = OS_GetSysTick();
        cur_tmr = (struct os_timer*)tmr_hd.next;
        if(cur_tmr != (struct os_timer*)&tmr_hd)
        {
            //LOG_PRINTF("cur_tmr=%x,%d,%d,xStartTime=%d\r\n",(u32)cur_tmr,cur_tmr->msTimeout,cur_tmr->msRemian,xStartTime);
            if( OS_SUCCESS == msg_evt_fetch_timeout( MBOX_TMR_TASK, &MsgEv,cur_tmr->msRemian))
            {
                xEndTime = OS_GetSysTick();
                xElapsed = ( xEndTime - xStartTime ) * OS_TICK_RATE_MS;
                //LOG_PRINTF("Not timeout wakup,cur_tmr=%x,xElapsed=%d,xEndTime=%d,msgtype=%d\r\n",(u32)cur_tmr,xElapsed,xEndTime,MsgEv->MsgType);
            }
            else //Time out; expire timer
            {
                xElapsed = cur_tmr->msRemian;//( xEndTime - xStartTime ) * OS_TICK_RATE_MS;
            }
        }
        else
        {
            //LOG_PRINTF("\r\nNO TMR\r\n");
            msg_evt_fetch(MBOX_TMR_TASK, &MsgEv); //There's no TMR.Block till get msg.
        }

        if(xElapsed)
        {
            _update_all_timer(xElapsed);
        }
         
        if(MsgEv)
        {           
            switch(MsgEv->MsgType)
            {
                case TMR_EVT_CREATE:
                {
                    struct os_timer *pOSTimer = (struct os_timer *)MsgEv->MsgData;
                    _create_timer(pOSTimer,xElapsed);
                }
                break;
                case TMR_EVT_CANCEL:
                {
                    _cancel_timer((timer_handler)MsgEv->MsgData, MsgEv->MsgData1, MsgEv->MsgData2);
                }
                break;
                default:
                break;
            }
            os_msg_free(MsgEv);
            MsgEv = NULL;
        }
    }
}

s32 os_create_timer(u32 ms, timer_handler handler, void *data1, void *data2, void* mbx)
{
    s32 ret = 0;


    struct os_timer *pOSTimer;
    MsgEvent *pMsgEv=NULL;

    //pOSTimer = (struct os_timer *)OS_MemAlloc(sizeof(struct os_timer));
    OS_MutexLock(g_tmr_mutex);
    //LOG_PRINTF("free_tmr_hd len=%d\r\n",free_tmr_hd.qlen);
    pOSTimer = (struct os_timer*)list_q_deq(&free_tmr_hd);
    OS_MutexUnLock(g_tmr_mutex);
    //LOG_PRINTF("create TMR=%x\r\n",(u32)pOSTimer);
    if(pOSTimer)
    {
        pOSTimer->nMsgType = MEVT_HOST_TIMER;
        pOSTimer->handler = handler;
        pOSTimer->nMTData0 = (u32)data1;
        pOSTimer->nMTData1 = (u32)data2;
        pOSTimer->msTimeout = pOSTimer->msRemian = ms;
        pOSTimer->infombx = mbx;

        pMsgEv=msg_evt_alloc();
        if(pMsgEv)
        {
            pMsgEv->MsgType=TMR_EVT_CREATE;
            pMsgEv->MsgData=(u32)pOSTimer;
            pMsgEv->MsgData1=0;
            pMsgEv->MsgData2=0;
            pMsgEv->MsgData3=0;
            ret = msg_evt_post(MBOX_TMR_TASK, pMsgEv);
            return ret;
        }
    }
    ret = OS_FAILED;

    return ret;
}

s32 os_cancel_timer(timer_handler handler, u32 data1, u32 data2)
{
    s32 ret = 0;
    MsgEvent *pMsgEv=NULL;

    pMsgEv=msg_evt_alloc();
    if(pMsgEv)
    {
        pMsgEv->MsgType=TMR_EVT_CANCEL;
        pMsgEv->MsgData=(u32)handler;
        pMsgEv->MsgData1=data1;
        pMsgEv->MsgData2=data2;
        pMsgEv->MsgData3=0;
        ret = msg_evt_post(MBOX_TMR_TASK, pMsgEv);
        return ret;
    }
    else
    {
        return OS_FAILED;
    }
}
void os_timer_expired(struct os_timer *pOSTimer)
{
#if 0
	int i;
	timer_handler	handler;
#ifdef __AP_DEBUG__
	bool bFound = FALSE;
#endif//__AP_DEBUG__


	for (i=0;i<OS_TIMER_DESP_NUM;i++)
	{	          
		if (pOSTimer == &gHCmdEngInfo->timer[i])
		{
#ifdef __AP_DEBUG__
			bFound = TRUE;
#endif//__AP_DEBUG__
			handler = pOSTimer->handler;
			pOSTimer->handler = NULL;
			handler((void*)pOSTimer->nMTData0, (void*)pOSTimer->nMTData1);
			break;
		}
			
	}

#ifdef __AP_DEBUG__
	if(!bFound)
		LOG_FATAL( "could not find any handler");
#endif//__AP_DEBUG__
	
#endif
}
void test_expired_handler(void* data1, void* data2)
{
    LOG_PRINTF("test_expired_handler %x\r\n",(u32)data1);
}
u32 tmrData1;
void cmd_tmr(s32 argc, s8 *argv[])
{
    u16 timeout;
    //u32 data1;
    if(argc > 1)
    {
        timeout = (u16)ssv6xxx_atoi(argv[1]);
        tmrData1 = OS_Random();
        LOG_PRINTF("test TMR = %dms,data1=%x\r\n",timeout,tmrData1);
        os_create_timer(timeout,test_expired_handler,(void*)tmrData1,NULL, (void*)MBOX_HCMD_ENGINE);
    }
    else
    {
        LOG_PRINTF("Cancel timer\r\n");
        os_cancel_timer(test_expired_handler,tmrData1, 0);
    }
}

