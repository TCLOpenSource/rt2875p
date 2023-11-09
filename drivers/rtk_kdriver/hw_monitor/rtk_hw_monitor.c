#include <linux/module.h>
#include <rtk_kdriver/io.h>
#include "rtk_hw_monitor.h"
#include "arm_wrapper.h"
#include "rtk_dc_mt.h"
#include "rtk_dc_mt_config.h"
#include "rtk_sb2.h"
#include <linux/slab.h>
#include <asm/signal.h>
#include <rbus/mc_secure_reg.h>
#ifdef CONFIG_ARM64
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include <asm/system_misc.h>
#define hook_fault_code hook_debug_fault_code
#endif

#include <linux/version.h>
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 13, 0))
#include <linux/panic_notifier.h>
#endif

extern void avk_auto_range_sync(void);
static void reset_arm_wrapper_all(void);
extern void dump_avk_auto_range(void);
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
extern int kernel_trap_warning;
extern int __init dc_mt_early_init(void);
extern int __init dc_mt_irq_init(void);
extern int dcmt_set_monitor_auto(DC_MT_DESC * pDesc);
extern int dump_dcmt_last_set_only(void);
extern void dcmt_unset_avk_intr_routing(void);
extern void dcmt_set_all_cpu_intr_routing(void);
extern int parse_module_name(char * name,unsigned char * pModuleID,int max_num);
extern void dcmt_clear_monitor_all(void);
extern int dc_mt_monitor_init(void);
extern int get_dcmt_modues(char * buff);
extern int get_module_keys(char * buff);
extern char* DCMT_module_str(unsigned char id);
#else
static inline char* DCMT_module_str(unsigned char id) { return 0; }
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
extern int sb2_set_id;
extern int sb2_dbg_kernel_trap_warning;
extern int sb2_dbg_monitor (sb2_dbg_param *pParam);
extern int sb2_show_last_set_only(void);
extern void rtk_sb2_monitor_init(void);
extern int sb2_dbg_clear(sb2_dbg_param *pParam);
#endif

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
extern int arm_wrapper_set_id;
extern int arm_wrapper_kernel_trap_warning;
extern int set_arm_wrapper_arange (unsigned int start,unsigned int end,int set_id,WRAPPER_RW rw);
extern void arm_wrapper_show_last_set_only(void);
extern int clear_arm_wrapper_arange (int set_id);
extern void arm_wrapper_monitor_init(void);
#endif


int rtk_mem_prison_ipi_handle_init(void);

#define HWM_ENV_RANGE_DFT      "hwm_range="
int is_hwm_early_inited = 0;
int need_monitor_range_dft = 1;
__maybe_unused int is_in_dcmt_isr = 0;
EXPORT_SYMBOL(is_in_dcmt_isr);

/******************************HW EARLY INIT***********************************/
void __init hw_monitor_early_init(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        dc_mt_early_init();
#endif

        is_hwm_early_inited = 1;
        return;
}

/******************************AVK AUTO RANGE SYNC***********************************/
#define ACPU_AUTOSYNC_RANGE_START_reg           DC_SYS_DC_MT_SADDR_0_reg
#define ACPU_AUTOSYNC_RANGE_END_reg             DC_SYS_DC_MT_EADDR_0_reg
#define VCPU_AUTOSYNC_RANGE_START_reg           DC_SYS_DC_MT_SADDR_1_reg
#define VCPU_AUTOSYNC_RANGE_END_reg             DC_SYS_DC_MT_EADDR_1_reg
#define VCPU2_AUTOSYNC_RANGE_START_reg          DC_SYS_DC_MT_SADDR_2_reg
#define VCPU2_AUTOSYNC_RANGE_END_reg            DC_SYS_DC_MT_EADDR_2_reg

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU3
#include <rbus/HEVC_ENC/hevc_enc_misc_reg.h>
#define VCPU3_AUTOSYNC_RANGE_START_reg          HEVC_ENC_MISC_DBG_START_0_reg 
#define VCPU3_AUTOSYNC_RANGE_END_reg            HEVC_ENC_MISC_DBG_END_0_reg
#endif

#define KCPU_AUTOSYNC_RANGE_START_reg           DC_SYS_DC_MTEX_SADDR_1_reg
#define KCPU_AUTOSYNC_RANGE_END_reg             DC_SYS_DC_MTEX_EADDR_1_reg
#define SECURE_AUTOSYNC_RANGE_START_reg         DC_SYS_DC_MT_SADDR_3_reg
#define SECURE_AUTOSYNC_RANGE_END_reg           DC_SYS_DC_MT_EADDR_3_reg

unsigned int avk_range[RANGE_SYNC_NUM];//avk auto sync range for hw-monitor

void dump_avk_auto_range(void)
{
        HWM_WARNING("get acpu auto sync range 0x%08x - 0x%08x \n",avk_range[RANGE_SYNC_A_START],avk_range[RANGE_SYNC_A_END]);
        HWM_WARNING("get vcpu auto sync range 0x%08x - 0x%08x \n",avk_range[RANGE_SYNC_V_START],avk_range[RANGE_SYNC_V_END]);
        HWM_WARNING("get kcpu auto sync range 0x%08x - 0x%08x \n",avk_range[RANGE_SYNC_K_START],avk_range[RANGE_SYNC_K_END]);
        HWM_WARNING("get secure auto sync range 0x%08x - 0x%08x \n",avk_range[RANGE_SYNC_SECURE_START],avk_range[RANGE_SYNC_SECURE_END]);
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	HWM_WARNING("get vcpu2 auto sync range 0x%08x - 0x%08x \n",avk_range[RANGE_SYNC_V2_START],avk_range[RANGE_SYNC_V2_END]); 
#endif
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU3
	HWM_WARNING("get vcpu3 auto sync range 0x%08x - 0x%08x \n",avk_range[RANGE_SYNC_V3_START],avk_range[RANGE_SYNC_V3_END]); 
#endif

        return;
}
 

void  hwm_get_acpu_range(unsigned int *start,  unsigned int * size)
{
    *start = *size = 0;
    *start = avk_range[RANGE_SYNC_A_START];
    *size  = avk_range[RANGE_SYNC_A_END] - avk_range[RANGE_SYNC_A_START];
}

EXPORT_SYMBOL(hwm_get_acpu_range);

void hwm_get_vcpu2_range(unsigned int *start,  unsigned int * size)
{
    *start = *size = 0;
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
    *start = avk_range[RANGE_SYNC_V2_START];
    *size  = avk_range[RANGE_SYNC_V2_END] - avk_range[RANGE_SYNC_V2_START];
#endif
}

EXPORT_SYMBOL(hwm_get_vcpu2_range);
void hwm_get_vcpu3_range(unsigned int *start,  unsigned int * size)
{
    *start = *size = 0;
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU3
    *start = avk_range[RANGE_SYNC_V3_START];
    *size  = avk_range[RANGE_SYNC_V3_END] - avk_range[RANGE_SYNC_V3_START];
#endif
}
EXPORT_SYMBOL(hwm_get_vcpu3_range);

void hwm_get_vcpu_range(unsigned int *start,  unsigned int * size)
{
    *start = *size = 0;
    *start = avk_range[RANGE_SYNC_V_START];
    *size  = avk_range[RANGE_SYNC_V_END] - avk_range[RANGE_SYNC_V_START];
}

EXPORT_SYMBOL(hwm_get_vcpu_range);


void avk_auto_range_sync(void)
{
        //avk text range auto sync
        memset(avk_range,0,sizeof(avk_range));

        avk_range[RANGE_SYNC_A_START] = rtd_inl(ACPU_AUTOSYNC_RANGE_START_reg);
        avk_range[RANGE_SYNC_A_END] = rtd_inl(ACPU_AUTOSYNC_RANGE_END_reg);

        avk_range[RANGE_SYNC_V_START] = rtd_inl(VCPU_AUTOSYNC_RANGE_START_reg);
        avk_range[RANGE_SYNC_V_END] = rtd_inl(VCPU_AUTOSYNC_RANGE_END_reg);
#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU2
	avk_range[RANGE_SYNC_V2_START] = rtd_inl(VCPU2_AUTOSYNC_RANGE_START_reg);
	avk_range[RANGE_SYNC_V2_END] = rtd_inl(VCPU2_AUTOSYNC_RANGE_END_reg);
#endif

#ifdef CONFIG_RTK_KDRV_SUPPORT_VCPU3
#ifndef CONFIG_REALTEK_RESCUE_LINUX
	avk_range[RANGE_SYNC_V3_START] = rtd_inl(VCPU3_AUTOSYNC_RANGE_START_reg);
	avk_range[RANGE_SYNC_V3_END] = rtd_inl(VCPU3_AUTOSYNC_RANGE_END_reg);
#endif
#endif
        avk_range[RANGE_SYNC_K_START] = rtd_inl(KCPU_AUTOSYNC_RANGE_START_reg);
        avk_range[RANGE_SYNC_K_END] = rtd_inl(KCPU_AUTOSYNC_RANGE_END_reg);

        avk_range[RANGE_SYNC_SECURE_START] = rtd_inl(SECURE_AUTOSYNC_RANGE_START_reg);
        avk_range[RANGE_SYNC_SECURE_END] = rtd_inl(SECURE_AUTOSYNC_RANGE_END_reg);
        #if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        arm_wrapper_monitor_init();
        #endif
        return;
}
EXPORT_SYMBOL(avk_auto_range_sync);


/**************************HW MONITOR UNIVERSAL API*******************************/
__maybe_unused int __find_char_pos(char * str,char aim)
{
        char * end = str+strlen(str);
        int cnt = 0;

        while(str < end)
        {
                if((*str) == aim)
                {
                        return cnt;
                }
                str++;
                cnt++;
        }

        return -1;
}

#define isnumber(c)      (c>=0x30 && c<=0x39)
#define isalphabet(c)    (c>=0x41 && c<=0x5A) || (c>=0x61 && c<=0x7A)
#define isalnum(c)       (isnumber(c) || isalphabet(c))
__maybe_unused char* __find_next_str(char* str)
{
        char* end;
        if(str == NULL)
                return NULL;

        end = str + strlen(str);

        while(str < end)
        {
                if (isalnum(*str)) // is alphabet or number
                        return str;
                str++;
        }

        return NULL;
}

__maybe_unused int get_arguments(char* str, int argc, char** argv)
{
        int ret = 0;
        char* sep;

        while ((str = __find_next_str(str)) && ret < argc)
        {
                argv[ret++] = str;
                //DC_MT_INFO("arg[%d]='%s'\n", ret, str);
                sep = strchr(str, ','); // devided by

                if (sep == NULL)
                        break;

                *sep = '\0'; // changed to null terminated
                //DC_MT_INFO("**** arg[%d]='%s'\n", ret, str);
                str = sep + 1;

        }

        return ret;
}

__maybe_unused int get_space_arguments(char* str, int argc, char** argv)
{
        int ret = 0;
        char* sep;

        while ((str = __find_next_str(str)) && ret < argc)
        {
                argv[ret++] = str;
                //DC_MT_INFO("arg[%d]='%s'\n", ret, str);
                sep = strchr(str, ' '); // devided by

                if (sep == NULL)
                        break;

                *sep = '\0'; // changed to null terminated
                //DC_MT_INFO("**** arg[%d]='%s'\n", ret, str);
                str = sep + 1;

        }

        return ret;
}

__maybe_unused void __replace_chr(char* str, char a, char b)
{
        char* end = str + strlen(str);
        while(str < end)
        {
                if (*str == a)
                        *str = b;
                str++;
        }
}

//basic API
static __maybe_unused void dump_hwm_param(hwm_t * pParam)
{
        int i;
        HWM_ERR("\ndump hwm param:\n");
        HWM_ERR("range:%x - %x\n", pParam->start, pParam->end);
        HWM_ERR("mode:%d\n", pParam->mode);
        HWM_ERR("keep go:%d\n", pParam->keep_go);
        for(i = 0;i < 8;++i)
        {
                HWM_ERR("ModuleID[%d]   :%x\n", i, pParam->ip[i]);
                HWM_ERR("Premission[%d] :%d\n", i, pParam->ip_permission[i]);
        }
}

static int is_hwm_desc_invalid(hwm_t *pParam)
{
        int ret = 0;
        int i;
        if(pParam == NULL)
        {
                HWM_ERR("[%s] pParam is NULL!\n", __FUNCTION__);
                return -1;
        }

        if(pParam->start > pParam->end)
        {
                HWM_ERR("[%s] start(0x%08x) > end(0x%08x)!\n", __FUNCTION__, pParam->start, pParam->end);
                ret = -1;
        }

        if(pParam->mode >= HWM_MODE_INVALID)
        {
                HWM_ERR("[%s] mode:%d is invalid!\n", __FUNCTION__, pParam->mode);
                ret = -1;
        }

        for(i = 0; i < 8; ++i)
        {
                if((pParam->ip[i] > 0xff) && (pParam->ip[i] != VIRT_MODULE_SCPU) && (pParam->ip[i] != VIRT_MODULE_NULL))
                {
                        HWM_ERR("[%s] ip[%d]:0x%08x is invalid!\n", __FUNCTION__, i, pParam->ip[i]);
                        ret = -1;
                }

                if(pParam->ip_permission[i] > HWM_PERMISION_INVALID)
                {
                        HWM_ERR("[%s] ip_permission[%d]:0x%d is invalid!\n", __FUNCTION__, i, pParam->ip_permission[i]);
                        ret = -1;
                }
        }

        return ret;
}

static int check_rbus_range(unsigned int start,unsigned int end)
{
        //RBUS_BASE_PHYS + RBUS_BASE_SIZE
        if((start >= (RBUS_BASE_PHYS + RBUS_BASE_SIZE)) || (end < RBUS_BASE_PHYS))
        {
                HWM_DBG("[%s] no rbus range including!\n", __FUNCTION__);
                return -1;
        }

        if((start < RBUS_BASE_PHYS) || (end > (RBUS_BASE_PHYS + RBUS_BASE_SIZE)))
        {
                HWM_ERR("[%s] rbus range check fail! param:(0x%08x - 0x%08x), rbus:(0x%08x - 0x%08x)\n",
                        __FUNCTION__, start, end, RBUS_BASE_PHYS, (RBUS_BASE_PHYS + RBUS_BASE_SIZE));
                return -1;
        }

        return 0;
}

/*
name : convert_sb2_desc
warning : need check pParam by the caller
*/
static int convert_sb2_desc(hwm_t *pParam, sb2_dbg_param * pSb2Desc)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
        //sb2_dbg_param sb2_desc[3]; //index --- HWM_R:0 HWM_W:1 HWM_RW:2
        int cpu_flag[7];
        int permission_flag[7];
        int flag_index;
        int i,j;
        unsigned int last_set_id;

        memset(pSb2Desc, 0, (sizeof(sb2_dbg_param)*3));
        for(i = 0;i < 3;++i)
        {
                pSb2Desc[i].set_id = -2; // init set_id as a invalid id
        }

        if(HWM_MODE_3 == pParam->mode)
        {
                HWM_ERR("[%s] SB2 MONITOR cannot support mode3!\n", __FUNCTION__);
                return -1;
        }

        for(i = 0;i < 3;++i)
        {
                //start - end
                pSb2Desc[i].start = pParam->start;
                pSb2Desc[i].end = pParam->end;
                pSb2Desc[i].operation = OPERA_MONITOR;
                pSb2Desc[i].bit_mask = 0;//no bit monitor
                pSb2Desc[i].str = STR_FLAG_YES;//support str
                pSb2Desc[i].cpu_flag[CPU_TYPE_ALL] = 0;
        }
        pSb2Desc[HWM_RW].rw = RW_FLAG_RW;
        pSb2Desc[HWM_R].rw = RW_FLAG_RO;
        pSb2Desc[HWM_W].rw = RW_FLAG_WO;

        //convert ip & permission to mode1
        for(i = 0;i < 7;++i)//init collection
        {
                cpu_flag[i] = 0;
                permission_flag[i] = HWM_PERMISION_INVALID;
        }

        for(i = 0;i < 8;++i)//collect cpu & permission
        {
                flag_index = -1;
                if(VIRT_MODULE_SCPU == pParam->ip[i])
                {
                        flag_index = CPU_TYPE_SCPU;
                }
                else if(MODULE_SB2_VCPU == pParam->ip[i])
                {
                        flag_index = CPU_TYPE_VCPU;
                }
#ifdef MODULE_SB2_VCPU2
                else if(MODULE_SB2_VCPU2 == pParam->ip[i])
                {
                        flag_index = CPU_TYPE_VCPU2;
                }
#endif
                else if(MODULE_ANY == pParam->ip[i])
                {
                        flag_index = CPU_TYPE_ALL;
                }

                if(flag_index >= 0)
                {
                        cpu_flag[flag_index] = 1;
                        if(permission_flag[flag_index] >= HWM_PERMISION_INVALID)
                        {
                                permission_flag[flag_index] = pParam->ip_permission[i];
                        }
                        else if(permission_flag[flag_index] != pParam->ip_permission[i]) //merge 2 diff permission as RW
                        {
                                permission_flag[flag_index] = HWM_RW;
                        }
                }
        }

        //handle CPU_TYPE_ALL
        if((1 == cpu_flag[CPU_TYPE_ALL]) && (permission_flag[CPU_TYPE_ALL] < HWM_PERMISION_INVALID))
        {
                for(i = 1;i < 7;++i)
                {
                        if(permission_flag[i] >= HWM_PERMISION_INVALID)
                        {
                                permission_flag[i] = permission_flag[CPU_TYPE_ALL];
                        }
                        else if(permission_flag[i] != permission_flag[CPU_TYPE_ALL]) //merge 2 diff permission as RW
                        {
                                permission_flag[i] = HWM_RW;
                        }
                }
                cpu_flag[CPU_TYPE_ALL] = 0;
                permission_flag[CPU_TYPE_ALL] = HWM_PERMISION_INVALID;
        }

        //-convert from mode2 to mode1
        if(HWM_MODE_2 == pParam->mode)
        {
                for(i = 1;i < 7;++i) //skip CPU_TYPE_ALL
                {
                        if((cpu_flag[i])&&(HWM_RW == permission_flag[i]))
                        {
                                cpu_flag[i] = 0;
                        }
                        else if((cpu_flag[i])&&(HWM_R == permission_flag[i]))
                        {
                                permission_flag[i] = HWM_W;
                        }
                        else if((cpu_flag[i])&&(HWM_W == permission_flag[i]))
                        {
                                permission_flag[i] = HWM_R;
                        }
                        else if(0 == cpu_flag[i])
                        {
                                cpu_flag[i] = 1;
                                permission_flag[i] = HWM_RW;
                        }
                }
        }

        //ip & permission
        for(i = 0;i < 7;++i)
        {
                if(cpu_flag[i] == 1)
                {
                        pSb2Desc[permission_flag[i]].cpu_flag[i] = 1;
                }
        }

        //set_id : get free set or force take one
        for(i = 0;i < 3;++i)
        {
                for(j = 0;j < 7;++j)
                {
                        if(pSb2Desc[i].cpu_flag[j] == 1)
                        {
                                pSb2Desc[i].set_id = -1;
                                break;
                        }
                }
        }

        last_set_id = sb2_set_id;
        for(i = 0;i < 3;++i)
        {
                if(-1 == pSb2Desc[i].set_id)
                {
                        pSb2Desc[i].set_id = last_set_id;
                        last_set_id = (last_set_id % 16) + 1;
                }
        }

        return 0;
#else //ifndef CONFIG_RTK_KDRV_SB2
        HWM_ERR("[%s]" SB2_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_RTK_KDRV_SB2
}

static int apply_sb2_desc(sb2_dbg_param * pSb2Desc)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
        return sb2_dbg_monitor(pSb2Desc);
#else //ifndef CONFIG_RTK_KDRV_SB2
        HWM_ERR("[%s]" SB2_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_RTK_KDRV_SB2
}

static void show_sb2_last_set(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
        sb2_show_last_set_only();
#else //ifndef CONFIG_RTK_KDRV_SB2
        HWM_ERR("[%s]" SB2_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_RTK_KDRV_SB2
}

static void clear_sb2_all(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
        sb2_dbg_param param;
        int i;

        for(i = 1;i <= 16;++i)
        {
                memset(&param,0,sizeof(param));
                param.operation = OPERA_CLEAR;
                param.set_id = i;
                param.str = 1;
                sb2_dbg_clear(&param);
        }

#else //ifndef CONFIG_RTK_KDRV_SB2
        HWM_ERR("[%s]" SB2_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_RTK_KDRV_SB2
}

static void reset_sb2_all(void) //rtk_sb2_monitor_init
{
        clear_sb2_all();
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
        rtk_sb2_monitor_init();
#else //ifndef CONFIG_RTK_KDRV_SB2
        HWM_ERR("[%s]" SB2_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_RTK_KDRV_SB2
}


/*
name : is_include_scpu_monitor
desc : check if param include scpu monitor
ret : 1:include scpu monitor, 0:not include scpu monitor
*/
static int is_include_scpu_monitor(hwm_t *pParam)
{
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        int i;
        if(HWM_MODE_3 == pParam->mode)
        {
                return 0; // arm_wrapper cannot support mode3
        }

        for(i = 0; i < 8; ++i)
        {
                if( (VIRT_MODULE_SCPU == pParam->ip[i]) || (MODULE_ANY == pParam->ip[i]) )
                {
                        break;
                }
        }

        if(i < 8) //include scpu
        {
                if(HWM_MODE_1 == pParam->mode)
                {
                        return 1;
                }
                else if(HWM_MODE_2 == pParam->mode)
                {
                        if(HWM_RW == pParam->ip_permission[i])
                        {
                                return 0;
                        }
                        else
                        {
                                return 1;
                        }
                }
        }
        else //not include scpu
        {
                if(HWM_MODE_1 == pParam->mode)
                {
                        return 0;
                }
                else if(HWM_MODE_2 == pParam->mode)
                {
                        return 1;
                }
        }

        return 0;
#else //ifndef CONFIG_REALTEK_ARM_WRAPPER
        HWM_ERR("[%s]" ARM_WRAPPER_DISABLE_TIP "\n", __FUNCTION__);
        return 0;
#endif //endif CONFIG_REALTEK_ARM_WRAPPER
}

static int convert_arm_wrapper_desc(hwm_t *pParam, arm_wrapper_t *pArmDesc)
{
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        int i;
        unsigned char collect_permission = HWM_PERMISION_INVALID;
        if(HWM_MODE_3 == pParam->mode)
        {
                return -1;
        }

        pArmDesc->start = pParam->start;
        pArmDesc->end = pParam->end;
        for(i = 0; i < 8; ++i)
        {
                if( (VIRT_MODULE_SCPU == pParam->ip[i]) || (MODULE_ANY == pParam->ip[i]) )
                {
                        if(HWM_PERMISION_INVALID == collect_permission)
                        {
                                collect_permission = pParam->ip_permission[i];
                        }
                        else if(collect_permission != pParam->ip_permission[i])
                        {
                                collect_permission = HWM_RW;
                        }
                }
        }

        if(collect_permission != HWM_PERMISION_INVALID) //include scpu
        {
                if(HWM_MODE_1 == pParam->mode)
                {
                        pArmDesc->permission = collect_permission;
                }
                else if(HWM_MODE_2 == pParam->mode)
                {
                        if(HWM_RW == collect_permission)
                        {
                                return -1;
                        }
                        else if(HWM_R == collect_permission)
                        {
                                pArmDesc->permission = HWM_W;
                        }
                        else if(HWM_W == collect_permission)
                        {
                                pArmDesc->permission = HWM_R;
                        }
                }
        }
        else //not include scpu
        {
                if(HWM_MODE_1 == pParam->mode)
                {
                        return -1;
                }
                else if(HWM_MODE_2 == pParam->mode)
                {
                        pArmDesc->permission = HWM_RW;
                }
        }

        pArmDesc->id = arm_wrapper_set_id; //get next set id

        return 0;
#else //ifndef CONFIG_REALTEK_ARM_WRAPPER
        HWM_ERR("[%s]" ARM_WRAPPER_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_REALTEK_ARM_WRAPPER

}

int apply_arm_wrapper_desc(arm_wrapper_t *pArmDesc)
{
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        return set_arm_wrapper_arange(pArmDesc->start, pArmDesc->end, pArmDesc->id, pArmDesc->permission);
#else //ifndef CONFIG_REALTEK_ARM_WRAPPER
        HWM_ERR("[%s]" ARM_WRAPPER_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_REALTEK_ARM_WRAPPER

}

static void show_arm_wrapper_last_set(void)
{
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        arm_wrapper_show_last_set_only();
#else //ifndef CONFIG_REALTEK_ARM_WRAPPER
        HWM_ERR("[%s]" ARM_WRAPPER_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_REALTEK_ARM_WRAPPER
}

static void clear_arm_wrapper_all(void)
{
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        int i;
        for(i = 1;i < WRAPPER_NUM;++i)
        {
                clear_arm_wrapper_arange(i);
        }
#else //ifndef CONFIG_REALTEK_ARM_WRAPPER
        HWM_ERR("[%s]" ARM_WRAPPER_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_REALTEK_ARM_WRAPPER
}

static void reset_arm_wrapper_all(void)
{
        clear_arm_wrapper_all();
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        arm_wrapper_monitor_init();
#else //ifndef CONFIG_REALTEK_ARM_WRAPPER
        HWM_ERR("[%s]" ARM_WRAPPER_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_REALTEK_ARM_WRAPPER
}

/*
name : is_include_dcmt
desc : check if param include dcmt setting
ret : 1:include dcmt, 0:not include dcmt
*/
static int is_include_dcmt(hwm_t *pParam)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)

        if(check_rbus_range(pParam->start, pParam->end))
        {
                return 1;
        }

        return 0;
#else //ifndef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
        HWM_ERR("[%s]" DCMT_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
}

static int convert_dcmt_desc(hwm_t *pParam, DC_MT_DESC *pDcmtDesc)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        int i,j;
        pDcmtDesc->start = pParam->start;
        pDcmtDesc->end = pParam->end + 32;
        pDcmtDesc->mode = pParam->mode;
        pDcmtDesc->type = MEMORY_TYPE_SERIAL_BLOCK;

        j = 0;
        for(i = 0;i < 8;++i)
        {
                if(pParam->ip[i] <= 0xff)
                {
                        pDcmtDesc->ip_entry[j].module = (unsigned char)pParam->ip[i];

                        if(HWM_RW == pParam->ip_permission[i])
                        {
                                pDcmtDesc->ip_entry[j].prot = PROT_MODE_RW;
                        }
                        else if(HWM_R == pParam->ip_permission[i])
                        {
                                pDcmtDesc->ip_entry[j].prot = PROT_MODE_RO;
                        }
                        else if(HWM_W == pParam->ip_permission[i])
                        {
                                pDcmtDesc->ip_entry[j].prot = PROT_MODE_WO;
                        }

                        ++j;

                        if((j > 1) && (HWM_MODE_3 == pParam->mode))
                        {
                                HWM_ERR("DCMT mode3 only support single IP monitor!\n");
                                return -1;
                        }
                }
        }

        return 0;
#else //ifndef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
        HWM_ERR("[%s]" DCMT_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
}

int apply_dcmt_desc(DC_MT_DESC *pDcmtDesc)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        return dcmt_set_monitor_auto(pDcmtDesc);
#else //ifndef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
        HWM_ERR("[%s]" DCMT_DISABLE_TIP "\n", __FUNCTION__);
        return -1;
#endif //endif CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER

}

static void show_dcmt_last_set(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        dump_dcmt_last_set_only();
#else //ifndef CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
        HWM_ERR("[%s]" DCMT_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
}

static void clear_dcmt_all(void)
{
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        dcmt_clear_monitor_all();
#else
        HWM_ERR("[%s]" DCMT_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
}

static void reset_dcmt_all(void)
{
        clear_dcmt_all();
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        dc_mt_monitor_init();
#else
        HWM_ERR("[%s]" DCMT_DISABLE_TIP "\n", __FUNCTION__);
#endif //endif CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER
}

static void switch_scpu_panic_mode(int keep_go)
{
        if(keep_go)
        {
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
                kernel_trap_warning = DCMT_INTR_MODE_SIMPLE;
                dcmt_unset_avk_intr_routing();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
                sb2_dbg_kernel_trap_warning = 0;
#endif

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
                arm_wrapper_kernel_trap_warning = 0;
#endif

        }
        else
        {
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
                kernel_trap_warning = DCMT_INTR_MODE_PANIC;
                dcmt_set_all_cpu_intr_routing();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
                sb2_dbg_kernel_trap_warning = 1;
#endif

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
                arm_wrapper_kernel_trap_warning = 1;
#endif
        }

}

static void dump_hwm_help_info(void)
{
        HWM_ERR("====================HW MONITOR HELP INFO====================\n");
        //.set mem
        HWM_ERR("[*]Set HW monitor\n");
        HWM_ERR("\t simple cmd : \n");
        HWM_ERR("\t set <start>,<end>\n");
        HWM_ERR("\t echo \"set  0x80000100,0x80000200\" > /sys/mt/cntl\n");
        HWM_ERR("\t full cmd : \n");
        HWM_ERR("\t set <start>,<end> [ip: ... ] [mode: ... ] [nopanic]\n");
        HWM_ERR("\t " FGREEN_START "mode1" FCOLOR_END " : ip cannot access <start - end> region with the listed permission.\n");
        HWM_ERR("\t " FGREEN_START "mode2" FCOLOR_END " : only listed ip can access <start - end> region with the listed permission.\n");
        HWM_ERR("\t " FGREEN_START "mode3" FCOLOR_END " : ip can only access <start - end> region with the listed permission.\n");
        HWM_ERR("\t nopanic : kernel will not panic when trash happened.\n");
        HWM_ERR("\t " FYELLOW_START "echo \"set 0x80000100,0x80000200 ip:USB(w),VE(rw),SCPU(r) mode:2 nopanic\" > /sys/realtek_boards/hwm" FCOLOR_END " \n");
        //.clear mem
        HWM_ERR("[*]Clear HW monitor\n");
        HWM_ERR("\t " FYELLOW_START "echo clear > /sys/realtek_boards/hwm" FCOLOR_END "\n");
        //.reset cntl
        HWM_ERR("[*]Reset HW monitor\n");
        HWM_ERR("\t " FYELLOW_START "echo reset > /sys/realtek_boards/hwm" FCOLOR_END "\n");
        //.list usable modules
        HWM_ERR("[*]Get HW monitor module names\n");
        HWM_ERR("\t " FYELLOW_START "echo list > /sys/realtek_boards/hwm" FCOLOR_END "\n");
}

static int translate_desc_to_str(hwm_t *pParam)
{
        int len = 0;
        int i;
        char ip_str[128];
        char region_str[32];
        char *panic_str[2] =
        {
                "panic",
                "keep going"
        };
        char *buf;
        __maybe_unused char *tmp = NULL;
        char *permission_str[4] = {"RW","R","W","NA"};
        int cnt;

        //elment : region ip mode panic

        //get ip string
        memset(ip_str,0,sizeof(ip_str));
        buf = ip_str;
        cnt = 0;
        for(i = 0;i < 8;++i)
        {
                if(VIRT_MODULE_SCPU == pParam->ip[i])
                {
                        len = snprintf(buf,128-cnt,"SCPU");
                        buf += len;
                        cnt += len;
                }
                else if(MODULE_SB2_VCPU == pParam->ip[i])
                {
                        len = snprintf(buf,128-cnt,"VCPU1");
                        buf += len;
                        cnt += len;
                }
#ifdef MODULE_SB2_VCPU2
                else if(MODULE_SB2_VCPU2 == pParam->ip[i])
                {
                        len = snprintf(buf,128-cnt,"VCPU2");
                        buf += len;
                        cnt += len;
                }
#endif
                else if(VIRT_MODULE_NULL == pParam->ip[i])
                {
                        //len = snprintf(buf,128-cnt,"NA");
                        //buf += len;
                        //cnt += len;
                        continue;
                }else if(MODULE_ANY == pParam->ip[i])
                {
                        len = snprintf(buf,128-cnt,"ANY");
                        buf += len;
                        cnt += len;
                }
                else
                {
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
                        if(pParam->ip[i] <= 0xff)
                        {
                                tmp = DCMT_module_str(pParam->ip[i]);
                        }

                        if(tmp == NULL)
                        {
                                continue;
                        }
                        len = snprintf(buf,128-cnt,tmp);
                        buf += len;
                        cnt += len;
#endif
                }

                if(pParam->ip_permission[i] < HWM_PERMISION_INVALID)
                {
                        len = snprintf(buf,128-cnt,"(%s)",permission_str[pParam->ip_permission[i]]);
                        buf += len;
                        cnt += len;
                }
                else
                {
                        len = snprintf(buf,128-cnt,"(%s)",permission_str[HWM_PERMISION_INVALID]);
                        buf += len;
                        cnt += len;
                }

                len = snprintf(buf,128-cnt,",");
                buf += len;
                cnt += len;
        }


        //get region string
        if(!check_rbus_range(pParam->start, pParam->end))
        {
                snprintf(region_str,32,"0x%08x-0x%08x",pParam->start,(pParam->end) + 3);
        }
        else
        {
                snprintf(region_str,32,"0x%08x-0x%08x",pParam->start,(pParam->end) + 63);
        }
        //get keepgo string

        HWM_ERR("\n");
        if(HWM_MODE_1 == pParam->mode)
        {
                HWM_ERR(FGREEN_START "you choose these ip<%s> cannot access region <%s>,kernel will <%s> when trash happened!\n" FCOLOR_END,
                        ip_str,region_str,panic_str[pParam->keep_go]);
        }
        else if(HWM_MODE_2 == pParam->mode)
        {
                HWM_ERR(FGREEN_START "you choose only these ip<%s> can access region <%s>,kernel will <%s> when trash happened!\n" FCOLOR_END,
                        ip_str,region_str,panic_str[pParam->keep_go]);
        }
        else if(HWM_MODE_3 == pParam->mode)
        {
                HWM_ERR(FGREEN_START "you choose this ip<%s> can only access region <%s>,kernel will <%s> when trash happened!\n" FCOLOR_END,
                        ip_str,region_str,panic_str[pParam->keep_go]);
                HWM_ERR(FYELLOW_START "warning : mode3 is only supported by DCMT!!!\n" FCOLOR_END);
        }
        else
        {
                HWM_ERR(FRED_START "you choose a invalid mode!\n" FCOLOR_END);
        }

        return 0;
}

//setting API
int HW_monitor_set(hwm_t *pParam)
{
        sb2_dbg_param sb2_desc[3];
        arm_wrapper_t arm_desc;
        DC_MT_DESC dcmt_desc;
        int sb2_ret = 0,arm_ret = 0,dcmt_ret = 0;
        int tmp_ret = 0;

        memset(&sb2_desc,0,3 * sizeof(sb2_dbg_param));
        memset(&arm_desc,0,sizeof(arm_desc));
        memset(&dcmt_desc,0,sizeof(dcmt_desc));
        //-param check
        if(is_hwm_desc_invalid(pParam))
        {
                HWM_ERR("[%s] pParam check failed!\n",__FUNCTION__);
                return -1;
        }

        //-apply panic/non-panic mode
        switch_scpu_panic_mode(pParam->keep_go);
        //-get rbus range,SB2 setting
        if(!check_rbus_range(pParam->start, pParam->end))
        {
                convert_sb2_desc(pParam, sb2_desc);
                sb2_ret = 0;
                tmp_ret = apply_sb2_desc(&sb2_desc[HWM_RW]);
                if(tmp_ret >= 0)
                {
                        show_sb2_last_set();
                }
                sb2_ret |= (sb2_desc[HWM_RW].set_id > 0)?(tmp_ret):(0);

                tmp_ret = apply_sb2_desc(&sb2_desc[HWM_R]);
                if(tmp_ret >= 0)
                {
                        show_sb2_last_set();
                }
                sb2_ret |= (sb2_desc[HWM_R].set_id > 0)?(tmp_ret):(0);

                tmp_ret = apply_sb2_desc(&sb2_desc[HWM_W]);
                if(tmp_ret >= 0)
                {
                        show_sb2_last_set();
                }
                sb2_ret |= (sb2_desc[HWM_W].set_id > 0)?(tmp_ret):(0);

                goto ret;
        }

        //-get dbus range,DCMT&arm_wrapper setting
        if(is_include_scpu_monitor(pParam))
        {
                arm_ret = convert_arm_wrapper_desc(pParam, &arm_desc);
                if(arm_ret >= 0)
                {
                        arm_ret = apply_arm_wrapper_desc(&arm_desc);
                }

                if(arm_ret >= 0)
                {
                        show_arm_wrapper_last_set();
                }
        }

        if(is_include_dcmt(pParam))
        {
                dcmt_ret = convert_dcmt_desc(pParam, &dcmt_desc);
                if(dcmt_ret >= 0)
                {
                        dcmt_ret = apply_dcmt_desc(&dcmt_desc);
                }

                if(dcmt_ret >= 0)
                {
                        show_dcmt_last_set();
                }
        }

ret:
        return (sb2_ret|arm_ret|dcmt_ret);
}
EXPORT_SYMBOL(HW_monitor_set);

void invalid_each_ip(hwm_t * param)
{
        int i;

        for(i = 0;i < 8;++i)
        {
                param->ip[i] = VIRT_MODULE_NULL;
                param->ip_permission[i] = HWM_PERMISION_INVALID;
        }
}
EXPORT_SYMBOL(invalid_each_ip);

/*shell cmd related API*/
static int parse_ip_and_permission(char * buffer,hwm_t * param)
{
        char * ip_list[8];
        char * tmp_ip;
        char * tmp_permission;
        char * dft_permission = "w";
        const char* prot_str[3] = {"rw", "r", "w"};//do not change the order, cause it's same as : HWM_RW,HWM_R,HWM_W
        int ip_cnt;
        int i,j,k;
        __maybe_unused int tmp;
        __maybe_unused unsigned char module_id[8];

        //init ip and permission as invalid value
        invalid_each_ip(param);

        //get each ip string
        ip_cnt = get_arguments((char*)buffer, 8, ip_list);
        if(ip_cnt <= 0)
        {
                return -1;
        }

        //poll each ip and permission
        k = 0;
        for(i = 0;i < ip_cnt;++i)
        {
                if(k >= 8)
                {
                        HWM_ERR("too much module in HW monitor list!\n");
                        return -1;
                }
                //HWM_DVP("[%s] round%d : \n",__FUNCTION__,i+1);
                //dump_hwm_param(param);
                tmp_ip = ip_list[i];
                tmp_permission = dft_permission;
                if(__find_char_pos(ip_list[i],')') >= 0) //process default rw prot
                {
                        tmp_permission = tmp_ip + __find_char_pos(ip_list[i],'(') + 1;
                        __replace_chr(ip_list[i], ')', '\0');//replace ')' first
                        __replace_chr(ip_list[i], '(', '\0');
                        //HWM_DVP("get permission[%d] : %s\n", i, tmp_permission);
                }

                for(j = 0; j < 3; ++j)
                {
                        if ( (strlen(tmp_permission) == strlen(prot_str[j])) && !strncasecmp(tmp_permission, prot_str[j], strlen(tmp_permission)) )
                        {
                                param->ip_permission[k] = j;
                                //HWM_DVP("get permission code[%d] : %d\n", k, j);
                        }
                }

                //check cpu name
                if(strncasecmp(tmp_ip,"SCPU",strlen("SCPU")) == 0)
                {
                        param->ip[k] = VIRT_MODULE_SCPU;
                        k += 1;
                        continue;
                }
                else if(strncasecmp(tmp_ip,"VCPU1",strlen("VCPU1")) == 0)
                {
                        param->ip[k] = MODULE_SB2_VCPU;
                        k += 1;
                        continue;
                }
#ifdef MODULE_SB2_VCPU2
                else if(strncasecmp(tmp_ip,"VCPU2",strlen("VCPU2")) == 0)
                {
                        param->ip[k] = MODULE_SB2_VCPU2;
                        k += 1;
                        continue;
                }
#endif
                else if(strncasecmp(tmp_ip,"VCPU",strlen("VCPU")) == 0)
                {
                        param->ip[k] = MODULE_SB2_VCPU;
                        k += 1;
                        continue;
                }

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
                memset(module_id,0xff,sizeof(module_id));
                tmp = parse_module_name(tmp_ip,(unsigned char *)module_id,8-k);
                if(tmp < 0)
                {
                        HWM_ERR("HW monitor parse module name failed!\n");
                        return -1;
                        //continue;// invalid module, skip this one
                }

                for(j = 0; j < tmp; ++j) //sync ip permission
                {
                        param->ip[k+j] = (unsigned int)module_id[j];
                        param->ip_permission[k+j] = param->ip_permission[k];
                }
                k += tmp;
#endif
        }

        return k;
}

static int parse_hwm_set_cmd (const char *buffer,hwm_t * param)
{
        const char* cmd = buffer;
        char* field[8]; //fields include : set + range + ip + mode + panic, reserve 3 filed
        int field_cnt = 0;
        int i;
        unsigned int start = 0,end = 0,mode = 0;

        field_cnt = get_space_arguments((char*)cmd, 8, field);
        if(field_cnt <= 0)
        {
                return -1;
        }

        param->keep_go = 0;//set hw monitor default value : panic mode
        param->ip[0] = MODULE_ANY;
        param->ip_permission[0] = HWM_RW;
        param->mode = HWM_MODE_2;

        for(i = 0;i < field_cnt;++i)
        {
                if(1 == sscanf(field[i],"mode:%d",&mode))
                {
                        param->mode = mode;
                        continue;
                }

                if(2 == sscanf(field[i],"%x,%x",&start,&end))
                {
                        param->start = start;
                        param->end = end;
                        continue;
                }

                if(strncmp(field[i],"nopanic",strlen("nopanic")) == 0)
                {
                        param->keep_go = 1;
                        continue;
                }

                if(strncmp(field[i],"set",strlen("set")) == 0)
                {
                        continue; //do nothing for this field
                }

                if(strncmp(field[i],"ip:",strlen("ip:")) == 0)
                {
                        field[i] += strlen("ip:");
                        if(parse_ip_and_permission(field[i],param) < 0)
                        {
                                return -1;
                        }
                        continue;
                }

                HWM_ERR(FCOLOR_END "Ignore Unknown Param Field : [%s]!" FCOLOR_END "\n",field[i]);
        }

        return 0;
}

void multi_print_buf(char * buf)
{
        char * subbuf = buf;
        int len = strlen(buf);
        int pos = 0;
        int tmp;

        for(pos = 0; pos < len; )
        {
                subbuf = buf+pos;
                tmp = __find_char_pos(subbuf,'\n');
                *(subbuf+tmp) = '\0';
                HWM_ERR("%s\n",subbuf);
                *(subbuf+tmp) = '\n';
                pos = pos + tmp + 1;
        }
}

void hw_monitor_set_cmd (const char *buffer)
{
        char * buf = NULL;
        hwm_t param;
        __maybe_unused char * pos;

        if (strncmp(buffer, "set", strlen("set")) == 0)
        {
                memset(&param, 0, sizeof(param));
                invalid_each_ip(&param);//use invalid module name and permission by default
                //get hwm_t
                if(parse_hwm_set_cmd(buffer,&param) < 0)
                {
                        HWM_ERR(FRED_START "HW monitor parse cmd fail!" FCOLOR_END "\n");
                        return;
                }
                //dump_hwm_param(&param);
                HW_monitor_set(&param);
                translate_desc_to_str(&param);
        }
        else if (strncmp(buffer, "reset", strlen("reset")) == 0)
        {
                //reset hw-monitor
                reset_dcmt_all();
                reset_arm_wrapper_all();
                reset_sb2_all();
                HWM_ERR("reset HW monitor finish!\n");
        }
        else if (strncmp(buffer, "clear", strlen("clear")) == 0)
        {
                //clear hw-monitor
                clear_dcmt_all();
                clear_arm_wrapper_all();
                clear_sb2_all();
                HWM_ERR("clear HW monitor finish!\n");
        }
        else if (strncmp(buffer, "list", strlen("list")) == 0)
        {
                //list
                buf = kmalloc(4096,GFP_KERNEL);
                if(buf)
                {
#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
                        get_dcmt_modues(buf);
                        get_module_keys(buf+strlen(buf));
#endif
                        snprintf(buf+strlen(buf), 4096-strlen(buf), "\nOthers : SCPU, ACPU1, ACPU2, VCPU1, VCPU2, KCPU\n");
                        multi_print_buf(buf);
                        kfree(buf);
                }
                buf = NULL;
        }
        else
        {
                //dump help log
                dump_hwm_help_info();
        }
}

#ifndef MODULE
static int __init bootparam_range_dft (char *str)
#else
static int bootparam_range_dft (char *str)
#endif
{
        char value = 0;

        if(str == NULL)
        {
                return -1;
        }

        HWM_DBG("get param %s%s\n",HWM_ENV_RANGE_DFT,str);
        //.get mode,change mode and routing
        if(sscanf(str, "%c", &value) != 1)
        {
                return -1;
        }

        if('y' == value)
        {
                need_monitor_range_dft = 1;//change intr log mode
        }
        else if('n' == value)
        {
                need_monitor_range_dft = 0;
        }

        return 0;
}

#ifndef MODULE
__setup(HWM_ENV_RANGE_DFT, bootparam_range_dft);
#else
extern bool rtk_parse_commandline_equal(const char *string, char *output_string, int string_size);
void get_bootparam_range_dft (void)
{
    char para_buff[100];

    if(rtk_parse_commandline_equal("hwm_range", para_buff,sizeof(para_buff)) == 0)
    {    
        rtd_pr_hw_monitor_err("Error : can't get hwm_range reclaim from bootargs\n");
        return;
    }    
    bootparam_range_dft(para_buff);
    return;
}
#endif

/******************************MODULE INIT FLOW***********************************/

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
extern int arm_wrapper_slverr(unsigned long addr, unsigned int fsr, struct pt_regs *regs);
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
extern int sb2_buserr_handler(unsigned long addr, unsigned int fsr, struct pt_regs *regs);
#endif

extern int arm_wrapper_slverr(unsigned long addr, unsigned int fsr, struct pt_regs *regs);
int hw_monitor_check_handler(unsigned long addr, unsigned int fsr, struct pt_regs *regs)
{
    __maybe_unused int ret = 0;
        
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
    ret = sb2_buserr_handler(addr, fsr, regs);
    if(ret!=-1)
    {
        return ret;
    }
#endif

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
    //arm wrapper read exception
    if(!arm_wrapper_slverr(addr, fsr, regs))
    {
    	return 4;
    }
#endif

        HWM_ERR("no hw monitor buserr happened with fsr:0x%x ? \n",fsr);
        return 5;
}
#if 1
static int hw_monitor_panic_register(struct notifier_block *np, unsigned long event, void *unused)
{
        hw_monitor_check_handler(0,0,NULL);
        return NOTIFY_DONE;
}
//for read handler
static struct notifier_block hw_monitor_read_panic= {
        .notifier_call = hw_monitor_panic_register,
};
extern int register_die_notifier(struct notifier_block *nb);
extern int unregister_die_notifier(struct notifier_block *nb);
//for write handler
static struct notifier_block  hw_monitor_write_panic = {
        .notifier_call = hw_monitor_panic_register,
};
#endif

int rtk_armwrapper_init(void);
int dc_mt_module_init(void);
int rtk_sb2_init(void);
void get_bootparam_arm_wrapper_unset (void);
//#ifdef CONFIG_ARM64
static int hw_monitor_module_init(void)
//#else
//static int __init hw_monitor_module_init(void)
//#endif
{

        avk_auto_range_sync();
        dump_avk_auto_range();

		rtk_mem_prison_ipi_handle_init();
#ifdef MODULE
        hw_monitor_early_init();
        get_bootparam_range_dft();

#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        get_bootparam_arm_wrapper_unset();
#endif

#if IS_ENABLED(CONFIG_RTK_ONLINE_HELP)
        //dbg_help_module();
#endif

#if IS_ENABLED(CONFIG_RTK_KDRV_DC_MEMORY_TRASH_DETCTER)
        dc_mt_module_init();
#endif
#if IS_ENABLED(CONFIG_REALTEK_ARM_WRAPPER)
        rtk_armwrapper_init();
#endif
#if IS_ENABLED(CONFIG_RTK_KDRV_SB2)
        rtk_sb2_init();
#endif

#endif


#ifndef CONFIG_ARM64
#ifdef CONFIG_ARM_LPAE
        hook_fault_code(0x10, hw_monitor_check_handler, SIGBUS, 0, "external abort on bus error");
        hook_fault_code(0x11, hw_monitor_check_handler, SIGBUS, 0, "external async abort on bus error");
#else
        hook_fault_code(3, hw_monitor_check_handler, SIGBUS, 0, "external abort on bus error");
        hook_fault_code(7, hw_monitor_check_handler, SIGBUS, 0, "external async abort on bus error");
#endif
#endif

        atomic_notifier_chain_register(&panic_notifier_list,&hw_monitor_write_panic);
        register_die_notifier(&hw_monitor_read_panic);
        return 0;
}

#if 1
static int hw_monitor_module_exit(struct platform_device *pdev)
{
        atomic_notifier_chain_unregister(&panic_notifier_list, &hw_monitor_write_panic);
        unregister_die_notifier(&hw_monitor_read_panic);
        return 0;
}
#endif

#ifdef CONFIG_ARM64
static int hw_monitor_probe(struct platform_device *pdev)
{
        hw_monitor_module_init();
        return 0;
}
#if 0
static const struct of_device_id hw_monitor_of_match[] =
{
        {
                .compatible = "realtek,hw-monitor",
        },
        {},
};

static struct platform_driver hw_monitor_driver =
{
        .probe          = hw_monitor_probe,
        .driver = {
                .name = "hw-monitor",
                .of_match_table = hw_monitor_of_match,
        },
};
#else
static const struct of_device_id hw_monitor_of_match[] =
{
        {
                .compatible = "realtek,hw-monitor",
        },
        {},
};

static struct platform_driver hw_monitor_driver =
{
        .probe          = hw_monitor_probe,
        .driver = {
                .name = "hw-monitor",
                .of_match_table = hw_monitor_of_match,
        },
	.remove = hw_monitor_module_exit,
};
#endif


MODULE_DEVICE_TABLE(of, hw_monitor_of_match);
module_platform_driver(hw_monitor_driver);
#else
module_init(hw_monitor_module_init);
module_exit(hw_monitor_module_exit);
#endif //CONFIG_ARM64


static DEFINE_RAW_SPINLOCK(show_lock);
static void showallcpu(void *dummy)
{
    unsigned long flags;

    raw_spin_lock_irqsave(&show_lock, flags);
    //console_flush();
    rtd_pr_hw_monitor_err("\n");
    dump_stack();
    rtd_pr_hw_monitor_err("\n");
    //console_flush();
    raw_spin_unlock_irqrestore(&show_lock, flags);
}

void rtk_dump_stacks(void)
{     
    dump_stack();
    smp_call_function(showallcpu, NULL, 0);
}
EXPORT_SYMBOL(rtk_dump_stacks);

void flush_dcache_area (const void *virt_start, const void *virt_end)
//void flush_dcache_area (unsigned long start, unsigned long size)
{
    unsigned long ctr_el0;
    unsigned long start=(unsigned long)virt_start;
    unsigned long size=(unsigned long)virt_end-(unsigned long)virt_start;
    unsigned long address = 0;

    unsigned int cacheline_size;
    asm volatile("mrs %0, ctr_el0" : "=r" (ctr_el0) :: "memory");
    cacheline_size=2<<((ctr_el0>>16)&0xF);

    address = start;
    for(;address<(start+size);address=address+cacheline_size)
    {
        asm volatile("dc civac, %0" : : "r" (address) : "memory");
    }
}
EXPORT_SYMBOL(flush_dcache_area);

void inval_dcache_area (unsigned long start, unsigned long size)
{
    unsigned long ctr_el0;
    unsigned int cacheline_size;
    unsigned long address = 0;
    asm volatile("mrs %0, ctr_el0" : "=r" (ctr_el0) :: "memory");
    cacheline_size=2<<((ctr_el0>>16)&0xF);

    address = start;
    for(;address<(start+size);address=address+cacheline_size)
    {
        asm volatile("dc ivac, %0" : : "r" (address) : "memory");
    }
}
EXPORT_SYMBOL(inval_dcache_area);

