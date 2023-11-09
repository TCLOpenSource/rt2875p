#ifndef BUILD_QUICK_SHOW
#include <linux/module.h>
#include <linux/types.h>
#include <rtd_log/rtd_module_log.h>
#include <linux/miscdevice.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/kfifo.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/bitops.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/signal.h>
#include <linux/interrupt.h>
#include <linux/jiffies.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/slab.h>
#else
#include <no_os/export.h>
#include <no_os/slab.h>
#include <no_os/init.h>
#include <string.h>
#include <vsprintf.h>
#endif /* !BUILD_QUICK_SHOW */
#include <rtk_kdriver/rtk_gpio.h>
#include <rtk_kdriver/pcbMgr.h>

#include <mach/platform.h>

extern platform_info_t platform_info;
extern struct kobject *realtek_boards_kobj;

static int pcb_enum_count = 0;
struct linux_pcb_enum_t   pcb_enum_all[PCB_ENUM_MAX];

#ifndef BUILD_QUICK_SHOW
static struct attribute  **realtek_pcb_enum_attrs = NULL;
static struct attribute_group realtek_pcb_enum_attr_group = {
	.name = "pcb_enum",
};
#endif

EXPORT_SYMBOL(pcb_enum_all);

int pcb_mgr_get_enum_info_byname(char *enum_name, unsigned long long *value) {


	char *buf, *token, *final;
	char pattern[256];

	buf = platform_info.pcb_enum_parameter;
	final = &platform_info.pcb_enum_parameter[platform_info.pcb_enum_parameter_len];
	snprintf(pattern, sizeof(pattern)-1, "pcb_enum=%s,", enum_name);
        pattern[sizeof(pattern)-1] = 0;

	//rtd_pr_pcb_mgr_info("PATTERN:%s\n", pattern);

	//rtd_pr_pcb_mgr_info("platform_info.pcb_enum_parameter = %s\n", platform_info.pcb_enum_parameter);
	token = strstr(buf, pattern);
	if (token) {
		token = token + strlen(pattern);
		if (value) {
			*value = simple_strtoull(token, 0, 0);
			return 0;
		}
	}
	return -1;
}

EXPORT_SYMBOL(pcb_mgr_get_enum_info_byname);

#ifndef BUILD_QUICK_SHOW
static ssize_t realtek_sysfs_pcb_enum_show(struct kobject *kobj, struct kobj_attribute *attr, char *page)
{

	struct linux_pcb_enum_t *pos;
       pos = (struct linux_pcb_enum_t *) attr->attr.name;
	return snprintf(page, PAGE_SIZE-1, "0x%llx\n", pos->value);

}
#endif

static int pcb_enum_add(char *buffer) {
	int i;
	char *name = NULL;
	int name_len, space_hit;
	char *value = NULL;
	int len = strlen(buffer);

	if (pcb_enum_count >= PCB_ENUM_MAX) return 0;

 	for (i = 0; i<len ; i++) {
		if (buffer[i] == ' ') continue;
		name = &buffer[i];
		break;
	}
	if (i == len) return -1;

 	name_len = 0;
 	space_hit = 0;
 	for (; i< (len-1) ; i++) {
		if (buffer[i] == ',') {
			 value = &buffer[i+1];
			 break;
		}
		if (buffer[i] != ' ' && space_hit == 0) {
			  name_len++;
		} else {
	  		space_hit = 1;
	  }
	}
	if (name_len >= (PCB_ENUM_NAME_LEN_MAX-1)) return 0;
	if (value == NULL) return -1;

	strncpy(pcb_enum_all[pcb_enum_count].name, name, name_len);
	pcb_enum_all[pcb_enum_count].value = simple_strtoull(value, 0, 0);
	pcb_enum_count++;
	if (pcb_enum_count >= PCB_ENUM_MAX) return 0;
	pcb_enum_all[pcb_enum_count].name[0] = 0;
	pcb_enum_all[pcb_enum_count].value = 0;
	return 0;
}
#if 0
int realtek_sysfs_pcb_enum_dir_init(void) {
    struct kobject *parent_kobj = realtek_boards_kobj;
	int i;
	int error;
	struct kobj_attribute *kobj_attr;
	char *buf, *token, *final;

	i = 0;
	buf = platform_info.pcb_enum_parameter;
	final = &platform_info.pcb_enum_parameter[platform_info.pcb_enum_parameter_len];
//	rtd_pr_pcb_mgr_info("platform_info.pcb_enum_parameter = %s\n", platform_info.pcb_enum_parameter);
	do {
		if (buf < platform_info.pcb_enum_parameter) {
	  	   rtd_pr_pcb_mgr_err("pcb enum parse error\n");
	  	   return -1;
	  }
		// skip next space
		while(buf < final) {
			if (buf[0] == ' ') {
				buf++;
			} else {
				break;
			}
		};

		if (buf >= final) break;

		token = strstr(buf, "pcb_enum=");
		if (token == NULL)
		    break;

		if (token != buf) {
				 rtd_pr_pcb_mgr_err("pcb enum parse error\n");
	  	   return -1;
		}
		token += 9;
//		rtd_pr_pcb_mgr_info("%s\n", token);
		// KWarning: checked ok by eaton.chiou@realtek.com
		pcb_enum_add(token);

		// find next space
		while(buf < final) {
			if (buf[0] != ' ') {
				buf++;
			} else {
				break;
			}
		};
		if (buf >= final) break;

	} while(token!=NULL);

  if (pcb_enum_count == 0) return 0;

#ifndef BUILD_QUICK_SHOW
	realtek_pcb_enum_attrs = kmalloc((pcb_enum_count+1) * sizeof(struct attribute *), GFP_KERNEL);
	if(realtek_pcb_enum_attrs == NULL)
	{
		rtd_pr_pcb_mgr_err("allocate pcb_enum_count failed\n");
		return 0;
	}

	for (i=0; i<pcb_enum_count; i++) {


		kobj_attr = kmalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
		if(!kobj_attr)
			panic("Error : allocate fail at %s:%d", __func__, __LINE__);
		else
			memset(kobj_attr, 0, sizeof(struct kobj_attribute));

		kobj_attr->show = realtek_sysfs_pcb_enum_show;
		kobj_attr->store = NULL;
		realtek_pcb_enum_attrs[i] = &kobj_attr->attr;

		realtek_pcb_enum_attrs[i]->mode = 0444;
		realtek_pcb_enum_attrs[i]->name = pcb_enum_all[i].name;
/*		realtek_pcb_enum_attrs[i]->owner = THIS_MODULE;*/
//		rtd_pr_pcb_mgr_info("i = %d %s %016llx\n", i, realtek_pcb_enum_attrs[i]->name , pcb_enum_all[i].value);
	}
	realtek_pcb_enum_attrs[i] = NULL;
	realtek_pcb_enum_attr_group.attrs = realtek_pcb_enum_attrs;
	error = sysfs_create_group(parent_kobj, &realtek_pcb_enum_attr_group);
	if(error) {
		for (i=0; i<pcb_enum_count; i++) {
			kobj_attr = container_of(realtek_pcb_enum_attrs[i],struct kobj_attribute, attr);
			kfree(kobj_attr);
		}
		kfree(realtek_pcb_enum_attrs);
	}
#endif /* !BUILD_QUICK_SHOW */
	return error;
}
#else
static int realtek_pcb_enum_init(void)
{
	int i;
	char *buf, *token, *final;

	i = 0;
	buf = platform_info.pcb_enum_parameter;
	final = &platform_info.pcb_enum_parameter[platform_info.pcb_enum_parameter_len];
//	rtd_pr_pcb_mgr_info("platform_info.pcb_enum_parameter = %s\n", platform_info.pcb_enum_parameter);
	do {
		if (buf < platform_info.pcb_enum_parameter) {
		   rtd_pr_pcb_mgr_err("pcb enum parse error\n");
		   return -1;
	  }
		// skip next space
		while(buf < final) {
			if (buf[0] == ' ') {
				buf++;
			} else {
				break;
			}
		};

		if (buf >= final) break;

		token = strstr(buf, "pcb_enum=");
		if (token == NULL)
			break;

		if (token != buf) {
				 rtd_pr_pcb_mgr_err("pcb enum parse error\n");
		   return -1;
		}
		token += 9;
//		rtd_pr_pcb_mgr_info("%s\n", token);
		// KWarning: checked ok by eaton.chiou@realtek.com
		pcb_enum_add(token);

		// find next space
		while(buf < final) {
			if (buf[0] != ' ') {
				buf++;
			} else {
				break;
			}
		};
		if (buf >= final) break;

	} while(token!=NULL);
	return 0;
}

#ifndef BUILD_QUICK_SHOW
int realtek_sysfs_pcb_enum_dir_init(struct kobject *parent_kobj) 
{
	int i;
	int error;
	struct kobj_attribute *kobj_attr;

	if(pcb_enum_count == 0)
	{
		rtd_pr_pcb_mgr_err("Error: realtek_sysfs_pcb_enum_dir_init fail\n");
		return 0;
	}

	realtek_pcb_enum_attrs = kmalloc((pcb_enum_count+1) * sizeof(struct attribute *), GFP_KERNEL);
	if(realtek_pcb_enum_attrs == NULL)
	{
		rtd_pr_pcb_mgr_err("allocate pcb_enum_count failed\n");
		return 0;
	}

	for (i=0; i<pcb_enum_count; i++) {


		kobj_attr = kmalloc(sizeof(struct kobj_attribute), GFP_KERNEL);
		if(!kobj_attr)
			panic("Error : allocate fail at %s:%d", __func__, __LINE__);
		else
			memset(kobj_attr, 0, sizeof(struct kobj_attribute));

		kobj_attr->show = realtek_sysfs_pcb_enum_show;
		kobj_attr->store = NULL;
		realtek_pcb_enum_attrs[i] = &kobj_attr->attr;

		realtek_pcb_enum_attrs[i]->mode = 0444;
		realtek_pcb_enum_attrs[i]->name = pcb_enum_all[i].name;
/*		realtek_pcb_enum_attrs[i]->owner = THIS_MODULE;*/
//		rtd_pr_pcb_mgr_info("i = %d %s %016llx\n", i, realtek_pcb_enum_attrs[i]->name , pcb_enum_all[i].value);
	}
	realtek_pcb_enum_attrs[i] = NULL;
	realtek_pcb_enum_attr_group.attrs = realtek_pcb_enum_attrs;
	error = sysfs_create_group(parent_kobj, &realtek_pcb_enum_attr_group);
	if(error) {
		for (i=0; i<pcb_enum_count; i++) {
			kobj_attr = container_of(realtek_pcb_enum_attrs[i],struct kobj_attribute, attr);
			kfree(kobj_attr);
		}
		kfree(realtek_pcb_enum_attrs);
	}

	return error;
}
EXPORT_SYMBOL(realtek_sysfs_pcb_enum_dir_init);
#endif /* BUILD_QUICK_SHOW */
#endif

int __init pcbmgr_module_init(void)
{

/*
built-in: arch_initcall(realtek_boards_sysfs_init) then module_init(pcbmgr_module_init)
module :rtk-sysfs.ko(realtek_boards_sysfs_init) then rtk-pcbMgr.ko(pcbmgr_module_init)
*/
	realtek_pcb_enum_init();
#ifndef BUILD_QUICK_SHOW
#ifndef CONFIG_RTK_KDRV_SYSFS_MODULE
	realtek_sysfs_pcb_enum_dir_init(realtek_boards_kobj);
#endif
#endif /* BUILD_QUICK_SHOW */
	rtd_pr_pcb_mgr_warn("pcbmgr_module_init init finish\n");
	return 0;

}

arch_initcall(pcbmgr_module_init);
MODULE_LICENSE("GPL");


/* API (porting from AP layer) */

#if 0
/*
  * Function: IO_PWM_SetFreq
  *
  * Description:
  *         given some "PIN_XX"'s value then setup pwm frequency
  *
  *   param:
  *        pin : the value corresponding to some "PIN_XXX"
  *        freq_hz: PWM frequency
  * return :   -1 : means unkwon pin type ...
  *         0: success
  *
*/
int IO_PWM_SetFreq(unsigned long long pin, int freq_hz)
{
    PCB_PIN_TYPE_T type;
    int index;
    int param1, param2, param3, param4, param5;
    type = GET_PIN_TYPE(pin);
    index = GET_PIN_INDEX(pin);
    switch (type) {

    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_UNIPWM:
        {
            param1 = GET_UNIPWM_INVERT(pin);
            param3 = GET_UNIPWM_DUTY_MAX(pin);
            param4 = GET_UNIPWM_INIT_DUTY(pin);
            param5 = GET_UNIPWM_PWM_NUM(pin);
            if (type == PCB_PIN_TYPE_UNIPWM) {
                unipwm_init(index, param5, freq_hz, param3,
                        param4, param1);
            } else {
                iso_unipwm_init(index, param5, freq_hz, param3,
                        param4, param1);
            }
        }
        break;
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
        {
            param1 = GET_PWM_INVERT(pin);
            param3 = GET_PWM_DUTY_MAX(pin);
            param4 = GET_PWM_INIT_DUTY(pin);
            if (type == PCB_PIN_TYPE_PWM) {
                pwm_init(index, freq_hz, param3, param4,
                     param1);
            } else {
                iso_pwm_init(index, freq_hz, param3, param4,
                         param1);
            }

        }
        break;

    case PCB_PIN_TYPE_UNUSED:
        return 0;
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_EMCU_GPIO:
    case PCB_PIN_TYPE_GPIO:
    case PCB_PIN_TYPE_ISO_GPIO:
        break;
    default:
        break;

    }

    return 0;

}

int IO_PWM_SetFreq1(unsigned long long pin, int freq_hz)
{
    PCB_PIN_TYPE_T type;
    int index;
    int param1, param2, param3, param4, param5;
    type = GET_PIN_TYPE(pin);
    index = GET_PIN_INDEX(pin);
    
    switch (type) {        
    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_UNIPWM:
        {
            param1 = GET_UNIPWM_INVERT(pin);
            param3 = GET_UNIPWM_DUTY_MAX(pin);
            param4 = GET_UNIPWM_INIT_DUTY(pin);
            param5 = GET_UNIPWM_PWM_NUM(pin);
            if (type == PCB_PIN_TYPE_UNIPWM) {
                unipwm_init(index, param5, freq_hz, param3,
                        param4, param1);
            } else {
                iso_unipwm_init(index, param5, freq_hz, param3,
                        param4, param1);
            }
        }
        break;
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
        {
            param1 = GET_PWM_INVERT(pin);
            param3 = GET_PWM_DUTY_MAX(pin);
            param4 = GET_PWM_INIT_DUTY(pin);
            if (type == PCB_PIN_TYPE_PWM) {
                pwm_init1(index, freq_hz, param3, param4,
                      param1);
            } else {
                iso_pwm_init(index, freq_hz, param3, param4,
                         param1);
            }

        }
        break;

    case PCB_PIN_TYPE_UNUSED:
        return 0;
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_EMCU_GPIO:
    case PCB_PIN_TYPE_GPIO:
    case PCB_PIN_TYPE_ISO_GPIO:
        break;
    default:
        break;

    }

    return 0;

}

/*
  * Function: IO_PWM_SetDuty
  *
  * Description:
  *         given some "PIN_XX"'s value then setup pwm duty
  *
  *   param:
  *        pin : the value corresponding to some "PIN_XXX"
  *             pin include duty_max function
  *        freq_hz: PWM frequency
  * return :   -1 : means unkwon pin type ...
  *         0: success
  *
*/

int IO_PWM_SetDuty(unsigned long long pin, int duty)
{

    PCB_PIN_TYPE_T type;
    int index;
    int param1, param2, param3, param4, param5;
    type = GET_PIN_TYPE(pin);
    index = GET_PIN_INDEX(pin);
    switch (type) {

    case PCB_PIN_TYPE_ISO_UNIPWM:
    case PCB_PIN_TYPE_UNIPWM:
        {
            if (type == PCB_PIN_TYPE_UNIPWM)
                unipwm_set(index, duty);
            else
                iso_unipwm_set(index, duty);

        }
        break;
    case PCB_PIN_TYPE_PWM:
    case PCB_PIN_TYPE_ISO_PWM:
        {
            if (type == PCB_PIN_TYPE_PWM)
                pwm_set(index, duty);
            else
                iso_pwm_set(index, duty);


        }
        break;

    case PCB_PIN_TYPE_UNUSED:
        return 0;
    case PCB_PIN_TYPE_LSADC:
    case PCB_PIN_TYPE_EMCU_GPIO:
    case PCB_PIN_TYPE_GPIO:
    case PCB_PIN_TYPE_ISO_GPIO:
        break;
    default:
        break;

    }

    return 0;
}

int IO_Direct_SetPWMFreq(char *enum_name, int freq_hz)
{
    unsigned long long info;
    if (pcb_mgr_get_enum_info_byname(enum_name, &info) == 0)
        IO_PWM_SetFreq(info, freq_hz);
}

int IO_Direct_SetPWMFreq1(char *enum_name, int freq_hz)
{
    unsigned long long info;
    if (pcb_mgr_get_enum_info_byname(enum_name, &info) == 0)
        IO_PWM_SetFreq1(info, freq_hz);
}

#endif



