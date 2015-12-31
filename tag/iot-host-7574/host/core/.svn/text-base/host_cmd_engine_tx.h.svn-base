#ifndef _HOST_CMD_ENGINE_TX_H_
#define _HOST_CMD_ENGINE_TX_H_


typedef s32 (*HCmdEng_TxIoctlHandler)(HostCmdEngInfo_st *pHCmdEngInfo, struct cfg_host_cmd *pPktInfo);
typedef s32 (*HCmdEng_TxCmdHandler)(struct cfg_host_cmd *hCmd);





/**
* struct HCmdEng_TrapHandler_st - To trap Host Command for processing.
*
*/
typedef struct HCmdEng_TrapHandler_st {
    u32                     hCmdID;
    HCmdEng_TxCmdHandler    hTrapHandler;

} HCmdEng_TrapHandler;




#endif /* _HOST_CMD_ENGINE_TX_H_ */

