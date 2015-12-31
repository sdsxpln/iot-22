#define __SFILE__ "ssv_msg.c"

#include <config.h>
#include <msgevt.h>
#include "os_wrapper.h"
//#include "ap_info.h"
#include <log.h>
#include <pbuf.h>



#include <dev.h>

//--------------------------------------------------

void os_net_wake_queue()
{




}


void os_net_stop_queue()
{




}

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

#include <rtos.h>

//--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------



void os_msg_free(void *msg)
{
	msg_evt_free((MsgEvent *)msg);
}

void* os_msg_alloc()
{
	return msg_evt_alloc();
}


s32 os_msg_send(void* msg, void *pBuffer)
{
	MsgEvent *MsgEv = (MsgEvent *)msg;
	/* Fill MsgEvent Content: */
	MsgEv->MsgType  = MEVT_PKT_BUF;
	MsgEv->MsgData  = (u32)pBuffer;
	MsgEv->MsgData1 = 0;
	MsgEv->MsgData2 = 0;
	MsgEv->MsgData3 = 0;
	ASSERT(OS_SUCCESS == msg_evt_post(MBOX_CMD_ENGINE, MsgEv));
	
	return SSV6XXX_SUCCESS;
}




s32 os_msg_send_tx_drv(void* msg, void *pBuffer)
{
	MsgEvent *MsgEv = (MsgEvent *)msg;


#if CONFIG_USE_LWIP_PBUF
#if CONFIG_MEMP_DEBUG    
    PBUF_DBG_FLAGS(((struct pbuf *)pBuffer), PBUF_DBG_FLAG_L2_TX_DRIVER);
#endif
#endif

    /* Fill MsgEvent Content: */
	MsgEv->MsgType  = MEVT_PKT_BUF;
	MsgEv->MsgData  = (u32)pBuffer;
	MsgEv->MsgData1 = 0;
	MsgEv->MsgData2 = 0;
	MsgEv->MsgData3 = 0;
	ASSERT(OS_SUCCESS == msg_evt_post(MBOX_SIM_TX_DRIVER, MsgEv));
	
	return SSV6XXX_SUCCESS;
}







