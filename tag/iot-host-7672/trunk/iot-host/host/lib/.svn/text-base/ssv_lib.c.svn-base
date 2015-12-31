#include <config.h>
#include <stdarg.h>
#include <hdr80211.h>
#include <ssv_pktdef.h>



#if (CONFIG_HOST_PLATFORM == 0)
#include <uart/drv_uart.h>
#endif

#include <log.h>
//#include "lib-impl.h"
#include "ssv_lib.h"


#define CONSOLE_UART   SSV6XXX_UART0

// return  -1 : fail
//        >=0 : ok

void list_q_init(struct list_q *qhd)
{
    qhd->prev = (struct list_q *)qhd;
    qhd->next = (struct list_q *)qhd;
    qhd->qlen = 0;
}

void list_q_qtail(struct list_q *qhd, struct list_q *newq)
{
    struct list_q *next = qhd;
    struct list_q* prev = qhd->prev;

    newq->next = next;
    newq->prev = prev;
    next->prev = newq;
    prev->next = newq;
    qhd->qlen++;
}

void list_q_insert(struct list_q *qhd, struct list_q *prev, struct list_q *newq)
{
    struct list_q *next = prev->next;

    newq->next = next;
    newq->prev = prev;
    next->prev = newq;
    prev->next = newq;
    qhd->qlen++;
}

void list_q_remove(struct list_q *qhd,struct list_q *curt)
{
    struct list_q *next = curt->next;
    struct list_q *prev = curt->prev;

    prev->next = next;
    next->prev = prev;
    qhd->qlen--;
}

struct list_q * list_q_deq(struct list_q *qhd)
{
    struct list_q *next, *prev;
    struct list_q *elm = qhd->next;

    if((qhd->qlen > 0) && (elm != NULL))
    {
        qhd->qlen--;
        next        = elm->next;
        prev        = elm->prev;
        elm->next   = NULL;
        elm->prev   = NULL;
        next->prev  = prev;
        prev->next  = next;

        return elm;
    }else{
        return NULL;
    }
}
unsigned int list_q_len(struct list_q *qhd)
{
    return qhd->qlen;
}

u32 list_q_len_safe(struct list_q *q, OsMutex *pmtx)
{ 
    u32 len = 0;
    OS_MutexLock(*pmtx);
    len = q->qlen;
    OS_MutexUnLock(*pmtx);
    return len;
}

void list_q_qtail_safe(struct list_q *qhd, struct list_q *newq, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    list_q_qtail(qhd, newq);
    OS_MutexUnLock(*pmtx);
}

struct list_q * list_q_deq_safe(struct list_q *qhd, OsMutex *pmtx)
{
    struct list_q *_list = NULL;
    OS_MutexLock(*pmtx);
    _list = list_q_deq(qhd);
    OS_MutexUnLock(*pmtx);
    return _list;
}

void list_q_insert_safe(struct list_q *qhd, struct list_q *prev, struct list_q *newq, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    list_q_insert(qhd, prev, newq);
    OS_MutexUnLock(*pmtx);
}

void list_q_remove_safe(struct list_q *qhd,struct list_q *curt, OsMutex *pmtx)
{
    OS_MutexLock(*pmtx);
    list_q_remove(qhd, curt);
    OS_MutexUnLock(*pmtx);
}

LIB_APIs s32 ssv6xxx_strrpos(const char *str, char delimiter)
{
	const char *p;

	for (p = (str + strlen(str)) - 1; (s32)p>=(s32)str; p--)
	{
		if (*p == delimiter)
			return ((s32)p - (s32)str);
	}
	
	return -1;	// find no matching delimiter

}

LIB_APIs s32	ssv6xxx_isalpha(s32 c)
{
	if (('A' <= c) && (c <= 'Z'))
		return 1;
	if (('a' <= c) && (c <= 'z'))
		return 1;
	return 0;
}

LIB_APIs s32 ssv6xxx_str_toupper(s8 *s)
{
	while (*s)
	{
		*s = ssv6xxx_toupper(*s);
		s++;
	}
	return 0;
}

LIB_APIs s32 ssv6xxx_str_tolower(s8 *s)
{
	while (*s)
	{
		*s = ssv6xxx_tolower(*s);
		s++;
	}
	return 0;
}

LIB_APIs u32 ssv6xxx_atoi_base( const s8 *s, u32 base )
{
    u32  index, upbound=base-1;
    u32  value = 0, v;

    while( (v = (u8)*s) != 0 ) {
        index = v - '0';
        if ( index > 10 && base==16 ) {
            index = (v >= 'a') ? (v - 'a') : (v - 'A');
            index += 10;
        }
        if ( index > upbound )
            break;
        value = value * base + index;
        s++;
    }
    
    return value;
}

LIB_APIs s32 ssv6xxx_atoi( const s8 *s )
{
    u32 neg=0, value, base=10;

    if ( *s=='0' ) {
        switch (*++s) {
        case 'x':
        case 'X': base = 16; break;
        case 'b':
        case 'B': base = 2; break;
        default: return 0;
        }
        s++;
    }
    else if ( *s=='-' ) {
        neg = 1;
        s++;
    }

    value = ssv6xxx_atoi_base(s, base);
    
    if ( neg==1 )
        return -(s32)value;
    return (s32)value;

}


#if (CONFIG_HOST_PLATFORM == 1)
u64 ssv6xxx_64atoi( s8 *s )
{
    u8 bchar='A', index, upbound=9;
    u32 neg=0, value=0, base=10;

    if ( *s=='0' ) {
        switch (*++s) {
                case 'x': bchar = 'a';
                case 'X': base = 16; upbound = 15; break;
                case 'b':
                case 'B': base = 2; upbound = 1; break;
                default: return 0;
        }
        s++;
    }
    else if ( *s=='-' ) {
        neg = 1;
        s++;
    }

    while( *s ) {
        index = *s - '0';
        if ( base==16 && (*s>=bchar) && (*s<=(bchar+5)) )
        {
                index = 10 + *s - bchar;
        }
        if ( index > upbound )
        {
                break;
        }
        value = value * base + index;
        s++;
    }

    if ( neg==1 )
        return -(s32)value;
    return (u64)value;

}
#endif




LIB_APIs s8 ssv6xxx_toupper(s8 ch)
{
	if (('a' <= ch) && (ch <= 'z'))
		return ('A' + (ch - 'a'));

	// else, make the original ch unchanged
	return ch;
}

LIB_APIs s8 ssv6xxx_tolower(s8 ch)
{
	if (('A' <= ch) && (ch <= 'Z'))
		return ('a' + (ch - 'A'));

	// else, make the original ch unchanged
	return ch;
}

LIB_APIs s32 ssv6xxx_isupper(s8 ch)
{
    return (ch >= 'A' && ch <= 'Z');
}

LIB_APIs s32 ssv6xxx_strcmp( const s8 *s0, const s8 *s1 )
{
    s32 c1, c2;

    do {
        c1 = (u8) *s0++;
        c2 = (u8) *s1++;
        if (c1 == '\0')
            return c1 - c2;
    } while (c1 == c2);

    return c1 - c2;
}

LIB_APIs s32 ssv6xxx_strncmp ( const s8 * s1, const s8 * s2, size_t n) 
{
  if ( !n )
      return(0);

  while (--n && *s1 && *s1 == *s2) { 
    s1++;
    s2++;
  }
  return( *s1 - *s2 );
}

LIB_APIs s8 *ssv6xxx_strcat(s8 *s, const s8 *append)
{   
    s8 *save = s;

    while (*s) { s++; }
    while ((*s++ = *append++) != 0) { }
    return(save);
}

LIB_APIs s8 *ssv6xxx_strncat(s8 *s, const s8 *append, size_t n)
{
     char* save = s;
     while(*s){ s++; }
     while((n--)&&((*s++ = *append++) != 0)){} 
     *s='\0';
     return(save);
}

/*Not considering the case of memory overlap*/
LIB_APIs s8 *ssv6xxx_strcpy(s8 *dst, const s8 *src)  
{  
    s8 *ret = dst; 
    assert(dst != NULL);  
    assert(src != NULL);  
    
    
    while((* dst++ = * src++) != '\0')   
        ;  
    return ret; 
}  

LIB_APIs s8 *ssv6xxx_strncpy(s8 *dst, const s8 *src, size_t n)
{
    register s8 *d = dst;
    register const s8 *s = src;
    
    if (n != 0) {
        do {
            if ((*d++ = *s++) == 0) {
                /* NUL pad the remaining n-1 bytes */
                while (--n != 0)
                *d++ = 0;
                break;
            }
        } while (--n != 0);
    }
    return (dst);
}


LIB_APIs size_t ssv6xxx_strlen(const s8 *s)
{
    const s8 *ptr = s;
    while (*ptr) ptr++;
    return (size_t)ptr-(size_t)s;
}

LIB_APIs s8 * ssv6xxx_strchr(const s8 * s, s8 c)
{
    const s8 * p = s;
    while(*p != c && *p)
        p++;
    return (*p=='\0' ? NULL : (char *)p);
}

LIB_APIs void *ssv6xxx_memset(void *s, s32 c, size_t n)
{
    if ( NULL != s ) {
		u8 * ps= (u8 *)s;
		const u8 * pes= ps+n;
        while( ps != pes )
			*(ps++) = (u8) c; 
    }
    return s;
}


LIB_APIs void *ssv6xxx_memcpy(void *dest, const void *src, size_t n)
{
    s8 *d = dest;
    s8 const *s = src;
    
    while (n-- > 0)
      *d++ = *s++;
    return dest;
}


LIB_APIs s32 ssv6xxx_memcmp(const void *s1, const void *s2, size_t n)
{
    const u8 *u1 = (const u8 *)s1, *u2 = (const u8 *)s2;

    while (n--) {
        s32 d = *u1++ - *u2++;
        if (d != 0)
            return d;
    }
    /*    
    for ( ; n-- ; s1++, s2++) {
        u1 = *(u8 *)s1;
        u2 = *(u8 *)s2;
        if (u1 != u2)
            return (u1-u2);
    } */
    return 0;
}

#if 0


//extern s32 gOsInFatal;
LIB_APIs void fatal_printf(const s8 *format, ...)
{
	
#if 0
   va_list args;
   

//   gOsInFatal = 1;
   /*lint -save -e530 */
   va_start( args, format );
   /*lint -restore */
   ret = print( 0, 0, format, args );
   va_end(args);
#endif
//    printf(format, ...);

   
}
#endif

#if 0
LIB_APIs void ssv6xxx_printf(const s8 *format, ...)
{
   va_list args;

   /*lint -save -e530 */
   va_start( args, format );
   /*lint -restore */

    printf(format, args);
//   ret = print( 0, 0, format, args );
   va_end(args);    
}


LIB_APIs void ssv6xxx_snprintf(char *out, size_t size, const char *format, ...)
{    
#if 0
    va_list args;
    s32     ret;
    /*lint -save -e530 */
    va_start( args, format ); /*lint -restore */
    ret = print( out, (out + size - 1), format, args );
    va_end(args);
#endif        
}

LIB_APIs void ssv6xxx_vsnprintf(char *out, size_t size, const char *format, va_list args)
{
#if 0
	return print( out, (out + size - 1), format, args );
#endif
}
#endif

//LIB_APIs s32 putstr (const char *s, size_t len)
//{
//    return  printstr(0, 0, s, len);
//}


//LIB_APIs s32 snputstr (char *out, size_t size, const char *s, size_t len)
//{
//    return  printstr( &out, (out + size - 1), s, len);
//}


//#endif


#if (CLI_ENABLE==1 && CONFIG_HOST_PLATFORM==0)
LIB_APIs s32 kbhit(void)
{
    return drv_uart_rx_ready(CONSOLE_UART);
}


LIB_APIs s32 getch(void)
{
    return drv_uart_rx(CONSOLE_UART);
}


LIB_APIs s32 putchar(s32 ch)
{
    return drv_uart_tx(CONSOLE_UART, ch);
}
#endif


#if 0
LIB_APIs void ssv6xxx_raw_dump(s8 *data, s32 len)
{
	ssv6xxx_raw_dump_ex(data, len, true, 10, 10, 16, LOG_LEVEL_ON, LOG_MODULE_EMPTY);
	return;
}


LIB_APIs bool ssv6xxx_raw_dump_ex(s8 *data, s32 len, bool with_addr, u8 addr_radix, s8 line_cols, u8 radix, u32 log_level, u32 log_module)
{
    s32 i;

	// check input parameters
	if ((addr_radix != 10) && (addr_radix != 16))
	{
		LOG_ERROR("%s(): invalid value 'addr_radix' = %d\n\r", __FUNCTION__, addr_radix);
		return false;
	}
	if ((line_cols != 8) && (line_cols != 10) && (line_cols != 16) && (line_cols != -1))
	{
		LOG_ERROR("%s(): invalid value 'line_cols' = %d\n\r", __FUNCTION__, line_cols);
		return false;
	}
	if ((radix != 10) && (radix != 16))
	{
		LOG_ERROR("%s(): invalid value 'radix' = %d\n\r", __FUNCTION__, radix);
		return false;
	}

	if (len == 0)	return true;

	// if ONLY have one line
	if (line_cols == -1)
	{
		LOG_TAG_SUPPRESS_ON();
		// only print addr heading at one time
		if ((with_addr == true))
		{
			if      (addr_radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%08d: ", 0);
			else if (addr_radix == 16)	LOG_PRINTF_LM(log_level, log_module, "0x%08x: ", 0);
		}

		for (i=0; i<len; i++) 
		{
			// print data
			if	    (radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%4d ",  (u8)data[i]);
			else if (radix == 16)	LOG_PRINTF_LM(log_level, log_module, "%02x ", (u8)data[i]);
		}
		LOG_PRINTF_LM(log_level, log_module, "\n\r");
		LOG_TAG_SUPPRESS_OFF();
		return true;
	}

	// normal case
	LOG_TAG_SUPPRESS_ON();
    for (i=0; i<len; i++) 
	{
		// print addr heading
		if ((with_addr == true) && (i % line_cols) == 0)
		{
			if      (addr_radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%08d: ", i);
			else if (addr_radix == 16)	LOG_PRINTF_LM(log_level, log_module, "0x%08x: ", i);
		}
		// print data
		if	    (radix == 10)	LOG_PRINTF_LM(log_level, log_module, "%4d ",  (u8)data[i]);
		else if (radix == 16)	LOG_PRINTF_LM(log_level, log_module, "%02x ", (u8)data[i]);
		// print newline
        if (((i+1) % line_cols) == 0)
            LOG_PRINTF_LM(log_level, log_module, "\n\r");
    }
    LOG_PRINTF_LM(log_level, log_module, "\n\r");
	LOG_TAG_SUPPRESS_OFF();
	return true;
}
#endif

#define ONE_RAW 16
void _packetdump(const char *title, const u8 *buf,
                             size_t len)
{
    size_t i;
    LOG_PRINTF("%s - hexdump(len=%d):\r\n    ", title, len);
    if (buf == NULL) {
        LOG_PRINTF(" [NULL]");
    }else{

        
        for (i = 0; i < ONE_RAW; i++)
            LOG_PRINTF("%02X ", i);
        
        LOG_PRINTF("\r\n---\r\n00|");
        
            
        for (i = 0; i < len; i++){                        
            LOG_PRINTF(" %02x", buf[i]);
            if((i+1)%ONE_RAW ==0)
                LOG_PRINTF("\r\n%02x|", (i+1));
        }
    }
    LOG_PRINTF("\r\n-----------------------------\r\n");
}



void pkt_dump_txinfo(PKT_TxInfo *p)	
{
	u32		payload_len;
	u8		*dat;
	u8		*a;

	LOG_PRINTF("========= TxInfo =========\n\r");
	LOG_PRINTF("%20s : %d\n\r", "len", 		p->len);
	LOG_PRINTF("%20s : %d\n\r", "c_type",		p->c_type);
	LOG_PRINTF("%20s : %d\n\r", "f80211",		p->f80211);
	LOG_PRINTF("%20s : %d\n\r", "qos",			p->qos);
	LOG_PRINTF("%20s : %d\n\r", "ht",			p->ht);
	LOG_PRINTF("%20s : %d\n\r", "use_4addr",	p->use_4addr);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_0",		p->RSVD_0);
	LOG_PRINTF("%20s : %d\n\r", "bc_que",		p->bc_que);	
	LOG_PRINTF("%20s : %d\n\r", "security",	p->security);
	LOG_PRINTF("%20s : %d\n\r", "more_data",	p->more_data);
	LOG_PRINTF("%20s : %d\n\r", "stype_b5b4",	p->stype_b5b4);
	LOG_PRINTF("%20s : %d\n\r", "extra_info",	p->extra_info);
	LOG_PRINTF("%20s : 0x%08x\n\r", "fCmd",	p->fCmd);
	LOG_PRINTF("%20s : %d\n\r", "hdr_offset",	p->hdr_offset);
	LOG_PRINTF("%20s : %d\n\r", "frag",		p->frag);
	LOG_PRINTF("%20s : %d\n\r", "unicast",		p->unicast);
	LOG_PRINTF("%20s : %d\n\r", "hdr_len",		p->hdr_len);
	LOG_PRINTF("%20s : %d\n\r", "tx_report",	p->tx_report);
	LOG_PRINTF("%20s : %d\n\r", "tx_burst",	p->tx_burst);
	LOG_PRINTF("%20s : %d\n\r", "ack_policy",  p->ack_policy);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_1",		p->RSVD_1);
	LOG_PRINTF("%20s : %d\n\r", "do_rts_cts",  p->do_rts_cts);
	LOG_PRINTF("%20s : %d\n\r", "reason",		p->reason);
	LOG_PRINTF("%20s : %d\n\r", "payload_offset",	p->payload_offset); 
	LOG_PRINTF("%20s : %d\n\r", "next_frag_pid",	p->next_frag_pid);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_2",		p->RSVD_2); 
	LOG_PRINTF("%20s : %d\n\r", "fCmdIdx",		p->fCmdIdx); 
	LOG_PRINTF("%20s : %d\n\r", "wsid",		p->wsid); 
	LOG_PRINTF("%20s : %d\n\r", "txq_idx",		p->txq_idx); 
	LOG_PRINTF("%20s : %d\n\r", "TxF_ID",		p->TxF_ID); 
	LOG_PRINTF("%20s : %d\n\r", "rts_cts_nav",	p->rts_cts_nav); 
	LOG_PRINTF("%20s : %d\n\r", "frame_consume_time",	p->frame_consume_time); 
	LOG_PRINTF("%20s : %d\n\r", "RSVD_3",		p->RSVD_3); 
	// printf("%20s : %d\n\r", "RSVD_5",		p->RSVD_5); 
	LOG_PRINTF("============================\n\r");
	payload_len = p->len - p->hdr_len;
	LOG_PRINTF("%20s : %d\n\r", "payload_len", payload_len); 

	dat = (u8 *)p + p->hdr_offset;
	LOG_PRINTF("========== hdr     ==========\n\r");
	LOG_PRINTF("frame ctl     : 0x%04x\n\r", (((u16)dat[1] << 8)|dat[0]));
	LOG_PRINTF("  - more_frag : %d\n\r", GET_HDR80211_FC_MOREFRAG(p)); 

	a = (u8*)p + p->hdr_offset +  4;
	LOG_PRINTF("address 1     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[0], a[1], a[2], a[3], a[4], a[5]);
	LOG_PRINTF("address 2     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[6], a[7], a[8], a[9], a[10], a[11]);
	LOG_PRINTF("address 3     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[12], a[13], a[14], a[15], a[16], a[17]);
	
	LOG_PRINTF("seq ctl       : 0x%04x\n\r", (((u16)dat[23] << 8)|dat[22]));
	LOG_PRINTF("  - seq num   : %d\n\r", GET_HDR80211_SC_SEQNUM(p));
	LOG_PRINTF("  - frag num  : %d\n\r", GET_HDR80211_SC_FRAGNUM(p));


	return;
}

void pkt_dump_rxinfo(PKT_RxInfo *p)
{
	u32		payload_len;
	u8		*dat;
	u8		*a;

	LOG_PRINTF("========= RxInfo =========\n\r");
	LOG_PRINTF("%20s : %d\n\r", "len",	        p->len);
	LOG_PRINTF("%20s : %d\n\r", "c_type",	    p->c_type);
	LOG_PRINTF("%20s : %d\n\r", "f80211",	    p->f80211);
	LOG_PRINTF("%20s : %d\n\r", "qos",	        p->qos);
	LOG_PRINTF("%20s : %d\n\r", "ht",		    p->ht);
	LOG_PRINTF("%20s : %d\n\r", "l3cs_err",	p->l3cs_err);
	LOG_PRINTF("%20s : %d\n\r", "l4cs_err",	p->l4cs_err);
	LOG_PRINTF("%20s : %d\n\r", "use_4addr",	p->use_4addr);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_0",		p->RSVD_0);
	LOG_PRINTF("%20s : %d\n\r", "psm",	        p->psm);
	LOG_PRINTF("%20s : %d\n\r", "stype_b5b4",	p->stype_b5b4);
	LOG_PRINTF("%20s : %d\n\r", "extra_info",	p->extra_info);	
	LOG_PRINTF("%20s : 0x%08x\n\r", "fCmd",	p->fCmd);
	LOG_PRINTF("%20s : %d\n\r", "hdr_offset",	p->hdr_offset);
	LOG_PRINTF("%20s : %d\n\r", "frag",	    p->frag);
	LOG_PRINTF("%20s : %d\n\r", "unicast",	    p->unicast);
	LOG_PRINTF("%20s : %d\n\r", "hdr_len",	    p->hdr_len);
	LOG_PRINTF("%20s : 0x%x\n\r", "RxResult",	p->RxResult);
	LOG_PRINTF("%20s : %d\n\r", "wildcard_bssid",	p->wildcard_bssid);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_1",	    p->RSVD_1);
	LOG_PRINTF("%20s : %d\n\r", "reason",	    p->reason);
	LOG_PRINTF("%20s : %d\n\r", "payload_offset",	p->payload_offset);
	LOG_PRINTF("%20s : %d\n\r", "next_frag_pid",	p->next_frag_pid);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_2",	    p->RSVD_2);
	LOG_PRINTF("%20s : %d\n\r", "fCmdIdx",	    p->fCmdIdx);
	LOG_PRINTF("%20s : %d\n\r", "wsid",	    p->wsid);
	LOG_PRINTF("%20s : %d\n\r", "RSVD_3",	    p->RSVD_3);
//	LOG_PRINTF("%20s : %d\n\r", "RxF_ID",	    p->RxF_ID);
	LOG_PRINTF("============================\n\r");

	payload_len = p->len - p->hdr_len;
	LOG_PRINTF("%20s : %d\n\r", "payload_len", payload_len);

	dat = (u8 *)p + p->hdr_offset;
	LOG_PRINTF("========== hdr     ==========\n\r");
	LOG_PRINTF("frame ctl     : 0x%04x\n\r", (((u16)dat[1] << 8)|dat[0]));
	LOG_PRINTF("  - more_frag : %d\n\r", GET_HDR80211_FC_MOREFRAG(p));

	a = (u8*)p + p->hdr_offset +  4;
	LOG_PRINTF("address 1     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[0], a[1], a[2], a[3], a[4], a[5]);
	LOG_PRINTF("address 2     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[6], a[7], a[8], a[9], a[10], a[11]);
	LOG_PRINTF("address 3     : %02x:%02x:%02x:%02x:%02x:%02x\n\r", a[12], a[13], a[14], a[15], a[16], a[17]);

	LOG_PRINTF("seq ctl       : 0x%04x\n\r", (((u16)dat[23] << 8)|dat[22]));
	LOG_PRINTF("  - seq num   : %d\n\r", GET_HDR80211_SC_SEQNUM(p));
	LOG_PRINTF("  - frag num  : %d\n\r", GET_HDR80211_SC_FRAGNUM(p));

	return;
}





LIB_APIs void hex_dump (const void *addr, u32 size)
{
    u32 i, j;
    const u32 *data = (const u32 *)addr;

    LOG_TAG_SUPPRESS_ON();
    LOG_PRINTF("        ");
    for (i = 0; i < 8; i++)
        LOG_PRINTF("       %02X", i*sizeof(u32));

    LOG_PRINTF("\r\n--------");
    for (i = 0; i < 8; i++)
        LOG_PRINTF("+--------");

    for (i = 0; i < size; i+= 8)
    {
        LOG_PRINTF("\r\n%08X:%08X", (s32)data, data[0]);
        for (j = 1; j < 8; j++)
        {
            LOG_PRINTF(" %08X", data[j]);
        }
        data = &data[8];
    }
    LOG_PRINTF("\r\n");
    LOG_TAG_SUPPRESS_OFF();
    return;
}

LIB_APIs void halt(void)
{
#if (__SSV_UNIX_SIM__)
	system("pause");
	exit(EXIT_FAILURE);
#else
	/*lint -save -e716 */
    while (1) ;
	/*lint -restore */
#endif
} 
