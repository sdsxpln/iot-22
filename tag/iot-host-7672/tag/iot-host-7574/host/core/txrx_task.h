#ifndef _TXRX_TASK_H
#define _TXRX_TASK_H

#include <host_apis.h>
#include <common.h>

s32 TXRX_Init();
s32 TXRX_FrameEnqueue(void* frame, u32 priority);
ssv6xxx_result TXRX_SetOpMode(ModeType mode);

#endif //_TXRX_TASK_H
