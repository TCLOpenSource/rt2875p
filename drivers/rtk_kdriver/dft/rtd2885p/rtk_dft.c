/*
 *  linux/arch/arm/mach-rtd299s/rtk_crt.c
 *
 *  Copyright (C) 2015 Realtek
 *  All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/delay.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>

#include <rbus/sb2_reg.h>
#include <rbus/sys_reg_reg.h>
#include <mach/system.h>
#include <rtk_kdriver/io.h>

#include <linux/pageremap.h>
#include <mach/rtk_platform.h>
#include <rbus/md_reg.h>
#include <rbus/wdog_reg.h>
#include <rbus/dc_mc_reg.h>
#include <rbus/dc_mc2_reg.h>
#include <rbus/dc_sys_reg.h>
#include <rbus/mc_ptg_reg.h>

#include <linux/kthread.h> //kthread_create(), kthread_run()

/* rtk_dft char device variable */
#define DFT_MAJOR 0   /* dynamic major by default */
#define DFT_NR_DEVS 1

#define RTKDFT_IOC_MAGIC  'D'
#define RTKDFT_IOC_PHASE_SCAN    _IO(RTKDFT_IOC_MAGIC, 1)
#define RTKDFT_IOC_GET_PHASE_SCAN_INFO    _IO(RTKDFT_IOC_MAGIC, 2)
#define RTKDFT_IOC_AUTO_PHASE_SCAN    _IO(RTKDFT_IOC_MAGIC, 3)

#define TX_VREF           8
#define TX_DQSPI          2
#define RX_VREF_DDR4_TAP  5
#define RX_VREF_DDR3_TAP  4
#define RX_TE_TAP         3
#define BYTE_MAX          8
#define RX_VREF           4
#define MD_LENGTH         0x480000

#define HW_SETTING_POK_KEY_ADDR        0x00016c00
#define HW_SETTING_POK_TX_ADDR         (HW_SETTING_POK_KEY_ADDR+0x10)  //0x00016c10
#define HW_SETTING_POK_RX_ADDR         (HW_SETTING_POK_KEY_ADDR+0x20)  //0x00016c20
#define HW_SETTING_POK_CHECKSUM_ADDR   (HW_SETTING_POK_KEY_ADDR+0x120) //0x00016d20
#define HW_SETTING_POK_TX_TAG          0xA1A25566
#define HW_SETTING_POK_RX_TAG          0xA1A43344

int dft_major 		= DFT_MAJOR;
int dft_minor 		= 0;
int dft_nr_devs 	= DFT_NR_DEVS;
dev_t dft_devnum;
static struct class *dft_class;
static struct kobject *dft_kobj;
struct cdev dft_cdev;
spinlock_t dft_lock;

static struct device *dft_device;
static struct platform_device *dft_platform_devs;
static unsigned char OutData[128];

static signed char tx_pre_dqspi_set = 0;
static char byte_num = BYTE_MAX;
struct task_struct *dft_kthread;
static int g_dft_kthreadRunning = false;
static int g_dftphasescan_ret=-1;
#if IS_ENABLED(CONFIG_RTK_KDRV_MD)
extern bool smd_waitComplete(uint64_t handle, unsigned long timeout, uint64_t *hwCnt) ;
#endif

static bool is_DDR4(void)
{
	dc_mc_mc_sys_0_RBUS dc_mc_mc_sys_0;
	dc_mc2_mc_sys_0_RBUS dc_mc2_mc_sys_0;

	dc_mc_mc_sys_0.regValue = rtd_inl(DC_MC_MC_SYS_0_reg);
	dc_mc2_mc_sys_0.regValue = rtd_inl(DC_MC2_MC_SYS_0_reg);

	if ((dc_mc_mc_sys_0.ddr4_en == 1) || (dc_mc2_mc_sys_0.ddr4_en == 1))
		return 1;
	else
		return 0;
}

// return  0: OK,  -1: fail
static char MD_rw(const unsigned int addr, const unsigned int length)
{
	unsigned int get_wr_crc, get_rd_crc;
	unsigned int timeoutcnt;
	unsigned int start_addr, dst_addr;
	unsigned char retry = 5;
#if IS_ENABLED(CONFIG_RTK_KDRV_MD)
	static const unsigned long timeout = 100;

	uint64_t unused;
	while (smd_waitComplete(0, timeout, &unused) == false) {
		rtd_pr_dft_info("MD_rw: smd_waitComplete more than %lu ms\n", timeout);
	}
#endif

	do {
		start_addr = addr;
		dst_addr = start_addr + length;
		rtd_outl(MD_MD_W_CRC_CTRL_reg, 0x00000003);
		rtd_outl(MD_MD_REG_MODE_reg, 0x00000000);
		rtd_outl(MD_MD_W_CRC_MASK1_reg, 0xffffffff);
		rtd_outl(MD_MD_W_CRC_MASK2_reg, 0xffffffff);
		rtd_outl(MD_MD_REG_CTRL_reg, 0x00000002);
		rtd_outl(MD_MD_REG_CMD1_reg, 0x00000045);
		rtd_outl(MD_MD_CONST_ASCENDING_VALUE_reg, 0x00000004);
		rtd_outl(MD_MD_REG_CMD2_reg, start_addr);
		rtd_outl(MD_MD_REG_CMD4_reg, length);
		rtd_outl(MD_MD_REG_CMD5_reg, 0xA5A55A5A);
		rtd_outl(MD_MD_REG_MODE_reg, 0x00000001);

		//check done bit
		timeoutcnt = 0xfffff;
		do {
			mdelay(1);
			if(rtd_inl(MD_MD_REG_MODE_reg) == 0)
				break;
		} while(timeoutcnt--);

		//write CRC result
		get_wr_crc = rtd_inl(MD_MD_W_CRC_RESULT_reg);

		// MD read
		rtd_outl(MD_MD_DES_CRC_reg, get_wr_crc);
		rtd_outl(MD_MD_R_CRC_CTRL_reg, 0x00000007);
		rtd_outl(MD_MD_R_CRC_MASK1_reg, 0xffffffff);
		rtd_outl(MD_MD_R_CRC_MASK2_reg, 0xffffffff);
		rtd_outl(MD_MD_REG_MODE_reg, 0x00000000);
		rtd_outl(MD_MD_REG_CTRL_reg, 0x00000002);
		rtd_outl(MD_MD_REG_CMD1_reg, 0x00000005);
		rtd_outl(MD_MD_REG_CMD2_reg, dst_addr);
		rtd_outl(MD_MD_REG_CMD3_reg, start_addr);
		rtd_outl(MD_MD_REG_CMD4_reg, length);
		rtd_outl(MD_MD_REG_MODE_reg, 0x00000001);

		//check done bit
		timeoutcnt = 0xfffff;
		do {
			mdelay(1);
			if(rtd_inl(MD_MD_REG_MODE_reg) == 0)
				break;
		} while(timeoutcnt--);

		//Read CRC result
		get_rd_crc = rtd_inl(MD_MD_R_CRC_RESULT_reg);

		if(!rtd_inl(MD_MD_CRC_STATUS_reg)) // value must = 0
			break;

	}while(retry--);

	if(retry)
		return 0;
	else
		return -1;
}

static void get_ddr_byte_num(void)
{
	unsigned int val;
	char ddr_num;

	val = rtd_inl(DC_SYS_DC_SYS_MISC_reg);
	ddr_num = ((val>>18) & 0x3) + 1;
	ddr_num += ((val>>20) & 0x3) + 1;

	byte_num = ddr_num*2;
	//rtd_pr_dft_info("byte_num = %02x\n", byte_num);
}

static void read_tx_vref(signed char *v_range, signed char *v_val, unsigned int *mr6)
{
	unsigned int reg[2] = {DC_MC_MC_DBG_MOD_0_reg, DC_MC2_MC_DBG_MOD_0_reg};
	unsigned int mr6_val;
	unsigned char i;

	for(i = 0; i < 2; i++)
	{
		rtd_maskl(reg[i], 0x40ffffff, 0x86000000);
		mr6_val = (rtd_inl(reg[i]) >> 4) & 0x3ffff;
		rtd_pr_dft_info("\nmr6_val = %x, %x\n", rtd_inl(reg[i]), mr6_val);
		*(v_range+i) = (mr6_val>>6) & 0x1;
		*(v_val+i) = (mr6_val & 0x3f);
		*(mr6+i) = (mr6_val >> 7); //record mr6 except for vref range & value
		rtd_pr_dft_info("mr6(%x): %x, %x, %x\n", i, *(mr6+i), *(v_range+i), *(v_val+i));
	}

}

static void read_tx_dqspi(signed char *dqspi)
{
	*(dqspi) = (rtd_inl(0xb80c9010) >> 16) & 0x1f;
	*(dqspi+1) = (rtd_inl(0xb80c9010) >> 24) & 0x1f;
	*(dqspi+2) = rtd_inl(0xb80c9014) & 0x1f;
	*(dqspi+3) = (rtd_inl(0xb80c9014) >> 8) & 0x1f;
	if(byte_num > 4)
	{
		*(dqspi+4) = (rtd_inl(0xb80c9014) >> 16) & 0x1f;
		*(dqspi+5) = rtd_inl(0xb80c9098) & 0x1f;
	}
	if(byte_num > 6)
	{
		*(dqspi+6) = (rtd_inl(0xb80c9064) >> 16) & 0x1f;
		*(dqspi+7) = (rtd_inl(0xb80c9064) >> 24) & 0x1f;
	}

}

static void set_tx_vref(signed char *range, signed char *s, unsigned int *mr6, const signed char diff)
{
	signed char tx_v_range[2], tx_v_value[2];
	unsigned char i;

	for(i = 0; i < 2; i++)
	{
		//range1: 60%~92.5%, range2: 45%~77.5%
		tx_v_range[i] = *(range+i);
		tx_v_value[i] = (*(s+i) + diff);
		if(diff)
		{
			if((tx_v_range[i] == 0) && (tx_v_value[i] < 0)) //change range 1 to 2. range2 base is 59.95%.
			{
				tx_v_range[i] = 1;
				tx_v_value[i] += 0x17;
			}
			if((tx_v_range[i] == 1) && (tx_v_value[i] > 0x32)) //change range 2 to 1. range1 base is 77.55%.
			{
				tx_v_range[i] = 0;
				tx_v_value[i] -= 0x17; //0x32-0x1b = 0x17
			}
			//rtd_pr_dft_info("{%02x %02x}\n", tx_v_range[i], tx_v_value[i]);
		}
	}

	//MRS(Mode Register Set):bit[31:28]=0x6; [21:19]=0x6; [7]=0x1:enable, 0x0:disable
	rtd_maskl(DC_MC_MC_CMD_0_reg, 0x0fc7ff00, (0x60300000 | ((*mr6) << 7) | (tx_v_range[0] << 6) | tx_v_value[0]));
	rtd_outl(DC_MC_MC_CMD_8_reg, 0x001e0003); //cmd_ctl_t1[31:16],cmd_ctl_t0[15:0]
	rtd_outl(DC_MC_MC_CMD_ctl_0_reg, 0x80000000); //cmd_start[31],cmd_execute_posi[9:8],cmd_num[2:0]
	rtd_outl(DC_MC_MC_CMD_ctl_0_reg, 0x80000000);

	if(byte_num > 4)
	{
		//MRS(Mode Register Set):bit[31:28]=0x6; [21:19]=0x6; [7]=0x1:enable, 0x0:disable
		rtd_maskl(DC_MC2_MC_CMD_0_reg, 0x0fc7ff00, (0x60300000 | ((*(mr6+1)) << 7) | (tx_v_range[1] << 6) | tx_v_value[1]));
		rtd_outl(DC_MC2_MC_CMD_8_reg, 0x001e0003); //cmd_ctl_t1[31:16],cmd_ctl_t0[15:0]
		rtd_outl(DC_MC2_MC_CMD_ctl_0_reg, 0x80000000); //cmd_start[31],cmd_execute_posi[9:8],cmd_num[2:0]
		rtd_outl(DC_MC2_MC_CMD_ctl_0_reg, 0x80000000);
	}

}

static void set_tx_dqspi(const signed char diff)
{
	signed char step;
	signed char pi_cur[BYTE_MAX];
	unsigned char i;

	read_tx_dqspi((signed char *)&pi_cur);

	step = diff - tx_pre_dqspi_set;
	while(step)
	{
		if(step > 0)
		{
			for(i = 0; i < byte_num; i++)
			{
				pi_cur[i] += 1;
				if(pi_cur[i] > 31) //up boundary
					pi_cur[i] -= 32;
			}
			step--;
		}
		else
		{
			for(i = 0; i < byte_num; i++)
			{
				pi_cur[i] -= 1;
				if(pi_cur[i] < 0) //low boundary
					pi_cur[i] += 32;
			}
			step++;
		}

		rtd_maskl(0xb80c9010, 0xe0e0ffff, ((pi_cur[1] << 24) | (pi_cur[0] << 16)));
		rtd_maskl(0xb80c9014, 0xffffe0e0, ((pi_cur[3] << 8) | pi_cur[2]));
		if(byte_num > 4)
		{
			rtd_maskl(0xb80c9014, 0xffe0ffff, (pi_cur[4] << 16));
			rtd_maskl(0xb80c9098, 0xffffffe0, pi_cur[5]);
		}
		if(byte_num > 6)
			rtd_maskl(0xb80c9064, 0xe0e0ffff, ((pi_cur[7] << 24) | (pi_cur[6] << 16)));

		//rtd_pr_dft_info("%02x %02x %02x %02x %02x %02x %02x %02x\n", pi_cur[0], pi_cur[1], pi_cur[2], pi_cur[3], pi_cur[4], pi_cur[5], pi_cur[6], pi_cur[7]);
	}

	tx_pre_dqspi_set = diff;

}

int tx_calibration(unsigned int md_len, signed char vref_tap, signed char dqppi)
{
	signed char tx_vref_dqspi_set4[4][2] = {{vref_tap, -dqppi}, {vref_tap, dqppi}, {-vref_tap, dqppi}, {-vref_tap, -dqppi}};
	signed char tx_dqspi_set3[2] = {-dqppi, dqppi};
	signed char vref_range[2], vref_value[2];
	signed char dqs[BYTE_MAX] = {[0 ... (BYTE_MAX-1)] = 0};
	unsigned char round;
	char res = -1, limit = 4;
	int ddr_type;
	unsigned long md_addr;
	unsigned int mr6_val[2];
	void * md_vir;

	md_vir = dvr_malloc(md_len*2);
	md_addr = dvr_to_phys(md_vir);
	//rtd_pr_dft_info("md_addr = %lx\n", md_addr);
	if(md_addr == INVALID_VAL)
	{
		rtd_pr_dft_info("md malloc fail\n");
		return -1;
	}

	if(MD_rw((unsigned int)md_addr, md_len))
	{
		dvr_free(md_vir);
		return -2;
	}

	ddr_type = is_DDR4();
	get_ddr_byte_num();

	//save original settings
	//Vref
	if(ddr_type) //DDR4
	{
		if(vref_tap == 0)
			limit = 2;

		read_tx_vref((signed char *)&vref_range, (signed char *)&vref_value, (unsigned int *)&mr6_val);
	}

	read_tx_dqspi((signed char *)&dqs);

	if(ddr_type) //DDR4  set vref & dqspi
	{
		for(round = 0; round < limit; round++)
		{
			//rtd_pr_dft_info("[%d, %d]\n", tx_vref_dqspi_set4[round][0], tx_vref_dqspi_set4[round][1]);
			set_tx_vref((signed char *)&vref_range, (signed char *)&vref_value, (unsigned int *)&mr6_val, tx_vref_dqspi_set4[round][0]);
			set_tx_dqspi(tx_vref_dqspi_set4[round][1]);

			res = MD_rw((unsigned int)md_addr, md_len);
			if(res) //fail
				break;
		}

		//back to original point
		set_tx_vref((signed char *)&vref_range, (signed char *)&vref_value, (unsigned int *)&mr6_val, 0);
		set_tx_dqspi(0);

	}
	else //DDR3
	{
		for(round = 0; round < 2; round++)
		{
			set_tx_dqspi(tx_dqspi_set3[round]);

			res = MD_rw((unsigned int)md_addr, md_len);
			if(res) //fail
				break;
		}

		//back to original point
		set_tx_dqspi(0);
	}

	dvr_free(md_vir);

	if(res) //fail
		return (round+1);
	else
		return 0;

}

static void read_rx_vref(signed char *v_range, signed char *v_val)
{
	*(v_range) = (rtd_inl(0xb80ca1fc)>>7) & 0x1;  // byte0/1
	*(v_range+1) = (rtd_inl(0xb80ca1fc)>>23) & 0x1; // byte2/3
	*(v_range+2) = (rtd_inl(0xb80c9300)>>7) & 0x1;  // byte4/5
	*(v_range+3) = (rtd_inl(0xb80c9300)>>23) & 0x1; // byte6/7
	*(v_val) = (rtd_inl(0xb80ca1fc)>>1) & 0x3f;  // byte0/1 (S[5] bit no use)
	*(v_val+1) = (rtd_inl(0xb80ca1fc)>>17) & 0x3f; // byte2/3
	*(v_val+2) = (rtd_inl(0xb80c9300)>>1) & 0x3f;  // byte4/5
	*(v_val+3) = (rtd_inl(0xb80c9300)>>17) & 0x3f; // byte6/7
}

static void read_rx_te(signed char *Te)
{
	short i, j;
	const unsigned int te_reg[BYTE_MAX] = {0xb80ca440, 0xb80ca444, 0xb80ca640, 0xb80ca644, 0xb80c9840, 0xb80c9844, 0xb80c9a40, 0xb80c9a44};
	unsigned int reg;
	unsigned char pos = 0, shift;

	for(i = 0; i < byte_num; i++)
	{
		reg = te_reg[i];
		for(j = 0; j < 8; j++)
		{
			if(((j%4) == 0) && (j != 0))
			{
				reg += 8;
			}

			shift = (j % 4) * 8;
			*(Te+pos) = (rtd_inl(reg) >> shift) & 0xff;

			pos++;
		}
	}
}

static void set_rx_vref(signed char *range, signed char *s, signed char diff)
{
	signed char rx_v_range[RX_VREF], rx_v_value[RX_VREF];
	signed char byte[RX_VREF]; //01, 23, 45, 67
	unsigned char i;
	unsigned int timeoutcnt = 0xffff;

	for(i = 0; i < RX_VREF; i++)
	{
		rx_v_range[i] = *(range+i);
		rx_v_value[i] = (*(s+i) + diff);

		if(diff)
		{
			//range=0, 55%~86%, range=1, 35%~66%
			if((rx_v_range[i] == 0) && (rx_v_value[i] < 0)) //change range 0 -> 1.
			{
				rx_v_range[i] = 1;
				rx_v_value[i] += 20; //55-35 = 20
			}
			if((rx_v_range[i] == 1) && (rx_v_value[i] > 31)) //change range 1 -> 0.
			{
				rx_v_range[i] = 0;
				rx_v_value[i] -= 20;
			}
		}
	}
	//rtd_pr_dft_info("range: %02x %02x %02x %02x, s: %02x %02x %02x %02x\n", rx_v_range[0], rx_v_range[1], rx_v_range[2], rx_v_range[3], rx_v_value[0], rx_v_value[1], rx_v_value[2], rx_v_value[3]);
	
	for(i = 0; i < RX_VREF; i++)
		byte[i] = ((rx_v_range[i] << 6) | rx_v_value[i]);

	rtd_outl(0xb80ca1fc, ((byte[1] << 25) | (byte[1] << 17) | (byte[0] << 9) | (byte[0] << 1)));
	rtd_outl(0xb80c9300, ((byte[3] << 25) | (byte[3] << 17) | (byte[2] << 9) | (byte[2] << 1)));

	rtd_maskl(0xb80ca238, 0xFFFFFFFC, 0x00000001); //update rx vref, [1:0]=0x1
	rtd_maskl(0xb80c9238, 0xFFFFFFFC, 0x00000001); //update rx vref, [1:0]=0x1
	rtd_maskl(0xb80c923c, 0xfffffff3, 0x0000000c); //update read delay tap
	rtd_outl(DC_MC_MC_SYS_REF_1_reg, 0x0a100200); //MC1 sync
	rtd_outl(DC_MC2_MC_SYS_REF_1_reg, 0x0a100200); //MC2 sync
	//check update
	do {
		mdelay(1);
		if((rtd_inl(0xb80ca2e4) == ((rx_v_value[1] << 24) | (rx_v_value[1] << 16) | (rx_v_value[0] << 8) | rx_v_value[0])) 
		&& (rtd_inl(0xb80c9310) == ((rx_v_value[3] << 24) | (rx_v_value[3] << 16) | (rx_v_value[2] << 8) | rx_v_value[2])))
			break;
	}while(timeoutcnt--);

	rtd_maskl(0xb80ca238, 0xFFFFFFFC, 0x00000000); //update rx vref, [1:0]=0x0
	rtd_maskl(0xb80c9238, 0xFFFFFFFC, 0x00000000); //update rx vref, [1:0]=0x0

}

static void set_rx_te(signed char *Te, signed char diff)
{
	const unsigned int te_reg[BYTE_MAX] = {0xb80ca440, 0xb80ca444, 0xb80ca640, 0xb80ca644, 0xb80c9840, 0xb80c9844, 0xb80c9a40, 0xb80c9a44};
	unsigned char i, j;
	signed char step, dqs;
	unsigned int reg, val;
	unsigned char shift;
	signed char *te = Te;
	signed char te_tar[4], te_cur[4];

	for(i = 0; i < byte_num; i++)
	{
		reg = te_reg[i];
		dqs = 8;
		while(dqs > 0)
		{
			for(j = 0; j < 4; j++)
			{
				shift = j*8;
				te_tar[j] = (*te)+diff;
				te_cur[j] = (rtd_inl(reg) >> shift) & 0xff;
				te++;
			}
			step = te_tar[0] - te_cur[0];
			while(step)
			{
				val = 0;
				if(step > 0)
				{
					for(j = 0; j < 4; j++)
					{
						shift = j*8;
						te_cur[j]++;
						if(te_cur[j] > 31)
							val |= (31 << shift);
						else
							val |= (te_cur[j] << shift);
					}
					step--;
				}
				else
				{
					for(j = 0; j < 4; j++)
					{
						shift = j*8;
						te_cur[j]--;
						if(te_cur[j] < 0)
							val |= 0;
						else
							val |= (te_cur[j] << shift);
					}
					step++;
				}
				rtd_outl(reg, val);
				//rtd_pr_dft_info("rtd_inl(0x%08x) = 0x%08x\n", reg, rtd_inl(reg));
			}

			reg += 8;
			dqs -= 4;
		}

	}

	rtd_outl(0xb80c923c, 0x0000000c);

}

int rx_calibration(unsigned int md_len, signed char vref_tap, signed char te_tap)
{
	signed char rx_vref_te_set[4][2] = {{vref_tap, -te_tap}, {vref_tap, te_tap}, {-vref_tap, te_tap}, {-vref_tap, -te_tap}};
	signed char vref_range[RX_VREF], vref_s[RX_VREF];
	signed char te[BYTE_MAX][8];
	unsigned char round, limit = 4;
	char res = -1, autoK;
	unsigned long md_addr;
	void * md_vir;

	md_vir = dvr_malloc(md_len*2);
	md_addr = dvr_to_phys(md_vir);
	//rtd_pr_dft_info("md_addr = %lx\n", md_addr);
	if(md_addr == INVALID_VAL)
	{
		rtd_pr_dft_info("md malloc fail\n");
		return -1;
	}

	if(MD_rw((unsigned int)md_addr, md_len))
	{
		dvr_free(md_vir);
		return -2;
	}

	get_ddr_byte_num();

	//save original settings
	//Vref
	if(vref_tap == 0)
		limit = 2;

	read_rx_vref((signed char *)&vref_range, (signed char *)&vref_s);
	//TE
	read_rx_te((signed char *)&te);

	//disable 3point
	rtd_maskl(0xb80ca238, 0xFFFFFFCF, 0x00000030); // [5:4]=0x3
	if(byte_num > 4)
		rtd_maskl(0xb80c9238, 0xFFFFFFCF, 0x00000030); // [5:4]=0x3

	if((rtd_inl(0xb80ca238) & 0x60000) == 0x60000) {
		autoK = 0;
	}
	else
	{
		rtd_maskl(0xb80ca238, 0xfff7ffff, 0x0); //dqs_en not update [19]=0
		rtd_maskl(0xb80c9238, 0xfff7ffff, 0x0);
		rtd_maskl(0xb80ca2d0, 0xffffffdf, (1<<5)); //delta k off [5]=1
		rtd_maskl(0xb80c92d0, 0xffffffdf, (1<<5));
	}

	for(round = 0; round < limit; round++)
	{
		//rtd_pr_dft_info("[%d %d]\n", rx_vref_te_set[round][0], rx_vref_te_set[round][1]);
		set_rx_vref((signed char *)&vref_range, (signed char *)&vref_s, rx_vref_te_set[round][0]);
		set_rx_te((signed char *)&te, rx_vref_te_set[round][1]);

		res = MD_rw((unsigned int)md_addr, md_len);
		if(res) //fail
			break;
	}

	//back to original point
	set_rx_vref((signed char *)&vref_range, (signed char *)&vref_s, 0);
	set_rx_te((signed char *)&te, 0);

	//enable 3point
	rtd_maskl(0xb80ca238, 0xFFFFFFCF, 0x00000000);// [5:4]=0x0
	if(byte_num > 4)
		rtd_maskl(0xb80c9238, 0xFFFFFFCF, 0x00000000);// [5:4]=0x0
	if(autoK) {
		rtd_maskl(0xb80ca238, 0xfff7ffff, (1<<19));
		rtd_maskl(0xb80c9238, 0xfff7ffff, (1<<19));
		rtd_maskl(0xb80ca2d0, 0xffffffdf, 0x0);
		rtd_maskl(0xb80c92d0, 0xffffffdf, 0x0);
	}

	dvr_free(md_vir);

	if(res) //fail
		return (round+1);
	else
		return 0;

}

static int check_pok_data(void)
{
	unsigned char *data_ptr = phys_to_virt(HW_SETTING_POK_KEY_ADDR);
	unsigned int i, checksum = 0, size;

	size = (HW_SETTING_POK_CHECKSUM_ADDR - HW_SETTING_POK_KEY_ADDR);
	for(i = 0; i < size; i++)
		checksum += *(unsigned char *)(data_ptr+i);
	rtd_pr_dft_info("%x, %x", *(unsigned int *)(data_ptr+size), checksum);
	if(*(unsigned int *)(data_ptr+size) == checksum)
		return 0;
	else
		return -1;
}

static int get_pok_tx_result(unsigned int *vref, signed char *pi)
{
	signed char *data_ptr = phys_to_virt(HW_SETTING_POK_TX_ADDR);
	signed char vref_range[2], vref_value[2];
	unsigned char i;

	//rtd_pr_dft_info("magic = %x\n", *(unsigned int *)data_ptr);
	if(*(unsigned int *)data_ptr == HW_SETTING_POK_TX_TAG)
	{
		if(is_DDR4()) {
			for(i = 0; i < 2; i++) {
				vref_range[i] = *(signed char *)(data_ptr+4+i);
				vref_value[i] = *(signed char *)(data_ptr+6+i);
				//*(vref+i) = ((vref_range[i] << 6) | vref_value[i]);
				if(vref_range[i] == 0)
					*(vref+i) = 6000 + 65*vref_value[i];
				else if(vref_range[i] == 1)
					*(vref+i) = 4500 + 65*vref_value[i];
			}

			for(i = 0; i < byte_num; i++) {
				*(pi+i) = *(signed char *)(data_ptr+8+i);
				if(*(pi+i) > 31) //up boundary
					*(pi+i) -= 32;

				if(*(pi+i) < 0) //low boundary
					*(pi+i) += 32;
			}
		}

		return 0;
	}
	else
		return -1;

}

static int get_pok_rx_result(unsigned int *vref, unsigned char *Te)
{
	unsigned char *data_ptr = phys_to_virt(HW_SETTING_POK_RX_ADDR);
	signed char vref_range[2], vref_value[2];
	unsigned char i, j, pos = 0;

	//rtd_pr_dft_info("magic = %x\n", *(unsigned int *)data_ptr);
	if(*(unsigned int *)data_ptr == HW_SETTING_POK_RX_TAG)
	{
		if(is_DDR4()) {
			for(i = 0; i < 2; i++) {
				vref_range[i] = *(unsigned char *)(data_ptr+4+2*i);
				vref_value[i] = *(unsigned char *)(data_ptr+8+2*i);
				//*(vref+i) = ((vref_range[i] << 6) | vref_value[i]);
				if(vref_range[i] == 0)
					*(vref+i) = 55 + vref_value[i];
				else if(vref_range[i] == 1)
					*(vref+i) = 35 + vref_value[i];
			}

			for(i = 0; i < byte_num; i++)
			{
				for(j = 0; j < 9; j++)
				{
					*(Te+pos) = *(unsigned char *)(data_ptr+0x10+pos);
					pos++;
				}
			}
		}

		return 0;
	}
	else
		return -1;

}

static int ddr_phase_scan(void)
{
	int res1 = 0, res2 = 0, loop = 0;

	rtd_pr_dft_notice("\n=== Check Tx/Rx Calibration===\n");
	do {
		res1 = tx_calibration(MD_LENGTH, TX_VREF, TX_DQSPI);
		if(is_DDR4()) {
			res2 = rx_calibration(MD_LENGTH, RX_VREF_DDR4_TAP, RX_TE_TAP);
		} else {
			res2 = rx_calibration(MD_LENGTH, RX_VREF_DDR3_TAP, RX_TE_TAP);
		}

		loop++;
		if((res1 == 0) && (res2 == 0))
			rtd_pr_dft_notice("=========TX/RX PASS (%d)=========\n", loop);
		else {
			if(res1)
				rtd_pr_dft_emerg("=========TX FAIL (%d.%d)=========\n", loop, res1);
			if(res2)
				rtd_pr_dft_emerg("=========RX FAIL (%d.%d)=========\n", loop, res2);

			return loop;
		}
	}while(loop < 10);
	rtd_pr_dft_notice("\n================================\n");

	return 0;
}

static int ddr_phase_scan_thread(void *arg)
{
	int ret = 0;

	g_dft_kthreadRunning = true;

	g_dftphasescan_ret = ddr_phase_scan();

	g_dft_kthreadRunning = false;
	kthread_stop(dft_kthread);

	return ret;

}

static ssize_t dft_phase_scan(struct kobject *kobj, struct kobj_attribute *attr,
						char *buf)
{
	int ret = 0;

	ret = ddr_phase_scan();
	if(ret)
	{
		return sprintf(buf, "TX/RX:FAIL(%d)\n", ret);
	}
	else
	{
		return sprintf(buf, "TX/RX:PASS\n");
	}

}

static ssize_t dft_phase_scan_tx_show(struct kobject *kobj, struct kobj_attribute *attr,
						char *buf)
{
	return sprintf(buf, "%s\n", OutData);
}

static ssize_t dft_phase_scan_tx_store(struct kobject *kobj, struct kobj_attribute *attr,
						const char *buf, size_t count)
{
	const char *ptr = buf;
	char *endptr;
	signed char tx_vref = TX_VREF, tx_dqspi = TX_DQSPI;
	unsigned long md_length = MD_LENGTH;
	int res;

	md_length = simple_strtol(ptr, &endptr, 16);
	ptr = endptr+1;
	tx_vref = simple_strtol(ptr, &endptr, 16);
	ptr = endptr+1;
	tx_dqspi = simple_strtol(ptr, &endptr, 16);

	rtd_pr_dft_notice("\n===Test Tx Calibration===\n");
	rtd_pr_dft_notice("md_length = %lx, tx_vref = %x, rx_te_tap = %x\n", md_length, tx_vref, tx_dqspi);
	res = tx_calibration(md_length, tx_vref, tx_dqspi);
	if(res)
	{
		rtd_pr_dft_notice("=========TX FAIL:%d=========\n", res);
		sprintf(OutData, "TX FAIL:%d\n", res);
	}
	else
	{
		sprintf(OutData, "TX PASS\n");
	}
	rtd_pr_dft_notice("\n================================\n");

	return count;
}

static ssize_t dft_phase_scan_rx_show(struct kobject *kobj, struct kobj_attribute *attr,
						char *buf)
{
	return sprintf(buf, "%s\n", OutData);
}

static ssize_t dft_phase_scan_rx_store(struct kobject *kobj, struct kobj_attribute *attr,
						const char *buf, size_t count)
{
	const char *ptr = buf;
	char *endptr;
	signed char rx_vref_tap = RX_VREF_DDR4_TAP, rx_te_tap = RX_TE_TAP;
	unsigned long md_length = MD_LENGTH;
	int res;

	md_length = simple_strtol(ptr, &endptr, 16);

	ptr = endptr+1;
	rx_vref_tap = simple_strtol(ptr, &endptr, 16);

	ptr = endptr+1;
	rx_te_tap = simple_strtol(ptr, &endptr, 16);
	rtd_pr_dft_notice("\n===Test Rx Calibration===\n");
	rtd_pr_dft_notice("md_length = %lx, rx_vref_tap = %x, rx_te_tap = %x\n", md_length, rx_vref_tap, rx_te_tap);
	res = rx_calibration(md_length, rx_vref_tap, rx_te_tap);
	if(res)
	{
		rtd_pr_dft_notice("=========RX FAIL:%d=========\n", res);
		sprintf(OutData, "RX FAIL:%d\n", res);
	}
	else
	{
		sprintf(OutData, "RX PASS\n");
	}
	rtd_pr_dft_notice("\n================================\n");

	return count;
}

static ssize_t dft_pok_result(struct kobject *kobj, struct kobj_attribute *attr,
						char *buf)
{
	unsigned int tx_vref[2] = {0, 0} ,rx_vref[2] = {0, 0};
	signed char dqspi[BYTE_MAX], te[BYTE_MAX][9];
	int res, len;
	unsigned char i, j;
	char str[512] = {0};

	rtd_pr_dft_notice("\n===Show POK Result===\n");
	if(check_pok_data())
	{
		len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "Get POK Result Fail: checksum error!\n");
		if(len < 0)
			goto end_proc_str;
	}
	get_ddr_byte_num();
	res = get_pok_tx_result((unsigned int *)&tx_vref, (signed char *)&dqspi);
	if(res)
	{
		len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "Get Tx Result Fail\n");
		if(len < 0)
			goto end_proc_str;
	}
	else
	{
		len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "Tx Vref: %d.%02d%c %d.%02d%c\nTx DQS_PI: ",
			(tx_vref[0]/100), (tx_vref[0]%100), 0x25, (tx_vref[1]/100), (tx_vref[1]%100), 0x25);
		if(len < 0)
			goto end_proc_str;

		for(i = 0; i < byte_num; i++) {
			len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "0x%02x ", dqspi[i]);
			if(len < 0)
				goto end_proc_str;
		}

		len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "\n");
		if(len < 0)
			goto end_proc_str;
	}
	res = get_pok_rx_result((unsigned int *)&rx_vref, (unsigned char *)&te);
	if(res)
	{
		len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "Get Rx Result Fail\n");
		if(len < 0)
			goto end_proc_str;
	}
	else
	{
		len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "Rx Vref: %d%c %d%c \nRx Te:\n", rx_vref[0], 0x25, rx_vref[1], 0x25);
		if(len < 0)
			goto end_proc_str;

		for(i = 0; i < byte_num; i++)
		{
			for(j = 0; j < 8; j++) {
				len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "0x%02x ", te[i][j]);
				if(len < 0)
					goto end_proc_str;
			}

			len = snprintf(str + strlen(str), sizeof(str) - strlen(str), "\n");
			if(len < 0)
				goto end_proc_str;
		}
	}
	rtd_pr_dft_notice("\n=====================\n");

end_proc_str:

	memcpy(buf, str, strlen(str) + 1); // +1 for null character '\0'

	return strlen(buf);
}

static struct kobj_attribute dft_phase_scan_attribute =
	__ATTR(phase_scan, 0644, dft_phase_scan, NULL);
static struct kobj_attribute dft_phase_scan_tx_attribute =
	__ATTR(phase_scan_tx, 0644, dft_phase_scan_tx_show, dft_phase_scan_tx_store);
static struct kobj_attribute dft_phase_scan_rx_attribute =
	__ATTR(phase_scan_rx, 0644, dft_phase_scan_rx_show, dft_phase_scan_rx_store);
static struct kobj_attribute dft_pok_result_attribute =
	__ATTR(pok_result, 0644, dft_pok_result, NULL);

static struct attribute *attrs[] = {
	&dft_phase_scan_attribute.attr,
	&dft_phase_scan_tx_attribute.attr,
	&dft_phase_scan_rx_attribute.attr,
	&dft_pok_result_attribute.attr,
	NULL,	/* need to NULL terminate the list of attributes */
};

static struct attribute_group attr_group = {
	.attrs = attrs,
};

/* DFT char device file operation session */
long dft_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long retval = 0;
	/* no user space ioctl  */

	switch (cmd) {
		case RTKDFT_IOC_PHASE_SCAN: {
			if(ddr_phase_scan())
				retval = -1;

			break;
		}
		default:
			rtd_pr_dft_err("DFT : wrong ioctl cmd\n");
			retval = -ENOTTY;
	}

	return retval;
}

#ifdef CONFIG_COMPAT
long dft_ioctl_compat(struct file *filp, unsigned int cmd, unsigned long arg)
{
	long retval = 0;
	int scan_ret = 0;
	/* no user space ioctl  */

	switch (cmd) {
		case RTKDFT_IOC_PHASE_SCAN: {
			if (g_dft_kthreadRunning == false){
				dft_kthread = kthread_run(ddr_phase_scan_thread, NULL, "rtkdtf_thread");
				if (!IS_ERR(dft_kthread)) {
					retval= 0; //create ddr_phase_scan success
				}
				else
					retval = -1;
			}
			break;
		}
		case RTKDFT_IOC_GET_PHASE_SCAN_INFO: {
			if((g_dft_kthreadRunning == true) && dft_kthread){
				scan_ret = 2; //busy
			}
			else if(g_dft_kthreadRunning == false){
				if(g_dftphasescan_ret == 0)
					scan_ret = 0; //success;
				else
					scan_ret = 1; //fail
			}

			if(copy_to_user((void __user *)arg, (void *)&scan_ret, sizeof(int)))
			{
				retval = -EFAULT;
				break;
			}
			retval= 0;
			break;
		}
		case RTKDFT_IOC_AUTO_PHASE_SCAN: {
			if(ddr_phase_scan())
				retval = -1;

			break;
		}
		default:
			rtd_pr_dft_err("DFT : wrong ioctl cmd\n");
			retval = -ENOTTY;
	}

	return retval;
}
#endif

int dft_open(struct inode *inode, struct file *filp)
{
	return 0;
}

int dft_release(struct inode *inode, struct file *filp)
{
	return 0;
}

/* DFT file operation */
struct file_operations dft_fops = {
read:
	NULL,
unlocked_ioctl:
	dft_ioctl,
#ifdef CONFIG_COMPAT
compat_ioctl:
	dft_ioctl_compat,
#endif
open:
	dft_open,
release:
	dft_release,
};

/* set /dev/dft mode to 0666  */
static char *dft_devnode(struct device *dev, umode_t *mode)
{
	if (mode)
		*mode = 0666;
	return NULL;
}

#ifdef CONFIG_PM
static int dft_suspend(struct platform_device *dev, pm_message_t state)
{
	rtd_pr_dft_debug("dft_suspend\n");
	return 0;
}
static int dft_resume(struct platform_device *dev)
{
	rtd_pr_dft_debug("dft_resume\n");
	return 0;
}
#endif

static struct platform_driver dft_platform_driver = {
#ifdef CONFIG_PM
	.suspend    = dft_suspend,
	.resume     = dft_resume,
#endif
	. driver = {
		.name       = "dft",
		.bus        = &platform_bus_type,
	} ,
} ;


static int __init dft_module_init(void)
{
	int retval;

	dft_kobj = kobject_create_and_add("dft", NULL);
	if (!dft_kobj)
		return -ENOMEM;

	retval = sysfs_create_group(dft_kobj, &attr_group);
	if (retval)
		kobject_put(dft_kobj);

	/* register device to get major and minor number */
	if (dft_major) {
		dft_devnum = MKDEV(dft_major, dft_minor);
		retval = register_chrdev_region(dft_devnum, dft_nr_devs, "dft");
	} else {
		retval = alloc_chrdev_region(&dft_devnum, dft_minor, dft_nr_devs, "dft");
		dft_major = MAJOR(dft_devnum);
	}
	if (retval < 0) {
		rtd_pr_dft_warn("DFT : can't get major %d\n", dft_major);
		return retval;
	}

	/* create device node by udev API */
	dft_class = class_create(THIS_MODULE, "dft");
	if (IS_ERR(dft_class)) {
		return PTR_ERR(dft_class);
	}
	dft_class->devnode = dft_devnode;
	dft_device = device_create(dft_class, NULL, dft_devnum, NULL, "dft");
	rtd_pr_dft_info("DFT module init, major number = %d, device name = %s \n", dft_major, dev_name(dft_device));

	/* cdev API to register file operation */
	cdev_init(&dft_cdev, &dft_fops);
	dft_cdev.owner = THIS_MODULE;
	dft_cdev.ops = &dft_fops;
	retval = cdev_add(&dft_cdev, dft_devnum , 1);
	if (retval) {
		rtd_pr_dft_err("Error %d adding char_reg_setup_cdev", retval);
	}

	dft_platform_devs = platform_device_register_simple("dft", -1, NULL, 0);
	/* rtd_pr_dft_info("DFT platform device name %s\n", dev_name(&dft_platform_devs->dev)); */

	if (platform_driver_register(&dft_platform_driver) != 0) {
		platform_device_unregister(dft_platform_devs);
		dft_platform_devs = NULL;
		rtd_pr_dft_warn("dft platform driver register fail\n");
		return retval;
	}

	spin_lock_init(&dft_lock);

	return retval;
}

static void __exit dft_module_exit(void)
{
	if (dft_platform_devs)
		platform_device_unregister(dft_platform_devs);

	platform_driver_unregister(&dft_platform_driver);

	kobject_put(dft_kobj);
	device_destroy(dft_class, dft_devnum);
	class_destroy(dft_class);
	unregister_chrdev_region(dft_devnum, dft_nr_devs);

}

module_init(dft_module_init);
module_exit(dft_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("realtek.com");
