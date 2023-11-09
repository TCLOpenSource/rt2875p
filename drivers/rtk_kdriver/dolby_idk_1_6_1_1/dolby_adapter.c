#include <linux/compiler.h>
#include <linux/string.h>
#include <linux/scatterlist.h>
#include <linux/of_irq.h>
#include "dolby_device.h"
#include <rbus/sb2_reg.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/scalerdrv/scaler_hdr_ctrl.h>
#include "dolby_adapter.h"
#include <rtd_log/rtd_module_log.h>

//static size_t dolby_vision_private_key_size = sizeof(dolby_vision_private_key);
static struct dolby_interface adapter;

/* dolby_proverlay_background_h_start_end */
unsigned int dolby_adapter_get_dolby_proverlay_background_h_start_end(void)
{
	return likely(adapter.get_dolby_proverlay_background_h_start_end) ? adapter.get_dolby_proverlay_background_h_start_end() : 0;
}

/* dolby_proverlay_background_v_start_end */
unsigned int dolby_adapter_get_dolby_proverlay_background_v_start_end(void)
{
	return likely(adapter.get_dolby_proverlay_background_v_start_end) ? adapter.get_dolby_proverlay_background_v_start_end() : 0;
}

/* dolby_letter_box_spinlock */
unsigned char dolby_adapter_get_letter_box_black_flag(void)
{
	return likely(adapter.get_letter_box_black_flag) ? adapter.get_letter_box_black_flag() : 0;
}

/* dolby_letter_box_spinlock */
void dolby_adapter_set_letter_box_black_flag(unsigned char flag)
{
	if (likely(adapter.set_letter_box_black_flag))
		adapter.set_letter_box_black_flag(flag);
}

/* letter_box_black_flag */
unsigned char dolby_adapter_get_request_letter_dtg_change(void)
{
	return likely(adapter.get_request_letter_dtg_change) ? adapter.get_request_letter_dtg_change() : 0;
}

/* letter_box_black_flag */
void dolby_adapter_set_request_letter_dtg_change(unsigned char flag)
{
	if (likely(adapter.set_request_letter_dtg_change))
		adapter.set_request_letter_dtg_change(flag);
}

/* dolby_letter_box_spinlock */
void dolby_adapter_dolby_letter_box_spin_lock_irq_save(unsigned long *flags)
{
	if (likely(adapter.dolby_letter_box_spin_lock_irqsave))
		adapter.dolby_letter_box_spin_lock_irqsave(flags);
}

/* dolby_letter_box_spinlock */
void dolby_adapter_dolby_letter_box_spin_unlock_irqrestore(unsigned long *flags)
{
	if (likely(adapter.dolby_letter_box_spin_unlock_irqrestore))
		adapter.dolby_letter_box_spin_unlock_irqrestore(flags);
}

/* HDMI_TEST */
void dolby_adapter_HDMI_TEST(unsigned int wid, unsigned int len, unsigned char *mdAddr)
{
	if (likely(adapter.HDMI_TEST))
		adapter.HDMI_TEST(wid, len, mdAddr);
}

/* Normal_TEST */
void dolby_adapter_Normal_TEST(void *p_mdOutput, unsigned int rpcType)
{
	if (likely(adapter.Normal_TEST))
		adapter.Normal_TEST(p_mdOutput, rpcType);
}

void dolby_adapter_force_run_ott(void)
{
	if (likely(adapter.force_run_ott))
		adapter.force_run_ott();
}

void dolby_adapter_dolby_trigger_timer6(void)
{
	if (likely(adapter.dolby_trigger_timer6))
		adapter.dolby_trigger_timer6();
}

unsigned int dolby_adapter_get_g_picModeUpdateFlag(void)
{
	return likely(adapter.get_g_picModeUpdateFlag) ? adapter.get_g_picModeUpdateFlag() : 0;
}

void dolby_adapter_set_g_picModeUpdateFlag(unsigned int value)
{
	if (likely(adapter.set_g_picModeUpdateFlag))
		adapter.set_g_picModeUpdateFlag(value);
}

/* DV_ForceSTOP */
int dolby_adapter_DV_ForceSTOP(unsigned long process_id)
{
	return likely(adapter.DV_ForceSTOP) ? adapter.DV_ForceSTOP(process_id) : 0;
}
EXPORT_SYMBOL(dolby_adapter_DV_ForceSTOP);

/* Check_DV_Mode() */
void dolby_adapter_Check_DV_Mode(void)
{
	if (likely(adapter.Check_DV_Mode))
		adapter.Check_DV_Mode();
}

/* DV_HDMI_Init() */
int dolby_adapter_DV_HDMI_Init(void)
{
	return likely(adapter.DV_HDMI_Init) ? adapter.DV_HDMI_Init() : 0;
}

/* check_ott_dolby_update_run() */
unsigned char dolby_adapter_check_ott_dolby_update_run(void)
{
	return likely(adapter.check_ott_dolby_update_run) ? adapter.check_ott_dolby_update_run() : 0;
}

int dolby_adapter_dolby_vision_suspend(struct device *p_dev)
{
	return likely(adapter.dolby_vision_suspend) ? adapter.dolby_vision_suspend(p_dev) : 0;
}

int dolby_adapter_dolby_vision_resume(struct device *p_dev)
{
	return likely(adapter.dolby_vision_resume) ? adapter.dolby_vision_resume(p_dev) : 0;
}

int dolby_adapter_get_support_status(void)
{
	return likely(adapter.get_support_status) ? adapter.get_support_status() : 0;
}

/* dolby_load_pq_file() */
int dolby_adapter_dolby_load_pq_file(char *pq_table_filepath)
{
	return likely(adapter.dolby_load_pq_file) ? adapter.dolby_load_pq_file(pq_table_filepath) : 0;
}

/* DV_RingBuffer_Init() */
int dolby_adapter_dolby_vision_ringbuffer_init(void *data)
{
	return likely(adapter.dolby_vision_ringbuffer_init) ? adapter.dolby_vision_ringbuffer_init(data) : 0;
}

/* DV_Stop(void) */
int dolby_adapter_dolby_vision_stop(void)
{
	return likely(adapter.dolby_vision_stop) ? adapter.dolby_vision_stop() : 0;
}

/* DV_Run(void) */
int dolby_adapter_dolby_vision_run(void)
{
	return likely(adapter.dolby_vision_run) ? adapter.dolby_vision_run() : 0;
}

/* DV_Pause(void) */
int dolby_adapter_dolby_vision_pause(void)
{
	return likely(adapter.dolby_vision_pause) ? adapter.dolby_vision_pause() : 0;
}

/* DV_Flush() */
int dolby_adapter_dolby_vision_flush(void)
{
	return likely(adapter.dolby_vision_flush) ? adapter.dolby_vision_flush() : 0;
}

/* ui_dv_backlight_value */
int dolby_adapter_get_ui_dv_backlight_value(void)
{
	return likely(adapter.get_ui_dv_backlight_value) ? adapter.get_ui_dv_backlight_value() : 0;
}

void dolby_adapter_set_ui_dv_backlight_value(int value)
{
	if (likely(adapter.set_ui_dv_backlight_value))
		adapter.set_ui_dv_backlight_value(value);
}

/* ui_dv_brightness_value */
int dolby_adapter_get_ui_dv_brightness_value(void)
{
	return likely(adapter.get_ui_dv_brightness_value) ? adapter.get_ui_dv_brightness_value() : 0;
}

void dolby_adapter_set_ui_dv_brightness_value(int value)
{
	if (likely(adapter.set_ui_dv_brightness_value))
		adapter.set_ui_dv_brightness_value(value);
}

/* ui_dv_color_value */
int dolby_adapter_get_ui_dv_color_value(void)
{
	return likely(adapter.get_ui_dv_color_value) ? adapter.get_ui_dv_color_value() : 0;
}

void dolby_adapter_set_ui_dv_color_value(int value)
{
	if (likely(adapter.set_ui_dv_color_value))
		adapter.set_ui_dv_color_value(value);
}

/* ui_dv_contrast_value */
int dolby_adapter_get_ui_dv_contrast_value(void)
{
	return likely(adapter.get_ui_dv_contrast_value) ? adapter.get_ui_dv_contrast_value() : 0;
}

void dolby_adapter_set_ui_dv_contrast_value(int value)
{
	if (likely(adapter.set_ui_dv_contrast_value))
		adapter.set_ui_dv_contrast_value(value);
}

/* ui_dv_picmode */
int dolby_adapter_get_ui_dv_picmode(void)
{
	return likely(adapter.get_ui_dv_picmode) ? adapter.get_ui_dv_picmode() : 0;
}

void dolby_adapter_set_ui_dv_picmode(int mode)
{
	if (likely(adapter.set_ui_dv_picmode))
		adapter.set_ui_dv_picmode(mode);
}

/* g_dv_pq_sem */
void dolby_adapter_g_dv_pq_sem_down(void)
{
	if (likely(adapter.g_dv_pq_sem_down))
		adapter.g_dv_pq_sem_down();
}

void dolby_adapter_g_dv_pq_sem_up(void)
{
	if (likely(adapter.g_dv_pq_sem_up))
		adapter.g_dv_pq_sem_up();
}

/* hdmi_ui_change_flag */
void dolby_adapter_set_hdmi_ui_change_flag(int value)
{
	if (likely(adapter.set_hdmi_ui_change_flag))
		adapter.set_hdmi_ui_change_flag(value);
}

/* rtk_timer_dolbyvision_interrupt */
irqreturn_t dolby_adapter_rtk_timer_dolbyvision_interrupt(int irq, void *dev_id)
{
	return likely(adapter.rtk_timer_dolbyvision_interrupt) ? adapter.rtk_timer_dolbyvision_interrupt(irq, dev_id) : IRQ_NONE;
}

/* dolby_load_pq_cfg_file */
int dolby_adapter_set_pq_table_cfg_path(char **cfg_path, int num)
{
	return likely(adapter.set_pq_table_cfg_path) ? adapter.set_pq_table_cfg_path(cfg_path, num) : 0;
}

void dolby_adapter_get_pq_table_cfg_path(char *cfg_path, int length)
{
	if (likely(adapter.get_pq_table_cfg_path))
		adapter.get_pq_table_cfg_path(cfg_path, length);
}

const char *dolby_adapter_get_dolby_vision_software_version(void)
{
	return likely(adapter.get_dolby_vision_software_version) ? adapter.get_dolby_vision_software_version() : NULL;
}

void dolby_adapter_set_dolby_pwm_ratio(int value)
{
	if (likely(adapter.set_dolby_pwm_ratio))
		adapter.set_dolby_pwm_ratio(value);
}

int dolby_adapter_get_dolby_pwm_ratio(void)
{
	return likely(adapter.get_dolby_pwm_ratio) ? adapter.get_dolby_pwm_ratio() : 0;
}

void dolby_adapter_set_gd_delay_table(void *data, size_t size)
{
	if (likely(adapter.set_gd_delay_table))
		adapter.set_gd_delay_table(data, size);
}

const uint8_t *dolby_adapter_get_gd_delay_table(size_t *size)
{
	return likely(adapter.get_gd_delay_table) ? adapter.get_gd_delay_table(size) : NULL;
}

void dolby_adapter_dm_crf_dump_testflow(void *dolby)
{
	if (likely(adapter.dm_crf_dump_testflow))
		adapter.dm_crf_dump_testflow(dolby);
}

void dolby_adapter_get_l11_content_type(uint8_t *l11)
{
	if (likely(adapter.get_l11_content_type))
		adapter.get_l11_content_type(l11);
}

bool dolby_adapter_is_force_dolby(void)
{
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
	extern bool is_force_dolby(void);

	return is_force_dolby();
#else
	return false;
#endif
}

void dolby_adapter_get_dolby_vision_idk_version(char *buffer, int size)
{
	if (likely(adapter.get_dolby_vision_idk_version))
		adapter.get_dolby_vision_idk_version(buffer, size);
}

void dolby_adapter_get_dolby_vision_dm_version(char *buffer, int size)
{
	if (likely(adapter.get_dolby_vision_dm_version))
		adapter.get_dolby_vision_dm_version(buffer, size);
}

int dolby_adapter_get_current_gd_delay(void)
{
	return (likely(adapter.get_current_gd_delay)) ? adapter.get_current_gd_delay() : 0;
}

const uint8_t *dolby_adapter_get_current_target_config(size_t *size)
{
	return likely(adapter.get_current_target_config) ? adapter.get_current_target_config(size) : NULL;
}

int dolby_adapter_get_running_mode(void)
{
	return likely(adapter.get_running_mode) ? adapter.get_running_mode() : 0;
}

void dolby_adapter_get_resolution(int *resolution)
{
	if (likely(adapter.get_resolution))
		adapter.get_resolution(resolution);
}

int dolby_adapter_wait_pq_table_cfg_path(char *cfg_path, int length)
{
	return (likely(adapter.wait_pq_table_cfg_path)) ? adapter.wait_pq_table_cfg_path(cfg_path, length) : -1;
}

ssize_t dolby_adapter_pq_table_bin_write(void *filp, const char __user *buffer, size_t count, loff_t *ppos, bool is_user_buffer)
{
	return (likely(adapter.pq_table_bin_write)) ? adapter.pq_table_bin_write(filp, buffer, count, ppos, is_user_buffer) : -1;
}


void dolby_adapter_set_ambient_mode(int mode)
{
	if (likely(adapter.set_ambient_mode))
		adapter.set_ambient_mode(mode);
}

int dolby_adapter_get_ambient_mode(void)
{
	return  (likely(adapter.get_ambient_mode)) ? adapter.get_ambient_mode() : 0;
}

void dolby_adapter_set_ambient_front(int64_t tFrontLux)
{
	if (likely(adapter.set_ambient_front))
		adapter.set_ambient_front(tFrontLux);
}

int64_t dolby_adapter_get_ambient_front(void)
{
	return  (likely(adapter.get_ambient_front)) ? adapter.get_ambient_front() : 0;
}

void dolby_adapter_set_ambient_rear(int64_t tRearLum)
{
	if (likely(adapter.set_ambient_rear))
		adapter.set_ambient_rear(tRearLum);
}

int64_t dolby_adapter_get_ambient_rear(void)
{
	return  (likely(adapter.get_ambient_rear)) ? adapter.get_ambient_rear() : 0;
}

void dolby_adapter_set_ambient_whitexy(int tWhiteX, int tWhiteY)
{
	if (likely(adapter.set_ambient_whitexy))
		adapter.set_ambient_whitexy(tWhiteX, tWhiteY);
}

void dolby_adapter_get_ambient_whitexy(int *tWhiteX, int *tWhiteY)
{
	if (likely(adapter.get_ambient_whitexy)) {
		adapter.get_ambient_whitexy(tWhiteX, tWhiteY);
	} else {
		*tWhiteX = 0;
		*tWhiteY = 0;
	}
}

void dolby_adapter_dolby_init(void)
{
	if (likely(adapter.dolby_init))
		adapter.dolby_init();
}

void dolby_adapter_clear_view_mode_id(void)
{
	if (likely(adapter.clear_view_mode_id))
		adapter.clear_view_mode_id();
}

void dolby_adapter_set_block_ambient_call(bool enable)
{
	if (likely(adapter.set_block_ambient_call))
		adapter.set_block_ambient_call(enable);
}

bool dolby_adapter_get_block_ambient_call(void)
{
	return  (likely(adapter.get_block_ambient_call)) ? adapter.get_block_ambient_call() : false;
}

void dolby_adapter_set_ambient_mode_force(int mode)
{
	if (likely(adapter.set_ambient_mode_force))
		adapter.set_ambient_mode_force(mode);
}

void dolby_adapter_set_ambient_front_force(int64_t tFrontLux)
{
	if (likely(adapter.set_ambient_front_force))
		adapter.set_ambient_front_force(tFrontLux);
}

void dolby_adapter_set_ambient_front_lux(uint32_t lux)
{
	if (likely(adapter.set_ambient_front_lux))
			adapter.set_ambient_front_lux(lux);
}

uint32_t dolby_adapter_get_ambient_front_lux(void)
{
	return (likely(adapter.get_ambient_front_lux)) ? adapter.get_ambient_front_lux() : 0;
}


void dolby_adapter_set_gd_delay_base_hdmi(int delay)
{
    if (likely(adapter.set_gd_delay_base_hdmi))
        adapter.set_gd_delay_base_hdmi(delay);
}

int dolby_adapter_get_gd_delay_base_hdmi(void)
{
    return (likely(adapter.get_gd_delay_base_hdmi)) ? adapter.get_gd_delay_base_hdmi() : 0;
}

void dolby_adapter_set_gd_delay_base_ott(int delay)
{
    if (likely(adapter.set_gd_delay_base_ott))
        adapter.set_gd_delay_base_ott(delay);
}

int dolby_adapter_get_gd_delay_base_ott(void)
{
    return (likely(adapter.get_gd_delay_base_ott)) ? adapter.get_gd_delay_base_ott() : 0;
}

void dolby_adapter_set_gd_delay_base_hdmi_ll(int delay)
{
    if (likely(adapter.set_gd_delay_base_hdmi_ll))
        adapter.set_gd_delay_base_hdmi_ll(delay);
}

int dolby_adapter_get_gd_delay_base_hdmi_ll(void)
{
    return (likely(adapter.get_gd_delay_base_hdmi_ll)) ? adapter.get_gd_delay_base_hdmi_ll() : 0;
}
void dolby_adapter_execute_guard_func(void)
{
    if (likely(adapter.execute_guard_func))
        adapter.execute_guard_func();
}

void dolby_adapater_set_ott_pause(bool enable)
{
    if (likely(adapter.set_ott_pause))
        adapter.set_ott_pause(enable);
}

bool dolby_adapater_get_ott_pause_status(void)
{
    return (likely(adapter.get_ott_pause_status)) ? adapter.get_ott_pause_status() : false;
}

int dolby_adapter_get_ambient_target_mode(void)
{
	return  (likely(adapter.get_ambient_target_mode)) ? adapter.get_ambient_target_mode() : 0;
}

void dolby_adapater_set_ott_hdmi_backlight_delay_frame(int ott_delay_frame[], int hdmi_delay_frame[])
{
    if (likely(adapter.set_ott_hdmi_backlight_delay_frame))
        adapter.set_ott_hdmi_backlight_delay_frame(ott_delay_frame, hdmi_delay_frame);
}

int dolby_adapter_handle_vsem_packet(void *data, int count, bool apply_setting)
{
	return (likely(adapter.handle_vsem_packet)) ? adapter.handle_vsem_packet(data, count, apply_setting) : 0;
}

void dolby_adapter_set_dolby_vsem_type(unsigned char port, int type)
{
	if (likely(adapter.set_dolby_vsem_type))
		adapter.set_dolby_vsem_type(port, type);
}

void dolby_adapter_wait_next_frame(void)
{
	if (likely(adapter.wait_next_frame))
		adapter.wait_next_frame();
}

#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
void dolby_adapter_wait_dolby_vision_start(void)
{
	if (likely(adapter.wait_dolby_vision_start))
		adapter.wait_dolby_vision_start();
}

#endif
int dolby_adapter_get_dolby_vsem_type(unsigned char port)
{
	int result = DOLBY_VISION_VSEM_NONE;

#ifdef CONFIG_ENABLE_DOLBY_VISION_VSEM
	extern bool is_available_dolby_vsem(unsigned char port);

	if (is_available_dolby_vsem(port))
		result = (likely(adapter.get_dolby_vsem_type)) ? adapter.get_dolby_vsem_type(port) : DOLBY_VISION_VSEM_NONE;

#endif
	return result;
}


long dolby_adapter_check_capability(void *table, size_t size)
{
	return (likely(adapter.check_capability)) ? adapter.check_capability(table, size) : false;
}

void *dolby_adapter_get_maker_id(size_t *size)
{
	return (likely(adapter.get_maker_id)) ? adapter.get_maker_id(size) : false;
}

long dolby_adapter_set_model_hash(void *model_hash, size_t size)
{
	return (likely(adapter.set_model_hash)) ? adapter.set_model_hash(model_hash, size) : -EINVAL;
}

irqreturn_t dolby_adapter_md_parser_interrupt(int irq, void *dev_id)
{
	const uint32_t INT_SCALER_9 = 1 << 9;
	
	uint32_t itr = IoReg_Read32(SB2_CPU_SW_INT_2_reg);
	irqreturn_t ret = IRQ_NONE;

	if ((itr & INT_SCALER_9) == 0)
		goto dolby_adapter_md_parser_interrupt_out;

	ret = IRQ_HANDLED;

	IoReg_Write32(SB2_CPU_SW_INT_2_reg, INT_SCALER_9 | (0 << 0));

	if (Get_DisplayMode_Src(SLR_MAIN_DISPLAY) == VSC_INPUTSRC_HDMI
		&& (get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_h14B_VSIF
			|| get_HDMI_Dolby_VSIF_mode() == DOLBY_HDMI_VSIF_STD)) {

		if (!(get_HDMI_HDR_mode() == HDR_DOLBY_HDMI
				&& HDR_DolbyVision_Get_CurHdmiMode() != DOLBY_HDMI_OFF)) {
			goto dolby_adapter_md_parser_interrupt_out;
		}
	}
	
	if (likely(adapter.md_parser_interrupt)) 
		adapter.md_parser_interrupt();

dolby_adapter_md_parser_interrupt_out:
	return ret;
}

#define DOLBY_VISION_DEVICE_NAME	"dolby_vision"
#define DOLBY_VISION_SOFTWARE_IRQ       "SOFTWARE_IRQ"
#define DOLBY_VISION_MISC_IRQ           "MISC_IRQ"
long register_dolby_vision_isr(void *device)
{
	struct device_node *dolby_vision_device = of_find_node_by_name(NULL, DOLBY_VISION_DEVICE_NAME);
	int irq_misc, irq_cpu2cpu;
	int err = 0;

	if (dolby_vision_device == NULL) {
		rtd_pr_hdr_err("could not find device %s from device tree\n", DOLBY_VISION_DEVICE_NAME);
		return -1;
	}

	irq_misc = of_irq_get_byname(dolby_vision_device, DOLBY_VISION_MISC_IRQ);
	if (irq_misc >= 0) {
		err = request_irq(irq_misc, dolby_adapter_rtk_timer_dolbyvision_interrupt, IRQF_SHARED, "timer_dv",device);
		if (err) {
			rtd_pr_hdr_err("[dolby] timer_dv: can't get assigned irq %d (%s)\n", irq_misc, DOLBY_VISION_MISC_IRQ);
		} else {
			rtd_pr_hdr_info("[%s:%d][dolby] register interruput success\n", __func__, __LINE__);
		}
	} else {
		rtd_pr_hdr_err("[DolbyVision] get irq %s fail\n", DOLBY_VISION_MISC_IRQ);
	}

	irq_cpu2cpu = of_irq_get_byname(dolby_vision_device, DOLBY_VISION_SOFTWARE_IRQ);
	if (irq_cpu2cpu >= 0) {
		err = request_irq(irq_cpu2cpu, dolby_adapter_md_parser_interrupt, IRQF_SHARED, "md_parser", device);
		if (err) {
			rtd_pr_hdr_err("[dolby] messaage: can't get assigned irq %d (%s)\n", irq_cpu2cpu, DOLBY_VISION_SOFTWARE_IRQ);
		} else {
			rtd_pr_hdr_info("[%s:%d][dolby] register interruput2 success\n", __func__, __LINE__);
		}
	} else {
		rtd_pr_hdr_err("[DolbyVision] get irq %s fail\n", DOLBY_VISION_SOFTWARE_IRQ);
	}

	return 0;
}

long release_dolby_vision_isr(void *device)
{
	struct device_node *dolby_vision_device = of_find_node_by_name(NULL, DOLBY_VISION_DEVICE_NAME);
	int irq_misc, irq_cpu2cpu;

	if (dolby_vision_device == NULL) {
		rtd_pr_hdr_err("could not find device %s from device tree\n", DOLBY_VISION_DEVICE_NAME);
		return -1;
	}

	irq_misc = of_irq_get_byname(dolby_vision_device, DOLBY_VISION_MISC_IRQ);
	irq_cpu2cpu = of_irq_get_byname(dolby_vision_device, DOLBY_VISION_SOFTWARE_IRQ);

	if (irq_misc >= 0)
		free_irq(irq_cpu2cpu, device);

	if (irq_cpu2cpu >= 0)
		free_irq(irq_misc, device);

	return 0;
}

void *dolby_adapter_pq_text2bin(uint8_t *buf_factory, size_t size_factory, uint8_t *buf_bestpq, size_t size_best_pq,  size_t *size_pq_bin)
{
	return (likely(adapter.pq_text2bin)) ? adapter.pq_text2bin(buf_factory, size_factory, buf_bestpq, size_best_pq, size_pq_bin) : NULL;
}

void dolby_adapter_worker(void)
{
	if (likely(adapter.worker))
		adapter.worker();
}

void dolby_adapter_get_dm_data(void *p_comp_reg, void *p_dm_reg, void *p_dm_lut, bool is_user_buffer)
{
	if (likely(adapter.get_dm_data))
		adapter.get_dm_data(p_comp_reg, p_dm_reg, p_dm_lut, is_user_buffer);
}

void dolby_adapter_get_dm_struct_size(int *size_composer, int *size_dm_register, int *size_dm_lut)
{
	if (likely(adapter.get_dm_struct_size))
		adapter.get_dm_struct_size(size_composer, size_dm_register, size_dm_lut);
}

void dolby_adapter_clear_dump_dm_register_counter(void)
{
	if (likely(adapter.clear_dump_dm_register_counter))
		adapter.clear_dump_dm_register_counter();
}

void dolby_adapter_set_update_crf_ambient_flag(bool enable)
{
	if (likely(adapter.set_update_crf_ambient_flag))
		adapter.set_update_crf_ambient_flag(enable);
}

void dolby_adapter_set_update_crf_dark_detail_flag(bool enable)
{
	if (likely(adapter.set_update_crf_dark_detail_flag))
		adapter.set_update_crf_dark_detail_flag(enable);
}

void dolby_adapter_set_update_crf_dtv_flag(bool enable)
{
	if (likely(adapter.set_update_crf_dtv_flag))
		adapter.set_update_crf_dtv_flag(enable);
}

void dolby_adapter_set_update_crf_best_pq_flag(bool enable)
{
	if (likely(adapter.set_update_crf_best_pq_flag))
		adapter.set_update_crf_best_pq_flag(enable);
}

void dolby_adapter_set_dark_detail(int32_t dark_detail)
{
	if (likely(adapter.set_dark_detail))
		adapter.set_dark_detail(dark_detail);
}

bool set_dolby_adapter(struct dolby_interface *interface, size_t size)
{
	bool ret = false;

	const size_t interface_size = sizeof(struct dolby_interface);
	size_t copy_size = min(interface_size, size);

	memset(&adapter, 0, interface_size);
	memcpy(&adapter, interface, copy_size);

	ret = true;

	return ret;
}
EXPORT_SYMBOL(set_dolby_adapter);

void dolby_adapter_release_hdmi_backlight_delay_queue(void)
{
	if (likely(adapter.release_hdmi_backlight_delay_queue))
		adapter.release_hdmi_backlight_delay_queue();
}

