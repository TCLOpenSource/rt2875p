/* rtk hw monitor
 *
 * Created by: davidwang@realtek.com
 * Copyright: (C) 2015 Realtek Semiconductor Corp.
 *
 *
 */

#include <rtd_log/rtd_module_log.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/kobject.h>
#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 1))
#include <linux/sched/types.h>
#endif
#include <uapi/linux/sched/types.h>
#include <linux/delay.h>
#include <linux/err.h> /* IS_ERR() PTR_ERR() */
#include <asm/io.h>    /* rtd_inl() rtd_outl() ... */
#include <rtk_kdriver/io.h>
#if (defined CONFIG_RTK_KDRIVER) || (defined CONFIG_RTK_KDRIVER_SUPPORT)
#include <rtk_kdriver/rtk_thermal_sensor.h>
#include <rtk_kdriver/rtk_gpio.h>
#else
#include <mach/rtk_thermal_sensor.h>
#include <mach/rtk_gpio.h>
#endif
#include <mach/rtk_platform.h>
#include <linux/freezer.h>
#include <rbus/stb_reg.h>
#include <scaler_vbedev.h>
#include <rbus/vby1_reg.h>
#include <rbus/pinmux_lvdsphy_reg.h>
static DEFINE_MUTEX(rtk_monitor_activation_lock);
static DEFINE_MUTEX(rtk_monitor_delay_lock);
static unsigned int rtk_monitor_active = 0;
static unsigned int rtk_monitor_delay = 1000; // ms
unsigned int rtk_monitor_debug = 0;
#ifdef VBE_DISP_TCON_SELF_CHECK
unsigned int monitor_rgb_rolling=0;
#include "tvscalercontrol/io/ioregdrv.h"
#include <rbus/mdomain_disp_reg.h>
#endif

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>

static unsigned int	ZQ_init_flag = 0;

#define CONFIG_RTK_ZQ_ENABLE
static struct task_struct *rtk_monitor_task;

static unsigned long pre_jiffies;

#define R480_INDEX 32
#define SET_INDEX 64
#define TOTAL_SET 8

#define DC1_OCDP_SET0 0xb80be190 //90~94
#define DC1_OCDN_SET0 0xb80be198 //98~9c
#define DC1_OCDP_SET1 0xb80be1B0 //b0~b4
#define DC1_OCDN_SET1 0xb80be1B8 //b8~bc
#define DC1_PAD_RZCTRL_STATUS 0xb80be14C

#define DC1_PAD_CTRL_PROG 0xb80be130
#define DC1_PAD_ZCTRL_STATUS 0xb80be140
#define DC1_PAD_CTRL_ZPROG 0xb80be314 //set:8
#define DC1_PAD_NOCD2_ZPROG 0xb80be334 //set:8

#define DC1_DDR3_CTL 0xb8008840
//dummy reg.
#define ZQFW_CTRL1 0xb8008E88
#define ZQFW_CTRL2 0xb8008E9C

unsigned int test_mode=0, ZQ_Config=0;

unsigned int counter = 0;
unsigned int hwsetting_idx = 5;
unsigned int hwsetting_result[5];
unsigned int DC1_R480[32];
unsigned int DC1_R480_max=0, DC1_R480_min=100;
unsigned int DC1_SET_max[16], DC1_SET_min[16];
unsigned int DC1_PMOS_SET0[64], DC1_PMOS_SET1[64], DC1_PMOS_SET2[64], DC1_PMOS_SET3[64];
unsigned int DC1_PMOS_SET4[64], DC1_PMOS_SET5[64], DC1_PMOS_SET6[64], DC1_PMOS_SET7[64];
unsigned int DC1_NMOS_SET0[64], DC1_NMOS_SET1[64], DC1_NMOS_SET2[64], DC1_NMOS_SET3[64];
unsigned int DC1_NMOS_SET4[64], DC1_NMOS_SET5[64], DC1_NMOS_SET6[64], DC1_NMOS_SET7[64];
unsigned int DC1_R480_err_msg[2];
unsigned int DC1_R480_Total[32];
unsigned int DC1_PMOS_SET0_Total[64], DC1_PMOS_SET1_Total[64], DC1_PMOS_SET2_Total[64], DC1_PMOS_SET3_Total[64];
unsigned int DC1_PMOS_SET4_Total[64], DC1_PMOS_SET5_Total[64], DC1_PMOS_SET6_Total[64], DC1_PMOS_SET7_Total[64];
unsigned int DC1_NMOS_SET0_Total[64], DC1_NMOS_SET1_Total[64], DC1_NMOS_SET2_Total[64], DC1_NMOS_SET3_Total[64];
unsigned int DC1_NMOS_SET4_Total[64], DC1_NMOS_SET5_Total[64], DC1_NMOS_SET6_Total[64], DC1_NMOS_SET7_Total[64];

static unsigned int rtd_ZQ_inl(unsigned int REG)
{
	unsigned int Val;

	rtd_inl(REG);
	Val = rtd_inl(REG);
	return Val;
}
static void rtd_ZQ_maskl(unsigned int REG, unsigned int mask_bits, unsigned int value)
{
    unsigned int Val;
	Val = rtd_ZQ_inl(REG);
	Val = (Val & mask_bits)|value;
	rtd_outl(REG, Val);
}
static void Array_init(unsigned int Array[], unsigned int element, unsigned int value)
{
	unsigned int i;

	for(i=0; i<element; i++)
		Array[i] = value;
}

static void Get_Ori_ZQ_Phase(unsigned int flag)
{
	unsigned int Reg[5]={DC1_OCDP_SET0,(DC1_OCDP_SET0+0x4),DC1_OCDN_SET0,(DC1_OCDN_SET0+0x4),DC1_PAD_RZCTRL_STATUS};
    unsigned int element = hwsetting_idx;
	unsigned int sts_index = 4;
	unsigned int i;

    if(flag == 0) {
        Array_init(hwsetting_result, element, 0);
        for(i = 0; i < element; i++) {
            if(i < sts_index)
				hwsetting_result[i] = rtd_ZQ_inl(Reg[i]);
			else
				hwsetting_result[i] = (rtd_ZQ_inl(Reg[i])&0x3E)>>1; //[5:1]
		}
    } else if(flag == 1) {
        for(i = 0; i < element; i++)
			rtd_pr_zq_debug("[ZqHwsetting]0x%08x=0x%x",Reg[i],hwsetting_result[i]);
	}
}

static void Kernal_ZqCalibration(void)
{
	int tmp_val=0, print_flag=10;
	unsigned int total_SET = TOTAL_SET; //SET0~7
	unsigned int i, loop, bit_shift;
	unsigned int R480_test_loop = 5, OCD_test_loop = 5;
	unsigned int R480_max_loop, ZQ_max_loop;
	unsigned int PMOS_SET[TOTAL_SET], NMOS_SET[TOTAL_SET];
	unsigned int R480, PMOS[TOTAL_SET], NMOS[TOTAL_SET];
	unsigned int loop_delay, para_delay;
	unsigned int err_type = 0;
	unsigned int OCDP_Addr, OCDN_Addr;

	rtd_ZQ_maskl(ZQFW_CTRL1, 0xffffefff, 0x1000); //[12]=1 zq fw start

    if((test_mode & 0x2) == 0x2) { //[1]Main Enable
        Array_init(PMOS_SET, total_SET, 0);
        Array_init(NMOS_SET, total_SET, 0);
        Array_init(PMOS, total_SET, 0);
        Array_init(NMOS, total_SET, 0);
		R480 = 0;

		loop_delay = ((rtd_ZQ_inl(ZQFW_CTRL2)&0xf00)>>8)*100; //[11:8]
		para_delay = ((rtd_ZQ_inl(ZQFW_CTRL2)&0xf000)>>12)*100; //[15:12]

			// ZQ Calibration

		if((ZQ_Config&0x2) == 0x2) //[1]=1,R480 one time calibration
			R480_max_loop = 1;
		else
			R480_max_loop = R480_test_loop;
		if((ZQ_Config&0x4) == 0x4) //[2]=1,SET0~3 one time calibration
			ZQ_max_loop = 1;
		else
			ZQ_max_loop = OCD_test_loop;

		//**DCU1 R480 Cal.**//
		//0xb803809C[5:4]=2'b01 ddr3
		//0xb803809C[5:4]=2'b10 ddr4
        if((test_mode & 0x4) == 0x4) { // [2]DCU1 R480 CAl. Enable
			//if(PLATFORM_KXL == get_platform()) {
			if((rtd_ZQ_inl(0xb803809c) & 0x30) == 0x20) {
				rtd_outl(0xb80be1f4,0x9fdb0000);  //Vref_range[27]:0.5%~64.5%,ZQ_Vref[26:21]:62.5%
				rtd_outl(0xb80be13c,0x00000000);  //zq_ena_nocd2 = 0
				rtd_outl(0xb80be130,0x04040003);  //R480_avgcal[31],zctrl_clk_sel[26:25]=zclk/32,rzq_ext_R240[19]
				rtd_outl(0xb80be23c,0x00000003);  //Update Write Delay Tap  
				rtd_pr_zq_debug("this is ddr4 package!\n");
			} else
			{
				rtd_pr_zq_debug("this is ddr3 package!\n");
			}	
				
            for(loop = 0; loop < R480_max_loop; loop++) { //scan loop
				rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFEFFFF, 0x00010000); // [16]=1,R480 calibration enable
            	usleep_range(180, 250);
                if((test_mode & 0x10) == 0x10) { // [4]:R480 msg
					if((rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS)&0x1) != 0x1)
						rtd_pr_zq_debug("[ZqCalDebugInfo]: R480 cal. done failed(DCU1)!\n");
						rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x): 0x%08x ", DC1_PAD_RZCTRL_STATUS, rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS));
						rtd_pr_zq_debug("[5:1]:rzq_480code(DCU1)    \n");
            	}
				rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFEFFFF, 0x00000000); // [16]=0,R480 calibration Disable

				//**calibration result**//
				tmp_val = (rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS)&0x3E)>>1; //[5:1]
				R480 += tmp_val;
				if((ZQ_Config&0x20000) == 0x20000) //[17]=1,record all scan results
					DC1_R480_Total[tmp_val]++;
				if((test_mode&0x10) == 0x10) // [4]:R480 msg
					rtd_pr_zq_debug("[ZqDebug]: %d)DC1_R480=%d\n",loop+1,tmp_val);
                if((tmp_val < 0) || (tmp_val > 31)) {
					DC1_R480_err_msg[0] += 1; //err counter
					DC1_R480_err_msg[1] = tmp_val;
					if((test_mode&0x10) == 0x10) // [4]:R480 msg
						rtd_pr_zq_info("[ZqErrorMsg] ERROR!!DC1_R480=%d\n",tmp_val);
				}
                if(tmp_val > DC1_R480_max) {
					DC1_R480_max = tmp_val;
					rtd_pr_zq_debug("[ZqCompare]: DC1_R480_max=%d\n",DC1_R480_max);
				}
                if(tmp_val < DC1_R480_min) {
					DC1_R480_min = tmp_val;
					rtd_pr_zq_debug("[ZqCompare]: DC1_R480_min=%d\n",DC1_R480_min);
        		}
        	}//scan loop(s)

            if(R480_max_loop == R480_test_loop) { //rounding for multi-round of R480 calibration
				if((R480_test_loop-(R480%R480_test_loop)) > (R480%R480_test_loop))
					R480 = R480/R480_test_loop;
				else
					R480 = 1+(R480/R480_test_loop);
			}

			if((ZQ_Config&0x10000) == 0x10000)//[16]=1,record average result
				DC1_R480[R480]++;
			if((test_mode&0x10) == 0x10) // [4]:R480 msg
				rtd_pr_zq_debug("[ZqAverage]: DC1_R480=%d\n",R480);

			//**phase set**//
			rtd_ZQ_maskl(DC1_PAD_RZCTRL_STATUS, 0xffffffC1, R480<<1); //DC1 R480[5:1]
		}//end of [2]DCU1 R480 CAl. Enable

		//**DCU1 ZQ Calibration for SET0~7**//
		//0xb803809C[5:4]=2'b01 ddr3
		//0xb803809C[5:4]=2'b10 ddr4
		//if(PLATFORM_KXL == get_platform()) {
			if((rtd_ZQ_inl(0xb803809c) & 0x30) == 0x20) {
			rtd_outl(0xb80be1f4,0x92db0000);  //  Vref_range[27]:28%~92%,ZQ_Vref[26:21]:50%
			rtd_outl(0xb80be130,0x04040c99);  //  auto update when calibration done.
			rtd_outl(0xb80be23c,0x00000003);  // Update Write Delay Tap
			rtd_pr_zq_debug("this is ddr4 package!\n");
		} else
		{
			rtd_pr_zq_debug("this is ddr3 package!\n");
		}
		
		rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xF7FFFFFF, 0x00000000);	//[27]=0,Disable Update Immediately
		rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFBFFFF, 0x00000000);	//[18]=0,Disable Update during Refresh

        for(i = 0; i < total_SET; i++) { //SET0~7
			tmp_val = (rtd_ZQ_inl(ZQFW_CTRL1)&0xFF0000)>>16;  //SET_i [23:16]

			if(tmp_val & (0x1<<i)) { //if [23:16]!=0 DC1 zq cal. SET_i is enable
                for(loop = 0; loop < ZQ_max_loop; loop++) { //scan loop
                	err_type = 0;
					rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0x8FFFFFFF, (i<<28)); // Enable auto update OCD / ODT set0~set7 [30:28]=i
					rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFEFFFFFF, 0x01000000); // [24]=1,Calibration Start
					usleep_range(180, 250);

					if((rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS)&0x80000000) != 0x80000000) //done[31]
					{
						rtd_ZQ_maskl(ZQFW_CTRL2, 0xBFFFFFFF, 1<<30);
                    	if((test_mode & 0x1) == 0x1)
							rtd_pr_zq_debug("[ZqCalDebugInfo]: DC1 zq cal. done failed!\n");
					}

					err_type = rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS) & 0x3FFFFFFF;
                    if(err_type != 0x0) //[29:0]=0
					{
                    	if((test_mode & 0x1) == 0x1)
							rtd_pr_zq_debug("[ZqCalDebugInfo]: zq cal. failed(DCU1),0x%08x!\n",err_type);
					}

                    if((test_mode & 0x1) == 0x1) { // [0]:debug msg
						rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", (DC1_PAD_CTRL_ZPROG+i*4),DC1_PAD_ZCTRL_STATUS,rtd_ZQ_inl(DC1_PAD_CTRL_ZPROG+i*4), rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS));
						rtd_pr_zq_debug(" value(0x%08x): 0x%08x", (DC1_PAD_NOCD2_ZPROG+i*4),rtd_ZQ_inl(DC1_PAD_NOCD2_ZPROG+i*4));
						rtd_pr_zq_debug(" [31:0]:zctrl_status(DCU1 SET%d)  \n",i);
					}
					rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFEFFFFFF, 0x00000000);    // [24]=0,Disable Calibration

					//**calibration result**//
                    //DC1 PMOS OCD SET0~3/4~7[5:0][13:8][21:16][29:24], NMOS SET0~7
					OCDP_Addr = DC1_OCDP_SET0+(i/4)*4;
					OCDN_Addr = DC1_OCDN_SET0+(i/4)*4;
					bit_shift = (i%4)*8;

					PMOS[i] = (rtd_ZQ_inl(OCDP_Addr)>>bit_shift) & 0x3f;
					PMOS_SET[i] += PMOS[i];
					if((test_mode&0x20) == 0x20)  // [5]:ZQ msg
						rtd_pr_zq_debug("[ZqDebug]: %d)DC1_PMOS_SET%d=%d\n",loop+1,i,PMOS[i]);
                    if(PMOS[i] > DC1_SET_max[i]) {
						DC1_SET_max[i] = PMOS[i];
						rtd_pr_zq_debug("[ZqCompare]: DC1_PMOS_SET%d_max=%d\n",i,DC1_SET_max[i]);
					}
                    if(PMOS[i] < DC1_SET_min[i]) {
						DC1_SET_min[i] = PMOS[i];
						rtd_pr_zq_debug("[ZqCompare]: DC1_PMOS_SET%d_min=%d\n",i,DC1_SET_min[i]);
					}
					//DC1 NMOS OCD SET0~7, NMOS SET0~7
					NMOS[i] = (rtd_ZQ_inl(OCDN_Addr)>>bit_shift) & 0x3f;
					NMOS_SET[i] += NMOS[i];
					if((test_mode&0x20) == 0x20)  // [5]:ZQ msg
						rtd_pr_zq_debug("[ZqDebug]: %d)DC1_NMOS_SET%d=%d\n",loop+1,i,NMOS[i]);
                    if(NMOS[i] > DC1_SET_max[i + total_SET]) {
						DC1_SET_max[i+total_SET] = NMOS[i];
						rtd_pr_zq_debug("[ZqCompare]: DC1_NMOS_SET%d_max=%d\n",i,DC1_SET_max[i+total_SET]);
					}
                    if(NMOS[i] < DC1_SET_min[i + total_SET]) {
						DC1_SET_min[i+total_SET] = NMOS[i];
						rtd_pr_zq_debug("[ZqCompare]: DC1_NMOS_SET%d_min=%d\n",i,DC1_SET_min[i+total_SET]);
					}

                    if((ZQ_Config & 0x20000) == 0x20000) { //[17]=1,record all scan results
                        if(i == 0) {
							DC1_PMOS_SET0_Total[PMOS[i]]++;
							DC1_NMOS_SET0_Total[NMOS[i]]++;
                        } else if(i == 1) {
							DC1_PMOS_SET1_Total[PMOS[i]]++;
							DC1_NMOS_SET1_Total[NMOS[i]]++;
                        } else if(i == 2) {
							DC1_PMOS_SET2_Total[PMOS[i]]++;
							DC1_NMOS_SET2_Total[NMOS[i]]++;
                        } else if(i == 3) {
							DC1_PMOS_SET3_Total[PMOS[i]]++;
							DC1_NMOS_SET3_Total[NMOS[i]]++;
						} else if(i == 4) {
							DC1_PMOS_SET4_Total[PMOS[i]]++;
							DC1_NMOS_SET4_Total[NMOS[i]]++;
						} else if(i == 5) {
							DC1_PMOS_SET5_Total[PMOS[i]]++;
							DC1_NMOS_SET5_Total[NMOS[i]]++;
						} else if(i == 6) {
							DC1_PMOS_SET6_Total[PMOS[i]]++;
							DC1_NMOS_SET6_Total[NMOS[i]]++;
						} else if(i == 7) {
							DC1_PMOS_SET7_Total[PMOS[i]]++;
							DC1_NMOS_SET7_Total[NMOS[i]]++;
						}
					}
				}//scan loop(s)

                if(ZQ_max_loop == OCD_test_loop) { //multi-round of ZQ SET0~3 calibration
					if((OCD_test_loop-(PMOS_SET[i]%OCD_test_loop)) > (PMOS_SET[i]%OCD_test_loop))
						PMOS_SET[i] = PMOS_SET[i]/OCD_test_loop;
					else
						PMOS_SET[i] = 1+(PMOS_SET[i]/OCD_test_loop);

					if((OCD_test_loop-(NMOS_SET[i]%OCD_test_loop)) > (NMOS_SET[i]%OCD_test_loop))
						NMOS_SET[i] = NMOS_SET[i]/OCD_test_loop;
					else
						NMOS_SET[i] = 1+(NMOS_SET[i]/OCD_test_loop);
				}//[2]=1, multi-round of ZQ SET0~3 calibration

                if((ZQ_Config & 0x10000) == 0x10000) { //[16]=1,record average result
                    if(i == 0) {
						DC1_PMOS_SET0[PMOS_SET[i]]++;
						DC1_NMOS_SET0[NMOS_SET[i]]++;
                    } else if(i == 1) {
						DC1_PMOS_SET1[PMOS_SET[i]]++;
						DC1_NMOS_SET1[NMOS_SET[i]]++;
                    } else if(i == 2) {
						DC1_PMOS_SET2[PMOS_SET[i]]++;
						DC1_NMOS_SET2[NMOS_SET[i]]++;
                    } else if(i == 3) {
						DC1_PMOS_SET3[PMOS_SET[i]]++;
						DC1_NMOS_SET3[NMOS_SET[i]]++;
                    } else if(i == 4) {
						DC1_PMOS_SET4[PMOS_SET[i]]++;
						DC1_NMOS_SET4[NMOS_SET[i]]++;
                    } else if(i == 5) {
						DC1_PMOS_SET5[PMOS_SET[i]]++;
						DC1_NMOS_SET5[NMOS_SET[i]]++;
                    } else if(i == 6) {
						DC1_PMOS_SET6[PMOS_SET[i]]++;
						DC1_NMOS_SET6[NMOS_SET[i]]++;
					} else if(i == 7) {
						DC1_PMOS_SET7[PMOS_SET[i]]++;
						DC1_NMOS_SET7[NMOS_SET[i]]++;
					}
				}
                if((test_mode & 0x20) == 0x20) { // [5]:ZQ msg
					rtd_pr_zq_debug("[ZqAverage]: DC1_PMOS_SET%d=%d\n",i,PMOS_SET[i]);
					rtd_pr_zq_debug("[ZqAverage]: DC1_NMOS_SET%d=%d\n",i,NMOS_SET[i]);
				}

				//**phase set**//
				rtd_ZQ_maskl(OCDP_Addr, ~(0x3f<<bit_shift), (PMOS_SET[i]<<bit_shift)); //DC1 PMOS OCD SET0[5:0][13:8][21:16][29:24]
				rtd_ZQ_maskl(OCDN_Addr, ~(0x3f<<bit_shift), (NMOS_SET[i]<<bit_shift)); //DC1 NMOS OCD SET0[5:0]
			}//if zq cal. SET_i is enable
		}//SET0~7

		if((test_mode&0x20) == 0x20) {  // [5]:ZQ msg
			rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET0, DC1_OCDN_SET0,rtd_ZQ_inl(DC1_OCDP_SET0), rtd_ZQ_inl(DC1_OCDN_SET0));
			rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET0+0x4, DC1_OCDN_SET0+0x4,rtd_ZQ_inl(DC1_OCDP_SET0+0x4), rtd_ZQ_inl(DC1_OCDN_SET0+0x4));
		}

		rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFBFFFF, 0x00040000);	// [18]=1,Enable Update during Refresh
		if((test_mode&0x20) == 0x20) {  // [5]:ZQ msg
			rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET1, DC1_OCDN_SET1,rtd_ZQ_inl(DC1_OCDP_SET1), rtd_ZQ_inl(DC1_OCDN_SET1));
			rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET1+0x4, DC1_OCDN_SET1+0x4,rtd_ZQ_inl(DC1_OCDP_SET1+0x4), rtd_ZQ_inl(DC1_OCDN_SET1+0x4));
		}

		//log print
		if((ZQ_Config&0x18) != 0) //[4:3]log
			counter++;
        if(counter == print_flag) {
			//DC1
			if((DC1_R480_err_msg[0] > 0)&&((test_mode&0x10) == 0x10)) //[4]:R480 msg
				rtd_pr_zq_debug("[ZqErrorMsg] DC1_R480 error cnt=%d, last value=%d\n",DC1_R480_err_msg[0],DC1_R480_err_msg[1]);

            if((ZQ_Config & 0x8) == 0x8) { //[3]=1,averaged calibration results
				rtd_pr_zq_debug("\n[ZqSummary]: DC1_R480\n");
                for(i = 0; i < R480_INDEX; i++) {
					if(loop_delay > 0)
						usleep_range(loop_delay, loop_delay+10);
					rtd_pr_zq_debug("%d,%d;\n", i, DC1_R480[i]);
				}
				if(para_delay > 0)
					usleep_range(para_delay, para_delay+10);

				for(loop=0; loop<total_SET; loop++) //SET0~7
				{
					rtd_pr_zq_debug("\n[ZqSummary]: DC1_PMOS_SET%d\n",loop);
                	for(i = 0; i < SET_INDEX; i++) {
						if(loop_delay > 0)
							usleep_range(loop_delay, loop_delay+10);
						if(loop == 0)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET0[i]);
						else if(loop == 1)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET1[i]);
						else if(loop == 2)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET2[i]);
						else if(loop == 3)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET3[i]);
						else if(loop == 4)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET4[i]);
						else if(loop == 5)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET5[i]);
						else if(loop == 6)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET6[i]);
						else if(loop == 7)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET7[i]);
					}
					if(para_delay > 0)
						usleep_range(para_delay, para_delay+10);

					rtd_pr_zq_debug("\n[ZqSummary]: DC1_NMOS_SET%d\n", loop);
                	for(i = 0; i < SET_INDEX; i++) {
						if(loop_delay > 0)
							usleep_range(loop_delay, loop_delay+10);
						if(loop == 0)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET0[i]);
						else if(loop == 1)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET1[i]);
						else if(loop == 2)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET2[i]);
						else if(loop == 3)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET3[i]);
						else if(loop == 4)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET4[i]);
						else if(loop == 5)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET5[i]);
						else if(loop == 6)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET6[i]);
						else if(loop == 7)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET7[i]);
    				}
					if(para_delay > 0)
						usleep_range(para_delay, para_delay+10);
				} //set0~7
			} //averaged result

			//Total count result, DC1
            if((ZQ_Config & 0x10) == 0x10) { //[4]=1,record all scan results
				rtd_pr_zq_debug("\n[ZqTotalCnt]: DC1_R480_Total\n");
                for(i = 0; i < R480_INDEX; i++) {
					if(loop_delay > 0)
						usleep_range(loop_delay, loop_delay+10);
					rtd_pr_zq_debug("%d,%d;\n", i, DC1_R480_Total[i]);
				}
				if(para_delay > 0)
					usleep_range(para_delay, para_delay+10);

				for(loop=0; loop<total_SET; loop++)
				{
					rtd_pr_zq_debug("\n[ZqTotalCnt]: DC1_PMOS_SET%d_Total\n",loop);
                	for(i = 0; i < SET_INDEX; i++) {
						if(loop_delay > 0)
							usleep_range(loop_delay, loop_delay+10);
						if(loop == 0)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET0_Total[i]);
						else if(loop == 1)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET1_Total[i]);
						else if(loop == 2)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET2_Total[i]);
						else if(loop == 3)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET3_Total[i]);
						else if(loop == 4)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET4_Total[i]);
						else if(loop == 5)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET5_Total[i]);
						else if(loop == 6)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET6_Total[i]);
						else if(loop == 7)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_PMOS_SET7_Total[i]);
					}
					if(para_delay > 0)
						usleep_range(para_delay, para_delay+10);

					rtd_pr_zq_debug("\n[ZqTotalCnt]: DC1_NMOS_SET%d_Total\n",loop);
                	for(i = 0; i < SET_INDEX; i++) {
						if(loop_delay > 0)
							usleep_range(loop_delay, loop_delay+10);
						if(loop == 0)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET0_Total[i]);
						else if(loop == 1)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET1_Total[i]);
						else if(loop == 2)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET2_Total[i]);
						else if(loop == 3)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET3_Total[i]);
						else if(loop == 4)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET4_Total[i]);
						else if(loop == 5)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET5_Total[i]);
						else if(loop == 6)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET6_Total[i]);
						else if(loop == 7)
							rtd_pr_zq_debug("%d,%d;\n", i, DC1_NMOS_SET7_Total[i]);
					}
					if(para_delay > 0)
						usleep_range(para_delay, para_delay+10);
				}
			} //edn of record all scan results

            if((ZQ_Config & 0x20) == 0x20) { //[5]=1, max and min for each set
                for(i = 0; i < total_SET; i++) { //SET0~7
					rtd_pr_zq_debug("[ZqCompare]: DC1_PMOS_SET%d_max=%d\n",i,DC1_SET_max[i]);
					rtd_pr_zq_debug("[ZqCompare]: DC1_PMOS_SET%d_min=%d\n",i,DC1_SET_min[i]);
				}
				if(para_delay > 0)
					usleep_range(para_delay, para_delay+10);
			}

			counter = 0;

		}// print result
    } //end of Main enable

	rtd_ZQ_maskl(ZQFW_CTRL1, 0xffffdfff, 0x2000); //[13]=1 zq fw finish
	rtd_pr_zq_debug("KGD ZQCS Calibration!\n");
	rtd_ZQ_maskl(DC1_DDR3_CTL, 0xffffe3ff, 0x1c00); //issue KGD ZQCS Calibration
}

static int rtk_monitor_thread(void *arg)
{
	struct sched_param param = { .sched_priority = 1 };
//	unsigned int diff;

	int reg_value;
	int monitor_value=0;
	int monitor_tmp=0;
	int monitor_diff, flag=0;
	int onetime_flag=0;

	//rtk_gpio_set_dir(rtk_gpio_id(MIS_GPIO, 59), 0);

	sched_setscheduler_nocheck(current, SCHED_FIFO, &param);

	pre_jiffies = jiffies; // initial

#ifdef CONFIG_RTK_ZQ_ENABLE
	rtd_ZQ_maskl(ZQFW_CTRL1, 0xfffff0ff, 0x100); //[11:8]zq fw version 1
	counter = 0;
	Array_init(DC1_R480_err_msg, 2, 0);
	Array_init(DC1_SET_max, 16, 0);
	Array_init(DC1_SET_min, 16, 100);

    Array_init(DC1_R480, R480_INDEX, 0);
    Array_init(DC1_PMOS_SET0, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET1, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET2, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET3, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET0, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET1, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET2, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET3, SET_INDEX, 0);

    Array_init(DC1_R480_Total, R480_INDEX, 0);
    Array_init(DC1_PMOS_SET0_Total, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET1_Total, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET2_Total, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET3_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET0_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET1_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET2_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET3_Total, SET_INDEX, 0);

	Get_Ori_ZQ_Phase(0); //get original phase
#endif

	do {
		set_freezable();
#if 0 //test
		diff = jiffies_to_msecs(jiffies - pre_jiffies);
		pre_jiffies = jiffies;

		rtd_pr_zq_info("diff=%d\n", diff);
#endif

		reg_value=rtk_get_thermal_reg();
		monitor_value = reg_value & 0x7FFFF;
        if((monitor_value & 0x40000) == 0x40000) { //if [18]=1,negative value
        	monitor_value = monitor_value - 1;
	    	monitor_value = (~monitor_value) & 0x0003FFFF;
        	monitor_value = -monitor_value;
   		}
		monitor_value = (1000*monitor_value)/1024;
/*adding the highest temperatue value*/
		if((monitor_value%1000) < 0)
		{
			rtd_pr_thermal_debug("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, -(monitor_value%1000),(rtd_inl(0xb8060134)&0x00FF0000)>>16,reg_value);
			if(rtk_monitor_debug) //message out with low level by debug flag
                               rtd_pr_zq_crit("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, -(monitor_value%1000),(rtd_inl(0xb8060134)&0x00FF0000)>>16,reg_value);
		}
		else
		{
			rtd_pr_thermal_debug("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, monitor_value%1000,(rtd_inl(0xb8060134)&0x00FF0000)>>16,reg_value);
			if(rtk_monitor_debug) //message out with low level by debug flag
                               rtd_pr_zq_crit("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, (monitor_value%1000),(rtd_inl(0xb8060134)&0x00FF0000)>>16,reg_value);
		}

		if(monitor_value/1000 > 125)
			rtd_pr_thermal_crit("[Rtk Monitor]:%d reg:%x\n",monitor_value/1000,reg_value);

		{
			int previous_value = (rtd_inl(STB_WDOG_DATA14_reg) & (0x00ff0000)) >> 16;
			int val = monitor_value/1000;
            if((val > 0) && (val < 255)) {
				//keep the highest monitor value
				if(val > previous_value)
					rtd_maskl(STB_WDOG_DATA14_reg,0xFF00FFFF, (val)<<16);
            } else { //invalid value
				rtd_maskl(STB_WDOG_DATA14_reg,0xFF00FFFF, (0xff)<<16);
			}
		}

		if(ZQ_init_flag == 0)
		{
			onetime_flag = 0;
			flag = 0;
			DC1_R480_max = 0;
			DC1_R480_min = 100;
			ZQ_init_flag = 1;
		}

#ifdef CONFIG_RTK_ZQ_ENABLE
		test_mode = rtd_ZQ_inl(ZQFW_CTRL1); //[7]:auto OCD; [6]:ZQ per sec; [5]:ZQ msg; [4]:R480 msg; [3:2]:DCU2/DCU1 R480 Cal Enable; [1]:Main Enable; [0]:debug msg
		ZQ_Config = rtd_ZQ_inl(ZQFW_CTRL2);

		if((ZQ_Config&0x40) == 0x40) //[6]=1
			Get_Ori_ZQ_Phase(1); //log print

        if(((ZQ_Config & 0x1) == 0x1) && (onetime_flag == 0)) { //[0]=1,one time calibration
			Kernal_ZqCalibration();
			onetime_flag = 1;
        } else {
            if((test_mode & 0x00000040) == 0x00000000) { //[6]=0 update by monitor_diff > 10 or <-10
				if(flag == 0)
					monitor_tmp = monitor_value;
				monitor_diff = (monitor_tmp - monitor_value)/1000;
                if((flag == 0) || (monitor_diff > 10) || (monitor_diff < -10)) {
					Kernal_ZqCalibration(); //DDR zq cal.
					monitor_tmp = monitor_value;
					flag = 1;
				}
            } else //[6]=1, update zq by second
				Kernal_ZqCalibration();
		}
#endif
#ifdef CONFIG_FIXED_ME_443
#ifdef VBE_DISP_TCON_SELF_CHECK
		if((!(IoReg_Read32(MDOMAIN_DISP_dispm_comp_decomp_dbg_mux_reg)&_BIT31))&&(monitor_rgb_rolling==0)) {
			vbe_disp_always_check_tcon();//every 1second to check
		}
#endif
#endif
		msleep(rtk_monitor_delay);

	} while (!kthread_should_stop());

	return 0;
}

static struct task_struct * rtk_monitor_thread_create(void)
{
	struct task_struct *task = NULL;
	int err;

	task = kthread_create(rtk_monitor_thread, NULL, "rtk_monitor_thread");
	if (IS_ERR(task)) {
		err = PTR_ERR(task);
		rtd_pr_zq_err("%s failed (%d)\n", __func__, err);
		task = NULL;
		return task;
	}

	wake_up_process(task);
	return task;
}

static int rtk_monitor_enable(void)
{
	int ret = 0;

	mutex_lock(&rtk_monitor_activation_lock);

	if (rtk_monitor_active) {
		mutex_unlock(&rtk_monitor_activation_lock);
		return 0;
	}

	rtk_monitor_task = rtk_monitor_thread_create();

	if (rtk_monitor_task == NULL) {
		ret = -1;
		goto out;
	}

	rtk_monitor_active = 1;

out:
	mutex_unlock(&rtk_monitor_activation_lock);

	if (rtk_monitor_task != NULL)
		rtd_pr_zq_info("rtk monitor thread created\n");
	else
		rtd_pr_zq_err("rtk monitor thread NOT created\n");

	return ret;
}

static void rtk_monitor_disable(void)
{
	mutex_lock(&rtk_monitor_activation_lock);

	if (!rtk_monitor_active)
		goto out;

	kthread_stop(rtk_monitor_task);

	rtk_monitor_active = 0;

out:
	mutex_unlock(&rtk_monitor_activation_lock);
}

ssize_t rtk_monitor_active_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_monitor_active);
}

ssize_t rtk_monitor_active_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;

	switch (buf[0]) {
	case '0':
		rtk_monitor_disable();
		ret = 0;
		break;
	case '1':
		ret = rtk_monitor_enable();
		break;
	default:
		ret = -EINVAL;
	}

	return (ret >= 0) ? count : ret;
}

ssize_t rtk_monitor_delay_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", rtk_monitor_delay);
}

ssize_t rtk_monitor_delay_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	int ret;

	mutex_lock(&rtk_monitor_delay_lock);
	ret = sscanf(buf, "%u", &rtk_monitor_delay);
	if (ret != 1) {
		mutex_unlock(&rtk_monitor_delay_lock);
		return -EINVAL;
	}

//out:
	mutex_unlock(&rtk_monitor_delay_lock);
	return count;
}

ssize_t rtk_monitor_debug_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
       return sprintf(buf, "%u\n", rtk_monitor_debug);
}

ssize_t rtk_monitor_debug_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
       int ret;

       mutex_lock(&rtk_monitor_delay_lock);
       ret = sscanf(buf, "%u", &rtk_monitor_debug);
       if (ret != 1) {
               mutex_unlock(&rtk_monitor_delay_lock);
               return -EINVAL;
       }

//out:
       mutex_unlock(&rtk_monitor_delay_lock);
      return count;
}

#if 0 // move to sysfs.c
static struct kobj_attribute rtk_monitor_active_attr =
	__ATTR(active, 0644, rtk_monitor_active_show, rtk_monitor_active_store);

static struct kobj_attribute rtk_monitor_delay_attr =
	__ATTR(delay, 0644, rtk_monitor_delay_show, rtk_monitor_delay_store);

//static struct attribute *rtk_monitor_attrs[] = {
//	&rtk_monitor_active_attr.attr,
//	&rtk_monitor_delay_attr.attr,
//	NULL,
//};

//static struct attribute_group rtk_monitor_attr_group = {
//	.attrs = rtk_monitor_attrs,
//};

static struct kobject *rtk_monitor_kobj;
extern struct kobject *realtek_boards_kobj;

static int __init rtk_monitor_sysfs_init(void)
{
	int ret;

//	rtk_monitor_kobj = kobject_create_and_add("rtk_monitor", kernel_kobj);
//	if (!rtk_monitor_kobj)
//		return -ENOMEM;
//	ret = sysfs_create_group(rtk_monitor_kobj, &rtk_monitor_attr_group);
//	if (ret)
//		kobject_put(rtk_monitor_kobj);

	if (!realtek_boards_kobj)
		return -ENOMEM;

	rtk_monitor_kobj = kobject_create();
	ret = kobject_set_name(rtk_monitor_kobj, "rtk_monitor");
	if (ret)
		goto out;
	ret = kobject_add(rtk_monitor_kobj, realtek_boards_kobj, "rtk_monitor");
	if (ret)
		goto out;

	ret = sysfs_create_file(rtk_monitor_kobj, &rtk_monitor_active_attr.attr);
	ret |= sysfs_create_file(rtk_monitor_kobj, &rtk_monitor_delay_attr.attr);
	if (ret)
		kobject_put(rtk_monitor_kobj);

out:
	return ret;
}
#endif

static int __init rtk_monitor_init(void)
{
	int ret;

	init_rtk_thermal_sensor();

	ret = rtk_monitor_enable();
	if (ret)
		return ret;

#if 0 // move to sysfs.c
	ret = rtk_monitor_sysfs_init();
	if (ret)
		rtd_pr_zq_err("%s: unable to create sysfs entry\n", __func__);
#endif

	return 0;
}


#ifdef CONFIG_OF
static int rtk_monitor_probe(struct platform_device *pdev)
{
	rtk_monitor_init();
	return 0;
}

static int rtk_monitor_remove(struct platform_device *pdev)
{
	return 0;
}

static int rtk_monitor_suspend(struct device *dev)
{

	return 0;
}

static int rtk_monitor_resume(struct device *dev)
{
	ZQ_init_flag = 0;

	return 0;
}

static const struct dev_pm_ops rtk_monitor_ops = {
	.suspend    = rtk_monitor_suspend,
	.resume     = rtk_monitor_resume,
#ifdef CONFIG_HIBERNATION
	.freeze     = rtk_monitor_suspend,
	.thaw	    = rtk_monitor_resume,
	.poweroff   = rtk_monitor_suspend,
	.restore    = rtk_monitor_resume,
#endif
};

static const struct of_device_id rtk_monitor_match[] = {
	{
	 .compatible = "rtk_monitor",
	 .data = (void *)&rtk_monitor_ops,
	 },
	{},
};

static struct platform_device_id rtk_monitor_ids[] = {
#if defined(CONFIG_ARCH_RTK289X)
	{
		 .name = "rtk-monitor",
		 .driver_data = (kernel_ulong_t) NULL,
	 },
#endif
	{},
};

MODULE_DEVICE_TABLE(of, rtk_monitor_match);

static struct platform_driver rtk_monitor_driver = {
	.driver = {
		   .name = "rtk-monitor",
		   .owner = THIS_MODULE,
#ifdef CONFIG_PM
		   .pm = &rtk_monitor_ops,
#endif
		   .of_match_table = of_match_ptr(rtk_monitor_match),
		   },
	.probe = rtk_monitor_probe,
	.remove = rtk_monitor_remove,
	.id_table = rtk_monitor_ids,
};

module_platform_driver(rtk_monitor_driver);

#else
late_initcall(rtk_monitor_init);
#endif
