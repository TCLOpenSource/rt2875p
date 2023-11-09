
#include <linux/syscalls.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/sysfs.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <mach/platform.h>
#include <rtd_log/rtd_os_print.h>
#include <rtk_kdriver/pcbMgr.h>
#include <core/event_log_core.h>
#define PR_TYPE_HEX     (16)
#define PR_TYPE_DEC     (10)
#define PR_TYPE_PCB     (0xff)
/*
######################################################################################
# static number
######################################################################################
*/
//static unsigned int module_number = 0;
static unsigned int read_number=0;
static unsigned int read_parse_number=0;
static unsigned int eventlog_enable = 1;
static unsigned int eventlog_filter_enable = 0;
static unsigned int INIT_LOCK = 0;
static char parse[4096] = {0};
/*
######################################################################################
# MEMORY DEFINE
######################################################################################
*/
spinlock_t event_lock;
/*
######################################################################################
# FUNCTION
######################################################################################
*/

/*
	output transform little/big endian
*/
static
void eventlog_endian_swap(int *buf, int *param, int size)
{
	int i;
	for(i=0; i<size; i++)
	{
		if( i < (memory_count % INT_LOG_SIZE))
		{
			buf[i] = eventlog_endian_32_swap(param[i]);
		}
	}
}

static
int eventlog_check_param_number(const char *buf, unsigned int number)
{
	int i = 0;
	for( i = 0 ; i < strlen(buf); i++)
	{
		if( buf[i] == ',')
		{
			number --;
		}
		if(1 > number)
			return 0;
	}

	if(number != 1)
		return 0;
	return 1;
}

static
int eventlog_get_param_multi(int type,
		int i_cnt,
		int* o_cnt,
		unsigned long long* param,
		const char *buf, size_t count)
{
	int ret = -1;
	int i;
	char last_char;
	char *opt = NULL;
	char *buftmp = NULL;
	char *buforg = NULL;
	unsigned long long tmp_long;

	if(0 == eventlog_check_param_number(buf, i_cnt)){
		EV_ERR("number fail\n");
		goto FINISH_OUT;
	}
	buforg = buftmp = kmalloc(count+1, GFP_KERNEL);
	if(!buftmp){
		goto FINISH_OUT;
	}
	memset(buftmp,0,count+1);
	strncpy(buftmp, buf, count);

	i = strlen(buftmp)-1;
	last_char = buftmp[i];

	if(!isdigit(last_char)&& !isalpha(last_char) ){
		buftmp[i] = 0;
	}

	i = 0;
	while ((opt = strsep(&buftmp, ",")) != NULL) {
		/* translate digit number HEX/DEC */
		if (kstrtoull(opt, type, &tmp_long)){
			goto FINISH_OUT;
		}else{
			param[i] = tmp_long;
			i++;
		}
	}
	*o_cnt = i;
	if((*o_cnt == 0) || ((*o_cnt) > i_cnt)) {
		goto FINISH_OUT;
	}
	ret = 0;

FINISH_OUT:
	if(buforg)
		kfree(buforg);
	if(ret)
		EV_ERR("get param fail\n");

	return ret;
}

static
int eventlog_get_param_single(int type,
		unsigned long long* param,
		const char *buf)
{
	int ret = -1;
	unsigned long long tmp_long;

	if (kstrtoull(buf, type, &tmp_long)){
		goto FINISH_OUT;
	}else{
		*param = tmp_long;
	}
	ret = 0;

FINISH_OUT:
	if(ret)
		EV_ERR("get param fail\n");

	return ret;
}

unsigned int eventlog_filter_verify(unsigned int fw_type, unsigned int event_type, \
unsigned int module, unsigned int event)
{
	if(fw_type >= FW_MAX || module >= MODULE_MAX || event >= EVENT_MAX || event_type >= EVENT_TYPE_MAX)
	{
		event_err_print("(fw_type module event event_type) > Specified range\n");
		return EVENT_ERROR;
	}

	if( 0 == fw_filter_data[fw_type].fw_enable )
	{
		return EVENT_ERROR;
	}

	if( 0 == fw_filter_data[fw_type].module_data[module].module_enable )
	{
		return EVENT_ERROR;
	}

	if( 0 == ( 0x01 << event_type & (fw_filter_data[fw_type].module_data[module].event_type)) )
	{
		return EVENT_ERROR;
	}

	if( 0 == ( 0x01 << ( (event) % 32) & (fw_filter_data[fw_type].module_data[module].event[(event)/32])) )
	{
		return EVENT_ERROR;
	}

	return EVENT_OK;

}

unsigned int event_log_kernel_save(unsigned int fw_type, unsigned int event_type, \
								   unsigned int module, unsigned int event, unsigned int event_val, unsigned int module_reserved)
{
	unsigned long flags;
	int ret = 0;

	if(0 == INIT_LOCK)
	{
		spin_lock_init(&event_lock);

		/*clean log_memory*/
		memset(log_memory, 0, sizeof(log_memory));
		INIT_LOCK = 1;
	}

	if(eventlog_enable)
	{

		if(eventlog_filter_enable)
		{
			if( EVENT_ERROR == eventlog_filter_verify(fw_type, event_type, module, event))
				return EVENT_ERROR;
		}

		spin_lock_irqsave(&event_lock, flags);
		ret = event_log_save(fw_type, event_type, module, event, event_val, module_reserved);
		spin_unlock_irqrestore(&event_lock, flags);

		return ret;
	}

	return EVENT_ERROR;
}

EXPORT_SYMBOL(event_log_kernel_save);

unsigned int event_log_filter_set(unsigned int fw_id, unsigned int fw_value,  unsigned int event_type_id, unsigned int event_type_value, \
unsigned int module_id, unsigned int module_value, unsigned int event_id, unsigned int event_value)
{
	if(1 != fw_value && 0 != fw_value)
	{
		EV_ERR("fw_value error, fw_value = %x\n", fw_value);
		return EVENT_ERROR;
	}

	if(1 != module_value && 0 != module_value)
	{
		EV_ERR("module_value error, module_value = %x\n", module_value);
		return EVENT_ERROR;
	}

	if(1 != event_value && 0 != event_value)
	{
		EV_ERR("event_value error, event_value = %x\n", event_value);
		return EVENT_ERROR;
	}

	if(1 != event_type_value && 0 != event_type_value)
	{
		EV_ERR("event_type_value error, event_type_value = %x\n", event_type_value);
		return EVENT_ERROR;
	}

	if(fw_id < FW_MAX && module_id < MODULE_MAX && event_id < EVENT_MAX && event_type_id < EVENT_TYPE_MAX)
	{
		fw_filter_data[fw_id].fw_enable = fw_value;
		fw_filter_data[fw_id].module_data[module_id].module_enable = module_value;
		fw_filter_data[fw_id].module_data[module_id].event_type |= event_type_value << event_type_id;
		fw_filter_data[fw_id].module_data[module_id].event[(event_id)/32] |= event_value << ((event_id) % 32);

		EV_WARNING("fw_id = %x, enable = %x \n", fw_id, fw_filter_data[fw_id].fw_enable);
		EV_WARNING("module_id = %x, enable = %x \n", module_id, fw_filter_data[fw_id].module_data[module_id].module_enable);
		EV_WARNING("event_type_id = %x, event_type = %x \n", event_type_id, fw_filter_data[fw_id].module_data[module_id].event_type);
		EV_WARNING("event_id = %x, event[%d] = %x \n", event_id, (event_id)/32, fw_filter_data[fw_id].module_data[module_id].event[(event_id)/32]);

		return EVENT_OK;
	}

	EV_ERR("(fw_id module_id event_id event_type_id) > Specified range\n");

	return EVENT_ERROR;
}

/************************************************************************
 * File Operations
 ************************************************************************/
int rtk_eventlog_open(struct inode *inode, struct file *file)
{
	EV_INFO("%s(%d) Open\n",__func__,__LINE__);
	return 0;
}

int rtk_eventlog_parser_open(struct inode *inode, struct file *file)
{
	EV_INFO("%s(%d) Open\n",__func__,__LINE__);
	return 0;
}

int rtk_eventlog_release(struct inode *inode, struct file *file)
{
	EV_INFO("%s(%d) Release\n",__func__,__LINE__);
	return 0;
}

int rtk_eventlog_parser_release(struct inode *inode, struct file *file)
{
	EV_INFO("%s(%d) Release\n",__func__,__LINE__);
	return 0;
}

long rtk_eventlog_ioctl(struct file *file, unsigned int cmd,
			     unsigned long arg)
{
	EV_INFO("%s(%d) \n",__func__,__LINE__);
	return 0;
}

ssize_t rtk_eventlog_parser_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned char *tmp_buf = NULL;
	unsigned int parse_index = 0;

	EV_DEBUG("%s(%d) read_parse_number = %d \n",__func__,__LINE__, read_parse_number);
	EV_DEBUG("%s(%d) memory_count = %d \n",__func__,__LINE__,memory_count);

	if(count != 0x1000)
	{
		EV_ERR("count = %d, must be 4096 bytes\n");
		return -EFAULT;
	}

	if ( (read_parse_number+3) >= memory_count)
	{
		read_parse_number = 0;
		return 0;
	}

	tmp_buf = (unsigned char *)kmalloc(count, GFP_KERNEL);

	if(!tmp_buf)
	{
        return -ENOMEM;
	}

	memset(parse, 0, 4096);

	while  ((read_parse_number+3) < memory_count)
	{
		parse_index = eventlog_parser_file(log_memory[read_parse_number], log_memory[read_parse_number+1], log_memory[read_parse_number+2], log_memory[read_parse_number+3], time_memory[read_parse_number/4], parse, count);

		if(EVENT_ERROR == parse_index)
		{
			EV_ERR("parse error\n");
		}

		if(-1 == parse_index)
		{
			memcpy(tmp_buf, parse, count);

			if(copy_to_user(buf, tmp_buf, count)) {
				read_parse_number = 0;
        		kfree(tmp_buf);
        		return -EFAULT;
   			 }

			kfree(tmp_buf);
			return count;
		}
		read_parse_number = read_parse_number+4;
	}

	memcpy(tmp_buf, parse, count);

	if(copy_to_user(buf, tmp_buf, count)) {
        kfree(tmp_buf);
        return -EFAULT;
   	}

	kfree(tmp_buf);

	return count;
}

ssize_t rtk_eventlog_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	unsigned char *tmp_buf = NULL;
	unsigned int int_log_index = 0;

	EV_DEBUG("%s(%d) read_number = %d \n",__func__,__LINE__,read_number);
	EV_DEBUG("%s(%d) memory_count = %d \n",__func__,__LINE__,memory_count);

	if(count != 0x1000)
	{
		EV_ERR("count = %d, must be 4096 bytes\n");
		return -EFAULT;
	}

	if ( ((read_number * count) >= (memory_count * 4)) || ((read_number * count) >= LOG_SIZE))
	{
		read_number = 0;
		return 0;
	}

	tmp_buf = (unsigned char *)kmalloc(count, GFP_KERNEL);

	if(!tmp_buf)
	{
        return -ENOMEM;
	}

	/*clean output_buf_log*/
	memset(output_buf_log, 0, count);

	int_log_index = read_number * count / 4;

	eventlog_endian_swap(output_buf_log, &log_memory[int_log_index], (count) / 4);

	memcpy(tmp_buf, output_buf_log, count);

    if(copy_to_user(buf, tmp_buf, count)) {
        kfree(tmp_buf);
        return -EFAULT;
    }

	kfree(tmp_buf);

	read_number++;

	return count;
}
/************************************************************************
 *  STRUCT
 ************************************************************************/

#define EVENTLOG_DRV_NAME  "eventlog_drv"

static
struct file_operations rtk_eventlog_parser_fops = {
	.owner = THIS_MODULE,
	.open = rtk_eventlog_parser_open,
	.read = rtk_eventlog_parser_read,
	.release = rtk_eventlog_parser_release,
};

static
struct file_operations rtk_eventlog_fops = {
	.owner = THIS_MODULE,
	.unlocked_ioctl = rtk_eventlog_ioctl,
	.open = rtk_eventlog_open,
	.read = rtk_eventlog_read,
	.release = rtk_eventlog_release,
};

static struct miscdevice rtk_eventlog_miscdev = {
	MISC_DYNAMIC_MINOR,
	"rtk_eventlog",
	&rtk_eventlog_fops
};

static struct miscdevice rtk_eventlog_parser_miscdev = {
	MISC_DYNAMIC_MINOR,
	"rtk_eventlog_parser",
	&rtk_eventlog_parser_fops
};

static struct platform_device eventlog_drv_pm_device = {
    .name          = EVENTLOG_DRV_NAME,
    .id            = -1,
};


static struct platform_driver eventlog_drv_pm_driver = {
    .driver    = {
        .name  = EVENTLOG_DRV_NAME,
        .owner = THIS_MODULE,
    },
};

/************************************************************************
 * DEVICE ATTR
 ************************************************************************/

static
ssize_t eventlog_filter_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
    return snprintf(buf, 256,"please key in \n"
	"echo fw,fw_enable,event_type,event_type_enable,module,module_enable,event,event_enable > "
		"/sys/devices/virtual/misc/rtk_eventlog/eventlog_filter\n");
}

static
ssize_t eventlog_filter_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long param[8];
	size_t ret = count;

	if(eventlog_get_param_multi(PR_TYPE_HEX, 8, &param_cnt,param,buf,count)){
		EV_ERR("(%s)get param fail\n",__func__);
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 8){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(EVENT_ERROR == event_log_filter_set(param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7]))
	{
		ret = -EINVAL;
		goto FINISH_OUT;
	}

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(eventlog_filter, S_IRUGO | S_IWUSR,
		eventlog_filter_dev_show,
		eventlog_filter_dev_store);



static
ssize_t eventlog_enable_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	return snprintf(buf, 256,"%d\n",eventlog_enable);
}

static
ssize_t eventlog_enable_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	ssize_t ret = count;
    unsigned long long in_val;
    if(eventlog_get_param_single(16,&in_val,buf)){
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(in_val == 1 || in_val == 0)
	{
    	eventlog_enable = in_val;
	}
	else{
		EV_ERR("eventlog_enable must be 1 or 0\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}

FINISH_OUT:
	return ret;
}


DEVICE_ATTR(eventlog_enable, S_IRUGO | S_IWUSR,
		eventlog_enable_dev_show,
		eventlog_enable_dev_store);

static
ssize_t eventlog_cmd_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	EV_ERR("echo enable,(0 or 1)\n");
	EV_ERR("echo filter_enable,(0 or 1)\n");
	EV_ERR("echo time_unit,(0 or 1): 0:unit->ms, 1:unit->us\n");
	EV_ERR("echo event_one_diff,fw,eventtype,module,event\n");
	EV_ERR("echo event_two_diff,fw,eventtype,module,event,fw,eventtype,module,event\n");
	EV_ERR("echo clean_memory\n");
	EV_ERR("echo show_define\n");
	EV_ERR("echo clean_filter\n");
	EV_ERR("echo event_number\n");

	return snprintf(buf, 256,"please key in \n"
	"echo cmd,value >"
	"/sys/devices/virtual/misc/rtk_eventlog/eventlog_cmd\n");
}

static
ssize_t eventlog_cmd_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long in_val;
	size_t ret = count;

	if(0 == strncmp(buf,"enable,",strlen("enable,")))
	{
		buf = buf + strlen("enable,");

		if(eventlog_get_param_single(16,&in_val,buf)){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(in_val == 1 || in_val == 0)
		{
    		eventlog_enable = in_val;
		}
		else{
			EV_ERR("enable must be 1 or 0\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}
	else if(0 == strncmp(buf,"filter_enable,",strlen("filter_enable,")))
	{
		buf = buf + strlen("filter_enable,");

		if(eventlog_get_param_single(16,&in_val,buf)){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(in_val == 1 || in_val == 0)
		{
    		eventlog_filter_enable = in_val;
		}
		else{
			EV_ERR("filter_enable must be 1 or 0\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}
	else if(0 == strncmp(buf,"filter,",strlen("filter,")))
	{
		unsigned long long param[8];

		buf = buf + strlen("filter,");
		if(eventlog_get_param_multi(PR_TYPE_HEX, 8, &param_cnt,param,buf,count)){
			EV_ERR("(%s)get param fail\n",__func__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		if(param_cnt != 8){
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		if(EVENT_ERROR == event_log_filter_set(param[0], param[1], param[2], param[3], param[4], param[5], param[6], param[7]))
		{
			ret = -EINVAL;
			goto FINISH_OUT;
		}

	}
	else if(0 == strncmp(buf,"time_unit,",strlen("time_unit,")))
	{
		buf = buf + strlen("time_unit,");

		if(eventlog_get_param_single(16,&in_val,buf)){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(in_val == 1 || in_val == 0)
		{
    		eventlog_time_unit = in_val;
		}
		else{
			EV_ERR("time_unit must be 1 or 0\n");
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}
	else if(0 == strncmp(buf,"event_number,",strlen("event_number,")))
	{
		unsigned long long param[4];

		buf = buf + strlen("event_number,");
		if(eventlog_get_param_multi(PR_TYPE_HEX, 4, &param_cnt,param,buf,count)){
			EV_ERR("(%s)get param fail\n",__func__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		if(param_cnt != 4){
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		eventlog_event_printf_number(param[0], param[1], param[2], param[3]);
	}
	else if(0 == strncmp(buf,"event_one_diff,",strlen("event_one_diff,")))
	{
		unsigned long long param[4];

		buf = buf + strlen("event_one_diff,");
		if(eventlog_get_param_multi(PR_TYPE_HEX, 4, &param_cnt,param,buf,count)){
			EV_ERR("(%s)get param fail\n",__func__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		if(param_cnt != 4){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
		if(EVENT_ERROR == eventlog_event_diff(param[0], param[1], param[2], param[3])){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}
	else if(0 == strncmp(buf,"show_define",strlen("show_define")))
	{
		EV_INFO("enter eventlog_show_define_dev_show\n");
		eventlog_show_define();
	}
	else if(0 == strncmp(buf,"event_two_diff,",strlen("event_two_diff,")))
	{
		unsigned long long param[8];
		unsigned int DW11 = 0;
		unsigned int DW12 = 0;

		buf = buf + strlen("event_two_diff,");
		if(eventlog_get_param_multi(PR_TYPE_HEX, 8, &param_cnt,param,buf,count)){
			EV_ERR("(%s)get param fail\n",__func__);
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		if(param_cnt != 8){
			ret = -EINVAL;
			goto FINISH_OUT;
		}

		DW11 = eventlog_merge_DW1(param[0], param[1], param[2], param[3]);
		DW12 = eventlog_merge_DW1(param[4], param[5], param[6], param[7]);

		if(EVENT_ERROR == eventlog_event_two_diff(DW11,DW12)){
			ret = -EINVAL;
			goto FINISH_OUT;
		}
	}
	else if(0 == strncmp(buf,"clean_memory",strlen("clean_memory")))
	{
		memset(log_memory, 0, sizeof(log_memory));
		memset(time_memory, 0, sizeof(time_memory));
		memory_count = 0;
	}
	else if(0 == strncmp(buf,"clean_filter",strlen("clean_filter")))
	{
		memset(fw_filter_data, 0, sizeof(fw_filter_data));
	}
	else
	{
		EV_ERR("NO find cmd\n");
		ret = -EINVAL;
		goto FINISH_OUT;
	}

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(eventlog_cmd, S_IRUGO | S_IWUSR,
		eventlog_cmd_dev_show,
		eventlog_cmd_dev_store);

static
ssize_t eventlog_save_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	return snprintf(buf, 256,"please key in \n"
	"echo fw_id,eventtype_id,module_id,event_id,event_value,module_reserved > "
		"/sys/devices/virtual/misc/rtk_eventlog/eventlog_save\n");
}

static
ssize_t eventlog_save_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	int param_cnt;
	unsigned long long param[6];
	size_t ret = count;

	if(eventlog_get_param_multi(PR_TYPE_HEX,6,&param_cnt,param,buf,count)){
		EV_ERR("(%s)get param fail\n",__func__);
		ret = -EINVAL;
		goto FINISH_OUT;
	}

	if(param_cnt != 6){
		ret = -EINVAL;
		goto FINISH_OUT;
	}
	if(EVENT_ERROR == event_log_kernel_save(param[0], param[1], param[2], param[3], param[4], param[5]))
	{
		ret = -EINVAL;
		EV_ERR("(%s)save log fail\n",__func__);
		goto FINISH_OUT;
	}

FINISH_OUT:
	return ret;
}

DEVICE_ATTR(eventlog_save, S_IRUGO | S_IWUSR,
		eventlog_save_dev_show,
		eventlog_save_dev_store);


static
ssize_t eventlog_log_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	int  count = PAGE_SIZE;

	memset(output_buf_log, 0, PAGE_SIZE);
	eventlog_endian_swap(output_buf_log, log_memory, INT_PAGE_SIZE);
	memcpy(buf, output_buf_log, PAGE_SIZE);

	return count;
}

static
ssize_t eventlog_log_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	return count;
}

DEVICE_ATTR(eventlog_log, S_IRUGO | S_IWUSR,
		eventlog_log_dev_show,
		eventlog_log_dev_store);

static
ssize_t eventlog_parse_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	eventlog_parser_printf_all();
	return 0;
}

static
ssize_t eventlog_parse_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	return count;
}

DEVICE_ATTR(eventlog_parse, S_IRUGO | S_IWUSR,
		eventlog_parse_dev_show,
		eventlog_parse_dev_store);

static
ssize_t eventlog_show_define_dev_show(struct device *dev,
	struct device_attribute *attr,
	char *buf)
{
	EV_INFO("enter eventlog_show_define_dev_show\n");
	eventlog_show_define();
	return 0;
}

static
ssize_t eventlog_show_define_dev_store(struct device *dev,
	struct device_attribute *attr,
	const char *buf, size_t count)
{
	return count;
}

DEVICE_ATTR(eventlog_show_define, S_IRUGO | S_IWUSR,
		eventlog_show_define_dev_show,
		eventlog_show_define_dev_store);
/************************************************************************
 * MODULR INIT/EXIT
 ************************************************************************/
static
int __init rtk_eventlog_module_init(void)
{
    int retval = 0;

    if (misc_register(&rtk_eventlog_miscdev)) {
        EV_ERR("rtk_eventlog_module_init failed - register misc device failed\n");
        retval = -ENODEV;
        goto FINISH_OUT;
    }

	if (misc_register(&rtk_eventlog_parser_miscdev)) {
        EV_ERR("rtk_eventlog_module_init failed - register misc device failed\n");
        retval = -ENODEV;
        goto FINISH_OUT;
    }

	device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_show_define);
    device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_filter);
	device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_enable);
	device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_cmd);
	device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_save);
	device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_log);
	device_create_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_parse);
    retval = platform_device_register(&eventlog_drv_pm_device);
    if (retval)
        goto FINISH_OUT;

    retval = platform_driver_probe(&eventlog_drv_pm_driver, NULL);

    if (retval)
    {
        goto FINISH_OUT;
    }

    memset(fw_filter_data, 0, sizeof(fw_filter_data));

    EV_INFO("eventlog_kdv driver install success\n");

FINISH_OUT:
	return retval;
}

void __exit rtk_eventlog_module_exit(void)
{
	platform_driver_unregister(&eventlog_drv_pm_driver);

	device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_show_define);
    device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_filter);
	device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_enable);
	device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_cmd);
	device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_save);
	device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_log);
	device_remove_file(rtk_eventlog_miscdev.this_device, &dev_attr_eventlog_parse);
}

subsys_initcall(rtk_eventlog_module_init);
module_exit(rtk_eventlog_module_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Baron <baron_yuan@realtek.com>");
