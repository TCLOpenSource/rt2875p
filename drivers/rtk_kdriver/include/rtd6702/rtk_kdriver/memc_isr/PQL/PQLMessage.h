#ifndef _PQL_MESSAGE_H
#define _PQL_MESSAGE_H


#ifdef __cplusplus
extern "C" {
#endif

#include "memc_isr/include/PQLAPI.h"

VOID PQL_PostMessage(const unsigned char CmdID);
VOID PQL_MessageProc(VOID);

VOID PQL_INFO_PostMessage(const unsigned char CmdID);
VOID PQL_INFO_MessageProc(VOID);

/////////////////////////////////////////////////external access to context
VOID PQL_ContextRequest0(_PQL_Request0 *pRequest0);
VOID PQL_ContextRequest1(_PQL_Request1 *pRequest1);

VOID PQL_CMD_MEMC_Module_ratio(const _PQL_MEMC_MODULE_RATIO *pCommand);

VOID PQL_CMD_MEMC_Mode_Dejudder(const unsigned char   *pCommand);
VOID PQL_CMD_MEMC_Mode_Deblur(const    unsigned char  *pCommand);
VOID PQL_CMD_MEMC_Mode_FB(const        unsigned char  *pCommand);

VOID PQL_CMD_InResolution(const  PQL_INPUT_RESOLUTION *pCommand);
VOID PQL_CMD_InFrameRate(const   PQL_INPUT_FRAME_RATE *pCommand);
VOID PQL_CMD_In3DFormat(const     PQL_INPUT_3D_FORMAT *pCommand);

VOID PQL_CMD_OutResolution(const PQL_OUTPUT_RESOLUTION *pCommand);
VOID PQL_CMD_OutFrameRate(const  PQL_OUTPUT_FRAME_RATE *pCommand);
VOID PQL_CMD_Out3DFormat(const    PQL_OUTPUT_3D_FORMAT *pCommand);

VOID PQL_CMD_OutMode(const PQL_OUT_MODE_ *pCommand);
VOID PQL_CMD_LR_Swap(const         unsigned char *pCommand);
VOID PQL_CMD_InsertBlack(const     unsigned char *pCommand);

VOID PQL_CMD_crtc_N(const     unsigned char *pCommand);
VOID PQL_CMD_crtc_M(const     unsigned char *pCommand);
VOID PQL_CMD_OsdForceMode(const     BOOL *pCommand);
VOID PQL_CMD_PullDownMode(const     unsigned char *pCommand);
VOID PQL_CMD_InputVSXral_FrameRepeatMode(const     unsigned char *pCommand);
VOID PQL_CMD_LowDelayMode(const     unsigned char *pCommand);
VOID PQL_CMD_MEMC_Mode(const     unsigned char *pCommand);
VOID PQL_CMD_PowerSaving(const     unsigned char *pCommand);

VOID PQL_INFO_SRCVO(const     unsigned char *pInfo);
VOID PQL_INFO_HDR_Chg(const     unsigned char *pInfo);
VOID PQL_INFO_Brightness(const unsigned short  *pInfo);
VOID PQL_INFO_Contrast(const unsigned short  *pInfo);
VOID PQL_INFO_Speed(const signed short  *pInfo);
VOID PQL_INFO_Zoom(const unsigned char  *pInfo);
VOID PQL_INFO_Shift(const unsigned char  *pInfo);
VOID PQL_INFO_SourceType(const unsigned short  *pInfo);
VOID PQL_INFO_VOSeek(const unsigned char  *pInfo);
VOID PQL_INFO_MEMCMode(const unsigned short  *pInfo);
VOID PQL_INFO_DarkLevel(const unsigned char  *pInfo);
VOID PQL_INFO_Hist_SC(const unsigned char  *pInfo);
VOID PQL_INFO_SoccerPatch(const unsigned char  *pInfo);
VOID PQL_INFO_triballPatch(const unsigned char  *pInfo);
VOID PQL_INFO_PictureMode_Chg(const unsigned char  *pInfo);
VOID PQL_INFO_VIP_Still(const     unsigned char *pInfo);
VOID PQL_INFO_MiraCast(const unsigned char  *pInfo);
VOID PQL_INFO_Forbes(const unsigned short  *pInfo);
VOID PQL_INFO_DTL_Step_Hist_sum_ratio(const unsigned int  *pInfo);
VOID PQL_INFO_VIP_project_ID(const unsigned char  *pInfo);

////////////////////////////////
VOID PQL_REQ_cadenceID(unsigned char *pRequest);


DBGDATA_DEFS(PQL_INPUT_RESOLUTION)
DBGDATA_DEFS(PQL_INPUT_3D_FORMAT)
DBGDATA_DEFS(PQL_INPUT_FRAME_RATE)
DBGDATA_DEFS(PQL_OUTPUT_RESOLUTION)
DBGDATA_DEFS(PQL_OUTPUT_3D_FORMAT)
DBGDATA_DEFS(PQL_OUTPUT_FRAME_RATE)
DBGDATA_DEFS(PQL_OUT_MODE_)


typedef struct
{
	unsigned char u8Dummy;
}_EXTERNAL_PARAM;

typedef struct
{
	_PQL_MEMC_MODULE_RATIO    _stc_memc_module_ratio;

	unsigned char                     u8_deblur_lvl;        // U1.4
	unsigned char                     u8_dejudder_lvl;      // U1.4
	unsigned char                     u8_flbk_lvl;          // U2.6

	PQL_INPUT_RESOLUTION      _input_resolution;
	PQL_INPUT_3D_FORMAT       _input_3dFormat;
	PQL_INPUT_FRAME_RATE      _input_frameRate;

	PQL_OUTPUT_RESOLUTION     _output_resolution;
	PQL_OUTPUT_3D_FORMAT      _output_3dFormat;
	PQL_OUTPUT_FRAME_RATE     _output_frameRate;

	PQL_OUT_MODE_             _output_mode;
	unsigned char                     u1_LR_swap;
	unsigned char                     u1_InsertBlack;
	unsigned char                     u8_crtc_m;
	unsigned char                     u8_crtc_n;
	BOOL		u1_osd_force_mode;
	unsigned char		u8_pulldown_mode;
	unsigned char		u8_InputVSXral_FrameRepeatMode;
	unsigned char		u1_lowDelayMode;
	unsigned char		u3_memcMode;
	unsigned char		u8_repeatMode;
	unsigned char		u8_repeatCase;
	BOOL		u1_powerSaving;
	BOOL		u1_ReplaceCad;
	unsigned char u8_ReplaceCase;
	BOOL		u1_ReplaceProtect;
	unsigned char u8_SpecialCase;
	/////
}_EXTERNAL_INPUT;

typedef struct
{
	BOOL                     u1_SRC_VO;
	BOOL                     u1_HDR_Chg;
	unsigned short                  u16_Brightness;
	unsigned short					u16_Contrast;
	signed short			s16_speed;
	BOOL			u1_ZoomAction;
	BOOL      		u1_ShiftAction;
	unsigned short			u16_source_type;
	unsigned char			u1_mVOSeek;
	unsigned short			u16_MEMCMode;
	unsigned char			u8_DarkLevel;
	BOOL			u1_Hist_SC_flg;
	unsigned char			u8_Soccer_holdfrm;
	BOOL			u1_PictureMode_Chg;
	BOOL			u1_VIP_Still;
	unsigned char			u1_mMiraCast;
	unsigned short			u1_Forbes_flg;
	unsigned int 			DTL_Step_Hist_sum_ratio[5];
	unsigned char			u8_VIP_Project_ID;
	BOOL					u1_402_plane;
	unsigned char			u8_triball_holdfrm;	
}_EXTERNAL_INFO;

DEBUGMODULE_BEGIN(_EXTERNAL_PARAM,_EXTERNAL_INPUT)

	ADD_PARAM(unsigned char, u8Dummy,0)

	//////////////////////////////////////////////////////
	ADD_OUTPUT(unsigned char,                 u8_deblur_lvl)
	ADD_OUTPUT(unsigned char,                 u8_dejudder_lvl)
	ADD_OUTPUT(unsigned char,                 u8_flbk_lvl)

	ADD_DUMMY_OUTPUT()

	ADD_OUTPUT(PQL_INPUT_RESOLUTION,  _input_resolution)
	ADD_OUTPUT(PQL_INPUT_3D_FORMAT,  _input_3dFormat)
	ADD_OUTPUT(PQL_INPUT_FRAME_RATE,  _input_frameRate)

	ADD_DUMMY_OUTPUT()

	ADD_OUTPUT(PQL_OUTPUT_RESOLUTION,  _output_resolution)
	ADD_OUTPUT(PQL_OUTPUT_3D_FORMAT,  _output_3dFormat)
	ADD_OUTPUT(PQL_OUTPUT_FRAME_RATE,  _output_frameRate)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(PQL_OUT_MODE_,     _output_mode)
	ADD_OUTPUT(unsigned char,             u1_LR_swap)
	ADD_OUTPUT(unsigned char,             u1_InsertBlack)

	ADD_DUMMY_OUTPUT()
	ADD_OUTPUT(unsigned char,             u8_crtc_m)
	ADD_OUTPUT(unsigned char,             u8_crtc_n)
	ADD_OUTPUT(BOOL,               u1_osd_user_mode)
	ADD_OUTPUT(unsigned char,             u8_pulldown_mode)
	ADD_OUTPUT(unsigned char,             u8_InputVSXral_FrameRepeatMode)
	ADD_OUTPUT(unsigned char,             u1_lowDelayMode)
	ADD_OUTPUT(unsigned char,             u1_powerSaving)

DEBUGMODULE_END

#ifdef __cplusplus
}
#endif


#endif
