#include <config.h>
#include <regs/ssv6200_reg.h>
#include <log.h>
#include "../ssv_drv_if.h"
#include "sdio.h"
#include "sdio_def.h"
#include "../../core/txrx_task.h"

u32 dataIOPort, regIOPort;

static void _sdio_host_isr(void)
{
    TXRXTask_Isr(INT_RX);
}

bool sdio_open(void)
{
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    sdio_host_init(_sdio_host_isr);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return TRUE;
}

bool sdio_close(void)
{
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return TRUE;
}

bool sdio_init(void)
{
    //get dataIOPort(Accesee packet buffer & SRAM)
    dataIOPort = 0;
    dataIOPort = dataIOPort | (sdio_readb_cmd52(REG_DATA_IO_PORT_0) << ( 8*0 ));
    dataIOPort = dataIOPort | (sdio_readb_cmd52(REG_DATA_IO_PORT_1) << ( 8*1 ));
    dataIOPort = dataIOPort | (sdio_readb_cmd52(REG_DATA_IO_PORT_2) << ( 8*2 ));

    //get regIOPort(Access register)
    regIOPort = 0;
    regIOPort = regIOPort | (sdio_readb_cmd52(REG_REG_IO_PORT_0) << ( 8*0 ));
    regIOPort = regIOPort | (sdio_readb_cmd52(REG_REG_IO_PORT_1) << ( 8*1 ));
    regIOPort = regIOPort | (sdio_readb_cmd52(REG_REG_IO_PORT_2) << ( 8*2 ));

    LOG_PRINTF("dataIOPort 0x%x regIOPort 0x%x\n",dataIOPort,regIOPort);

    // output timing
    if (!sdio_writeb_cmd52(REG_OUTPUT_TIMING_REG, SDIO_DEF_OUTPUT_TIMING))
        SDIO_FAIL_RET(0, "sdio_write_byte(0x55, %d)\n", SDIO_DEF_OUTPUT_TIMING);
    LOG_PRINTF("<sdio init> : output timing to %d (0x%08x)\n", SDIO_DEF_OUTPUT_TIMING, SDIO_DEF_OUTPUT_TIMING);
    // switch to normal mode
    // bit[1] , 0:normal mode, 1: Download mode
    if (!sdio_writeb_cmd52(REG_Fn1_STATUS, 0x00))
        SDIO_FAIL_RET(0, "sdio_write_byte(0x0c, 0x00)\n");
    LOG_PRINTF("<sdio init> : switch to normal mode (0x0c, 0x00)\n");

    LOG_PRINTF("<sdio init> : success!! \n");
    return true;
}

// mask rx/tx complete int
// 0: rx int
// 1: tx complete int
void sdio_irq_enable(void)
{
    if (!sdio_writeb_cmd52(REG_INT_MASK, 0xfe))
        SDIO_FAIL(0, "sdio_write_byte(0x04, 0x02)\n");
    LOG_PRINTF("<sdio init> : mask rx/tx complete int (0x04, 0xfe)\n");
}

void sdio_irq_disable(void)
{
    if (!sdio_writeb_cmd52(REG_INT_MASK, 0xff))
        SDIO_FAIL(0, "sdio_write_byte(0x04, 0x03)\n");
    LOG_PRINTF("<sdio init> : mask rx/tx complete int (0x04, 0xff)\n");
}

s32 sdio_recv_data(u8 *dat, size_t len)
{
    size_t rlen;
    //int ret;
    s32 rsize;

    rlen = (u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_0);
    rlen = rlen | ((u32)sdio_readb_cmd52(REG_CARD_PKT_LEN_1)<<0x8);

    rlen=(rlen<len)?rlen:len;
    rsize = sdio_read_cmd53(dataIOPort,(void*)dat,rlen);

    return rsize;
}

s32	sdio_write_data(void *dat, size_t size)
{
    s32 ret;
    int retry=10;
    do
    {
        ret = sdio_write_cmd53(dataIOPort,dat,size);
    }while((ret==FALSE)&&(--retry));

    return ret;
}

bool _sdio_read_reg(u32 addr , u32 *data)    
{
	u8      in_buf[4], out_buf[4];
	u32     in_len, out_len;
    s32     dwRet;
	//bool    status;

	in_len	= 4;
	//memset(in_buf, 0x00, in_len);
	out_len	= 4;
	ssv6xxx_memset((void *)out_buf,0x00, out_len);

	in_buf[0] = ((u8)(addr >> 0) & 0xff);
	in_buf[1] = ((u8)(addr >> 8) & 0xff);
	in_buf[2] = ((u8)(addr >> 16) & 0xff);
	in_buf[3] = ((u8)(addr >> 24) & 0xff);

    do{
        dwRet = sdio_write_cmd53(dataIOPort,in_buf,in_len);
        if(dwRet<=0)
            break;
        
        dwRet = sdio_read_cmd53(dataIOPort,out_buf,out_len);        
        if(dwRet<=0)
            break;

        *data = (out_buf[0]&0xff);
        *data = *data | ((out_buf[1]&0xff)<<( 8 ));
        *data = *data | ((out_buf[2]&0xff)<<( 16 ));
        *data = *data | ((out_buf[3]&0xff)<<( 24 ));
        SDIO_TRACE("%-20s() : 0x%08x, 0x%08x\n", "READ_REG", addr, *data);
        return TRUE;
    }while(0);
	return FALSE;
}
u32 sdio_read_reg(u32 addr)
{
    u32 val=0;
    _sdio_read_reg(addr,&val);

    return val;
}
bool sdio_write_reg(u32 addr, u32 data)
{
    u8      in_buf[8];
    u32     in_len, out_len;
    //s32     dwRet;
    //bool    status;
	
    in_len	= 8;

    in_buf[0] = ((u8)(addr >> 0) & 0xff);
    in_buf[1] = ((u8)(addr >> 8) & 0xff);
    in_buf[2] = ((u8)(addr >> 16) & 0xff);
    in_buf[3] = ((u8)(addr >> 24) & 0xff);

    in_buf[4] = ((u8)(data >> 0) & 0xff);
    in_buf[5] = ((u8)(data >> 8) & 0xff);
    in_buf[6] = ((u8)(data >> 16) & 0xff);
    in_buf[7] = ((u8)(data >> 24) & 0xff);

    out_len = sdio_write_cmd53(dataIOPort,in_buf,in_len);

    SDIO_TRACE("%-20s : 0x%08x, 0x%08x\n", "WRITE_REG", addr, data);
    if(out_len > 0)
        return TRUE;

    return FALSE;
}

bool sdio_write_sram(u32 addr, u8 *data, u32 size)
{
    s32 ret;
    // Set SDIO DMA start address
    sdio_write_reg(0xc0000860, addr);

    // Set data path to DMA to SRAM
    if(TRUE==sdio_writeb_cmd52(0x2, REG_Fn1_STATUS))
    {
        ret = sdio_write_data(data, size);

        // Set data path back to packet
        sdio_writeb_cmd52(0, REG_Fn1_STATUS);
        if(ret > 0)
            return TRUE;
    }
    return FALSE;
}// end of - sdio_write_sram -

static u32 _sdio_write_fw_to_sram(u8* fw_bin, u32 fw_bin_len)
{
    u8   *fw_buffer = NULL;
    u8   *fw_ptr = fw_bin;
    u32  sram_addr = 0x00000000;
    u32  bin_len=fw_bin_len;
    u32  i=0,len=0, checksum=FW_CHECKSUM_INIT, temp=0;
    u32  j=0;

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    fw_buffer = (u8 *)OS_MemAlloc(FW_BLOCK_SIZE);
    if (fw_buffer == NULL) {
        SDRV_FAIL("%s(): Failed to allocate buffer for firmware.\r\n",__FUNCTION__);
        return 0;
    }

    while(bin_len > 0){
        len=(bin_len >= FW_BLOCK_SIZE)?FW_BLOCK_SIZE:bin_len;
        bin_len -= len;
        if(len!=FW_BLOCK_SIZE){
            ssv6xxx_memset((void *)fw_buffer, 0xA5, FW_BLOCK_SIZE);
        }
        OS_MemCPY(fw_buffer,fw_ptr,len);
        fw_ptr += len;

        SDRV_DEBUG("%s(): read len=0x%x,sram_addr=0x%x\r\n",__FUNCTION__,len,sram_addr);

		if(FALSE== sdio_write_sram(sram_addr,fw_buffer,FW_BLOCK_SIZE)) goto ERROR;
        for (i = 0; i < (FW_BLOCK_SIZE)/4; i++) /* force 1024 bytes a set. */
        {
            temp = *((u32 *)(&fw_buffer[i*4]));
            checksum += temp;
        }
		sram_addr += FW_BLOCK_SIZE;
        j++;
        LOG_PRINTF("* ",__FUNCTION__);
        if(j%16==0)
            LOG_PRINTF("\r\n",__FUNCTION__);
    }// while(bin_len > 0){
    LOG_PRINTF("\r\n",__FUNCTION__);
    SDRV_DEBUG("%s(): checksum = 0x%x\r\n",__FUNCTION__, checksum);
    OS_MemFree(fw_buffer);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return checksum;
ERROR:
    SDRV_INFO("\r\n",__FUNCTION__);
    OS_MemFree(fw_buffer);
    SDRV_ERROR("%s(): FAIL\r\n",__FUNCTION__);
    return 0;
}

static bool _sdio_do_firmware_checksum(u32 origin)
{
    #define RETRY_COUNT 30
    u32 retry=0;
    u32 fw_checksum=0;
	u32 fw_status = 0;
	origin = (((origin >> 24) + (origin >> 16) + (origin >> 8) + origin) & FW_CHK_SUM_SIZE);
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    retry=0;
    do{

		do{
            _sdio_read_reg(FW_STATUS_REG,&fw_status);
            if(fw_status & FW_STATUS_FW_CHKSUM_BIT)
                break;
        }while(1);


        fw_checksum = (fw_status & FW_CHK_SUM_MASK)>>16;



        SDRV_DEBUG("%s(): fw check sum = 0x%x, check sum = 0x%x\r\n",__FUNCTION__, fw_checksum, origin);
        if (fw_checksum == origin)
        {
            SDRV_INFO("%s(): [ssv] check sum is the same.\r\n",__FUNCTION__);
            sdio_write_reg(FW_STATUS_REG, (~origin & FW_STATUS_MASK));
            break;
        }
        else
        {
            SDRV_FAIL("%s(): [ssv] check sum is fail.\r\n",__FUNCTION__);
        }

        retry++;
        OS_MsDelay(100);
    }while(retry!=RETRY_COUNT);

    if(retry==RETRY_COUNT)
    {
        SDRV_FAIL("%s(): [ssv] check sum is fail.sum = 0x%x, org = 0x%x\r\n",__FUNCTION__,fw_checksum, origin);
        return FALSE;
    }
    else
    {
        SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
        return TRUE;
    }
}

bool sdio_load_fw(u8* fw_bin, u32 fw_bin_len)
{
    bool ret = TRUE;
//    u32 fw_status;
#ifdef ENABLE_FW_SELF_CHECK
    u32   checksum = FW_CHECKSUM_INIT;
//    u32   fw_checksum = 0x0;
    u32   retry_count = 1;
#else
    int   writesize=0;
    u32   retry_count = 1;
#endif
    u32 clk_en;
    int block_count = 0;

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    if((fw_bin==NULL)||(fw_bin_len==0))
    {
        SDRV_TRACE("%s():wrong input parameters\r\n",__FUNCTION__);
        return FALSE;
    }

    do { //retry loop

        SDRV_INFO("%s(): bin size=0x%x\r\n",__FUNCTION__,fw_bin_len);
        ret=TRUE;
        if(ret==TRUE) ret = sdio_write_reg(ADR_BRG_SW_RST, 0x0);
        if(ret==TRUE) ret = sdio_write_reg(ADR_BOOT, 0x01);
        if(ret==TRUE) ret = _sdio_read_reg(ADR_PLATFORM_CLOCK_ENABLE, &clk_en);

        if(ret==TRUE) ret = sdio_write_reg(ADR_PLATFORM_CLOCK_ENABLE, clk_en | (1 << 2));

        SDRV_INFO("%s(): Writing firmware to SSV6XXX...\r\n",__FUNCTION__);

        checksum = _sdio_write_fw_to_sram(fw_bin,fw_bin_len);



        SDRV_INFO("%s(): checksum = 0x%x\r\n",__FUNCTION__, checksum);

        block_count = fw_bin_len / FW_BLOCK_SIZE;
        block_count = ((fw_bin_len % FW_BLOCK_SIZE)> 0)?block_count+1:block_count;
        SDRV_INFO("%s(): block_count = 0x%x\r\n",__FUNCTION__, block_count);

        if(ret==TRUE) ret= sdio_write_reg(FW_STATUS_REG, (block_count << 16));

        if(ret==TRUE) ret = sdio_write_reg(ADR_BRG_SW_RST, 0x01);

        LOG_PRINTF("wait fw running...\n");
        OS_MsDelay(50);

        LOG_PRINTF("start to check\n");

        //checksum = (((checksum >> 24) + (checksum >> 16) + (checksum >> 8) + checksum) & FW_CHK_SUM_SIZE);

        SDRV_INFO("%s(): Firmware \" loaded, checksum = %x\r\n",__FUNCTION__, checksum);
        // Wait FW to calculate checksum.
        if(TRUE==_sdio_do_firmware_checksum(checksum))
        {
            ret=TRUE;

            //break;
        }
		else
			ret = FALSE;

    } while (--retry_count); //do { //retry loop
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return ret;
}

s32 sdio_start(void)
{

    return TRUE;
}

s32 sdio_stop(void)
{

    return TRUE;
}

bool sdio_wakeup_wifi(bool sw)
{
    if(sw)
        sdio_writeb_cmd52(0x01, REG_PMU_WAKEUP);
    else
        sdio_writeb_cmd52(0x00, REG_PMU_WAKEUP);
}
struct ssv6xxx_drv_ops	g_drv_sdio =
{
    DRV_NAME_SDIO,
    sdio_open,
    sdio_close,
    sdio_init,
    sdio_recv_data,
    sdio_write_data,
    NULL,
    NULL,
    NULL,
    NULL,
    sdio_write_sram,
    NULL,
    sdio_write_reg,
    sdio_read_reg,
    NULL,
    NULL,
    sdio_load_fw,
    sdio_start,
    sdio_stop,
    sdio_irq_enable,
    sdio_irq_disable,
    sdio_wakeup_wifi    
};
