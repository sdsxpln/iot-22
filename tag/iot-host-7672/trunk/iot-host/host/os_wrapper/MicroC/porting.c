#include "config.h"
#include "log.h"
#include "host_global.h"
#include "rtos.h"
#include "porting.h"

u8 hal_getchar(void)
{
    u8 data=0;
#if (GP_PLATFORM_SEL == GP_19B)
    if(STATUS_OK == uart0_data_get(&data, 0))
#elif (GP_PLATFORM_SEL == GP_15B)
    if(STATUS_OK == drv_l1_uart1_data_get(&data, 0))
#endif
        return data;
    else
        return 0;
}

