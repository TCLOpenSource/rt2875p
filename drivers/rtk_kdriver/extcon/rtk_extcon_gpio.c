#include <linux/init.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/pwm.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/stat.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <rtk_kdriver/pcbMgr.h>
#include <linux/gpio.h>
#include <rtk_kdriver/rtk_gpio.h>
#include <linux/extcon.h>
#include <linux/device.h>
#include "rtk_extcon_gpio.h"
#include "../../extcon/extcon.h"
#include "../../base/base.h"

static GPIO_EXTCON_LIST g_extcon_gpio_list;
static struct platform_device *g_extcon_gpio_platform_dev;

static const char *g_extcon_gpio_name_list[] = {
    "PIN_HP_OUT_JD",
    NULL
};

static const unsigned int g_extcon_gpio_cable[] = {
    EXTCON_JACK_HEADPHONE,
    EXTCON_NONE,
};

static void rtk_extcon_gpio_irq_handler(RTK_GPIO_ID gid, unsigned char assert, void *p_dev_id)
{
    struct gpio_rtkextcon_data *p_extcon_data = (struct gpio_rtkextcon_data *)p_dev_id;
    schedule_work(&p_extcon_data->work);
    return;
}

static void rtk_extcon_gpio_sync_state(struct gpio_rtkextcon_data *p_extcon_data)
{
    int state = rtk_gpio_input(p_extcon_data->gpio);

    if(p_extcon_data->type == HP_OUT_JD){
        if (p_extcon_data->gpio_active_low)
            state = !state;

        if(state == 1)
            state = PLUG_IN_HEADSET_ANLG;

        extcon_set_state_sync(p_extcon_data->edev, g_extcon_gpio_cable[0], state);
    }
    /* other type not supportted yet, if need please add table g_extcon_gpio_cable and table g_extcon_gpio_name_list*/
    RTK_EXTCON_INFO("extcon gid:%d type:%d sync state:%d to kernel\n",p_extcon_data->gpio, p_extcon_data->type, state);
}

static void rtk_extcon_gpio_work(struct work_struct *work)
{
    struct gpio_rtkextcon_data *p_extcon_data = container_of(work, struct gpio_rtkextcon_data, work);
    rtk_extcon_gpio_sync_state(p_extcon_data);
}

struct gpio_rtkextcon_data* rtk_extcon_gpio_extcon_find_by_gid(RTK_GPIO_ID gid)
{
    int i = 0;
    struct gpio_rtkextcon_data* p_extcon_data = NULL;
    down_read(&g_extcon_gpio_list.lock);
    for(i = 0; i < RTK_EXTCON_GPIO_DEV_NUM; i++) {
        if(g_extcon_gpio_list.gpio_extcons[i] != NULL
            && g_extcon_gpio_list.gpio_extcons[i]->gpio == gid) {
            p_extcon_data = g_extcon_gpio_list.gpio_extcons[i];
            break;
        }
    }
    up_read(&g_extcon_gpio_list.lock);
    return p_extcon_data;
}

struct gpio_rtkextcon_data* rtk_extcon_gpio_extcon_find_by_index(int index)
{
    struct gpio_rtkextcon_data* p_extcon_data = NULL;
    down_read(&g_extcon_gpio_list.lock);
    if(index >= 0 && index < RTK_EXTCON_GPIO_DEV_NUM)
        p_extcon_data = g_extcon_gpio_list.gpio_extcons[index];
    up_read(&g_extcon_gpio_list.lock);
    return p_extcon_data;
}

struct gpio_rtkextcon_data * rtk_extcon_gpio_extcon_remove_by_gid(RTK_GPIO_ID gid)
{
    int i = 0;
    struct gpio_rtkextcon_data *p_extcon_data = NULL;
    down_write(&g_extcon_gpio_list.lock);
    for(i = 0; i < RTK_EXTCON_GPIO_DEV_NUM; i++) {
        if(g_extcon_gpio_list.gpio_extcons[i] != NULL
            && g_extcon_gpio_list.gpio_extcons[i]->gpio == gid) {
            p_extcon_data = g_extcon_gpio_list.gpio_extcons[i];
            break;
        }
    }

    if(i < RTK_EXTCON_GPIO_DEV_NUM) {
        g_extcon_gpio_list.gpio_extcons[i] = NULL;
    }
    up_write(&g_extcon_gpio_list.lock);
    return p_extcon_data;
}

int rtk_extcon_gpio_extcon_add(struct gpio_rtkextcon_data *p_extcon_data)
{
    int ret = -1;
    int i = 0;
    int first_usable_index = -1;
    struct gpio_rtkextcon_data *tmp = NULL;
    if(p_extcon_data) {
        down_write(&g_extcon_gpio_list.lock);
        for(i = 0; i < RTK_EXTCON_GPIO_DEV_NUM; i++) {
            if(first_usable_index == -1 && g_extcon_gpio_list.gpio_extcons[i] == NULL)
                first_usable_index = i;
            if(g_extcon_gpio_list.gpio_extcons[i] != NULL
                && g_extcon_gpio_list.gpio_extcons[i]->gpio == p_extcon_data->gpio) {
                tmp = g_extcon_gpio_list.gpio_extcons[i];
                break;
            }
        }
        if(tmp == NULL) {
            if(first_usable_index != -1) {
                g_extcon_gpio_list.gpio_extcons[first_usable_index] = p_extcon_data;
                ret = 0;
            }
        }
        up_write(&g_extcon_gpio_list.lock);
    }
    return ret;
}

void rtk_extcon_gpio_extcon_remove(struct gpio_rtkextcon_data *p_extcon_data)
{
    int i = 0;
    if(p_extcon_data) {
        down_write(&g_extcon_gpio_list.lock);
        for(i = 0; i < RTK_EXTCON_GPIO_DEV_NUM; i++) {
            if(g_extcon_gpio_list.gpio_extcons[i] != NULL
                && g_extcon_gpio_list.gpio_extcons[i]->gpio == p_extcon_data->gpio) {
                break;
            }
        }

        if(i < RTK_EXTCON_GPIO_DEV_NUM) {
            g_extcon_gpio_list.gpio_extcons[i] = NULL;
        }
        up_write(&g_extcon_gpio_list.lock);
    }
}

int rtk_extcon_gpio_register(struct gpio_rtkextcon_data *p_extcon_data, struct platform_device *pdev)
{
    int ret = 0;

    /* Allocate the memory of extcon devie and register extcon device */
    p_extcon_data->edev = devm_extcon_dev_allocate(&pdev->dev, g_extcon_gpio_cable);
    if (IS_ERR(p_extcon_data->edev)) {
        RTK_EXTCON_ERR("failed to allocate extcon device\n");
        goto err_extcon_dev_register;
    }

    ret = devm_extcon_dev_register(&pdev->dev, p_extcon_data->edev);
    if (ret != 0) {
        RTK_EXTCON_ERR("failed to register extcon device\n");
        goto err_extcon_dev_register;
    }
    /**  workaround
    ***  Android framework need like /sys/class/extcon/audiox, but kernel created file is /sys/class/extcon/extcon0
    ***  mismatched, so create link to fix this issue
    ***/
    ret = sysfs_create_link(&(p_extcon_data->edev->dev.class->p->subsys.kobj), &(p_extcon_data->edev->dev.kobj), p_extcon_data->name);
    if (ret != 0) {
        RTK_EXTCON_ERR("device create link failed. gpio:%d ret:%d\n", p_extcon_data->gpio, ret);
        goto err_extcon_dev_register;
    }

    rtk_gpio_set_dir(p_extcon_data->gpio, 0);
    rtk_gpio_set_irq_polarity(p_extcon_data->gpio, 1);
    rtk_gpio_set_debounce(p_extcon_data->gpio, RTK_GPIO_DEBOUNCE_10ms);

    INIT_WORK(&p_extcon_data->work, rtk_extcon_gpio_work);
    ret = rtk_gpio_request_irq(p_extcon_data->gpio, rtk_extcon_gpio_irq_handler, "EXTCON_GPIO_INT", p_extcon_data);
    if (ret != 0) {
        RTK_EXTCON_ERR("%s request irq fail\n", __func__);
        goto err_extcon_dev_request_irq;
    }
    rtk_gpio_set_irq_enable(p_extcon_data->gpio, 1);

    ret = rtk_extcon_gpio_extcon_add(p_extcon_data);
    if (ret != 0) {
        RTK_EXTCON_ERR("rtk_extcon_gpio_extcon_add fail\n", __func__);
        goto err_extcon_add_dev;
    }

    rtk_extcon_gpio_sync_state(p_extcon_data);
    return 0;

err_extcon_add_dev:
    rtk_gpio_set_irq_enable(p_extcon_data->gpio, 0);
    rtk_gpio_free_irq(p_extcon_data->gpio, p_extcon_data);
err_extcon_dev_request_irq:
    devm_extcon_dev_unregister(&pdev->dev, p_extcon_data->edev);
err_extcon_dev_register:
    ret = -ENOMEM;
    return ret;
}

int rtk_extcon_gpio_unregister(struct platform_device *pdev, RTK_GPIO_ID gid)
{
    struct gpio_rtkextcon_data *p_extcon_data = NULL;
    p_extcon_data = rtk_extcon_gpio_extcon_remove_by_gid(gid);
    if(!p_extcon_data)
        return 0;

    rtk_gpio_set_irq_enable(p_extcon_data->gpio, 0);
    rtk_gpio_free_irq(p_extcon_data->gpio, p_extcon_data);

    cancel_work_sync(&p_extcon_data->work);

    sysfs_remove_link(&(p_extcon_data->edev->dev.class->p->subsys.kobj), p_extcon_data->name);
    devm_extcon_dev_unregister(&pdev->dev, p_extcon_data->edev);
    kfree(p_extcon_data);
    return 0;
}

#ifdef CONFIG_PM
static int rtk_extcon_gpio_suspend(struct device *dev)
{
    RTK_EXTCON_WARN("suspend\n");
    return 0;
}
static int rtk_extcon_gpio_resume(struct device *dev)
{
    int i = 0;
    GPIO_EXTCON_LIST *extcon_list = &g_extcon_gpio_list;

    for(i=0; i<RTK_EXTCON_GPIO_DEV_NUM; i++)
    {
        if (extcon_list->gpio_extcons[i] != NULL)
        {
            rtk_extcon_gpio_sync_state(extcon_list->gpio_extcons[i]);
        }
    }
    RTK_EXTCON_WARN("resume\n");
    return 0;
}


#ifdef CONFIG_HIBERNATION
static int rtk_extcon_gpio_suspend_std(struct device *dev)
{
    RTK_EXTCON_WARN("rtk_extcon_gpio_suspend_std\n");
    return 0;
}
static int rtk_extcon_gpio_resume_std(struct device *dev)
{
    RTK_EXTCON_WARN("rtk_extcon_gpio_resume_std\n");
    return 0;
}
#endif

static const struct dev_pm_ops rtk_extcon_gpio_pm_ops = {

    .suspend = rtk_extcon_gpio_suspend,
    .resume = rtk_extcon_gpio_resume,

#ifdef CONFIG_HIBERNATION
    .freeze     = rtk_extcon_gpio_suspend_std,
    .thaw       = rtk_extcon_gpio_resume_std,
#endif
};

#endif

static char * rtk_extcon_gpio_get_dev_name(char *name)
{
    char *dev_name = NULL;
    if(name != NULL) {
        dev_name = strrchr(name, '_');
        if(dev_name) {
            dev_name++;
        }
    }
    return dev_name;
}

static int rtk_extcon_gpio_parse_and_register_dev(char *name, struct platform_device *pdev)
{
    int ret = 0;
    int i = 0;
    RTK_GPIO_ID gid;
    int gpio_index = 0;
    char dev_name[32] = {0};
    unsigned char g_gpio_invert;
    RTK_GPIO_GROUP gpio_group = MIS_GPIO;
    unsigned long long ullPcbMgrValue = 0x0;
    struct gpio_rtkextcon_data *p_extcon_data = NULL;

    if(!name)
        return -1;
    ret = pcb_mgr_get_enum_info_byname(name, &ullPcbMgrValue);
    if(ret != 0){
        RTK_EXTCON_WARN("%s not config by pcb\n", name);
        return -1;
    }

    if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_GPIO) {
        gpio_group = MIS_GPIO;
    } else if (GET_PIN_TYPE(ullPcbMgrValue) == PCB_PIN_TYPE_ISO_GPIO) {
        gpio_group = ISO_GPIO;
    } else {
        return -1;
    }

    g_gpio_invert = GET_PIN_PARAM2(ullPcbMgrValue);
    gpio_index = GET_PIN_INDEX(ullPcbMgrValue);
    gid = rtk_gpio_id(gpio_group, gpio_index);
    name = rtk_extcon_gpio_get_dev_name(name);

    if(name == NULL || strlen(name) == 0)
        return -1;
    while(*name != '0' && i < 32) {
        dev_name[i] = *name;
        if(dev_name[i] <= 'Z' && dev_name[i] >= 'A')
            dev_name[i] = dev_name[i] + 0x20;
        name++;
        i++;
    }
    RTK_EXTCON_WARN("gid:%d, g_gpio_invert :%d, dev name:%s\n", gid, g_gpio_invert, dev_name);

    p_extcon_data = kmalloc(sizeof(struct gpio_rtkextcon_data), GFP_KERNEL);
    if (!p_extcon_data){
        RTK_EXTCON_ERR("rtk_gpio_extcon_register malloc failed\n");
        return -ENOMEM;
    }
    memset(p_extcon_data, 0, sizeof(struct gpio_rtkextcon_data));

    if(strcmp("jd",dev_name) == 0){
        snprintf(p_extcon_data->name, 16, "audio_%d", gid);
        p_extcon_data->gpio_active_low = (g_gpio_invert? 0: 1);
        p_extcon_data->type = HP_OUT_JD;
    }else{
        snprintf(p_extcon_data->name, 16, "%s", dev_name);
        p_extcon_data->gpio_active_low = 0;
        p_extcon_data->type = GENERAL_TYPE;
    }
    p_extcon_data->gpio = gid;

    if((ret = rtk_extcon_gpio_register(p_extcon_data, pdev)) != 0)
        kfree(p_extcon_data);

    return ret;
}

static int rtk_extcon_gpio_probe(struct platform_device *pdev)
{
    char **namelist = (char **)g_extcon_gpio_name_list;
    init_rwsem(&g_extcon_gpio_list.lock);

    RTK_EXTCON_WARN("rtk_extcon_gpio_probe \n");

    while (*namelist != NULL) {
        rtk_extcon_gpio_parse_and_register_dev(*namelist, pdev);
        namelist++;
    }
    return 0;
}

static int rtk_extcon_gpio_remove(struct platform_device *pdev)
{
    int i = 0;
    struct gpio_rtkextcon_data *p_extcon_data = NULL;
    for(i = 0; i < RTK_EXTCON_GPIO_DEV_NUM; i++) {
        p_extcon_data = rtk_extcon_gpio_extcon_find_by_index(i);
        if(p_extcon_data)
            rtk_extcon_gpio_unregister(pdev, p_extcon_data->gpio);
    }
    return 0;
}


static const struct of_device_id rtk_extcon_gpio_devices[] = {
    {.compatible = "rtk_extcon_gpio",},
    {},
};
MODULE_DEVICE_TABLE(of, rtk_extcon_gpio_devices);

static struct platform_driver rtk_extcon_gpio_driver = {
    .driver = {
            .name = "rtk_extcon_gpio",
            .owner = THIS_MODULE,
            .of_match_table = of_match_ptr(rtk_extcon_gpio_devices),

            #ifdef CONFIG_PM
            .pm = &rtk_extcon_gpio_pm_ops,
            #endif
        },
    .probe = rtk_extcon_gpio_probe,
    .remove = rtk_extcon_gpio_remove,
};

static int __init rtk_extcon_gpio_init(void)
{
    int ret = 0;
    ret = platform_driver_register(&rtk_extcon_gpio_driver);
    if (ret != 0)
        goto REGISTER_PLATFORM_DRIVER_FAIL;

    g_extcon_gpio_platform_dev = platform_device_register_simple("rtk_extcon_gpio", -1, NULL, 0);
    if (g_extcon_gpio_platform_dev == NULL)
        goto REGISTER_PLATFORM_DEVICE_FAIL;

    RTK_EXTCON_INFO("switch gpio driver init sucess\n");
    return 0;
REGISTER_PLATFORM_DEVICE_FAIL:
    platform_driver_unregister(&rtk_extcon_gpio_driver);
REGISTER_PLATFORM_DRIVER_FAIL:
    return -1;
}

static void __exit rtk_extcon_gpio_exit(void)
{
    platform_driver_unregister(&rtk_extcon_gpio_driver);
    platform_device_unregister(g_extcon_gpio_platform_dev);
}

module_init(rtk_extcon_gpio_init);
module_exit(rtk_extcon_gpio_exit);
MODULE_DESCRIPTION("Realtek extcon GPIO Driver");
MODULE_LICENSE("GPL");
