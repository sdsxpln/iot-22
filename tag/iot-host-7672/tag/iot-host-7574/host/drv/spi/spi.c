#include "config.h"
#include "msgevt.h"

#include "drv_l1_spi.h"
#include "drv_l1_ext_int.h"

#include "ssv_drv.h"
#include "ssv_drv_config.h"
#include "spi_def.h"
#include "ssv6200_reg.h"
#include "spi.h"


#define SSV_SPI_NUM             SPI_1
#define SSV_CS_PIN              IO_A15
#define SSV_IRQ_PIN             IO_F5


static u8 temp_wbuf[1600]={0};

static void _spi_host_cs_init(void);
static void _spi_host_irq_pin_init(void (*spi_isr)(void));
static void _spi_host_cs_high(void);
static void _spi_host_cs_low(void);
static bool spi_host_readwrite(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options, bool IsRead);

static void _spi_host_cs_init(void)
{

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    gpio_set_memcs(1,0); //cs1 set as IO
    gpio_init_io(SSV_CS_PIN,GPIO_OUTPUT);
    gpio_set_port_attribute(SSV_CS_PIN, 1);
    gpio_write_io(SSV_CS_PIN, 1);
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
}



static void _spi_host_irq_pin_init(void (*spi_isr)(void))
{

    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    gpio_init_io(SSV_IRQ_PIN,GPIO_INPUT);
    gpio_set_port_attribute(SSV_IRQ_PIN, 0);
    
    extab_edge_set(EXTA,RISING); /* rising trigger */
    extab_user_isr_set(EXTA,spi_isr); /* register ext A interrupt callback function */
    extab_enable_set(EXTA,FALSE);

    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
}


static void _spi_host_cs_high(void)
{
    gpio_write_io(SSV_CS_PIN, 1);
}
 
static void _spi_host_cs_low(void)
{
    gpio_write_io(SSV_CS_PIN, 0);
}

bool spi_host_init(void (*spi_isr)(void))
{
    SDRV_TRACE("%s() => :%d\r\n",__FUNCTION__,__LINE__);
    if(spi_isr==NULL)
    {
        SDRV_ERROR("%s(): spi_isr is a null pointer\r\n",__FUNCTION__);
        return FALSE;
    }
    spi_init(SSV_SPI_NUM);
    spi_cs_by_external_set(SSV_SPI_NUM);    
    spi_clk_set(SSV_SPI_NUM, SYSCLK_8);
    _spi_host_cs_init();    
    _spi_host_cs_high();
    _spi_host_irq_pin_init(spi_isr);
    OS_MemSET(temp_wbuf,0,sizeof(temp_wbuf));
    SDRV_TRACE("%s() <= :%d\r\n",__FUNCTION__,__LINE__);
    return TRUE;
}

static bool spi_host_readwrite(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options, bool IsRead)
{
    bool ret=TRUE;
    if(options & SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE)
    {
        _spi_host_cs_low();
    }
    if(options & SPI_TRANSFER_OPTIONS_CPU_POLLING_MODE)
    {
        if(TRUE==IsRead){
            if(STATUS_OK != spi_transceive_cpu(SSV_SPI_NUM, temp_wbuf, sizeToTransfer, buf, sizeToTransfer)) ret=FALSE;
        }else{
            if(STATUS_OK != spi_transceive_cpu(SSV_SPI_NUM, buf, sizeToTransfer, buf, sizeToTransfer)) ret=FALSE;
        }
    }
    else
    {
        spi_clk_set(SSV_SPI_NUM, SYSCLK_4);
        if(TRUE==IsRead){
            if(STATUS_OK != spi_transceive(SSV_SPI_NUM, temp_wbuf, sizeToTransfer, buf, sizeToTransfer)) ret=FALSE;
        }else{
            if(STATUS_OK != spi_transceive(SSV_SPI_NUM, buf, sizeToTransfer, buf, 0)) ret=FALSE;
        }
        //if(STATUS_OK != spi_transceive(SSV_SPI_NUM, buf, sizeToTransfer, buf, (IsRead==TRUE)?sizeToTransfer:0)) ret=FALSE;
        spi_clk_set(SSV_SPI_NUM, SYSCLK_8);
    }
    if(options & SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE)
    {
        _spi_host_cs_high();
    }
    return ret;
}

bool spi_host_write(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options)
{
    return spi_host_readwrite(buf, sizeToTransfer, sizeToTransfered, options, FALSE);
}

bool spi_host_read(u8 *buf, u32 sizeToTransfer, u32 *sizeToTransfered, u32 options)
{
    return spi_host_readwrite(buf, sizeToTransfer, sizeToTransfered, options, TRUE);
}

void spi_host_irq_enable(bool enable)
{
    extab_enable_set(EXTA,enable);
}


