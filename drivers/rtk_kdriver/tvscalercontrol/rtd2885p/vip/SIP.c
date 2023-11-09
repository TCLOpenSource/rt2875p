/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2014
  * All rights reserved.
  * ========================================================================*/
  /*================= File Description =======================================*/
  /**
   * @file
   *  This file is for SIP related functions.
   *
   * @author  $Author$
   * @date	  $Date$
   * @version	  $Revision$
   */

#ifdef VIP_SUPPORT_SIP_1_SPI_2
  /*============================ Module dependency	===========================*/

#include <linux/string.h>
#include <linux/kernel.h>
#include <asm/cacheflush.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/vipinclude.h> // for typedef that used in VIP_reg_def.h
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/scalerColor.h>
#include <rbus/dmato3dtable_reg.h>
#include <rbus/dmato3dlut_reg.h>
#include <rbus/sw_def_vip_reg.h>
#include <rtd_log/rtd_module_log.h>
#include "rtk_vip_logger.h"

#include <rbus/sys_reg_reg.h>
#include <rbus/ssrs_reg.h>
#include <rbus/sdemura_reg.h>
#include <rbus/dmatosip_1_reg.h>
#include <rbus/dmatosip_2_reg.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/SIP.h>



void drvif_color_VPQ_SIP_TOP_CLK(void)
{
	sys_reg_sys_clken1_RBUS sys_reg_sys_clken1_reg;
	sys_reg_sys_dispclksel_RBUS sys_reg_sys_dispclksel_reg;
	sys_reg_sys_clken4_RBUS sys_reg_sys_clken4_reg;
	sys_reg_sys_dclk_gate_sel0_RBUS sys_reg_sys_dclk_gate_sel0_reg;
	sys_reg_sys_dclk_gate_sel1_RBUS sys_reg_sys_dclk_gate_sel1_reg;
	ppoverlay_dispd_smooth_toggle2_RBUS ppoverlay_dispd_smooth_toggle2_reg;
	sys_reg_sys_srst4_RBUS sys_reg_sys_srst4_reg;
	ssrs_ssrs_ctrl_RBUS ssrs_ssrs_ctrl_reg;
	ppoverlay_display_timing_ctrl2_RBUS ppoverlay_display_timing_ctrl2_reg;
	sdemura_sdemura_ctrl_RBUS sdemura_sdemura_ctrl_reg;

	// clk_init()
	//rtd_outl(0xB8400114, 0xffffffff);     // clk_init()
	// IoReg_Write32(SYS_REG_SYS_CLKEN1_reg, SYS_REG_SYS_CLKEN1_clken_tvsb4_mask | SYS_REG_SYS_CLKEN1_clken_dispd_mask | SYS_REG_SYS_CLKEN1_write_data_mask);
	//sys_reg_sys_clken1_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN1_reg);
	sys_reg_sys_clken1_reg.regValue = 0;
	//sys_reg_sys_clken1_reg.clken_tvsb4 = 1;
	//sys_reg_sys_clken1_reg.clken_dispd = 1;
	//sys_reg_sys_clken1_reg.write_data = 1;
	//IoReg_Write32(SYS_REG_SYS_CLKEN1_reg, sys_reg_sys_clken1_reg.regValue);

	// rtd_outl(0xB8400208, 0x0000802e);    // clk_init()
	sys_reg_sys_dispclksel_reg.regValue = IoReg_Read32(SYS_REG_SYS_DISPCLKSEL_reg);
	//sys_reg_sys_dispclksel_reg.clken_disp_stage1 = 1;
	//IoReg_Write32(SYS_REG_SYS_DISPCLKSEL_reg, sys_reg_sys_dispclksel_reg.regValue);

	// rtd_outl(0xB8400130, 0x30000001);    // clk_init(), srnn clk need to disable
	// IoReg_Write32(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_d_s1_f2p_ssrs_mask | SYS_REG_SYS_CLKEN4_clken_d_s1_f4p_ssrs_mask | SYS_REG_SYS_CLKEN4_write_data_mask);
	// IoReg_Write32(SYS_REG_SYS_CLKEN4_reg, SYS_REG_SYS_CLKEN4_clken_srnn_mask);
	//sys_reg_sys_clken4_reg.regValue = IoReg_Read32(SYS_REG_SYS_CLKEN4_reg);
	sys_reg_sys_clken4_reg.regValue = 0;
	sys_reg_sys_clken4_reg.clken_d_s2_f2p_sdemura = 1;
	sys_reg_sys_clken4_reg.clken_d_s2_f4p_sdemura = 1;
	sys_reg_sys_clken4_reg.clken_d_s1_f2p_ssrs = 1;
	sys_reg_sys_clken4_reg.clken_d_s1_f4p_ssrs = 1;
	//sys_reg_sys_clken4_reg.clken_akl = 0;
	//sys_reg_sys_clken4_reg.clken_ng = 0;
	//sys_reg_sys_clken4_reg.clken_memc_mc_func = 0;
	//sys_reg_sys_clken4_reg.clken_gdcmp = 0;
	sys_reg_sys_clken4_reg.write_data = 1;
	IoReg_Write32(SYS_REG_SYS_CLKEN4_reg, sys_reg_sys_clken4_reg.regValue);
	// clk_init(), srnn clk need to disable
	sys_reg_sys_clken4_reg.regValue = 0;
	sys_reg_sys_clken4_reg.clken_srnn = 1;
	sys_reg_sys_clken4_reg.write_data = 0;
	IoReg_Write32(SYS_REG_SYS_CLKEN4_reg, sys_reg_sys_clken4_reg.regValue);

	// rtd_outl(0xB8400230, 0x22101010);    // clk_init()   //ssrs test case: 22101010, ptg script: 211010
	sys_reg_sys_dclk_gate_sel0_reg.regValue = IoReg_Read32(SYS_REG_SYS_DCLK_GATE_SEL0_reg);
	sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f4p_gate_sel = 2;
	sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f4p_gate_sel = 2;
	//sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f2p_gate_sel = 1;
	//sys_reg_sys_dclk_gate_sel0_reg.dclk_osd_f1p_gate_sel = 0;
	//sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f2p_gate_sel = 1;
	//sys_reg_sys_dclk_gate_sel0_reg.dclk_s2_f1p_gate_sel = 0;
	//sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f2p_gate_sel = 1;
	//sys_reg_sys_dclk_gate_sel0_reg.dclk_s1_f1p_gate_sel = 0;
	IoReg_Write32(SYS_REG_SYS_DCLK_GATE_SEL0_reg, sys_reg_sys_dclk_gate_sel0_reg.regValue);

	//	rtd_outl(0xB8400234, 0x00000011);      // clk_init()
	sys_reg_sys_dclk_gate_sel1_reg.regValue = IoReg_Read32(SYS_REG_SYS_DCLK_GATE_SEL1_reg);
	//sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_in_if_gate_sel = 1;
	//sys_reg_sys_dclk_gate_sel1_reg.dclk_memc_out_if_gate_sel = 1;
	//IoReg_Write32(SYS_REG_SYS_DCLK_GATE_SEL1_reg, sys_reg_sys_dclk_gate_sel1_reg.regValue);

	// rtd_outl(0xB842811C, 0x00000005);      // clk_init()    // set "Dispd_stage1_smooth_toggle_apply=1"
	ppoverlay_dispd_smooth_toggle2_reg.regValue = IoReg_Read32(PPOVERLAY_dispd_smooth_toggle1_reg);
	ppoverlay_dispd_smooth_toggle2_reg.dispd_stage2_smooth_toggle_apply = 1;
	IoReg_Write32(PPOVERLAY_dispd_smooth_toggle1_reg, ppoverlay_dispd_smooth_toggle2_reg.regValue);

	// set_ssrs_rst(1, 1, 1);
	//rtd_outl(0xB8400120, 0x30000001);    // set_ssrs_rst(1, 1, 1);
	// IoReg_Write32(SYS_REG_SYS_SRST4_reg, SYS_REG_SYS_SRST4_rstn_ssrs_0_mask | SYS_REG_SYS_SRST4_rstn_ssrs_1_mask | SYS_REG_SYS_SRST4_write_data_mask);
	//sys_reg_sys_srst4_reg.regValue = IoReg_Read32(SYS_REG_SYS_SRST4_reg);
	sys_reg_sys_srst4_reg.regValue = 0;
	sys_reg_sys_srst4_reg.rstn_ssrs_0 = 1;
	sys_reg_sys_srst4_reg.rstn_ssrs_1 = 1;
	sys_reg_sys_srst4_reg.rstn_sdemura_0 = 1;
	sys_reg_sys_srst4_reg.rstn_sdemura_1 = 1;
	sys_reg_sys_srst4_reg.write_data = 1;
	IoReg_Write32(SYS_REG_SYS_SRST4_reg, sys_reg_sys_srst4_reg.regValue);

	// rtd_outl(0xB85C0000, 0x000f8f00);  // set_ssrs_ssrs_disable(0)
	ssrs_ssrs_ctrl_reg.regValue = IoReg_Read32(SSRS_SSRS_CTRL_reg);
	ssrs_ssrs_ctrl_reg.ssrs_disable = 0;
	IoReg_Write32(SSRS_SSRS_CTRL_reg, ssrs_ssrs_ctrl_reg.regValue);

	sdemura_sdemura_ctrl_reg.regValue = IoReg_Read32(SDEMURA_SDEMURA_CTRL_reg);
	sdemura_sdemura_ctrl_reg.sdemura_disable = 0;
	IoReg_Write32(SDEMURA_SDEMURA_CTRL_reg, sdemura_sdemura_ctrl_reg.regValue);

	//  rtd_outl(0xB8428004, 0x10060408);    // set_pip_swap_en(1), set_dtg_ssrs_en(1)
	ppoverlay_display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	ppoverlay_display_timing_ctrl2_reg.pip_swap_en = 1;	// SSRS only turn on in 2nd pip condition, in spec
	ppoverlay_display_timing_ctrl2_reg.ssrs_en = 1;
	ppoverlay_display_timing_ctrl2_reg.sdemura_en = 1;
	IoReg_Write32(PPOVERLAY_Display_Timing_CTRL2_reg, ppoverlay_display_timing_ctrl2_reg.regValue);

	// rtd_outl(0xB842AC00 , 0x00000000);      //  d_initial_reg(), disable y2r? we can do it in fw, if needed
	// rtd_outl(0xb842C5F8 , 0x00000003);      //  dither_crc_enable (1), DITHER_Dither_CRC_CTRL_reg
	rtd_pr_vpq_info("drvif_color_VPQ_init_SSRS done\n");
}

void drvif_color_VPQ_SIP_1_init(unsigned short h_sta, unsigned short v_sta, unsigned short h_size, unsigned short v_size, unsigned int addr, unsigned int tbl_len)
{
	unsigned int size, sta;
	dmatosip_1_dmatosip_1_db_ctl_RBUS dmatosip_1_dmatosip_1_db_ctl_reg;
	dmatosip_1_dmatosip_1_table0_format0_RBUS dmatosip_1_dmatosip_1_table0_format0_reg;

	// set DMA DB = 1
	dmatosip_1_dmatosip_1_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg);
	dmatosip_1_dmatosip_1_db_ctl_reg.db_en = 1;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg, dmatosip_1_dmatosip_1_db_ctl_reg.regValue);

	// set DMA 
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_Table0_Addr_reg, addr);

	// set tablex_dma_en to 0
	dmatosip_1_dmatosip_1_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg);
	dmatosip_1_dmatosip_1_table0_format0_reg.table0_dma_en = 0;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg, dmatosip_1_dmatosip_1_table0_format0_reg.regValue);

	dmatosip_1_dmatosip_1_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg);
	dmatosip_1_dmatosip_1_db_ctl_reg.wtable_apply = 1;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg, dmatosip_1_dmatosip_1_db_ctl_reg.regValue);
	
	// set SIP inside
	size = (h_size<<16) + (v_size);
	sta =  (h_sta<<16) + (v_sta);
	// reg set come from ssrs, no spec
	IoReg_Write32(SW_DEF_SIP1_0200_reg, 0x00000307);
	IoReg_Write32(SW_DEF_SIP1_0800_reg, 0x00000307);
	//IoReg_Write32(SW_DEF_SIP1_2000_reg, 0x00c00014);
	IoReg_Write32(SW_DEF_SIP1_2000_reg,sta);
	IoReg_Write32(SW_DEF_SIP1_2004_reg,size);
	IoReg_Write32(SW_DEF_SIP1_4000_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4004_reg, 0x00600210);
	IoReg_Write32(SW_DEF_SIP1_400c_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4010_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4014_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4100_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4110_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4114_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_4118_reg, 0x00010380);
	IoReg_Write32(SW_DEF_SIP1_411c_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4120_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_4124_reg, 0x00020239);
	IoReg_Write32(SW_DEF_SIP1_4240_reg, 0x00000011);
	IoReg_Write32(SW_DEF_SIP1_4400_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4404_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4410_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4414_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4418_reg, 0x76543210);
	IoReg_Write32(SW_DEF_SIP1_441c_reg, 0x08010020);
	IoReg_Write32(SW_DEF_SIP1_4420_reg, 0x00201008);
	IoReg_Write32(SW_DEF_SIP1_4424_reg, 0x08010020);
	IoReg_Write32(SW_DEF_SIP1_4428_reg, 0x00201008);
	IoReg_Write32(SW_DEF_SIP1_4430_reg, 0x00200010);
	IoReg_Write32(SW_DEF_SIP1_4434_reg, 0x02000100);
	IoReg_Write32(SW_DEF_SIP1_4438_reg, 0x00000300);
	IoReg_Write32(SW_DEF_SIP1_443c_reg, 0x22000400);
	IoReg_Write32(SW_DEF_SIP1_4440_reg, 0x00c00040);
	IoReg_Write32(SW_DEF_SIP1_4444_reg, 0x01000000);
	IoReg_Write32(SW_DEF_SIP1_4448_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_444c_reg, 0x12000015);
	IoReg_Write32(SW_DEF_SIP1_4450_reg, 0x00000080);
	IoReg_Write32(SW_DEF_SIP1_4454_reg, 0x02000000);
	IoReg_Write32(SW_DEF_SIP1_4458_reg, 0x00030505);
	IoReg_Write32(SW_DEF_SIP1_4460_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_4490_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4494_reg, 0x00000011);
	IoReg_Write32(SW_DEF_SIP1_44a0_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_44a4_reg, 0x02000200);
	IoReg_Write32(SW_DEF_SIP1_44a8_reg, 0x00030000);
	IoReg_Write32(SW_DEF_SIP1_44b0_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_44b4_reg, 0x01c30200);
	IoReg_Write32(SW_DEF_SIP1_44b8_reg, 0x0002073b);
	IoReg_Write32(SW_DEF_SIP1_44c0_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_44c4_reg, 0x01710200);
	IoReg_Write32(SW_DEF_SIP1_44c8_reg, 0x00020635);
	IoReg_Write32(SW_DEF_SIP1_44d0_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_44d4_reg, 0x011f0200);
	IoReg_Write32(SW_DEF_SIP1_44d8_reg, 0x0002052f);
	IoReg_Write32(SW_DEF_SIP1_44e0_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_44e4_reg, 0x02000200);
	IoReg_Write32(SW_DEF_SIP1_44e8_reg, 0x0003052f);
	IoReg_Write32(SW_DEF_SIP1_44f0_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_44f4_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_44f8_reg, 0x00010666);
	IoReg_Write32(SW_DEF_SIP1_4500_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_4504_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_4508_reg, 0x00010666);
	IoReg_Write32(SW_DEF_SIP1_4510_reg, 0x00000050);
	IoReg_Write32(SW_DEF_SIP1_4514_reg, 0x00000200);
	IoReg_Write32(SW_DEF_SIP1_4518_reg, 0x00010666);
	IoReg_Write32(SW_DEF_SIP1_4520_reg, 0x02000000);
	IoReg_Write32(SW_DEF_SIP1_4524_reg, 0x00000100);
	IoReg_Write32(SW_DEF_SIP1_4528_reg, 0x02000000);
	IoReg_Write32(SW_DEF_SIP1_452c_reg, 0x00020140);
	IoReg_Write32(SW_DEF_SIP1_4600_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4604_reg, 0x7da8d4ff);
	IoReg_Write32(SW_DEF_SIP1_4608_reg, 0x2d333952);
	IoReg_Write32(SW_DEF_SIP1_460c_reg, 0x00000028);
	IoReg_Write32(SW_DEF_SIP1_4614_reg, 0x1e28323c);
	IoReg_Write32(SW_DEF_SIP1_4618_reg, 0x00000a14);
	IoReg_Write32(SW_DEF_SIP1_461c_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4620_reg, 0x2dff0000);
	IoReg_Write32(SW_DEF_SIP1_4624_reg, 0x000304b8);
	IoReg_Write32(SW_DEF_SIP1_4630_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4634_reg, 0xffff0000);
	IoReg_Write32(SW_DEF_SIP1_4638_reg, 0x00030000);
	IoReg_Write32(SW_DEF_SIP1_4640_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4644_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4648_reg, 0x47350000);
	IoReg_Write32(SW_DEF_SIP1_464c_reg, 0x000300c0);
	IoReg_Write32(SW_DEF_SIP1_4650_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4654_reg, 0x30600000);
	IoReg_Write32(SW_DEF_SIP1_4658_reg, 0x000302e4);
	IoReg_Write32(SW_DEF_SIP1_4660_reg, 0x60402000);
	IoReg_Write32(SW_DEF_SIP1_4664_reg, 0xe0c0a080);
	IoReg_Write32(SW_DEF_SIP1_4668_reg, 0x000000ff);
	IoReg_Write32(SW_DEF_SIP1_4710_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4714_reg, 0x00500000);
	IoReg_Write32(SW_DEF_SIP1_4718_reg, 0x00400080);
	IoReg_Write32(SW_DEF_SIP1_471c_reg, 0x00000040);
	IoReg_Write32(SW_DEF_SIP1_4720_reg, 0x30000333);
	IoReg_Write32(SW_DEF_SIP1_4730_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4734_reg, 0x000a0000);
	IoReg_Write32(SW_DEF_SIP1_4738_reg, 0x230f0a03);
	IoReg_Write32(SW_DEF_SIP1_473c_reg, 0x000a0000);
	IoReg_Write32(SW_DEF_SIP1_4740_reg, 0x32231405);
	IoReg_Write32(SW_DEF_SIP1_4744_reg, 0x00400020);
	IoReg_Write32(SW_DEF_SIP1_4750_reg, 0x00000000);
	IoReg_Write32(SW_DEF_SIP1_4754_reg, 0x80801008);
	IoReg_Write32(SW_DEF_SIP1_4758_reg, 0x10808080);
	IoReg_Write32(SW_DEF_SIP1_475c_reg, 0x00000008);
	IoReg_Write32(SW_DEF_SIP1_4760_reg, 0x40ffffff);
	IoReg_Write32(SW_DEF_SIP1_4764_reg, 0x8080ffff);
	IoReg_Write32(SW_DEF_SIP1_4774_reg, 0x00000080);
	IoReg_Write32(SW_DEF_SIP1_4780_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_4784_reg, 0x10100000);
	IoReg_Write32(SW_DEF_SIP1_4788_reg, 0x01ff01ff);
	IoReg_Write32(SW_DEF_SIP1_6020_reg, 0x00000001);
	IoReg_Write32(SW_DEF_SIP1_6024_reg, 0x00000010);
	IoReg_Write32(SW_DEF_SIP1_6028_reg, 0x00400080);
	IoReg_Write32(SW_DEF_SIP1_602c_reg, 0xff1c2020);
	IoReg_Write32(SW_DEF_SIP1_6030_reg, 0xff1c2020);
	IoReg_Write32(SW_DEF_SIP1_6100_reg, 0x01000001);

}

void drvif_color_VPQ_SIP_2_init(unsigned short h_sta, unsigned short v_sta, unsigned short h_size, unsigned short v_size, unsigned int addr, unsigned int tbl_len)
{
	dmatosip_2_dmatosip_2_db_ctl_RBUS dmatosip_2_dmatosip_2_db_ctl_reg;
	dmatosip_2_dmatosip_2_table0_format0_RBUS dmatosip_2_dmatosip_2_table0_format0_reg;

	// set DMA DB = 1
	dmatosip_2_dmatosip_2_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg);
	dmatosip_2_dmatosip_2_db_ctl_reg.db_en = 1;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg, dmatosip_2_dmatosip_2_db_ctl_reg.regValue);

	// set DMA 
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_Table0_Addr_reg, addr);

	// set tablex_dma_en to 0
	dmatosip_2_dmatosip_2_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg);
	dmatosip_2_dmatosip_2_table0_format0_reg.table0_dma_en = 0;
	dmatosip_2_dmatosip_2_table0_format0_reg.table0_num_x = 0x3780;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg, dmatosip_2_dmatosip_2_table0_format0_reg.regValue);
	
	dmatosip_2_dmatosip_2_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg);
	dmatosip_2_dmatosip_2_db_ctl_reg.wtable_apply = 1;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg, dmatosip_2_dmatosip_2_db_ctl_reg.regValue);

}

char drvif_color_Set_SIP_REG(unsigned int  *pTBL, unsigned int sta_addr, unsigned int size)
{
	//#define VIP_STA_ADDR 0xb81xxxxx

	unsigned int i;

	if (pTBL == NULL) {
		rtd_pr_vpq_emerg("drvif_color_set_SIP, pTBL=NULL\n");
		return -1;
	}

	for (i=0;i<size;i++) {
		IoReg_Write32(sta_addr+(i*4), *(pTBL+i));
	}
	return 0;
}

char drvif_color_Get_SIP_REG(unsigned int  *pTBL, unsigned int sta_addr, unsigned int size)
{
	//#define VIP_STA_ADDR 0xb81xxxxx

	unsigned int i;

	if (pTBL == NULL) {
		rtd_pr_vpq_emerg("drvif_color_Get_SIP, pTBL=NULL\n");
		return -1;
	}

	for (i=0;i<size;i++) {
		pTBL[i] = IoReg_Read32(sta_addr+(i*4));
	}
	return 0;
}

char drvif_color_reset_SIP1_DMA(void)
{
	// Clear tableX_wdone first
	IoReg_SetBits(DMATOSIP_1_DMAtoSIP_1_Table0_Status_reg, DMATOSIP_1_DMAtoSIP_1_Table0_Status_table0_wdone_mask);
	// clr DMA error status
	IoReg_Mask32(DMATOSIP_1_SIP_1_CTI_DMA_RD_status_d_domain_reg, 0xffffffff,
		DMATOSIP_1_SIP_1_CTI_DMA_RD_status_d_domain_soft_rst_with_data_left_mask|DMATOSIP_1_SIP_1_CTI_DMA_RD_status_d_domain_soft_rst_before_cmd_finish_mask);
	IoReg_SetBits(DMATOSIP_1_SIP_1_CTI_DMA_RD_status1_d_domain_reg, DMATOSIP_1_SIP_1_CTI_DMA_RD_status1_d_domain_ack_num_error_mask);
	// clr table error status which table used
	/*IoReg_SetBits(DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_reg, DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_table0_err_clr_mask);*/
	/*IoReg_Mask32(DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_reg, 0xffffffff, DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_table0_err_status_mask);*/
	IoReg_Mask32(DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_reg, 0xffffffff,
		DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_table0_err_status_mask|DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_table0_err_clr_mask);
	return 0;
}

char drvif_color_set_SIP1_DMA(unsigned char LUT_Ctrl, unsigned int *pArray, unsigned int addr, unsigned int* vir_addr, unsigned int flush_size)
{
	ssrs_ssrs_ctrl_RBUS ssrs_ssrs_ctrl_reg;
	dmatosip_1_dmatosip_1_table0_format0_RBUS dmatosip_1_dmatosip_1_table0_format0_reg;
	dmatosip_1_dmatosip_1_db_ctl_RBUS dmatosip_1_dmatosip_1_db_ctl_reg;
	unsigned char *vir_addr_8=NULL;

	// set DMA DB = 1
	dmatosip_1_dmatosip_1_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg);
	dmatosip_1_dmatosip_1_db_ctl_reg.db_en = 1;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg, dmatosip_1_dmatosip_1_db_ctl_reg.regValue);

	// set DMAtoSIP_1 enable which IP need DMA to write table in IP spec
	// coef TBL addr = 0xb8190000~0xb81a7ffc, total is 786432 bit
	dmatosip_1_dmatosip_1_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg);
	dmatosip_1_dmatosip_1_table0_format0_reg.table0_dma_en = 1;
	//dmatosip_1_dmatosip_1_table0_format0_reg.table0_bitwidth = 20;
	//dmatosip_1_dmatosip_1_table0_format0_reg.table0_num_x = 0x6000;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg, dmatosip_1_dmatosip_1_table0_format0_reg.regValue);
	// set arb
	ssrs_ssrs_ctrl_reg.regValue = IoReg_Read32(SSRS_SSRS_CTRL_reg);
	ssrs_ssrs_ctrl_reg.arb_en_0 = 1;
	ssrs_ssrs_ctrl_reg.arb_en_1 = 0;
	IoReg_Write32(SSRS_SSRS_CTRL_reg, ssrs_ssrs_ctrl_reg.regValue);

	// set tablex_dma_en to 0
	dmatosip_1_dmatosip_1_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg);
	dmatosip_1_dmatosip_1_table0_format0_reg.table0_dma_en = 0;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg, dmatosip_1_dmatosip_1_table0_format0_reg.regValue);

	// set all tablex_xxx: 0x181C0104 - 0x181C015c, need check tbl size change or not
	// ms start
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_Table0_Addr_reg, addr);

	// table set
	memcpy(vir_addr, pArray, sizeof(char)*VIP_SIP_1_Size_default);	// need to set table size if table len is not VIP_SIP_1_Size_default
	if (flush_size != 0) {
		vir_addr_8 = (unsigned char*)vir_addr;
		dmac_flush_range((void*)vir_addr, (void*)(vir_addr_8 + flush_size));
		outer_flush_range(addr, addr + flush_size);
	}

	// Clear tableX_wdone first
	IoReg_SetBits(DMATOSIP_1_DMAtoSIP_1_Table0_Status_reg, DMATOSIP_1_DMAtoSIP_1_Table0_Status_table0_wdone_mask);

	// dma en =1
	dmatosip_1_dmatosip_1_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg);
	dmatosip_1_dmatosip_1_table0_format0_reg.table0_dma_en = 1;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_Table0_Format0_reg, dmatosip_1_dmatosip_1_table0_format0_reg.regValue);

	// set wtable_apply = 1;
	dmatosip_1_dmatosip_1_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg);
	dmatosip_1_dmatosip_1_db_ctl_reg.wtable_apply = 1;
	IoReg_Write32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg, dmatosip_1_dmatosip_1_db_ctl_reg.regValue);
	return 0;

}

char drvif_color_SIP1_ERR_Check_DMA(unsigned char showMSG)
{
	char ret_val = 0;
	short Err = 0;
	Err = DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_get_table0_err_status(IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_reg));

	if (Err  != 0) {
		ret_val = ret_val | _BIT0;
		if (showMSG == 1)
			rtd_pr_vpq_emerg("SIP_DMA, SIP1_DMA, DMA_Err_Check, SIP1 TBL0=%x\n", Err);
	}
	return ret_val;
}

char drvif_color_SIP1_apply_done_Check_DMA(unsigned char showMSG)
{
	char ret_val = 0;
	short Err = 0;
	Err = DMATOSIP_1_DMAtoSIP_1_db_ctl_get_wtable_apply(IoReg_Read32(DMATOSIP_1_DMAtoSIP_1_db_ctl_reg));

	if (Err  != 0) {
		ret_val = ret_val | _BIT0;
		if (showMSG == 1)
			rtd_pr_vpq_emerg("SIP_DMA, SIP1_DMA, DMA_apply_done_Check, SIP1 TBL0=%x\n", Err);
	}
	return ret_val;

}

char drvif_color_reset_SIP2_DMA(void)
{
	// Clear tableX_wdone first
	IoReg_SetBits(DMATOSIP_2_DMAtoSIP_2_Table0_Status_reg, DMATOSIP_2_DMAtoSIP_2_Table0_Status_table0_wdone_mask);
	// clr DMA error status
	IoReg_Mask32(DMATOSIP_2_SIP_2_CTI_DMA_RD_status_d_domain_reg, 0xffffffff,
		DMATOSIP_2_SIP_2_CTI_DMA_RD_status_d_domain_soft_rst_with_data_left_mask|DMATOSIP_2_SIP_2_CTI_DMA_RD_status_d_domain_soft_rst_before_cmd_finish_mask);
	IoReg_SetBits(DMATOSIP_2_SIP_2_CTI_DMA_RD_status1_d_domain_reg, DMATOSIP_2_SIP_2_CTI_DMA_RD_status1_d_domain_ack_num_error_mask);
	// clr table error status which table used
	/*IoReg_SetBits(DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_reg, DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_table0_err_clr_mask);*/
	/*IoReg_Mask32(DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_reg, 0xffffffff, DMATOSIP_1_DMAtoSIP_1_Table0_ErrStatus2Irq_table0_err_status_mask);*/
	IoReg_Mask32(DMATOSIP_2_DMAtoSIP_2_Table0_ErrStatus2Irq_reg, 0xffffffff,
		DMATOSIP_2_DMAtoSIP_2_Table0_ErrStatus2Irq_table0_err_status_mask|DMATOSIP_2_DMAtoSIP_2_Table0_ErrStatus2Irq_table0_err_clr_mask);
	return 0;
}

char drvif_color_set_SIP2_DMA(unsigned char LUT_Ctrl, unsigned int *pArray, unsigned int addr, unsigned int* vir_addr, unsigned int flush_size)
{
	sdemura_sdemura_ctrl_RBUS sdemura_sdemura_ctrl_reg;
	dmatosip_2_dmatosip_2_table0_format0_RBUS dmatosip_2_dmatosip_2_table0_format0_reg;
	dmatosip_2_dmatosip_2_db_ctl_RBUS dmatosip_2_dmatosip_2_db_ctl_reg;
	unsigned char *vir_addr_8=NULL;

	// set DMA DB = 1
	dmatosip_2_dmatosip_2_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg);
	dmatosip_2_dmatosip_2_db_ctl_reg.db_en = 1;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg, dmatosip_2_dmatosip_2_db_ctl_reg.regValue);

	// set DMAtoSIP_1 enable which IP need DMA to write table in IP spec
	// coef TBL addr = 0xb8190000~0xb81a7ffc, total is 786432 bit
	dmatosip_2_dmatosip_2_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg);
	dmatosip_2_dmatosip_2_table0_format0_reg.table0_dma_en = 1;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg, dmatosip_2_dmatosip_2_table0_format0_reg.regValue);
	// set arb
	sdemura_sdemura_ctrl_reg.regValue = IoReg_Read32(SDEMURA_SDEMURA_CTRL_reg);
	sdemura_sdemura_ctrl_reg.arb_en_0 = 1;
	sdemura_sdemura_ctrl_reg.arb_en_1 = 0;
	IoReg_Write32(SDEMURA_SDEMURA_CTRL_reg, sdemura_sdemura_ctrl_reg.regValue);

	// set tablex_dma_en to 0
	dmatosip_2_dmatosip_2_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg);
	dmatosip_2_dmatosip_2_table0_format0_reg.table0_dma_en = 0;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg, dmatosip_2_dmatosip_2_table0_format0_reg.regValue);

	// set all tablex_xxx: 0x181C0104 - 0x181C015c, need check tbl size change or not
	// ms start
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_Table0_Addr_reg, addr);

	// table set
	memcpy(vir_addr, pArray, sizeof(char)*VIP_SIP_2_Size_default);	// need to set table size if table len is not VIP_SIP_1_Size_default
	if (flush_size != 0) {
		vir_addr_8 = (unsigned char*)vir_addr;
		dmac_flush_range((void*)vir_addr, (void*)(vir_addr_8 + flush_size));
		outer_flush_range(addr, addr + flush_size);
	}

	// Clear tableX_wdone first
	IoReg_SetBits(DMATOSIP_2_DMAtoSIP_2_Table0_Status_reg, DMATOSIP_2_DMAtoSIP_2_Table0_Status_table0_wdone_mask);

	// dma en =1
	dmatosip_2_dmatosip_2_table0_format0_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg);
	dmatosip_2_dmatosip_2_table0_format0_reg.table0_dma_en = 1;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_Table0_Format0_reg, dmatosip_2_dmatosip_2_table0_format0_reg.regValue);

	// set wtable_apply = 1;
	dmatosip_2_dmatosip_2_db_ctl_reg.regValue = IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg);
	dmatosip_2_dmatosip_2_db_ctl_reg.wtable_apply = 1;
	IoReg_Write32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg, dmatosip_2_dmatosip_2_db_ctl_reg.regValue);
	return 0;

}

char drvif_color_SIP2_ERR_Check_DMA(unsigned char showMSG)
{
	char ret_val = 0;
	short Err = 0;
	Err = DMATOSIP_2_DMAtoSIP_2_Table0_ErrStatus2Irq_get_table0_err_status(IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_Table0_ErrStatus2Irq_reg));

	if (Err  != 0) {
		ret_val = ret_val | _BIT0;
		if (showMSG == 1)
			rtd_pr_vpq_emerg("SIP_DMA, SIP2_DMA, DMA_Err_Check, SIP2 TBL0=%x\n", Err);
	}
	return ret_val;
}

char drvif_color_SIP2_apply_done_Check_DMA(unsigned char showMSG)
{
	char ret_val = 0;
	short Err = 0;
	Err = DMATOSIP_2_DMAtoSIP_2_db_ctl_get_wtable_apply(IoReg_Read32(DMATOSIP_2_DMAtoSIP_2_db_ctl_reg));

	if (Err  != 0) {
		ret_val = ret_val | _BIT0;
		if (showMSG == 1)
			rtd_pr_vpq_emerg("SIP_DMA, SIP2_DMA, DMA_apply_done_Check, SIP2 TBL0=%x\n", Err);
	}
	return ret_val;

}
#endif
