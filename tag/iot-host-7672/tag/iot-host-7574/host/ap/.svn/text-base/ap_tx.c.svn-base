#define __SFILE__ "ap_tx.c"


#include <config.h>

#include "common/ieee80211.h"
#include "common/ether.h"
#include "ssv_timer.h"

#include "ap_info.h"
#include "ap_tx.h"
#include "ap_sta_info.h"
#include "common/ieee802_11_defs.h"

#include <host_apis.h>
#include <os_wrapper.h>
#include "dev.h"


extern u8* ssv6xxx_host_tx_req_get_qos_ptr(struct cfg_host_txreq0 *req0);
extern u8* ssv6xxx_host_tx_req_get_data_ptr(struct cfg_host_txreq0 *req0);



#ifdef __AP_DEBUG__


void ap_check_queue(struct ap_tx_desp_head *head)
{
	int i;
	struct ap_tx_desp *desp=(struct ap_tx_desp *)head;
	
	for(i=0;i<head->len;i++)
	{
		desp = desp->next;
	}


	if(desp->next!=(struct ap_tx_desp *)head)
		ASSERT(FALSE);


}

#endif//__AP_DEBUG__


void ap_print_queue_info(struct ap_tx_desp_head *head)
{
	int i;
	struct ap_tx_desp *desp=(struct ap_tx_desp *)head;
	
	
	for(i=0;i<head->len;i++)
	{
		desp = desp->next;
		LOG_PRINTF("Item[%d] Jiffies:%d ",i, desp->jiffies);		
	}
	LOG_PRINTF("\r\n");

	if(desp->next!=(struct ap_tx_desp *)head)
		ASSERT(FALSE);
}












/* 802.1d to AC mapping. Refer pg 57 of WMM-test-plan-v1.2 */
const u8 up_to_ac[] = {
	IEEE80211_AC_BE,
	IEEE80211_AC_BK,
	IEEE80211_AC_BK,
	IEEE80211_AC_BE,
	IEEE80211_AC_VI,
	IEEE80211_AC_VI,
	IEEE80211_AC_VO,
	IEEE80211_AC_VO,
};


u32 ap_tx_desp_queue_len(const struct ap_tx_desp_head *list_)
{
	return list_->len;
}

void ap_tx_desp_head_init(struct ap_tx_desp_head *list)
{
	list->prev = list->next = (struct ap_tx_desp *)list;
	list->len = 0;
}

struct ap_tx_desp *ap_tx_desp_peek(const struct ap_tx_desp_head *list_)
{
	struct ap_tx_desp *list = ((const struct ap_tx_desp *)list_)->next;
	if (list == (struct ap_tx_desp *)list_)
		list = NULL;
	return list;
}


struct ap_tx_desp *ap_tx_desp_dequeue(struct ap_tx_desp_head *list)
{
	
	struct ap_tx_desp *data = ((const struct ap_tx_desp *)list)->next;
	

#ifdef __AP_DEBUG__
	ap_check_queue(list);
#endif
	
	do 
	{		
		//Get data		
		if (data == (struct ap_tx_desp *)list)
		{
			data = NULL;
			break;
		}

		if(0 == list->len)
			ASSERT(FALSE);

		
		//Unlink
		{
			struct ap_tx_desp *next, *prev;
			list->len--;
			next	   = data->next;
			prev	   = data->prev;
			data->next  = data->prev = NULL;
			
			next->prev = prev;
			prev->next = next;
		}
	} while (0);

#ifdef __AP_DEBUG__
		ap_check_queue(list);
#endif



	
	return data;
}



static inline void ap_tx_desp_insert(struct ap_tx_desp *new,
struct ap_tx_desp *prev, struct ap_tx_desp *next,
struct ap_tx_desp_head *list)

{
	new->next = next;
	new->prev = prev;
	next->prev  = prev->next = new;
	list->len++;
}

void ap_tx_desp_queue_tail(struct ap_tx_desp_head *list, struct ap_tx_desp *new)
{
#ifdef __AP_DEBUG__
	ap_check_queue(list);
#endif

	ap_tx_desp_insert(new, (struct ap_tx_desp*)list->prev, (struct ap_tx_desp*)list, list);	

#ifdef __AP_DEBUG__
		ap_check_queue(list);
#endif
	
}








void ap_release_tx_desp(struct ap_tx_desp *tx_desp);

//--------------------------------------------------------------------------------------------------------
//Tx descriptor related function

void APTxDespInit()
{
	gDeviceInfo->APInfo->pApTxData = MALLOC(sizeof(struct ap_tx_desp)*AP_TOTAL_MAX_TX_DSP);
	ASSERT(gDeviceInfo->APInfo->pApTxData);
	MEMSET(gDeviceInfo->APInfo->pApTxData, 0, sizeof(struct ap_tx_desp)*AP_TOTAL_MAX_TX_DSP);
}


struct ap_tx_desp* ap_get_tx_desp()
{
	int i;

	for (i=0;i<AP_TOTAL_MAX_TX_DSP;i++)
	{
		struct ap_tx_desp *tx_desp = &gDeviceInfo->APInfo->pApTxData[i];
		if (NULL == tx_desp->host_txreq0)
		{
			return tx_desp;
		}
	}

	return NULL;
	
}

void ap_release_tx_desp(struct ap_tx_desp *tx_desp)
{	
	MEMSET(tx_desp,0, sizeof(struct ap_tx_desp));
}

void ap_release_tx_desp_and_frame(struct ap_tx_desp *tx_desp)
{
	os_frame_free(tx_desp->frame);
	ap_release_tx_desp(tx_desp);	
}



//-----------------------------------

/* This function is called whenever the AP is about to exceed the maximum limit
 * of buffered frames for power saving STAs. This situation should not really
 * happen often during normal operation, so dropping the oldest buffered packet
 * from each queue should be OK to make some room for new frames. */
static void purge_old_ps_buffers()
{
	int i, ac = 0;
	struct ap_tx_desp *tx_desp = NULL;
	

	tx_desp = ap_tx_desp_dequeue(&gDeviceInfo->APInfo->ps_bc_buf);

	if (tx_desp)
	{
		ap_release_tx_desp_and_frame(tx_desp);
		gDeviceInfo->APInfo->total_ps_buffered--;
	}
	
	
	for(i=0;i<AP_MAX_STA;i++ )
	{
		APStaInfo_st *pAPStaInfo = &gDeviceInfo->StaConInfo[i];
		if( !test_sta_flag(pAPStaInfo, WLAN_STA_VALID) )
			continue;
					
	/*
	 * Drop one frame from each station from the lowest-priority
	 * AC that has frames at all.
	 */
		for (ac = IEEE80211_AC_BK; ac >= IEEE80211_AC_VO; ac--) {
			tx_desp = ap_tx_desp_dequeue(&pAPStaInfo->ps_tx_buf[ac]);
			if (tx_desp) {
				ap_release_tx_desp_and_frame(tx_desp);				
				gDeviceInfo->APInfo->total_ps_buffered--;
				break;
			}
		}
	}
	
}

void purge_all_ps_buffers(APStaInfo_st *pAPStaInfo)
{
	int  ac = 0;
	struct ap_tx_desp *tx_desp = NULL;
					
	/*
	 * Drop frames from each station
	 */
	for (ac = IEEE80211_AC_BK; ac >= IEEE80211_AC_VO; ac--) 
	{		
		while( tx_desp = ap_tx_desp_dequeue(&pAPStaInfo->ps_tx_buf[ac]) )
		{
			ap_release_tx_desp_and_frame(tx_desp);				
			gDeviceInfo->APInfo->total_ps_buffered--;			
		}		
	}	
}



void purge_all_bc_buffers()
{	
	struct ap_tx_desp *tx_desp = NULL;
					
	/*
	 * Drop All BC frames 
	 */	
	while( tx_desp = ap_tx_desp_dequeue(&gDeviceInfo->APInfo->ps_bc_buf) )
	{
		ap_release_tx_desp_and_frame(tx_desp);				
		gDeviceInfo->APInfo->total_ps_buffered--;			
	}		
		
}





//---------------------
extern void sta_info_cleanup(void *data1, void *data2);
extern void sta_info_recalc_tim(APStaInfo_st *sta);


tx_result tx_h_unicast_ps_buf(struct ap_tx_desp *tx_desp)
{
	APStaInfo_st *sta = tx_desp->sta;	

	/* only deauth, disassoc and action are bufferable MMPDUs */
 	if (!sta)
		return TX_CONTINUE;

		     

	 if (test_sta_flag(sta, WLAN_STA_PS_STA) && !(tx_desp->flags & AP_TX_NATIVE_AP_FRAME))
	 {
		 int ac = up_to_ac[tx_desp->priority];
 		  
		 if (gDeviceInfo->APInfo->total_ps_buffered >= AP_TOTAL_MAX_TX_BUFFER)
		 	purge_old_ps_buffers();

		if (ap_tx_desp_queue_len(&sta->ps_tx_buf[ac]) >= AP_STA_MAX_TX_BUFFER) {
			struct ap_tx_desp *old = ap_tx_desp_dequeue(&sta->ps_tx_buf[ac]);			
			ap_release_tx_desp_and_frame(old);
		} else
			gDeviceInfo->APInfo->total_ps_buffered++;

		tx_desp->jiffies= os_sys_jiffies();
		
		
		ap_tx_desp_queue_tail(&sta->ps_tx_buf[ac], tx_desp);

		os_create_timer(AP_STA_INFO_CLEANUP_INTERVAL, sta_info_cleanup, NULL, NULL, (void*)MBOX_HCMD_ENGINE);

		/*
		* We queued up some frames, so the TIM bit might
		* need to be set, recalculate it.
		*/
		sta_info_recalc_tim(sta);

        #ifdef __AP_DEBUG__ 

		LOG_TRACE("STA %x:%x:%x:%x:%x:%x in PS mode, Queue Frame in AC[%d]\r\n",
			sta->addr[0],sta->addr[1],sta->addr[2],sta->addr[3],sta->addr[4],sta->addr[5], ac);
		#endif

		return TX_QUEUED;
	}
	else if ((test_sta_flag(sta, WLAN_STA_PS_STA))) {
        #ifdef __AP_DEBUG__ 
		LOG_TRACE("STA %x:%x:%x:%x:%x:%x in PS mode, but polling/in SP -> send frame\r\n",
			sta->addr[0],sta->addr[1],sta->addr[2],sta->addr[3],sta->addr[4],sta->addr[5]);
        #endif
	}
	else
	{}


	return TX_CONTINUE;	
}

tx_result tx_h_multicast_ps_buf(struct ap_tx_desp *tx_desp)
{
#if 1
    //Skip multicast packet, send it
    return TX_CONTINUE;

#else

    //if any one station is in ps, need to queue it.	
	if (0 == gDeviceInfo->APInfo->num_sta_ps)
		return TX_CONTINUE;
	
	if (gDeviceInfo->APInfo->total_ps_buffered >= AP_TOTAL_MAX_TX_BUFFER)
 		purge_old_ps_buffers();

	if (ap_tx_desp_queue_len(&gDeviceInfo->APInfo->ps_bc_buf) >= AP_MAX_BC_BUFFER) 
		ap_release_tx_desp_and_frame(ap_tx_desp_dequeue(&gDeviceInfo->APInfo->ps_bc_buf));	
	else
		gDeviceInfo->APInfo->total_ps_buffered++;
	
	ap_tx_desp_queue_tail(&gDeviceInfo->APInfo->ps_bc_buf, tx_desp);

	return TX_QUEUED;	
#endif
}



tx_result ssv6xxx_data_need_queue(struct ap_tx_desp *tx_desp)
{	
	if (tx_desp->flags & AP_TX_POLL_RESPONSE || tx_desp->flags & AP_TX_NATIVE_AP_FRAME)
		return TX_CONTINUE;
	
	if (tx_desp->flags & AP_TX_UNICAST)
		return tx_h_unicast_ps_buf(tx_desp);
	else
		return tx_h_multicast_ps_buf(tx_desp);

	return TX_CONTINUE;	
}


tx_result ssv6xxx_data_check_assoc(struct ap_tx_desp *tx_desp)
{
	//ieee80211_tx_result ret = TX_CONTINUE;
	bool bAssoc = false;
	
	if (tx_desp->sta)
			bAssoc = test_sta_flag(tx_desp->sta, WLAN_STA_ASSOC);


	if (tx_desp->flags & AP_TX_UNICAST)
	{
		//Unicast
		if (!bAssoc && (tx_desp->flags & AP_TX_DATA)) 		
		    return TX_DROP;		
	} 
	else if (
	(tx_desp->flags & AP_TX_DATA) && 
			    !gDeviceInfo->APInfo->num_sta_authorized) 
	{		

		/*
		 * No associated STAs - no need to send multicast
		 * frames.
		 */

		LOG_TRACE("%s No Authorized Sta. Drop multicat frame.\r\n",__func__);
		return TX_DROP;

	}

	return TX_CONTINUE;
}

void ssv6xxx_data_prepare_tx_data(struct ap_tx_desp *tx_desp, u32 *frame, struct cfg_host_txreq0 *host_txreq0, bool bAPFrame, u32 TxFlags)
{
	//int nHeaderLen;
//	u8 *dat = (u8*)host_txreq0;	
	ETHER_ADDR *pDaMAC = NULL; 
	APStaInfo_st *pStaInfo = NULL;

	MEMSET(tx_desp, 0, sizeof(struct ap_tx_desp *));


	tx_desp->flags |= TxFlags;


	if(bAPFrame)
		tx_desp->flags |= AP_TX_NATIVE_AP_FRAME;


	if(host_txreq0->RSVD0 & AP_PS_FRAME)
		tx_desp->flags |= AP_TX_POLL_RESPONSE;
	


	tx_desp->frame = frame;
	tx_desp->host_txreq0 = host_txreq0;
	tx_desp->data = ssv6xxx_host_tx_req_get_data_ptr(host_txreq0);
	tx_desp->nDataLen = host_txreq0->len-(tx_desp->data-(u8*)host_txreq0);
	

	if(1 == host_txreq0->f80211)	
	{
		//80211 frame
		struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)tx_desp->data;
		pDaMAC = (ETHER_ADDR *) (ieee80211_get_DA((struct ieee80211_hdr_4addr *)tx_desp->data));
		(ieee80211_is_data(hdr->frame_control)) ? (tx_desp->flags |= AP_TX_DATA):(tx_desp->flags &= ~AP_TX_DATA);
		
	}
	else
	{
		//ether frame
		ethhdr *pDaEthHdr =  (ethhdr *)tx_desp->data;
		pDaMAC = (ETHER_ADDR *)&pDaEthHdr->h_dest;		
		tx_desp->flags |= AP_TX_DATA;
	}
		
	pStaInfo = APStaInfo_FindStaByAddr(pDaMAC); 	
	tx_desp->sta = pStaInfo;


	if (is_multicast_ether_addr((const u8*)pDaMAC)) {
		tx_desp->flags &= ~AP_TX_UNICAST;		
	} else
		tx_desp->flags |= AP_TX_UNICAST;

	
	{
		u16 *qoshdr = (u16*)ssv6xxx_host_tx_req_get_qos_ptr(host_txreq0);

		if(qoshdr)
			tx_desp->priority = (*qoshdr&IEEE80211_QOS_CTL_TAG1D_MASK);
		else
			tx_desp->priority = 0;
	}

	

	
	
}



tx_result ssv6xxx_data_could_be_send(void *frame, bool bAPFrame, u32 TxFlags)
{
	//bool bRet = FALSE;
	tx_result res = TX_DROP;
	struct cfg_host_txreq0 *host_txreq0 = (struct cfg_host_txreq0 *)OS_FRAME_GET_DATA(frame);	
	struct ap_tx_desp *tx_desp = NULL;

	#define CALL_TXH(txh) \
		res = txh(tx_desp);		\
		if (res != TX_CONTINUE)	\
			break

		


	do 
	{

		tx_desp = ap_get_tx_desp();
		ASSERT(NULL != tx_desp);

		ssv6xxx_data_prepare_tx_data(tx_desp, frame, host_txreq0, bAPFrame, TxFlags);

		CALL_TXH(ssv6xxx_data_check_assoc);
		CALL_TXH(ssv6xxx_data_need_queue);
		
	} while (0);

	if (TX_DROP == res || TX_CONTINUE == res)				
		ap_release_tx_desp(tx_desp);//Just need memory set description, frame will be sent or drop by parent function.
	else
	{;}


	return res;
}




struct ap_tx_desp *
ap_get_buffered_bc(ApInfo_st *pAPInfo)
{

	struct ap_tx_desp *tx_desp = NULL;
	//struct cfg_host_txreq0 *txreq0 = NULL; 
	//sdata = vif_to_sdata(vif);
	//bss = &sdata->u.ap;

	//rcu_read_lock();
	//beacon = rcu_dereference(bss->beacon);

	//if (sdata->vif.type != NL80211_IFTYPE_AP || !beacon || !beacon->head)
	//	goto out;

	if (pAPInfo->dtim_count != 0 || !pAPInfo->dtim_bc_mc)
		goto out; /* send buffered bc/mc only after DTIM beacon */

	//while (1) 
	{
		tx_desp = ap_tx_desp_dequeue(&pAPInfo->ps_bc_buf);
		if (!tx_desp)
			goto out;
		pAPInfo->total_ps_buffered--;

		if (!ap_tx_desp_dequeue(&pAPInfo->ps_bc_buf) && tx_desp->nDataLen >= 2) {
			struct ieee80211_hdr *hdr =
				(struct ieee80211_hdr *) tx_desp->data;
			/* more buffered multicast/broadcast frames ==> set
			 * MoreData flag in IEEE 802.11 header to inform PS
			 * STAs */
			hdr->frame_control |=
				cpu_to_le16(IEEE80211_FCTL_MOREDATA);
		}

		//if (!ieee80211_tx_prepare(sdata, &tx, skb))
		//	break;
		//dev_kfree_skb_any(skb);
	}

	//info = IEEE80211_SKB_CB(skb);

	//tx.flags |= IEEE80211_TX_PS_BUFFERED;
	//tx.channel = local->hw.conf.channel;
	//info->band = tx.channel->band;

	//if (invoke_tx_handlers(&tx))
	//	skb = NULL;

	//txreq0 = tx_desp->host_txreq0;
	//-------------------------------------------
	//tx_desp->data = NULL;
	//ap_release_tx_desp(tx_desp);
	
 out:

	return tx_desp;
}

//TO DO-------------------------------------------------------------------------

#pragma message("===================================================")
#pragma message("Todo Need to discuss flow....")
#pragma message("===================================================")



#if 0
void ap_tx_status(ApInfo_st *pAPInfo, const u8 *addr,
		      /* const u8 *buf, size_t len, */int ack)
{
	APStaInfo  *sta;


	sta=STAInfo_FindSTAByAddr((ETHER_ADDR *)addr);
	
	if (sta == NULL || !test_sta_flag(sta, WLAN_STA_ASSOC))
		return;
	if (sta->flags & WLAN_STA_PENDING_POLL) {
		LOG_DEBUG("STA " MACSTR " %s pending "
			   "activity poll", MAC2STR(sta->addr),
			   ack ? "ACKed" : "did not ACK");
		if (ack)
			sta->flags &= ~WLAN_STA_PENDING_POLL;
	}

//	ieee802_1x_tx_status(hapd, sta, buf, len, ack);
}

#endif


extern s32 ssv6xxx_drv_send(void *dat, size_t len);
void ap_tx_dtimexpiry_event()
{	
	
	{	
		struct ap_tx_desp *tx_desp = NULL; 
		tx_desp = ap_get_buffered_bc(gDeviceInfo->APInfo);
	
		while (tx_desp) {
	
			//ssv6xxx_drv_send(tx_desp->host_txreq0, tx_desp->host_txreq0->len);
			ssv6xxx_drv_send(OS_FRAME_GET_DATA(tx_desp->frame), OS_FRAME_GET_DATA_LEN(tx_desp->frame));	//??
	
			//Todo:  need to check if queue have size to queue
	
			//ath5k_tx_queue(ah->hw, skb, ah->cabq);
			//if (ah->cabq->txq_len >= ah->cabq->txq_max)
			//	break;
	
			//tx_desp->data = NULL;
			ap_release_tx_desp(tx_desp);
	
			tx_desp = ap_get_buffered_bc(gDeviceInfo->APInfo);
		}
	}



	
}





