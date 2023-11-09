#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <rtk_kdriver/io.h>
#include "rtk_ddc_config.h"
#include "rtk_ddc_drv.h"
#include "rtk_ddc_priv.h"
#include "rtk_ddc_sysfs.h"
#include "rtk_ddc_dbg.h"
#if (defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)) && defined(CONFIG_RTK_KDRV_DDCCI)
#include "rtk_ddcci_priv.h"
#include "rtk_ddcci_drv.h"
#include <linux/kthread.h>
#endif

int g_rtk_ddc_dbg = 1;
static unsigned int ddc_irq_init = 0;

static int rtk_ddc_platform_driver_probe(struct platform_device* pdev)
{
    int ret = 0;
    int irq = 0;
    RTK_DDC_WARNING("%s \n", __func__);
    
    if(ddc_irq_init == 0)
    {
        irq = platform_get_irq(pdev, 0);
        ret = rtk_ddc_drv_interrupt(irq);
        if(ret)
        {
            RTK_DDC_WARNING("%s, fail to init interrupt!!\n", __func__);
        }
        else
        {
            RTK_DDC_WARNING("%s, Request interrupt successed!! - IRQ : %d\n", __func__, irq);
            ddc_irq_init = 1;
        }
    }

    return ret;
}

static int rtk_ddc_platform_driver_remove(struct platform_device* pdev)
{
    int ret = 0;
    int irq = 0;
    RTK_DDC_WARNING("%s \n", __func__);

    if(ddc_irq_init == 1)
    {
        irq = platform_get_irq(pdev, 0);
        if (irq >= 0) 
        {
            rtk_ddc_drv_free_interrupt(irq);
            ddc_irq_init=0;
        }
    }

    return ret;
}


int rtk_ddc_lib_init(void)
{
    int i = 0;
    int ret = 0;
    srtk_ddc_chip* pchip = NULL;

    for(i=0; i<ARRAY_SIZE(rtk_ddc_chip); i++)
    {
        if(rtk_ddc_chip[i].reg_remap == NULL)
            continue;
        
        pchip = &rtk_ddc_chip[i];
        snprintf(rtk_ddc_chip[i].name,sizeof(rtk_ddc_chip[i].name),"%s.%d", RTK_DDC_NAME, rtk_ddc_chip[i].id);

        ret = rtk_ddc_sysfs_create(pchip);
        if(ret)
        {
            RTK_DDC_ERR("%s fail to create sysfs[%d]\n", __func__, i);
            return -EINVAL;
        }
        spin_lock_init(&pchip->reg_remap->g_ddc_spinlock);
    }

    return ret;
}


int rtk_ddc_lib_sysfs_destroy(void)
{
    int i = 0;
    for(i=0; i<ARRAY_SIZE(rtk_ddc_chip); i++)
    {
        rtk_ddc_sysfs_destroy(&rtk_ddc_chip[i]);
    }
    return 0;
}

static const struct of_device_id rtk_ddc_of_match[] = {
    {
        .compatible = "realtek,rtk_ddc",
    },
    {},
};

MODULE_DEVICE_TABLE(of, rtk_ddc_of_match);

static struct platform_driver rtk_ddc_platform_driver =
{
    .probe      = rtk_ddc_platform_driver_probe,
    .remove     = rtk_ddc_platform_driver_remove,
    .driver     =
    {
        .name = RTK_DDC_NAME,
        .of_match_table = rtk_ddc_of_match,
    },
};

static int rtk_ddc_platform_driver_exit(void)
{
    rtk_ddc_lib_sysfs_destroy();
    return 0;
}

static int rtk_ddc_platform_driver_init(void)
{
    int ret     = 0;
    ret = rtk_ddc_lib_init();
    if(ret)
    {
        RTK_DDC_ERR("%s, fail to init rtk ddc driver!!\n", __func__);
    }

    ret = platform_driver_register(&rtk_ddc_platform_driver);
    if(ret)
    {
        RTK_DDC_ERR("platform_driver_register failed.\n");
    }
    return ret;
}

static int rtk_ddc_init(void)
{
    int ret     = 0;
    RTK_DDC_WARNING("%s\n",__func__);

    ret = rtk_ddc_platform_driver_init();
    if(ret)
    {
        RTK_DDC_ERR("fail to init rtk ddc platform driver!!\n");
        goto fail_init_ddc_platform_drv;
    }

    ret = rtk_ddc_drv_set_scpu_init(1);
    ret = rtk_ddc_drv_set_emcu_init(0);

#if (defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)) && defined(CONFIG_RTK_KDRV_DDCCI)
    rtk_ddcci_cdev_init();
#endif

    return ret;

fail_init_ddc_platform_drv:
    rtk_ddc_platform_driver_exit();
    return ret;
}

static void rtk_ddc_exit(void)
{
    rtk_ddc_platform_driver_exit();

#if (defined(CONFIG_ARCH_RTK2851C) || defined(CONFIG_ARCH_RTK2851F)) && defined(CONFIG_RTK_KDRV_DDCCI)
    if(g_rtk_ddcci_fifo_thread)
    {
        kthread_stop(g_rtk_ddcci_fifo_thread);
        g_rtk_ddcci_fifo_thread = NULL;
    }

    if(g_rtk_ddcci_dbg_thread)
    {
        kthread_stop(g_rtk_ddcci_dbg_thread);
        g_rtk_ddcci_dbg_thread = NULL;
    }
    rtk_ddcci_cdev_uninit();
#endif
}

fs_initcall(rtk_ddc_init);
module_exit(rtk_ddc_exit);

MODULE_DESCRIPTION("Realtek DDC");
MODULE_AUTHOR("Xiao Tim <xiao_tim@realtek.com>");
MODULE_LICENSE("GPL");
