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
//#include <unistd.h>
//#include <errno.h>
//#include <sys/types.h>
//#include <sys/errno.h>
//#include <assert.h>
//#include <string.h>

#include <os_wrapper.h>
#include <rtos.h>
#include <log.h>
#include "common.h"

#include "lwip/sockets.h"
#include "lwip/tcp.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"

#include "iperf_util.h"
#include "net.h"
#include "timer.h"


/* netdial and netannouce code comes from libtask: http://swtch.com/libtask/
 * Copyright: http://swtch.com/libtask/COPYRIGHT
*/

/* make connection to server */
int
netdial(int domain, int proto, char *local, int local_port, char *server, int port)
{
    struct addrinfo hints, *local_res, *server_res;
    int s;

    if (local) {
        MEMSET(&hints, 0, sizeof(hints));
        hints.ai_family = domain;
        hints.ai_socktype = proto;
        if (getaddrinfo(local, NULL, &hints, &local_res) != 0)
            return -1;
    }

    MEMSET(&hints, 0, sizeof(hints));
    hints.ai_family = domain;
    hints.ai_socktype = proto;
    if (getaddrinfo(server, NULL, &hints, &server_res) != 0)
        return -1;

    s = socket(server_res->ai_family, proto, 0);
    if (s < 0) {
    	if (local)
    	    freeaddrinfo(local_res);
    	freeaddrinfo(server_res);
        return -1;
    }

    if (local) {
        if (local_port) {
            struct sockaddr_in *lcladdr;
            lcladdr = (struct sockaddr_in *)local_res->ai_addr;
            lcladdr->sin_port = htons(local_port);
            local_res->ai_addr = (struct sockaddr *)lcladdr;
        }

        if (bind(s, (struct sockaddr *) local_res->ai_addr, local_res->ai_addrlen) < 0) {
    	    close(s);
    	    freeaddrinfo(local_res);
    	    freeaddrinfo(server_res);
            return -1;
    	}
        freeaddrinfo(local_res);
    }

    ((struct sockaddr_in *) server_res->ai_addr)->sin_port = htons(port);
    if (connect(s, (struct sockaddr *) server_res->ai_addr, server_res->ai_addrlen) < 0 && errno != EINPROGRESS) {
    	close(s);
    	freeaddrinfo(server_res);
        return -1;
    }

    freeaddrinfo(server_res);
    return s;
}

/***************************************************************/

int
netannounce(int domain, int proto, char *local, int port)
{
    struct addrinfo hints, *res;
    char portstr[6];
    int s, opt;

    sprintf(portstr, "%d", port);
    MEMSET(&hints, 0, sizeof(hints));
    /* 
     * If binding to the wildcard address with no explicit address
     * family specified, then force us to get an AF_INET6 socket.  On
     * CentOS 6 and MacOS, getaddrinfo(3) with AF_UNSPEC in ai_family,
     * and ai_flags containing AI_PASSIVE returns a result structure
     * with ai_family set to AF_INET, with the result that we create
     * and bind an IPv4 address wildcard address and by default, we
     * can't accept IPv6 connections.
     *
     * On FreeBSD, under the above circumstances, ai_family in the
     * result structure is set to AF_INET6.
     */
    if (domain == AF_UNSPEC && !local) {
    	return -1;
    }
    else {
    	hints.ai_family = domain;
    }
    hints.ai_socktype = proto;
    //hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(local, portstr, &hints, &res) != 0)
        return -2; 

    s = socket(res->ai_family, proto, 0);
    if (s < 0) {
    	freeaddrinfo(res);
        return -3;
    }

    opt = 1;
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 
		   (char *) &opt, sizeof(opt)) < 0) {
    	close(s);
    	freeaddrinfo(res);
    	return -4;
    }

    res->ai_addr->sa_data[2]=res->ai_addr->sa_data[3]=res->ai_addr->sa_data[4]=res->ai_addr->sa_data[5]=0;
    if (bind(s, (struct sockaddr *) res->ai_addr, res->ai_addrlen) < 0) {
        close(s);
    	freeaddrinfo(res);
        return -5;
    }

    freeaddrinfo(res);
    
    if (proto == SOCK_STREAM) {
        if (listen(s, 5) < 0) {
    	    close(s);
            return -6;
        }
    }

    return s;
}


/*******************************************************************/
/* reads 'count' bytes from a socket  */
/********************************************************************/

int
Nread(int fd, char *buf, size_t count, int prot)
{
    register int r;
    register size_t nleft = count;

    while (nleft > 0) {
        r = read(fd, buf, nleft);
    	//printf("Nread: fd:%d, buf[0]=%c[%d], size:%d, read return:%d, errno:%d, (count-nleft):%d \n", fd, buf[0], buf[0], count, r, errno, count-nleft);
        if (r < 0) {
            if (errno == EINTR || errno == EAGAIN)
                break;
            else
                return NET_HARDERROR;
        } else if (r == 0)
            break;

        nleft -= r;
        buf += r;
    }
    return count - nleft;
}


/*
 *                      N W R I T E
 */

int
Nwrite(int fd, const char *buf, size_t count, int prot)
{
    register int r;
    register size_t nleft = count;

    while (nleft > 0) {
    	r = write(fd, buf, nleft);
    	//printf("Nwrite: fd:%d, buf[0]=%c[%d], size:%d, write return:%d, errno:%d, (count-nleft):%d \n", fd, buf[0], buf[0], count, r, errno, count-nleft);
    	if (r < 0) {
    	    switch (errno) {
    		case EINTR:
    		case EAGAIN:
        		//return count - nleft;

    		case ENOBUFS:
        		return NET_SOFTERROR;

    		default:
        		return NET_HARDERROR;
    	    }
    	} else if (r == 0)
    	    return NET_SOFTERROR;
    	nleft -= r;
    	buf += r;
    }
    return count;
}


int
setnonblocking(int fd, int nonblocking)
{
    int flags, newflags;

    //printf("setnonblocking: fd:%d, flag:%d \n", fd, nonblocking);
    flags = lwip_fcntl(fd, F_GETFL, 0);
    if (flags < 0) {
        PRINTF("fcntl(F_GETFL)");
        return -1;
    }
    if (nonblocking)
    	newflags = flags | (int) O_NONBLOCK;
    else
    	newflags = flags & ~((int) O_NONBLOCK);
    if (newflags != flags)
    	if (lwip_fcntl(fd, F_SETFL, newflags) < 0) {
    	    PRINTF("lwip_fcntl(F_SETFL)");
    	    return -1;
    	}
    return 0;
}

/****************************************************************************/

int
getsockdomain(int sock)
{
    struct sockaddr sa;
    socklen_t len = sizeof(sa);

    if (getsockname(sock, &sa, &len) < 0)
    	return -1;
    
    return sa.sa_family;
}
