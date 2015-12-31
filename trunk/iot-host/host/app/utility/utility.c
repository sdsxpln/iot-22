#include "rtos.h"
#include "log.h"
#include "utility.h"
#include "common.h"
#include <host_apis.h>
#include "udhcp/udhcp_common.h"
#include "udhcp/dhcpd.h"

extern H_APIs s32 ssv6xxx_get_rssi_by_mac(u8 *macaddr);

typedef struct dhcpdipmac_st
{
    u32 ip;
    u8 mac[6];
}dhcpd_ipmac;

int get_ipmac_pair_in_dhcp(dhcpd_ipmac *ipmac, int *size_count)
{
    int i = 0;
    struct dyn_lease *lease = NULL;
    int ret = 0;

    if (!ipmac || !size_count || (*size_count <= 0))
    {
        return -1;
    }

    lease = (struct dyn_lease *)MALLOC(sizeof(struct dyn_lease) * (*size_count));
    if (!lease)
    {
        return -1;
    }

    ret = dhcpd_lease_get_api(lease, size_count);

    if (ret == 0)
    {
        for (i = 0; i < *size_count; i++)
        {
           ((dhcpd_ipmac *) (ipmac + i))->ip = lease[i].lease_nip;
            MEMCPY((void*)(((dhcpd_ipmac *) (ipmac + i))->mac), (void*)(lease[i].lease_mac), 6);
        }
    }

    FREE((void*)lease);

    return ret;
}

int get_mac_by_ipv4(u8 *mac, u32 *ipaddr)
{
    dhcpd_ipmac *ipmac = NULL;
    int size_count = DEFAULT_DHCP_MAX_LEASES;
    int i;
    u32 nip;
    
    ipmac = (dhcpd_ipmac *)MALLOC(DEFAULT_DHCP_MAX_LEASES * sizeof(dhcpd_ipmac));
    if (ipmac == NULL)
    {
        return -1;
    }
    MEMSET((void *)ipmac, 0, DEFAULT_DHCP_MAX_LEASES * sizeof(dhcpd_ipmac));

    if (get_ipmac_pair_in_dhcp(ipmac, &size_count)){
        LOG_PRINTF("get_ipmac_pair_in_dhcp return failure \r\n");
        FREE(ipmac);
        return -1;
    }
    
    udhcp_str2nip((const char *)ipaddr, (void *)&nip);
    for (i=0; i<size_count; i++){
        if (!MEMCMP(&(ipmac[i].ip), &nip, 4)){

            MEMCPY(mac,ipmac[i].mac,ETH_ALEN);
            FREE(ipmac);
            return 0;
        }
    }

    LOG_PRINTF("get_ipmac_pair_in_dhcp get ipaddr failure\r\n");
    FREE(ipmac);
    return -1;
}


s32 get_rssi_by_ip(void *ipaddr)
{
    u8 dest_mac[ETH_ALEN]={0}; 

    if(!ipaddr)
    {
        return -1;
    }
    //step 1 : use IP to get MAC address 
    get_mac_by_ipv4(dest_mac,(u32 *)ipaddr);
    
    //step 2 : get RSSI by MAC address
    if(is_valid_ether_addr(dest_mac))
    {
        s8 ret_rssi = ssv6xxx_get_rssi_by_mac(dest_mac);
        return ret_rssi;
    }
    else
    {
        return -1;
    }
}





