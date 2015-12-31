#include <config.h>
#include <regs/ssv6200_reg.h>
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <lwip/dhcp.h>
#include <lwip/sys.h>
#include <host_global.h>
#include <netapp/net_app.h>
#include <drv/ssv_drv.h>
#include "net_mgr.h"

#include <log.h>
#include <rtos.h>

#define DHCPC_RETRY_TIMES   15

static struct netif wlan0;

static bool s_dhcpd_enable = false;
static bool s_dhcpc_enable = false;
static void netmgr_net_init(char hwmac[6]);

extern err_t tcpip_input(struct pbuf *p, struct netif *inp);
extern err_t ethernetif_init(struct netif *netif);




//void sim_set_link_status(bool on)
void set_l2_link(bool on)
{
	LOG_PRINTF("L2 Link %s\r\n",(on==true?"ON":"OFF"));
    if (on)
        netif_set_link_up(&wlan0);
    else 
        netif_set_link_down(&wlan0);
}




void netmgr_init()
{
    u32 STA_MAC_0;
    u32 STA_MAC_1;
    char mac[6];
    
    STA_MAC_0 = ssv6xxx_drv_read_reg(ADR_STA_MAC_0); 
    STA_MAC_1 = ssv6xxx_drv_read_reg(ADR_STA_MAC_1);

    OS_MemCPY(mac,(u8 *)&STA_MAC_0,4);
    OS_MemCPY((mac + 4),(u8 *)&STA_MAC_1,2);
    
    netmgr_net_init(mac);   
}

static void netmgr_net_init(char hwmac[6])
{
	struct ip_addr ipaddr, netmask, gw;    
    struct netif * pwlan = NULL;
    
    /* net if init */
    pwlan = netif_find(WLAN_IFNAME);
    if (pwlan)
    {
        netif_remove(pwlan);
    }
    
    OS_MemCPY(wlan0.hwaddr, hwmac, 6);
    OS_MemCPY(wlan0.name,WLAN_IFNAME, 6);

    ipaddr.addr = ipaddr_addr(DEFAULT_IPADDR);
    netmask.addr = ipaddr_addr(DEFAULT_SUBNET);
    gw.addr = ipaddr_addr(DEFAULT_GATEWAY);
    
    netif_add(&wlan0, &ipaddr, &netmask, &gw, NULL, ethernetif_init, tcpip_input);
    netif_set_default(&wlan0);
    netif_set_up(&wlan0);

    netmgr_dhcpd_default();
    
    LOG_PRINTF("MAC[%02x:%02x:%02x:%02x:%02x:%02x]\n", 
        wlan0.hwaddr[0], wlan0.hwaddr[1], wlan0.hwaddr[2],
        wlan0.hwaddr[3], wlan0.hwaddr[4], wlan0.hwaddr[5]);
}

int netmgr_ipinfo_get(char *ifname, ipinfo *info)
{
    struct netif * pwlan = NULL;
    
    pwlan = netif_find(ifname);
    if (!pwlan)
    {
        LOG_PRINTF("%s is not exist\n", ifname);
        return -1;
    }

    info->ipv4 = pwlan->ip_addr.addr;
    info->netmask = pwlan->netmask.addr;
    info->gateway = pwlan->gw.addr;
    info->dns = 0;
    return 0;
}

int netmgr_ipinfo_set(char *ifname, ipinfo *info)
{
    struct netif * pwlan = NULL;
    ip_addr_t ipaddr, netmask, gw;
    
    pwlan = netif_find(ifname);
    if (!pwlan)
    {
        LOG_PRINTF("%s is not exist\n", ifname);
        return -1;
    }
    
    ipaddr.addr = info->ipv4;
    netmask.addr = info->netmask;
    gw.addr = info->gateway;

    netif_set_ipaddr(pwlan, &ipaddr);
    netif_set_netmask(pwlan, &netmask);
    netif_set_gw(pwlan, &gw);

    return 0;
}

int netmgr_hwmac_get(char *ifname, char mac[6])
{
    struct netif * pwlan = NULL;
    
    pwlan = netif_find(ifname);
    if (!pwlan)
    {
        LOG_PRINTF("%s is not exist\n", ifname);
        return -1;
    }

    OS_MemCPY(mac, pwlan->hwaddr, 6);
    
    return 0;
}

int netmgr_dhcpd_default()
{
    dhcps_info if_dhcps;
    
    /* dhcps info init */
    if_dhcps.start_ip = ipaddr_addr(DEFAULT_DHCP_START_IP);
    if_dhcps.end_ip = ipaddr_addr(DEFAULT_DHCP_END_IP);
    if_dhcps.max_leases = DEFAULT_DHCP_MAX_LEASES;
    
    if_dhcps.subnet = ipaddr_addr(DEFAULT_SUBNET);
    if_dhcps.gw = ipaddr_addr(DEFAULT_GATEWAY);
    if_dhcps.dns = ipaddr_addr(DEFAULT_DNS);
    
    if_dhcps.auto_time = DEFAULT_DHCP_AUTO_TIME;
    if_dhcps.decline_time = DEFAULT_DHCP_DECLINE_TIME;
    if_dhcps.conflict_time = DEFAULT_DHCP_CONFLICT_TIME;
    if_dhcps.offer_time = DEFAULT_DHCP_OFFER_TIME;
    if_dhcps.min_lease_sec = DEFAULT_DHCP_MIN_LEASE_SEC;
    
    if (dhcps_set_info_api(&if_dhcps) != 0)
    {
        LOG_PRINTF("dhcps_set_info faild\n");
        return -1;
    }

    return 0;
}

int netmgr_dhcpd_set(bool enable)
{
    int ret = 0;
    if (enable)
    {
        ret = udhcpd_start();
        
        if (ret == 0)
        {
            s_dhcpd_enable = true;
        }
    }
    else
    {
        ret = udhcpd_stop();
        if (ret == 0)
        {
            s_dhcpd_enable = false;
        }
    }
    return ret;
}

int netmgr_dhcpc_set(bool enable)
{
    struct ip_addr ipaddr, netmask, gw;    
    struct ip_addr ipaddr_old, netmask_old, gw_old;    
    struct netif *pwlan = NULL;
	s32 mscnt  = 0;
    s32 retry_count = DHCPC_RETRY_TIMES;
    
    OS_MemSET(&ipaddr, 0, sizeof(ipaddr));
    OS_MemSET(&netmask, 0, sizeof(netmask));
    OS_MemSET(&gw, 0, sizeof(gw));
    
    OS_MemSET(&ipaddr_old, 0, sizeof(ipaddr_old));
    OS_MemSET(&netmask_old, 0, sizeof(netmask_old));
    OS_MemSET(&gw_old, 0, sizeof(gw_old));
    
    pwlan = netif_find(WLAN_IFNAME);
    if (!pwlan)
    {
        LOG_PRINTF("%s error\n", WLAN_IFNAME);
        return -1;
    }

    if (enable && !s_dhcpc_enable)
    {
        ipaddr_old.addr = pwlan->ip_addr.addr;
        netmask_old.addr = pwlan->netmask.addr;
        gw_old.addr = pwlan->gw.addr;
        
    	netif_set_addr(pwlan, &ipaddr, &netmask, &gw);
    	dhcp_start(pwlan);

    	while (pwlan->ip_addr.addr==0 && retry_count) 
        {
            LOG_PRINTF("Waiting for IP ready. %d\r\n", retry_count--);
    		sys_msleep(DHCP_FINE_TIMER_MSECS);
    		dhcp_fine_tmr();
    		mscnt += DHCP_FINE_TIMER_MSECS;
    		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
    			dhcp_coarse_tmr();
    			mscnt = 0;
    		}
    	}
        
    	if (pwlan->ip_addr.addr==0)
    	{  
            dhcp_stop(pwlan);
            s_dhcpc_enable = 0;
            netif_set_addr(pwlan, &ipaddr_old, &netmask_old, &gw_old);
            return -1;        
    	}
        
        s_dhcpc_enable = 1;
    }
    else if (!enable && s_dhcpc_enable)
    {
        dhcp_stop(pwlan);
        
        ipaddr_old.addr = ipaddr_addr(DEFAULT_IPADDR);
        netmask_old.addr = ipaddr_addr(DEFAULT_SUBNET);
        gw_old.addr = ipaddr_addr(DEFAULT_GATEWAY);

        netif_set_addr(pwlan, &ipaddr_old, &netmask_old, &gw_old);
        s_dhcpc_enable = 0;
    }
    return 0;
}

int netmgr_dhcp_status_get(bool *dhcpc_enable, bool *dhcpd_enable)
{
    if (!dhcpc_enable || !dhcpd_enable)
    {
        return -1;
    }
    
    *dhcpc_enable = s_dhcpc_enable;
    *dhcpd_enable = s_dhcpd_enable;

    return 0;
}

int netmgr_dhcps_info_set(dhcps_info *if_dhcps)
{
    if (dhcps_set_info_api(if_dhcps) < 0)
    {
        return -1;
    }

    return 0;
}

int netmgr_wifi_mode_set(wifi_mode *mode, wifi_ap_cfg *ap_cfg, wifi_sta_cfg *sta_cfg)
{    
    if (!mode || (*mode >= SSV6XXX_HWM_INVALID))
    {
        return -1;
    }

    if (*mode == SSV6XXX_HWM_STA)
    {
        if (sta_cfg)
        {
            ssv6xxx_wifi_station(sta_cfg);
        }
    }
    else if(*mode == SSV6XXX_HWM_AP)
    {
        if (ap_cfg)
        {
            ssv6xxx_wifi_ap(ap_cfg);
        }
    }
    else
    {
        // not support
    }
    
    return 0;
}

int netmgr_wifi_mode_get(wifi_mode *mode, bool *status)
{
    Ap_sta_status info;
    
    if (!mode || !status)
    {
        return -1;
    }

    OS_MemSET(&info, 0, sizeof(Ap_sta_status));
    
    ssv6xxx_wifi_status(&info);

    *mode = info.operate;
    *status = info.status ? true : false;

    #if NET_MGR_DEBUG
    if(info.status)
        LOG_PRINTF("status:ON\n");
    else
        LOG_PRINTF("status:OFF\n");
    if(SSV6XXX_HWM_STA==info.operate)
    {
        LOG_PRINTF("Mode:Station\n");
    }
    else
    {
        LOG_PRINTF("Mode:AP\n");
        LOG_PRINTF("SSID:%s\n",info.ap.ssid.ssid);
        LOG_PRINTF("Station number:%d\n",info.ap.stanum);
    }

    LOG_PRINTF("Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        info.ap.selfmac[0],
        info.ap.selfmac[1],
        info.ap.selfmac[2],
        info.ap.selfmac[3],
        info.ap.selfmac[4],
        info.ap.selfmac[5]);
    
    #endif
    
    return 0;
}

int netmgr_wifi_info_get(Ap_sta_status *info)
{
    if (!info)
    {
        return -1;
    }

    OS_MemSET(info, 0, sizeof(Ap_sta_status));
    
    ssv6xxx_wifi_status(info);

#if NET_MGR_DEBUG
    if(info->status)
        LOG_PRINTF("status:ON\n");
    else
        LOG_PRINTF("status:OFF\n");
    if(SSV6XXX_HWM_STA==info->operate)
    {
        LOG_PRINTF("Mode:Station\n");
    }
    else
    {
        LOG_PRINTF("Mode:AP\n");
        LOG_PRINTF("SSID:%s\n",info->ap.ssid.ssid);
        LOG_PRINTF("Station number:%d\n",info->ap.stanum);
    }

    LOG_PRINTF("Mac addr: %02x:%02x:%02x:%02x:%02x:%02x\n",
        info->ap.selfmac[0],
        info->ap.selfmac[1],
        info->ap.selfmac[2],
        info->ap.selfmac[3],
        info->ap.selfmac[4],
        info->ap.selfmac[5]);
    
#endif
    
    return 0;
}


int netmgr_wifi_scan()
{
    return 0;
}

int netmgr_wifi_join()
{
    return 0;
}

int netmgr_wifi_leave()
{
    return 0;
}

int netmgr_wifi_aplist_get()
{
    return 0;
}

int netmgr_wifi_event_cb()
{
    return 0;
}

