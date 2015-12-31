#ifndef _CONFIG_H_
#define _CONFIG_H_

// Common configuration should be applied to for both host simuator and device
// FW calculate checksum to check correctness of downloaded image.
#define ENABLE_FW_SELF_CHECK
#define FW_BLOCK_SIZE                   (1024)
#define FW_CHECKSUM_INIT                (0x12345678)
#define _INTERNAL_MCU_SUPPLICANT_SUPPORT_
// Use signle command response event instead of specific response event for each command.
// #define USE_CMD_RESP                    1

// Get TX queue statistics in EDCA handler.
#define ENABLE_TX_QUEUE_STATS

//#define THROUGHPUT_TEST
//HOST_SDIO_THROUGHPUT_TEST base on THROUGHPUT_TEST
//#define HOST_SDIO_THROUGHPUT_TEST

// Send log messages to host.
//#define ENABLE_LOG_TO_HOST


// ------------------------- config -------------------------------
#define CONFIG_HOST_PLATFORM         1
#include <host_config.h>

typedef enum {
    SSV6XXX_IQK_CFG_XTAL_26M = 0,
    SSV6XXX_IQK_CFG_XTAL_40M,
    SSV6XXX_IQK_CFG_XTAL_24M,
    SSV6XXX_IQK_CFG_XTAL_MAX,
} ssv6xxx_iqk_cfg_xtal;


//#define CONFIG_SSV_CABRIO_A			1
#define CONFIG_SSV_CABRIO_E			1
#define SSV6XXX_IQK_CFG_XTAL		SSV6XXX_IQK_CFG_XTAL_26M

#endif	/* _CONFIG_H_ */
