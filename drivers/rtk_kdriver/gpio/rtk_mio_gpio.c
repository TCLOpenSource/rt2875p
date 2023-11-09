#include <rtk_kdriver/rtk_gpio.h>
//#include <rtk_kdriver/rtk_gpio-chipset.h>
#ifdef CONFIG_RTK_KDRV_GPIO_BUILT_IN_TEST
#include <rtk_kdriver/rtk_gpio-bist.h>
#endif


#include "rtk_gpio-reg.h"
#ifndef BUILD_QUICK_SHOW

#include <rtk_gpio-debug.h>
#endif
#include <rtk_kdriver/rtk_gpio-db.h>
#include "rtk_mio_gpio.h"

#ifdef BUILD_QUICK_SHOW
#include <errno.h>
#include <mach/pcbMgr.h>
#include <no_os/export.h>
#include <no_os/printk.h>
#include <no_os/spinlock.h>
#include <no_os/slab.h>
#include <sysdefs.h>
#include <string.h>
#include <malloc.h>
#include "rtk_gpio-reg.h"
#include <rbus/iso_gpio_reg.h>
#include <rbus/mio_gpio_reg.h>

#endif


#define MIO_GPIO_REG_SETS           ((MIO_GPIO_CNT+31)>>5)
#define MIO_REG_SHADOW              (0x100)




void iowrite_mio_reg_bit(unsigned long reg, unsigned char bit,unsigned char val)
{
    val = (val) ? 1 : 0 ;
    //if ((gpio_ioread32(reg) & (0x1 << bit)) != (val << bit))  //MIO func read&write wil caused unnormal error
    gpio_iowrite32(reg, val | (0x1 << bit));

}

static int _rtk_mio_get_reg_ofst_bit(RTK_GPIO_ID gid ,unsigned int *pOffset, unsigned char *pBit)
{
    int id = gpio_idx(gid);

    switch (gpio_group(gid))
    {

        case MIO_GPIO:
            if (id < MIO_GPIO_CNT)
            {
                if(id < 31) {

                    *pOffset = (0 << 2);
                    *pBit = id + 1;

                }else if(id < 61) {
                    *pOffset = (1 << 2);
                    *pBit = (id - 31)+ 1;

                }else if(id < 92) {
                    *pOffset = (2 << 2);
                    *pBit = (id - 61)+ 1;
                }else if(id < MIO_GPIO_CNT) {
                    *pOffset = (3 << 2);
                    *pBit = (id - 91)+ 1;
                }
                //rtd_pr_gpio_info("gpio pOffset:%d,pBit:%d\n",*pOffset,*pBit);
                return 0;
            }
            break;

        default:
            break;
    }

    return -EFAULT;
}

void  rtk_mio_gpio_disable_irq_all(void)
{
    int i;

    for (i = 0; i < MIO_GPIO_REG_SETS; i++){
        gpio_iowrite32(MIO_GPIE_reg + (i << 2), 0xFFFFFFFE);
        gpio_iowrite32(MIO_GPIE_DA_reg + (i << 2), 0xFFFFFFFE);
    }

}


void rtk_mio_gpio_clear_isr_all(void)
{
    int i;

    for (i = 0; i < MIO_GPIO_REG_SETS; i++)
    {
        gpio_iowrite32(MIO_ISR_GP0A_reg + (i << 2), 0xFFFFFFFE);
        gpio_iowrite32(MIO_ISR_GP0DA_reg + (i << 2), 0xFFFFFFFE);
    }

}



int rtk_mio_gpio_set_dir(RTK_GPIO_ID gid, unsigned char Out)
{
    unsigned int offset=0;
    unsigned char bit=0;
    if( _rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            iowrite_mio_reg_bit(MIO_GPDIR_reg + offset,bit, Out);
            return 0;

        default:
            break;
    }

    return -EFAULT;
}
int rtk_mio_gpio_get_dir(RTK_GPIO_ID gid)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if(_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit)!=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ ,gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) {

        case MIO_GPIO:
            return ioread_reg_bit(MIO_GPDIR_reg + offset,bit);
        default:
            break;
    }
    return -EFAULT;
}
int rtk_mio_gpio_set_irq_polarity(RTK_GPIO_ID gid, unsigned char positive)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if( _rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) {
        case MIO_GPIO:
            iowrite_mio_reg_bit(MIO_GPDP_reg + offset, bit, positive);
            return 0;

        default:
            break;
    }

    return -EFAULT;
}

int rtk_mio_gpio_set_irq_enable(RTK_GPIO_ID gid, unsigned char On)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ ,gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {

        case MIO_GPIO:
            iowrite_mio_reg_bit(MIO_GPINT_SCPU_reg + offset,bit, 1);    //mask irq enable in scpu

            iowrite_mio_reg_bit(MIO_GPIE_reg + offset,bit, On);
            return 0;

            default:
                break;
    }

    return -EFAULT;
}
int rtk_mio_gpio_chk_irq_enable(RTK_GPIO_ID gid)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            return ioread_reg_bit(MIO_GPIE_reg + offset,bit);  //if changed to read in shawn, will clear IE bit;
        default:
            break;
    }

    return 0;
}

int rtk_mio_gpio_set_dis_irq_enable(RTK_GPIO_ID gid, unsigned char On)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ ,gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:      //MIO_GPIE_DA_reg
            iowrite_mio_reg_bit(MIO_GPIE_DA_reg + offset,bit, On);
            return 0;
        default:
            break;
    }

    return -EFAULT;
}

int rtk_mio_gpio_chk_dis_irq_enable(RTK_GPIO_ID gid)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            return ioread_reg_bit(MIO_GPIE_DA_reg + offset,bit); //if changed to read in shawn, will clear IE bit;
        default:
            break;
    }

    return 0;
}

int rtk_mio_gpio_clear_isr(RTK_GPIO_ID gid)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            iowrite_mio_reg_bit(MIO_ISR_GP0A_reg + offset, bit, 0);
            iowrite_mio_reg_bit(MIO_ISR_GP0DA_reg + offset, bit, 0);
            return 0;
        default:
            break;
    }

    return -EFAULT;
}

int rtk_mio_gpio_set_debounce(RTK_GPIO_ID gid, unsigned char val)
{
    int id = gpio_idx(gid);

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            if (id < MIO_GPIO_CNT && val <= 7)
            {
                gpio_iowrite32(MIO_GPDEB_reg, (0x8 | val) << ((id >> 4) * 4));
                return 0;
            }
            break;
        
        default:
            break;
    }

    return -EFAULT;
}

int rtk_mio_gpio_input(RTK_GPIO_ID gid)
{
    unsigned int offset=0;
    unsigned char bit=0;

    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            return ioread_reg_bit((MIO_GPDATI_reg + MIO_REG_SHADOW) + offset, bit);
        default:
            break;
    }

    return -EFAULT;
}
int rtk_mio_gpio_output(RTK_GPIO_ID gid, unsigned char val)
{
    unsigned int offset=0;
    unsigned char bit=0;
    if( _rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            iowrite_mio_reg_bit(MIO_GPDATO_reg + offset,bit, val);
            return 0;
        default:
            break;
    }

    return -EFAULT;
}


int rtk_mio_gpio_output_get(RTK_GPIO_ID gid )
{
    unsigned int offset=0;
    unsigned char bit=0;
    if (_rtk_mio_get_reg_ofst_bit(gid,&offset,&bit) !=0 )
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
        case MIO_GPIO:
            return ioread_reg_bit(MIO_GPDATO_reg + offset,bit);
            break;
        default:
            break;
    }

    return -EFAULT;
}

