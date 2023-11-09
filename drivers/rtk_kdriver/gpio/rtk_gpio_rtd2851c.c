#ifndef BUILD_QUICK_SHOW
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/irq.h>
#include <linux/bitops.h>
#include <linux/ioctl.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <rtd_log/rtd_module_log.h>
#include <mach/pcbMgr.h>
#include <base_types.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/rtk_gpio-chipset.h>
#ifdef CONFIG_RTK_KDRV_GPIO_BUILT_IN_TEST
#include <rtk_kdriver/rtk_gpio-bist.h>
#endif
#include "rtk_gpio-reg.h"
#include <rtk_gpio-debug.h>
#include <rtk_kdriver/rtk_gpio-db.h>

#ifdef CONFIG_OF
#include <linux/of_irq.h>
#include <linux/of_address.h>
#endif

#else /* !BUILD_QUICK_SHOW */
#include <errno.h>
#include <mach/pcbMgr.h>
#include <no_os/export.h>
#include <no_os/printk.h>
#include <no_os/spinlock.h>
#include <no_os/slab.h>
#include <sysdefs.h>
#include <string.h>
#include <malloc.h>
#endif /* !BUILD_QUICK_SHOW */
#include <rtd_log/rtd_module_log.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/rtk_gpio.h>
#include "rtk_gpio-reg.h"

#ifndef BUILD_QUICK_SHOW
#if 1//def CONFIG_ARM64
extern u32 gic_irq_find_mapping(u32 hwirq);
#define IRQ_GET_KERNEL_IRQ_NUM(hwirq)       gic_irq_find_mapping(hwirq)
#else
#define IRQ_GET_KERNEL_IRQ_NUM(hwirq)       (hwirq)
#endif

extern Array RTK_GPIO_DB;

//=============================================================================


#define ENABLE_GPIO_STR

static int g_lg_hal_int_flag = 0;
#define MIS_GPIO_REG_SETS           ((MIS_GPIO_CNT+31)>>5)
#define ISO_GPIO_REG_SETS           ((ISO_GPIO_CNT+31)>>5)
#define GPI_MIS_REG_SETS            ((GPI_MIS_CNT+31)>>5)
#define GPI_ISO_REG_SETS            ((GPI_ISO_CNT+31)>>5)
#define GPO_MIS_REG_SETS            ((GPO_MIS_CNT+31)>>5)
#define GPO_ISO_REG_SETS            ((GPO_ISO_CNT+31)>>5)
#define GPIO_REG_OFST(id)           ((id/31)<<2)
#define GPIO_REG_BIT(id)            ((id%31)+1)  // reg bit = id % 32
#define GPIO_INT_REG_OFST(id)       ((id/31)<<2)
#define GPIO_INT_REG_BIT(id)        ((id%31)+1)

#else  /* BUILD_QUICK_SHOW */
#define RTK_GPIO_TRACE(fmt, args...)
#define RTK_GPIO_INFO(fmt, args...)

#define RTK_GPIO_WARNING(fmt, args...)
#define GPIO_WARNING_DEBUG_LEVEL(fmt, args...)
#define RTK_GPIO_ERROR(fmt, args...)                printf2(fmt, ## args)
typedef RTK_GPIO_GROUP RT_GPIO_TYPE;
#endif  /* !BUILD_QUICK_SHOW */

RTK_GPIO_PCB_T* pPcbInfo = NULL;
static RTK_GPIO_IRQ rtk_gpio_irq[MIS_GPIO_CNT + ISO_GPIO_CNT + GPI_MIS_CNT + GPI_ISO_CNT];

#ifdef ENABLE_GPIO_STR
/* only LG pcb need to  store and recovery gpio in/output values */
static unsigned int backup_mis_gpie[MIS_GPIO_REG_SETS];
//static unsigned int backup_iso_gpie[ISO_GPIO_REG_SETS];

static RTK_GPIO_CONFIG backupMisGpioConfig[MIS_GPIO_REG_SETS];
//static RTK_GPIO_CONFIG backupIsoGpioConfig[ISO_GPIO_REG_SETS];

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
static RTK_GPIO_CONFIG backupGpoMisConfig[GPO_MIS_REG_SETS];
static RTK_GPIO_CONFIG backupGpoIsoConfig[GPO_ISO_REG_SETS];
#endif

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
static unsigned int backup_gpi_mis_gpie[GPI_MIS_REG_SETS];
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
static unsigned int backup_gpi_iso_gpie[GPI_ISO_REG_SETS];
#endif


#else


int getGpioHwSemaphore( void )
{
    return 0;
}

int releaseGpioHwSemaphore( void )
{
    return 0 ;
}

#endif

extern int bShowStrMessage ;

static DEFINE_SPINLOCK(rtk_gpio_irq_lock);

void iowrite_reg_bit(unsigned long reg, unsigned char bit,unsigned char val)
{
    unsigned long _GPIOFlags = 0;
    val = (val) ? 1 : 0 ;
    if ((gpio_ioread32(reg) & (0x1 << bit)) != (val << bit))
    {
        spin_lock_irqsave(&rtk_gpio_irq_lock, _GPIOFlags);
        gpio_iowrite32(reg, val | (0x1 << bit));
        spin_unlock_irqrestore(&rtk_gpio_irq_lock, _GPIOFlags);
    }

}

void PowerOnGPIO(void)
{
    RTK_GPIO_WARNING("power on\n");
    CRT_CLK_OnOff( GPIO, CLK_ON, 0);
}


static RTK_GPIO_IRQ *_get_gpio_entry(RTK_GPIO_ID gid)
{
    int id = gpio_idx(gid);

    switch (gpio_group(gid)) 
    {
    case MIS_GPIO:
        if (id < MIS_GPIO_CNT)
            return &rtk_gpio_irq[id];
        break;

    case ISO_GPIO:
        if (id < ISO_GPIO_CNT)
            return &rtk_gpio_irq[id + MIS_GPIO_CNT];
        break;
    case MIS_GPI:
        if (id < GPI_MIS_CNT)
            return &rtk_gpio_irq[id + MIS_GPIO_CNT + ISO_GPIO_CNT];
        break;
#if GPI_ISO_CNT
    case ISO_GPI:
        if (id < GPI_ISO_CNT)
            return &rtk_gpio_irq[id + MIS_GPIO_CNT + ISO_GPIO_CNT + GPI_MIS_CNT];
        break;
#endif

    default:
        break;
    }
    
    RTK_GPIO_WARNING("%s failed  %s  index = %d \n",
            __func__ ,gpio_type(gpio_group(gid)), id );

    return NULL;
}


static int _get_reg_ofst_bit(RTK_GPIO_ID gid ,unsigned int *pOffset, unsigned char *pBit)
{
    int id = gpio_idx(gid);

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        if (id < MIS_GPIO_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);
            return 0;
        }
        break;

    case ISO_GPIO:
        if (id < ISO_GPIO_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);
            return 0;
        }
        break;

    case MIS_GPI:
        if (id < GPI_MIS_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);
            return 0;
        }
        break;

#if GPO_MIS_CNT
    case MIS_GPO:
        if (id < GPO_MIS_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);
            return 0;
        }
        break;
#endif

    case ISO_GPO:
        if (id < GPO_ISO_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);
            return 0;
        }
        break;

    case ISO_GPI:
        if (id < GPI_ISO_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);
            return 0;
        }
        break;

    default:
        break;
    }

/*
    RTK_GPIO_WARNING("%s failed  %s  index = %d \n" , __func__ , ( ISO_GPIO == gpio_group(gid) )?("ISO"):
        ( ( ISO_GPIO == gpio_group(gid) )?("MIS"):("ISO2") )  , id );
*/

    return -EFAULT;
}

static int _get_int_reg_ofst_bit(RTK_GPIO_ID gid ,unsigned int *pOffset, unsigned char *pBit)
{

    int id = gpio_idx(gid);

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        if (id < MIS_GPIO_CNT)
        {
            if(id>122)
            {
                id++;
                *pOffset = ((id/31)<<2);
                *pBit = ((id%31)+1);
            }
            else
            {
                *pOffset = ((id/31)<<2);
                *pBit = ((id%31)+1);
            }
            return 0;
        }
        break;

    case ISO_GPIO:
        if (id < ISO_GPIO_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);

            return 0;
        }
        break;

    case MIS_GPI:
        if (id < GPI_MIS_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);

            return 0;
        }
        break;

#if GPI_ISO_CNT
    case ISO_GPI:
        if (id < GPI_ISO_CNT)
        {
            *pOffset = ((id/31)<<2);
            *pBit = ((id%31)+1);

            return 0;
        }
        break;
#endif

    default:
        break;
    }

    /*RTK_GPIO_WARNING("%s failed  %s  index = %d \n" , __func__ , ( ISO_GPIO == gpio_group(gid) )?("ISO"):
        ( ( ISO_GPIO == gpio_group(gid) )?("MIS"):("ISO2") )  , id );
    */

    return -EFAULT;
}

RTK_GPIO_ID rtk_gpio_get_gid_by_pintype(PCB_PIN_TYPE_T pin_type, unsigned char pin_index)
{
    switch (pin_type) {
        case PCB_PIN_TYPE_GPIO:
            return rtk_gpio_id(MIS_GPIO, pin_index);
        case PCB_PIN_TYPE_ISO_GPIO:
            return rtk_gpio_id(ISO_GPIO, pin_index);
        case PCB_PIN_TYPE_GPO:
            return rtk_gpio_id(MIS_GPO, pin_index);
        case PCB_PIN_TYPE_ISO_GPO:
            return rtk_gpio_id(ISO_GPO, pin_index);
        case PCB_PIN_TYPE_GPI:
            return rtk_gpio_id(MIS_GPI, pin_index);
        case PCB_PIN_TYPE_ISO_GPI:
            return rtk_gpio_id(ISO_GPO, pin_index);
        default:
            RTK_GPIO_WARNING("pin type incorrect!!!\n");
            return 0xFFFFFFFF;
      }
}
EXPORT_SYMBOL(rtk_gpio_get_gid_by_pintype);

/*------------------------------------------------------------------                                                                               
 * * Func : rtk_gpio_set_dir
 * *
 * * Desc :  Set GPIO direction
 * *
 * *        Must set GPIO output value firstly!!!!
 * *        If not,GPIO pin will output the default value which is not 
 * *        your wanted,then gilch maked.
 * *
 * * Parm : gid :RTK GPIO include gpio type and index Number
 * *        out :direction of gpio 1:output 0:intput
 * *
 * * Retn : -1 : Set output value failed.
 * *         0 : Set output value success.
 * *------------------------------------------------------------------*/

int rtk_gpio_set_dir(RTK_GPIO_ID gid, unsigned char Out)
{
    unsigned int offset;
    unsigned char bit;

    if( _get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        iowrite_reg_bit(MIS_GP0DIR_reg + offset,bit, Out);
        return 0;

    case ISO_GPIO:
        iowrite_reg_bit(ISO_GPDIR_reg + offset,bit, Out);
        return 0;

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_set_dir);

int rtk_gpio_get_dir(RTK_GPIO_ID gid)
{
    unsigned int offset;
    unsigned char bit;

    if(_get_reg_ofst_bit(gid,&offset,&bit)!=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ ,gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) {
    case MIS_GPIO:
        return ioread_reg_bit(MIS_GP0DIR_reg + offset,bit);

    case ISO_GPIO:
        return ioread_reg_bit(ISO_GPDIR_reg + offset,bit);

    default:
        break;
    }
    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_get_dir);

int rtk_gpio_set_debounce(RTK_GPIO_ID gid, unsigned char val)
{
    int id = gpio_idx(gid);

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        if (id < MIS_GPIO_CNT && val <= 7)
        {
            gpio_iowrite32(MIS_GPDEB_reg, (0x8 | val) << ((id >> 4) * 4));
            return 0;
        }
        break;
        
    case ISO_GPIO:
        if (id < ISO_GPIO_CNT && val <= 7)
        {
            gpio_iowrite32(ISO_GPDEB_reg, (0x8 | val) << ((id >> 4) * 4));
            return 0;
        }
        break;
        
    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_set_debounce);

int rtk_gpio_input(RTK_GPIO_ID gid)
{
    unsigned int offset;
    unsigned char bit;

    if (_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) 
    {
    case MIS_GPIO:
        return ioread_reg_bit(MIS_GP0DATI_reg + offset, bit);

    case ISO_GPIO:
        return ioread_reg_bit(ISO_GPDATI_reg + offset, bit);

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        return ioread_reg_bit(GPI_MIS_DATI_reg + offset, bit);
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        return ioread_reg_bit(GPI_ISO_DATI_reg + offset, bit);
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_input);

int rtk_gpio_output(RTK_GPIO_ID gid, unsigned char val)
{
    unsigned int offset;
    unsigned char bit;

    if( _get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) {

    case MIS_GPIO:
        iowrite_reg_bit(MIS_GP0DATO_reg + offset,bit, val);
        return 0;

    case ISO_GPIO:
        iowrite_reg_bit(ISO_GPDATO_reg + offset,bit, val);
        return 0;

    case MIS_GPO:

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
        iowrite_reg_bit(MIS_GPO_DATO_reg + offset, bit, val);
        return 0;
#else
        RTK_GPIO_WARNING("unknown gpio type\n");
        break;
#endif

    case ISO_GPO:

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
        iowrite_reg_bit(ISO_GPO_DATO_reg + offset, bit, val);
        return 0;
#else
        RTK_GPIO_WARNING("unknown gpio type\n");
        break;
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_output);

int rtk_gpio_output_get(RTK_GPIO_ID gid )
{
    unsigned int offset;
    unsigned char bit;

    if (_get_reg_ofst_bit(gid,&offset,&bit) !=0 )
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) 
    {
    case MIS_GPIO:
        return ioread_reg_bit(MIS_GP0DATO_reg + offset,bit);

    case ISO_GPIO:
        return ioread_reg_bit(ISO_GPDATO_reg + offset,bit);
        break;
    case MIS_GPO:

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
        return ioread_reg_bit(MIS_GPO_DATO_reg + offset, bit);
#else
        RTK_GPIO_WARNING("unknown gpio type\n");
        break;
#endif

    case ISO_GPO:
#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
        return ioread_reg_bit(ISO_GPO_DATO_reg + offset, bit);
#else
        RTK_GPIO_WARNING("unknown gpio type\n");
        break;
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_output_get);


void rtk_gpio_init_pcb_pin(void){
    int i;
    int idx = 0;
    unsigned char pin_type;
    RTK_GPIO_ID gpio_id;
    unsigned char pin_index = 0;
    PCB_GPIO_TYPE_T input_type;
    unsigned char invert;
    unsigned char init_value=0;
    bool gpio_pin = true;
    RTK_GPIO_GROUP gpio_type;

    if(pPcbInfo==NULL)
        pPcbInfo = (RTK_GPIO_PCB_T*)kmalloc(sizeof(RTK_GPIO_PCB_T), GFP_KERNEL);

    memset(pPcbInfo,0,sizeof(RTK_GPIO_PCB_T));

    for(i = 0; i < PCB_ENUM_MAX; i++){
        if((memcmp(pcb_enum_all[i].name, "PIN", 3) == 0) && (pcb_enum_all[i].value != 0) && (strstr(pcb_enum_all[i].name,"GPIO_INFO") == NULL))
        {
            pin_type  = GET_PIN_TYPE(pcb_enum_all[i].value);
            pin_index = GET_PIN_INDEX(pcb_enum_all[i].value);
            input_type = GET_PIN_PARAM1(pcb_enum_all[i].value);
            invert    = GET_PIN_PARAM2(pcb_enum_all[i].value);
            init_value= GET_PIN_PARAM3(pcb_enum_all[i].value);
            gpio_pin = true;
            switch (pin_type) {
                case PCB_PIN_TYPE_GPIO:
                    gpio_id = rtk_gpio_id(MIS_GPIO, pin_index);
                    gpio_type = MIS_GPIO;
                    break;
                case PCB_PIN_TYPE_ISO_GPIO:
                    gpio_id = rtk_gpio_id(ISO_GPIO, pin_index);
                    gpio_type = ISO_GPIO;
                    break;
                case PCB_PIN_TYPE_GPO:
                    gpio_id = rtk_gpio_id(MIS_GPO, pin_index);
                    gpio_type = MIS_GPO;
                    break;
                case PCB_PIN_TYPE_ISO_GPO:
                    gpio_id = rtk_gpio_id(ISO_GPO, pin_index);
                    gpio_type = ISO_GPO;
                    break;
                case PCB_PIN_TYPE_GPI:
                    gpio_id = rtk_gpio_id(MIS_GPI, pin_index);
                    gpio_type = MIS_GPI;
                    break;
                case PCB_PIN_TYPE_ISO_GPI:
                    gpio_id = rtk_gpio_id(ISO_GPI, pin_index);
                    gpio_type = ISO_GPI;
                    break;
                case PCB_PIN_TYPE_EMCU_GPIO:
                    gpio_id = rtk_gpio_id(ISO_GPIO, pin_index);
                    gpio_type = ISO_GPIO;
                    break;
                default:
                    gpio_pin = false;
                    break;
            }
            if(gpio_pin)
            {
                strncpy(pPcbInfo->pcblist[idx].pin_name,pcb_enum_all[i].name,31);
                pPcbInfo->pcblist[idx].pin_name[31] = '\0';
                pPcbInfo->pcblist[idx].pin_type     = pin_type;
                pPcbInfo->pcblist[idx].pin_index    = pin_index;
                pPcbInfo->pcblist[idx].input_type   = input_type;
                pPcbInfo->pcblist[idx].invert       = invert;
                pPcbInfo->pcblist[idx].init_value   = init_value;
                pPcbInfo->pcblist[idx].gid          = gpio_id;
                pPcbInfo->pcblist[idx].gpio_type    = gpio_type;
                pPcbInfo->pcblist[idx].valid        = 1;
                idx++;
            }
            //RTK_GPIO_WARNING(" PIN !!!! , pcb name=%s ,value=%lld, pin_type=%d pin_index=%d, input_type=%d gpio_pin=%d\n",pcb_enum_all[i].name,pcb_enum_all[i].value,pin_type,pin_index,input_type,gpio_pin);
        }
            	
    }
    pPcbInfo->pcbcnt=idx;

#if 0
    RTK_GPIO_WARNING(" total gpio pin=%d\n",idx);
    for(i = 0; i < idx; i++){
        RTK_GPIO_PCBINFO_T pin = pPcbInfo->pcblist[i];
        RTK_GPIO_ERROR(" gpio pin_name=%s ,pin_type=%d, pin_index=%d, input_type=%d, invert=%d, init_value=%d, gid=%03x\n",
            pin.pin_name,pin.pin_type,pin.pin_index,pin.gpio_type,pin.invert,pin.init_value,pin.gid);
    }
#endif
}

int rtk_gpio_get_pcb_info(char* pinName,RTK_GPIO_PCBINFO_T*pInfo){
    int i;
    for(i = 0; i < MAX_PCB_GPIO_COUNT; i++){
        if(strcmp(pPcbInfo->pcblist[i].pin_name, pinName)==0){
            memcpy(pInfo,&pPcbInfo->pcblist[i],sizeof(RTK_GPIO_PCBINFO_T));
            return 0;
        }
    }
    return -1;
}
EXPORT_SYMBOL(rtk_gpio_get_pcb_info);

void rtk_gpio_get_all_pinInfo(RTK_GPIO_PCBINFO_T* pinInfolist,int *count){
    int i,idx=0;
    for(i = 0; i < pPcbInfo->pcbcnt; i++){
        if(pPcbInfo->pcblist[i].valid){
            memcpy(&pinInfolist[idx++],&pPcbInfo->pcblist[i],sizeof(RTK_GPIO_PCBINFO_T));
        }
    }
    *count = pPcbInfo->pcbcnt;
    RTK_GPIO_WARNING("rtk_gpio_copy_all_pinInfo total gpio pin= %d\n",idx);
}


#ifndef BUILD_QUICK_SHOW
int rtk_gpio_request_irq(RTK_GPIO_ID gid,
             void (*handler) (RTK_GPIO_ID gid,
                      unsigned char assert, void *dev_id),
             char *name, void *dev_id)
{
    RTK_GPIO_IRQ *p_irq = _get_gpio_entry(gid);
    unsigned long flags;

    if (p_irq == NULL)
    {
        RTK_GPIO_WARNING("request_rtk_gpio_irq failed - %s GPIO(%d) does not exist\n",
             gpio_type(gpio_group(gid)), gpio_idx(gid));
        return -1;
    }

    if (name == NULL || handler == NULL || dev_id == NULL)
    {
        RTK_GPIO_WARNING("request_rtk_gpio_irq failed - in valid argument\n");
        return -1;
    }

    spin_lock_irqsave(&rtk_gpio_irq_lock, flags);

    if (p_irq->handler) {
        RTK_GPIO_WARNING("request_rtk_gpio_irq failed - %s GPIO (%d) is occupied by [%s]\n",
             gpio_type(gpio_group(gid)), gpio_idx(gid), p_irq->name);
        spin_unlock_irqrestore(&rtk_gpio_irq_lock, flags);
        return -1;
    }

    strncpy(p_irq->name, name, sizeof(p_irq->name)-1);
    p_irq->dev_id = dev_id;
    p_irq->handler = handler;
    p_irq->irq_flag = 0;

    spin_unlock_irqrestore(&rtk_gpio_irq_lock, flags);

    RTK_GPIO_INFO("request_rtk_gpio_irq - %s GPIO (%d) irq requested by (%s)\n",
         gpio_type(gpio_group(gid)), gpio_idx(gid), p_irq->name);

    return 0;
}

EXPORT_SYMBOL(rtk_gpio_request_irq);

void rtk_gpio_free_irq(RTK_GPIO_ID gid, void *dev_id)
{
    RTK_GPIO_IRQ *p_irq = _get_gpio_entry(gid);
    unsigned long flags;
    
    if (p_irq)
    {
        spin_lock_irqsave(&rtk_gpio_irq_lock, flags);

        if (p_irq->dev_id == dev_id)
        {
            memset(p_irq, 0, sizeof(RTK_GPIO_IRQ));
            RTK_GPIO_INFO("rtk_gpio_free_irq - %s GPIO %d irq released\n",
                 gpio_type(gpio_group(gid)), gpio_idx(gid));
        }

        spin_unlock_irqrestore(&rtk_gpio_irq_lock, flags);
    }
    else
    {
        RTK_GPIO_WARNING("rtk_gpio_free_irq failed - %s GPIO %d irq does not exists \n",
             gpio_type(gpio_group(gid)), gpio_idx(gid));
    }
}

EXPORT_SYMBOL(rtk_gpio_free_irq);

static int _do_rtk_gpio_irq(RTK_GPIO_ID id, unsigned char assert)
{
    RTK_GPIO_IRQ *p_irq = _get_gpio_entry(id);
    unsigned long flags;
    int ret = 0;

    if (p_irq) 
    {
        spin_lock_irqsave(&rtk_gpio_irq_lock, flags);

        if (p_irq->handler)
            p_irq->handler(id, assert, p_irq->dev_id);

        spin_unlock_irqrestore(&rtk_gpio_irq_lock, flags);
        ret = 1;
    }
    return ret;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// Interrupt Handler                                                                                //
//////////////////////////////////////////////////////////////////////////////////////////////////////

static irqreturn_t gpio_interrupt_handler(int irq, void *dev_id)
{
    RTK_GPIO_GROUP group = (RTK_GPIO_GROUP)~((unsigned long)dev_id);
    unsigned long GPIO_ISR = MIS_ISR_reg;
    unsigned long ISR_GPIO_ASSERT = (0x1 << MIS_ISR_assert_int_shift);
    unsigned long GPIO_CNT = MIS_GPIO_CNT;
    unsigned long UMSK_ISR_GPDA = MIS_UMSK_ISR_GP0DA_reg;
    unsigned long UMSK_ISR_GPA = MIS_UMSK_ISR_GP0A_reg;
    uint32_t status;
    uint32_t event;
    unsigned int i;
    unsigned int gpioNr = 0;
    unsigned int reg_ofst = 0;

    if ( (rtd_inl(MIS_ISR_reg) & (0x1 << MIS_ISR_assert_int_shift) ) == 0 &&
        (rtd_inl(ISO_ISR_reg) & (0x1 << ISO_ISR_assert_int_shift) ) == 0 )
        return IRQ_NONE;

    /* misc gpio using default value*/
    if (group == ISO_GPIO)
    {
        GPIO_ISR = ISO_ISR_reg;
        ISR_GPIO_ASSERT = (0x1 << ISO_ISR_assert_int_shift);
        GPIO_CNT = ISO_GPIO_CNT;
        UMSK_ISR_GPDA = ISO_ISR_GP0DA_reg;
        UMSK_ISR_GPA = ISO_ISR_GP0A_reg;
    }

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    else if (group == MIS_GPI)
    {
        GPIO_ISR = GPI_MIS_ISR_reg;
        ISR_GPIO_ASSERT = (0x1 << GPI_MIS_ISR_assert_int_shift);
        GPIO_CNT = GPI_MIS_CNT;
        UMSK_ISR_GPDA = GPI_MIS_ISR_DEASSERT_reg;
        UMSK_ISR_GPA = GPI_MIS_ISR_ASSERT_reg;
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    else if (group == ISO_GPI)
    {
        GPIO_ISR = GPI_ISO_ISR_reg;
        ISR_GPIO_ASSERT = (0x1 << GPI_ISO_ISR_assert_int_shift);
        GPIO_CNT = GPI_ISO_CNT;
        UMSK_ISR_GPDA = GPI_ISO_ISR_DEASSERT_reg;
        UMSK_ISR_GPA = GPI_ISO_ISR_ASSERT_reg;
    }
#endif

    event = gpio_ioread32(GPIO_ISR) & ISR_GPIO_ASSERT;
    /*RTK_GPIO_WARNING("event = 0x%X ,  GPIO_ISR = 0x%X , GPIO_ISR_value = 0x%X , ISR_GPIO_ASSERT = 0x%X \n" , event ,  GPIO_ISR, gpio_ioread32(GPIO_ISR), ISR_GPIO_ASSERT);*/

    if (event == 0)
        return IRQ_NONE;

    if (event & ISR_GPIO_ASSERT)
    {
        /* dis-assert */
        gpioNr = 0;
        reg_ofst = 0;
        while (gpioNr < GPIO_CNT)
        {
            status = gpio_ioread32(UMSK_ISR_GPDA + reg_ofst) >> 1;
            if (status == 0)
            {
                gpioNr += 31;
                reg_ofst += 4;
                continue;
            }

            gpio_iowrite32(UMSK_ISR_GPDA + reg_ofst, status << 1);
            i = gpioNr;

            while (status && i < GPIO_CNT)
            {
                /* rtd_pr_gpio_info("status = %08x\n", status); */
                if ((status & 0x1) && rtk_gpio_chk_irq_enable(rtk_gpio_id(group, i)))
                    _do_rtk_gpio_irq(rtk_gpio_id (group, i), 0);

                i++;
                status >>= 1;
            }
            gpioNr += 31;
            reg_ofst += 4;
        }

        /* assert */
        gpioNr = 0;
        reg_ofst = 0;
        while (gpioNr < GPIO_CNT)
        {
            status = gpio_ioread32(UMSK_ISR_GPA + reg_ofst) >> 1;
            if (status == 0)
            {
                gpioNr += 31;
                reg_ofst += 4;
                continue;
            }

            gpio_iowrite32(UMSK_ISR_GPA + reg_ofst,status << 1);
            i = gpioNr;

            while (status && i < GPIO_CNT)
            {
                if ((status & 0x1)&&rtk_gpio_chk_irq_enable(rtk_gpio_id(group, i)))
                    _do_rtk_gpio_irq( rtk_gpio_id(group, i), 1);

                i++;
                status >>= 1;
            }

            gpioNr += 31;
            reg_ofst += 4;
        }
    }

    gpio_iowrite32(GPIO_ISR,event);

    return IRQ_HANDLED;
}

/*
 * Internal Used API
 */

static void __inline _rtk_gpio_disable_irq_all(void)
{
    int i;

    for (i = 0; i < MIS_GPIO_REG_SETS; i++){
        if(i >= 1){
            gpio_iowrite32( (MIS_GP0IE_reg + 4) + (i << 2), 0xFFFFFFFE);
        }
        else{
            gpio_iowrite32( MIS_GP0IE_reg + (i << 2), 0xFFFFFFFE);
        }

    }

    for (i = 0; i < ISO_GPIO_REG_SETS; i++){
        gpio_iowrite32(ISO_GPIE_reg + (i << 2), 0xFFFFFFFE);
    }

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_MIS_REG_SETS; i++){
        gpio_iowrite32( GPI_MIS_GPIE_reg + (i << 2), 0xFFFFFFFE);
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_ISO_REG_SETS; i++){
        gpio_iowrite32( GPI_ISO_GPIE_reg + (i << 2), 0xFFFFFFFE);
    }
#endif
}

static void __inline _rtk_gpio_clear_isr_all(void)
{
    int i;

    for (i = 0; i < MIS_GPIO_REG_SETS; i++)
    {
        gpio_iowrite32(MIS_UMSK_ISR_GP0A_reg + (i << 2), 0xFFFFFFFE);
        gpio_iowrite32(MIS_UMSK_ISR_GP0DA_reg + (i << 2), 0xFFFFFFFE);
    }

    for (i = 0; i < ISO_GPIO_REG_SETS; i++)
    {
        gpio_iowrite32(ISO_ISR_GP0A_reg + (i << 2), 0xFFFFFFFE);
        gpio_iowrite32(ISO_ISR_GP0DA_reg + (i << 2), 0xFFFFFFFE);
    }

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_MIS_REG_SETS; i++)
    {
        gpio_iowrite32(GPI_MIS_ISR_ASSERT_reg + (i << 2), 0xFFFFFFFE);
        gpio_iowrite32(GPI_MIS_ISR_DEASSERT_reg + (i << 2), 0xFFFFFFFE);
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_ISO_REG_SETS; i++)
    {
        gpio_iowrite32(GPI_ISO_ISR_ASSERT_reg + (i << 2), 0xFFFFFFFE);
        gpio_iowrite32(GPI_ISO_ISR_DEASSERT_reg + (i << 2), 0xFFFFFFFE);
    }
#endif
}



int rtk_gpio_set_irq_polarity(RTK_GPIO_ID gid, unsigned char positive)
{
    unsigned int offset;
    unsigned char bit;

    if( _get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) {
    case MIS_GPIO:
        iowrite_reg_bit(MIS_GP0DP_reg + offset, bit, positive);
        return 0;

    case ISO_GPIO:
        iowrite_reg_bit(ISO_GPDP_reg + offset, bit, positive);
        return 0;

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        iowrite_reg_bit(GPI_MIS_DP_reg + offset, bit, positive);
        return 0;
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        iowrite_reg_bit(GPI_ISO_DP_reg + offset, bit, positive);
        return 0;
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_set_irq_polarity);

int rtk_gpio_set_irq_trigger_type(RTK_GPIO_ID gid, RTK_GPIO_TRIGGER type)
{
    RTK_GPIO_POLARITY polarity=GPIO_LOW_ACTIVE;

    RTK_GPIO_GLOBAL_INTSEL global_int_sel = MIS_GPIO_GP_INT_SEL_get_intsel(gpio_ioread32(MIS_GP_INT_SEL_reg));
    if(type==GPIO_RISING_EDGE){
        if(global_int_sel==GPIO_DISASSERT_INTERRUPT){
            polarity=GPIO_LOW_ACTIVE;
        }else{
            polarity=GPIO_HIGH_ACTIVE;
        }
    }else{
        if(global_int_sel==GPIO_DISASSERT_INTERRUPT){
            polarity=GPIO_HIGH_ACTIVE;
        }else{
            polarity=GPIO_LOW_ACTIVE;
        }
    }
	return rtk_gpio_set_irq_polarity(gid,polarity);
}

EXPORT_SYMBOL(rtk_gpio_set_irq_trigger_type);

int rtk_gpio_set_irq_enable(RTK_GPIO_ID gid, unsigned char On)
{
    unsigned int offset;
    unsigned char bit;

    if (_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ ,gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }
    rtk_gpio_set_dis_irq_enable(gid,On);

    switch (gpio_group(gid)) 
    {
    case MIS_GPIO:
        if(offset >= 4){
            iowrite_reg_bit((MIS_GP0IE_reg + 4) + offset,bit, On);
        }
        else{
            iowrite_reg_bit(MIS_GP0IE_reg + offset,bit, On);
        }

        return 0;

    case ISO_GPIO:
        if(0 != rtk_gpio_scpu_iso_irq_unmask(gid))
            return -EFAULT;
        iowrite_reg_bit(ISO_GPIE_reg + offset,bit, On);
        return 0;

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        iowrite_reg_bit(GPI_MIS_GPIE_reg + offset,bit, On);
        return 0;
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        iowrite_reg_bit(GPI_ISO_GPIE_reg + offset,bit, On);
        return 0;
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_set_irq_enable);

int rtk_gpio_chk_irq_enable(RTK_GPIO_ID gid)
{
    unsigned int offset;
    unsigned char bit;

    if (_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid)) 
    {
    case MIS_GPIO:
        if(offset >= 4){
            return ioread_reg_bit((MIS_GP0IE_reg + 4) + offset, bit);
        }
        else{
            return ioread_reg_bit(MIS_GP0IE_reg + offset, bit);
        }

    case ISO_GPIO:
        return ioread_reg_bit(ISO_GPIE_reg + offset,bit);

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        return ioread_reg_bit(GPI_MIS_GPIE_reg + offset,bit);
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        return ioread_reg_bit(GPI_ISO_GPIE_reg + offset,bit);
#endif

    default:
        break;
    }

    return 0;
}

EXPORT_SYMBOL(rtk_gpio_chk_irq_enable);

int rtk_gpio_set_dis_irq_enable(RTK_GPIO_ID gid, unsigned char On)
{
    unsigned int offset;
    unsigned char bit;

    if (_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ ,gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        iowrite_reg_bit(MIS_GPIO_GP0IE_reg + offset,bit, On);

        return 0;

    case ISO_GPIO:
        if(0 != rtk_gpio_scpu_iso_irq_unmask(gid))
            return -EFAULT;
        iowrite_reg_bit(GPIO_GPIE_reg + offset,bit, On);
        return 0;

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        iowrite_reg_bit(MIS_GPIO_GPI0_GPIE_reg + offset,bit, On);
        return 0;
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        iowrite_reg_bit(GPI_ISO_GPIE_reg + offset,bit, On);
        return 0;
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_set_dis_irq_enable);

int rtk_gpio_chk_dis_irq_enable(RTK_GPIO_ID gid)
{
    unsigned int offset;
    unsigned char bit;

    if (_get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        if(offset >= 4){
            return ioread_reg_bit((MIS_GPIO_GP0IE_reg + 4) + offset, bit);
        }
        else{
            return  ioread_reg_bit(MIS_GPIO_GP0IE_reg + offset, bit);
        }

    case ISO_GPIO:
        return ioread_reg_bit(GPIO_GPIE_reg + offset,bit);

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        return ioread_reg_bit(MIS_GPIO_GPI0_GPIE_reg + offset,bit);
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        return ioread_reg_bit(GPI_ISO_GPIE_reg + offset,bit);
#endif

    default:
        break;
    }

    return 0;
}

EXPORT_SYMBOL(rtk_gpio_chk_dis_irq_enable);

int rtk_gpio_clear_isr(RTK_GPIO_ID gid)
{
    unsigned int offset;
    unsigned char bit;

    if (_get_int_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
            __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    switch (gpio_group(gid))
    {
    case MIS_GPIO:
        iowrite_reg_bit(MIS_UMSK_ISR_GP0A_reg +offset, bit, 0);
        iowrite_reg_bit(MIS_UMSK_ISR_GP0DA_reg + offset, bit, 0);
        return 0;

    case ISO_GPIO:
        iowrite_reg_bit(ISO_ISR_GP0A_reg + offset, bit, 0);
        iowrite_reg_bit(ISO_ISR_GP0DA_reg + offset, bit, 0);
        return 0;

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    case MIS_GPI:
        iowrite_reg_bit(GPI_MIS_ISR_ASSERT_reg + offset, bit, 0);
        iowrite_reg_bit(GPI_MIS_ISR_DEASSERT_reg +offset, bit, 0);
        return 0;
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    case ISO_GPI:
        iowrite_reg_bit(GPI_ISO_ISR_ASSERT_reg + offset, bit, 0);
        iowrite_reg_bit(GPI_ISO_ISR_DEASSERT_reg +offset, bit, 0);
        return 0;
#endif

    default:
        break;
    }

    return -EFAULT;
}

EXPORT_SYMBOL(rtk_gpio_clear_isr);


/*
 * for Power Management
 */


#ifdef CONFIG_PM
#ifdef CONFIG_HIBERNATION
static int rtk_gpio_pm_suspend_noirq_std (struct device *dev)
{
    int i = 0;
    RTK_GPIO_WARNING("power std suspend \033[1;31m  [sttest]%s,%s ,%d \n\033[m",
                    __FILE__,__FUNCTION__ ,__LINE__);

#ifdef ENABLE_GPIO_STR

    /*** backup gpio enable configuration.... ***/
    for (i = 0; i < MIS_GPIO_REG_SETS; i++)
    {
        if(i >= 1){
            backup_mis_gpie[i] = gpio_ioread32((MIS_GP0IE_reg + 4) + (i << 2));
        }
        else{
            backup_mis_gpie[i] = gpio_ioread32(MIS_GP0IE_reg + (i << 2));
        }

        backupMisGpioConfig[i].direction = gpio_ioread32(MIS_GP0DIR_reg + (i << 2));
        backupMisGpioConfig[i].outputValue= gpio_ioread32(MIS_GP0DATO_reg + (i << 2));

        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][MISC_GPIO]  dir      0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DIR_reg + (i << 2)) , gpio_ioread32( MIS_GP0DIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][MISC_GPIO]  output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DATO_reg + (i << 2)) , gpio_ioread32( MIS_GP0DATO_reg + (i << 2))  );
        }
    }

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_MIS_REG_SETS; i++)
    {
        backupGpoMisConfig[i].outputValue= gpio_ioread32(MIS_GPO_DATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_MISC]  output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GPO_DATO_reg + (i << 2)) , gpio_ioread32( MIS_GPO_DATO_reg + (i << 2))  );
    }
#endif
/*
    for (i = 0; i < ISO_GPIO_REG_SETS; i++)
    {
        backup_iso_gpie[i] = gpio_ioread32(ISO_GPIE_reg + (i << 2));
        backupIsoGpioConfig[i].direction = gpio_ioread32(ISO_GPDIR_reg + (i << 2));
        backupIsoGpioConfig[i].outputValue= gpio_ioread32(ISO_GPDATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][ISO_GPIO]  dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDIR_reg + (i << 2)) , gpio_ioread32( ISO_GPDIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][ISO_GPIO]  output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDATO_reg + (i << 2)) , gpio_ioread32( ISO_GPDATO_reg + (i << 2))  );
        }
    }
*/
#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_ISO_REG_SETS; i++)
    {
        backupGpoIsoConfig[i].outputValue= gpio_ioread32(ISO_GPO_DATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_ISO]  output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPO_DATO_reg + (i << 2)) , gpio_ioread32( ISO_GPO_DATO_reg + (i << 2))  );
    }
#endif

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_MIS_REG_SETS; i++)
    {
        backup_gpi_mis_gpie[i] = gpio_ioread32(GPI_MIS_GPIE_reg + (i << 2));
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_ISO_REG_SETS; i++)
    {
        backup_gpi_iso_gpie[i] = gpio_ioread32(GPI_ISO_GPIE_reg + (i << 2));
    }
#endif

#endif

    _rtk_gpio_disable_irq_all();

    return 0;
}



static int rtk_gpio_pm_resume_noirq_std(struct device *dev)
{
       //resume code here
    int i = 0;

    RTK_GPIO_WARNING("power std reusme\033[1;31m  [sttest]%s,%s ,%d \n\033[m",
        __FILE__,__FUNCTION__ ,__LINE__);

    _rtk_gpio_clear_isr_all();
    PowerOnGPIO();
    
#ifdef ENABLE_GPIO_STR

    for (i = 0; i < MIS_GPIO_REG_SETS; i++)
    {
        if(i >= 1){
            gpio_iowrite32_protect( (MIS_GP0IE_reg + 4) + (i << 2), backup_mis_gpie[i]);
        }
        else{
            gpio_iowrite32_protect( MIS_GP0IE_reg + (i << 2), backup_mis_gpie[i]);
        }

        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][MISC_GPIO] pre dir      0x%X = 0x%08x.\n",
                            __func__ , ( MIS_GP0DIR_reg + (i << 2)) , gpio_ioread32( MIS_GP0DIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][MISC_GPIO] pre output 0x%X = 0x%08x.\n",
                            __func__ , ( MIS_GP0DATO_reg + (i << 2)) , gpio_ioread32( MIS_GP0DATO_reg + (i << 2))  );
        }

        gpio_iowrite32_protect( MIS_GP0DATO_reg  + (i << 2), backupMisGpioConfig[i].outputValue);
        gpio_iowrite32_protect( MIS_GP0DIR_reg + (i << 2), backupMisGpioConfig[i].direction);

        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][MISC_GPIO] after dir      0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DIR_reg + (i << 2)) , gpio_ioread32( MIS_GP0DIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][MISC_GPIO] after output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DATO_reg + (i << 2)) , gpio_ioread32( MIS_GP0DATO_reg + (i << 2))  );
        }

    }

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_MIS_REG_SETS; i++){
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_MISC] pre output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GPO_DATO_reg + (i << 2)) , gpio_ioread32( MIS_GPO_DATO_reg + (i << 2))  );

        gpio_iowrite32_protect( MIS_GPO_DATO_reg  + (i << 2), backupGpoMisConfig[i].outputValue);

        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_MISC] after output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GPO_DATO_reg + (i << 2)) , gpio_ioread32( MIS_GPO_DATO_reg + (i << 2))  );
    }
#endif
/*
    for (i = 0; i < ISO_GPIO_REG_SETS; i++){
        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][ISO_GPIO] pre dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDIR_reg + (i << 2)) , gpio_ioread32( ISO_GPDIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][ISO_GPIO] pre output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDATO_reg + (i << 2)) , gpio_ioread32( ISO_GPDATO_reg + (i << 2))  );
        }
        gpio_iowrite32_protect( ISO_GPIE_reg + (i << 2), backup_iso_gpie[i]);
        gpio_iowrite32_protect( ISO_GPDATO_reg + (i << 2), backupIsoGpioConfig[i].outputValue);
        gpio_iowrite32_protect( ISO_GPDIR_reg + (i << 2), backupIsoGpioConfig[i].direction);

        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][ISO_GPIO] after dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDIR_reg + (i << 2)) , gpio_ioread32( ISO_GPDIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][ISO_GPIO] after output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDATO_reg + (i << 2)) , gpio_ioread32( ISO_GPDATO_reg + (i << 2))  );
        }
    }
*/
#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_ISO_REG_SETS; i++)
    {
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_ISO] pre dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPO_DATO_reg + (i << 2)) , gpio_ioread32( ISO_GPO_DATO_reg + (i << 2))  );

        gpio_iowrite32_protect( ISO_GPO_DATO_reg + (i << 2), backupGpoIsoConfig[i].outputValue );

        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_ISO] after dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPO_DATO_reg + (i << 2)) , gpio_ioread32( ISO_GPO_DATO_reg + (i << 2))  );

    }
#endif

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_MIS_REG_SETS; i++)
    {
        gpio_iowrite32_protect( GPI_MIS_GPIE_reg + (i << 2), backup_gpi_mis_gpie[i]);
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_ISO_REG_SETS; i++)
    {
        gpio_iowrite32_protect( GPI_ISO_GPIE_reg + (i << 2), backup_gpi_iso_gpie[i]);
    }
#endif

#endif

    return 0;
}
#endif

static int rtk_gpio_pm_suspend_noirq (struct device *dev)
{
    int i = 0;
    u32 reginfo = 0;

    RTK_GPIO_WARNING("power str suspend\n");

#ifdef MIS_ISR_reg
    reginfo = gpio_ioread32((MIS_ISR_reg) ) & ~(0x1 << MIS_ISR_gpio_int_shift);
    gpio_iowrite32(MIS_ISR_reg,reginfo);
#endif

#ifdef ISO_ISR_reg
    reginfo = gpio_ioread32(ISO_ISR_reg) & ~(0x1 << ISO_ISR_gpio_int_shift);
    gpio_iowrite32(ISO_ISR_reg, reginfo);
#endif



#ifdef ENABLE_GPIO_STR
    /* backup gpio enable configuration.... */

    for (i = 0; i < MIS_GPIO_REG_SETS; i++)
    {
        if(i >= 1){
            backup_mis_gpie[i] = gpio_ioread32((MIS_GP0IE_reg + 4) + (i << 2));
        }
        else{
            backup_mis_gpie[i] = gpio_ioread32(MIS_GP0IE_reg + (i << 2));
        }

        backupMisGpioConfig[i].direction = gpio_ioread32(MIS_GP0DIR_reg + (i << 2));
        backupMisGpioConfig[i].outputValue= gpio_ioread32(MIS_GP0DATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG ){
            RTK_GPIO_WARNING("[%s][MISC_GPIO]  dir      0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DIR_reg + (i << 2)) , gpio_ioread32( MIS_GP0DIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][MISC_GPIO]  output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DATO_reg + (i << 2)) , gpio_ioread32( MIS_GP0DATO_reg + (i << 2))  );
        }

    }

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_MIS_REG_SETS; i++){
        backupGpoMisConfig[i].outputValue= gpio_ioread32(MIS_GPO_DATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_MISC]  output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GPO_DATO_reg + (i << 2)) , gpio_ioread32( MIS_GPO_DATO_reg + (i << 2))  );

    }
#endif
/*
    for (i = 0; i < ISO_GPIO_REG_SETS; i++)
    {
        backup_iso_gpie[i] = gpio_ioread32(ISO_GPIE_reg + (i << 2));
        backupIsoGpioConfig[i].direction = gpio_ioread32(ISO_GPDIR_reg + (i << 2));
        backupIsoGpioConfig[i].outputValue= gpio_ioread32(ISO_GPDATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][ISO_GPIO]  dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDIR_reg + (i << 2)) , gpio_ioread32( ISO_GPDIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][ISO_GPIO]  output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDATO_reg + (i << 2)) , gpio_ioread32( ISO_GPDATO_reg + (i << 2))  );
        }
    }
*/
#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_ISO_REG_SETS; i++){
        backupGpoIsoConfig[i].outputValue= gpio_ioread32(ISO_GPO_DATO_reg + (i << 2));
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_ISO]  output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPO_DATO_reg + (i << 2)) , gpio_ioread32( ISO_GPO_DATO_reg + (i << 2))  );

    }
#endif

#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_MIS_REG_SETS; i++)
    {
        backup_gpi_mis_gpie[i] = gpio_ioread32(GPI_MIS_GPIE_reg + (i << 2));
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_ISO_REG_SETS; i++)
    {
        backup_gpi_iso_gpie[i] = gpio_ioread32(GPI_ISO_GPIE_reg + (i << 2));
    }
#endif

#endif

    _rtk_gpio_disable_irq_all();

    return 0;
}



static int rtk_gpio_pm_resume_noirq(struct device *dev)
{
        //resume code here
    int i = 0;
    u32 reginfo = 0;

    RTK_GPIO_WARNING("power str reusme\n");
    _rtk_gpio_clear_isr_all();

#ifdef ENABLE_GPIO_STR     // only for hmdi setting //keep hpd low while resume back.
    //backupIsoGpioConfig[0].outputValue  = backupIsoGpioConfig[0].outputValue | ( 0x1 << 6  )  | ( 0x1 << 11  )  | ( 0x1 << 14  )  | ( 0x1 << 18  );
#endif

#ifdef ENABLE_GPIO_STR
/* backup gpio enable configuration.... */
    for (i = 0; i < MIS_GPIO_REG_SETS; i++){
        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][MISC_GPIO] pre dir      0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DIR_reg + (i << 2)) , gpio_ioread32( MIS_GP0DIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][MISC_GPIO] pre output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DATO_reg + (i << 2)) , gpio_ioread32( MIS_GP0DATO_reg + (i << 2))  );
        }

        gpio_iowrite32_protect( MIS_GP0DATO_reg  + (i << 2), backupMisGpioConfig[i].outputValue);
        gpio_iowrite32_protect( MIS_GP0DIR_reg   + (i << 2), backupMisGpioConfig[i].direction);

        if(i >= 1){
            gpio_iowrite32_protect( (MIS_GP0IE_reg + 4)+ (i << 2), backup_mis_gpie[i]);
        }
        else{
            gpio_iowrite32_protect( MIS_GP0IE_reg  + (i << 2), backup_mis_gpie[i]);
        }

        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][MISC_GPIO] after dir      0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DIR_reg + (i << 2)) , gpio_ioread32( MIS_GP0DIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][MISC_GPIO] after output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GP0DATO_reg + (i << 2)) , gpio_ioread32( MIS_GP0DATO_reg + (i << 2))  );
        }

    }

#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_MIS_REG_SETS; i++)
    {
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_MISC] pre output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GPO_DATO_reg + (i << 2)) , gpio_ioread32( MIS_GPO_DATO_reg + (i << 2))  );

        gpio_iowrite32_protect( MIS_GPO_DATO_reg  + (i << 2), backupGpoMisConfig[i].outputValue);

        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][MISC_GPO] after output 0x%X = 0x%08x.\n",
                __func__ , ( MIS_GPO_DATO_reg + (i << 2)) , gpio_ioread32( MIS_GPO_DATO_reg + (i << 2))  );

    }
#endif
/*
    for (i = 0; i < ISO_GPIO_REG_SETS; i++){
        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][ISO_GPIO] pre dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDIR_reg + (i << 2)) , gpio_ioread32( ISO_GPDIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][ISO_GPIO] pre output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDATO_reg + (i << 2)) , gpio_ioread32( ISO_GPDATO_reg + (i << 2))  );
        }
        gpio_iowrite32_protect( ISO_GPDATO_reg  + (i << 2), backupIsoGpioConfig[i].outputValue);
        gpio_iowrite32_protect( ISO_GPDIR_reg   + (i << 2), backupIsoGpioConfig[i].direction);
        gpio_iowrite32_protect( ISO_GPIE_reg  + (i << 2), backup_iso_gpie[i]);

        if( bShowStrMessage == Enable_MSG )
        {
            RTK_GPIO_WARNING("[%s][ISO_GPIO] after dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDIR_reg + (i << 2)) , gpio_ioread32( ISO_GPDIR_reg + (i << 2))  );
            RTK_GPIO_WARNING("[%s][ISO_GPIO] after output 0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPDATO_reg + (i << 2)) , gpio_ioread32( ISO_GPDATO_reg + (i << 2))  );
        }
    }
*/
#ifdef ENABLE_GPO_TYPE_BACKUP_CONFIG
    for (i = 0; i < GPO_ISO_REG_SETS; i++){
        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_ISO] pre dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPO_DATO_reg + (i << 2)) , gpio_ioread32( ISO_GPO_DATO_reg + (i << 2))  );

        gpio_iowrite32_protect( ISO_GPO_DATO_reg  + (i << 2), backupGpoIsoConfig[i].outputValue);

        if( bShowStrMessage == Enable_MSG )
            RTK_GPIO_WARNING("[%s][GPO_ISO] after dir      0x%X = 0x%08x.\n",
                __func__ , ( ISO_GPO_DATO_reg + (i << 2)) , gpio_ioread32( ISO_GPO_DATO_reg + (i << 2))  );

    }
#endif


#ifdef RTK_SUPPORT_MIS_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_MIS_REG_SETS; i++)
    {
        gpio_iowrite32_protect(GPI_MIS_GPIE_reg  + (i << 2), backup_gpi_mis_gpie[i]);
    }
#endif

#ifdef RTK_SUPPORT_ISO_GPI_BACKUP_CONFIG
    for (i = 0; i < GPI_ISO_REG_SETS; i++)
    {
        gpio_iowrite32_protect( GPI_ISO_GPIE_reg  + (i << 2), backup_gpi_iso_gpie[i]);
    }
#endif

#endif


#ifdef MIS_ISR_reg
    reginfo = gpio_ioread32(MIS_ISR_reg) | (0x1 << MIS_ISR_gpio_int_shift);

    gpio_iowrite32(MIS_ISR_reg, reginfo);
#endif

#ifdef ISO_ISR_reg
    reginfo = gpio_ioread32(ISO_ISR_reg) | (0x1 << ISO_ISR_gpio_int_shift);

    gpio_iowrite32(ISO_ISR_reg, reginfo);
#endif

    PowerOnGPIO();


    return 0;
}
#endif


static const struct dev_pm_ops gpio_pm_ops = 
{
    .suspend_noirq = rtk_gpio_pm_suspend_noirq,
    .resume_noirq  = rtk_gpio_pm_resume_noirq,

#ifdef CONFIG_HIBERNATION
    .freeze_noirq = rtk_gpio_pm_suspend_noirq_std,
    .thaw_noirq   = rtk_gpio_pm_resume_noirq_std,

    .poweroff_noirq = rtk_gpio_pm_suspend_noirq_std,
    .restore_noirq  = rtk_gpio_pm_resume_noirq_std,
#endif

};

static int rtk_gpio_probe(struct platform_device *pdev)
{
    int iso_irq, misc_irq, ret;

    misc_irq = platform_get_irq(pdev, 0);
    iso_irq = platform_get_irq(pdev, 1);
    if (misc_irq < 0)
    {
        RTK_GPIO_DBG("[%s] get misc irq num:%d failed\n", __func__, misc_irq);
    }
    else if (request_irq(misc_irq, gpio_interrupt_handler,IRQF_SHARED, "MIS_GPIO", (void *)~MIS_GPIO) < 0)
    {
        RTK_GPIO_DBG("[%s] request misc hwirq %lu failed\n", __func__,
                irqd_to_hwirq(irq_get_irq_data(misc_irq)));
        ret = -EIO;
        return ret;
    }

    if(ISO_GPIO_CNT)
    {
        if (iso_irq < 0)
        {
            RTK_GPIO_DBG("[%s] get iso irq num:%d failed\n", __func__, iso_irq);
        }
        else if(request_irq(iso_irq, gpio_interrupt_handler, IRQF_SHARED, "ISO_GPIO",(void *)~ISO_GPIO) < 0)
        {
            RTK_GPIO_DBG("[%s] request iso hwirq %lu failed\n", __func__,
                irqd_to_hwirq(irq_get_irq_data(iso_irq)));
            ret = -EIO;
            goto err_request_iso_irq_failed;
        }

    #ifdef ISO_ISR_reg
        /* enable scpu gpio interrupt */
        gpio_iowrite32(ISO_ISR_reg, gpio_ioread32(ISO_ISR_reg) | (0x1 << ISO_ISR_gpio_int_shift));
    #endif
    }

    if (GPI_ISO_CNT)
    {
        if (iso_irq < 0)
        {
            RTK_GPIO_DBG("[%s] get iso irq num:%d failed\n", __func__, iso_irq);
        }
        else if (request_irq(iso_irq, gpio_interrupt_handler,IRQF_SHARED, "GPI_ISO", (void *)~ISO_GPI) < 0)
        {
            RTK_GPIO_DBG("[%s] request iso hwirq %lu failed\n", __func__,
                irqd_to_hwirq(irq_get_irq_data(iso_irq)));
            ret = -EIO;
            goto err_request_iso_gpi_irq_failed;
        }

    #ifdef GPI_ISO_ISR_reg
            /* enable scpu gpio interrupt */
            gpio_iowrite32(GPI_ISO_ISR_reg, gpio_ioread32(GPI_ISO_ISR_reg) |(0x1 << GPI_ISO_ISR_int_shift));
    #endif
    }

    return 0;

err_request_iso_gpi_irq_failed:
    if (ISO_GPIO_CNT)
    {
        free_irq(iso_irq, (void *)~ISO_GPIO);
    }

err_request_iso_irq_failed:
    if (misc_irq >= 0)
        free_irq(misc_irq, (void *)~MIS_GPIO);
    return ret;
}

#ifdef CONFIG_OF
static const struct of_device_id rtk_gpio_device_id[] = {
    {.compatible = "realtek, rtk-gpio",},
    {},
};
#endif

static int rtk_gpio_remove(struct platform_device *pdev)
{
    int misc_irq = platform_get_irq(pdev, 0);
    int iso_irq = platform_get_irq(pdev, 1);

    if (misc_irq >= 0)
        free_irq(misc_irq, (void *)~MIS_GPIO);

    if (iso_irq >= 0) {
        if (ISO_GPIO_CNT)
            free_irq(iso_irq, (void *)~ISO_GPIO);

        if (GPI_ISO_CNT)
            free_irq(iso_irq, (void *)~ISO_GPI);
    }

    return 0;
}

static struct platform_driver rtk_gpio_platform_driver =
{
    .driver = {
        .name = "rtk_gpio",
        .bus = &platform_bus_type,
#ifdef CONFIG_PM
        .pm = &gpio_pm_ops,
#endif
#ifdef CONFIG_OF
        .of_match_table = rtk_gpio_device_id,
#endif
    },
    .probe = rtk_gpio_probe,
    .remove = rtk_gpio_remove,
};

int __init rtk_gpio_init(void)
{
    int ret;
	
    /*parser all gpio pcb enmu*/
    rtk_gpio_init_pcb_pin();

    memset(rtk_gpio_irq, 0, sizeof(rtk_gpio_irq));

    _rtk_gpio_disable_irq_all();    /* disable all interrupts */
    _rtk_gpio_clear_isr_all();  /* clear all interrupts status*/

    PowerOnGPIO();
#ifdef MIS_ISR_reg
    /* enable scpu gpio interrupt */
    gpio_iowrite32(MIS_ISR_reg, gpio_ioread32(MIS_ISR_reg) | (0x1 << MIS_ISR_gpio_int_shift));
#endif

    /* set GPIO  global interrupt to :*/
    gpio_iowrite32(MIS_GP_INT_SEL_reg, GPIO_BOTH_INTERRUPT); // set for both direction

    if (platform_driver_register(&rtk_gpio_platform_driver) != 0)
    {
        RTK_GPIO_DBG("rtk_gpio_init failed, register platform device failed\n");
        ret = -EFAULT;
        goto err_register_platform_driver_failed;
    }

    rtk_gpio_register_gpio_chipset();

#ifdef CONFIG_RTK_KDRV_GPIO_BUILT_IN_TEST
    rtk_gpio_built_in_test();
#endif

    return 0;

err_register_platform_driver_failed:

    return ret;
}

void __exit rtk_gpio_exit(void)
{
    _rtk_gpio_disable_irq_all();
    _rtk_gpio_clear_isr_all();      /* clear all interrupts status*/

    platform_driver_unregister(&rtk_gpio_platform_driver);
}

void rtk_gpio_lg_hal_int_isr(RTK_GPIO_ID gid, unsigned char assert, void* dev_id)
{
    //RTK_GPIO_WARNING("%s GPIO %d assert=%d, val=%d\n",
    //    gpio_type(gpio_group(gid)), gpio_idx(gid),
    //    assert, rtk_gpio_input(pin));
    //unsigned long flags;

        //spin_lock_irqsave(&rtk_gpio_irq_lock, flags);

    g_lg_hal_int_flag = 1;

    //spin_unlock_irqrestore(&rtk_gpio_irq_lock, flags);

}

EXPORT_SYMBOL(rtk_gpio_lg_hal_int_isr);


int rtk_gpio_get_irq_flag( RTK_GPIO_ID gid, unsigned char positive )
{

    unsigned long flags;
    int ret = 0;

    spin_lock_irqsave(&rtk_gpio_irq_lock, flags);

    if( 1 == g_lg_hal_int_flag )
    {
        g_lg_hal_int_flag = 0;
        rtk_gpio_free_irq(gid , rtk_gpio_lg_hal_int_isr );
        ret = 1;
    }
    else
    {
        ret = 0;
    }

    spin_unlock_irqrestore(&rtk_gpio_irq_lock, flags);

    return ret;
}

/*for new gpio function*/
void rtk_gpio_rtk_int_isr(RTK_GPIO_ID gid, unsigned char assert, void* dev_id)
{
    RTK_GPIO_IRQ *p_irq = _get_gpio_entry(gid);
    if (p_irq)
    {
        p_irq->trigger_type = assert;
        p_irq->irq_flag = 1;
    }
}

EXPORT_SYMBOL(rtk_gpio_rtk_int_isr);

int rtk_gpio_get_irq_flag_ex( RTK_GPIO_ID gid, unsigned char trigger_type )
{
    int ret = 0;
    RTK_GPIO_IRQ *p_irq = _get_gpio_entry(gid);

    if (p_irq)
    {
        if((trigger_type == GPIO_BOTH_EDGE) ||(trigger_type == p_irq->trigger_type)){
            ret = p_irq->irq_flag;
            if(p_irq->irq_flag){
                //rtk_gpio_free_irq(gid , rtk_gpio_rtk_int_isr );
                p_irq->irq_flag = 0;
            }
        }
    }
    return ret;
}


static bool _is_iso_gpio_forScpu(RTK_GPIO_ID gid)
{
    int i;
    for(i = 0; i < MAX_PCB_GPIO_COUNT; i++){
        if(pPcbInfo->pcblist[i].gid == gid){
           if(pPcbInfo->pcblist[i].pin_type == PCB_PIN_TYPE_EMCU_GPIO)
            return false;
        }else
            return true;
    }
    return false;
}

int rtk_gpio_scpu_iso_irq_unmask(RTK_GPIO_ID gid)
{

    unsigned int offset;
    unsigned char bit;

    if(gpio_group(gid) != ISO_GPIO)
    {
        RTK_GPIO_WARNING("%s gid type group do not match \n", __func__ );
        return -1;
    }

    if(!_is_iso_gpio_forScpu(gid)){
        RTK_GPIO_WARNING("%s gid =%d used by EMCU\n", __func__ ,gid);
        return -1;
    }

    if( _get_reg_ofst_bit(gid,&offset,&bit) !=0)
    {
        RTK_GPIO_WARNING("%s failed  %s  index = %d \n" ,
                __func__ , gpio_type(gpio_group(gid)), gpio_idx(gid) );
        return -EFAULT;
    }

    iowrite_reg_bit(ISO_GPINT_SCPU_reg + offset,bit, 1);

    return 0;

}

void rtk_gpio_syncInfo_toDB(void)
{
    int i;
    GPIO_MAP_ENTRY *entry;
    entry = (GPIO_MAP_ENTRY*)kzalloc((pPcbInfo->pcbcnt)*sizeof(GPIO_MAP_ENTRY),GFP_KERNEL);
    if (unlikely(!entry)) {
        RTK_GPIO_WARNING("%s Out of memory \n", __func__);
        return;
    }

    for(i = 0;i < pPcbInfo->pcbcnt; i++){
        strncpy(entry[i].Name, pPcbInfo->pcblist[i].pin_name, MAX_GPIO_MAP_ENTRY_NAME_LEN-1);
        entry[i].Type = pPcbInfo->pcblist[i].gpio_type;
        entry[i].Usage = 
            (pPcbInfo->pcblist[i].input_type == PCB_GPIO_TYPE_INPUT) ? GPIO_ALLOW_RD : GPIO_ALLOW_WR;
        entry[i].Number = pPcbInfo->pcblist[i].pin_index;
        rtk_gpio_array_add(&RTK_GPIO_DB,&entry[i]);
    }

    kfree(entry);
}

MODULE_AUTHOR("Chih-pin Wu, Realtek Semiconductor");
MODULE_LICENSE("GPL");

#else
int rt_gpio_get(int gpioNum, RT_GPIO_TYPE rtGpioType)
{
    RTK_GPIO_ID gid;
    gid = rtk_gpio_id(rtGpioType, gpioNum);

    rtk_gpio_set_dir(gid, 0);

    return rtk_gpio_input(gid);
}

int rt_gpio_set(int gpioNum, int value, RT_GPIO_TYPE rtGpioType)
{
    RTK_GPIO_ID gid;
    gid = rtk_gpio_id(rtGpioType, gpioNum);

    rtk_gpio_set_dir(gid, 1);
    return rtk_gpio_output(gid, value);
}

int  rtk_gpio_init(void)
{
    int ret;
	
    /*parser all gpio pcb enmu*/
    rtk_gpio_init_pcb_pin();
    PowerOnGPIO();

    return 0;
}

#endif

