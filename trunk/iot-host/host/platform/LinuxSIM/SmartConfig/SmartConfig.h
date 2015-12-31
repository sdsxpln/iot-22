#ifndef _USER_SCONFIG_H_
#define _USER_SCONFIG_H_


int UserSconfigInit(void);
int UserSconfigSM(void);
void UserSconfigDeinit(void);
void UserSconfigPaserData(u8 channel, u8 *rx_buf, u32 len);
void UserSconfigConnect(void);

#endif /* _USER_SCONFIG_H_ */

