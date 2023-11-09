#ifndef __TP_DRV_API_H__
#define __TP_DRV_API_H__

#include "tp_def.h"
#include "tp_reg_ctrl.h"

#define TP_KERNEL_DRIVER

/* function parameter input/output comment */
#define IN
#define OUT
#define IN_OUT

/* define return code */
#define TPK_SUCCESS                 0
#define TPK_FAIL                    -1
#define TPK_BUFFER_FULL             -2
#define TPK_BUFFER_UNALIGNMENT      -3
#define TPK_BUFFER_EMPTY            -4
#define TPK_BAD_PARAMETER           -5
#define TPK_TIMEOUT                 -6
#define TPK_NOT_ENOUGH_RESOURCE     -7
#define TPK_NOT_SUPPORT             -8
#define TPK_NOT_INIT                -9
#define TPK_RESOURCE_CONFLICT       -10
#define TPK_BUFFER_UNDER_THRESHOLD  -11
#define TPK_ERROR_HANDEL            -12
#define TPK_NOT_SET                 -13
#define TPK_DATA_OUT_OF_RANGE       -14
#define TPK_SECTION_DATA_ERROR      -15

typedef long (*REFERENCE_CLOCK_CALLBACK)(int, long, int);

/* TP driver interface */
/* TP APIs */
TP_EXTERN INT32 RHAL_TPInit(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_TPUninit(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_SetTPMode(IN TPK_TP_ENGINE_T tp_id, IN TPK_TP_TS_PARAM_T Param);
TP_EXTERN INT32 RHAL_SetTPSource(IN TPK_TP_ENGINE_T tp_id, IN TPK_TP_SOURCE_T tp_src, IN TPK_CAS_TYPE_T casType);
TP_EXTERN INT32 RHAL_SetTPSource_By_Demod(TPK_TP_ENGINE_T tp_id, TPK_TP_SOURCE_T tp_src, TPK_CAS_TYPE_T casType);
TP_EXTERN INT32 RHAL_TPFlushBuffer(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_SetTPRingBuffer(IN TPK_TP_ENGINE_T tp_id, IN UINT8* pPhyAddr, IN UINT8* pVirAddr, IN UINT32 size);
TP_EXTERN INT32 RHAL_DumpTPRingBuffer(IN TPK_TP_ENGINE_T tp_id, IN UINT8 enable, IN UINT8* pSharedRP, IN UINT8* pSharedWP);
TP_EXTERN INT32 RHAL_GetTPBufferStatus(IN TPK_TP_ENGINE_T tp_id, OUT UINT32* pBufferSize, OUT UINT32* pDataSize);
TP_EXTERN INT32 RHAL_TPStreamControl(IN TPK_TP_ENGINE_T tp_id, IN TPK_TP_STREAM_CTRL_T ctrl);
TP_EXTERN INT32 RHAL_TPReadData(IN TPK_TP_ENGINE_T tp_id, OUT UINT8** ppReadPhyPtr, OUT UINT32 * pContinueReadSize, IN UINT32 Threshold);
TP_EXTERN INT32 RHAL_TPReleaseData(IN TPK_TP_ENGINE_T tp_id, IN UINT8* pReadPhyPtr, IN UINT32 Size);
TP_EXTERN INT32 RHAL_SetInputSourcePol(IN TPK_TPI_PIN_T tpi, IN TPK_TP_INPUT_PARAM_T pol);

/* TPOUT APIs */
TP_EXTERN INT32 RHAL_TPOUT_SetMode(IN UINT8 tpout_id, IN TPK_TPOUT_TS_PARAM_T Param);
TP_EXTERN INT32 RHAL_TPOUT_GetMode(IN UINT8 tpout_id, IN TPK_TPOUT_TS_PARAM_T *pParam);
TP_EXTERN INT32 RHAL_TPOUT_GetStatus(IN UINT8 tpout_id, IN TPK_TPOUT_STATUS_T* pTpoutStatus);
TP_EXTERN INT32 RHAL_TPOUT_Enable(IN UINT8 tpout_id, IN TPK_TPOUT_STREAM_CTRL_T ctrl);
TP_EXTERN INT32 RHAL_TPOUT_SetDataSource(IN UINT8 tpout_id, IN TPK_TPOUT_SOURCE_T tpout_src);
TP_EXTERN INT32 RHAL_TPOUT_SetBypassClk(UINT8 tpout_id, UINT32 bypass_out_clk_h, UINT32 bypass_out_clk_l);
TP_EXTERN INT32 RHAL_TPOUT_SetClockSpeed(IN UINT8 tpout_id, IN UINT32 pre_div, IN UINT32 post_div, IN UINT32 div16);
TP_EXTERN INT32 RHAL_TPOUT_GetClockSpeed(IN UINT8 tpout_id, IN UINT32 *ppre_div, IN UINT32 *ppost_div, IN UINT32 *pdiv16);

/* TPP APIs */
TP_EXTERN INT32 RHAL_TPP_Init(IN TPK_TPP_ENGINE_T tpp_id);
TP_EXTERN INT32 RHAL_TPP_Uninit(IN TPK_TPP_ENGINE_T tpp_id);
TP_EXTERN INT32 RHAL_TPP_SetMode(IN TPK_TPP_ENGINE_T tpp_id, IN TPK_TPP_TS_PARAM_T Param);
TP_EXTERN INT32 RHAL_TPP_GetMode(IN TPK_TPP_ENGINE_T tpp_id, OUT TPK_TPP_TS_PARAM_T *pParam);
TP_EXTERN INT32 RHAL_TPP_EnablePIDFilter(IN TPK_TPP_ENGINE_T tpp_id, IN UINT8 on);
TP_EXTERN INT32 RHAL_TPP_StreamControl(IN TPK_TPP_ENGINE_T tpp_id, IN TPK_TPP_STREAM_CTRL_T ctrl);
TP_EXTERN INT32 RHAL_TPP_OutEnable(IN TPK_TPP_ENGINE_T tpp_id,IN UINT8 enable);
TP_EXTERN INT32 RHAL_TPP_SetSyncReplace(IN TPK_TPP_ENGINE_T tpp_id, IN UINT8 sync_byte);
TP_EXTERN INT32 RHAL_TPP_SetDataSource(IN TPK_TPP_ENGINE_T tpp_id, IN TPK_TP_SOURCE_T tpp_src);
TP_EXTERN INT32 RHAL_TPP_GetDataSource(IN TPK_TPP_ENGINE_T tpp_id);
TP_EXTERN INT32 RHAL_TPP_FlushBuffer(IN TPK_TPP_ENGINE_T tpp_id);
TP_EXTERN INT32 RHAL_TPP_GetBufferStatus(IN TPK_TPP_ENGINE_T tpp_id, OUT UINT32 * pBufferSize, OUT UINT32 * pDataSize);
TP_EXTERN INT32  RHAL_TPP_Delivery_Data(UINT8 id, char *buffer, UINT32 *buffer_size);
TP_EXTERN INT32  RHAL_Delivery_TPP_SetInputConfig(UINT8 id, TPK_INPUT_PORT_T          inputPort, TPK_INPUT_TYPE_T inputType);
/* MTP APIs */
TP_EXTERN INT32 RHAL_SetMTPBuffer(IN TPK_TP_MTP_T mtp_id, IN UINT8* pPhyAddr, IN UINT8* pVirAddr, IN UINT32 size);
TP_EXTERN INT32 RHAL_WriteMTPBuffer(IN TPK_TP_MTP_T mtp_id, IN UINT8* pPhyWP, IN UINT32 ContinueWrite);
TP_EXTERN INT32 RHAL_GetMTPBufferStatus(IN TPK_TP_MTP_T mtp_id, OUT UINT8** ppWritePhyPtr, OUT UINT32* pContinueWriteSize, OUT UINT32* pWritableSize);
TP_EXTERN INT32 RHAL_MTPStreamControl(IN TPK_TP_MTP_T mtp_id, IN TPK_MTP_STREAM_CTRL_T ctrl);
TP_EXTERN INT32 RHAL_MTPFlushBuffer(IN TPK_TP_MTP_T mtp_id, IN MTP_BUFF_FLUSH_MODE_T mode);
TP_EXTERN INT32 RHAL_MTPFrameControl(IN TPK_TP_MTP_T mtp_id, IN UINT8 enable);
TP_EXTERN INT32 RHAL_MTP_TPOutEnable(IN TPK_TP_MTP_T mtp_id, IN UINT8 enable);

/* PID filter APIs */
TP_EXTERN INT32 RHAL_AddPIDFilter(IN TPK_TP_ENGINE_T tp_id, IN TPK_PID_FILTER_PARAM_T PIDParm, IN void* file_handle);
TP_EXTERN INT32 RHAL_RemovePIDFilter(IN TPK_TP_ENGINE_T tp_id, IN UINT16 PID, IN void* file_handle);
/* For RHAL_RemoveAllPIDFilters(), if file_handle=REGARDLESS_OF_FILE_HANDLE, it will remove all of PID filter regardless of file_handle control */
TP_EXTERN INT32  RHAL_RemoveAllPIDFilters(IN TPK_TP_ENGINE_T tp_id, IN void* file_handle);
TP_EXTERN INT32 RHAL_GetPIDFilterStatus(IN TPK_TP_ENGINE_T tp_id, IN UINT16 PID_Buf_Cnt, OUT TPK_PID_FILTER_PARAM_T* pPIDParm, OUT UINT16* pPIDLen);
TP_EXTERN INT32 RHAL_EnablePIDFilter(IN TPK_TP_ENGINE_T tp_id, IN UINT8 on);


/* Section filter APIs */
TP_EXTERN INT32 RHAL_AddSectionFilter(IN TPK_TP_ENGINE_T tp_id, IN  TPK_SEC_FILTER_PARAM_T Param, IN void* file_handle, OUT void** ppSec_handle);
TP_EXTERN INT32 RHAL_RemoveSectionFilter(IN TPK_TP_ENGINE_T tp_id, IN void* pSec_handle);
/* For RHAL_RemoveAllSectionFilters(), if file_handle=REGARDLESS_OF_FILE_HANDLE, it will remove all of Section filter regardless of file_handle control */
TP_EXTERN INT32 RHAL_RemoveAllSectionFilters(IN TPK_TP_ENGINE_T tp_id, IN void* file_handle);
TP_EXTERN INT32 RHAL_GetSectionFilterStatus(IN TPK_TP_ENGINE_T tp_id, IN UINT16 Sec_Buf_cnt, OUT TPK_SEC_FILTER_STATUS_T* pSecStatus, OUT UINT16* usSecLen);

/* PCR tracking */
/* tracking disable:  MISC_90KHz */
/* tracking enable:  TP_LOCAL_90KHz */
TP_EXTERN INT32 RHAL_PCRTrackingEnable(IN TPK_TP_ENGINE_T tp_id, IN UINT8 On, IN UINT16 PID, IN TPK_PCR_CLK_SRC_T clk_src);
TP_EXTERN INT32 RHAL_GetPCRTrackingStatus(IN TPK_TP_ENGINE_T tp_id, OUT UINT64* pPCR, OUT UINT64* pSTC, OUT UINT64* pPCRBase, OUT UINT64* pSTCBase);
TP_EXTERN INT32 RHAL_ResetPCRTrackingStatus(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_GetPCRAdaptionField(IN TPK_TP_ENGINE_T tp_id,IN UINT16 PID,OUT UINT8 *pData,OUT UINT8 *pDataSize);

/* Descrambler APIs */
TP_EXTERN INT32 RHAL_SetDescrambleAlgorithm(IN TPK_TP_ENGINE_T tp_id, IN TPK_DESCRAMBLE_ALGORITHM_T algo);

/* Start Code Search */
TP_EXTERN INT32 RHAL_SetTPPatternSearchInfoBuffer(IN UINT8 tp_id, IN UINT8* pPhyAddr, IN UINT8* pVirAddr, IN UINT32 size, IN UINT8* pSharedRP, IN UINT8* pSharedWP);
TP_EXTERN INT32 RHAL_SetTPPatternSearchPID(IN UINT8 tp_id, IN UINT16 PID);

/* Debug */
INT32 RHAL_DumpTPRegister(char *str);

/******************************************************
   Multi2 set key flow:
   1. set system key:
   (1) key_id = TP_SETCW_KEYID_MULTI2_SYS_KEY
   (2) system key field into Key[TPK_MAX_KEY_LENGTH] (32 bytes)
   RHAL_SetCW(tp_id, KeySel, TP_SETCW_KEYID_MULTI2_SYS_KEY, system_key,dummy_IV); --  IV don't care, but please don't input NULL pointer

   2. set "CBC  key+even" key
   (1) key_id = TP_SETCW_KEYID_EVEN_KEY
   (2) CBC  key field into IV[TPK_MAX_IV_LENGTH] (8 bytes)
   (3) even key field into Key[TPK_MAX_KEY_LENGTH] (8 byte)
   RHAL_SetCW(tp_id, KeySel, TP_SETCW_KEYID_EVEN_KEY,even_key,cbc_key);

   3. set "CBC  key+odd" key
   (1) key_id = TP_SETCW_KEYID_ODD_KEY
   (2) CBC  key field into IV[TPK_MAX_IV_LENGTH] (8 bytes)
   (3) odd key field into Key[TPK_MAX_KEY_LENGTH] (8 byte)
   RHAL_SetCW(tp_id, KeySel, TP_SETCW_KEYID_ODD_KEY,odd_key,cbc_key);

   Other descramble algorithm set key flow:
   1. set even key + IV
   (1) key_id = TP_SETCW_KEYID_EVEN_KEY
   (2) even IV field into IV[TPK_MAX_IV_LENGTH] (maximun 16 bytes)
   (3) even key field into Key[TPK_MAX_KEY_LENGTH] (maximun 32 bytes)
   RHAL_SetCW(tp_id, KeySel, TP_SETCW_KEYID_EVEN_KEY, even_key,even_iv);

   2. set odd key + IV
   (1) key_id = TP_SETCW_KEYID_ODD_KEY
   (2) odd IV field into IV[TPK_MAX_IV_LENGTH] (maximun 16 bytes)
   (3) odd key field into Key[TPK_MAX_KEY_LENGTH] (maximun 32 bytes)
   RHAL_SetCW(tp_id, KeySel, TP_SETCW_KEYID_ODD_KEY, even_key,even_iv);
******************************************************/


/*      Parm : key_set   : key set */
/*             key_id    : key id (0: even key and iv, 1 : odd key & iv, 2 : system key (for multi 2 only)) */
/*             Key       : key value */
/*             IV        : intial value */
TP_EXTERN INT32 RHAL_SetCW(IN TPK_TP_ENGINE_T tp_id, IN UINT8 KeySel, IN UINT8 key_id, IN UINT8 Key[TPK_MAX_KEY_LENGTH], IN UINT8 IV[TPK_MAX_IV_LENGTH]);
TP_EXTERN INT32 RHAL_DescrambleControl(IN TPK_TP_ENGINE_T tp_id, IN TPK_DESCRAMBLE_MODE_T mode);

/* get debug message */
TP_EXTERN INT32 RHAL_TrackErrorPacket(IN TPK_TP_ENGINE_T tp_id, OUT UINT32 *pErrPkt, OUT UINT32 *pRevPkt, OUT UINT32 *pDropPkt);
TP_EXTERN INT32 RHAL_TPDbgRecTSData(IN TPK_TP_ENGINE_T tp_id, IN UINT8 enable, IN UINT8* path);
TP_EXTERN INT32 RHAL_DisplayTPFramerInformation(TPK_TP_ENGINE_T tp_id);

/* hardware info and status for AP */
TP_EXTERN INT32 RHAL_GetHardwareInfo(OUT TPK_HARDWARE_INFO_T *pTpHardwareInfo);
TP_EXTERN INT32 RHAL_GetTpStatus(IN TPK_TP_ENGINE_T tp_id, OUT TPK_TP_STATUS_T *pTpStatus);
TP_EXTERN INT32 RHAL_GetMtpStatus(IN UINT8 mtp_id, OUT TPK_MTP_STATUS_T *pMtpStatus);
TP_EXTERN INT32 RHAL_GetTpoutStatus(IN UINT8 tpout_id, OUT TPK_TPOUT_STATUS_T *pTpoutStatus);

/* Extend function */
TP_EXTERN INT32 RHAL_GetTPSource(TPK_INPUT_PORT_T portType, TPK_CAS_TYPE_T casType);
TP_EXTERN INT32 RHAL_SetTPHardwareReset(UINT8 restore_value);
TP_EXTERN INT32 RHAL_SetPIDDescrambleAlgorithm(UINT8 key_sel, TPK_DESCRAMBLE_ALGORITHM_T     algo);
TP_EXTERN INT32 RHAL_FreePIDDescrambleAlgorithm(void);

/* CI1.4 download buffer function */
TP_EXTERN INT32 RHAL_CIP_SetBufferRP(IN TPK_TP_ENGINE_T tp_id, IN UINT32 u32PhyRP);
TP_EXTERN INT32 RHAL_CIP_GetBufferStatus(IN TPK_TP_ENGINE_T tp_id, OUT UINT32 *pPhyBase, OUT UINT32 *pPhyLimit, OUT UINT32 *pPhyRead, OUT UINT32 *pPhyWrite);
TP_EXTERN INT32 RHAL_CIP_GetCAMBitRate(OUT UINT32 * inputBitRate, OUT UINT32 * outputBitRate);
//TP_EXTERN INT32 RHAL_SetTpSyncCP14(IN TPK_TP_ENGINE_T tp_id, IN  UINT8 sync_byte_cip14);
TP_EXTERN INT32 RHAL_CIP_ReadData(IN TPK_TP_ENGINE_T tp_id, OUT UINT8** ppReadPhyPtr, OUT UINT32 * pContinueReadSize, IN UINT32 Threshold,OUT long* phyAddrOffset);
TP_EXTERN INT32 RHAL_CIP_ReleaseData(IN TPK_TP_ENGINE_T tp_id, IN UINT8* pReadPhyPtr, IN UINT32 Size);

#if defined(CONFIG_RTK_KDRV_ATSC3_DMX)
TP_EXTERN INT32 RHAL_TPRawModeEnable(TPK_TP_ENGINE_T tp_id, TPK_TP_RAW_CTRL_T enable);
TP_EXTERN INT32 RHAL_ATSC30_Init(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_ATSC30_UnInit(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_ATSC30_Open(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_ATSC30_Close(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_ATSC30_RequestBBFrame(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_ATSC30_CancelBBFrame(IN TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_ATSC30_GetBBFrame(IN TPK_TP_ENGINE_T tp_id, OUT UINT32 *bbframe_address, OUT UINT32 *count);
TP_EXTERN INT32 RHAL_ATSC30_ReturnBBFrame(IN TPK_TP_ENGINE_T tp_id, IN UINT32 bbframe_address);
TP_EXTERN INT32 RHAL_ATSC30_GetBBFrameInfo(IN TPK_TP_ENGINE_T tp_id, OUT UINT32 *buff_phy, OUT UINT32 *buff_size);
TP_EXTERN INT32 RHAL_ATSC30_GetTimeInfo(TPK_TP_ENGINE_T tp_id, UINT32 *second, UINT32 *n_second, UINT8 *wall_clock);
TP_EXTERN INT32 RHAL_ATSC30_SetTimeInfo(TPK_TP_ENGINE_T tp_id, UINT32 second, UINT32 n_second, UINT8 wall_clock);
TP_EXTERN INT32 RHAL_ATSC30_SetSystemTimeInfo(TPK_TP_ENGINE_T tp_id, UINT8 current_utc_offset, UINT16 ptp_prepend, UINT8 leap59, UINT8 leap61);
TP_EXTERN INT32 RHAL_ATSC30_SetClockRecovery(TPK_TP_ENGINE_T tp_id, UINT8 isNeedClockRecovery);
TP_EXTERN INT32 RHAL_Delivery_ATSC30_SetInputConfig(TPK_TP_ENGINE_T tp_id, TPK_INPUT_PORT_T  inputPort, TPK_INPUT_TYPE_T  inputType);
#endif

TP_EXTERN void  RHAL_Delivery_SetMode(TPK_TP_ENGINE_T tp_id, UINT8 mode);
TP_EXTERN UINT16 RHAL_Delivery_GetSourceType(TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_Delivery_RequestData(TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_Delivery_CancelData(TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_Delivery_GetData(TPK_TP_ENGINE_T tp_id, char *buffer, UINT32 *buffer_size);
TP_EXTERN INT32 RHAL_JAPAN4K_Open(TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_JAPAN4K_Close(TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_Delivery_JAPAN4K_SetInputConfig(TPK_TP_ENGINE_T tp_id, TPK_INPUT_PORT_T  inputPort, TPK_INPUT_TYPE_T  inputType, TPK_DELIVERY_SRC_TYPE_T srcType);
TP_EXTERN INT32 RHAL_Delivery_ReadDataFromHWTLV(struct japan4k_dmx* p_this);

TP_EXTERN INT32 RHAL_Delivery_TS_SetInputConfig(TPK_TP_ENGINE_T tp_id, TPK_INPUT_PORT_T  inputPort, TPK_INPUT_TYPE_T  inputType, TPK_DELIVERY_SRC_TYPE_T srcType);

TP_EXTERN INT32 RHAL_Delivery_HWTLV_EnableFilter(RTK_TLV_REG_T *ptlv_regs);
TP_EXTERN INT32 RHAL_Delivery_HWTLV_DisableFilter(RTK_TLV_REG_T *ptlv_regs);
TP_EXTERN INT32 RHAL_Delivery_HWTLV_AddFilter(RTK_TLV_REG_T *ptlv_regs, UINT8 filter_idx, JP4K_DELIVERY_FILTER_T *p_filter_param);
TP_EXTERN INT32 RHAL_Delivery_HWTLV_RemoveFilter(RTK_TLV_REG_T *ptlv_regs, UINT8 filter_idx);
TP_EXTERN UINT8 RHAL_Delivery_GetValidFilterTableIdx(UINT32 tp_id);
TP_EXTERN void  RHAL_Delivery_ReturnFilterTableIdx(UINT32 tp_id, UINT8 idx);
TP_EXTERN INT32 RHAL_Delivery_HWTLV_GetNTP(RTK_TLV_REG_T *ptlv_regs, UINT32 *ntp_high, UINT32 *ntp_low);
TP_EXTERN INT32 RHAL_Delivery_HWTLV_GetSTC(RTK_TLV_REG_T *ptlv_regs, UINT32 *stc_high, UINT32 *stc_low);

TP_EXTERN UINT8 RHAL_Delivery_GetMode(TPK_TP_ENGINE_T tp_id);
TP_EXTERN INT32 RHAL_TP_EnableMemProtect(TPK_TP_ENGINE_T tp_id, TP_TA_MEMORY_PRISON_TYPE_T mem_type, UINT32 start_address, UINT32 stop_address);
TP_EXTERN INT32 RHAL_TP_DisableMemProtect(TPK_TP_ENGINE_T tp_id, TP_TA_MEMORY_PRISON_TYPE_T mem_type);
TP_EXTERN INT32 RHAL_TP_CheckProtectStatus(TPK_TP_ENGINE_T tp_id);

TP_EXTERN void RHAL_SetTPRefClockMode(REFERENCE_CLOCK_CALLBACK callback);

INT32 RHAL_TP_IsJapan4K(void);
INT32 RHAL_TP_IsATSC30(void);

INT32 RHAL_setGetTPPcr90KCnt(UINT32 pcr_type, UINT32 rOrw_flag, UINT64 *cnt);
INT32 RHAL_adjustStcClock(UINT32 stc_type, UINT32 fcode, UINT32 ncode);

/*MIO TP*/
INT32 RHAL_MIO_SetIOMode(UINT32 mode);

//nagra support
INT32 RHAL_SetNagxx_PID_update_bit(TPK_TP_ENGINE_T tp_id, BOOL enable);
INT32 RHAL_Tp_GetNagxx_param(TPK_NAGXX_Conrol_Data *nag_ctl_data);


#endif /* __TP_DRV_API_H__ */

