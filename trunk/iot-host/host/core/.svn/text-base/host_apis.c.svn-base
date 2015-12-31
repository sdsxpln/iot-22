#include <config.h>
#include <pbuf.h>
#include <log.h>
#include <hdr80211.h>
#include <common.h>
#include <cmd_def.h>
#include <host_apis.h>
#include <drv/ssv_drv.h>
#include <core/mlme.h>
#include "os_wrapper.h"
#include "host_cmd_engine_sm.h"
#include "ssv_timer.h"
#include "dev.h"
#include <txrx_hdl.h>
#include <cmd_def.h>
#include "ssv_dev.h"
#include <regs/ssv6200_reg.h>
#if (CONFIG_SIM_PLATFORM == 1)
#include  <wsimp/wsimp_lib.h>
#endif
#include "ap_tx.h"
#include <cli/cmds/cli_cmd_wifi.h>

//Mac address
extern u8 config_mac[];

typedef enum{
  HOST_API_DEACTIVE,
  HOST_API_ACTIVE,
  HOST_API_PWR_SAVING,
}HOST_API_STATE;

HOST_API_STATE active_host_api = HOST_API_ACTIVE;
static u32 sg_host_cmd_seq_no;


OsMutex	        g_host_api_mutex;

static s32 _ssv6xxx_wifi_auto_channel_selection(u16 channel_mask);
void* os_hcmd_msg_evt_alloc();
void os_hcmd_msg_evt_free();
s32 os_hcmd_msg_send(void* msg, void *pBuffer);
void ap_sta_mode_off(void);
void sta_mode_off(void);
void ap_mode_off(void);




#if 0
#define CFG_HOST_TXREQ0(a, b)                       \
    (a) = (struct cfg_host_txreq0 *)                \
    ((u8 *)(b) - sizeof(struct cfg_host_txreq0))
#define CFG_HOST_TXREQ1(a, b)                       \
    (a) = (struct cfg_host_txreq1 *)                \
    ((u8 *)(b) - sizeof(struct cfg_host_txreq1))
#define CFG_HOST_TXREQ2(a, b)                       \
    (a) = (struct cfg_host_txreq2 *)                \
    ((u8 *)(b) - sizeof(struct cfg_host_txreq2))
#endif

struct qos_ctrl_st {
    u16                 tid:4;
    u16                 bit4:1;
    u16                 ack_policy:2;
    u16                 rsvd:1;
    u16                 bit8_15:8;
};

struct ht_ctrl_st {
	u32 				ht;
};


struct a4_ctrl_st {
	ETHER_ADDR			a4;
};

// Keep security request and status
//static bool	_is_security_enabled = false;
//static bool _join_security = false;


// ssv6xxx_hw_mode _ssv_wifi_get_hw_mode(void)
// {
// 	ssv6xxx_hw_mode hw_mode;
//
// 	os_cmd_eng_mutex_lock();
// 	hw_mode = gHCmdEngInfo->hw_mode;
// 	os_cmd_eng_mutex_unlock();
//
// 	return hw_mode;
// }


// u32 _ssv_wifi_get_cmd_eng_state(void)
// {
// 	u32 state;
//
// 	os_cmd_eng_mutex_lock();
// //	state =		SM_GET_STATE(HCMDE);	//		gHCmdEngInfo->HCMDE_state;
// 	os_cmd_eng_mutex_unlock();
//
// 	return state;
// }
s32 _ssv6xxx_wifi_ioctl_Ext(u32 cmd_id, void *data, u32 len, bool blocking,const bool mutexLock);
static bool _ssv6xxx_wakeup_wifi(void);
s32 _ssv6xxx_send_msg_to_hcmd_eng(void *data)
{
	s32 ret = SSV6XXX_SUCCESS;
	void *msg = os_msg_alloc();
	if (msg)
		os_msg_send(msg, data);
	else
		ret = SSV6XXX_NO_MEM;
	return ret;
}


static s32 _ssv6xxx_wifi_send_cmd(void *pCusData, int nCuslen, ssv6xxx_host_cmd_id eCmdID)
{
  	struct cfg_host_cmd *host_cmd;
	u32 nSize = HOST_CMD_HDR_LEN+nCuslen;
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	u8* frame;

	do
	{

		frame = os_frame_alloc((u32)nCuslen);

        if (frame == NULL) {
            ret = SSV6XXX_NO_MEM;
            break;
        }
		host_cmd = os_frame_push(frame, HOST_CMD_HDR_LEN);
		host_cmd->c_type = HOST_CMD;
        host_cmd->RSVD0 = 0;//(eCmdID>>8)&0x1F;
		host_cmd->h_cmd = (u8)eCmdID;
		host_cmd->len = nSize;
		sg_host_cmd_seq_no++;
		host_cmd->cmd_seq_no=sg_host_cmd_seq_no;
		MEMCPY(host_cmd->un.dat8, pCusData, (u32)nCuslen);

		ret = _ssv6xxx_send_msg_to_hcmd_eng((void *)frame);
	} while (0);

	return ret;
}

s32 _ssv6xxx_wifi_ioctl(u32 cmd_id, void *data, u32 len, const bool mutexLock)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;

    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {

        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }

        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

        ret = _ssv6xxx_wifi_send_cmd(data, len, cmd_id);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

	return ret;
}


H_APIs s32 ssv6xxx_wifi_ioctl(u32 cmd_id, void *data, u32 len)
{
	return _ssv6xxx_wifi_ioctl(cmd_id, data, len, TRUE);
}

/**
 * H_APIs s32 ssv6xxx_wifi_sconfig() - Smart config Request from host to wifi controller
 *                                               to collect nearby APs.
 *
 * This API triggers wifi controller to broadcast probe request frames and
 * wait for the probe response frames from APs - active scan.
 * This API also can issue passive scan request and monitor bradocast
 * beacon from nearby APs siliently.
 */

s32 _ssv6xxx_wifi_sconfig(struct cfg_sconfig_request *csreq,const bool mutexLock)
{
//	struct cfg_host_cmd *host_cmd = NULL;
	s32 size,ret=SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        if (csreq == NULL)
        {
            ret = SSV6XXX_INVA_PARAM;
            break;
        }

        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

    	size = sizeof(struct cfg_sconfig_request);
        ret = _ssv6xxx_wifi_send_cmd(csreq, size, SSV6XXX_HOST_CMD_SMART_CONFIG);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;


}
H_APIs s32 ssv6xxx_wifi_sconfig(struct cfg_sconfig_request *csreq)
{
    return _ssv6xxx_wifi_sconfig(csreq, TRUE);
}

static bool _ssv6xxx_wakeup_wifi(void)
{
    bool ret;

    ret=ssv6xxx_drv_wakeup_wifi(TRUE);
    if(TRUE == ret)
    {
        while(1)
        {
            OS_MsDelay(20);
            if(active_host_api != HOST_API_ACTIVE)
            {
                ssv6xxx_drv_wakeup_wifi(TRUE);
                //LOG_PRINTF("+ ");
            }
            else
                break;
        }
        if(FALSE == ssv6xxx_drv_wakeup_wifi(FALSE))
            LOG_PRINTF("clear PMU wakeup signal fail!!\r\n");
    }
    else
    {
        LOG_PRINTF("Wakeup wifi fail!!\r\n");
    }
    return ret;
}

H_APIs bool ssv6xxx_wifi_wakeup(void)
{
    bool ret;

    ret=_ssv6xxx_wakeup_wifi();
    return ret;
}


/**
 * H_APIs s32 ssv6xxx_wifi_pwr_saving() - Info FW to start power saving procedure
 *                                               .
 */

s32 _ssv6xxx_wifi_pwr_saving(struct cfg_ps_wow_request* wowreq,const bool mutexLock)
{
    s32 size,ret=SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do
    {
        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        size = sizeof(struct cfg_ps_wow_request);
        ret = _ssv6xxx_wifi_send_cmd(wowreq, size, SSV6XXX_HOST_CMD_SET_PWR_SAVING);
        if (ret == SSV6XXX_SUCCESS)
            active_host_api = HOST_API_PWR_SAVING;
        else
            LOG_PRINTF("Send CMD_SET_PWR_SAVING fail!!\r\n");
    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;


}
H_APIs s32 ssv6xxx_wifi_pwr_saving(struct cfg_ps_wow_request* wowreq, bool on)
{
    u32 ret=SSV6XXX_SUCCESS;
    OS_MutexLock(g_host_api_mutex);
    if(on == TRUE)
    {
        if(active_host_api == HOST_API_PWR_SAVING)
            return SSV6XXX_SUCCESS;
            
        ret = _ssv6xxx_wifi_pwr_saving(wowreq, FALSE);
    }
    else
    {
        active_host_api = HOST_API_ACTIVE;
    }
    OS_MutexUnLock(g_host_api_mutex);

    return ret;
}
/**
 * H_APIs s32 ssv6xxx_wifi_scan() - Scan Request from host to wifi controller
 *                                               to collect nearby APs.
 *
 * This API triggers wifi controller to broadcast probe request frames and
 * wait for the probe response frames from APs - active scan.
 * This API also can issue passive scan request and monitor bradocast
 * beacon from nearby APs siliently.
 */

s32 _ssv6xxx_wifi_scan(struct cfg_scan_request *csreq,const bool mutexLock)
{
//	struct cfg_host_cmd *host_cmd = NULL;
	s32 size,ret=SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        if (csreq == NULL)
        {
            ret = SSV6XXX_INVA_PARAM;
            break;
        }

        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

    	size = sizeof(struct cfg_scan_request) +
    		sizeof(struct cfg_80211_ssid) * csreq->n_ssids;
        ret = _ssv6xxx_wifi_send_cmd(csreq, size, SSV6XXX_HOST_CMD_SCAN);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;


}
H_APIs s32 ssv6xxx_wifi_scan(struct cfg_scan_request *csreq)
{
    if(gDeviceInfo->hw_mode != SSV6XXX_HWM_STA)
    {
        LOG_PRINTF("Invalid arguments.\n");
        return SSV6XXX_FAILED;
    }

    return _ssv6xxx_wifi_scan(csreq, TRUE);
}

/**
 * H_APIs s32 ssv6xxx_wifi_join() - Request a AP to create a link between
 *                                             requested AP and a STA.
 *
 * This API triggers authentication and association reqests to an
 * explicitly specify AP and wait for authentication and association
 * response from the AP.
 */
s32 _ssv6xxx_wifi_join(struct cfg_join_request *cjreq, const bool mutexLock)
{
//    struct cfg_host_cmd *host_cmd = NULL;
    s32 size,ret=SSV6XXX_SUCCESS;

    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE ||gDeviceInfo->status == CONNECT)
        //if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        if (cjreq == NULL)
        {
            ret = SSV6XXX_INVA_PARAM;
            break;
        }
        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }
        //MEMCPY(gDeviceInfo->joincfg , cjreq, sizeof(struct cfg_join_request));

        size =   HOST_CMD_HDR_LEN
            + sizeof(struct cfg_join_request)
            + sizeof(struct cfg_80211_ssid);
        ret = _ssv6xxx_wifi_send_cmd(cjreq, size, SSV6XXX_HOST_CMD_JOIN);

        if (ret == SSV6XXX_SUCCESS)
            MEMCPY(gDeviceInfo->joincfg , cjreq, sizeof(struct cfg_join_request));
            //_join_security = (cjreq->sec_type != SSV6XXX_SEC_NONE);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;


}
H_APIs s32 ssv6xxx_wifi_join(struct cfg_join_request *cjreq)
{

    if(gDeviceInfo->hw_mode != SSV6XXX_HWM_STA)
    {
        LOG_PRINTF("Invalid arguments.\n");
        return SSV6XXX_FAILED;
    }
    return _ssv6xxx_wifi_join(cjreq, TRUE);
}


/**
 * H_APIs s32 ssv6xxx_wifi_leave() - Request wifi controller to leave an
 *                                                explicitly speicify BSS.
 *
 */
s32 _ssv6xxx_wifi_leave(struct cfg_leave_request *clreq,const bool mutexLock)
{

//    struct cfg_host_cmd *host_cmd = NULL;
    s32 size,ret=SSV6XXX_SUCCESS;

    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }
        //clean sta mode global variable
        gDeviceInfo->status= DISCONNECT;
        MEMSET(gDeviceInfo->joincfg, 0, sizeof(struct cfg_join_request));

        size = HOST_CMD_HDR_LEN	+ sizeof(struct cfg_leave_request);
        ret = _ssv6xxx_wifi_send_cmd(clreq, size, SSV6XXX_HOST_CMD_LEAVE);


    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}
H_APIs s32 ssv6xxx_wifi_leave(struct cfg_leave_request *clreq)
{
    if(gDeviceInfo->hw_mode != SSV6XXX_HWM_STA)
    {
        LOG_PRINTF("Invalid arguments.\n");
        return SSV6XXX_FAILED;
    }

    return _ssv6xxx_wifi_leave(clreq, TRUE);
}

#if 0
s32 _ssv6xxx_wifi_send_addba_resp(struct cfg_addba_resp *addba_resp,const bool mutexLock)
{
    s32 size,ret=SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {

        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }

        size = HOST_CMD_HDR_LEN	+ sizeof(struct cfg_addba_resp);
        ret = _ssv6xxx_wifi_send_cmd(addba_resp, size, SSV6XXX_HOST_CMD_ADDBA_RESP);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}
H_APIs s32 ssv6xxx_wifi_send_addba_resp(struct cfg_addba_resp *addba_resp)
{
    return _ssv6xxx_wifi_send_addba_resp(addba_resp,TRUE);
}



s32 _ssv6xxx_wifi_send_delba(struct cfg_delba *delba,const bool mutexLock)
{

   s32 size,ret=SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }

        size = HOST_CMD_HDR_LEN + sizeof(struct cfg_delba);
        ret = _ssv6xxx_wifi_send_cmd(delba, size, SSV6XXX_HOST_CMD_DELBA);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}

H_APIs s32 ssv6xxx_wifi_send_delba(struct cfg_delba *delba)
{
    return _ssv6xxx_wifi_send_delba(delba, TRUE);
}
#endif

#if (AP_MODE_ENABLE == 1)
extern tx_result ssv6xxx_data_could_be_send(struct cfg_host_txreq0 *host_txreq0, bool bAPFrame, u32 TxFlags);
#endif

extern void CmdEng_TxHdlData(void *frame);
inline s32 _ssv6xxx_wifi_send_ethernet(void *frame, s32 len, enum ssv6xxx_data_priority priority, u8 tx_dscrp_flag,const bool mutexLock)
{

	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	//Detect host API status
	if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
    		os_frame_free(frame);
	        ret=SSV6XXX_FAILED;
            break;
        }

        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

        if(TxHdl_prepare_wifi_txreq(frame, (u32)len, FALSE, priority, tx_dscrp_flag)==FALSE)
    	{
    		LOG_TRACE("tx req fail. release frame.\n");
    		os_frame_free(frame);
    		ret = SSV6XXX_FAILED;
            break;
    	}
        ret = SSV6XXX_SUCCESS;

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);



	return ret;
}
H_APIs s32 ssv6xxx_wifi_send_ethernet(void *frame, s32 len, enum ssv6xxx_data_priority priority)
{
	return _ssv6xxx_wifi_send_ethernet(frame, len, priority, FALSE, TRUE);
}

#if 0
inline s32 _ssv6xxx_wifi_send_80211(void *frame, s32 len, u8 tx_dscrp_flag, const bool mutexLock  )
{

	ssv6xxx_result ret = SSV6XXX_SUCCESS;

    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }

    	if(ssv6xxx_prepare_wifi_txreq(frame, len, TRUE, 0, tx_dscrp_flag)==FALSE)
    	{
    		LOG_TRACE("tx req fail. release frame.\n");
    		os_frame_free(frame);
    		return SSV6XXX_FAILED;
    	}
    	ret = _ssv6xxx_send_msg_to_hcmd_eng(frame);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

	return ret;

}
H_APIs s32 ssv6xxx_wifi_send_80211(void *frame, s32 len)
{
	return _ssv6xxx_wifi_send_80211(frame, len, FALSE, TRUE);
}
#endif


//----------------------------------------------

u8* ssv6xxx_host_tx_req_get_qos_ptr(struct cfg_host_txreq0 *req0)
{
	u16 nOffset = 0;
	switch (req0->c_type)
	{
		case SSV6XXX_TX_REQ0:
			nOffset += sizeof(struct cfg_host_txreq0);
			break;
// 		case SSV6XXX_TX_REQ1:
// 			nOffset += sizeof(struct cfg_host_txreq1);
// 			break;
// 		case SSV6XXX_TX_REQ2:
// 			nOffset += sizeof(struct cfg_host_txreq2);
// 			break;
		default:
			ASSERT(FALSE);
			break;
	}


	if (req0->use_4addr)
		nOffset+=ETHER_ADDR_LEN;

	if (!req0->qos)
		return NULL;

	if (req0->ht)
		nOffset+=IEEE80211_HT_CTL_LEN;

	return (u8*)(((u8*)req0)+nOffset);
}

u8* ssv6xxx_host_tx_req_get_data_ptr(struct cfg_host_txreq0 *req0)
{
	u16 nOffset = 0;
	switch (req0->c_type)
	{
		case SSV6XXX_TX_REQ0:
			nOffset += sizeof(struct cfg_host_txreq0);
			break;
// 		case SSV6XXX_TX_REQ1:
// 			nOffset += sizeof(struct cfg_host_txreq1);
// 			break;
// 		case SSV6XXX_TX_REQ2:
// 			nOffset += sizeof(struct cfg_host_txreq2);
// 			break;
		default:
			ASSERT(FALSE);
			break;
	}


	if (req0->use_4addr)
		nOffset+=ETHER_ADDR_LEN;

	if (req0->qos)
		nOffset+=IEEE80211_QOS_CTL_LEN;

	if (req0->ht)
		nOffset+=IEEE80211_HT_CTL_LEN;

    nOffset+=req0->padding;
	return (u8*)(((u8*)req0)+nOffset);
}
//----------------------------------------------------------------------------------------------------------

//callback function handle
s32  ssv6xxx_wifi_cb(s32 arraylen, u32 **array, u32 *handler, ssv6xxx_cb_action action)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
	int i, nEmptyIdx=-1;
	bool bFound = FALSE;

	do
	{
		for (i=0;i<arraylen;i++)
		{
			if (NULL == array[i])
			{
				nEmptyIdx = i;
			}
			else if(handler == array[i])
			{
				nEmptyIdx = i;
				bFound = TRUE;
				break;
			}
			else
			{}
		}

		if(action == SSV6XXX_CB_ADD)
		{
			//add
			if (-1 == nEmptyIdx)
			{
				LOG_ERROR("queue is not enough\n");
				ret = SSV6XXX_QUEUE_FULL;
#ifdef __AP_DEBUG__
				ASSERT(FALSE);
#endif//__AP_DEBUG__
				break;
			}
			array[nEmptyIdx] = handler;
		}
		else
		{
			//remove
			if(bFound)
				array[nEmptyIdx] = NULL;
		}

	} while (0);


	return ret;


}
//----------------------------------------------------------------------------------------------------------

//Register handler to get RX data
ssv6xxx_result _ssv6xxx_wifi_reg_rx_cb(data_handler handler,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }

        ret = ssv6xxx_wifi_cb(HOST_DATA_CB_NUM, (u32 **)&gDeviceInfo->data_cb , (u32 *)handler, SSV6XXX_CB_ADD);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

	return ret;
}


//Register handler to get RX data
H_APIs ssv6xxx_result ssv6xxx_wifi_reg_rx_cb(data_handler handler)
{
	return _ssv6xxx_wifi_reg_rx_cb(handler, TRUE);
}

ssv6xxx_result _ssv6xxx_wifi_reg_evt_cb(evt_handler evtcb, const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }
        ret = ssv6xxx_wifi_cb(HOST_EVT_CB_NUM, (u32 **)&gDeviceInfo->evt_cb , (u32 *)evtcb, SSV6XXX_CB_ADD);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

	return ret;
}

H_APIs ssv6xxx_result ssv6xxx_wifi_reg_evt_cb(evt_handler evtcb)
{
	return _ssv6xxx_wifi_reg_evt_cb(evtcb,TRUE);
}


ssv6xxx_result _ssv6xxx_wifi_unreg_rx_cb(data_handler handler,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE )
        {
	        ret=SSV6XXX_FAILED;
            break;
        }

        ret = ssv6xxx_wifi_cb(HOST_DATA_CB_NUM, (u32 **)&gDeviceInfo->data_cb , (u32 *)handler, SSV6XXX_CB_REMOVE);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);


	return ret;
}
//Register handler to get RX data
H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_rx_cb(data_handler handler)
{
	return _ssv6xxx_wifi_unreg_rx_cb(handler,TRUE);
}


ssv6xxx_result _ssv6xxx_wifi_unreg_evt_cb(evt_handler evtcb,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }

        ret = ssv6xxx_wifi_cb(HOST_EVT_CB_NUM, (u32 **)&gDeviceInfo->evt_cb , (u32 *)evtcb, SSV6XXX_CB_REMOVE);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);


	return ret;
}

//Register handler to get event
H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_evt_cb(evt_handler evtcb)
{
	return _ssv6xxx_wifi_unreg_evt_cb(evtcb,TRUE);
}


extern int ssv6xxx_get_cust_mac(u8 *mac);

ssv6xxx_result ssv6xxx_wifi_init(void)
{
    extern ssv6xxx_result CmdEng_Init(void);
    ssv6xxx_result res;
    //init mutex
    OS_MutexInit(&g_host_api_mutex);



    res = CmdEng_Init();
    gDeviceInfo = (DeviceInfo_st *)MALLOC(sizeof(DeviceInfo_st));
    MEMSET(gDeviceInfo, 0 , sizeof(DeviceInfo_st));
    gDeviceInfo->hw_mode = SSV6XXX_HWM_STA;

    gDeviceInfo->joincfg= (struct cfg_join_request *)MALLOC(sizeof(struct cfg_join_request));
    MEMSET(gDeviceInfo->joincfg, 0 , sizeof(struct cfg_join_request));

    //Set default MAC addr
    ssv6xxx_memcpy(gDeviceInfo->self_mac,config_mac,6);

	ssv6xxx_get_cust_mac(gDeviceInfo->self_mac);

	//init mutex
    OS_MutexInit(&gDeviceInfo->g_dev_info_mutex);

	sg_host_cmd_seq_no=0;

    return res;
}


ssv6xxx_result ssv6xxx_wifi_deinit(void)
{
    if(active_host_api == HOST_API_DEACTIVE)
    {
        ap_sta_mode_off();
    }
    return SSV6XXX_SUCCESS;
}
#if 0
void _ssv6xxx_wifi_clear_security (const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE )
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        _is_security_enabled = false;

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

}

H_APIs void ssv6xxx_wifi_clear_security (void)
{
    _ssv6xxx_wifi_clear_security(TRUE);

}

void _ssv6xxx_wifi_apply_security (const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
            ret=SSV6XXX_FAILED;
            break;
        }
        //_is_security_enabled = _join_security;
        _is_security_enabled = (gDeviceInfo->joincfg.sec_type != SSV6XXX_SEC_NONE);

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);



}


H_APIs void ssv6xxx_wifi_apply_security (void)
{
    _ssv6xxx_wifi_apply_security(TRUE);
}
#endif


static s32 _ssv6xxx_wifi_send_cmd_directly(void *pCusData, int nCuslen, ssv6xxx_host_cmd_id eCmdID)
{
    struct cfg_host_cmd *host_cmd;
    u32 nSize = HOST_CMD_HDR_LEN+nCuslen;
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    u8* frame;

    frame = os_frame_alloc((u32)nCuslen);

    if (frame == NULL) {
        return SSV6XXX_NO_MEM;
    }
    host_cmd = os_frame_push(frame, HOST_CMD_HDR_LEN);
    host_cmd->c_type = HOST_CMD;
    host_cmd->RSVD0 = 0;//(eCmdID>>8)&0x1F;
    host_cmd->h_cmd = (u8)eCmdID;
    host_cmd->len = nSize;
    sg_host_cmd_seq_no++;
    host_cmd->cmd_seq_no=sg_host_cmd_seq_no;
    memcpy(host_cmd->un.dat8, (u8*)pCusData, (u32)nCuslen);

    if(ssv6xxx_drv_send(OS_FRAME_GET_DATA(frame), OS_FRAME_GET_DATA_LEN(frame)) <0){
        ret=SSV6XXX_FAILED;
    }

    os_frame_free(frame);
    return ret;
}


s32 _ssv6xxx_wifi_ioctl_Ext(u32 cmd_id, void *data, u32 len, bool blocking,const bool mutexLock)
{
	ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }
        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

        if(TRUE == blocking)
            ret = _ssv6xxx_wifi_send_cmd_directly(data, len, cmd_id);
        else
            ret = _ssv6xxx_wifi_send_cmd(data, len, cmd_id);


    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}


H_APIs s32 ssv6xxx_wifi_ioctl_Ext(u32 cmd_id, void *data, u32 len, bool blocking)
{
    return _ssv6xxx_wifi_ioctl_Ext(cmd_id,data,len,blocking,TRUE);
}
extern int ssv6xxx_chip_init(void);
extern u16 g_acs_channel_mask;
extern u16 g_rc_rate_msk;
extern u16 g_rc_target_pf;
extern u8 g_rc_arith_sft;
s32 _ssv6xxx_set_rate_mask(u16 mask, bool FromAPI);
s32 _ssv6xxx_set_rc_param(u16 target_pf, u8 arith_sft, bool FromAPI);
void ap_mode_on(Ap_setting *ap_setting)
{
    s32 channel=0;
    ssv6xxx_HW_disable();

    if(ssv6xxx_chip_init() == 0)
    {
        LOG_PRINTF("ssv6xxx_chip_init fail\n\r");
    }
    //Set Host API on
    active_host_api = HOST_API_ACTIVE;
    OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
    //Set global variable
    gDeviceInfo->hw_mode = SSV6XXX_HWM_AP;
    //SEC
    gDeviceInfo->APInfo->sec_type = ap_setting->security;
    //password
    MEMCPY((void*)gDeviceInfo->APInfo->password, (void*)ap_setting->password, strlen((void*)ap_setting->password));
    gDeviceInfo->APInfo->password[strlen((void*)ap_setting->password)]=0;

    //SSID
    MEMCPY((void*)gDeviceInfo->APInfo->config.ssid, (void*)ap_setting->ssid.ssid, ap_setting->ssid.ssid_len);
    gDeviceInfo->APInfo->config.ssid[ap_setting->ssid.ssid_len]=0;
    gDeviceInfo->APInfo->config.ssid_len = ap_setting->ssid.ssid_len;
    //set channel
    if(ap_setting->channel==EN_CHANNEL_AUTO_SELECT){
        gDeviceInfo->APInfo->nCurrentChannel    =   AP_DEFAULT_CHANNEL;
        gDeviceInfo->APInfo->config.nChannel    =   AP_DEFAULT_CHANNEL;
    }else{
    gDeviceInfo->APInfo->nCurrentChannel    =   ap_setting->channel;
        gDeviceInfo->APInfo->config.nChannel    =   gDeviceInfo->APInfo->nCurrentChannel;
    }

    //Load FW, init mac
    ssv6xxx_start();

    //send host_cmd
    CmdEng_SetOpMode(MT_RUNNING);
    TXRXTask_SetOpMode(MT_RUNNING);

    //Enable HW
    ssv6xxx_HW_enable();

    OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
    _ssv6xxx_set_rc_param(g_rc_target_pf, g_rc_arith_sft, FALSE);
    _ssv6xxx_set_rate_mask(g_rc_rate_msk, FALSE);

    if(ap_setting->channel==EN_CHANNEL_AUTO_SELECT){
        channel=_ssv6xxx_wifi_auto_channel_selection(g_acs_channel_mask);
        if(-1!=channel){
            gDeviceInfo->APInfo->nCurrentChannel=channel;
            gDeviceInfo->APInfo->config.nChannel=gDeviceInfo->APInfo->nCurrentChannel;
            {
                unsigned char cmd_data[] = {
                    0x00, 0x00, 0x00, 0x00};
                cmd_data[0]= gDeviceInfo->APInfo->nCurrentChannel;
                _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_CAL, cmd_data, 4, TRUE, FALSE);
            }
        }
    }

    //set ap config
    AP_Start();

}
void ap_sta_mode_off(void)
{

    if(gDeviceInfo->hw_mode ==SSV6XXX_HWM_SCONFIG){
        // soc set RX-FLOW-DATA        { M_ENG_MACRX M_ENG_ENCRYPT_SEC M_ENG_CPU M_ENG_MIC_SEC M_ENG_HWHCI M_ENG_CPU M_ENG_CPU M_ENG_CPU }
        const unsigned char cmd_data[] = {0xB4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
        ssv6xxx_drop_none_wsid_frame();
    }
    //send host cmd
    CmdEng_SetOpMode(MT_STOP);
    TXRXTask_SetOpMode(MT_STOP);


    OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
    //Set global variable
    gDeviceInfo->hw_mode = SSV6XXX_HWM_INVALID;
    OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);


    //clean AP mode global variable
    AP_Stop();

//--------------------------------
    //clean sta mode global variable
    gDeviceInfo->status= DISCONNECT;
    MEMSET(gDeviceInfo->joincfg, 0, sizeof(struct cfg_join_request));

    //mlme station mode deinit
    mlme_sta_mode_deinit();
//---------------------------------

    //HCI stop
    ssv6xxx_drv_stop();

    //Disable HW
    ssv6xxx_HW_disable();


    //Set API off
    active_host_api = HOST_API_DEACTIVE;
}

void ap_mode_off(void)
{


    //send host cmd
    CmdEng_SetOpMode(MT_STOP);
    TXRXTask_SetOpMode(MT_STOP);

    //clean AP mode global variable
    AP_Stop();

    //HCI stop
    ssv6xxx_drv_stop();

    //Disable HW
    ssv6xxx_HW_disable();


    //Set API off
    active_host_api = HOST_API_DEACTIVE;
}

ssv6xxx_result _ssv6xxx_wifi_ap(Ap_setting *ap_setting,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;

    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {

        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

        if(ap_setting->status)//AP on
        {
            if( HOST_API_ACTIVE ==active_host_api)
                ap_sta_mode_off();

            ap_mode_on(ap_setting);


        }
        else // AP off
        {
            ap_sta_mode_off();
        }

    }while(0);
    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}

/**********************************************************
Function : ssv6xxx_wifi_ap
Description: Setting ap on or off
1. AP on : The current status must be mode off (ap mode or station mode off)
2. AP off : The current status must be AP on
*********************************************************/
H_APIs ssv6xxx_result ssv6xxx_wifi_ap(Ap_setting *ap_setting)
{
    return _ssv6xxx_wifi_ap(ap_setting,TRUE);
}

void sta_mode_on(ssv6xxx_hw_mode hw_mode)
{

    ssv6xxx_HW_disable();

    if(ssv6xxx_chip_init() == 0)
    {
        LOG_PRINTF("ssv6xxx_chip_init fail\n\r");
    }
    //Set Host API on
    active_host_api = HOST_API_ACTIVE;

    OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
    //Set global variable
    gDeviceInfo->hw_mode = hw_mode;
    OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);

    //Load FW, init mac
    ssv6xxx_start();
    //Host cmd
    CmdEng_SetOpMode(MT_RUNNING);
    TXRXTask_SetOpMode(MT_RUNNING);

    //enable RF
    ssv6xxx_HW_enable();

    if(gDeviceInfo->hw_mode ==SSV6XXX_HWM_SCONFIG){
        // soc set RX-FLOW-DATA        { M_ENG_MACRX M_ENG_ENCRYPT_SEC M_ENG_CPU M_ENG_MIC_SEC M_ENG_HWHCI M_ENG_CPU M_ENG_CPU M_ENG_CPU }
        const unsigned char cmd_data[] = {0x04, 0x01, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
        ssv6xxx_accept_none_wsid_frame();
    }
    _ssv6xxx_set_rc_param(g_rc_target_pf, g_rc_arith_sft, FALSE);
    _ssv6xxx_set_rate_mask(g_rc_rate_msk, FALSE);
}

void sta_mode_off(void)
{

    if(gDeviceInfo->hw_mode ==SSV6XXX_HWM_SCONFIG){
        // soc set RX-FLOW-DATA        { M_ENG_MACRX M_ENG_ENCRYPT_SEC M_ENG_CPU M_ENG_MIC_SEC M_ENG_HWHCI M_ENG_CPU M_ENG_CPU M_ENG_CPU }
        const unsigned char cmd_data[] = {0xB4, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
        ssv6xxx_drop_none_wsid_frame();
    }

    //send host cmd
    CmdEng_SetOpMode(MT_STOP);
    TXRXTask_SetOpMode(MT_STOP);

    //clean sta mode global variable
	 gDeviceInfo->status= DISCONNECT;
     MEMSET(gDeviceInfo->joincfg, 0, sizeof(struct cfg_join_request));

    //mlme station mode deinit
    mlme_sta_mode_deinit();

    //HCI stop
    ssv6xxx_drv_stop();


    //Disable HW
    ssv6xxx_HW_disable();


    //Set API off
    active_host_api = HOST_API_DEACTIVE;


}
ssv6xxx_result _ssv6xxx_wifi_station(u8 hw_mode,Sta_setting *sta_station,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if((SSV6XXX_HWM_STA != hw_mode)&&(SSV6XXX_HWM_SCONFIG != hw_mode)){
        return SSV6XXX_FAILED;
    }

    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {

        if(active_host_api == HOST_API_PWR_SAVING)
        {
            if(mutexLock)
                OS_MutexUnLock(g_host_api_mutex);

            ret = _ssv6xxx_wakeup_wifi();

            if(mutexLock)
                OS_MutexLock(g_host_api_mutex);

            if(FALSE == ret)
                break;
        }

        if(sta_station->status) //Station on
        {
            if(HOST_API_ACTIVE==active_host_api)
                ap_sta_mode_off();

                sta_mode_on(hw_mode);

        }
        else //station off
        {
            ap_sta_mode_off();

        }
    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}

/**********************************************************
Function : ssv6xxx_wifi_station
Description: Setting station on or off
1. station on : The current status must be mode off (ap mode or station mode off)
2. station off : The current status must be Station on
*********************************************************/
H_APIs ssv6xxx_result ssv6xxx_wifi_station(u8 hw_mode,Sta_setting *sta_station)
{
    return _ssv6xxx_wifi_station(hw_mode,sta_station,TRUE);

}

void ap_mode_status(Ap_sta_status *status_info)
{
    u32 idx=0;
    int sta = 0;

    //mac addr
    MEMCPY(status_info->u.ap.selfmac, gDeviceInfo->self_mac,ETH_ALEN);
    //ssid
    MEMCPY((void*)status_info->u.ap.ssid.ssid,(void*)gDeviceInfo->APInfo->config.ssid,gDeviceInfo->APInfo->config.ssid_len);
    status_info->u.ap.ssid.ssid[gDeviceInfo->APInfo->config.ssid_len]=0;
    status_info->u.ap.ssid.ssid_len = gDeviceInfo->APInfo->config.ssid_len;
    //station number
    status_info->u.ap.stanum = gDeviceInfo->APInfo->num_sta;
    //channel
    status_info->u.ap.channel = gDeviceInfo->APInfo->nCurrentChannel;
    for(idx=0; idx<gMaxAID; idx++)
    {
        APStaInfo_st *StaInfo= &gDeviceInfo->StaConInfo[idx];
		if (!test_sta_flag(StaInfo, WLAN_STA_VALID))
        {
        	continue;
		}
        //station info
        MEMCPY(status_info->u.ap.stainfo[sta].Mac,gDeviceInfo->StaConInfo[sta].addr,6);
        if(test_sta_flag(StaInfo, WLAN_STA_AUTH))
            status_info->u.ap.stainfo[sta].status= AUTH;
        if(test_sta_flag(StaInfo, WLAN_STA_AUTHORIZED))
            status_info->u.ap.stainfo[sta].status= ASSOC;
        if(test_sta_flag(StaInfo, WLAN_STA_ASSOC))
            status_info->u.ap.stainfo[sta].status= CONNECT;

        sta++;
    }

    switch(gDeviceInfo->APInfo->sec_type)
    {
        case SSV6XXX_SEC_NONE:
			status_info->u.ap.key_mgmt = WPA_KEY_MGMT_NONE;
            status_info->u.ap.pairwise_cipher = WPA_CIPHER_NONE;
            status_info->u.ap.group_cipher = WPA_CIPHER_NONE;
            break;
    	case SSV6XXX_SEC_WEP_40:
			status_info->u.ap.key_mgmt = WPA_KEY_MGMT_NONE;
            status_info->u.ap.pairwise_cipher = WPA_CIPHER_WEP40;
            status_info->u.ap.group_cipher = WPA_CIPHER_WEP40;
            break;
    	case SSV6XXX_SEC_WEP_104:
			status_info->u.ap.key_mgmt = WPA_KEY_MGMT_NONE;
            status_info->u.ap.pairwise_cipher = WPA_CIPHER_WEP104;
            status_info->u.ap.group_cipher = WPA_CIPHER_WEP104;
            break;
    	case SSV6XXX_SEC_WPA_PSK:
            status_info->u.ap.proto = WPA_PROTO_WPA;
			status_info->u.ap.key_mgmt = WPA_KEY_MGMT_PSK;
            status_info->u.ap.pairwise_cipher = WPA_CIPHER_TKIP;
            status_info->u.ap.group_cipher = WPA_CIPHER_TKIP;
            break;
    	case SSV6XXX_SEC_WPA2_PSK:
            status_info->u.ap.proto = WPA_PROTO_RSN;
			status_info->u.ap.key_mgmt = WPA_KEY_MGMT_PSK;
            status_info->u.ap.pairwise_cipher = WPA_CIPHER_CCMP;
            status_info->u.ap.group_cipher = WPA_CIPHER_CCMP;
            break;
        default:
            break;
    }

}
void sta_mode_status(Ap_sta_status *status_info)
{
    //mac addr
    MEMCPY(status_info->u.station.selfmac, gDeviceInfo->self_mac,ETH_ALEN);
    if(gDeviceInfo->status != DISCONNECT)
    {
        //ssid
        MEMCPY((void*)status_info->u.station.ssid.ssid,(void*)gDeviceInfo->joincfg->bss.ssid.ssid,gDeviceInfo->joincfg->bss.ssid.ssid_len);
        status_info->u.station.ssid.ssid[gDeviceInfo->joincfg->bss.ssid.ssid_len]=0;
        status_info->u.station.ssid.ssid_len = gDeviceInfo->joincfg->bss.ssid.ssid_len;
        //channel
        status_info->u.station.channel = gDeviceInfo->joincfg->bss.channel_id ;
        //AP info mac
        MEMCPY(status_info->u.station.apinfo.Mac,(void*)gDeviceInfo->joincfg->bss.bssid.addr,6);
        status_info->u.station.apinfo.status = gDeviceInfo->status;
        status_info->u.station.capab_info = gDeviceInfo->joincfg->bss.capab_info;
        switch(gDeviceInfo->joincfg->sec_type)
        {
            case SSV6XXX_SEC_NONE:
    			status_info->u.station.key_mgmt = WPA_KEY_MGMT_NONE;

                break;
        	case SSV6XXX_SEC_WEP_40:
                status_info->u.station.proto = 0;
    			status_info->u.station.key_mgmt = WPA_KEY_MGMT_NONE;

                break;
        	case SSV6XXX_SEC_WEP_104:
                status_info->u.station.proto = 0;
    			status_info->u.station.key_mgmt = WPA_KEY_MGMT_NONE;

                break;
        	case SSV6XXX_SEC_WPA_PSK:
                status_info->u.station.proto = WPA_PROTO_WPA;
    			status_info->u.station.key_mgmt = WPA_KEY_MGMT_PSK;

                break;
        	case SSV6XXX_SEC_WPA2_PSK:
                status_info->u.station.proto = WPA_PROTO_RSN;
    			status_info->u.station.key_mgmt = WPA_KEY_MGMT_PSK;

                break;
            default:
                break;
        }

        status_info->u.station.pairwise_cipher=gDeviceInfo->joincfg->bss.pairwise_cipher[0];
        status_info->u.station.group_cipher = gDeviceInfo->joincfg->bss.group_cipher;
    }
}
ssv6xxx_result _ssv6xxx_wifi_status(Ap_sta_status *status_info,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        status_info->status= active_host_api;
        OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
        status_info->operate = gDeviceInfo->hw_mode;

        if(SSV6XXX_HWM_AP == status_info->operate) // AP  mode
        {
            ap_mode_status(status_info);
        }
        else if((SSV6XXX_HWM_STA == status_info->operate)||(SSV6XXX_HWM_SCONFIG == status_info->operate))//station mode  & sconfig mode
        {
            sta_mode_status(status_info);
        }
        OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

    return ret;

}

/**********************************************************
Function : ssv6xxx_wifi_status
Description: Return status information about AP or station
*********************************************************/
H_APIs ssv6xxx_result ssv6xxx_wifi_status(Ap_sta_status *status_info)
{
    return _ssv6xxx_wifi_status(status_info,TRUE);
}

H_APIs u32 ssv6xxx_wifi_get_mode(void)
{
    return (u32)gDeviceInfo->hw_mode;
}

H_APIs struct ssv6xxx_ieee80211_bss * ssv6xxx_wifi_find_ap_ssid(char *ssid)
{

    int i = 0;
    DeviceInfo_st *gdeviceInfo = gDeviceInfo ;

	OS_MutexLock(gDeviceInfo->g_dev_info_mutex);

	for (i = 0; i < NUM_AP_INFO; i++)
    {
        if(gdeviceInfo->ap_list[i].channel_id!= 0)
		{
            if (strcmp((const char *) gdeviceInfo->ap_list[i].ssid.ssid, ssid) == 0)
            {
                break;
            }
		}
	}

	OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);

    if (i == NUM_AP_INFO)
    {
        return NULL;
    }

    return  &(gdeviceInfo->ap_list[i]);
}


H_APIs int ssv6xxx_aplist_get_count()
{
    int i = 0;
    int count = 0;
    DeviceInfo_st *gdeviceInfo = gDeviceInfo ;

    if (!gdeviceInfo)
    {
        return 0;
    }

    for (i=0; i<NUM_AP_INFO; i++)
    {
        if (gdeviceInfo->ap_list[i].channel_id!= 0)
        {
            count++;
        }
    }

    return (count%SSV6XXX_APLIST_PAGE_COUNT) ? (count/SSV6XXX_APLIST_PAGE_COUNT + 1) : (count/SSV6XXX_APLIST_PAGE_COUNT);
}
H_APIs s32 ssv6xxx_aplist_get_count_by_channel(u8 channel)
{
    int i = 0;
    int count = 0;
    DeviceInfo_st *gdeviceInfo = gDeviceInfo ;

    if (!gdeviceInfo)
    {
        return 0;
    }

    for (i=0; i<NUM_AP_INFO; i++)
    {
        if (gdeviceInfo->ap_list[i].channel_id== channel)
        {
            count++;
        }
    }

    return count;
}

H_APIs void ssv6xxx_wifi_set_channel(u8 num)
{
    unsigned char cmd_data[] = {
        0x00, 0x00, 0x00, 0x00};
    cmd_data[0]= num;
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_CAL, cmd_data, 4);
}

H_APIs void ssv6xxx_aplist_get_str(char *wifi_list_str, int page)
{
    u32 i = 0;
    u8  temp[128];
    u32 count = 0;
    DeviceInfo_st *gdeviceInfo = gDeviceInfo ;
    u8 page_num = 1;

    if (!wifi_list_str)
    {
	    LOG_PRINTF("wifi_list_str null !!\r\n");
        return;
    }

    if (!gdeviceInfo)
    {
	    LOG_PRINTF("gdeviceInfo null !!\r\n");
        return;
    }

	OS_MutexLock(gDeviceInfo->g_dev_info_mutex);
    wifi_list_str[0]=0;

    for (i=0; i<NUM_AP_INFO; i++)
    {
        if(gdeviceInfo->ap_list[i].channel_id!= 0)
        {
            if (page_num == page)
            {
                sprintf((void *)temp, "BSSID#%02x:%02x:%02x:%02x:%02x:%02x;",
                gdeviceInfo->ap_list[i].bssid.addr[0],  gdeviceInfo->ap_list[i].bssid.addr[1], gdeviceInfo->ap_list[i].bssid.addr[2],  gdeviceInfo->ap_list[i].bssid.addr[3], gdeviceInfo->ap_list[i].bssid.addr[4],  gdeviceInfo->ap_list[i].bssid.addr[5]);
                strcat(wifi_list_str, (void *)temp);
                sprintf((void *)temp, "SSID#%s;", gdeviceInfo->ap_list[i].ssid.ssid);
                strcat(wifi_list_str, (void *)temp);
                if(gdeviceInfo->ap_list[i].capab_info&BIT(4))
                {
                    sprintf((void *)temp, "proto#%s;",
                       gdeviceInfo->ap_list[i].proto&WPA_PROTO_WPA?"WPA":
                        gdeviceInfo->ap_list[i].proto&WPA_PROTO_RSN?"WPA2":"WEP");
                }
                else
                {
                    sprintf((void *)temp, "proto#%s;", "OPEN");
                }
                strcat(wifi_list_str, (void *)temp);
                sprintf((void *)temp, "channel#%d;\r\n",
                    gdeviceInfo->ap_list[i].channel_id);
                strcat(wifi_list_str, (void *)temp);
            }

            count++;
            if (count >= SSV6XXX_APLIST_PAGE_COUNT)
            {
                count = 0;
                page_num++;
            }
        }
    }

	OS_MutexUnLock(gDeviceInfo->g_dev_info_mutex);
}


//For GP want to set max rate at specific rate
//TODO: The implementation should be moved to rate.c and refined once
//the operation of rate at host is established
s32 _ssv6xxx_set_rate_mask(u16 mask, bool FromAPI)
{
    return _ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_SET_DEFAULT_RATEMASK, &mask, sizeof(mask), FromAPI);
}

H_APIs s32 ssv6xxx_set_rate_mask(u16 mask)
{
    g_rc_rate_msk = mask&0x0FFF;
    return _ssv6xxx_set_rate_mask(g_rc_rate_msk, TRUE);
}

s32 _ssv6xxx_set_rc_param(u16 target_pf, u8 arith_sft, bool FromAPI)
{

    u32 param = 0;

    param = target_pf;
    param = (param<<16)|arith_sft;

    return _ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_SET_RC_PARAM, &param, sizeof(param), FromAPI);
}

H_APIs s32 ssv6xxx_set_rc_param(u16 target_pf, u8 arith_sft)
{
    if ((target_pf == 0) && (arith_sft == 0))
    {
        target_pf = RC_TARGET_PF;
        arith_sft = RC_ARITH_SHIFT;
    }

    if ((target_pf > 24) || (target_pf < 12) || (arith_sft > 8) || arith_sft < 4)
    {
        LOG_PRINTF ("Invalid parameter, PF=[12-24] SFT=[8-4]\r\n");
        return;
    }

    g_rc_target_pf = target_pf;
    g_rc_arith_sft = arith_sft;

    return _ssv6xxx_set_rc_param(g_rc_target_pf, g_rc_arith_sft, TRUE);
}

extern APStaInfo_st *APStaInfo_FindStaByAddr( ETHER_ADDR *mac );
H_APIs s32 ssv6xxx_get_rssi_by_mac(u8 *macaddr)
{
    APStaInfo_st * sta = APStaInfo_FindStaByAddr( (ETHER_ADDR *)macaddr );
    u8 ret_rcpi;

    if(sta)
    {
        OS_MutexLock(sta->apsta_mutex);
        ret_rcpi=sta->rcpi;
        OS_MutexUnLock(sta->apsta_mutex);
        return ret_rcpi;
    }
    return -1;

}

H_APIs void ssv6xxx_get_aplist_info(void *ap_list)
{
    MEMCPY((void *)ap_list,(void *)gDeviceInfo->ap_list,sizeof(gDeviceInfo->ap_list));
}

//Trap RX MGMT Frame to CPU
// soc set RX-FLOW-MGMT        { M_ENG_MACRX M_CPU_RXMGMT M_ENG_CPU M_ENG_CPU }
#define TRAP_RX_MGMT_TO_MCU() \
    {   \
        const unsigned char cmd_data[] = {  \
            0x04, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00};    \
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);    \
    }

//Trap RX MGMT Frame to Host
//soc set RX-FLOW-MGMT        { M_ENG_MACRX M_ENG_ENCRYPT M_ENG_HWHCI M_ENG_CPU}
#define TRAP_RX_MGMT_TO_HOST() \
    {   \
        const unsigned char cmd_data[] = {  \
            0x34, 0x01, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00};    \
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);    \
    }

//Accept Beacon
//#define FRAME_ACCEPT 0
#define ACCEPT_BEACON_IN_AP_MODE() \
    MAC_REG_READ(ADR_MRX_FLT_TB6, _regval); \
    _regFltTb6=_regval; \
    _regval&=~0x01; \
    _regval|=0; \
    MAC_REG_WRITE(ADR_MRX_FLT_TB6, _regval);

//Accept unicat data/mgmt to me in different bssid
#define ACCEPT_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID() \
    MAC_REG_READ(ADR_MRX_FLT_EN7, _regval); \
    _regFltEn7=_regval; \
    _regval |= 0x800; \
    MAC_REG_WRITE(ADR_MRX_FLT_EN7, _regval);

//Drop Beacon
//Restore ADR_MRX_FLT_EN7
#define DROP_BEACON_IN_AP_MODE() \
    MAC_REG_WRITE(ADR_MRX_FLT_EN7, _regFltEn7);

//Restore ADR_MRX_FLT_TB6
#define DROP_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID() \
    MAC_REG_WRITE(ADR_MRX_FLT_TB6, _regFltTb6);

// Fix AGC gain
#define FIX_AGC_GAIN() \
    MAC_REG_READ(ADR_PHY_REG_03_AGC, _regPhy03AGC); \
    MAC_REG_WRITE(ADR_PHY_REG_03_AGC,0x1f30Cfff);

//Restore AGC gain
#define RESTORE_AGC_GAIN() \
    MAC_REG_WRITE(ADR_PHY_REG_03_AGC,_regPhy03AGC);


OsSemaphore scanning_sphr=0;
static inline int _ssv6xxx_start_scan(void)
{
    if(OS_SUCCESS==OS_SemInit(&scanning_sphr, 1, 0))
        return 0;
    else
        return -1;

}
static inline bool _ssv6xxx_wait_scan_done(u32 timeOut)
{
    bool ret=FALSE;
    if(OS_SUCCESS==OS_SemWait(scanning_sphr,timeOut)){
        ret=TRUE;
    }else{
        ret=FALSE;
    }

    OS_SemDelete(scanning_sphr);
    scanning_sphr=0;
    return ret;

}

extern u16 g_acs_channel_scanning_interval;
extern u8  g_acs_channel_scanning_loop;
static s32 _ssv6xxx_wifi_auto_channel_selection(u16 channel_mask)
{
#define INDICATOR_OF_CHANNEL_CHOISE 2 //1: choose by edcca counter, 2: choose by packet counter   3: choose by AP number

    struct cfg_scan_request csreq;
    u32 _regval=0, _regFltEn7=0, _regFltTb6=0,_regPhy03AGC=0;
    u32 i=0,j=0;
    u32 count=0xFFFFFFFF,minCount=0xFFFFFFFF, best_channel=0xFFFFFFFF;
    u16 _channel_mask=channel_mask;
    u32 ap_num1=0,ap_num2=0;
    u8 total_channel=0;
    u8 filter_channel=0;

    if(gDeviceInfo->hw_mode != SSV6XXX_HWM_AP){
        LOG_PRINTF("Invalid arguments.\n");
        return SSV6XXX_FAILED;
    }
    _channel_mask&=~(0xF001); //unmask ch0 , ch12, ch13, ch14, ch15;

    /*
    Count how many channels that user want to scan
    */
    best_channel=6; //Default channel 6
    for(i=0;i<MAX_CHANNEL_NUM;i++){
        if(_channel_mask&(0x01<<i)){
            total_channel++;
            best_channel=i;
        }
    }

    if((total_channel==1)||(total_channel==0)) return best_channel;

    TRAP_RX_MGMT_TO_MCU();

    ACCEPT_BEACON_IN_AP_MODE();

    ACCEPT_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID();

    FIX_AGC_GAIN();

    ssv6xxx_memset(gDeviceInfo->channel_edcca_count,0,sizeof(gDeviceInfo->channel_edcca_count));
    ssv6xxx_memset(gDeviceInfo->channel_packet_count,0,sizeof(gDeviceInfo->channel_packet_count));

    for(i=0;i<g_acs_channel_scanning_loop;i++){
        csreq.is_active=FALSE;
        csreq.n_ssids=0;
        csreq.dwell_time=g_acs_channel_scanning_interval;
        csreq.channel_mask=_channel_mask;

        if(-1==_ssv6xxx_start_scan()) {
            LOG_PRINTF("Auto channel selection fail\r\n");
            return -1;
        }

        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SCAN, (void *)&csreq, sizeof(csreq), TRUE, FALSE);

        if(FALSE==_ssv6xxx_wait_scan_done(g_acs_channel_scanning_interval*best_channel+200)){
            LOG_PRINTF("Auto channel selection time out\r\n");
            break;
        }
    }

    RESTORE_AGC_GAIN();

    DROP_BEACON_IN_AP_MODE();

    DROP_UNICAST_DATA_MGMT_FRAME_FROM_DIFFERENT_BSSID();

    TRAP_RX_MGMT_TO_HOST();

    if(i!=g_acs_channel_scanning_loop){
        return best_channel;
    }

#if 0
    LOG_PRINTF("Channel  edcca count  packet count  AP num\r\n");
    LOG_PRINTF("------------------------------------------------------\r\n");
    for(i=0;i<MAX_CHANNEL_NUM;i++){
        if(_channel_mask&(0x01<<i)){
            LOG_PRINTF("  %2d      %7d       %5d     %5d \r\n",i,gDeviceInfo->channel_edcca_count[i],gDeviceInfo->channel_packet_count[i],ssv6xxx_aplist_get_count_by_channel(i));
        }
    }
#endif


    /*
    Filter the top n channels which have the higher edcca count
    */
    #define MAX_COUNT minCount
    #define WORST_CHANNEL best_channel
    MAX_COUNT=0xFFFFFFFF;
    WORST_CHANNEL=0xFFFFFFFF;
    filter_channel=total_channel>>1;

    for(j=0;j<filter_channel;j++){
        for(i=0;i<MAX_CHANNEL_NUM;i++){
            if(_channel_mask&(0x01<<i)){
                count=gDeviceInfo->channel_edcca_count[i];

                if((MAX_COUNT==0xFFFFFFFF)&&(WORST_CHANNEL==0xFFFFFFFF)){
                    MAX_COUNT=count;
                    WORST_CHANNEL=i;
                }

                if(count>=MAX_COUNT){
                    MAX_COUNT=count;
                    WORST_CHANNEL=i;
                }
            }
        }
        _channel_mask&=(~(0x01<<WORST_CHANNEL));
        MAX_COUNT=0xFFFFFFFF;
        WORST_CHANNEL=0xFFFFFFFF;
    }
    #undef MAX_COUNT
    #undef WORST_CHANNEL

    /*
     Choose a best channel
    */
    minCount=0xFFFFFFFF;
    best_channel=0xFFFFFFFF;
    //LOG_PRINTF("Channel  edcca count  packet count  AP num\r\n");
    //LOG_PRINTF("------------------------------------------------------\r\n");
    for(i=0;i<MAX_CHANNEL_NUM;i++){
        if(_channel_mask&(0x01<<i)){
            //LOG_PRINTF("  %2d      %7d       %5d     %5d \r\n",i,gDeviceInfo->channel_edcca_count[i],gDeviceInfo->channel_packet_count[i],ssv6xxx_aplist_get_count_by_channel(i));
            #if(INDICATOR_OF_CHANNEL_CHOISE==1)
            count=gDeviceInfo->channel_edcca_count[i];
            #elif(INDICATOR_OF_CHANNEL_CHOISE==2)
            count=gDeviceInfo->channel_packet_count[i];
            #elif(INDICATOR_OF_CHANNEL_CHOISE==3)
            count=ssv6xxx_aplist_get_count_by_channel(i);
            #else
            error
            #endif

            if((minCount==0xFFFFFFFF)&&(best_channel==0xFFFFFFFF)){
                minCount=count;
                best_channel=i;
            }

            if(count<minCount){
                minCount=count;
                best_channel=i;
            }

            //If two channels have the same edcca count or packet count, compare their ap num
            #if(INDICATOR_OF_CHANNEL_CHOISE!=3)
            if(count==minCount){
                ap_num1=ssv6xxx_aplist_get_count_by_channel(best_channel);
                ap_num2=ssv6xxx_aplist_get_count_by_channel(i);
                if(ap_num2<ap_num1){
                    minCount=count;
                    best_channel=i;
                }
            }
            #endif

        }
    }
    if(best_channel==0xFFFFFFFF)
        best_channel=6;

    LOG_PRINTF("The best channel is %d\r\n",best_channel);
    return best_channel;
}

ssv6xxx_result _ssv6xxx_wifi_reg_promiscuous_rx_cb(promiscuous_data_handler promiscuous_cb, const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE)
        {
	        ret=SSV6XXX_FAILED;
            break;
        }
        ret = ssv6xxx_wifi_cb(PRMOISCUOUS_CB_NUM, (u32 **)&gDeviceInfo->promiscuous_cb , (u32 *)promiscuous_cb, SSV6XXX_CB_ADD);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);

	return ret;
}

H_APIs ssv6xxx_result ssv6xxx_wifi_reg_promiscuous_rx_cb(promiscuous_data_handler promiscuous_cb)
{
	return _ssv6xxx_wifi_reg_promiscuous_rx_cb(promiscuous_cb,TRUE);
}


ssv6xxx_result _ssv6xxx_wifi_unreg_promiscuous_rx_cb(promiscuous_data_handler promiscuous_cb,const bool mutexLock)
{
    ssv6xxx_result ret = SSV6XXX_SUCCESS;
    //Detect host API status
    if(mutexLock)
        OS_MutexLock(g_host_api_mutex);

    do {
        if(active_host_api == HOST_API_DEACTIVE )
        {
	        ret=SSV6XXX_FAILED;
            break;
        }

        ret = ssv6xxx_wifi_cb(PRMOISCUOUS_CB_NUM, (u32 **)&gDeviceInfo->promiscuous_cb , (u32 *)promiscuous_cb, SSV6XXX_CB_REMOVE);

    }while(0);

    if(mutexLock)
        OS_MutexUnLock(g_host_api_mutex);


	return ret;
}

H_APIs ssv6xxx_result ssv6xxx_wifi_unreg_promiscuous_rx_cb(promiscuous_data_handler promiscuous_cb)
{
	return _ssv6xxx_wifi_unreg_promiscuous_rx_cb(promiscuous_cb,TRUE);
}

