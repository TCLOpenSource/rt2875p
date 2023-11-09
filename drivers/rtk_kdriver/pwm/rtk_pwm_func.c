#ifndef BUILD_QUICK_SHOW
#include <linux/pwm.h>
#include <linux/sched.h>
#include <linux/math64.h>
#include <asm/div64.h>
#include <linux/platform_device.h>
#include <mach/pcbMgr.h>
#include <tvscalercontrol/panel/panelapi.h>
#else
#include <no_os/export.h>
#include <errno.h>
#include <div64.h>
#include <vsprintf.h>
#include <string.h>
#endif
#include <rbus/pll27x_reg_reg.h>
#include <rbus/ppoverlay_reg.h>
#include <rbus/iso_misc_reg.h>
#include "rtk_pwm-reg.h"
#include "rtk_pwm_crt.h"
#include "rtk_pwm_func.h"
#include <rtk_kdriver/rtk_pwm.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/rtk_gpio-db.h>
#include <rtk_kdriver/pcbMgr.h>
#include <rtk_kdriver/quick_show/quick_show.h>

#define NEW_PWM_FORMULA 1

#ifndef BUILD_QUICK_SHOW
#define LOCK_RTK_PWM(a,b)    spin_lock_irqsave(a, b)
#define UNLOCK_RTK_PWM(a, b) spin_unlock_irqrestore(a, b)

static DEFINE_SPINLOCK(pwm_lock);
extern struct mutex rtk_pwm_mutex;
#else
#define LOCK_RTK_PWM(a,b)
#define UNLOCK_RTK_PWM(a, b)

#define synchronize_irq(x)
#define likely(x)           (x)
#define DIV_ROUND_CLOSEST(x, divisor) (\
{ \
    typeof(divisor)__divisor = divisor; \
    (((x)+ ((__divisor) / 2)) / (__divisor)); \
} \
)
#endif

extern unsigned char pwm_resume_done;
static int rtk_pwm_panel_mode=0;
static int rtk_pwm_backlight_pin_index = -1;
static int rtk_pwm_index = 0;
static R_CHIP_T rtk_pwm_chip[MAX_PWM_NODE];
static int m_ioctl_printk=0;
static int m_ioctl_disable=0;
static unsigned int m_ioctl_counter = 0;
static unsigned long iso_pwm_usage = 0;
static unsigned long misc_pwm_usage = 0;


//======================================================================
// Display Scaler call back function
//======================================================================
typedef unsigned int (*Get_DISP_HTOTAL)(void);
Get_DISP_HTOTAL p_get_disp_htotal = NULL;
typedef unsigned int (*Get_DISP_VTOTAL)(void);
Get_DISP_VTOTAL p_get_disp_vtotal = NULL;
typedef unsigned int (*Get_DISP_DCLK)(void);
Get_DISP_DCLK p_get_disp_dclk = NULL;
typedef unsigned int (*Get_DISP_REFRESH_RATE)(void);
Get_DISP_REFRESH_RATE p_get_disp_refresh_rate = NULL;

// Htotal
unsigned int pwm_get_disp_htotal(void)
{
    if(p_get_disp_htotal) {
        return p_get_disp_htotal();
    }
    return 2200;
}

void register_get_disp_htotal_ptr(void* fun_ptr)
{
    p_get_disp_htotal = (Get_DISP_HTOTAL)fun_ptr;
}
EXPORT_SYMBOL(register_get_disp_htotal_ptr);

// Vtotal
unsigned int pwm_get_disp_vtotal(void)
{
    if(p_get_disp_vtotal) {
        return p_get_disp_vtotal();
    }
    return 1100;
}

void register_get_disp_vtotal_ptr(void* fun_ptr)
{
    p_get_disp_vtotal = (Get_DISP_VTOTAL)fun_ptr;
}
EXPORT_SYMBOL(register_get_disp_vtotal_ptr);

// DCLK
unsigned int pwm_get_disp_dclk(void)
{
    if(p_get_disp_dclk) {
        return p_get_disp_dclk();
    }
    return 594*1000000;
}

void register_get_disp_dclk_ptr(void* fun_ptr)
{
    p_get_disp_dclk = (Get_DISP_DCLK)fun_ptr;
}
EXPORT_SYMBOL(register_get_disp_dclk_ptr);

// refresh rate
unsigned int pwm_get_disp_refresh_rate(void)
{
    if(p_get_disp_refresh_rate) {
        return p_get_disp_refresh_rate();
    }
    return 0;
}

void register_get_disp_refresh_rate_ptr(void* fun_ptr)
{
    p_get_disp_refresh_rate = (Get_DISP_REFRESH_RATE)fun_ptr;
}
EXPORT_SYMBOL(register_get_disp_refresh_rate_ptr);

/*************************************************************************
*** workaround for PWM support full HIGH and full low: change pwm to gpio.
*************************************************************************/
extern void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value);

int rtk_pwm_gpio_value(R_CHIP_T *pchip2)
{    return rtk_gpio_output_get(pchip2->gid);    }

void rtk_change_to_gpio_mode(R_CHIP_T *pchip2, GPIOPOT outputValue)
{
    if(INVALID_GPIO_GID == pchip2->gid || 0 == pchip2->pin_mux_info.addr){
        PWM_ERR("Probably pwm init get pinmux info error by pcbenum!\n");
        PWM_ERR("Change to gpio failed: addr:%ld pwmvalue:%d;Gid:%d;L:%d\n",
                pchip2->pin_mux_info.addr,
                pchip2->pin_mux_info.pwmValue,
                pchip2->gid, __LINE__);
        return;
    }

    pchip2->currentMode = GPIO_MODE;
    rtd_part_outl(pchip2->pin_mux_info.addr,
                  pchip2->pin_mux_info.mask_h,
                  pchip2->pin_mux_info.mask_l,
                  pchip2->pin_mux_info.gpioValue);

    rtk_gpio_output(pchip2->gid, outputValue);
    rtk_gpio_set_dir(pchip2->gid, 1);
}

void rtk_change_to_pwm_mode(R_CHIP_T *pchip2)
{
    if(0 == pchip2->pin_mux_info.addr){
        PWM_ERR("Probably pwm init get pinmux info error by pcbenum!\n");
        PWM_ERR("Change to pwm failed: addr:%ld pwmvalue:%d;Gid:%d;L:%d\n",
                pchip2->pin_mux_info.addr,
                pchip2->pin_mux_info.pwmValue,
                pchip2->gid, __LINE__);
        return;
    }

    pchip2->currentMode = PWM_MODE;
    rtd_part_outl(pchip2->pin_mux_info.addr,
                  pchip2->pin_mux_info.mask_h,
                  pchip2->pin_mux_info.mask_l,
                  pchip2->pin_mux_info.pwmValue);
}

int rtk_pwm_gpio_pinmux_init(R_CHIP_T *pwmPin)
{
    int ret = 0;
    char     pcbNameWithPinmux[128];
    unsigned long long ullPcbPinMuxValue = 0;

    (void)snprintf(pcbNameWithPinmux, sizeof(pcbNameWithPinmux), "%s%s", pwmPin->pcbname, "_GPIO_INFO");
    ret = pcb_mgr_get_enum_info_byname(pcbNameWithPinmux, &ullPcbPinMuxValue);
    if(0 != ret){
        PWM_ERR("Get pcbenum error.enum name:%s\n", pcbNameWithPinmux);
        pwmPin->gid = INVALID_GPIO_GID;
    }
    else{
        pwmPin->pin_mux_info.addr      = GET_PWM_PINMUX_ADDR(ullPcbPinMuxValue);
        pwmPin->pin_mux_info.mask_h    = GET_PWM_PINMUX_REG_ENDBIT(ullPcbPinMuxValue);
        pwmPin->pin_mux_info.mask_l    = GET_PWM_PINMUX_REG_STARTBIT(ullPcbPinMuxValue);
        pwmPin->pin_mux_info.pwmValue  = GET_PWM_PINMUX_PWMVALUE(ullPcbPinMuxValue);
        pwmPin->pin_mux_info.gpioValue = GET_PWM_PINMUX_GPIOVALUE(ullPcbPinMuxValue);
        pwmPin->gid = rtk_gpio_get_gid_by_pintype(GET_PWM_PINMUX_GPIO_PINTYPE(ullPcbPinMuxValue),
                                                  GET_PWM_PINMUX_GPIO_NUM(ullPcbPinMuxValue));
    }
    pwmPin->currentMode = PWM_MODE;
    return 0;
}

//======================================================================
//Backlight EXPORT Function
//======================================================================
int rtk_pwm_backlight_enable(int enable)
{
    int gpioLevel = 0;
    unsigned long long param = 0 ;

    if( enable < 0 )
    {
        gpioLevel = 0;
        PWM_WARN("%s enable = %d , too strange \n", __func__, enable );
    }
    else if( enable > 1 )
    {
        gpioLevel = 1;
    }
    else
    {
        gpioLevel = enable ;
    }

    if (pcb_mgr_get_enum_info_byname("PIN_BL_ON_OFF", &param) != 0)
    {
        PWM_WARN("%s PIN_BL_ON_OFF is not existed , please check pcb parameters = %llx \n", __func__, param);
        return (-1);
    }

    PWM_WARN("%s PIN_BL_ON_OFF gpio_type = %d gpio_num=%d\n", __func__, GET_PIN_TYPE(param) , GET_PIN_INDEX(param) );

    if ( rtk_SetIOPin(param, gpioLevel) != 0 )
    {
        PWM_WARN("%s output operation fail!\n", __func__ );
        return (-1);
    }

    if ( rtk_SetIOPinDirection(param, 1) != 0 )
    {
        PWM_WARN("%s direction operation fail!\n", __func__ );
        return (-1);
    }

    return 0;
}
EXPORT_SYMBOL(rtk_pwm_backlight_enable);

int rtk_pwm_backlight_status(void)
{
    unsigned long long param = 0 ;
    RTK_GPIO_ID BL_pin;

    if (pcb_mgr_get_enum_info_byname("PIN_BL_ON_OFF", &param) != 0) {
        PWM_WARN("%s PIN_BL_ON_OFF is not existed , please check pcb parameters = %llx \n", __func__, param);
        return (-1);
    }

    if (GET_PIN_TYPE(param) == PCB_PIN_TYPE_ISO_GPIO) {
        BL_pin = rtk_gpio_id(ISO_GPIO, GET_PIN_INDEX(param));
        PWM_WARN("%s PIN_BL_ON_OFF PIN_type = %d gpio_type = ISO_GPIO gpio_num=%d\n", __func__, GET_PIN_TYPE(param) , GET_PIN_INDEX(param) );
    }
    else {
        BL_pin = rtk_gpio_id(MIS_GPIO, GET_PIN_INDEX(param));
        PWM_WARN("%s PIN_BL_ON_OFF PIN_type = %d gpio_type = MIS_GPIO gpio_num=%d\n", __func__, GET_PIN_TYPE(param) , GET_PIN_INDEX(param) );
    }
    return rtk_gpio_output_get(BL_pin);
}
EXPORT_SYMBOL(rtk_pwm_backlight_status);

int rtk_pwm_backlight_set_duty( int duty )
{
    R_CHIP_T *pchip2 = NULL;

    rtd_pr_pwm_notice("%s():\n", __FUNCTION__);

    if( duty > 255 || duty < 0) {
        PWM_ERR("backlight duty %d error!\n",duty );
        return -1;
    }

    if(pwm_resume_done != 1) {
        PWM_ERR("pwm resume not ready\n");
        return -1;
    }

    /*get pin index of backlight pwm*/
    if( rtk_pwm_backlight_pin_get() != (-1) ){
        pchip2 = &rtk_pwm_chip[rtk_pwm_backlight_pin_get()];
    }else{
        PWM_WARN("backlight pwm source is missed\n");
    }

    if(pchip2 == NULL){
        PWM_ERR("Get pchip2 NuLL!!\n");
        return (-1);
    }

    pchip2->rtk_duty = duty;
    if(duty == 0){
        if(pchip2->rtk_polarity == 1)
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_HIGH);
        else
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_LOW);
    }
    else{
        rtk_pwm_force_mode_w(pchip2,PWM_NON_FORCE);
        rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);
    }
    rtk_pwm_db_wb(pchip2);

    if( m_ioctl_printk_get() > 0 ){

        PWM_WARN("%s set pwm[%d], duty=%d freq=%d lock=%d (pos=%d,%08x) adp = %d  adap_freq=%d\n",
                __func__ ,
                pchip2->index,
                pchip2->rtk_duty,
                pchip2->rtk_freq,
                pchip2->rtk_vsync,
                pchip2->rtk_pos_start,
                pchip2->rtk_pos_start_clk,
                pchip2->rtk_adpt_en,
                pchip2->rtk_adapt_freq);
    }

    return 0;
}
EXPORT_SYMBOL(rtk_pwm_backlight_set_duty);

//======================================================================
//CHIP PCB Fucntion
//======================================================================
int rtk_pwm_pcb_mgr_get(const char *pcbname,int ricindex)
{
    int ret = 0;
    ret = pcb_mgr_get_enum_info_byname((char *)pcbname,
                     &rtk_pwm_chip[ricindex].ullPcbMgrValue);
    return ret;
}

int rtk_pwm_pcb_mgr_get_value(RTK_PCB_ENUM *pcbinfo,int ricindex)
{
    int ret = 0;
    pcbinfo->value = rtk_pwm_chip[ricindex].ullPcbMgrValue;
    return ret;
}

int rtk_pwm_chip_index_get(int ricindex)
{    return rtk_pwm_chip[ricindex].index;    }

unsigned long long rtk_pwm_chip_pcb_mgr_value_get(int ricindex)
{    return rtk_pwm_chip[ricindex].ullPcbMgrValue;    }

const rtk_pwm_reg_map* rtk_pwm_chip_reg_get(int ricindex)
{    return rtk_pwm_chip[ricindex].reg;    }


#ifndef BUILD_QUICK_SHOW
void rtk_pwm_chip_device_set(struct platform_device *pdev)
{
#ifdef CONFIG_RTK_FEATURE_FOR_GKI
    //rtk_pwm_chip[pin_index_get()].pst_rtk_pwm_dev =pwm_get(&pdev->dev, pdev->name);
#endif
}
#endif

R_CHIP_T* rtk_pwm_chip_get(int m_index)
{    return &rtk_pwm_chip[m_index];    }

R_CHIP_T* rtk_pwm_chip_get_by_index(int m_index,int m_type)
{
    int i = 0;
    R_CHIP_T *pchip2 = NULL;
    int pwm_type = 0;
    for (i = 0; i < MAX_PWM_NODE; i++) {
        pchip2 = &rtk_pwm_chip[i];
        if(GET_PIN_TYPE(pchip2->ullPcbMgrValue) == PCB_PIN_TYPE_PWM)
            pwm_type = PWM_MISC;
        else if (GET_PIN_TYPE(pchip2->ullPcbMgrValue) == PCB_PIN_TYPE_ISO_PWM)
            pwm_type = PWM_ISO;

        if(pchip2->index == m_index && pwm_type == m_type)
            return &rtk_pwm_chip[i];
    }
    return NULL;
}

R_CHIP_T* rtk_pwm_chip_get_by_name(char* pcbname)
{
    int i = 0;
    R_CHIP_T *pchip2 = NULL;
    for (i = 0; i < MAX_PWM_NODE; i++) {
        pchip2 = &rtk_pwm_chip[i];

        if(strcmp(pchip2->pcbname,pcbname) == 0){
            return &rtk_pwm_chip[i];
        }
    }
    return NULL;
}

//======================================================================
//IOCTL Debug Function
//======================================================================
void m_ioctl_printk_set(int en_printk)         {    m_ioctl_printk = en_printk;    }
int  m_ioctl_printk_get(void)                  {    return m_ioctl_printk;         }
void m_ioctl_disable_set(int disable)          {    m_ioctl_disable = disable;     }
int  m_ioctl_disable_get(void)                 {    return m_ioctl_disable;        }
void m_ioctl_counter_inc(void)                 {    m_ioctl_counter++;             }
void m_ioctl_counter_set(unsigned int counter) {    m_ioctl_counter = counter;     }
unsigned int m_ioctl_counter_get(void)         {    return m_ioctl_counter;        }

//======================================================================
//General Function
//======================================================================
void getHVTotal(int *htotal, int *vtotal)
{
    ppoverlay_dv_total_RBUS dvReg;
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
    *htotal = pwm_get_disp_htotal();
#endif
    dvReg.regValue = rtd_inl(PPOVERLAY_DV_TOTAL_VADDR);
#ifdef NEW_PWM_FORMULA
    *vtotal = dvReg.dv_total*100;
#else
    *vtotal = dvReg.dv_total;
#endif

    if (*htotal == 0) {
        PWM_ERR("Get 0 htotal, use 1 instead of 0\n");
        *htotal = 0x897;
    }

    if (*vtotal == 0) {
        PWM_ERR("Get 0 vtotal, use 0x464 instead of 0\n");
        *vtotal = 0x464;
    }

};

void get_v_delay(R_CHIP_T *pchip2)
{
    int mode = pchip2->rtk_clk_sel;
    int clk_period = 1000000000/rtk_hw_get_clock(mode);
    unsigned long phase_us=(1000000000/pchip2->rtk_freq);

    pchip2->rtk_pos_start_clk = pchip2->rtk_pos_start*phase_us/pchip2->rtk_duty_max / clk_period;
    if(pchip2->rtk_pos_start_clk >=0x1fffff) {

        PWM_ERR("PWM vsync overflow!\n");
        PWM_ERR("adapt_fre (base 0.01Hz) = %d, freq = %d, pos = %d, phase_us = %ld\n",
            pchip2->rtk_adapt_freq, pchip2->rtk_freq,
            pchip2->rtk_pos_start, phase_us);

        pchip2->rtk_pos_start_clk= 1;
    }

};

/*porting from Ben:scalerDisplay.cpp
*/

int getI2Dstatus(void)
{
    ppoverlay_i2d_ctrl_0_RBUS ppoverlay_i2d_ctrl_0_reg = {0};
    ppoverlay_i2d_ctrl_0_reg.regValue = rtd_inl(PPOVERLAY_I2D_CTRL_0_reg);
    return ppoverlay_i2d_ctrl_0_reg.i2d_special_dly_en;
}

void getDClk(UINT64 *dclk)
{
    pll27x_reg_sys_pll_disp1_RBUS pll_disp1_reg = {0};
    pll27x_reg_pll_ssc0_RBUS  pll_ssc0_reg = {0};
    pll27x_reg_sys_pll_disp3_RBUS pll_disp3_reg = {0};
    sys_reg_sys_dispclksel_RBUS disp_clk_reg = {0};
    unsigned int Mcode = 0, Ncode = 0, Fcode = 0, Odiv = 0 ;
    unsigned int temp = 0;

    pll_disp1_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_DISP1_reg);
    pll_ssc0_reg.regValue = rtd_inl(PLL27X_REG_PLL_SSC0_reg);
    pll_disp3_reg.regValue = rtd_inl(PLL27X_REG_SYS_PLL_DISP3_reg);
    disp_clk_reg.regValue = rtd_inl(SYS_REG_SYS_DISPCLKSEL_reg);

    Mcode = pll_disp1_reg.dpll_m;
    Ncode = pll_disp1_reg.dpll_n;
    Fcode = pll_ssc0_reg.fcode_t_ssc;

    Odiv = (0x1 << pll_disp3_reg.dpll_o);
    temp = 2700 * (Mcode + 3 + (Fcode/2048)) / ((Ncode + 2)*Odiv);

    //temp = temp/(0x1 << disp_clk_reg.dispd_osd_div);  // fixed me, no this member in header file
    *dclk = ((UINT64)temp * 10000);

}

int rtk_get_pwm_duty_base_change(int duty,int duty_max,int totalcnt)
{
    int base_duty = 0;
    base_duty = DIV_ROUND_CLOSEST(duty*(totalcnt+1),(duty_max));
    return base_duty;
}

unsigned long rtk_hw_get_clock(int mode)
{
    UINT64 dclk = 0;
    unsigned long clock_freq;
    switch(mode){
        case CLK_27MHZ:
            clock_freq = CONFIG_CPU_XTAL_FREQ;
            break;

        case CLK_196MHZ:
            clock_freq = CONFIG_PLL_FREQ;
            break;

        case D_CLK_DIV4:
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            dclk = pwm_get_disp_dclk();
#endif
            clock_freq = (unsigned long) dclk >> 2; //Divide 4
            break;
        case CLK_147MHZ:
            clock_freq = CONFIG_ISO_PLL_FREQ;
            break;

        default:
            clock_freq = CONFIG_CPU_XTAL_FREQ;
    }
    return clock_freq;
}

unsigned long rtk_cpu_ns_get(int mode)
{
    UINT64 dclk = 0;
    UINT64 cpu_ns = CONFIG_CPU_XTAL_NS_27MHZ;
    switch(mode){
        case CLK_27MHZ:
            cpu_ns = CONFIG_CPU_XTAL_NS_27MHZ;
            break;

        case CLK_196MHZ:
            cpu_ns = CONFIG_CPU_PLL_NS_196MHZ;
            break;

        case D_CLK_DIV4:
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
            dclk = pwm_get_disp_dclk();
#endif
            cpu_ns = div64_u64(1000000000,dclk);//Divide 4
            break;
        case CLK_147MHZ:
            cpu_ns = CONFIG_CPU_PLL_NS_147MHZ;
            break;

        default:
            cpu_ns = CONFIG_CPU_XTAL_NS_27MHZ;
    }

    return cpu_ns;
}

/*
    PWM freq = CLK/(2^M*(N+1)*(TotalCnt+1))
*/
void rtk_hw_get_divider(int pwm_freq_hz, int duty_max, int *totalcnt,
                   unsigned short *M_best, unsigned short *N_best,
                   int pwm_freq_min, int mode)
{
    int j = 0,stop = 0,m = 0,m_mod,n_mod,pwmFreqHz = pwm_freq_hz / 100,approachFreq = 0;
    uint64_t ftmp_1 = 0,ftmp_2 = 0;
    uint64_t n_total = 0,new_n_total=0;
    uint64_t hw_clock = rtk_hw_get_clock(mode);

    //new algorithm to calculate the best M,N,TOTALCNT
    for (m=0; m<=3; m++){
        ftmp_1 = (1 << m)*pwmFreqHz;
        m_mod = hw_clock % ftmp_1;

        if(m_mod < ftmp_1/2){
            n_total = (hw_clock / ftmp_1);
        }else{
            n_total = (hw_clock / ftmp_1)+1;
        }
        if(pwmFreqHz  > 105000){  //if freq large than 105K, we should not care totalnum because there is no duty accurate in this case
            j=2; //totalcnt must large than 1
        }else{
            j=256;
        }
        for (; j<4097;j++){ //j is totalnum
            if(j > n_total)
                break;
            if((n_total / j) > 4096)
                continue;

            n_mod = n_total % j;
            if(n_mod == 0){
                new_n_total = n_total;
                if(m_mod == 0){
                    stop = 1;//accurate is 100%
                }
            }else{
                if(n_mod < j/2){
                    new_n_total = (n_total / j)*j;  //j=250, ntotal=300, new_n_total=250
                }else{
                    new_n_total = (n_total / j)*j+j;//j=250, ntotal=450, new_n_total=500
                }
            }
            ftmp_2 = (hw_clock*100)/((1 << m)*new_n_total);
            if((approachFreq==0) || ((new_n_total / j)>0 &&(ABS(ftmp_2,pwm_freq_hz) < ABS(approachFreq,pwm_freq_hz)))){
                approachFreq = ftmp_2;
                *M_best = m;
                *N_best = (new_n_total / j)-1;
                *totalcnt = j-1;
            }

            if(approachFreq == pwm_freq_hz)
                stop = 1;
            	
            if(stop)
                break;
        }
        if(stop)
            break;
    }

    approachFreq = (hw_clock*100)/((1 << *M_best)*(*N_best+1)*(*totalcnt+1));

    PWM_WARN("%s CLK_SRC:%ld freq:%d approachFreq=%d diff=%d M_best = 0x%X N_best = 0x%X totalcnt:%d duty_max:%d\n",__func__, rtk_hw_get_clock(mode),pwm_freq_hz,approachFreq,(approachFreq-pwm_freq_hz) , *M_best,*N_best ,*totalcnt,duty_max);
}


int calculate_start_end(UINT64 *target,int *n_start,int *n_end)
{
    int i =0;
    UINT64 result = 0;

    for( i = 8; i <= 12 ; ++i){//from duty_max 255 to found solution
        if((div64_u64((*target),(UINT64)(1<< i))) > N_MAX)
            continue;

        if((div64_u64((*target),(UINT64)(1<< i))) < N_MAX)
            break;
    }
    *n_start = (1 << i);
    result=div64_u64((*target),(UINT64)(*n_start));

    for( i = 1;i <= 12 ; ++i){
        if((div64_u64(result,(UINT64)(1<<i))==1) && ((div64_u64(result,(UINT64)(1<<(i+1))))==0))
            break;
    }
    *n_end = (1<<i);
    return 0;
}

/*
DVS = DCLK/(Htotal*Vtotal) , general is 60Hz
PWM = n_pwm*DVS = (n_pwm*DCLK)/(Htotal*Vtotal) =DCLK/4/(2^M*(N+1)*(TotalCnt+1))
(Htotal*Vtotal)/(n_pwm*4) = 2^M*(N+1)*(TotalCnt+1)

spec Vsync_period = V_total_reg* (H_total_reg+1) + (Last_line_reg+1)
if Last_line_reg ==H_total_reg
Vsync_period = (vtotal_reg+1)* (htotal_reg+1)

Params:
h_total : mean H_total_reg+1 according to scaler
v_total : mean V_total_reg according to scaler
n_pwm   : multiple of DVS
last_line = h_total = (H_total_reg+1) = (Last_line_reg+1)

pwm vsync pireod : h_total*v_total+h_total =(v_total+1)*h_total =(vtotal_reg+1)* (htotal_reg+1)
*/
int rtk_hw_getMN(int h_total,int v_total,int n_pwm,int last_line,
                 int duty_max, int *totalcnt,
                 unsigned short *M_best, unsigned short *N_best){
    int m = 0,mod=0;
    int total = 0;
    int stop = 0;
    int n_total = 0,new_n_total = 0,m_mod,n_mod;
    UINT64 target = 0, ftmp_1 = 0,ftmp_2 = 0,approachTarget = 0;
    UINT64 CLK = rtk_hw_get_clock(D_CLK_DIV4);
    UINT64 approachFreq = 0,targetFreq = 0;

    if(h_total == 0 || v_total == 0 || n_pwm == 0 || last_line == 0){
        PWM_ERR("[%s] param error, set to min freq, h_total:%d, v_total:%d, n_pwm:%d, last_line:%d\n",__func__, h_total,v_total,n_pwm,last_line);
        goto abnormal;
    }
    //new algorithm to calculate the best M,N,TOTALCNT
    ftmp_1 = (UINT64)h_total * (UINT64)v_total+last_line;

    targetFreq = div64_u64(CLK*4*n_pwm*100,ftmp_1);//multi 100

    if(targetFreq == 0){
        PWM_ERR("[%s] targetFreq == 0, set to min freq, CLK:%llu, n_pwm:%d, ftmp_1:%llu\n",__func__, CLK,n_pwm,ftmp_1);
        goto abnormal;
    }

    target = div_u64_rem(ftmp_1,4*n_pwm,&mod);//(2^m)*n*totalcnt

    //round(target)
    if(mod > 2*n_pwm){
        target=target+1;
    }
    //new algorithm to calculate the best M,N,TOTALCNT
    for (m=0; m<=3; m++){
        m_mod = target % (1 << m);

        n_total = DIV_ROUND_CLOSEST(target,(1 << m));

        for (total=256; total<4097; total++){
            if(total > n_total)
                break;
            if(div64_u64(n_total,total)>4096)
                continue;

            n_mod = n_total % total;
            if(n_mod == 0){
                new_n_total = n_total;
                if(m_mod == 0){
                    stop = 1;//accurate is 100%
                }
            }else{
                new_n_total = DIV_ROUND_CLOSEST(n_total,total) * total;
            }
            ftmp_2 = (1 << m) * new_n_total;

            if((approachTarget == 0) || (new_n_total > total &&(ABS(ftmp_2,target) < ABS(approachTarget,target)))){
                approachTarget = ftmp_2;
                *M_best = m;
                *N_best = div64_u64(new_n_total,total)-1;
                *totalcnt = total-1;
            }

            if(approachTarget == target)
                stop = 1;

            if(stop)
                break;
        }
        if(stop)
            break;
    }
    if( m_ioctl_printk_get() > 0 ){
        approachFreq = div64_u64(CLK*100,((UINT64)1 << *M_best) * (*N_best+1) * (*totalcnt+1));
        PWM_WARN("[%s] new found M/N Solution H:%d V:%d M:%d N:%d totalcnt:%d,target=%llu,"
                "approachTarget=%llu,error:%llu, targetFreq=%llu, approachFreq=%llu, diff=%llu\n",__func__, \
                h_total,v_total,*M_best,*N_best,*totalcnt,target,approachTarget,(target-approachTarget),\
                targetFreq,approachFreq,(targetFreq-approachFreq));
    }
    if(*totalcnt == 0)
        goto abnormal;

    return 0;

abnormal:
    *M_best = 3;
    *N_best = 0xfff;
    *totalcnt = duty_max;
    return -1;
}


#ifdef NEW_PWM_FORMULA
int rtk_hw_getMN_ex(int h_total,int v_total,int n_pwm,int last_line,
                 int duty_max, int *totalcnt,
                 unsigned short *M_best, unsigned short *N_best){
    int m = 0;
    int n = 0;
    int total = 0;
    UINT64 pwm_div = 0;
    int error = 0;
    int found = 0;
    int increase = 0;
    int max_increase = 0;
    int ht_modulo = 0;
    int increase_n = 0, increase_total =0;
    int min_error = 0;
    int n_start, n_end;
    UINT64 target = 0;
    UINT64 target_ex = 0;

    target = div64_u64((((UINT64)h_total*(UINT64)v_total)+(UINT64)last_line),(4*(UINT64)n_pwm));

    ht_modulo = h_total % 100;

    while (!(target & 0x1) && m < M_MAX) {
        target >>= 1;
        m++;
        if(m == M_MAX){
            m--;
            target <<= 1;
            break;
        }
    }
    *M_best = m;

    target_ex = div64_u64(target,100);
    calculate_start_end(&target_ex,&n_start,&n_end);

    for (total = n_start; total < 4096; total++) {
        for (n = n_end; n < N_MAX; n++){
            pwm_div = (UINT64)(n + 1) * (UINT64)(total + 1)*100;

            error = ABS(target, pwm_div);

            if(error < 500){
               if( error < min_error || min_error == 0){
                    increase_n = n;
                    increase_total = total;
                    min_error = error;
                }
            }

            if(total == 4095){
                *N_best = increase_n;
                *totalcnt = increase_total;
                max_increase = 1;
                goto out_loop;
            }

            if (likely(!ht_modulo)){
                if (likely(!error)) {
                    *N_best = n;
                    *totalcnt = total;
                    found = 1;
                    goto out_loop;
                }
                else{
                    if(error < 50){
                        *N_best = n;
                        *totalcnt = total;
                        increase = 1;
                        goto out_loop;
                    }
                }
            } else { // endurable.
                if(error < 50){
                    *N_best = n;
                    *totalcnt = total;
                    increase = 1;
                    goto out_loop;
                }
            }
        }
    }

out_loop:

    if(found == 1)
        PWM_WARN("[%s]Found M/N Solution H:%d V:%d M:%d N:%d totalcnt:%d error:%d\n",__func__, h_total,v_total,*M_best,*N_best,*totalcnt,error);
    else if(increase == 1)
        PWM_WARN("[%s]Found Increase M/N Solution H:%d V:%d M:%d N:%d totalcnt:%d error:%d\n",__func__ ,h_total,v_total,*M_best,*N_best,*totalcnt,error);
    else if(max_increase == 1)
        PWM_WARN("[%s]Found Max Increase M/N Solution H:%d V:%d M:%d N:%d totalcnt:%d min_error:%d\n", __func__,h_total,v_total,*M_best,*N_best,*totalcnt,min_error);
    else
        PWM_WARN("[%s]NO found M/N Solution H:%d V:%d M:%d N:%d totalcnt:%d error:%d\n",__func__, h_total,v_total,*M_best,*N_best,*totalcnt,error);

    return 0;
}
#endif

/*
 * NOTICE: Caller should hold the pchip2->lock
 * to protect critical section.
 * */
int rtk_pwm_set_freq(R_CHIP_T *pchip2, int h_total, int v_total, int ifold,bool bEnableDB)
{
    UINT64 result = 0;

    if (unlikely(!pchip2)) {
        return -ENODEV;
    }

    //Skip LED_CURRENT Panel PWM Frequency
    if (rtk_pwm_panel_mode_get() == PWM_DRIVING_LED_CURRENT && pchip2->rtk_freq == 200) {
        return -EPERM;
    }

    if (unlikely(ifold > 20 || ifold <= 0)) {
        PWM_ERR( "PWM %d-fold error!\n",ifold);
        return -EINVAL;
    }
 
    if(pchip2->rtk_adpt_en !=0)
    {
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
        pchip2->dclk = pwm_get_disp_dclk();
#endif
        pchip2->ht = h_total;
        pchip2->vt = v_total;
        result = div64_u64(pchip2->dclk,(pchip2->ht))*10;
#ifdef NEW_PWM_FORMULA
        pchip2->rtk_freq_100times =div64_u64(result*100,(pchip2->vt+1))*ifold*10;
#else
        pchip2->rtk_freq_100times =div64_u64(result,(pchip2->vt+1))*ifold*10;
#endif
        pchip2->rtk_adapt_freq = pchip2->rtk_freq = (pchip2->rtk_freq_100times)/100;
        pchip2->rtk_freq_changed = true;
    }
    else
    {
        pchip2->rtk_adapt_freq = rtk_pwm_get_adapt_freq(pchip2);
        pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;
        if( m_ioctl_printk_get() > 0 ){
            PWM_WARN("%s set pwm[%d], diable pwm Adaptive , duty=%d freq=%d lock=%d , iFreqTemp=%d \n",
            __func__ ,
            pchip2->index,
            pchip2->rtk_duty,
            pchip2->rtk_freq,
            pchip2->rtk_vsync,
            pchip2->rtk_freq_100times);
        }
    }

    /*set timing ctrl , freq */

    rtk_pwm_freq_w(pchip2);


    if(pchip2->rtk_duty == 0){
        if(pchip2->rtk_polarity == 1)
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_HIGH);
        else
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_LOW);
    }
    else{
        rtk_pwm_force_mode_w(pchip2,PWM_NON_FORCE);
        rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);
    }

    rtk_pwm_totalcnt_w(pchip2,pchip2->rtk_totalcnt);
    rtk_pwm_vsync_interval_w(pchip2,pchip2->rtk_vsync_interval);

/*set DB */

    rtk_pwm_db_sel_enable(pchip2,0);
    rtk_pwm_db_enable(pchip2,1);

    rtk_pwm_db_wb(pchip2);
 
    PWM_WARN("%s set pwm[%d], duty=%d, freq=%d, lock=%d, (pos=%d,%08x), adp = %d, adap_freq(base 0.01Hz)=%d, iFreqTemp=%d\n",
                __func__ ,
                pchip2->index,
                pchip2->rtk_duty,
                pchip2->rtk_freq,
                pchip2->rtk_vsync,
                pchip2->rtk_pos_start,
                pchip2->rtk_pos_start_clk,
                pchip2->rtk_adpt_en,
                pchip2->rtk_adapt_freq,
                pchip2->rtk_freq_100times);

    return 0;
}

int rtk_pwm_get_freq(R_CHIP_T *pchip2,unsigned short M, unsigned short N)
{

    int mode = pchip2->rtk_clk_sel;
    unsigned long clock_freq = rtk_hw_get_clock(mode);
    pchip2->rtk_freq_100times = ((clock_freq/(1 << M)/(N+1))*100)/(pchip2->rtk_totalcnt+1);
    return pchip2->rtk_freq_100times;
}


int rtk_pwm_get_adapt_freq(R_CHIP_T *pchip2)
{
    int retAdapFreq = 0;
    int ht,vt = 0;
    UINT64 dclk = 0;
    if(getI2Dstatus() == 1){
        getHVTotal(&ht, &vt);
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
        dclk = pwm_get_disp_dclk();
#endif
        retAdapFreq = div64_u64(dclk,ht);
        retAdapFreq = div64_u64(retAdapFreq,vt);
        pchip2->rtk_freq_changed = false;
    }
    else{
        getHVTotal(&ht, &vt);
        if(pchip2->ht == ht && pchip2->vt == vt){
            pchip2->rtk_freq_changed = false;
        }
        else{
            pchip2->rtk_freq_changed = true;
            pchip2->ht = ht;
            pchip2->vt = vt;
        }
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
        pchip2->dclk = pwm_get_disp_dclk();
#endif
        retAdapFreq = div64_u64(pchip2->dclk,pchip2->ht);
        retAdapFreq = div64_u64(retAdapFreq,pchip2->vt);
    }
    return retAdapFreq;
}

int rtk_get_pos(R_CHIP_T *pchip2,int delay_time,int m_freq)
{
    //Delay clock cycle to unit
    //CONFIG_VSYNC_FREQ default is 60 for boot
    unsigned int phase_us= 0, pos =0, temp=0;
    int mode = pchip2->rtk_clk_sel;
    int clk_period = 1000000000/rtk_hw_get_clock(mode);
    phase_us= 1000000000/m_freq;
    temp = phase_us/pchip2->rtk_duty_max/clk_period;
    pos = delay_time/temp;
        PWM_WARN("%s pos %d, temp:%d\n",__func__, pos,temp);
    return pos;
}

//======================================================================
//Probe Init Function
//======================================================================
int rtk_pwm_hw_type_init(R_CHIP_T *pwmPin)
{
    unsigned long pwm_id;
    pwm_id = GET_PIN_INDEX(pwmPin->ullPcbMgrValue);
    if (GET_PIN_TYPE(pwmPin->ullPcbMgrValue) == PCB_PIN_TYPE_ISO_PWM) {

        if ((iso_pwm_usage &  (1<<pwm_id)))
            return -1;

        iso_pwm_usage |= 1<<pwm_id;

        rtk_pwm_setting_power_on(CRT_PWM_ISO);

        snprintf(pwmPin->nodename, 16, "pwm-I-%d",
             GET_PIN_INDEX(pwmPin->ullPcbMgrValue));

        pwmPin->reg =
            pwm_iso_phy[GET_PIN_INDEX(pwmPin->ullPcbMgrValue)].p_reg_map;

        pwmPin->rtk_db_reg = ISO_PWM_DB_CTRL_VADDR;

    } else if (GET_PIN_TYPE(pwmPin->ullPcbMgrValue) ==  PCB_PIN_TYPE_PWM) {

        if ((misc_pwm_usage &  (1<<pwm_id)))
            return -1;

        misc_pwm_usage |= 1<<pwm_id;

        rtk_pwm_setting_power_on(CRT_PWM_MISC);

        snprintf(pwmPin->nodename, 16, "pwm-M-%d",
                 GET_PIN_INDEX(pwmPin->ullPcbMgrValue));

        pwmPin->reg =pwm_mis_phy[GET_PIN_INDEX(pwmPin->ullPcbMgrValue)].p_reg_map;

        pwmPin->rtk_db_reg = MIS_PWM_DB0_CTRL_VADDR;
    } else {
        pwmPin->index = (-1);
        return -1;
    }

    pwmPin->index = GET_PIN_INDEX(pwmPin->ullPcbMgrValue);
    return 0;
}

void rtk_pwm_hw_chip_init(R_CHIP_T *pwmPin, int ricindex)
{
    //Initial Clock source
    pwmPin->rtk_clk_sel = GET_PWM_CLK_SRC(pwmPin->ullPcbMgrValue);
    if(pwmPin->rtk_clk_sel == D_CLK_DIV4)
        pwmPin->rtk_freq_range = (pwmPin->rtk_totalcnt == 2047) ? (CLK_DIV4_8BIT):(CLK_DIV4_12BIT);
    else if(pwmPin->rtk_clk_sel == CLK_27MHZ)
        pwmPin->rtk_freq_range = (pwmPin->rtk_duty_max == 255) ? (CLK_27MHZ_8BIT):(CLK_27MHZ_12BIT);
    else if(pwmPin->rtk_clk_sel == CLK_196MHZ)
        pwmPin->rtk_freq_range = (pwmPin->rtk_duty_max == 255) ? (CLK_196MHZ_8BIT):(CLK_196MHZ_12BIT);
    else if(pwmPin->rtk_clk_sel == CLK_147MHZ)
        pwmPin->rtk_freq_range = (pwmPin->rtk_duty_max == 255) ? (CLK_147MHZ_8BIT):(CLK_147MHZ_12BIT);

    rtk_pwm_clock_freq_set(pwmPin);
    rtk_pwm_clock_source(pwmPin);

    pwmPin->index = ricindex;
    pwmPin->mapped = true;
    pwmPin->rtk_force_update = true;
    pwmPin->rtk_duty_max = GET_PWM_DUTY_MAX(pwmPin->ullPcbMgrValue);
    pwmPin->rtk_totalcnt = rtk_pwm_totalcnt_r(pwmPin);
    pwmPin->rtk_freq_100times = rtk_pwm_freq_r(pwmPin);

    pwmPin->rtk_freq = (pwmPin->rtk_freq_100times/100);
    pwmPin->lg_freq48n = 96;
    pwmPin->lg_freq50n = 100;
    pwmPin->lg_freq60n = 120;
    pwmPin->rtk_duty = ((MAX_PWM_DUTY+1)*rtk_pwm_duty_r(pwmPin))/(rtk_pwm_totalcnt_r(pwmPin)+1);
    pwmPin->rtk_vsync = rtk_pwm_vsync_r(pwmPin);
    pwmPin->rtk_enable = rtk_pwm_output_r(pwmPin);
    pwmPin->rtk_polarity = GET_PWM_INVERT(pwmPin->ullPcbMgrValue);

    if (pwmPin->rtk_polarity != rtk_pwm_ctrl_polarity_r(pwmPin)) {
        rtk_pwm_ctrl_polarity_w(pwmPin, pwmPin->rtk_polarity);
    } 

    /*rtk_adapt_freq  base is 0.01hz*/
    pwmPin->rtk_adapt_freq =rtk_pwm_get_adapt_freq(pwmPin);

    if( pwmPin->rtk_vsync == PWM_VSYNC_ENABLE ){
        pwmPin->rtk_pos_start_clk = rtk_pwm_vs_delay_r(pwmPin);
        pwmPin->rtk_pos_start = rtk_get_pos(pwmPin,pwmPin->rtk_pos_start_clk,pwmPin->rtk_freq);

    }else{
        pwmPin->rtk_pos_start = 0;
        pwmPin->rtk_pos_start_clk = 1;
    }

    pwmPin->rtk_adpt_en = 0;
    if(pwmPin->rtk_clk_sel == D_CLK_DIV4)
        pwmPin->rtk_adpt_en = 1;
}

void rtk_pwm_ctrl_probe_init(R_CHIP_T *pchip2,int pwm_mode,int invert)
{
    rtk_pwm_ctrl_polarity_w(pchip2,invert);
    rtk_pwm_vsync_w(pchip2,pchip2->rtk_vsync);
    rtk_pwm_dvs_mux_w(pchip2,PWM_DVS_MUX_FORM_DVS);
    rtk_pwm_output_w(pchip2,PWM_OUTPUT_ENABLE);

    if(pwm_mode ==PWM_DRIVING_2CH_PHASE_DIFF){
        rtk_pwm_vs_delay_w(pchip2,1);
    }
    else{
        rtk_pwm_vs_delay_w(pchip2,0);
    }
}

void rtk_pwm_timing_probe_init(R_CHIP_T *pchip2,int pwm_freq_hz, int duty_max,int *totalcnt)
{
    pchip2->rtk_clk_sel = GET_PWM_CLK_SRC(pchip2->ullPcbMgrValue);

    if(pchip2->rtk_clk_sel == D_CLK_DIV4)
        pchip2->rtk_freq_range = (pchip2->rtk_totalcnt == 2047) ? (CLK_DIV4_8BIT):(CLK_DIV4_12BIT);
    else if(pchip2->rtk_clk_sel == CLK_27MHZ)
        pchip2->rtk_freq_range = (pchip2->rtk_duty_max == 255) ? (CLK_27MHZ_8BIT):(CLK_27MHZ_12BIT);
    else if(pchip2->rtk_clk_sel == CLK_196MHZ)
        pchip2->rtk_freq_range = (pchip2->rtk_duty_max == 255) ? (CLK_196MHZ_8BIT):(CLK_196MHZ_12BIT);
    else if(pchip2->rtk_clk_sel == CLK_147MHZ)
        pchip2->rtk_freq_range = (pchip2->rtk_duty_max == 255) ? (CLK_147MHZ_8BIT):(CLK_147MHZ_12BIT);

    rtk_pwm_clock_freq_set(pchip2);
    rtk_pwm_clock_source(pchip2);
    pchip2->rtk_freq_changed = true;
    /*pchip2->rtk_freq = pwm_freq_hz;
    pchip2->rtk_freq_100times = pwm_freq_hz * 100;*/
    rtk_pwm_freq_w(pchip2);
    *totalcnt = pchip2->rtk_totalcnt;
}

void rtk_pwm_duty_probe_init(R_CHIP_T *pchip2,int duty, int duty_max,int totalcnt)
{
    pchip2->rtk_duty = duty;
    rtk_pwm_totalcnt_w(pchip2,totalcnt);
    rtk_pwm_duty_w(pchip2,duty);
}

void rtk_pwm_db_probe_init(R_CHIP_T *pchip2)
{
    rtk_pwm_db_enable(pchip2,1);
    rtk_pwm_db_wb(pchip2);
}

void rtk_pwm_user_setting(R_CHIP_T *pchip2,RTK_PWM_INFO_T *stpwminfo)
{
    unsigned long dvs_count = rtd_inl(DVS_COUNT);
    unsigned long output_vsync = 0;
    int mode = pchip2->rtk_clk_sel;

    pchip2->rtk_enable = stpwminfo->m_run;
    pchip2->rtk_duty = stpwminfo->m_duty;
    pchip2->rtk_vsync = stpwminfo->m_vsync?1:0;
    pchip2->rtk_adpt_en= stpwminfo->m_adpt_en;
    pchip2->rtk_pos_start = stpwminfo->m_pos;

    if(stpwminfo->m_freq48n <= 0 || stpwminfo->m_freq50n <= 0 || stpwminfo->m_freq60n <= 0 || stpwminfo->m_freq <= 0){
        PWM_ERR("[%s] stpwminfo->m_freq48n = %d, stpwminfo->m_freq50n = %d, stpwminfo->m_freq60n = %d, stpwminfo->m_freq = %d\n",
                            __func__, stpwminfo->m_freq48n, stpwminfo->m_freq50n, stpwminfo->m_freq60n, stpwminfo->m_freq);
        stpwminfo->m_freq48n = 1;
        stpwminfo->m_freq50n = 1;
        stpwminfo->m_freq60n = 1;
        stpwminfo->m_freq = 1;
    }


    /*rtk_adapt_freq  base is 0.01hz*/
    if(stpwminfo->m_adpt_en)
    {
        output_vsync = (rtk_hw_get_clock(mode)* 100)/ dvs_count;
        if(pchip2->rtk_adapt_freq == HZ_48)
        {
            if(output_vsync > 4790 && output_vsync < 4800)
                pchip2->rtk_freq_100times = output_vsync + (48*100);
            else
                pchip2->rtk_freq_100times = stpwminfo->m_freq48n * 100;

            pchip2->rtk_freq           = stpwminfo->m_freq48n;
            pchip2->lg_freq48n         = stpwminfo->m_freq48n;
            pchip2->rtk_vsync_interval = 0;
        }
        else if(pchip2->rtk_adapt_freq == HZ_50)
        {
            if(output_vsync > 4990 && output_vsync < 5000)
                pchip2->rtk_freq_100times = output_vsync + (50*100);
            else
                pchip2->rtk_freq_100times = stpwminfo->m_freq50n * 100;

            pchip2->rtk_freq           = stpwminfo->m_freq50n;
            pchip2->lg_freq50n         = stpwminfo->m_freq50n;
            pchip2->rtk_vsync_interval = 0;

        }
        else if(pchip2->rtk_adapt_freq == HZ_60)
        {
            if(output_vsync > 5990 && output_vsync < 6000)
                pchip2->rtk_freq_100times = output_vsync + (60*100);
            else
                pchip2->rtk_freq_100times = stpwminfo->m_freq60n * 100;

            pchip2->rtk_freq           = stpwminfo->m_freq60n;
            pchip2->lg_freq60n         = stpwminfo->m_freq60n;
            pchip2->rtk_vsync_interval = 0;
        }else if(((pchip2->rtk_adapt_freq >= 51)&&(pchip2->rtk_adapt_freq <= 59) )
               ||((pchip2->rtk_adapt_freq >= 45)&&(pchip2->rtk_adapt_freq <= 47))){

            pchip2->rtk_freq_100times =    pchip2->rtk_adapt_freq*2*100;

            pchip2->rtk_freq           = pchip2->rtk_adapt_freq*2;
            pchip2->lg_freq60n         = stpwminfo->m_freq60n;
            pchip2->rtk_vsync_interval = 0;

            if( m_ioctl_printk_get() > 0 )
                PWM_WARN("PWM[%d]  special freq=%d \n",stpwminfo->m_index,pchip2->rtk_adapt_freq);
        }
        else
        {
            if( m_ioctl_printk_get() > 0 )
                PWM_ERR("PWM[%d] illegal freq=%d \n",stpwminfo->m_index,pchip2->rtk_adapt_freq);

            pchip2->rtk_freq           = stpwminfo->m_freq60n;
            pchip2->rtk_freq_100times  = stpwminfo->m_freq60n * 100;
            pchip2->rtk_vsync_interval = 0;
        }
    }
    else
    {
        pchip2->rtk_freq           = stpwminfo->m_freq ;
        pchip2->rtk_freq_100times  = stpwminfo->m_freq * 100;
        pchip2->rtk_vsync_interval = 0;
    }
}

void rtk_pwm_timing_freq_set_misc(R_CHIP_T *pchip2,misc_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg)
{
    unsigned short M = 0, N = 0;
    int min_freq = pwm_freq_range[pchip2->rtk_freq_range].min_freq;
    int mode = pchip2->rtk_clk_sel;
    int dvsync = 0;
    int times = 0;
#if IS_ENABLED(CONFIG_SUPPORT_SCALER)
    dvsync = ((pwm_get_disp_dclk()/pchip2->ht)/(pchip2->vt/100));
#endif
    times = (pchip2->rtk_freq/dvsync);

    if(pchip2->rtk_freq_changed == false && pchip2->rtk_force_update == false)
    {
        //Skip M/N Calculation
        PWM_WARN("[%s] Skip M/N Calculation (pchip2->rtk_freq_changed = %x, pchip2->rtk_force_update = %x)\n",
                                                            __func__, pchip2->rtk_freq_changed, pchip2->rtk_force_update);
    }
    else
    {
        if(pchip2->rtk_clk_sel == D_CLK_DIV4 && pchip2->rtk_adpt_en == 1){
#ifdef NEW_PWM_FORMULA
            rtk_hw_getMN(pchip2->ht,(pchip2->vt)/100,2,pchip2->ht,pchip2->rtk_duty_max, &pchip2->rtk_totalcnt,&M,&N);
#else
            rtk_hw_getMN(pchip2->ht,pchip2->vt,times,pchip2->ht,pchip2->rtk_duty_max, &pchip2->rtk_totalcnt,&M,&N);
#endif
        }
        else
            rtk_hw_get_divider(pchip2->rtk_freq_100times ,pchip2->rtk_duty_max, &pchip2->rtk_totalcnt,&M, &N,min_freq ,mode);

        if(PWM_MISC_TYPE(pchip2)){
            pwm_timing_ctrl_reg->pwm0_m = M;
            pwm_timing_ctrl_reg->pwm0_n = N;
        }
    }
}

void rtk_pwm_timing_freq_set_iso(R_CHIP_T *pchip2,iso_pwm_timing_ctrl_RBUS* pwm_timing_ctrl_reg)
{
    unsigned short M = 0, N = 0;
    int min_freq = pwm_freq_range[pchip2->rtk_freq_range].min_freq;
    int mode = pchip2->rtk_clk_sel;

    if(pchip2->rtk_freq_changed == false && pchip2->rtk_force_update == false)
    {
        //Skip M/N Calculation
        PWM_WARN("[%s] Skip M/N Calculation (pchip2->rtk_freq_changed = %x, pchip2->rtk_force_update = %x)\n",
                                                            __func__, pchip2->rtk_freq_changed, pchip2->rtk_force_update);
    }
    else
    {
        if(pchip2->rtk_clk_sel == D_CLK_DIV4 && pchip2->rtk_adpt_en == 1){
#ifdef NEW_PWM_FORMULA
            rtk_hw_getMN(pchip2->ht,(pchip2->vt)/100,2,pchip2->ht,pchip2->rtk_duty_max, &pchip2->rtk_totalcnt,&M,&N);

#else
            rtk_hw_getMN(pchip2->ht,pchip2->vt,2,pchip2->ht,pchip2->rtk_duty_max, &pchip2->rtk_totalcnt,&M,&N);
#endif
        }
        else
            rtk_hw_get_divider(pchip2->rtk_freq_100times ,pchip2->rtk_duty_max, &pchip2->rtk_totalcnt,&M, &N,min_freq ,mode);

        if(PWM_ISO_TYPE(pchip2)){
            pwm_timing_ctrl_reg->mis_pwm0_m = M;
            pwm_timing_ctrl_reg->mis_pwm0_n = N;
        }
    }
}

int rtk_pwm_duty_get(int duty,int duty_max,int totalcnt)
{    return DIV_ROUND_CLOSEST((duty)*(duty_max),(totalcnt+1));    }

unsigned long pwm_duty_ratio_get(unsigned long duty ,unsigned long pwm0_totalcnt){
    return (duty*1000000)/pwm0_totalcnt;
}

//======================================================================
//Clock Source Function
//======================================================================
#define SET_BIT(x, n) ( (x) |= (1<< (n)) )
#define CLR_BIT(x, n) ( (x) &= (~(1<< (n))))
void rtk_pwm_clock_source(R_CHIP_T *pchip2)
{
    if(PWM_MISC_TYPE(pchip2)){
        pwm_sys_clk_div_RBUS pwm_sys_clk_reg = {0};
        pwm_sys_clk_reg.regValue = rtd_inl(PWM_SYS_CLKDIV);
        switch(pchip2->rtk_clk_sel){
            case CLK_27MHZ:
                pchip2->rtk_clk_src = PWM_CRYSTAL;
                SET_BIT(pwm_sys_clk_reg.regValue,(SYS_REG_SYS_CLKDIV_pwm0_clksel_shift));
                pwm_sys_clk_reg.pwm_pll_clksel = 1;
                break;
            case CLK_196MHZ:
                pchip2->rtk_clk_src = PWM_PLL;
                CLR_BIT(pwm_sys_clk_reg.regValue,(SYS_REG_SYS_CLKDIV_pwm0_clksel_shift));
                 pwm_sys_clk_reg.pwm_pll_clksel = 0;
                break;
            case D_CLK_DIV4:
                pchip2->rtk_clk_src = PWM_PLL;
                CLR_BIT(pwm_sys_clk_reg.regValue,(SYS_REG_SYS_CLKDIV_pwm0_clksel_shift));
                pwm_sys_clk_reg.pwm_pll_clksel = 1;
                break;
            default:
                pchip2->rtk_clk_src = PWM_CRYSTAL;
                SET_BIT(pwm_sys_clk_reg.regValue,(SYS_REG_SYS_CLKDIV_pwm0_clksel_shift));
                break;
        }
        rtd_outl(PWM_SYS_CLKDIV,pwm_sys_clk_reg.regValue);
    }
    else if(PWM_ISO_TYPE(pchip2)){
#if defined (CONFIG_ARCH_RTK2851A) || defined (CONFIG_ARCH_RTK2851C) || defined (CONFIG_ARCH_RTK2851F)
        iso_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        I_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        switch(pchip2->rtk_clk_sel){
            case CLK_27MHZ:
                pwm_timing_ctrl_reg.mis_pwm0_ck_sel = PWM_CRYSTAL;
                break;
            case CLK_196MHZ:
                pwm_timing_ctrl_reg.mis_pwm0_ck_sel = PWM_PLL;
                break;
            default:
                pwm_timing_ctrl_reg.mis_pwm0_ck_sel = PWM_CRYSTAL;
                break;
        }
        I_PWM_TIMING_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
#else
        iso_pwm_clk_sel_RBUS iso_pwm_clk_sel_reg = {0};
        iso_pwm_clk_sel_reg.regValue = rtd_inl(ISO_PWM_SYS_CLKDIV);
        switch(pchip2->rtk_clk_sel){
            case CLK_27MHZ:
                iso_pwm_clk_sel_reg.iso_pwm_clk_sel = ISO_CLK_27MHZ;
                break;
            case CLK_147MHZ:
                iso_pwm_clk_sel_reg.iso_pwm_clk_sel = ISO_CLK_147MHZ;
                break;
            default:
                iso_pwm_clk_sel_reg.iso_pwm_clk_sel = ISO_CLK_27MHZ;
                break;
        }
        rtd_outl(ISO_PWM_SYS_CLKDIV,iso_pwm_clk_sel_reg.regValue);
#endif
    }
}

void rtk_pwm_clock_freq_set(R_CHIP_T *pchip2)
{
    switch(pchip2->rtk_freq_range){
        case CLK_27MHZ_8BIT:
        case CLK_27MHZ_12BIT:
            pchip2->rtk_clk_sel = CLK_27MHZ;
            pchip2->rtk_clk_src = PWM_CRYSTAL;
            break;

        case CLK_196MHZ_8BIT:
        case CLK_196MHZ_12BIT:
            pchip2->rtk_clk_sel = CLK_196MHZ;
            pchip2->rtk_clk_src = PWM_PLL;
            break;

        case CLK_DIV4_8BIT:
        case CLK_DIV4_12BIT:
            pchip2->rtk_clk_sel = D_CLK_DIV4;
            pchip2->rtk_clk_src = PWM_PLL;
            break;
        case CLK_147MHZ_8BIT:
        case CLK_147MHZ_12BIT:
            pchip2->rtk_clk_sel = CLK_147MHZ;
            pchip2->rtk_clk_src = PWM_PLL;
            break;

        default:
            pchip2->rtk_clk_sel = CLK_27MHZ;
            pchip2->rtk_clk_src = PWM_CRYSTAL;
    }
}



//======================================================================
//Global Variable Function
//======================================================================
void rtk_pwm_panel_mode_set(int mode)          {    rtk_pwm_panel_mode = mode;     }
int  rtk_pwm_panel_mode_get(void)              {    return rtk_pwm_panel_mode;     }
void pin_index_count(void)                     {    rtk_pwm_index++;               }
int  pin_index_get(void)                       {    return rtk_pwm_index;          }
void rtk_pwm_backlight_pin_set(int ricindex)   {    rtk_pwm_backlight_pin_index = ricindex;    }
int  rtk_pwm_backlight_pin_get(void)           {    return rtk_pwm_backlight_pin_index;        }
//==================================================================================

/** ISO Interrupt Control Function **/

void rtk_pwm_iso_interrupt_ctrl_enable_w(int enable){
    iso_misc_int_ctrl_RBUS pwm_iso_int_ctrl_reg = {0};
    PWM_ISO_INT_CTR_REG_R(&pwm_iso_int_ctrl_reg);
    pwm_iso_int_ctrl_reg.pwm0_int_to_scpu_en = enable;
    pwm_iso_int_ctrl_reg.pwm1_int_to_scpu_en = enable;
    PWM_ISO_INT_CTR_REG_W(&pwm_iso_int_ctrl_reg);
}

/** Register Fucntion**/
//MISC PWM_DB0_CTRL
#define M_GLOBAL_VSYNC_EN_R(reg) reg.global_vsync_en
#define M_PWM_W_DB0_MODE_R(num,reg) reg.pwm##num##_w_db0_mode
#define M_PWM_DB0_RD_SEL_R(num,reg) reg.pwm##num##_db0_rd_sel
#define M_PWM_W_DB0_EN_R(num,reg) reg.pwm##num##_w_db0_en
#define M_GLOBAL_VSYNC_EN_W(reg,val) reg.global_vsync_en = val
#define M_PWM_W_DB0_MODE_W(num,reg,val) reg.pwm##num##_w_db0_mode = val
#define M_PWM_DB0_RD_SEL_W(num,reg,val) reg.pwm##num##_db0_rd_sel = val
#define M_PWM_W_DB0_EN_W(num,reg,val) reg.pwm##num##_w_db0_en = val

void rtk_pwm_global_apply_wr(int enable){
    misc_pwm_db0_ctrl_RBUS pwm_mis_db0_reg = {0};
    M_PWM_DB0_CTRL_REG_R(0,&pwm_mis_db0_reg);
    M_GLOBAL_VSYNC_EN_W(pwm_mis_db0_reg,enable);
    M_PWM_DB0_CTRL_REG_W(0,&pwm_mis_db0_reg);
}

int rtk_pwm_global_enable_mw(R_CHIP_T *pchip2,int enable){
    misc_pwm_db0_ctrl_RBUS pwm_mis_db0_reg = {0};
    M_PWM_DB0_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_mis_db0_reg);
    M_GLOBAL_VSYNC_EN_W(pwm_mis_db0_reg,enable);
    M_PWM_DB0_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_mis_db0_reg);
    return 0;
}

int rtk_pwm_db_sel_enable_mw(R_CHIP_T *pchip2,int enable){
    misc_pwm_db0_ctrl_RBUS pwm_mis_db0_reg = {0};
    M_PWM_DB0_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_mis_db0_reg);
    switch(RTK_PWM_INDEX(pchip2)){
        case PWM0_DB0:
            M_PWM_DB0_RD_SEL_W(0,pwm_mis_db0_reg,enable);
            break;
        case PWM1_DB0:
            M_PWM_DB0_RD_SEL_W(1,pwm_mis_db0_reg,enable);
            break;
        case PWM2_DB0:
            M_PWM_DB0_RD_SEL_W(2,pwm_mis_db0_reg,enable);
            break;
        case PWM3_DB0:
            M_PWM_DB0_RD_SEL_W(3,pwm_mis_db0_reg,enable);
            break;
        case PWM4_DB0:
            M_PWM_DB0_RD_SEL_W(4,pwm_mis_db0_reg,enable);
            break;
        case PWM5_DB0:
            M_PWM_DB0_RD_SEL_W(5,pwm_mis_db0_reg,enable);
            break;
        case PWM6_DB0:
            M_PWM_DB0_RD_SEL_W(6,pwm_mis_db0_reg,enable);
            break;
        case PWM7_DB0:
            M_PWM_DB0_RD_SEL_W(7,pwm_mis_db0_reg,enable);
            break;
        default:
            break;
    }

    M_PWM_DB0_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_mis_db0_reg);
    return 0;
}

int rtk_pwm_db_enable_mw(R_CHIP_T *pchip2,int enable){
    misc_pwm_db0_ctrl_RBUS pwm_mis_db0_reg = {0};
    M_PWM_DB0_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_mis_db0_reg);
    switch(RTK_PWM_INDEX(pchip2)){
        case PWM0_DB0:
            M_PWM_W_DB0_MODE_W(0,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(0,pwm_mis_db0_reg,enable);
            break;
        case PWM1_DB0:
            M_PWM_W_DB0_MODE_W(1,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(1,pwm_mis_db0_reg,enable);
            break;
        case PWM2_DB0:
            M_PWM_W_DB0_MODE_W(2,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(2,pwm_mis_db0_reg,enable);
            break;
        case PWM3_DB0:
            M_PWM_W_DB0_MODE_W(3,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(3,pwm_mis_db0_reg,enable);
            break;
        case PWM4_DB0:
            M_PWM_W_DB0_MODE_W(4,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(4,pwm_mis_db0_reg,enable);
            break;
        case PWM5_DB0:
            M_PWM_W_DB0_MODE_W(5,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(5,pwm_mis_db0_reg,enable);
            break;
        case PWM6_DB0:
            M_PWM_W_DB0_MODE_W(6,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(6,pwm_mis_db0_reg,enable);
            break;
        case PWM7_DB0:
            M_PWM_W_DB0_MODE_W(7,pwm_mis_db0_reg,pchip2->rtk_vsync);
            M_PWM_W_DB0_EN_W(7,pwm_mis_db0_reg,enable);
            break;
        default:
            break;
    }

    M_PWM_DB0_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_mis_db0_reg);
    return 0;
}

//ISO PWM_DB_CTRL
#define I_GLOBAL_VSYNC_EN_R(reg) reg.mis_global_delay_en
#define I_PWM_W_DB_MODE_R(reg) reg.mis_pwm_w_db_mode
#define I_PWM_DB_RD_SEL_R(reg) reg.db_rd_sel
#define I_PWM_W_DB_EN_R(reg) reg.mis_pwm_w_db_en
#define I_GLOBAL_VSYNC_EN_W(reg,val) reg.mis_global_delay_en = val
#define I_PWM_W_DB_MODE_W(reg,val) reg.mis_pwm_w_db_mode = val
#define I_PWM_DB_RD_SEL_W(reg,val) reg.db_rd_sel = val
#define I_PWM_W_DB_EN_W(reg,val) reg.mis_pwm_w_db_en = val
int rtk_pwm_global_enable_iw(R_CHIP_T *pchip2,int enable){
    iso_pwm_db0_ctrl_RBUS pwm_iso_db0_reg = {0};
    I_PWM_DB_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_iso_db0_reg);
    I_GLOBAL_VSYNC_EN_W(pwm_iso_db0_reg,enable);
    I_PWM_DB_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_iso_db0_reg);
    return 0;
}

int rtk_pwm_db_enable_iw(R_CHIP_T *pchip2,int enable){
    iso_pwm_db0_ctrl_RBUS pwm_iso_db_reg = {0};
    I_PWM_DB_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_iso_db_reg);
    I_PWM_W_DB_MODE_W(pwm_iso_db_reg,pchip2->rtk_vsync);
    I_PWM_W_DB_EN_W(pwm_iso_db_reg,enable);
    I_PWM_DB_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_iso_db_reg);
    return 0;
}

int rtk_pwm_db_sel_enable_iw(R_CHIP_T *pchip2,int enable){
    iso_pwm_db0_ctrl_RBUS pwm_iso_db_reg = {0};
    I_PWM_DB_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_iso_db_reg);
    I_PWM_DB_RD_SEL_W(pwm_iso_db_reg,enable);
    I_PWM_DB_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_iso_db_reg);
    return 0;
}

int rtk_pwm_db_sel_enable(R_CHIP_T *pchip2,int enable){
    if(PWM_MISC_TYPE(pchip2)){
        rtk_pwm_db_sel_enable_mw(pchip2,enable);
    }
    else if(PWM_ISO_TYPE(pchip2)){
        rtk_pwm_db_sel_enable_iw(pchip2,enable);
    }

    return 0;
}

int rtk_pwm_db_enable(R_CHIP_T *pchip2,int enable){
    if(PWM_MISC_TYPE(pchip2)){
        rtk_pwm_db_enable_mw(pchip2,enable);
    }
    else if(PWM_ISO_TYPE(pchip2)){
        rtk_pwm_db_enable_iw(pchip2,enable);
    }

    return 0;
}

int rtk_pwm_global_enable(R_CHIP_T *pchip2,int enable){
    if(PWM_MISC_TYPE(pchip2)){
        rtk_pwm_global_enable_mw(pchip2,enable);
    }
    else if(PWM_ISO_TYPE(pchip2)){
        rtk_pwm_global_enable_iw(pchip2,enable);
    }
    return 0;
}

//Double Buffer Write Bit (DB Apply)
void rtk_pwm_db_wb(R_CHIP_T *pchip2){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_w_db0_wr = 1;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_w_db_wr = 1;
        I_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
}

//Only MISC PWM
void rtk_pwm_db1_wb(R_CHIP_T *pchip2){
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    pwm_ctrl_reg.pwm0_w_db1_wr = 1;
    M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
}

void rtk_pwm_ctrl_polarity_w(R_CHIP_T *pchip2,int enable){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0l = enable;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.mis_pwm0l = enable;
        I_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
}

int rtk_pwm_fixed_num_mode_r(R_CHIP_T *pchip2){
    int pwm_duty_select = 0;
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_duty_select = pwm_ctrl_reg.pwm0_fixed_num_mode;
    }

    return pwm_duty_select;
}

void rtk_pwm_fixed_num_mode_w(R_CHIP_T *pchip2,int enable){
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_fixed_num_mode = enable;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
}

int rtk_pwm_ctrl_polarity_r(R_CHIP_T *pchip2){
    int polarity = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        polarity = pwm_ctrl_reg.pwm0l;
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        polarity = pwm_ctrl_reg.mis_pwm0l;
    }
    return polarity;
}

void rtk_pwm_freq_w(R_CHIP_T *pchip2){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        M_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        rtk_pwm_timing_freq_set_misc(pchip2,&pwm_timing_ctrl_reg);
        M_PWM_TIMING_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
    }
    else{
        iso_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        I_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        rtk_pwm_timing_freq_set_iso(pchip2,&pwm_timing_ctrl_reg);
        I_PWM_TIMING_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
    }
}

int rtk_pwm_freq_r(R_CHIP_T *pchip2){
    unsigned short M = 0, N = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        M_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        M = pwm_timing_ctrl_reg.pwm0_m;
        N = pwm_timing_ctrl_reg.pwm0_n;
    }
    else{
        iso_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        I_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        M = pwm_timing_ctrl_reg.mis_pwm0_m;
        N = pwm_timing_ctrl_reg.mis_pwm0_n;
    }
    return rtk_pwm_get_freq(pchip2,M,N);
}

void rtk_pwm_mn_r(R_CHIP_T *pchip2,unsigned long *M, unsigned long *N){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        M_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        (*M) = pwm_timing_ctrl_reg.pwm0_m;
        (*N) = pwm_timing_ctrl_reg.pwm0_n;
    }
    else{
        iso_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        I_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        (*M) = pwm_timing_ctrl_reg.mis_pwm0_m;
        (*N) = pwm_timing_ctrl_reg.mis_pwm0_n;
    }
}

void rtk_pwm_num_w(R_CHIP_T *pchip2,int num){
    misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.pwm0_num = num;
        M_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
    //ISO no this
}

void rtk_pwm_duty_w(R_CHIP_T *pchip2,int duty){
    pchip2->rtk_duty = duty;
    duty = rtk_get_pwm_duty_base_change(pchip2->rtk_duty,pchip2->rtk_duty_max,pchip2->rtk_totalcnt);
    if(duty == 0) {
        if(pchip2->rtk_polarity == 1)
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_HIGH);
        else
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_LOW);
        //duty = 1;
    }
    else{
        rtk_pwm_force_mode_w(pchip2,PWM_NON_FORCE);
    }
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.pwm0_dut = duty;
        pwm_duty_set_reg.pwm0_totalcnt = pchip2->rtk_totalcnt;
        M_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
    else{
        iso_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        I_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.mis_pwm0_dut = duty;
        I_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
}

int rtk_pwm_duty_r(R_CHIP_T *pchip2){
    int duty = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        duty = pwm_duty_set_reg.pwm0_dut;
    }
    else{
        iso_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        I_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        duty = pwm_duty_set_reg.mis_pwm0_dut;
    }
    return duty;
}

void rtk_pwm_totalcnt_w(R_CHIP_T *pchip2,int totalcnt){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.pwm0_totalcnt = totalcnt;
        M_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
    else{
        iso_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        I_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.mis_pwm0_totalcnt = totalcnt;
        I_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
}

int rtk_pwm_totalcnt_r(R_CHIP_T *pchip2){
    int totalcnt = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        totalcnt = pwm_duty_set_reg.pwm0_totalcnt;
    }
    else{
        iso_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        I_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        totalcnt = pwm_duty_set_reg.mis_pwm0_totalcnt;
    }
    return totalcnt;
}

void rtk_pwm_vsync_w(R_CHIP_T *pchip2,int vsync){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_dvs_rst_en = vsync;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.mis_pwm0_vs_rst_en = vsync;
        I_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
}

int rtk_pwm_dvs_mux_r(R_CHIP_T *pchip2){
    int vsync = 0;
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        vsync = pwm_ctrl_reg.pwm0_dvs_mux;
    }
    //ISO no this register
    return vsync;
}

void rtk_pwm_dvs_mux_w(R_CHIP_T *pchip2,int vsync){
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_dvs_mux = vsync;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    //ISO no this register
}

int rtk_pwm_vsync_r(R_CHIP_T *pchip2){
    int vsync = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        vsync = pwm_ctrl_reg.pwm0_dvs_rst_en;
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        vsync = pwm_ctrl_reg.mis_pwm0_vs_rst_en;
    }
    return vsync;
}

void rtk_pwm_output_w(R_CHIP_T *pchip2,int output){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_en = output;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.mis_pwm0_en = output;
        I_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
}

int rtk_pwm_output_r(R_CHIP_T *pchip2){
    int output = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        output = pwm_ctrl_reg.pwm0_en;
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        output = pwm_ctrl_reg.mis_pwm0_en;
    }
    return  output;
}

void rtk_pwm_vs_delay_w(R_CHIP_T *pchip2,int delay){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_vs_delay_thr = delay;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_vs_delay_thr = delay;
        I_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
}

unsigned int rtk_pwm_vs_delay_r(R_CHIP_T *pchip2){
    unsigned int vdelay = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        vdelay = pwm_ctrl_reg.pwm0_vs_delay_thr;
    }
    else{
        iso_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
        I_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        vdelay = pwm_ctrl_reg.pwm0_vs_delay_thr;
    }
    return vdelay;
}

void rtk_pwm_vsync_interval_w(R_CHIP_T *pchip2,int num){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        M_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        pwm_timing_ctrl_reg.pwm0_vsync_interval_num = num;
        M_PWM_TIMING_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
    }
    else{
        iso_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        I_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        pwm_timing_ctrl_reg.pwm0_vsync_interval_num = num;
        I_PWM_TIMING_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
    }
}

int rtk_pwm_vsync_interval_r(R_CHIP_T *pchip2){
    int interval = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        M_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        interval = pwm_timing_ctrl_reg.pwm0_vsync_interval_num;
    }
    else{
        iso_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};
        I_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        interval = pwm_timing_ctrl_reg.pwm0_vsync_interval_num;
    }
    return  interval;
}

void rtk_pwm_cycle_max_w(R_CHIP_T *pchip2,int cycle_max){
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.pwm0_cycle_max = cycle_max;
        M_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
    else{
        iso_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        I_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        pwm_duty_set_reg.mis_pwm0_cycle_max = cycle_max;
        I_PWM_DUTY_SET_REG_W(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
    }
}

int rtk_pwm_cycle_max_r(R_CHIP_T *pchip2){
    int cycle_max = 0;
    if(PWM_MISC_TYPE(pchip2)){
        misc_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        M_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        cycle_max = pwm_duty_set_reg.pwm0_cycle_max;
    }
    else{
        iso_pwm_duty_set_RBUS pwm_duty_set_reg = {0};
        I_PWM_DUTY_SET_REG_R(RTK_PWM_INDEX(pchip2),&pwm_duty_set_reg);
        cycle_max = pwm_duty_set_reg.mis_pwm0_cycle_max;
    }
    return cycle_max;
}

void rtk_pwm_force_mode_w(R_CHIP_T *pchip2,int force_mode){
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        pwm_ctrl_reg.pwm0_mode = force_mode;
        M_PWM_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
    }
    //ISO PWM No Force mode
}

int rtk_pwm_force_mode_r(R_CHIP_T *pchip2){
    int force_mode = 0;
    misc_pwm_ctrl_RBUS pwm_ctrl_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_ctrl_reg);
        force_mode = pwm_ctrl_reg.pwm0_mode;
    }
    //ISO PWM No Force mode
    return force_mode;
}

void rtk_pwm_duty_ratio_w(R_CHIP_T *pchip2,unsigned long ratio){
    misc_pwm_timing_ctrl_RBUS pwm_timing_ctrl_reg = {0};

    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TIMING_CTRL_REG_R(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
        pwm_timing_ctrl_reg.pwm0_duty_ratio = ratio;
        M_PWM_TIMING_CTRL_REG_W(RTK_PWM_INDEX(pchip2),&pwm_timing_ctrl_reg);
    }
    //ISO no this
}

//======================================================================
//Vsync Tracking & Predict Register Function
//======================================================================
//Register::PWM0_DVS_PERIOD
void rtk_pwm_dvs_max_period_w(R_CHIP_T *pchip2,unsigned long dvs_max_period){
    misc_pwm_mis_dvs_period_RBUS pwm_dvs_period_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DVS_PERIOD_REG_R(RTK_PWM_INDEX(pchip2),&pwm_dvs_period_reg);
        pwm_dvs_period_reg.pwm0_vs_max_period = dvs_max_period;
        M_PWM_DVS_PERIOD_REG_W(RTK_PWM_INDEX(pchip2),&pwm_dvs_period_reg);
    }
}

unsigned long rtk_pwm_dvs_max_period_r(R_CHIP_T *pchip2){
    unsigned long dvs_max_period = 0;
    misc_pwm_mis_dvs_period_RBUS pwm_dvs_period_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DVS_PERIOD_REG_R(RTK_PWM_INDEX(pchip2),&pwm_dvs_period_reg);
        dvs_max_period = pwm_dvs_period_reg.pwm0_vs_max_period;
    }
    return dvs_max_period;
}

void rtk_pwm_dvs_min_period_w(R_CHIP_T *pchip2,unsigned long dvs_min_period){
    misc_pwm_mis_dvs_period_RBUS pwm_dvs_period_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DVS_PERIOD_REG_R(RTK_PWM_INDEX(pchip2),&pwm_dvs_period_reg);
        pwm_dvs_period_reg.pwm0_vs_min_period = dvs_min_period;
        M_PWM_DVS_PERIOD_REG_W(RTK_PWM_INDEX(pchip2),&pwm_dvs_period_reg);
    }
}

unsigned long rtk_pwm_dvs_min_period_r(R_CHIP_T *pchip2){
    unsigned long dvs_min_period = 0;
    misc_pwm_mis_dvs_period_RBUS pwm_dvs_period_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DVS_PERIOD_REG_R(RTK_PWM_INDEX(pchip2),&pwm_dvs_period_reg);
        dvs_min_period = pwm_dvs_period_reg.pwm0_vs_min_period;
    }
    return dvs_min_period;
}

//Register::PWM0_DVS_MONITOR (ReadOnly)
unsigned long rtk_pwm_dvs_monitor_period_r(R_CHIP_T *pchip2){
    unsigned long pwm_dvs_period = 0;
    misc_pwm_mis_dvs_monitor_RBUS pwm_dvs_monitor_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DVS_MONITOR_REG_R(RTK_PWM_INDEX(pchip2),&pwm_dvs_monitor_reg);
        pwm_dvs_period = pwm_dvs_monitor_reg.pwm0_vs_period;
    }
    return pwm_dvs_period;
}

unsigned long rtk_pwm_dvs_monitor_realtime_r(R_CHIP_T *pchip2){
    unsigned long pwm_dvs_realtime = 0;
    misc_pwm_mis_dvs_monitor_RBUS pwm_dvs_monitor_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_DVS_MONITOR_REG_R(RTK_PWM_INDEX(pchip2),&pwm_dvs_monitor_reg);
        pwm_dvs_realtime = pwm_dvs_monitor_reg.pwm0_vs_real_time;
    }
    return pwm_dvs_realtime;
}

//Register::PWM0_MONITOR(ReadOnly)
int rtk_pwm_monitor_interrupt_r(R_CHIP_T *pchip2){
    int pwm_dvs_interrupt = 0;
    misc_pwm_mis_monitor_RBUS pwm_monitor_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_MONITOR_REG_R(RTK_PWM_INDEX(pchip2),&pwm_monitor_reg);
        pwm_dvs_interrupt = pwm_monitor_reg.pwm0_int;
    }
    return pwm_dvs_interrupt;
}

unsigned long rtk_pwm_monitor_real_totalcnt_r(R_CHIP_T *pchip2){
    unsigned long pwm_real_totalcnt = 0;
    misc_pwm_mis_monitor_RBUS pwm_monitor_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_MONITOR_REG_R(RTK_PWM_INDEX(pchip2),&pwm_monitor_reg);
        pwm_real_totalcnt = pwm_monitor_reg.pwm0_real_totalcnt;
    }
    return pwm_real_totalcnt;
}

unsigned long rtk_pwm_monitor_real_duty_r(R_CHIP_T *pchip2){
    unsigned long pwm_real_duty = 0;
    misc_pwm_mis_monitor_RBUS pwm_monitor_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_MONITOR_REG_R(RTK_PWM_INDEX(pchip2),&pwm_monitor_reg);
        pwm_real_duty = pwm_monitor_reg.pwm0_real_duty;
    }
    return pwm_real_duty;
}

//Register::PWM0_ERR_STATUS
int rtk_pwm_err2_int_r(R_CHIP_T *pchip2){
    misc_pwm_mis_err_status_RBUS pwm_mis_err_status_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_ERR_STATUS_R(RTK_PWM_INDEX(pchip2),&pwm_mis_err_status_reg);
    }
    return pwm_mis_err_status_reg.pwm0_err2int_en;
}

int rtk_pwm_unstable_status_r(R_CHIP_T *pchip2){
    misc_pwm_mis_err_status_RBUS pwm_mis_err_status_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_ERR_STATUS_R(RTK_PWM_INDEX(pchip2),&pwm_mis_err_status_reg);
    }
    return pwm_mis_err_status_reg.pwm0_vs_unstable;
}


void rtk_pwm_track_set_track_en_w(R_CHIP_T *pchip2,int en){
    misc_pwm_pwm0_track_set_RBUS pwm_track_set_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_SET_R(RTK_PWM_INDEX(pchip2),&pwm_track_set_reg);
        pwm_track_set_reg.pwm0_track_en = en;
        M_PWM_TRACK_SET_W(RTK_PWM_INDEX(pchip2),&pwm_track_set_reg);
    }
}


#if !defined(CONFIG_ARCH_RTK2851A) && !defined(CONFIG_ARCH_RTK2851C) && !defined(CONFIG_ARCH_RTK2851F)
//Register::PWM0_TRACK_SET
void rtk_pwm_track_set_fixed_phase_en_w(R_CHIP_T *pchip2,int en){
    misc_pwm_pwm0_track_set_RBUS pwm_track_set_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_SET_R(RTK_PWM_INDEX(pchip2),&pwm_track_set_reg);
        pwm_track_set_reg.pwm0_fixed_phase_en = en;
        M_PWM_TRACK_SET_W(RTK_PWM_INDEX(pchip2),&pwm_track_set_reg);
    }
}

//Register::PWM0_TRACK_SET2
void rtk_pwm_track_set2_phase_offset_w(R_CHIP_T *pchip2,unsigned int phase_offset){
    misc_pwm_pwm0_track_set2_RBUS pwm_track_set2_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_SET2_R(RTK_PWM_INDEX(pchip2),&pwm_track_set2_reg);
        pwm_track_set2_reg.pwm0_phase_offset_set = phase_offset;
        M_PWM_TRACK_SET2_W(RTK_PWM_INDEX(pchip2),&pwm_track_set2_reg);
    }
}

void rtk_pwm_track_set2_dvs_range_w(R_CHIP_T *pchip2,unsigned int dvs_range){
    misc_pwm_pwm0_track_set2_RBUS pwm_track_set2_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_SET2_R(RTK_PWM_INDEX(pchip2),&pwm_track_set2_reg);
        pwm_track_set2_reg.pwm0_dvs_range_set = dvs_range;
        M_PWM_TRACK_SET2_W(RTK_PWM_INDEX(pchip2),&pwm_track_set2_reg);
    }
}

//Register::PWM0_TRACK_STEP
void rtk_pwm_track_step_max_step_w(R_CHIP_T *pchip2,unsigned int max_step){
    misc_pwm_track_step_RBUS pwm_track_step_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_STEP_R(RTK_PWM_INDEX(pchip2),&pwm_track_step_reg);
        pwm_track_step_reg.pwm0_max_step = max_step;
        M_PWM_TRACK_STEP_W(RTK_PWM_INDEX(pchip2),&pwm_track_step_reg);
    }
}

//Register::PWM0_TRACK_STATUS
int rtk_pwm_track_status_track_freq_r(R_CHIP_T *pchip2){
    misc_pwm_track_status_RBUS pwm_track_status_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_STATUS_R(RTK_PWM_INDEX(pchip2),&pwm_track_status_reg);
    }
    return pwm_track_status_reg.pwm0_track_freq;
}

int rtk_pwm_track_status_phase_offset_r(R_CHIP_T *pchip2){
    misc_pwm_track_status_RBUS pwm_track_status_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_STATUS_R(RTK_PWM_INDEX(pchip2),&pwm_track_status_reg);
    }
    return pwm_track_status_reg.pwm0_phase_offset;
}

int rtk_pwm_track_status_pwm_full_r(R_CHIP_T *pchip2){
    misc_pwm_track_status_RBUS pwm_track_status_reg = {0};
    if(PWM_MISC_TYPE(pchip2)){
        M_PWM_TRACK_STATUS_R(RTK_PWM_INDEX(pchip2),&pwm_track_status_reg);
    }
    return pwm_track_status_reg.pwm0_full;
}
#else
//Register::PWM0_TRACK_SET
void rtk_pwm_track_set_fixed_phase_en_w(R_CHIP_T *pchip2,int en){
}

//Register::PWM0_TRACK_SET2
void rtk_pwm_track_set2_phase_offset_w(R_CHIP_T *pchip2,unsigned int phase_offset){
}

void rtk_pwm_track_set2_dvs_range_w(R_CHIP_T *pchip2,unsigned int dvs_range){
}

//Register::PWM0_TRACK_STEP
void rtk_pwm_track_step_max_step_w(R_CHIP_T *pchip2,unsigned int max_step){
}

#endif


void rtk_pwm_enable_tracking_mode(int index)
{
    R_CHIP_T *pchip2 = rtk_pwm_chip_get(index);

    rtk_pwm_duty_ratio_w(pchip2,0x100);
    rtk_pwm_num_w(pchip2,0x2);
    rtk_pwm_dvs_max_period_w(pchip2,0xFFFF);
    rtk_pwm_track_set2_dvs_range_w(pchip2,0x10);
    rtk_pwm_track_step_max_step_w(pchip2,0x10);
    rtk_pwm_track_set_fixed_phase_en_w(pchip2,1);
    rtk_pwm_track_set_fixed_phase_en_w(pchip2,1);
}

void rtk_pwm_disable_tracking_mode(int index)
{
    R_CHIP_T *pchip2 = rtk_pwm_chip_get(index);

    rtk_pwm_track_set_fixed_phase_en_w(pchip2,0);
    rtk_pwm_track_set_fixed_phase_en_w(pchip2,0);
}

void pwm_duty_ratio_set(int index, unsigned long ratio){
    int i = 0;
    unsigned int duty_ratio_flag = 0;
    unsigned long ratio_check = 0;
    unsigned long duty_ratio_array = 1000000;
    R_CHIP_T *pchip2 = rtk_pwm_chip_get(index);
    if( ratio == duty_ratio_array)
    {
        rtk_pwm_duty_ratio_w(pchip2,DUTY_RATIO_BIT9);
    }
    else
    {
        ratio_check = ratio;
        for(i = 8; i> 0; i--)
        {
            duty_ratio_array = (duty_ratio_array >> 1);
            if( ratio_check >= duty_ratio_array)
            {
                ratio_check = ratio_check - duty_ratio_array;
                duty_ratio_flag |= (1 << i );
            }
        }
        rtk_pwm_duty_ratio_w(pchip2,duty_ratio_flag);
    }
}

void rtk_pwm_vsync_thr_get(R_CHIP_T *pchip2,int M, int N,unsigned long *thr_max,unsigned long *thr_min){
    unsigned long vsync_thr = 0;
    vsync_thr = (pchip2->ht+1)*(pchip2->vt+1)/(4*(1<<M)*(N+1));
    (*thr_max) = vsync_thr+20;
    (*thr_min) = vsync_thr-20;
}

int pwm_vsync_disable(int index){
    R_CHIP_T *pchip2 = NULL;
    pchip2 = rtk_pwm_chip_get(index);
    if(rtk_pwm_panel_mode_get() == PWM_DRIVING_LED_CURRENT && pchip2->rtk_freq == 200){
        return 0;
    }
    pchip2->rtk_vsync = PWM_VSYNC_DISABLE;
    rtk_pwm_vsync_w(pchip2,PWM_VSYNC_DISABLE);
    rtk_pwm_db_wb(pchip2);
    rtk_pwm_db1_wb(pchip2);
    return 0;
}

//======================================================================
//Parameter Function
//======================================================================
int rtk_pwm_polarity_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    if((param->val != 0) && (param->val != 1)){
        PWM_ERR("SET %s  POLARITY INVALID param_val:%d\n",pchip2->nodename,param->val);
        return (-PWM_POLARITY_INVALID);
    }

    pchip2->rtk_polarity = param->val;
    rtk_pwm_ctrl_polarity_w(pchip2,pchip2->rtk_polarity);

    if( m_ioctl_printk_get() > 0 )
        PWM_ERR("SET %s POLARITY param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_polarity_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_polarity = rtk_pwm_ctrl_polarity_r(pchip2);
    param->val = pchip2->rtk_polarity;

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("GET %s POLARITY param_val:%d\n",pchip2->nodename,param->val);

}

int rtk_pwm_frequency_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    if((param->val < 4) || (param->val > MAX_PWM_FREQ)){
        PWM_ERR("SET %s  FREQEUNCY INVALID param_val:%d\n",pchip2->nodename,param->val);
        return (-PWM_FREQEUNCY_INVALID);
    }

    pchip2->rtk_freq = param->val;
    pchip2->rtk_freq_100times = pchip2->rtk_freq * 100;
    rtk_pwm_freq_w(pchip2);
    rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);//If frequency change, duty need re-calculate
    rtk_pwm_totalcnt_w(pchip2,pchip2->rtk_totalcnt);

    if( m_ioctl_printk_get() > 0 )
        PWM_ERR("SET %s FREQUENCY param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_frequency_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    param->val = pchip2->rtk_freq;

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("GET %s FREQUENCY param_val:%d\n",pchip2->nodename,param->val);
}

int rtk_pwm_duty_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    if(param->val > (pchip2->rtk_duty_max + 1)){
        PWM_ERR("SET %s  DUTY INVALID param_val:%d\n",pchip2->nodename,param->val);
        return (-PWM_DUTY_INVALID);
    }

#ifndef BUILD_QUICK_SHOW
    if (is_QS_panel_enable()) {
        PWM_INFO("%s: in quick-show, skip set duty to %u\n", __func__, param->val);
        return 0;
    }
#endif

    pchip2->rtk_duty = param->val;
    if(pchip2->rtk_duty == 0 && PWM_MISC_TYPE(pchip2)){
        if(pchip2->rtk_polarity == 1)
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_HIGH);
        else
            rtk_pwm_force_mode_w(pchip2,PWM_FORCE_LOW);
    }
    else{
        rtk_pwm_force_mode_w(pchip2,PWM_NON_FORCE);
        rtk_pwm_duty_w(pchip2,pchip2->rtk_duty);
    }
    if( m_ioctl_printk_get() > 0 )
        PWM_ERR("SET %s DUTY param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_duty_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    int duty = rtk_pwm_duty_r(pchip2);
    int duty_max = pchip2->rtk_duty_max;
    int totalcnt = rtk_pwm_totalcnt_r(pchip2);
    pchip2->rtk_duty = rtk_pwm_duty_get(duty,duty_max,totalcnt);

    param->val = pchip2->rtk_duty;

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("GET %s DUTY param_val:%d\n",pchip2->nodename,param->val);
}

int rtk_pwm_totalcnt_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_totalcnt = param->val;
    rtk_pwm_totalcnt_w(pchip2,pchip2->rtk_totalcnt);
    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("SET %s TOTALCNT param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_totalcnt_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_totalcnt = rtk_pwm_totalcnt_r(pchip2);
    param->val = pchip2->rtk_totalcnt;

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("GET %s TOTALCNT param_val:%d\n",pchip2->nodename,param->val);
}

int rtk_pwm_vsync_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    if((param->val != 0) && (param->val != 1)){
        PWM_ERR("SET %s VSYNC INVALID param_val:%d\n",pchip2->nodename,param->val);
        return (-PWM_VSYNC_INVALID);
    }
    pchip2->rtk_vsync = param->val;
    rtk_pwm_vsync_w(pchip2,pchip2->rtk_vsync);
    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("SET %s VSYNC param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_vsync_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_vsync = rtk_pwm_vsync_r(pchip2);
    param->val = pchip2->rtk_vsync;

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("GET %s VSYNC param_val:%d\n",pchip2->nodename,param->val);
}

int rtk_pwm_output_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    if((param->val != 0) && (param->val != 1)){
        PWM_ERR("SET %s OUTPUT INVALID param_val:%d\n",pchip2->nodename,param->val);
        return (-PWM_OUTPUT_INVALID);
    }

    pchip2->rtk_enable = param->val;
    rtk_pwm_output_w(pchip2,pchip2->rtk_enable);

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("SET %s OUTPUT param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_output_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_enable = rtk_pwm_output_r(pchip2);
    param->val = pchip2->rtk_enable;

    if( m_ioctl_printk_get() > 0 )
        PWM_WARN("GET %s OUTPUT param_val:%d\n",pchip2->nodename,param->val);
}

int rtk_pwm_vs_delay_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    if(param->val > pchip2->rtk_totalcnt || pchip2->rtk_freq == 0){
        PWM_ERR("SET %s  VSYNC_DELAY INVALID param_val:%d\n",pchip2->nodename,param->val);
        return (-PWM_VSYNC_INVALID);
    }

    pchip2->rtk_pos_start = param->val;
    get_v_delay(pchip2);
    rtk_pwm_vs_delay_w(pchip2,pchip2->rtk_pos_start_clk);
    if( m_ioctl_printk_get() > 0 )
        PWM_ERR("SET %s VSYNC_DELAY param_val:%d\n",pchip2->nodename,param->val);

    return 0;
}

void rtk_pwm_vs_delay_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    //pchip2->rtk_pos_start_clk = rtk_pwm_vs_delay_r(pchip2);
    param->val = pchip2->rtk_pos_start;

    if( m_ioctl_printk_get() > 0 )
        PWM_ERR("GET %s VSYNC_DELAY param_val:%d\n",pchip2->nodename,param->val);
}

void rtk_pwm_vs_ineterval_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_vsync_interval = param->val;
    rtk_pwm_vsync_interval_w(pchip2,pchip2->rtk_vsync_interval);
}

void rtk_pwm_vs_ineterval_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->rtk_vsync_interval = rtk_pwm_vsync_interval_r(pchip2);
    param->val = pchip2->rtk_vsync_interval;

}

void rtk_pwm_cycle_max_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->cycle_max = param->val;
    rtk_pwm_cycle_max_w(pchip2,pchip2->cycle_max);
}

void rtk_pwm_cycle_max_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->cycle_max = rtk_pwm_cycle_max_r(pchip2);
    param->val = pchip2->cycle_max;
}

void rtk_pwm_force_mode_param_set(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->force_mode = param->val;
    rtk_pwm_force_mode_w(pchip2,pchip2->force_mode);

}

void rtk_pwm_force_mode_param_get(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    pchip2->force_mode = rtk_pwm_force_mode_r(pchip2);
    param->val = pchip2->force_mode;

}

int rtk_pwm_set_param_ex(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    int ret = PWM_SUCCESS;

    switch (param->id) {
        case PWM_PARAM_POLARITY:
            rtk_pwm_polarity_param_set(pchip2,param);
            break;
        case PWM_PARAM_FREQUENCY:
            rtk_pwm_frequency_param_set(pchip2,param);
            break;
        case PWM_PARAM_DUTY:
            rtk_pwm_duty_param_set(pchip2,param);
            break;
        case PWM_PARAM_VSYNC_RESET:
            rtk_pwm_vsync_param_set(pchip2,param);
            break;
        case PWM_PARAM_OUTPUT:
            rtk_pwm_output_param_set(pchip2,param);
            break;
        case PWM_PARAM_TOTALCNT:
            rtk_pwm_totalcnt_param_set(pchip2,param);
            break;
        case PWM_PARAM_VSYNC_DELAY:
            rtk_pwm_vs_delay_param_set(pchip2,param);
            break;
        case PWM_PARAM_VSYNC_INTERVAL:
            rtk_pwm_vs_ineterval_param_set(pchip2,param);
            break;
        case PWM_PARAM_CYCLE_MAX:
            rtk_pwm_cycle_max_param_set(pchip2,param);
            break;
        case PWM_PARAM_FORCE_MODE:
            rtk_pwm_force_mode_param_set(pchip2,param);
            break;
        default:
            ret = PWM_FAIL;
    }

    //DB Write
    rtk_pwm_db_wb(pchip2);

    return ret;
}

int rtk_pwm_get_param_ex(R_CHIP_T *pchip2,RTK_PWM_PARAM_EX_T* param)
{
    int ret = PWM_SUCCESS;

    switch (param->id) {
        case PWM_PARAM_POLARITY:
            rtk_pwm_polarity_param_get(pchip2,param);
            break;
        case PWM_PARAM_FREQUENCY:
            rtk_pwm_frequency_param_get(pchip2,param);
            break;
        case PWM_PARAM_DUTY:
            rtk_pwm_duty_param_get(pchip2,param);
            break;
        case PWM_PARAM_VSYNC_RESET:
            rtk_pwm_vsync_param_get(pchip2,param);
            break;
        case PWM_PARAM_OUTPUT:
            rtk_pwm_output_param_get(pchip2,param);
            break;
        case PWM_PARAM_TOTALCNT:
            rtk_pwm_totalcnt_param_get(pchip2,param);
            break;
        case PWM_PARAM_VSYNC_DELAY:
            rtk_pwm_vs_delay_param_get(pchip2,param);
            break;
        case PWM_PARAM_VSYNC_INTERVAL:
            rtk_pwm_vs_ineterval_param_get(pchip2,param);
            break;
        case PWM_PARAM_CYCLE_MAX:
            rtk_pwm_cycle_max_param_get(pchip2,param);
            break;
        case PWM_PARAM_FORCE_MODE:
            rtk_pwm_force_mode_param_get(pchip2,param);
	    break;
        default:
            ret = PWM_FAIL;
    }

    //DB Write
    rtk_pwm_db_wb(pchip2);

    return ret;
}

int rtk_pwm_set_all_param_ex(R_CHIP_T *pchip2,RTK_PWM_INFO_EX_T* param_info)
{
    int ret = PWM_SUCCESS;
    RTK_PWM_PARAM_EX_T param_all;
    strncpy(param_all.pcbname, param_info->pcbname, sizeof(param_info->pcbname)-1);
    //Set Polarity
    if((param_info->m_polarity != 0) && (param_info->m_polarity != 1)){
        PWM_ERR("SET %s  POLARITY INVALID param_val:%d\n",pchip2->nodename,param_info->m_polarity);
        return (-PWM_POLARITY_INVALID);
    }
    param_all.id = PWM_PARAM_POLARITY;
    param_all.val = param_info->m_polarity;
    //rtk_pwm_polarity_param_set(pchip2,&param_all);
    rtk_pwm_set_param_ex(pchip2,&param_all);

    //Set Frequency
    if((param_info->m_freq < 1) || (param_info->m_freq > MAX_PWM_FREQ)){
        PWM_ERR("SET %s FREQEUNCY INVALID param_val:%d\n",pchip2->nodename,param_info->m_freq);
        return (-PWM_FREQEUNCY_INVALID);
    }
    param_all.id = PWM_PARAM_FREQUENCY;
    param_all.val = param_info->m_freq;
    //rtk_pwm_frequency_param_set(pchip2,&param_all);
    rtk_pwm_set_param_ex(pchip2,&param_all);

    //Set Duty
    if(param_info->m_duty > pchip2->rtk_duty_max){
        PWM_ERR("SET %s  DUTY INVALID param_val:%d\n",pchip2->nodename,param_info->m_duty);
        return (-PWM_DUTY_INVALID);
    }
    param_all.id = PWM_PARAM_DUTY;
    param_all.val = param_info->m_duty;
    //rtk_pwm_duty_param_set(pchip2,&param_all);
    rtk_pwm_set_param_ex(pchip2,&param_all);

    //Set Vsync
    if((param_info->m_vsync != 0) && (param_info->m_vsync != 1)){
        PWM_ERR("SET %s  VSYNC INVALID param_val:%d\n",pchip2->nodename,param_info->m_vsync);
        return (-PWM_VSYNC_INVALID);
    }
    param_all.id = PWM_PARAM_VSYNC_RESET;
    param_all.val = param_info->m_vsync;
    //rtk_pwm_vsync_param_set(pchip2,&param_all);
    rtk_pwm_set_param_ex(pchip2,&param_all);

    //Set Output
    if((param_info->m_run != 0) && (param_info->m_run != 1)){
        PWM_ERR("SET %s  OUTPUT INVALID param_val:%d\n",pchip2->nodename,param_info->m_run);
        return (-PWM_OUTPUT_INVALID);
    }
    param_all.id = PWM_PARAM_OUTPUT;
    param_all.val = param_info->m_run;
    //rtk_pwm_output_param_set(pchip2,&param_all);
    rtk_pwm_set_param_ex(pchip2,&param_all);

    //DB Write
    rtk_pwm_db_wb(pchip2);

    return ret;
}

int rtk_pwm_get_all_param_ex(R_CHIP_T *pchip2,RTK_PWM_INFO_EX_T* param_info)
{
    int ret = PWM_SUCCESS;
    RTK_PWM_PARAM_EX_T param_all;
    strncpy(param_all.pcbname, param_info->pcbname,sizeof(param_info->pcbname)-1);
    param_all.id = PWM_PARAM_POLARITY;
    rtk_pwm_polarity_param_get(pchip2,&param_all);
    param_info->m_polarity = param_all.val;

    param_all.id = PWM_PARAM_FREQUENCY;
    rtk_pwm_frequency_param_get(pchip2,&param_all);
    param_info->m_freq = param_all.val;

    param_all.id = PWM_PARAM_DUTY;
    rtk_pwm_duty_param_get(pchip2,&param_all);
    param_info->m_duty = param_all.val;

    param_all.id = PWM_PARAM_VSYNC_RESET;
    rtk_pwm_vsync_param_get(pchip2,&param_all);
    param_info->m_vsync = param_all.val;

    param_all.id = PWM_PARAM_OUTPUT;
    rtk_pwm_output_param_get(pchip2,&param_all);
    param_info->m_run = param_all.val;

    return ret;
}

int rtk_pwm_set_info(RTK_PWM_INFO_T* stpwminfo)
{
    unsigned long flags;
    int ret = PWM_SUCCESS;
    R_CHIP_T *pchip2 = NULL;
    RTK_PWM_PARAM_EX_T param;
    LOCK_RTK_PWM(&pwm_lock, flags);
    pchip2 = rtk_pwm_chip_get(stpwminfo->m_index);
    strncpy(param.pcbname, pchip2->pcbname, sizeof(pchip2->pcbname)-1);

    /*rtk_adapt_freq  base is 0.01hz*/
    pchip2->rtk_adapt_freq = rtk_pwm_get_adapt_freq(pchip2);
    rtk_pwm_user_setting(pchip2,stpwminfo);

    if(pchip2->rtk_adapt_freq != 0 && pchip2->rtk_freq != 0)
        get_v_delay(pchip2);

    /*set timing ctrl , freq */
    param.id = PWM_PARAM_FREQUENCY;
    param.val = pchip2->rtk_freq;
    rtk_pwm_set_param_ex(pchip2,&param);

    param.id = PWM_PARAM_VSYNC_INTERVAL;
    param.val = pchip2->rtk_vsync_interval;
    rtk_pwm_set_param_ex(pchip2,&param);

    /*set duty */
    param.id = PWM_PARAM_DUTY;
    param.val = pchip2->rtk_duty;
    rtk_pwm_set_param_ex(pchip2,&param);

    /*param.id = PWM_PARAM_TOTALCNT;
    param.val = pchip2->rtk_totalcnt;
    rtk_pwm_set_param_ex(pchip2,&param);*/

    /*set CTRL */
    param.id = PWM_PARAM_POLARITY;
    param.val = pchip2->rtk_polarity;
    rtk_pwm_set_param_ex(pchip2,&param);

    param.id = PWM_PARAM_OUTPUT;
    param.val = pchip2->rtk_enable;
    rtk_pwm_set_param_ex(pchip2,&param);

    param.id = PWM_PARAM_VSYNC_RESET;
    param.val = pchip2->rtk_vsync;
    rtk_pwm_set_param_ex(pchip2,&param);

    /*vsync delay*/
    param.id = PWM_PARAM_VSYNC_DELAY;
    param.val = pchip2->rtk_pos_start_clk;
    rtk_pwm_set_param_ex(pchip2,&param);

    /*set DB */
    rtk_pwm_db_wb(pchip2);

    UNLOCK_RTK_PWM(&pwm_lock, flags);
    if( m_ioctl_printk_get() > 0 ){
        PWM_WARN("%s set pwm[%d], duty=%d freq=%d lock=%d (pos=%d,%08x) adp = %d  adap_freq(base 0.01Hz)=%d, tempFreq = %d \n",
        __func__ ,
        stpwminfo->m_index,
        pchip2->rtk_duty,
        pchip2->rtk_freq,
        pchip2->rtk_vsync,
        pchip2->rtk_pos_start,
        pchip2->rtk_pos_start_clk,
        pchip2->rtk_adpt_en,
        pchip2->rtk_adapt_freq,
        pchip2->rtk_freq_100times);
    }
    return ret;

}
