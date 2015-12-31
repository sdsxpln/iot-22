#include "config.h"
#include "log.h"
#include "host_global.h"
#include "rtos.h"
#include "porting.h"

u8 hal_getchar(void)
{
    u8 data=0;
    if(STATUS_OK == uart0_data_get(&data, 0))
        return data;
    else
        return 0;
}

