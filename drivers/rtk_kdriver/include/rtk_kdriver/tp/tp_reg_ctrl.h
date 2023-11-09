#ifndef __TP_REG_CTRL_H__
#define __TP_REG_CTRL_H__

#include <tp/tp_def.h>

#define BYTE3(x)        ((x>>24) & 0xFF)
#define BYTE2(x)        ((x>>16) & 0xFF)
#define BYTE1(x)        ((x>> 8) & 0xFF)
#define BYTE0(x)        (x       & 0xFF)

#define BUFFER_WRITE_MASK_LIMIT _BIT3
#define BUFFER_WRITE_MASK_BASE  _BIT2
#define BUFFER_WRITE_MASK_RP    _BIT1
#define BUFFER_WRITE_MASK_WP    _BIT0
#define BUFFER_WRITE_MASK_NONE  _ZERO
#define BUFFER_WRITE_MASK_ALL  (BUFFER_WRITE_MASK_LIMIT | BUFFER_WRITE_MASK_BASE | BUFFER_WRITE_MASK_RP | BUFFER_WRITE_MASK_WP)

#define BUF_SEL_MASS_DATA   _BIT0
#define BUF_SEL_INFO_PACK   _BIT1
#define BUF_SEL_SECTION     _BIT2
#define BYTE_TO_LONG(w, x, y, z) ((w<<24)+(x<<16)+(y<<8)+z)

#define PID_NO_UPDATE         0
#define PID_UPDATE_PARAM      1
#define PID_ADD               2

#define SECTION_ASM_TIMEOUT                     1       /* wait 1 ms */
#define TP_DBG_TIMEOUT                          1       /* wait 1 ms */
#define TP_INPUT_RATE_CNT_DETECT_TIMEOUT        1       /* wait 1s */

#define TPK_CW_LENGTH           8

/* TP buffer flag */
#define TP_BUFFER_MASS_DATA     0
#define TP_BUFFER_SECTION       1
#define TP_BUFFER_INFO_PACK     2
#define TP_BUFFER_CI            3


#define FORMAT_TP_INFO         0x00
#define FORMAT_AF_INFO         0x01
#define FORMAT_SEC_INFO        0x02
#define FORMAT_UNKNOWN_INFO    0xFF

#define FORMAT_PES_INFO        0x80     /* for RTD299X */
#define FORMAT_PTS_INFO        0x81     /* for RTD299X */
#define FORMAT_DTS_INFO        0x82     /* for RTD299X */

#define FORMAT_AF_INFO1        0x11     /* AF1 INFO that with High 32 bits STC */
#define FORMAT_SEC_INFO_V2     0x12     /* for RTD294X that with version 2 sec info pack */

#define SEC_CRC_CORRECT     0
#define SEC_CRC_ERR         1
#define SEC_NO_CRC_CHECK    2

#define INT_SYNC            _BIT1
#define INT_DROP            _BIT2
#define INT_OVERFLOW        _BIT3
#define INT_ADF_PCR_IN      _BIT4

#define STC_PCR_TOLERANCE 5000

#ifdef CAM_SRC_CHANGE_BASE_ON_TP_INPUT_COUNT
#define TPINPUT_MONITOR_TPID         TP_TP3
#endif

#define CHECK_MAX_TPI_COUNT(tpi)    \
{ \
        if(tpi >= MAX_TPI_COUNT) \
        { \
                TP_WARNING("bad parameter, tpi=%u\n", tpi); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_TP_COUNT(tp_id)    \
{ \
        if(tp_id >= MAX_TP_COUNT) \
        { \
                TP_WARNING("bad parameter, tp_id=%u\n", tp_id); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_TPP_COUNT(tpp_id)    \
{ \
        if(tpp_id >= MAX_TP_P_COUNT) \
        { \
                TP_WARNING("bad parameter, tpp_id=%u\n", tpp_id); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_KEY_COUNT(cnt)    \
{ \
        if(cnt >= MAX_KEY_COUNT) \
        { \
                TP_WARNING("bad parameter, key_set=%u\n",cnt); \
                return TPK_BAD_PARAMETER; \
        } \
}


#define CHECK_MTP_IS_START(id)    \
{ \
        if(pTp_drv->mtp[id].mtp_stream_status !=MTP_STREAM_START)    \
        {   \
                udelay(10);     \
                _tp_unlock();   \
                TP_WARNING("mtp_id[%u] stream did not start yet\n", id); \
                return TPK_NOT_SET; \
        }   \
}

#define CHECK_MTP_BUF_VALID(mtp_id)    \
{ \
        if(pTp_drv->mtp[mtp_id].valid != 1)   \
        {   \
                _tp_unlock();   \
                TP_WARNING("mtp_id[%u] does not set buffer yet\n", mtp_id);  \
                return TPK_NOT_SET; \
        }   \
}

#define CHECK_MTP_ID(mtp_id)    \
{ \
        if(mtp_id >= MAX_MTP_COUNT) \
        { \
                TP_WARNING("bad parameter, mtp_id=%u\n", mtp_id); \
                return TPK_BAD_PARAMETER; \
        } \
}


#define CHECK_MAX_TPOUT_COUNT(tpout_id)    \
{ \
        if(tpout_id >= MAX_TPOUT_COUNT) \
        { \
                TP_WARNING("bad parameter, tpout_id=%u\n", tpout_id); \
                return TPK_BAD_PARAMETER; \
        } \
}



#define CHECK_MAX_TP_COUNT_RT_VOID(id)    \
{ \
        if(id >= MAX_TP_COUNT) \
        { \
                TP_WARNING("bad parameter, tp_id=%u\n", id); \
                return ; \
        } \
}

#define CHECK_MAX_TP_COUNT_RT_NULL(id)    \
{ \
        if(id >= MAX_TP_COUNT) \
        { \
                TP_WARNING("bad parameter, tp_id=%u\n", id); \
                return NULL; \
        } \
}

#define CHECK_MAX_TPP_COUNT_RT_NULL(tpp_id)    \
{ \
        if(tpp_id >= MAX_TP_P_COUNT) \
        { \
                TP_WARNING("bad parameter, tpp_id=%u\n", tpp_id); \
                return NULL; \
        } \
}

#define RESET_SECTION_ASSEMBLE_BUF(P_SEC)    \
{ \
        P_SEC->sec_data_cnt   = 0;      \
        P_SEC->sec_expect_len = 0;    \
}

#define GET_BUFF_SIZE(buf_param)            (buf_param.Limit - buf_param.Base)
#define GET_BUFF_DATA_SIZE(buf_param)       ((buf_param.RP > buf_param.WP) ? (buf_param.Limit - buf_param.RP) :  (buf_param.WP - buf_param.RP))
#define GET_BUFF_DATA_ALL_SIZE(buf_param)   ((buf_param.RP > buf_param.WP) ? ((buf_param.Limit - buf_param.RP)+(buf_param.WP-buf_param.Base)) :  (buf_param.WP - buf_param.RP))
#define CHECK_RD_BUF_STATUS(result)         ((result==TPK_SUCCESS)||(result==TPK_BUFFER_FULL))
#define CHECK_RD_BUF_STATUS_ERR(result)     ((result!=TPK_SUCCESS)&&(result!=TPK_BUFFER_FULL))

#define TP_GET_TIME_CNT(time_cnt, new_t, saved_t) \
        time_cnt = (new_t.tv_sec >= saved_t.tv_sec) ? \
                   ((new_t.tv_sec - saved_t.tv_sec) * 1000) : \
                   ((new_t.tv_sec + (0xffffffff - saved_t.tv_sec)) * 1000); \
        time_cnt += ((new_t.tv_usec - saved_t.tv_usec) / 1000)

#define TP_INPUT_LOG_PRINT(tpp_id, tpi_cnt_old, tpi_cnt_new, tpo_pre, tpo_post) \
        if(tpi_cnt_new != 0) { \
                switch(pTp_drv->tpi_log_print[tpp_id]) { \
                        case TPI_LOG_RF: \
                                TP_WARNING("===== From Demod to TP, Signal of %08d Max Bps is currently being supplied =====\n", TP_SPEED_MHZ*100000/tpi_cnt_new); \
                                break; \
                        case TPI_LOG_CAM: \
                                TP_WARNING("===== Demod output : %08d Max Bps, TPO : %08d Max Bps ====\n", TP_SPEED_MHZ*100000/tpi_cnt_new, TP_SPEED_MHZ*100000/(tpo_pre+2)/(tpo_post+2)); \
                                break; \
                        case TPI_LOG_RF_CLK_CHANGE: \
                                TP_WARNING("===== From Demod to TP, Max Bps of Signal is changed from previous %08d bps to new %08d bps =====\n", TP_SPEED_MHZ*100000/tpi_cnt_old, TP_SPEED_MHZ*100000/tpi_cnt_new); \
                                TP_WARNING("===== Demod output : %08d Max Bps, TPO : %08d Max Bps ====\n", TP_SPEED_MHZ*100000/tpi_cnt_new, TP_SPEED_MHZ*100000/(tpo_pre+2)/(tpo_post+2) ); \
                                break; \
                        case TPI_LOG_NONE: \
                        default: \
                                break; \
                } \
                pTp_drv->tpi_log_print[tpp_id] = TPI_LOG_NONE; \
        }

#define CHECK_MAX_TS_COUNT(id)    \
{ \
        if(id >= MAX_TS_COUNT) \
        { \
                TP_WARNING("bad parameter, ts_id=%u\n",id); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_TP_P_COUNT(tp_p_id)    \
{ \
        if(tp_p_id >= MAX_TP_P_COUNT) \
        { \
                TP_WARNING("bad parameter, tp_p_id=%u\n", tp_p_id); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_BUFFER_COUNT(buf_idx)    \
{ \
        if(buf_idx > TP_BUFFER_MAX_COUNT) \
        { \
                TP_WARNING("bad parameter, buf_idx=%u\n", buf_idx); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_BUFFER_COUNT_RT_VOID(buf_idx)    \
{ \
        if(buf_idx > TP_BUFFER_MAX_COUNT) \
        { \
                TP_WARNING("bad parameter, buf_idx=%u\n", buf_idx); \
                return; \
        } \
}

#define CHECK_TPP_MAX_BUFFER_COUNT(buf_idx)    \
{ \
        if(buf_idx > TPP_BUFFER_MAX_COUNT) \
        { \
                TP_WARNING("bad parameter, buf_idx=%u\n", buf_idx); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_PID_FILTER_COUNT(pid_idx)    \
{ \
        if(pid_idx >= MAX_PID_FILTER_COUNT) \
        { \
                TP_WARNING("pid_idx=%u not support\n", pid_idx); \
                return TPK_BAD_PARAMETER; \
        } \
}

#define CHECK_MAX_PID_PER_FILTER_COUNT(pid_idx)    \
{ \
        if(pid_idx >= MAX_PID_PER_TP) \
        { \
                TP_WARNING("pid_idx=%u not support of per TP\n", pid_idx); \
                return TPK_BAD_PARAMETER; \
        } \
}


#define CHECK_MAX_SEC_FILTER_COUNT(sec_idx)    \
{ \
        if(sec_idx >= MAX_SEC_COUNT) \
        { \
                TP_WARNING("sec_idx (%u) over limitation (%u)\n", sec_idx, MAX_SEC_COUNT); \
                return TPK_BAD_PARAMETER; \
        } \
}


/* enum value */
typedef enum {
        RP_FLUSH_TO_BASE = 0,   /* move both RP and WP to BASE */
        RP_FLUSH_TO_WP,         /* move RP to WP */
} RP_FLUSH_MODE_T;

/* structure */
typedef struct {
        UADDRESS           Limit;
        UADDRESS           Base;
        UADDRESS           RP;
        UADDRESS           WP;
} TP_BUFFER_PARAM_T;

typedef struct {
        UINT8 format;
        UADDRESS pointer_vir;
        UADDRESS pointer_phy;
        UADDRESS tsp_hdr_vir;
        UADDRESS tsp_hdr_phy;
        UINT16 PID;
        UINT8 pid_idx_reg;
        UINT8 pid_idx;
        UINT8 section_length;   /*  sub section length */
        UINT8 tail_status;
        UINT8 header_status;
        UINT8 match_id;         /* matched section index.  it's not correct in rtd294x+ (MSB 2 bits is missing), do not use this field */
} SEC_INFO_T;



typedef struct {
        UINT8 serial: 1;                /* 0:parallel, 1:serial */
        UINT8 data_order: 1;            /* 0:MSB(bit7), 1:LSB(bit0), e.g. 0x47='b01000111 or 'b11100010 */
        UINT8 datapin: 1;               /* 0:LSB(D0), 1:LSB(D7) */

        UINT8 sync_byte;                /* default:0x47 */
        TPK_TSP_LEN_T packet_size;
} TP_INPUT_INTERFACE_PARAM_T;


typedef struct TP_REGISTER {
        char name[32];
        unsigned int address;
} TP_REGISTER;

typedef struct rtk_tlv_reg_map_t
{
    unsigned long TLV_CTRL;
    unsigned long TLV_SYNC;
    unsigned long TLV_CFG;
    unsigned long TLV_STC;
    unsigned long TLV_DESCRIPTOR;
    unsigned long TLV_DESCRIPTOR_DATA0;
    unsigned long TLV_DESCRIPTOR_DATA1;
    unsigned long TLV_DESCRIPTOR_WRITE;
    unsigned long TLV_DESCRIPTOR_READ;
    unsigned long TLV_DESCRIPTOR_FLAG;
    unsigned long TLV_DESCRIPTOR_INT;
    unsigned long TLV_DESCRIPTOR_STATUS;
    unsigned long TLV_DESCRIPTOR_TIME;
    unsigned long TLV_DESCRIPTOR_TIME_DBG;
    unsigned long TLV_DESCRIPTOR_PKT_NUM;
    unsigned long TLV_BUFFER;
    unsigned long TLV_BUFFER_FLAG;
    unsigned long TLV_BUFFER_INT;
    unsigned long TLV_BUFFER_STATUS;
    unsigned long TLV_PARSER_MAIN;
    unsigned long TLV_PARSER_DMA;
    unsigned long TLV_PKT;
    unsigned long TLV_DROP;
#ifdef CONFIG_RTK_SUPPORT_HWTLV_FILTER
    unsigned long TLV_FILTER_CFG;
    unsigned long TLV_FILTER_CFG_CTRL;
    unsigned long TLV_FILTER_CFG_EN;
    unsigned long TLV_FILTER_CFG_D0;
    unsigned long TLV_FILTER_CFG_D1;
    unsigned long TLV_FILTER_CFG_D2;
    unsigned long TLV_FILTER_CFG_D3;
    unsigned long TLV_FILTER_CFG_D4;
    unsigned long TLV_FILTER_CFG_D5;
    unsigned long TLV_FILTER_CFG_D6;
    unsigned long TLV_FILTER_CFG_D7;
    unsigned long TLV_FILTER_CFG_D8;
    unsigned long TLV_FILTER_CFG_D9;
    unsigned long TLV_FILTER_CFG_D10;
    unsigned long TLV_FILTER_CFG_D11;
    unsigned long TLV_FILTER_DROP;
    unsigned long TLV_NTP;
    unsigned long TLV_NTP_MASK;
    unsigned long TLV_NTP_CFG0;
    unsigned long TLV_NTP_CFG1;
    unsigned long TLV_NTP_CFG2;
    unsigned long TLV_NTP_CFG3;
    unsigned long TLV_NTP_CFG4;
    unsigned long TLV_NTP_STC;
    unsigned long TLV_STC_HIGH;
    unsigned long TLV_STC_LOW;
    unsigned long TLV_NTP_HIGH;
    unsigned long TLV_NTP_LOW;
    unsigned long TLV_PARSER_MAIN_ERROR;
#endif
}RTK_TLV_REG_T;

typedef struct rtk_tlv_filter_params
{
	unsigned char		flag;		//0: ignore ip address 1:ipv4 2:ipv6 3:mmtp si
	unsigned char		src_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
	unsigned char		dst_ip[16];	//ipv4: ipaddr[0-3],ipv6: ipaddr[0-15]
	unsigned short		packet_id;	//mmtp packet_id
}RTK_TLV_FILTER_PARAMS;

typedef struct rtk_tlv_NTP_params
{
	TLV_NTP_SET_T		flag;   /* 0: set port params  1: set ip params  2: port and ip */
	unsigned short		port;
	unsigned short		port_mask;
	unsigned int		ip_addr[4];
	unsigned short		ip_mask;
}RTK_TLV_NTP_FILTER_PARAMS;

/* basic function */
void Tp_Write_Data_Reg_Write(UADDRESS reg, UINT32 data);
INT32 Tp_SetTpCtrlReg(TPK_TP_ENGINE_T tp_id, UINT32 bit_sel, UINT8 data);
UINT32 BYTES_TO_LONG(UINT8* pData, UINT8 Len);


/* init function */
void Tp_Rst_Ring_Buffer_Avail(void);
void Tp_Rst_Ring_Buffer_Full(void);
void Tp_Clean_Ring_Buffer_Reg(void);
void Tp_Clean_PID_Filter_Reg(void);
void Tp_Clean_Section_Filter_Reg(void);
void Tp_Clean_Packet_Count_Reg(void);
void Tp_GlobalHardwareReset(void);
INT32 Tp_rst(TPK_TP_ENGINE_T tp_id);
INT32 Tp_init(TPK_TP_ENGINE_T tp_id);
INT32 Tp_uninit(TPK_TP_ENGINE_T tp_id);
INT32 Tp_ResetCounter(TPK_TP_ENGINE_T tp_id);
void Tp_Init_PID_Filter(TPK_TP_ENGINE_T tp_id);

/* TP ENG */
INT32 Tp_ENG_FrameEnable( TPK_TP_ENGINE_T tp_id, BOOL enable );
INT32 Tp_ENG_SetInputInterface( TPK_TP_ENGINE_T tp_id, TP_INPUT_INTERFACE_PARAM_T tp_if_param );
INT32 Tp_ENG_TwoBitsModeEnable( TPK_TP_ENGINE_T tp_id, BOOL enable );
INT32 Tp_ENG_RawModeEnable( TPK_TP_ENGINE_T tp_id, BOOL enable);
INT32 Tp_Raw_Buffer_Set(TPK_TP_ENGINE_T tp_id);
INT32 Tp_ENG_SetSyncReplace( TPK_TP_ENGINE_T tp_id, UINT8 sync_byte );

/* TP setting function */
INT32 Tp_SetInputPol(TPK_TPI_PIN_T tpi, TPK_TP_INPUT_PARAM_T pol);


/* ring buffer control */
UINT32 Tp_Buf_Get_Mass_RR_Conect(TPK_TP_ENGINE_T tp_id, UINT32 mass_flag);
INT32 Tp_Buf_Set_Mass_RR_Conect(TPK_TP_ENGINE_T tp_id, UINT8 buf_idx, UINT32 mass_flag, UINT32 u32PhyRP);
UINT8* Tp_Buf_Vir_to_Phy(TPK_TP_ENGINE_T tp_id, UINT8 buffer_flag, UINT8* pVirAddr);
UINT8* Tp_Buf_Phy_to_Vir(TPK_TP_ENGINE_T tp_id, UINT8 buffer_flag, UINT8* pPhyAddr);
INT32 Tp_Set_Buffer(UINT8 buf_idx, TP_BUFFER_PARAM_T param, UINT8 write_mask);
INT32 Tp_Get_Buffer(UINT8 buf_idx, TP_BUFFER_PARAM_T* param);
INT32 Tp_Set_Buffer_Avail_status(UINT8 buf_idx, UINT8 data);
INT32 Tp_Set_Buffer_Full_status(UINT8 buf_idx, UINT8 data);
INT32 Tp_Get_Buffer_Full_status(UINT8 buf_idx, UINT8* data);
INT32 Tp_Set_Buffer_Avail_int_En(UINT8 buf_idx, UINT8 Enable);
INT32 Tp_Set_Buffer_Avail_int_En(UINT8 buf_idx, UINT8 Enable);
INT32 Tp_flush_Buf(TPK_TP_ENGINE_T tp_id, UINT8 stop_stream_before_flush, UINT32 buffer_sel);
INT32 Tpp_Get_Buffer(UINT8 buf_idx, TP_BUFFER_PARAM_T* param);
UINT32 Tp_Check_WP_Alignment(UINT8 buf_idx, UINT32 block_size);
void Tp_Update_Buffer_RP(TPK_TP_ENGINE_T tp_id, UINT8 buf_idx, TPK_TP_BUF_T *pBuf_status);
INT32 TP_ReadData(TPK_TP_ENGINE_T tp_id, UINT8** ppReadPhyPtr, UINT32* pContinueReadSize, UINT32 Threshold, UINT8 buffer_flag);
INT32 TP_ReleaseData(TPK_TP_ENGINE_T tp_id, UINT8* pReadPhyPtr, UINT32 Size, UINT8 buffer_flag);
INT32 TP_ReadData_NoNeedAligment(TPK_TP_ENGINE_T tp_id, UINT8** ppReadPhyPtr, UINT32* pContinueReadSize, UINT32 Threshold, UINT8 buffer_flag);
INT32 TP_ReleaseData_NoNeedAligment(TPK_TP_ENGINE_T tp_id, UINT8* pReadPhyPtr, UINT32 Size, UINT8 buffer_flag);
INT32 TP_Get_TLVBufferInformation(TPK_TP_ENGINE_T tp_id, UINT32 *max_size, UINT32 *now_size);
UINT8 Tp_Get_RingBuffer_Index(TPK_TP_ENGINE_T tp_id);

/* MTP control */
INT32 Tp_SetMemoryMode(TPK_TP_ENGINE_T tp_id, UINT8 Mem_mode_en);
INT32 Tp_Get_MTP_Buffer(TPK_TP_MTP_T mtp_id, TP_BUFFER_PARAM_T* param);
INT32 TP_MTPStreamControl(TPK_TP_MTP_T mtp_id, TPK_MTP_STREAM_CTRL_T ctrl);

/* pid filter control */
INT32 Tp_SetPidParam(UINT16 pid_idx, TPK_PID_REG_PARAM_T Param);
INT32 Tp_GetPidParam(UINT16 pid_idx, TPK_PID_REG_PARAM_T* pParam);

INT32 Tp_Remove_PID(TPK_TP_ENGINE_T tp_id, UINT16 pid_idx);
INT32 Tp_AddPIDFilter( TPK_TP_ENGINE_T tp_id, TPK_PID_FILTER_PARAM_T PIDParm, void* file_handle);
INT32 Tp_RemovePIDFilter( TPK_TP_ENGINE_T tp_id, UINT16 PID,  void* file_handle);
INT32 Tp_Get_MaxPID_Cnt(TPK_TP_ENGINE_T tp_id, UINT16 *pMax_PID_Cnt);
INT32 Tp_Dump_PID_filter_Status(TPK_TP_ENGINE_T tp_id, UINT16 pid_idx);
INT32 Tp_Cmd_Dump_PID_filter_Status(TPK_TP_ENGINE_T tp_id, char *buffer);
TPK_SEC_REG_T Tp_Cmd_Dump_Section_filter_Status(char *buffer, UINT32 sec_idx);

/* section filter control */
INT32 Tp_Get_MaxSec_Cnt(TPK_TP_ENGINE_T tp_id, UINT16 *pMax_Sec_Cnt);
INT32 Tp_SetSecParam(UINT16 sec_idx, TPK_SEC_REG_T sec_reg);
INT32 Tp_GetPID_FirstSecFilter(TPK_TP_ENGINE_T tp_id, UINT16 PID, UINT16 *pSec_idx);
INT32 Tp_GetPID_LastSecFilter(TPK_TP_ENGINE_T tp_id, UINT16 PID, UINT16 *pSec_idx);
INT32 Tp_AddSecFilter(TPK_TP_ENGINE_T tp_id, TPK_SEC_FILTER_PARAM_T Param, void* file_handle, void** ppSec_handle);
INT32 Tp_RemoveSectionFilter(TPK_TP_ENGINE_T tp_id, void* pSec_handle);
INT32 section_assemble_thread_start_stop(UINT8 thread_en);
INT32 tp_dbg_thread_start_stop(UINT8 enable);
#ifdef TP_SUPPORT_CI
INT32 tp_input_rate_cnt_detect_thread_start_stop(UINT8 enable);
#endif
INT32 Tp_Dump_Section_List(TPK_TP_ENGINE_T tp_id, UINT16 pid_idx);

/* PCR tracking */
INT32 Tp_ResetPCRStatus(TPK_TP_ENGINE_T tp_id);
INT32 Tp_PCRTrackingEnable(TPK_TP_ENGINE_T tp_id, UINT8 On, UINT16 PID, TPK_PCR_CLK_SRC_T clk_src);
INT32 Tp_GetPCRTrackingStatus(TPK_TP_ENGINE_T tp_id, UINT64* pPCR, UINT64* pSTC, UINT64* pPCRBase, UINT64* pSTCBase);
INT32 Tp_UpdatePCRTrackingStatus(TPK_TP_ENGINE_T tp_id);
INT32 Tp_Get_PCR(TPK_TP_ENGINE_T tp_id, UINT32 *value_hi, UINT32 *value_low);
INT32 Tp_Set_PCR(TPK_TP_ENGINE_T tp_id, UINT32 value_hi, UINT32 value_low);

/* Descramble API */
INT32 TP_SetCW( TPK_TP_ENGINE_T tp_id, UINT8 id, UINT8 ofst, UINT32 cw_h, UINT32 cw_l);
UINT32 BYTES_TO_LONG(UINT8* pData, UINT8 Len);

/* FRC API */
INT32 Tp_FRC_Enable( TPK_TP_ENGINE_T tp_id, BOOL enable );
INT32 Tp_FRC_SetRate( TPK_TP_ENGINE_T tp_id, TPK_FRC_RATE_T rate );
INT32 Tp_FRC_SetPrefix( TPK_TP_ENGINE_T tp_id, TPK_FRC_PREFIX_T prefix_sel, UINT32 meta_data);

#ifdef TP_SUPPORT_CI
/* TPO  API */
INT32 Tp_TPO_Init(UINT8 tpout_id);
INT32 Tp_TPO_GetClkRate( INT32 * tpo_clk_rate );
INT32 Tp_TPO_SetBypassClk( UINT32 bypass_out_clk_h, UINT32 bypass_out_clk_l);
INT32 Tp_TPO_SetClkDiv( UINT32 pre_div, UINT32 post_div, UINT32 div16);
INT32 Tp_TPO_GetClkDiv(UINT32 *pre_div, UINT32 *post_div, UINT32 *div16);
INT32 Tp_TPO_SetClkByTPIRateCnt( UINT32 rate_cnt );
INT32 Tp_TPO_SetMode(UINT8 tpout_id, TPK_TPOUT_TS_PARAM_T   Param);
INT32 Tp_TPO_GetMode(UINT8 tpout_id, TPK_TPOUT_TS_PARAM_T *pParam);
INT32 Tp_TPO_Enable( UINT8 tpout_id, TPK_TPOUT_STREAM_CTRL_T ctrl);
INT32 Tp_TPO_SetDataSource(TPK_TP_SOURCE_T src_type);
#endif

INT32 Tp_atsc30_adj_f_n_code(TPK_TP_ENGINE_T tp_id, UINT32 n_code, UINT32 f_code);
INT32 Tp_Update_PIDFilter_predes( TPK_TP_ENGINE_T tp_id,  UINT8 predes_value);

INT32 Tp_is_SupportPatternSearch( TPK_TP_ENGINE_T tp_id);
INT32 Tp_SetPatternSearchPID( TPK_TP_ENGINE_T tp_id, UINT16 PID);
INT32 Tp_SetPatternSearchEnable( TPK_TP_ENGINE_T tp_id, UINT8 enable_value);
INT32 Tp_SetPatternSearchInfoBuffer( TPK_TP_ENGINE_T tp_id, UINT8* pPhyAddr, UINT8* pVirAddr, UINT32 size, UINT8* pSharedRP, UINT8* pSharedWP);
INT32 Tp_FlushPatternSearch( TPK_TP_ENGINE_T tp_id);

/* debug function */
INT32 Tp_TrackErrorPacket( TPK_TP_ENGINE_T tp_id, UINT64 *pPkt, UINT64 *pDropPkt, UINT64 *pErrPkt);
INT32 Tp_DumpTPRegister(char *str);

INT32 Tp_GetTpCtrlReg(TPK_TP_ENGINE_T tp_id, UINT32 *data);
INT32 Tp_GetTpFrmReg(TPK_TP_ENGINE_T tp_id, UINT32 *data);



UINT32 Tp_Get_MTPFrameControlStatus(TPK_TP_ENGINE_T tp_id, UINT8 *fr_status);
UINT32 Tp_Get_PrefixStatus(TPK_TP_ENGINE_T tp_id, UINT8 *prefix_sel);
INT32 Tp_SetPIDEncryption( TPK_TP_ENGINE_T tp_id, UINT16 PID, UINT8 keytype);
INT32 TP_DisplayTPFramerInformation(TPK_TP_ENGINE_T tp_id);

#ifdef TP_SUPPORT_CI
/* TPP API */
INT32 Tp_TPP_Init(TPK_TPP_ENGINE_T tpp_id);
INT32 Tp_TPP_SetCtrlReg(TPK_TPP_ENGINE_T tpp_id, UINT32 bit_sel, UINT8 data);
INT32 Tp_TPP_GetCtrlReg(TPK_TPP_ENGINE_T tpp_id, UINT32 *data);
INT32 Tp_TPP_SetFrmReg(TPK_TPP_ENGINE_T tpp_id, UINT32 data);
INT32 Tp_TPP_GetFrmReg(TPK_TPP_ENGINE_T tpp_id, UINT32 *data);
INT32 Tp_TPP_SetDataSource(TPK_TPP_ENGINE_T tpp_id, TPK_TP_SOURCE_T remux_src);
INT32 TP_TPP_StreamControl(TPK_TPP_ENGINE_T tpp_id, TPK_TPP_STREAM_CTRL_T ctrl);
INT32 Tp_TPP_OutEnable(TPK_TPP_ENGINE_T tpp_id, UINT8 on );
INT32 Tp_TPP_SetSyncReplace(TPK_TPP_ENGINE_T tpp_id, UINT8 sync_byte );
INT32 Tp_TPP_ResetCounter(TPK_TPP_ENGINE_T tpp_id);
INT32 Tp_TPP_TrackPacket( TPK_TPP_ENGINE_T tpp_id, UINT64 *pPkt, UINT64 *pDropPkt);
INT32 Tp_TPP_MassBuffInit(TPK_TPP_ENGINE_T tpp_id);
INT32 Tp_TPP_FlushBuf(TPK_TPP_ENGINE_T tpp_id);
INT32 Tp_TPP_Get_Buffer(UINT8 buf_idx, TP_BUFFER_PARAM_T* param);
INT32 Tp_Tpp_Set_Buffer(UINT8 buf_idx, TP_BUFFER_PARAM_T param, UINT8 write_mask);
void  Tp_TPP_CheckRingBuffer(void);
INT32 Tp_TPP_GetM2MRingCtrlReg(TPK_TPP_ENGINE_T tpp_id, UINT32 *data);
INT32 Tp_TPP_rst(TPK_TPP_ENGINE_T tpp_id);
void Tp_TPP_Update_Buffer_RP(TPK_TPP_ENGINE_T tpp_id, TPK_TP_BUF_T *pBuf_status);
UINT8* Tp_TPP_Buf_Phy_to_Vir(TPK_TPP_ENGINE_T tpp_id, UINT8* pPhyAddr);
UINT8* Tp_TPP_Buf_Vir_to_Phy(TPK_TPP_ENGINE_T tpp_id, UINT8* pVirAddr);
INT32 Tp_TPP_Set_Buffer_Avail_status(TPK_TPP_ENGINE_T tpp_id, UINT8 data);
INT32 Tp_TPP_Set_Buffer_Full_status(TPK_TPP_ENGINE_T tpp_id, UINT8 data);
INT32 Tp_TPP_Get_Buffer_Full_status(TPK_TPP_ENGINE_T tpp_id, UINT8* data);
INT32 Tp_TPP_Set_Buffer_Avail_int_En(TPK_TPP_ENGINE_T tpp_id, UINT8 Enable);
INT32 Tp_TPP_Set_Buffer_Full_int_En(TPK_TPP_ENGINE_T tpp_id, UINT8 Enable);
INT32 Tp_TPP_ReadData(TPK_TPP_ENGINE_T tpp_id, UINT8** ppReadPhyPtr, UINT32* pContinueReadSize, UINT32 Threshold);
INT32 Tp_TPP_ReleaseData(TPK_TPP_ENGINE_T tpp_id, UINT8* pReadPhyPtr, UINT32 Size);
#endif

#ifdef CAM_SRC_CHANGE_BASE_ON_TP_INPUT_COUNT
INT32 TP_TPInput_Monitor_Init(TPK_TP_ENGINE_T tp_id);
INT32 TP_TPInput_Monitor_Uninit(TPK_TP_ENGINE_T tp_id);
#endif

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
/* ATSC3.0 */
void Tp_atsc3_dmx_callback(void* p_ctx, unsigned char plp, unsigned char* p_data, unsigned char type, unsigned int len, unsigned char cc, unsigned char*          pTimeinfo);
INT32 Tp_ATSC30_Setting(TPK_TP_ENGINE_T tp_id);
INT32 Tp_ATSC30_StreamControl(TPK_TP_ENGINE_T tp_id, TPK_TP_ATSC_STREAM_CTRL_T ctrl);
INT32 Tp_Delivery_Set_SystemTime(TPK_TP_ENGINE_T tp_id, UINT8 current_utc_offset, UINT16 ptp_prepend, UINT8 leap59, UINT8 leap61);
INT32 TP_Get_MCM_ATSC3_Exist(void);
#endif
void  Tp_japan4k_dmx_callback(void* p_ctx, unsigned char* p_data, unsigned int len);
INT32 TP_TLV_Config(TPK_TP_ENGINE_T tp_id, RTK_TLV_REG_T *ptlv_regs);
void  TP_TLV_Reset(TPK_TP_ENGINE_T tp_id, RTK_TLV_REG_T *ptlv_regs);
void  TP_TLV_Release(RTK_TLV_REG_T *ptlv_regs);
INT32 Tp_Get_ProtectStatus(TPK_TP_ENGINE_T tp_id, UINT8 *isNeedProtect);
INT32 Tp_Ta_set_mem_prison(UINT32 startaddr, UINT32 endaddr, TP_TA_MEMORY_PRISON_TYPE_T memoryType, UINT8 isEnable);

void Tp_get_curtime(struct __kernel_old_timeval *time);
/*Mio_TP*/
INT32 TP_MIO_init(void);
INT32 TP_MIO_Enable(BOOL enable);

INT32 TP_MIO_Set_T2PMode(TPK_MIO_T2P_PARAM_T Param);
INT32 TP_MIO_Get_T2PMode(TPK_MIO_T2P_PARAM_T *Param);

INT32 TP_MIO_Set_P2TMode_(TPK_MIO_P2T_PARAM_T Param);
INT32 TP_MIO_Get_P2TMode(TPK_MIO_P2T_PARAM_T *Param);

void TP_TLV_EnableHWParseNTP(RTK_TLV_REG_T *ptlv_regs);
void TP_TLV_DisableHWParseNTP(RTK_TLV_REG_T *ptlv_regs);
void TP_TLV_SetNTPFilterParams(RTK_TLV_REG_T *ptlv_regs, RTK_TLV_NTP_FILTER_PARAMS *p_params);

void TP_TLV_FilterInit(RTK_TLV_REG_T *ptlv_regs);
void TP_TLV_FilterUnInit(RTK_TLV_REG_T *ptlv_regs);
void TP_TLV_FilterEnable(RTK_TLV_REG_T *ptlv_regs);
void TP_TLV_FilterDisable(RTK_TLV_REG_T *ptlv_regs);
void TP_TLV_SetFilterParams(RTK_TLV_REG_T *ptlv_regs, UINT8 table_idx, RTK_TLV_FILTER_PARAMS *p_filter_param);
UINT32 TP_TLV_GetCIDMappedwithIP(RTK_TLV_REG_T *ptlv_regs, UINT8 table_idx);

#endif      /*  __TP_REG_CTRL_H__ */

