#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/irq.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/interrupt.h>


#include <mach/platform.h>
#include <rtd_log/rtd_module_log.h>
#include <rbus/sys_reg_reg.h>
#include <rtk_kdriver/rtk_crt.h>
#include <rtk_kdriver/io.h>

#include <rtk_kdriver/rtk_misc_interrupt.h>


static spinlock_t   isoSpinLock;
static spinlock_t  miscSpinLock;

static int         isoIpRecord = 0;
static int        miscIpRecord = 0;
static int    bInitIsoSpinLock = 0;
static int   bInitMiscSpinLock = 0;


char * getIsoIpName(int maskIp)
{
    switch(maskIp)
    {
    case ISO_EMPTY: return "ISO_EMPTY"  ; break;
    case ISO_UART : return "ISO_UART"   ; break;
    case ISO_I2C  : return "ISO_I2C"    ; break;
    case ISO_GPIO : return "ISO_GPIO"   ; break;
    default:        return "unknown IP" ; break;
    }

/* coverity check: Structurally dead code (UNREACHABLE), so mark it.
    return "unknown IP";
*/
}

char * getMiscIpName(int maskIp)
{
    switch(maskIp)
    {
    case MISC_EMPTY: return "MISC_EMPTY" ; break;
    case MISC_UART : return "ISO_UART"   ; break;
    case MISC_I2C  : return "ISO_I2C"    ; break;
    case MISC_GPIO : return "ISO_GPIO"   ; break;
    default:         return "unknown IP" ; break;
    }

/* coverity check: Structurally dead code (UNREACHABLE), so mark it.
    return "unknown IP";
*/
}


/*---------------------------------------------------
 * Func : setIsoInterrupt
 *
 * Desc : enable/disable iso ip interrupt in SYS_REG_INT_CTRL_SCPU_reg
 *
 * Para : isoIp : which iso ip to be used enable/disable interrupt
 *        en      : enable/disable hpd
 *                  0 : disable,
 *                  others : enable
 *
 * Retn : N/A
 *--------------------------------------------------*/

int setIsoInterrupt(int isoIp , int enable )
{
    MISC_INTERRUPT_WARN("%s isoIp = %s " , __func__ , getIsoIpName(isoIp));

    if( bInitIsoSpinLock == 0 )
    {
        spin_lock_init(&isoSpinLock);
        bInitIsoSpinLock = 1;
    }

    spin_lock(&isoSpinLock);

    if( enable == 0 )
    {
        isoIpRecord = isoIpRecord & (~isoIp) ;
        MISC_INTERRUPT_WARN("%s isoIpRecord = 0x%08X " , __func__ , isoIpRecord);

        if ( isoIpRecord == ISO_EMPTY )
        {
            rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_standby_iso_misc_off_int_scpu_routing_en_mask );
            MISC_INTERRUPT_WARN("%s iso interrupt final disable by %s\n" , __func__ , getIsoIpName(isoIp));
        }
    }
    else
    {
        isoIpRecord = isoIpRecord | isoIp ;

        if( SYS_REG_INT_CTRL_SCPU_get_standby_iso_misc_off_int_scpu_routing_en(rtd_inl(SYS_REG_INT_CTRL_SCPU_reg)) == 0 )
        {
            rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_standby_iso_misc_off_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data_mask );

            MISC_INTERRUPT_WARN("%s iso interrupt enable by %s\n" , __func__ , getIsoIpName(isoIp));
        }
        else
        {
            MISC_INTERRUPT_WARN("%s iso interrupt has enabled \n" , __func__ );
        }
    }

    spin_unlock(&isoSpinLock);

    return 0;
}
EXPORT_SYMBOL(setIsoInterrupt);

/*---------------------------------------------------
 * Func : setMiscInterrupt
 *
 * Desc : enable/disable misc ip interrupt in SYS_REG_INT_CTRL_SCPU_reg
 *
 * Para : miscIp : which misc ip to be used enable/disable interrupt
 *        en      : enable/disable hpd
 *                  0 : disable,
 *                  others : enable
 *
 * Retn : N/A
 *--------------------------------------------------*/

int setMiscInterrupt(int miscIp , int enable )
{
    MISC_INTERRUPT_WARN("%s miscIp = %s " , __func__ , getMiscIpName(miscIp));


    if( bInitMiscSpinLock == 0 )
    {
        spin_lock_init(&miscSpinLock);
        bInitMiscSpinLock = 1;
    }


    spin_lock(&miscSpinLock);

    if( enable == 0 )
    {
        miscIpRecord = miscIpRecord & (~miscIp) ;
        MISC_INTERRUPT_WARN("%s miscIpRecord = 0x%08X " , __func__ , miscIpRecord);

        if ( miscIpRecord == MISC_EMPTY )
        {
            rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_standby_iso_misc_int_scpu_routing_en_mask );
            MISC_INTERRUPT_WARN("%s misc interrupt final disable by %s\n" , __func__ , getMiscIpName(miscIp));
        }
    }
    else
    {
        miscIpRecord = miscIpRecord | miscIp ;

        if( SYS_REG_INT_CTRL_SCPU_get_standby_iso_misc_int_scpu_routing_en(rtd_inl(SYS_REG_INT_CTRL_SCPU_reg)) == 0 )
        {
            rtd_outl(SYS_REG_INT_CTRL_SCPU_reg, SYS_REG_INT_CTRL_SCPU_standby_iso_misc_int_scpu_routing_en_mask | SYS_REG_INT_CTRL_SCPU_write_data_mask );

            MISC_INTERRUPT_WARN("%s misc interrupt enable by %s\n" , __func__ , getMiscIpName(miscIp));
        }
        else
        {
            MISC_INTERRUPT_WARN("%s misc interrupt has enabled \n" , __func__ );
        }
    }

    spin_unlock(&miscSpinLock);

    return 0;
}
EXPORT_SYMBOL(setMiscInterrupt);



