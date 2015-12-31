#include <config.h>
#include <log.h>

#include "sdio.h"
#include "sdio_def.h"

bool sdio_set_bus_width(u8 busWidth)
{
	SDIO_TRACE("%-20s : %d\n", "SET_BUS_WIDTH", busWidth);
	return true;
}

bool sdio_set_bus_clock(u32 clock_rate)
{
	return true;
}

// note : this func will automatically update 's_cur_block_size' value after success
bool sdio_set_block_size(u16 size)
{
	return true;
}

//Only for SDIO register access
u8	sdio_readb_cmd52(u32 addr)
{
	u8 out_buf[4];

	SDIO_TRACE("%-20s : 0x%08x, 0x%02x\n", "READ_BYTE", addr, out_buf[0]);
	return out_buf[0];
}

//Only for SDIO register access
bool sdio_writeb_cmd52(u32 addr, u8 data)
{

	SDIO_TRACE("%-20s : 0x%08x, 0x%02x\n", "WRITE_BYTE", addr, data);
	return true;
}


s32	sdio_read_cmd53(u32 dataPort,u8 *dat, size_t size)
{

	return size;
}

s32	sdio_write_cmd53(u32 dataPort,void *dat, size_t size)
{

	return size;
}

void (*host_isr)(void);
void platform_sdio_isr(void)
{
    if(host_isr)
        host_isr();
}

bool sdio_host_init(void (*sdio_isr)(void))
{
    
	// set clock
	if (!sdio_set_bus_clock(SDIO_DEF_CLOCK_RATE))
		SDIO_FAIL_RET(0, "sdio_set_bus_clock(%d)\n", SDIO_DEF_CLOCK_RATE);

	// set block size
	if (!sdio_set_block_size(SDIO_DEF_BLOCK_SIZE))
		SDIO_FAIL_RET(0, "sdio_set_block_size(%d)\n", SDIO_DEF_BLOCK_SIZE);
	LOG_PRINTF("<sdio init> : set block size to %d\n", SDIO_DEF_BLOCK_SIZE);

	if (!sdio_set_bus_width(4))
		SDIO_FAIL_RET(0, "sdio_set_bus_width(%d)\n",4);
	LOG_PRINTF("<sdio init> : sdio_set_bus_width(%d)\n",4);

    //install isr here
    host_isr = sdio_isr
    //request_sdio_irq(platform_sdio_isr);
    return TRUE;
}



