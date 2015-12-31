#include <config.h>
#include <cmd_def.h>
#include <log.h>
#include <host_global.h>
#include "lwip/netif.h"
#include <hdr80211.h>
#include "cli_cmd_wifi.h"
#include "cli_cmd_net.h"
#include <drv_mac.h>
#include <os_wrapper.h>
#include "dev.h"
#include <core/mlme.h>



#define WPA_AUTH_ALG_OPEN BIT(0)
#define WPA_AUTH_ALG_SHARED BIT(1)
#define WPA_AUTH_ALG_LEAP BIT(2)
#define WPA_AUTH_ALG_FT BIT(3)

//#define SEC_USE_NONE
//#define SEC_USE_WEP40_PSK
//#define SEC_USE_WEP40_OPEN
//#define SEC_USE_WEP104_PSK
//#define SEC_USE_WEP104_OPEN
//#define SEC_USE_WPA_TKIP
#define SEC_USE_WPA2_CCMP

extern OsMutex  g_dev_info_mutex;

// iw command executioner
void _cmd_wifi_scan (s32 argc, s8 *argv[]);
static void _cmd_wifi_join (s32 argc, s8 *argv[]);
static void _cmd_wifi_leave (s32 argc, s8 *argv[]);
#if (MLME_SETTING ==0)
static void _cmd_wifi_list (s32 argc, s8 *argv[]);
#else
static void _cmd_wifi_list_new (s32 argc, s8 *argv[]);
#endif
void _cmd_wifi_ap (s32 argc, s8 *argv[]);

#ifdef USE_CMD_RESP
static void _handle_cmd_resp (u32 evt_id, u8 *data, s32 len);
static void _deauth_handler (void *data);
#endif
static void _soc_evt_get_wsid_tbl(u32 evt_id, void *data, s32 len);
static void _soc_evt_get_phy_info(u32 evt_id, void *data, s32 len);
static void _soc_evt_get_decision_tbl(u32 evt_id, void *data, s32 len);
static void _soc_evt_get_sta_mac(u32 evt_id, void *data, s32 len);
static void _soc_evt_get_bssid(u32 evt_id, void *data, s32 len);
static void _soc_evt_get_ether_trap_tbl(u32 evt_id, void *data, s32 len);
static void _soc_evt_get_fcmds(u32 evt_id, void *data, s32 len);

extern OsMutex  g_dev_info_mutex;

// CB for host event
static void _host_event_handler(u32 evt_id, void *data, s32 len);
// Individual event handler
#if (MLME_SETTING ==0)
static void _scan_result_handler (void *data);
#else
static void _scan_result_handler_new(void * data);
#endif
static void _join_result_handler (void *data);
static void _leave_result_handler (void *data);
static void _get_soc_reg_response(u32 eid, void *data, s32 len);
void _soc_evt_handler_ssv6xxx_log(u32 eid, void *data, s32 len);


typedef struct ap_info_st {
    struct ssv6xxx_ieee80211_bss       *ap_info;
}AP_INFO;

typedef struct ba_session_st {
	u16 tid;
	u16 buf_size;
	u16 policy;
	u16 timeout;
} BA_SESSION ;

#define NUM_AP_INFO     (30)
#if(MLME_SETTING==0)
static struct ap_info_st sg_ap_info[NUM_AP_INFO] = {0};
#endif
static BA_SESSION g_ba_session;


#ifdef THROUGHPUT_TEST
extern void cmd_sdio_get_result_cb(u32 evt_id, void *data, s32 len);
#endif


#define ON_OFF_LAG_INTERVAL 1000


/*===================== Start of Get Command Handlers ====================*/


void _soc_evt_get_soc_status(void *data)
{
    char *sta_status[]={ "STA_STATE_UNAUTH_UNASSOC",/*STA_STATE_UNAUTH_UNASSOC*/
                        "STA_STATE_AUTHED_UNASSOC",/*STA_STATE_AUTHED_UNASSOC*/
                        "STA_STATE_AUTHED_ASSOCED",/*STA_STATE_AUTHED_ASSOCED*/
                        "STA_STATE_ASSOCED_4way",/*STA_STATE_ASSOCED_4way*/
                        };
    char *sta_action[]={
                        "STA_ACTION_INIT",/*STA_ACTION_INIT*/
                        "STA_ACTION_IDLE",/*STA_ACTION_IDLE*/
                        "STA_ACTION_READY",/*STA_ACTION_READY*/
                        "STA_ACTION_RUNNING",/*STA_ACTION_RUNNING*/
                        "STA_ACTION_SCANING",/*STA_ACTION_SCANING*/
                        "STA_ACTION_JOING",/*STA_ACTION_JOING*/
                        "STA_ACTION_JOING_4WAY",/*STA_ACTION_JOING_4WAY*/
                        "STA_ACTION_LEAVING" /*STA_ACTION_LEAVING*/
                        };

    struct ST_SOC_STATUS{
        u8  u8SocState;
        u32 u32SocAction;
    }*ps1=NULL;

    ps1=(struct ST_SOC_STATUS *)data;
    //LOG_PRINTF("u8SocState=%d, u32SocAction=%d\r\n",ps1->u8SocState,ps1->u32SocAction);
    LOG_PRINTF("\n  >> soc status:%s\r\n",sta_status[ps1->u8SocState]);
    LOG_PRINTF("\n  >> soc action:%s\r\n",sta_action[ps1->u32SocAction]);
}
void _soc_evt_get_phy_info(u32 evt_id, void *data, s32 len)
{
    u32 i, *val=(u32 *)data;

    ASSERT(len == 4*39*2+4*8+4+4+4+4+4);
    LOG_PRINTF("\n  >> PHY-INFO:\n      ");
    for(i=0; i<39; i++) {
        /* Dump PHY-Info Table: */
        LOG_PRINTF("0x%08x ", *val++);
        if (((i+1)%4) == 0) {
            LOG_PRINTF("\n      ");
        }
    }
    LOG_PRINTF("\n\n      ");

    /* Dump PHY Index Table: */
    for(i=0; i<39; i++) {
        LOG_PRINTF("0x%08x ", *val++);
        if (((i+1)%4) == 0) {
            LOG_PRINTF("\n      ");
        }
    }
    LOG_PRINTF("\n\n      ");

    /* Dump PHY L-LENGTH Table: */
    for(i=0; i<8; i++) {
        LOG_PRINTF("0x%08x ", *val++);
         if (((i+1)%4) == 0) {
             LOG_PRINTF("\n      ");
         }
    }
    LOG_PRINTF("\n");
    LOG_PRINTF("      ADR_INFO_IDX_ADDR: 0x%08x\n", *val++);
    LOG_PRINTF("      ADR_INFO_LEN_ADDR: 0x%08x\n", *val++);
    LOG_PRINTF("      ADR_INFO_MASK: 0x%08x\n", *val++);
    LOG_PRINTF("      ADR_INFO_DEF_RATE: 0x%08x\n", *val++);
    LOG_PRINTF("      ADR_INFO_MRX_OFFSET: 0x%08x\n", *val++);

}


void _soc_evt_get_decision_tbl(u32 evt_id, void *data, s32 len)
{
    u32 i;
    u16 *val=(u16 *)data;

    ASSERT(len == 2*25+2);
    LOG_PRINTF("\n  >> Decision Table:\n      ");
    for(i=0; i<16; i++) {
        LOG_PRINTF("0x%04x ", *val++);
        if (((i+1)&0x03) == 0) {
            LOG_PRINTF("\n      ");
        }
    }
    LOG_PRINTF("\n      ");
    for(i=0; i<9; i++) {
        LOG_PRINTF("0x%04x ", *val++);
        if (((i+1)&0x03) == 0) {
            LOG_PRINTF("\n      ");
        }
    }
    LOG_PRINTF("\n");

}


void _soc_evt_get_sta_mac(u32 evt_id, void *data, s32 len)
{
    ETHER_ADDR *mac=(ETHER_ADDR *)data;
    ASSERT(len == ETHER_ADDR_LEN);
    LOG_PRINTF("    STA-MAC: %02x:%02x:%02x:%02x:%02x:%02x\n\n",
            mac->addr[0], mac->addr[1], mac->addr[2],
            mac->addr[3], mac->addr[4], mac->addr[5]
    );
}


void _soc_evt_get_bssid(u32 evt_id, void *data, s32 len)
{
    ETHER_ADDR *mac=(ETHER_ADDR *)data;
    ASSERT(len == ETHER_ADDR_LEN);
    LOG_PRINTF("    BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n\n",
            mac->addr[0], mac->addr[1], mac->addr[2],
            mac->addr[3], mac->addr[4], mac->addr[5]
    );
}


void _soc_evt_get_ether_trap_tbl(u32 evt_id, void *data, s32 len)
{
    u16 *eth_type = (u16 *)data;
    ASSERT(len == sizeof(u16)*8);
    LOG_PRINTF("  >> Ether Trap Table (Tx):\n");
    LOG_PRINTF("     0x%04x 0x%04x 0x%04x 0x%04x\n ",
        eth_type[0], eth_type[1], eth_type[2], eth_type[3]
    );
    LOG_PRINTF("  >> Ether Trap Table (Rx):\n");
    LOG_PRINTF("     0x%04x 0x%04x 0x%04x 0x%04x\n ",
        eth_type[4], eth_type[5], eth_type[6], eth_type[7]
    );
}


void _soc_evt_get_fcmds(u32 evt_id, void *data, s32 len)
{
    u32 *fcmd=(u32 *)data;
    ASSERT(len == sizeof(u32)*10);
    LOG_PRINTF("  Tx DATA FCmd: 0x%08x, 0x%08x\n", fcmd[0], fcmd[1]);
    LOG_PRINTF("  Tx MGMT FCmd: 0x%08x, 0x%08x\n", fcmd[2], fcmd[3]);
    LOG_PRINTF("  Tx CTRL FCmd: 0x%08x\n", fcmd[4]);
    LOG_PRINTF("  Rx DATA FCmd: 0x%08x, 0x%08x\n", fcmd[5], fcmd[6]);
    LOG_PRINTF("  Rx MGMT FCmd: 0x%08x, 0x%08x\n", fcmd[7], fcmd[8]);
    LOG_PRINTF("  Rx CTRL FCmd: 0x%08x\n", fcmd[9]);
}


void _soc_evt_get_wsid_tbl(u32 evt_id, void *data, s32 len)
{
    struct mac_wsid_entry_st    *wsid_entry;
    s32 i;

    ASSERT(len == sizeof(struct mac_wsid_entry_st)*4);
    wsid_entry = (struct mac_wsid_entry_st *)data;
    LOG_PRINTF("  >> WSID Table:\n      ");
    for(i=0; i<4; i++) {
        if (GET_WSID_INFO_VALID(wsid_entry) == 0) {
            LOG_PRINTF("[%d]: Invalid\n      ", i);
            continue;
        }
        LOG_PRINTF("[%d]: OP Mode: %d, QoS: %s, HT: %s\n      ",
            i, GET_WSID_INFO_OP_MODE(wsid_entry),
            ((GET_WSID_INFO_QOS_EN(wsid_entry)==0)? "disable": "enable"),
            ((GET_WSID_INFO_HT_MODE(wsid_entry)==0)? "disable": "enable")
        );
        LOG_PRINTF("      STA-MAC: %02x:%02x:%02x:%02x:%02x:%02x\n      ",
            wsid_entry->sta_mac.addr[0], wsid_entry->sta_mac.addr[1],
            wsid_entry->sta_mac.addr[2], wsid_entry->sta_mac.addr[3],
            wsid_entry->sta_mac.addr[4], wsid_entry->sta_mac.addr[5]
        );
    }
}
void _soc_evt_get_addba_req(u32 evt_id, void *data, s32 len)
{

	struct cfg_addba_resp *addba_resp;
    struct resp_evt_result *rx_addba_req = (struct resp_evt_result *)data;
	g_ba_session.policy=rx_addba_req->u.addba_req.policy;
	g_ba_session.tid=rx_addba_req->u.addba_req.tid;
	g_ba_session.buf_size=rx_addba_req->u.addba_req.agg_size;
	g_ba_session.timeout=rx_addba_req->u.addba_req.timeout;



    addba_resp = (void *)MALLOC (sizeof(struct cfg_addba_resp));
	addba_resp->dialog_token=1;//spec mention to set nonzero value
	addba_resp->policy=g_ba_session.policy;
	addba_resp->tid=g_ba_session.tid;
	addba_resp->buf_size=g_ba_session.buf_size;
	addba_resp->timeout=g_ba_session.timeout;
	addba_resp->status=0;
	addba_resp->start_seq_num=rx_addba_req->u.addba_req.start_seq_num;


    if (ssv6xxx_wifi_send_addba_resp(addba_resp) < 0)
       	LOG_PRINTF("Command failed !!\n");

    FREE(addba_resp);

}


void _soc_evt_get_delba(u32 evt_id, void *data, s32 len)
{
    struct resp_evt_result *rx_delba = (struct resp_evt_result *)data;
	LOG_PRINTF("RCV DELBA: reason:%d\n",rx_delba->u.delba_req.reason_code);
	ssv6xxx_memset((void *)&g_ba_session,0x00,sizeof(BA_SESSION));

}




/* ====================== End of Get Command Handlers ====================*/



//-------------------------------------------------------------------------------------

extern void set_l2_link(bool on);
extern void cmd_loop_pattern(void);



void _host_event_handler(u32 evt_id, void *data, s32 len)
{
    switch (evt_id) {
    case SOC_EVT_LOG:
        //LOG_PRINTF("SOC_EVT_LOG\n");
        //_soc_evt_handler_ssv6xxx_log(evt_id, data, len);
        break;
    case SOC_EVT_SCAN_RESULT:
#if (MLME_SETTING ==0)
        _scan_result_handler(data);
#else
        _scan_result_handler_new(data);
#endif
        break;
    case SOC_EVT_GET_SOC_STATUS:
        _soc_evt_get_soc_status(data);
        break;
    #ifdef USE_CMD_RESP
    case SOC_EVT_CMD_RESP:
        _handle_cmd_resp(evt_id, data, len);
        break;
    case SOC_EVT_DEAUTH:
        _deauth_handler(data);
        break;
    #else // USE_CMD_RESP
    case SOC_EVT_JOIN_RESULT:
        _join_result_handler(data);
        break;
    case SOC_EVT_LEAVE_RESULT:
        _leave_result_handler(data);
        break;
    case SOC_EVT_GET_REG_RESP:
        _get_soc_reg_response(evt_id, data, len);
        break;

    /*=================================================*/
    case SOC_EVT_GET_STA_MAC_RESP:
        _soc_evt_get_sta_mac(evt_id, data, len);
        break;
    case SOC_EVT_GET_BSSID_RESP:
        _soc_evt_get_bssid(evt_id, data, len);
        break;
    case SOC_EVT_GET_ETHER_TRAP_RESP:
        _soc_evt_get_ether_trap_tbl(evt_id, data, len);
        break;
    case SOC_EVT_GET_FCMDS_RESP:
        _soc_evt_get_fcmds(evt_id, data, len);
        break;
    case SOC_EVT_GET_PHY_INFO_TBL_RESP:
        _soc_evt_get_phy_info(evt_id, data, len);
        break;
    case SOC_EVT_GET_DECI_TABLE_RESP:
        _soc_evt_get_decision_tbl(evt_id, data, len);
        break;
    case SOC_EVT_GET_WSID_TABLE_RESP:
        _soc_evt_get_wsid_tbl(evt_id, data, len);
        break;
	case SOC_EVT_RCV_ADDBA_REQ :
		 _soc_evt_get_addba_req(evt_id, data, len);
		break;
	case SOC_EVT_RCV_DELBA:
		_soc_evt_get_delba(evt_id, data, len);
		break;
    #endif // USE_CMD_RESP
	//----------------------------------------------------------------------------
#if (EDCA_DBG == 1)
	case SOC_EVT_TX_ALL_DONE:


		cmd_loop_pattern();
		break;
#endif//EDCA_DBG
    default:
        LOG_PRINTF("Unknown host event received. %d\n", evt_id);
        break;
    }
} // end of - _host_event_handler -


#ifdef USE_CMD_RESP
void _handle_cmd_resp (u32 evt_id, u8 *data, s32 len)
{
	struct resp_evt_result *resp = (struct resp_evt_result *)data;

	if (resp->result != CMD_OK)
	{
		LOG_PRINTF("Command %d is not OK with code %d.\n", resp->cmd, resp->result);
		return;
    }
    switch (resp->cmd)
        {
        case SSV6XXX_HOST_CMD_SCAN:
			LOG_PRINTF("Scan done.\n");
            break;
        case SSV6XXX_HOST_CMD_JOIN:
            _join_result_handler(data);
            break;
        case SSV6XXX_HOST_CMD_LEAVE:
            _leave_result_handler(data);
            break;
        }

    data += RESP_EVT_HEADER_SIZE,
    len -= RESP_EVT_HEADER_SIZE;
    switch (resp->cmd)
        {
        case SSV6XXX_HOST_CMD_GET_REG:
            _get_soc_reg_response(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_STA_MAC:
            _soc_evt_get_sta_mac(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_BSSID:
            _soc_evt_get_bssid(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_ETHER_TRAP:
            _soc_evt_get_ether_trap_tbl(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_FCMDS:
            _soc_evt_get_fcmds(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_PHY_INFO_TBL:
            _soc_evt_get_phy_info(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_DECI_TBL:
            _soc_evt_get_decision_tbl(evt_id, data, len);
            break;
        case SSV6XXX_HOST_CMD_GET_WSID_TBL:
            _soc_evt_get_wsid_tbl(evt_id, data, len);
            break;
        }
} // end of - _handle_cmd_resp -

void _deauth_handler (void *data)
{
    struct resp_evt_result *leave_res = (struct resp_evt_result *)data;
	LOG_PRINTF("Deauth from AP (reason=%d) !!\n", leave_res->u.leave.reason_code);

    //clean sta mode global variable
    OS_MutexLock(g_dev_info_mutex);
    gDeviceInfo->status= DISCONNECT;
    MEMSET(gDeviceInfo->joincfg, 0, sizeof(struct cfg_join_request));
    OS_MutexUnLock(g_dev_info_mutex);

    set_l2_link(LINK_DOWN);
}

#endif // USE_CMD_RESP

#if (MLME_SETTING == 0)
void _scan_result_handler (void *data)
{
	struct resp_evt_result *scan_res = (struct resp_evt_result *)data;
    struct ssv6xxx_ieee80211_bss *bss =&scan_res->u.scan.bss_info;
    u32                      size;
    s32                      i,pairwise_cipher_index=0,group_cipher_index=0;
	u8      sec_str[][7]={"OPEN","WEP40","WEP104","TKIP","CCMP"};

    /* Check to see if duplicate bssid */
    for (i=0; i<NUM_AP_INFO; i++)
    {
        if (sg_ap_info[i].ap_info==NULL)
            continue;
        if (memcmp((void*)sg_ap_info[i].ap_info->bssid.addr, (void*)bss->bssid.addr, 6) != 0)
            continue;
        /* duplidate response, free old one and keep new one */
        //FREE((void*)sg_ap_info[i].ap_info);
		size = sizeof(struct ssv6xxx_ieee80211_bss);
        //LOG_PRINTF("realloc size=%d\r\n",size);
        //sg_ap_info[i].ap_info = (struct ssv6xxx_ieee80211_bss *)MALLOC(size);
        MEMSET((void*)sg_ap_info[i].ap_info,0,size);
		memcpy((void*)sg_ap_info[i].ap_info, (void*)bss, size);
        return;
    }

    /* Find an empty entry */
    for (i=0; i<NUM_AP_INFO; i++)
    {
        if (sg_ap_info[i].ap_info == NULL)
        {
			size = sizeof(struct ssv6xxx_ieee80211_bss);
            sg_ap_info[i].ap_info = (struct ssv6xxx_ieee80211_bss *)MALLOC(size);
			memcpy((void*)sg_ap_info[i].ap_info, (void*)bss, size);
            LOG_PRINTF("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
                       sg_ap_info[i].ap_info->bssid.addr[0], sg_ap_info[i].ap_info->bssid.addr[1],
					   sg_ap_info[i].ap_info->bssid.addr[2], sg_ap_info[i].ap_info->bssid.addr[3],
					   sg_ap_info[i].ap_info->bssid.addr[4], sg_ap_info[i].ap_info->bssid.addr[5]);
            LOG_PRINTF("SSID: %s \t", sg_ap_info[i].ap_info->ssid.ssid);
			LOG_PRINTF("@Channel Idx: %d\r\n", sg_ap_info[i].ap_info->channel_id);
            #if 1
            LOG_PRINTF("proto: %s\r\n",
                sg_ap_info[i].ap_info->proto&WPA_PROTO_WPA?"WPA":
                sg_ap_info[i].ap_info->proto&WPA_PROTO_RSN?"WPA2":"NONE");

            if(sg_ap_info[i].ap_info->pairwise_cipher[0]){
                pairwise_cipher_index=0;
                LOG_PRINTF("Pairwise cipher=");
                for(pairwise_cipher_index=0;pairwise_cipher_index<8;pairwise_cipher_index++){
                    if(sg_ap_info[i].ap_info->pairwise_cipher[0]&BIT(pairwise_cipher_index)){
                        LOG_PRINTF("[%s] ",sec_str[pairwise_cipher_index]);
                    }
                }
                LOG_PRINTF("\r\n");
            }
            if(sg_ap_info[i].ap_info->group_cipher){
                group_cipher_index=0;
                LOG_PRINTF("Group cipher=");
                for(group_cipher_index=0;group_cipher_index<8;group_cipher_index++){
                    if(sg_ap_info[i].ap_info->group_cipher&BIT(group_cipher_index)){
                        LOG_PRINTF("[%s] ",sec_str[group_cipher_index]);
                    }
                }
                LOG_PRINTF("\r\n");
            }
            //LOG_PRINTF("rpci=%d, snr=%d\r\n",sg_ap_info[i].ap_info->rxphypad.rpci,sg_ap_info[i].ap_info->rxphypad.snr);
            LOG_PRINTF("\r\n");
            #else
			if(sg_ap_info[i].ap_info->pairwise_cipher[0])
				GET_SEC_INDEX(pairwise_cipher_index,8,sg_ap_info[i].ap_info->pairwise_cipher[0]);
			if(sg_ap_info[i].ap_info->group_cipher)
				GET_SEC_INDEX(group_cipher_index,8,sg_ap_info[i].ap_info->group_cipher);
		    LOG_PRINTF("Pairwise cipher=[%s] Group cipher=[%s]\r\n",sec_str[pairwise_cipher_index],sec_str[group_cipher_index]);
            #endif
            return;
        }
    }
    LOG_PRINTF("No empty field for scan responds from SSID: %s\n", bss->ssid.ssid);
} // end of - _scan_result_handler -

#else
void _scan_result_handler_new(void *data)
{
    ap_info_state *scan_res = (ap_info_state*)data;
	LOG_PRINTF("[DEBUG] scan result event: gdevice pointer: %p ; location %d ; ssid: %s ; status: %d \r\n",
		scan_res->apInfo,scan_res->index,scan_res->apInfo[scan_res->index].ssid.ssid,scan_res->act);
}
#endif

void _join_result_handler (void *data)
{
    struct resp_evt_result *join_res = (struct resp_evt_result *)data;
    if (join_res->u.join.status_code != 0)
    {
        LOG_PRINTF("Join failure!!\r\n");
        gDeviceInfo->status = DISCONNECT;
		return;
    }

    LOG_PRINTF("Join success!! AID=%d\r\n", join_res->u.join.aid);
    gDeviceInfo->status = CONNECT;
	//ssv6xxx_wifi_apply_security();
	set_l2_link(LINK_UP);

#if 0
			netif_add(&wlan, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
			netif_set_default(&wlan);
			netif_set_up(&wlan);
			dhcp_start(&wlan);
		   for(i=0;i<6;i++)
		   {
			 wlan.hwaddr[ i ] = MAC_ADDR[i];


		   }
			while (wlan.ip_addr.addr==0) {
				sys_msleep(DHCP_FINE_TIMER_MSECS);
				dhcp_fine_tmr();
				mscnt += DHCP_FINE_TIMER_MSECS;
			 if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
				dhcp_coarse_tmr();
				mscnt = 0;
			 }
		  }
#endif
} // end of - _join_result_handler -


void _leave_result_handler (void *data)
{
    struct resp_evt_result *leave_res = (struct resp_evt_result *)data;
	LOG_PRINTF("Leave received deauth from AP (reason=%d) !!\n", leave_res->u.leave.reason_code);

    //clean sta mode global variable
    OS_MutexLock(g_dev_info_mutex);
    gDeviceInfo->status= DISCONNECT;
    MEMSET(gDeviceInfo->joincfg, 0, sizeof(struct cfg_join_request));
    OS_MutexUnLock(g_dev_info_mutex);

    //set_l2_link(LINK_DOWN);
}


void _get_soc_reg_response(u32 eid, void *data, s32 len)
{
    LOG_PRINTF("%s(): HOST_EVENT=%d: len=%d\n", __FUNCTION__, eid, len);
    memcpy((void *)g_soc_cmd_rx_buffer, (void *)data, len);
    g_soc_cmd_rx_ready = 1;
}

void cmd_iw(s32 argc, s8 *argv[])
{
//    ssv6xxx_wifi_reg_evt_cb(_host_event_handler);
//gHCmdEngInfo

	if (argc<2)
		return;


    if (strcmp(argv[1], "scan")==0) {
        if (argc >= 3)
		    _cmd_wifi_scan(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
	} else if (strcmp(argv[1], "join")==0) {
        if (argc >= 3)
            _cmd_wifi_join(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    } else if (strcmp(argv[1], "leave")==0) {
        if (argc==2)
            _cmd_wifi_leave(argc - 2, &argv[2]);
        else
            LOG_PRINTF("Invalid arguments.\n");
    } else if (strcmp(argv[1], "list")==0) {
        if (argc == 2)
            {
#if(MLME_SETTING == 0)
            _cmd_wifi_list(argc - 2, &argv[2]);
#else
            _cmd_wifi_list_new(argc - 2, &argv[2]);
#endif
            }
        else
            LOG_PRINTF("Invalid arguments.\n");
        /*
    } else if (strcmp(argv[1], "ap")==0) {
        if (argc >= 3)
            _cmd_wifi_ap(argc - 2, &argv[2]);
		else
            LOG_PRINTF("Invalid arguments.\n");
        */
    }
	else {
        LOG_PRINTF("Invalid iw command.\n");
    }
} // end of - cmd_iw -



void _cmd_wifi_scan (s32 argc, s8 *argv[])
{
    /**
     *  Scan Command Usage:
     *  iw scan <chan_mask> <ssid0> <ssid1> <ssid2> ...
     */
    struct cfg_80211_ssid   *ssid;
    struct cfg_scan_request *ScanReq;
    int                      num_ssids = argc - 1;
    int                      i;

    ScanReq = (void *)MALLOC(sizeof(struct cfg_scan_request)+num_ssids*sizeof(struct cfg_80211_ssid));
    ScanReq->is_active      = true;
    ScanReq->n_ssids        = num_ssids;
    ScanReq->channel_mask   = (u16)strtol(argv[0],NULL,16);
    ScanReq->dwell_time = 0;


    ssid = (struct cfg_80211_ssid *)ScanReq->ssids;
    for (i=0; i < num_ssids; i++)
    {
        ScanReq->ssids[i].ssid_len = strlen(argv[i+1]);
        memcpy((void*)ScanReq->ssids[i].ssid, (void*)argv[i+1], ScanReq->ssids[i].ssid_len);
    }

    if (ssv6xxx_wifi_scan(ScanReq) < 0)
       	LOG_PRINTF("Command failed !!\n");

    FREE(ScanReq);
} // end of - _cmd_wifi_scan -


static void host_set_cal( u32 channel )
{
    ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_CAL, &channel, sizeof(u32));
}
//iw join ap_name [wep|wpa|wpa2] passwd
void _cmd_wifi_join (s32 argc, s8 *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw join ssid
     */
    s32    size,i;
    struct cfg_join_request *JoinReq;
    const char *sec_name;
	
	
#if(MLME_SETTING==0)
    for (i=0; i<NUM_AP_INFO; i++)
    {
        if (sg_ap_info[i].ap_info == NULL)
            continue;
        if (strcmp((const char *) sg_ap_info[i].ap_info->ssid.ssid, argv[0]) != 0)
            continue;
        break;
    }
#else
	DeviceInfo_st *gdeviceInfo = gDeviceInfo ;
	OS_MutexLock(g_dev_info_mutex); 
	
	for (i=0; i<MAX_NUM_OF_AP; i++)
    {
        if (gdeviceInfo->ap_list[i].channel_id != 0)
    	{
			if (strcmp( argv[0], (const char *)gdeviceInfo->ap_list[i].ssid.ssid) != 0)
				{
					continue;
				}
			else
				{
					break;
				}
    	}
    }
#endif

    if (i == NUM_AP_INFO)
    {
        LOG_PRINTF("No AP \"%s\" was found.\n", argv[0]);
        return;
    }
	
#if(MLME_SETTING==0)
    host_set_cal(sg_ap_info[i].ap_info->channel_id);
#else
	host_set_cal(gdeviceInfo->ap_list[i].channel_id);
#endif

    size = sizeof(*JoinReq)+ sizeof(struct ssv6xxx_ieee80211_bss);
    JoinReq = (struct cfg_join_request *)MALLOC(size);
    ssv6xxx_memset((void *)JoinReq, 0, size);

    if (argc == 3)
        memcpy(JoinReq->password, argv[2], strlen(argv[2]));

    if (argc == 1)
    {
//#ifdef SEC_USE_NONE
#if 1
        sec_name = "open";
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_NONE;
#endif // SEC_USE_NONE
    } else if (strcmp(argv[1], "wep40") == 0)
    {
//#if defined(SEC_USE_WEP40_OPEN) || defined(SEC_USE_WEP40_PSK) || defined(SEC_USE_WEP104_OPEN) || defined(SEC_USE_WEP104_PSK)
#if 1
//WEP40 Share Key
        sec_name = "wep40";
        #if defined(SEC_USE_WEP40_OPEN) || defined(SEC_USE_WEP104_OPEN)
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        #else
	    JoinReq->auth_alg = WPA_AUTH_ALG_SHARED;
        #endif

        JoinReq->wep_keyidx = 0;

        //#if defined(SEC_USE_WEP40_OPEN) || defined(SEC_USE_WEP40_PSK)
        #if 1
    	JoinReq->sec_type = SSV6XXX_SEC_WEP_40;
		if (argc != 3)
		{
	        JoinReq->password[0]= 0x31;
			JoinReq->password[1]= 0x32;
    		JoinReq->password[2]= 0x33;
	    	JoinReq->password[3]= 0x34;
			JoinReq->password[4]= 0x35;
    		JoinReq->password[5]= '\0';
		}

        #else
        JoinReq->sec_type = SSV6XXX_SEC_WEP_104;
        JoinReq->password[0]= '0';
        JoinReq->password[1]= '1';
        JoinReq->password[2]= '2';
        JoinReq->password[3]= '3';
        JoinReq->password[4]= '4';
        JoinReq->password[5]= '5';
        JoinReq->password[6]= '6';
        JoinReq->password[7]= '7';
        JoinReq->password[8]= '8';
        JoinReq->password[9]= '9';
        JoinReq->password[10]= '0';
        JoinReq->password[11]= '1';
        JoinReq->password[12]= '2';
        JoinReq->password[13]= '\0';
        #endif
#endif // WEP
		} else if (strcmp(argv[1], "wep104") == 0)
		{
			sec_name = "wep104";
        #if defined(SEC_USE_WEP40_OPEN) || defined(SEC_USE_WEP104_OPEN)
			JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        #else
			JoinReq->auth_alg = WPA_AUTH_ALG_SHARED;
        #endif

			JoinReq->wep_keyidx = 0;

			JoinReq->sec_type = SSV6XXX_SEC_WEP_104;
			if (argc != 3)
			{
				JoinReq->password[0]= '0';
				JoinReq->password[1]= '1';
				JoinReq->password[2]= '2';
				JoinReq->password[3]= '3';
				JoinReq->password[4]= '4';
				JoinReq->password[5]= '5';
				JoinReq->password[6]= '6';
				JoinReq->password[7]= '7';
				JoinReq->password[8]= '8';
				JoinReq->password[9]= '9';
				JoinReq->password[10]= '0';
				JoinReq->password[11]= '1';
				JoinReq->password[12]= '2';
				JoinReq->password[13]= '\0';
			}
    } else if (strcmp(argv[1], "wpa2") == 0)
    {
//#ifdef SEC_USE_WPA2_CCMP
#if 1
        sec_name = "wpa2";
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_WPA2_PSK;
		if (argc != 3)
		{
	        JoinReq->password[0]= 's';
		    JoinReq->password[1]= 'e';
	        JoinReq->password[2]= 'c';
		    JoinReq->password[3]= 'r';
			JoinReq->password[4]= 'e';
			JoinReq->password[5]= 't';
	        JoinReq->password[6]= '0';
		    JoinReq->password[7]= '0';
			JoinReq->password[8]= '\0';
		}
#endif // SEC_USE_WPA
    } else if (strcmp(argv[1], "wpa") == 0)
    {
//#ifdef SEC_USE_WPA_TKIP
#if 1
	    sec_name = "wpa";
		JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
	    JoinReq->sec_type = SSV6XXX_SEC_WPA_PSK;
		if (argc != 3)
		{
			JoinReq->password[0]= 's';
			JoinReq->password[1]= 'e';
			JoinReq->password[2]= 'c';
			JoinReq->password[3]= 'r';
			JoinReq->password[4]= 'e';
			JoinReq->password[5]= 't';
			JoinReq->password[6]= '0';
			JoinReq->password[7]= '0';
		    JoinReq->password[8]= '\0';
		}
#endif // SEC_USE_WPA
    } else
    {
        LOG_PRINTF("ERROR: unkown security type: %s\n", argv[1]);
        sec_name = "open";
        JoinReq->auth_alg = WPA_AUTH_ALG_OPEN;
        JoinReq->sec_type = SSV6XXX_SEC_NONE;
    }

#if(MLME_SETTING==0)
	memcpy((void*)&JoinReq->bss, (void*)sg_ap_info[i].ap_info, sizeof(struct ssv6xxx_ieee80211_bss));
#else
	JoinReq->bss = gdeviceInfo->ap_list[i];
	OS_MutexUnLock(g_dev_info_mutex); 
#endif

    LOG_PRINTF("dtim_period=%d\r\n",JoinReq->bss.dtim_period);
    LOG_PRINTF("wmm_used=%d\r\n",JoinReq->bss.wmm_used);
    LOG_PRINTF("Joining \"%s\" using security type \"%s\".\r\n", JoinReq->bss.ssid.ssid, sec_name);

    if (ssv6xxx_wifi_join(JoinReq) < 0)
	    LOG_PRINTF("Command failed !!\n");

    FREE(JoinReq);
} // end of - _cmd_wifi_join -

void _cmd_wifi_leave(s32 argc, s8 *argv[])
{
    /**
	 *	Leave Command Usage:
	 *	host leave ... ...
	 */
    struct cfg_leave_request *LeaveReq;
	//u16 bssid[6]={0x02,0x00,0x00,0x00,0x00,0x00};
	LeaveReq = (void *)MALLOC(sizeof(*LeaveReq));
	LeaveReq->reason = 1;
	//memcpy(LeaveReq->bssid.addr,bssid,6);

    if (ssv6xxx_wifi_leave(LeaveReq) < 0)
	    LOG_PRINTF("Command failed !!\n");
    else // Down the link if leave command is sent.
        set_l2_link(LINK_DOWN);
    FREE(LeaveReq);
} // end of - _cmd_wifi_leave -

#if (MLME_SETTING == 0)
void _cmd_wifi_list(s32 argc, s8 *argv[])
{
    u32 i=0;
    s32 pairwise_cipher_index=0,group_cipher_index=0;
    u8  sec_str[][7]={"OPEN","WEP40","WEP104","TKIP","CCMP"};

    LOG_PRINTF("\r\n");
    for (i=0; i<NUM_AP_INFO; i++)
    {
        if(sg_ap_info[i].ap_info!=NULL)
        {
            LOG_PRINTF("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\r\n",
            sg_ap_info[i].ap_info->bssid.addr[0],  sg_ap_info[i].ap_info->bssid.addr[1], sg_ap_info[i].ap_info->bssid.addr[2],  sg_ap_info[i].ap_info->bssid.addr[3],  sg_ap_info[i].ap_info->bssid.addr[4],  sg_ap_info[i].ap_info->bssid.addr[5]);
            LOG_PRINTF("SSID: %s\r\n", sg_ap_info[i].ap_info->ssid.ssid);
#if 1
            LOG_PRINTF("proto: %s\r\n",
                sg_ap_info[i].ap_info->proto&WPA_PROTO_WPA?"WPA":
                sg_ap_info[i].ap_info->proto&WPA_PROTO_RSN?"WPA2":"NONE");

            if(sg_ap_info[i].ap_info->pairwise_cipher[0]){
                pairwise_cipher_index=0;
                LOG_PRINTF("Pairwise cipher=");
                for(pairwise_cipher_index=0;pairwise_cipher_index<8;pairwise_cipher_index++){
                    if(sg_ap_info[i].ap_info->pairwise_cipher[0]&BIT(pairwise_cipher_index)){
                        LOG_PRINTF("[%s] ",sec_str[pairwise_cipher_index]);
                    }
                }
                LOG_PRINTF("\r\n");
            }
            if(sg_ap_info[i].ap_info->group_cipher){
                group_cipher_index=0;
                LOG_PRINTF("Group cipher=");
                for(group_cipher_index=0;group_cipher_index<8;group_cipher_index++){
                    if(sg_ap_info[i].ap_info->group_cipher&BIT(group_cipher_index)){
                        LOG_PRINTF("[%s] ",sec_str[group_cipher_index]);
                    }
                }
                LOG_PRINTF("\r\n");
            }
            LOG_PRINTF("\r\n");
            //LOG_PRINTF("rpci=%d, snr=%d\r\n",sg_ap_info[i].ap_info->rxphypad.rpci,sg_ap_info[i].ap_info->rxphypad.snr);
#else
            if(sg_ap_info[i].ap_info->pairwise_cipher[0])
                GET_SEC_INDEX(pairwise_cipher_index,8,sg_ap_info[i].ap_info->pairwise_cipher[0]);
            if(sg_ap_info[i].ap_info->rsn_group_cipher)
                GET_SEC_INDEX(group_cipher_index,8,sg_ap_info[i].ap_info->group_cipher);

            LOG_PRINTF("SECURITY Pairwise[%s] Group[%s]\r\n",sec_str[pairwise_cipher_index],sec_str[group_cipher_index]);
#endif
            pairwise_cipher_index=0,group_cipher_index=0;
        }
    }
} // end of - _cmd_wifi_list -

#else
void _cmd_wifi_list_new(s32 argc, s8 *argv[])
{
    u32 i=0;
    s32     pairwise_cipher_index=0,group_cipher_index=0;
	u8      sec_str[][7]={"OPEN","WEP40","WEP104","TKIP","CCMP"};
    DeviceInfo_st *gdeviceInfo = gDeviceInfo ;

	LOG_PRINTF("\n");
	OS_MutexLock(g_dev_info_mutex); 
	
	for (i=0; i<NUM_AP_INFO; i++)
    {
        if(gdeviceInfo->ap_list[i].channel_id!= 0)	
		{
		    LOG_PRINTF("BSSID: %02x:%02x:%02x:%02x:%02x:%02x\n",
            gdeviceInfo->ap_list[i].bssid.addr[0],  gdeviceInfo->ap_list[i].bssid.addr[1], gdeviceInfo->ap_list[i].bssid.addr[2],  gdeviceInfo->ap_list[i].bssid.addr[3],  gdeviceInfo->ap_list[i].bssid.addr[4],  gdeviceInfo->ap_list[i].bssid.addr[5]);
            LOG_PRINTF("SSID: %s\n", gdeviceInfo->ap_list[i].ssid.ssid);


            LOG_PRINTF("proto: %s\r\n",
            gdeviceInfo->ap_list[i].proto&WPA_PROTO_WPA?"WPA":
            gdeviceInfo->ap_list[i].proto&WPA_PROTO_RSN?"WPA2":"NONE");

            if(gdeviceInfo->ap_list[i].pairwise_cipher[0]){
                pairwise_cipher_index=0;
                LOG_PRINTF("Pairwise cipher=");
                for(pairwise_cipher_index=0;pairwise_cipher_index<8;pairwise_cipher_index++){
                    if(gdeviceInfo->ap_list[i].pairwise_cipher[0]&BIT(pairwise_cipher_index)){
                        LOG_PRINTF("[%s] ",sec_str[pairwise_cipher_index]);
                    }
                }
                LOG_PRINTF("\r\n");
            }
            if(gdeviceInfo->ap_list[i].group_cipher){
                group_cipher_index=0;
                LOG_PRINTF("Group cipher=");
                for(group_cipher_index=0;group_cipher_index<8;group_cipher_index++){
                    if(gdeviceInfo->ap_list[i].group_cipher&BIT(group_cipher_index)){
                        LOG_PRINTF("[%s] ",sec_str[group_cipher_index]);
                    }
                }
                LOG_PRINTF("\r\n");
            }
            LOG_PRINTF("\r\n");
            //LOG_PRINTF("rpci=%d, snr=%d\r\n",gdeviceInfo[i].ap_info->rxphypad.rpci,gdeviceInfo[i].ap_info->rxphypad.snr);
		}
	}
	OS_MutexUnLock(g_dev_info_mutex); 
} // end of - _cmd_wifi_list -
#endif

void cmd_wifi_delba (s32 argc, s8 *argv[])
{


	struct cfg_delba *tx_delba;

    tx_delba = (struct cfg_delba *)MALLOC (sizeof(struct cfg_delba));
	tx_delba->tid=g_ba_session.tid;
	tx_delba->initiator=1;//set to 1 to indicate the originator


    if (ssv6xxx_wifi_send_delba(tx_delba) < 0)
       	LOG_PRINTF("Command failed !!\n");

    FREE(tx_delba);

}



// iw ap ssid sectype password
// iw ap ssv  password
void _cmd_wifi_ap (s32 argc, s8 *argv[])
{
    /**
	 * Join Command Usage:
	 *	iw ap ssid
     */
    const char *sec_name;
	struct cfg_set_ap_cfg ApCfg;
    MEMSET(&ApCfg, 0, sizeof(struct cfg_set_ap_cfg));

//Fill SSID
    ApCfg.ssid.ssid_len = strlen(argv[0]);
    memcpy( (void*)&ApCfg.ssid.ssid, (void*)argv[0], strlen(argv[0]));

//Fill PASSWD
	if (argc == 3)
		memcpy(&ApCfg.password, argv[2], strlen(argv[2]));


    if (argc == 1){
        sec_name = "open";
        ApCfg.sec_type = SSV6XXX_SEC_NONE;
    }
	else if (strcmp(argv[1], "wep40") == 0){
        sec_name = "wep40";
    	ApCfg.sec_type = SSV6XXX_SEC_WEP_40;
		if (argc != 3)
		{
	        ApCfg.password[0]= 0x31;
			ApCfg.password[1]= 0x32;
    		ApCfg.password[2]= 0x33;
	    	ApCfg.password[3]= 0x34;
			ApCfg.password[4]= 0x35;
    		ApCfg.password[5]= '\0';
		}


	}
	else if (strcmp(argv[1], "wep104") == 0){
			sec_name = "wep104";

			ApCfg.sec_type = SSV6XXX_SEC_WEP_104;
			if (argc != 3)
			{
				ApCfg.password[0]= '0';
				ApCfg.password[1]= '1';
				ApCfg.password[2]= '2';
				ApCfg.password[3]= '3';
				ApCfg.password[4]= '4';
				ApCfg.password[5]= '5';
				ApCfg.password[6]= '6';
				ApCfg.password[7]= '7';
				ApCfg.password[8]= '8';
				ApCfg.password[9]= '9';
				ApCfg.password[10]= '0';
				ApCfg.password[11]= '1';
				ApCfg.password[12]= '2';
				ApCfg.password[13]= '\0';
			}
    }
	else if (strcmp(argv[1], "wpa2") == 0){
        sec_name = "wpa2";
	  	ApCfg.sec_type = SSV6XXX_SEC_WPA2_PSK;

		if (argc != 3)
		{
	        ApCfg.password[0]= 's';
		    ApCfg.password[1]= 'e';
	       	ApCfg.password[2]= 'c';
		    ApCfg.password[3]= 'r';
			ApCfg.password[4]= 'e';
			ApCfg.password[5]= 't';
	        ApCfg.password[6]= '0';
		    ApCfg.password[7]= '0';
			ApCfg.password[8]= '\0';
		}

    }
	else if (strcmp(argv[1], "wpa") == 0){

	    sec_name = "wpa";
	    ApCfg.sec_type = SSV6XXX_SEC_WPA_PSK;

		if (argc != 3)
		{
			ApCfg.password[0]= 's';
			ApCfg.password[1]= 'e';
			ApCfg.password[2]= 'c';
			ApCfg.password[3]= 'r';
			ApCfg.password[4]= 'e';
			ApCfg.password[5]= 't';
			ApCfg.password[6]= '0';
			ApCfg.password[7]= '0';
		    ApCfg.password[8]= '\0';
		}

    }
	else{
        LOG_PRINTF("ERROR: unkown security type: %s\n", argv[1]);
        sec_name = "open";
        //ApCfg.auth_alg = WPA_AUTH_ALG_OPEN;
        ApCfg.sec_type = SSV6XXX_SEC_NONE;
    }


    LOG_PRINTF("AP configuration==>\nSSID:\"%s\" \nSEC Type:\"%s\" \nPASSWD:\"%s\"\n",
		ApCfg.ssid.ssid, sec_name, ApCfg.password);



    if (ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_SET_AP_CFG, &ApCfg, sizeof(ApCfg)) < 0)
	    LOG_PRINTF("Command failed !!\n");


} // end of - _cmd_wifi_join -




void ssv6xxx_wifi_cfg(void)
{
    ssv6xxx_wifi_reg_evt_cb(_host_event_handler);
}

void cmd_ctl(s32 argc, s8 *argv[])
{

    bool errormsg = TRUE;
    if (argc <= 1)
    {
        errormsg = TRUE;
    }
	else if (strcmp(argv[1], "status")==0)
    {
        Ap_sta_status info;
        MEMSET(&info , 0 , sizeof(Ap_sta_status));
        errormsg = FALSE;
        ssv6xxx_wifi_status(&info);
        if(info.status)
            LOG_PRINTF("status:ON\n");
        else
            LOG_PRINTF("status:OFF\n");
        if(SSV6XXX_HWM_STA==info.operate)
        {
            LOG_PRINTF("Mode:Station\n");
            LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                info.u.station.selfmac[0],
                info.u.station.selfmac[1],
                info.u.station.selfmac[2],
                info.u.station.selfmac[3],
                info.u.station.selfmac[4],
                info.u.station.selfmac[5]);
            LOG_PRINTF("SSID:%s\n",info.u.station.ssid.ssid);
            LOG_PRINTF("AP Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                info.u.station.apinfo.Mac[0],
                info.u.station.apinfo.Mac[1],
                info.u.station.apinfo.Mac[2],
                info.u.station.apinfo.Mac[3],
                info.u.station.apinfo.Mac[4],
                info.u.station.apinfo.Mac[5]);


        }
        else
        {
            u32 statemp;
            LOG_PRINTF("Mode:AP\n");
            LOG_PRINTF("self Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                info.u.ap.selfmac[0],
                info.u.ap.selfmac[1],
                info.u.ap.selfmac[2],
                info.u.ap.selfmac[3],
                info.u.ap.selfmac[4],
                info.u.ap.selfmac[5]);
            LOG_PRINTF("SSID:%s\n",info.u.ap.ssid.ssid);
            LOG_PRINTF("channel:%d\n",info.u.ap.channel);
            LOG_PRINTF("Station number:%d\n",info.u.ap.stanum);
            for(statemp=0; statemp < info.u.ap.stanum ;statemp ++ )
            {
                LOG_PRINTF("station Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                    info.u.ap.stainfo[statemp].Mac[0],
                    info.u.ap.stainfo[statemp].Mac[1],
                    info.u.ap.stainfo[statemp].Mac[2],
                    info.u.ap.stainfo[statemp].Mac[3],
                    info.u.ap.stainfo[statemp].Mac[4],
                    info.u.ap.stainfo[statemp].Mac[5]);
            }
            APStaInfo_PrintStaInfo();
        }
    }
    else if (strcmp(argv[1], "ap")==0&&argc >= 3)
    {
        Ap_setting ap;
        MEMSET(&ap , 0 , sizeof(Ap_setting));
        ap.channel =AP_DEFAULT_CHANNEL;

        //instruction dispatch
        // wifictl ap on [ap_name] [security] [password] [channel]
        switch(argc)
        {
            case 3: // wifi ap off
                if(strcmp(argv[2], "off")==0)
                {
                    errormsg =FALSE;
                    ap.status = FALSE;                                       
                }

                break;
            case 4: // only ssid , security open
                if(strcmp(argv[2], "on" )== 0)
                {
                    errormsg =FALSE;
                    ap.status = TRUE;
                    ap.security = SSV6XXX_SEC_NONE;
                    ap.ssid.ssid_len = strlen(argv[3]);		
                    MEMCPY( (void*)ap.ssid.ssid, (void*)argv[3], strlen(argv[3]));
                }
                break;
                
            case 5: // only ssid , security open, set channel
                    if(strcmp(argv[2], "on" )== 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = SSV6XXX_SEC_NONE;
                        ap.ssid.ssid_len = strlen(argv[3]);  
                        ap.channel = (u8)strtol(argv[4],NULL,10);
                        MEMCPY( (void*)ap.ssid.ssid, (void*)argv[3], strlen(argv[3]));
                    }
                    break;

                
            case 6: //have security type
                if(strcmp(argv[2], "on") == 0)
                {
                    if(strcmp(argv[4], "wep40") == 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = 	SSV6XXX_SEC_WEP_40;
                    }
                    else if(strcmp(argv[4], "wep104") == 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = 	SSV6XXX_SEC_WEP_104;
                    }
                    else if(strcmp(argv[4], "wpa") == 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = SSV6XXX_SEC_WPA_PSK;
                        ap.proto = WPA_PROTO_WPA;
                        ap.key_mgmt = WPA_KEY_MGMT_PSK ;
                        ap.group_cipher=WPA_CIPHER_TKIP;
                        ap.pairwise_cipher = WPA_CIPHER_TKIP;
                    }
                    else if (strcmp(argv[4], "wpa2") == 0)
                    {
                        errormsg =FALSE;
                        ap.status = TRUE;
                        ap.security = SSV6XXX_SEC_WPA2_PSK;
                        ap.proto = WPA_PROTO_RSN;
                        ap.key_mgmt = WPA_KEY_MGMT_PSK ;
                        ap.group_cipher=WPA_CIPHER_CCMP;
                        ap.pairwise_cipher = WPA_CIPHER_CCMP;
                    }

                }
                break;
            default:
                errormsg = TRUE;
                break;


        }
        if(!errormsg)
        {
	        if(ssv6xxx_wifi_ap(&ap) == SSV6XXX_FAILED)
                errormsg =  TRUE;
            else{
                if(ap.status == TRUE)
                    set_l2_link(TRUE);
                else
                    set_l2_link(FALSE);
            }

        }


	}
    else if (strcmp(argv[1], "sta")==0&&argc >= 3)
	{
        Sta_setting sta;
        MEMSET(&sta, 0 , sizeof(Sta_setting));

        if(strcmp(argv[2], "on") == 0)
        {
            sta.status = TRUE;
        }
        else
        {
            sta.status = FALSE;
        }


        if(ssv6xxx_wifi_station(&sta) == SSV6XXX_FAILED)
            errormsg =  TRUE;
        else
            errormsg =FALSE;


    }
    else if (strcmp(argv[1], "test")==0 && argc >= 4)
    {
        u32 r;
        u32 maxcouunt = (u32)ssv6xxx_atoi_base(argv[3], 16);
        u32 testcount=1;
        
        Sta_setting sta;
        Ap_setting ap;
        MEMSET(&sta, 0 , sizeof(Sta_setting));
        MEMSET(&ap, 0 , sizeof(Ap_setting));


        ap.security = SSV6XXX_SEC_NONE;
        ap.ssid.ssid_len = strlen(argv[2]);		
        ap.channel =AP_DEFAULT_CHANNEL;
        MEMCPY( (void*)ap.ssid.ssid, (void*)argv[2], strlen(argv[2]));
        
        errormsg =FALSE;

        sta.status = FALSE;
        ssv6xxx_wifi_station(&sta);
        ap.status = FALSE;
        ssv6xxx_wifi_ap(&ap);


        for(testcount=1 ;testcount <= maxcouunt;testcount++)
        {
            r = OS_Random();
            if(r&0x0001 == 1)//ap mode
            {

                ap.status = TRUE;
                if(ssv6xxx_wifi_ap(&ap) == SSV6XXX_SUCCESS)
                    LOG_PRINTF("No.%d:\tAP off.\n",testcount);
                Sleep(ON_OFF_LAG_INTERVAL);
                ap.status = FALSE;
                if(ssv6xxx_wifi_ap(&ap) == SSV6XXX_SUCCESS)
                    LOG_PRINTF("No.:%d\tAP on.\n",testcount);

            }
            else //station mode
            {
                sta.status = TRUE;
                if(ssv6xxx_wifi_station(&sta) == SSV6XXX_SUCCESS)
                    LOG_PRINTF("No.%d:\tSta on.\n",testcount);
                Sleep(ON_OFF_LAG_INTERVAL);
                sta.status = FALSE;
                if(ssv6xxx_wifi_station(&sta) == SSV6XXX_SUCCESS)
                    LOG_PRINTF("No.%d:\tSta off.\n",testcount);
            }
        }

    }

	if(errormsg){
        LOG_PRINTF("Invalid wifictl command.\n");
    }
    else
    {
        LOG_PRINTF("OK.\n");
    }
} // end of - cmd_iw -


