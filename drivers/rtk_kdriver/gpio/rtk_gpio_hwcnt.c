#include <rtd_log/rtd_module_log.h>
#include <rtk_kdriver/rtk_gpio_hwcnt.h>
#include "rtk_gpio-reg.h"

static unsigned char gpio_count = 0;
RTK_GPIO_ID panel_error_detect_pin = 0xffffffff;

/**************************************************
** RTK_GPIO_DEBOUNCE set gpio hwcnt debounce
**************************************************/
void rtk_hwcnt_set_debounce(RTK_GPIO_DEBOUNCE val)
{
     gpio_iowrite32(ISO_GPIO_GPDEB_HWCNT, ((0x8 | val) << ISO_GPIO_GPDEB_clk5_SHIFT_HWCNT));
}
/**************************************************
** set HWCNT IRQ enable/disable && HWCNT START
**************************************************/
void rtk_hwcnt_set_irq_enable(unsigned char On_off)
{
    iowrite_reg_bit(ISO_GPIO_GPIE_HWCNT ,ISO_GPIO_GPIE_HWCNT_SGIFT_HWCNT, On_off);
}

/**************************************************
** set HWCNT IRQ active with HIGH/LOW
**************************************************/
void rtk_hwcnt_set_irq_polarity(RTK_GPIO_POLARITY val){
        iowrite_reg_bit(ISO_GPIO_GPDP_HWCNT , ISO_GPIO_GPDP_HWCNT_SHIFT_HWCNT, val);
}
/**************************************************
** set HWCNT IRQ  with ASSERT/DISASSERT/BOTH
**************************************************/
void rtk_hwcnt_set_irq_intsel(RTK_GPIO_GLOBAL_INTSEL val){
        gpio_iowrite32(ISO_GPIO_HWCNT_1_HWCNT , val << ISO_GPIO_HWCNT_1_HWCNT_EDGE_SHIFT_HWCNT);
}
/**************************************************
** set HWCNT GPIO func.
** irq enable
**************************************************/
int rtk_hwcnt_set_gpio_func( RTK_GPIO_IRQ_SET_T gpio_irq_set){
    RTK_GPIO_PCBINFO_T pinInfo;
    int ret = 0;

    ret = rtk_gpio_get_pcb_info(gpio_irq_set.pin_name,&pinInfo);

    if(ret != 0)
    {
        RTK_GPIO_WARNING("%s can't find the pcbname \n", __func__);
        return -1;
    }

    if(pinInfo.gpio_type == ISO_GPIO || pinInfo.gpio_type == MIS_GPIO) {
        ret =  rtk_gpio_set_dir(pinInfo.gid,GPIO_INPUT);
        if(ret != 0)
        {
            RTK_GPIO_WARNING("%s %s set dir failed \n", __func__,pinInfo.pin_name);
            return -1;
        }
    }

    rtk_hwcnt_set_irq_intsel(gpio_irq_set.edge_type);
    rtk_hwcnt_set_irq_polarity(gpio_irq_set.edge_type);

    rtk_hwcnt_set_irq_enable(gpio_irq_set.irq_en);

    gpio_iowrite32(ISO_GPIO_HWCNT_1_HWCNT , (gpio_ioread32(ISO_GPIO_HWCNT_1_HWCNT)&(~ISO_GPIO_HWCNT_1_HWCNT_MASK_HWCNT) ) | ((pinInfo.pin_index) << ISO_GPIO_HWCNT_1_HWCNT_I_SHIFT_HWCNT));

    return ret;
}

/**************************************************
** set HWCNT Enable and clear the count value
**************************************************/
int rtk_hwcnt_set_enable(unsigned char On_off){
    if(panel_error_detect_pin == 0xFFFFFFFF){
        RTK_GPIO_WARNING("NO define PIN_OLED_ERROR_DET\n");
        return 0;
    }
    if(gpio_group(panel_error_detect_pin) == MIS_GPIO)      //MISC gpio using gpio irq count
    {
        rtk_gpio_set_irq_enable(panel_error_detect_pin, On_off);
        return 0;
    }else{                                                 //ISO gpio using HWCNT func count
    if(On_off)
        gpio_iowrite32(ISO_GPIO_HWCNT_1_HWCNT ,gpio_ioread32(ISO_GPIO_HWCNT_1_HWCNT) | ISO_GPIO_HWCNT_1_SWEN_MASK_HWCNT );
    else
        gpio_iowrite32(ISO_GPIO_HWCNT_1_HWCNT ,gpio_ioread32(ISO_GPIO_HWCNT_1_HWCNT) & (~ISO_GPIO_HWCNT_1_SWEN_MASK_HWCNT)  );
    return 0;
    }
}
int rtk_hwcnt_get_enable(void){
    if(panel_error_detect_pin == 0xFFFFFFFF){
        RTK_GPIO_WARNING("NO define PIN_OLED_ERROR_DET\n");
        return 0;
    }
    if(gpio_group(panel_error_detect_pin) == MIS_GPIO)      //MISC gpio using gpio irq count
    {
        return rtk_gpio_chk_irq_enable(panel_error_detect_pin);
    }else                                                 //ISO gpio using HWCNT func count
        return gpio_ioread32((ISO_GPIO_HWCNT_1_HWCNT) & (~ISO_GPIO_HWCNT_1_SWEN_MASK_HWCNT));
}
/**************************************************
** set HWCNT Enable and clear the count value
**************************************************/
int rtk_hwcnt_get_counter(void){
    if(panel_error_detect_pin == 0xffffffff)
        return 0;

    return (gpio_group(panel_error_detect_pin) == MIS_GPIO)? gpio_count
            : (gpio_ioread32(ISO_GPIO_HWCNT_2_HWCNT) & ISO_GPIO_HWCNT_2_HW_ERROR_CNT_MASK_HWCNT);
}

/**************************************************
**  clear the count value
**************************************************/
void rtk_hwcnt_clear_count(void){
     if(panel_error_detect_pin == 0xffffffff)
         return;

     if(gpio_group(panel_error_detect_pin) == MIS_GPIO)      //MISC gpio using gpio irq count
    {
         gpio_count = 0;
     }else{
         rtk_hwcnt_set_enable(0);
         rtk_hwcnt_set_enable(1);
     }
}

void rtk_panel_error_detect_isr(
    RTK_GPIO_ID      gid,
    unsigned char    assert,
    void*            dev_id
){
    static unsigned long long assert_time = 0;
    static unsigned long long disassert_time = 0;
    if(assert)
    {
        assert_time = jiffies_to_msecs(jiffies);
        disassert_time = jiffies_to_msecs(jiffies);
    }
    else{
        disassert_time = jiffies_to_msecs(jiffies);
    }
    if((disassert_time-assert_time)> 100)
    gpio_count++;
    //rtd_outl(0xb806015c, count);
}

void rtk_panel_error_detect_init(void)
{
    int ret = 0, index = 0, type = 0;
    unsigned long long value = 0;
    ret = pcb_mgr_get_enum_info_byname("PIN_OLED_ERROR_DET", &value);
    if (ret || (0xFFFFFFFF&value)==0xFFFFFFFF) {
        //RTK_GPIO_WARNING("NO define PIN_OLED_ERROR_DET\n");
        rtd_pr_gpio_warn("NO define PIN_OLED_ERROR_DET\n");
        return;
    }

    if(GET_PIN_TYPE(value) == PCB_PIN_TYPE_GPIO){
        type = MIS_GPIO;
        index = GET_PIN_INDEX(value);
        panel_error_detect_pin = rtk_gpio_id(type, index);

        rtk_gpio_set_dir(panel_error_detect_pin, 0);
        rtk_gpio_set_debounce(panel_error_detect_pin, RTK_GPIO_DEBOUNCE_1ms);    /* 1 us */
        rtk_gpio_set_irq_polarity(panel_error_detect_pin, 1);
        rtk_gpio_request_irq(panel_error_detect_pin, rtk_panel_error_detect_isr,
                                    "rtk_pwm_panel_error", rtk_panel_error_detect_isr);

        rtk_gpio_set_irq_enable(panel_error_detect_pin, 1);
        if(ret < 0) {
            //RTK_GPIO_WARNING("panel_error_detect_pin isr register %s GPIO %d (%x) failed\n",
            RTK_GPIO_ERROR("GPIO panel_error_detect_pin isr register %s GPIO %d (%x) failed\n",
                gpio_type(gpio_group(panel_error_detect_pin)),
                gpio_idx(panel_error_detect_pin),
                panel_error_detect_pin);
        }
        else {
            //RTK_GPIO_WARNING("panel_error_detect_pin isr register %s GPIO %d (%x) ready\n",
            RTK_GPIO_ERROR("GPIO panel_error_detect_pin isr register %s GPIO %d (%x) ready\n",
                gpio_type(gpio_group(panel_error_detect_pin)),
                gpio_idx(panel_error_detect_pin),
                panel_error_detect_pin);
        }

    }
    else if(GET_PIN_TYPE(value) == PCB_PIN_TYPE_ISO_GPIO){
        RTK_GPIO_IRQ_SET_T hwcnt_irq = {     .pin_name  = "PIN_OLED_ERROR_DET",
                                             .irq_en       = 1,
                                             .debounce  = RTK_GPIO_DEBOUNCE_37ns,
                                             .edge_type = GPIO_BOTH_EDGE,
                                       };
        rtd_pr_gpio_info("GPIO panel_error_detect_pin hwcnt func\n");
        rtk_hwcnt_set_gpio_func(hwcnt_irq);
        rtk_hwcnt_set_enable(1);
    }

    return;
}
void rtk_panel_error_detect_exit(void)
{
    rtk_gpio_set_irq_enable(panel_error_detect_pin, 0);
    rtk_gpio_free_irq(panel_error_detect_pin,0);

}
