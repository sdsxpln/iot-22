#ifndef _MLME_TASK_H
#define _MLME_TASK_H

#include <host_apis.h>

#define MBOX_MLME_TASK       g_mlme_task_info[0].qevt
#define MAX_NUM_OF_AP       (20)

void mlme_task( void *args );
s32 mlme_init(void);
void mlme_sta_mode_init(void);
void mlme_sta_mode_deinit(void);

#endif //_MLME_TASK_H
