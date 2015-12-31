#ifndef _NET_APP_H_
#define _NET_APP_H_


#define IPV4_ADDR(a)    ip4_addr1(a), ip4_addr2(a), ip4_addr3(a), ip4_addr4(a)

typedef void (*net_app_func)(s32, s8 **);


typedef struct net_app_info_st
{
    s8              *app_name;
    net_app_func    app_func;
    
} net_app_info;


/**
* The arguments for the net_app task. 
*/
typedef struct net_app_args_st 
{
    s8              *cmd_buf; /* to hold command line buffer */
    net_app_info    *app_info;
} net_app_args;



s32 net_app_init(void);
s32 net_app_run(s32 argc, s8 *argv[]);
s32 net_app_show(void);
void net_app_task(void *args);




#endif /* _NET_APP_H_ */

