//#include "host_global.h"
#include "os_wrapper.h"
#include "log.h"
#include "regs/ssv6200_reg.h"
#include "regs/ssv6200_aux.h"
#include "hctrl.h"
#include "dev_tbl.h"
#include "regs/ssv6200_configuration.h"
#include "ssv6200_uart_bin.h"
#include <drv/ssv_drv.h>
#include "pbuf.h"
#include "ssv_dev.h"
#include <cli/cmds/cli_cmd_wifi.h>
#include "dev.h"
#include "httpserver_raw/httpd.h"




#if 0
struct ap_info_st sg_ap_info[20];
#endif // 0

void stop_and_halt (void)
{
    //while (1) {}
    /*lint -restore */
} // end of - stop_and_halt -


bool SSV6XXX_SET_HW_TABLE(const ssv_cabrio_reg tbl_[], u32 size)
{
    bool ret = FALSE ;
    u32 i=0;
    for(; i<size; i++) {
        ret = MAC_REG_WRITE(tbl_[i].address, tbl_[i].data);
        if (ret==FALSE) break;
    }
    return ret;
}

int ssv6xxx_chip_init()
{
    bool ret;


#ifdef CONFIG_SSV_CABRIO_E
        u32 i,regval=0;
#endif

#ifdef CONFIG_SSV_CABRIO_E
        /*
                    Temp solution.Default 26M.
            */
        //priv->crystal_type = SSV6XXX_IQK_CFG_XTAL_26M;
        /*
            //Xctal setting
            Remodify RF setting For 24M 26M 40M or other xtals.
            */
        //if(priv->crystal_type == SSV6XXX_IQK_CFG_XTAL_26M)
        {
            //init_iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_26M;
            //printk("SSV6XXX_IQK_CFG_XTAL_26M\n");

            for(i=0; i<sizeof(ssv6200_rf_tbl)/sizeof(struct ssv6xxx_dev_table); i++)
            {
                //0xCE010038
                if(ssv6200_rf_tbl[i].address == ADR_SX_ENABLE_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFF7FFF;
                    ssv6200_rf_tbl[i].data |= 0x00008000;
                }
                //0xCE010060
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_DIVIDER_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xE0380FFF;
                    ssv6200_rf_tbl[i].data |= 0x00406000;
                }
                //0xCE01009C
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_I)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFFF800;
                    ssv6200_rf_tbl[i].data |= 0x00000024;
                }
                //0xCE0100A0
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_II)
                {
                    ssv6200_rf_tbl[i].data &= 0xFF000000;
                    ssv6200_rf_tbl[i].data |= 0x00EC4CC5;
                }
            }
        }
        #if 0
        else if(priv->crystal_type == SSV6XXX_IQK_CFG_XTAL_40M)
        {
            //init_iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_40M;
            printk("SSV6XXX_IQK_CFG_XTAL_40M\n");
            for(i=0; i<sizeof(ssv6200_rf_tbl)/sizeof(struct ssv6xxx_dev_table); i++)
            {
                //0xCE010038
                if(ssv6200_rf_tbl[i].address == ADR_SX_ENABLE_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFF7FFF;
                    ssv6200_rf_tbl[i].data |= 0x00000000;
                }
                //0xCE010060
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_DIVIDER_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xE0380FFF;
                    ssv6200_rf_tbl[i].data |= 0x00406000;
                }
                //0xCE01009C
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_I)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFFF800;
                    ssv6200_rf_tbl[i].data |= 0x00000030;
                }
                //0xCE0100A0
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_II)
                {
                    ssv6200_rf_tbl[i].data &= 0xFF000000;
                    ssv6200_rf_tbl[i].data |= 0x00EC4CC5;
                }
            }
        }
        else if(priv->crystal_type == SSV6XXX_IQK_CFG_XTAL_24M)
        {
            printk("SSV6XXX_IQK_CFG_XTAL_24M\n");
            //init_iqk_cfg.cfg_xtal = SSV6XXX_IQK_CFG_XTAL_24M;
            for(i=0; i<sizeof(ssv6200_rf_tbl)/sizeof(struct ssv6xxx_dev_table); i++)
            {
                //0xCE010038
                if(ssv6200_rf_tbl[i].address == ADR_SX_ENABLE_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFF7FFF;
                    ssv6200_rf_tbl[i].data |= 0x00008000;
                }
                //0xCE010060
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_DIVIDER_REGISTER)
                {
                    ssv6200_rf_tbl[i].data &= 0xE0380FFF;
                    ssv6200_rf_tbl[i].data |= 0x00406000;
                }
                //0xCE01009C
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_I)
                {
                    ssv6200_rf_tbl[i].data &= 0xFFFFF800;
                    ssv6200_rf_tbl[i].data |= 0x00000028;
                }
                //0xCE0100A0
                if(ssv6200_rf_tbl[i].address == ADR_DPLL_FB_DIVIDER_REGISTERS_II)
                {
                    ssv6200_rf_tbl[i].data &= 0xFF000000;
                    ssv6200_rf_tbl[i].data |= 0x00000000;
                }
            }
        }
        else
        {
            printk("Illegal xtal setting \n");
            BUG_ON(1);
        }
        #endif
#endif

    /* reset ssv6200 mac */

    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */


    //write rf table
    ret = SSV6XXX_SET_HW_TABLE(ssv6200_rf_tbl,sizeof(ssv6200_rf_tbl));

    if (ret == TRUE) ret = MAC_REG_WRITE( 0xce000004, 0x00000000); /* ???? */

    /* Turn off phy before configuration */

    //write phy table
    if (ret == TRUE) ret = SSV6XXX_SET_HW_TABLE(ssv6200_phy_tbl,sizeof(ssv6200_phy_tbl));

#ifdef CONFIG_SSV_CABRIO_E
    //Avoid SDIO issue.

    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_TRX_DUMMY_REGISTER, 0xEAAAAAAA);
    if (ret == TRUE)  MAC_REG_READ(ADR_TRX_DUMMY_REGISTER,regval);

    if(regval != 0xEAAAAAAA)
    {
        LOG_PRINTF("@@@@@@@@@@@@\r\n");
        LOG_PRINTF(" SDIO issue -- please check 0xCE01008C %08x!!\r\n",regval);
        LOG_PRINTF(" It shouble be 0xEAAAAAAA!!\r\n");
        LOG_PRINTF("@@@@@@@@@@@@ \r\n");
    }
#endif

#ifdef CONFIG_SSV_CABRIO_E
    /* Cabrio E: GPIO setting */

    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD53, 0x21);        /* like debug-uart config ? */
    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD54, 0x3000);
    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PIN_SEL_0, 0x4000);
    
    MAC_REG_READ(ADR_PAD20,regval);
    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_PAD20, regval|0x01);

    /* TR switch: */
    if (ret == TRUE) ret = MAC_REG_WRITE(0xc0000304, 0x01);
    if (ret == TRUE) ret = MAC_REG_WRITE(0xc0000308, 0x01);

#endif // CONFIG_SSV_CABRIO_E

    //Switch clock to PLL output of RF
    //MAC and MCU clock selection :   00 : OSC clock   01 : RTC clock   10 : synthesis 80MHz clock   11 : synthesis 40MHz clock

    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_CLOCK_SELECTION, 0x3);


#ifdef CONFIG_SSV_CABRIO_E
        //Avoid consumprion of electricity
        //SDIO issue

    if (ret == TRUE) ret = MAC_REG_WRITE(ADR_TRX_DUMMY_REGISTER, 0xAAAAAAAA);
#endif

    MAC_REG_WRITE(ADR_SYS_INT_FOR_HOST, 0);
    /* reset ssv6200 mac */
    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */

	if(ret == TRUE)
		return 1;
	else
		return 0;

}

int cabrio_init_mac()
{
    int i;
    bool ret;
    u32 regval,id_len,j;
    char    chip_id[24]="";
    u32     chip_tag1,chip_tag2;
    u32 *ptr;


    ssv6xxx_drv_irq_enable();

    //CHIP TAG

    MAC_REG_READ(ADR_IC_TIME_TAG_1,regval);
    chip_tag1 = regval;
    MAC_REG_READ(ADR_IC_TIME_TAG_0,regval);
    chip_tag2= regval;
    //LOG_DEBUG("CHIP TAG: %llx \r\n",chip_tag);
    LOG_DEBUG("CHIP TAG: %08x%08x \r\n",chip_tag1,chip_tag2);
    //CHIP ID
    MAC_REG_READ(ADR_CHIP_ID_3,regval);
    *((u32 *)&chip_id[0]) = (u32)LONGSWAP(regval);
    MAC_REG_READ(ADR_CHIP_ID_2,regval);
    *((u32 *)&chip_id[4]) = (u32)LONGSWAP(regval);
    MAC_REG_READ(ADR_CHIP_ID_1,regval);
    *((u32 *)&chip_id[8]) = (u32)LONGSWAP(regval);
    MAC_REG_READ(ADR_CHIP_ID_0,regval);

    *((u32 *)&chip_id[12]) = (u32)LONGSWAP(regval);
    LOG_DEBUG("CHIP ID: %s \r\n",chip_id);


    //soc set HDR-STRIP-OFF       enable
    //soc set HCI-RX2HOST         enable
    //soc set AUTO-SEQNO          enable
    //soc set ERP-PROTECT          disable
    //soc set MGMT-TXQID            3
    //soc set NONQOS-TXQID      1
    regval = (RX_2_HOST_MSK|AUTO_SEQNO_MSK|HDR_STRIP_MSK|(3<<TXQ_ID0_SFT)|(1<<TXQ_ID1_SFT)|RX_ETHER_TRAP_EN_MSK);

    ret = MAC_REG_WRITE(ADR_CONTROL,regval);

    /* Enable hardware timestamp for TSF */
    // 28 => time stamp write location
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_RX_TIME_STAMP_CFG,((28<<MRX_STP_OFST_SFT)|0x01));

    //MMU[decide packet buffer no.]
    /* Setting MMU to 256 pages */
//    MAC_REG_READ(ADR_MMU_CTRL, regval);
//    regval |= (0xff<<MMU_SHARE_MCU_SFT);
//    MAC_REG_WRITE(ADR_MMU_CTRL, regval);

    /**
        * Tx/RX threshold setting for packet buffer resource.
        */

    MAC_REG_READ(ADR_TRX_ID_THRESHOLD,id_len);
    id_len = (id_len&0xffff0000 ) |
             (SSV6200_ID_TX_THRESHOLD<<TX_ID_THOLD_SFT)|
             (SSV6200_ID_RX_THRESHOLD<<RX_ID_THOLD_SFT);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_TRX_ID_THRESHOLD, id_len);

    MAC_REG_READ(ADR_ID_LEN_THREADSHOLD1,id_len);
    id_len = (id_len&0x0f )|
             (SSV6200_PAGE_TX_THRESHOLD<<ID_TX_LEN_THOLD_SFT)|
             (SSV6200_PAGE_RX_THRESHOLD<<ID_RX_LEN_THOLD_SFT);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_ID_LEN_THREADSHOLD1, id_len);

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_STA_MAC_0, *((u32 *)&(gDeviceInfo->self_mac[0])));
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_STA_MAC_1, *((u32 *)&(gDeviceInfo->self_mac[4])));
    /**
        * Reset all wsid table entry to invalid.
        */
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_WSID1, 0x00000000);

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_SDIO_MASK, 0xfffe1fff);
#if 0
    //Enable EDCA low threshold
    MAC_REG_WRITE(ADR_MB_THRESHOLD6, 0x80000000);
    //Enable EDCA low threshold EDCA-1[8] EDCA-0[4]
    MAC_REG_WRITE(ADR_MB_THRESHOLD8, 0x08040000);
    //Enable EDCA low threshold EDCA-3[8] EDCA-2[8]
    MAC_REG_WRITE(ADR_MB_THRESHOLD9, 0x00000808);
#endif

    /**
        * Disable tx/rx ether trap table.
        */

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_TX_ETHER_TYPE_0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_TX_ETHER_TYPE_1, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_RX_ETHER_TYPE_0, 0x00000000);
    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_RX_ETHER_TYPE_1, 0x00000000);


//-----------------------------------------------------------------------------------------------------------------------------------------
//PHY and security table
    /**
        * Allocate a hardware packet buffer space. This buffer is for security
        * key caching and phy info space.
        */
    /*lint -save -e732  Loss of sign (assignment) (int to unsigned int)*/
    gDeviceInfo->hw_buf_ptr = ssv6xxx_pbuf_alloc((s32)sizeof(phy_info_tbl)+
                                            (s32)sizeof(struct ssv6xxx_hw_sec),(int)NOTYPE_BUF);
   /*lint -restore */
    if((gDeviceInfo->hw_buf_ptr>>28) != 8)
    {
    	//asic pbuf address start from 0x8xxxxxxxx
    	LOG_PRINTF("opps allocate pbuf error\n");
    	//WARN_ON(1);	
    	ret = 1;
    	goto exit;
    }

    LOG_PRINTF("%s(): ssv6200 reserved space=0x%08x, size=%d\n", 
        __FUNCTION__, gDeviceInfo->hw_buf_ptr, (u32)(sizeof(phy_info_tbl)+sizeof(struct ssv6xxx_hw_sec)));

	

/**	
Part 1. SRAM
	**********************
	*				          * 
	*	1. Security key table *
	* 				          *
	* *********************
	*				          *
	*    	2. PHY index table     *
	* 				          *
	* *********************
	* 				          *
	*	3. PHY ll-length table * 
	*				          *
	* *********************	
=============================================	
Part 2. Register     
	**********************
	*				          * 
	*	PHY Infor Table         *
	* 				          *
	* *********************
*
*/

    /**
        * Init ssv6200 hardware security table: clean the table.
        * And set PKT_ID for hardware security.
        */
    gDeviceInfo->hw_sec_key = gDeviceInfo->hw_buf_ptr;
	
	//==>Section 1. Write Sec table to SRAM
    for(j=0; j<sizeof(struct ssv6xxx_hw_sec); j+=4) {
        MAC_REG_WRITE(gDeviceInfo->hw_sec_key+j, 0);
    }
    /*lint -save -e838*/
    regval = ((gDeviceInfo->hw_sec_key >> 16) << SCRT_PKT_ID_SFT);
    MAC_REG_READ(ADR_SCRT_SET, regval);
	regval &= SCRT_PKT_ID_I_MSK;
	regval |= ((gDeviceInfo->hw_sec_key >> 16) << SCRT_PKT_ID_SFT);
	MAC_REG_WRITE(ADR_SCRT_SET, regval);
    /*lint -restore*/

    /**
        * Set default ssv6200 phy infomation table.
        */
    gDeviceInfo->hw_pinfo = gDeviceInfo->hw_sec_key + sizeof(struct ssv6xxx_hw_sec);
    for(i=0, ptr=phy_info_tbl; i<PHY_INFO_TBL1_SIZE; i++, ptr++) {
        MAC_REG_WRITE(ADR_INFO0+(u32)i*4, *ptr);
    }	

	
	//==>Section 2. Write PHY index table and PHY ll-length table to SRAM
	for(i=0; i<PHY_INFO_TBL2_SIZE; i++, ptr++) {
        MAC_REG_WRITE(gDeviceInfo->hw_pinfo+(u32)i*4, *ptr);
    }
    for(i=0; i<PHY_INFO_TBL3_SIZE; i++, ptr++) {
        MAC_REG_WRITE(gDeviceInfo->hw_pinfo+(PHY_INFO_TBL2_SIZE<<2)+(u32)i*4, *ptr);
    }


    MAC_REG_WRITE(ADR_INFO_RATE_OFFSET, 0x00040000);
	
	//Set SRAM address to register
	MAC_REG_READ(ADR_INFO_IDX_ADDR, regval);
    LOG_PRINTF("ADR_INFO_IDX_ADDR:%08x\n",regval);
	MAC_REG_WRITE(ADR_INFO_IDX_ADDR, gDeviceInfo->hw_pinfo);
    MAC_REG_WRITE(ADR_INFO_LEN_ADDR, gDeviceInfo->hw_pinfo+(PHY_INFO_TBL2_SIZE)*4); //4byte for one entry
    MAC_REG_READ(ADR_INFO_IDX_ADDR, regval);

	LOG_PRINTF("ADR_INFO_IDX_ADDR[%08x] ADR_INFO_LEN_ADDR[%08x]\n", regval, gDeviceInfo->hw_pinfo+(PHY_INFO_TBL2_SIZE)*4); 
 
    //-----------------------------------------------------------------------------------------------------------------------------------------

    if(ret == TRUE) ret = MAC_REG_WRITE(0xca000800,0xffffffff);
    
    if(ret == TRUE) ret = MAC_REG_WRITE(0xCE000004,0x0000017F);//PHY b/g/n on

    //-----------------------------------------------------------------------------------------------------------------------------------------
    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //C_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);
    
    /* Setup q4 behavior STA mode-> act as normal queue    
      * 
      */
        MAC_REG_READ(ADR_MTX_BCN_EN_MISC,regval);       
        regval&= ~(MTX_HALT_MNG_UNTIL_DTIM_MSK);
        regval |= (0);
    if(ret == TRUE) ret = MAC_REG_WRITE( ADR_MTX_BCN_EN_MISC, regval);
    
    //-----------------------------------------------------------------------------------------------------------------------------------------
    
    //-----------------------------------------------------------------------------------------------------------------------------------------
    //MMU[decide packet buffer no.]
        /* Setting MMU to 256 pages */
    
        //MAC_REG_READ(ADR_MMU_CTRL, regval);
        //regval |= (0xff<<MMU_SHARE_MCU_SFT);
        //MAC_REG_WRITE(ADR_MMU_CTRL, regval);
    
    //-----------------------------------------------------------------------------------------------------------------------------------------

    if(ret == TRUE) ret = MAC_REG_WRITE(ADR_INFO_RATE_OFFSET, 0x00040000);

    exit:
    //Load FW
    LOG_PRINTF("bin size =%d\r\n",sizeof(ssv6200_uart_bin));
    MAC_LOAD_FW((u8 *)ssv6200_uart_bin,sizeof(ssv6200_uart_bin));//??? u8* bin

    return ((ret == TRUE)?0:1);
}

int cabrio_init_sta_mac()
{
    int i, ret=0;
    u16 *mac_deci_tbl;

    MAC_REG_WRITE(ADR_GLBLE_SET,
        //(0 << OP_MODE_SFT)  |                           /* STA mode by default */
        //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
        (1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
        (3 << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
        (80 << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
    );
    
    MAC_REG_WRITE(ADR_BSSID_0,   0x00000000);//*((u32 *)&priv->bssid[0]));
    MAC_REG_WRITE(ADR_BSSID_1,   0x00000000);//*((u32 *)&priv->bssid[4]));

     /**
        * Set reason trap to discard frames.
        */
    MAC_REG_WRITE(ADR_REASON_TRAP0, 0x7FBC7F8F);
    MAC_REG_WRITE(ADR_REASON_TRAP1, 0x00000000);

    //soc set TX-FLOW-MGMT        { M_ENG_HWHCI M_ENG_ENCRYPT M_ENG_TX_EDCA0 M_ENG_CPU }
    //soc set TX-FLOW-CTRL        { M_ENG_HWHCI M_ENG_TX_EDCA0 M_ENG_CPU M_ENG_CPU }

	
	MAC_REG_WRITE(ADR_TX_FLOW_0,  M_ENG_HWHCI|(M_ENG_TX_MNG<<4)|
	                              /*(M_ENG_CPU<<8)|
								  (M_ENG_CPU<<12)|*/
		 						(M_ENG_HWHCI<<16)|
								(M_ENG_ENCRYPT<<20)|
								(M_ENG_TX_MNG<<24)/*|
								(M_ENG_CPU<<28)*/);
    //Info :845 the right argument to operator '|' is certain to be 0

	
    // soc set TX-FLOW-DATA        { M_ENG_HWHCI M_ENG_MIC M_ENG_FRAG M_ENG_ENCRYPT M_CPU_EDCATX M_ENG_TX_EDCA0 M_ENG_CPU M_ENG_CPU }
    {
        const unsigned char cmd_data[] = {
            0x51, 0x03, 0x06, 0x00, 0x00, 0x40, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXDATA, (void *)cmd_data, 8, TRUE, FALSE);
    }
    // soc set RX-FLOW-DATA        { M_ENG_MACRX M_ENG_ENCRYPT_SEC M_CPU_DEFRAG M_ENG_MIC_SEC M_ENG_HWHCI M_ENG_CPU M_ENG_CPU M_ENG_CPU }
    //0xB4, 0xC0, 0x01, 0x00, 0x00, 0x03, 0x00, 0x00};
    // soc set RX-FLOW-DATA        { M_ENG_MACRX M_ENG_ENCRYPT_SEC M_ENG_CPU M_ENG_MIC_SEC M_ENG_HWHCI M_ENG_CPU M_ENG_CPU M_ENG_CPU }
    {
        const unsigned char cmd_data[] = {
            0xB4, 0xC0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);
    }
    // soc set RX-FLOW-MGMT        { M_ENG_MACRX M_CPU_RXMGMT M_ENG_CPU M_ENG_CPU }
    {
        const unsigned char cmd_data[] = {
            0x04, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);
    }
    // soc set RX-FLOW-CTRL        { M_ENG_MACRX M_ENG_CPU M_ENG_CPU M_ENG_CPU }
    {
        const unsigned char cmd_data[] = {
            0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXCTRL, (void *)cmd_data, 8, TRUE, FALSE);
    }


    //#set EDCA parameter AP-a/g BK[0], BE[1], VI[2], VO[3]
    //soc set WMM-PARAM[0]      { aifsn=0 acm=0 cwmin=5 cwmax=10 txop=0 backoffvalue=6 }

    
    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_AIFSN,

          (6 << TXQ0_MTX_Q_AIFSN_SFT)  |                           /* aifsn=7 */
          (4 << TXQ0_MTX_Q_ECWMIN_SFT) |                            /*cwmin=4 */
          (10 << TXQ0_MTX_Q_ECWMAX_SFT)                           /* cwmax=10 */
                                     
    );


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);

    //soc set WMM-PARAM[1]      { aifsn=0 acm=0 cwmin=4 cwmax=10 txop=0 backoffvalue=5 }
    
    MAC_REG_WRITE(ADR_TXQ1_MTX_Q_AIFSN,
    
              (2 << TXQ1_MTX_Q_AIFSN_SFT)  |                       /* aifsn=3 */
              (4 << TXQ1_MTX_Q_ECWMIN_SFT) |                        /*cwmin=4 */
              (10 << TXQ1_MTX_Q_ECWMAX_SFT)                      /* cwmax=10 */
    );


    //soc set WMM-PARAM[2]      { aifsn=0 acm=0 cwmin=3 cwmax=4 txop=94 backoffvalue=4 }
    {

        MAC_REG_WRITE( ADR_TXQ2_MTX_Q_AIFSN,

              (1 << TXQ2_MTX_Q_AIFSN_SFT)  |                       /* aifsn=2 */
              (3 << TXQ2_MTX_Q_ECWMIN_SFT) |                        /*cwmin=3 */
              (4 << TXQ2_MTX_Q_ECWMAX_SFT) |                       /* cwmax=4 */
              (94 << TXQ2_MTX_Q_TXOP_LIMIT_SFT)                        /*  txop=94 */
        );

    }




    //soc set WMM-PARAM[3]      { aifsn=0 acm=0 cwmin=2 cwmax=3 txop=47 backoffvalue=3 }
    {
        //info 845: The right argument to operator '|' is certain to be 0

        MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,

              (1 << TXQ3_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
              (2 << TXQ3_MTX_Q_ECWMIN_SFT) |                    /*cwmin=2 */
              (3 << TXQ3_MTX_Q_ECWMAX_SFT) |                  /* cwmax=3 */
              (47 << TXQ3_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=47 */
        );

    }
    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //MAC_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);
        /*lint -save -e648 Overflow in computing constant for operation:    'shift left'*/
        MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,
    
          (1 << TXQ4_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
          (1 << TXQ4_MTX_Q_ECWMIN_SFT) |                    /*cwmin=1 */
          (2 << TXQ4_MTX_Q_ECWMAX_SFT) |                  /* cwmax=2 */
          (65535 << TXQ4_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
    );


    /* By default, we apply staion decion table. */
    mac_deci_tbl= sta_deci_tbl;

    for(i=0; i<MAC_DECITBL1_SIZE; i++) {

        MAC_REG_WRITE((ADR_MRX_FLT_TB0+(u32)i*4),(u32 )(mac_deci_tbl[i]));

    }
    for(i=0; i<MAC_DECITBL2_SIZE; i++) {

        MAC_REG_WRITE(ADR_MRX_FLT_EN0+(u32)i*4,

        mac_deci_tbl[i+MAC_DECITBL1_SIZE]);

    }
    
    // cal 6
    {
        const unsigned char cmd_data[] = {
            0x06, 0x00, 0x00, 0x00};
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_CAL, (void *)cmd_data, 4, TRUE, FALSE);
    }

    return ret;
}
int cabrio_init_ap_mac()
{
    int i, ret=0;
    u32 temp;
    u16 *mac_deci_tbl;
    
    MAC_REG_WRITE(ADR_GLBLE_SET,

          (1 << OP_MODE_SFT)  |                           /* AP mode by default */
          //(0 << SNIFFER_MODE_SFT) |                           /* disable sniffer mode */
          (1 << DUP_FLT_SFT) |                           /* Enable duplicate detection */
          (3 << TX_PKT_RSVD_SFT) |                           /* PKT Reserve */
          (80 << PB_OFFSET_SFT)                          /* set rx packet buffer offset */
    );

    MAC_REG_READ(ADR_SCRT_SET,temp);
    temp = temp & SCRT_RPLY_IGNORE_I_MSK;
    temp |= (1 << SCRT_RPLY_IGNORE_SFT);
    MAC_REG_WRITE(ADR_SCRT_SET, temp);

    MAC_REG_WRITE(ADR_BSSID_0,   *((u32 *)&gDeviceInfo->self_mac[0]));//*((u32 *)&priv->bssid[0]));
    MAC_REG_WRITE(ADR_BSSID_1,   *((u32 *)&gDeviceInfo->self_mac[4]));//*((u32 *)&priv->bssid[4]));
    /**
        * Set reason trap to discard frames.
        */

    //soc set TX-FLOW-DATA        { M_ENG_HWHCI M_ENG_MIC M_ENG_ENCRYPT M_CPU_EDCATX M_ENG_TX_EDCA0 M_ENG_CPU M_ENG_CPU M_ENG_CPU }
    //FMAC_REG_WRITE(priv, ADR_TX_FLOW_1,  M_ENG_HWHCI|(M_ENG_MIC<<4)|(M_ENG_ENCRYPT<<8)|(M_CPU_EDCATX<<12)|
    //                                    (M_ENG_TX_EDCA0<<16)|(M_ENG_CPU<<20)|(M_ENG_CPU<<24)|(M_ENG_CPU<<28));
    //soc set TX-FLOW-MGMT        { M_ENG_HWHCI M_ENG_ENCRYPT M_ENG_TX_EDCA0 M_ENG_CPU }

    {
    const unsigned char cmd_data[] = {
        0x01, 0x06, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00};
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXMGMT, (void *)cmd_data, 8, TRUE, FALSE);

    
    }
    //soc set TX-FLOW-CTRL        { M_ENG_HWHCI M_ENG_TX_EDCA0 M_ENG_CPU M_ENG_CPU }
    {
    const unsigned char cmd_data[] = {
        0x01, 0x03, 0x06, 0x00, 0x40, 0x00, 0x00, 0x00};
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXCTRL, (void *)cmd_data, 8, TRUE, FALSE);

    }
    //info 845: The right argument to operator '|' is certain to be 0 
    
    //MAC_REG_WRITE(ADR_TX_FLOW_0,  M_ENG_HWHCI|(M_ENG_TX_EDCA0<<4)|(M_ENG_CPU<<8)|(M_ENG_CPU<<12)|
    //                                (M_ENG_HWHCI<<16)|(M_ENG_TX_EDCA0<<20)|(M_ENG_CPU<<24)|(M_ENG_CPU<<28));

    //soc set TX-FLOW-DATA        { M_ENG_HWHCI M_ENG_MIC M_ENG_FRAG M_ENG_ENCRYPT M_CPU_EDCATX M_ENG_TX_EDCA0 M_ENG_CPU M_ENG_CPU }
    {
    const unsigned char cmd_data[] = {
        0x51, 0x03, 0x06, 0x00, 0x00, 0x40, 0x00, 0x00};
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_TXDATA, (void *)cmd_data, 8, TRUE, FALSE);

    
    }

    //soc set RX-FLOW-DATA        { M_ENG_MACRX M_ENG_ENCRYPT M_CPU_DEFRAG M_ENG_MIC M_ENG_HWHCI M_ENG_CPU M_ENG_CPU M_ENG_CPU }
    {
    const unsigned char cmd_data[] = {
        0xB4, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00};
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXDATA, (void *)cmd_data, 8, TRUE, FALSE);  

    }
     //soc set RX-FLOW-MGMT        { M_ENG_MACRX M_ENG_ENCRYPT M_ENG_HWHCI M_ENG_CPU}
    {
    const unsigned char cmd_data[] = {
        0x34, 0x01, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00};
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXMGMT, (void *)cmd_data, 8, TRUE, FALSE);


    }
     //soc set RX-FLOW-CTRL        { M_ENG_MACRX M_CPU_RXCTRL M_ENG_CPU M_ENG_CPU }
    {
    const unsigned char cmd_data[] = {
        0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_SET_FCMD_RXCTRL, (void *)cmd_data, 8, TRUE, FALSE);

    }

     //soc set REASON-TRAP         { 0x7fbf7f8f 0xffffffff }
    {

    MAC_REG_WRITE(ADR_REASON_TRAP0,0x7FBF7F8F);
    MAC_REG_WRITE(ADR_REASON_TRAP1,0xFFFFFFFF);

    }

    //#set EDCA parameter AP-a/g BK[0], BE[1], VI[2], VO[3]
    //soc set WMM-PARAM[0]      { aifsn=0 acm=0 cwmin=5 cwmax=10 txop=0 backoffvalue=6 }

    
    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_AIFSN,

          (6 << TXQ0_MTX_Q_AIFSN_SFT)  |                           /* aifsn=7 */
          (4 << TXQ0_MTX_Q_ECWMIN_SFT) |                            /*cwmin=4 */
          (10 << TXQ0_MTX_Q_ECWMAX_SFT)                           /* cwmax=10 */
         //| (0 << TXQ0_MTX_Q_TXOP_LIMIT_SFT )                                      /*  txop=0 */
                                     
    );


    MAC_REG_WRITE(ADR_TXQ0_MTX_Q_BKF_CNT,0x00000006);

    //soc set WMM-PARAM[1]      { aifsn=0 acm=0 cwmin=4 cwmax=10 txop=0 backoffvalue=5 }
    
    MAC_REG_WRITE(ADR_TXQ1_MTX_Q_AIFSN,
    
              (2 << TXQ1_MTX_Q_AIFSN_SFT)  |                       /* aifsn=3 */
              (4 << TXQ1_MTX_Q_ECWMIN_SFT) |                        /*cwmin=4 */
              (6 << TXQ1_MTX_Q_ECWMAX_SFT)                      /* cwmax=6 */
              //|(0 << TXQ1_MTX_Q_TXOP_LIMIT_SFT)                         /*  txop=0 */
    );

    //MAC_REG_WRITE(ADR_TXQ1_MTX_Q_BKF_CNT,0x00000005);




    //soc set WMM-PARAM[2]      { aifsn=0 acm=0 cwmin=3 cwmax=4 txop=94 backoffvalue=4 }
    {

        MAC_REG_WRITE( ADR_TXQ2_MTX_Q_AIFSN,

              (0 << TXQ2_MTX_Q_AIFSN_SFT)  |                       /* aifsn=1 */
              (3 << TXQ2_MTX_Q_ECWMIN_SFT) |                        /*cwmin=3 */
              (4 << TXQ2_MTX_Q_ECWMAX_SFT) |                       /* cwmax=4 */
              (94 << TXQ2_MTX_Q_TXOP_LIMIT_SFT)                        /*  txop=94 */
        );


        //MAC_REG_WRITE( ADR_TXQ2_MTX_Q_BKF_CNT,0x00000004);


    }




    //soc set WMM-PARAM[3]      { aifsn=0 acm=0 cwmin=2 cwmax=3 txop=47 backoffvalue=3 }
    {
        //info 845: The right argument to operator '|' is certain to be 0

        MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,

              (0 << TXQ3_MTX_Q_AIFSN_SFT)  |                   /* aifsn=1 */
              (2 << TXQ3_MTX_Q_ECWMIN_SFT) |                    /*cwmin=2 */
              (3 << TXQ3_MTX_Q_ECWMAX_SFT) |                  /* cwmax=3 */
              (47 << TXQ3_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=47 */
        );


        //MAC_REG_WRITE( ADR_TXQ3_MTX_Q_BKF_CNT,0x00000003);

    }
    /* Set wmm parameter to EDCA Q4
        (use to send mgmt frame/null data frame in STA mode and broadcast frame in AP mode) */
        //MAC_REG_WRITE(ADR_TXQ4_MTX_Q_AIFSN, 0xffff2101);
        /*lint -save -e648 Overflow in computing constant for operation:    'shift left'*/
        MAC_REG_WRITE( ADR_TXQ3_MTX_Q_AIFSN,
    
          (1 << TXQ4_MTX_Q_AIFSN_SFT)  |                   /* aifsn=2 */
          (1 << TXQ4_MTX_Q_ECWMIN_SFT) |                    /*cwmin=1 */
          (2 << TXQ4_MTX_Q_ECWMAX_SFT) |                  /* cwmax=2 */
          (65535 << TXQ4_MTX_Q_TXOP_LIMIT_SFT)                     /*  txop=65535 */
    );
    /*lint -restore*/





    /* By default, we apply ap decion table. */

    mac_deci_tbl = ap_deci_tbl;

    for(i=0; i<MAC_DECITBL1_SIZE; i++) {

        MAC_REG_WRITE( ADR_MRX_FLT_TB0+(u32)i*4,
        mac_deci_tbl[i]);
    }
    for(i=0; i<MAC_DECITBL2_SIZE; i++) {
        MAC_REG_WRITE( ADR_MRX_FLT_EN0+(u32)i*4,

        mac_deci_tbl[i+MAC_DECITBL1_SIZE]);

    }
    //trap null data
    //SET_RX_NULL_TRAP_EN(1)
    //MAC_REG_SET_BITS(ADR_CONTROL,1 << RX_NULL_TRAP_EN_SFT,RX_NULL_TRAP_EN_MSK);
    MAC_REG_READ(ADR_CONTROL,temp);
    temp = temp & RX_NULL_TRAP_EN_I_MSK;
    temp |= (1 << RX_NULL_TRAP_EN_SFT);
    MAC_REG_WRITE(ADR_CONTROL, temp);

    // cal 6
    {
        unsigned char cmd_data[] = {
            0x00, 0x00, 0x00, 0x00};
        cmd_data[0]= gDeviceInfo->APInfo->nCurrentChannel;
        _ssv6xxx_wifi_ioctl_Ext(SSV6XXX_HOST_CMD_CAL, cmd_data, 4, TRUE, FALSE);
    }


    return ret;
}
int ssv6xxx_add_interface(ssv6xxx_hw_mode hw_mode)
{
    //0:false 1:true
    return 1;

}

void ssv6xxx_rf_enable()
{
                          
    u32 _regval;                               
    MAC_REG_READ(0xce010000, _regval);        
    _regval &= ~(0x03<<12);                        
    _regval |= (0x02<<12);                                                     
    MAC_REG_WRITE(0xce010000, _regval);
    return;                                       

}

void ssv6xxx_rf_disable()
{
                                
    u32 _regval;                               
    MAC_REG_READ(0xce010000, _regval);        
    _regval &= ~(0x03<<12);                        
    _regval |= (0x01<<12);              
    MAC_REG_WRITE(0xce010000, _regval);
    return; 


}
void ssv6xxx_HW_disable()
{

    ssv6xxx_rf_disable();
    
    //Disable MCU
    MAC_REG_WRITE(ADR_BRG_SW_RST, 0x0);
    MAC_REG_WRITE(ADR_BRG_SW_RST, 1 << 1);  /* bug if reset ?? */

    return; 


}
void ssv6xxx_HW_enable(void)
{
    ssv6xxx_rf_enable();

    //Enable MCU, it's duplicate, load fw has already set this bit
    MAC_REG_WRITE(ADR_BRG_SW_RST, 0x01);

    return; 


}

//0 :success 1:fail
int ssv6xxx_start()
{

    /* Reset MAC & Re-Init */
    
    /* Initialize ssv6200 mac */
    if(cabrio_init_mac()!= 0)
    {
    	return 1;
    }

    //Set ap or station register
    if (SSV6XXX_HWM_STA == gDeviceInfo->hw_mode)
    {
        cabrio_init_sta_mac();
    }
    else
    {
        cabrio_init_ap_mac();
    }
#ifdef CONFIG_SSV_CABRIO_E
        /* Do RF-IQ cali. */
        //ssv6xxx_do_iq_calib(sc->sh, &init_iqk_cfg);
#endif // CONFIG_SSV_CABRIO_E
    
        //ssv6xxx_set_channel(sc, chan->hw_value);
        ssv6xxx_drv_start();
    
    
        return 0;
}



/**
 *  Entry point of the firmware code. After system booting, this is the
 *  first function to be called from boot code. This function need to 
 *  initialize the chip register, software protoctol, RTOS and create
 *  tasks. Note that, all memory resources needed for each software
 *  modulle shall be pre-allocated in initialization time.
 */
/* return int to avoid from compiler warning */

extern void lwip_reg_rx_data(void);
extern void lwip_sys_init(void);

int ssv6xxx_dev_init(int argc, char *argv[])
{
	/**
        * On simulation/emulation platform, initialize RTOS (simulation OS)
        * first. We use this simulation RTOS to create the whole simulation
        * and emulation platform.
        */
    ASSERT( OS_Init() == OS_SUCCESS );
    host_global_init();

	LOG_init(true, true, LOG_LEVEL_ON, LOG_MODULE_MASK(LOG_MODULE_EMPTY), false);
	LOG_out_dst_open(LOG_OUT_HOST_TERM, NULL);
	LOG_out_dst_turn_on(LOG_OUT_HOST_TERM);

    

    
        
    ASSERT( msg_evt_init() == OS_SUCCESS ); 

#if (CONFIG_USE_LWIP_PBUF==0) 
    ASSERT( PBUF_Init() == OS_SUCCESS );
#endif//#if CONFIG_USE_LWIP_PBUF 
    

            
    lwip_sys_init();
    
    /**
        * Initialize Host simulation platform. The Host initialization sequence
        * shall be the same as the sequence on the real host platform.
        *   @ Initialize host device drivers (SDIO/SIM/UART/SPI ...)
        */     
    {
    	
        ASSERT(ssv6xxx_drv_module_init() == true);
        LOG_PRINTF("Try to connecting CABRIO via %s...\n\r",INTERFACE);
        if (ssv6xxx_drv_select(INTERFACE) == false)
        {
       
            {     
            LOG_PRINTF("==============================\n\r");
        	LOG_PRINTF("Please Insert %S wifi device\n",INTERFACE);
			LOG_PRINTF("==============================\n\r");
        	}
			return -1;
        }

		
        if(ssv6xxx_wifi_init()!=SSV6XXX_SUCCESS)
			return -1;
    }    
    

    //------------------------------------------
    //Read E-fuse
    //efuse_read_all_map();
    //------------------------------------------
    if(ssv6xxx_chip_init() == 0)
    {
        LOG_PRINTF("ssv6xxx_chip_init fail\n\r");
		return 0;
    }
    #if AUTO_INIT_STATION
    sta_mode_on();    
    if(ssv6xxx_add_interface(gDeviceInfo->hw_mode) == 0)
	{	
	    LOG_PRINTF("ssv6xxx_add_interface fail\n\r");
		return 0;
    }
    #endif
    /**
        * Initialize TCP/IP Protocol Stack. If tcpip is init, the net_app_ask()
        * also need to be init.
        */
    
    tcpip_init(NULL, NULL);
    lwip_reg_rx_data();
	
    httpd_init();
    if(net_app_init()== 1)
	{	
		LOG_PRINTF("net_app_init fail\n\r");
		return 0;
    }
    netmgr_init();
    Cli_Init(argc, argv);


    /* Init simulation & emulation configuration */
#ifdef THROUGHPUT_TEST
//	ssv6xxx_wifi_reg_evt_cb(sdio_rx_test_evt_cb);
	_ssv6xxx_wifi_reg_rx_cb(sdio_rx_test_dat_cb, TRUE);
#endif

    ssv6xxx_wifi_cfg();
#if(MLME_SETTING ==1)
    mlme_init();    //MLME task initial  
#endif    
    return 1;
}

