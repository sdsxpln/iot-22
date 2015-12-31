/*
 * iperf, Copyright (c) 2014, The Regents of the University of
 * California, through Lawrence Berkeley National Laboratory (subject
 * to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.
 *
 * If you have questions about your rights to use or distribute this
 * software, please contact Berkeley Lab's Technology Transfer
 * Department at TTD@lbl.gov.
 *
 * NOTICE.  This software is owned by the U.S. Department of Energy.
 * As such, the U.S. Government has been granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable,
 * worldwide license in the Software to reproduce, prepare derivative
 * works, and perform publicly and display publicly.  Beginning five
 * (5) years after the date permission to assert copyright is obtained
 * from the U.S. Department of Energy, and subject to any subsequent
 * five (5) year renewals, the U.S. Government is granted for itself
 * and others acting on its behalf a paid-up, nonexclusive,
 * irrevocable, worldwide license in the Software to reproduce,
 * prepare derivative works, distribute copies to the public, perform
 * publicly and display publicly, and to permit others to do so.
 *
 * This code is distributed under a BSD style license, see the LICENSE file
 * for complete information.
 */
#define _GNU_SOURCE
#define __USE_GNU

#include <config.h>
#include <ssv_lib.h>
#include "lwip/opt.h"
#include "common.h"

#include <os_wrapper.h>
#include <rtos.h>
#include <log.h>
#include "iperf_config.h"

/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <pthread.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif

#include <sys/time.h>
#include <sys/stat.h>
#include <sched.h>
#include <stdarg.h>
*/
#include <stdarg.h>
#include <string.h>
#include "lwip/ip_addr.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/tcp.h"

#include "net.h"
#include "iperf.h"
#include "iperf_api.h"
#include "iperf_udp.h"
#include "iperf_tcp.h"
#include "timer.h"

#include "cjson.h"
#include "units.h"
#include "iperf_util.h"
#include "iperf_locale.h"
#include "version.h"

/* Forwards. */
static int send_parameters(struct iperf_test *test);
static int get_parameters(struct iperf_test *test);
static int send_results(struct iperf_test *test);
static int get_results(struct iperf_test *test);
static int JSON_write(int fd, cJSON *json);
static void print_interval_results(struct iperf_test *test, struct iperf_stream *sp, cJSON *json_interval_streams);
static cJSON *JSON_read(int fd);

#ifdef IPERF_DEBUG_MEM
int iperf_mallocs = 0;
int iperf_frees = 0;
int iperf_msize = 0;
int iperf_max_mallocs = 0;
int iperf_max_msize = 0;
#endif

void *iperf_malloc(size_t size)
{
#ifdef IPERF_DEBUG_MEM
    u8 * ptr;
    ptr = (u8 *)MALLOC(size+4);
    if (ptr) {
        iperf_mallocs++;
        iperf_msize += size;
        *(int*)ptr = size;

        if (iperf_mallocs - iperf_frees > iperf_max_mallocs)
            iperf_max_mallocs = iperf_mallocs - iperf_frees;
        if (iperf_msize > iperf_max_msize)
            iperf_max_msize = iperf_msize;
        //PRINTF("iperf_malloc size: %u\r\n", size);
        ptr += 4;
    }    
    return (void*)ptr;
#else
    return MALLOC(size);
#endif    
}

void iperf_free(void *ptr)
{
#ifdef IPERF_DEBUG_MEM
    if (ptr){
        ptr = (u8 *)ptr - 4;
        iperf_frees++;
        iperf_msize -= *(int*)ptr;
        //PRINTF("iperf_free size: %u\n", *(int*)ptr);
        FREE(ptr);
    }
#else
    if (ptr)
        FREE(ptr);
#endif
}

/*************************** Print usage functions ****************************/

void
iperf_usage()
{
    PRINTF("Usage: iperf [-s|-c host] [options]\r\n");
    PRINTF("Try `iperf --help' for more information.\r\n");
}

void
usage_long()
{
    PRINTF("Usage: iperf [-s|-c host] [options]\r\n");
    PRINTF("       iperf [-h|--help] [-v|--version]\r\n\r\n");
    PRINTF("Server or Client:\r\n");
    PRINTF("  -p, --port      #         server port to listen on/connect to\r\n");
    PRINTF("  -i, --interval  #         seconds between periodic bandwidth reports\r\n");
    PRINTF("  -V, --verbose             more detailed output\r\n");
    PRINTF("  -d, --debug               emit debugging output\r\n");
    PRINTF("  -v, --version             show version information and quit\r\n");
    PRINTF("  -h, --help                show this message and quit\r\n");
    PRINTF("  stop                      kill the iperf threads \r\n");
    PRINTF("  list                      show the running iperf threads \r\n");
    PRINTF("Server specific:\r\n");
    PRINTF("  -s, --server              run in server mode\r\n");
    PRINTF("Client specific:\r\n");
    PRINTF("  -c, --client    <host>    run in client mode, connecting to <host>\r\n");
    PRINTF("  -u, --udp                 use UDP rather than TCP\r\n");
    PRINTF("  -b, --bandwidth #[KMG][/#] target bandwidth in bits/sec (0 for unlimited)\r\n");
    PRINTF("                            (default %d Mbit/sec for UDP, unlimited for TCP)\r\n", UDP_RATE / (1024*1024));
    PRINTF("                            (optional slash and packet count for burst mode)\r\n");
    PRINTF("  -t, --time      #         time in seconds to transmit for (default %d secs)\r\n", DURATION);
    PRINTF("  -l, --len       #[KMG]    length of buffer to read or write\r\n");
    PRINTF("                            (default %d KB for TCP, %d Bytes for UDP)\r\n", DEFAULT_TCP_BLKSIZE/1024, DEFAULT_UDP_BLKSIZE);
    PRINTF("  -R, --reverse             run in reverse mode (server sends, client receives)\r\n");
    PRINTF("  -N, --no-delay            set TCP no delay, disabling Nagle's Algorithm\r\n");
    PRINTF("  -P, --parallel  #         number of parallel client streams to run\r\n");
}


void warning(char *str)
{
    PRINTF("warning: %s\n", str);
}


/************** Getter routines for some fields inside iperf_test *************/

int
iperf_get_verbose(struct iperf_test *ipt)
{
    return ipt->verbose;
}

int
iperf_get_control_socket(struct iperf_test *ipt)
{
    return ipt->ctrl_sck;
}

int
iperf_get_test_omit(struct iperf_test *ipt)
{
    return ipt->omit;
}

int
iperf_get_test_duration(struct iperf_test *ipt)
{
    return ipt->duration;
}

int
iperf_get_test_burst(struct iperf_test *ipt)
{
    return ipt->settings->burst;
}

char
iperf_get_test_role(struct iperf_test *ipt)
{
    return ipt->role;
}

int
iperf_get_test_reverse(struct iperf_test *ipt)
{
    return ipt->reverse;
}

int
iperf_get_test_blksize(struct iperf_test *ipt)
{
    return ipt->settings->blksize;
}

int
iperf_get_test_socket_bufsize(struct iperf_test *ipt)
{
    return ipt->settings->socket_bufsize;
}

int
iperf_get_test_num_streams(struct iperf_test *ipt)
{
    return ipt->num_streams;
}

int
iperf_get_test_server_port(struct iperf_test *ipt)
{
    return ipt->server_port;
}

char*
iperf_get_test_server_hostname(struct iperf_test *ipt)
{
    return ipt->server_hostname;
}

int
iperf_get_test_protocol_id(struct iperf_test *ipt)
{
    return ipt->protocol->id;
}


int
iperf_get_test_zerocopy(struct iperf_test *ipt)
{
    return ipt->zerocopy;
}

int
iperf_get_test_get_server_output(struct iperf_test *ipt)
{
    return ipt->get_server_output;
}

char
iperf_get_test_unit_format(struct iperf_test *ipt)
{
    return ipt->settings->unit_format;
}

char *
iperf_get_test_bind_address(struct iperf_test *ipt)
{
    return ipt->bind_address;
}

/************** Setter routines for some fields inside iperf_test *************/

void
iperf_set_verbose(struct iperf_test *ipt, int verbose)
{
    ipt->verbose = verbose;
}

void
iperf_set_control_socket(struct iperf_test *ipt, int ctrl_sck)
{
    ipt->ctrl_sck = ctrl_sck;
}

void
iperf_set_test_omit(struct iperf_test *ipt, int omit)
{
    ipt->omit = omit;
}

void
iperf_set_test_duration(struct iperf_test *ipt, int duration)
{
    ipt->duration = duration;
}

void
iperf_set_test_state(struct iperf_test *ipt, signed char state)
{
    ipt->state = state;
}

void
iperf_set_test_blksize(struct iperf_test *ipt, int blksize)
{
    ipt->settings->blksize = blksize;
}


void
iperf_set_test_burst(struct iperf_test *ipt, int burst)
{
    ipt->settings->burst = burst;
}

void
iperf_set_test_server_port(struct iperf_test *ipt, int server_port)
{
    ipt->server_port = server_port;
}

void
iperf_set_test_socket_bufsize(struct iperf_test *ipt, int socket_bufsize)
{
    ipt->settings->socket_bufsize = socket_bufsize;
}

void
iperf_set_test_num_streams(struct iperf_test *ipt, int num_streams)
{
    ipt->num_streams = num_streams;
}

static void
check_sender_has_retransmits(struct iperf_test *ipt)
{
    if (ipt->sender && ipt->protocol->id == Ptcp && has_tcpinfo_retransmits())
    	ipt->sender_has_retransmits = 1;
    else
	    ipt->sender_has_retransmits = 0;
}

void
iperf_set_test_role(struct iperf_test *ipt, char role)
{
    ipt->role = role;
    if (role == 'c')
    	ipt->sender = 1;
    else if (role == 's')
	    ipt->sender = 0;
    if (ipt->reverse)
        ipt->sender = ! ipt->sender;
    check_sender_has_retransmits(ipt);
}

char * iperf_strdup (char * source)
{
    char *str_dup;
    
    if (source == NULL)
        return NULL;

    str_dup = iperf_malloc(ssv6xxx_strlen(source)+1);
    if (str_dup == NULL)
        return NULL;

    STRCPY(str_dup, source);
    return str_dup;
}

void
iperf_set_test_server_hostname(struct iperf_test *ipt, char *server_hostname)
{
    ipt->server_hostname = iperf_strdup(server_hostname);
}

void
iperf_set_test_reverse(struct iperf_test *ipt, int reverse)
{
    ipt->reverse = reverse;
    if (ipt->reverse)
        ipt->sender = ! ipt->sender;
    check_sender_has_retransmits(ipt);
}

void
iperf_set_test_get_server_output(struct iperf_test *ipt, int get_server_output)
{
    ipt->get_server_output = get_server_output;
}

void
iperf_set_test_unit_format(struct iperf_test *ipt, char unit_format)
{
    ipt->settings->unit_format = unit_format;
}

void
iperf_set_test_bind_address(struct iperf_test *ipt, char *bind_address)
{
    ipt->bind_address = iperf_strdup(bind_address);
}


/********************** Get/set test protocol structure ***********************/

struct protocol *
get_protocol(struct iperf_test *test, int prot_id)
{
    struct protocol *prot;

    SLIST_FOREACH(prot, &test->protocols, protocols) {
        if (prot->id == prot_id)
            break;
    }

    if (prot == NULL)
        i_errno = IEPROTOCOL;

    return prot;
}

int
set_protocol(struct iperf_test *test, int prot_id)
{
    struct protocol *prot = NULL;

    SLIST_FOREACH(prot, &test->protocols, protocols) {
        if (prot->id == prot_id) {
            test->protocol = prot;
	        check_sender_has_retransmits(test);
            return 0;
        }
    }

    i_errno = IEPROTOCOL;
    return -1;
}


/************************** Iperf callback functions **************************/

void
iperf_on_new_stream(struct iperf_stream *sp)
{
    connect_msg(sp);
}

void
iperf_on_test_start(struct iperf_test *test)
{
    if (test->verbose) {
        if (test->settings->bytes)
        	iprintf(test, iperf_test_start_bytes, test->protocol->name, test->num_streams, test->settings->blksize, test->omit, test->settings->bytes);
        else if (test->settings->blocks)
        	iprintf(test, iperf_test_start_blocks, test->protocol->name, test->num_streams, test->settings->blksize, test->omit, test->settings->blocks);
        else
        	iprintf(test, iperf_test_start_time, test->protocol->name, test->num_streams, test->settings->blksize, test->omit, test->duration);
    }
}

void
iperf_on_connect(struct iperf_test *test)
{
    //time_t now_secs;
    int port;
    char ipr[100];
    //struct sockaddr_storage sa;
    struct sockaddr_in sa;    
    struct sockaddr_in *sa_inP;
    socklen_t len;

    //now_secs = time((time_t*) 0);
    //(void) strftime(now_str, sizeof(now_str), rfc1123_fmt, gmtime(&now_secs));
    //if (test->verbose)
	//    iprintf(test, report_time, now_str);

    if (test->role == 'c') {
        iprintf(test, iperf_report_connecting, test->server_hostname, test->server_port);
        if (test->reverse)
    	    iprintf(test, iperf_report_reverse, test->server_hostname);
    } else {
        len = sizeof(sa);
        getpeername(test->ctrl_sck, (struct sockaddr *) &sa, &len);
        if (getsockdomain(test->ctrl_sck) == AF_INET) {
            ip_addr_t remote_addr;
            char *pstr;
	        sa_inP = (struct sockaddr_in *) &sa;
	        inet_addr_to_ipaddr(&remote_addr, &sa_inP->sin_addr);
            pstr = ipaddr_ntoa(&remote_addr);
            STRCPY(ipr, pstr);
	        port = ntohs(sa_inP->sin_port);
        }
	    iprintf(test, iperf_report_accepted, ipr, port);
    }
    if (test->verbose) {
        iprintf(test, iperf_report_cookie, test->cookie);
        if (test->protocol->id == SOCK_STREAM) {
            if (test->settings->mss)
                iprintf(test, "      TCP MSS: %d\n", test->settings->mss);
        }
    }
}

void
iperf_on_test_finish(struct iperf_test *test)
{
}


/******************************************************************************/
/*
    return values:
        <0, error
        0, go ahead to run client or server
        100, end of iperf
*/
int
iperf_parse_arguments(struct iperf_test *test, int argc, char **argv)
{
/*
    static struct option longopts[] =
    {
        {"port", required_argument, NULL, 'p'},
        {"interval", required_argument, NULL, 'i'},
        {"verbose", no_argument, NULL, 'V'},
        {"version", no_argument, NULL, 'v'},
        {"server", no_argument, NULL, 's'},
        {"client", required_argument, NULL, 'c'},
        {"udp", no_argument, NULL, 'u'},
        {"parallel", required_argument, NULL, 'P'},
        {"time", required_argument, NULL, 't'},        
        {"bandwidth", required_argument, NULL, 'b'},
        {"length", required_argument, NULL, 'l'},
        {"no-delay", no_argument, NULL, 'N'},
        {"reverse", no_argument, NULL, 'R'},
        {"debug", no_argument, NULL, 'd'},
        {"help", no_argument, NULL, 'h'},
        {NULL, 0, NULL, 0}
    };
    */
    int i;
    int blksize;
    int server_flag, client_flag, rate_flag, duration_flag;   
    s8* slash;

    blksize = 0;
    server_flag = client_flag = rate_flag = duration_flag = 0;   
    for(i=1; i<argc; i++) {
        if (STRCMP(argv[i], "-p") == 0 || STRCMP(argv[i], "--port") == 0 )
        {
            if (++i >= argc) {
                return -1;
            }
            test->server_port = ATOI(argv[i]);
            continue;
        }
        else if (STRCMP(argv[i], "-i") == 0 || STRCMP(argv[i], "--interval") == 0 ){
            if (++i >= argc) {
                return -1;
            }
            test->stats_interval = test->reporter_interval = ATOI(argv[i]);
            if ((test->stats_interval < MIN_INTERVAL || test->stats_interval > MAX_INTERVAL) && test->stats_interval != 0) {
                i_errno = IEINTERVAL;
                return -1;
            }
            continue;
        }
        else if (STRCMP(argv[i], "-c") == 0 || STRCMP(argv[i], "--client") == 0 )
        {
            if (++i >= argc) {
                return -1;
            }
            if (test->role == 's') {
                i_errno = IESERVCLIENT;
                return -1;
            }
            iperf_set_test_role(test, 'c');
            iperf_set_test_server_hostname(test, argv[i]);
            continue;
        }
        else if (STRCMP(argv[i], "-t") == 0 || STRCMP(argv[i], "--time") == 0 )
        {
            if (++i >= argc) {
                return -1;
            }
            test->duration = ATOI(argv[i]);
            if (test->duration > MAX_TIME) {
                i_errno = IEDURATION;
                return -1;
            }
            duration_flag = 1;
            client_flag = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-P") == 0 || STRCMP(argv[i], "--parallel") == 0 )
        {
            if (++i >= argc) {
                return -1;
            }
            test->num_streams = ATOI(argv[i]);
            if (test->num_streams > MAX_STREAMS) {
                i_errno = IENUMSTREAMS;
                return -1;
            }
            client_flag = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-b") == 0 || STRCMP(argv[i], "--bandwidth") == 0 )
        {
            if (++i >= argc) {
                return -1;
            }
            slash = (s8 * )STRCHR((const s8 *)argv[i], (s8)'/');
            if (slash) {
                *slash = '\0';
                ++slash;
                test->settings->burst = ATOI(slash);
                if (test->settings->burst <= 0 || test->settings->burst > MAX_BURST) {
                    i_errno = IEBURST;
                    return -1;
                }
            }
            test->settings->rate = unit_atoi_rate(argv[i]);
            if (test->settings->rate > MAX_BANDWIDTH)
            {
                i_errno = IEBANDWIDTH;
                return -1;
            }
    		rate_flag = 1;
            client_flag = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-l") == 0 || STRCMP(argv[i], "--len") == 0 ){
            if (++i >= argc) {
                return -1;
            }
            blksize = unit_atoi_rate(argv[i]);
    		client_flag = 1;
			continue;
        }
        else if (STRCMP(argv[i], "-V") == 0 || STRCMP(argv[i], "--verbose") == 0){
            test->verbose = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-v") == 0 || STRCMP(argv[i], "--version") == 0 ){
            PRINTF("%s\n", iperf_version);
            return 100;
        }
        else if (STRCMP(argv[i], "-s") == 0 || STRCMP(argv[i], "--server") == 0 ){
            if (test->role == 'c') {
                i_errno = IESERVCLIENT;
                return -1;
            }
            iperf_set_test_role(test, 's');
            continue;
        }
        else if (STRCMP(argv[i], "-u") == 0 || STRCMP(argv[i], "--udp") == 0 ){
            set_protocol(test, Pudp);
            client_flag = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-N") == 0 || STRCMP(argv[i], "--no-delay") == 0 ){
            test->no_delay = 1;
            client_flag = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-R") == 0 || STRCMP(argv[i], "--reverse") == 0 ){
            iperf_set_test_reverse(test, 1);
            client_flag = 1;
            continue;
        }
        else if (STRCMP(argv[i], "-d") == 0 || STRCMP(argv[i], "--debug") == 0 ){
            test->debug = 1;
            continue;
        }
        else
        {
            usage_long();
            return 100;
        }
    }

    /* Check flag / role compatibility. */
    if (test->role == 'c' && server_flag) {
    	i_errno = IESERVERONLY;
    	return -1;
    }
    if (test->role == 's' && client_flag) {
    	i_errno = IECLIENTONLY;
    	return -1;
    }

    if (!test->bind_address && test->bind_port) {
        i_errno = IEBIND;
        return -1;
    }
    if (blksize == 0) {
    	if (test->protocol->id == Pudp)
    	    blksize = DEFAULT_UDP_BLKSIZE;
    	else if (test->protocol->id == Psctp)
    	    blksize = DEFAULT_SCTP_BLKSIZE;
    	else
    	    blksize = DEFAULT_TCP_BLKSIZE;
    }
    if (blksize < MIN_BLOCKSIZE || blksize > MAX_BLOCKSIZE) {
    	i_errno = IEBLOCKSIZE;
    	return -1;
    }
    if (test->protocol->id == Pudp &&
    	blksize > MAX_UDP_BLOCKSIZE) {
    	i_errno = IEUDPBLOCKSIZE;
    	return -1;
    }
    test->settings->blksize = blksize;

    if (!rate_flag)
    	test->settings->rate = test->protocol->id == Pudp ? UDP_RATE : 0;

    if ((test->settings->bytes != 0 || test->settings->blocks != 0) && ! duration_flag)
        test->duration = 0;

    /* Disallow specifying multiple test end conditions. The code actually
    ** works just fine without this prohibition. As soon as any one of the
    ** three possible end conditions is met, the test ends. So this check
    ** could be removed if desired.
    */
    if ((duration_flag && test->settings->bytes != 0) ||
        (duration_flag && test->settings->blocks != 0) ||
    	(test->settings->bytes != 0 && test->settings->blocks != 0)) {
        i_errno = IEENDCONDITIONS;
        return -1;
    }

    if ((test->role != 'c') && (test->role != 's')) {
        i_errno = IENOROLE;
        return -1;
    }
        
    return 0;
}

void iperf_set_state (struct iperf_test* test, signed char state)
{
    if (test->debug)
        PRINTF("iperf state changed: %d --> %d\n", test->state, state);
    
    test->state = state;
}


int
iperf_set_send_state(struct iperf_test *test, signed char state)
{
    iperf_set_state(test, state);
    if (Nwrite(test->ctrl_sck, (char*) &state, sizeof(state), Ptcp) < 0) {
    	i_errno = IESENDMESSAGE;
    	return -1;
    }

    return 0;
}

void
iperf_check_throttle(struct iperf_stream *sp, struct timeval *nowP)
{
    s32 msec;
    u32 bits_per_second;

    
    if (sp->test->done)
        return;
    msec = msecond_delta(nowP, &sp->result->start_time);
    if (msec == 0)
        return;
    bits_per_second = sp->result->bytes_sent_this_interval * 8 * 1000 / (u32)msec;    

    if (sp->test->debug){
        //PRINTF("check_id:%ld, msec:%ld, bits_per_sec:%u \n", check_id++, msec, bits_per_second);
    }
    
    if (bits_per_second < sp->test->settings->rate) {
        sp->green_light = 1;
        FD_SET(sp->socket, &sp->test->write_set);
    } else {
        sp->green_light = 0;
        FD_CLR(sp->socket, &sp->test->write_set);
    }
}

int
iperf_send(struct iperf_test *test, fd_set *write_setP)
{
    register int multisend, r, streams_active;
    register struct iperf_stream *sp;
    struct timeval now;

    /* Can we do multisend mode? */
    if (test->settings->burst != 0)
        multisend = test->settings->burst;
    else if (test->settings->rate == 0)
        multisend = test->multisend;
    else
        multisend = 1;	/* nope */

    for (; multisend > 0; --multisend) {
    	if (test->settings->rate != 0 && test->settings->burst == 0)
    	    iperf_gettimeofday(&now, NULL);

    	streams_active = 0;
    	SLIST_FOREACH(sp, &test->streams, streams) {
    	    if (sp->green_light &&
    	        (write_setP == NULL || FD_ISSET(sp->socket, write_setP))) {
        		if ((r = sp->snd(sp)) < 0) {
        		    if (r == NET_SOFTERROR)
            			break;
        		    i_errno = IESTREAMWRITE;
        		    return r;
        		}
        		streams_active = 1;
        		test->bytes_sent += r;
        		++test->blocks_sent;
		        if (test->settings->rate != 0 && test->settings->burst == 0)
		            iperf_check_throttle(sp, &now);
        		if (multisend > 1 && test->settings->bytes != 0 && test->bytes_sent >= test->settings->bytes)
        		    break;
        		if (multisend > 1 && test->settings->blocks != 0 && test->blocks_sent >= test->settings->blocks)
        		    break;
    	    }
    	}
    	if (!streams_active)
    	    break;
    }

    if (test->settings->burst != 0) {
    	iperf_gettimeofday(&now, NULL);
    	SLIST_FOREACH(sp, &test->streams, streams)
    	    iperf_check_throttle(sp, &now);
    }
    if (write_setP != NULL)
    	SLIST_FOREACH(sp, &test->streams, streams)
    	    if (FD_ISSET(sp->socket, write_setP))
        		FD_CLR(sp->socket, write_setP);

    return 0;
}

int
iperf_recv(struct iperf_test *test, fd_set *read_setP)
{
    int r;
    struct iperf_stream *sp;

    SLIST_FOREACH(sp, &test->streams, streams) {
    	if (FD_ISSET(sp->socket, read_setP)) {
    	    while ((r = sp->rcv(sp)) > 0) {
                test->bytes_sent += r;
                ++test->blocks_sent;
    	    }
    	    FD_CLR(sp->socket, read_setP);
            if (r == NET_SOFTERROR || r==0)  //next stream
                continue;

    		i_errno = IESTREAMREAD;
    		return r;
    	}
    }
    return 0;
}

int
iperf_init_test(struct iperf_test *test)
{
    struct timeval now;
    struct iperf_stream *sp;

    if (test->protocol->init) {
        if (test->protocol->init(test) < 0)
            return -1;
    }

    /* Init each stream. */
    if (iperf_gettimeofday(&now, NULL) < 0) {
    	i_errno = IEINITTEST;
    	return -1;
    }
    SLIST_FOREACH(sp, &test->streams, streams) {
    	sp->result->start_time = now;
    }

    if (test->on_test_start)
        test->on_test_start(test);

    return 0;
}

static void
send_timer_proc(TimerClientData client_data, struct timeval *nowP)
{
    struct iperf_stream *sp = client_data.p;

    /* All we do here is set or clear the flag saying that this stream may
    ** be sent to.  The actual sending gets done in the send proc, after
    ** checking the flag.
    */
    iperf_check_throttle(sp, nowP);
}

int
iperf_create_send_timers(struct iperf_test * test)
{
    struct iperf_stream *sp;
    TimerClientData cd;

    SLIST_FOREACH(sp, &test->streams, streams) {
        sp->green_light = 1;
    	if (test->settings->rate != 0) {
    	    cd.p = sp;
    	    sp->send_timer = tmr_create((struct timeval*) 0, send_timer_proc, cd, 100L, 1, test);
    	    /* (Repeat every tenth second - arbitrary often value.) */
    	    if (sp->send_timer == NULL) {
        		i_errno = IEINITTEST;
        		return -1;
    	    }
    	}
    }
    return 0;
}

/**
 * iperf_exchange_parameters - handles the param_Exchange part for client
 *
 */

int
iperf_exchange_parameters(struct iperf_test *test)
{
    int s;
    s32 err;

    if (test->role == 'c') {
        if (send_parameters(test) < 0)
            return -1;
    } else {
        if (get_parameters(test) < 0)
            return -1;

        if ((s = test->protocol->sock_listen(test)) < 0) {
    	    if (iperf_set_send_state(test, SERVER_ERROR) != 0)
                return -1;
            err = htonl(i_errno);
            if (Nwrite(test->ctrl_sck, (char*) &err, sizeof(err), Ptcp) < 0) {
                i_errno = IECTRLWRITE;
                return -1;
            }
            err = htonl(errno);
            if (Nwrite(test->ctrl_sck, (char*) &err, sizeof(err), Ptcp) < 0) {
                i_errno = IECTRLWRITE;
                return -1;
            }
            return -1;
        }
        FD_SET(s, &test->read_set);
        test->max_fd = (s > test->max_fd) ? s : test->max_fd;
        test->prot_listener = s;

        // Send the control message to create streams and start the test
    	if (iperf_set_send_state(test, CREATE_STREAMS) != 0)
            return -1;
    }

    return 0;
}

/*************************************************************/

int
iperf_exchange_results(struct iperf_test *test)
{
    if (test->role == 'c') {
        /* Send results to server. */
    	if (send_results(test) < 0)
            return -1;
        /* Get server results. */
        if (get_results(test) < 0)
            return -1;
    } else {
        /* Get client results. */
        if (get_results(test) < 0)
            return -1;
        /* Send results to client. */
	    if (send_results(test) < 0)
            return -1;
    }
    return 0;
}

/*************************************************************/

static int
send_parameters(struct iperf_test *test)
{
    int r = 0;
    cJSON *j;

    j = cJSON_CreateObject();
    if (j == NULL) {
    	i_errno = IESENDPARAMS;
    	r = -1;
    } else {
    	if (test->protocol->id == Ptcp)
    	    cJSON_AddTrueToObject(j, "tcp");
    	else if (test->protocol->id == Pudp)
            cJSON_AddTrueToObject(j, "udp");
        else if (test->protocol->id == Psctp)
            cJSON_AddTrueToObject(j, "sctp");
    	cJSON_AddIntToObject(j, "omit", test->omit);
    	if (test->duration)
    	    cJSON_AddIntToObject(j, "time", test->duration);
    	if (test->settings->bytes)
    	    cJSON_AddIntToObject(j, "num", test->settings->bytes);
    	if (test->settings->blocks)
    	    cJSON_AddIntToObject(j, "blockcount", test->settings->blocks);
    	if (test->settings->mss)
    	    cJSON_AddIntToObject(j, "MSS", test->settings->mss);
    	if (test->no_delay)
    	    cJSON_AddTrueToObject(j, "nodelay");
    	cJSON_AddIntToObject(j, "parallel", test->num_streams);
    	if (test->reverse)
    	    cJSON_AddTrueToObject(j, "reverse");
    	if (test->settings->socket_bufsize)
    	    cJSON_AddIntToObject(j, "window", test->settings->socket_bufsize);
    	if (test->settings->blksize)
    	    cJSON_AddIntToObject(j, "len", test->settings->blksize);
    	if (test->settings->rate)
    	    cJSON_AddIntToObject(j, "bandwidth", test->settings->rate);
    	if (test->settings->burst)
    	    cJSON_AddIntToObject(j, "burst", test->settings->burst);
    	if (test->settings->tos)
    	    cJSON_AddIntToObject(j, "TOS", test->settings->tos);
    	if (test->settings->flowlabel)
    	    cJSON_AddIntToObject(j, "flowlabel", test->settings->flowlabel);
    	if (test->title)
    	    cJSON_AddStringToObject(j, "title", test->title);
    	if (test->congestion)
    	    cJSON_AddStringToObject(j, "congestion", test->congestion);
    	if (test->get_server_output)
    	    cJSON_AddIntToObject(j, "get_server_output", iperf_get_test_get_server_output(test));

    	cJSON_AddStringToObject(j, "client_version", IPERF_VERSION);

    	if (test->debug) {
    	    PRINTF("send_parameters:\n%s\n", cJSON_Print(j));
    	}

    	if (JSON_write(test->ctrl_sck, j) < 0) {
    	    i_errno = IESENDPARAMS;
    	    r = -1;
    	}
    	cJSON_Delete(j);
    }
    return r;
}

/*************************************************************/
static int
get_parameters(struct iperf_test *test)
{
    int r = 0;
    cJSON *j;
    cJSON *j_p;

    j = JSON_read(test->ctrl_sck);
    if (j == NULL) {
    	i_errno = IERECVPARAMS;
        r = -1;
    } else {
    	if (test->debug) {
    	    PRINTF("get_parameters:\n%s\n", cJSON_Print(j));
    	}

    	if ((j_p = cJSON_GetObjectItem(j, "tcp")) != NULL)
    	    set_protocol(test, Ptcp);
    	if ((j_p = cJSON_GetObjectItem(j, "udp")) != NULL)
    	    set_protocol(test, Pudp);
        if ((j_p = cJSON_GetObjectItem(j, "sctp")) != NULL)
            set_protocol(test, Psctp);
    	if ((j_p = cJSON_GetObjectItem(j, "omit")) != NULL)
    	    test->omit = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "time")) != NULL)
    	    test->duration = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "num")) != NULL)
    	    test->settings->bytes = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "blockcount")) != NULL)
    	    test->settings->blocks = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "MSS")) != NULL)
    	    test->settings->mss = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "nodelay")) != NULL)
    	    test->no_delay = 1;
    	if ((j_p = cJSON_GetObjectItem(j, "parallel")) != NULL)	{
    	    test->num_streams = j_p->valueint;
    	    if (test->num_streams > MAX_STREAMS)
    	    {
    	        if (test->debug)
    	            PRINTF("max streams in parallel allowed: %d \n", MAX_STREAMS);

                i_errno = IERECVPARAMS;
                cJSON_Delete(j);
                return -1;
    	    }
    	}
    	if ((j_p = cJSON_GetObjectItem(j, "reverse")) != NULL){
    	    {
    	        if (test->debug)
    	            PRINTF("NOT support reverse option in server mode \n");

                i_errno = IERECVPARAMS;
                cJSON_Delete(j);
                return -1;
    	    }
    	    //iperf_set_test_reverse(test, 1);    //there is still a bug yet!
    	}
    	if ((j_p = cJSON_GetObjectItem(j, "window")) != NULL)
    	    test->settings->socket_bufsize = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "len")) != NULL){
    	    test->settings->blksize = j_p->valueint;
    	    if (test->settings->blksize > MAX_BLOCKSIZE)
    	    {
    	        if (test->debug)
    	            PRINTF("max block size allowed: %d \n", MAX_BLOCKSIZE);
    	        test->settings->blksize = MAX_BLOCKSIZE;
    	    }
    	    else if (test->settings->blksize < MIN_BLOCKSIZE)
    	    {
    	        if (test->debug)
    	            PRINTF("min block size allowed: %d \n", MIN_BLOCKSIZE);
    	        test->settings->blksize = MIN_BLOCKSIZE;
    	    }
        }
    	if ((j_p = cJSON_GetObjectItem(j, "bandwidth")) != NULL)
    	    test->settings->rate = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "burst")) != NULL)
    	    test->settings->burst = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "TOS")) != NULL)
    	    test->settings->tos = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "flowlabel")) != NULL)
    	    test->settings->flowlabel = j_p->valueint;
    	if ((j_p = cJSON_GetObjectItem(j, "title")) != NULL)
    	    test->title = iperf_strdup(j_p->valuestring);
    	if ((j_p = cJSON_GetObjectItem(j, "congestion")) != NULL)
    	    test->congestion = iperf_strdup(j_p->valuestring);
    	if ((j_p = cJSON_GetObjectItem(j, "get_server_output")) != NULL)
    	    iperf_set_test_get_server_output(test, 1);
    	if (test->sender && test->protocol->id == Ptcp && has_tcpinfo_retransmits())
    	    test->sender_has_retransmits = 1;
    	cJSON_Delete(j);
    }

    return r;
}

/*************************************************************/
static int
send_results(struct iperf_test *test)
{
    int r = 0;
    cJSON *j;
    cJSON *j_streams;
    struct iperf_stream *sp;
    cJSON *j_stream;
    int sender_has_retransmits;
    iperf_size_t bytes_transferred;
    int retransmits;

    j = cJSON_CreateObject();
    if (j == NULL) {
    	i_errno = IEPACKAGERESULTS;
    	r = -1;
    } else {
	    cJSON_AddStringToObject(j, "cpu_util_total", "15.98");
	    cJSON_AddStringToObject(j, "cpu_util_user", "10.0");
	    cJSON_AddStringToObject(j, "cpu_util_system", "5.98");
	    if ( ! test->sender )
	        sender_has_retransmits = -1;
	    else
	        sender_has_retransmits = test->sender_has_retransmits;
     	cJSON_AddIntToObject(j, "sender_has_retransmits", sender_has_retransmits);

    	/* If on the server and sending server output, then do this */
    	if (test->role == 's' && test->get_server_output) {
    		/* Add textual output */
    		size_t buflen = 0;
            char *output;
            
    		/* Figure out how much room we need to hold the complete output string */
    		struct iperf_textline *t;
    		TAILQ_FOREACH(t, &(test->server_output_list), textlineentries) {
    		    buflen += ssv6xxx_strlen(t->line);
    		}

    		/* Allocate and build it up from the component lines */
    		output = iperf_malloc(buflen + 1);
		    if (output == NULL) {
                i_errno = IEPACKAGERESULTS;
                r = -1;
            }
    		MEMSET(output, 0, sizeof(buflen+1));
    		TAILQ_FOREACH(t, &(test->server_output_list), textlineentries) {
    		    STRNCAT(output, t->line, buflen);
    		    buflen -= STRLEN(t->line);
    		}

    		cJSON_AddStringToObject(j, "server_output_text", output);
    	}

    	j_streams = cJSON_CreateArray();
    	if (j_streams == NULL) {
    	    i_errno = IEPACKAGERESULTS;
    	    r = -1;
    	} else {
    	    cJSON_AddItemToObject(j, "streams", j_streams);
    	    SLIST_FOREACH(sp, &test->streams, streams) {
        		j_stream = cJSON_CreateObject();
        		if (j_stream == NULL) {
        		    i_errno = IEPACKAGERESULTS;
        		    r = -1;
        		} else {
        		    cJSON_AddItemToArray(j_streams, j_stream);
        		    bytes_transferred = test->sender ? sp->result->bytes_sent : sp->result->bytes_received;
        		    retransmits = (test->sender && test->sender_has_retransmits) ? sp->result->stream_retrans : -1;
        		    cJSON_AddIntToObject(j_stream, "id", sp->id);
        		    cJSON_AddIntToObject(j_stream, "bytes", bytes_transferred);
        		    cJSON_AddIntToObject(j_stream, "retransmits", retransmits);
        		    if (sp->packet_count > sp->cnt_error)
    		    #ifdef IPERF_SUPPORT_DOUBLE    		    //in seconds
                        cJSON_AddFloatToObject(j_stream, "jitter", sp->jitter/(1000*(sp->packet_count - sp->cnt_error)));
    		    #else                                   //in ms, otherwise jitter would be zero.
            		    cJSON_AddIntToObject(j_stream, "jitter", sp->jitter/(sp->packet_count - sp->cnt_error));
        		#endif
            		else
            		    cJSON_AddIntToObject(j_stream, "jitter", 0);
        		    cJSON_AddIntToObject(j_stream, "errors", sp->cnt_error);
        		    cJSON_AddIntToObject(j_stream, "packets", sp->packet_count);
        		}
    	    }
    	    if (r == 0 && test->debug) {
        		PRINTF("send_results\n%s\n", cJSON_Print(j));
    	    }
    	    if (r == 0 && JSON_write(test->ctrl_sck, j) < 0) {
        		i_errno = IESENDRESULTS;
        		r = -1;
    	    }
    	}
    	cJSON_Delete(j);
    }
    return r;
}
/*************************************************************/

static int
get_results(struct iperf_test *test)
{
    int r = 0;
    cJSON *j;
    cJSON *j_sender_has_retransmits;
    int result_has_retransmits;
    cJSON *j_streams;
    int n, i;
    cJSON *j_stream;
    cJSON *j_id;
    cJSON *j_bytes;
    cJSON *j_retransmits;
    cJSON *j_jitter;
    cJSON *j_errors;
    cJSON *j_packets;
    cJSON *j_server_output;
    int sid, cerror, pcount;
    iperf_jit_t jitter;
    iperf_size_t bytes_transferred;
    int retransmits;
    struct iperf_stream *sp;

    j = JSON_read(test->ctrl_sck);
    if (j == NULL) {
    	i_errno = IERECVRESULTS;
        r = -1;
    } else {
    	j_sender_has_retransmits = cJSON_GetObjectItem(j, "sender_has_retransmits");
    	if (j_sender_has_retransmits == NULL) {
    	    i_errno = IERECVRESULTS;
    	    r = -1;
    	} else {
    	    if (test->debug) {
        		PRINTF("get_results:\n%s\n", cJSON_Print(j));
    	    }

    	    result_has_retransmits = j_sender_has_retransmits->valueint;
    	    if (! test->sender)
        		test->sender_has_retransmits = result_has_retransmits;
    	    j_streams = cJSON_GetObjectItem(j, "streams");
    	    if (j_streams == NULL) {
        		i_errno = IERECVRESULTS;
        		r = -1;
    	    } else {
    	        n = cJSON_GetArraySize(j_streams);
        		for (i=0; i<n; ++i) {
        		    j_stream = cJSON_GetArrayItem(j_streams, i);
        		    if (j_stream == NULL) {
            			i_errno = IERECVRESULTS;
            			r = -1;
        		    } else {
            			j_id = cJSON_GetObjectItem(j_stream, "id");
            			j_bytes = cJSON_GetObjectItem(j_stream, "bytes");
            			j_retransmits = cJSON_GetObjectItem(j_stream, "retransmits");
            			j_jitter = cJSON_GetObjectItem(j_stream, "jitter");
            			j_errors = cJSON_GetObjectItem(j_stream, "errors");
            			j_packets = cJSON_GetObjectItem(j_stream, "packets");
            			if (j_id == NULL || j_bytes == NULL || j_retransmits == NULL || j_jitter == NULL || j_errors == NULL || j_packets == NULL) {
            			    i_errno = IERECVRESULTS;
            			    r = -1;
            			} else {
            			    sid = j_id->valueint;
            			    bytes_transferred = j_bytes->valueint;
            			    retransmits = j_retransmits->valueint;
            			    if (test->sender) {
                		        #ifdef IPERF_SUPPORT_DOUBLE
                			    jitter = j_jitter->valuefloat*1000.0;  //get in seconds, print in ms.
                			    #else
                			    jitter = 0;
                			    #endif
                			}
            			    cerror = j_errors->valueint;
            			    pcount = j_packets->valueint;
            			    SLIST_FOREACH(sp, &test->streams, streams)
            				if (sp->id == sid) break;
            			    if (sp == NULL) {
                				i_errno = IESTREAMID;
                				r = -1;
            			    } else {
                				if (test->sender) {
                				    sp->jitter = jitter;
                				    sp->cnt_error = cerror;
                				    sp->packet_count = pcount;
                				    sp->result->bytes_received = bytes_transferred;
                				} else {
                				    sp->result->bytes_sent = bytes_transferred;
                				    sp->result->stream_retrans = retransmits;
                				}
            			    }
            			}
        		    }
        		}
        		/*
            		 * If we're the client and we're supposed to get remote results,
            		 * look them up and process accordingly.
            		 */
        		if (test->role == 'c' && iperf_get_test_get_server_output(test)) {
        		    /* Look for JSON.  If we find it, grab the object so it doesn't get deleted. */
        		    j_server_output = cJSON_DetachItemFromObject(j, "server_output_json");
        		    if (j_server_output != NULL) {
            			test->json_server_output = j_server_output;
        		    }
        		    else {
            			/* No JSON, look for textual output.  Make a copy of the text for later. */
            			j_server_output = cJSON_GetObjectItem(j, "server_output_text");
            			if (j_server_output != NULL) {
            			    test->server_output_text = iperf_strdup(j_server_output->valuestring);
            			}
        		    }
        		}
    	    }
    	}
    	cJSON_Delete(j);
    }
    return r;
}

/*************************************************************/

static int
JSON_write(int fd, cJSON *json)
{
    u32 hsize, nsize;
    char *str;
    int r = 0;

    str = cJSON_PrintUnformatted(json);
    if (str == NULL)
    	r = -1;
    else {
    	hsize = STRLEN(str);
    	nsize = htonl(hsize);
    	if (Nwrite(fd, (char*) &nsize, sizeof(nsize), Ptcp) < 0)
    	    r = -1;
    	else {
    	    if (Nwrite(fd, str, hsize, Ptcp) < 0)
    		r = -1;
    	}
    	iperf_free(str);
    }
    
    return r;
}

/*************************************************************/

static cJSON *
JSON_read(int fd)
{
    u32 hsize, nsize;
    char *str;
    cJSON *json = NULL;

    if (Nread(fd, (char*) &nsize, sizeof(nsize), Ptcp) >= 0) {
    	hsize = ntohl(nsize);
    	str = (char *) iperf_malloc(hsize+1);	/* +1 for EOS */
    	if (str != NULL) {
    	    if (Nread(fd, str, hsize, Ptcp) >= 0) {
        		str[hsize] = '\0';	/* add the EOS */
        		json = cJSON_Parse(str);
    	    }
            iperf_free(str);
    	}
    }
    return json;
}

/*************************************************************/
/**
 * add_to_interval_list -- adds new interval to the interval_list
 */

void
add_to_interval_list(struct iperf_stream_result * rp, struct iperf_interval_results * new)
{
    struct iperf_interval_results *irp;

    irp = (struct iperf_interval_results *) iperf_malloc(sizeof(struct iperf_interval_results));
    if (irp != NULL){
        MEMCPY(irp, new, sizeof(struct iperf_interval_results));
        TAILQ_INSERT_TAIL(&rp->interval_results, irp, irlistentries);
    }
}


/************************************************************/

/**
 * connect_msg -- displays connection message
 * denoting sender/receiver details
 *
 */

void
connect_msg(struct iperf_stream *sp)
{
    char ipl[100], ipr[100], *pstr;
    int lport, rport;

    if (getsockdomain(sp->socket) == AF_INET) {
        ip_addr_t ipv4_addr;
         
        inet_addr_to_ipaddr(&ipv4_addr, &((struct sockaddr_in *) &sp->local_addr)->sin_addr);
        pstr = ipaddr_ntoa(&ipv4_addr);
        STRCPY(ipl, pstr);

        inet_addr_to_ipaddr(&ipv4_addr, &((struct sockaddr_in *) &sp->remote_addr)->sin_addr);
        pstr = ipaddr_ntoa(&ipv4_addr);
        STRCPY(ipr, pstr);
        
        lport = ntohs(((struct sockaddr_in *) &sp->local_addr)->sin_port);
        rport = ntohs(((struct sockaddr_in *) &sp->remote_addr)->sin_port);
    }

	iprintf(sp->test, iperf_report_connected, sp->socket, ipl, lport, ipr, rport);
}


/**************************************************************************/

struct iperf_test *
iperf_new_test()
{
    struct iperf_test *test;

    test = (struct iperf_test *) iperf_malloc(sizeof(struct iperf_test));
    if (!test) {
        i_errno = IENEWTEST;
        return NULL;
    }
    /* initialize everything to zero */
    MEMSET(test, 0, sizeof(struct iperf_test));

    test->settings = (struct iperf_settings *) iperf_malloc(sizeof(struct iperf_settings));
    if (!test->settings) {
        iperf_free(test);
    	i_errno = IENEWTEST;
	    return NULL;
    }
    MEMSET(test->settings, 0, sizeof(struct iperf_settings));
    test->listener = test->prot_listener = test->ctrl_sck = -1;
    
    return test;
}

/**************************************************************************/

struct protocol *
protocol_new(void)
{
    struct protocol *proto;

    proto = iperf_malloc(sizeof(struct protocol));
    if(proto != NULL)
        MEMSET(proto, 0, sizeof(struct protocol));

    return proto;
}

void
protocol_free(struct protocol *proto)
{
    iperf_free(proto); 
}

/**************************************************************************/
int
iperf_defaults(struct iperf_test *testp)
{
    struct protocol *tcp, *udp;

    testp->omit = OMIT;
    testp->duration = DURATION;
    TAILQ_INIT(&testp->xbind_addrs);
	
    testp->title = NULL;
    testp->congestion = NULL;
    testp->server_port = PORT;
    testp->ctrl_sck = -1;
    testp->prot_listener = -1;

    testp->stats_callback = iperf_stats_callback;
    testp->reporter_callback = iperf_reporter_callback;

    testp->stats_interval = testp->reporter_interval = 1;
    testp->num_streams = 1;
    testp->no_delay = 0;

    testp->settings->domain = AF_INET;  //AF_UNSPEC;
    testp->settings->unit_format = 'a';
    testp->settings->socket_bufsize = 0;    /* use autotuning */
    testp->settings->blksize = DEFAULT_TCP_BLKSIZE;
    testp->settings->burst = 0;
    testp->settings->mss = 0;
    testp->settings->bytes = 0;
    testp->settings->blocks = 0;
    MEMSET(testp->cookie, 0, COOKIE_SIZE);

    testp->multisend = 10;	/* arbitrary */

    /* Set up protocol list */
    SLIST_INIT(&testp->streams);
    SLIST_INIT(&testp->protocols);

    tcp = protocol_new();
    if (!tcp)
        return -1;

    tcp->id = Ptcp;
    tcp->name = "TCP";
    tcp->sock_accept = iperf_tcp_accept;
    tcp->sock_listen = iperf_tcp_listen;
    tcp->sock_connect = iperf_tcp_connect;
    tcp->sock_send = iperf_tcp_send;
    tcp->sock_recv = iperf_tcp_recv;
    tcp->init = NULL;
    SLIST_INSERT_HEAD(&testp->protocols, tcp, protocols);

    udp = protocol_new();
    if (!udp) {
        protocol_free(tcp);
        return -1;
    }

    udp->id = Pudp;
    udp->name = "UDP";
    udp->sock_accept = iperf_udp_accept;
    udp->sock_listen = iperf_udp_listen;
    udp->sock_connect = iperf_udp_connect;
    udp->sock_send = iperf_udp_send;
    udp->sock_recv = iperf_udp_recv;
    udp->init = iperf_udp_init;
    SLIST_INSERT_AFTER(tcp, udp, protocols);

    set_protocol(testp, Ptcp);

    testp->on_new_stream = iperf_on_new_stream;
    testp->on_test_start = iperf_on_test_start;
    testp->on_connect = iperf_on_connect;
    testp->on_test_finish = iperf_on_test_finish;

    TAILQ_INIT(&testp->server_output_list);

    return 0;
}


/**************************************************************************/
void
iperf_free_test(struct iperf_test *test)
{
    struct protocol *prot;
    struct iperf_stream *sp;
    struct iperf_textline *t;
    
    /* iperf_free streams */
    while (!SLIST_EMPTY(&test->streams)) {
        sp = SLIST_FIRST(&test->streams);
        SLIST_REMOVE_HEAD(&test->streams, streams);
        iperf_free_stream(sp);
    }

    if (test->server_hostname)
	    iperf_free(test->server_hostname);
    if (test->bind_address)
	    iperf_free(test->bind_address);

    if (!TAILQ_EMPTY(&test->xbind_addrs)) {
        struct xbind_entry *xbe;

        while (!TAILQ_EMPTY(&test->xbind_addrs)) {
            xbe = TAILQ_FIRST(&test->xbind_addrs);
            TAILQ_REMOVE(&test->xbind_addrs, xbe, link);
            if (xbe->ai)
                freeaddrinfo(xbe->ai);
            if (xbe->name)
                iperf_free(xbe->name);
            iperf_free(xbe);
        }
    }

    iperf_free(test->settings);
    if (test->title)
	    iperf_free(test->title);
    if (test->congestion)
	    iperf_free(test->congestion);
    if (test->omit_timer != NULL)
	    tmr_cancel(test->omit_timer, test);
    if (test->timer != NULL)
	    tmr_cancel(test->timer, test);
    if (test->stats_timer != NULL)
	    tmr_cancel(test->stats_timer, test);
    if (test->reporter_timer != NULL)
	    tmr_cancel(test->reporter_timer, test);
    tmr_destroy(test);

    /* iperf_free protocol list */
    while (!SLIST_EMPTY(&test->protocols)) {
        prot = SLIST_FIRST(&test->protocols);
        SLIST_REMOVE_HEAD(&test->protocols, protocols);        
        iperf_free(prot);
    }

    if (test->server_output_text) {
	    iperf_free(test->server_output_text);
	    test->server_output_text = NULL;
    }

    /* iperf_free output line buffers, if any (on the server only) */    
    while (!TAILQ_EMPTY(&test->server_output_list)) {
    	t = TAILQ_FIRST(&test->server_output_list);
    	TAILQ_REMOVE(&test->server_output_list, t, textlineentries);
    	if (t->line)
    	    iperf_free(t->line);
    	iperf_free(t);
    }

    /* sctp_bindx: do not free the arguments, only the resolver results */
    if (!TAILQ_EMPTY(&test->xbind_addrs)) {
        struct xbind_entry *xbe;

        TAILQ_FOREACH(xbe, &test->xbind_addrs, link) {
            if (xbe->ai) {
                freeaddrinfo(xbe->ai);
                xbe->ai = NULL;
            }
        }
    }

    /* XXX: Why are we setting these values to NULL? */
    // test->streams = NULL;
    test->stats_callback = NULL;
    test->reporter_callback = NULL;

    if (test->ctrl_sck >= 0)
        close (test->ctrl_sck);

    if (test->listener >= 0)
        close (test->listener);

    if (test->prot_listener >= 0)
        close(test->prot_listener);    

    iperf_free(test);
}


void
iperf_reset_test(struct iperf_test *test)
{
    struct iperf_stream *sp;
    struct iperf_textline *t;

    /* iperf_free streams */
    while (!SLIST_EMPTY(&test->streams)) {
        sp = SLIST_FIRST(&test->streams);
        SLIST_REMOVE_HEAD(&test->streams, streams);
        iperf_free_stream(sp);
    }
    if (test->omit_timer != NULL) {
    	tmr_cancel(test->omit_timer, test);
    	test->omit_timer = NULL;
    }
    if (test->timer != NULL) {
    	tmr_cancel(test->timer, test);
    	test->timer = NULL;
    }
    if (test->stats_timer != NULL) {
    	tmr_cancel(test->stats_timer, test);
    	test->stats_timer = NULL;
    }
    if (test->reporter_timer != NULL) {
    	tmr_cancel(test->reporter_timer, test);
    	test->reporter_timer = NULL;
    }
    tmr_destroy(test);
    
    test->done = 0;
    test->periods = 0;
    
    SLIST_INIT(&test->streams);

    test->role = 's';
    test->sender = 0;
    test->sender_has_retransmits = 0;
    set_protocol(test, Ptcp);
    test->omit = OMIT;
    test->duration = DURATION;
    iperf_set_state(test, 0);
    
	if (test->ctrl_sck >= 0)
		close(test->ctrl_sck);
	if (test->listener >= 0)
		close(test->listener);
	if (test->prot_listener >= 0)
	    close(test->prot_listener);
	
    test->ctrl_sck = -1;
    test->listener = -1;
    test->prot_listener = -1;

    test->bytes_sent = 0;
    test->blocks_sent = 0;

    test->reverse = 0;
    test->no_delay = 0;

    FD_ZERO(&test->read_set);
    FD_ZERO(&test->write_set);
    
    test->num_streams = 1;
    test->settings->socket_bufsize = 0;
    test->settings->blksize = DEFAULT_TCP_BLKSIZE;
    test->settings->burst = 0;
    test->settings->mss = 0;
    MEMSET(test->cookie, 0, COOKIE_SIZE);
    test->multisend = 10;	/* arbitrary */

    /* iperf_free output line buffers, if any (on the server only) */
    while (!TAILQ_EMPTY(&test->server_output_list)) {
    	t = TAILQ_FIRST(&test->server_output_list);
    	TAILQ_REMOVE(&test->server_output_list, t, textlineentries);
    	iperf_free((void*)t->line);
    	iperf_free((void*)t);
    }
}

/**************************************************************************/

/**
 * Gather statistics during a test.
 * This function works for both the client and server side.
 */
void
iperf_stats_callback(struct iperf_test *test)
{
    struct iperf_stream *sp;
    struct iperf_stream_result *rp = NULL;
    struct iperf_interval_results *irp, temp;

    if (test->state == TEST_RUNNING || test->state == TEST_END)
        test->periods++ ;

    temp.omitted = test->omitting;    
    SLIST_FOREACH(sp, &test->streams, streams) {
        rp = sp->result;
        
    	temp.bytes_transferred = test->sender ? rp->bytes_sent_this_interval : rp->bytes_received_this_interval;
         
    	temp.end_time = test->periods;
        temp.interval_duration = test->stats_interval;
    	if (test->protocol->id == Ptcp) {
    	    if ( has_tcpinfo()) {
    		    save_tcpinfo(sp, &temp);
        		if (test->sender && test->sender_has_retransmits) {
        		    long total_retrans = get_total_retransmits(&temp);
        		    temp.interval_retrans = total_retrans - rp->stream_prev_total_retrans;
        		    rp->stream_retrans += temp.interval_retrans;
        		    rp->stream_prev_total_retrans = total_retrans;

        		    temp.snd_cwnd = get_snd_cwnd(&temp);
        		    if (temp.snd_cwnd > rp->stream_max_snd_cwnd) {
        			    rp->stream_max_snd_cwnd = temp.snd_cwnd;
        		    }
        		    
        		    temp.rtt = get_rtt(&temp);
        		    if (temp.rtt > rp->stream_max_rtt) {
        			    rp->stream_max_rtt = temp.rtt;
        		    }
        		    if (rp->stream_min_rtt == 0 ||
            			temp.rtt < rp->stream_min_rtt) {
            			rp->stream_min_rtt = temp.rtt;
        		    }
        		    rp->stream_sum_rtt += temp.rtt;
        		    rp->stream_count_rtt++;
        		}
    	    }
    	} else {
            irp = TAILQ_LAST(&rp->interval_results, irlisthead);
    	    if (irp == NULL) {
        		temp.interval_packet_count = sp->packet_count;
        		temp.interval_outoforder_packets = sp->outoforder_packets;
        		temp.interval_cnt_error = sp->cnt_error;
        		if (temp.interval_packet_count - temp.interval_cnt_error > 0)
        		    temp.jitter = sp->jitter/(temp.interval_packet_count - temp.interval_cnt_error);
        		else
        		    temp.jitter = 0;
    	    } else {
        		temp.interval_packet_count = sp->packet_count - irp->packet_count;
        		temp.interval_outoforder_packets = sp->outoforder_packets - irp->outoforder_packets;
        		temp.interval_cnt_error = sp->cnt_error - irp->cnt_error;
        		if (temp.interval_packet_count - temp.interval_cnt_error > 0)
            		temp.jitter = (sp->jitter - irp->total_jitter)/(temp.interval_packet_count - temp.interval_cnt_error);
        		else
        		    temp.jitter = 0;
    	    }

    	    if (test->debug){
        	    //PRINTF("sp->jit:%ld, tmp.packet count:%ld\n", sp->jitter, temp.interval_packet_count);
        	    //if (irp){
        	    //    PRINTF("irp->jit:%ld, irp->packet cout:%ld\n", irp->jitter, irp->interval_packet_count);
        	    //}
        	    //PRINTF("jitter: %ld", temp.jitter);
        	}
    	    temp.packet_count = sp->packet_count;
    	    temp.outoforder_packets = sp->outoforder_packets;
    	    temp.cnt_error = sp->cnt_error;
    	    temp.total_jitter = sp->jitter;
    	}
        add_to_interval_list(rp, &temp);
        rp->bytes_sent_this_interval = rp->bytes_received_this_interval = 0;
        iperf_gettimeofday(&rp->start_time, NULL);
    }
}

/**
 * Print intermediate results during a test (interval report).
 * Uses print_interval_results to print the results for each stream,
 * then prints an interval summary for all streams in this
 * interval.
 */
static void
iperf_print_intermediate(struct iperf_test *test)
{
    char ubuf[UNIT_LEN];
    char nbuf[UNIT_LEN];
    struct iperf_stream *sp = NULL;
    struct iperf_interval_results *irp;
    iperf_size_t bytes = 0;
    u32 bandwidth;
    int retransmits = 0;
    u32 start_time, end_time;
    int total_packets = 0, lost_packets = 0;
    s32 lost_percent;
    iperf_jit_t avg_jitter = 0;

    SLIST_FOREACH(sp, &test->streams, streams) {
        print_interval_results(test, sp, NULL);

    	/* sum up all streams */
	    irp = TAILQ_LAST(&sp->result->interval_results, irlisthead);
	    if (irp == NULL) {
	        PRINTF("iperf_print_intermediate error: interval_results is NULL \n");
	        return;
	    }
        bytes += irp->bytes_transferred;
	    if (test->protocol->id == Ptcp) {
	        if (test->sender && test->sender_has_retransmits) {
    		    retransmits += irp->interval_retrans;
	        }
	    } else {
            total_packets += irp->interval_packet_count;
            lost_packets += irp->interval_cnt_error;
            avg_jitter += irp->jitter;
        }
    }
    
    /* next build string with sum of all streams */
    if (test->num_streams > 1) {
        sp = SLIST_FIRST(&test->streams); /* reset back to 1st stream */
       	/* Only do this of course if there was a first stream */
       	if (sp) {
            irp = TAILQ_LAST(&sp->result->interval_results, irlisthead);    /* use 1st stream for timing info */

            unit_snprintf(ubuf, UNIT_LEN, bytes, 'A');
            bandwidth = bytes / irp->interval_duration;
            unit_snprintf(nbuf, UNIT_LEN, bandwidth, test->settings->unit_format);

            start_time = (irp->end_time-1) * test->stats_interval;
            end_time = irp->end_time * test->stats_interval;
        	if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
        	    if (test->sender && test->sender_has_retransmits) {
        	        /* Interval sum, TCP with retransmits. */
    		        iprintf(test, iperf_report_sum_bw_retrans_format, start_time, end_time, ubuf, nbuf, retransmits, irp->omitted?iperf_report_omitted:""); /* XXX irp->omitted or test->omitting? */
        	    } else {
        		    /* Interval sum, TCP without retransmits. */
    		        iprintf(test, iperf_report_sum_bw_format, start_time, end_time, ubuf, nbuf, test->omitting?iperf_report_omitted:"");
        	    }
        	} else {
        	    /* Interval sum, UDP. */
        	    if (test->sender) {
    		        iprintf(test, iperf_report_sum_bw_udp_sender_format, start_time, end_time, ubuf, nbuf, total_packets, test->omitting?iperf_report_omitted:"");
        	    } else {
            		avg_jitter /= test->num_streams;
            		if (total_packets > 0)
            		    lost_percent = 100 * lost_packets / total_packets;
            		else
            		    lost_percent = 0;
	                iprintf(test, iperf_report_sum_bw_udp_format, start_time, end_time, ubuf, nbuf, avg_jitter, lost_packets, total_packets, lost_percent, test->omitting?iperf_report_omitted:"");
        	    }
        	}
    	}
    }

    /// free the memory once it's not used any futher!!!
    SLIST_FOREACH(sp, &test->streams, streams) {
        irp = TAILQ_LAST(&sp->result->interval_results, irlisthead);
        if (irp != NULL)
        {
            struct iperf_interval_results *irp_tmp;
            irp_tmp = TAILQ_PREV(irp, irlisthead, irlistentries);
            if (irp_tmp != NULL)
            {
                TAILQ_REMOVE(&sp->result->interval_results, irp_tmp, irlistentries);
                iperf_free(irp_tmp);
            }
        }
    }
}

/**
 * Print overall summary statistics at the end of a test.
 */
static void
iperf_print_results(struct iperf_test *test)
{

    int total_retransmits = 0;
    int total_packets = 0, lost_packets = 0;
    char ubuf[UNIT_LEN];
    char nbuf[UNIT_LEN];
    struct iperf_stream *sp = NULL;
    iperf_size_t bytes_sent, total_sent = 0;
    iperf_size_t bytes_received, total_received = 0;
    u32   start_time, end_time, lost_percent;
    iperf_jit_t avg_jitter = 0;
    u32   bandwidth;

    /* print final summary for all intervals */
	iprintf(test, "%s", iperf_report_bw_separator);
	if (test->verbose)
	    iprintf(test, "%s", iperf_report_summary);
	if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
	    if (test->sender_has_retransmits)
	    	iprintf(test, "%s", iperf_report_bw_retrans_header);
	    else
    		iprintf(test, "%s", iperf_report_bw_header);
	} else
	    iprintf(test, "%s", iperf_report_bw_udp_header);

    start_time = 0;
    sp = SLIST_FIRST(&test->streams);

    /* 
     * If there is at least one stream, then figure out the length of time
     * we were running the tests and print out some statistics about
     * the streams.  It's possible to not have any streams at all
     * if the client got interrupted before it got to do anything.
     */
    if (sp) {
        end_time = test->stats_interval * test->periods;
        SLIST_FOREACH(sp, &test->streams, streams) {
            bytes_sent = sp->result->bytes_sent;
            bytes_received = sp->result->bytes_received;
            total_sent += bytes_sent;
            total_received += bytes_received;

            if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
        	    if (test->sender_has_retransmits) {
            		total_retransmits += sp->result->stream_retrans;
        	    }
        	} else {
                total_packets += (sp->packet_count - sp->omitted_packet_count);
                lost_packets += sp->cnt_error;
                if (test->role == 's')  //else : client, just print what see.
                {
                    if (sp->jitter < 0)
                        sp->jitter = -sp->jitter;
                    if (total_packets > lost_packets)
                        sp->jitter /= total_packets - lost_packets;
                    else
                        sp->jitter = 0;
                    avg_jitter += sp->jitter;
                }
            }

        	unit_snprintf(ubuf, UNIT_LEN, bytes_sent, 'A');
        	if (end_time > 0)
            	bandwidth = bytes_sent / end_time;
        	else
        	    bandwidth = 0;
        	unit_snprintf(nbuf, UNIT_LEN, bandwidth, test->settings->unit_format);
        	if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
        	    if (test->sender_has_retransmits) {
            		/* Summary, TCP with retransmits. */
        		    iprintf(test, iperf_report_bw_retrans_format, sp->socket, start_time, end_time, ubuf, nbuf, sp->result->stream_retrans, iperf_report_sender);
            	} else {
            		/* Summary, TCP without retransmits. */
        		    iprintf(test, iperf_report_bw_format, sp->socket, start_time, end_time, ubuf, nbuf, iperf_report_sender);
        	    }
        	} else {
        	    /* Summary, UDP. */
        	    if (sp->packet_count - sp->omitted_packet_count > 0)
        	        lost_percent = 100 * sp->cnt_error / (sp->packet_count - sp->omitted_packet_count);
        	    else
        	        lost_percent = 100;

        		iprintf(test, iperf_report_bw_udp_format, sp->socket, start_time, end_time, ubuf, nbuf, sp->jitter, sp->cnt_error, (sp->packet_count - sp->omitted_packet_count), lost_percent, "");
        		if (test->role == 'c')
        		    iprintf(test, iperf_report_datagrams, sp->socket, (sp->packet_count - sp->omitted_packet_count));
        		if (sp->outoforder_packets > 0 || sp->cnt_error > 0)
        		    iprintf(test, iperf_report_sum_outoforder, start_time, end_time, sp->outoforder_packets, sp->cnt_error);
        	}

        	unit_snprintf(ubuf, UNIT_LEN, bytes_received, 'A');
        	bandwidth = bytes_received / end_time;
        	unit_snprintf(nbuf, UNIT_LEN, bandwidth, test->settings->unit_format);
        	if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
        		iprintf(test, iperf_report_bw_format, sp->socket, start_time, end_time, ubuf, nbuf, iperf_report_receiver);
        	}
        }
    }

    if (test->num_streams > 1) {
        unit_snprintf(ubuf, UNIT_LEN, total_sent, 'A');
        
    	/* If no tests were run, arbitrariliy set bandwidth to 0. */
    	if (end_time > 0) {
    	    bandwidth = total_sent / end_time;
    	}else {
    	    bandwidth = 0;
    	}
        unit_snprintf(nbuf, UNIT_LEN, bandwidth, test->settings->unit_format);
        if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
    	    if (test->sender_has_retransmits) {
    		    /* Summary sum, TCP with retransmits. */
    		    iprintf(test, iperf_report_sum_bw_retrans_format, start_time, end_time, ubuf, nbuf, total_retransmits, iperf_report_sender);
    	    } else {
    		    /* Summary sum, TCP without retransmits. */
    		    iprintf(test, iperf_report_sum_bw_format, start_time, end_time, ubuf, nbuf, iperf_report_sender);
    	    }
            unit_snprintf(ubuf, UNIT_LEN, total_received, 'A');
    	    /* If no tests were run, set received bandwidth to 0 */
    	    if (end_time > 0) {
    		    bandwidth = total_received / end_time;
    	    }
    	    else {
    		    bandwidth = 0;
    	    }
            unit_snprintf(nbuf, UNIT_LEN, bandwidth, test->settings->unit_format);
    		iprintf(test, iperf_report_sum_bw_format, start_time, end_time, ubuf, nbuf, iperf_report_receiver);
        } else {
    	    /* Summary sum, UDP. */
            avg_jitter /= test->num_streams;
                
    	    /* If no packets were sent, arbitrarily set loss percentage to 100. */
    	    if (total_packets > 0) {
        		lost_percent = 100 * lost_packets / total_packets;
    	    }
    	    else {
        		lost_percent = 0;
    	    }
    		iprintf(test, iperf_report_sum_bw_udp_format, start_time, end_time, ubuf, nbuf, avg_jitter, lost_packets, total_packets, lost_percent, "");
        }
    }

	/* Print server output if we're on the client and it was requested/provided */
	if (test->role == 'c' && iperf_get_test_get_server_output(test)) {
	    if (test->json_server_output) {
    		iprintf(test, "\nServer JSON output:\n%s\n", cJSON_Print(test->json_server_output));
    		cJSON_Delete(test->json_server_output);
    		test->json_server_output = NULL;
    	}
	    if (test->server_output_text) {
    		iprintf(test, "\nServer output:\n%s\n", test->server_output_text);
    		test->server_output_text = NULL;
	    }
    }
}

/**************************************************************************/

/**
 * Main report-printing callback.
 * Prints results either during a test (interval report only) or 
 * after the entire test has been run (last interval report plus 
 * overall summary).
 */
void
iperf_reporter_callback(struct iperf_test *test)
{
    switch (test->state) {
        case TEST_RUNNING:
        case STREAM_RUNNING:
            /* print interval results for each stream */
            iperf_print_intermediate(test);
            break;
        case DISPLAY_RESULTS:
            if (test->role == 'c')
                iperf_print_intermediate(test);
            iperf_print_results(test);
            break;
    } 
}

/**
 * Print the interval results for one stream.
 * This function needs to know about the overall test so it can determine the
 * context for printing headers, separators, etc.
 */
static void
print_interval_results(struct iperf_test *test, struct iperf_stream *sp, cJSON *json_interval_streams)
{
    char ubuf[UNIT_LEN];
    char nbuf[UNIT_LEN];
    char cbuf[UNIT_LEN];
    u32  st = 0, et = 0.;
    struct iperf_interval_results *irp = NULL;
    u32  bandwidth, lost_percent;

    irp = TAILQ_LAST(&sp->result->interval_results, irlisthead); /* get last entry in linked list */
    if (irp == NULL) {
    	PRINTF("print_interval_results error: interval_results is NULL \n");
        return;
    }
	/* First stream? */
	if (sp == SLIST_FIRST(&test->streams)) {
	    /* It it's the first interval, print the header;
    	    ** else if there's more than one stream, print the separator;
    	    ** else nothing.
    	    */
	    if (test->periods == 1){
    		if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
    		    if (test->sender && test->sender_has_retransmits)
        			iprintf(test, "%s", iperf_report_bw_retrans_cwnd_header);
    		    else
        			iprintf(test, "%s", iperf_report_bw_header);
    		} else {
    		    if (test->sender)
        			iprintf(test, "%s", iperf_report_bw_udp_sender_header);
    		    else
        			iprintf(test, "%s", iperf_report_bw_udp_header);
    		}
	    } else if (test->num_streams > 1)
    		iprintf(test, "%s", iperf_report_bw_separator);
    }

    unit_snprintf(ubuf, UNIT_LEN, irp->bytes_transferred, 'A');
    bandwidth = irp->bytes_transferred / irp->interval_duration;
    unit_snprintf(nbuf, UNIT_LEN, bandwidth, test->settings->unit_format);
    
    st = (irp->end_time-1) * test->stats_interval;
    et = irp->end_time * test->stats_interval;
    if (test->protocol->id == Ptcp || test->protocol->id == Psctp) {
    	if (test->sender && test->sender_has_retransmits) {
    	    /* Interval, TCP with retransmits. */
    		unit_snprintf(cbuf, UNIT_LEN, irp->snd_cwnd, 'A');
    		iprintf(test, iperf_report_bw_retrans_cwnd_format, sp->socket, st, et, ubuf, nbuf, irp->interval_retrans, cbuf, irp->omitted?iperf_report_omitted:"");
    	} else {
    	    /* Interval, TCP without retransmits. */
    		iprintf(test, iperf_report_bw_format, sp->socket, st, et, ubuf, nbuf, irp->omitted?iperf_report_omitted:"");
    	}
    } else {
    	/* Interval, UDP. */
    	if (test->sender) {
    		iprintf(test, iperf_report_bw_udp_sender_format, sp->socket, st, et, ubuf, nbuf, irp->interval_packet_count, irp->omitted?iperf_report_omitted:"");
    	} else {
    	    if (irp->interval_packet_count){
    	        lost_percent = 100 * irp->interval_cnt_error / irp->interval_packet_count;
	        }else{
    	        lost_percent = 100;
    	        irp->jitter = 0;
	        }
    		iprintf(test, iperf_report_bw_udp_format, sp->socket, st, et, ubuf, nbuf, irp->jitter, irp->interval_cnt_error, irp->interval_packet_count, lost_percent, irp->omitted?iperf_report_omitted:"");
    	}
    }
}

/**************************************************************************/
void
iperf_free_stream(struct iperf_stream *sp)
{
    struct iperf_interval_results *irp, *nirp;

    if (sp->buffer)
        iperf_free(sp->buffer);
    /* XXX: need to free interval list too! */
    for (irp = TAILQ_FIRST(&sp->result->interval_results); irp != TAILQ_END(sp->result->interval_results); irp = nirp) {
        nirp = TAILQ_NEXT(irp, irlistentries);
        iperf_free(irp);
    }
    if (sp->result)
        iperf_free(sp->result);
    if (sp->send_timer != NULL)
    	tmr_cancel(sp->send_timer, sp->test);

    if (sp->socket >= 0)
        close(sp->socket);

    iperf_free(sp);
}

/**************************************************************************/
struct iperf_stream *
iperf_new_stream(struct iperf_test *test, int s)
{
    struct iperf_stream *sp;

    h_errno = 0;

    sp = (struct iperf_stream *) iperf_malloc(sizeof(struct iperf_stream));
    if (!sp) {
        i_errno = IECREATESTREAM;
        return NULL;
    }

    MEMSET(sp, 0, sizeof(struct iperf_stream));

    sp->test = test;
    sp->settings = test->settings;
    sp->result = (struct iperf_stream_result *) iperf_malloc(sizeof(struct iperf_stream_result));
    if (!sp->result) {
        iperf_free(sp);
        i_errno = IECREATESTREAM;
        return NULL;
    }

    sp->jitter = 0;
    MEMSET(sp->result, 0, sizeof(struct iperf_stream_result));
    TAILQ_INIT(&sp->result->interval_results);
    
    /* Create and randomize the buffer */
    sp->buffer = (char *) iperf_malloc(test->settings->blksize);
    if (sp->buffer == NULL)
    {
        iperf_free(sp->result);
        iperf_free(sp);
        i_errno = IECREATESTREAM;
        return NULL;
    }
    MEMSET(sp->buffer, 'A', test->settings->blksize);

    /* Set socket */
    sp->socket = s;

    sp->snd = test->protocol->sock_send;
    sp->rcv = test->protocol->sock_recv;

    /* Initialize stream */
    if (iperf_init_stream(sp, test) < 0) {
        iperf_free (sp->buffer);
        iperf_free(sp->result);
        iperf_free(sp);
        return NULL;
    }
    iperf_add_stream(test, sp);

    return sp;
}

/**************************************************************************/
int
iperf_init_stream(struct iperf_stream *sp, struct iperf_test *test)
{
    socklen_t len;
    int opt;

    //len = sizeof(struct sockaddr_storage); 
    len = sizeof(struct sockaddr_in); 
    if (getsockname(sp->socket, (struct sockaddr *) &sp->local_addr, &len) < 0) {
        i_errno = IEINITSTREAM;
        return -1;
    }
    //len = sizeof(struct sockaddr_storage);
    len = sizeof(struct sockaddr_in);
    if (getpeername(sp->socket, (struct sockaddr *) &sp->remote_addr, &len) < 0) {
        i_errno = IEINITSTREAM;
        return -1;
    }

    /* Set IPv4 TOS */
    if ((opt = test->settings->tos)) {
        if (setsockopt(sp->socket, IPPROTO_IP, IP_TOS, &opt, sizeof(opt)) < 0) {
            i_errno = IESETTOS;
            return -1;
        }
    }

    return 0;
}

/**************************************************************************/
void
iperf_add_stream(struct iperf_test *test, struct iperf_stream *sp)
{
    int i;
    struct iperf_stream *n, *prev;

    if (SLIST_EMPTY(&test->streams)) {
        SLIST_INSERT_HEAD(&test->streams, sp, streams);
        sp->id = 1;
    } else {
        // for (n = test->streams, i = 2; n->next; n = n->next, ++i);
        i = 2;
        SLIST_FOREACH(n, &test->streams, streams) {
            prev = n;
            ++i;
        }
        SLIST_INSERT_AFTER(prev, sp, streams);
        sp->id = i;
    }
}


int
iprintf(struct iperf_test *test, const char* format, ...)
{
    va_list argp;
    int r = -1;
   	char linebuffer[1024];

    /*
     * There are roughly two use cases here.  If we're the client,
     * want to print stuff directly to the output stream.
     * If we're the sender we might need to buffer up output to send
     * to the client.
     *
     * This doesn't make a whole lot of difference except there are
     * some chunks of output on the client (on particular the whole
     * of the server output with --get-server-output) that could
     * easily exceed the size of the line buffer, but which don't need
     * to be buffered up anyway.
     */
    if (test->role == 'c') {
    	if (test->title)
    	    PRINTF("%s:  ", test->title);
    	va_start(argp, format);
    	r = vsprintf(linebuffer, format, argp);
    	va_end(argp);
    	PRINTF("%s", linebuffer);
    }
    else if (test->role == 's') {
    	va_start(argp, format);
    	r = vsnprintf(linebuffer, sizeof(linebuffer), format, argp);
    	va_end(argp);
    	PRINTF("%s", linebuffer);
    	if (test->role == 's' && iperf_get_test_get_server_output(test)) {
    	    struct iperf_textline *l = (struct iperf_textline *) iperf_malloc(sizeof(struct iperf_textline));
    	    if (l != NULL){
        	    l->line = iperf_strdup(linebuffer);
        	    TAILQ_INSERT_TAIL(&(test->server_output_list), l, textlineentries);
    	    }
    	}
    }
    return r;
}
