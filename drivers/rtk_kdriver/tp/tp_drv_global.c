/******************************************************************************
 *
 *   Copyright(c) 2014 Realtek Semiconductor Corp. All rights reserved.
 *
 *   @author yping@realtek.com
 *
 *****************************************************************************/

#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/delay.h>
#include <linux/pageremap.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/version.h>

#include <rbus/tp_reg.h>

#include <base_types.h>
#include <tp/tp_def.h>
#include <tp/tp_dbg.h>
#include <tp/tp_ioctl.h>
#include <tp/tp_drv_api.h>
#include <tp/tp_drv.h>
#include <tp/tp_reg_ctrl.h>
#include <tp/tp_drv_global.h>
#include <tp/tp_drv_verify.h>
#include <tp/tp_ta.h>
#include "tp_soc.h"

TP_DRV_T* pTp_drv = NULL;
TP_DRV_MUTEXLOCK_T* pTp_MutexLockList = NULL;
UINT32 logStatusFlag[MAX_TP_COUNT] = {TP_STATUS_LOG_FLAG_NONE, TP_STATUS_LOG_FLAG_NONE, TP_STATUS_LOG_FLAG_NONE, TP_STATUS_LOG_FLAG_NONE};
UINT32 enable_tp_hw_reset = 0;

#if 0
DEFINE_MUTEX(rhal_tp_mutex);
DEFINE_MUTEX(tp_buf_mutex);
#endif

DEFINE_MUTEX(tp_mutexlocklist_mutex);
DEFINE_MUTEX(tp_dump_mutex);

/* parameters for debug */
struct proc_dir_entry * tp_proc_dir = NULL;
struct proc_dir_entry * tp_proc_entry = NULL;

struct proc_dir_entry * tp_proc_entry_monitor = NULL;
struct proc_dir_entry * tp_proc_entry_regdump = NULL;
struct proc_dir_entry * tp_proc_entry_pidList = NULL;
struct proc_dir_entry * tp_proc_entry_mutex = NULL;

unsigned long tp_dbg_log_level_mask = TP_LOG_LEVEL_ERR | TP_LOG_LEVEL_WARNING | TP_LOG_LEVEL_DEBUG;
unsigned long tp_dbg_log_block_mask = TP_LOG_BLOCK_ALWAYS;

UINT32 debug_log_interval_ms = LOG_DISPLAY_TIME_MS;
UINT32 status_log_interval_ms = 3000;
UINT32 check_tpp_buffer_interval_ms = 100;
UINT8 tp_log_onoff = 1;
//UINT8 verbos_log_en = 0;
//UINT8 info_log_en = 0;
//UINT8 warning_log_en = 1;
//UINT8 tp_cnt_log_en = 0;
//UINT8 pcr_log_en = 0;
//UINT8 buffer_log_en = 0;
//UINT8 pid_log_en = 0;
//UINT8 sec_log_en = 0;
//UINT8 sec_deliver_log_en = 0;
//UINT16 sec_deliver_max_data_cnt = 16;
//UINT8 tp_src_log_en = 0;
//UINT8 tp_func_trace_en = 0;
//UINT8 tp_tpo_dbg_en = 0;
INT32 tp_tpoclk_dbg_en = 0;
UINT32 tp_tpoclk_dbg_div = 0x25;
//UINT8 startcode_log_en = 0;
//UINT8 mtp_log_en = 0;
struct  file *rec_fd[MAX_TP_COUNT] = {NULL, NULL, NULL};
struct  file *rec_tlv_fd[MAX_TP_COUNT] = {NULL, NULL, NULL};
struct  file *rec_filter_tlv_fd[MAX_TP_COUNT] = {NULL, NULL, NULL};

UINT8 tp_file_path[MAX_TP_COUNT][256];
UINT8 tp_tlv_file_path[MAX_TP_COUNT][256];
UINT8 tp_filter_tlv_file_path[MAX_TP_COUNT][256];

struct  file *rec_mtp_fd = NULL;
struct  file *rec_oif_fd = NULL;
struct  file *rec_bbp_fd = NULL;

UINT32 record_tlv_use_crc = 0;

UINT32 record_filter_tlv1_use_crc = 0;
UINT32 record_filter_tlv2_use_crc = 0;
UINT32  rec_mtp_id;
//UINT8   dump_tp_pvr_buff_en = 0;
UINT16  dump_tp_pvr_buff_pid[MAX_TP_COUNT] = {0xffff, 0xffff, 0xffff, 0xffff};


static const char* cmd_str[TP_PROC_COUNT] = {
        TP_PROC_PARAM_PRESET_ALLOFF,
        TP_PROC_PARAM_PRESET_BASIC,
        TP_PROC_PARAM_PRESET_NIGHTRUN,
        TP_PROC_PARAM_LOG_ONOFF,
        TP_PROC_PARAM_DBG_LOG_TIME,
        TP_PROC_PARAM_VERBOSE_LOG_EN,
        TP_PROC_PARAM_INFO_LOG_EN,
        TP_PROC_PARAM_WARNING_LOG_EN,
        TP_PROC_PARAM_TRACE_LOG_EN,
        TP_PROC_PARAM_TP_COUNT,
        TP_PROC_PARAM_PCR,
        TP_PROC_PARAM_BUFFER,
        TP_PROC_PARAM_PID_STATUS,
        TP_PROC_PARAM_SEC_STAUTS,
        TP_PROC_PARAM_SEC_DELIVER,
        TP_PROC_PARAM_PID_EN,
        TP_PROC_PARAM_TP_SRC,
        TP_PROC_PARAM_RECORD_TS_EN,
        TP_PROC_PARAM_RECORD_TS_MTP_EN,
        TP_PROC_PARAM_TPO_DBG_EN,
        TP_PROC_PARAM_TPOCLK_DBG_EN,
        TP_PROC_PARAM_DUMP_TP_PVR_BUFF_EN,
        TP_PROC_PARAM_DUMP_TP_PVR_BUFF_PID,
        TP_PROC_PARAM_DUMP_PID_LIST,
        TP_PROC_PARAM_RESET,
        TP_PROC_PARAM_STARTCODE_LOG_EN,
        TP_PROC_PARAM_MTP_LOG_EN,
        TP_PROC_PARAM_FR_EN,
        TP_PROC_PARAM_MUTEX_DEBUG,
        TP_PROC_PARAM_RUN_VERIFY_TEST,
        TP_PROC_PARAM_CHANNEL_SCAN,
        TP_PROC_PARAM_TPO_CLK_RADOM_GEN,
        TP_PROC_PARAM_RECORD_OIF_EN,
        TP_PROC_PARAM_RECORD_BBP_EN,
        TP_PROC_PARAM_RECORD_TLV_EN,
        TP_PROC_PARAM_RECORD_FILTER_TLV_EN,

};


RTK_TLV_REG_T tlv_reg_tlv0 =
{
    .TLV_CTRL = TP_TLV_CTRL_reg,
    .TLV_SYNC = TP_TLV_SYNC_reg,
    .TLV_CFG = TP_TLV_CFG_reg,
    .TLV_STC = TP_TLV_STC_reg,
    .TLV_DESCRIPTOR = TP_TLV_DESCRIPTOR_reg,
    .TLV_DESCRIPTOR_DATA0 = TP_TLV_DESCRIPTOR_DATA0_reg,
    .TLV_DESCRIPTOR_DATA1 = TP_TLV_DESCRIPTOR_DATA1_reg,
    .TLV_DESCRIPTOR_WRITE = TP_TLV_DESCRIPTOR_WRITE_reg,
    .TLV_DESCRIPTOR_READ = TP_TLV_DESCRIPTOR_READ_reg,
    .TLV_DESCRIPTOR_FLAG = TP_TLV_DESCRIPTOR_FLAG_reg,
    .TLV_DESCRIPTOR_INT = TP_TLV_DESCRIPTOR_INT_reg,
    .TLV_DESCRIPTOR_STATUS = TP_TLV_DESCRIPTOR_STATUS_reg,
    .TLV_DESCRIPTOR_TIME = TP_TLV_DESCRIPTOR_TIME_reg,
    .TLV_DESCRIPTOR_TIME_DBG = TP_TLV_DESCRIPTOR_TIME_DBG_reg,
    .TLV_DESCRIPTOR_PKT_NUM = TP_TLV_DESCRIPTOR_PKT_NUM_reg,
    .TLV_BUFFER = TP_TLV_BUFFER_reg,
    .TLV_BUFFER_FLAG = TP_TLV_BUFFER_FLAG_reg,
    .TLV_BUFFER_INT = TP_TLV_BUFFER_INT_reg,
    .TLV_BUFFER_STATUS = TP_TLV_BUFFER_STATUS_reg,
    .TLV_PARSER_MAIN = TP_TLV_PARSER_MAIN_reg,
    .TLV_PARSER_DMA = TP_TLV_PARSER_DMA_reg,
    .TLV_PKT = TP_TLV_PKT_reg,
    .TLV_DROP = TP_TLV_DROP_reg,
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
    .TLV_FILTER_CFG = TP_TLV_FILTER_CFG_reg,
    .TLV_FILTER_CFG_CTRL = TP_TLV_FILTER_CFG_CTRL_reg,
    .TLV_FILTER_CFG_EN = TP_TLV_FILTER_CFG_EN_reg,
    .TLV_FILTER_CFG_D0 = TP_TLV_FILTER_CFG_D0_reg,
    .TLV_FILTER_CFG_D1 = TP_TLV_FILTER_CFG_D1_reg,
    .TLV_FILTER_CFG_D2 = TP_TLV_FILTER_CFG_D2_reg,
    .TLV_FILTER_CFG_D3 = TP_TLV_FILTER_CFG_D3_reg,
    .TLV_FILTER_CFG_D4 = TP_TLV_FILTER_CFG_D4_reg,
    .TLV_FILTER_CFG_D5 = TP_TLV_FILTER_CFG_D5_reg,
    .TLV_FILTER_CFG_D6 = TP_TLV_FILTER_CFG_D6_reg,
    .TLV_FILTER_CFG_D7 = TP_TLV_FILTER_CFG_D7_reg,
    .TLV_FILTER_CFG_D8 = TP_TLV_FILTER_CFG_D8_reg,
    .TLV_FILTER_CFG_D9 = TP_TLV_FILTER_CFG_D9_reg,
    .TLV_FILTER_CFG_D10 = TP_TLV_FILTER_CFG_D10_reg,
    .TLV_FILTER_CFG_D11 = TP_TLV_FILTER_CFG_D11_reg,
    .TLV_FILTER_DROP = TP_TLV_FILTER_DROP_reg,
    .TLV_NTP = TP_TLV_NTP_reg,
    .TLV_NTP_MASK = TP_TLV_NTP_MASK_reg,
    .TLV_NTP_CFG0 = TP_TLV_NTP_CFG0_reg,
    .TLV_NTP_CFG1 = TP_TLV_NTP_CFG1_reg,
    .TLV_NTP_CFG2 = TP_TLV_NTP_CFG2_reg,
    .TLV_NTP_CFG3 = TP_TLV_NTP_CFG3_reg,
    .TLV_NTP_CFG4 = TP_TLV_NTP_CFG4_reg,
    .TLV_NTP_STC = TP_TLV_NTP_STC_reg,
    .TLV_STC_HIGH = TP_TLV_STC_HIGH_reg,
    .TLV_STC_LOW = TP_TLV_STC_LOW_reg,
    .TLV_NTP_HIGH = TP_TLV_NTP_HIGH_reg,
    .TLV_NTP_LOW = TP_TLV_NTP_LOW_reg,
    .TLV_PARSER_MAIN_ERROR = TP_TLV_PARSER_MAIN_ERROR_reg,
#endif
};
#ifdef CONFIG_RTK_SUPPORT_TWO_HWTLV
RTK_TLV_REG_T tlv_reg_tlv1 =
{
    .TLV_CTRL = TP_TLV1_CTRL_reg,
    .TLV_SYNC = TP_TLV1_SYNC_reg,
    .TLV_CFG = TP_TLV1_CFG_reg,
    .TLV_STC = TP_TLV1_STC_reg,
    .TLV_DESCRIPTOR = TP_TLV1_DESCRIPTOR_reg,
    .TLV_DESCRIPTOR_DATA0 = TP_TLV1_DESCRIPTOR_DATA0_reg,
    .TLV_DESCRIPTOR_DATA1 = TP_TLV1_DESCRIPTOR_DATA1_reg,
    .TLV_DESCRIPTOR_WRITE = TP_TLV1_DESCRIPTOR_WRITE_reg,
    .TLV_DESCRIPTOR_READ = TP_TLV1_DESCRIPTOR_READ_reg,
    .TLV_DESCRIPTOR_FLAG = TP_TLV1_DESCRIPTOR_FLAG_reg,
    .TLV_DESCRIPTOR_INT = TP_TLV1_DESCRIPTOR_INT_reg,
    .TLV_DESCRIPTOR_STATUS = TP_TLV1_DESCRIPTOR_STATUS_reg,
    .TLV_DESCRIPTOR_TIME = TP_TLV1_DESCRIPTOR_TIME_reg,
    .TLV_DESCRIPTOR_TIME_DBG = TP_TLV1_DESCRIPTOR_TIME_DBG_reg,
    .TLV_DESCRIPTOR_PKT_NUM = TP_TLV1_DESCRIPTOR_PKT_NUM_reg,
    .TLV_BUFFER = TP_TLV1_BUFFER_reg,
    .TLV_BUFFER_FLAG = TP_TLV1_BUFFER_FLAG_reg,
    .TLV_BUFFER_INT = TP_TLV1_BUFFER_INT_reg,
    .TLV_BUFFER_STATUS = TP_TLV1_BUFFER_STATUS_reg,
    .TLV_PARSER_MAIN = TP_TLV1_PARSER_MAIN_reg,
    .TLV_PARSER_DMA = TP_TLV1_PARSER_DMA_reg,
    .TLV_PKT = TP_TLV1_PKT_reg,
    .TLV_DROP = TP_TLV1_DROP_reg,
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
    .TLV_FILTER_CFG = TP_TLV1_FILTER_CFG_reg,
    .TLV_FILTER_CFG_CTRL = TP_TLV1_FILTER_CFG_CTRL_reg,
    .TLV_FILTER_CFG_EN = TP_TLV1_FILTER_CFG_EN_reg,
    .TLV_FILTER_CFG_D0 = TP_TLV1_FILTER_CFG_D0_reg,
    .TLV_FILTER_CFG_D1 = TP_TLV1_FILTER_CFG_D1_reg,
    .TLV_FILTER_CFG_D2 = TP_TLV1_FILTER_CFG_D2_reg,
    .TLV_FILTER_CFG_D3 = TP_TLV1_FILTER_CFG_D3_reg,
    .TLV_FILTER_CFG_D4 = TP_TLV1_FILTER_CFG_D4_reg,
    .TLV_FILTER_CFG_D5 = TP_TLV1_FILTER_CFG_D5_reg,
    .TLV_FILTER_CFG_D6 = TP_TLV1_FILTER_CFG_D6_reg,
    .TLV_FILTER_CFG_D7 = TP_TLV1_FILTER_CFG_D7_reg,
    .TLV_FILTER_CFG_D8 = TP_TLV1_FILTER_CFG_D8_reg,
    .TLV_FILTER_CFG_D9 = TP_TLV1_FILTER_CFG_D9_reg,
    .TLV_FILTER_CFG_D10 = TP_TLV1_FILTER_CFG_D10_reg,
    .TLV_FILTER_CFG_D11 = TP_TLV1_FILTER_CFG_D11_reg,
    .TLV_FILTER_DROP = TP_TLV1_FILTER_DROP_reg,
    .TLV_NTP = TP_TLV1_NTP_reg,
    .TLV_NTP_MASK = TP_TLV1_NTP_MASK_reg,
    .TLV_NTP_CFG0 = TP_TLV1_NTP_CFG0_reg,
    .TLV_NTP_CFG1 = TP_TLV1_NTP_CFG1_reg,
    .TLV_NTP_CFG2 = TP_TLV1_NTP_CFG2_reg,
    .TLV_NTP_CFG3 = TP_TLV1_NTP_CFG3_reg,
    .TLV_NTP_CFG4 = TP_TLV1_NTP_CFG4_reg,
    .TLV_NTP_STC = TP_TLV1_NTP_STC_reg,
    .TLV_STC_HIGH = TP_TLV1_STC_HIGH_reg,
    .TLV_STC_LOW = TP_TLV1_STC_LOW_reg,
    .TLV_NTP_HIGH = TP_TLV1_NTP_HIGH_reg,
    .TLV_NTP_LOW = TP_TLV1_NTP_LOW_reg,
    .TLV_PARSER_MAIN_ERROR = TP_TLV1_PARSER_MAIN_ERROR_reg,
#endif
};
#endif
/****************************************/

#define isnumber(c)      (c>=0x30 && c<=0x39)
#define isalphabet(c)    (c>=0x41 && c<=0x5A) || (c>=0x61 && c<=0x7A)
#define isalnum(c)       (isnumber(c) || isalphabet(c))

/***************************************************************************
  ------------------- Proc FS  --------------------
 ****************************************************************************/
#if 0
static UINT8* __find_next_str(UINT8* str)
{
        char* end = str + strlen(str);

        while(str < (UINT8*)end) {
                if (isalnum(*str)) /* is alphabet or number */
                        return str;
                str++;
        }

        return NULL;
}
#endif

static ssize_t tp_proc_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
        UINT8 str[128 + 1];
        UINT8 rec_file[257];

        UINT32 i;
        UINT8 *cmd = str;
        INT32 input_data, input_data2, enable, temp_data;
        INT32 verify_test_itme;
        UINT8 pid_filter_en = 1, tp_id;
        UINT32 u32_tp_id = 0;
        UINT32 pid_number = 0;
        tp_tp_tf0_cntl_RBUS tp_cntl;
        UINT16 pid_idx;
        UINT8 ddr_buf_idx;
        TPK_PID_REG_PARAM_T pid_reg;
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
        UINT8 rec_mtp_file[257];
        struct  file *tmp_fd = NULL;
        UINT32 n;
#endif
        UINT32 j;

#ifdef TP_SUPPORT_CI
        UINT32 tpo_pre, tpo_post, div16;
        INT32 ret;
        UINT32 tpo_clk[12][2] = /* pre, post */ {
                {7, 7}, {3, 8}, {3, 5}, {1, 7}, {1, 5},
                {1, 4}, {0, 5}, {0, 4}, {0, 3}, {1, 1},
                {0, 2}, {0, 1}
        };
#endif

        /* check parameter */
        if(buf == NULL) {
                TP_ERROR("buf=%p\n", buf);
                return -EFAULT;
        }

        if (count > 128) { /* procfs write and read has PAGE_SIZE limit */
                TP_WARNING("command too long, set length to 128 Byte\n");
                count = 128;
        }

        if (copy_from_user(str, buf, count)) {
                TP_ERROR("copy_from_user failed! (buf=%p, count=%u)\n", buf, (unsigned int)count);
                return -EFAULT;
        }

        str[count] = '\0';
#if 1
        TP_DBG_SIMPLE("proc data: %s\n", str);
#endif

        /* get command string */
        for( i = 0; i < TP_PROC_COUNT; i++) {
                if (strncmp(str, cmd_str[i], strlen(cmd_str[i])) == 0) {
                        TP_DBG_SIMPLE("TP debug command: %s\n", cmd_str[i]);
                        break;
                }
        }

        /* command out of range check */
        if ( i >= TP_PROC_COUNT ) {
                TP_ERROR("Cannot find your command: \"%s\"\n", str);
                return -EINVAL;

        } else {
                /* get parameter */
                cmd += strlen(cmd_str[i]);
                if (sscanf(cmd, "%d", &input_data) < 1) {
                        TP_DBG_SIMPLE("sscanf get more data failed (%s) - there is no more parameter.\n", cmd);
                        //return -EFAULT;
                } else {
                        TP_DBG_SIMPLE("input data=%d\n", input_data);
                }
        }

        switch(i) {
                case TP_PROC_PRESET_ALLOFF: {
                        debug_log_interval_ms = LOG_DISPLAY_TIME_MS;
                        tp_log_onoff = 0;

                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_VERBOSE);
                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_INFO);
                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_WARNING);

                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_CNT);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PCR);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_BUFFER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PID);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_SECTION);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FRAMER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FUNCTIN_CALLER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_TPOUT);

                        TP_DBG_SIMPLE("Turn Off all TP logs\n");

                        break;
                }
                case TP_PROC_PRESET_BASIC: {
                        debug_log_interval_ms = LOG_DISPLAY_TIME_MS;
                        tp_log_onoff = 1;

                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_VERBOSE);
                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_INFO);
                        SET_LOG_LEVEL_MASK(TP_LOG_LEVEL_WARNING);

                        SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_CNT);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PCR);
                        SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_BUFFER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PID);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_SECTION);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FRAMER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FUNCTIN_CALLER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_TPOUT);

                        TP_DBG_SIMPLE("Enable TP Preset Basic log\n");

                        break;
                }
                case TP_PROC_PRESET_NIGHTRUN: {
                        debug_log_interval_ms = LOG_DISPLAY_TIME_MS;
                        tp_log_onoff = 1;

                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_VERBOSE);
                        CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_INFO);
                        SET_LOG_LEVEL_MASK(TP_LOG_LEVEL_WARNING);

                        SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_CNT);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PCR);
                        SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_BUFFER);
                        SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_PID);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_SECTION);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FRAMER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FUNCTIN_CALLER);
                        CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_TPOUT);

                        TP_DBG_SIMPLE("Enable TP Preset Night Run log\n");

                        break;
                }
                case TP_PROC_LOG_ONOFF: {
                        tp_log_onoff = (UINT32)input_data;

                        if ( input_data == 1 ) {
                                TP_DBG_SIMPLE("Enable TP log\n");
                        } else {
                                TP_DBG_SIMPLE("Disable TP log\n");
                        }

                        break;
                }
                case TP_PROC_DBG_LOG_TIME: {
                        TP_DBG_SIMPLE("Original TP debug log interval time :%d(ms)\n", input_data);

                        debug_log_interval_ms = (UINT32)input_data;

                        TP_DBG_SIMPLE("Set TP debug log interval time to %d(ms)\n", input_data);
                        break;
                }
                case TP_PROC_VERBOSE_LOG_EN: {
                        if(input_data == 1) {
                                SET_LOG_LEVEL_MASK(TP_LOG_LEVEL_VERBOSE);
                        } else {
                                CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_VERBOSE);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP verbos log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP verbos log\n");

                        break;
                }
                case TP_PROC_INFO_LOG_EN: {
                        if(input_data == 1) {
                                SET_LOG_LEVEL_MASK(TP_LOG_LEVEL_INFO);
                        } else {
                                CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_INFO);
                        }

                        if ( input_data == 1 ) {
                                TP_DBG_SIMPLE("Enable TP info log\n");
                        } else {
                                TP_DBG_SIMPLE("Disable TP info log\n");
                        }

                        break;
                }
                case TP_PROC_WARNING_LOG_EN: {
                        if(input_data == 1) {
                                SET_LOG_LEVEL_MASK(TP_LOG_LEVEL_WARNING);
                        } else {
                                CLEAR_LOG_LEVEL_MASK(TP_LOG_LEVEL_WARNING);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP warning log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP warning log\n");

                        break;
                }
                case TP_PROC_TRACE_LOG_EN: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_FUNCTIN_CALLER);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FUNCTIN_CALLER);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP function trace log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP function trace log\n");

                        break;
                }
                case TP_PROC_TP_COUNT: {

                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_CNT);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_CNT);
                        }


                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP packet count log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP packet count log\n");

                        break;
                }
                case TP_PROC_PCR: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_PCR);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PCR);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP PCR log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP PCR log\n");

                        break;
                }
                case TP_PROC_BUFFER: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_BUFFER);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_BUFFER);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP buffer log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP buffer log\n");

                        break;
                }
                case TP_PROC_PID_STATUS: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_PID);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PID);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP pid status log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP pid status log\n");

                        break;
                }
                case TP_PROC_SEC_STAUTS: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_SECTION);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_SECTION);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP section status log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP section status log\n");

                        break;
                }
                case TP_PROC_SEC_DELIVER: {
#if 0
                        sec_deliver_log_en = (UINT8)input_data;

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP section deliver log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP section deliver log\n");

                        if(sec_deliver_log_en > 1) {
                                cmd = __find_next_str(cmd);
                                if(cmd != NULL) {
                                        if (sscanf(cmd, "%d", &input_data) < 1) {
                                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                                return -EFAULT;
                                        } else {
                                                TP_DBG_SIMPLE("deliver max data count=%d\n", input_data);
                                        }
                                        sec_deliver_max_data_cnt = (UINT16)input_data;
                                }
                        }
#endif
                        break;
                }
                case TP_PROC_PID_EN: {
                        tp_id = (UINT8)input_data;
#if 0
                        cmd = __find_next_str(cmd);
                        if(cmd != NULL) {
                                if (sscanf(cmd, "%d", &input_data) < 1) {
                                        TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                        return -EFAULT;
                                } else {
                                        TP_INFO("input data2=%d\n", input_data);
                                }
                                pid_filter_en = (UINT16)input_data;
                        }
#endif
                        if (sscanf(cmd, "%d %d", &input_data, &input_data2) < 1) {
                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                TP_INFO("input data2=%d\n", input_data2);
                        }
                        pid_filter_en = (UINT16)input_data2;

                        if(pid_filter_en) {
                                /* enable pid filter */
                                tp_cntl.pid_en = 1;
                                _tp_lock();
                                Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, 1);
                                _tp_unlock();
                                TP_DBG_SIMPLE("Enable TP%d pid filter\n", tp_id);
                        } else {
                                if(tp_id == 0) {
                                        pid_idx = TP0_FIRST_PID_FILTER_IDX;
                                        ddr_buf_idx = TP0_BUFFER_MASS_DATA;
                                } else if(tp_id == 1) {
                                        pid_idx = TP1_FIRST_PID_FILTER_IDX;
                                        ddr_buf_idx = TP1_BUFFER_MASS_DATA;
                                } else {
                                        pid_idx = TP2_FIRST_PID_FILTER_IDX;
                                        ddr_buf_idx = TP2_BUFFER_MASS_DATA;
                                }

                                _tp_lock();
                                /* set ddr buffer to mass data buf_idx, other parameter won't change */
                                Tp_GetPidParam(pid_idx, &pid_reg);
                                pid_reg.ddr_q = ddr_buf_idx;
                                Tp_SetPidParam(pid_idx, pid_reg);

                                /* disable pid filter */
                                tp_cntl.pid_en = 1;   /* set select bit=1 */
                                Tp_SetTpCtrlReg(tp_id, tp_cntl.regValue, 0);
                                _tp_unlock();

                                TP_DBG_SIMPLE("Disable TP%d pid filter\n", tp_id);

                        }
                        break;
                }
                case TP_PROC_TP_SRC: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_FRAMER);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_FRAMER);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP source log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP source log\n");

                        break;
                }
                case TP_PROC_RECORD_TS_EN: {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                        //cmd +=strlen(cmd_str[i]);
                        if (sscanf(cmd, "%d tp=%u tspath=%256s", &enable, &u32_tp_id, rec_file) < 1) {
                                TP_ERROR("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                TP_DBG_SIMPLE("record_ts_en=%d ,rec_tp_id=%d ,ts_record file=%s\n", enable, u32_tp_id, rec_file);
                                _tp_dump_lock();
                                if(enable) {
                                        if(rec_fd[u32_tp_id]) {
                                                for ( n = 0; n < MAX_TP_COUNT; n++) {
                                                        if (0 == strncmp(rec_file, tp_file_path[n], strlen(tp_file_path[n])))
                                                                break;
                                                }

                                                if(n < MAX_TP_COUNT) {
                                                        TP_ERROR("File path is same, %s\n", rec_file);
                                                        _tp_dump_unlock();
                                                        return -EEXIST;
                                                }

                                                TP_DBG_SIMPLE("TS Record Stopped\n");
                                                filp_close(rec_fd[u32_tp_id], NULL);
                                                rec_fd[u32_tp_id] = NULL;
                                                memset(tp_file_path[u32_tp_id], 0, sizeof(tp_file_path[u32_tp_id]));
                                        }

                                        tmp_fd = filp_open(rec_file, O_RDWR | O_CREAT | O_TRUNC, 0);
                                        if(IS_ERR(tmp_fd)) {
                                                TP_ERROR("Start Rec TS failed, open %s failed\n", rec_file);
                                        } else {
                                                for (n = 0; n < MAX_TP_COUNT; n++) {
                                                        if (0 == strncmp(rec_file, tp_file_path[n], strlen(rec_file)))
                                                                break;
                                                }

                                                if(n < MAX_TP_COUNT) {
                                                        TP_WARNING("File path is same, %s\n", rec_file);
                                                } else {
                                                        TP_DBG_SIMPLE("Start Rec TS to - %s,rec_fd=%p\n", rec_file, tmp_fd);
                                                        memcpy(tp_file_path[u32_tp_id], rec_file, strlen(rec_file));
                                                        rec_fd[u32_tp_id] = tmp_fd;
                                                }
                                        }
                                } else {
                                        if(rec_fd[u32_tp_id]) {
                                                TP_DBG_SIMPLE("TS Record Stopped\n");
                                                filp_close(rec_fd[u32_tp_id], NULL);
                                                rec_fd[u32_tp_id] = NULL;
                                                memset(tp_file_path[u32_tp_id], 0, sizeof(tp_file_path[u32_tp_id]));
                                        }
                                }
                                _tp_dump_unlock();
                        }
#endif
                        break;
                }
                case TP_PROC_RECORD_TS_MTP_EN: {
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                        if (sscanf(cmd, "%d mtp=%u tspath=%256s", &enable, &rec_mtp_id, rec_mtp_file) < 1) {
                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                TP_DBG_SIMPLE("record_ts_mtp_en=%d, rec_mtp_id=%d, ts_mtp_record_file=%s\n", enable, rec_mtp_id, rec_mtp_file);
                                _tp_dump_lock();
                                if(enable) {
                                        if(rec_mtp_fd) {
                                                TP_DBG_SIMPLE("MTP fd exit, MTP TS Record Stopping...\n");
                                                filp_close(rec_mtp_fd, NULL);
                                                rec_mtp_fd = NULL;
                                        }
                                        rec_mtp_fd = filp_open(rec_mtp_file, O_RDWR | O_CREAT | O_TRUNC, 0);

                                        if(rec_mtp_fd)
                                                TP_DBG_SIMPLE("Start Rec MTP TS to - %s, rec_mtp_fd=%p\n", rec_mtp_file, rec_mtp_fd);
                                        else
                                                TP_DBG_SIMPLE("Start Rec MTP TS failed, open %s failed\n", rec_mtp_file);
                                } else {
                                        if(rec_mtp_fd) {
                                                TP_DBG_SIMPLE("MTP TS Record Stopped\n");
                                                filp_close(rec_mtp_fd, NULL);
                                                rec_mtp_fd = NULL;
                                        }
                                }
                                _tp_dump_unlock();
                        }
#endif
                        break;
                }
                case TP_PROC_TPO_DBG_EN: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_TPOUT);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_TPOUT);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TPO debug log\n");
                        else
                                TP_DBG_SIMPLE("Disable TPO debug log\n");

                        break;
                }
                case TP_PROC_TPOCLK_DBG_EN: {
                        int pre_div = 0, post_div = 0;
                        /* tpo_clk = 250 / (pre+post+2) */
                        /* tpo_clk = 1000 / (pre+post+2) //for mark2/mio */
                        if (sscanf(cmd, "%d pre_div=%d post_div=%d", &tp_tpoclk_dbg_en, &pre_div, &post_div) < 1) {
                                TP_ERROR("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        }
                        if(tp_tpoclk_dbg_en){
                                tp_tpoclk_dbg_div = post_div << 8 | pre_div;
                                RHAL_TPOUT_SetClockSpeed(0, pre_div, post_div, 0);
                                TP_WARNING("tp_tpoclk_dbg_div=%x, pre_div=%x, post_div = %x\n", tp_tpoclk_dbg_div, pre_div, post_div);
                        }
                        else{
                                RHAL_TPOUT_SetClockSpeed(0, 0x15, 0x15, 0);
                                TP_WARNING("set tpo clock to default value: 9MHZ\n");
                        }
                        break;
                }
                case TP_PROC_DUMP_TP_PVR_BUFF_EN: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_PVR);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_PVR);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable dump tp PVR buff log\n");
                        else
                                TP_DBG_SIMPLE("Disable dump tp PVR buff log\n");

                        break;
                }
                case TP_PROC_DUMP_TP_PVR_BUFF_PID: {
                        TP_DBG_SIMPLE("cmd = %s\n", cmd);

                        if (sscanf(cmd, "%d, %x,%d", &u32_tp_id, &pid_number, &temp_data) < 2 || pid_number > 0xffff) {
                                TP_WARNING("get dump pvr pid setting failed - invalid parameter (%s)(%d)(%x)(%d)\n", cmd, u32_tp_id, pid_number, temp_data );
                                return -EFAULT;
                        }

                        /* for K3, only support PVR on TP0 & TP1 */
                        if ( u32_tp_id >= 2 ) {
                                TP_WARNING("get dump pvr pid setting failed - invalid parameter of TP number %d, only support PVR(K3*) on TP0 & TP1\n", u32_tp_id);
                                return -EFAULT;
                        }

                        dump_tp_pvr_buff_pid[u32_tp_id] = (UINT16)(pid_number & 0xffff);

                        if ( pid_number == 0xffff ) {
                                TP_DBG_SIMPLE("Set dump TP%d PVR buffer all pid data.\n", u32_tp_id);
                        } else {
                                TP_DBG_SIMPLE("Set dump TP%d PVR buffer Pid.\n", u32_tp_id);
                        }

                        if ( !CHECK_LOG_BLOCK_MASK(TP_LOG_BLOCK_PVR) )
                                TP_DBG_SIMPLE("But you do not enable PVR buff log\n");

                        break;
                }
                case TP_PROC_DUMP_PID_LIST: {
                        _tp_lock();
                        TP_DBG_SIMPLE("\n");
                        TP_DBG_SIMPLE("=================================================================================================================================\n");

                        for ( j = 0 ; j < MAX_TP_COUNT ; j++ ) {
                                Tp_Cmd_Dump_PID_filter_Status(j, NULL);
                        }
                        TP_DBG_SIMPLE("=================================================================================================================================\n");
                        _tp_unlock();
                        break;
                }
                case TP_PROC_RESET: {
                        TP_DBG_SIMPLE("Do TP HW Reset!\n");

                        //RHAL_SetTPHardwareReset(1);
                        enable_tp_hw_reset = 1;
                        break;
                }
                case TP_PROC_STARTCODE_LOG_EN: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_STARTCODE);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_STARTCODE);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP start code log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP start code log\n");

                        break;
                }
                case TP_PROC_MTP_LOG_EN: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP);
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_MTP);
                        }

                        if ( input_data == 1 )
                                TP_DBG_SIMPLE("Enable TP MTP log\n");
                        else
                                TP_DBG_SIMPLE("Disable TP MTP log\n");

                        break;
                }
                case TP_PROC_FR_EN: {
                        if (sscanf(cmd, "%d %d", &enable, &u32_tp_id) < 1) {
                                TP_ERROR("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                RHAL_MTPFrameControl(u32_tp_id, enable);
                                TP_DBG_SIMPLE("fr_en=%d ,tp=%d\n", enable, u32_tp_id);
                        }
                        break;
                }

                case TP_PROC_MUTEX_DEBUG: {
                        UINT32 numOfMutex = 0, numOfFun = 0;
                        struct list_head * ilist = NULL, * jlist = NULL;

                        _tp_mutexlocklist_lock();

                        TP_DBG_SIMPLE("--------------------[TP Mutex Debug]--------------------\n");

                        if (pTp_MutexLockList) {
                                list_for_each(ilist, &pTp_MutexLockList->list) {
                                        TP_DRV_MUTEXLOCK_T* tmp = list_entry(ilist, TP_DRV_MUTEXLOCK_T, list);
                                        if (tmp->funCallStackHead) {
                                                TP_DBG_SIMPLE("========[mutex_%u][%s]========\n", numOfMutex, tmp->name);

                                                numOfFun = 0;
                                                list_for_each(jlist, &tmp->funCallStackHead->funName_list) {
                                                        TP_DRV_MUTEXFUNNAME_T* tmpFun = list_entry(jlist, TP_DRV_MUTEXFUNNAME_T, funName_list);
                                                        if (numOfFun < TP_MUTEX_DBG_NUM_FUN_CALL) {
                                                                TP_DBG_SIMPLE("[%u][%s][%d]\n", numOfFun, tmpFun->funName, tmpFun->lock);
                                                        } else {
                                                                TP_ERROR("The fun call stack is more than %d\n", TP_MUTEX_DBG_NUM_FUN_CALL);
                                                                break;
                                                        }
                                                        numOfFun++;
                                                }
                                        }
                                        numOfMutex++;
                                }
                                TP_DBG_SIMPLE("Total number of Mutex in TP driver: %u\n", numOfMutex);
                        } else
                                TP_ERROR("There is no mutex lock in TP driver.\n");

                        TP_DBG_SIMPLE("--------------------[TP Mutex Debug]--------------------\n");
                        _tp_mutexlocklist_unlock();
                        break;
                }

                case TP_PROC_RUN_VERIFY_TEST: {
                        if (sscanf(cmd, "%d %256s", &verify_test_itme, rec_file) < 1) {
                                TP_ERROR("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        }
                        if(verify_test_itme == 1)
                                TPVerify_ModuleTest(rec_file);
                        break;
                }

                case TP_PROC_CHANNEL_SCAN: {
                        if(input_data == 1) {
                                SET_LOG_BLOCK_MASK(TP_LOG_BLOCK_CHANNEL_SCAN);
                                TP_DBG_SIMPLE("Enable TP channel scan log\n");
                        } else {
                                CLEAR_LOG_BLOCK_MASK(TP_LOG_BLOCK_CHANNEL_SCAN);
                                TP_DBG_SIMPLE("Disable TP channel scan log\n");
                        }

                        break;
                }
#ifdef TP_SUPPORT_CI
                case TP_PROC_TPO_CLK_RANDOM_GEN: {
                        TP_WARNING("Start of TPOut Clock Test!!!\n");
                        ret = TPK_SUCCESS;

                        _tp_lock();
                        ret = tp_input_rate_cnt_detect_thread_start_stop(0);
                        if ( ret != TPK_SUCCESS ) {
                                TP_WARNING("STOP tp input rate cnt detect thread fail! ret=%d\n", ret);
                                TP_WARNING("End of TPOut Clock Test!!!\n");
                                _tp_unlock();
                                break;
                        }
                        /* get original tpo command */
                        Tp_TPO_GetClkDiv( &tpo_pre, &tpo_post, &div16);

                        for( i = 0; i < 12 ; i++) {
                                Tp_TPO_SetClkDiv(tpo_clk[i][0], tpo_clk[i][1], 0);
                                TP_WARNING("Set TPO to %dMhz for 5 seconds\n", TP_SPEED_MHZ / (tpo_clk[i][0] + 2) / (tpo_clk[i][1] + 2));
                                msleep(5000);
                        }

                        Tp_TPO_SetClkDiv(tpo_pre, tpo_post, 0);
                        ret = tp_input_rate_cnt_detect_thread_start_stop(1);
                        if ( ret != TPK_SUCCESS ) {
                                TP_WARNING("START tp input rate cnt detect thread fail! ret=%d\n", ret);
                                TP_WARNING("End of TPOut Clock Test!!!\n");
                                _tp_unlock();
                                break;
                        }

                        _tp_unlock();
                        TP_WARNING("End of TPOut Clock Test!!!\n");
                        break;
                }
#endif
#if IS_ENABLED(CONFIG_RTK_FEATURE_FOR_GKI)
                case TP_PROC_RECORD_OIF_EN: {
                        if (sscanf(cmd, "%d tp=%u tspath=%256s", &enable, &u32_tp_id, rec_file) < 1) {
                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                TP_DBG_SIMPLE("record_oif_en=%d, tp_id=%d, rec_file=%s\n", enable, u32_tp_id, rec_file);
                                if(enable) {
                                        if(rec_oif_fd) {
                                                TP_DBG_SIMPLE("OIF fd exit, OIF Record Stopping...\n");
                                                filp_close(rec_oif_fd, NULL);
                                                rec_oif_fd = NULL;
                                        }
                                        rec_oif_fd = filp_open(rec_file, O_RDWR | O_CREAT | O_TRUNC, 0);

                                        if(rec_oif_fd)
                                                TP_DBG_SIMPLE("Start Rec OIF to - %s, rec_oif_fd=%p\n", rec_file, rec_oif_fd);
                                        else
                                                TP_DBG_SIMPLE("Start Rec OIF failed, open %s failed\n", rec_file);
                                } else {
                                        if(rec_oif_fd) {
                                                TP_DBG_SIMPLE("OIF Record Stopped\n");
                                                filp_close(rec_oif_fd, NULL);
                                                rec_oif_fd = NULL;
                                        }
                                }
                        }
                        break;
                }
                case TP_PROC_RECORD_BBP_EN: {
                        if (sscanf(cmd, "%d tp=%u tspath=%256s", &enable, &u32_tp_id, rec_file) < 1) {
                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                TP_DBG_SIMPLE("record_bbp_en=%d, tp_id=%d, rec_file=%s\n", enable, u32_tp_id, rec_file);
                                if(enable) {
                                        if(rec_bbp_fd) {
                                                TP_DBG_SIMPLE("BBP fd exit, BBP Record Stopping...\n");
                                                filp_close(rec_bbp_fd, NULL);
                                                rec_bbp_fd = NULL;
                                        }
                                        rec_bbp_fd = filp_open(rec_file, O_RDWR | O_CREAT | O_TRUNC, 0);

                                        if(rec_bbp_fd)
                                                TP_DBG_SIMPLE("Start Rec BBP to - %s, rec_bbp_fd=%p\n", rec_file, rec_bbp_fd);
                                        else
                                                TP_DBG_SIMPLE("Start Rec BBP failed, open %s failed\n", rec_file);
                                } else {
                                        if(rec_bbp_fd) {
                                                TP_DBG_SIMPLE("BBP Record Stopped\n");
                                                filp_close(rec_bbp_fd, NULL);
                                                rec_bbp_fd = NULL;
                                        }
                                }
                        }

                        break;
                }
                case TP_PROC_RECORD_TLV_EN: {
                        if (sscanf(cmd, "%d tp=%u tspath=%256s", &enable, &u32_tp_id,rec_file) < 1) {
                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                _tp_dump_lock();
                                TP_DBG_SIMPLE("record_tlv_en=%d, tp_id=%d, rec_file=%s\n", enable, u32_tp_id, rec_file);
                                if(enable) {
                                        if(rec_tlv_fd[u32_tp_id]) {
                                                for ( n = 0; n < MAX_TP_COUNT; n++) {
                                                        if (0 == strncmp(rec_file, tp_tlv_file_path[n], strlen(tp_tlv_file_path[n])))
                                                                break;
                                                }

                                                if(n < MAX_TP_COUNT) {
                                                        TP_ERROR("File path is same, %s\n", rec_file);
                                                        _tp_dump_unlock();
                                                        return -EEXIST;
                                                }

                                                TP_DBG_SIMPLE("TS TLV Record Stopped\n");
                                                filp_close(rec_tlv_fd[u32_tp_id], NULL);
                                                rec_tlv_fd[u32_tp_id] = NULL;
                                                memset(tp_tlv_file_path[u32_tp_id], 0, sizeof(tp_tlv_file_path[u32_tp_id]));
                                        }

                                        tmp_fd = filp_open(rec_file, O_RDWR | O_CREAT | O_TRUNC, 0);
                                        if(IS_ERR(tmp_fd)) {
                                                TP_ERROR("Start Rec TLV failed, open %s failed\n", rec_file);
                                        } else {
                                                for (n = 0; n < MAX_TP_COUNT; n++) {
                                                        if (0 == strncmp(rec_file, tp_tlv_file_path[n], strlen(rec_file)))
                                                                break;
                                                }

                                                if(n < MAX_TP_COUNT) {
                                                        TP_WARNING("File path is same, %s\n", rec_file);
                                                } else {
                                                        TP_DBG_SIMPLE("Start Rec TLV to - %s,rec_fd=%p\n", rec_file, tmp_fd);
                                                        memcpy(tp_tlv_file_path[u32_tp_id], rec_file, strlen(rec_file));
                                                        rec_tlv_fd[u32_tp_id] = tmp_fd;
                                                }
                                        }
                                } else {
                                        if(rec_tlv_fd[u32_tp_id]) {
                                                TP_DBG_SIMPLE("TLV Record Stopped\n");
                                                filp_close(rec_tlv_fd[u32_tp_id], NULL);
                                                rec_tlv_fd[u32_tp_id] = NULL;
                                                memset(tp_tlv_file_path[u32_tp_id], 0, sizeof(tp_tlv_file_path[u32_tp_id]));
                                        }
                                }
                                _tp_dump_unlock();
                        }

                        break;
                }
                case TP_PROC_RECORD_FILTER_TLV_EN: {
                        if (sscanf(cmd, "%d tp=%u tspath=%256s", &enable, &u32_tp_id,rec_file) < 1) {
                                TP_WARNING("sscanf get data failed (%s)\n", cmd);
                                return -EFAULT;
                        } else {
                                _tp_dump_lock();
                                TP_DBG_SIMPLE("record_tlv_en=%d, tp_id=%d, rec_file=%s\n", enable, u32_tp_id, rec_file);
                                if(enable) {
                                        if(rec_filter_tlv_fd[u32_tp_id]) {
                                                for ( n = 0; n < MAX_TP_COUNT; n++) {
                                                        if (0 == strncmp(rec_file, tp_filter_tlv_file_path[n], strlen(tp_filter_tlv_file_path[n])))
                                                                break;
                                                }

                                                if(n < MAX_TP_COUNT) {
                                                        TP_ERROR("File path is same, %s\n", rec_file);
                                                        _tp_dump_unlock();
                                                        return -EEXIST;
                                                }

                                                TP_DBG_SIMPLE("TS TLV Record Stopped\n");
                                                filp_close(rec_filter_tlv_fd[u32_tp_id], NULL);
                                                rec_filter_tlv_fd[u32_tp_id] = NULL;
                                                memset(tp_filter_tlv_file_path[u32_tp_id], 0, sizeof(tp_filter_tlv_file_path[u32_tp_id]));
                                        }

                                        tmp_fd = filp_open(rec_file, O_RDWR | O_CREAT | O_TRUNC, 0);
                                        if(IS_ERR(tmp_fd)) {
                                                TP_ERROR("Start Rec TLV failed, open %s failed\n", rec_file);
                                        } else {
                                                for (n = 0; n < MAX_TP_COUNT; n++) {
                                                        if (0 == strncmp(rec_file, tp_filter_tlv_file_path[n], strlen(rec_file)))
                                                                break;
                                                }

                                                if(n < MAX_TP_COUNT) {
                                                        TP_WARNING("File path is same, %s\n", rec_file);
                                                } else {
                                                        TP_DBG_SIMPLE("Start Rec TLV to - %s,rec_fd=%p\n", rec_file, tmp_fd);
                                                        memcpy(tp_filter_tlv_file_path[u32_tp_id], rec_file, strlen(rec_file));
                                                        rec_filter_tlv_fd[u32_tp_id] = tmp_fd;
                                                }
                                        }
                                } else {
                                        if(rec_filter_tlv_fd[u32_tp_id]) {
                                                TP_DBG_SIMPLE("TLV Record Stopped\n");
                                                filp_close(rec_filter_tlv_fd[u32_tp_id], NULL);
                                                rec_filter_tlv_fd[u32_tp_id] = NULL;
                                                memset(tp_filter_tlv_file_path[u32_tp_id], 0, sizeof(tp_filter_tlv_file_path[u32_tp_id]));
                                        }
                                }
                                _tp_dump_unlock();
                        }

                        break;
                }
#endif
#if 0
                default: {
                        TP_ERROR("unsupport proc command=%s\n", str);
                        return -EINVAL;
                }
#endif
        }

        return count;

}

static ssize_t tp_proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        int len = strlen(pTp_drv->proc_read_message);

        if(copy_to_user(buf, pTp_drv->proc_read_message, len)) {
                TP_WARNING("Copy proc data to user space failed\n");
                return 0;
        }

        if (*ppos == 0)
                *ppos += len;
        else
                len = 0;
        return len;
}

ssize_t tp_proc_read_monitor(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        char *str;
        int len;
        TPK_TP_ENGINE_T tp_id;
        UINT32 i;
#ifdef TP_SUPPORT_CI
        TPK_TPP_ENGINE_T tpp_id;
        tp_tp_tf_out_frmcfg_RBUS tp_tf_out_frmcfg_data;
        UINT32 pre_div, post_div, div16, tp_out_speed;
        tp_tp_tf0_p_cntl_RBUS tpp_tf_cntl_data;
        tp_tp_tf0_p_frmcfg_RBUS tpp_tf_frm_data;
        tp_tp0_p_m2m_ring_ctrl_RBUS tpp_m2m_ring_ctrl;
#endif
        tp_tp_tf0_cntl_RBUS tp_tf_cntl_data;
        tp_tp_tf0_frmcfg_RBUS tp_tf_frm_data;
        UINT64 pErrPkt, pRevPkt, pDropPkt;
        TP_BUFFER_PARAM_T buffer_param;
        UINT8 fr_status, prefix_sel;
        unsigned char *tp_source_name[] = {"TPI_0", "TPI_1", "TPI_2", "InternalDemod"};
#ifdef TP_SUPPORT_CI
        unsigned char *tp_out_source_name[] = {"memory", "TPI_0", "TPI_1", "TPI_2"};
#endif
        //unsigned char *clk_latch_name[] = {"rising", "falling"};
        unsigned char *mode_name[] = {"non-MTP", "MTP"};

        if (*ppos != 0) {
                TP_WARNING("*ppos!=0 just exit\n");
                return 0;
        }

        //TP_DBG("tp_proc_read_monitor Enter 01 *ppos=0x%llx count=0x%x\n", *ppos, count);

        len = 0;
        str = kmalloc(DUMP_DBG_MSG_BUFFER_LENGTH, GFP_KERNEL);
        if (str) {
                memset(str, 0, DUMP_DBG_MSG_BUFFER_LENGTH);

                for(tp_id = TP_TP0; tp_id < MAX_TP_COUNT; tp_id++) {
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- TP%d ---------------\n", str, tp_id);

                        Tp_GetTpCtrlReg(tp_id, (UINT32*)&tp_tf_cntl_data);
                        Tp_GetTpFrmReg(tp_id, (UINT32*)&tp_tf_frm_data);

                        pRevPkt = pTp_drv->tp[tp_id].tp_pkt_cnt.pkt;
                        pDropPkt = pTp_drv->tp[tp_id].tp_pkt_cnt.droppedPkt;
                        pErrPkt = pTp_drv->tp[tp_id].tp_pkt_cnt.errorPkt;

#ifdef TP_SUPPORT_CI
#ifdef CONFIG_RTK_TP_WITH_MIO
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sfrm_en=%d mode=%s tp_src=%s data_src_mio=%d 2bit_mode=%d pid_en=%d serial=%d ci=%d de_en=%d cnt=%llx drop=%llx Err=%llx\n", str, tp_tf_frm_data.frm_en, mode_name[tp_tf_cntl_data.mode], tp_source_name[tp_tf_cntl_data.src_sel_h << 1 | tp_tf_cntl_data.src_sel_l], tp_tf_cntl_data.src_sel_mio, tp_tf_cntl_data.two_bit, tp_tf_cntl_data.pid_en, tp_tf_frm_data.serial, tp_tf_cntl_data.ci_mode, tp_tf_cntl_data.de_en, pRevPkt, pDropPkt, pErrPkt);
#else
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sfrm_en=%d mode=%s tp_src=%s pid_en=%d serial=%d ci=%d de_en=%d cnt=%llx drop=%llx Err=%llx\n", str, tp_tf_frm_data.frm_en, mode_name[tp_tf_cntl_data.mode], tp_source_name[tp_tf_cntl_data.src_sel_h << 1 | tp_tf_cntl_data.src_sel_l], tp_tf_cntl_data.pid_en, tp_tf_frm_data.serial, tp_tf_cntl_data.ci_mode, tp_tf_cntl_data.de_en, pRevPkt, pDropPkt, pErrPkt);
#endif
#else
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sfrm_en=%d mode=%s tp_src=%s pid_en=%d serial=%d de_en=%d cnt=%llx drop=%llx Err=%llx\n", str, tp_tf_frm_data.frm_en, mode_name[tp_tf_cntl_data.mode], tp_source_name[tp_tf_cntl_data.src_sel_h << 1 | tp_tf_cntl_data.src_sel_l], tp_tf_cntl_data.pid_en, tp_tf_frm_data.serial, tp_tf_cntl_data.de_en, pRevPkt, pDropPkt, pErrPkt);
#endif

                        Tp_Get_MTPFrameControlStatus(tp_id, &fr_status);
                        Tp_Get_PrefixStatus(tp_id, &prefix_sel);

                        if(tp_id == TP_TP0) {
                                Tp_Get_Buffer(TP0_BUFFER_MASS_DATA, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[TS_DATA] B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" RR =%dKB/s RP((demux)"PT_FMT_UADDR"/(tp_sec)"PT_FMT_UADDR"/(pvr)"PT_FMT_UADDR")\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->tp_pid[tp_id].ringrate_KB, pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[DUMP_BUFFER_RP]);
                                Tp_Get_Buffer(TP0_BUFFER_INFO_PACK, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[INFO]    B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                                Tp_Get_Buffer(TP0_START_CODE_PACK, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[START]   B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                                Tp_Get_MTP_Buffer(TP_TP0_MTP, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[MTP]     B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" inited=%d FR=%d prefix_sel=%d\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->mtp[tp_id].mtp_buf_param.inited, fr_status, prefix_sel);
                        }

                        if(tp_id == TP_TP1) {
                                Tp_Get_Buffer(TP1_BUFFER_MASS_DATA, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[TS_DATA] B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" RR =%dKB/s RP((demux)"PT_FMT_UADDR"/(tp_sec)"PT_FMT_UADDR"/(pvr)"PT_FMT_UADDR")\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->tp_pid[tp_id].ringrate_KB, pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[DUMP_BUFFER_RP]);
                                Tp_Get_Buffer(TP1_BUFFER_INFO_PACK, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[INFO]    B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                                Tp_Get_Buffer(TP1_START_CODE_PACK, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[START]   B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                                Tp_Get_MTP_Buffer(TP_TP1_MTP, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[MTP]     B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" inited=%d FR=%d prefix_sel=%d\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->mtp[tp_id].mtp_buf_param.inited, fr_status, prefix_sel);
                        }

                        if(tp_id == TP_TP2) {
                                Tp_Get_Buffer(TP2_BUFFER_MASS_DATA, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[TS_DATA] B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" RR =%dKB/s RP((demux)"PT_FMT_UADDR"/(tp_sec)"PT_FMT_UADDR"/(pvr)"PT_FMT_UADDR")\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->tp_pid[tp_id].ringrate_KB, pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[DUMP_BUFFER_RP]);
                                Tp_Get_Buffer(TP2_BUFFER_INFO_PACK, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[INFO]    B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                                Tp_Get_MTP_Buffer(TP_TP2_MTP, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[MTP] 	B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" inited=%d FR=%d prefix_sel=%d\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->mtp[tp_id].mtp_buf_param.inited, fr_status, prefix_sel);
                        }
#ifdef TP_SUPPORT_TP3
                        if(tp_id == TP_TP3) {
                                Tp_Get_Buffer(TP3_BUFFER_MASS_DATA, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[TS_DATA] B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" RR =%dKB/s RP((demux)"PT_FMT_UADDR"/(tp_sec)"PT_FMT_UADDR"/(pvr)"PT_FMT_UADDR")\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->tp_pid[tp_id].ringrate_KB, pTp_drv->tp[tp_id].mass_buffer.RP_Connect[MASS_DATA_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[SECTION_BUFFER_RP], pTp_drv->tp[tp_id].mass_buffer.RP_Connect[DUMP_BUFFER_RP]);
                                Tp_Get_Buffer(TP3_BUFFER_INFO_PACK, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[INFO]    B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                                Tp_Get_MTP_Buffer(TP_TP3_MTP, &buffer_param);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[MTP]     B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR" inited=%d FR=%d prefix_sel=%d\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP, pTp_drv->mtp[tp_id].mtp_buf_param.inited, fr_status, prefix_sel);
                        }
#endif
                }

#ifdef TP_SUPPORT_CI
                for(tpp_id = TP_TPP0; tpp_id < MAX_TP_P_COUNT; tpp_id++) {
                        pRevPkt = pTp_drv->tpp[tpp_id].tpp_pkt_cnt.pkt;
                        pDropPkt = pTp_drv->tpp[tpp_id].tpp_pkt_cnt.droppedPkt;

                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- TPP%d ---------------\n", str, tpp_id);

                        Tp_TPP_GetCtrlReg(tpp_id, (UINT32*)&tpp_tf_cntl_data);
                        Tp_TPP_GetFrmReg(tpp_id, (UINT32*)&tpp_tf_frm_data);
                        Tp_TPP_GetM2MRingCtrlReg(tpp_id, (UINT32*)&tpp_m2m_ring_ctrl);
#ifdef CONFIG_RTK_TP_WITH_MIO
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sfrm_en=%d mode=%s tp_src=%s data_src_mio=%d pid_en=%d serial=%d ci=%d de_en=%d cnt=%llx drop=%llx in_go=%d go=%d\n", str, tpp_tf_frm_data.frm_en, mode_name[tpp_tf_cntl_data.mode], tp_source_name[tpp_tf_cntl_data.src_sel_h << 1 | tpp_tf_cntl_data.src_sel_l], tpp_tf_cntl_data.src_sel_mio, tpp_tf_cntl_data.pid_en, tpp_tf_frm_data.serial, tpp_tf_cntl_data.ci_mode, tpp_tf_cntl_data.de_en, pRevPkt, pDropPkt, tpp_m2m_ring_ctrl.in_go, tpp_m2m_ring_ctrl.go);
#else
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sfrm_en=%d mode=%s tp_src=%s pid_en=%d serial=%d ci=%d de_en=%d cnt=%llx drop=%llx in_go=%d go=%d\n", str, tpp_tf_frm_data.frm_en, mode_name[tpp_tf_cntl_data.mode], tp_source_name[tpp_tf_cntl_data.src_sel_h << 1 | tpp_tf_cntl_data.src_sel_l], tpp_tf_cntl_data.pid_en, tpp_tf_frm_data.serial, tpp_tf_cntl_data.ci_mode, tpp_tf_cntl_data.de_en, pRevPkt, pDropPkt, tpp_m2m_ring_ctrl.in_go, tpp_m2m_ring_ctrl.go);
#endif
                        Tp_TPP_Get_Buffer(tpp_id, &buffer_param);
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[TS_DATA] B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);

                        Tp_Get_MTP_Buffer((TPK_TP_MTP_T)tpp_id, &buffer_param);
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[MTP]     B="PT_FMT_UADDR" L="PT_FMT_UADDR" R="PT_FMT_UADDR" W="PT_FMT_UADDR"\n", str, buffer_param.Base, buffer_param.Limit, buffer_param.RP, buffer_param.WP);
                }

                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- TPOut ---------------\n", str);
                Tp_TPO_GetClkDiv(&pre_div, &post_div, &div16);
                tp_tf_out_frmcfg_data.regValue = READ_REG32(TP_TP_TF_OUT_FRMCFG_reg);
#ifdef TP_VDEC_SPEED_MHZ
                tp_out_speed = (TP_VDEC_SPEED_MHZ * 100 / (pre_div + post_div + 2));
#else
                tp_out_speed = (TP_SPEED_MHZ * 100 / (pre_div + post_div + 2));
#endif
#ifdef CONFIG_RTK_TP_WITH_MIO
                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sdata_src=%s out_2bit=%d in_tpp_out_en=%d in_tp_out_en=%d pre_div=%d post_div=%d div16=%d speed=%d.%dMhz\n", str, tp_out_source_name[tp_tf_out_frmcfg_data.data_src], tp_tf_out_frmcfg_data.in_tpp_out_en, tp_tf_out_frmcfg_data.out_2bit, tp_tf_out_frmcfg_data.in_tp_out_en, pre_div, post_div, div16, tp_out_speed / 100, tp_out_speed % 100);
#else
                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sdata_src=%s in_tpp_out_en=%d in_tp_out_en=%d pre_div=%d post_div=%d div16=%d speed=%d.%dMhz\n", str, tp_out_source_name[tp_tf_out_frmcfg_data.data_src], tp_tf_out_frmcfg_data.in_tpp_out_en, tp_tf_out_frmcfg_data.in_tp_out_en, pre_div, post_div, div16, tp_out_speed / 100, tp_out_speed % 100);
#endif
#endif
                {
                        UINT32 tp_pcra_h, tp_pcra, tp_pcrb_h, tp_pcrb;

                        tp_pcra_h  = READ_REG32(TP_TP_PCRA_CNT_HIGH_reg);
                        tp_pcra    = READ_REG32(TP_TP_PCRA_CNT_LOW_reg);
                        tp_pcrb_h  = READ_REG32(TP_TP_PCRB_CNT_HIGH_reg);
                        tp_pcrb    = READ_REG32(TP_TP_PCRB_CNT_LOW_reg);

                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- PCR ---------------\n", str);
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sTP_PCRA=0x%x/0x%x TP_PCRB=0x%x/0x%x\n",
                                      str, tp_pcra_h, tp_pcra, tp_pcrb_h, tp_pcrb);
                }

                {
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- ATSC3.0 Time Info ---------------\n", str);
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", str, pTp_drv->tp_atsc[0].pTimeInfo[0], pTp_drv->tp_atsc[0].pTimeInfo[1], pTp_drv->tp_atsc[0].pTimeInfo[2], pTp_drv->tp_atsc[0].pTimeInfo[3], pTp_drv->tp_atsc[0].pTimeInfo[4], pTp_drv->tp_atsc[0].pTimeInfo[5], pTp_drv->tp_atsc[0].pTimeInfo[6], pTp_drv->tp_atsc[0].pTimeInfo[7], pTp_drv->tp_atsc[0].pTimeInfo[8]);
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s clock_recovery=%d\n", str, pTp_drv->tp_atsc[0].clock_recovery);
                }


                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sCNTL=0x%08x CONFIG1=0x%08x CONFIG2=0x%08x CONFIG3=0x%08x\n", str, READ_REG32(TP_TP_ATSC_SRC_CNTL_reg), READ_REG32(TP_TP_ATSC_CONFIG1_reg), READ_REG32(TP_TP_ATSC_CONFIG2_reg), READ_REG32(TP_TP_ATSC_CONFIG3_reg));
                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sDATA B=0x%08x L=0x%08x W=0x%08x R=0x%08x\n", str, READ_REG32(TP_TP_ATSC_D_BASE_reg), READ_REG32(TP_TP_ATSC_D_LIMIT_reg), READ_REG32(TP_TP_ATSC_D_WP_reg), READ_REG32(TP_TP_ATSC_D_RP_reg));
                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sINFO B=0x%08x L=0x%08x W=0x%08x R=0x%08x\n", str, READ_REG32(TP_TP_ATSC_I_BASE_reg), READ_REG32(TP_TP_ATSC_I_LIMIT_reg), READ_REG32(TP_TP_ATSC_I_WP_reg), READ_REG32(TP_TP_ATSC_I_RP_reg));
                len = snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sCNT=0x%08x ERR=0x%08x DROP=0x%08x RING_CTRL=0x%08x\n", str, READ_REG32(TP_TP_ATSC_CNT_reg), READ_REG32(TP_TP_ATSC_ERR_CNT_reg), READ_REG32(TP_TP_ATSC_DRP_CNT_reg), READ_REG32(TP_TP_ATSC_RING_CTRL_reg));

                for(tp_id = TP_TP0; tp_id < MAX_TP_COUNT; tp_id++) {
                        UINT32 cancel_temp;
                        if(pTp_drv->tp_atsc[tp_id].isATSC30 == 1) {
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- TP%d ATSC ---------------\n", str, tp_id);
                                for(i = 0; i < ATSC30_PLP_MAX; i++) {
                                        if(pTp_drv->tp_atsc[tp_id].plp_cnt[i] != 0) {
                                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sPLP[%d]=%u ", str, i, pTp_drv->tp_atsc[tp_id].plp_cnt[i]);
                                        }
                                }
                                cancel_temp = pTp_drv->tp_atsc[tp_id].for_debug_atsc_cancel_bbf_time;
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sErr=%u   ", str, pTp_drv->tp_atsc[tp_id].error_cnt);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%scannel=%u reeust=%u bbp_in=%u valid_bbp_in=%u get_bbp=%u release_bbp=%u   ", str, cancel_temp, pTp_drv->tp_atsc[tp_id].for_debug_atsc_request_bbf_time - cancel_temp, pTp_drv->tp_atsc[tp_id].for_debug_first_bbf_in_time - cancel_temp, pTp_drv->tp_atsc[tp_id].for_debug_first_valid_bbf_in_time - cancel_temp, pTp_drv->tp_atsc[tp_id].for_debug_first_get_bbf_time - cancel_temp, pTp_drv->tp_atsc[tp_id].for_debug_first_release_bbf_time - cancel_temp);
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sSTC=%llu STC_Local=%llu STC_offset=%lld diff=%lld     w_idx=%u r_idx=%u  size=%u full=%u\n", str, pTp_drv->tp_atsc[tp_id].for_debug_stc_value, pTp_drv->tp_atsc[tp_id].for_debug_stc_local_value, pTp_drv->tp_atsc[tp_id].wallclock_stc_offset, pTp_drv->tp_atsc[tp_id].for_debug_stc_value - pTp_drv->tp_atsc[tp_id].for_debug_stc_local_value - pTp_drv->tp_atsc[tp_id].wallclock_stc_offset, pTp_drv->tp_atsc[tp_id].bbp_array_write_index, pTp_drv->tp_atsc[tp_id].bbp_array_read_index, pTp_drv->tp_atsc[tp_id].for_debug_bbf_total_size, pTp_drv->tp_atsc[tp_id].for_debug_ring_buffer_full_cnt);
                                len = strlen(str);
                        }
                        if(pTp_drv->tp_japan[tp_id].isJAPAN4K == 1) {
                                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------- TP%d Japan4k ---------------\n", str, tp_id);
                                //snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sw_idx=%u r_idx=%u used_max=%d size=%u full=%u\n", str, pTp_drv->tp_japan[tp_id].tlv_array_write_index, pTp_drv->tp_japan[tp_id].tlv_array_read_index, pTp_drv->tp_japan[tp_id].used_max, pTp_drv->tp_japan[tp_id].for_debug_tlv_total_size, pTp_drv->tp_japan[tp_id].for_debug_ring_buffer_full_cnt);
                                len = snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%ssize=%u full=%u\n", str, pTp_drv->tp_japan[tp_id].for_debug_tlv_total_size, pTp_drv->tp_japan[tp_id].for_debug_ring_buffer_full_cnt);
                                len = snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sr=%3d w=%3d data0=0x%x data1=0x%x\n", str, READ_REG32(TP_TLV_DESCRIPTOR_READ_reg), READ_REG32(TP_TLV_DESCRIPTOR_WRITE_reg), READ_REG32(TP_TLV_DESCRIPTOR_DATA0_reg), READ_REG32(TP_TLV_DESCRIPTOR_DATA1_reg));
                                len = strlen(str);
                        }
                }
                if(count > len) {
                        if(copy_to_user(buf, str, len)) {
                                TP_WARNING("Copy proc data to user space failed\n");
                                kfree(str);
                                return 0;
                        }

                        if (*ppos == 0)
                                *ppos += len;
                        else
                                len = 0;
                } else {
                        len = 0;
                }

                kfree(str);
        }
        //TP_DBG("tp_proc_read_monitor Exit 06 *ppos=0x%llx count=0x%x len=0x%x\n", *ppos, count, len);
        return len;
}

char *regdump_result;
unsigned long regdump_offset = 0;

ssize_t tp_proc_read_regdump(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        int len;

        if (*ppos == 0) {
                regdump_result = kmalloc(65536, GFP_KERNEL);
                if(regdump_result) {
                        memset(regdump_result, 0, 65536);
                        RHAL_DumpTPRegister(regdump_result);
                        regdump_offset = 0;
                } else {
                        regdump_result = NULL;
                        regdump_offset = 0;
                        TP_ERROR("Fail to malloc in tp_proc_read_regdump\n");
                        return 0;
                }
        }

        len = strlen(regdump_result);

        if( (len - regdump_offset) > count)
                len = count;
        else
                len = len - regdump_offset;

        if(copy_to_user(buf, regdump_result + regdump_offset , len)) {
                kfree(regdump_result);
                regdump_result = NULL;
                regdump_offset = 0;
                return 0;
        }

        *ppos += len;
        regdump_offset = regdump_offset + len;

        if(len == 0) {
                kfree(regdump_result);
                regdump_result = NULL;
                regdump_offset = 0;
        }

        //TP_DBG("tp_proc_read_monitor Exit *ppos=0x%llx count=0x%x len=0x%x\n", *ppos, count, len);
        return len;
}

ssize_t tp_proc_read_pidList(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        char *str;
        int len, i;

        if (*ppos != 0) {
                TP_WARNING("*ppos!=0 just exit\n");
                return 0;
        }

        //TP_DBG("tp_proc_read_monitor Enter 01 *ppos=0x%llx count=0x%x\n", *ppos, count);

        len = 0;
        str = kmalloc(DUMP_DBG_MSG_BUFFER_LENGTH, GFP_KERNEL);
        if (str) {
                memset(str, 0, DUMP_DBG_MSG_BUFFER_LENGTH);

                for ( i = 0 ; i < MAX_TP_COUNT ; i++ ) {
                        Tp_Cmd_Dump_PID_filter_Status(i, str);
                }

#if 0
                sprintf(str, "%s----------------------------------------------------------------------------------------------------------------------------\n", str);
                sprintf(str, "%sSection Filter\n", str);
                for ( i = 0; i < MAX_SEC_COUNT; i++) {
                        Tp_Cmd_Dump_Section_filter_Status(str, i);
                }
#endif
                len = strlen(str);

                if(count > len) {
                        if(copy_to_user(buf, str, len)) {
                                TP_WARNING("Copy proc data to user space failed\n");
                                kfree(str);
                                return 0;
                        }

                        if (*ppos == 0)
                                *ppos += len;
                        else
                                len = 0;
                } else {
                        len = 0;
                }

                kfree(str);
                //TP_DBG("tp_proc_read_monitor Exit 06 *ppos=0x%llx count=0x%x len=0x%x\n", *ppos, count, len);
        }
        return len;
}

ssize_t tp_proc_read_mutex(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
        char *str;
        int len;
        UINT32 numOfMutex = 0, numOfFun = 0;
        struct list_head * ilist = NULL, * jlist = NULL;

        if (*ppos != 0) {
                TP_WARNING("*ppos!=0 just exit\n");
                return 0;
        }

        //TP_DBG("tp_proc_read_monitor Enter 01 *ppos=0x%llx count=0x%x\n", *ppos, count);

        len = 0;
        str = kmalloc(DUMP_DBG_MSG_BUFFER_LENGTH, GFP_KERNEL);
        if (str) {
                memset(str, 0, DUMP_DBG_MSG_BUFFER_LENGTH);

                _tp_mutexlocklist_lock();

                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------------[TP Mutex Debug]--------------------\n", str);

                if (pTp_MutexLockList) {
                        list_for_each(ilist, &pTp_MutexLockList->list) {
                                TP_DRV_MUTEXLOCK_T* tmp = list_entry(ilist, TP_DRV_MUTEXLOCK_T, list);
                                if (tmp->funCallStackHead) {
                                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH,"%s========[mutex_%u][%s]========\n", str, numOfMutex, tmp->name);

                                        numOfFun = 0;
                                        list_for_each(jlist, &tmp->funCallStackHead->funName_list) {
                                                TP_DRV_MUTEXFUNNAME_T* tmpFun = list_entry(jlist, TP_DRV_MUTEXFUNNAME_T, funName_list);
                                                if (numOfFun < TP_MUTEX_DBG_NUM_FUN_CALL) {
                                                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s[%u][%s][%d]\n", str, numOfFun, tmpFun->funName, tmpFun->lock);
                                                } else {
                                                        TP_ERROR("The fun call stack is more than %d\n", TP_MUTEX_DBG_NUM_FUN_CALL);
                                                        break;
                                                }
                                                numOfFun++;
                                        }
                                }
                                numOfMutex++;
                        }
                        snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%sTotal number of Mutex in TP driver: %u\n", str, numOfMutex);
                } else
                        TP_ERROR("There is no mutex lock in TP driver.\n");

                snprintf(str, DUMP_DBG_MSG_BUFFER_LENGTH, "%s--------------------[TP Mutex Debug]--------------------\n", str);
                _tp_mutexlocklist_unlock();
                len = strlen(str);

                if(count > len) {
                        if(copy_to_user(buf, str, len)) {
                                TP_WARNING("Copy proc data to user space failed\n");
                                kfree(str);
                                return 0;
                        }

                        if (*ppos == 0)
                                *ppos += len;
                        else
                                len = 0;
                } else {
                        len = 0;
                }

                kfree(str);
        }
        //TP_DBG("tp_proc_read_monitor Exit 06 *ppos=0x%llx count=0x%x len=0x%x\n", *ppos, count, len);
        return len;
}


/***************************************************************************
  ------------------- TP driver init & un-init  --------------------
 ****************************************************************************/
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct proc_ops  tp_proc_fops = {
        .proc_write  = tp_proc_write,
        .proc_read  = tp_proc_read,
};
#else
static const struct file_operations tp_proc_fops = {
        .owner = THIS_MODULE,
        .write  = tp_proc_write,
        .read  = tp_proc_read,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct proc_ops  tp_proc_fops_monitor = {
        .proc_write  = tp_proc_write,
        .proc_read  = tp_proc_read_monitor,
};
#else
static const struct file_operations tp_proc_fops_monitor = {
        .owner = THIS_MODULE,
        .write  = tp_proc_write,
        .read  = tp_proc_read_monitor,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct proc_ops  tp_proc_fops_regdump = {
        .proc_write  = tp_proc_write,
        .proc_read  = tp_proc_read_regdump,
};
#else
static const struct file_operations tp_proc_fops_regdump = {
        .owner = THIS_MODULE,
        .write  = tp_proc_write,
        .read  = tp_proc_read_regdump,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct proc_ops  tp_proc_fops_pidList = {
        .proc_write  = tp_proc_write,
        .proc_read  = tp_proc_read_pidList,
};
#else
static const struct file_operations tp_proc_fops_pidList = {
        .owner = THIS_MODULE,
        .write  = tp_proc_write,
        .read  = tp_proc_read_pidList,
};
#endif

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0))
static struct proc_ops  tp_proc_fops_mutex = {
        .proc_write  = tp_proc_write,
        .proc_read  = tp_proc_read_mutex,
};
#else
static const struct file_operations tp_proc_fops_mutex = {
        .owner = THIS_MODULE,
        .write  = tp_proc_write,
        .read  = tp_proc_read_mutex,
};
#endif

extern struct optee_ta tp_ta;

INT32 create_tp_controller(void)
{
        TP_DRV_T* p_this = NULL;
        void *info_pack_buf = NULL;
        UADDRESS info_pack_buf_phy = 0;
        void *info_pack_buf_cached = NULL;
        UINT32 i, j, pid_cnt;
        UINT8 *section_data_buf;

        if(pTp_drv != NULL)
                return TPK_SUCCESS;

        memset(&tp_ta, 0, sizeof(struct optee_ta));

        p_this = (TP_DRV_T*)kmalloc(sizeof(TP_DRV_T), GFP_KERNEL);
        if (p_this) {
                memset(p_this, 0, sizeof(TP_DRV_T));

                /* allocate buffer for info pack */
                info_pack_buf_cached = dvr_malloc_uncached( TP_INFO_PACK_BUFFER_SIZE * MAX_TP_COUNT, &info_pack_buf );
                if ( ( info_pack_buf == NULL ) || ( info_pack_buf_cached == NULL ) ) {
                        TP_WARNING( "can not allocate memory for TP info pack (size=%d)\n", ( TP_INFO_PACK_BUFFER_SIZE * MAX_TP_COUNT ) );
                        kfree( p_this );
                        return TPK_FAIL;
                }

                p_this->info_pack_buf_cached = info_pack_buf_cached;
                info_pack_buf_phy = dvr_to_phys( info_pack_buf_cached ) ;    /* translate pack buffer from virtual & cached to phys */
                if ( info_pack_buf_phy == 0 ) {
                        TP_WARNING( "error, allocate info pack buffer fail, buffer size = %u \n", TP_INFO_PACK_BUFFER_SIZE * MAX_TP_COUNT );
                        kfree( p_this );
                        return TPK_NOT_ENOUGH_RESOURCE;
                }


                /* allocate buffer for section data (4k bytes for each pid filter) */
                section_data_buf = (UINT8*)dvr_malloc( SECTION_BUFFER_SIZE * MAX_PID_FILTER_COUNT );
                if ( section_data_buf == NULL ) {
                        TP_WARNING( "can not allocate memory for section buffer for PID filter (size=%d)\n", ( SECTION_BUFFER_SIZE * MAX_PID_FILTER_COUNT ) );
                        dvr_free( info_pack_buf_cached );
                        kfree( p_this );
                        return TPK_FAIL;
                }
                p_this->section_data_buf_of_pid_filter = section_data_buf;

                for( i = 0; i < MAX_TP_COUNT; i++) {
                        /* init info pack buffer */
                        p_this->tp[i].info_pack_buffer.BufSize = TP_INFO_PACK_BUFFER_SIZE;
                        p_this->tp[i].info_pack_buffer.BaseVir = (UADDRESS)(info_pack_buf + (TP_INFO_PACK_BUFFER_SIZE * i));
                        p_this->tp[i].info_pack_buffer.LimitVir = (UADDRESS)(info_pack_buf + (TP_INFO_PACK_BUFFER_SIZE * (i + 1)));
                        p_this->tp[i].info_pack_buffer.BasePhy = (info_pack_buf_phy + (TP_INFO_PACK_BUFFER_SIZE * i));
                        p_this->tp[i].info_pack_buffer.LimitPhy = (info_pack_buf_phy + (TP_INFO_PACK_BUFFER_SIZE * (i + 1)));
                        p_this->tp[i].info_pack_buffer.RP = p_this->tp[i].info_pack_buffer.BasePhy;
                        p_this->tp[i].info_pack_buffer.WP = p_this->tp[i].info_pack_buffer.BasePhy;
                        p_this->tp[i].info_pack_buffer.RP_Connect_cnt = INFO_PACK_BUFFER_RP_CONNECT;
                        p_this->tp[i].info_pack_buffer.RP_Connect[INFO_PACK_BUFFER_RP] = p_this->tp[i].info_pack_buffer.BasePhy;
                        p_this->tp[i].info_pack_buffer.bock_size = INFO_PACK_BUFFER_BLOCK_SIZE;
                        p_this->tp[i].info_pack_buffer.inited = 1;
                        p_this->deliveryMode[i] = TP_DELIVERY_MODE_TS; //for ecp

                        switch(i) {
                                case TP_TP0:
                                        pid_cnt = TP0_PID_FILTER_COUNT;
                                        break;
                                case TP_TP1:
                                        pid_cnt = TP1_PID_FILTER_COUNT;
                                        break;
                                case TP_TP2:
                                        pid_cnt = TP2_PID_FILTER_COUNT;
                                        break;
                                case TP_TP3:
                                        pid_cnt = TP3_PID_FILTER_COUNT;
                                        break;
                        }

                        /* init section buffer for each pid filter */
                        for( j = 0; j < pid_cnt; j++) {
                                p_this->tp_pid[i].pid_sec[j].pSec_buf = section_data_buf;
                                section_data_buf = section_data_buf + SECTION_BUFFER_SIZE;
                        }
                }

                pTp_drv = p_this;

#ifdef CONFIG_PROC_FS
                /* add proc entry */
                if((tp_proc_dir == NULL) && (tp_proc_entry == NULL) && (tp_proc_entry_monitor == NULL) && (tp_proc_entry_regdump == NULL) && (tp_proc_entry_pidList == NULL) && (tp_proc_entry_mutex == NULL)) {
                        tp_proc_dir = proc_mkdir(TP_PROC_DIR , NULL);

                        if (tp_proc_dir == NULL) {
                                TP_INFO("create rhal_tp dir proc entry (%s) failed\n", TP_PROC_DIR);
                        } else {
                                tp_proc_entry = proc_create(TP_PROC_ENTRY, 0666, tp_proc_dir, &tp_proc_fops);
                                if(tp_proc_entry == NULL) {
                                        TP_WARNING("failed to get proc entry for %s/%s \n", TP_PROC_DIR, TP_PROC_ENTRY);
                                }

                                tp_proc_entry_monitor = proc_create(TP_PROC_ENTRY_MONITOR, 0666, tp_proc_dir, &tp_proc_fops_monitor);
                                if(tp_proc_entry_monitor == NULL) {
                                        TP_WARNING("failed to get proc entry for %s/%s \n", TP_PROC_DIR, TP_PROC_ENTRY_MONITOR);
                                }

                                tp_proc_entry_regdump = proc_create(TP_PROC_ENTRY_REGDUMP, 0666, tp_proc_dir, &tp_proc_fops_regdump);
                                if(tp_proc_entry_regdump == NULL) {
                                        TP_WARNING("failed to get proc entry for %s/%s \n", TP_PROC_DIR, TP_PROC_ENTRY_REGDUMP);
                                }

                                tp_proc_entry_pidList = proc_create(TP_PROC_ENTRY_PIDLIST, 0666, tp_proc_dir, &tp_proc_fops_pidList);
                                if(tp_proc_entry_pidList == NULL) {
                                        TP_WARNING("failed to get proc entry for %s/%s \n", TP_PROC_DIR, TP_PROC_ENTRY_PIDLIST);
                                }

                                tp_proc_entry_mutex = proc_create(TP_PROC_ENTRY_MUTEX, 0666, tp_proc_dir, &tp_proc_fops_mutex);
                                if(tp_proc_entry_mutex == NULL) {
                                        TP_WARNING("failed to get proc entry for %s/%s \n", TP_PROC_DIR, TP_PROC_ENTRY_MUTEX);
                                }
                        }
                }
#else
                TP_WARNING("This module requests the kernel to support procfs,need set CONFIG_PROC_FS configure Y\n");
#endif

                return TPK_SUCCESS;
        } else {
                TP_WARNING("can not allocate memory for TP_DRV_T (structure size=%ld)\n", (unsigned long)sizeof(TP_DRV_T));
                return TPK_FAIL;
        }

}

void release_tp_controller(void)
{
        if (pTp_drv) {
#ifdef CONFIG_PROC_FS
                proc_remove(tp_proc_entry_mutex);
                tp_proc_entry_mutex = NULL;

                proc_remove(tp_proc_entry_pidList);
                tp_proc_entry_pidList = NULL;

                proc_remove(tp_proc_entry_regdump);
                tp_proc_entry_regdump = NULL;

                proc_remove(tp_proc_entry_monitor);
                tp_proc_entry_monitor = NULL;

                proc_remove(tp_proc_entry);
                tp_proc_entry = NULL;

                proc_remove(tp_proc_dir);
                tp_proc_dir = NULL;
#endif
                /* just in case, stop section thread */
                section_assemble_thread_start_stop(0);

                /* free info pack buffer */
                if(pTp_drv->info_pack_buf_cached != NULL) {
                        dvr_free((const void *)pTp_drv->info_pack_buf_cached);
                        pTp_drv->info_pack_buf_cached = NULL;
                }

                /* free section buffer of PID filter */
                if(pTp_drv->section_data_buf_of_pid_filter != NULL) {
                        dvr_free((const void *)pTp_drv->section_data_buf_of_pid_filter);
                        pTp_drv->section_data_buf_of_pid_filter = NULL;
                }

                kfree(pTp_drv);
                pTp_drv = NULL;
        }
}
EXPORT_SYMBOL(create_tp_controller);



