#include "lwip/debug.h"
#include "httpd.h"
#include "lwip/tcp.h"
#include <lwip/sockets.h>
#include <lwip/netif.h>
#include <lwip/ip_addr.h>
#include <lwip/dhcp.h>
#include <lwip/sys.h>
#include <host_global.h>
#include <netapp/net_app.h>

#include "http_fs.h"
#include <log.h>

#include <ssv_ex_lib.h>

#if HTTPD_SUPPORT

#define NUM_CONFIG_CGI_URIS	(sizeof(ppcURLs) / sizeof(tCGI))
#define NUM_CONFIG_SSI_TAGS	(sizeof(ppcTAGs) / sizeof(char *))
	
const char* LOGIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* WIFI_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);
const char* NETCFG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[]);

static const char *ppcTAGs[]=  
{
	"ipaddr",
	"wifimode", 
	"ssid",
	"encrypt",
};


static const tCGI ppcURLs[]=  
{
    {"/login.cgi",LOGIN_CGI_Handler},
    {"/wifimode.cgi",WIFI_CGI_Handler},
    {"/netcfg.cgi",NETCFG_CGI_Handler},
};

static int FindCGIParameter(const char *pcToFind,char *pcParam[],int iNumParams)
{
	int iLoop;
	for(iLoop = 0;iLoop < iNumParams;iLoop ++ )
	{
		if(strcmp(pcToFind,pcParam[iLoop]) == 0)
		{
			return (iLoop); 
		}
	}
	return (-1);
}

void IPaddr_Handler(char *pcInsert)
{
    struct netif *netif;
    
    netif = netif_find("wlan0");
    if (netif)    
    {
        sprintf(pcInsert, "%d.%d.%d.%d", IPV4_ADDR(&netif->ip_addr.addr));
    }
}

void WIFImode_Handler(char *pcInsert)
{
    strcpy(pcInsert, "Station");
}

void SSID_Handler(char *pcInsert)
{
    strcpy(pcInsert, "sssv");
}

void Encrypt_Handler(char *pcInsert)
{
    strcpy(pcInsert, "wpa2");
}

static u16_t SSIHandler(int iIndex,char *pcInsert,int iInsertLen)
{
    LOG_PRINTF("SSIHandler iIndex=%d\r\n", iIndex);
	switch(iIndex)
	{
        case 0: 
				IPaddr_Handler(pcInsert);       
				break;
		case 1:
				WIFImode_Handler(pcInsert);  
				break;
		case 2:
				SSID_Handler(pcInsert);    
				break;
		case 3:
				Encrypt_Handler(pcInsert);
				break;
	}
	return strlen(pcInsert);
}

const char* LOGIN_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{  
  LOG_PRINTF("LOGIN_CGI_Handler\r\n");
  
  iIndex = FindCGIParameter("username",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("username: %s\r\n", pcValue[iIndex]);
  }
  
  iIndex = FindCGIParameter("password",pcParam,iNumParams);
  if (iIndex != -1)
  {
      LOG_PRINTF("password: %s\r\n", pcValue[iIndex]);
  }
  
  return "/run_status.shtml";   
}

const char* WIFI_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

  LOG_PRINTF("WIFI_CGI_Handler\r\n");
  
  return "/run_status.shtml";   
}


const char* NETCFG_CGI_Handler(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{

  LOG_PRINTF("NETCFG_CGI_Handler\r\n");
  
  
  return "/run_status.shtml";   
}

void httpd_ssi_init(void)
{  
	http_set_ssi_handler(SSIHandler,ppcTAGs,NUM_CONFIG_SSI_TAGS);
}

void httpd_cgi_init(void)
{ 
     http_set_cgi_handlers(ppcURLs, NUM_CONFIG_CGI_URIS);
}
#endif /* HTTPD_SUPPORT */

