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
 * This code is distributed under a BSD style license, see the LICENSE
 * file for complete information.
 */
#include "iperf_config.h"

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <errno.h>
//#include <unistd.h>
//#include <sys/types.h>
//#include <sys/time.h>

#include <os_wrapper.h>
#include <rtos.h>
#include <log.h>
#include "common.h"

#include "lwip/netdb.h"
#include "lwip/tcp.h"
#include "lwip/sockets.h"

#include "iperf.h"
#include "iperf_api.h"
#include "iperf_tcp.h"
#include "net.h"


/* iperf_tcp_recv
 *
 * receives the data for TCP
 */
int
iperf_tcp_recv(struct iperf_stream *sp)
{
    int r;

    r = Nread(sp->socket, sp->buffer, sp->settings->blksize, Ptcp);

    if (r < 0)
        return r;

    sp->result->bytes_received += r;
    sp->result->bytes_received_this_interval += r;

    return r;
}


/* iperf_tcp_send 
 *
 * sends the data for TCP
 */
int
iperf_tcp_send(struct iperf_stream *sp)
{
    int r;

	r = Nwrite(sp->socket, sp->buffer, sp->settings->blksize, Ptcp);
    if (r < 0)
        return r;

    sp->result->bytes_sent += r;
    sp->result->bytes_sent_this_interval += r;

    return r;
}


/* iperf_tcp_accept
 *
 * accept a new TCP stream connection
 */
int
iperf_tcp_accept(struct iperf_test * test)
{
    int     s;
    signed char rbuf = ACCESS_DENIED;
    char    cookie[COOKIE_SIZE];
    socklen_t len;
    //struct sockaddr_storage addr;
    struct sockaddr_in addr;

    len = sizeof(addr);
    if ((s = accept(test->listener, (struct sockaddr *) &addr, &len)) < 0) {
        i_errno = IESTREAMCONNECT;
        return -1;
    }

    if (Nread(s, cookie, COOKIE_SIZE, Ptcp) < 0) {
        i_errno = IERECVCOOKIE;
        return -1;
    }

    if (STRCMP(test->cookie, cookie) != 0) {
        if (Nwrite(s, (char*) &rbuf, sizeof(rbuf), Ptcp) < 0) {
            i_errno = IESENDMESSAGE;
            return -1;
        }
        close(s);
        s = -1;
    }

    return s;
}


/* iperf_tcp_listen
 *
 * start up a listener for TCP stream connections
 */
int
iperf_tcp_listen(struct iperf_test *test)
{
    struct addrinfo hints, *res;
    char portstr[6];
    int s, opt;
    u32_t optlen = sizeof(opt);
    int saved_errno;

    s = test->listener;

    /*
     * If certain parameters are specified (such as socket buffer
     * size), then throw away the listening socket (the one for which
     * we just accepted the control connection) and recreate it with
     * those parameters.  That way, when new data connections are
     * set, they'll have all the correct parameters in place.
     *
     * It's not clear whether this is a requirement or a convenience.
     */
    if (test->no_delay || test->settings->mss || test->settings->socket_bufsize) {
        FD_CLR(s, &test->read_set);
        close(s);

        sprintf(portstr, "%d", test->server_port);
        MEMSET(&hints, 0, sizeof(hints));

        /*
        	 * If binding to the wildcard address with no explicit address
        	 * family specified, then force us to get an AF_INET6 socket.
        	 * More details in the comments in netanounce().
        	 */
    	if (test->settings->domain == AF_UNSPEC && !test->bind_address) {
            i_errno = IEV4ONLY;
            return -1;
    	}
    	else {
    	    hints.ai_family = test->settings->domain;
    	}
        
        hints.ai_socktype = SOCK_STREAM;
        //hints.ai_flags = AI_PASSIVE;
        if (getaddrinfo(test->bind_address, portstr, &hints, &res) != 0) {
            i_errno = IESTREAMLISTEN;
            return -1;
        }

        if ((s = socket(res->ai_family, SOCK_STREAM, 0)) < 0) {
    	    freeaddrinfo(res);
            i_errno = IESTREAMLISTEN;
            return -1;
        }

        if (test->no_delay) {
            opt = 1;
            if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        		saved_errno = errno;
        		close(s);
        		freeaddrinfo(res);
        		errno = saved_errno;
                i_errno = IESETNODELAY;
                return -1;
            }
            if( getsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, &optlen) >= 0)
            {
                if( opt ){
                    if (test->debug){
                        PRINTF( " sock(%d) TCP_NODELAY : ENABLED \r\n", s);
                    }
                }
                else{
                    if (test->debug){
                        PRINTF( " sock(%d) TCP_NODELAY : DISABLED \r\n", s);
                    }
                }
            }
            else{
                if (test->debug){
                    PRINTF( " sock(%d) getsockopt failed \r\n", s);
                }
            }
        }
        if ((opt = test->settings->socket_bufsize)) {
            if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)) < 0) {
        		saved_errno = errno;
        		close(s);
        		freeaddrinfo(res);
        		errno = saved_errno;
                i_errno = IESETBUF;
                return -1;
            }
            /* unimplemented yet!
            if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt)) < 0) {
        		saved_errno = errno;
        		close(s);
        		freeaddrinfo(res);
        		errno = saved_errno;
                i_errno = IESETBUF;
                return -1;
            }*/
        }
    	if (test->debug) {
            /* unimplemented yet!
    	    socklen_t optlen = sizeof(opt);
    	    if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, &opt, &optlen) < 0) {
        		saved_errno = errno;
        		close(s);
        		freeaddrinfo(res);
        		errno = saved_errno;
        		i_errno = IESETBUF;
        		return -1;
    	    }
    	    printf("SO_SNDBUF is %u\n", opt);
    	    */
    	}
        
        opt = 1;
        if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    	    saved_errno = errno;
            close(s);
    	    freeaddrinfo(res);
    	    errno = saved_errno;
            i_errno = IEREUSEADDR;
            return -1;
        }

    	/*
    	 * If we got an IPv6 socket, figure out if it shoudl accept IPv4
    	 * connections as well.  See documentation in netannounce() for
    	 * more details.
    	 */
	    res->ai_addr->sa_data[2]=res->ai_addr->sa_data[3]=res->ai_addr->sa_data[4]=res->ai_addr->sa_data[5]=0; 
	    if (bind(s, (struct sockaddr *) res->ai_addr, res->ai_addrlen) < 0) {
            saved_errno = errno;
            close(s);
            freeaddrinfo(res);
            errno = saved_errno;
            i_errno = IESTREAMLISTEN;
            return -1;
        }

        freeaddrinfo(res);

        if (listen(s, 5) < 0) {
            i_errno = IESTREAMLISTEN;
            return -1;
        }

        test->listener = s;
    }
    
    return s;
}


/* iperf_tcp_connect
 *
 * connect to a TCP stream listener
 */
int
iperf_tcp_connect(struct iperf_test *test)
{
    struct addrinfo hints, *local_res, *server_res;
    char portstr[6];
    int s, opt;
    u32_t optlen = sizeof(opt);
    int saved_errno;

    if (test->bind_address) {
        MEMSET(&hints, 0, sizeof(hints));
        hints.ai_family = test->settings->domain;
        hints.ai_socktype = SOCK_STREAM;
        if (getaddrinfo(test->bind_address, NULL, &hints, &local_res) != 0) {
            i_errno = IESTREAMCONNECT;
            return -1;
        }
    }

    MEMSET(&hints, 0, sizeof(hints));
    hints.ai_family = test->settings->domain;
    hints.ai_socktype = SOCK_STREAM;
    sprintf(portstr, "%d", test->server_port);
    if (getaddrinfo(test->server_hostname, portstr, &hints, &server_res) != 0) {
    	if (test->bind_address)
    	    freeaddrinfo(local_res);
            i_errno = IESTREAMCONNECT;
            return -1;
        }

        if ((s = socket(server_res->ai_family, SOCK_STREAM, 0)) < 0) {
        	if (test->bind_address)
        	    freeaddrinfo(local_res);
            	freeaddrinfo(server_res);
                i_errno = IESTREAMCONNECT;
                return -1;
            }

        if (test->bind_address) {
            struct sockaddr_in *lcladdr;
            lcladdr = (struct sockaddr_in *)local_res->ai_addr;
            lcladdr->sin_port = htons(test->bind_port);
            local_res->ai_addr = (struct sockaddr *)lcladdr;

            if (bind(s, (struct sockaddr *) local_res->ai_addr, local_res->ai_addrlen) < 0) {
        	    saved_errno = errno;
        	    close(s);
        	    freeaddrinfo(local_res);
        	    freeaddrinfo(server_res);
        	    errno = saved_errno;
                i_errno = IESTREAMCONNECT;
                return -1;
            }
            freeaddrinfo(local_res);
        }

        /* Set socket options */
        if (test->no_delay) {
            opt = 1;
            if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt)) < 0) {
        	    saved_errno = errno;
        	    close(s);
        	    freeaddrinfo(server_res);
        	    errno = saved_errno;
                i_errno = IESETNODELAY;
                return -1;
            }
            
            if( getsockopt(s, IPPROTO_TCP, TCP_NODELAY, &opt, &optlen) >= 0)
            {
                if( opt ){
                    if (test->debug){
                        PRINTF( " sock(%d) TCP_NODELAY : ENABLED \r\n", s);
                    }
                }
                else{
                    if (test->debug){
                        PRINTF( " sock(%d) TCP_NODELAY : DISABLED \r\n", s);
                    }
                }
           }
           else{
               if (test->debug){
                   PRINTF( " sock(%d) getsockopt failed \r\n", s);
               }
            }
        }
        
        if ((opt = test->settings->socket_bufsize)) {
            if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)) < 0) {
        	    saved_errno = errno;
        	    close(s);
        	    freeaddrinfo(server_res);
        	    errno = saved_errno;
                i_errno = IESETBUF;
                return -1;
            }
            /* unimplemented yet!
            if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt)) < 0) {
        	    saved_errno = errno;
        	    close(s);
        	    freeaddrinfo(server_res);
        	    errno = saved_errno;
                i_errno = IESETBUF;
                return -1;
            }*/
        }
        if (test->debug) {
            /* unimplemented yet!
        	socklen_t optlen = sizeof(opt);
        	if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, &opt, &optlen) < 0) {
        	    saved_errno = errno;
        	    close(s);
        	    freeaddrinfo(server_res);
        	    errno = saved_errno;
        	    i_errno = IESETBUF;
        	    return -1;
        	}
        	printf("SO_SNDBUF is %u\n", opt);
        	*/
        }

        if (connect(s, (struct sockaddr *) server_res->ai_addr, server_res->ai_addrlen) < 0 && errno != EINPROGRESS) {
        	saved_errno = errno;
        	close(s);
        	freeaddrinfo(server_res);
        	errno = saved_errno;
            i_errno = IESTREAMCONNECT;
            return -1;
        }

        freeaddrinfo(server_res);

        /* Send cookie for verification */
        if (Nwrite(s, test->cookie, COOKIE_SIZE, Ptcp) < 0) {
    	saved_errno = errno;
    	close(s);
    	errno = saved_errno;
        i_errno = IESENDCOOKIE;
        return -1;
    }

    return s;
}
