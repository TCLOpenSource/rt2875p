/*
 * Realtek MMC/SD/SDIO driver
 *
 * Authors:
 * Copyright (C) 2008-2009 Realtek Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/mbus.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/scatterlist.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/mmc/host.h>
#include <asm/unaligned.h>

#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/mmc/card.h>
#include <linux/mmc/mmc.h>
#include <linux/mmc/sd.h>
#include <linux/workqueue.h>
#include <linux/completion.h>
#include <linux/pm.h>
#include <linux/pm_runtime.h>
#include "mmc_reg.h"
#include "rtksdio.h"
#include "rtksdio_dbg.h"
#include "rtksd_ops.h"
#include <mach/pcbMgr.h>
#ifdef CONFIG_RTK_KDRIVER_SUPPORT
#include <rtk_kdriver/rtk_gpio.h>
#else
#include <mach/rtk_gpio.h>
#endif

#ifdef CONFIG_OF
#include <linux/of_device.h>
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif
/************************************************************************
 *  Local Define
 ************************************************************************/
#define DRIVER_NAME "rtksdio"
#define BANNER      "Realtek Card-Reader Driver"
#define VERSION     "$Id: rtksdio.c RTK_287O 2018-05-18 21:30 $"

#ifdef CONFIG_ARM64
    extern u32 gic_irq_find_mapping(u32 hwirq);
    #define IRQ_GET_KERNEL_IRQ_NUM(hwirq)            gic_irq_find_mapping(hwirq)
#else
    #define IRQ_GET_KERNEL_IRQ_NUM(hwirq)            (hwirq)
#endif

#define cr_maskl(reg,msk,sht,val)  \
        cr_writel((cr_readl(reg) & ~(msk<<sht))|(val<<sht), reg)


/************************************************************************
 *  Local veriable
 ************************************************************************/
static int maxfreq = RTKSD_CLOCKRATE_MAX;
static int nodma;

static u32 rtk_power_bits = 0xFF;
static u32 rtk_cr_bus_wid = 0;
static bool rtkcr_drv_dis = false;
static u32 rtk_cr_sta_record = 0;


/************************************************************************
 *  Local Function Protal Type
 ************************************************************************/
static void rtksd_request(struct mmc_host *host, struct mmc_request *mrq);
static int rtksd_get_ro(struct mmc_host *mmc);
static void rtksd_set_ios(struct mmc_host *host, struct mmc_ios *ios);
static int rtkcr_get_cd(struct mmc_host *host);
static void rtksd_hw_reset(struct mmc_host *host);

/************************************************************************
 * Function Body
 ************************************************************************/
static
const struct mmc_host_ops rtksd_ops = {
	.request        = rtksd_request,
	.get_ro         = rtksd_get_ro,
	.set_ios        = rtksd_set_ios,
	.hw_reset       = rtksd_hw_reset,
	.get_cd         = rtkcr_get_cd,
};


static
int rtkcr_save_status(struct rtksd_host *sdport)
{
	u32 iobase = sdport->base_io;
	struct rtk_host_status* p_host_status;

	//p_host_status = (struct cr_host_status*)sdport->cur_host_status;
	p_host_status = &(sdport->cur_host_status);
	p_host_status->config1	= cr_readl(iobase + SD_CONFIGURE1);
	p_host_status->config2	= cr_readl(iobase + SD_CONFIGURE2);
	p_host_status->config3	= cr_readl(iobase + SD_CONFIGURE3);
	p_host_status->int_mask	= cr_readl(iobase + CR_INT_MASK);
	return 0;
}

static
int rtkcr_restore_status(struct rtksd_host *sdport)
{
	u32 iobase = sdport->base_io;
	struct rtk_host_status* p_host_status;

	//p_host_status = (struct cr_host_status*)sdport->cur_host_status;
	p_host_status = &(sdport->cur_host_status);
	cr_writel(p_host_status->config1,iobase  + SD_CONFIGURE1);
	cr_writel(p_host_status->config2,iobase  + SD_CONFIGURE2);
	cr_writel(p_host_status->config3,iobase  + SD_CONFIGURE3);
	cr_writel(p_host_status->int_mask,iobase + CR_INT_MASK);
	return 0;
}

static
int rtkcr_clear_status(struct rtksd_host *sdport)
{
	struct rtk_host_status* p_host_status;

	//p_host_status = (struct cr_host_status*)sdport->cur_host_status;
	p_host_status = &(sdport->cur_host_status);
	p_host_status->config1	= 0;
	p_host_status->config2	= 0;
	p_host_status->config3	= 0;
	p_host_status->int_mask	= 0;
	sdport->rtflags |= RTKCR_IDENT_STATE;
	sdport->rtflags &= ~RTKCR_FCARD_POWER;
	sdport->cur_clock = 0;
	return 0;
}

static
int rtk_wait_opt_end(struct rtksd_host *sdport,u8 cmdcode)
{
	struct sd_cmd_pkt * cmd_info = NULL;
	u32 iobase = sdport->base_io;
	int err = 0;

	BUG_ON(!sdport->cmd_info);
	cmd_info = sdport->cmd_info;

	/* record pervious cmd here*/
	rtk_cr_sta_record = (rtk_cr_sta_record<<8) & 0xffffff00;
	rtk_cr_sta_record |= (u32)(sdport->cmd_opcode);

   	cmd_info->cmdcode = cmdcode;
 	cmd_info->cmd->error = 0;

	sdport->int_status  = 0;
	sdport->sd_status   = 0;
	sdport->sd_status2  = 0;
	sdport->sd_trans    = 0;

	cr_writel(cmdcode, iobase+SD_TRANSFER);
	cr_writel(CMD_RESP_TIMEOUT_EN,iobase+SD_CONFIGURE3);         // enable RSP timeout check

	rtkcr_save_status(sdport);
	rtkcr_hold_int_dec(iobase);

	WARN_ON(sdport->task_state != STATE_IDLE);

	sdport->task_state = STATE_SENDING_CMD;
	mod_timer(&sdport->timer, jiffies + msecs_to_jiffies(600)); // max 250ms for SDHC, 500 for SDXC
	queue_delayed_work(sdport->cr_workqueue, &sdport->cr_work, 0);

	return err;
}

/*
 * reset and turn off card reader host
 */
static void rtkcr_enable_sb2_int(void)
{
	cr_writel(BIT(RTK_SD_INT_SHT_S)|RTK_CRT_WRT_MASK, RTK_SD_INTEN_S);	//SCPU
	cr_writel(BIT(RTK_SD_INT_SHT_K)|RTK_CRT_WRT_MASK, RTK_SD_INTEN_K);	//KCPU
}

static void rtkcr_disable_sb2_int(void)
{
	cr_writel(BIT(RTK_SD_INT_SHT_S), RTK_SD_INTEN_S);	//SCPU
	cr_writel(BIT(RTK_SD_INT_SHT_K), RTK_SD_INTEN_K);	//KCPU
}


void rtkcr_crt_onoff_host(struct rtksd_host *sdport,u8 clken)
{
	/* disable interrupt */
	rtkcr_disable_sb2_int();

	if (clken == CLK_ON) {
        cr_writel(BIT(RTK_SD_CLKEN_SHT), RTK_SD_CLKEN);
        cr_writel(BIT(RTK_SD_SRST_SHT), RTK_SD_SRST);
		udelay(10);
        cr_writel(BIT(RTK_SD_SRST_SHT) | RTK_CRT_WRT_MASK, RTK_SD_SRST);
        cr_writel(BIT(RTK_SD_CLKEN_SHT) | RTK_CRT_WRT_MASK, RTK_SD_CLKEN);

		/* enable interrupt */
		if(sdport->rtflags & RTKCR_FOPEN_INT){
			WARN_ON(1);
			rtkcr_enable_sb2_int();
		}
	} else {
        cr_writel(BIT(RTK_SD_CLKEN_SHT), RTK_SD_CLKEN);
        cr_writel(BIT(RTK_SD_SRST_SHT), RTK_SD_SRST);
	}
	udelay(10);
}

static
void rtkcr_reset_host(struct rtksd_host *sdport,u8 save_en)
{
	/*
	  reset all card-Reader module.
	  private_data that reserve for furture is NULL
	 */
	sdport->reset_event = 1;
	rtkcr_crt_onoff_host(sdport,CLK_ON);
	sdport->reset_event = 0;

	/* disable interrupt and clear pending bits */
	rtkcr_hold_int_all(sdport->base_io);
	rtkcr_clr_int_sta(sdport->base_io);
	rtkcr_clr_int_card_sta(sdport->base_io);

	/* rtkcr_en_int_card(sdport->base_io); */

//#define PAD_PIPE_FUNC_EN
#ifdef PAD_PIPE_FUNC_EN
/*
 new func: pad_pipe_inout
 For improve timing option.
	00: merlin2 version. No Pad DFF inout
	01: Pad DFF inout 1T
	10: Pad DFF inout 2T
	11: Pad DFF inout 4T
 */
	cr_maskl(iobase+CR_CTRL,0x3,24,0x2);	//10: Pad DFF inout 2T
#endif

	if(save_en){
		rtkcr_restore_status(sdport);
	}

    cr_writel(BIT(0), sdport->base_io + SD_AUTO_RST_FIFO);  // autoreset FIFO
}

/*
	hold clock pin mux to GPIO,
	until card insert, switch to card_reader

 */
static
void rtkcr_set_clk_muxpad(struct rtksd_host *sdport,u8 status)
{
	u32 reginfo;

	reginfo = cr_readl(RT_CR_CLK_MUX) & ~RT_CR_CLK_MASK;
	if(status == ON){
		/* set pin mux to Card-Reader CLK */
		 cr_writel(reginfo | RT_CR_CLK_VAL,RT_CR_CLK_MUX);
	}else{
		/* switch clock pin to GPIO and output low */
		cr_writel(RT_CR_GP_CLK,RT_CR_GPDATO_REG1);
		cr_writel(RT_CR_GP_CLK|BIT(0),RT_CR_GPDIR_REG1);
		cr_writel(reginfo | RT_CR_CLK_MASK,RT_CR_CLK_MUX);
	}
}

/****************************************************************************

 ****************************************************************************/

static void rtkcr_set_crt_muxpad(struct rtksd_host *sdport)
{
	u32 reginfo;

	reginfo = cr_readl(RT_CR_SRC_REG) & ~0x03UL;
	cr_writel(reginfo | ACTIVE_SRC, RT_CR_SRC_REG);

	//To set PinMux
	reginfo = cr_readl(RT_CR_MUX_REG1) & ~RT_CR_MUX_MASK1;
	cr_writel(reginfo | RT_CR_MUX_VAL1,RT_CR_MUX_REG1);

	reginfo = cr_readl(RT_CR_MUX_REG2) & ~RT_CR_MUX_MASK2;
	cr_writel(reginfo | RT_CR_MUX_VAL2,RT_CR_MUX_REG2);

	reginfo = cr_readl(RT_CR_MUX_REG3) & ~RT_CR_MUX_MASK3;
	cr_writel(reginfo | RT_CR_MUX_VAL3,RT_CR_MUX_REG3);

	/* set power pin */
	//cr_writel(RT_CR_POW_MASK|BIT(0),RT_CR_POW_GPDATO_REG);
	//cr_writel(RT_CR_POW_MASK|BIT(0),RT_CR_POW_GPDIR_REG);
	//cr_writel(cr_readl(RT_CR_POW_MUX_REG) | RT_CR_POW_MUX_MASK,RT_CR_POW_GPDIR_REG);

}

/****************************************************************************
Note: CLOCK pin was exclused in here. It will be seted at other func.
*****************************************************************************/
static
void rtkcr_set_crt_muxpad_out_low(struct rtksd_host *sdport)
{
	/*
	 Note: The level of GPIO will affect card detect level.
			Make sure which level is working well.
	*/
	/* set out high */
	cr_writel(RT_CR_GP_MASK1|BIT(0),RT_CR_GPDATO_REG1);

	/* set dir out */
	cr_writel(RT_CR_GP_MASK1|BIT(0),RT_CR_GPDIR_REG1);

	if(ACTIVE_SRC == 1){
		/* set data low */
		cr_writel(RT_CR_GP_MASK2,RT_CR_GPDATO_REG2);
		/* set dir out */
		cr_writel(RT_CR_GP_MASK2|BIT(0),RT_CR_GPDIR_REG2);
	}

	/* set Pinmux to GPIO */
	cr_writel(cr_readl(RT_CR_MUX_REG1) | RT_CR_MUX_LOW1,RT_CR_MUX_REG1);
	cr_writel(cr_readl(RT_CR_MUX_REG2) | RT_CR_MUX_LOW2,RT_CR_MUX_REG2);
	cr_writel(cr_readl(RT_CR_MUX_REG3) | RT_CR_MUX_LOW3,RT_CR_MUX_REG3);

}

/*
 * card power have ON/OFF item.
 * if want to power on card,the rtksd_host is necessary.
 * but power off is always working, no matter rtksd_host wether exist.
 */
static void rtkcr_card_power(struct rtksd_host *sdport,u8 status)
{
	//struct mmc_host *host = NULL;
	bool f_info = false;

	if(status==ON){ //power on

		if( sdport &&
		!(sdport->rtflags &  RTKCR_FCARD_POWER) )
		{
			f_info = true;
			/* ST_GPIO40 write 1; config out pin; set pinmux as GPIO */
			cr_writel(RT_CR_POW_MASK|BIT(0),RT_CR_POW_GPDATO_REG);
			cr_writel(RT_CR_POW_MASK|BIT(0),RT_CR_POW_GPDIR_REG);
			cr_writel(cr_readl(RT_CR_POW_MUX_REG) | RT_CR_POW_MUX_MASK,RT_CR_POW_MUX_REG);
			sdport->rtflags |= RTKCR_FCARD_POWER;
		}

	}else{

		if( sdport &&
			(sdport->rtflags |  RTKCR_FCARD_POWER) )
		{
			f_info = true;
			/* ST_GPIO40 write 0; config out pin; set pinmux as GPIO */
			cr_writel(RT_CR_POW_MASK,RT_CR_POW_GPDATO_REG);
			cr_writel(RT_CR_POW_MASK|BIT(0),RT_CR_POW_GPDIR_REG);
			cr_writel(cr_readl(RT_CR_POW_MUX_REG) | RT_CR_POW_MUX_MASK,RT_CR_POW_MUX_REG);

			/* switch pinmux to GPIO output low for reset card at here*/
			rtkcr_set_crt_muxpad_out_low(sdport);
			sdport->rtflags &= ~RTKCR_FCARD_POWER;
		}
	}

	if(f_info ){
		mmcinfo("%s(%d)power %s\n",
			__func__,__LINE__,
			(status == ON)?"On":"Off");
	}
}

static int rtkcr_card_det(struct rtksd_host *sdport)
{
	u32 iobase = sdport->base_io;
	int det = 0;

	if(cr_readl( iobase+CR_INT_STATUS ) & RTKCR_INT_CARD_DETEC)
		det = 1;

	return det;
}

static
void rtksd_read_rsp(struct rtksd_host *sdport,u32 *rsp, int reg_count)
{
	u32 iobase = sdport->base_io;

	mmcrtk("*rsp=0x%p; reg_count=%u\n",rsp,reg_count);

	if ( reg_count==6 ){
		rsp[0] = cr_readl(iobase+CR_CARD_RESP6_1) << 24 |
				 cr_readl(iobase+CR_CARD_RESP6_2) << 16 |
				 cr_readl(iobase+CR_CARD_RESP6_3) << 8 |
				 cr_readl(iobase+CR_CARD_RESP6_4);
		mmcrtk("rsp[0]=0x%x\n",rsp[0]);

	}else if(reg_count==16){
		rsp[0] = (cr_readl(iobase+RSP_17B_0) & 0xff00UL) << 16 |
				 (cr_readl(iobase+RSP_17B_1) & 0x00ffUL) << 16 |
				 (cr_readl(iobase+RSP_17B_1) & 0xff00UL) |
				 (cr_readl(iobase+RSP_17B_2) & 0x00ffUL);

		rsp[1] = (cr_readl(iobase+RSP_17B_2) & 0xff00UL) << 16 |
				 (cr_readl(iobase+RSP_17B_3) & 0x00ffUL) << 16 |
				 (cr_readl(iobase+RSP_17B_3) & 0xff00UL) |
				 (cr_readl(iobase+RSP_17B_4) & 0x00ffUL);

		rsp[2] = (cr_readl(iobase+RSP_17B_4) & 0xff00UL) << 16 |
				 (cr_readl(iobase+RSP_17B_5) & 0x00ffUL) << 16 |
				 (cr_readl(iobase+RSP_17B_5) & 0xff00UL) |
				 (cr_readl(iobase+RSP_17B_6) & 0x00ffUL);

		rsp[3] = (cr_readl(iobase+RSP_17B_6) & 0xff00UL) << 16 |
				 (cr_readl(iobase+RSP_17B_7) & 0x00ffUL) << 16 |
				 (cr_readl(iobase+RSP_17B_7) & 0xff00UL) |
				 0xff;

		mmcrtk("rsp[0]=0x%x\n",rsp[0]);
		mmcrtk("rsp[1]=0x%x\n",rsp[1]);
		mmcrtk("rsp[2]=0x%x\n",rsp[2]);
		mmcrtk("rsp[3]=0x%x\n",rsp[3]);
	}
}

static
void rtkcr_set_div(struct rtksd_host *sdport,u32 set_div)
{
	u32 iobase = sdport->base_io;
	u32 tmp_div;

	tmp_div = cr_readl(iobase+SD_CONFIGURE1);

	if((tmp_div & MASK_CLOCK_DIV) != set_div){
		tmp_div = tmp_div & ~MASK_CLOCK_DIV;
		cr_writel(tmp_div|set_div,iobase+SD_CONFIGURE1);

		mmcinfo("%s: %s\n",
			DRIVER_NAME,
			(set_div == CLOCK_DIV_NON)?
				"DIV_NON":
				(set_div == CLOCK_DIV_128)?
					"DIV_128":
					"DIV_256");
	}
}

static
void rtksd_set_bits(struct rtksd_host *sdport,u8 set_bit)
{
	u32 iobase = sdport->base_io;
	u32 tmp_bits;

	tmp_bits = cr_readl(iobase+SD_CONFIGURE1);
	if((tmp_bits & MASK_BUS_WIDTH) != set_bit ){
		tmp_bits &= ~MASK_BUS_WIDTH;
		cr_writel(tmp_bits|set_bit,iobase+SD_CONFIGURE1);
		CR_INFO( "%s: change to %s mode\n",
				DRIVER_NAME,bit_tlb[set_bit]);
	}

}

#if 0
static
void rtksd_set_phase(struct rtksd_host *sdport,u8 level)
{
	u32 iobase = sdport->base_io;
	u32 tmp_bits;

	tmp_bits = cr_readl(iobase+SD_CONFIGURE1) & ~DELAY_PHA_MASK;

	if(level == LVL_HI){
		tmp_bits |= DELAY_PHA_HIGH;
	}else{
		tmp_bits |= DELAY_PHA_NORM;
	}

	cr_writel(tmp_bits,iobase+SD_CONFIGURE1);

}
#endif


#if 0
static u32 rtkcr_get_clk(struct rtksd_host *sdport)
{
	u32 old_clk;

	old_clk = cr_readl(RTK_SD_CLKSEL);
	return old_clk;
}
#endif

#define  SRC_CLK_250 0
#define  SRC_CLK_295 1
static void rtkcr_clk_src_sel(struct rtksd_host *sdport,u32 src)
{
	u32 reginfo;

	reginfo = cr_readl(RTK_SD_CLKSEL) & ~(MASK_EMMC_SOURCECLKSEL);
/*
	Card Reader clock source selection
			0: from rbus_clk 250MHz
			1: from Audio 295MHz
*/
	reginfo |= src;
	cr_writel(reginfo,RTK_SD_CLKSEL);
}

static void rtkcr_set_clk(struct rtksd_host *sdport,u32 mmc_clk)
{
	u32 clksel_mask = 0;
	u32 clksel_sht = 0;
	u32 old_clk = 0;

	if(LOWEST_CLOCK < mmc_clk)  //to limit lowest speed
		  mmc_clk = LOWEST_CLOCK;

	clksel_mask = SD_CLKSEL_MASK;
	clksel_sht = SD_CLKSEL_SHT;
	mmc_clk = mmc_clk<<clksel_sht;

	hw_semaphore_try_lock();
	old_clk = cr_readl(RTK_SD_CLKSEL);

	if((old_clk & clksel_mask) != mmc_clk){
		if(!(cr_readl(sdport->base_io+SD_CONFIGURE1) & BIT(7)))
			CR_INFO( "%s: bus clock change to %s\n",
				DRIVER_NAME,clk_tlb[(mmc_clk>>clksel_sht) & 0x7]);

		cr_writel(BIT(RTK_SD_CLKEN_SHT),   RTK_SD_CLKEN);

		udelay(10);
		old_clk &= ~clksel_mask;
		cr_writel(old_clk|mmc_clk,   RTK_SD_CLKSEL);
		cr_writel(BIT(RTK_SD_CLKEN_SHT) | RTK_CRT_WRT_MASK, RTK_SD_CLKEN);

		udelay(10);
	}
	hw_semaphore_unlock();
}

static
void rtkcr_bus_speed_down(struct rtksd_host *sdport)
{
	u32 clksel_mask = 0;
	u32 clksel_sht = 0;
	u32 old_clk = 0;
	u32 mmc_clk = 0;

	clksel_mask = SD_CLKSEL_MASK;
	clksel_sht = SD_CLKSEL_SHT;

	mmc_clk = (cr_readl(RTK_SD_CLKSEL) & clksel_mask)>>clksel_sht;

	if(mmc_clk < LOWEST_CLOCK){
		mmc_clk++;
	}

	/*
	 * following code should same with rtkcr_set_clk(sdport,mmc_clk)
	 */
	mmc_clk = mmc_clk<<clksel_sht;

	old_clk = cr_readl(RTK_SD_CLKSEL);

	if((old_clk & clksel_mask) != mmc_clk){
		CR_INFO( "%s: bus clock down speed to %s\n",
				DRIVER_NAME,clk_tlb[(mmc_clk>>clksel_sht) & 0x7]);

		old_clk &= ~clksel_mask;
		cr_writel(old_clk|mmc_clk,   RTK_SD_CLKSEL);
	}

}

static
u8 rtksd_get_rsp_len(u8 rsp_para)
{

	switch (rsp_para & 0x3) {
	case 0:
		return 0;
	case 1:
		return 6;
	case 2:
		return 16;
	default:
		return 0;
	}
}

static
u32 rtksd_get_cmd_timeout(struct sd_cmd_pkt *cmd_info)
{
	struct rtksd_host *sdport   = cmd_info->sdport;
	u32 tmout = 0;
	u32 target_tmout = 0;

	target_tmout = 3000;  // default 1000ms
	tmout = msecs_to_jiffies(target_tmout);

	cmd_info->timeout = sdport->tmout = tmout;
	return 0;
}

static
int SD_SendCMDGetRSP_Cmd(struct sd_cmd_pkt *cmd_info)
{
	u8 cmd_idx              = cmd_info->cmd->opcode;
	u32 sd_arg              = cmd_info->cmd->arg;
	u8 rsp_para             = cmd_info->rsp_para;
	u32 *rsp                = (u32 *)&cmd_info->cmd->resp;
	struct rtksd_host *sdport = cmd_info->sdport;
	u32 iobase = sdport->base_io;
	int err;

	if ((sdport->rtflags & RTKCR_FOPEN_LOG)
	 || (sdport->rtflags & RTKCR_RECOVER_STATE))
	{
		CR_ALERT("%s: cmd_idx=%u\nsd_arg=0x%08x; rsp_para=0x%x\n",
				DRIVER_NAME,cmd_idx,sd_arg,rsp_para);
	}

	if(rsp == NULL) {
		BUG_ON(1);
	}

	cr_writel(0x40| cmd_idx,    iobase+SD_CMD0);
	cr_writel(sd_arg>>24,       iobase+SD_CMD1);
	cr_writel(sd_arg>>16,       iobase+SD_CMD2);
	cr_writel(sd_arg>>8,        iobase+SD_CMD3);
	cr_writel(sd_arg,           iobase+SD_CMD4);
	cr_writel((u32)rsp_para,    iobase+SD_CONFIGURE2);

	sdport->cmd_opcode = cmd_idx;

	err = rtk_wait_opt_end(sdport,SD_SENDCMDGETRSP);

	return err;
}

static
int SD_SendCMDGetRSP(struct sd_cmd_pkt * cmd_info)
{
	int rc;
	rc = SD_SendCMDGetRSP_Cmd(cmd_info);

	return rc;
}

static
int SD_Stream_Cmd(u16 cmdcode,struct sd_cmd_pkt *cmd_info)
{
	u8 cmd_idx              = cmd_info->cmd->opcode;
	u32 sd_arg              = cmd_info->cmd->arg;
	u8 rsp_para             = cmd_info->rsp_para;
	u32 *rsp                = (u32 *)&cmd_info->cmd->resp;
	u16 byte_count          = cmd_info->byte_count;
	u16 block_count         = cmd_info->block_count;
	struct rtksd_host *sdport = cmd_info->sdport;
	u32 iobase = sdport->base_io;
	u32 reginfo;
	int err;
	void *data              = cmd_info->dma_buffer;

	if((cmdcode==SD_NORMALWRITE) || (cmdcode==SD_NORMALREAD))
	{
		rsp_para|=CRC16_CAL_DIS;
		byte_count = 512;
	}

	if(data == NULL) {
		BUG_ON(1);
	}
	if(rsp == NULL) {
		BUG_ON(1);
	}

#ifdef TEST_POWER_RESCYCLE
	cmd_info->sdport->test_count++;
	mmcspec("test_count=%d\n",cmd_info->sdport->test_count);
#endif

	/* clear data at sram and trun-off boot mode */

	reginfo = cr_readl(iobase+CR_CTRL) & ~MASK_XFER_DIR;
	if(cmd_info->cmd->data->flags & MMC_DATA_READ){
		cr_writel(reginfo|DIR_READ,    iobase+CR_CTRL);
		cr_writel((unsigned int) data, iobase+CR_DMA_IN_ADDR);
		cr_writel((unsigned int) data, iobase+CR_DMA_OUT_ADDR);
	}else{
		cr_writel(reginfo|DIR_WRITE,   iobase+CR_CTRL);
		cr_writel((unsigned int) data, iobase+CR_DMA_IN_ADDR);
		cr_writel((unsigned int) data, iobase+CR_DMA_OUT_ADDR);
	}

	if ((sdport->rtflags & RTKCR_FOPEN_LOG)
	 || (sdport->rtflags & RTKCR_RECOVER_STATE))
	{
		CR_ALERT("%s: cmd_idx=%u\nsd_arg=0x%08x;rsp_para=0x%x;"
			/*"bye_cnt=0x%x;"*/
			"blk_cnt=0x%x; data=0x%x\n",
			DRIVER_NAME,
			cmd_idx,sd_arg,rsp_para,
			/*byte_count,*/
			block_count,(unsigned int) data);
	}

	cr_writel(0x40| cmd_idx,    iobase+SD_CMD0);           //0x10
	cr_writel(sd_arg>>24,       iobase+SD_CMD1);           //0x14
	cr_writel(sd_arg>>16,       iobase+SD_CMD2);           //0x18
	cr_writel(sd_arg>>8,        iobase+SD_CMD3);           //0x1C
	cr_writel(sd_arg,           iobase+SD_CMD4);           //0x20
	cr_writel(rsp_para,         iobase+SD_CONFIGURE2);     //0x0C
	cr_writel(byte_count,       iobase+SD_BYTE_CNT_L);     //0x24
	cr_writel(byte_count>>8,    iobase+SD_BYTE_CNT_H);     //0x28
	cr_writel(block_count,      iobase+SD_BLOCK_CNT_L);    //0x2C
	cr_writel(block_count>>8,   iobase+SD_BLOCK_CNT_H);    //0x30

	sdport->cmd_opcode = cmd_idx;
	sdport->cmd_info = cmd_info;
	err = rtk_wait_opt_end(sdport,cmdcode);

	return err;
}

static
u32 rtkcr_chk_cmdcode(struct mmc_command* cmd){
	u32 cmdcode;

	if(cmd->opcode < 56){
		cmdcode = (u32)rtk_sd_cmdcode[cmd->opcode][0];
		WARN_ON(cmd->data == NULL);
		if(cmd->data->flags & MMC_DATA_WRITE){
			if(cmd->opcode == 42)
				cmdcode = SD_NORMALWRITE;
			else if(cmd->opcode == 56)
				cmdcode = SD_AUTOWRITE2;
		}
	}else{
		cmdcode = SD_CMD_UNKNOW;
	}

	return cmdcode;

}

static
u32 rtkcr_chk_r1_type(struct mmc_command* cmd){
	return 0;
}

static
u8 rtk_get_rsp_type(struct mmc_command* cmd){
	u32 rsp_type;

	if ( mmc_resp_type(cmd)==MMC_RSP_R1 ){
		rsp_type = SD_R1;
	}else if ( mmc_resp_type(cmd)== MMC_RSP_R1B ){
		rsp_type = SD_R1b;
	}else if ( mmc_resp_type(cmd)== MMC_RSP_R2 ){
		rsp_type = SD_R2;
	}else if ( mmc_resp_type(cmd)== MMC_RSP_R3 ){
		rsp_type = SD_R3;
	}else if ( mmc_resp_type(cmd)== MMC_RSP_R6 ){
		rsp_type = SD_R6;
	}else if ( mmc_resp_type(cmd)== MMC_RSP_R7 ){
		rsp_type = SD_R7;
	}else{
		rsp_type = SD_R0;
	}
	return rsp_type;
}

static
int SD_Stream(struct sd_cmd_pkt *cmd_info)
{
	int err = 0;
	struct scatterlist *sg;
	u32 dir = 0;
	int dma_nents = 0;
	u32 dma_leng = 0;
	u32 tmp_leng = 0;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	u64 dma_addr = 0;
#else
	u32 dma_addr = 0;
#endif
	u16 cmdcode = 0;
	u32 blk_cnt = 0;

	struct mmc_host *host = cmd_info->sdport->mmc;
	struct rtksd_host *sdport = cmd_info->sdport;

	if(cmd_info->data->flags & MMC_DATA_READ){
		dir=DMA_FROM_DEVICE;
	}else{
		dir=DMA_TO_DEVICE;
	}

	cmd_info->data->bytes_xfered=0;
	dma_nents = dma_map_sg( mmc_dev(host), cmd_info->data->sg,
							cmd_info->data->sg_len,  dir);
	sg = cmd_info->data->sg;
	if((dma_nents < 0) || (dma_nents > host->max_segs) ){
		WARN(1, "Unexpected scatter&gather case. dma_nents=%d\n",
				dma_nents);
		return -ENXIO;
	}

#ifdef SHOW_MMC_PRD
	CR_ALERT("sg_len:%u\n",cmd_info->data->sg_len);
	CR_ALERT("sg:0x%p; dma_nents:%u\n",sg,dma_nents);
#endif

	tmp_leng = 0;
	dma_leng = sg_dma_len(sg);
	dma_addr = sg_dma_address(sg);

#ifdef SHOW_MMC_PRD
	CR_ALERT("dma_addr:0x%x; dma_leng:0x%x\n",dma_addr,dma_leng);
	mmcinfo("host=%p\n",host);
	if(host->card){
		mmcinfo("card=%p\n",host->card);
		if(mmc_card_blockaddr(host->card))
			CR_ALERT("arg:0x%x blk\n",cmd_info->cmd->arg);
		else
			CR_ALERT("arg:0x%x byte\n",cmd_info->cmd->arg);
	}
#endif

	blk_cnt = dma_leng/BYTE_CNT;
	if(dma_leng%BYTE_CNT){
		blk_cnt += 1;
	}

	/* rtk HW limite, one trigger 512 byte pass. */
	cmd_info->byte_count = BYTE_CNT;
	cmd_info->block_count = blk_cnt;

	if(dma_leng & 0x1ff){
		if(dma_leng > 512)
			WARN_ON(1);
		cmd_info->dma_buffer = (unsigned char *)sdport->scrip_buf_phy;
	}else{
		cmd_info->dma_buffer = (unsigned char *)dma_addr;
	}

	cmdcode = sdport->ops->chk_cmdcode(cmd_info->cmd);
	err = SD_Stream_Cmd(cmdcode,cmd_info);

	return err;
}

static void rtkcr_show_err_info(struct sd_cmd_pkt* cmd_info)
{
	struct rtksd_host* sdport = NULL;
	struct mmc_command* cmd = NULL;
	u32 blk_cnt_tmp = 0;

	sdport = cmd_info->sdport;
	cmd = cmd_info->cmd;

	if( opcode_d_type[cmd_info->cmdcode]){
		if(!(sdport->rtflags & RTKCR_RECOVER_STATE)){
			CR_ERR( "%s(%d):recover state alert\n",__func__,__LINE__);
			sdport->rtflags |= RTKCR_RECOVER_STATE;
		}

		blk_cnt_tmp = ((u32)cr_readl(sdport->base_io+SD_BLOCK_CNT_H)& 0xf)<<8
					  |(u32)cr_readl(sdport->base_io+SD_BLOCK_CNT_L);
	}

	if(!(sdport->rtflags & RTKCR_IDENT_STATE)){
		CR_ERR( "%s:cmd%u  %s! blk_cnt=%u\n"
				"SD_STAT=0x%08x SD_STAT2=0x%08x INT_STAT=0x%08x SD_TRANS=0x%08x\n"
				"cmd_step[0x%08x]\n",
				DRIVER_NAME,sdport->cmd_opcode,
				(cmd_info->cmd->error == -EILSEQ )?
				"cmd decode fail":"rtk_opt_timeout",
				blk_cnt_tmp,
				sdport->sd_status,sdport->sd_status2,
				sdport->int_status,sdport->sd_trans,
				rtk_cr_sta_record );

	}
}

static void rtkcr_finish_sg(struct sd_cmd_pkt* cmd_info)
{
	u32 dir = 0;
	struct mmc_host *host = cmd_info->sdport->mmc;
	struct rtksd_host* sdport = cmd_info->sdport;
	struct scatterlist *sg;
	u32 dma_leng = 0;
	u32 dma_addr = 0;

	if(cmd_info->data){
		sg = cmd_info->data->sg;
		dma_leng = sg_dma_len(sg);
		dma_addr = sg_dma_address(sg);

		if(dma_leng & 0x1ff){
			memcpy((void*)phys_to_virt(dma_addr),
				(void*)sdport->scrip_buf , dma_leng);
		}

		if (!(cmd_info->cmd->error)){
			if(host->card && mmc_card_blockaddr(host->card))
				cmd_info->cmd->arg += cmd_info->block_count;
			else
				cmd_info->cmd->arg += dma_leng;

			cmd_info->data->bytes_xfered += dma_leng;
		}

		if(cmd_info->data->flags & MMC_DATA_READ){
			dir=DMA_FROM_DEVICE;
		}else{
			dir=DMA_TO_DEVICE;
		}
		dma_unmap_sg( mmc_dev(host), cmd_info->data->sg,
			cmd_info->data->sg_len,  dir);
	}
}

static void rtkcr_polling_work(struct work_struct *work)
{
	struct rtksd_host *sdport = container_of(work, struct rtksd_host,
			cr_work.work);
	struct sd_cmd_pkt* cmd_info = sdport->cmd_info;
	u32 state = 0;
	unsigned long flags;
	u8 cmdcode = 0;
	u32 bus_status, cmd_state, data_state;
	u32 reginfo = 0;

	BUG_ON(!sdport);
	if(cmd_info == NULL){
		//WARN_ON(1,"%s: cmd_info lose!\n",DRIVER_NAME);
		WARN_ON(1);
		return;
	}
	spin_lock_irqsave(&sdport->lock,flags);

	state = sdport->task_state;
	cmdcode = cmd_info->cmdcode;

	switch (state) {
	case STATE_SENDING_CMD:

		if(sdport->reset_event){
			CR_ALERT( "%s(%d):wait for reset finish\n",__func__,__LINE__);
			queue_delayed_work(sdport->cr_workqueue, &sdport->cr_work, 1);
			break;
		}

		bus_status = cr_readl(sdport->base_io + SD_BUS_STATUS);
		cmd_state = cr_readl(sdport->base_io + SD_CMD_STATE);
		data_state = cr_readl(sdport->base_io + SD_DATA_STATE);
		if (!(bus_status & DAT0_LEVEL) ||
			!(cmd_state & 0x80) ||
			!(data_state & 0x80)) {
			//CR_INFO("!!! WAIT BUSY\n");
			//CR_INFO("bus_status = %x, cmd_state = %x,"
			//"data_state = %x\n", bus_status, cmd_state, data_state);
			queue_delayed_work(sdport->cr_workqueue, &sdport->cr_work, 1);
			break;
		}

		reginfo = cr_readl(sdport->base_io+CR_INT_STATUS);
		if (reginfo & RTKCR_INT_CARD_DETEC) {
			CR_ERR("%s(%d): no card exist, abort this cmd\n",
					DRIVER_NAME, __LINE__);
			sdport->task_state = state = STATE_DATA_ERROR;
			if(sdport->mmc->card)
			{
				mmc_card_set_removed(sdport->mmc->card);
			}
			tasklet_schedule(&sdport->polling_tasklet);
			break;  // leave workqueue
		}

		if(sdport->rtflags & RTKCR_FOPEN_INT) {
			rtkcr_en_int_dec(sdport->base_io);
		}else{
			rtkcr_hold_int_dec(sdport->base_io);
		}

		rtksd_get_cmd_timeout(cmd_info);
		cr_writel(cmdcode|START_EN, sdport->base_io + SD_TRANSFER);
		smp_wmb();

		if (timer_pending(&sdport->timer))
			del_timer_sync(&sdport->timer);

		mod_timer(&sdport->timer, jiffies + cmd_info->timeout);

		if (sdport->rtflags & RTKCR_FOPEN_INT)
			sdport->task_state = state = STATE_IDLE;
		else {
			sdport->task_state = state = STATE_SENDING_DATA;
			tasklet_schedule(&sdport->polling_tasklet);
		}
		break;

	default:
		CR_WARNING("%s: state SHOULD NOT BE %d in polling_work!\n", DRIVER_NAME, state);
		reginfo = cr_readl(sdport->base_io+CR_INT_STATUS);
		if (reginfo & RTKCR_INT_CARD_DETEC) {
			CR_ERR("%s(%d): no card exist, abort this cmd\n",
					DRIVER_NAME, __LINE__);
			sdport->task_state = state = STATE_DATA_ERROR;
			if(sdport->mmc->card)
			{
				mmc_card_set_removed(sdport->mmc->card);
			}
			tasklet_schedule(&sdport->polling_tasklet);
		}
		break;
	}

	spin_unlock_irqrestore(&sdport->lock, flags);
}

static void rtkcr_polling_tasklet(unsigned long param)
{
	struct mmc_request* mrq = NULL;
	struct rtksd_host* sdport = NULL;
	struct mmc_command* cmd = NULL;
	struct sd_cmd_pkt* cmd_info = NULL;
	u32 iobase = 0;
	u32 state = 0;
	u32 prev_state = 0;
	u8 cmdcode = 0;
	u8 rsp_len = 0;
	u32 *rsp = NULL;
	unsigned long last_jiffies;
	unsigned long flags;

	last_jiffies = jiffies;

	sdport = (struct rtksd_host *)param;
	BUG_ON(!sdport);
	BUG_ON(!(sdport->cmd_info));

#define SLICE_TIME 1

	/* lock before check task_state*/
	spin_lock_irqsave(&sdport->lock,flags);

	if(sdport->task_state == STATE_IDLE){
		spin_unlock_irqrestore(&sdport->lock,flags);
		return;
	}

	iobase = sdport->base_io;
	cmd_info = sdport->cmd_info;
	cmd = cmd_info->cmd;
	cmdcode = cmd_info->cmdcode;

	state = sdport->task_state;

	do {
		prev_state = state;
		switch (state) {
		case STATE_IDLE:
			break;

		case STATE_SENDING_DATA:

			if (!(sdport->rtflags & RTKCR_FOPEN_INT))
				rtkcr_get_int_sta(iobase,&sdport->int_status);
			if (!(sdport->rtflags & RTKCR_FOPEN_INT) &&
				!(sdport->int_status & (RTKCR_INT_DECODE_ERROR | RTKCR_INT_DECODE_FINISH))) {
				tasklet_schedule(&sdport->polling_tasklet);
				break;
			}

			rtkcr_get_sd_sta(iobase,&sdport->sd_status);
			rtkcr_get_sd_sta2(iobase,&sdport->sd_status2);
			rtkcr_get_sd_trans(iobase,&sdport->sd_trans);

			if((sdport->int_status & RTKCR_INT_DECODE_ERROR)
			|| (sdport->sd_trans  & ERR_BIT)) {
				cmd_info->cmd->error = -EILSEQ;
				if (!(sdport->rtflags & RTKCR_FOPEN_INT))
					rtkcr_clr_int_dec_sta(sdport->base_io);
				state = STATE_DATA_ERROR;

				if (sdport->int_status & RTKCR_INT_CARD_DETEC) {
					CR_ERR("%s: card is removed\n", DRIVER_NAME);
					cmd_info->cmd->error = -ENOMEDIUM;
					if(sdport->mmc->card)
					{
						mmc_card_set_removed(sdport->mmc->card);
					}
				}

				break;
			}
			if(sdport->int_status & RTKCR_INT_DECODE_FINISH) {

				if (!(sdport->rtflags & RTKCR_FOPEN_INT)) {
						rtkcr_clr_int_dec_sta(sdport->base_io);
				}

				state = STATE_DATA_COMPLETE;
				break;
			}

			break;
		case STATE_POST_POLLING:
			prev_state = state = STATE_DATA_COMPLETE;

		case STATE_DATA_ERROR:
			prev_state = state = STATE_DATA_COMPLETE;

		case STATE_DATA_COMPLETE:
			rsp_len = cmd_info->rsp_len;
			rsp = (u32 *)&cmd_info->cmd->resp;
			del_timer_sync(&sdport->timer);
			rtksd_read_rsp(sdport,rsp, rsp_len);

			rtkcr_finish_sg(cmd_info);

			if (cmd_info->cmd->error)
			{
				rtkcr_show_err_info(cmd_info);

				/* recover pinmux and host here */
				sdport->ops->set_crt_muxpad(sdport);
				spin_unlock_irqrestore(&sdport->lock,flags);
				sdport->ops->reset_host(sdport,1);
				spin_lock_irqsave(&sdport->lock,flags);
			}else{
				if ((sdport->rtflags & RTKCR_RECOVER_STATE)
				 && (opcode_d_type[cmdcode]))
				{
					CR_ERR( "%s(%d):recover state finish\n",__func__,__LINE__);
					sdport->rtflags &= ~RTKCR_RECOVER_STATE;
				}
				if(cmd_info->cmd->opcode == MMC_SELECT_CARD)
				{
					sdport->rtflags &= ~RTKCR_IDENT_STATE;
					sdport->rtflags |= (RTKCR_FCARD_DETECTED);
					/*
					 I don't understand why stream cmd can not use clock that divided.
					 remove divid anyway.
					 */
					rtkcr_set_div(sdport,CLOCK_DIV_NON);
					spin_unlock_irqrestore(&sdport->lock,flags);
					rtkcr_set_clk(sdport,LOWEST_CLOCK);
					spin_lock_irqsave(&sdport->lock,flags);

				}
				if(sdport->mrq->stop){
					if(((cmd_info->cmd->opcode == 18) && (cmdcode == SD_AUTOREAD2))
					 || ((cmd_info->cmd->opcode == 25) && (cmdcode == SD_AUTOWRITE2)))
					{
						sdport->mrq->stop->error = -EILSEQ;
					}
				}
			}
			if(sdport->stop_info){
				sdport->stop_info = NULL;
			}
			prev_state = state = STATE_REQUEST_END;

		case STATE_REQUEST_END:
			if(sdport->cmd_info){
				kfree(sdport->cmd_info);
				sdport->cmd_info = NULL;
			}else{
				WARN_ON(1);
			}
			sdport->task_state = prev_state = state = STATE_IDLE;
			mrq = sdport->mrq;
			sdport->mrq = NULL;
			spin_unlock_irqrestore(&sdport->lock,flags);
			mmc_request_done(sdport->mmc, mrq);
			return;

		default:
			WARN_ON(1);

		}
	} while (state != prev_state);

	sdport->task_state = state;
	spin_unlock_irqrestore(&sdport->lock,flags);
}

static
void rtksd_send_command(struct rtksd_host *sdport, struct mmc_command *cmd)
{
	struct sd_cmd_pkt *cmd_info = NULL;

	if ( !sdport || !cmd ){
		WARN_ON(1);
		return;
	}

	cmd_info = sdport->cmd_info;
	cmd_info->cmd    = sdport->mrq->cmd;
	cmd_info->sdport = sdport;
	cmd_info->rsp_para = rtk_get_rsp_type(cmd_info->cmd);
	cmd_info->rsp_len  = rtksd_get_rsp_len(cmd_info->rsp_para);

	if (cmd->data){
		cmd_info->data = cmd->data;

		/*
		 * write protect should check by mmc layer, we do not check here.
		 * But ret_ro func must be finish
		 */
		SD_Stream(cmd_info);

	}else{
		SD_SendCMDGetRSP(cmd_info);
	}
}

static
void rtksd_request(struct mmc_host *host, struct mmc_request *mrq)
{
	struct rtksd_host *sdport;
	struct mmc_command *cmd;
	struct sd_cmd_pkt *cmd_info = NULL;
	unsigned long flags;
	sdport = mmc_priv(host);

	WARN_ON(!sdport);
	WARN_ON(sdport->mrq != NULL);

	spin_lock_irqsave(&sdport->lock,flags);

	cmd_info = kmalloc(sizeof(struct sd_cmd_pkt), GFP_KERNEL);
	if(!cmd_info){
		spin_unlock_irqrestore(&sdport->lock, flags);
		WARN_ON(1);
		CR_ALERT("can't request memory for cmd_info!!!!!\n");
		mrq->cmd->error = -ENOMEM;
		mmc_request_done(host, mrq);
		return;
	}

	memset(cmd_info, 0, sizeof(struct sd_cmd_pkt));

	sdport->mrq = mrq;
	sdport->cmd_info = cmd_info;
	cmd = mrq->cmd;

	WARN_ON(!cmd);

	rtksd_send_command(sdport, cmd);
	spin_unlock_irqrestore(&sdport->lock,flags);
}

/************************************************************************
 *
 ************************************************************************/
static
void rtksd_hw_reset(struct mmc_host *host)
{
	struct rtksd_host *sdport;
	unsigned long flags;

	sdport = mmc_priv(host);

	if(sdport && sdport->ops){
		sdport->ops->reset_host(sdport,0);
		spin_lock_irqsave(&sdport->lock,flags);
		sdport->ops->set_crt_muxpad(sdport);
		rtkcr_clear_status(sdport);
		spin_unlock_irqrestore(&sdport->lock,flags);
	}

}

static void rtksd_set_ios(struct mmc_host *host, struct mmc_ios *ios)
{
	struct rtksd_host *sdport;
	u32 tmp_clock = 0;
	u32 tmp_bits = 0;
	u32 tmp_div = 0;

	sdport = mmc_priv(host);
	mmcinfo("ios->bus_mode = %u\n",ios->bus_mode);
	mmcinfo("ios->clock = %u\n",ios->clock);

	/*
	About bus_mode ==>  MMC_BUSMODE_PUSHPULL/MMC_BUSMODE_OPENDRAIN
	 In fact, RTK have not any setting for PUSHPULL or OPENDRAIN.
	 So, do not care about this.
	 */
	if (ios->bus_width == MMC_BUS_WIDTH_8){
		tmp_bits = BUS_WIDTH_8;
	}else if (ios->bus_width == MMC_BUS_WIDTH_4){
		tmp_bits = BUS_WIDTH_4;
	}else{
		tmp_bits = BUS_WIDTH_1;
	}

	if(ios->clock > 400000){
		if(ios->clock >= 50000000){
			tmp_clock = CARD_SWITCHCLOCK_50MHZ;
		}else if(ios->clock >= 37000000){
			tmp_clock = CARD_SWITCHCLOCK_37MHZ;
		}else if(ios->clock >= 30000000){
			tmp_clock = CARD_SWITCHCLOCK_30MHZ;
		}else if(ios->clock >= 25000000){
			tmp_clock = CARD_SWITCHCLOCK_25MHZ;
		}else if(ios->clock >= 18000000){
			tmp_clock = CARD_SWITCHCLOCK_18MHZ;
		}else{
			tmp_clock = CARD_SWITCHCLOCK_15MHZ;
		}
		tmp_div = CLOCK_DIV_NON;

		// designer suggest to use 1/4T phase shift
		cr_writel(BIT(3), sdport->base_io + SD_SAMPLE_POINT_CTL);
	}else{
		if(ios->clock > 300000){
			/* (50Mhz/128) = 390KHz */
			tmp_clock = CARD_SWITCHCLOCK_50MHZ;
			tmp_div = CLOCK_DIV_128;
		}else if(ios->clock > 200000){
			/* (37Mhz/128) = 289KHz */
			tmp_clock = CARD_SWITCHCLOCK_37MHZ;
			tmp_div = CLOCK_DIV_128;
		}else if(ios->clock > 100000){
			/* (50Mhz/256) = 195KHz */
			tmp_clock = CARD_SWITCHCLOCK_50MHZ;
			tmp_div = CLOCK_DIV_256;
		}else{
			/* (30Mhz/256) = 117KHz */
			tmp_clock = CARD_SWITCHCLOCK_30MHZ;
			tmp_div = CLOCK_DIV_256;
		}

		cr_writel(0, sdport->base_io + SD_SAMPLE_POINT_CTL);
	}
	rtksd_set_bits(sdport,tmp_bits);
	sdport->cur_clock = ios->clock;

	if(sdport->rtflags & RTKCR_IDENT_STATE)
		rtkcr_set_div(sdport,tmp_div);
	rtkcr_set_clk(sdport,tmp_clock);

	if (ios->power_mode == MMC_POWER_OFF){
		sdport->rtflags |= RTKCR_IDENT_STATE;
		rtkcr_card_power(sdport,OFF);
	}else{  //MMC_POWER_UP or MMC_POWER_ON
		/* have any setting for power up mode ? */
		rtkcr_card_power(sdport,ON);
	}
}

static
void rtkcr_chk_card_insert(struct rtksd_host *sdport)
{
	struct mmc_host *host=sdport->mmc;

	rtkcr_clk_src_sel(sdport,CR_SRC_CLK_295M);
	host->ops = &rtksd_ops;
	sdport->rtflags |= (RTKCR_IDENT_STATE);
}

/******************************************************************************
in_interrupt() if not 0, it's at interrupt enviroment currently
inMatomic() if not 0, Do no schedule, includde HW/SW interrupt
	and any other process keep spin_lock

include at <asm/hardirq.h>
 ******************************************************************************/
static
int rtkcr_get_cd(struct mmc_host *host)
{
	struct rtksd_host *sdport = mmc_priv(host);
	u32 iobase;
	u32 reginfo;
	unsigned int present = 0;

	spin_lock_bh(&sdport->lock);
	iobase = sdport->base_io;
	reginfo = cr_readl(iobase+CR_INT_STATUS);
	/* CD bit is 0 when detected, Maybe this is PCB issue */
	present = !(reginfo & RTKCR_INT_CARD_DETEC);

//#define INITIAL_DEBUG
#ifdef INITIAL_DEBUG
	CR_ALERT("%s(%d) Card status change, reginfo=0x%x, int_status=0x%x\n",
		__func__,__LINE__,
		reginfo,sdport->int_status_old);

#endif

	if (present && sdport->int_status_old & RTKCR_INT_CARD_DETEC) {
		CR_NOTICE("%s: card is present\n", DRIVER_NAME);
		sdport->rtflags |= RTKCR_FCARD_DETECTED;
		sdport->rtflags |= (RTKCR_IDENT_STATE);
		rtkcr_set_clk_muxpad(sdport,ON);
	}
	else if (!present && !(sdport->int_status_old & RTKCR_INT_CARD_DETEC)) {
		CR_NOTICE("%s: card is not present\n", DRIVER_NAME);
		sdport->rtflags &= ~(RTKCR_FCARD_DETECTED);
		sdport->rtflags &= ~(RTKCR_IDENT_STATE);
		rtkcr_set_clk_muxpad(sdport,OFF);
	}

    rtkcr_clr_int_card_sta(sdport->base_io);
	sdport->int_status_old = reginfo;
	spin_unlock_bh(&sdport->lock);

	return present;
}

static
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
void rtksd_timeout_timer(struct timer_list *t)
#else
void rtksd_timeout_timer(unsigned long data)
#endif
{
	struct rtksd_host *sdport;

	unsigned long flags;
	//int i;
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	sdport = from_timer(sdport, t, timer);
#else
	sdport = (struct rtksd_host *)data;
#endif
	WARN_ON(!sdport);
	/*
	 In some case. the host may hang on by cmd fail.
	 That is nothing we can to prevent system show message. like below
		"You have accessed an reserved register address"
	 Don't care about this!
	 */
	spin_lock_irqsave(&sdport->lock, flags);

#if 0
	// dump all register
	CR_ERR("\n======================================\n");
	for (i=0; i<81; i+=4) {
		CR_ERR( "REG 0x%08x = 0x%08x, REG 0x%08x = 0x%08x, "
			"REG 0x%08x = 0x%08x, REG 0x%08x = 0x%08x \n",
			sdport->base_io + i*4, cr_readl(sdport->base_io + i*4),
			sdport->base_io + (i+1)*4, cr_readl(sdport->base_io + (i+1)*4),
			sdport->base_io + (i+2)*4, cr_readl(sdport->base_io + (i+2)*4),
			sdport->base_io + (i+3)*4, cr_readl(sdport->base_io + (i+3)*4) );
	}

	CR_ERR("SCPU_INT_MUX(0x%08x) = 0x%08x\n ", 0xb8000290,
					cr_readl(0xb8000290));
	CR_ERR("VCPU_INT_MUX(0x%08x) = 0x%08x\n ", 0xb8000294,
					cr_readl(0xb8000294));
	CR_ERR("ACPU_INT_MUX(0x%08x) = 0x%08x\n ", 0xb8000298,
					cr_readl(0xb8000298));
	CR_ERR("KCPU_INT_MUX(0x%08x) = 0x%08x\n ", 0xb800029c,
					cr_readl(0xb800029c));

	CR_ERR("WRAPPER SCPU SEL(0x%08x) = 0x%08x\n", 0xb8010800, cr_readl(0xb8010800));
	CR_ERR("WRAPPER SCPU INTSTS(0x%08x) = 0x%08x\n", 0xb8010804, cr_readl(0xb8010804));
	CR_ERR("WRAPPER SCPU INTMASK(0x%08x) = 0x%08x\n", 0xb8010808, cr_readl(0xb8010808));
	CR_ERR("SD BUS PRIORITY is 0x%08x\n", cr_readl(0xb801c004));
	CR_ERR("\n======================================\n");
	if (sdport->task_state == STATE_SENDING_CMD) {
		CR_ERR("%s: cmd is not sent yet!\n", DRIVER_NAME);
	}
	else
		CR_ERR( "%s: card access time out!\n",DRIVER_NAME);
#endif

	rtkcr_hold_int_dec(sdport->base_io);
	rtkcr_clr_int_dec_sta(sdport->base_io);

	sdport->mrq->cmd->error = -ETIMEDOUT;
	sdport->task_state = STATE_DATA_ERROR;

	tasklet_schedule(&sdport->polling_tasklet);

	spin_unlock_irqrestore(&sdport->lock, flags);
	/*
	 * At int enable. it has not schedule tasklet before. schedule here
	 * No matter int or polling, call  tasklet_schedule() is ok
	 */
}

static
irqreturn_t rtksd_irq(int irq, void *dev)
{
	struct rtksd_host *sdport = NULL;
	struct sd_cmd_pkt * cmd_info = NULL;
	int irq_handled = 0;
	u32 int_mask = 0;
	u32 state = 0;

#if 0
	unsigned int reg1;
	unsigned int reg2;
	sdport = dev;

	CR_ALERT( "%s: int hit!!!\n",DRIVER_NAME);
	reg1 = cr_readl(sdport->base_io+CR_INT_STATUS);
	reg2 = cr_readl(sdport->base_io+CR_INT_MASK);
	if(reg1 || reg2){
		CR_ALERT("%s(%d)0x%08x=0x%08x; 0x%08x=0x%08x\n",
			__func__,__LINE__,
			sdport->base_io+CR_INT_STATUS,cr_readl(sdport->base_io+CR_INT_STATUS),
			sdport->base_io+CR_INT_MASK,cr_readl(sdport->base_io+CR_INT_MASK));
		rtkcr_hold_int_card(sdport->base_io);
	}

#else

	sdport = dev;
	spin_lock(&sdport->lock);
	if(sdport->reset_event){
#if 0
		CR_ALERT( "%s: sd clock off at reset process, no interrupt service\n",
				DRIVER_NAME);

		CR_ALERT( "%s: RTK_SD_CLKEN=0x%08x\n",
				DRIVER_NAME,(u32)cr_readl(RTK_SD_CLKEN));
#endif
		goto OUT;
	}

	int_mask = cr_readl(sdport->base_io + CR_INT_MASK);
	if (!(int_mask & (RTKCR_DECODE_ERR_MASK | RTKCR_DECODE_FINISH_MASK)))
		goto OUT;

	rtkcr_get_int_sta(sdport->base_io,&sdport->int_status);

	if (sdport->int_status & int_mask)
	{
		cmd_info = sdport->cmd_info;

		rtkcr_hold_int_dec(sdport->base_io);
		rtkcr_clr_int_dec_sta(sdport->base_io);

		state = STATE_SENDING_DATA;

		if (sdport->int_status & RTKCR_INT_DECODE_ERROR) {
			cmd_info->cmd->error = -EILSEQ;
		}

		sdport->task_state = state;
		tasklet_schedule(&sdport->polling_tasklet);

		irq_handled = 1;
	}

OUT:
	spin_unlock(&sdport->lock);
	if(irq_handled)
		return IRQ_HANDLED;
	else
		return IRQ_NONE;
#endif
}

/***************************
 * check read only func
 *
 *
 ***************************/
static
int rtksd_get_ro(struct mmc_host *mmc)
{
	struct rtksd_host *sdport;
	unsigned long flags;

	sdport = mmc_priv(mmc);
	spin_lock_irqsave(&sdport->lock,flags);

	if ( sdport->int_status_old & RTKCR_INT_WRT_PROTEC ){
		CR_INFO( "%s: SD card locked\n",
				DRIVER_NAME);
		sdport->wp = 1;
	}else{
		sdport->wp = 0;
	}

	sdport->wp = 0;
	spin_unlock_irqrestore(&sdport->lock, flags);
	return  sdport->wp;
}

static
void rtkcr_reset_card(struct rtksd_host *sdport)
{
	rtkcr_card_power(sdport,OFF);
	rtkcr_mdelay(1);
	rtkcr_card_power(sdport,ON);
	rtkcr_mdelay(1);
}

static struct rtk_host_ops sdio_ops = {
	.func_irq       = NULL,
	.re_init_proc   = NULL,
	.card_det       = rtkcr_card_det,
	.card_power     = rtkcr_card_power,
	.chk_card_insert= rtkcr_chk_card_insert,
	.set_crt_muxpad = rtkcr_set_crt_muxpad,
	.set_clk        = NULL,
	.reset_card     = rtkcr_reset_card,
	.reset_host     = rtkcr_reset_host,
	.bus_speed_down = rtkcr_bus_speed_down,
	.get_cmdcode    = NULL,
	.get_r1_type    = NULL,
	.chk_cmdcode    = rtkcr_chk_cmdcode,
	.chk_r1_type    = rtkcr_chk_r1_type,
};


/* mmc device attribute *********************************************************** */

static ssize_t
cr_getCID_dev_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct mmc_host * host = dev_get_drvdata(dev);
	struct mmc_card *card = NULL;

	if(host)
		card = host->card;

	if(host && card){
		u8 *pcid = (u8 *)card->raw_cid;

		return sprintf(buf, "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x \n",
			pcid[3] ,pcid[2] ,pcid[1] ,pcid[0],
			pcid[7] ,pcid[6] ,pcid[5] ,pcid[4],
			pcid[11],pcid[10],pcid[9] ,pcid[8],
			pcid[15],pcid[14],pcid[13],pcid[12]);
	}
	return sprintf(buf, "0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 0x00 \n");
}

static ssize_t
cr_getCID_dev_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return 1;
}
DEVICE_ATTR(cr_getCID, S_IRUGO | S_IWUSR,
			cr_getCID_dev_show,cr_getCID_dev_store);

static ssize_t
cr_chkwp_dev_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct mmc_host * host = dev_get_drvdata(dev);

	if (host->ops->get_ro){
		return sprintf(buf, "%u\n",host->ops->get_ro(host) );
	}else{
		return sprintf(buf, "0\n");
	}
}

static ssize_t
cr_chkwp_dev_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return 1;
}
DEVICE_ATTR(cr_chkwp, S_IRUGO | S_IWUSR,
			cr_chkwp_dev_show,cr_chkwp_dev_store);

static ssize_t
cr_offline_dev_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct mmc_host * host = NULL;
	struct rtksd_host *sdport = NULL;
	unsigned long flags;

	host = dev_get_drvdata(dev);
	if(host){
		sdport = mmc_priv(host);

		if(sdport == NULL){
			return sprintf(buf, "no sdport found!!!\n");
		}
	}else{
		return sprintf(buf, "no mmc host found!!!\n");
	}

	/* should turn off plug timer */
	spin_lock_irqsave(&sdport->lock, flags);
	//sdport->ins_event = EVENT_REMOV;    //|EVENT_USER;
	sdport->rtflags &= ~(RTKCR_FCARD_DETECTED);
	spin_unlock_irqrestore(&sdport->lock, flags);

	rtkcr_set_clk_muxpad(sdport,OFF);

	if(sdport->mmc->card)
		mmc_card_set_removed(sdport->mmc->card);

	mmc_detect_change(sdport->mmc, 0);

	return sprintf(buf, "off-line card\n");
}

static ssize_t
cr_offline_dev_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return 1;
}
DEVICE_ATTR(cr_offline, S_IRUGO | S_IWUSR,
			cr_offline_dev_show,cr_offline_dev_store);

static ssize_t
cr_rescan_dev_show(struct device *dev, struct device_attribute *attr,
	char *buf)
{
	struct mmc_host * host = dev_get_drvdata(dev);
	struct rtksd_host *sdport = mmc_priv(host);
	unsigned long flags;

	/* should turn off plug timer */
	spin_lock_irqsave(&sdport->lock, flags);
	sdport->ins_event = EVENT_REMOV;    //|EVENT_USER;
	spin_unlock_irqrestore(&sdport->lock, flags);

	flush_scheduled_work();
	/* should turn of plug_timer */

	spin_lock_irqsave(&sdport->lock, flags);
	sdport->ins_event = EVENT_INSER;
	spin_unlock_irqrestore(&sdport->lock, flags);

	return sprintf(buf, "re-scan card\n");
}

static ssize_t
cr_rescan_dev_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	return 1;
}
DEVICE_ATTR(cr_rescan, S_IRUGO | S_IWUSR,
			cr_rescan_dev_show,cr_rescan_dev_store);

static ssize_t
cr_open_log_dev_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	struct mmc_host * host = dev_get_drvdata(dev);
	struct rtksd_host *sdport = mmc_priv(host);

	if(sdport->rtflags & RTKCR_FOPEN_LOG){
		sdport->rtflags &= ~RTKCR_FOPEN_LOG;
	}else{
		sdport->rtflags |=  RTKCR_FOPEN_LOG;
	}

	return sprintf(buf, "%s log %s\n",
			DRIVER_NAME,
			(sdport->rtflags & RTKCR_FOPEN_LOG)?"open":"close");
}

static ssize_t
cr_open_log_dev_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf,
					 size_t count)
{

	CR_INFO("%s(%u)Not thing to do.\n",__func__,__LINE__);

	return count;
}
DEVICE_ATTR(cr_open_log, S_IRUGO | S_IWUSR,
			cr_open_log_dev_show,cr_open_log_dev_store);

static ssize_t
cr_open_int_dev_show(struct device *dev, struct device_attribute *attr,
                char *buf)
{
        struct mmc_host * host = dev_get_drvdata(dev);
        struct rtksd_host *sdport = mmc_priv(host);

        if(sdport->rtflags & RTKCR_FOPEN_INT){
                sdport->rtflags &= ~RTKCR_FOPEN_INT;
        }else{
                sdport->rtflags |=  RTKCR_FOPEN_INT;
        }

        return sprintf(buf, "%s int_mode is %s\n",
                        DRIVER_NAME,
                        (sdport->rtflags & RTKCR_FOPEN_INT)?"open":"close");
}

static ssize_t
cr_open_int_dev_store(struct device *dev,
                                         struct device_attribute *attr,
                                         const char *buf,
                                         size_t count)
{

        CR_INFO("%s(%u)Not thing to do.\n",__func__,__LINE__);

        return count;
}
DEVICE_ATTR(cr_open_int, S_IRUGO | S_IWUSR,
                        cr_open_int_dev_show,cr_open_int_dev_store);

/* mmc attribute &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& */
static
int rtkcr_debug_info(void)
{
//#define CR_BUS_DEBUG
#ifdef CR_BUS_DEBUG
	u32 *des_base;
	u32 i;

	CR_ALERT("\n****** BUS Status Info ******\n");

	CR_ALERT("======DC_SYS FIFO Status======\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x\n",
			(u32)0xb8007064,(u32)cr_readl(0xb8007064),
			(u32)0xb8003064,(u32)cr_readl(0xb8003064));
	CR_ALERT("======IB FIFO Status======\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x\n",
			(u32)0xb803F0F0,(u32)cr_readl(0xb803F0F0),
			(u32)0xb803F1F0,(u32)cr_readl(0xb803F1F0),
			(u32)0xb803F2F0,(u32)cr_readl(0xb803F2F0),
			(u32)0xb803F3F0,(u32)cr_readl(0xb803F3F0),
			(u32)0xb803F4F0,(u32)cr_readl(0xb803F4F0),
			(u32)0xb803F5F0,(u32)cr_readl(0xb803F5F0),
			(u32)0xb803F6F0,(u32)cr_readl(0xb803F6F0),
			(u32)0xb803F7F0,(u32)cr_readl(0xb803F7F0),
			(u32)0xb803F8F0,(u32)cr_readl(0xb803F8F0),
			(u32)0xb803F9F0,(u32)cr_readl(0xb803F9F0),
			(u32)0xb803FAF0,(u32)cr_readl(0xb803FAF0),
			(u32)0xb803FBF0,(u32)cr_readl(0xb803FBF0),
			(u32)0xb803FCF0,(u32)cr_readl(0xb803FCF0),
			(u32)0xb803FCF0,(u32)cr_readl(0xb803FCF0));
	CR_ALERT("\n****** WRAPPER setting ******\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x\n",
		 	(u32)0xb8007008,(u32)cr_readl(0xb8007008),
		 	(u32)0xb80070c8,(u32)cr_readl(0xb80070c8));


	CR_ALERT("\n****** Bandwidth Test ******\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x 0x%08x=0x%08x\n",
		 	(u32)0xb8007114,(u32)cr_readl(0xb8007114),
		 	(u32)0xb8007168,(u32)cr_readl(0xb8007168),
		 	(u32)0xb800716c,(u32)cr_readl(0xb800716c),
		 	(u32)0xb80071b0,(u32)cr_readl(0xb80071b0));


	CR_ALERT("\n****** PRIORITY INFO ******\n"
		 "    0x%08x=0x%08x 0x%08x=0x%08x\n",
		 	(u32)0xb8007088,(u32)cr_readl(0xb8007088),
		 	(u32)0xb8003088,(u32)cr_readl(0xb8003088));

	CR_ALERT("\n&&&&&& BUS Status Info &&&&&&\n");

#endif
	return 0;
}

void rtkcr_sb1_setting(void)
{
	CR_ALERT("\n%s(%d)No thing to do now\n",__func__,__LINE__);

}


static const struct of_device_id of_rtksdio_match[] = {
	{ .compatible = "realtek,rtk-sdio" },
	{}
};

MODULE_DEVICE_TABLE(of, of_rtksdio_match);

static
int __init rtksd_probe(struct platform_device *pdev)
{
	struct mmc_host *mmc = NULL;
	struct rtksd_host *sdport = NULL;
	int ret, irq_err;
	int att_err;
	struct resource *r;
	const struct of_device_id *match;

	att_err = device_create_file(&pdev->dev, &dev_attr_cr_rescan);
	att_err = device_create_file(&pdev->dev, &dev_attr_cr_offline);
	att_err = device_create_file(&pdev->dev, &dev_attr_cr_chkwp);
	att_err = device_create_file(&pdev->dev, &dev_attr_cr_getCID);
	att_err = device_create_file(&pdev->dev, &dev_attr_cr_open_log);
	att_err = device_create_file(&pdev->dev, &dev_attr_cr_open_int);

#ifdef CONFIG_OF
    {
        u32 of_irq = 0;
        struct resource of_reg_res;

        match = of_match_device(of_rtksdio_match, &pdev->dev);
        if (!match) {
		CR_ERR("can't find rtk-sdio in dts\n");
            return -ENXIO;
	    }
	    if (of_address_to_resource(pdev->dev.of_node, 0, &of_reg_res)) {
		    return -ENXIO;
	    }
	    r = request_mem_region(of_reg_res.start, resource_size(&of_reg_res),
	                       pdev->dev.of_node->name);
	    if (!r) {
		    return -ENXIO;
	    }

        of_irq = irq_of_parse_and_map(pdev->dev.of_node, 0);
        irq_err = IRQ_GET_KERNEL_IRQ_NUM(IRQ_CR); // assign irq number
        CR_INFO("%s: IRQ %u\n", pdev->dev.of_node->name, irq_err);
    }
#else

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irq = platform_get_irq(pdev, 0);
	CR_INFO("%s: IRQ %u\n",DRIVER_NAME,irq);

	if (!res || irq < 0){
		return -ENXIO;
	}

	res = request_mem_region(r->start, 0x3FF, DRIVER_NAME);
	if (!res)
		return -EBUSY;

#endif

	mmc = mmc_alloc_host(sizeof(struct rtksd_host), &pdev->dev);

	if (!mmc) {
		return -ENOMEM;
	}

	sdport = mmc_priv(mmc);
	memset(sdport, 0, sizeof(struct rtksd_host));
	sdport->base = of_io_request_and_map(pdev->dev.of_node, 0, pdev->dev.of_node->name);
	if (!sdport->base) {
		ret = -EBUSY;
		goto FREE_HOST;
	}

	ret = devm_request_irq(&pdev->dev, irq_err, rtksd_irq,
			IRQF_SHARED, DRIVER_NAME, sdport);
	if (ret) {
		CR_ERR( "%s: cannot assign irq %d\n", DRIVER_NAME, irq_err);
		goto FREE_HOST;
	}else{
		sdport->irq = irq_err;
	}

	sdport->mmc     = mmc;
	sdport->dev     = &pdev->dev;
	sdport->base_io = CR_BASE_ADDR;
	sdport->ops     = &sdio_ops;
	sdport->gpio_isr_info = rtk_power_bits;
	sdport->int_status_old &= ~RTKCR_INT_CARD_DETEC;
	sdport->rtflags &= ~RTKCR_FOPEN_INT;

	/* Right now device-tree probed devices don't get dma_mask yet.
	 * we check here
	 */
	if (!pdev->dev.dma_mask)
		pdev->dev.dma_mask = &pdev->dev.coherent_dma_mask;
	if (!pdev->dev.coherent_dma_mask)
		pdev->dev.coherent_dma_mask = DMA_BIT_MASK(32);

	mmc->ocr_avail = MMC_VDD_30_31 | MMC_VDD_31_32
		       | MMC_VDD_32_33 | MMC_VDD_33_34;

	mmc->f_min = 100000;
	mmc->f_max = 50000000;

	ret = mmc_of_parse(mmc);
	if (ret) {
		CR_ERR("mmc of parse error\n");
		goto FREE_HOST;
	}

	mmc->max_segs = 1;
	mmc->max_blk_size   = 512;

	mmc->max_blk_count  = 0x200;
	mmc->max_seg_size   = mmc->max_blk_size * mmc->max_blk_count;
	mmc->max_req_size   = mmc->max_blk_size * mmc->max_blk_count;

	spin_lock_init(&sdport->lock);
	tasklet_init(&sdport->polling_tasklet, rtkcr_polling_tasklet,
				(unsigned long)sdport);

	sdport->cr_workqueue = alloc_workqueue("rtkcr_wq",
			WQ_UNBOUND, 1);
	if (!sdport->cr_workqueue) {
		CR_ERR("%s: cannot alloc workqueue!\n", DRIVER_NAME);
		goto FREE_HOST;
	}

	INIT_DELAYED_WORK(&sdport->cr_work, rtkcr_polling_work);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	timer_setup(&sdport->timer, rtksd_timeout_timer, 0);
#else
	setup_timer(&sdport->timer, rtksd_timeout_timer, (unsigned long)sdport);
#endif
	sdport->ops->chk_card_insert(sdport);
	sdport->ops->set_crt_muxpad(sdport);
	sdport->ops->reset_host(sdport,0);

	sdport->scrip_buf = dma_alloc_coherent(sdport->dev,4096,
				    &sdport->scrip_buf_phy, GFP_KERNEL);
	if(!sdport->scrip_buf){
		ret = -ENOMEM;
		goto FREE_HOST;
	}

	platform_set_drvdata(pdev, mmc);

#if 0
	sdport->rtflags |=  RTKCR_FOPEN_LOG;
#endif

	ret = mmc_add_host(mmc);
	if (ret)
		goto FREE_HOST;

	CR_ALERT( "%s: %s driver initialized\n",
			   mmc_hostname(mmc), DRIVER_NAME);

	rtkcr_debug_info();
	rtkcr_sb1_setting();

	return 0;

FREE_HOST:
	if (sdport) {
		if (sdport->irq)
			free_irq(sdport->irq, sdport);

		if (sdport->base)
			iounmap(sdport->base);

		if (sdport->cr_workqueue)
			destroy_workqueue(sdport->cr_workqueue);

		tasklet_kill(&sdport->polling_tasklet);
	}

	if (mmc)
		mmc_free_host(mmc);
	return ret;
}

static
int __exit rtksd_remove(struct platform_device *pdev)
{
	struct mmc_host *mmc = platform_get_drvdata(pdev);
	CR_INFO("%s: Driver romove\n",DRIVER_NAME);

	device_remove_file(&pdev->dev, &dev_attr_cr_rescan);
	device_remove_file(&pdev->dev, &dev_attr_cr_offline);
	device_remove_file(&pdev->dev, &dev_attr_cr_chkwp);
	device_remove_file(&pdev->dev, &dev_attr_cr_getCID);
	device_remove_file(&pdev->dev, &dev_attr_cr_open_log);
	device_remove_file(&pdev->dev, &dev_attr_cr_open_int);
	if (mmc) {
		struct rtksd_host *sdport = mmc_priv(mmc);

		cancel_delayed_work(&sdport->cr_work);
		flush_workqueue(sdport->cr_workqueue);
		destroy_workqueue(sdport->cr_workqueue);
		tasklet_kill(&sdport->polling_tasklet);
		mmc_remove_host(mmc);
		free_irq(sdport->irq, sdport);

		del_timer_sync(&sdport->timer);

		if (sdport->base)
			iounmap(sdport->base);
		//kfree(sdport->cur_host_status);

		mmc_free_host(mmc);
	}
	platform_set_drvdata(pdev, NULL);
	return 0;
}

#ifdef CONFIG_PM
static
int rtksd_suspend(struct platform_device *dev, pm_message_t state)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	CR_INFO( "%s: Prepare to suspend...\n",DRIVER_NAME);

	if (mmc){
		struct rtksd_host *sdport = mmc_priv(mmc);

		cancel_delayed_work(&sdport->cr_work);
		flush_workqueue(sdport->cr_workqueue);

		del_timer_sync(&sdport->timer);
		rtkcr_crt_onoff_host(sdport,CLK_OFF);
	}

	return ret;
}

static
int rtksd_resume(struct platform_device *dev)
{
	struct mmc_host *mmc = platform_get_drvdata(dev);
	int ret = 0;

	CR_INFO( "%s: wake up to resume...\n",DRIVER_NAME);

	if (mmc){
		struct rtksd_host *sdport = mmc_priv(mmc);

		rtkcr_crt_onoff_host(sdport,CLK_ON);

		if(sdport->mmc->card){
			CR_NOTICE("%s: %s(%d)remove card\n",DRIVER_NAME,__func__,__LINE__);
			mmc_card_set_removed(sdport->mmc->card);
		}
	}

	CR_INFO("%s(%u)resume finish~~\n",__func__,__LINE__);
	return ret;
}
#else
#define rtksd_suspend   NULL
#define rtksd_resume    NULL
#endif



static
struct platform_driver rtksd_driver = {
	.driver     = {
		.name   = DRIVER_NAME,
		.owner  = THIS_MODULE,
#ifdef CONFIG_OF
		.of_match_table = of_rtksdio_match,
#endif
	},
	.remove     = __exit_p(rtksd_remove),
	.suspend    = rtksd_suspend,
	.resume     = rtksd_resume,

};

static
void rtkcr_display_version (void)
{
	const __u8 *revision;
	const __u8 *date;
	const __u8 *time;
	char *running = (__u8 *)VERSION;

	strsep(&running, " ");
	strsep(&running, " ");
	revision = strsep(&running, " ");
	date = strsep(&running, " ");
	time = strsep(&running, " ");
	CR_NOTICE(BANNER " Rev:%s (%s %s)\n", revision, date, time);

}

static int rtkcr_chk_pw_pin(char * buf){
	/*
	get card power GPIO address and control bit
	To add power pin infromation at bootcode parameter, like below
	bootargs=console=ttyS0,115200 earlyprintk cr_pw=0x010
	the keyword is "cr_pw"
	the getted parameter is hex.
	example:
		cr_pw=010 <== GPIO16
	*/

	rtkcr_chk_param(&rtk_power_bits,3,buf+3);
	CR_NOTICE("%s:power pin setting from bootcode is GPIO%u(MIS)\n",
				DRIVER_NAME,rtk_power_bits);
	return 0;
}

static int rtkcr_chk_disable(char *buf) {
    rtkcr_drv_dis = true;
    return 0;
}

#define DEFAULT_SD_POWER_PIN (29)
static int rtkcr_get_gpio(void)
{
#if 1
	return 0;
#else
	unsigned int gpio_group;
	unsigned int rtk_gpio_num;
	set_gpio_func_t gpio_func;
	u64 rtk_tmp_gpio;

	CR_NOTICE("%s: check Power Pin.\n",DRIVER_NAME);

	if(rtk_power_bits != 0xff){
		CR_NOTICE("%s: Bootcode parameter setting Card-Reader power pin got.\n",
				DRIVER_NAME);
		rtk_gpio_num = GPIO_GET_NUM(rtk_power_bits);
		if(GPIO_CHK_ISO(rtk_power_bits)){
			gpio_group = ISO_GPIO;
		}else{
			gpio_group = MIS_GPIO;
		}
		goto CHECK_PIN;
	}
#ifndef CONFIG_REALTEK_PCBMGR
	CR_NOTICE("%s: CONFIG_REALTEK_PCBMGR not define\n"
					"",DRIVER_NAME);
#else
	CR_NOTICE("%s: ### CONFIG_REALTEK_PCBMGR alert !!! ###\n",DRIVER_NAME);
	if(pcb_mgr_get_enum_info_byname("PIN_CR_PWR_EN", (u64*)&rtk_tmp_gpio)==0)
	{
		//cat /sys/realtek_boards/pcb_enum/PIN_LVDS_ON_OFF
		if(rtk_tmp_gpio == 0xffffffff){
			CR_NOTICE("%s: PIN_CR_PWR_EN got(0x%x). Use HW-Fixed power pin.\n",
					DRIVER_NAME,(u32)rtk_tmp_gpio);

			rtk_gpio_num = 0xff;
			gpio_group   = MIS_GPIO;
			gpio_func    = rtkcr_set_mis_gpio;
			goto SETTING_OUT;
		}else{
			CR_NOTICE("%s: PIN_CR_PWR_EN got.(0x%x)\n",
					DRIVER_NAME,(u32)rtk_tmp_gpio);

			gpio_group   = GET_PIN_TYPE(rtk_tmp_gpio);
			rtk_gpio_num = GET_PIN_INDEX(rtk_tmp_gpio);
		}
#endif

CHECK_PIN:
		switch(gpio_group){
		case PCB_PIN_TYPE_GPIO:
			CR_NOTICE("%s: PIN type is PCB_PIN_TYPE_GPIO\n",DRIVER_NAME);
			if(rtk_gpio_num > MAX_MIS_GPIO_CNT)
			{
				CR_NOTICE("%s: PIN_CR_PWR_EN is invaild!\n",DRIVER_NAME);
				break;
			}
			gpio_func = rtkcr_set_mis_gpio;
			goto SETTING_PIN;
		case PCB_PIN_TYPE_ISO_GPIO:
			CR_NOTICE("%s: PIN type is PCB_PIN_TYPE_ISO_GPIO\n",DRIVER_NAME);
			if(iso_gpio_mux_tbl[rtk_gpio_num][0] == 0xff ||
			   rtk_gpio_num > MAX_ISO_GPIO_CNT)
			{
				CR_NOTICE("%s: PIN_CR_PWR_EN is invaild!\n",DRIVER_NAME);
				break;
			}
			gpio_func = rtkcr_set_iso_gpio;
			goto SETTING_PIN;
		default:
			CR_NOTICE("PIN group not match\n");
			break;
		}
#ifdef CONFIG_REALTEK_PCBMGR
	}
#endif

	rtk_gpio_num = DEFAULT_SD_POWER_PIN;
	gpio_group   = MIS_GPIO;
	gpio_func    = rtkcr_set_mis_gpio;
	CR_NOTICE("         have not PIN_CR_PWR_EN parameter,\n"
			  "         use DEMO board setting.\n");

SETTING_PIN:

	CR_NOTICE("%s: power pin is GPIO%u(%s).\n",
			DRIVER_NAME,rtk_gpio_num,
			(gpio_group == ISO_GPIO)?"ISO":"MIS");

	gpio_func(rtk_gpio_num,DIR_OUT|GPIO_H);

	if(gpio_group == ISO_GPIO){ //set ISO flag for later
		CR_NOTICE("%s: set ISO flag.\n",DRIVER_NAME);
		rtk_gpio_num |= ISO_FLAG;
	}

SETTING_OUT:
	rtk_power_bits = rtk_gpio_num;

	return 0;
#endif
}

static
int rtkcr_set_bus_width(char * buf){
	/*
	get Card-Reader bus width setting by bootcode parameter, like below
	bootargs=console=ttyS0,115200 earlyprintk cr_bus=8
	the keyword is "cr_bus"
	the getted parameter is hex.
	example:
		cr_bus=8
	*/

	rtkcr_chk_param(&rtk_cr_bus_wid,1,buf+1);
	CR_NOTICE("%s:Bootcode setting bus width is %u-bit\n",
				DRIVER_NAME,rtk_cr_bus_wid);
	return 0;
}

static
int __init rtksd_init(void)
{
	int rc = 0;

    rtkcr_display_version();

	if (rtkcr_drv_dis)
	    goto POWER_ERR;

	rc = rtkcr_get_gpio();

	if (rc < 0){
		CR_NOTICE( "%s: power GPIO detect fail!!\n",
				DRIVER_NAME);
		goto POWER_ERR;

	}

	rc = platform_driver_probe(&rtksd_driver, rtksd_probe);

	if (rc < 0){
POWER_ERR:
		CR_NOTICE( "Realtek Card-Reader Controller Driver installation fails.\n\n");
		return -ENODEV;
	}else{
		CR_NOTICE( "Realtek Card-Reader Controller Driver is successfully installing.\n\n");
		return 0;
	}

}

static
void __exit rtksd_exit(void)
{
	platform_driver_unregister(&rtksd_driver);
}

module_init(rtksd_init);
module_exit(rtksd_exit);

/* maximum card clock frequency (default 50MHz) */
module_param(maxfreq, int, 0);

/* force PIO transfers all the time */
module_param(nodma, int, 0);

MODULE_AUTHOR("Elbereth");
MODULE_DESCRIPTION("Realtek MMC,SD,Host Controller driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:rtksdio");

__setup("no_cr", rtkcr_chk_disable);
__setup("cr_pw",rtkcr_chk_pw_pin);
__setup("cr_bus",rtkcr_set_bus_width);

