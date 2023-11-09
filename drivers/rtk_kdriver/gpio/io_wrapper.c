#ifndef BUILD_QUICK_SHOW
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#else
#include <no_os/export.h>
#endif

#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/pcbMgr.h>


int rtk_SetIOPinDirection(unsigned long long pininfo, unsigned int value)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_GPIO)
    PCB_PIN_TYPE_T pintype;
    RTK_GPIO_ID s_gpio;
    int pinindex = 0;
    int pinparam1 = 0, pinparam2 = 0, pinparam3 = 0;
    if (value != 0x0 && value != 0x1)
    {
        RTK_GPIO_WARNING("[%s] unsupported value = 0x%08x.\n",
            __func__ , value);
        return -1;
    }

    pintype = GET_PIN_TYPE(pininfo);
    pinindex = GET_PIN_INDEX(pininfo);
    RTK_IO_P("[%s] pin=0x%llx value=0x%x, type=%08x, index=%d\n",
        __func__ , (unsigned int)pininfo, value, pintype, pinindex);

    switch (pintype) {
    case PCB_PIN_TYPE_UNUSED:
        return 0;
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_EMCU_GPIO:
        RTK_GPIO_WARNING("[%s] Function isn't support(%x).\n",
            __func__ , (unsigned int)pininfo);
        return -1;
    case PCB_PIN_TYPE_GPIO:
        {
            pinparam1 = GET_GPIO_TYPE(pininfo);
            pinparam2 = GET_GPIO_INVERT(pininfo);
            pinparam3 = GET_GPIO_INIT_VALUE(pininfo);
            RTK_IO_P("[GPIO-MISC][%s] param1 0x%x param2 0x%x param3 0x%x\n",
                __func__ , pinparam1, pinparam2, pinparam3);
            if (pinparam1 == PCB_GPIO_TYPE_INPUT && value != 0)
            {
                RTK_GPIO_DBG("[GPIO-MISC][%s][PCB_PIN_TYPE_GPIO] Input-Only pin %d is configured to output.\n",
                    __func__ , pinindex);
            }
            else if (pinparam1 == PCB_GPIO_TYPE_OUPUT && value != 1)
            {
                RTK_GPIO_DBG("[GPIO-MISC][%s][PCB_PIN_TYPE_GPIO] Output-Only pin %d is configured config to input.\n",
                    __func__ , pinindex);
            }else if (pinparam1 >= PCB_GPIO_TYPE_UNDEF)
            {
                RTK_GPIO_WARNING ("[GPIO-MISC] wrong GPIO type\n");
                return -1;
            }
            s_gpio = rtk_gpio_id(MIS_GPIO, pinindex);
            rtk_gpio_set_dir(s_gpio, value);
            RTK_IO_P("[GPIO-MISC][%s] config pin %d to %d.\n",
                __func__ , pinindex, (int)value);
            return 0;
        }
    case PCB_PIN_TYPE_ISO_GPIO:
        {
            pinparam1 = GET_GPIO_TYPE(pininfo);
            pinparam2 = GET_GPIO_INVERT(pininfo);
            pinparam3 = GET_GPIO_INIT_VALUE(pininfo);
            RTK_IO_P("[GPIO-ISO][%s] param1 0x%x param2 0x%x param3 0x%x\n",
                __func__, pinparam1, pinparam2, pinparam3);
            if (pinparam1 == PCB_GPIO_TYPE_INPUT && value != 0)
            {
                RTK_GPIO_DBG("[GPIO-ISO][%s][PCB_PIN_TYPE_GPIO] Input-Only pin %d is configured to output.\n",
                    __func__ , pinindex);
            }
            else if (pinparam1 == PCB_GPIO_TYPE_OUPUT&& value != 1)
            {
                RTK_GPIO_DBG("[GPIO-ISO][%s][PCB_PIN_TYPE_GPIO] Output-Only pin %d is configured to input.\n",
                    __func__ , pinindex);
            }
            else if (pinparam1 >= PCB_GPIO_TYPE_UNDEF)
            {
                RTK_GPIO_WARNING("[GPIO-ISO] wrong GPIO type\n");
                return -1;
            }
            s_gpio = rtk_gpio_id(ISO_GPIO, pinindex);
            rtk_gpio_set_dir(s_gpio, value);
            RTK_IO_P("[GPIO-ISO][%s] config pin %d to %d.\n",
                __func__ , pinindex, (int)value);
            return 0;
        }
    case PCB_PIN_TYPE_MIO_GPIO:
        {
            pinparam1 = GET_GPIO_TYPE(pininfo);
            pinparam2 = GET_GPIO_INVERT(pininfo);
            pinparam3 = GET_GPIO_INIT_VALUE(pininfo);
            RTK_IO_P("[GPIO-MIO][%s] param1 0x%x param2 0x%x param3 0x%x\n",
                __func__ , pinparam1, pinparam2, pinparam3);
            if (pinparam1 == PCB_GPIO_TYPE_INPUT && value != 0)
            {
                RTK_GPIO_DBG("[GPIO-MIO][%s][PCB_PIN_TYPE_GPIO] Input-Only pin %d is configured to output.\n",
                    __func__ , pinindex);
            }
            else if (pinparam1 == PCB_GPIO_TYPE_OUPUT && value != 1)
            {
                RTK_GPIO_DBG("[GPIO-MIO][%s][PCB_PIN_TYPE_GPIO] Output-Only pin %d is configured config to input.\n",
                    __func__ , pinindex);
            }else if (pinparam1 >= PCB_GPIO_TYPE_UNDEF)
            {
                RTK_GPIO_WARNING ("[GPIO-MIO] wrong GPIO type\n");
                                return -1;
            }
            s_gpio = rtk_gpio_id(MIO_GPIO, pinindex); //!MIO: FIX ME
            rtk_gpio_set_dir(s_gpio, value);
            RTK_IO_P("[GPIO-MIO][%s] config pin %d to %d.\n",
                __func__ , pinindex, (int)value);
            return 0;
        }
    case PCB_PIN_TYPE_ISO_GPIO_MIO:
        {
            pinparam1 = GET_GPIO_TYPE(pininfo);
            pinparam2 = GET_GPIO_INVERT(pininfo);
            pinparam3 = GET_GPIO_INIT_VALUE(pininfo);
            RTK_IO_P("[ISO_GPIO-MIO][%s] param1 0x%x param2 0x%x param3 0x%x\n",
                __func__ , pinparam1, pinparam2, pinparam3);
            if (pinparam1 == PCB_GPIO_TYPE_INPUT && value != 0)
            {
                RTK_GPIO_DBG("[ISO_GPIO-MIO][%s][PCB_PIN_TYPE_GPIO] Input-Only pin %d is configured to output.\n",
                    __func__ , pinindex);
            }
            else if (pinparam1 == PCB_GPIO_TYPE_OUPUT && value != 1)
            {
                RTK_GPIO_DBG("[ISO_GPIO-MIO][%s][PCB_PIN_TYPE_GPIO] Output-Only pin %d is configured config to input.\n",
                    __func__ , pinindex);
            }else if (pinparam1 >= PCB_GPIO_TYPE_UNDEF)
            {
                RTK_GPIO_WARNING ("[ISO_GPIO-MIO] wrong GPIO type\n");
                                return -1;
            }
            s_gpio = rtk_gpio_id(ISO_GPIO_MIO, pinindex); //!ISO_GPIO_MIO: FIX ME
            rtk_gpio_set_dir(s_gpio, value);
            RTK_IO_P("[ISO_GPIO-MIO][%s] config pin %d to %d.\n",
                __func__ , pinindex, (int)value);
            return 0;
        }
    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_UNIPWM:
#if 0
        if (pintype == PCB_PIN_TYPE_UNIPWM)
            unipwm_set(pinindex, value);
        else
            iso_unipwm_set(pinindex, value);

        break;
#endif
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
#if 0
        {
            if (pintype == PCB_PIN_TYPE_PWM)
                pwm_set(pinindex, value);
            else
                iso_pwm_set(pinindex, value);

        }
#endif
        break;
    case PCB_PIN_TYPE_AUDIO:
        {
            RTK_GPIO_WARNING("[%s] AUDIO PIN unsupported.\n",__func__);
            break;
        }
    default:
            RTK_GPIO_WARNING("[%s] Error, unkown type.\n", __func__);
            return -1;
    }
#endif
    return -1;
}

EXPORT_SYMBOL(rtk_SetIOPinDirection);

int rtk_GetIOPinDirection(unsigned long long pininfo)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_GPIO)
    PCB_PIN_TYPE_T pintype;
    RTK_GPIO_ID s_gpio;
    int pinindex = 0;

    pintype = GET_PIN_TYPE(pininfo);
    pinindex = GET_PIN_INDEX(pininfo);

    switch (pintype) {
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_UNIPWM:
    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
    case PCB_PIN_TYPE_EMCU_GPIO:
    case PCB_PIN_TYPE_AUDIO:
        RTK_GPIO_WARNING("[%s] Function isn't support(%x).\n", __func__ , (unsigned int)pininfo);
        return -1;
    case PCB_PIN_TYPE_GPIO:
        s_gpio = rtk_gpio_id(MIS_GPIO, pinindex);
        return rtk_gpio_get_dir(s_gpio);
    case PCB_PIN_TYPE_ISO_GPIO:
        s_gpio = rtk_gpio_id(ISO_GPIO, pinindex);
        return rtk_gpio_get_dir(s_gpio);
    case PCB_PIN_TYPE_MIO_GPIO:
        s_gpio = rtk_gpio_id(MIO_GPIO, pinindex); //!MIO: FIX ME
        return rtk_gpio_get_dir(s_gpio);
    case PCB_PIN_TYPE_ISO_GPIO_MIO:
        s_gpio = rtk_gpio_id(ISO_GPIO_MIO, pinindex); //!ISO_GPIO_MIO: FIX ME
        return rtk_gpio_get_dir(s_gpio);
    case PCB_PIN_TYPE_UNUSED:
        return 0;
    default:
        RTK_GPIO_WARNING("[%s] Error, unkown type.\n", __func__);
        return -1;
    }
    return -1;
#else
    return -1;
#endif
}

EXPORT_SYMBOL(rtk_GetIOPinDirection);


int rtk_SetIOPin(unsigned long long pininfo, unsigned int value)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_GPIO)
    PCB_PIN_TYPE_T pintype;
    RTK_GPIO_ID s_gpio;
    int pinindex = 0;
    int ret = 0;

    //change origial flow
    //before STR,call this api changed flow,when setIO output,will set output value and set dir to output;

    pintype = GET_PIN_TYPE(pininfo);
    pinindex = GET_PIN_INDEX(pininfo);
    switch (pintype) {
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_UNIPWM:
    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
    case PCB_PIN_TYPE_EMCU_GPIO:
    case PCB_PIN_TYPE_AUDIO:
        RTK_GPIO_WARNING("[%s] Function isn't support(0x%X).\n",
                    __func__ , (unsigned int)pininfo);
        return -1;
    case PCB_PIN_TYPE_GPIO:
        s_gpio = rtk_gpio_id(MIS_GPIO, pinindex);
        ret = rtk_gpio_output(s_gpio, value);
        rtk_gpio_set_dir(s_gpio, 1);
        if (ret != 0)
        {
            RTK_GPIO_WARNING("[%s] set MIS GPIO %d error.\n",
                     __func__ , pinindex);
            return -1;
        }
        return 0;
    case PCB_PIN_TYPE_ISO_GPIO:
        s_gpio = rtk_gpio_id(ISO_GPIO, pinindex);
        ret = rtk_gpio_output(s_gpio, value);
        rtk_gpio_set_dir(s_gpio, 1);
        if (ret != 0)
        {
            RTK_GPIO_WARNING("[%s] set ISO GPIO %d error.\n",
                     __func__ , pinindex);
            return -1;
        }
        return 0;
    case PCB_PIN_TYPE_MIO_GPIO:
        s_gpio = rtk_gpio_id(MIO_GPIO, pinindex); //!MIO: FIX ME
        ret = rtk_gpio_output(s_gpio, value);
        rtk_gpio_set_dir(s_gpio, 1);
        if (ret != 0)
        {
            RTK_GPIO_WARNING("[%s] set MIO GPIO %d error.\n",
                     __func__ , pinindex);
            return -1;
        }
        return 0;
    case PCB_PIN_TYPE_ISO_GPIO_MIO:
        s_gpio = rtk_gpio_id(ISO_GPIO_MIO, pinindex); //!ISO_GPIO_MIO: FIX ME
        ret = rtk_gpio_output(s_gpio, value);
        rtk_gpio_set_dir(s_gpio, 1);
        if (ret != 0)
        {
            RTK_GPIO_WARNING("[%s] set ISO_GPIO_MIO %d error.\n",
                     __func__ , pinindex);
            return -1;
        }
        return 0;
    case PCB_PIN_TYPE_UNUSED:
        return 0;
    default:
        RTK_GPIO_WARNING("[%s] Error, unkown type.\n", __func__);
        return -1;
    }
    return -1;
#else
    return -1;
#endif
}

EXPORT_SYMBOL(rtk_SetIOPin);

int rtk_GetIOPin(unsigned long long pininfo)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_GPIO)
    PCB_PIN_TYPE_T pintype;
    RTK_GPIO_ID s_gpio;
    int pinindex = 0;

    //change origial flow
    //Now,call this api changed flow,when Get IOinput,will set dir to Input;

    pintype = GET_PIN_TYPE(pininfo);
    pinindex = GET_PIN_INDEX(pininfo);

    switch (pintype) {
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_UNIPWM:
    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
    case PCB_PIN_TYPE_EMCU_GPIO:
    case PCB_PIN_TYPE_AUDIO:
        RTK_GPIO_WARNING("[%s] Function isn't support(%x).\n",
                    __func__ , (unsigned int)pininfo);
        return -1;
    case PCB_PIN_TYPE_GPIO:
        s_gpio = rtk_gpio_id(MIS_GPIO, pinindex);
        rtk_gpio_set_dir(s_gpio, 0);
        return rtk_gpio_input(s_gpio);
    case PCB_PIN_TYPE_ISO_GPIO:
        s_gpio = rtk_gpio_id(ISO_GPIO, pinindex);
        rtk_gpio_set_dir(s_gpio, 0);
        return rtk_gpio_input(s_gpio);
    case PCB_PIN_TYPE_MIO_GPIO:
        s_gpio = rtk_gpio_id(MIO_GPIO, pinindex); //!MIO: FIX ME
        rtk_gpio_set_dir(s_gpio, 0);
        return rtk_gpio_input(s_gpio);
    case PCB_PIN_TYPE_ISO_GPIO_MIO:
        s_gpio = rtk_gpio_id(ISO_GPIO_MIO, pinindex); //!ISO_GPIO_MIO: FIX ME
        rtk_gpio_set_dir(s_gpio, 0);
        return rtk_gpio_input(s_gpio);
    case PCB_PIN_TYPE_UNUSED:
        return 0;
    default:
        RTK_GPIO_WARNING("[%s] Error, unkown type.\n", __func__);
        return -1;
    }
    return -1;
#else
    return -1;
#endif
}

EXPORT_SYMBOL(rtk_GetIOPin);

/**
 * @brief set output level by name
 */
int IO_Direct_Set(char *enum_name, int value)
{
    unsigned long long pin;
    int ret = -1;

    if (pcb_mgr_get_enum_info_byname(enum_name, &pin) == 0)
    {
        ret = rtk_SetIOPin(pin, value);
        ret |= rtk_SetIOPinDirection(pin, 1);
    }
    return ret;
}
EXPORT_SYMBOL(IO_Direct_Set);

/**
 * @brief get input level by name
 */
int IO_Direct_Get(char *enum_name)
{
    unsigned long long pin;
    if (pcb_mgr_get_enum_info_byname(enum_name, &pin) == 0)
    {
        return rtk_GetIOPin(pin);
    }
    return -1;
}
EXPORT_SYMBOL(IO_Direct_Get);

