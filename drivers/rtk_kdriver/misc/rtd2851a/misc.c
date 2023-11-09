#include <generated/autoconf.h>
#include <linux/init.h>
#include <linux/kernel.h>   /* MISC_DBG_PRINT() */
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>           /* everything... */
#include <linux/errno.h>        /* error codes */
#include <linux/types.h>        /* size_t */
#include <linux/fcntl.h>        /* O_ACCMODE */
#include <linux/cdev.h>
#include <linux/module.h>
//#include <asm/system.h>       /* cli(), *_flags */
#include <linux/uaccess.h>        /* copy_*_user */
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <rtk_kdriver/misc.h>
#include <rtk_kdriver/io.h>
//#include <rbus/crt_reg.h>
#include <rbus/dc_sys_reg.h>
#include <rbus/stb_reg.h>
#include "rtd_types.h"
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/pinmux_main_reg.h>
#if 0 //code for IB/DC2
#include <rbus/dc2_sys_reg.h>
#endif
#include <rbus/dc1_mc_reg.h>
#if 0 //code for IB/DC2
#include <rbus/dc2_mc_reg.h>
#endif
#include <rbus/sb2_reg.h>
#include <rbus/tvsb1_reg.h>
#include <rbus/tvsb2_reg.h>
#include <rbus/tvsb3_reg.h>
#include <rbus/tvsb4_reg.h>
#include <rbus/tvsb5_reg.h>
#include <rbus/tvsb7_reg.h>
#include <rbus/dc_sys_64bit_wrapper_reg.h>
//#include <rbus/crt_reg.h>
#if 0 //code for IB/DC2
#include <rbus/ib_reg.h>
#endif
#include <rbus/vodma_reg.h>

#include <rtd_log/rtd_module_log.h>
#include <mach/pcbMgr.h>
#include <linux/console.h>
#include <linux/sched.h>
#include <linux/freezer.h>

#ifdef CONFIG_RTK_KDRV_WATCHDOG
#include <rtk_kdriver/rtk_watchdog.h>
#endif

#define MISC_TAG "MISC"

#define MISC_DBG(fmt,args...) rtd_pr_misc_debug(fmt, ## args)
#define MISC_INFO(fmt,args...) rtd_pr_misc_info(fmt, ## args)
#define MISC_ERR(fmt,args...) rtd_pr_misc_err(fmt, ## args)
#define MISC_EMERG(fmt,args...) rtd_pr_misc_emerg(fmt, ## args)
//#define MISC_DBG_PRINT(s, args...) rtd_pr_misc_debug(s, ## args)
//#define MISC_DBG_PRINT(s, args...)

#define FRED_START "\033[1;31;40m"
#define FGREEN_START "\033[1;32;40m"
#define FYELLOW_START "\033[1;33;40m"
#define FBLUE_START "\033[1;34;40m"
#define FCOLOR_END "\033[0m"

int misc_major      = 0;
int misc_minor      = 0;

#define REG_BUSH_OC_CONTROL 0xb8000438
#define REG_BUSH_OC_DONE 0xb8000438
#define REG_BUSH_SSC_CONTROL 0xb80003d0
#define REG_BUSH_NCODE_SSC 0xb80003d4
#define REG_BUSH_FCODE_SSC 0xb80003d4
#define REG_BUSH_GRAN_EST 0xb80003d0

#define REG_BUS_OC_CONTROL 0xb8000428
#define REG_BUS_OC_DONE 0xb8000428
#define REG_BUS_SSC_CONTROL 0xb80003c0
#define REG_BUS_NCODE_SSC 0xb80003c4
#define REG_BUS_FCODE_SSC 0xb80003c4
#define REG_BUS_GRAN_EST 0xb80003c0

#define REG_DDR_OC_CONTROL 0xb80be028
#define REG_DDR_OC_DONE 0xb80be028
#define REG_DDR_SSC_CONTROL 0xb80be01c
#define REG_DDR_NCODE_SSC 0xb80be028
#define REG_DDR_FCODE_SSC 0xb80be024
#define REG_DDR_GRAN_EST 0xb80be020

#define REG_DDR2_OC_CONTROL 0xb8004028
#define REG_DDR2_OC_DONE 0xb8004028
#define REG_DDR2_SSC_CONTROL 0xb800401c
#define REG_DDR2_NCODE_SSC 0xb8004028
#define REG_DDR2_FCODE_SSC 0xb8004024
#define REG_DDR2_GRAN_EST 0xb8004020

#define REG_DISPLAY_OC_CONTROL 0xb80006a0
#define REG_DISPLAY_OC_DONE 0xb80006b4
#define REG_DISPLAY_SSC_CONTROL 0xb80006b0
#define REG_DISPLAY_NCODE_SSC 0xb80006ac
#define REG_DISPLAY_FCODE_SSC 0xb80006ac
#define REG_DISPLAY_GRAN_EST 0xb80006b0

#define DC_PHY_EFF_MEAS_CTRL_reg 0xB8008D80
#define DC_PHY_TMCTRL3_reg 0xB800880C
#define DC_PHY_READ_CMD_reg 0xB8008D84
#define DC_PHY_WRITE_CMD_reg 0xB8008D88

#define JTAG_CONTROL_REG 0xB810E004

#define PERIOD 30//60 //period 30 kHz
#define DOT_GRAN 4
#define RTD_LOG_CHECKING_REGISTER 0xb801a610
#define RTD_LOG_CHECK_REG 0x100

#define IB_CLIENTS_NUM 13
#define IB_CLIENTS_REG_ADDRESS_OFFSET 0x100
#define IB_TRAP_CASE_NUMBER 11
#define IB_ERR_STATUS_NUMBER 32

typedef enum DC_ID {
        DC_ID_1 = 0,
        //DC_ID_2 = 1,
        DC_NUMBER,
} DC_ID;

typedef enum DC_SYS_ID {
        DC_SYS1 = 0,
        DC_SYS2 = 1,
        DC_SYS3 = 2,
        DC_SYS_NUMBER,
} DC_SYS_ID;

static struct class *misc_class;
static struct misc_dev *misc_drv_dev;                           /* allocated in misc_init_module */
extern void msleep(unsigned int msecs);
extern void dump_stacks (void);
extern u32 gic_irq_find_mapping(u32 hwirq);
static void rtk_ddr_debug_resume(void);

//extern unsigned int console_rtdlog_level;
//extern unsigned int console_rtdlog_module;
extern rtk_rtdlog_info_t console_rtdlog;

__maybe_unused int is_in_misc_isr = 0;

typedef struct DC_ERRCMD_REGS {
        unsigned int addcmd_hi[DC_NUMBER][DC_SYS_NUMBER];
        unsigned int addcmd_lo[DC_NUMBER][DC_SYS_NUMBER];
        unsigned int dc_int[DC_NUMBER][DC_SYS_NUMBER];
} DC_ERRCMD_REGS;

DC_ERRCMD_REGS dcsys_errcmd_regs;

/*
static void  rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
        unsigned int X, A, result;
        X = (1 << (endBit - startBit + 1)) - 1;
        A = rtd_inl(reg_addr);
        result = (A & (~(X << startBit))) | (value << startBit);
        MISC_INFO("[%s]origin=0x%x,startBit=%d, endBit=%d, value=0x%x\n", __FUNCTION__, A, startBit, endBit, value);
        MISC_INFO("[%s]reg=0x%x, value=0x%x\n", __FUNCTION__, reg_addr, result);
        rtd_outl(reg_addr, result);

}*/

#define MODULE_STR_SIZE    (8)
static bool StrToHex(unsigned char* pu8string, unsigned int u32Size, unsigned int* pu32Value)
{
        unsigned int u32Value = 0;
        unsigned int i;
        unsigned char u8Data[MODULE_STR_SIZE];
        bool bRet = true;

        if (u32Size > MODULE_STR_SIZE) {
                u32Size = MODULE_STR_SIZE;
        }
        for (i = 0; i < u32Size; i++) {
                //Covert 'A' to 'a'
                if ((pu8string[i] >= 'A') && (pu8string[i] <= 'F')) {
                        u8Data[i] = pu8string[i] - 'A' + 'a';
                } else {
                        u8Data[i] = pu8string[i];
                }
        }

        for (i = 0; i < u32Size; i++) {
                //printf("u8Data[%d]: %d.\n", i, u8Data[i]);
                if ((u8Data[i] >= '0') && (u8Data[i] <= '9')) {
                        u32Value *= 16;
                        u32Value += (u8Data[i] - '0');
                } else if ((u8Data[i] >= 'a') && (u8Data[i] <= 'f')) {
                        u32Value *= 16;
                        u32Value += (u8Data[i] + 10 - 'a');
                } else if (u8Data[i] != '\n') {
                        bRet = false;
                }
        }
        if (bRet) {
                *pu32Value = u32Value;
        }
        // MISC_INFO("[MISC][StrToHex]%d,leng=%s,%s, ===> %d\n", bRet,u32Size ,u8Data, *pu32Value  );

        return bRet;
}



static unsigned int DDR_period = PERIOD;
static RHAL_SYS_SPREAD_SPECTRUM_RATIO_T DDR_nRatio[RHAL_SYS_SPREAD_SPECTRUM_MODULE_MAX] =
        {
                RHAL_SYS_SPREAD_SPECTRUM_OFF,
#ifdef SUPPORT_DCU2
                RHAL_SYS_SPREAD_SPECTRUM_OFF
#endif
         };

static unsigned int DDR1_freq_old = 0;
static unsigned int DDR1_N_CODE = 0;
static unsigned int DDR1_F_CODE = 0;
static unsigned int DDR1_freq_new = 0;
#ifdef SUPPORT_DCU2
static unsigned int DDR2_freq_old = 0;
static unsigned int DDR2_N_CODE = 0;
static unsigned int DDR2_F_CODE = 0;
static unsigned int DDR2_freq_new = 0;
#endif
static unsigned int getSSCParameter(unsigned int inputFreq, unsigned int inRatio,
                                    unsigned int *Ncode_ssc,  unsigned int *Fcode_ssc, unsigned int *Gran_est)
{
        unsigned int M_target = 0;
        unsigned int target_F_Code = 0;
        unsigned int ssc_clock = 0;
        unsigned int step = 0;
        unsigned int ret_Ncode_ssc = 0;
        unsigned int ret_Fcode_ssc = 0;
        unsigned int ret_Gran_est = 0;
        unsigned int tempValue = 0;
        unsigned int tempValue3 = 0;
        unsigned int tempValue2 = 0;
        unsigned int temp_a = 0;
        unsigned int temp_b = 0;

        M_target = (inputFreq / 27) - 3;

        tempValue = ((100000 * inputFreq) / 27) % 100000;
        target_F_Code = (tempValue * 2048) / 100000;

        ret_Ncode_ssc = ((inputFreq * (10000 - inRatio)) / 27) / 10000 - 3;

        tempValue = ((inputFreq * (10000 - inRatio)) / 270) % 1000;
        ret_Fcode_ssc = (tempValue * 2048) / 1000;

        tempValue2 = (inputFreq * (10000 - inRatio / 2));
        tempValue = (((M_target - ret_Ncode_ssc) * 2048 + (target_F_Code - ret_Fcode_ssc)) / 2 / 2048) + (ret_Ncode_ssc + 3);
        temp_a = inputFreq * 10000;
        temp_b = inputFreq * (inRatio / 2);
        tempValue3 = (temp_a - temp_b);
        ssc_clock = tempValue3 / tempValue / 10;
        step = ssc_clock / DDR_period;

        tempValue2 = ((M_target - ret_Ncode_ssc) * 2048 + (target_F_Code - ret_Fcode_ssc));
        tempValue = (1000 * (tempValue2 * 2)) / step;
        tempValue2 = 1 << (15 - DOT_GRAN);
        ret_Gran_est = (tempValue * tempValue2) / 1000; //pow(2,15-DOT_GRAN);

        if(Ncode_ssc != NULL)
                *Ncode_ssc = ret_Ncode_ssc;
        if(Fcode_ssc != NULL)
                *Fcode_ssc = ret_Fcode_ssc;
        if(Gran_est != NULL)
                *Gran_est = ret_Gran_est;

        MISC_INFO("[%s] M_target=%d, target_F_Code=%d\n", __FUNCTION__, M_target, target_F_Code);
        MISC_INFO("[%s] ssc_clock=%d, step=%d\n", __FUNCTION__, ssc_clock, step);
        MISC_INFO("[%s] input(%d,%d),output(%d,,%d,,%d,)\n", __FUNCTION__, inputFreq, inRatio, ret_Ncode_ssc, ret_Fcode_ssc, ret_Gran_est);

        return 1;

}

static unsigned int getDDR1Frequency(void)//reference from bootcode:/common/cmd_dssinfo.c  pll_ddr_freq()
{
        unsigned int regValueForN = rtd_inl(REG_DDR_NCODE_SSC);
        unsigned int regValueForF = rtd_inl(REG_DDR_FCODE_SSC);
        unsigned int N_CODE, F_CODE, FOUT;
        N_CODE = (regValueForN & 0xFF00) >> 8;
        F_CODE = (regValueForF & 0x7FF0000) >> 16;

        FOUT = (unsigned int)(((27 * N_CODE + 27 * 3 + (27 * F_CODE) / 2048)));

        MISC_INFO("[%s] regvalueForN= 0x%x regvalueForF= 0x%x ==>N:%d, F=%d , out=%d\n", __FUNCTION__, regValueForN, regValueForF, N_CODE, F_CODE, FOUT);
        return FOUT;
}

static void setDDR1Frequency(unsigned int fin)//reference from bootcode:/common/cmd_dssinfo.c  pll_ddr_freq()
{
        unsigned int regValueForN = 0;
        unsigned int regValueForF = 0;

        unsigned int N_CODE = fin/27 - 3;
        unsigned int F_CODE = (fin%27 * 10000)/27*2048/10000 ;

        regValueForN = (N_CODE&0xFF) << 8;
        regValueForF = ((F_CODE & 0x7FF) <<16);
        rtd_maskl(REG_DDR_NCODE_SSC, 0xFFFF00FF, regValueForN);
        rtd_maskl(REG_DDR_FCODE_SSC, 0xF800FFFF, regValueForF);

        MISC_INFO("[%s] regvalueForN= 0x%x regvalueForF= 0x%x ==>N:%d, F=%d , in=%d\n", __FUNCTION__, regValueForN, regValueForF, N_CODE, F_CODE, fin);
        return;
}

#ifdef SUPPORT_DCU2
static unsigned int getDDR2Frequency(void)//reference from bootcode:/common/cmd_dssinfo.c  pll_ddr_freq()
{
        unsigned int regValueForN = rtd_inl(REG_DDR2_NCODE_SSC);
        unsigned int regValueForF = rtd_inl(REG_DDR2_FCODE_SSC);
        unsigned int N_CODE, F_CODE, FOUT;
        N_CODE = (regValueForN & 0xFF00) >> 8;
        F_CODE = (regValueForF & 0x7FF0000) >> 16;

        FOUT = (unsigned int)(((27 * N_CODE + 27 * 3 + (27 * F_CODE) / 2048)));

        MISC_INFO("[%s] regvalueForN= 0x%x regvalueForF= 0x%x ==>N:%d, F=%d , out=%d\n", __FUNCTION__, regValueForN, regValueForF, N_CODE, F_CODE, FOUT);
        return FOUT;
}

static void setDDR2Frequency(unsigned int fin)//reference from bootcode:/common/cmd_dssinfo.c  pll_ddr_freq()
{
        unsigned int regValueForN = 0;
        unsigned int regValueForF = 0;

        unsigned int N_CODE = fin/27 - 3;
        unsigned int F_CODE = (fin%27 * 10000)/27*2048/10000 ;

        regValueForN = (N_CODE&0xFF) << 8;
        regValueForF = ((F_CODE & 0x7FF) <<16);
        rtd_maskl(REG_DDR2_NCODE_SSC, 0xFFFF00FF, regValueForN);
        rtd_maskl(REG_DDR2_FCODE_SSC, 0xF800FFFF, regValueForF);

        MISC_INFO("[%s] regvalueForN= 0x%x regvalueForF= 0x%x ==>N:%d, F=%d , in=%d\n", __FUNCTION__, regValueForN, regValueForF, N_CODE, F_CODE, fin);
        return;
}

#endif

static DEFINE_SPINLOCK(spread_spectrum_lock);
//-----------------------------------------------------
//
//
//-----------------------------------------------------
int RHAL_SYS_SetDDRSpreadSpectrum(RHAL_SYS_SPREAD_SPRECTRUM_MODULE_T module, RHAL_SYS_SPREAD_SPECTRUM_RATIO_T nRatio)
{
        unsigned int NCode_ssc = 0;
        unsigned int FCode_ssc = 0;
        unsigned int Gran_est = 0;
        unsigned int freq_MHz = 0;
        unsigned int waitBit = 0;
        unsigned long flag;
        int ret = -1;

        spin_lock_irqsave(&spread_spectrum_lock, flag);
        if(module == RHAL_SYS_SPREAD_SPECTRUM_MODULE_DDR)
       {
                rtd_part_outl(REG_DDR_SSC_CONTROL, 1, 1, 0x0);//En_ssc =0
                if(DDR1_freq_old == 0)
                {
                        DDR1_N_CODE = (rtd_inl(REG_DDR_NCODE_SSC) & 0xFF00) >> 8; // save ddr freq N_CODE
                        DDR1_F_CODE = (rtd_inl(REG_DDR_FCODE_SSC) & 0x7FF0000) >> 16; // save ddr freq F_CODE
                        DDR1_freq_old = (unsigned int)(((27 * DDR1_N_CODE + 27 * 3 + (27 * DDR1_F_CODE) / 2048)));
                }

                if(nRatio == RHAL_SYS_SPREAD_SPECTRUM_OFF)
                {
                        rtd_maskl(REG_DDR_NCODE_SSC, 0xFFFF00FF, ((DDR1_N_CODE&0xFF) << 8));
                        rtd_maskl(REG_DDR_FCODE_SSC, 0xF800FFFF, ((DDR1_F_CODE & 0x7FF) <<16));

                        //rtd_part_outl(REG_DDR_SSC_CONTROL, 1, 1, 0x0);//En_ssc =0
                        spin_unlock_irqrestore(&spread_spectrum_lock, flag);
                        return 0;
                }
                else if(nRatio <= RHAL_SYS_SPREAD_SPECTRUM_RATIO_6_00){
                        rtd_maskl(REG_DDR_NCODE_SSC, 0xFFFF00FF, ((DDR1_N_CODE&0xFF) << 8));
                        rtd_maskl(REG_DDR_FCODE_SSC, 0xF800FFFF, ((DDR1_F_CODE & 0x7FF) <<16));
                        nRatio = nRatio* 25;
                }
                else if(nRatio <RHAL_SYS_SPREAD_SPECTRUM_RATIO_MAX)
                {
                        nRatio = (nRatio - RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_25 + 1)*25;
                        DDR1_freq_new = DDR1_freq_old*(nRatio +10000)/10000;
                        setDDR1Frequency(DDR1_freq_new);
                        nRatio = nRatio *2;
                }
                /******************************DCU1********************************/
                //Step 1. Get frequnecy
                freq_MHz = getDDR1Frequency();

                //Step 2. Get Ncode_ssc, Fcode_ssc and Gran
                getSSCParameter(freq_MHz, nRatio, &NCode_ssc, &FCode_ssc, &Gran_est);
                //Step 3. Write to register
                //Step 3.1 Disable ssc
                rtd_part_outl(REG_DDR_SSC_CONTROL, 3, 3, 0x1);//dpi_ssc_flag_init =1
                rtd_part_outl(REG_DDR_SSC_CONTROL, 1, 1, 0x0);//En_ssc =0
                //Step 3.2 Disable oc
                rtd_part_outl(REG_DDR_OC_CONTROL, 26, 26, 0);//oc_en =0
                //Step 3.3 Set Ncode_ssc, Fcode_ssc
                rtd_part_outl(REG_DDR_NCODE_SSC, 7, 0, NCode_ssc);//Ncode_ssc
                rtd_part_outl(REG_DDR_FCODE_SSC, 10, 0, FCode_ssc);//Fcode_ssc
                //Step 3.4 Set Gran_est
                rtd_part_outl(REG_DDR_GRAN_EST, 22, 20, DOT_GRAN);//Gran_set
                rtd_part_outl(REG_DDR_GRAN_EST, 18, 0, Gran_est);//Gran_set
                //Step 3.5 Enable oc
                rtd_part_outl(REG_DDR_OC_CONTROL, 26, 26, 1);//oc_en =1

                waitBit = rtd_inl(REG_DDR_OC_DONE);
                while((waitBit & 0x4000000) == 0) {
                        waitBit = rtd_inl(REG_DDR_OC_DONE); // polling for oc_done
                        rtd_pr_misc_err("waiting.....DDR  =0x%x\n", waitBit);
                }
                //Step 3.6 Disable oc
                rtd_part_outl(REG_DDR_OC_CONTROL, 26, 26, 0);//oc_en =0
                //Step 3.7 Enable ssc
                rtd_part_outl(REG_DDR_SSC_CONTROL, 1, 1, 0x1);//En_ssc =1
                //Step 4.End
                ret = 0;
        }
#ifdef SUPPORT_DCU2
        else if(module == RHAL_SYS_SPREAD_SPECTRUM_MODULE_DDR2)
        {
                rtd_part_outl(REG_DDR2_SSC_CONTROL, 1, 1, 0x0);//En_ssc =0
                if(DDR2_freq_old == 0)
                {
                        DDR2_N_CODE = (rtd_inl(REG_DDR2_NCODE_SSC) & 0xFF00) >> 8; // save ddr freq N_CODE
                        DDR2_F_CODE = (rtd_inl(REG_DDR2_FCODE_SSC) & 0x7FF0000) >> 16; // save ddr freq F_CODE
                        DDR2_freq_old = (unsigned int)(((27 * DDR2_N_CODE + 27 * 3 + (27 * DDR2_F_CODE) / 2048)));
                }

                if(nRatio == RHAL_SYS_SPREAD_SPECTRUM_OFF)
                {
                        rtd_maskl(REG_DDR2_NCODE_SSC, 0xFFFF00FF, ((DDR2_N_CODE&0xFF) << 8));
                        rtd_maskl(REG_DDR2_FCODE_SSC, 0xF800FFFF, ((DDR2_F_CODE & 0x7FF) <<16));
                        //rtd_part_outl(REG_DDR2_SSC_CONTROL, 1, 1, 0x0);//En_ssc =0
                        spin_unlock_irqrestore(&spread_spectrum_lock, flag);
                        return 0;
                }
                else if(nRatio <= RHAL_SYS_SPREAD_SPECTRUM_RATIO_6_00){
                        rtd_maskl(REG_DDR2_NCODE_SSC, 0xFFFF00FF, ((DDR2_N_CODE&0xFF) << 8));
                        rtd_maskl(REG_DDR2_FCODE_SSC, 0xF800FFFF, ((DDR2_F_CODE & 0x7FF) <<16));
                        nRatio = nRatio* 25;
                }
                else if(nRatio <RHAL_SYS_SPREAD_SPECTRUM_RATIO_MAX)
                {
                        nRatio = (nRatio - RHAL_SYS_SPREAD_SPECTRUM_RATIO_up_0_25 + 1)*25;
                        DDR2_freq_new = DDR2_freq_old*(nRatio +10000)/10000;
                        setDDR2Frequency(DDR2_freq_new);
                        nRatio = nRatio *2;
                }

                /******************************DCU2********************************/
                freq_MHz = getDDR2Frequency();
                //Step 2. Get Ncode_ssc, Fcode_ssc and Gran
                getSSCParameter(freq_MHz, nRatio, &NCode_ssc, &FCode_ssc, &Gran_est);
                //Step 3. Write to register
                //Step 3.1 Disable ssc
                rtd_part_outl(REG_DDR2_SSC_CONTROL, 3, 3, 0x1);////dpi_ssc_flag_init =1
                rtd_part_outl(REG_DDR2_SSC_CONTROL, 1, 1, 0x0);//////En_ssc =0
                //Step 3.2 Disable oc
                rtd_part_outl(REG_DDR2_OC_CONTROL, 26, 26, 0);//oc_en =0
                //Step 3.3 Set Ncode_ssc, Fcode_ssc
                rtd_part_outl(REG_DDR2_NCODE_SSC, 7, 0, NCode_ssc);//Ncode_ssc
                rtd_part_outl(REG_DDR2_FCODE_SSC, 10, 0, FCode_ssc);//Fcode_ssc
                //Step 3.4 Set Gran_est
                rtd_part_outl(REG_DDR2_GRAN_EST, 22, 20, DOT_GRAN);//Gran_set
                rtd_part_outl(REG_DDR2_GRAN_EST, 18, 0, Gran_est);//Gran_set
                //Step 3.5 Enable oc
                rtd_part_outl(REG_DDR2_OC_CONTROL, 26, 26, 1);//oc_en =1

                waitBit = 0;
                waitBit = rtd_inl(REG_DDR2_OC_DONE);
                while((waitBit & 0x4000000) == 0) {
                        waitBit = rtd_inl(REG_DDR2_OC_DONE); // polling for oc_done
                        rtd_pr_misc_err("waiting.....DDR2  =0x%x\n", waitBit);
                }
                //Step 3.6 Disable oc
                rtd_part_outl(REG_DDR2_OC_CONTROL, 26, 26, 0);//oc_en =0
                //Step 3.7 Enable ssc
                rtd_part_outl(REG_DDR2_SSC_CONTROL, 1, 1, 0x1);//En_ssc =1
                //Step 4.End
                ret = 0;
        }
#endif

        spin_unlock_irqrestore(&spread_spectrum_lock, flag);
        return ret;
}

/*
 * Open and close
 */

int misc_open(
        struct inode*           inode,
        struct file*            filp
)
{
        struct misc_dev *pdev; /* device information */

        pdev = container_of(inode->i_cdev, struct misc_dev, cdev);
        filp->private_data = pdev; /* for other methods */

        MISC_DBG("misc open\n");
        return 0;
}

int misc_release(
        struct inode*           inode,
        struct file*            filp
)
{
        filp->private_data = NULL;

        MISC_DBG("misc release\n");
        return 0;
}

#ifdef CONFIG_RTK_KDRV_R8168
static struct task_struct *thread_wol, *thread_wolether;
static int wol_polling(void *arg);
static int wolether_polling(void *arg);
extern struct platform_device *network_devs;
extern int rtl8168_suspend(struct platform_device *dev, pm_message_t state);
extern int rtl8168_resume(struct platform_device *dev);

static int wol_polling(void *arg)
{
        pm_message_t pmstate = {0};

        rtl8168_suspend(network_devs, pmstate);
        MISC_INFO("ether driver suspend for WOL\n");
        // Reset WOL Pin to high for the falling edge trigger of the ext. micom
        rtd_outl(0xb8061108, (rtd_inl(0xb8061108) | (1 << 6)));
        // Clear WOL Flag
        rtd_outl(0xb80160b0, 0xf0031e00);  //0xf0031e00
        rtd_outl(0xb806050c, 0xbeef7777);
        while(!kthread_should_stop()) {
                msleep(300);
                rtd_outl(0xb80160b0, 0x70030000);  //0xf0031e00
                MISC_INFO("%08x\n", rtd_inl(0xb80160b0));   // check 0x1e00(default)  -->WOL 0x9e00
                MISC_INFO("%08x\n", rtd_inl(0xb8061108));
                if ( (rtd_inl(0xb80160b0) & 0xffff) == 0x9e00)
                        break;
                if ( rtd_inl(0xb806050c) == 0xbeef9999)
                        break;
        }
        MISC_INFO("WOL!! wol_polling thread break\n");
        if ( rtd_inl(0xb806050c) != 0xbeef9999) {
                MISC_INFO("Set WOL Pin to low for the falling edge trigger of the ext. micom!\n");
                MISC_INFO("%08x\n", rtd_inl(0xb8061100));
                MISC_INFO("%08x\n", rtd_inl(0xb8061108));
                // Set WOL Pin to low for the falling edge trigger of the ext. micom
                rtd_outl(0xb8061108, (rtd_inl(0xb8061108) & ~(1 << 6)));
                MISC_INFO("%08x\n", rtd_inl(0xb8061108));
        }
        // Resume
        rtl8168_resume(network_devs);
        MISC_INFO("ether driver resume for WOL\n");
        rtd_outl(0xb806050c, 0x0);
        MISC_INFO("stop wol_polling thread!\n");
        kthread_stop(thread_wol);

        return 0;
}

#if 1
static int wolether_polling(void *arg)
{
        //pm_message_t pmstate;

        while(!kthread_should_stop()) {
                msleep(300);
                if ( rtd_inl(0xb8060110) == 0x9021affe )
                        break;
                if ( rtd_inl(0xb8060110) == 0xbeef8888 )
                        break;
        }
        if ( rtd_inl(0xb8060110) == 0xbeef8888 ) {
                MISC_INFO("wolether_polling thread break(magic:%08x)\n", rtd_inl(0xb8060110));
                MISC_INFO("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
#if 0
                /* Reset Geth IP */
                MISC_INFO("Reset Geth IP!!\n");
                rtd_outl(0xb8016037, rtd_inl(0xb8016037) | 0x10); // BIT4
                MISC_INFO("checking rtd_inl(0xb8016037)=%08x \n", rtd_inl(0xb8016037));
                mdelay(50);
                // enable WOL IP setting for MAC
                MISC_INFO("enable WOL IP setting for MAC!!!\n");
                rtd_outl(0xb8016050, 0x1dcfc010);
                MISC_INFO("rtd_inl(0xb8016050)=%08x\n", rtd_inl(0xb8016050));
                mdelay(50);
                //rtd_outl(0xb80160d0, 0xb2000021);
                rtd_outl(0xb80160d0, 0x22000021); // fix auto wakeup issue
                MISC_INFO("rtd_inl(0xb80160d0)=%08x\n", rtd_inl(0xb80160d0));
                mdelay(50);
                rtd_outl(0xb8016050, 0x1dcf0010);
                MISC_INFO("rtd_inl(0xb8016050)=%08x\n", rtd_inl(0xb8016050));
                mdelay(50);
                // 0x18016c10[0] = 0x1 (default =0) before enable emcu clock
                // gmac clock select  0: gmac/emcu2 use bus clock 1: gmac/emcu2 use GPHY125MHz clock
                rtd_outl(0xb8016c10, (rtd_inl(0xb8016c10) & 0xfffffffe) | 0x1);
                mdelay(50);
                rtd_inl(0xb8016c10); // for asynchronous access
#endif
                //rtl8168_suspend(network_devs, pmstate);
                //MISC_INFO("ether driver suspend for WOL\n");
                rtd_outl(0xb8060110, 0xbeef6666);
        }
        MISC_INFO("stop wolether_polling thread!\n");
        kthread_stop(thread_wolether);

        return 0;
}
#endif
#endif
/*
 * The QSM Off implementation: tunning OSC & enable OSC
 */
void MISC_OSC_Clk_init(void)
{
    //   rtd_outl(0x0044, rtd_inl(0x0044)|_BIT11); //emb osc clock enable
    rtd_outl(0xb8060044, _BIT11 | _BIT0);

    //   rtd_outl(0x0044, rtd_inl(0x0044)&~_BIT11); //emb osc clock disable
    rtd_outl(0xb8060044, _BIT11);

    //   rtd_outl(0x0034, rtd_inl(0x0034)&~_BIT12); //hold osc reset
    rtd_outl(0xb8060034, _BIT12);

    //   rtd_outl(0x0034, rtd_inl(0x0034)|_BIT12); //release osc reset
    rtd_outl(0xb8060034, _BIT12 | _BIT0);

    //delayms(0x1); //delay at least > 150us
    udelay(200);
    //   rtd_outl(0x0044, rtd_inl(0x0044)|_BIT11); //emb osc clock enable
    rtd_outl(0xb8060044, _BIT11 | _BIT0);
}

void MISC_OSC_tracking(void)
{
    rtd_outl(0xb8060058, rtd_inl(0xb8060058)|_BIT1);// select external 27Mhz
    //rtd_setBits(0x0058, _BIT1);

    rtd_outl(0xb80600d0, rtd_inl(0xb80600d0)&~_BIT1);// auto mode
    //rtd_clearBits(0x00d0, _BIT1);

    rtd_outl(0xb80600d0, rtd_inl(0xb80600d0)|_BIT3);// osc_track_lock_en
    //rtd_setBits(0x00d0, _BIT3);

    rtd_outl(0xb80600d0, rtd_inl(0xb80600d0)|_BIT2);// osc_calibratable_en
    //rtd_setBits(0x00d0, _BIT2);

    rtd_outl(0xb80600d0, rtd_inl(0xb80600d0)|_BIT0);// Osc tracking enable
    //rtd_setBits(0x00d0, _BIT0);

    //    while(!(rtd_inl(0xb80600d4) & _BIT3));//polling for osc_track_lock_status=1
    while(!(rtd_inl(0xb80600d4) & _BIT3));//polling for osc_track_lock_status=1

    rtd_outl(0xb80600d0, rtd_inl(0xb80600d0)&~_BIT0);// Osc tracking disable
    //rtd_clearBits(0x00d0, _BIT0);

    rtd_outl(0xb8060058, rtd_inl(0xb8060058)&~_BIT1);// select ring_osc
    //rtd_clearBits(0x0058, _BIT1);
}

/*
 * The ioctl() implementation
 */

long misc_ioctl(
        struct file             *filp,
        unsigned int            cmd,
        unsigned long           arg)
{
        struct misc_dev *pdev = filp->private_data;
        int retval = 0;
        unsigned int i;
        pm_message_t pmstate = {0};

        if (down_interruptible(&pdev->sem))
                return -ERESTARTSYS;

        switch(cmd) {
                case MISC_WAKEUP_ECPU: {
                       MISC_INFO( "Wake-up ECPU...\n");
                       MISC_INFO("289x rtd_inl(0xb8060110)=%08x\n", rtd_inl(0xb8060110));
                       if(rtd_inl(0xb8060110) == 0x9021aebe) {  // sync system timer between A/V CPU
                           rtd_outl(0xb8060110, 0);
                           rtd_outl(STB_ST_SRST1_reg, BIT(9));
														}
                       rtd_outl(STB_ST_CLKEN1_reg, BIT(9) | BIT(0));       // clk enable
                       udelay(500);
                       rtd_outl(STB_ST_SRST1_reg, BIT(9) | BIT(0));
						                }
                break;

                case MISC_SET_WOL_ENABLE: {
                        int nEnable = arg;
			//pm_message_t pmstate;

#ifdef CONFIG_RTK_KDRV_R8168

                        if(nEnable) {
                                // stop wol_polling thread and resume ether driver in case of Warm standby mode is on
                                if ( rtd_inl(0xb806050c) == 0xbeef7777 ) {
                                MISC_INFO( "Enter WOL flow in Warm standby mode...\n");
                                rtd_outl(0xb80160b0, 0x70030000);  //0xf0031e00
                                        // set WOL flag disabled for emcu
                                        MISC_INFO("set WOL flag disabled for emcu\n");
                                        rtd_outl(0xb8060500, (rtd_inl(0xb8060500) & 0xfeffffff));
                                        // stop wol_polling thread and resume ether driver in case of Warm standby mode is on
                                        rtd_outl(0xb806050c, 0xbeef9999);
                                        while ( rtd_inl(0xb806050c) == 0xbeef9999 ) {
                                                MISC_INFO( "stop wol_polling thread waiting! %x\n", rtd_inl(0xb806050c));
                                                msleep(300);
                                        }
						         }
						         else {
                                         if(rtd_inl(0xb8060110) ==0x9021affa) {
		                                  MISC_INFO( "Enter twice WOL flow...\n");
										  break;
								       }
								  MISC_INFO( "Enable WOL...\n");
								  rtd_outl(0xb80160b0, 0x70030000);  //0xf0031e00	                                    MISC_INFO( "Enter WOL flow...\n");
	                                MISC_INFO( "Enter WOL flow...\n");
                                        MISC_INFO( "%s: WOL:%08x\n", __FUNCTION__, rtd_inl(0xb80160b0));   // check 0x1e00(default)  -->WOL 0x9e00
                                        MISC_INFO( "warm standby wol_polling thread does not exist!\n");
                                }
#if 1
#if 0
                                /* Reset Geth IP */
                                MISC_INFO("Reset Geth IP!!\n");
                                rtd_outl(0xb8016037, 0x10); // BIT4
                                MISC_INFO("checking rtd_inl(0xb8016037)=%08x \n", rtd_inl(0xb8016037));
                                mdelay(50);
                                // enable WOL IP setting for PHY
#if 0
                                MISC_INFO("enable WOL IP setting for PHY!!!\n");
                                rtd_outl(0xb8016060, 0x841f0bc4);       // Goto to Page bc4
                                MISC_INFO("rtd_inl(0xb8016060)=%08x\n", rtd_inl(0xb8016060));
                                rtd_outl(0xb8016060, 0x84150203);       // set internal LDO turn on
                                MISC_INFO("rtd_inl(0xb8016060)=%08x\n", rtd_inl(0xb8016060));
                                udelay(500);
#endif
                                // enable WOL IP setting for MAC
                                MISC_INFO("enable WOL IP setting for MAC!!!\n");
                                rtd_outl(0xb8016050, 0x1dcfc010);
                                MISC_INFO("rtd_inl(0xb8016050)=%08x\n", rtd_inl(0xb8016050));
                                mdelay(50);
                                //rtd_outl(0xb80160d0, 0xb2000021);
                                rtd_outl(0xb80160d0, 0x22000021); // fix auto wakeup issue
                                MISC_INFO("rtd_inl(0xb80160d0)=%08x\n", rtd_inl(0xb80160d0));
                                mdelay(50);
                                rtd_outl(0xb8016050, 0x1dcf0010);
                                MISC_INFO("rtd_inl(0xb8016050)=%08x\n", rtd_inl(0xb8016050));
                                mdelay(50);
#endif
                                MISC_INFO("289x  rtd_inl(0xb8060110)=%08x\n", rtd_inl(0xb8060110));

                                if((rtd_inl(0xb8060110) == 0x9021aebe) || (rtd_inl(0xb8060110) == 0x9021affc)) {
                                        rtd_outl(STB_ST_SRST1_reg, BIT(9));
                                }
                                // clear STR flag for emcu
                                //rtd_outl(0xb8060110, 0x00000000);
                                // clear share memory for emcu
#define RTD_SHARE_MEM_LEN       32
#define RTD_SHARE_MEM_BASE      0xb8060500
                                MISC_INFO("clear share memory in %s...\n", __FUNCTION__);
                                for(i = 0; i < RTD_SHARE_MEM_LEN; i++)
                                        rtd_outl(RTD_SHARE_MEM_BASE + (4 * i), 0);
                                // set WOL flag enabled for emcu
                                MISC_INFO("set WOL flag enabled for emcu\n");
                                rtd_outl(0xb8060500, (rtd_inl(0xb8060500) & 0xfeffffff) | 0x01000000);
                                // 0x18016c10[0] = 0x1 (default =0) before enable emcu clock
                                // gmac clock select  0: gmac/emcu2 use bus clock 1: gmac/emcu2 use GPHY125MHz clock
                                rtd_outl(0xb8016c10, (rtd_inl(0xb8016c10) & 0xfffffffe) | 0x1);
                                mdelay(50);
                                rtd_inl(0xb8016c10); // for asynchronous access
                                // run a thread for WOL polling
                                thread_wolether = kthread_run(wolether_polling, NULL, "wolether_polling");
                                if(IS_ERR(thread_wolether)) {
                                        MISC_INFO("create wolether_polling thread failed\n");
                                        retval = -EFAULT;
                                        break;
																				 }
				MISC_INFO("create wolether_polling thread successfully\n");

				MISC_INFO("ether driver suspend for WOL before\n");

				rtl8168_suspend(network_devs, pmstate);
				MISC_INFO("ether driver suspend for WOL done\n");
#else
                                // clear STR flag for emcu
                                //rtd_outl(0xb8060110, 0x00000000);
                                // clear share memory for emcu
#define RTD_SHARE_MEM_LEN       32
#define RTD_SHARE_MEM_BASE      0xb8060500
                                MISC_INFO("clear share memory in %s...\n", __FUNCTION__);
                                for(i = 0; i < RTD_SHARE_MEM_LEN; i++)
                                        rtd_outl(RTD_SHARE_MEM_BASE + (4 * i), 0);
                                // set WOL flag enabled for emcu
                                MISC_INFO("set WOL flag enabled for emcu\n");
                                rtd_outl(0xb8060500, (rtd_inl(0xb8060500) & 0xfeffffff) | 0x01000000);
                                // run a thread for WOL polling
                                //thread_wolether = kthread_run(wolether_polling, NULL, "wolether_polling");
                                //if(IS_ERR(thread_wolether)) {
                                //        MISC_INFO("create wolether_polling thread failed\n");
                                //        retval = -EFAULT;
                                //        break;
 																				//}
                                //MISC_INFO("create wolether_polling thread successfully\n");
#endif
                                // enable emcu clock
                             MISC_INFO( "Wake-up ECPU...\n");
                             MISC_INFO("289x  rtd_inl(0xb8060110)=%08x\n", rtd_inl(0xb8060110));
                             rtd_outl(0xb8060110, 0x9021affa);
                             rtd_outl(STB_ST_CLKEN1_reg, BIT(9) | BIT(0));  // clk enable for emcu
                             udelay(500);
                             rtd_outl(STB_ST_SRST1_reg, BIT(9) | BIT(0));        // release reset for emcu
                        } else {
                                MISC_INFO( "Disable WOL...\n");
                                // clear STR flag for emcu
                                rtd_outl(0xb8060110, 0x00000000);
																				rtd_outl(STB_ST_SRST1_reg, _BIT9);//reset 8051
                                // clear share memory for emcu
                                MISC_INFO("clear share memory in %s...\n", __FUNCTION__);
                                for(i = 0; i < RTD_SHARE_MEM_LEN; i++)
                                        rtd_outl(RTD_SHARE_MEM_BASE + (4 * i), 0);
                                // set WOL flag disabled for emcu
                                MISC_INFO("set WOL flag disabled for emcu\n");
                                rtd_outl(0xb8060500, (rtd_inl(0xb8060500) & 0xfeffffff));
                                // disable emcu clock
                                //rtd_outl(STB_ST_CLKEN1_reg, rtd_inl(STB_ST_CLKEN1_reg) & ~BIT(9));    // clk disable for emcu
                                //udelay(500);
                                //rtd_outl(STB_ST_SRST1_reg, rtd_inl(STB_ST_SRST1_reg) & ~BIT(9));  // hold reset for emcu
                        }
#endif
                }
                break;
                case MISC_GET_WOL_ENABLE: {
                        int nEnable;

                        if((rtd_inl(0xb8060500) & 0x01000000) != 0)
                                nEnable = true;
                        else
                                nEnable = false;

                        if(copy_to_user((void __user *)arg, (void *)&nEnable, sizeof(int))) {
                                retval = -EFAULT;
                                break;
                        }
                }
                break;
#ifdef CONFIG_RTK_KDRV_RTICE
                case MISC_SET_DEBUG_LOCK: {

                        int nLock = arg;
                        extern  UINT8 g_ByPassRTICECmd;

                        if(nLock) {
                                //pin switch to uart2
                                //according to KTASKWBS-2863, always enable log.
                                //rtd_maskl(0xb806020C, 0xFF0FFFFF, 0x00600000);//06: uart2_rXdi_src0,<I> 0x1800_020C[23:20]
                                //  rtd_maskl(0xb806020C, 0x0FFFFFFF, 0x60000000);//06: uart2_tXdo,<O> 0x1800_020C[31:28]

                                //disable JTAG debug
                                rtd_maskl(JTAG_CONTROL_REG, 0xFFFFFFEF, 0x00000010);//ejtag_gpu_bist    set 1 to mux out gpu mbist ejtag port

                                //disable connect RTICE
                                g_ByPassRTICECmd = 1;
                        } else {
                                //enable JTAG debug
                                //pin switch to uart1
                                //according to KTASKWBS-2863, always enable log.
                                //rtd_maskl(0xb806020C, 0xFF0FFFFF, 0x00900000);//09: uart0_rXdi_src0,<I> 0x1806_020C[23:20]
                                //  rtd_maskl(0xb806020C, 0x0FFFFFFF, 0x90000000);//09: uart0_tXdo,<O> 0x1806_020C[31:28]

                                //enable JTAG debug
                                rtd_maskl(JTAG_CONTROL_REG, 0xFFFFFFEF, 0x00000000);//ejtag_gpu_bist

                                //enbale connect RTICE
                                g_ByPassRTICECmd = 0;

                        }
                }
                break;
#endif
                case MISC_SET_DDR_SSC_PARAM:
                {
                        DDR_SSC_PARA para;
                        int nRet = 0;
                        if(copy_from_user((void *)&para, (const void __user *)arg, sizeof(DDR_SSC_PARA))) {
                                retval = -EFAULT;
                                break;
                        }

                        switch (para.cmd)
                        {
                                case RHAL_SYS_CMD_SET_SPREAD_SPRECTRUM:
                                        DDR_nRatio[para.module] = para.ratio;
                                        nRet = RHAL_SYS_SetDDRSpreadSpectrum(para.module, DDR_nRatio[para.module]);
                                        break;

                                case RHAL_SYS_CMD_GET_SPREAD_SPRECTRUM:
                                        para.ratio = DDR_nRatio[para.module];
                                        break;

                                case RHAL_SYS_CMD_SET_DDR_PERIOD:
                                         if(para.period == RHAL_SYS_DDR_PERIOD_30K)
                                                DDR_period = 30;
                                        else if(para.period == RHAL_SYS_DDR_PERIOD_33K)
                                                 DDR_period = 33;
                                        else if(para.period == RHAL_SYS_DDR_PERIOD_60K)
                                                DDR_period = 60;
                                        else if(para.period == RHAL_SYS_DDR_PERIOD_66K)
                                                DDR_period = 66;
                                        else
                                        {
                                                nRet = -1;
                                                break;
                                         }
                                         nRet = RHAL_SYS_SetDDRSpreadSpectrum(para.module, DDR_nRatio[para.module]);
                                         break;

                                case RHAL_SYS_CMD_GET_DDR_PERIOD:
                                        if(DDR_period == 30)
                                                para.period = RHAL_SYS_DDR_PERIOD_30K;
                                        else if(DDR_period == 33)
                                                 para.period = RHAL_SYS_DDR_PERIOD_33K;
                                        else if(DDR_period == 60)
                                                para.period = RHAL_SYS_DDR_PERIOD_60K;
                                        else if(DDR_period == 66)
                                                para.period = RHAL_SYS_DDR_PERIOD_66K;
                                        else
                                                nRet = -1;
                                        break;

                                 case RHAL_SYS_CMD_GET_SUPPORT_MODULE:
                                        if(para.module == RHAL_SYS_SPREAD_SPECTRUM_MODULE_DDR)
                                                para.module_support = 1;
#ifdef SUPPORT_DCU2
                                        else if(para.module == RHAL_SYS_SPREAD_SPECTRUM_MODULE_DDR2)
                                                para.module_support = 1;
#endif
                                        else
                                                para.module_support = 0;
                                        break;

                                 default:
                                        nRet = -1;
                                        break;
                        }

                        if(nRet != 0) {
                                retval = -EFAULT;
                                break;
                        }
                        if(copy_to_user((void __user *)arg, (void *)&para, sizeof(DDR_SSC_PARA))) {
                                retval = -EFAULT;
                                break;
                        }
                        break;
                }


                case MISC_SET_RTDLOG: {
                        rtk_rtdlog_info_t rtd_log = {0};
                        if(copy_from_user((void *)&rtd_log, (const void __user *)arg, sizeof(rtk_rtdlog_info_t))) {
                                MISC_INFO("[RTDLOG_LEVEL] copy_from_user fail...\n");
                                retval = -EFAULT;
                                break;
                        }
                        console_rtdlog.rtdlog_level = rtd_log.rtdlog_level;
                        console_rtdlog.rtdlog_module = rtd_log.rtdlog_module;
                        console_rtdlog.rtdlog_option = rtd_log.rtdlog_option;
                }
                break;
                case MISC_GET_RTDLOG: {

                        if((unsigned int)rtd_inl(RTD_LOG_CHECKING_REGISTER) == 0xdeadbeef) {
                                /*using unused register 0xb801a610 to check whether to using assert
                                when rtd log fatal
                                */
                                console_rtdlog.rtdlog_option = (console_rtdlog.rtdlog_option | RTD_LOG_CHECK_REG);
                                //MISC_INFO("test the Reg : 0x%x, the option value:%x \n",rtd_inl(RTD_LOG_CHECKING_REGISTER), console_rtdlog.rtdlog_option );
                        }

                        if(copy_to_user((void __user *)arg, (void *)&console_rtdlog, sizeof(rtk_rtdlog_info_t))) {
                                MISC_INFO("[RTDLOG_LEVEL] copy_to_user fail...\n");
                                retval = -EFAULT;
                                break;
                        }
                }
                break;

                /*case MISC_SET_RTDLOG_MODULE:
                {
                    unsigned int log_module = 0;
                    if(copy_from_user((void *)&log_module, (const void __user *)arg, sizeof(unsigned int)))
                    {
                        MISC_INFO("[RTDLOG_MODULE] copy_from_user fail...\n");
                        retval = -EFAULT;
                        break;
                    }
                    console_rtdlog_module = log_module;
                }
                break;

                case MISC_GET_RTDLOG_MODULE:
                {
                    if(copy_to_user((void __user *)arg, (void *)&console_rtdlog_module, sizeof(unsigned int)))
                    {
                        MISC_INFO("[RTDLOG_MODULE] copy_to_user fail...\n");
                        retval = -EFAULT;
                        break;
                    }
                }
                break;*/
#ifdef CONFIG_RTK_KDRV_R8168
                case MISC_SET_WARM_WOL_ENABLE: {
                        int nEnable = arg;

                        if(nEnable) {
                                MISC_INFO( "Enable Warm Standby WOL...\n");
                                // clear STR flag for emcu
                                //rtd_outl(0xb8060110, 0x00000000);
                                // clear share memory for emcu
#define RTD_SHARE_MEM_LEN       32
#define RTD_SHARE_MEM_BASE      0xb8060500
                                MISC_INFO("clear share memory in %s...\n", __FUNCTION__);
                                for(i = 0; i < RTD_SHARE_MEM_LEN; i++)
                                        rtd_outl(RTD_SHARE_MEM_BASE + (4 * i), 0);
                                // set WOL flag enabled for emcu
                                MISC_INFO("set WOL flag enabled for emcu\n");
                                rtd_outl(0xb8060500, (rtd_inl(0xb8060500) & 0xfeffffff) | 0x01000000);
                                // run a thread for WOL polling
                                thread_wol = kthread_run(wol_polling, NULL, "wol_polling");
                                if(IS_ERR(thread_wol)) {
                                        MISC_INFO("create wol_polling thread failed\n");
                                        retval = -EFAULT;
                                        break;
                                }
                                MISC_INFO("create wol_polling thread successfully\n");
                                // 0x18016c10[0] = 0x1 (default =0) before enable emcu clock
                                // gmac clock select  0: gmac/emcu2 use bus clock 1: gmac/emcu2 use GPHY125MHz clock
                                //rtd_outl(0xb8016c10, (rtd_inl(0xb8016c10) & 0xfffffffe) | 0x1);
                                // enable emcu clock
                                //MISC_INFO( "Wake-up ECPU...\n");
                                //rtd_outl(STB_ST_CLKEN1_reg, rtd_inl(STB_ST_CLKEN1_reg) | BIT(9)); // clk enable for emcu
                                //udelay(500);
                                //rtd_outl(STB_ST_SRST1_reg, rtd_inl(STB_ST_SRST1_reg) | BIT(9));       // release reset for emcu
                        } else {
                                MISC_INFO( "Disable WOL...\n");
                                // clear STR flag for emcu
                                //rtd_outl(0xb8060110, 0x00000000);
                                // clear share memory for emcu
                                MISC_INFO("clear share memory in %s...\n", __FUNCTION__);
                                for(i = 0; i < RTD_SHARE_MEM_LEN; i++)
                                        rtd_outl(RTD_SHARE_MEM_BASE + (4 * i), 0);
                                // set WOL flag disabled for emcu
                                MISC_INFO("set WOL flag disabled for emcu\n");
                                rtd_outl(0xb8060500, (rtd_inl(0xb8060500) & 0xfeffffff));
                                // stop a thread for WOL polling
                                rtd_outl(0xb80160b0, 0x70030000);  //0xf0031e00
                                if ( (thread_wol != NULL) && ((rtd_inl(0xb80160b0) & 0xffff) != 0x9e00) ) {
                                        // Resume
                                        rtl8168_resume(network_devs);
                                        MISC_INFO( "ether driver resume for WOL\n");
                                        kthread_stop(thread_wol);
                                        MISC_INFO( "stop wol_polling thread!\n");
                                } else {
                                        MISC_INFO( "%s: WOL:%08x\n", __FUNCTION__, rtd_inl(0xb80160b0));   // check 0x1e00(default)  -->WOL 0x9e00
                                        MISC_INFO( "wol_polling thread does not exist!\n");
                                }
                                // disable emcu clock
                                //rtd_outl(STB_ST_CLKEN1_reg, rtd_inl(STB_ST_CLKEN1_reg) & ~BIT(9));    // clk disable for emcu
                                //udelay(500);
                                //rtd_outl(STB_ST_SRST1_reg, rtd_inl(STB_ST_SRST1_reg) & ~BIT(9));  // hold reset for emcu
                        }
                }
                break;
#endif
                case MISC_GET_WARM_WOL_ENABLE: {
                        int nEnable;

                        if((rtd_inl(0xb8060500) & 0x01000000) != 0)
                                nEnable = true;
                        else
                                nEnable = false;

                        if(copy_to_user((void __user *)arg, (void *)&nEnable, sizeof(int))) {
                                retval = -EFAULT;
                                break;
                        }
                }
                break;
                case MISC_QSM_OFF: {
                       MISC_INFO( "QSM OFF Wake-Up Internal Micom...\n");
#if 0
                       /* disable LVR function */
                       rtd_outl(STB_SC_LV_RST_reg, (rtd_inl(STB_SC_LV_RST_reg) & 0xffffdfff));
                       MISC_INFO("289x rtd_inl(0xb8060004)=%08x\n", rtd_inl(STB_SC_LV_RST_reg));
                       MISC_OSC_Clk_init();
                       MISC_INFO("289x OSC Clk init ok\n");
                       /* tunning OSC & enable OSC */
                       MISC_OSC_tracking();
#endif
                       if( rtd_inl(0xb8060110) != 0x9021affa) {
                           rtd_outl(STB_ST_SRST1_reg, BIT(9));
                           rtd_outl(0xb8060110, 0x9021affc);
                           rtd_outl(STB_ST_CLKEN1_reg, BIT(9) | BIT(0));  // clk enable for emcu
                           udelay(500);
                           rtd_outl(STB_ST_SRST1_reg, BIT(9) | BIT(0));        // release reset for emcu
                           udelay(500);
                           MISC_INFO("289x OSC Clk tracking ok, rtd_inl(0xb8060110)=%08x\n", rtd_inl(0xb8060110));
                        }
                }
                break;
#ifdef CONFIG_RTK_KDRV_WATCHDOG
                case MISC_SET_WATCHDOG_ENABLE: {
	                int On = arg;
			   		watchdog_enable(On);
                }
                break;
#endif
               case MISC_GET_MODULE_TEST_PIN:
			   {
					MODULE_TEST_PIN_T mt_pin;
					pcb_mgr_get_enum_info_byname("PIN_SIG", (unsigned long long *)&mt_pin.pin_signal);
					pcb_mgr_get_enum_info_byname("PIN_SCL", (unsigned long long *)&mt_pin.pin_scl);
					pcb_mgr_get_enum_info_byname("PIN_SDA", (unsigned long long *)&mt_pin.pin_sda);
					pcb_mgr_get_enum_info_byname("PIN_STATUS0", (unsigned long long *)&mt_pin.pin_status0);
					pcb_mgr_get_enum_info_byname("PIN_STATUS1", (unsigned long long *)&mt_pin.pin_status1);

					if(copy_to_user((void __user *)arg, (void *)&mt_pin, sizeof(MODULE_TEST_PIN_T))) {
							retval = -EFAULT;
							break;
					}
                }
                break;

                case MISC_UART_PINMUX_SWITCH:
                {
                        UART_PINMUX_ENABLE_T   param;

                        if(copy_from_user((void *)&param, (const void __user *)arg, sizeof(UART_PINMUX_ENABLE_T )))
                        {
                                retval = -EFAULT;
                                break;
                        }
                        switch(param)
                        {
                                case UART0_PINMUX_ENABLE:
                                        rtd_maskl(PINMUX_MAIN_ST_GPIO_ST_CFG_1_reg, 0x0F0FFFFF, 0x40400000); //switch uart pinmux
                                        rtd_maskl(PINMUX_MAIN_ST_Pin_Mux_Ctrl0_reg, 0xFFFF8FFF, 0x3000); //set uart0 sel 0x18060240[14:12]
                                        break;
                                case UART1_PINMUX_ENABLE:
                                        rtd_maskl(PINMUX_MAIN_ST_GPIO_ST_CFG_1_reg, 0x0F0FFFFF, 0x50500000); //switch uart pinmux
                                        rtd_maskl(PINMUX_MAIN_Pin_Mux_Ctrl3_reg, 0xFFF8FFFF, 0x30000); //switch uart1 sel 0x180008CC[18:16]
                                        break;
                                case UART0_UART1_PINMUX_DISABLE:
                                        rtd_maskl(PINMUX_MAIN_ST_GPIO_ST_CFG_1_reg, 0x0F0FFFFF, 0xF0F00000); //switch uart pinmux
                                        break;
                                default:
                                        MISC_ERR(KERN_ERR "misc ioctl MISC_UART_PINMUX_SWITCH not supported\n");
                                        retval = -EFAULT;
                                        break;
                        }
                        break;
                }

                case MISC_UART_GET_UART0_ENABLE:
                {
                        unsigned int tmp_val = rtd_inl(PINMUX_MAIN_ST_GPIO_ST_CFG_1_reg);

                        if( (tmp_val & 0xF0F00000) == 0x40400000)
                        {
                                tmp_val = 1;
                        }
                        else
                        {
                                tmp_val = 0;
                        }

                        if(copy_to_user((void __user *)arg, (const void *)&tmp_val, sizeof(unsigned int)))
                        {
                                retval = -EFAULT;
                                goto out;
                        }

                        break;
                }

                case MISC_UART_GET_UART1_ENABLE:
                {
                        unsigned int tmp_val = rtd_inl(PINMUX_MAIN_ST_GPIO_ST_CFG_1_reg);

                        if( (tmp_val & 0xF0F00000) == 0x50500000)
                        {
                                tmp_val = 1;
                        }
                        else
                        {
                                tmp_val = 0;
                        }
                        
                        if(copy_to_user((void __user *)arg, (const void *)&tmp_val, sizeof(unsigned int)))
                        {
                                retval = -EFAULT;
                                goto out;
                        }

                        break;
                }

                default:  /* redundant, as cmd was checked against MAXNR */
                        MISC_ERR("misc ioctl not supported\n");
                        retval = -EFAULT;
                        goto out;
        }
out:
        up(&pdev->sem);
        return retval;
}

/*
Add for rtd_logger to change log module and level
*/

ssize_t misc_read(struct file * filp, char __user * buf, size_t count,
                  loff_t * f_pos)
{
        //using KERN_EMERG to get information at any time
        MISC_EMERG(" ==========Read information==========\n" );
        MISC_EMERG(" LOGGER Module=0x%x\n", console_rtdlog.rtdlog_module );
        MISC_EMERG(" LOGGER level=%d\n", console_rtdlog.rtdlog_level);
        MISC_EMERG(" LOGGER option=0x%x\n", console_rtdlog.rtdlog_option);
        MISC_EMERG(" ========Read information End=========\n" );
        return 0;
}

ssize_t misc_write(struct file * filp, const char __user * buf, size_t count,
                   loff_t * f_pos)
{
        ssize_t retval = -ENOMEM;
        char cmd[20];
        char value[MODULE_STR_SIZE] = {'\n'};
        unsigned int convertValue = 0;
        char str_length = 0;
        int i = 0;

        if (count <= 256) {
                uint8_t data[256] = {0};
                if (copy_from_user(data, buf, count)) {
                        retval = -EFAULT;
                        goto out;
                }
                MISC_ERR("[misc_write]Get string =%s\n", data);

                //sscanf (data,"%s %u",cmd,&convertValue);
                sscanf (data, "%s %s", cmd, value);
                //count  value str size
                for (i = 0; i < MODULE_STR_SIZE; i++) {
                        if ((value[i] >= 'A') && (value[i] <= 'F')) {
                                str_length++;
                        } else if((value[i] >= 'a') && (value[i] <= 'f')) {
                                str_length++;
                        } else if((value[i] >= '0') && (value[i] <= '9')) {
                                str_length++;
                        }
                }

                if(StrToHex(value, str_length, &convertValue) == true) {
                        // MISC_ERR("[misc_write]value  %s convert=0x%x\n",value,convertValue);
                        if(strcmp(cmd, "logmodule") == 0) {
                                // MISC_ERR("[misc_write]convert=%s, 0x%x\n",cmd,convertValue);
                                // console_rtdlog_module = convertValue;
                                console_rtdlog.rtdlog_module = convertValue;
                        } else if(strcmp(cmd, "loglevel") == 0) {
                                //MISC_ERR("[misc_write]convert=%s, 0x%x\n",cmd,convertValue);
                                //console_rtdlog_level = convertValue;
                                console_rtdlog.rtdlog_level = convertValue;
                        } else if(strcmp(cmd, "logoption") == 0) {
                                //MISC_ERR("[misc_write]convert=%s, 0x%x\n",cmd,convertValue);
                                //console_rtdlog_level = convertValue;
                                console_rtdlog.rtdlog_option = convertValue;
                        } else {
                                MISC_ERR("[misc_write]NO USING COMMAND\n");
                        }
                }
        }

        retval = count;

out:
        return retval;
}

static char* dc_sys_id_str [16][16] = {
        {"TVSB2_DIW",    "TVSB1_VD",      "SB1_CP_KC",    "SB2_NULL",   "SB3_AEE",    "VE_VM1",   "NULL", "GPU_0", "TVSB5_DEMOD",    "NULL", "SE_SEQ0", "TVSB4_ODW",        "NULL", "TVSB7_ME",   "NULL", "NULL"},
        {"TVSB2_DIR",    "TVSB1_AUDIO1",  "SB1_CR",       "SB2_ACPU",   "SB3_ADE",    "VE_VM2",   "NULL", "GPU_1", "TVSB5_DEBUGDMA", "NULL", "SE_SEQ1", "TVSB4_ODR",        "NULL", "TVSB7_VDE",  "NULL", "NULL"},
        {"TVSB2_M_CAP",  "TVSB1_AUDIO2",  "SB1_USB2"      "SB2_NULL",   "SB3_ME",     "VE_PRD",   "NULL", "GPU_2", "TVSB5_DEMOD2",   "NULL", "SE_NULL", "TVSB4_NULL",       "NULL", "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_M_DISP", "TVSB1_VBI",     "SB1_NF_KC",    "SB2_VCPU",   "SB3_MD_KCPU","VE_DBK",   "NULL", "GPU_3", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL", "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_S_CAP",  "TVSB1_TVE_VD",  "SB1_TP2_KC",   "SB2_NULL",   "SB3_VE",     "VE_MVD",   "NULL", "GPU_4", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_OSD1",       "NULL", "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_S_DISP", "TVSB1_TVE_VBI", "SB1_ETN",      "SB2_UART",   "SB3_VDE",    "VE_NDB",   "NULL", "GPU_5", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_OSD2",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_VO1_Y",  "TVSB1_NULL",    "SB1_TP_KC",    "SB2_HOST_IF","SB3_NULL",   "VE_R1SEQ", "NULL", "GPU_6", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_OSD3",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_VO1_C",  "TVSB1_NULL",    "SB1_EMMC_KC",  "SB2_NULL",   "SB3_NULL",   "VE_BS",    "NULL", "GPU_7", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_DEMURA",     "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_VO2_Y",  "TVSB1_NULL",    "SB1_NULL",     "SB2_NULL",   "SB3_NULL",   "VE_R1BLK", "NULL", "GPU_8", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_DMATO3DLUT", "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_VO2_C",  "TVSB1_NULL",    "SB1_NULL",     "SB2_NULL",   "SB3_NULL",   "VE_NULL",  "NULL", "GPU_9", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_I3DDMA", "TVSB1_NULL",    "SB1_CP_NKC",   "SB2_NULL",   "SB3_UART2",  "VE_GC",    "NULL", "GPU_a", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_NULL",   "TVSB1_NULL",    "SB1_NF_NKC",   "SB2_NULL",   "SB3_MD_SCPU","VE_IME",   "NULL", "GPU_b", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_NULL",   "TVSB1_NULL",    "SB1_TP2_NKC",  "SB2_NULL",   "SB3_MD_VCPU","VE_FME",   "NULL", "GPU_c", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_SNR_R",  "TVSB1_NULL",    "SB1_TP_NKC",   "SB2_NULL",   "SB3_NULL",   "VE_SAO",   "NULL", "GPU_e", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2_3DTABLE","TVSB1_NULL",    "SB1_EMMC_NKC", "SB2_NULL",   "SB3_NULL",   "VE_SEG",   "NULL", "GPU_f", "TVSB5_NULL",     "NULL", "SE_NULL", "TVSB4_NULL",       "NULL",  "TVSB7_NULL", "NULL", "NULL"},
        {"TVSB2",        "TVSB1",         "SB1",          "SB2",        "SB3",        "VE",       "NULL", "GPU",   "TVSB5",          "NULL", "SE",      "TVSB4",            "NULL",  "TVSB7",      "NULL", "NULL"}
};

void dump_addcmd_status (unsigned int addcmd0, unsigned int addcmd1)
{
        unsigned int module_id = (addcmd0 >> 15) & 0xFF;
        if (addcmd1 & BIT(0)) {
                MISC_ERR("Block Mode access: "FRED_START"%s"FCOLOR_END"\n", (addcmd1 & BIT(1)) ? "Block write" : "Block read");
                MISC_ERR("module_ID is "FRED_START"0x%x(%s)"FCOLOR_END"\n", module_id,dc_sys_id_str[module_id&0xF][(module_id>>4)&0xF]);
                MISC_ERR("Sequence starting address is : "FRED_START"0x%x (unit: byte).  (0x%x Unit : 8-Byte )"FCOLOR_END"\n", ( (addcmd1 & 0x7FFFFFFF) >> 2) << 3, (addcmd1 & 0x7FFFFFFF)>>2);
                MISC_ERR("Sequence burst length ( Unit : 8-Byte ) is : "FRED_START"%d"FCOLOR_END"\n", (addcmd1 >> 31) + ((addcmd0 & 0x7F)<<1));

#ifdef DCMT_DC2
                if (((addcmd0 >> 18) & 0xF) < 13) {
                        MISC_ERR("IB_ERROR_TRAP_STATUS[0x%x] %x, IB_ERROR_TRAP_STATUS2[0x%x] %x\n",
                                  (IB_IB_ERROR_TRAP_STATUS_reg + ((addcmd0 >> 18) & 0xF) * 0x100), rtd_inl(IB_IB_ERROR_TRAP_STATUS_reg + ((addcmd0 >> 18) & 0xF) * 0x100),
                                  (IB_IB_ERROR_TRAP_STATUS2_reg + ((addcmd0 >> 18) & 0xF) * 0x100), rtd_inl(IB_IB_ERROR_TRAP_STATUS2_reg + ((addcmd0 >> 18) & 0xF) * 0x100));
                }
#endif
        } else {
                MISC_ERR("Sequence Mode access: "FRED_START"%s"FCOLOR_END"\n", (addcmd1 & BIT(1)) ? "sequence write" : "sequence read");
                MISC_ERR("module_ID is "FRED_START"0x%x(%s)"FCOLOR_END"\n", module_id,dc_sys_id_str[module_id&0xF][(module_id>>4)&0xF]);
                MISC_ERR("Sequence starting address is : "FRED_START"0x%x (unit: byte).  (0x%x Unit : 8-Byte )"FCOLOR_END"\n", ( (addcmd1 & 0x7FFFFFFF) >> 2) << 3, (addcmd1 & 0x7FFFFFFF)>>2);
                MISC_ERR("Sequence burst length ( Unit : 8-Byte ) is : "FRED_START"%d"FCOLOR_END"\n", (addcmd1 >> 31) + ((addcmd0 & 0x7F)<<1));

#ifdef DCMT_DC2
                if (((addcmd0 >> 18) & 0xF) < 13) {
                        MISC_ERR("IB_ERROR_TRAP_STATUS[0x%x] %x, IB_ERROR_TRAP_STATUS2[0x%x] %x\n",
                                  (IB_IB_ERROR_TRAP_STATUS_reg + ((addcmd0 >> 18) & 0xF) * 0x100), rtd_inl(IB_IB_ERROR_TRAP_STATUS_reg + ((addcmd0 >> 18) & 0xF) * 0x100),
                                  (IB_IB_ERROR_TRAP_STATUS2_reg + ((addcmd0 >> 18) & 0xF) * 0x100), rtd_inl(IB_IB_ERROR_TRAP_STATUS2_reg + ((addcmd0 >> 18) & 0xF) * 0x100));
                }
#endif
        }
        MISC_ERR("\n\n\n\n\n\n\n");
}

static void dump_dc_errcmd(int dc_id, int sys_id, char * msg)
{
        int i;
        unsigned int add_hi_reg = (dcsys_errcmd_regs.addcmd_hi[dc_id][sys_id]);
        unsigned int add_lo_reg = (dcsys_errcmd_regs.addcmd_lo[dc_id][sys_id]);
        unsigned int dc_int_reg = (dcsys_errcmd_regs.dc_int[dc_id][sys_id]);
        unsigned int addcmd[2];
        unsigned int trap_case;
        unsigned int mask = BIT(24) | BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15);
        char* errcmd_str[8] = {
                "seq_sa_odd",
                "seq_bl_zero",
                "seq_bl_odd",
                "blk_cmd",
                "err_disram_switch_int",
                "null",
                "null",
                "null"
        };

        if (rtd_inl(add_hi_reg) & BIT(31)) {
                trap_case = (rtd_inl(add_hi_reg) & 0x7F800000) >> 23;
                addcmd[0] = (rtd_inl(add_hi_reg) & 0x007FFFFF);
                addcmd[1] = rtd_inl(add_lo_reg);

                MISC_ERR("%s\n", msg);

                MISC_ERR("dump regs\n add_hi_reg 0x%08x = 0x%08x\n add_lo_reg 0x%08x = 0x%08x\n dc_int_reg 0x%08x = 0x%08x\n",
                        add_hi_reg, rtd_inl(add_hi_reg),
                        add_lo_reg, rtd_inl(add_lo_reg),
                        dc_int_reg, rtd_inl(dc_int_reg));
          	MISC_ERR("\n\n\n\n\n\n\n");
                MISC_ERR("DC error command detected: trap case %08x, addcmd %08x-%08x\n", trap_case, addcmd[0], addcmd[1]);
                for(i = 0; i < 8; ++i) {
                        if(trap_case & (1 << i)) {
                                MISC_ERR("trap case : %d (%s)\n", i, errcmd_str[i]);
                        }
                }
                dump_addcmd_status(addcmd[0], addcmd[1]);
                rtd_outl(dc_int_reg, mask | BIT(0));  //write 0 to clear the pending
        }
}

static void dump_dc_errcmd_reg(int dc_id, int sys_id, char * msg)
{
        unsigned int add_hi_reg = (dcsys_errcmd_regs.addcmd_hi[dc_id][sys_id]);
        unsigned int add_lo_reg = (dcsys_errcmd_regs.addcmd_lo[dc_id][sys_id]);
        unsigned int dc_int_reg = (dcsys_errcmd_regs.dc_int[dc_id][sys_id]);

        MISC_ERR("%s\n", msg);
        MISC_ERR("add_hi_reg(0x%08x),val:0x%08x\n",add_hi_reg,rtd_inl(add_hi_reg));
        MISC_ERR("add_lo_reg(0x%08x),val:0x%08x\n",add_lo_reg,rtd_inl(add_lo_reg));
        MISC_ERR("dc_int_reg(0x%08x),val:0x%08x\n",dc_int_reg,rtd_inl(dc_int_reg));
}

static void dump_dc_errcmd_all(void)
{
        dump_dc_errcmd_reg(DC_ID_1, DC_SYS1, "DC1 SYS1 ERRCMD REG");
        dump_dc_errcmd_reg(DC_ID_1, DC_SYS2, "DC1 SYS2 ERRCMD REG");
        dump_dc_errcmd_reg(DC_ID_1, DC_SYS3, "DC1 SYS3 ERRCMD REG");
#ifdef DCMT_DC2
        dump_dc_errcmd_reg(DC_ID_2, DC_SYS1, "DC2 SYS1 ERRCMD REG");
        dump_dc_errcmd_reg(DC_ID_2, DC_SYS2, "DC2 SYS2 ERRCMD REG");
        dump_dc_errcmd_reg(DC_ID_2, DC_SYS3, "DC2 SYS3 ERRCMD REG");
#endif // #ifdef DCMT_DC2
        dump_dc_errcmd(DC_ID_1, DC_SYS1, "DC1 SYS1 ERRCMD");
        dump_dc_errcmd(DC_ID_1, DC_SYS2, "DC1 SYS2 ERRCMD");
        dump_dc_errcmd(DC_ID_1, DC_SYS3, "DC1 SYS3 ERRCMD");
#ifdef DCMT_DC2
        dump_dc_errcmd(DC_ID_2, DC_SYS1, "DC2 SYS1 ERRCMD");
        dump_dc_errcmd(DC_ID_2, DC_SYS2, "DC2 SYS2 ERRCMD");
        dump_dc_errcmd(DC_ID_2, DC_SYS3, "DC2 SYS3 ERRCMD");
#endif // #ifdef DCMT_DC2
}

static void dump_dcsys_debug_status(void)
{
        unsigned int val;
        val = rtd_inl(DC_SYS_DC_debug_status_reg);
        MISC_ERR("DC1 debug status is %08x: \n", val);
        if (val & BIT(25)) {
                MISC_ERR("DC1 BIT 25:  exsram write data fifo underflow\n");
        }
        if (val & BIT(24)) {
                MISC_ERR("DC1 BIT 24:  exsram write data fifo overflow\n");
        }
#if 0 //removed status
        if (val & BIT(23)) {
                MISC_ERR("DC1 BIT 23:  DC_SYS3 write cmd fifo underflow\n");
        }
        if (val & BIT(22)) {
                MISC_ERR("DC1 BIT 22:  DC_SYS2 write cmd fifo underflow\n");
        }
        if (val & BIT(21)) {
                MISC_ERR("DC1 BIT 21:  DC_SYS write cmd fifo underflow\n");
        }
        if (val & BIT(20)) {
                MISC_ERR("DC1 BIT 20:  DC_SYS3 read cmd fifo underflow\n");
        }
        if (val & BIT(19)) {
                MISC_ERR("DC1 BIT 19:  DC_SYS2 read cmd fifo underflow\n");
        }
        if (val & BIT(18)) {
                MISC_ERR("DC1 BIT 18:  DC_SYS read cmd fifo underflow\n");
        }
        if (val & BIT(17)) {
                MISC_ERR("DC1 BIT 17:  DC_SYS3 write cmd fifo overflow\n");
        }
        if (val & BIT(16)) {
                MISC_ERR("DC1 BIT 16:  DC_SYS2 write cmd fifo overflow\n");
        }
        if (val & BIT(15)) {
                MISC_ERR("DC1 BIT 15:  DC_SYS write cmd fifo overflow\n");
        }
        if (val & BIT(14)) {
                MISC_ERR("DC1 BIT 14:  DC_SYS3 read cmd fifo overflow\n");
        }
        if (val & BIT(13)) {
                MISC_ERR("DC1 BIT 13:  DC_SYS2 read cmd fifo overflow\n");
        }
        if (val & BIT(12)) {
                MISC_ERR("DC1 BIT 12:  DC_SYS read cmd fifo overflow\n");
        }
        if (val & BIT(11)) {
                MISC_ERR("DC1 BIT 11:  DC_SYS3 write data fifo underflow\n");
        }
        if (val & BIT(10)) {
                MISC_ERR("DC1 BIT 10:  DC_SYS2 write data fifo underflow\n");
        }
        if (val & BIT(9)) {
                MISC_ERR("DC1 BIT 9:  DC_SYS write data fifo underflow\n");
        }
        if (val & BIT(8)) {
                MISC_ERR("DC1 BIT 8:  DC_SYS3 read data fifo underflow\n");
        }
        if (val & BIT(7)) {
                MISC_ERR("DC1 BIT 7:  DC_SYS2 read data fifo underflow\n");
        }
        if (val & BIT(6)) {
                MISC_ERR("DC1 BIT 6:  DC_SYS read data fifo underflow\n");
        }
        if (val & BIT(5)) {
                MISC_ERR("DC1 BIT 5:  DC_SYS3 write data fifo overflow\n");
        }
        if (val & BIT(4)) {
                MISC_ERR("DC1 BIT 4:  DC_SYS2 write data fifo overflow\n");
        }
        if (val & BIT(3)) {
                MISC_ERR("DC1 BIT 3:  DC_SYS write data fifo overflow\n");
        }
        if (val & BIT(2)) {
                MISC_ERR("DC1 BIT 2:  DC_SYS3 read data fifo overflow\n");
        }
        if (val & BIT(1)) {
                MISC_ERR("DC1 BIT 1:  DC_SYS2 read data fifo overflow\n");
        }
        if (val & BIT(0)) {
                MISC_ERR("DC1 BIT 0:  DC_SYS read data fifo overflow\n");
        }
#endif

#ifdef DCMT_DC2
        val = rtd_inl(DC2_SYS_DC_debug_status_reg);
        MISC_ERR("\n\n\n\n");
        MISC_ERR("DC2 debug status is %08x: \n", val);
        if (val & BIT(25)) {
                MISC_ERR("DC2 BIT 25:  exsram write data fifo underflow\n");
        }
        if (val & BIT(24)) {
                MISC_ERR("DC2 BIT 24:  exsram write data fifo overflow\n");
        }
#if 0 //removed status
        if (val & BIT(23)) {
                MISC_ERR("DC2 BIT 23:  DC_SYS3 write cmd fifo underflow\n");
        }
        if (val & BIT(22)) {
                MISC_ERR("DC2 BIT 22:  DC_SYS2 write cmd fifo underflow\n");
        }
        if (val & BIT(21)) {
                MISC_ERR("DC2 BIT 21:  DC_SYS write cmd fifo underflow\n");
        }
        if (val & BIT(20)) {
                MISC_ERR("DC2 BIT 20:  DC_SYS3 read cmd fifo underflow\n");
        }
        if (val & BIT(19)) {
                MISC_ERR("DC2 BIT 19:  DC_SYS2 read cmd fifo underflow\n");
        }
        if (val & BIT(18)) {
                MISC_ERR("DC2 BIT 18:  DC_SYS read cmd fifo underflow\n");
        }
        if (val & BIT(17)) {
                MISC_ERR("DC2 BIT 17:  DC_SYS3 write cmd fifo overflow\n");
        }
        if (val & BIT(16)) {
                MISC_ERR("DC2 BIT 16:  DC_SYS2 write cmd fifo overflow\n");
        }
        if (val & BIT(15)) {
                MISC_ERR("DC2 BIT 15:  DC_SYS write cmd fifo overflow\n");
        }
        if (val & BIT(14)) {
                MISC_ERR("DC2 BIT 14:  DC_SYS3 read cmd fifo overflow\n");
        }
        if (val & BIT(13)) {
                MISC_ERR("DC2 BIT 13:  DC_SYS2 read cmd fifo overflow\n");
        }
        if (val & BIT(12)) {
                MISC_ERR("DC2 BIT 12:  DC_SYS read cmd fifo overflow\n");
        }
        if (val & BIT(11)) {
                MISC_ERR("DC2 BIT 11:  DC_SYS3 write data fifo underflow\n");
        }
        if (val & BIT(10)) {
                MISC_ERR("DC2 BIT 10:  DC_SYS2 write data fifo underflow\n");
        }
        if (val & BIT(9)) {
                MISC_ERR("DC2 BIT 9:  DC_SYS write data fifo underflow\n");
        }
        if (val & BIT(8)) {
                MISC_ERR("DC2 BIT 8:  DC_SYS3 read data fifo underflow\n");
        }
        if (val & BIT(7)) {
                MISC_ERR("DC2 BIT 7:  DC_SYS2 read data fifo underflow\n");
        }
        if (val & BIT(6)) {
                MISC_ERR("DC2 BIT 6:  DC_SYS read data fifo underflow\n");
        }
        if (val & BIT(5)) {
                MISC_ERR("DC2 BIT 5:  DC_SYS3 write data fifo overflow\n");
        }
        if (val & BIT(4)) {
                MISC_ERR("DC2 BIT 4:  DC_SYS2 write data fifo overflow\n");
        }
        if (val & BIT(3)) {
                MISC_ERR("DC2 BIT 3:  DC_SYS write data fifo overflow\n");
        }
        if (val & BIT(2)) {
                MISC_ERR("DC2 BIT 2:  DC_SYS3 read data fifo overflow\n");
        }
        if (val & BIT(1)) {
                MISC_ERR("DC2 BIT 1:  DC_SYS2 read data fifo overflow\n");
        }
        if (val & BIT(0)) {
                MISC_ERR("DC2 BIT 0:  DC_SYS read data fifo overflow\n");
        }
#endif
#endif // #ifdef DCMT_DC2
}
#ifdef DCMT_DC2
static void dump_ib_errcmd_reg(void)
{
        unsigned int ib_err_status_reg = 0;
        unsigned int ib_err_status = 0;
        unsigned int ib_trap_status_reg = 0;
        unsigned int ib_trap_status = 0;
        unsigned int ib_trap_status2_reg = 0;
        unsigned int ib_trap_status2 = 0;
        int i;

        char * ib_client_str[IB_CLIENTS_NUM] = {
                "c0 (TVSB2)",
                "c1 (TVSB1)",
                "c2 (SB1)",
                "c3 (SB2)",
                "c4 (SB3)",
                "c5 (VE)",
                "c6 (Reserved)",
                "c7 (GDE)",
                "c8 (TVSB5)",
                "c9 (VE2)",
                "c10 (SE2)",
                "c11 (TVSB4)",
                "c12 (MEMC)"
        };

        for(i = 0; i < IB_CLIENTS_NUM; i++)
        {
                ib_err_status_reg = IB_IB_ERROR_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                ib_err_status = rtd_inl(ib_err_status_reg);
                ib_trap_status_reg = IB_IB_ERROR_TRAP_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                ib_trap_status = rtd_inl(ib_trap_status_reg);
                ib_trap_status2_reg = IB_IB_ERROR_TRAP_STATUS2_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                ib_trap_status2 = rtd_inl(ib_trap_status2_reg);

                MISC_ERR("IB CLIENT %d:%s\n",i,ib_client_str[i]);
                MISC_ERR("ib_err_status(0x%08x),val:0x%08x\n",ib_err_status_reg,ib_err_status);
                MISC_ERR("ib_trap_status(0x%08x),val:0x%08x\n",ib_trap_status_reg,ib_trap_status);
                MISC_ERR("ib_trap_status2(0x%08x),val:0x%08x\n",ib_trap_status2_reg,ib_trap_status2);
        }
}

static void dump_ib_errcmd(void)
{
        char * ib_client_str[IB_CLIENTS_NUM] = {
                "c0 (TVSB2)",
                "c1 (TVSB1)",
                "c2 (SB1)",
                "c3 (SB2)",
                "c4 (SB3)",
                "c5 (VE)",
                "c6 (Reserved)",
                "c7 (GPU)",
                "c8 (TVSB5)",
                "c9 (VE2)",
                "c10 (SE2)",
                "c11 (TVSB4)",
                "c12 (MEMC)"
        };

        char * ib_trapcase_str[IB_TRAP_CASE_NUMBER] = {
                "null ",
                "null",
                "null",
                "null",
                "err_seq_bl_odd_int ",
                "err_seq_bl_zero_int",
                "err_seq_sa_odd_int",
                "null",
                "null",
                "null",
                "null"
        };

        char * ib_err_status_str[IB_ERR_STATUS_NUMBER] = {
                "Seq_blen_zero",
                "Seq_blen_odd",
                "Seq_saddr_odd",
                "Rinfo_overflow",
                "Rinfo_underflow",
                "Winfo_overflow",
                "Winfo_underflow",
                "Wdone_overflow",
                "Wdone_underflow",
                "Cmd_dc1_overflow",
                "Cmd_dc1_underflow",
                "Cmd_dc2_overflow",
                "Cmd_dc2_underflow",
                "Rdata_dc1_overflow",
                "Rdata_dc1_underflow",
                "Rdout_dc1_underflow",
                "Rdata_dc2_overflow",
                "Rdout_dc2_underflow",
                "Rdout_dc2_underflow",
                "wdata_dc1_overflow",
                "wdata_dc1_underflow",
                "wdout_dc1_underflow",
                "wdata_dc2_overflow",
                "wdata_dc2_underflow",
                "wdout_dc2_underflow",
                "region_error(addcmd cross region)",
                "null",
                "null",
                "dc1_end_in_disram",
                "dc1_st_in_disram",
                "is_blk_cmd",
                "null"
        };

        int i = 0, j = 0;
        for(i = 0; i < IB_CLIENTS_NUM; i++) {
                unsigned int ib_err_status = rtd_inl(IB_IB_ERROR_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i);
                unsigned int ib_trap_address = IB_IB_ERROR_TRAP_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                unsigned int ib_trap_status = rtd_inl(ib_trap_address);

                MISC_ERR("--------------------------------------------------\n");
                MISC_ERR("Client[%s] IB command status: IB_ERROR_STATUS(0x%x)\n", ib_client_str[i], ib_err_status);

                /*detect IB error trap happen or not*/
                if(((ib_trap_status & IB_IB_ERROR_TRAP_STATUS_trap_case_mask) >> IB_IB_ERROR_TRAP_STATUS_trap_case_shift) != 0x0) {
                        MISC_ERR("IB_ERROR_TRAP: IDX(0x%x),Trap_Case(0x%x),CMD_HI(0x%x), CMD_LO(0x%x)\n",
                                  i, ib_trap_status, ib_trap_status & IB_IB_ERROR_TRAP_STATUS_addcmd_hi_mask, rtd_inl(ib_trap_address + 0x4));

                        for(j = 0; j < IB_TRAP_CASE_NUMBER; ++j) {
                                if(ib_trap_status & (1 << (20 + j))) {
                                        MISC_ERR("IB CLIENT : %s -> TRAP CASE : %s\n", ib_client_str[i], ib_trapcase_str[j]);
                                }
                        }

                        MISC_ERR("IB CLIENT : %s -> IB_ERROR_STATUS(0x%x)\n", ib_client_str[i], ib_err_status);
                        for(j = 0; j < IB_ERR_STATUS_NUMBER; ++j) {
                                if(ib_err_status & (1 << j)) {
                                        MISC_ERR("IB CLIENT : %s -> ERR STATUS : %s\n", ib_client_str[i], ib_err_status_str[j]);
                                }
                        }

                        //write 1 to clear error trap status
                        rtd_outl(ib_trap_address, IB_IB_ERROR_TRAP_STATUS_trap_status_clr_mask | BIT(0));

                        //write 0 to reset to default for next time latch
                        rtd_outl(ib_trap_address, IB_IB_ERROR_TRAP_STATUS_trap_status_clr_mask & (~BIT(0)));
                }
        }
}
#endif // #ifdef DCMT_DC2

static __maybe_unused void dump_vo_regs(void)
{
        /*dump VO realted register for checking*/
        rtd_setbits(VODMA_VODMA_V1CHROMA_FMT_reg, BIT(12));
        MISC_ERR("Dump VODMA Setting(1) 0xB8005030(%x), 0xb80050DC(%x),0xb8005000(%x)\n",
                  rtd_inl(VODMA_VODMA_V1CHROMA_FMT_reg),
                  rtd_inl(VODMA_VODMA_DMA_OPTION_reg), rtd_inl(VODMA_VODMA_V1_DCFG_reg));

        rtd_clearbits(VODMA_VODMA_V1CHROMA_FMT_reg, BIT(12));
        MISC_ERR("Dump VODMA Setting(0) 0xB8005030(%x), 0xb80050DC(%x),0xb8005000(%x)\n",
                  rtd_inl(VODMA_VODMA_V1CHROMA_FMT_reg),
                  rtd_inl(VODMA_VODMA_DMA_OPTION_reg), rtd_inl(VODMA_VODMA_V1_DCFG_reg));

        MISC_ERR("Dump 0xB8005100(%x), 0xB8005104(%x),0xB800500C(%x), 0xB800511C(%x)\n",
                  rtd_inl(0xB8005100), rtd_inl(0xB8005104), rtd_inl(0xB800500C), rtd_inl(0xB800511C));

        MISC_ERR("Dump 0xB8005120(%x), 0xB8005134(%x), 0xB8005144(%x), 0xB8005018(%x)\n",
                  rtd_inl(0xB8005120), rtd_inl(0xB8005134), rtd_inl(0xB8005144), rtd_inl(0xB8005018));

        MISC_ERR("Dump 0xB800501c(%x), 0xB8005020(%x), 0xB8005024(%x), 0xB80050E4(%x)\n",
                  rtd_inl(0xB800501c), rtd_inl(0xB8005020), rtd_inl(0xB8005024), rtd_inl(0xB80050E4));
}

static void dump_mc_error_status(unsigned int reg, char * msg)
{
        unsigned int val;
        MISC_ERR("[%s]reg val:0x%08x\n", msg, reg);
        if(reg & BIT(30)) {
                MISC_ERR("[%s] ASYNC FIFO for wdata from DC_SYS overflows\n", msg);
        }
        if(reg & BIT(29)) {
                MISC_ERR("[%s] ASYNC FIFO for command from DC_SYS overflows\n", msg);
        }
        if(reg & BIT(27)) {
                MISC_ERR("[%s] Tag with wdata ack from MC mismatch with the expected qfifo cmd tag from DC_SYS\n", msg);
        }
        if(reg & BIT(26)) {
                MISC_ERR("[%s] Tag with rdata valid from MC mismatch with the expected qfifo cmd tag from DC_SYS\n", msg);
        }
        if(reg & BIT(25)) {
                MISC_ERR("[%s] Burst length of write command from DC_SYS mismatch with the one from MC\n", msg);
        }
        if(reg & BIT(24)) {
                MISC_ERR("[%s] Burst length of read command from DC_SYS mismatch with the one from MC\n", msg);
        }

        val = reg & 0x3F0;
        val = val >> 4;
        MISC_ERR("[%s] The maximum number of read command ever reached in MC_FIFO is 0x%08x\n", msg, val);

        val = reg & 0x7;
        MISC_ERR("[%s] The bank number of the maximum number of write command ever reached in MC_FIFO is 0x%08x\n", msg, val);
}

static void dump_mc_error_status_all(void)
{
        unsigned int mc1_fifo_dbg_cmd0_val = rtd_inl(DC1_MC_MCFIFO_DBG_CMD0_reg);
        unsigned int mc1_fifo_dbg_cmd1_val = rtd_inl(DC1_MC_MCFIFO_DBG_CMD1_reg);
        unsigned int mc1_fifo_dbg_cmd2_val = rtd_inl(DC1_MC_MCFIFO_DBG_CMD2_reg);
        unsigned int mc1_fifo_dbg_cmd3_val = rtd_inl(DC1_MC_MCFIFO_DBG_CMD3_reg);
#ifdef DCMT_DC2
        unsigned int mc2_fifo_dbg_cmd0_val = rtd_inl(DC2_MC_MCFIFO_DBG_CMD0_reg);
        unsigned int mc2_fifo_dbg_cmd1_val = rtd_inl(DC2_MC_MCFIFO_DBG_CMD1_reg);
        unsigned int mc2_fifo_dbg_cmd2_val = rtd_inl(DC2_MC_MCFIFO_DBG_CMD2_reg);
        unsigned int mc2_fifo_dbg_cmd3_val = rtd_inl(DC2_MC_MCFIFO_DBG_CMD3_reg);
#endif // #ifdef DCMT_DC2
        dump_mc_error_status(mc1_fifo_dbg_cmd0_val, "MC1 SCPU");
        dump_mc_error_status(mc1_fifo_dbg_cmd1_val, "MC1 DCSYS1");
        dump_mc_error_status(mc1_fifo_dbg_cmd2_val, "MC1 DCSYS2");
        dump_mc_error_status(mc1_fifo_dbg_cmd3_val, "MC1 DCSYS3");
#ifdef DCMT_DC2
        dump_mc_error_status(mc2_fifo_dbg_cmd0_val, "MC2 SCPU");
        dump_mc_error_status(mc2_fifo_dbg_cmd1_val, "MC2 DCSYS1");
        dump_mc_error_status(mc2_fifo_dbg_cmd2_val, "MC2 DCSYS2");
        dump_mc_error_status(mc2_fifo_dbg_cmd3_val, "MC2 DCSYS3");
#endif // #ifdef DCMT_DC2
}

void dump_tvsb1_error_status(unsigned int val, char * msg)
{
#if 0//unused error req id
        unsigned int tmp = 0;
        char * Error_Client_ID[8] =
        {
                "null",
                "Video Decoder",
                "Audio1",
                "Audio2",
                "VBI(W)",
                "TVE(R)-VD",
                "TVE(R)-VBI",
                "audio hdmi(R)"
        };
#endif

        if(val & BIT(9))
        {
                MISC_ERR("%s - Error in read fifo\n", msg);
        }

        if(val & BIT(8))
        {
                MISC_ERR("%s - Error in write fifo\n", msg);
        }

#if 0//unused error req id
        tmp = val & 0xf;
        if(tmp)
        {
                if(tmp < 8)
                {
                        MISC_ERR("%s - Error req in %s\n", msg, Error_Client_ID[tmp]);
                }
        }
#endif
}

void dump_tvsb2_error_status(unsigned int val, char * msg)
{
        unsigned int tmp;
        char * Error_zero_length_id_str[17] = {
                "none error",
                "De-Interlace(W)",
                "De-Interlace(R)",
                "Main Capture(W)",
                "Main Display(R)",
                "Sub Capture (W)",
                "Sub Display (R)",
                "VO1_Y (R)",
                "VO1_C (R)",
                "VO2_Y (R)",
                "VO2_C (R)",
                "I3DDMA (W)",
                "UNKNOWN",
                "UNKNOWN",
                "SNR(R)",
                "DMAto3DTable (R)",
                "I3DDMA1 (W)"
        };

        MISC_ERR("[%s]reg val:0x%08x\n", msg, val);

        if(val & 0x1F000) {
                tmp = (val & 0x1F000) >> 12;
                if(tmp < 17)
                    MISC_ERR("%s - Error_zero_length : %s\n", msg, Error_zero_length_id_str[tmp]);
        }

        if(val & BIT(9)) {
                MISC_ERR("%s - Error_read_full\n", msg);
        }

        if(val & BIT(8)) {
                MISC_ERR("%s - Error_write_full\n", msg);
        }

#if 0//unused error req id
        if(val & 0xF) {
                tmp = val & 0xF;
                MISC_ERR("%s - Error_req : %s\n", msg, Error_req_id[tmp]);
        }
#endif
}

void dump_tvsb3_error_status(unsigned int val, char * msg)
{
        unsigned int tmp;
        char * Error_zero_length_id_str[3] = {
                "null",
                "SE (W/R)",
                "CP (W/R)"
        };


        MISC_ERR("[%s]reg val:0x%08x\n", msg, val);

        if(val & 0xF000) {
                tmp = (val & 0xF000) >> 12;
                if(tmp < 3)
                    MISC_ERR("%s - Error_zero_length : %s\n", msg, Error_zero_length_id_str[tmp]);
        }

        if(val & BIT(9)) {
                MISC_ERR("%s - Error_read_full\n", msg);
        }

        if(val & BIT(8)) {
                MISC_ERR("%s - Error_write_full\n", msg);
        }
        
}

void dump_tvsb4_error_status(unsigned int val, char * msg)
{
        unsigned int tmp;
        char * Error_zero_length_id_str[10] = {
                "null",
                "OD (W)",
                "OD (R)",
                "UNKNOWN",
                "UNKNOWN",
                "OSD1 (R)",
                "OSD2 (R)",
                "OSD3 (R)",
                "Demura (R)",
                "dmato3dlut (R)"
        };

        MISC_ERR("[%s]reg val:0x%08x\n", msg, val);

        if(val & 0xF000) {
                tmp = (val & 0xF000) >> 12;
                if(tmp < 10)
                    MISC_ERR("%s - Error_zero_length : %s\n", msg, Error_zero_length_id_str[tmp]);
        }

        if(val & BIT(9)) {
                MISC_ERR("%s - Error_read_full\n", msg);
        }

        if(val & BIT(8)) {
                MISC_ERR("%s - Error_write_full\n", msg);
        }

#if 0//unused error req id
        if(val & 0xF) {
                tmp = val & 0xF;
                MISC_ERR("%s - Error_req : %s\n", msg, Error_id_str[tmp]);
        }
#endif
}

void dump_tvsb5_error_status(unsigned int val, char * msg)
{
        unsigned int tmp;
        char * Error_zero_length_id_str[4] = {
                "null",
                "demod(W/R)",
                "demoddbg(W)",
                "demod2(W/R)"
        };

        MISC_ERR("[%s]reg val:0x%08x\n", msg, val);

        if(val & 0xF000) {
                tmp = (val & 0xF000) >> 12;
                if(tmp < 4)
                    MISC_ERR("%s - Error_zero_length : %s\n", msg, Error_zero_length_id_str[tmp]);
        }

        if(val & BIT(10)) {
                MISC_ERR("%s - Error_wdone_fifo\n", msg);
        }

        if(val & BIT(9)) {
                MISC_ERR("%s - Error_read_full\n", msg);
        }

        if(val & BIT(8)) {
                MISC_ERR("%s - Error_write_full\n", msg);
        }

#if 0//unused error req id
        if(val & 0xF) {
                tmp = val & 0xF;
                MISC_ERR("%s - Error_req : %s\n", msg, Error_id_str[tmp]);
        }
#endif
}

void dump_tvsb7_error_status(unsigned int val, char * msg)
{
        unsigned int tmp;
        char * Error_zero_length_id_str[3] = {
                "null",
                "VE (W/R)",
                "TV_SSB7 (W/R)"
        };

        MISC_ERR("[%s]reg val:0x%08x\n", msg, val);

        if(val & 0xF000) {
                tmp = (val & 0xF000) >> 12;
                if(tmp < 3)
                    MISC_ERR("%s - Error_zero_length : %s\n", msg, Error_zero_length_id_str[tmp]);
        }

        if(val & BIT(9)) {
                MISC_ERR("%s - Error_read_full\n", msg);
        }

        if(val & BIT(8)) {
                MISC_ERR("%s - Error_write_full\n", msg);
        }

}


void dump_128bit_bridge_error_status_all(void)
{
        unsigned int sb1_dcu1_error_status = rtd_inl(TVSB1_TV_SB1_DCU1_error_status_reg);
        unsigned int sb2_dcu1_error_status = rtd_inl(TVSB2_TV_SB2_DCU1_error_status_reg);
        unsigned int sb3_dcu1_error_status = rtd_inl(TVSB3_TV_SB3_DCU1_error_status_reg);
        unsigned int sb4_dcu1_error_status = rtd_inl(TVSB4_TV_SB4_DCU1_error_status_reg);
        unsigned int sb5_dcu1_error_status = rtd_inl(TVSB5_TV_SB5_DCU1_error_status_reg);
        unsigned int sb7_dcu1_error_status = rtd_inl(TVSB7_TV_SB7_DCU1_error_status_reg);

	dump_tvsb1_error_status(sb1_dcu1_error_status, "128bit_sb1_error");
        dump_tvsb2_error_status(sb2_dcu1_error_status, "128bit_sb2_error");
        dump_tvsb3_error_status(sb3_dcu1_error_status, "128bit_sb3_error");
        dump_tvsb4_error_status(sb4_dcu1_error_status, "128bit_sb4_error");
        dump_tvsb5_error_status(sb5_dcu1_error_status, "128bit_sb5_error");
        dump_tvsb7_error_status(sb7_dcu1_error_status, "128bit_sb7_error");
}

void dump_64bit_bridge_error_status(unsigned int val, char * msg)
{
        unsigned int tmp;

        MISC_ERR("[%s]reg val:0x%08x\n", msg, val);

        if(val & BIT(30)) {
                MISC_ERR("%s : err_bl_zero\n", msg);
        }

        if(val & BIT(29)) {
                MISC_ERR("%s : err_blk_wxh_odd\n", msg);
        }

        tmp = val & 0x0F000000;
        tmp = tmp >> 24;
        MISC_ERR("%s : max_wcmd_in_fifo is %d\n", msg, tmp);

        tmp = val & 0x00F00000;
        tmp = tmp >> 20;
        MISC_ERR("%s : max_rcmd_in_fifo is %d\n", msg, tmp);

        tmp = val & 0x000FC000;
        tmp = tmp >> 14;
        MISC_ERR("%s : max_wait_wdone is %d\n", msg, tmp);

        tmp = val & 0x00003F80;
        tmp = tmp >> 7;
        MISC_ERR("%s : max_wdata_in_fifo is %d\n", msg, tmp);

        tmp = val & 0x0000007F;
        MISC_ERR("%s : max_rdata_in_fifo is %d\n", msg, tmp);
}

void dump_64bit_bridge_error_status_all(void)
{
        unsigned int dc_64_err_status_aio_status = rtd_inl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_aio_reg);
        unsigned int dc_64_err_status_sb1_status = rtd_inl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_sb1_reg);
        unsigned int dc_64_err_status_sb3_status = rtd_inl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_sb3_reg);

        dump_64bit_bridge_error_status(dc_64_err_status_aio_status, "dc_64_err_status_aio");
        dump_64bit_bridge_error_status(dc_64_err_status_sb1_status, "dc_64_err_status_sb1");
        dump_64bit_bridge_error_status(dc_64_err_status_sb3_status, "dc_64_err_status_sb3");
}

int is_dcsys_errcmd(void)
{
        int i, j;
        for(i = 0; i < DC_NUMBER; ++i) {
                for(j = 0; j < DC_SYS_NUMBER; ++j) {
                        if(rtd_inl(dcsys_errcmd_regs.addcmd_hi[i][j]) & BIT(31)) {
                                return 1;
                        }
                }
        }

        return 0;
}

#ifdef DCMT_DC2
int is_ib_err_status(void)
{
        unsigned int ib_err_status_reg = 0;
        unsigned int ib_err_status = 0;
        //unsigned int ib_trap_status_reg = 0;
        //unsigned int ib_trap_status = 0;
        int i;


        for(i = 0; i < IB_CLIENTS_NUM; i++)
        {
                ib_err_status_reg = IB_IB_ERROR_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                ib_err_status = rtd_inl(ib_err_status_reg);
                ib_err_status &= ~(1<<31);//ignore bit 31(clear status bit)
                if(ib_err_status)
                {
                        return 1;
                }
        }

        return 0;
}

int is_ib_err_trap(void)
{
        unsigned int ib_trap_status_reg = 0;
        unsigned int ib_trap_status = 0;
        int i;


        for(i = 0; i < IB_CLIENTS_NUM; i++)
        {
                ib_trap_status_reg = IB_IB_ERROR_TRAP_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                ib_trap_status = rtd_inl(ib_trap_status_reg);
                if(ib_trap_status & 0x7ff00000) //check bit 30..20 for ib error status bit
                {
                        return 1;
                }
        }

        return 0;
}
#endif // #ifdef DCMT_DC2

int is_dcsys_debug_status(void)
{
        unsigned int tmp = 0;

        tmp |= rtd_inl(DC_SYS_DC_debug_status_reg);
#ifdef DCMT_DC2
        tmp |= rtd_inl(DC2_SYS_DC_debug_status_reg);
#endif // #ifdef DCMT_DC2

        if(tmp)
        {
                return 1;
        }

        return 0;
}

int is_mc_err_status(void)
{
        unsigned int tmp = 0;

        tmp |= rtd_inl(DC1_MC_MCFIFO_DBG_CMD0_reg);
        tmp |= rtd_inl(DC1_MC_MCFIFO_DBG_CMD1_reg);
        tmp |= rtd_inl(DC1_MC_MCFIFO_DBG_CMD2_reg);
        tmp |= rtd_inl(DC1_MC_MCFIFO_DBG_CMD3_reg);
#ifdef DCMT_DC2
        tmp |= rtd_inl(DC2_MC_MCFIFO_DBG_CMD0_reg);
        tmp |= rtd_inl(DC2_MC_MCFIFO_DBG_CMD1_reg);
        tmp |= rtd_inl(DC2_MC_MCFIFO_DBG_CMD2_reg);
        tmp |= rtd_inl(DC2_MC_MCFIFO_DBG_CMD3_reg);
#endif // #ifdef DCMT_DC2

        if(tmp)
        {
                return 1;
        }

        return 0;
}

int is_128bit_bridge_error_status(void)
{
        unsigned int tmp = 0;

        tmp |= rtd_inl(TVSB1_TV_SB1_DCU1_error_status_reg);
        tmp |= rtd_inl(TVSB2_TV_SB2_DCU1_error_status_reg);
        tmp |= rtd_inl(TVSB4_TV_SB4_DCU1_error_status_reg);
        tmp |= rtd_inl(TVSB5_TV_SB5_DCU1_error_status_reg);

        if(tmp)
        {
                return 1;
        }

        return 0;
}

int is_64bit_bridge_error_status(void)
{
        unsigned int tmp = 0;

        tmp |= rtd_inl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_aio_reg);
        tmp |= rtd_inl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_sb1_reg);
        tmp |= rtd_inl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_sb3_reg);

        if(tmp)
        {
                return 1;
        }

        return 0;
}

int is_misc_intr(void)
{
        int ret = 0;
        if(is_dcsys_errcmd())
        {
                MISC_ERR("detected dcsys errcmd!\n");
                ret =  1;
        }

#if 0 //there is no intr for debug status
        if(is_dcsys_debug_status())
        {
                MISC_ERR("detected dcsys error status!\n");
                ret =  1;
        }
#endif

#ifdef DCMT_DC2
        if(is_ib_err_status())
        {
                MISC_ERR("detected ib error status!\n");
                ret =  1;
        }
#endif // #ifdef DCMT_DC2
        /*
        if(is_ib_err_trap())
        {
                MISC_ERR("detected ib error trap!\n");
                ret =  1;
        }

        if(is_mc_err_status())
        {
                MISC_ERR("detected mc error status!\n");
                ret =  1;
        }
        if(is_128bit_bridge_error_status())
        {
                MISC_ERR("detected 128-bridge error status!\n");
                ret =  1;
        }
        if(is_64bit_bridge_error_status())
        {
                MISC_ERR("detected 64-bridge error status!\n");
                ret =  1;
        }
        */

        return ret;
}


static void clear_dc_errcmd_reg(int dc_id, int sys_id)
{
        rtd_outl(dcsys_errcmd_regs.addcmd_hi[dc_id][sys_id], 0);
        rtd_outl(dcsys_errcmd_regs.addcmd_lo[dc_id][sys_id], 0);
        rtd_outl(dcsys_errcmd_regs.dc_int[dc_id][sys_id], BIT(23)|BIT(22)|BIT(21)|BIT(20));
}

static void clear_dc_errcmd_reg_all(void)
{
        clear_dc_errcmd_reg(DC_ID_1, DC_SYS1);
        clear_dc_errcmd_reg(DC_ID_1, DC_SYS2);
        clear_dc_errcmd_reg(DC_ID_1, DC_SYS3);
#ifdef DCMT_DC2
        clear_dc_errcmd_reg(DC_ID_2, DC_SYS1);
        clear_dc_errcmd_reg(DC_ID_2, DC_SYS2);
        clear_dc_errcmd_reg(DC_ID_2, DC_SYS3);
#endif // #ifdef DCMT_DC2
}

static void clear_dcsys_debug_status(void)
{
        rtd_outl(DC_SYS_DC_debug_status_reg, 0);
#ifdef DCMT_DC2
        rtd_outl(DC2_SYS_DC_debug_status_reg, 0);
#endif // #ifdef DCMT_DC2
}
#ifdef DCMT_DC2
static void clear_ib_errcmd(void)
{
        unsigned int ib_err_status_reg = 0;
        unsigned int ib_trap_status_reg = 0;
        unsigned int ib_trap_status2_reg = 0;
        int i;

        for(i = 0; i < IB_CLIENTS_NUM; i++)
        {
                ib_err_status_reg = IB_IB_ERROR_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                rtd_outl(ib_err_status_reg, 0);
                ib_trap_status_reg = IB_IB_ERROR_TRAP_STATUS_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                rtd_outl(ib_trap_status_reg, 0);
                ib_trap_status2_reg = IB_IB_ERROR_TRAP_STATUS2_reg + IB_CLIENTS_REG_ADDRESS_OFFSET * i;
                rtd_outl(ib_trap_status2_reg, 0);
        }
}
#endif // #ifdef DCMT_DC2
static void clear_mc_err_status(void)
{
        rtd_outl(DC1_MC_MCFIFO_DBG_CMD0_reg, BIT(31));
        rtd_outl(DC1_MC_MCFIFO_DBG_CMD1_reg, BIT(31));
        rtd_outl(DC1_MC_MCFIFO_DBG_CMD2_reg, BIT(31));
        rtd_outl(DC1_MC_MCFIFO_DBG_CMD3_reg, BIT(31));
#ifdef DCMT_DC2
        rtd_outl(DC2_MC_MCFIFO_DBG_CMD0_reg, BIT(31));
        rtd_outl(DC2_MC_MCFIFO_DBG_CMD1_reg, BIT(31));
        rtd_outl(DC2_MC_MCFIFO_DBG_CMD2_reg, BIT(31));
        rtd_outl(DC2_MC_MCFIFO_DBG_CMD3_reg, BIT(31));
#endif // #ifdef DCMT_DC2
}

static void clear_128bit_bridge_error_status(void)
{
        rtd_outl(TVSB1_TV_SB1_DCU1_error_status_reg, BIT(31));
        rtd_outl(TVSB2_TV_SB2_DCU1_error_status_reg, BIT(31));
        rtd_outl(TVSB4_TV_SB4_DCU1_error_status_reg, BIT(31));
        rtd_outl(TVSB5_TV_SB5_DCU1_error_status_reg, BIT(31));
}

static void clear_64bit_bridge_error_status(void)
{
        rtd_outl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_aio_reg, BIT(31));
        rtd_outl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_sb1_reg, BIT(31));
        rtd_outl(DC_SYS_64BIT_WRAPPER_DC_64_err_status_sb3_reg, BIT(31));
}

void dump_misc_error_stauts(void)
{
        is_misc_intr();

        dump_dc_errcmd_all();

        dump_dcsys_debug_status();
        //console_flush_on_panic();
#ifdef DCMT_DC2
        dump_ib_errcmd_reg();
        dump_ib_errcmd();
#endif // #ifdef DCMT_DC2
        //console_flush_on_panic();

        dump_mc_error_status_all();

        dump_128bit_bridge_error_status_all();

        dump_64bit_bridge_error_status_all();
        //console_flush_on_panic();
}
EXPORT_SYMBOL(dump_misc_error_stauts);


unsigned int rtk_dbg_atomic_inc_read(void)
{
    return SB2_DBG4_1_get_atom_inc_read(rtd_inl(SB2_DBG4_1_reg));
}
EXPORT_SYMBOL(rtk_dbg_atomic_inc_read);


unsigned int rtk_dbg_atomic_read(void)
{
    return SB2_DBG4_2_get_atom_read(rtd_inl(SB2_DBG4_2_reg));
}
EXPORT_SYMBOL(rtk_dbg_atomic_read);


void rtk_dbg_atomic_val_set(unsigned int val)
{
    rtd_outl(SB2_DBG4_1_reg, SB2_DBG4_1_atom_inc_read(val));
}
EXPORT_SYMBOL(rtk_dbg_atomic_val_set);


irqreturn_t misc_isr (int irq, void *dev_id)
{
        unsigned int old_loglevel;

        if(!is_misc_intr())
                return IRQ_NONE;

        old_loglevel = console_loglevel;
        if(console_loglevel < 3)
        {
                console_loglevel = 5;
        }

        //dump_vo_regs();
        is_in_misc_isr = 1;

        dump_misc_error_stauts();
        
        MISC_ERR("MISC ISR Dump Stacks!!!!\n");
        dump_stacks();

#if 1 //panic mode
        panic("%s detected error!",__FUNCTION__);
#else //non panic mode
        dump_stacks();
#endif
        /*clear DC_SYS intr pendding : because DC_SYS interrupt sending when any error status set to '1'
          so, if we clear every error status bits, there will be no interrupt any more.
        */
        clear_dc_errcmd_reg_all();
        clear_dcsys_debug_status();
#ifdef DCMT_DC2
        clear_ib_errcmd();
#endif // #ifdef DCMT_DC2
        clear_mc_err_status();
        clear_128bit_bridge_error_status();
        clear_64bit_bridge_error_status();

        console_loglevel = old_loglevel;
        
        is_in_misc_isr = 0;
        
        return IRQ_HANDLED;
}

//                          Add for new device PM driver.
#ifdef CONFIG_PM
static int misc_pm_suspend(
        struct platform_device           *dev,
        pm_message_t                    state
)
{
        return 0;
}

static int misc_pm_resume(
        struct platform_device           *dev
)
{
        MISC_INFO("misc resume\n");

        /* enable error commands detection of DCSYS1 and DCSYS2 */
        rtd_outl(DC_SYS_DC_int_enable_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC_SYS_DC_int_enable_SYS2_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC_SYS_DC_int_enable_SYS3_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC_SYS_DC_EC_CTRL_reg, BIT(3) | BIT(2) | BIT(1) | BIT(0));

#if 0 //code for IB/DC2
        rtd_outl(DC2_SYS_DC_int_enable_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC2_SYS_DC_int_enable_SYS2_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC2_SYS_DC_int_enable_SYS3_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC2_SYS_DC_EC_CTRL_reg, BIT(3) | BIT(2) | BIT(1) | BIT(0));
#endif
        rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, BIT(11) | BIT(0));   //enable global interrupt

		rtk_ddr_debug_resume();

        return 0;
}

static struct platform_device *misc_platform_devs;

static struct platform_driver misc_device_driver = {
        .suspend    = misc_pm_suspend,
        .resume     = misc_pm_resume,
        .driver = {
                .name       = "sys-misc",
                .bus        = &platform_bus_type,
        } ,
} ;
#endif // CONFIG_PM



#ifdef CONFIG_MEM_BACKGROUND_TEST

extern u32 __pure crc32_le(u32 crc, unsigned char const *p, size_t len);

enum MC_REGION {
    MC_region_48,
    MC_region_32,
    MC_region_16,
    MC_region_all,
};


static char *region_label[]= {
    "MC_region_48",
    "MC_region_32",
    "MC_region_16",
    "MC_region_all",
};

static enum MC_REGION mtest_region = MC_region_all;
static size_t mtest_size = 0x400 * 0x400;
static unsigned int mtest_pattern = 0x2379beef;
static struct task_struct *test_th = NULL;
static bool mtest_active = false;



static int mtest_alloc_region(void **addr, void **uaddr,
        size_t size, enum MC_REGION region)
{
    int ret = 0;
    static int region_cnt = 0;

    if (region == MC_region_all) {
        region = region_cnt % MC_region_all;
        region_cnt++;
        if (region_cnt == MC_region_all)
            region_cnt = 0;
    }

    /*MISC_ERR("%s: %s\n", __func__, region_label[region]);*/

    switch (region) {
    case MC_region_48:
    case MC_region_32:
    case MC_region_16:
        *addr = dvr_malloc_uncached(size, uaddr);
        if (!(*addr))
            ret = -ENOMEM;
        break;
    default:
        ret = -ENOMEM;
    }

    return ret;
}


static void mtest_free_region(void *buf)
{
    dvr_free(buf);
}



static int mtest_rw_verify(void *read_from, void *write_to,
        size_t size, unsigned int pattern)
{
    int i = 0;
    unsigned int *buf;
    u32 golden_crc, crc;

    // fill pattern
    for (buf = read_from; i < size; i += sizeof(pattern)) {
        *buf = pattern;
        buf++;
    }

    /* check previous step is ok */
    for (buf = read_from; i < size; i += sizeof(pattern)) {
        if (*buf != pattern) {
            rtd_pr_misc_err("%s(%d), [%d] buf=%x, pattern=%x\n",  __func__, __LINE__, i, *buf, pattern);
            /*panic("%s: test data not match to pattern\n", __func__);*/
            return -EFAULT;
        }
        buf++;
    }

    /*dmac_flush_range(ret, ret + size);*/

    golden_crc = crc32_le(0, (unsigned char const *)read_from, size);

    memcpy(write_to, read_from, size);

    crc = crc32_le(0, (unsigned char const *)write_to, size);

    if (golden_crc != crc) {
        rtd_pr_misc_err("%s(%d), crc not match\n",  __func__, __LINE__);
        return -EFAULT;
    }

    return 0;
}


static void mtest_dump_diff(unsigned int *src, unsigned int *dest,
        size_t size, unsigned int pattern)
{
    local_irq_disable();

    console_loglevel = 4;

    rtd_pr_misc_emerg("mtest fail: start from source(%p), dest(%p), size=0x%x, pattern=%x\n",
            src, dest, size, pattern);

    while (size > 0) {
        if (*src != *dest)
            rtd_pr_misc_emerg("diff: [%p]=%x, [%p]=%x\n", src, *src, dest, *dest);
        src++;
        dest++;
        size -= sizeof(*src);
    }

    local_irq_enable();
}


static int mtest_th_fn(void *data)
{
    int ret = 0;
    int region_cnt = 0;


    set_freezable();

    while (!kthread_should_stop()) {
        unsigned int pattern;
        void *read_from = NULL, *read_from_u = NULL, *write_to = NULL, *write_to_u = NULL;
        size_t size = mtest_size;
        enum MC_REGION region = mtest_region;

        try_to_freeze();

        /* ddr scan loop */
        /**
         *  1. allocate mem
         *  2. rw then compare two buffer checksum
         *  3. free mem
         */
        if (region == MC_region_all)
            region += region_cnt;

        ret = mtest_alloc_region(&read_from, &read_from_u, size, region) ||
            mtest_alloc_region(&write_to, &write_to_u, size, region);
        if (ret) {
            MISC_ERR("%s: alloc_testing_region fail r(%px) w(%px)\n", __func__, read_from, write_to);

            if (read_from) {
                mtest_free_region(read_from);
            }
            if (write_to) {
                mtest_free_region(write_to);
            }
            goto nr;
        }


        pattern = mtest_pattern;

        ret = mtest_rw_verify(read_from_u, write_to_u, mtest_size, pattern);
        if (ret) {
            mtest_dump_diff(read_from_u, write_to_u, mtest_size, pattern);
            panic("%s: mtest fail\n", __func__);
        }

        mtest_free_region(read_from);
        mtest_free_region(write_to);
nr:
        msleep_interruptible(3000);
    }


    return 0;
}


static int start_mtest_th(void)
{
    struct task_struct *th;
    if (test_th) {
        MISC_ERR("%s: thread existed!!\n", __func__);
        return -EBUSY;
    }

    th = kthread_run(mtest_th_fn, NULL, "mtest_th");
    if (IS_ERR(th)) {
        MISC_ERR("%s: Failed to create thread\n", __func__);
        return PTR_ERR(th);
    }
    MISC_INFO("%s: mtest_th ok\n", __func__);
    test_th = th;

    return 0;
}


static void stop_mtest_th(void)
{
    if (test_th) {
        kthread_stop(test_th);
        MISC_ERR("%s: stop mtest_th ok\n", __func__);
        test_th = NULL;
    }
}



static ssize_t show_mtest_active(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", mtest_active);
}


static ssize_t store_mtest_active(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    int active = 0, ret = 0;

    if (sscanf(buf, "%d\n", &active) != 1)
        return -EINVAL;

    if (!!active) {
        ret = start_mtest_th();
    } else {
        stop_mtest_th();
    }

    mtest_active = !!active;

    return ret != 0 ? ret : count;
}
static DEVICE_ATTR(mtest_active, 0644, show_mtest_active, store_mtest_active);



static ssize_t show_mtest_pattern(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "0x%08x\n", mtest_pattern);
}

static ssize_t store_mtest_pattern(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    unsigned int pattern = 0x0;

    if (sscanf(buf, "%x\n", &pattern) != 1)
        return -EINVAL;

    mtest_pattern = pattern;
    return count;
}
static DEVICE_ATTR(mtest_pattern, 0644, show_mtest_pattern, store_mtest_pattern);



static ssize_t show_mtest_size(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE-1, "0x%x\n", mtest_size);
}

static ssize_t store_mtest_size(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    unsigned int size = 0x0;

    if (sscanf(buf, "%x\n", &size) != 1)
        return -EINVAL;

    mtest_size = ALIGN(size, sizeof(mtest_pattern));
    return count;
}
static DEVICE_ATTR(mtest_size, 0644, show_mtest_size, store_mtest_size);



static ssize_t show_mtest_region(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    int n = 0, i = 0;
    size_t size = PAGE_SIZE;

    for (i = 0; i <= MC_region_all || size <= 0; i++) {
        if (i == mtest_region)
            n = snprintf(buf, size, "[v] %d: %s\n", i, region_label[i]);
        else
            n = snprintf(buf, size, "[]  %d: %s\n", i, region_label[i]);

        size -= n;
        buf += n;
    }

    return PAGE_SIZE - size;
}

static ssize_t store_mtest_region(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    int region_num = 0;

    if (sscanf(buf, "%d\n", &region_num) != 1)
        return -EINVAL;

    if (region_num < 0 || region_num > MC_region_all)
        return -EINVAL;

    mtest_region = region_num;

    return count;
}
static DEVICE_ATTR(mtest_region, 0644, show_mtest_region, store_mtest_region);


#ifdef CONFIG_RTK_KDRV_MD
#include <rtk_kdriver/md/rtk_md.h>

#define SIZE_PER_SCAN    0x1000

static struct task_struct *test_md_th = NULL;
static bool mtest_md_active = false;


static void dump_data(unsigned int *start, size_t size)
{
    int i = 0;

    for (i = 0; i < size; i += sizeof(unsigned int)) {
        MISC_EMERG("%s[%p]=0x%08x\n", __func__, start, *start);
        start++;
    }
}


static int mtest_md_th_fn(void *data)
{
    unsigned long phys_to;
    void *va_to, *uva_to, *va_from;
    void *test_start, *test_end;


    test_start = (void *)round_up((unsigned long)_stext, SIZE_PER_SCAN);
    test_end = (void *)round_down((unsigned long)_etext, SIZE_PER_SCAN);

    va_to = dvr_malloc_uncached(SIZE_PER_SCAN, &uva_to);
    if (!va_to)
        return -ENOMEM;

    phys_to = dvr_to_phys(va_to);

    set_freezable();

    while (1) {
        for (va_from = test_start; va_from < (test_end - SIZE_PER_SCAN); va_from += SIZE_PER_SCAN) {
            unsigned long pa_from = (unsigned long)virt_to_phys(va_from);
            u32 golden_crc, crc;

            try_to_freeze();

            golden_crc = crc32_le(0, (unsigned char const *)va_from, SIZE_PER_SCAN);

            smd_memcpy((uint32_t)phys_to, (uint32_t)pa_from, SIZE_PER_SCAN);
            smd_checkComplete();

            crc = crc32_le(0, (unsigned char const *)uva_to, SIZE_PER_SCAN);
            if (crc != golden_crc) {
                /* TODO: write alarm bit to dummy register */
                dump_data((unsigned int *)va_to, SIZE_PER_SCAN);
                panic("%s: fail, crc mismatch. 0x%08x ; 0x%08x(golden). from(0x%lx) to(0x%lx), size(0x%x)",
                        __func__, crc, golden_crc, pa_from, phys_to, SIZE_PER_SCAN);
            }

            MISC_DBG("%s: [0x%lx - 0x%lx) crc=%08x. golden=%08x\n", __func__,
                    pa_from, pa_from + SIZE_PER_SCAN,
                    crc, golden_crc);

            if (kthread_should_stop())
                goto out;

            msleep_interruptible(5000);
        }
    }
out:
    dvr_free(va_to);

    return 0;
}


static int start_mtest_md_th(void)
{
    struct task_struct *th;
    if (test_md_th) {
        MISC_ERR("%s: thread existed!!\n", __func__);
        return -EBUSY;
    }

    th = kthread_run(mtest_md_th_fn, NULL, "mtest_md_th");
    if (IS_ERR(th)) {
        MISC_ERR("%s: Failed to create thread\n", __func__);
        return PTR_ERR(th);
    }
    MISC_INFO("%s: mtest_md_th ok\n", __func__);
    test_md_th = th;

    return 0;
}


static void stop_mtest_md_th(void)
{
    if (test_md_th) {
        kthread_stop(test_md_th);
        MISC_INFO("%s: stop mtest_md_th ok\n", __func__);
        test_md_th = NULL;
    }
}


static ssize_t show_mtest_md_active(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    return snprintf(buf, PAGE_SIZE, "%d\n", mtest_md_active);
}


static ssize_t store_mtest_md_active(struct device *dev,
        struct device_attribute *attr,
        const char *buf, size_t count)
{
    int active = 0, ret = 0;

    if (sscanf(buf, "%d\n", &active) != 1)
        return -EINVAL;

    if (!!active) {
        ret = start_mtest_md_th();
    } else {
        stop_mtest_md_th();
    }

    mtest_md_active = !!active;

    return ret != 0 ? ret : count;
}
static DEVICE_ATTR(mtest_md_active, 0644, show_mtest_md_active, store_mtest_md_active);
#endif /* CONFIG_RTK_KDRV_MD */


static struct attribute *dev_attrs[] = {
    &dev_attr_mtest_active.attr,
    &dev_attr_mtest_pattern.attr,
    &dev_attr_mtest_size.attr,
    &dev_attr_mtest_region.attr,
#ifdef CONFIG_RTK_KDRV_MD
    &dev_attr_mtest_md_active.attr,
#endif
    NULL,
};

static const struct attribute_group dev_attr_grp = {
    .attrs = dev_attrs,
};


static int mtest_kernel_setup(char *buf)
{
    char *p = NULL;

    p = strchr(buf, (int)'1');
    if (p)
        mtest_active = true;

    return 0;
}
__setup("mtest_kernel", mtest_kernel_setup);


#ifdef CONFIG_RTK_KDRV_MD
static int mtest_md_kernel_setup(char *buf)
{
    char *p = NULL;

    p = strchr(buf, (int)'1');
    if (p)
        mtest_md_active = true;

    return 0;
}
__setup("mtest_md_kernel", mtest_md_kernel_setup);
#endif

#endif  /*  CONFIG_MEM_BACKGROUND_TEST */


struct file_operations misc_fops = {
        .owner              = THIS_MODULE,
        .unlocked_ioctl     = misc_ioctl,
        .open               = misc_open,
        .release            = misc_release,
        .read           = misc_read,
        .write = misc_write,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void misc_cleanup_module(void)
{
        dev_t dev = MKDEV(misc_major, misc_minor);

        MISC_INFO( "misc clean module\n");

#ifdef CONFIG_MEM_BACKGROUND_TEST
        if (mtest_active)
                stop_mtest_th();
#ifdef CONFIG_RTK_KDRV_MD
         if (mtest_md_active)
                 stop_mtest_md_th();
#endif
#endif
        /* Get rid of our char dev entries */
        if (misc_drv_dev) {
                device_destroy(misc_class, dev);
                cdev_del(&misc_drv_dev->cdev);
                kfree(misc_drv_dev);
#ifdef CONFIG_PM
                platform_device_unregister(misc_platform_devs);
#endif
        }

        unregister_chrdev_region(dev, 1);
}

static void init_dc_errcmd_regs(void)
{
        dcsys_errcmd_regs.addcmd_hi[0][0] = DC_SYS_DC_EC_ADDCMD_HI_reg;
        dcsys_errcmd_regs.addcmd_hi[0][1] = DC_SYS_DC_EC_ADDCMD_HI_SYS2_reg;
        dcsys_errcmd_regs.addcmd_hi[0][2] = DC_SYS_DC_EC_ADDCMD_HI_SYS3_reg;
#ifdef DCMT_DC2
        dcsys_errcmd_regs.addcmd_hi[1][0] = DC2_SYS_DC_EC_ADDCMD_HI_reg;
        dcsys_errcmd_regs.addcmd_hi[1][1] = DC2_SYS_DC_EC_ADDCMD_HI_SYS2_reg;
        dcsys_errcmd_regs.addcmd_hi[1][2] = DC2_SYS_DC_EC_ADDCMD_HI_SYS3_reg;
#endif // #ifdef DCMT_DC2

        dcsys_errcmd_regs.addcmd_lo[0][0] = DC_SYS_DC_EC_ADDCMD_LO_reg;
        dcsys_errcmd_regs.addcmd_lo[0][1] = DC_SYS_DC_EC_ADDCMD_LO_SYS2_reg;
        dcsys_errcmd_regs.addcmd_lo[0][2] = DC_SYS_DC_EC_ADDCMD_LO_SYS3_reg;
#ifdef DCMT_DC2
        dcsys_errcmd_regs.addcmd_lo[1][0] = DC2_SYS_DC_EC_ADDCMD_LO_reg;
        dcsys_errcmd_regs.addcmd_lo[1][1] = DC2_SYS_DC_EC_ADDCMD_LO_SYS2_reg;
        dcsys_errcmd_regs.addcmd_lo[1][2] = DC2_SYS_DC_EC_ADDCMD_LO_SYS3_reg;
#endif // #ifdef DCMT_DC2

        dcsys_errcmd_regs.dc_int[0][0] = DC_SYS_DC_int_status_reg;
        dcsys_errcmd_regs.dc_int[0][1] = DC_SYS_DC_int_status_SYS2_reg;
        dcsys_errcmd_regs.dc_int[0][2] = DC_SYS_DC_int_status_SYS3_reg;
#ifdef DCMT_DC2
        dcsys_errcmd_regs.dc_int[1][0] = DC2_SYS_DC_int_status_reg;
        dcsys_errcmd_regs.dc_int[1][1] = DC2_SYS_DC_int_status_SYS2_reg;
        dcsys_errcmd_regs.dc_int[1][2] = DC2_SYS_DC_int_status_SYS3_reg;
#endif // #ifdef DCMT_DC2
}

int misc_init_module(void)
{
        int result;
        dev_t dev = 0;
        struct device *device;

        /*
         * Get a range of minor numbers to work with, asking for a dynamic
         * major unless directed otherwise at load time.
         */

        MISC_INFO( " ***************** misc init module ********************* \n");
        if (misc_major) {
                dev = MKDEV(misc_major, misc_minor);
                result = register_chrdev_region(dev, 1, "sys-misc");
        } else {
                result = alloc_chrdev_region(&dev, misc_minor, 1, "sys-misc");
                misc_major = MAJOR(dev);
        }
        if (result < 0) {
                MISC_ERR("misc: can't get major %d\n", misc_major);
                return result;
        }

        MISC_INFO( "misc init module major number = %d\n", misc_major);

        misc_class = class_create(THIS_MODULE, "sys-misc");
        if (IS_ERR(misc_class))
                return PTR_ERR(misc_class);

        device = device_create(misc_class, NULL, dev, NULL, "sys-misc");
        if (IS_ERR(device)) {
                result = PTR_ERR(device);
                goto fail;
        }
#ifdef CONFIG_MEM_BACKGROUND_TEST
        // sysfs
        result = sysfs_create_group(&device->kobj, &dev_attr_grp);
        if (result) {
                MISC_ERR("%s: Create sysfs group fail. ret=%d\n", __func__, result);
                device_destroy(misc_class, dev);
                goto fail;
        }
#endif

        /*
         * allocate the devices
         */
        misc_drv_dev = kmalloc(sizeof(struct misc_dev), GFP_KERNEL);
        if (!misc_drv_dev) {
                device_destroy(misc_class, dev);
                result = -ENOMEM;
                goto fail;  /* Make this more graceful */
        }
        memset(misc_drv_dev, 0, sizeof(struct misc_dev));

        //initialize device structure
        sema_init(&misc_drv_dev->sem, 1);
        cdev_init(&misc_drv_dev->cdev, &misc_fops);
        misc_drv_dev->cdev.owner = THIS_MODULE;
        misc_drv_dev->cdev.ops = &misc_fops;
        result = cdev_add(&misc_drv_dev->cdev, dev, 1);
        /* Fail gracefully if need be */
        if (result) {
                device_destroy(misc_class, dev);
                kfree(misc_drv_dev);
                MISC_ERR("Error %d adding cdev misc", result);
                goto fail;
        }

#ifdef CONFIG_PM
        misc_platform_devs = platform_device_register_simple("sys-misc", -1, NULL, 0);

        if(platform_driver_register(&misc_device_driver) != 0) {
                device_destroy(misc_class, dev);
                cdev_del(&misc_drv_dev->cdev);
                kfree(misc_drv_dev);
                misc_platform_devs = NULL;
                goto fail;  /* Make this more graceful */
        }
#endif  //CONFIG_PM


#ifdef CONFIG_MEM_BACKGROUND_TEST
        if (mtest_active)
            start_mtest_th();

#ifdef CONFIG_RTK_KDRV_MD
        if (mtest_md_active)
            start_mtest_md_th();
#endif
#endif

        return 0; /* succeed */

fail:
        return result;
}

module_init(misc_init_module);
module_exit(misc_cleanup_module);

#include <mach/rtk_platform.h>
#define DDR_DEBUG_RAM_DUMMY_ADDR	0xb8008EB0 // Dora gave
#define DDR_DEBUG_RAM_INVALID		0xbeef2379

unsigned int ddr_debug_ram_addr = 0;

static void rtk_ddr_debug_fill_dummy(unsigned int addr)
{
	rtd_outl(DDR_DEBUG_RAM_DUMMY_ADDR, addr);
}

static void rtk_ddr_debug_init_dummy(void)
{
	unsigned long addr = 0 ;
	unsigned long size = 0 ;

	size = carvedout_buf_query(CARVEDOUT_K_OS, (void **)&addr);

	if (size > 0) {
		// Use addr and size here
		ddr_debug_ram_addr = addr;
		rtk_ddr_debug_fill_dummy(ddr_debug_ram_addr);
	} else {
		rtk_ddr_debug_fill_dummy(DDR_DEBUG_RAM_INVALID);
		rtd_pr_misc_err("No Security OS memory is reserved\n");
	}
}

static void rtk_ddr_debug_resume(void)
{
	rtk_ddr_debug_fill_dummy(ddr_debug_ram_addr);
}

static int __init rtk_ddr_debug_init(void)
{
	rtk_ddr_debug_init_dummy();

	return 0;
}
late_initcall(rtk_ddr_debug_init);

static int misc_probe(struct platform_device *pdev)
{
        int ret = 0;
        int virq = -1;
        struct device_node *np = pdev->dev.of_node;

        if (!np)
        {
                MISC_ERR("[%s]there is no device node\n",__func__);
                return -ENODEV;
        }

        virq = irq_of_parse_and_map(np, 0);
        if(!virq){
                MISC_ERR("[%s] map misc vitual_irq failed\n",__func__);
                of_node_put(np);
                return -ENODEV;
        }

        if(request_irq(virq,
                       misc_isr,
                       IRQF_SHARED,
                       "DC_ERR",
                       misc_isr)) {
                MISC_ERR("MISC: cannot register IRQ %d\n", irqd_to_hwirq(irq_get_irq_data(virq)));
                return -1;
        }

        /* enable error commands detection of DCSYS1 and DCSYS2 */
        rtd_outl(DC_SYS_DC_int_enable_reg,  BIT(26) | BIT(24) | BIT(23) | BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18)  | BIT(0));
        rtd_outl(DC_SYS_DC_int_enable_SYS2_reg, BIT(23) | BIT(22) | BIT(21) | BIT(20) | BIT(19)  | BIT(0));
        rtd_outl(DC_SYS_DC_int_enable_SYS3_reg, BIT(23) | BIT(22) | BIT(21) | BIT(20) | BIT(19)  | BIT(0));
        rtd_outl(DC_SYS_DC_EC_CTRL_reg, BIT(3) | BIT(2) | BIT(1) | BIT(0));
#ifdef DCMT_DC2
        rtd_outl(DC2_SYS_DC_int_enable_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC2_SYS_DC_int_enable_SYS2_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC2_SYS_DC_int_enable_SYS3_reg, BIT(22) | BIT(21) | BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16) | BIT(15) | BIT(0));
        rtd_outl(DC2_SYS_DC_EC_CTRL_reg, BIT(3) | BIT(2) | BIT(1) | BIT(0));
#endif // #ifdef DCMT_DC2
        rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, BIT(11) | BIT(0));    //enable global interrupt

        init_dc_errcmd_regs();

        of_node_get(np);

        return ret;
}

static int misc_probe_remove(struct platform_device *pdev)
{
        return 0;
}

static const struct of_device_id misc_of_match[] =
{
        {
                .compatible = "realtek,misc",
        },
        {},
};

static struct platform_driver misc_driver =
{
        .probe          = misc_probe,
        .driver = {
                .name = "misc",
                .of_match_table = misc_of_match,
        },
        .remove = misc_probe_remove,
};
MODULE_DEVICE_TABLE(of, misc_of_match);
module_platform_driver(misc_driver);

