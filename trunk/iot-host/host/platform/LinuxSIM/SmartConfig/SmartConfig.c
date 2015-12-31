#include <config.h>
#include <log.h>
#include <host_global.h>
#include "rtos.h"
#include "porting.h"
#include "SmartConfig.h"
#include "airkiss.h"
#include <ssv_lib.h>
#include "netmgr/net_mgr.h"

//#define MAX_CHANNEL_NUM 14

/*
The requirement of AirKiss solution
*/
airkiss_context_t g_smtcfg;
airkiss_result_t ares;
const airkiss_config_t g_airkscfg = {
    (airkiss_memset_fn)&ssv6xxx_memset,
    (airkiss_memcpy_fn)&ssv6xxx_memcpy,
    (airkiss_memcmp_fn)&ssv6xxx_memcmp,
    (airkiss_printf_fn)NULL,
};

/*
These status are general for every SmartConfig solution (ex: Airkiss is a kind of SmartConfig solution)
*/
typedef enum{
    EN_SCONFIG_NOT_READY=0, //SmartConfig solution is not ready.
    EN_SCONFIG_GOING, //SmartConfig solution is running, we can feed the data to it)
    EN_SCONFIG_LOCK, //SmartConfig solution lock the channel of target AP.
    EN_SCONFIG_DONE, //SmartConfig solution get the AP's information
}EN_SCONFIG_STATUS;

/*
Set default status
*/
EN_SCONFIG_STATUS enSCstat=EN_SCONFIG_NOT_READY;

/*
Local variables
*/
OsMutex SCMutex=0;
u32 StartTick=0;
u32 LastTick=0;
u32 CurTick=0;
u8 last_channel=0;
u8 lock_channel=0;
u8 current_channel=0;
u16 UserSconfigChannelMask=0;
u16 ChannelMask=0;
u16 get_next_channel(void)
{
    u16 i=1;
REPEAT:
    for(i=1;i<MAX_CHANNEL_NUM;i++){
        if((ChannelMask&(1<<i))==0){
            continue;
        }
        ChannelMask=ChannelMask&(~(1<<i));
        break;
    }

    if(i==MAX_CHANNEL_NUM){
        ChannelMask=UserSconfigChannelMask;
        goto REPEAT;
    }
    OS_MutexLock(SCMutex);
    current_channel=i;
    OS_MutexUnLock(SCMutex);
    LOG_PRINTF("Change channel to %d\r\n",i);
    return i;
}

/*
Init your SmartConfig solution, and the state machine
*/
int UserSconfigInit(void)
{
    s32 ret=0;

    /*===========================
    [Start] Init your SmartConfig solution.
    ============================*/
    LOG_PRINTF("\33[32mAirKiss version:%s\33[0m\n",airkiss_version());
    ret=airkiss_init(&g_smtcfg,&g_airkscfg);
    if(ret<0){
        LOG_PRINTF("AirKiss init fail\r\n");
        return -1;
    }
    /*===========================
    Init your SmartConfig solution. [End]
    ============================*/

    /*
        Init the variables for state machine of SmartConfig
    */
    OS_MutexInit(&SCMutex);
    UserSconfigChannelMask&=~(0xF001); //unmask ch0 , ch12, ch13, ch14, ch15;

    StartTick=OS_GetSysTick();
    LastTick=StartTick;
    CurTick=StartTick;
    ChannelMask=UserSconfigChannelMask;

    /*
        Choose the first channel, and set channel
    */
    lock_channel=0xFF;
    last_channel=current_channel=get_next_channel();
    ssv6xxx_wifi_set_channel(current_channel);

    /*
        Set status of state machine to "EN_SCONFIG_GOING"
    */
    enSCstat=EN_SCONFIG_GOING;

    return 0;

}

//**************************************************************************************************************************************************************
//**************************************************************************************************************************************************************
//You don't need to change any fucntion names here. In ssv6xxx_dev_init, it automatically assign thses function to the call back function table
//If you really want to change the function name, you must modify the ssv6xxx_dev_init at the same time
//**************************************************************************************************************************************************************
//**************************************************************************************************************************************************************

/*
Deinit your SmartConfig solution, set the status of state machine, and release the resources
*/
void UserSconfigDeinit(void)
{
    /*===========================
    [Start] Deinit your SmartConfig solution.
    ============================*/

    /*===========================
    DeInit your SmartConfig solution. [End]
    ============================*/

    /*
        Relase the local resources.
    */
    if(SCMutex!=0){
        OS_MutexLock(SCMutex);
        enSCstat=EN_SCONFIG_NOT_READY;
        OS_MutexUnLock(SCMutex);

        OS_MutexDelete(SCMutex);
    }
    SCMutex=0;
    return;
}

/*
The state machine of SmartConfig.
*/
int UserSconfigSM(void)
{

    /*
        Check the status
    */
    if(enSCstat==EN_SCONFIG_NOT_READY){
        LOG_PRINTF("SmartConfig status is not ready\r\n");
        return 0;
    }

    /*
        Run the deinit function, and switch to the STA mode when the status is "EN_SCONFIG_DONE"
    */
    if(enSCstat==EN_SCONFIG_DONE){

        wifi_sta_join_cfg *join_cfg = NULL;
        LOG_PRINTF("SmartConfig status is done\r\n");

        join_cfg = MALLOC(sizeof(wifi_sta_join_cfg));
        if(join_cfg==NULL){
            LOG_PRINTF("malloc for join_cfg fail\r\n");
            return -1;
        }

        /*
            Switch to STA mode, and join to this BSSID automatically
        */
        STRCPY((const char *)join_cfg->ssid.ssid, ares.ssid);
        STRCPY((const char *)join_cfg->password, ares.pwd);
        netmgr_wifi_switch_to_sta(join_cfg,lock_channel);

        UserSconfigDeinit();
        return 0;
    }

    CurTick=OS_GetSysTick();

    /*
        If we can't run into "EN_SCONFIG_DONE" in 60s, we restart the SmartConfig
    */
    if(((CurTick-StartTick)>=6000)&&
        ((enSCstat==EN_SCONFIG_GOING)||(enSCstat==EN_SCONFIG_LOCK))){ //over 60s
        //Re-start
        LOG_PRINTF("SmartConfig restart,%d %d\r\n",CurTick,StartTick);
        UserSconfigDeinit();
        UserSconfigInit();
        return 1;
    }

    /*
        Before running into "EN_SCONFIG_LOCK ", we change the channel every 100 ms
    */
    if(((CurTick - LastTick)>=10)&&
        (enSCstat==EN_SCONFIG_GOING)){ // over 100ms
        //airkiss_change_channel(&g_smtcfg);
        ssv6xxx_wifi_set_channel(get_next_channel());
        LastTick=CurTick;
        return 1;
    }

    return 1;
}

/*
Feed the rx data to your SmartConfig solution.
*/
void UserSconfigPaserData(u8 channel, u8 *rx_buf, u32 len)
{
    s32 ret=0;

    /*
        Check the status
    */
    if((enSCstat==EN_SCONFIG_NOT_READY)||(enSCstat==EN_SCONFIG_DONE)){
        return;
    }

    /*
        Drop the rx data packet if this packet is not from lock_channel
    */
    if(enSCstat==EN_SCONFIG_LOCK){
        if(lock_channel!=channel){
            LOG_PRINTF("This packet is from channel %d, not locking channel %d, drop it \r\n",channel,lock_channel);
            return;
        }
    }

    /*
        Flush channel if your SmartConfig solution offers this function.
    */
    if(last_channel!=channel){
        LOG_PRINTF("AirKiss flush: last packet from channel %d, current packet from channel %d \r\n",last_channel,channel);
        airkiss_change_channel(&g_smtcfg);
        last_channel=channel;
    }


    /*=======================================================
    [Start] Feed the rx data to your SmartConfig solution, and check it's status
    ========================================================*/
    ret=airkiss_recv(&g_smtcfg,rx_buf,len);
    /*=======================================================
    Feed the rx data to your SmartConfig solution, and check it's status [End]
    ========================================================*/

    if(AIRKISS_STATUS_CHANNEL_LOCKED==ret){
        OS_MutexLock(SCMutex);
        enSCstat=EN_SCONFIG_LOCK;
        OS_MutexUnLock(SCMutex);
        LOG_PRINTF("\33[32mThe Airkiss lock on channel %d\33[0m\r\n",channel);

        lock_channel=channel;
        /*
            Sometimes we can't get the complete status after airkiss return the lock.
            The state machine will restart the scanning, I think we don't need to scan whole channel  in this case.
            we just need to scan the previos and after channel of locking channel
        */
        UserSconfigChannelMask=0;

        if(lock_channel>1)
            UserSconfigChannelMask|=(1<<(lock_channel-1));

        if(lock_channel>2)
            UserSconfigChannelMask|=(1<<(lock_channel-2));

        UserSconfigChannelMask|=(1<<lock_channel);

        if(lock_channel<10)
            UserSconfigChannelMask|=(1<<(lock_channel+2));

        if(lock_channel<11)
            UserSconfigChannelMask|=(1<<(lock_channel+1));

        /*
            Change to lock channel if the lock channel and current channel are not the same
        */
        if(lock_channel!=current_channel){
            OS_MutexLock(SCMutex);
            current_channel=lock_channel;
            OS_MutexUnLock(SCMutex);
            LOG_PRINTF("Change channel to %d\r\n",lock_channel);
            ssv6xxx_wifi_set_channel(lock_channel);
        }

    }else if(AIRKISS_STATUS_COMPLETE==ret){
        OS_MutexLock(SCMutex);
        enSCstat=EN_SCONFIG_DONE;
        OS_MutexUnLock(SCMutex);

        LOG_PRINTF("\33[32mThe Airkiss is completed\33[0m\n");
        /*=============================================
        [Start] Get the AP information from your SmartConfig solution
        ==============================================*/
        airkiss_get_result(&g_smtcfg,&ares);
        /*=============================================
        Get the AP information from your SmartConfig solution [End]
        ==============================================*/

        LOG_PRINTF("ssid=%s\r\n",ares.ssid);
        LOG_PRINTF("ssid_length=%u\r\n",ares.ssid_length);
        LOG_PRINTF("pwd=%s\r\n",ares.pwd);
        LOG_PRINTF("pwd_length=%u\r\n",ares.pwd_length);
        LOG_PRINTF("random=%u\r\n",ares.random);

    }


    return;
}
extern int netmgr_wifi_sconfig_done(u8 *resp_data, u32 len, bool IsUDP,u32 port);

/*
    Response the finish status to smart phone
*/
void UserSconfigConnect(void)
{
    /*==================================================================
    [Start] You can do something after get the IP address (ex: broadcast data to smartphone
    ====================================================================*/
    LOG_PRINTF("The airkiss send the broadcast packet\r\n");
    netmgr_wifi_sconfig_done(&ares.random,1,TRUE,10000);
    /*==================================================================
    You can do something after get the IP address (ex: broadcast data to smartphone [End]
    ====================================================================*/


    return;
}
