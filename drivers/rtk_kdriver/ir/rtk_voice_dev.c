#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/rtk_gpio-dev.h>
#include <rbus/pinmux_reg.h>
#include "ir_input.h"
static RTK_GPIO_ID io_pin_1, io_pin_2, io_pin_3, io_pin_4;
static RTK_GPIO_ID int_pin;

/*------------------------------------------------------------------
 * Func : voice_dev_gpio_set
 *
 * Desc : voice_dev_gpio_set
 *
 * Parm : NA
 *
 * Retn : NA
 *------------------------------------------------------------------*/
void voice_dev_gpio_set(void)
{
    /*ALC GPIO pin ISO_GPIO1*/
    /*config as gpio pin*/
    io_pin_1 = rtk_gpio_id(ISO_GPIO, 1);

    rtk_gpio_set_dir(io_pin_1, 0);
    RTK_GPIO_WARNING("voice_device_gpio for %s %d (%x) ready!!\n",
                     gpio_type(gpio_group(io_pin_1)), gpio_idx(io_pin_1), io_pin_1);

    /*ALC GPIO pin ISO_GPIO32*/
    /*config as gpio pin*/
    io_pin_2 = rtk_gpio_id(ISO_GPIO, 32);

    rtk_gpio_set_dir(io_pin_2, 0);
    RTK_GPIO_WARNING("voice_device_gpio for %s %d (%x) ready!!\n",
                     gpio_type(gpio_group(io_pin_2)), gpio_idx(io_pin_2), io_pin_2);

    /*ALC GPIO pin ISO_GPIO31*/
    /*config as gpio pin*/
    io_pin_3 = rtk_gpio_id(ISO_GPIO, 31);

    rtk_gpio_set_dir(io_pin_3, 0);
    RTK_GPIO_WARNING("voice_device_gpio for %s %d (%x) ready!!\n",
                     gpio_type(gpio_group(io_pin_3)), gpio_idx(io_pin_3), io_pin_3);

    /*ALC GPIO pin ISO_GPIO30*/
    /*config as gpio pin*/
    io_pin_4 = rtk_gpio_id(ISO_GPIO, 30);

    rtk_gpio_set_dir(io_pin_4, 0);
    RTK_GPIO_WARNING("voice_device_gpio for %s %d (%x) ready!!\n",
                     gpio_type(gpio_group(io_pin_4)), gpio_idx(io_pin_4), io_pin_4);
}

/*------------------------------------------------------------------
 * Func : voice_dev_pinmux_set
 *
 * Desc : voice_dev_pinmux_set
 *
 * Parm : NA
 *
 * Retn : NA
 *------------------------------------------------------------------*/
void voice_dev_pinmux_set(void)
{
    /*GPIO pinmux setting*/
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_5_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_5_reg) | (0xF << 28)); /* ALC interrupt pin ISO_GPIO28*/

    rtd_outl(PINMUX_ST_STB_0_reg,         rtd_inl(PINMUX_ST_STB_0_reg) | (0xF << 20)); /* bit3,ALC GPIO pin ISO_GPIO1*/
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_6_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_6_reg) | (0xF << 16)); /* bit2,ALC GPIO pin ISO_GPIO32*/
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_6_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_6_reg) | (0xF << 20)); /* bit1,ALC GPIO pin ISO_GPIO31*/
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_6_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_6_reg) | (0xF << 24)); /* bit0,ALC GPIO pin ISO_GPIO30*/

    rtd_outl(PINMUX_ST_GPIO_ST_CFG_6_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_6_reg) | (0x1 << 2)); /*ISO_GPIO32,GPI_EN*/
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_6_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_6_reg) | (0x1 << 3)); /*ISO_GPIO31,GPI_EN*/
    rtd_outl(PINMUX_ST_GPIO_ST_CFG_6_reg, rtd_inl(PINMUX_ST_GPIO_ST_CFG_6_reg) | (0x1 << 4)); /*ISO_GPIO30,GPI_EN*/
}



u32 voice_dev_cmd_to_keycode(uint32_t value)
{
    u32 keycode = 0;
    if(value >= 16)
        return 0;
    switch(value) {
        case 0x0:
            keycode = KEY_ALC_HELLO_MIDOLI;
            break;
        case 0x3:
            keycode = KEY_ALC_GO_RIGHT;
            break;
        case 0x4:
            keycode = KEY_ALC_GO_LEFT;
            break;
        case 0x7:
            keycode = KEY_ALC_STOP;
            break;
        case 0x8:
            keycode = KEY_ALC_HELLO_MENU;
            break;
        case 0x9:
            keycode = KEY_ALC_HELLO_VOLUME_LOUDER;
            break;
        case 0xa:
            keycode = KEY_ALC_HELLO_VOLUME_DOWN;
            break;
        case 0xb:
            keycode = KEY_ALC_GO_PREVIOUS;
            break;
        case 0xc:
            keycode = KEY_ALC_GO_NEXT;
            break;
        case 0xd:
            keycode = KEY_ALC_CONFIRM;
            break;
        case 0xe:
            keycode = KEY_ALC_START_PLAY;
            break;
        case 0xf:
            keycode = KEY_ALC_PAUSE;
            break;
        default:
            keycode = 0;
    }
    return keycode;
}



/*------------------------------------------------------------------
 * Func : voice_dev_interrupt_handler
 *
 * Desc : voice_dev_interrupt_handler
 *
 * Parm : NA
 *
 * Retn : NA
 *------------------------------------------------------------------*/
static void voice_dev_int_isr(RTK_GPIO_ID gid, unsigned char assert, void *dev)
{
    u32 regVal = 0;
    u32 keycode = 0;

    if (assert)
        return;

    regVal = rtk_gpio_input(io_pin_4) | (rtk_gpio_input(io_pin_3) << 1)
             | (rtk_gpio_input(io_pin_2) << 2) | (rtk_gpio_input(io_pin_1) << 3);
    keycode = voice_dev_cmd_to_keycode(regVal);
    if(keycode != 0)
        venus_ir_gen_keyupdown_event(keycode);
}

/*------------------------------------------------------------------
 * Func : voice_dev_interrupt_set
 *
 * Desc : voice_dev_interrupt_set
 *
 * Parm : NA
 *
 * Retn : NA
 *------------------------------------------------------------------*/
void voice_dev_interrupt_set(void)
{
    int err;
    /*ALC interrupt pin ISO_GPIO34*/
    /*config as interrupt pin*/
    int_pin = rtk_gpio_id(ISO_GPIO, 28);
    rtk_gpio_set_dir(int_pin, 0);
    /* 100 us*/
    rtk_gpio_set_debounce(int_pin, RTK_GPIO_DEBOUNCE_100us);

    /*low active*/
    rtk_gpio_set_irq_polarity(int_pin, 0);

    err = rtk_gpio_request_irq(int_pin,
                               voice_dev_int_isr, "VOICE_DEV_INT",
                               voice_dev_int_isr);
    if (err)
        RTK_GPIO_WARNING("%s request irq fail\n", __func__);

    rtk_gpio_set_irq_enable(int_pin, 1);
    RTK_GPIO_WARNING("voice_device_gpio for %s %d (%x) ready!!\n",
                     gpio_type(gpio_group(int_pin)), gpio_idx(int_pin), int_pin);
}

/*------------------------------------------------------------------
 * Func : voice_device_init
 *
 * Desc : voice_device_init
 *
 * Parm : NA
 *
 * Retn : NA
 *------------------------------------------------------------------*/
void voice_dev_init(void)
{
    /*pinmux setting*/
    voice_dev_pinmux_set();
    /*four gpio pin config*/
    voice_dev_gpio_set();
    /*interrupt pin config*/
    voice_dev_interrupt_set();
}

/*------------------------------------------------------------------
 * Func : voice_device_uninit
 *
 * Desc : voice_device_uninit
 *
 * Parm : NA
 *
 * Retn : NA
 *------------------------------------------------------------------*/
void voice_dev_uninit(void)
{
    rtk_gpio_set_irq_enable(int_pin, 0);
    rtk_gpio_clear_isr(int_pin);
    rtk_gpio_free_irq(int_pin, voice_dev_int_isr);
}

