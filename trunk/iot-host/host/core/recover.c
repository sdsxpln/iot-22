#include <txrx_hdl.h>
#include "recover.h"
#include "txrx_task.h"

void ssv6xxx_wifi_ap_recover()
{
    u32 idx,CPUSR;    
    s32 reload_fw_cnt;
    
    reload_fw_cnt = gDeviceInfo->reload_fw_cnt;
    reload_fw_cnt++;
    gDeviceInfo->reload_fw_cnt = MAX_RECOVER_COUNT;
    CPUSR = OS_EnterCritical();
    //LOG_PRINTF("ssv6xxx_wifi_ap_recover:%d \r\n",gDeviceInfo->reload_fw_cnt);
    /*************************************/
    //turn off tx queue 
    /*************************************/
    TXRXTask_TxLock(TRUE);
    //spi init
    ssv6xxx_drv_init();
            
    /*************************************/
    //stop ap mode 
    /*************************************/

    //clean beacon status
	release_beacon_info();
    ssv6xxx_beacon_enable(false); 

    //HCI stop
    ssv6xxx_drv_stop();

    //Disable HW
    ssv6xxx_HW_disable();

    /*************************************/
    //reload and setting ap mode 
    /*************************************/
    ssv6xxx_chip_init();
    //Load FW, init mac
    ssv6xxx_start();

    //GenBeacon
	StartBeacon();
	ap_soc_set_bcn(SSV6XXX_SET_INIT_BEACON, gDeviceInfo->APInfo->bcn, &gDeviceInfo->APInfo->bcn_info, AP_DEFAULT_DTIM_PERIOD-1, AP_DEFAULT_BEACON_INT);
    
    //Enable HW
    ssv6xxx_HW_enable();

    /*************************************/
    //add station 
    /*************************************/
    for(idx=0; idx<gMaxAID; idx++)
    {

		APStaInfo_st *sta= &gDeviceInfo->StaConInfo[idx];
        if (!test_sta_flag(sta, WLAN_STA_VALID))
            continue;
        else
            ap_soc_cmd_sta_oper(CFG_STA_ADD, sta, CFG_HT_NONE, test_sta_flag(sta, WLAN_STA_WMM));
    }
   
    

    /*************************************/
    //turn on tx queue 
    /*************************************/
    TXRXTask_TxLock(FALSE);
    
    gDeviceInfo->reload_fw_cnt = reload_fw_cnt ;
    OS_ExitCritical(CPUSR);

}


