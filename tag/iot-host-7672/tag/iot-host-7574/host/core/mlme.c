#include "rtos.h"
#include "log.h"
#include "mlme.h"
#include "ssv_timer.h"
#include "host_cmd_engine_priv.h"
#include "dev.h"
#include "common.h"
#include <cli/cmds/cli_cmd_wifi.h>
#include <cmd_def.h>

#define TIMER_COUNT_DOWN    OS_MS2TICK(10*1000)   //(ms)->tick
#define TIME_STAMP_EXPIRE   OS_MS2TICK(60*1000)   //(ms)->tick

struct task_info_st g_mlme_task_info[] =
{
    { "SSV_MLME",    (OsMsgQ)0, 8,   OS_MLME_TASK_PRIO,   64, NULL, mlme_task},
};

extern OsMutex  g_dev_info_mutex;
extern void os_timer_init(void);
extern void _cmd_wifi_scan (s32 argc, s8 *argv[]);

void update_ap_info_evt (u16 apIndex ,enum ap_act apAct);
void refresh_timer(void);
s32 mlme_host_event_handler_(struct cfg_host_event *pPktInfo);
void sent_iw_scan_req (void);
void sta_mode_ap_list_handler (void *data,struct ssv6xxx_ieee80211_bss *ap_list);
void sta_mode_handler(MsgEvent *pMsgEv); 
void sta_mode_scan_timer(void * data1,void * data2);
void ap_mode_handler(MsgEvent *pMsgEv);

void mlme_sta_mode_init(void)
{
	OS_MutexLock(g_dev_info_mutex); 

	/*init gDeviceInfo ap_list*/
	MEMSET((void *)&(gDeviceInfo->ap_list),0,sizeof(gDeviceInfo->ap_list));

	OS_MutexUnLock(g_dev_info_mutex); 
}


void mlme_sta_mode_deinit(void)
{
	OS_MutexLock(g_dev_info_mutex);

	/*stop scan timer*/
	os_cancel_timer(sta_mode_scan_timer, (u32)NULL, (u32)NULL);
	/*clean gDeviceInfo ap_list record*/
	MEMSET((void *)&(gDeviceInfo->ap_list),0,sizeof(gDeviceInfo->ap_list));

	OS_MutexUnLock(g_dev_info_mutex);

}


inline void update_ap_info_evt (u16 apIndex ,enum ap_act apAct)
{   
	u8 i;

	ap_info_state apInfoState ;
    apInfoState.apInfo=gDeviceInfo->ap_list;
    apInfoState.index=apIndex;
    apInfoState.act=apAct;
	
    for (i=0;i<HOST_EVT_CB_NUM;i++)
	{
		evt_handler handler = gDeviceInfo->evt_cb[i];
		if (handler) {
			handler(SOC_EVT_SCAN_RESULT, &apInfoState,(s32)sizeof(ap_info_state));
        }
	}	
}

inline void refresh_timer(void)
{
    os_cancel_timer(sta_mode_scan_timer, (u32)NULL, (u32)NULL);
	os_create_timer(TIMER_COUNT_DOWN, sta_mode_scan_timer, NULL, NULL, (void*)MBOX_MLME_TASK);
}

void sta_mode_scan_timer(void * data1,void * data2)
{
#if ( CONFIG_REGULAR_IW_SCAN == 1)
	// do regular IW Scan
	s8 *fullCh = "0xffff";
    _cmd_wifi_scan((s32)1, &fullCh);
#else
	// do regular refresh AP list table
	sta_mode_ap_list_handler(NULL,gDeviceInfo->ap_list);
#endif
}

void sta_mode_ap_list_handler (void *data,struct ssv6xxx_ieee80211_bss *ap_list)
{
    struct resp_evt_result *scan_res;
    struct ssv6xxx_ieee80211_bss *bss=NULL;
    u32    current_tick=OS_GetSysTick(); //tick
    u8     i;
	u8     oldest = 0;  
    u8     empty = 0xff;
    u8     duplicate = 0xff;

    if(data!=NULL)
    {
        scan_res = (struct resp_evt_result *)data;
        bss =&scan_res->u.scan.bss_info;
    }
    /*record status*/
    for(i=0;i<MAX_NUM_OF_AP;i++)
    {
        /*check empty by using channel id*/
        if(ap_list[i].channel_id==0)
        {
            empty=i;
        }
        /*check duplicate signal by using bssid*/
        else if((bss != NULL) && 
            MEMCMP((void*)(ap_list[i].bssid.addr), (void*)(bss->bssid.addr), ETHER_ADDR_LEN) == 0)
        {
            duplicate=i;
        }
        else
        {
            /*check whether time stamp is expire or not*/
            if(time_after((unsigned long)(current_tick),
                        (unsigned long)(ap_list[i].last_probe_resp+TIME_STAMP_EXPIRE)))
            {
                update_ap_info_evt(i,AP_ACT_REMOVE_AP);
                MEMSET((void *)&(ap_list[i]),0,sizeof(struct ssv6xxx_ieee80211_bss));
                empty=i;
            }
            else
            {   
                /*check whether "i" is the oldest one*/
                if(time_after((unsigned long)(ap_list[oldest].last_probe_resp),
                    (unsigned long)(ap_list[i].last_probe_resp)))
                {
                  oldest=i;
                }
            }
        }
    }
    /*run the record*/
    //   LOG_PRINTF("[Debug] oldest One : %d\n", oldest);
    //   LOG_PRINTF("[Debug] empty One : %d\n", empty);
    //   LOG_PRINTF("[Debug] duplicate One : %d\n", duplicate);
    if(bss != NULL)
    { 
        /*priority : duplicate> empty> oldest*/
        if(duplicate!=0xff)
        {
            i=duplicate;
		}
        else if(empty!=0xff)
        {   
            i=empty;
        }
        else{
            i = oldest;
            update_ap_info_evt(i,AP_ACT_REMOVE_AP);
        }
        
        ap_list[i]=*bss;
        ap_list[i].last_probe_resp=current_tick;
		if(duplicate==0xff)
		{
			update_ap_info_evt(i,AP_ACT_ADD_AP);
		}
		else
		{
			if(ap_list[i].channel_id != bss->channel_id)
			{
				update_ap_info_evt(i,AP_ACT_MODIFY_AP);
			}
		}

    }

    refresh_timer();

    return;
} // end of - _scan_result_handler -

s32 mlme_init(void)
{
    if (g_mlme_task_info[0].qlength> 0) {
        ASSERT(OS_MsgQCreate(&g_mlme_task_info[0].qevt,
        (u32)g_mlme_task_info[0].qlength)==OS_SUCCESS);
    }

    /* Create Registered Task: */
    OS_TaskCreate(g_mlme_task_info[0].task_func,
    g_mlme_task_info[0].task_name,
    g_mlme_task_info[0].stack_size<<4,
    g_mlme_task_info[0].args,
    g_mlme_task_info[0].prio,
    NULL);

	mlme_sta_mode_init();
	
	return OS_SUCCESS;
}

void mlme_task( void *args )
{
    s32 res;
    MsgEvent *pMsgEv = NULL;

    while(1)
    {
        res = msg_evt_fetch(MBOX_MLME_TASK, &pMsgEv);
        ASSERT(res == OS_SUCCESS);

        if(pMsgEv)
        {   
            if(gDeviceInfo->hw_mode == SSV6XXX_HWM_STA)    
            {   
                sta_mode_handler(pMsgEv);
            }
            else    
            {
                ap_mode_handler(pMsgEv);
            }
            os_msg_free(pMsgEv);
        }
    }
}

inline void sta_mode_handler(MsgEvent *pMsgEv)
{
    void* pMsgData = NULL;

    switch(pMsgEv->MsgType)
    {    
        case MEVT_PKT_BUF:  
            switch(pMsgEv->MsgData1)
            {
                case SOC_EVT_SCAN_RESULT:
					pMsgData = (void*)pMsgEv->MsgData;
                    sta_mode_ap_list_handler(pMsgData,gDeviceInfo->ap_list);
                    FREE(pMsgData);
                    break;
                default:
                    break;
            }
            break;
        case MEVT_HOST_TIMER:
            {
				timer_handler thdr = (timer_handler)pMsgEv->MsgData;
                thdr((void*)pMsgEv->MsgData1, (void*)pMsgEv->MsgData2);
            }
            break;
        default:
            break;
    }
}

inline void ap_mode_handler(MsgEvent *pMsgEv)
{
}

s32 mlme_host_event_handler_(struct cfg_host_event *pPktInfo)
{

    MsgEvent *pMsgEv= NULL;
    void     *pRecvData = NULL;
    u32 len = pPktInfo->len-sizeof(HDR_HostEvent);
 
    pRecvData =  MALLOC(len);
    MEMCPY(pRecvData,pPktInfo->dat,len);
        
    pMsgEv = msg_evt_alloc();
    pMsgEv->MsgType  = MEVT_PKT_BUF;    
    pMsgEv->MsgData  = (u32)pRecvData;
    pMsgEv->MsgData1 = SOC_EVT_SCAN_RESULT;
    pMsgEv->MsgData2 = 0;
    pMsgEv->MsgData3 = 0;
    msg_evt_post(MBOX_MLME_TASK, pMsgEv);

    return 0;
} // end of - _host_event_handler -






