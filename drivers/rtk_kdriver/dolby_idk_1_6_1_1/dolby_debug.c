#include <stddef.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
#include <tvscalercontrol/scaler_vfedev.h>
#endif

#include "dolby_adapter.h"
#include <rtd_log/rtd_module_log.h>

static bool debug_parser_time;
static int ambient_light_change_time;
static bool flag_ott_pause;
static bool stop_apply_dolby;

bool get_flag_print_parser_time(void)
{
	bool return_val = debug_parser_time;

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	return_val = true;
#endif
	return return_val;
}
EXPORT_SYMBOL(get_flag_print_parser_time);

bool get_flag_stop_apply_dolby(void)
{
	return stop_apply_dolby;
}
EXPORT_SYMBOL(get_flag_stop_apply_dolby);

/* create debugfs */

static ssize_t view_mode_id_read(struct file *filp, char __user *buffer,
		size_t count, loff_t *ppos)
{
	int n, copied;
	int view_mode_id = dolby_adapter_get_ui_dv_picmode();

	#define view_mode_id_string_size 127
	char view_mode_id_string[view_mode_id_string_size + 1] = { 0 };

	snprintf(view_mode_id_string,
			view_mode_id_string_size,
			"%d\n",
			view_mode_id
		);

	if (*ppos >= view_mode_id_string_size)
		return 0;

	if (*ppos + count > view_mode_id_string_size)
		count = view_mode_id_string_size - *ppos;

	n = copy_to_user(buffer, view_mode_id_string + *ppos, count);

	copied = count - n;
	*ppos += copied;

	return copied;
}

static ssize_t view_mode_id_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	int ret;
	unsigned long long view_mode_id;

	if (*ppos != 0)
		goto view_mode_id_write_out;

	ret = kstrtoull_from_user(buffer, count, 10, (unsigned long long *)&view_mode_id);
	if (ret)
		goto view_mode_id_write_out;

	rtd_pr_hdr_debug("[DolbyVision][%s:%d] set view mode id = %llu\n", __func__, __LINE__, view_mode_id);

	dolby_adapter_g_dv_pq_sem_down();
	dolby_adapter_set_ui_dv_picmode((int) view_mode_id);
	dolby_adapter_g_dv_pq_sem_up();

	dolby_adapter_set_hdmi_ui_change_flag(true);

view_mode_id_write_out:
	*ppos += count;

	return count;
}

struct file_operations view_mode_id_fops = {
	.owner = THIS_MODULE,
	.read  = view_mode_id_read,
	.write = view_mode_id_write,
};

static void create_picmode_debugfs (struct dentry *dolby_debugfs_root)
{
	debugfs_create_file("picture_mode", 0666, dolby_debugfs_root, NULL, &view_mode_id_fops);
}

/* debug/cfg */
static ssize_t pq_table_cfg_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	return dolby_adapter_pq_table_bin_write((void *)filp, buffer, count, ppos, true);
}

static struct file_operations pq_table_cfg_fops = {
	.owner = THIS_MODULE,
	.write = pq_table_cfg_write,
};

static void create_pqtable_debugfs(struct dentry *dolby_debugfs_root)
{
	debugfs_create_file("cfg", 0222, dolby_debugfs_root, NULL, &pq_table_cfg_fops);
}

static ssize_t pq_table_cfg_filepath_read(struct file *filp, char __user *buffer,
		size_t count, loff_t *ppos)
{
	char pq_cfg_filename[128];
	size_t pq_cfg_filename_length = ARRAY_SIZE(pq_cfg_filename);

	memset(pq_cfg_filename, 0, pq_cfg_filename_length);

	dolby_adapter_get_pq_table_cfg_path(pq_cfg_filename, pq_cfg_filename_length);

	if (*ppos >= pq_cfg_filename_length)
		return 0;

	if (*ppos + count > pq_cfg_filename_length)
		count = pq_cfg_filename_length - *ppos;

	if (copy_to_user(buffer, pq_cfg_filename + *ppos, count))
		return -EFAULT;

	*ppos += count;

	return count;
}

static ssize_t pq_table_cfg_filepath_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	int n, copied = 0;
	int cfg_path_size = 127;
	char *cfg_path;
	char *path_array[3] = { NULL };

	if ((*ppos + count > cfg_path_size) || *ppos != 0) {
		copied = count;
		*ppos += count;
		goto pq_table_cfg_filepath_write_err;
	}

	cfg_path = kzalloc(cfg_path_size + 1, GFP_KERNEL);
	if (cfg_path == NULL)
		goto pq_table_cfg_filepath_write_err;

	n = copy_from_user(cfg_path + *ppos, buffer, count);
	copied = count - n;
	*ppos += copied;

	rtd_pr_hdr_debug("[%s:%d][Dolby] get cfg file path = %s\n", __func__, __LINE__, cfg_path);

	path_array[0] = cfg_path;
	dolby_adapter_set_pq_table_cfg_path(path_array, 3);

	kfree(cfg_path);

pq_table_cfg_filepath_write_err:
	return copied;
}

struct file_operations pq_table_cfg_filepath_fops = {
	.owner = THIS_MODULE,
	.read  = pq_table_cfg_filepath_read,
	.write = pq_table_cfg_filepath_write,
};

static void create_pqtable_filepath_debugfs(struct dentry *dolby_debugfs_root)
{
	debugfs_create_file("cfg_filepath", 0666, dolby_debugfs_root, NULL, &pq_table_cfg_filepath_fops);
}

static void create_pqtable_debug_debugfs(struct dentry *dolby_debugfs_root)
{
	struct dentry *debug_debugfs_root = debugfs_create_dir("debug", dolby_debugfs_root);

	debugfs_create_bool("parser_time", 0666, debug_debugfs_root, (bool *)&debug_parser_time);
	debugfs_create_u32("ai_brightness_target_change_time", 0666, debug_debugfs_root, (uint32_t *)&ambient_light_change_time);
	debugfs_create_bool("ott_pause", 0666, debug_debugfs_root, (bool *)&flag_ott_pause);
	debugfs_create_bool("stop_apply_dolby", 0666, debug_debugfs_root, (bool *)&stop_apply_dolby);
}

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
static unsigned int bit_depth = 12;
static unsigned int dm_src_color_format = 0;

unsigned int get_bit_depth(void)
{
	return bit_depth;
}

void set_bit_depth(unsigned int depth)
{
	bit_depth = depth;
}

unsigned int get_dm_src_color_format(void)
{
	return dm_src_color_format;
}

void set_dm_src_color_format(unsigned int format)
{
	dm_src_color_format = format;
}

static unsigned int signal_format = 0;
unsigned int get_signal_format(void)
{
	return signal_format;
}

void set_signal_format(unsigned int format)
{
	signal_format = format;
}

static bool flag_force_dolby;
static bool flag_dump_dm_register;

bool is_dump_dm_register_enable(void)
{
	return flag_dump_dm_register;
}

void set_dump_dm_register(bool enable)
{
	flag_dump_dm_register = enable;
}

bool is_force_dolby(void)
{
	return flag_force_dolby;
}

void set_force_dolby(bool enable)
{
	flag_force_dolby = enable;
}

static int dolby_crf_hdmi_open(struct inode *inode, struct file *filp)
{
	return 0;
}

//dm_metadata_t temp_dm_metadata = { 0 };
static ssize_t dolby_crf_hdmi_write(struct file *filp, const char __user *buffer,
		size_t count, loff_t *ppos)
{
	int ret = count;
	unsigned char *cmd_buffer;

	cmd_buffer = (unsigned char *) kzalloc(count + 1, GFP_KERNEL);

	if (!cmd_buffer) {
		ret = -ENOMEM;
		goto dolby_crf_hdmi_write_out;
	}

	if (0 != copy_from_user(cmd_buffer, buffer, count)) {
		ret = -ENOMEM;
		goto dolby_crf_hdmi_write_out;
	}

	if (cmd_buffer[count - 1] == '\n')
		cmd_buffer[count - 1] = 0;

	if (0 == strncmp(cmd_buffer, "1", count)) {
		extern void reset_hdmi_timing_ready(void);
		extern void vfe_hdmi_drv_handle_on_line_measure_error(unsigned char vfe_call);
		extern struct semaphore* get_hdmi_detectsemaphore(void);
		extern unsigned char get_HDMI_Global_Status(void);
		extern void Set_Reply_Zero_Timing_Flag(unsigned char src, unsigned char flag);
		extern void HDMI_set_detect_flag(unsigned char enable);

	unsigned char REPORT_ZERO_TIMING = _BIT0;

		reset_hdmi_timing_ready();
		vfe_hdmi_drv_handle_on_line_measure_error(0);
		down(get_hdmi_detectsemaphore());
		if (get_HDMI_Global_Status() == SRC_CONNECT_DONE) {
			Set_Reply_Zero_Timing_Flag(VSC_INPUTSRC_HDMI, REPORT_ZERO_TIMING);/*Let Webos get zero timing*/
			HDMI_set_detect_flag(TRUE);
		}
		up(get_hdmi_detectsemaphore());
	} else if (0 == strncmp(cmd_buffer, "2", count)) {
		rtd_pr_hdr_info("[DolbyVision][%s:%d]\n", __func__, __LINE__);
		dolby_adapter_HDMI_TEST(1920, 1080, NULL);
	}

dolby_crf_hdmi_write_out:
	kfree(cmd_buffer);
	return ret;
}
struct file_operations dolby_crf_debugfs_fops = {
	.owner = THIS_MODULE,
	.open  = dolby_crf_hdmi_open,
	.write = dolby_crf_hdmi_write,
};

static void create_crf_debugfs(struct dentry *dolby_debugfs_root)
{
	struct dentry *crf_debugfs_root = debugfs_create_dir("crf", dolby_debugfs_root);
	debugfs_create_file("crf_hdmi", 0666, crf_debugfs_root, NULL, &dolby_crf_debugfs_fops);
	debugfs_create_u32("signal_format", 0666, crf_debugfs_root, (uint32_t *)&signal_format);
	debugfs_create_u32("bit_depth", 0666, crf_debugfs_root, (uint32_t *)&bit_depth);
	debugfs_create_u32("dm_src_color_format", 0666, crf_debugfs_root, (uint32_t *)&dm_src_color_format);
	debugfs_create_bool("force_dolby", 0666, crf_debugfs_root, (bool *)&flag_force_dolby);
	debugfs_create_bool("dump_dm_register", 0666, crf_debugfs_root, (bool *)&flag_dump_dm_register);
}
#endif

static struct dentry *dolby_debugfs_root;
void create_dolby_debugfs(void)
{
	dolby_debugfs_root = debugfs_create_dir("dolby", NULL);

	create_picmode_debugfs(dolby_debugfs_root);
	create_pqtable_debugfs(dolby_debugfs_root);
	create_pqtable_filepath_debugfs(dolby_debugfs_root);
	create_pqtable_debug_debugfs(dolby_debugfs_root);
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	create_crf_debugfs(dolby_debugfs_root);
#endif
}

void remove_dolby_debugfs(void)
{
	debugfs_remove_recursive(dolby_debugfs_root);
}
