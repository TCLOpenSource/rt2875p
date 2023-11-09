#ifndef	__DOLBY_IDK_1_6_1_1_DOLBY_ADAPTER_H__
#define	__DOLBY_IDK_1_6_1_1_DOLBY_ADAPTER_H__

#include <linux/interrupt.h>

struct dolby_interface {
	/* dolby_proverlay_background_h_start_end */
	unsigned int (*get_dolby_proverlay_background_h_start_end)(void);

	/* dolby_proverlay_background_v_start_end */
	unsigned int (*get_dolby_proverlay_background_v_start_end)(void);

	/* letter_box_black_flag */
	unsigned char (*get_letter_box_black_flag)(void);
	void (*set_letter_box_black_flag)(unsigned char flag);

	/* letter_box_black_flag */
	unsigned char (*get_request_letter_dtg_change)(void);
	void (*set_request_letter_dtg_change)(unsigned char flag);

	/* dolby_letter_box_spinlock */
	void (*dolby_letter_box_spin_lock_irqsave)(unsigned long *flag);
	void (*dolby_letter_box_spin_unlock_irqrestore)(unsigned long *flag);

	/* HDMI_TEST */
	void (*HDMI_TEST)(unsigned int wid, unsigned int len, unsigned char *mdAddr);

	/* Normal_TEST */
	void (*Normal_TEST)(void *p_mdOutput, unsigned int rpcType);

	/* dolby_trigger_timer6 */
	void (*dolby_trigger_timer6)(void);

	/* g_picModeUpdateFlag */
	unsigned int (*get_g_picModeUpdateFlag)(void);
	void (*set_g_picModeUpdateFlag)(unsigned int value);

	/* DV_RingBuffer_Init() */
	int (*dolby_vision_ringbuffer_init)(void *data);

	/* DV_ForceSTOP */
	int (*DV_ForceSTOP)(unsigned long process_id);

	/* Check_DV_Mode() */
	void (*Check_DV_Mode)(void);

	/* DV_HDMI_Init() */
	int (*DV_HDMI_Init)(void);

	/* check_ott_dolby_update_run() */
	unsigned char (*check_ott_dolby_update_run)(void);

	/* dolby_vision_suspend() */
	int (*dolby_vision_suspend)(struct device *p_dev);

	/* dolby_vision_resume() */
	int (*dolby_vision_resume)(struct device *p_dev);

	/* dolby_load_pq_file() */
	int (*dolby_load_pq_file)(char *pq_table_filepath);

	unsigned int (*get_support_status)(void);

	/* DV_Stop(void) */
	int (*dolby_vision_stop)(void);

	/* DV_Run(void) */
	int (*dolby_vision_run)(void);

	/* DV_Pause(void) */
	int (*dolby_vision_pause)(void);

	/* DV_Flush() */
	int (*dolby_vision_flush)(void);

	/* ui_dv_backlight_value */
	int (*get_ui_dv_backlight_value)(void);
	void (*set_ui_dv_backlight_value)(int value);

	/* ui_dv_brightness_value */
	int (*get_ui_dv_brightness_value)(void);
	void (*set_ui_dv_brightness_value)(int value);

	/* ui_dv_color_value */
	int (*get_ui_dv_color_value)(void);
	void (*set_ui_dv_color_value)(int value);

	/* ui_dv_contrast_value */
	int (*get_ui_dv_contrast_value)(void);
	void (*set_ui_dv_contrast_value)(int value);

	/* ui_dv_picmode */
	int (*get_ui_dv_picmode)(void);
	void (*set_ui_dv_picmode)(int mode);

	/* g_dv_pq_sem */
	void (*g_dv_pq_sem_down)(void);
	void (*g_dv_pq_sem_up)(void);

	/* hdmi_ui_change_flag */
	void (*set_hdmi_ui_change_flag)(int value);

	/* rtk_timer_dolbyvision_interrupt */
	irqreturn_t (*rtk_timer_dolbyvision_interrupt)(int irq, void *dev_id);

	/* dolby_load_pq_cfg_file */
	int (*set_pq_table_cfg_path)(char **cfg_path, int num);

	void (*get_pq_table_cfg_path)(char *cfg_path, int length);

	const char *(*get_dolby_vision_software_version)(void);

	void (*set_gd_delay_table)(void *data, size_t size);

	const uint8_t *(*get_gd_delay_table)(size_t *size);

	void (*set_dolby_pwm_ratio)(int value);
	int (*get_dolby_pwm_ratio)(void);
	
	void (*dm_crf_dump_testflow)(void *dolby);

	void (*get_l11_content_type)(uint8_t *l11);

	bool (*is_force_dolby)(void);

	void (*get_dolby_vision_idk_version)(char *buffer, int size);

	void (*get_dolby_vision_dm_version)(char *buffer, int size);

	int (*get_current_gd_delay)(void);

	int (*get_running_mode)(void);

	void (*get_resolution)(int *resolution);

	const uint8_t *(*get_current_target_config)(size_t *size);

	int (*wait_pq_table_cfg_path)(char *cfg_path, int length);

	ssize_t (*pq_table_bin_write)(void *filp, const char *buffer, size_t count, loff_t *ppos, bool is_user_buffer);

	void (*set_ambient_mode)(int mode);

	int (*get_ambient_mode)(void);

	void (*set_ambient_front)(int64_t tFrontLux);

	int64_t (*get_ambient_front)(void);

	void (*set_ambient_rear)(int64_t tRearLum);

	int64_t (*get_ambient_rear)(void);

	void (*set_ambient_whitexy)(int tWhiteX, int tWhiteY);

	void (*get_ambient_whitexy)(int *tWhiteX, int *tWhiteY);

	void (*dolby_init)(void);

	void (*clear_view_mode_id)(void);

	void (*set_block_ambient_call)(bool enable);

	bool (*get_block_ambient_call)(void);

	void (*set_ambient_mode_force)(int mode);

	void (*set_ambient_front_force)(int64_t tFrontLux);

	void (*set_ambient_front_lux)(uint32_t lux);

	uint32_t (*get_ambient_front_lux)(void);
    
    	void (*set_gd_delay_base_hdmi)(int delay);
	int (*get_gd_delay_base_hdmi)(void);

	void (*set_gd_delay_base_ott)(int delay);
	int (*get_gd_delay_base_ott)(void);

	void (*set_gd_delay_base_hdmi_ll)(int delay);
	int (*get_gd_delay_base_hdmi_ll)(void);
	void (*execute_guard_func)(void);

	/* for ui setting could apply at ott pause */
	void (*set_ott_pause)(bool enable);

	bool (*get_ott_pause_status)(void);

	int (*get_ambient_target_mode)(void);

	void (*set_ott_hdmi_backlight_delay_frame)(int ott_delay_frame[],int hdmi_delay_frame[]);

	int (*handle_vsem_packet)(void *data, int count, bool apply_setting);

	void (*set_dolby_vsem_type)(unsigned char port, int type);
	int (*get_dolby_vsem_type)(unsigned char port);

	long (*check_capability)(void *table, size_t size);
	void *(*get_maker_id)(size_t *size);

	long (*set_model_hash)(void *hash, size_t size);

	bool (*md_parser_interrupt)(void);

	void *(*pq_text2bin)(uint8_t *buf_factory, size_t size_factory, uint8_t *buf_bestpq, size_t size_best_pq, size_t *size_pq_bin);

	void (*get_dm_data)(void *p_comp_reg, void *p_dm_reg, void *p_dm_lut, bool is_user_buffer);
	void (*get_dm_struct_size)(int *size_composer, int *size_dm_register, int *size_dm_lut);
	void (*clear_dump_dm_register_counter)(void);

	void (*worker)(void);

	void (*wait_next_frame)(void);
	void (*wait_dolby_vision_start)(void);

	void (*force_run_ott)(void);
	void (*set_update_crf_ambient_flag)(bool enable);
	void (*set_update_crf_dark_detail_flag)(bool enable);
	void (*set_update_crf_dtv_flag)(bool enable);
	void (*set_update_crf_best_pq_flag)(bool enable);

	void (*set_dark_detail)(int32_t dark_detail);

	void (*release_hdmi_backlight_delay_queue)(void);
};

/* dolby_proverlay_background_h_start_end */
unsigned int dolby_adapter_get_dolby_proverlay_background_h_start_end(void);
/* dolby_proverlay_background_v_start_end */
unsigned int dolby_adapter_get_dolby_proverlay_background_v_start_end(void);

/* letter_box_black_flag */
unsigned char dolby_adapter_get_letter_box_black_flag(void);
void dolby_adapter_set_letter_box_black_flag(unsigned char flag);

/* letter_box_black_flag */
unsigned char dolby_adapter_get_request_letter_dtg_change(void);
void dolby_adapter_set_request_letter_dtg_change(unsigned char flag);

/* dolby_letter_box_spinlock */
void dolby_adapter_dolby_letter_box_spin_lock_irq_save(unsigned long *flags);
void dolby_adapter_dolby_letter_box_spin_unlock_irqrestore(unsigned long *flags);

/* HDMI_TEST() */
void dolby_adapter_HDMI_TEST(unsigned int wid, unsigned int len, unsigned char *mdAddr);

/* Normal_TEST() */
void dolby_adapter_Normal_TEST(void *p_mdOutput, unsigned int rpcType);

void dolby_adapter_force_run_ott(void);

/* dolby_trigger_timer6 */
void dolby_adapter_dolby_trigger_timer6(void);

/* g_picModeUpdateFlag */
unsigned int dolby_adapter_get_g_picModeUpdateFlag(void);
void dolby_adapter_set_g_picModeUpdateFlag(unsigned int);

/* DV_ForceSTOP() */
int dolby_adapter_DV_ForceSTOP(unsigned long process_id);

/* Check_DV_Mode() */
void dolby_adapter_Check_DV_Mode(void);

/* DV_HDMI_Init() */
int dolby_adapter_DV_HDMI_Init(void);

/* check_ott_dolby_update_run() */
unsigned char dolby_adapter_check_ott_dolby_update_run(void);

/* dolby_vision_suspend() */
int dolby_adapter_dolby_vision_suspend(struct device *p_dev);

/* dolby_vision_resume() */
int dolby_adapter_dolby_vision_resume(struct device *p_dev);

int dolby_adapter_get_support_status(void);

/* dolby_load_pq_file() */
int dolby_adapter_dolby_load_pq_file(char *pq_table_filepath);

/* DV_RingBuffer_Init() */
int dolby_adapter_dolby_vision_ringbuffer_init(void *data);

/* DV_Stop(void) */
int dolby_adapter_dolby_vision_stop(void);

/* DV_Run(void) */
int dolby_adapter_dolby_vision_run(void);

/* DV_Pause(void) */
int dolby_adapter_dolby_vision_pause(void);

/* DV_Flush() */
int dolby_adapter_dolby_vision_flush(void);

/* ui_dv_backlight_value */
int dolby_adapter_get_ui_dv_backlight_value(void);
void dolby_adapter_set_ui_dv_backlight_value(int value);

/* ui_dv_brightntess_value */
int dolby_adapter_get_ui_dv_brightness_value(void);
void dolby_adapter_set_ui_dv_brightness_value(int value);

/* ui_dv_color_value */
int dolby_adapter_get_ui_dv_color_value(void);
void dolby_adapter_set_ui_dv_color_value(int value);

/* ui_dv_contrast_value */
int dolby_adapter_get_ui_dv_contrast_value(void);
void dolby_adapter_set_ui_dv_contrast_value(int value);

/* ui_dv_picmode */
int dolby_adapter_get_ui_dv_picmode(void);
void dolby_adapter_set_ui_dv_picmode(int mode);

/* g_dv_pq_sem */
void dolby_adapter_g_dv_pq_sem_down(void);
void dolby_adapter_g_dv_pq_sem_up(void);

/* hdmi_ui_change_flag */
void dolby_adapter_set_hdmi_ui_change_flag(int value);

/* rtk_timer_dolbyvision_interrupt */
irqreturn_t dolby_adapter_rtk_timer_dolbyvision_interrupt(int irq, void *dev_id);

/* dolby_load_pq_cfg_file */
int dolby_adapter_set_pq_table_cfg_path(char **cfg_path, int num);

void dolby_adapter_get_pq_table_cfg_path(char *cfg_path, int length);

const char *dolby_adapter_get_dolby_vision_software_version(void);

void dolby_adapter_set_gd_delay_table(void *data, size_t size);

const uint8_t *dolby_adapter_get_gd_delay_table(size_t *size);

void dolby_adapter_set_dolby_pwm_ratio(int value);

void dolby_adapter_dm_crf_dump_testflow(void *dolby);

void dolby_adapter_get_l11_content_type(uint8_t *l11);

bool dolby_adapter_is_force_dolby(void);

void dolby_adapter_get_dolby_vision_idk_version(char *buffer, int size);

void dolby_adapter_get_dolby_vision_dm_version(char *buffer, int size);

int dolby_adapter_get_current_gd_delay(void);

const uint8_t *dolby_adapter_get_current_target_config(size_t *size);

int dolby_adapter_get_running_mode(void);

void dolby_adapter_get_resolution(int *resolution);

int dolby_adapter_wait_pq_table_cfg_path(char *cfg_path, int length);

ssize_t dolby_adapter_pq_table_bin_write(void *filp, const char __user *buffer, size_t count, loff_t *ppos, bool is_user_buffer);

void dolby_adapter_set_ambient_mode(int mode);

int dolby_adapter_get_ambient_mode(void);

int dolby_adapter_get_ambient_target_mode(void);

void dolby_adapter_set_ambient_front(int64_t tFrontLux);

int64_t dolby_adapter_get_ambient_front(void);

void dolby_adapter_set_ambient_rear(int64_t tRearLum);

int64_t dolby_adapter_get_ambient_rear(void);

void dolby_adapter_set_ambient_whitexy(int tWhiteX, int tWhiteY);

void dolby_adapter_get_ambient_whitexy(int *tWhiteX, int *tWhiteY);

void dolby_adapter_dolby_init(void);

void dolby_adapter_clear_view_mode_id(void);

void dolby_adapter_set_block_ambient_call(bool enable);

bool dolby_adapter_get_block_ambient_call(void);

void dolby_adapter_set_ambient_front_force(int64_t tFrontLux);

void dolby_adapter_set_ambient_mode_force(int mode);

void dolby_adapter_set_ambient_front_lux(uint32_t lux);

uint32_t dolby_adapter_get_ambient_front_lux(void);
void dolby_adapter_set_gd_delay_base_hdmi(int delay);
int dolby_adapter_get_gd_delay_base_hdmi(void);

void dolby_adapter_set_gd_delay_base_ott(int delay);
int dolby_adapter_get_gd_delay_base_ott(void);

void dolby_adapter_set_gd_delay_base_hdmi_ll(int delay);
int dolby_adapter_get_gd_delay_base_hdmi_ll(void);
void dolby_adapter_execute_guard_func(void);

void dolby_adapater_set_ott_pause(bool enable);
bool dolby_adapater_get_ott_pause_status(void);

void dolby_adapater_set_ott_hdmi_backlight_delay_frame(int ott_delay_frame[], int hdmi_delay_frame[]);

int dolby_adapter_handle_vsem_packet(void *data, int count, bool apply_setting);

void dolby_adapter_set_dolby_vsem_type(unsigned char port, int type);
int dolby_adapter_get_dolby_vsem_type(unsigned char port);

long dolby_adapter_check_capability(void *table, size_t size);
long dolby_adapter_set_model_hash(void *model_hash, size_t size);

void dolby_adapter_wait_next_frame(void);
#ifdef CONFIG_RTK_KDRV_DV_IDK_DUMP
void dolby_adapter_wait_dolby_vision_start(void);
#endif

irqreturn_t dolby_adapter_md_parser_interrupt(int irq, void *dev_id);
irqreturn_t dolby_adapter_rtk_timer_dolbyvision_interrupt(int irq, void *dev_id);

long register_dolby_vision_isr(void *device);
long release_dolby_vision_isr(void *device);

void *dolby_adapter_pq_text2bin(uint8_t *buf_factory, size_t size_factory, uint8_t *buf_bestpq, size_t best_pq, size_t *size_pq_bin);

void dolby_adapter_worker(void);

void dolby_adapter_get_dm_data(void *p_comp_reg, void *p_dm_reg, void *p_dm_lut, bool is_user_buffer);
void dolby_adapter_get_dm_struct_size(int *size_composer, int *size_dm_register, int *size_dm_lut);
void dolby_adapter_clear_dump_dm_register_counter(void);

void dolby_adapter_set_update_crf_ambient_flag(bool enable);
void dolby_adapter_set_update_crf_dark_detail_flag(bool enable);
void dolby_adapter_set_update_crf_dtv_flag(bool enable);
void dolby_adapter_set_update_crf_best_pq_flag(bool enable);

void dolby_adapter_set_dark_detail(int32_t dark_detail);

bool set_dolby_adapter(struct dolby_interface *interface, size_t size);

void dolby_adapter_release_hdmi_backlight_delay_queue(void);
#endif	/* __DOLBY_IDK_1_6_1_1_DOLBY_ADAPTER_H__ */
