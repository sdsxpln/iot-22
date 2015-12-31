#ifndef _SDIO_H_
#define _SDIO_H_


bool	sdio_set_bus_clock(u32 clock_rate);
bool	sdio_set_block_size(u16 size);
/*
	the difference between r/w 'byte' & 'data'
	'byte' -> command mode,    need 'addr' 
	'data' -> data    mode, NO need 'addr'
*/
u8		sdio_readb_cmd52(u32 addr);
bool	sdio_writeb_cmd52(u32 addr, u8 data);
s32	    sdio_write_cmd53(u32 dataPort,void *dat, size_t size);

// return
//  < 0 : fail
// >= 0 : # of bytes recieved
s32	    sdio_read_cmd53(u32 dataPort,u8 *dat, size_t size);

bool    sdio_host_init(void (*sdio_isr)(void));

#endif /* _SDIO_H_ */

