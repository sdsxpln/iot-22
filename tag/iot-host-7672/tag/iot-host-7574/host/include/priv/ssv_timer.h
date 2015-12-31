#ifndef _SSV_TIMER_H_
#define _SSV_TIMER_H_
#include "ssv_lib.h"

typedef void (*timer_handler)(void *, void *);

//Timer related
s32 os_create_timer(u32 ms, timer_handler handler, void *data1, void *data2, void* mbx);
s32 os_cancel_timer(timer_handler handler, u32 data1, u32 data2);


typedef struct os_timer
{
    struct list_q tmr_list;
    u32             nMsgType;
    //OsTimer         MTimer;
    timer_handler   handler;
    void*           infombx;
    u32             nMTData0;
    u32             nMTData1;
    u32             msTimeout;
    u32             msRemian;
} os_timer_st;

enum msgtype_tmr
{
    TMR_EVT_CREATE,
    TMR_EVT_CANCEL,
    TMR_EVT_FLUSH,
};

#endif
