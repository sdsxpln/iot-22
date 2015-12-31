

typedef unsigned char               u8;

#ifdef __SSV_UNIX_SIM__

//==========

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>

int ssv6xxx_get_cust_mac(u8 *mac)
{
  struct ifreq s;
  int ret = 1;
  int fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);

  strcpy(s.ifr_name, "eth0");
  if (0 == ioctl(fd, SIOCGIFHWADDR, &s)) {
    int i;
    for (i = 2; i < 6; ++i)
        mac[i]=(u8)s.ifr_addr.sa_data[i];

    ret = 0;
  }
  //lint -e746 -e718
  close(fd);
  return ret;
  
}

#else//__SSV_UNIX_SIM__

int ssv6xxx_get_cust_mac(u8 *mac)
{
    return 0;
}
#endif//__SSV_UNIX_SIM__

//==========
