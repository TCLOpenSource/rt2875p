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
#include <linux/sched/clock.h>
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
#define RTK_MONITOR_THERMAL_TEMP_PRINT_THRESHOLD 120

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>

#ifdef CONFIG_ARCH_RTK6748
#define CONFIG_RTK_ZQ_ENABLE
#endif

static unsigned int	ZQ_init_flag = 0;
static struct task_struct *rtk_monitor_task;
static unsigned long pre_jiffies;

#define R480_INDEX 32
#define SET_INDEX 64
#define TOTAL_SET 8

#define DC1_OCDP_SET0 0xb80c9190 //90~94
#define DC1_OCDN_SET0 0xb80c9198 //98~9c
#define DC1_OCDP_SET1 0xb80c91B0 //b0~b4
#define DC1_OCDN_SET1 0xb80c91B8 //b8~bc
#define DC1_PAD_RZCTRL_STATUS 0xb80c914C

#define DC1_PAD_CTRL_PROG 0xb80c9130
#define DC1_PAD_ZCTRL_STATUS 0xb80c9140
#define DC1_PAD_CTRL_ZPROG 0xb80c9314 //set:8
#define DC1_PAD_NOCD2_ZPROG 0xb80c9334 //set:8

//dummy reg.
#define ZQFW_CTRL1 0xb80c2750
#define ZQFW_CTRL2 0xb80c2754
//2MC ref sync
#define MC1_SYS_REF_1 0xb80c2064
#define MC2_SYS_REF_1 0xb80c3064

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
unsigned long long now, start;
unsigned int zq_set_addr, zq_set_msb, zq_set_lsb;

#ifdef CONFIG_RTK_ZQ_ENABLE
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
        Val = (Val & mask_bits) | value;
        rtd_outl(REG, Val);
}

static void Array_init(unsigned int Array[], unsigned int element, unsigned int value)
{
        unsigned int i;

        for(i = 0; i < element; i++)
                Array[i] = value;
}

static void Get_Ori_ZQ_Phase(unsigned int flag)
{
        unsigned int Reg[5] = {DC1_OCDP_SET0, (DC1_OCDP_SET0 + 0x4), DC1_OCDN_SET0, (DC1_OCDN_SET0 + 0x4), DC1_PAD_RZCTRL_STATUS};
        unsigned int element = hwsetting_idx;
        unsigned int sts_index = 4;
        unsigned int i;

        if(flag == 0)
        {
                Array_init(hwsetting_result, element, 0);

                for(i = 0; i < element; i++)
                {
                        if(i < sts_index)
                                hwsetting_result[i] = rtd_ZQ_inl(Reg[i]);
                        else
                                hwsetting_result[i] = (rtd_ZQ_inl(Reg[i]) & 0x3E) >> 1; //[5:1]
                }
        }
        else if(flag == 1)
        {
                for(i = 0; i < element; i++)
                        rtd_pr_zq_debug("[ZqHwsetting]0x%08x=0x%x", Reg[i], hwsetting_result[i]);
        }
}

void rtd_copy(unsigned int addr_src, unsigned int src_end, unsigned int src_start, unsigned int addr_dst, unsigned int dst_end, unsigned int dst_start)
{
    unsigned int src_reg_val, dst_reg_val;
    src_reg_val = rtd_inl(addr_src);
    src_reg_val = src_reg_val >> src_start;
    src_reg_val = src_reg_val & ((1UL << (src_end - src_start + 1)) - 1);
   
    dst_reg_val = rtd_inl(addr_dst);
    dst_reg_val = dst_reg_val & ~(((1UL << (dst_end - dst_start + 1)) - 1) << dst_start);
    src_reg_val = (src_reg_val & ((1UL << (dst_end - dst_start + 1)) - 1)) << dst_start;
    dst_reg_val = dst_reg_val | src_reg_val;
    rtd_outl(addr_dst, dst_reg_val);   
}

void zq_odt_set_sel(unsigned int zq_set_type, unsigned int zq_set_num)
{
    switch (zq_set_type) { 
        case 0:  //PODT
            if(zq_set_num < 4){
                zq_set_addr = 0xb80c9180;
            }else{
                zq_set_addr = 0xb80c9184;
            }
            break;
            
        case 1: //NODT
            if(zq_set_num < 4){
                zq_set_addr = 0xb80c9188;
            }else{
                zq_set_addr = 0xb80c918c;
            }
            break;    
        default: 
            if(zq_set_num < 4){
                zq_set_addr = 0xb80c9180;
            }else{
                zq_set_addr = 0xb80c9184;
            }         
    }         
    switch (zq_set_num % 4) { 
        case 0: 
            zq_set_msb = 5;
            zq_set_lsb = 0;
            break; 

        case 1: 
            zq_set_msb = 13;
            zq_set_lsb = 8;
            break; 

        case 2: 
            zq_set_msb = 21;
            zq_set_lsb = 16;
            break; 

        case 3: 
	default:
            zq_set_msb = 29;
            zq_set_lsb = 24;
    }     
}

void zq_ocd_set_sel(unsigned int zq_set_type, unsigned int zq_set_num)
{
    switch (zq_set_type) { 
        case 0:  //PODT
            if(zq_set_num < 4){
                zq_set_addr = 0xb80c9190;
            }else{
                zq_set_addr = 0xb80c9194;
            }
            break;
            
        case 1: //NODT
            if(zq_set_num < 4){
                zq_set_addr = 0xb80c9198;
            }else{
                zq_set_addr = 0xb80c919c;
            }
            break;    
        default: 
            if(zq_set_num < 4){
                zq_set_addr = 0xb80c9190;
            }else{
                zq_set_addr = 0xb80c9194;
            }
            break;  
    }         
    switch (zq_set_num % 4) { 
        case 0: 
            zq_set_msb = 5;
            zq_set_lsb = 0;
            break; 

        case 1: 
            zq_set_msb = 13;
            zq_set_lsb = 8;
            break; 

        case 2: 
            zq_set_msb = 21;
            zq_set_lsb = 16;
            break; 

        case 3: 
	default:
            zq_set_msb = 29;
            zq_set_lsb = 24;
    }     
}



static void Kernal_ZqCalibration(void)
{
        int tmp_val = 0, print_flag = 10;
        unsigned int total_SET = TOTAL_SET; //SET0~7
        unsigned int i, loop, bit_shift;
        unsigned int R480_test_loop = 5, OCD_test_loop = 5;
        unsigned int R480_max_loop, ZQ_max_loop;
        unsigned int r480_loops = 0, ocd_loops = 0;
        unsigned int r480_fail = 0, ocd_fail = 0;
        unsigned int PMOS_SET[TOTAL_SET], NMOS_SET[TOTAL_SET];
        unsigned int R480, PMOS[TOTAL_SET], NMOS[TOTAL_SET];
        unsigned int loop_delay, para_delay;
        unsigned int err_type = 0;
        unsigned int OCDP_Addr, OCDN_Addr;
        int retry_timeout;


    // /* 2MC mode need to check skip this iteration if  SOC update not de-asserted. */
    if((rtd_ZQ_inl(0xb80c2028) & 0x1) == 0x1)
    {  
        if ((rtd_ZQ_inl(MC1_SYS_REF_1) & 0x00100000) || (rtd_ZQ_inl(MC2_SYS_REF_1) & 0x00100000)) //bit20
        {
            rtd_pr_zq_info("ZQ_WARN: %s starting, but MC1_SYS_REF_1 (0x%x) bit 20 asserted. return. \n",
                   __func__, rtd_ZQ_inl(MC1_SYS_REF_1));
            rtd_pr_zq_info("ZQ_WARN: %s starting, but MC2_SYS_REF_1 (0x%x) bit 20 asserted. return. \n",
                   __func__, rtd_ZQ_inl(MC2_SYS_REF_1));
            return;
        }
    }
    
    rtd_ZQ_maskl(ZQFW_CTRL1, 0xffffefff, 0x1000); //[12]=1 zq fw start

    if((test_mode & 0x2) == 0x2)   //[1]Main Enable
    {
            Array_init(PMOS_SET, total_SET, 0);
            Array_init(NMOS_SET, total_SET, 0);
            Array_init(PMOS, total_SET, 0);
            Array_init(NMOS, total_SET, 0);
            R480 = 0;

            loop_delay = ((rtd_ZQ_inl(ZQFW_CTRL2) & 0xf00) >> 8) * 100; //[11:8]
            para_delay = ((rtd_ZQ_inl(ZQFW_CTRL2) & 0xf000) >> 12) * 100; //[15:12]

            if((ZQ_Config & 0x2) == 0x2) //[1]=1,R480 one time calibration
                    R480_max_loop = 1;
            else
                    R480_max_loop = R480_test_loop;

            if((ZQ_Config & 0x4) == 0x4) //[2]=1,SET0~3 one time calibration
                    ZQ_max_loop = 1;
            else
                    ZQ_max_loop = OCD_test_loop;

            //**DCU1 R480 Cal.**//
            if((test_mode & 0x4) == 0x4)   // [2]DCU1 R480 CAl. Enable
            {
                    //if(PLATFORM_KXL == get_platform()) {
                    if((rtd_ZQ_inl(0xb803809c) & 0x30) == 0x20)
                    {
                            rtd_maskl(0xb80c9238, 0xfffffffc, 0x00000002);   //fw_set[1:0]=immediately
                            rtd_maskl(0xb80ca238, 0xfffffffc, 0x00000002);   //fw_set[1:0]=immediately
                            rtd_outl(0xb80c91f4, 0x9fdb0000); //Vref_range[27]:0.5%~64.5%,ZQ_Vref[26:21]:62.5%
                            rtd_outl(0xb80c913c, 0x00000000); //zq_ena_nocd2 = 0
                            rtd_outl(0xb80c9130, 0x04040003); //R480_avgcal[31],zctrl_clk_sel[26:25]=zclk/32,rzq_ext_R240[19]
                            rtd_outl(0xb80c923c, 0x00000003); //Update Write Delay Tap
                            udelay(5);
                            rtd_maskl(0xb80c9238, 0xfffffffc, 0x00000000);   //fw_set[1:0]=read:during no read,write:during refresh
                            rtd_maskl(0xb80ca238, 0xfffffffc, 0x00000000);   //fw_set[1:0]=read:during no read,write:during refresh
                            rtd_pr_zq_debug("   ******* DDR4 R480 Calibration *******    \n");
                            rtd_pr_zq_debug("0xb80c91f4 = 0x%08x\n", rtd_inl(0xb80c91f4));
                    }
                    else
                    {
                            rtd_pr_zq_debug("   ******* DDR3 R480 Calibration *******    \n");
                            rtd_pr_zq_debug("0xb80c91f4 = 0x%08x\n", rtd_inl(0xb80c91f4));
                    }

                    for(r480_loops = 0, loop = 0; loop < R480_max_loop; loop++)   //scan loop
                    {
                            rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFEFFFF, 0x00010000); // [16]=1,R480 calibration enable
                            retry_timeout = (250 * 4); // (250 * 4) us

                            while (retry_timeout > 0)
                            {
                                    retry_timeout -= 5;
                                    udelay(5);
                                    smp_mb();

                                    if ((rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS) & 0x1) == 0x1)
                                    {
                                            break;
                                    }
                            }

                            if((test_mode & 0x10) == 0x10)   // [4]:R480 msg
                            {
                                    if((rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS) & 0x1) != 0x1)
                                            rtd_pr_zq_debug("[ZqCalDebugInfo]: R480 cal. done failed(DCU1)!\n");

                                    rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x): 0x%08x ", DC1_PAD_RZCTRL_STATUS, rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS));
                                    rtd_pr_zq_debug("[5:1]:rzq_480code(DCU1)    \n");
                            }

                            /* if done bit is not asserted, then ignore this iteration. */
                            if((rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS) & 0x1) != 0x1)
                            {
                                    rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFEFFFF, 0x00000000); // [16]=0,R480 calibration Disable
                                    rtd_pr_zq_info("ZQ_WARN: R480 ZQ CAL DONE BIT=0x%08x\n", rtd_inl(DC1_PAD_RZCTRL_STATUS));
                                    continue;
                            }

                            rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFEFFFF, 0x00000000); // [16]=0,R480 calibration Disable
                            r480_loops++;

                            //**calibration result**//
                            tmp_val = (rtd_ZQ_inl(DC1_PAD_RZCTRL_STATUS) & 0x3E) >> 1; //[5:1]
                            R480 += tmp_val;

                            if((ZQ_Config & 0x20000) == 0x20000) //[17]=1,record all scan results
                                    DC1_R480_Total[tmp_val]++;

                            if((test_mode & 0x10) == 0x10) // [4]:R480 msg
                                    rtd_pr_zq_debug("[ZqDebug]: %d)DC1_R480=%d\n", loop + 1, tmp_val);

                            if((tmp_val < 0) || (tmp_val > 31))
                            {
                                    DC1_R480_err_msg[0] += 1; //err counter
                                    DC1_R480_err_msg[1] = tmp_val;

                                    if((test_mode & 0x10) == 0x10) // [4]:R480 msg
                                            rtd_pr_zq_info("[ZqErrorMsg] ERROR!!DC1_R480=%d\n", tmp_val);
                            }

                            if(tmp_val > DC1_R480_max)
                            {
                                    DC1_R480_max = tmp_val;
                                    rtd_pr_zq_info("ZQ_WARN: [ZqCompare]: DC1_R480_max=%d\n", DC1_R480_max);
                            }

                            if(tmp_val < DC1_R480_min)
                            {
                                    DC1_R480_min = tmp_val;
                                    rtd_pr_zq_info("ZQ_WARN: [ZqCompare]: DC1_R480_min=%d\n", DC1_R480_min);
                            }
                    }//scan loop(s)

                    if(r480_loops > 1 && (R480_max_loop == R480_test_loop))   //multi-round of R480 calibration
                    {
                            if((r480_loops - (R480 % r480_loops)) > (R480 % r480_loops))
                                    R480 = R480 / r480_loops;
                            else
                                    R480 = 1 + (R480 / r480_loops);
                    }

                    if((ZQ_Config & 0x10000) == 0x10000) //[16]=1,record average result
                            DC1_R480[R480]++;

                    if((test_mode & 0x10) == 0x10) // [4]:R480 msg
                            rtd_pr_zq_debug("[ZqAverage]: DC1_R480=%d\n", R480);

                    //**phase set**//
                    // rtd_pr_zq_debug("update R480 calibration data!\n");
                    if (r480_loops > 0)
                            rtd_ZQ_maskl(DC1_PAD_RZCTRL_STATUS, 0xffffffC1, R480 << 1); //DC1 R480[5:1]
                    else
                    {
                            r480_fail = 1;
                    }
            }//end of [2]DCU1 R480 CAl. Enable
            
            /* if any failure of calibration, we ignore all synch-up */
            if ((r480_fail != 0) || (ocd_fail != 0))
            {
                    rtd_pr_zq_info("ZQ_WARN: r480_fail(%d) ocd_fail(%d), thus ignore the sync process \n", r480_fail, ocd_fail);
                    return;
            }            
            
            //**DCU1 ZQ Calibration for SET0~7**//
            //if(PLATFORM_KXL == get_platform()) {
            if((rtd_ZQ_inl(0xb803809c) & 0x00000030) == 0x20)
            {
                    rtd_maskl(0xb80c9238, 0xfffffffc, 0x00000002);   //fw_set[1:0]=immediately
                    rtd_maskl(0xb80ca238, 0xfffffffc, 0x00000002);   //fw_set[1:0]=immediately
                    rtd_outl(0xb80c91f4, 0x92db0000); //  Vref_range[27]:28%~92%,ZQ_Vref[26:21]:50%
                    //rtd_outl(0xb80be130, 0x04040c99); //  auto update when calibration done.
                    rtd_outl(0xb80c9130,0x0c080c99); //dzq_auto_up[27]=1:auto update when calibration done,rzq_ext_R240[19]=1 external,zq_cal_cmd[7]=1
                    rtd_outl(0xb80c923c, 0x00000003); // Update Write Delay Tap
                    udelay(5);
                    rtd_maskl(0xb80c9238, 0xfffffffc, 0x00000000);   //fw_set[1:0]=read:during no read,write:during refresh
                    rtd_maskl(0xb80ca238, 0xfffffffc, 0x00000000);   //fw_set[1:0]=read:during no read,write:during refresh
                    rtd_pr_zq_debug("    ******* DDR4 ZQ Calibration *******    \n");
                    rtd_pr_zq_debug("0xb80c91f4 = 0x%08x\n", rtd_inl(0xb80c91f4));
            }
            else
            {
                    rtd_pr_zq_debug("    ******* DDR3 ZQ Calibration *******    \n");
                    rtd_pr_zq_debug("0xb80c91f4 = 0x%08x\n", rtd_inl(0xb80c91f4));
            }

            rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xF7FFFFFF, 0x00000000);	//[27]=0,Disable ZQ Update Immediately
            rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFBFFFF, 0x00000000);	//[18]=0,Disable ZQ Update during Refresh

            for(i = 0; i < total_SET; i++)   //SET0~7
            {
                    tmp_val = (rtd_ZQ_inl(ZQFW_CTRL1) & 0xFF0000) >> 16; //SET_i [23:16]

                    if(tmp_val & (0x1 << i)) //if [23:16]!=0 DC1 zq cal. SET_i is enable
                    {
                            for(ocd_loops = 0, loop = 0; loop < ZQ_max_loop; loop++)   //scan loop
                            {
                                    err_type = 0;
                                    rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0x8FFFFFFF, (i << 28)); // Enable auto update OCD / ODT set0~set7 [30:28]=i
                                    rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFEFFFFFF, 0x01000000); // [24]=1,Calibration Start
                                    retry_timeout = (250 * 4); //(250*4) us

                                    while (retry_timeout > 0)
                                    {
                                            retry_timeout -= 5;
                                            udelay(5);
                                            smp_mb();

                                            if((rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS) & 0x80000000) == 0x80000000) //done[31]
                                                    break;
                                    }

                                    if((rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS) & 0x80000000) != 0x80000000)   //done[31]
                                    {
                                            rtd_ZQ_maskl(ZQFW_CTRL2, 0xBFFFFFFF, 1 << 30);

                                            if((test_mode & 0x1) == 0x1)
                                                    rtd_pr_zq_debug("[ZqCalDebugInfo]: DC1 zq cal. done failed!\n");
                                    }

                                    err_type = rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS) & 0x3FFFFFFF;

                                    if(err_type != 0x0)   //[29:0]=0
                                    {
                                            if((test_mode & 0x1) == 0x1)
                                                    rtd_pr_zq_debug("[ZqCalDebugInfo]: zq cal. failed(DCU1),0x%08x!\n", err_type);
                                    }

                                    if((test_mode & 0x1) == 0x1)   // [0]:debug msg
                                    {
                                            rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", (DC1_PAD_CTRL_ZPROG + i * 4), DC1_PAD_ZCTRL_STATUS, rtd_ZQ_inl(DC1_PAD_CTRL_ZPROG + i * 4), rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS));
                                            rtd_pr_zq_debug(" value(0x%08x): 0x%08x", (DC1_PAD_NOCD2_ZPROG + i * 4), rtd_ZQ_inl(DC1_PAD_NOCD2_ZPROG + i * 4));
                                            rtd_pr_zq_debug(" [31:0]:zctrl_status(DCU1 SET%d)  \n", i);
                                    }

                                    /* if done bit not asserted */
                                    if((rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS) & 0x80000000) != 0x80000000) //done[31]
                                    {
                                            rtd_pr_zq_info("ZQ_WARN: [ZqCalDebugInfo]: DC1 zq cal. done failed (0x%x) !\n", rtd_ZQ_inl(DC1_PAD_ZCTRL_STATUS));
                                            rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFEFFFFFF, 0x00000000);    // [24]=0,Disable Calibration
                                            continue;
                                    }

                                    rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFEFFFFFF, 0x00000000);    // [24]=0,Disable Calibration
                                    ocd_loops++;

                                    //**calibration result**//
                                    //DC1 PMOS OCD SET0~3/4~7[5:0][13:8][21:16][29:24], NMOS SET0~3/4~7
                                    OCDP_Addr = DC1_OCDP_SET0 + (i / 4) * 4;
                                    OCDN_Addr = DC1_OCDN_SET0 + (i / 4) * 4;
                                    bit_shift = (i % 4) * 8;

                                    PMOS[i] = (rtd_ZQ_inl(OCDP_Addr) >> bit_shift) & 0x3f;
                                    PMOS_SET[i] += PMOS[i];

                                    if((test_mode & 0x20) == 0x20) // [5]:ZQ msg
                                            rtd_pr_zq_debug("[ZqDebug]: %d)DC1_PMOS_SET%d=%d\n", loop + 1, i, PMOS[i]);

                                    if(PMOS[i] > DC1_SET_max[i])
                                    {
                                            DC1_SET_max[i] = PMOS[i];
                                            rtd_pr_zq_info("ZQ_WARN: [ZqCompare]: DC1_PMOS_SET%d_max=%d\n", i, DC1_SET_max[i]);
                                    }

                                    if(PMOS[i] < DC1_SET_min[i])
                                    {
                                            DC1_SET_min[i] = PMOS[i];
                                            rtd_pr_zq_info("ZQ_WARN: [ZqCompare]: DC1_PMOS_SET%d_min=%d\n", i, DC1_SET_min[i]);
                                    }

                                    //DC1 NMOS OCD SET0~3, NMOS SET0~3
                                    NMOS[i] = (rtd_ZQ_inl(OCDN_Addr) >> bit_shift) & 0x3f;
                                    NMOS_SET[i] += NMOS[i];

                                    if((test_mode & 0x20) == 0x20) // [5]:ZQ msg
                                            rtd_pr_zq_debug("[ZqDebug]: %d)DC1_NMOS_SET%d=%d\n", loop + 1, i, NMOS[i]);

                                    if(NMOS[i] > DC1_SET_max[i + total_SET])
                                    {
                                            DC1_SET_max[i + total_SET] = NMOS[i];
                                            rtd_pr_zq_info("ZQ_WARN: [ZqCompare]: DC1_NMOS_SET%d_max=%d\n", i, DC1_SET_max[i + total_SET]);
                                    }

                                    if(NMOS[i] < DC1_SET_min[i + total_SET])
                                    {
                                            DC1_SET_min[i + total_SET] = NMOS[i];
                                            rtd_pr_zq_info("ZQ_WARN: [ZqCompare]: DC1_NMOS_SET%d_min=%d\n", i, DC1_SET_min[i + total_SET]);
                                    }

                                    if((ZQ_Config & 0x20000) == 0x20000)   //[17]=1,record all scan results
                                    {
                                            if(i == 0)
                                            {
                                                    DC1_PMOS_SET0_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET0_Total[NMOS[i]]++;
                                            }
                                            else if(i == 1)
                                            {
                                                    DC1_PMOS_SET1_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET1_Total[NMOS[i]]++;
                                            }
                                            else if(i == 2)
                                            {
                                                    DC1_PMOS_SET2_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET2_Total[NMOS[i]]++;
                                            }
                                            else if(i == 3)
                                            {
                                                    DC1_PMOS_SET3_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET3_Total[NMOS[i]]++;
                                            }
                                            else if(i == 4)
                                            {
                                                    DC1_PMOS_SET4_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET4_Total[NMOS[i]]++;
                                            }
                                            else if(i == 5)
                                            {
                                                    DC1_PMOS_SET5_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET5_Total[NMOS[i]]++;
                                            }
                                            else if(i == 6)
                                            {
                                                    DC1_PMOS_SET6_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET6_Total[NMOS[i]]++;
                                            }
                                            else if(i == 7)
                                            {
                                                    DC1_PMOS_SET7_Total[PMOS[i]]++;
                                                    DC1_NMOS_SET7_Total[NMOS[i]]++;
                                            }
                                    }
                            }//scan loop(s)

                            /* if done bit not asserted */
                            if((ocd_loops > 2) && (ZQ_max_loop == OCD_test_loop))   //multi-round of ZQ SET0~3 calibration
                            {
                                    if((ocd_loops - (PMOS_SET[i] % ocd_loops)) > (PMOS_SET[i] % ocd_loops))
                                            PMOS_SET[i] = PMOS_SET[i] / ocd_loops;
                                    else
                                            PMOS_SET[i] = 1 + (PMOS_SET[i] / ocd_loops);

                                    if((ocd_loops - (NMOS_SET[i] %  ocd_loops)) > (NMOS_SET[i] % ocd_loops))
                                            NMOS_SET[i] = NMOS_SET[i] / ocd_loops;
                                    else
                                            NMOS_SET[i] = 1 + (NMOS_SET[i] / ocd_loops);
                            }//[2]=1, multi-round of ZQ SET0~3 calibration

                            if((ZQ_Config & 0x10000) == 0x10000)   //[16]=1,record average result
                            {
                                    if(i == 0)
                                    {
                                            DC1_PMOS_SET0[PMOS_SET[i]]++;
                                            DC1_NMOS_SET0[NMOS_SET[i]]++;
                                    }
                                    else if(i == 1)
                                    {
                                            DC1_PMOS_SET1[PMOS_SET[i]]++;
                                            DC1_NMOS_SET1[NMOS_SET[i]]++;
                                    }
                                    else if(i == 2)
                                    {
                                            DC1_PMOS_SET2[PMOS_SET[i]]++;
                                            DC1_NMOS_SET2[NMOS_SET[i]]++;
                                    }
                                    else if(i == 3)
                                    {
                                            DC1_PMOS_SET3[PMOS_SET[i]]++;
                                            DC1_NMOS_SET3[NMOS_SET[i]]++;
                                    }
                                    else if(i == 4)
                                    {
                                            DC1_PMOS_SET4[PMOS_SET[i]]++;
                                            DC1_NMOS_SET4[NMOS_SET[i]]++;
                                    }
                                    else if(i == 5)
                                    {
                                            DC1_PMOS_SET5[PMOS_SET[i]]++;
                                            DC1_NMOS_SET5[NMOS_SET[i]]++;
                                    }
                                    else if(i == 6)
                                    {
                                            DC1_PMOS_SET6[PMOS_SET[i]]++;
                                            DC1_NMOS_SET6[NMOS_SET[i]]++;
                                    }
                                    else if(i == 7)
                                    {
                                            DC1_PMOS_SET7[PMOS_SET[i]]++;
                                            DC1_NMOS_SET7[NMOS_SET[i]]++;
                                    }
                            }

                            if((test_mode & 0x20) == 0x20)   // [5]:ZQ msg
                            {
                                    rtd_pr_zq_debug("[ZqAverage]: DC1_PMOS_SET%d=%d\n", i, PMOS_SET[i]);
                                    rtd_pr_zq_debug("[ZqAverage]: DC1_NMOS_SET%d=%d\n", i, NMOS_SET[i]);
                            }

                            //**phase set**//
                            if (ocd_loops > 0)
                            {
                                    rtd_ZQ_maskl(OCDP_Addr, ~(0x3f << bit_shift), (PMOS_SET[i] << bit_shift)); //DC1 PMOS OCD SET0[5:0][13:8][21:16][29:24]
                                    rtd_ZQ_maskl(OCDN_Addr, ~(0x3f << bit_shift), (NMOS_SET[i] << bit_shift)); //DC1 NMOS OCD SET0[5:0]
                            }
                            else
                            {
                                    ocd_fail++;
                            }
                    }//if zq cal. SET_i is enable
            }//SET0~7

            if((test_mode & 0x20) == 0x20)   // [5]:ZQ msg
            {
                    rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET0, DC1_OCDN_SET0, rtd_ZQ_inl(DC1_OCDP_SET0), rtd_ZQ_inl(DC1_OCDN_SET0));
                    rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET0 + 0x4, DC1_OCDN_SET0 + 0x4, rtd_ZQ_inl(DC1_OCDP_SET0 + 0x4), rtd_ZQ_inl(DC1_OCDN_SET0 + 0x4));
            }

            if((test_mode & 0x20) == 0x20)   // [5]:ZQ msg
            {
                    rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET1, DC1_OCDN_SET1, rtd_ZQ_inl(DC1_OCDP_SET1), rtd_ZQ_inl(DC1_OCDN_SET1));
                    rtd_pr_zq_debug("[ZqCalDebugInfo]: value(0x%08x,0x%08x): 0x%08x, 0x%08x", DC1_OCDP_SET1 + 0x4, DC1_OCDN_SET1 + 0x4, rtd_ZQ_inl(DC1_OCDP_SET1 + 0x4), rtd_ZQ_inl(DC1_OCDN_SET1 + 0x4));
            }

            //log print
            if((ZQ_Config & 0x18) != 0) //[4:3]log
                    counter++;

            if(counter == print_flag)
            {
                    //DC1
                    if((DC1_R480_err_msg[0] > 0) && ((test_mode & 0x10) == 0x10)) //[4]:R480 msg
                            rtd_pr_zq_debug("[ZqErrorMsg] DC1_R480 error cnt=%d, last value=%d\n", DC1_R480_err_msg[0], DC1_R480_err_msg[1]);

                    if((ZQ_Config & 0x8) == 0x8)   //[3]=1,averaged calibration results
                    {
                            rtd_pr_zq_debug("\n[ZqSummary]: DC1_R480\n");

                            for(i = 0; i < R480_INDEX; i++)
                            {
                                    if(loop_delay > 0)
                                            usleep_range(loop_delay, loop_delay + 10);

                                    rtd_pr_zq_debug("%d,%d;\n", i, DC1_R480[i]);
                            }

                            if(para_delay > 0)
                                    usleep_range(para_delay, para_delay + 10);

                            for(loop = 0; loop < total_SET; loop++)   //SET0~7
                            {
                                    rtd_pr_zq_debug("\n[ZqSummary]: DC1_PMOS_SET%d\n", loop);

                                    for(i = 0; i < SET_INDEX; i++)
                                    {
                                            if(loop_delay > 0)
                                                    usleep_range(loop_delay, loop_delay + 10);

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
                                            usleep_range(para_delay, para_delay + 10);

                                    rtd_pr_zq_debug("\n[ZqSummary]: DC1_NMOS_SET%d\n", loop);

                                    for(i = 0; i < SET_INDEX; i++)
                                    {
                                            if(loop_delay > 0)
                                                    usleep_range(loop_delay, loop_delay + 10);

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
                                            usleep_range(para_delay, para_delay + 10);
                            } //set0~7
                    } //averaged result

                    //Total count result, DC1
                    if((ZQ_Config & 0x10) == 0x10)   //[4]=1,record all scan results
                    {
                            rtd_pr_zq_debug("\n[ZqTotalCnt]: DC1_R480_Total\n");

                            for(i = 0; i < R480_INDEX; i++)
                            {
                                    if(loop_delay > 0)
                                            usleep_range(loop_delay, loop_delay + 10);

                                    rtd_pr_zq_debug("%d,%d;\n", i, DC1_R480_Total[i]);
                            }

                            if(para_delay > 0)
                                    usleep_range(para_delay, para_delay + 10);

                            for(loop = 0; loop < total_SET; loop++)
                            {
                                    rtd_pr_zq_debug("\n[ZqTotalCnt]: DC1_PMOS_SET%d_Total\n", loop);

                                    for(i = 0; i < SET_INDEX; i++)
                                    {
                                            if(loop_delay > 0)
                                                    usleep_range(loop_delay, loop_delay + 10);

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
                                            usleep_range(para_delay, para_delay + 10);

                                    rtd_pr_zq_debug("\n[ZqTotalCnt]: DC1_NMOS_SET%d_Total\n", loop);

                                    for(i = 0; i < SET_INDEX; i++)
                                    {
                                            if(loop_delay > 0)
                                                    usleep_range(loop_delay, loop_delay + 10);

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
                                            usleep_range(para_delay, para_delay + 10);
                            }
                    } //edn of record all scan results

                    if((ZQ_Config & 0x20) == 0x20)   //[5]=1, max and min for each set
                    {
                            for(i = 0; i < total_SET; i++)   //SET0~3
                            {
                                    rtd_pr_zq_debug("[ZqCompare]: DC1_PMOS_SET%d_max=%d\n", i, DC1_SET_max[i]);
                                    rtd_pr_zq_debug("[ZqCompare]: DC1_PMOS_SET%d_min=%d\n", i, DC1_SET_min[i]);
                            }

                            if(para_delay > 0)
                                    usleep_range(para_delay, para_delay + 10);
                    }

                    counter = 0;

            }// print result


        rtd_ZQ_maskl(ZQFW_CTRL1, 0xffffdfff, 0x2000); //[13]=1 zq fw finish


        //**KGD ZQCS Cal.**//
        rtd_pr_zq_debug("KGD ZQCS Calibration!\n");

        if((rtd_ZQ_inl(0xb80c2028) & 0x1) == 0x0)
        {	//1MC mode
            //issue MC1 KGD ZQCS Calibration
            rtd_outl(0xb80c2110,0xd0000000);
            rtd_outl(0xb80c2130,0x00800003);
            rtd_outl(0xb80c2100,0x80000000);
            udelay(1);
        }
        else
        {
        //2MC mode
            //issue MC1 KGD ZQCS Calibration
            rtd_outl(0xb80c2110,0xd0000000);
            rtd_outl(0xb80c2130,0x00800003);
            rtd_outl(0xb80c2100,0x80000000);
            udelay(1);
            //issue MC2 KGD ZQCS Calibration
            rtd_outl(0xb80c3110,0xd0000000);
            rtd_outl(0xb80c3130,0x00800003);
            rtd_outl(0xb80c3100,0x80000000);
            udelay(1);
        }

        //** update soc zq calibration data **//
        rtd_pr_zq_debug("update soc zq calibration data\n");
        {
            unsigned int DQ_PODT_SET, DQ_NODT_SET;
            unsigned int DQ_POCD_SET, DQ_NOCD_SET;
            unsigned int DQSP_PODT_SET, DQSP_NODT_SET, DQSN_PODT_SET, DQSN_NODT_SET;
            unsigned int DQSP_POCD_SET, DQSP_NOCD_SET, DQSN_POCD_SET, DQSN_NOCD_SET;
            //DQ_ODT
            DQ_PODT_SET = (rtd_inl(0xb80c275c) & 0xf);
            DQ_NODT_SET = (rtd_inl(0xb80c275c) & 0xf0)>>4;
            //DQ_OCD
            DQ_POCD_SET = (rtd_inl(0xb80c275c) & 0xf00)>>8;
            DQ_NOCD_SET = (rtd_inl(0xb80c275c) & 0xf000)>>12;
            //DQS_ODT
            DQSP_PODT_SET = (rtd_inl(0xb80c2784) & 0xf);
            DQSP_NODT_SET = (rtd_inl(0xb80c2784) & 0xf0)>>4;
            DQSN_PODT_SET = (rtd_inl(0xb80c2784) & 0xf00)>>8;
            DQSN_NODT_SET = (rtd_inl(0xb80c2784) & 0xf000)>>12;
            //DQS_OCD
            DQSP_POCD_SET = (rtd_inl(0xb80c2784) & 0xf0000)>>16;
            DQSP_NOCD_SET = (rtd_inl(0xb80c2784) & 0xf00000)>>20;
            DQSN_POCD_SET = (rtd_inl(0xb80c2784) & 0xf000000)>>24;
            DQSN_NOCD_SET = (rtd_inl(0xb80c2784) & 0xf0000000)>>28;    

            //DQ[31:0] ODT
            zq_odt_set_sel(0,DQ_PODT_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca070,29,24); //DQ[7:0]PODT=set4
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca074,29,24); //DQ[15:8]PODT=set4
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca078,29,24); //DQ[23:16]PODT=set4
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca07c,29,24); //DQ[31:24]PODT=set4
            // rtd_pr_zq_alert("ZQ_WARN: DQ_PODT_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            
            zq_odt_set_sel(1,DQ_NODT_SET);      
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca070,21,16); //DQ[7:0]NODT=set7
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca074,21,16); //DQ[15:8]NODT=set7
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca078,21,16); //DQ[23:16]NODT=set7
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca07c,21,16); //DQ[31:24]NODT=set7
            // rtd_pr_zq_alert("ZQ_WARN: DQ_NODT_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);

            //DQ[31:0] OCD
            zq_ocd_set_sel(0,DQ_POCD_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca070,13,8); //DQ[7:0]POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca074,13,8); //DQ[15:8]POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca078,13,8); //DQ[23:16]POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca07c,13,8); //DQ[31:24]POCD=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQ_POCD_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_ocd_set_sel(1,DQ_NOCD_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca070,5,0); //DQ[7:0]NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca074,5,0); //DQ[15:8]NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca078,5,0); //DQ[23:16]NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca07c,5,0); //DQ[31:24]NOCD=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQ_NOCD_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            //DQS[3:0] ODT
            zq_odt_set_sel(0,DQSP_PODT_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca010,29,24); //DQS0 PODT=set1
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca014,29,24); //DQS1 PODT=set1
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca018,29,24); //DQS2 PODT=set1
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca01c,29,24); //DQS3 PODT=set1
            // rtd_pr_zq_alert("ZQ_WARN: DQSP_PODT_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_odt_set_sel(1,DQSP_NODT_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca010,21,16); //DQS0 NODT=set2
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca014,21,16); //DQS1 NODT=set2
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca018,21,16); //DQS2 NODT=set2
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca01c,21,16); //DQS3 NODT=set2
            // rtd_pr_zq_alert("ZQ_WARN: DQSP_NODT_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_odt_set_sel(0,DQSN_PODT_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca020,29,24); //DQS#0 PODT=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca024,29,24); //DQS#1 PODT=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca028,29,24); //DQS#2 PODT=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca02c,29,24); //DQS#3 PODT=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQSN_PODT_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_odt_set_sel(1,DQSN_NODT_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca020,21,16); //DQS#0 NODT=set7
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca024,21,16); //DQS#1 NODT=set7
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca028,21,16); //DQS#2 NODT=set7
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca02c,21,16); //DQS#3 NODT=set7
            // rtd_pr_zq_alert("ZQ_WARN: DQSN_NODT_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            //DQS[3:0] OCD
            zq_ocd_set_sel(0,DQSP_POCD_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca010,13,8); //DQS0 POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca014,13,8); //DQS1 POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca018,13,8); //DQS2 POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca01c,13,8); //DQS3 POCD=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQSP_POCD_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_ocd_set_sel(1,DQSP_NOCD_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca010,5,0); //DQS0 NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca014,5,0); //DQS1 NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca018,5,0); //DQS2 NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca01c,5,0); //DQS3 NOCD=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQSP_NOCD_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_ocd_set_sel(0,DQSN_POCD_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca020,13,8); //DQS#0 POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca024,13,8); //DQS#1 POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca028,13,8); //DQS#2 POCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca02c,13,8); //DQS#3 POCD=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQSN_POCD_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);
            
            zq_ocd_set_sel(1,DQSN_NOCD_SET);
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca020,5,0); //DQS#0 NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca024,5,0); //DQS#1 NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca028,5,0); //DQS#2 NOCD=set0
            rtd_copy(zq_set_addr,zq_set_msb,zq_set_lsb,0xb80ca02c,5,0); //DQS#3 NOCD=set0
            // rtd_pr_zq_alert("ZQ_WARN: DQSN_NOCD_SET\n");
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_addr = (0x%08x) \n", zq_set_addr);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_msb = (0x%08x) \n", zq_set_msb);
            // rtd_pr_zq_alert("ZQ_WARN: zq_set_lsb = (0x%08x) \n", zq_set_lsb);

            rtd_maskl(0xb80c92d0,0xfffffff0,0x00000000); //[3:0]disable vr_upd7~0
            rtd_maskl(0xb80ca2d0,0xfffffff0,0x00000000);
            rtd_maskl(0xb80c9304,0xfffffff0,0x00000000);
            rtd_maskl(0xb80ca520,0xfffffff8,0x00000000); //disable fw_rd_te_upd / fw_rd_delta_upd / fw_dqsen_ftun_upd
            rtd_maskl(0xb80ca524,0xfffffff8,0x00000000);
            rtd_maskl(0xb80ca720,0xfffffff8,0x00000000);
            rtd_maskl(0xb80ca724,0xfffffff8,0x00000000);
            rtd_maskl(0xb80c9920,0xfffffff8,0x00000000);
            rtd_maskl(0xb80c9924,0xfffffff8,0x00000000);
            rtd_maskl(0xb80c9b20,0xfffffff8,0x00000000);
            rtd_maskl(0xb80c9b24,0xfffffff8,0x00000000);

            rtd_outl(0xb80c923c,0x0000000c); //update read delay tap
            udelay(5);

            rtd_maskl(0xb80c92d0,0xfffffff0,0x0000000f); //[3:0]enable vr_upd7~0
            rtd_maskl(0xb80ca2d0,0xfffffff0,0x0000000f);
            rtd_maskl(0xb80c9304,0xfffffff0,0x0000000f);
            rtd_maskl(0xb80ca520,0xfffffff8,0x00000007); //enable fw_rd_te_upd / fw_rd_delta_upd / fw_dqsen_ftun_upd
            rtd_maskl(0xb80ca524,0xfffffff8,0x00000007);
            rtd_maskl(0xb80ca720,0xfffffff8,0x00000007);
            rtd_maskl(0xb80ca724,0xfffffff8,0x00000007);
            rtd_maskl(0xb80c9920,0xfffffff8,0x00000007);
            rtd_maskl(0xb80c9924,0xfffffff8,0x00000007);
            rtd_maskl(0xb80c9b20,0xfffffff8,0x00000007);
            rtd_maskl(0xb80c9b24,0xfffffff8,0x00000007);
        
            //ZQ update 1MC mode:during refresh, 2MC mode: refresh sync 
            if((rtd_ZQ_inl(0xb80c2028) & 0x1) == 0x0)
            {	//1MC mode
                rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFBFFFF, 0x00040000);	// [18]=1,Enable Update during Refresh
                udelay(1);
            }
            else
            {             
                //2MC mode
                rtd_ZQ_maskl(DC1_PAD_CTRL_PROG, 0xFFFBFFFF, 0x00040000);	// [18]=1,Enable Update during Refresh
                udelay(1);
                start = local_clock();
                rtd_ZQ_maskl(MC1_SYS_REF_1, 0xffefffff, 0x00100000); //bit20, auto clear
                rtd_ZQ_maskl(MC2_SYS_REF_1, 0xffefffff, 0x00100000); //bit20, auto clear
                while ((rtd_ZQ_inl(MC1_SYS_REF_1) & 0x00100000) && (rtd_ZQ_inl(MC2_SYS_REF_1) & 0x00100000) )
                {
                  now = local_clock();
                  if (div_u64((now - start), 1000UL) > 64 * 1000ULL /*64ms*/)
                  {
                      printk_once(KERN_INFO "after soc update, MC1_SYS_REF_1 (0x%x) still hold REF_EN high in 64ms.\n", rtd_ZQ_inl(MC1_SYS_REF_1));
                      printk_once(KERN_INFO "after soc update, MC2_SYS_REF_1 (0x%x) still hold REF_EN high in 64ms.\n", rtd_ZQ_inl(MC2_SYS_REF_1));
                      start = now;
                      //break;  // 64ms expiration
                      /* sleep 50ms, and polling infinitely */
                      //schedule_timeout_interruptible(HZ/20);
                  }
                }
            }
           
            if((test_mode & 0x00000040) != 0x00000000)  //for debug purpose
            {    
                rtd_pr_zq_alert("ZQ_WARN: DQ_PODT_SET = (0x%08x) \n", DQ_PODT_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQ_NODT_SET = (0x%08x) \n", DQ_NODT_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQ_POCD_SET = (0x%08x) \n", DQ_POCD_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQ_NOCD_SET = (0x%08x) \n", DQ_NOCD_SET);

                rtd_pr_zq_alert("ZQ_WARN: DQSP_PODT_SET = (0x%08x) \n", DQSP_PODT_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQSP_NODT_SET = (0x%08x) \n", DQSP_NODT_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQSN_PODT_SET = (0x%08x) \n", DQSN_PODT_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQSN_NODT_SET = (0x%08x) \n", DQSN_NODT_SET);
                                           
                rtd_pr_zq_alert("ZQ_WARN: DQSP_POCD_SET = (0x%08x) \n", DQSP_POCD_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQSP_NOCD_SET = (0x%08x) \n", DQSP_NOCD_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQSN_POCD_SET = (0x%08x) \n", DQSN_POCD_SET);
                rtd_pr_zq_alert("ZQ_WARN: DQSN_NOCD_SET = (0x%08x) \n", DQSN_NOCD_SET);

                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9190 value: (0x%08x) \n", rtd_inl(0xb80c9190));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9194 value: (0x%08x) \n", rtd_inl(0xb80c9194));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9198 value: (0x%08x) \n", rtd_inl(0xb80c9198));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c919c value: (0x%08x) \n", rtd_inl(0xb80c919c));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91b0 value: (0x%08x) \n", rtd_inl(0xb80c91b0));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91b4 value: (0x%08x) \n", rtd_inl(0xb80c91b4));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91b8 value: (0x%08x) \n", rtd_inl(0xb80c91b8));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91bc value: (0x%08x) \n", rtd_inl(0xb80c91bc));

                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9180 value: (0x%08x) \n", rtd_inl(0xb80c9180));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9184 value: (0x%08x) \n", rtd_inl(0xb80c9184));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9188 value: (0x%08x) \n", rtd_inl(0xb80c9188));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c918c value: (0x%08x) \n", rtd_inl(0xb80c918c));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91a0 value: (0x%08x) \n", rtd_inl(0xb80c91a0));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91a4 value: (0x%08x) \n", rtd_inl(0xb80c91a4));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91a8 value: (0x%08x) \n", rtd_inl(0xb80c91a8));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c91ac value: (0x%08x) \n", rtd_inl(0xb80c91ac));

                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca070 value: (0x%08x) \n", rtd_inl(0xb80ca070));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca074 value: (0x%08x) \n", rtd_inl(0xb80ca074));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca078 value: (0x%08x) \n", rtd_inl(0xb80ca078));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca07c value: (0x%08x) \n", rtd_inl(0xb80ca07c));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca010 value: (0x%08x) \n", rtd_inl(0xb80ca010));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca014 value: (0x%08x) \n", rtd_inl(0xb80ca014));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca018 value: (0x%08x) \n", rtd_inl(0xb80ca018));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca01c value: (0x%08x) \n", rtd_inl(0xb80ca01c));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca020 value: (0x%08x) \n", rtd_inl(0xb80ca020));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca024 value: (0x%08x) \n", rtd_inl(0xb80ca024));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca028 value: (0x%08x) \n", rtd_inl(0xb80ca028));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca02c value: (0x%08x) \n", rtd_inl(0xb80ca02c));
               
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9258 value: (0x%08x) \n", rtd_inl(0xb80c9258));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9384 value: (0x%08x) \n", rtd_inl(0xb80c9384));

                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca4d0 value: (0x%08x) \n", rtd_inl(0xb80ca4d0));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca4d4 value: (0x%08x) \n", rtd_inl(0xb80ca4d4));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca6d0 value: (0x%08x) \n", rtd_inl(0xb80ca6d0));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca6d4 value: (0x%08x) \n", rtd_inl(0xb80ca6d4));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c98d0 value: (0x%08x) \n", rtd_inl(0xb80c98d0));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c98d4 value: (0x%08x) \n", rtd_inl(0xb80c98d4));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9ad0 value: (0x%08x) \n", rtd_inl(0xb80c9ad0));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9ad4 value: (0x%08x) \n", rtd_inl(0xb80c9ad4));

                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca4c8 value: (0x%08x) \n", rtd_inl(0xb80ca4c8));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca4cc value: (0x%08x) \n", rtd_inl(0xb80ca4cc));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca6c8 value: (0x%08x) \n", rtd_inl(0xb80ca6c8));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80ca6cc value: (0x%08x) \n", rtd_inl(0xb80ca6cc));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c98c8 value: (0x%08x) \n", rtd_inl(0xb80c98c8));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c98cc value: (0x%08x) \n", rtd_inl(0xb80c98cc));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9ac8 value: (0x%08x) \n", rtd_inl(0xb80c9ac8));
                rtd_pr_zq_alert("ZQ_WARN: before zq update: reg: 0xb80c9acc value: (0x%08x) \n", rtd_inl(0xb80c9acc));
            }
        }
    } //end of Main enable

#if 0 // debug purpose
        {

#define BITS_5_1(v)  ((int)((v >> 1) & 0x01f))
#if 1
#define PRINT_DIVERGENT_DATA(reg, pdata, cdata)  do { rtd_pr_zq_info("ZQ_WARN: => divergent data. reg: 0x%x value: (0x%x, 0x%x) \n", reg, pdata, cdata); } while(0)
#define PRINT_RAW_DATA(reg, cdata)  do { rtd_pr_zq_alert("ZQ_WARN: raw data: reg: 0x%08x value: (0x%08x) \n", reg, cdata); } while(0)
#else
#define PRINT_DIVERGENT_DATA(reg, pdata, cdata)  do {  } while(0)
#define PRINT_RAW_DATA(reg, cdata)  do {  } while(0)
#endif
                static unsigned int prev_data[32];
                static unsigned int zq_data_regs[] =
                        {
                                // R480 value : 0xb80be14C[5¡G1]
                                0xb80c914C,

                                //***** OCD SET value *****//
                                0xb80c9190,       //P-OCD set_3~0
                                0xb80c9194,     //P-OCD set_7~4
                                0xb80c9198,     //N-OCD set_3~0
                                0xb80c919c,     //N-OCD set_7~4
                                0xb80c91b0,     //P-OCD set_3~0
                                0xb80c91b4,     //P-OCD set_7~4
                                0xb80c91b8,     //N-OCD set_3~0
                                0xb80c91bc,     //N-OCD set_7~4

                                //***** ODT SET value *****//
                                0xb80c9180,       //P-ODT set_3~0
                                0xb80c9184,     //P-ODT set_7~4
                                0xb80c9188,     //N-ODT set_3~0
                                0xb80c918c,     //N-ODT set_7~4
                                0xb80c91a0,       //P-ODT set_3~0
                                0xb80c91a4,     //P-ODT set_7~4
                                0xb80c91a8,     //N-ODT set_3~0
                                0xb80c91ac,       //N-ODT set_7~4

                                //dqs en int
                                0xb80c9258,     //dqsen_int[21:20][3:0]
                                0xb80c9384,     //dqsen_int[1:0]

                                //RX FIFO
                                0xb80ca4d0,     //slice0 [17:9]NFIFO, [8:0]PFIFO
                                0xb80ca4d4,     //slice1 [17:9]NFIFO, [8:0]PFIFO
                                0xb80ca6d0,     //slice2 [17:9]NFIFO, [8:0]PFIFO
                                0xb80ca6d4,     //slice3 [17:9]NFIFO, [8:0]PFIFO
                                0xb80c98d0,     //slice4 [17:9]NFIFO, [8:0]PFIFO
                                0xb80c98d4,     //slice5 [17:9]NFIFO, [8:0]PFIFO
                                0xb80c9ad0,     //slice6 [17:9]NFIFO, [8:0]PFIFO
                                0xb80c9ad4,     //slice7 [17:9]NFIFO, [8:0]PFIFO

                                //DQS_3P_INT flag
                                0xb80ca4c8,  //slice0_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80ca4cc,  //slice1_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80ca6c8,  //slice2_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80ca6cc,  //slice3_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80c98c8,  //slice4_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80c98cc,  //slice5_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80c9ac8,  //slice6_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p
                                0xb80c9acc,  //slice7_dq, [31:24] rb_int_n, [23:16]:lb_int_n, [15:8] rb_int_p, [7:0]:lb_int_p

        };

                typedef struct _ltb
                {
                        union
                        {
                                unsigned int L;
                                char C[4];
                        };
                } LTB;

                int size = ARRAY_SIZE(zq_data_regs);
                int i;

                for (i = 0; i < size; i++)
                {
                        unsigned int data;
                        LTB *prev;
                        LTB *curr;

                        data = rtd_ZQ_inl(zq_data_regs[i]);
                        curr = (LTB *)&data;
                        prev = (LTB *)&prev_data[i];

                        PRINT_RAW_DATA(zq_data_regs[i], data);

                        if (i == 0)   //r480
                        {
                                int diff;
                                diff = (BITS_5_1(prev->L) - BITS_5_1(data));

                                if ((prev->L != 0) && (diff > 3 || diff < -3))
                                        PRINT_DIVERGENT_DATA(zq_data_regs[i], prev_data[i], data);

                                prev->L = data;
                        }
                        else     //others
                        {
                                int n;
                                int res;

                                res = 0;

                                for (n = 0; n < 4; n++)
                                {
                                        int diff;
                                        diff = prev->C[n] - curr->C[n];

                                        if ((prev->L != 0) && (diff > 3 || diff < -3))
                                        {
                                                res = 1;
                                                break;
                                        }
                                }

                                if (res != 0)
                                        PRINT_DIVERGENT_DATA(zq_data_regs[i], prev_data[i], data);

                                prev->L = data;
                        }
                }
        }
#endif  //#if 0 // debug purpose
}
#endif //CONFIG_RTK_ZQ_ENABLE


static int rtk_monitor_thread(void *arg)
{
	struct sched_param param = { .sched_priority = 1 };
//	unsigned int diff;

	int reg_value;
	int monitor_value=0;

#ifdef CONFIG_RTK_ZQ_ENABLE
	int monitor_tmp=0;
        int monitor_diff, flag = 0;
        int onetime_flag = 0;
#endif //CONFIG_RTK_ZQ_ENABLE

	//rtk_gpio_set_dir(rtk_gpio_id(MIS_GPIO, 59), 0);

	set_freezable();
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
        Array_init(DC1_PMOS_SET4, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET5, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET6, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET7, SET_INDEX, 0);

    Array_init(DC1_NMOS_SET0, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET1, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET2, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET3, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET4, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET5, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET6, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET7, SET_INDEX, 0);

    Array_init(DC1_R480_Total, R480_INDEX, 0);
    Array_init(DC1_PMOS_SET0_Total, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET1_Total, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET2_Total, SET_INDEX, 0);
    Array_init(DC1_PMOS_SET3_Total, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET4_Total, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET5_Total, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET6_Total, SET_INDEX, 0);
        Array_init(DC1_PMOS_SET7_Total, SET_INDEX, 0);

    Array_init(DC1_NMOS_SET0_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET1_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET2_Total, SET_INDEX, 0);
    Array_init(DC1_NMOS_SET3_Total, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET4_Total, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET5_Total, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET6_Total, SET_INDEX, 0);
        Array_init(DC1_NMOS_SET7_Total, SET_INDEX, 0);
		
	Get_Ori_ZQ_Phase(0); //get original phase

#endif //CONFIG_RTK_ZQ_ENABLE

        do
        {
		try_to_freeze();
		msleep(rtk_monitor_delay >> 1);
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
			rtd_pr_thermal_debug("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, -(monitor_value%1000),(rtd_inl(STB_WDOG_DATA14_reg)&0x00FF0000)>>16,reg_value);
                       if(rtk_monitor_debug == 1) //message out with low level by debug flag
                               rtd_pr_thermal_crit("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, -(monitor_value%1000),(rtd_inl(STB_WDOG_DATA14_reg)&0x00FF0000)>>16,reg_value);
		}
		else
		{
			rtd_pr_thermal_debug("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, monitor_value%1000,(rtd_inl(STB_WDOG_DATA14_reg)&0x00FF0000)>>16,reg_value);
                       if(rtk_monitor_debug == 1) //message out with low level by debug flag
                               rtd_pr_thermal_crit("[Rtk Monitor] =%3d.%03d Highest:%d reg:%x\n",monitor_value/1000, (monitor_value%1000),(rtd_inl(STB_WDOG_DATA14_reg)&0x00FF0000)>>16,reg_value);
		}

		if(monitor_value/1000 >= RTK_MONITOR_THERMAL_TEMP_PRINT_THRESHOLD)
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

#ifdef CONFIG_RTK_ZQ_ENABLE
		if(ZQ_init_flag == 0)
		{
			onetime_flag = 0;
			flag = 0;
			DC1_R480_max = 0;
			DC1_R480_min = 100;
			ZQ_init_flag = 1;
		}

                test_mode = rtd_ZQ_inl(ZQFW_CTRL1); //[7]:auto OCD; [6]:ZQ per sec; [5]:ZQ msg; [4]:R480 msg; [3:2]:DCU2/DCU1 R480 Cal Enable; [1]:Main Enable; [0]:debug msg
                ZQ_Config = rtd_ZQ_inl(ZQFW_CTRL2);

		if((ZQ_Config&0x40) == 0x40) //[6]=1
			Get_Ori_ZQ_Phase(1); //log print

                if(((ZQ_Config & 0x1) == 0x1) && (onetime_flag == 0))   //[0]=1,one time calibration
                {
			Kernal_ZqCalibration();
			onetime_flag = 1;
                }
                else
                {
                        //test_mode |= 0x00000040 ;; // forece test mode for debug purpose, once K per second
                        if((test_mode & 0x00000040) == 0x00000000)   //[6]=0 update by monitor_diff > 10 or <-10
                        {
				if(flag == 0)
					monitor_tmp = monitor_value;

				monitor_diff = (monitor_tmp - monitor_value)/1000;

                                if((flag == 0) || (monitor_diff > 10) || (monitor_diff < -10))
                                {
					Kernal_ZqCalibration(); //DDR zq cal.
					monitor_tmp = monitor_value;
					flag = 1;
				}
                        }
                        else   //[6]=1, update zq by second
				Kernal_ZqCalibration();
		}

#endif //CONFIG_RTK_ZQ_ENABLE

#ifdef CONFIG_FIXED_ME_443
#ifdef VBE_DISP_TCON_SELF_CHECK
		if((!(IoReg_Read32(MDOMAIN_DISP_dispm_comp_decomp_dbg_mux_reg)&_BIT31))&&(monitor_rgb_rolling==0)) {
			vbe_disp_always_check_tcon();//every 1second to check
		}
#endif
#endif
		try_to_freeze();
		msleep(rtk_monitor_delay >> 1);
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

void rtk_monitor_disable(void)
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

EXPORT_SYMBOL(rtk_monitor_active_show);
EXPORT_SYMBOL(rtk_monitor_active_store);
EXPORT_SYMBOL(rtk_monitor_delay_show);
EXPORT_SYMBOL(rtk_monitor_delay_store);
EXPORT_SYMBOL(rtk_monitor_debug_show);
EXPORT_SYMBOL(rtk_monitor_debug_store);

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

int rtk_monitor_init(void)
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


#if 0 // move to rtk_tmu_probe
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
//late_initcall(rtk_monitor_init); //move to rtk_tmu_probe
int rtk_monitor_resume(void)
{
        ZQ_init_flag = 0;
        return 0;
}
#endif
