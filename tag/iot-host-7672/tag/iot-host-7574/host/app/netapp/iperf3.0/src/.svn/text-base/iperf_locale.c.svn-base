/*--------------------------------------------------------------- 
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
 *
 * Based on code that is:
 *
 * Copyright (c) 1999,2000,2001,2002,2003
 * The Board of Trustees of the University of Illinois            
 * All Rights Reserved.                                           
 *--------------------------------------------------------------- 
 * Permission is hereby granted, free of charge, to any person    
 * obtaining a copy of this software (Iperf) and associated       
 * documentation files (the "Software"), to deal in the Software  
 * without restriction, including without limitation the          
 * rights to use, copy, modify, merge, publish, distribute,        
 * sublicense, and/or sell copies of the Software, and to permit     
 * persons to whom the Software is furnished to do
 * so, subject to the following conditions: 
 *
 *     
 * Redistributions of source code must retain the above 
 * copyright notice, this list of conditions and 
 * the following disclaimers. 
 *
 *     
 * Redistributions in binary form must reproduce the above 
 * copyright notice, this list of conditions and the following 
 * disclaimers in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 *     
 * Neither the names of the University of Illinois, NCSA, 
 * nor the names of its contributors may be used to endorse 
 * or promote products derived from this Software without
 * specific prior written permission. 
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES 
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE CONTIBUTORS OR COPYRIGHT 
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
 * ________________________________________________________________
 * National Laboratory for Applied Network Research 
 * National Center for Supercomputing Applications 
 * University of Illinois at Urbana-Champaign 
 * http://www.ncsa.uiuc.edu
 * ________________________________________________________________ 
 *
 * Locale.c
 * by Ajay Tirumala <tirumala@ncsa.uiuc.edu>
 * & Mark Gates <mgates@nlanr.net>
 * -------------------------------------------------------------------
 * Strings and other stuff that is locale specific.
 * ------------------------------------------------------------------- */
#include "common.h"
#include "iperf_config.h"

#ifdef __cplusplus
extern    "C"
{
#endif


/* -------------------------------------------------------------------
 * usage
 * ------------------------------------------------------------------- */

const char iperf_version[] = "iperf 3.0.1";

/* -------------------------------------------------------------------
 * settings
 * ------------------------------------------------------------------- */

const char iperf_test_start_time[] =
"Starting Test: protocol: %s, %d streams, %d byte blocks, omitting %d seconds, %d second test\n";

const char iperf_test_start_bytes[] =
"Starting Test: protocol: %s, %d streams, %d byte blocks, omitting %d seconds, %llu bytes to send\n";

const char iperf_test_start_blocks[] =
"Starting Test: protocol: %s, %d streams, %d byte blocks, omitting %d seconds, %d blocks to send\n";


/* -------------------------------------------------------------------
 * reports
 * ------------------------------------------------------------------- */
const char iperf_report_done[] =
    "iperf Done.\n";
    

const char iperf_report_connecting[] =
"Connecting to host %s, port %d\n";

const char iperf_report_reverse[] =
"Reverse mode, remote host %s is sending\n";

const char iperf_report_accepted[] =
"Accepted connection from %s, port %d\n";

const char iperf_report_cookie[] =
"      Cookie: %s\n";

const char iperf_report_connected[] =
"[%3d] local %s port %d connected to %s port %d\n";

const char iperf_report_bw_header[] =
"[ ID] Interval           Transfer     Bandwidth\n";

const char iperf_report_bw_retrans_header[] =
"[ ID] Interval           Transfer     Bandwidth       Retr\n";

const char iperf_report_bw_retrans_cwnd_header[] =
"[ ID] Interval           Transfer     Bandwidth       Retr  Cwnd\n";

const char iperf_report_bw_udp_header[] =
"[ ID] Interval           Transfer     Bandwidth       Jitter    Lost/Total Datagrams\n";

const char iperf_report_bw_udp_sender_header[] =
"[ ID] Interval           Transfer     Bandwidth       Total Datagrams\n";

const char iperf_report_bw_format[] =
"[%3d] %3d.00-%3d.00 sec  %ss  %ss/sec                  %s\n";

const char iperf_report_bw_retrans_format[] =
"[%3d] %3d.00-%3d.00 sec  %ss  %ss/sec  %3u             %s\n";

const char iperf_report_bw_retrans_cwnd_format[] =
"[%3d] %3d.00-%3d.00 sec  %ss  %ss/sec  %3u   %ss       %s\n";

const char iperf_report_bw_udp_sender_format[] =
"[%3d] %3d.00-%3d.00 sec  %ss  %ss/sec  %d  %s\n";

const char iperf_report_summary[] =
"Test Complete. Summary Results:\n";

const char iperf_report_sum_bw_format[] =
"[SUM] %3d.00-%3d.00 sec  %ss  %ss/sec                  %s\n";

const char iperf_report_sum_bw_retrans_format[] =
"[SUM] %3d.00-%3d.00 sec  %ss  %ss/sec  %3d             %s\n";

#ifdef IPERF_SUPPORT_DOUBLE
const char iperf_report_bw_udp_format[] =
"[%3d] %3d.00-%3d.00 sec  %ss  %ss/sec  %5.3f ms  %d/%d (%d%%)  %s\n";

const char iperf_report_sum_bw_udp_format[] =
"[SUM] %3d.00-%3d.00 sec  %ss  %ss/sec  %5.3f ms  %d/%d (%d%%)  %s\n";

#else

const char iperf_report_bw_udp_format[] =
"[%3d] %3d.00-%3d.00 sec  %ss  %ss/sec  %3d ms  %d/%d (%d%%)  %s\n";

const char iperf_report_sum_bw_udp_format[] =
"[SUM] %3d.00-%3d.00 sec  %ss  %ss/sec  %3d ms  %d/%d (%d%%)  %s\n";
#endif

const char iperf_report_sum_bw_udp_sender_format[] =
"[SUM] %3d.00-%3d.00 sec  %ss  %ss/sec  %d  %s\n";

const char iperf_report_omitted[] = "(omitted)";

const char iperf_report_bw_separator[] =
"- - - - - - - - - - - - - - - - - - - - - - - - -\n";

const char iperf_report_sum_outoforder[] =
"[SUM] %3d.00-%3d.00 sec  %d datagrams received duplicate, %d datagrams lost\n";

const char iperf_report_datagrams[] =
"[%3d] Sent %d datagrams\n";

const char iperf_report_sender[] = "sender";
const char iperf_report_receiver[] = "receiver";

#ifdef __cplusplus
} /* end extern "C" */
#endif
