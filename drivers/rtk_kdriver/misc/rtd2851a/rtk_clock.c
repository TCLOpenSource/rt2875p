#include <linux/slab.h> //kzalloc
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/spinlock.h>
#include <mach/platform.h>
#include <linux/io.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <asm/uaccess.h>
//#include <asm/uaccess.h>
#define CPUFREQ_DRIVER_GET_FACTORY  //must in this
#include <rtk_kdriver/rtk_otp_region_api.h>
#include <rtk_kdriver/rtk_clock.h>
#include <rtk_kdriver/pcbMgr.h>
#include <rtk_gpio.h>
#include <asm/delay.h>
#include <io.h>

static LIST_HEAD(clock_list);
static DEFINE_SPINLOCK(clock_lock);
static DEFINE_MUTEX(clock_list_sem);
static unsigned int boot_freq=900000;
#define SYS_PLL_SCPU (0xb8000404)
#define SYS_PLL_DVFS_SCPU (0xb8000440)
#define SYS_DYN_SW_CPU_SCPU_MASK 0xFFFFFFF0
#define SYS_DYN_SW_CPU 0xb8000220
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
	unsigned int cpu_freq;
	unsigned int oc_control;
	unsigned int oc_status;
}tRtkDvfs;

tRtkDvfs  rtk_dvfs_reg[2]=
{
	/*sys_pll2_scpu2, sys_scpu2_dvfs*/
	/*{0xB805B024,0xB805B028,0xB805B02C},*/
	/*sys_pll_scpu2, sys_scpu_dvfs*/
	{0xB805B004,0xB805B008,0xB805B00C},
	{0xB805B004,0xB805B008,0xB805B00C}
};


#define FREQ_LEVEL 12
#define FREQ_STEP 100000   //khz
#define FREQ_XTAL 27000    //khz

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

static struct clk rtk_cpuclk = {
	.name = "cpu_clk",
	.flags = CLK_ALWAYS_ENABLED | CLK_RATE_PROPAGATES,
	.rate = 800000, //kHz
};

struct clk *__rtk_clk_get(struct device *dev, const char *id)
{
	return &rtk_cpuclk;
}
EXPORT_SYMBOL(__rtk_clk_get);

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
	SCPU_H_H,	// 3rd level
	SCPU_H_L, // 2nd level
	SCPU_L_H,
	SCPU_L_L,
};

struct rtk_level_freq
{
	enum VOLTAGE_LEVEL gpio_level;
	unsigned int freq;
};

#ifdef CONFIG_CUSTOMER_TV006
#define FREQ_TABLE_SIZE 4
#else
#define FREQ_TABLE_SIZE 5
#endif
struct rtk_voltage_desc 
{
	unsigned char scpu_vid1_level;
	unsigned char scpu_vid0_level;
	struct rtk_level_freq level[FREQ_TABLE_SIZE];
};

struct rtk_voltage_desc voltage_level=
{
	SCPU_VID1,
	SCPU_VID0,
	{ {SCPU_L_L,SCPU_FREQ_STEP0},
	  {SCPU_L_L,SCPU_FREQ_STEP1},
	  {SCPU_L_L,SCPU_FREQ_STEP2},
#ifdef CONFIG_CUSTOMER_TV006
	  {SCPU_H_H,SCPU_FREQ_STEP3},
	  {SCPU_H_H,SCPU_FREQ_STEP4}},
#else
	  {SCPU_L_L,SCPU_FREQ_STEP3},
	  {SCPU_H_H,SCPU_FREQ_STEP4}}, //reserved 
#endif
};

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
#else

#endif

void rtk_set_io_direction(unsigned char *pin_info,int out_value)
{
	unsigned long long pin;
	if (pcb_mgr_get_enum_info_byname(pin_info, &pin) == 0)
	{
//		rtd_pr_cpu_freq_info("IO set :%s value:%d\n",pin_info,out_value);
		rtk_SetIOPinDirection(pin,1);
		rtk_SetIOPin (pin,out_value) ;  
	}

}
void rtk_set_voltage(int freq)
{
	int i=0,delay_flag=0;
	static int old_freq=0;
	static int gpu_dvfs_flag=0;

	if(is_platform_dvs_enable()==0)
		return;

	for(i=0;i<FREQ_TABLE_SIZE;i++)
	{
		if(voltage_level.level[i].freq == freq)
			break;
	}
    
	if(i >= FREQ_TABLE_SIZE){
		return ;
	}

	rtd_pr_cpu_freq_debug("clamp freq:%d i=%d voltage_level.level[i].gpio_level:%d\n",freq,i,voltage_level.level[i].gpio_level);	
	delay_flag=1;

	if(freq == -1 || freq == -2)
		delay_flag=1;

		
	if((i<FREQ_TABLE_SIZE) && (freq!=-1 && freq!=-2))
	{
		if(freq > old_freq || old_freq==0)
			delay_flag=1;	
	
		old_freq=freq;
		switch(voltage_level.level[i].gpio_level)
		{
#ifdef CONFIG_PV88080_PMIC

			case SCPU_H_H:
				pv88080_adjust_scpu_voltage(SCPU_H_H);		
				break;
			default:
				pv88080_adjust_scpu_voltage(SCPU_H_L);
				break;
#else
			case SCPU_L_L:
				if(gpu_dvfs_flag==1) {
					gpu_dvfs_flag=0;
					if(dvfs_func[0].func!=NULL) {
						dvfs_func[0].func(1,600);
				//		rtd_pr_cpu_freq_err("gpu low\n");
					}
				}
				rtk_set_io_direction("PIN_SCPU_VID_0",0);
				rtk_set_io_direction("PIN_SCPU_VID_1",0);
				break;
			case SCPU_L_H:
				rtk_set_io_direction("PIN_SCPU_VID_0",1);
				rtk_set_io_direction("PIN_SCPU_VID_1",0);
				break;
			case SCPU_H_L:
				rtk_set_io_direction("PIN_SCPU_VID_1",1);
				rtk_set_io_direction("PIN_SCPU_VID_0",0);
				break;
			case SCPU_H_H:
				rtk_set_io_direction("PIN_SCPU_VID_1",1);
				rtk_set_io_direction("PIN_SCPU_VID_0",1);
				break;
#endif
		}
	}
	else if(freq==-1)//set to max
	{
#ifdef CONFIG_PV88080_PMIC
		pv88080_adjust_scpu_voltage(SCPU_H_H);		
#else
		rtk_set_io_direction("PIN_SCPU_VID_1",1);
		rtk_set_io_direction("PIN_SCPU_VID_0",1);
#endif
	}

	else if(freq==-2)//set to max
	{
#ifdef CONFIG_PV88080_PMIC
		pv88080_adjust_scpu_voltage(SCPU_H_H);		
#else
		rtk_set_io_direction("PIN_SCPU_VID_1",1);
		rtk_set_io_direction("PIN_SCPU_VID_0",1);
#endif
	}
	if(delay_flag)
		udelay(50);
	if(gpu_dvfs_flag==0 && (voltage_level.level[i].gpio_level==SCPU_H_H)) {
		gpu_dvfs_flag=1;
		if(dvfs_func[0].func!=NULL){
			dvfs_func[0].func(0,800);
//			rtd_pr_cpu_freq_err("gpu high\n");
		}
	}
}
#endif

int rtk_get_overfreq_otp(void)
{
	static int otp = -1;
	if(otp != -1) //already read
	{
		return otp;
	}
	else //first time, read from otp
	{
#if IS_ENABLED(CONFIG_RTK_KDRV_OTP)
		if(rtk_otp_field_read(OTP_FIELD_OVER_FREQ, (unsigned char *)&otp, 4) != 1)
			panic("Read otp OTP_FIELD_OVER_FREQ fail\n");
		else
			return otp;
#else
		return otp;
#endif
	}
}

int __rtk_clk_set_rate(struct clk *clk, unsigned long rate)
{
	
	return __rtk_clk_set_rate_ex(clk, rate, 0);
}
EXPORT_SYMBOL_GPL(__rtk_clk_set_rate);

typedef struct scpu_freq_map
{
        unsigned int reg_value;
        unsigned int freq_kHz;
}tscpu_freq_map;

tscpu_freq_map rtk_scpu_freq_map[]= {
        { 0x07B43B00, 1700000 },//1.7
        { 0x02133800, 1600000 }, //1.6
        { 0x04723400, 1500000 }, //1.5
        { 0x06D13000, 1400000 }, //1.4
        { 0x012F2D00, 1300000 }, //1.3
        { 0x038E2900, 1200000 }, //1.2
        { 0x05ED2500, 1100000 }, //1.1
        { 0x071C2300, 1050000 }, //1.05
        { 0x004C2200, 1000000 }, //1.0
        { 0x02AB1E00, 900000 }, //900
        { 0x05091A00, 800000 }, //800
        { 0x07681600, 700000 }, //700
};

unsigned int rtk_get_boot_freq(void)
{
        static int get_boot_freq=0;
        int i;
        if(get_boot_freq==0) {
                for(i=0;i<(sizeof(rtk_scpu_freq_map)/sizeof(tscpu_freq_map));i++)
                {
                        if((rtd_inl(0xB805B004) & 0xFFFFFFF0)==rtk_scpu_freq_map[i].reg_value)
                        {
                                boot_freq=rtk_scpu_freq_map[i].freq_kHz;
                                break;
                        }
                }
                rtd_pr_cpu_freq_info("boot_freq:%d %x i=%d\n",boot_freq,(rtd_inl(0xB805B004) & 0xFFFFFFF0),i);
                get_boot_freq=1;
        }
        return boot_freq;
}

int __rtk_clk_set_rate_ex(struct clk *clk, unsigned long rate, int algo_id)
{
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	static int old_rate=0;
#endif
	int ret = 0;
	unsigned int  n_code, f_code, cpu_freq_reg_val;
        	register unsigned int dvfs_reg_val;
	unsigned long flags;
	int real_rate;
//	int digital_adjust_freq=0;
	int cluster_id=0; //make default to zero

	real_rate=rate;
#ifdef CONFIG_FIXED_CLOCK_TEST
	//fixed the rate into 150MHz for test dhrystone time to know clock actually change or not
	real_rate=150000;
	rate=300000;
//	real_rate=900000;
#else
//	if(real_rate<300000)
//	        rate=300000;
#endif
	

	if (likely(clk->ops && clk->ops->set_rate))
	{
		spin_lock_irqsave(&clock_lock, flags);
		ret = clk->ops->set_rate(clk, rate, algo_id);
		spin_unlock_irqrestore(&clock_lock, flags);
	}

	if (unlikely(clk->flags & CLK_RATE_PROPAGATES))
		__rtk_propagate_rate(clk);

	clk->rate = rate;

	rtk_get_boot_freq();
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(old_rate < rate)	//frequency up
	{
		rtk_set_voltage(rate);
	}
#endif
	spin_lock_irqsave(&clock_lock, flags);

	dvfs_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].oc_control);
	cpu_freq_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq);
	n_code = (cpu_freq_reg_val >> N_CODE_START) & N_CODE_MAX;  //8 bits
	f_code = (cpu_freq_reg_val >> F_CODE_START) & F_CODE_MAX; //11 bits
	rtd_pr_cpu_freq_debug("*********** original freq=%d MHz\n", ( FREQ_XTAL_MHZ * ((n_code+FREQ_ADDITION)*1000+f_code*1000/2048))/1000);

	//mask off scpu freq bits
	cpu_freq_reg_val &=(~(N_CODE_MAX<<N_CODE_START | F_CODE_MAX << F_CODE_START));
	//recalculate n/f code , rate,FREQ_XTAL is kHz
	n_code= (rate/FREQ_XTAL) - FREQ_ADDITION;
	f_code= (unsigned int )((rate - (n_code+3)*FREQ_XTAL)*2048/FREQ_XTAL)+1;

	//write scpu relate bits
	cpu_freq_reg_val|=(((n_code<<N_CODE_START)|f_code <<F_CODE_START)& (N_CODE_MAX<<N_CODE_START | F_CODE_MAX << F_CODE_START));
	rtd_outl(rtk_dvfs_reg[cluster_id].oc_control, dvfs_reg_val&(~(1<<SYS_OC_EN_BIT)));
	dvfs_reg_val=rtd_inl(rtk_dvfs_reg[cluster_id].oc_control);
	rtd_outl(rtk_dvfs_reg[cluster_id].cpu_freq, cpu_freq_reg_val);
	//adjust digital freq


	rtd_outl(rtk_dvfs_reg[cluster_id].oc_control, dvfs_reg_val|(1<<SYS_OC_EN_BIT));
	//firing oc action
	//	rtd_outl(SYS_PLL_DVFS_SCPU, dvfs_reg_val|(1<<SYS_OC_EN_BIT));
	//Waiting
	dvfs_reg_val=rtd_inl(rtk_dvfs_reg[cluster_id].oc_status);
	while((dvfs_reg_val & (1<<SYS_OC_DONE_BIT))==0)
	dvfs_reg_val=rtd_inl(rtk_dvfs_reg[cluster_id].oc_status);
	//dump result
	cpu_freq_reg_val = rtd_inl(rtk_dvfs_reg[cluster_id].cpu_freq);
	n_code = (cpu_freq_reg_val >> N_CODE_START) &0xFF;  //8 bits
	f_code = (cpu_freq_reg_val >> F_CODE_START) &0x7FF; //11 bits
	//rtd_pr_cpu_freq_debug("*********** original freq=%d MHz\n", ( FREQ_XTAL_MHZ * ((n_code+FREQ_ADDITION)*1000+f_code*1000/2048))/1000);
	//rtd_pr_cpu_freq_debug("**********rate:%lx set freq=%lu MHz\n",rate, ( FREQ_XTAL_MHZ * ((n_code+FREQ_ADDITION)*1000+f_code*1000/2048))/1000);
	spin_unlock_irqrestore(&clock_lock, flags);
#ifdef CONFIG_REALTEK_VOLTAGE_CTRL
	if(old_rate > rate)	//frequency down
	{
		rtk_set_voltage(rate);
	}
	old_rate=rate;
#endif


	return ret;
}
EXPORT_SYMBOL_GPL(__rtk_clk_set_rate_ex);

long __rtk_clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (likely(clk->ops && clk->ops->round_rate))
	{
		unsigned long flags, rounded;

		spin_lock_irqsave(&clock_lock, flags);
		rounded = clk->ops->round_rate(clk, rate);
		spin_unlock_irqrestore(&clock_lock, flags);

		return rounded;
	}

	return rate;
}
EXPORT_SYMBOL_GPL(__rtk_clk_round_rate);

static int freqTbl4[]={
	SCPU_FREQ_STEP0,	
	SCPU_FREQ_STEP1,	
	SCPU_FREQ_STEP2,	
	SCPU_FREQ_STEP3,
	SCPU_FREQ_STEP4,
};

/*static int freqTbl2[]={
	SCPU_FREQ_STEP0,	
	SCPU_FREQ_STEP3,
	SCPU_FREQ_STEP4,
};*/

#if 0
static unsigned int rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
    unsigned int X,A,result;
    X=(1<<(endBit-startBit+1))-1;
    A=rtd_inl(reg_addr);
    result = (A & (~(X<<startBit))) | (value<<startBit);
    rtd_outl(reg_addr,result);
    return 0;
}
#endif

static int rtk_early_dfs[6]={0,0,0,0,0,-1};

extern void * __init get_platform_cpu_resource(void);
int __rtk_init_cpufreq_table(struct cpufreq_frequency_table **table)
{
	//[21:20]=N, [19:12]=M, [8:7]=O
	unsigned int max_freq, next_freq;
	unsigned int i;
	unsigned int num_freq;
	struct cpufreq_frequency_table *freq_table;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
#ifdef CONFIG_OF
	struct device_node *np = NULL;
	int value[5];

	np = of_find_compatible_node(NULL,NULL,"arm,rtk-cpufreq");
	if (!np)
		rtd_pr_cpu_freq_err("%s: failed to find cpufreq table on DT node\n",__func__);
	if(!of_property_read_u32_array(np, "freq", value,5))
	{
		for(i=0;i<5;i++)
			freqTbl4[i] = value[i]*1000;
	}
#endif
#else
	CPU_CONTENT_T *pCpufreq=NULL;
	pCpufreq=(CPU_CONTENT_T *)get_platform_cpu_resource();

	//Here config from resource table
	if(pCpufreq)
	{
		int i=0;
		for(i=0;i<5;i++)
		{
			if(pCpufreq[i].content[0]==1)
			{
				freqTbl4[0]=pCpufreq[i].content[1]*1000;
				freqTbl4[1]=pCpufreq[i].content[2]*1000;
				freqTbl4[2]=pCpufreq[i].content[3]*1000;
				freqTbl4[3]=pCpufreq[i].content[4]*1000;
				freqTbl4[4]=pCpufreq[i].content[5]*1000;
				rtd_pr_cpu_freq_info("Get CPU freq from resouce table\n");
				break;
			}
		}
	}
#endif

#ifdef CONFIG_RTK_SUPPORT_CPUFREQ_OTP
//#ifndef CONFIG_TV001_PLATFORM
	{
		int otp = rtk_get_overfreq_otp() & 0x7;
		int org = freqTbl4[3];
		rtk_get_boot_freq(); //make sure get boot_freq first, then start to set boot_freq
		if((otp == 5) || (otp == 7))	//GRADE_A
			freqTbl4[3] = 1300000;
		else
			freqTbl4[3] = 1200000;
		rtd_pr_cpu_freq_info("Set CPU freq table(%d)(org:%d), boot_freq(%d), otp(%d)\n", freqTbl4[3], org, boot_freq, otp);
		boot_freq = freqTbl4[3];
	}
//#endif
#endif

#ifdef CONFIG_REALTEK_VOLTAGE_CTRL  
#ifdef CONFIG_PV88080_PMIC
	//Voltage confing defined, but no pv88080 pmic
	if(pv88080_adjust_scpu_voltage(SCPU_H_H)==-1) {
		pv88080_exist=0;
		freqTbl4[4]=SCPU_FREQ_STEP3; //there is no pv88080, override freq
	}
#endif
#endif

#if 0
	if(boot_freqnum==2){
		freq_table = kzalloc(sizeof(struct cpufreq_frequency_table) * (sizeof(freqTbl2)/4+1), GFP_KERNEL);
		max_freq = freqTbl2[sizeof(freqTbl2)/4 -1 ];
		num_freq=sizeof(freqTbl2)/4;
	}
	else
#endif
	{
		int k=0;
		freq_table = kzalloc(sizeof(struct cpufreq_frequency_table) * (sizeof(freqTbl4)/4+1), GFP_KERNEL);
		max_freq = freqTbl4[sizeof(freqTbl4)/4 -1 ];
		num_freq=sizeof(freqTbl4)/4;
                for(k=0;k<sizeof(freqTbl4)/4;k++)
                        if(rtk_early_dfs[k]!=0)
                                freqTbl4[k]=rtk_early_dfs[k]*1000;  //MHz to KHz
	}
	if (!freq_table) {
		rtd_pr_cpu_freq_err("fail to alloc freq table\n");
		return -ENOMEM;
	}

	next_freq = max_freq;
	for (i = num_freq ; i > 0; i--)
	{
		//freq_table[i-1].index = i;
		freq_table[i-1].frequency = next_freq;
#ifndef CONFIG_RTK_SUPPORT_BOOST_MODE
		rtd_pr_cpu_freq_info("table[%d].freq=%dkHz\n", i-1, freq_table[i-1].frequency);
#else
		if(rtk_get_boot_freq() < next_freq)
			freq_table[i-1].flags = CPUFREQ_BOOST_FREQ;
		rtd_pr_cpu_freq_info("table[%d].freq=%dkHz, boost=%d\n", i-1, freq_table[i-1].frequency, (freq_table[i-1].flags & CPUFREQ_BOOST_FREQ));
#endif
		if(i>=2) 
			next_freq = freqTbl4[i - 2];
	}

	freq_table[num_freq].frequency = CPUFREQ_TABLE_END;

	*table = &freq_table[0];

	return 0;
}
//EXPORT_SYMBOL_GPL(__rtk_init_cpufreq_table);

void __rtk_free_cpufreq_table(struct cpufreq_frequency_table **table)
{
	if (!table)
		return;

	kfree(*table);
	*table = NULL;
}

void __rtk_freqtable_switch(struct cpufreq_frequency_table **table)
{
	__rtk_free_cpufreq_table(table);

}

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

static int __init early_parse_rtk_dfs (char *str)
{
    if(str) {
        sscanf(str,"%d_%d_%d_%d_%d",&rtk_early_dfs[0],&rtk_early_dfs[1],&rtk_early_dfs[2],&rtk_early_dfs[3],&rtk_early_dfs[4]);
       rtd_pr_cpu_freq_info("%s %d %d %d %d %d\n",str,rtk_early_dfs[0],rtk_early_dfs[1],rtk_early_dfs[2],rtk_early_dfs[3],rtk_early_dfs[4]);
    }

    return 0 ;
}
early_param("rtk_dfs", early_parse_rtk_dfs);


/*[MA7PBU-162]*/
#if 1
#define CPUFREQ_LEVEL_0_UP_DEGREE 110
#define CPUFREQ_LEVEL_1_DOWN_DEGREE 105
#define CPUFREQ_LEVEL_1_UP_DEGREE 120
#define CPUFREQ_LEVEL_2_DOWN_DEGREE 115

int init_cpu_freq(char *name, char *path)  //setup sysfs node value
{
	struct file *filp = NULL;

	char buf[20]="";

	mm_segment_t old_fs;

	filp = filp_open(path, O_RDWR | O_CREAT, 0);

	if (IS_ERR(filp)) {
		rtd_pr_cpu_freq_info("open failed %s %s...(%ld)\n", path,__func__, PTR_ERR(filp));
		return -1;
	}
	if(filp == NULL)
		return -1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	sprintf(buf, "%s", name);
	if (!IS_ERR(filp)) {
		filp->f_op->llseek(filp,0,0);
		filp->f_op->write(filp, (char *)buf, strlen(buf), &filp->f_pos);
	}
	set_fs(old_fs);
	if(filp != NULL)
		filp_close(filp, NULL);
	return 0;
}

void cpu_thermal_callback(void *data, int cur_degree , int reg_degree, char *module_name)
{
	static int current_level  = 0;
	int previous_level = current_level;
	if(current_level == 0)
	{
		if(cur_degree > CPUFREQ_LEVEL_0_UP_DEGREE)
		{
			current_level = 1;
		}
	}
	else if(current_level == 1)
	{
		if(cur_degree > CPUFREQ_LEVEL_1_UP_DEGREE)
		{
			current_level = 2;
		}
		else if(cur_degree < CPUFREQ_LEVEL_1_DOWN_DEGREE)
		{
			current_level = 0;
		}
	}
	else if(current_level == 2)
	{
		if(cur_degree < CPUFREQ_LEVEL_2_DOWN_DEGREE)
		{
			current_level = 1;
		}
	}

	if(current_level != previous_level)
	{
		if(current_level == 2)
		{
			if(init_cpu_freq("powersave","/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor")!=0)
				rtd_pr_cpu_freq_err("Set CPUFreq powersave fail %s : %d\n", __func__, __LINE__);
		}
		else if(current_level == 1)
		{
			if(init_cpu_freq("userspace","/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor")!=0)
				rtd_pr_cpu_freq_err("Set CPUFreq userspace fail %s : %d\n", __func__, __LINE__);
			if(boot_freq > 1100000)
			{
				if(init_cpu_freq("1100000","/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed")!=0)
					rtd_pr_cpu_freq_err("Set CPUFreq scaling_setspeed fail %s : %d\n", __func__, __LINE__);
			}
			else
			{
				if(init_cpu_freq("1000000","/sys/devices/system/cpu/cpu0/cpufreq/scaling_setspeed")!=0)
					rtd_pr_cpu_freq_err("Set CPUFreq scaling_setspeed fail %s : %d\n", __func__, __LINE__);
			}
		}
		else if(current_level == 0)
		{
			if(init_cpu_freq("performance","/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor")!=0)
				rtd_pr_cpu_freq_err("Set CPUFreq performance fail %s : %d\n", __func__, __LINE__);
		}
	}
}

#ifdef CONFIG_RTK_KDRV_THERMAL_SENSOR //
extern int register_temperature_callback(int degree, void *fn, void* data, char *module_name);
static int register_cpu_thermal_handler(void)
{
        int ret;
	int temp;

        rtd_pr_cpu_freq_info("%s\n",__func__);
        if ((ret = register_temperature_callback(CPUFREQ_LEVEL_1_DOWN_DEGREE -5, cpu_thermal_callback, &temp, "cpufreq")) < 0)
                rtd_pr_cpu_freq_info("register cpufreq thermal handler fail, ret:%d \n", ret);

        return 0;
}
#endif

late_initcall(register_cpu_thermal_handler);
#endif

//EXPORT_SYMBOL_GPL(__rtk_free_cpufreq_table);
