#include <linux/slab.h> //kzalloc
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <mach/platform.h>
#include <linux/io.h>
#include <linux/cpu.h>
#include <linux/pm_opp.h>
#include <linux/version.h>
#define CPUFREQ_DRIVER_GET_FACTORY  //must in this
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <rtk_kdriver/rtk_clock.h>
#include <rtk_kdriver/pcbMgr.h>
#include <rtk_gpio.h>
#include <asm/delay.h>
#include <rtk_kdriver/io.h>
#include <rbus/scpu_core_reg.h>
#include <rbus/mis_gpio_reg.h>
#ifdef CONFIG_RTK_KDRV_CPU_FREQUENCY_MODULE
#include <rtk_kdriver/rtk-kdrv-common.h>
#endif
#include <rtd_log/rtd_module_log.h>

#ifndef BUILD_QUICK_SHOW
static LIST_HEAD(clock_list);
static DEFINE_SPINLOCK(clock_lock);
static DEFINE_SPINLOCK(ratio_lock);
static DEFINE_MUTEX(clock_list_sem);
#endif
#define SYS_OC_EN_BIT     0
#define SYS_OC_DONE_BIT     20
#define N_CODE_START 8  //integer portion, 8 bits
#define F_CODE_START 16 //float portion, 11 bits
#define N_CODE_MAX   0xFF   //8 bits
#define F_CODE_MAX   0x7FF  // 11 bits
#define FREQ_ADDITION  3
#define FREQ_XTAL_MHZ	27


#ifdef CONFIG_PV88080_PMIC
static unsigned char pv88080_power_value[2]={0xfd,0xc0};
static int pv88080_exist=1;
#endif
typedef struct  rtk_dvfs_freq_reg
{
	unsigned int divider;
	unsigned int cpu_freq;
	unsigned int oc_control;
	unsigned int oc_status;
}tRtkDvfs;

#define RTK_CPUFREQ_MAX_PLL	1
tRtkDvfs  rtk_dvfs_reg[RTK_CPUFREQ_MAX_PLL]=
{
	{SCPU_CORE_sys_pll_scpu1_reg, SCPU_CORE_sys_pll_scpu2_reg, SCPU_CORE_sys_scpu_dvfs_reg, SCPU_CORE_sys_scpu_status_reg}
};


#define FREQ_LEVEL 12
#define FREQ_STEP 100000   //khz
#define FREQ_XTAL 27000    //khz

#if 0 //no need to control gpu's dfs on 2851c
typedef struct rtk_gpu_dvfs_callback {
    void (*func)(int dfsmode,unsigned int freq);
} st_rtk_gpu_dvfs_callback;

#define RTK_DVFS_CALLBACK_COUNT 2
static st_rtk_gpu_dvfs_callback dvfs_func[RTK_DVFS_CALLBACK_COUNT];


int register_gpu_dvfs_callback(void *fn)
{
	dvfs_func[0].func=fn;
	rtd_pr_cpu_freq_err("function registered:%lx\n",(unsigned long)fn);
	return 0;
}
EXPORT_SYMBOL(register_gpu_dvfs_callback);
#endif

static struct clk rtk_cpuclk[1] = {
{
	.name = "cpu_clk_0",
	.flags = CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES,
},
};

struct clk *__rtk_clk_get(int cpu)
{
	return &rtk_cpuclk[0];
}

static void __rtk_propagate_rate(struct clk *clk)
{
	struct clk *clkp;

	list_for_each_entry(clkp, &clock_list, node) {
		if (likely(clkp->parent != clk))
			continue;
		if (likely(clkp->ops && clkp->ops->recalc))
			clkp->ops->recalc(clkp);
		if (unlikely(clkp->flags & CLK_RATE_PROPAGATES))
			__rtk_propagate_rate(clkp);
	}
}

unsigned long __rtk_clk_get_rate(struct clk *clk)
{
	rtd_pr_cpu_freq_debug("get_rate, %lu\n", (unsigned long)clk->rate);
	return (unsigned long)clk->rate;
}
EXPORT_SYMBOL(__rtk_clk_get_rate);



#ifdef CONFIG_REALTEK_VOLTAGE_CTRL 
//Adding freq table
enum VOLTAGE_LEVEL
{
	SCPU_NONE	= 0,
	SCPU_L_L,
	SCPU_L_H,
	SCPU_H_L,
	SCPU_H_H
};

/*
Set default compatible for 1st and 2nd PCB.
Then after init, driver run dvs then re-set according to dts value.
*/
#define VOLTAGE_LEVEL_DEDAULT	SCPU_L_H

#ifdef  CONFIG_PV88080_PMIC
#define PV88080_I2C_ADDR 0x49
#define PV88080_I2C_SCPU_SUB_ADDR 0x2A
#define PV88080_I2C_CORE_SUB_ADDR 0x33
#define PV88080_I2C_DDR_SUB_ADDR  0x2D
#define PV88080_I2C_STB_SUB_ADDR  0x30
#define PV88080_I2C_BUS_ID 2

static int pv88080_adjust_scpu_voltage(enum VOLTAGE_LEVEL scpu_voltage)
{
	static enum VOLTAGE_LEVEL previous_setting=SCPU_L_L;
	unsigned char buf[2];
	unsigned char saved_value;
	int ret=0;
	if(pv88080_exist==0) //init failed, there is no pv88080, skipping i2c
		return 0;
	switch(scpu_voltage)
	{
		case SCPU_H_H:
			if(previous_setting == SCPU_H_H) //skipping for previous same
				return 0;
			
			//write 0x2A sub add with 0xfd value.
			buf[0]=PV88080_I2C_SCPU_SUB_ADDR;
			buf[1]=pv88080_power_value[0];
			if(buf[1]>=(unsigned char )0xfd)
				buf[1]=(unsigned char)0xfd;
			if(buf[1]<=(unsigned char )0xc0)
				buf[1]=(unsigned char)0xc0;

			saved_value=buf[1];
			i2c_master_send_ex_flag(PV88080_I2C_BUS_ID,PV88080_I2C_ADDR,buf,2,I2C_M_NORMAL_SPEED);

			//Verify
			buf[0]=PV88080_I2C_SCPU_SUB_ADDR;
			buf[1]=0;;
			i2c_master_recv_ex(PV88080_I2C_BUS_ID,PV88080_I2C_ADDR,&buf[0],1,&buf[1],1);
			if(buf[1]==(unsigned char)saved_value)
			{
				previous_setting=SCPU_H_H;	
			}
			else
				ret=-1;
			//rtd_pr_cpu_freq_debug("0 clamp:%x %d %d\n",buf[1],scpu_voltage,previous_setting);
			break;
		default:
			//write 0x2A sub add with 0xc0 value.
			if(previous_setting != SCPU_H_H) //skipping
				return 0;

			//write 0x2A sub add with 0xfd value.
			buf[0]=PV88080_I2C_SCPU_SUB_ADDR;
			buf[1]=pv88080_power_value[1];
			if(buf[1]>=(unsigned char )0xfd)
				buf[1]=(unsigned char)0xfd;
			if(buf[1]<=(unsigned char )0xc0)
				buf[1]=(unsigned char)0xc0;

			saved_value=buf[1];
			i2c_master_send_ex_flag(PV88080_I2C_BUS_ID,PV88080_I2C_ADDR,buf,2,I2C_M_NORMAL_SPEED);

			//Verify
			buf[0]=PV88080_I2C_SCPU_SUB_ADDR;
			buf[1]=0;;
			i2c_master_recv_ex(PV88080_I2C_BUS_ID,PV88080_I2C_ADDR,&buf[0],1,&buf[1],1);
			if(buf[1]==(unsigned char)saved_value)
			{
				previous_setting=SCPU_H_L;	
			}
			else
				ret=-1;
			//rtd_pr_cpu_freq_debug("1 clamp:%x %d %d\n",buf[1],scpu_voltage,previous_setting);
			break;
	}
	return ret;

}
#endif

void rtk_set_io_direction(unsigned char *pin_info,int out_value)
{
	unsigned long long pin;

	if (pcb_mgr_get_enum_info_byname(pin_info, &pin) == 0)
	{
//		rtd_pr_cpu_freq_info("IO set :%s value:%d\n",pin_info,out_value);
		rtk_SetIOPin(pin, out_value);
		rtk_SetIOPinDirection(pin, 1);
	}

}


int rtk_get_voltage_level(struct rtk_cpufreq_info *info, unsigned int pll, unsigned int rate)
{
	unsigned int level;
	struct device *cpu_dev = get_cpu_device(info->policy->cpu);
	struct dev_pm_opp *opp;

#if 0
	if(pll == info->Rpll)
		opp = dev_pm_opp_find_freq_exact(cpu_dev, rate * 1000, 1);
	else
		opp = dev_pm_opp_find_freq_exact(cpu_dev, pll * 1000, 1);
#else
	//always use the rate on 2851c
	opp = dev_pm_opp_find_freq_exact(cpu_dev, rate * 1000, 1);
#endif

	if(IS_ERR(opp))
	{
		rtd_pr_cpu_freq_err("cpu%d: failed to find OPP for %d/%d\n", info->policy->cpu, pll, rate);
		return 0;
	}

	level = dev_pm_opp_get_level(opp);
	dev_pm_opp_put(opp);
	return level;
}
#ifdef RTK_CPUFREQ_BRINGUP_VERIFY
int rtk_cpu_dvs_enable = 0;
#else
int rtk_cpu_dvs_enable = 1;
#endif
EXPORT_SYMBOL_GPL(rtk_cpu_dvs_enable);
#ifdef CONFIG_RTK_KDRV_CPU_FREQUENCY
static int __init rtk_set_cpu_dvs_control(char *str)
{
	if (!strncmp(str, "1", 1))
		rtk_cpu_dvs_enable = 1;
	else if(!strncmp(str, "0", 1))
		rtk_cpu_dvs_enable = 0;
	return 1;
}
__setup("cpu_dvs_control=", rtk_set_cpu_dvs_control);
#else
void rtk_parse_cpu_dvs_control(void)
{

	char strings[4];
	char *str = strings;

	if(rtk_parse_commandline_equal("cpu_dvs_control", str, sizeof(strings)) == 0)
	{
		rtd_pr_cpu_freq_err("Error : can't get cpu_dvs_control from bootargs\n");
		return;
	}

	if (!strncmp(str, "1", 1))
		rtk_cpu_dvs_enable = 1;
	else if(!strncmp(str, "0", 1))
		rtk_cpu_dvs_enable = 0;
	return;
}
#endif
void rtk_set_voltage_pin(int voltage_level)
{
	//GPO_55/GPO_53: PIN_SCPU_VID_0/PIN_SCPU_VID_1
	switch(voltage_level)
	{
		case SCPU_L_L:
		rtk_set_io_direction("PIN_SCPU_VID_1",0);
		rtk_set_io_direction("PIN_SCPU_VID_0",0);
		break;

		case SCPU_L_H:
		rtk_set_io_direction("PIN_SCPU_VID_1",1);
		rtk_set_io_direction("PIN_SCPU_VID_0",0);
		break;

		case SCPU_H_L:
		rtk_set_io_direction("PIN_SCPU_VID_0",1);
		rtk_set_io_direction("PIN_SCPU_VID_1",0);
		break;

		case SCPU_H_H:
		rtk_set_io_direction("PIN_SCPU_VID_0",1);
		rtk_set_io_direction("PIN_SCPU_VID_1",1);
		break;

		default:
		break;
	}

}

static int old_voltage_level = SCPU_NONE;
void rtk_set_voltage(struct rtk_cpufreq_info *info, unsigned int pll, unsigned int rate)
{
	int voltage_level;

	if(rtk_cpu_dvs_enable == 0)
		return;

	/* [ML8BU-507]
		2885p use two pin GPIO55/GPIO53 as SCPU_VID/SCPU_VID2 
		(bootcode define with PIN_SCPU_VID_0/PIN_SCPU_VID_1)
		The DTS use four level {1/2/3/4} as original SCPU_VID/SCPU_VID2 two pin mapping.
	*/
	voltage_level = rtk_get_voltage_level(info, pll, rate);
	if(voltage_level == old_voltage_level)
		return;

	rtd_pr_cpu_freq_err("rtk_set_voltage org(%d): GPO(%x/%x)\n", old_voltage_level, rtd_inl(MIS_GPIO_GP1DATO_reg), rtd_inl(MIS_GPIO_GP1DIR_reg));

	rtk_set_voltage_pin(voltage_level);
	rtd_pr_cpu_freq_err("rtk_set_voltage: pll(%d), rate(%d), vol(%d/%d), GPO(%x/%x)\n",
		pll, rate, voltage_level, old_voltage_level, rtd_inl(MIS_GPIO_GP1DATO_reg), rtd_inl(MIS_GPIO_GP1DIR_reg));

	udelay(50);
	old_voltage_level = voltage_level;
}

void rtk_set_default_voltage(void)
{
	int voltage_level = VOLTAGE_LEVEL_DEDAULT;

	if(rtk_cpu_dvs_enable == 0)
		return;

	rtd_pr_cpu_freq_err("rtk_set_default_voltage org:(%d), GPO(%x/%x)\n", old_voltage_level, rtd_inl(MIS_GPIO_GP1DATO_reg), rtd_inl(MIS_GPIO_GP1DIR_reg));

	rtk_set_voltage_pin(voltage_level);
	old_voltage_level = voltage_level;
	udelay(50);
	rtd_pr_cpu_freq_err("rtk_set_default_voltage:(%d), GPO(%x/%x)\n", voltage_level, rtd_inl(MIS_GPIO_GP1DATO_reg), rtd_inl(MIS_GPIO_GP1DIR_reg));
}
#endif //CONFIG_REALTEK_VOLTAGE_CTRL

#ifdef CONFIG_TV030_PLATFORM
int rtk_get_wafer_class_otp(void)
{
	static int otp = -1;
	if(otp != -1) //already read
	{
		return otp;
	}
	else //first time, read from otp
	{
		if(rtk_otp_field_read(OTP_FIELD_WAFER_CLASS, (unsigned char *)&otp, 4) != 1)
			panic("Read otp OTP_FIELD_WAFER_CLASS fail\n");
		else
			return otp;
	}
}


#define TV030_I2C_ID	1
#define I2C_RT6203E_DEV_ADDR 0x34
static unsigned char rt6203e_fix_voltage[2]={0xaa, 0x2d};// sepc code 42/45 => 1.07V/ 1.10V
static unsigned char rt6203e_value = 0x0;
enum TCL_VOLTAGE_SELECTION
{
	DVFS_GPIO,
	DVFS_RT6203E,
	DVFS_SY8842B,
	DVFS_NONE
};
static enum TCL_VOLTAGE_SELECTION tv030_voltage_init = DVFS_RT6203E;

void rtk_tv030_init_voltage(void)
{
	if((tv030_voltage_init == DVFS_RT6203E) || (rt6203e_value != 0x0) )
	{
		unsigned char buf = 0x71; //over current limit level
		if(i2c_master_send_ex_flag(TV030_I2C_ID,I2C_RT6203E_DEV_ADDR,&buf,1,I2C_M_FAST_SPEED)<0)
		{
			rtd_pr_cpu_freq_err("send rt6203e i2c current:%x\n", buf);
		}

		if(rt6203e_value != 0x0)
			buf = rt6203e_value;
		else
		{
			if((rtk_get_wafer_class_otp() & 0x3) == 0x0)
				buf = rt6203e_fix_voltage[1];	//1.10V
			else
				buf = rt6203e_fix_voltage[0];	//1.07V
		}
		if(i2c_master_send_ex_flag(TV030_I2C_ID,I2C_RT6203E_DEV_ADDR,&buf,1,I2C_M_FAST_SPEED)<0)
		{
			rtd_pr_cpu_freq_err("send rt6203e i2c vout:%x\n", buf);
		}

		rtd_pr_cpu_freq_err("Init rt6203e with 0x:%x\n", buf);
	}
}
#endif //#CONFIG_TV030_PLATFORM

extern int rtk_cpu_dvfs_disable;
int __rtk_clk_set_rate(struct clk *clk, unsigned int rate, int cpu,int idx)
{

	if(rtk_cpu_dvfs_disable == 1)
		return 0;
	
	return __rtk_clk_set_rate_ex(clk, rate, 0, cpu, idx);
}

#if 0
typedef struct scpu_freq_map
{
        unsigned int reg_value;
        unsigned int freq_kHz;
        unsigned int freq2_kHz;
}tscpu_freq_map;

tscpu_freq_map rtk_scpu_freq_map[]= {
        { 0x17B43B00, 1700000, 1511000 }, //1.7
        { 0x12133800, 1600000, 1422000 }, //1.6
        { 0x13433600, 1550000, 1378000 }, //1.55
        { 0x14723400, 1500000, 1333000 }, //1.5
        { 0x16D13000, 1400000, 1244000 }, //1.4
        { 0x11302D00, 1300000, 1156000 }, //1.3
        { 0x138F2900, 1200000, 1067000 }, //1.2
        //{ 0x05ED2500, 1100000, 978000 }, //1.1 old previous setting verify, TODO remove
        { 0x15EE2500, 1100000, 978000 }, //1.1
        { 0x171D2300, 1050000, 933000 }, //1.05
        { 0x104C2200, 1000000, 889000 }, //1.0
        { 0x12AB1E00, 900000,  800000 }, //900
        { 0x150A1A00, 800000,  711000 }, //800
        { 0x17691600, 700000,  622000 }, //700
};


unsigned int rtk_get_boot_freq(int cpu)
{
	static int boot_freq_index = -1;
        int i;

	if(boot_freq_index == -1)
	{
                for(i=0;i<(sizeof(rtk_scpu_freq_map)/sizeof(tscpu_freq_map));i++)
                {
                        if((rtd_inl(SCPU_CORE_sys_pll_scpu2_reg) & 0xFFFFFFF0)==rtk_scpu_freq_map[i].reg_value)
                        {
				boot_freq_index = i;
                                break;
                        }
                }

		if(boot_freq_index == -1)
			panic("%s: can't find boot freq from table(%x)\n", __func__, rtd_inl(SCPU_CORE_sys_pll_scpu2_reg));
		else
		{
			rtd_pr_cpu_freq_warn("boot_freq:%d %x i=%d\n",rtk_scpu_freq_map[i].freq_kHz,(rtd_inl(SCPU_CORE_sys_pll_scpu2_reg) & 0xFFFFFFF0), boot_freq_index);
		}
	}

	if(rtk_check_control_pll_cpu(cpu))
	{
#ifdef CONFIG_RTK_CPUFREQ_FORCE_TO_USE_FREQ_TABLE
		if(rtk_scpu_freq_map[boot_freq_index].freq_kHz !=  rtk_cpufreq_get_pll(cpu))
			rtd_pr_cpu_freq_err("Warning: boot_freq(%d) not match, force to set to (%d)\n", rtk_scpu_freq_map[boot_freq_index].freq_kHz,  rtk_cpufreq_get_pll(cpu));
#endif
		return rtk_scpu_freq_map[boot_freq_index].freq_kHz;
	}
	else
		return rtk_scpu_freq_map[boot_freq_index].freq2_kHz;
}
#else
unsigned int rtk_get_boot_freq(int cluster_id)
{

	unsigned int pllscpu_divmode, pllscpu_prediv, n_code, f_code;
	unsigned int rate;

	if(cluster_id >= RTK_CPUFREQ_MAX_PLL)
		panic("%s: cluster_id mismatch(%d)\n",__func__, cluster_id);

	//Mac8p: 27*(Ncode+((pllscpu_divmode == 0) ? 4: 3) + Fcode/2048)/(pllscpu_prediv + 1);
	//CRT set pllscpu_divmode = 0, pllscpu_prediv = 0;
	pllscpu_divmode = (rtd_inl(SCPU_CORE_sys_scpu_reserve_reg) & SCPU_CORE_sys_scpu_reserve_reg_pllscpu_divmode_mask) >> SCPU_CORE_sys_scpu_reserve_reg_pllscpu_divmode_shift;
	//pllscpu_divmode = (rtd_inl(rtk_dvfs_reg[cluster_id].divider) & SCPU_CORE_sys_pll_scpu1_pllscpu_divmode_mask) >> SCPU_CORE_sys_pll_scpu1_pllscpu_divmode_shift;
	pllscpu_prediv = (rtd_inl(rtk_dvfs_reg[cluster_id].divider) & SCPU_CORE_sys_pll_scpu1_reg_pllscpu_prediv_mask) >> SCPU_CORE_sys_pll_scpu1_reg_pllscpu_prediv_shift;
	n_code = (rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq) & SCPU_CORE_sys_pll_scpu2_reg_pllscpu_ncode_mask) >> SCPU_CORE_sys_pll_scpu2_reg_pllscpu_ncode_shift;
	f_code = (rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq) & SCPU_CORE_sys_pll_scpu2_reg_pllscpu_fcode_mask) >> SCPU_CORE_sys_pll_scpu2_reg_pllscpu_fcode_shift;

	//if((pllscpu_divmode != 0) || (pllscpu_prediv != 0))
	rtd_pr_cpu_freq_info("%s: pllscpu_divmode(%d), pllscpu_prediv(%d)\n", __func__, pllscpu_divmode, pllscpu_prediv);

#if 0 //original calculate consider the parameter
	//rate = 27 *(n_code + 3 + ((pllscpu_divmode == 0) ? 1:0)+ (f_code/2048));
#else //simplify to fix parameter
	rate = (FREQ_XTAL_MHZ * 2048 * (n_code + ((pllscpu_divmode == 0) ? 4: 3)) + FREQ_XTAL_MHZ * f_code) >> (11 + pllscpu_prediv);
#endif

	rtd_pr_cpu_freq_err("boot_freq:(%d)/(%d)\n", cluster_id, rate);
	return rate;
}

#endif

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
unsigned int rtk_clk_rate = 0;
#endif
unsigned int rtk_clk_pll = 0;
void rtk_clk_set_pll(struct rtk_cpufreq_info *info, unsigned int rate, unsigned int rate2)
{
	unsigned int  n_code, f_code, cpu_freq_reg_val;
	unsigned int pllscpu_divmode, pllscpu_prediv;
	register unsigned int dvfs_reg_val;
	unsigned long flags;
	int cluster_id=0; //make default to zero

	pllscpu_divmode = (rtd_inl(SCPU_CORE_sys_scpu_reserve_reg) & SCPU_CORE_sys_scpu_reserve_reg_pllscpu_divmode_mask) >> SCPU_CORE_sys_scpu_reserve_reg_pllscpu_divmode_shift;
	pllscpu_prediv = (rtd_inl(rtk_dvfs_reg[cluster_id].divider) & SCPU_CORE_sys_pll_scpu1_reg_pllscpu_prediv_mask) >> SCPU_CORE_sys_pll_scpu1_reg_pllscpu_prediv_shift;

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(rtk_clk_rate < rate)	//frequency up
	{
		rtk_set_voltage(info, rate, rate2);
	}
#endif
	spin_lock_irqsave(&clock_lock, flags);

	dvfs_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].oc_control);

#if 0	//dump original freq
	cpu_freq_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq);
	n_code = (cpu_freq_reg_val >> N_CODE_START) & N_CODE_MAX;  //8 bits
	f_code = (cpu_freq_reg_val >> F_CODE_START) & F_CODE_MAX; //11 bits
	rtd_pr_cpu_freq_debug("*********** original freq=%d MHz\n", ( FREQ_XTAL_MHZ * ((n_code+FREQ_ADDITION)*1000+f_code*1000/2048))/1000);
#endif
	//mask off scpu freq bits
	cpu_freq_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq);
	cpu_freq_reg_val &=(~(N_CODE_MAX<<N_CODE_START | F_CODE_MAX << F_CODE_START));

	//Mac8p: 27*(Ncode+((pllscpu_divmode == 0) ? 4: 3) + Fcode/2048)/(pllscpu_prediv + 1);
	//CRT set pllscpu_divmode = 0, pllscpu_prediv = 0;
	//recalculate n/f code , rate,FREQ_XTAL is kHz
	n_code= ((rate <<pllscpu_prediv)/FREQ_XTAL) -((pllscpu_divmode == 0) ? 4: 3);
	f_code= (unsigned int )(((rate <<pllscpu_prediv) - (n_code +((pllscpu_divmode == 0) ? 4: 3))*FREQ_XTAL)*2048/FREQ_XTAL)+1;


	cpu_freq_reg_val|=(((n_code<<N_CODE_START)|f_code <<F_CODE_START)& (N_CODE_MAX<<N_CODE_START | F_CODE_MAX << F_CODE_START));
	if(cpu_freq_reg_val == rtk_clk_pll)
		goto DONE;

	//oc_en off
	rtd_outl(rtk_dvfs_reg[cluster_id].oc_control, dvfs_reg_val&(~(1<<SYS_OC_EN_BIT)));

	//adjust f/n code
	rtd_outl(rtk_dvfs_reg[cluster_id].cpu_freq, cpu_freq_reg_val);

	//oc_en on
	rtd_outl(rtk_dvfs_reg[cluster_id].oc_control, dvfs_reg_val|(1<<SYS_OC_EN_BIT));

	rtd_outl(rtk_dvfs_reg[cluster_id].oc_control, dvfs_reg_val|(1<<SYS_OC_EN_BIT));
	//firing oc action
	//	rtd_outl(SYS_PLL_DVFS_SCPU, dvfs_reg_val|(1<<SYS_OC_EN_BIT));
	//Waiting
	dvfs_reg_val=rtd_inl(rtk_dvfs_reg[cluster_id].oc_status);
	while((dvfs_reg_val & (1<<SYS_OC_DONE_BIT))==0)
	dvfs_reg_val=rtd_inl(rtk_dvfs_reg[cluster_id].oc_status);

	DONE:
	//dump result
	cpu_freq_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq);

#if 0 //dump after setting
	n_code = (cpu_freq_reg_val >> N_CODE_START) &0xFF;  //8 bits
	f_code = (cpu_freq_reg_val >> F_CODE_START) &0x7FF; //11 bits
	//rtd_pr_cpu_freq_debug("*********** original freq=%d MHz\n", ( FREQ_XTAL_MHZ * ((n_code+FREQ_ADDITION)*1000+f_code*1000/2048))/1000);
	//rtd_pr_cpu_freq_debug("**********rate:%lx set freq=%lu MHz\n",rate, ( FREQ_XTAL_MHZ * ((n_code+FREQ_ADDITION)*1000+f_code*1000/2048))/1000);
#endif

	rtk_clk_pll = cpu_freq_reg_val;
	spin_unlock_irqrestore(&clock_lock, flags);

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(rtk_clk_rate > rate)	//frequency down
	{
		rtk_set_voltage(info, rate, rate2);
	}
	rtk_clk_rate = rate;
#endif
	info->cur_pll = rate;

}

void rtk_clk_set_ratio(int cpu, unsigned int pll, unsigned int rate)
{
	int ratio, value;
	unsigned long flags;

	if(cpu !=0)
		panic("%s: cpu id mismatch(%d)\n",__func__, cpu);

	//rtd_pr_cpu_freq_err("rtk_clk_set_ratio:cpu(%d),pll(%d),rate(%d)\n", cpu, pll, rate);
	if(rate > pll)
		rtd_pr_cpu_freq_err("Error: rtk_clk_set_ratio set rate fail, rate (%d), pll (%d), policy(%d)\n", rate, pll, cpu);

	if(rate >= pll)
		ratio = 0;
	else
	{
		//rounding up
		ratio = (rate * 320)/pll;
		if((ratio % 10) != 0)
			ratio = ratio/10 +1;
		else
			ratio/=10;
		if(ratio > 32)
		{
			rtd_pr_cpu_freq_err("Error: rtk_clk_set_ratio set rate fail, rate (%d), pll (%d), policy(%d)\n", rate, pll, cpu);
			ratio = 32;
		}
		ratio = 32 - ratio;
	}

	//rtd_pr_cpu_freq_err("debug: rtk_clk_set_ratio set rate(%d), pll(%d), policy(%d), ratio(%d)\n", rate, pll, cpu, ratio);

	spin_lock_irqsave(&ratio_lock, flags);

	value = rtd_inl(SCPU_CORE_sys_scpu_clkratio_ctrl_0_reg);
	value &= (~SCPU_CORE_sys_scpu_clkratio_ctrl_0_clkratio_scpu_mask);
	value |= (ratio << SCPU_CORE_sys_scpu_clkratio_ctrl_0_clkratio_scpu_shift);
	rtd_outl(SCPU_CORE_sys_scpu_clkratio_ctrl_0_reg, value);
	rtd_outl(SCPU_CORE_sys_scpu_clkratio_ctrl_1_reg, 1<<SCPU_CORE_sys_scpu_clkratio_ctrl_1_clkratio_scpu_active_shift);
	while(rtd_inl(SCPU_CORE_sys_scpu_clkratio_ctrl_1_reg) !=0);

	spin_unlock_irqrestore(&ratio_lock, flags);

}

int __rtk_clk_set_rate_ex(struct clk *clk, unsigned int rate, int algo_id, int cpu, int idx)
{

	int ret = 0;
	int new_state, pre_state;
	unsigned long flags;
	struct rtk_cpufreq_info *info = rtk_cpufreq_info_lookup(cpu);
	if (!info)
	{
		rtd_pr_cpu_freq_err("rtk_cpufreq_info_lookup for cpu%d is not initialized.\n", cpu);
		return -EINVAL;
	}
	pre_state = info->policy->freq_table[info->freq_tbl_idx].driver_data;
	new_state = info->policy->freq_table[idx].driver_data;

#if 1 //TODO: need to check
	if (likely(clk->ops && clk->ops->set_rate))
	{
		spin_lock_irqsave(&clock_lock, flags);
		ret = clk->ops->set_rate(clk, rate, algo_id);
		spin_unlock_irqrestore(&clock_lock, flags);
	}

	if (unlikely(clk->flags & CLK_RATE_PROPAGATES))
		__rtk_propagate_rate(clk);

	clk->rate = rate;
#endif

	//rtd_pr_cpu_freq_err("(%d)debug new_state(%d), pre_state(%d)\n", cpu, new_state,  pre_state);
	if(new_state == pre_state) // P state switch
	{
		//set ratio
		rtk_clk_set_ratio(0, info->Ppll, rate);
	}
	else
	{
		int pll;

#ifdef CONFIG_RTK_SUPPORT_BOOST_PERFORMANCE_MODE
		if(new_state == RTK_CPUFREQ_STATE_X)
			pll = info->Xpll;
		else
#endif
		if(new_state == RTK_CPUFREQ_STATE_A)
			pll = info->Apll;
		else if(new_state == RTK_CPUFREQ_STATE_R)
			pll = info->Rpll;
		else
			pll = info->Ppll;

		//step1: set pll
		rtk_clk_set_pll(info, pll, rate);

		//step2: set ratio
		rtk_clk_set_ratio(0, pll, rate);
	}

	return ret;
}

#ifdef CONFIG_TV030_PLATFORM
static int __init early_parse_rt6203e_value (char *str)
{
	if(str)
		rt6203e_value = simple_strtoull(str, NULL, 16);
	rtd_pr_cpu_freq_err("rt6203e_value :0x%x\n",rt6203e_value);

    return 0 ;
}
early_param("rt6203e", early_parse_rt6203e_value);
#endif

#ifdef  CONFIG_PV88080_PMIC
static int __init early_parse_dvfs_high (char *str)
{
    if(str) {
	pv88080_power_value[0]= simple_strtoull(str, NULL, 16);
    }
    rtd_pr_cpu_freq_debug("pv88080_power_value[0]:%x\n",pv88080_power_value[0]);

    return 0 ;
}

static int __init early_parse_dvfs_low (char *str)
{
    if(str)
	pv88080_power_value[1]= simple_strtoull(str, NULL, 16);
    rtd_pr_cpu_freq_debug("pv88080_power_value[1]:%x\n",pv88080_power_value[1]);

    return 0 ;
}
early_param("dvfs_high", early_parse_dvfs_high);
early_param("dvfs_low", early_parse_dvfs_low);
#endif

//EXPORT_SYMBOL_GPL(__rtk_free_cpufreq_table);
