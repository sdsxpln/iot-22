#include <host_cmd_engine.h>
#include <host_global.h>
#include <os_wrapper.h>
#include <log.h>
#include "cli_cmd_sys.h"

char CmdEngModeString[3][8] = {"STOP", "RUNNING", "EXIT"};
#ifdef __SSV_UNIX_SIM__
char SysProfBuf[256];
s8 SysProfSt = -1;
#endif
void usage(void)
{
    LOG_PRINTF("Usage:\r\n");
    LOG_PRINTF("      sys cmdeng st\r\n");
    LOG_PRINTF("      sys soc st\r\n");
#ifdef __SSV_UNIX_SIM__
    LOG_PRINTF("      sys profile\r\n");
#endif
}

static void GetSocStatus(void)
{
	 ssv6xxx_wifi_ioctl(SSV6XXX_HOST_CMD_GET_SOC_STATUS, NULL, 0);
}


void cmd_sys(s32 argc, s8 *argv[])
{
    if ( (strcmp(argv[1], "cmdeng") == 0) && (argc == 3))
    {
        if(strcmp(argv[2], "st") == 0)
        {
            struct CmdEng_st st;
            MEMSET(&st, 0, sizeof(struct CmdEng_st));
            
            CmdEng_GetStatus(&st);
            LOG_PRINTF("Mode:%s , BlockCmd:%s, %d Cmds In pendingQ\r\n", CmdEngModeString[st.mode], (st.BlkCmdIn == true)?"YES":"NO", st.BlkCmdNum);            
        }
        else if(strcmp(argv[2], "dbg") == 0)
        {
            bool dbgst = CmdEng_DbgSwitch();
            LOG_PRINTF("CmdEng debug %s\r\n", (dbgst != false)?"on":"off");
        }
        else
            LOG_PRINTF("Invalid Command\r\n");
    }
    else if ( (strcmp(argv[1], "soc") == 0) && (argc == 3))
    {
        if(strcmp(argv[2], "st") == 0)
        {
            GetSocStatus();
        }
    }
#ifdef __SSV_UNIX_SIM__
    else if ((strcmp(argv[1],"profile") == 0) && (argc == 2))
    {
	s32 ret = OS_SysProfiling(&SysProfBuf);
	if(ret == OS_SUCCESS)
	{
            SysProfSt = 1;
            LOG_PRINTF("Enable system profiling\n");
	}
        else
            LOG_PRINTF("Fail to enable system profiling\n");
    }
#endif
    else
    {
        LOG_PRINTF("Invalid Command\r\n");
        usage();
    }   
}
