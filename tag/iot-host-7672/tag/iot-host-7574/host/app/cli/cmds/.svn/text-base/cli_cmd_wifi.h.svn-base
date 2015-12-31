#ifndef _CLI_CMD_WIFI_H_
#define _CLI_CMD_WIFI_H_

void cmd_iw(s32 argc, s8 *argv[]);
void cmd_ctl(s32 argc, s8 *argv[]);
void cmd_wifi_delba(s32 argc, s8 *argv[]);
void ssv6xxx_wifi_cfg(void);



#define GET_SEC_INDEX(start,end,c)         \
	while(start <end ) {                   \
		if(c & BIT(start)){                \
			break;					       \
		}                                  \
		start++;                           \
	}                                      \


#endif /* _CLI_CMD_WIFI_H_ */

