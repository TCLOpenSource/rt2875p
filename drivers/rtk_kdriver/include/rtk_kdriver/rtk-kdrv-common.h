#ifndef _RTK_KDRV_COMMON_H_
#define _RTK_KDRV_COMMON_H_

/*common.c*/
bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
bool rtk_parse_commandline(const char *string);
//void enable_uart2rbus(unsigned int value);

/*init.c*/
#ifdef CONFIG_RTK_KDRV_COMMON_MODULE
void parse_envp(void);
void parse_reclaim(void);
void parse_last_image(void);
void parse_debugMode(void);
#endif

/*platform.c*/
int rtk_init_machine(void);
enum PLAFTORM_TYPE get_platform (void);
enum PLATFORM_MODEL get_platform_model (void);
unsigned int get_ic_version(void);
void rtk_init_early(void);
void rtk_init_late(void);
#ifdef CONFIG_RTK_KDRV_COMMON_MODULE
void parse_product(void);
void parse_platform_model(void);
void parse_platform(void);
void parse_disp_res(void);
void parse_ui_res(void);
void parse_mc_dram_num(void);
#endif

#ifdef CONFIG_RTK_LOW_POWER_MODE
#define PM_LPM_POWER_MODE_S0	0x0000
#define PM_LPM_POWER_MODE_S1	0x0001
#define PM_LPM_POWER_MODE_S2	0x0002
#define PM_LPM_POWER_MODE_UNKNOW	0x0003
extern int register_lpm_notifier(struct notifier_block *nb);
extern int unregister_lpm_notifier(struct notifier_block *nb);
extern struct list_head rtk_lpm_dev_list;
extern int lpm_notifier_call_chain(unsigned long val);
struct rtk_lpm_dev_info
{
	struct list_head list_head;
	struct device *dev;
};
extern void rtk_lpm_add_device(struct device *dev);

#endif


#endif //_RTK_KDRV_COMMON_H_

