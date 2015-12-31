#ifndef NET_MGR_H_
#define NET_MGR_H_

//#include "host_cmd_engine_priv.h"
#include <dev.h>
#include <host_apis.h>

#define WLAN_IFNAME "wlan0"

typedef struct st_ipinfo
{
    u32 ipv4;
    u32 netmask;
    u32 gateway;
    u32 dns;
}ipinfo;

typedef struct st_dhcps_info{
	/* start,end are in host order: we need to compare start <= ip <= end */
	u32 start_ip;              /* start address of leases, in host order */
	u32 end_ip;                /* end of leases, in host order */
	u32 max_leases;            /* maximum number of leases (including reserved addresses) */
    
    u32 subnet;
    u32 gw;
    u32 dns;
    
	u32 auto_time;             /* how long should udhcpd wait before writing a config file.
			                         * if this is zero, it will only write one on SIGUSR1 */
	u32 decline_time;          /* how long an address is reserved if a client returns a
			                         * decline message */
	u32 conflict_time;         /* how long an arp conflict offender is leased for */
	u32 offer_time;            /* how long an offered address is reserved */
	u32 max_lease_sec;         /* maximum lease time (host order) */
	u32 min_lease_sec;         /* minimum lease time a client can request */
}dhcps_info;

typedef ssv6xxx_hw_mode wifi_mode;
typedef Ap_setting wifi_ap_cfg;
typedef Sta_setting wifi_sta_cfg;
typedef Ap_sta_status wifi_info;

void netmgr_init();
int netmgr_ipinfo_get(char *ifname, ipinfo *info);
int netmgr_ipinfo_set(char *ifname, ipinfo *info);
int netmgr_hwmac_get(char *ifname, char mac[6]);
int netmgr_dhcpd_default();
int netmgr_dhcpd_set(bool enable);
int netmgr_dhcpc_set(bool enable);
int netmgr_dhcp_status_get(bool *dhcpc_enable, bool *dhcpd_enable);
int netmgr_dhcps_info_set(dhcps_info *if_dhcps);

#endif //NET_MGR_H_
