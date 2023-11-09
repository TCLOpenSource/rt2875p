#ifndef	__VIP_GSR_ADAPTER_H__
#define	__VIP_GSR_ADAPTER_H__

#ifndef BUILD_QUICK_SHOW
#include <linux/interrupt.h>
#endif

struct VIP_GSR_interface {
	unsigned short (*get_LG_GSR_GetAdaptiveRgbGain)(unsigned int * pstParams, unsigned short pApl[34][60], unsigned short maxGain);
	int *pLG_GSR_Status;
};

unsigned short fwif_color_get_LG_GSR_GetAdaptiveRgbGain(unsigned int * pstParams, unsigned short pApl[34][60], unsigned short maxGain);
int fwif_color_get_LG_GSR_Status(void);
unsigned int fwif_color_get_LG_GSR_StressCount(void);
#endif

#if 0	// sample code
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
	void (*HDMI_TEST)(unsigned int wid, unsigned int len, unsigned int mdAddr);

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

	ssize_t (*pq_table_bin_write)(void *filp, const char __user *buffer, size_t count, loff_t *ppos);

	void (*set_ambient_mode)(int mode);

	int (*get_ambient_mode)(void);

	void (*set_ambient_front)(int64_t tFrontLux);

	int64_t (*get_ambient_front)(void);

	void (*set_ambient_rear)(int64_t tRearLum);

	int64_t (*get_ambient_rear)(void);

	void (*set_ambient_whitexy)(int tWhiteX, int tWhiteY);

	void (*get_ambient_whitexy)(int *tWhiteX, int *tWhiteY);
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
void dolby_adapter_HDMI_TEST(unsigned int wid, unsigned int len, unsigned int mdAddr);

/* Normal_TEST() */
void dolby_adapter_Normal_TEST(void *p_mdOutput, unsigned int rpcType);

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

ssize_t dolby_adapter_pq_table_bin_write(void *filp, const char __user *buffer, size_t count, loff_t *ppos);

void dolby_adapter_set_ambient_mode(int mode);

int dolby_adapter_get_ambient_mode(void);

void dolby_adapter_set_ambient_front(int64_t tFrontLux);

int64_t dolby_adapter_get_ambient_front(void);

void dolby_adapter_set_ambient_rear(int64_t tRearLum);

int64_t dolby_adapter_get_ambient_rear(void);

void dolby_adapter_set_ambient_whitexy(int tWhiteX, int tWhiteY);

void dolby_adapter_get_ambient_whitexy(int *tWhiteX, int *tWhiteY);

void set_dolby_adapter(struct dolby_interface *interface);

#endif	/* __DOLBY_IDK_1_6_1_1_DOLBY_ADAPTER_H__ */
#endif

