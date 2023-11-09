#ifndef _PQLCONTEXT_H
#define _PQLCONTEXT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "memc_isr/PQL/PQLMessage.h"
#include "memc_isr/PQL/Read_ComReg.h"
#include "memc_isr/PQL/FBLevelCtrl.h"
#include "memc_isr/PQL/RimCtrl.h"
#include "memc_isr/PQL/FilmDetectCtrl.h"
#include "memc_isr/PQL/FRC_SceneAnalysis.h"
#include "memc_isr/PQL/Dh_close.h"
#include "memc_isr/PQL/Patch_Manage.h"
#include "memc_isr/PQL/Mc_lbmcSwitch.h"
#include "memc_isr/PQL/FRC_phTable.h"
#include "memc_isr/PQL/FRC_LogoDetect.h"
#include "memc_isr/PQL/MESceneAnalysis.h"
#include "memc_isr/PQL/Write_ComReg.h"
#include "memc_isr/PQL/ME_VstCtrl.h"


/*typedef enum _SLR_INPUT_PQL_INFO{
	SLR_INPUT_PQL_H_FREQ	=	0x00,
	SLR_INPUT_PQL_V_FREQ,
	SLR_INPUT_PQL_H_LEN,		// Input Horizontal Total Length
	SLR_INPUT_PQL_V_LEN,		// Input vertical Total Length
	SLR_INPUT_PQL_H_COUNT,		// Input Horizontal Count from off-line measure
	SLR_INPUT_PQL_V_COUNT,		// Input Horizontal Count from off-line measure
	SLR_INPUT_PQL_IPH_ACT_STA,
	SLR_INPUT_PQL_IPV_ACT_STA,
	SLR_INPUT_PQL_IPH_ACT_WID,
	SLR_INPUT_PQL_IPV_ACT_LEN,
	SLR_INPUT_PQL_ADC_CLOCK,
	SLR_INPUT_PQL_CHANNEL,
	SLR_INPUT_PQL_DATA_ARRAY_IDX,
	SLR_INPUT_PQL_MODE_CURR,		//unsigned char
	SLR_INPUT_PQL_MODE_TABLE_INDEX,
	SLR_INPUT_PQL_ASPECT_RATIO,
	SLR_INPUT_PQL_AFD,
	SLR_INPUT_PQL_MEM_ACT_HSTA,		// M Domain scale
	SLR_INPUT_PQL_MEM_ACT_VSTA,		// M Domain scale
	SLR_INPUT_PQL_MEM_ACT_LEN,		// M Domain scale
	SLR_INPUT_PQL_MEM_ACT_WID,		// M Domain scale
	SLR_INPUT_PQL_DISPLAY,
	SLR_INPUT_PQL_FAILURECOUNT,
	SLR_INPUT_PQL_FAILURECOUNT_NOSUPPORT,
	SLR_INPUT_PQL_FRAMESYNC,
	SLR_INPUT_PQL_CAP_LEN,		// UZD output & M Domian Input
	SLR_INPUT_PQL_CAP_WID,		// UZD output & M Domian Input
	SLR_INPUT_PQL_DISP_LEN,			// DISPLAY output
	SLR_INPUT_PQL_DISP_WID,			// DISPLAY output
	SLR_INPUT_PQL_DISP_LEN_PRE,			// Panel Size
	SLR_INPUT_PQL_DISP_WID_PRE,			// Panel Size
	SLR_INPUT_PQL_POLARITY,
	SLR_INPUT_PQL_IPV_SMARTFIT_START,
	SLR_INPUT_PQL_IPH_SMARTFIT_START,
	SLR_INPUT_PQL_IPV_ACT_LEN_PRE,
	SLR_INPUT_PQL_IPV_ACT_STA_PRE,
	SLR_INPUT_PQL_IPH_ACT_WID_PRE,
	SLR_INPUT_PQL_IPH_ACT_STA_PRE,
	SLR_INPUT_PQL_IHSYNCPULSECOUNT,
	SLR_INPUT_PQL_HSYNC,
	SLR_INPUT_PQL_VSYNC,
	SLR_INPUT_PQL_STATE,
	SLR_INPUT_PQL_NSHOWMPTYPE,
	SLR_INPUT_PQL_INPUT_SRC,
	SLR_INPUT_PQL_OSD_INPUT_SRC,
	SLR_INPUT_PQL_IPV_DLY_PRE,
	SLR_INPUT_PQL_BGCOLOR_R,
	SLR_INPUT_PQL_BGCOLOR_G,
	SLR_INPUT_PQL_BGCOLOR_B,
	SLR_INPUT_PQL_DISP_STATUS,
	SLR_INPUT_PQL_WSS,//for wss aspect ratio
	SLR_INPUT_PQL_COLOR_SPACE,
	SLR_INPUT_PQL_COLOR_DEPTH,
	SLR_INPUT_PQL_COLOR_IMETRY,
	SLR_INPUT_PQL_THROUGH_I3DDMA,
	SLR_INPUT_PQL_SR_MODE,
	SLR_INPUT_PQL_PIXEL_MODE,
	SLR_INPUT_PQL_DFRC_ENABLE,
	SLR_INPUT_PQL_DI_WID,//add this variable to control DI process width
	SLR_INPUT_PQL_DI_LEN,//add this variable to control DI process width		After Mac5P   for DI output
	SLR_INPUT_PQL_VO_SOURCE_TYPE,
	SLR_INPUT_PQL_FRAC_FS_REMOVE_RATIO,
	SLR_INPUT_PQL_FRAC_FS_MULTIPLE_RATIO,
	SLR_INPUT_PQL_DTG_MASTER_V_FREQ,
#ifdef CONFIG_SDNR_CROP
	SLR_INPUT_PQL_SDNRH_ACT_STA_PRE,
	SLR_INPUT_PQL_SDNRV_ACT_STA_PRE,
	SLR_INPUT_PQL_SDNRH_ACT_WID_PRE,
	SLR_INPUT_PQL_SDNRV_ACT_LEN_PRE,
	SLR_INPUT_PQL_SDNRH_ACT_STA,
	SLR_INPUT_PQL_SDNRV_ACT_STA,
	SLR_INPUT_PQL_SDNRH_ACT_WID,
	SLR_INPUT_PQL_SDNRV_ACT_LEN,
#endif
	SLR_INPUT_PQL_IVSYNCPULSECOUNT,
	SLR_INPUT_PQL_V_SYNC_HIGH_PERIOD,
	SLR_INPUT_PQL_VODMA_PLANE,
	SLR_INPUT_PQL_VO_SRC_FRAMERATE,
}SLR_INPUT_PQL_INFO;*/

////////////////////////////////////////////////////////////////////////
typedef struct
{
	_EXTERNAL_PARAM            _param_external;
	_PARAM_ReadComReg          _param_read_comreg;
	_PARAM_FBLevelCtrl         _param_fblevelctrl;
	_PARAM_RimCtrl             _param_rimctrl;
	_PARAM_DH_CLOSE            _param_dh_close;
	_PARAM_MC_LBMC_SWITCH      _param_mc_lbmcswitch;


	_PARAM_FRC_PH_TABLE        _param_frc_phtable;
	_PARAM_FilmDetectCtrl      _param_filmDetectctrl;
	_PARAM_FRC_LGDet           _param_FRC_LGDet;
	_PARAM_FRC_SceneAnalysis   _param_frc_sceneAnalysis;

	_PARAM_ME_SCENE_ANALYSIS   _param_me_sceneAnalysis;

	_PARAM_WRT_COM_REG         _param_wrt_comreg;

	_PARAM_PATCH_M             _param_Patch_Manage;
	_PARAM_ME_VST_CTRL         _param_me_vst_ctrl;
	_PARAM_bRMV_rFB_CTRL       _param_bRMV_rFB_ctrl;
}_PQLPARAMETER;

typedef struct
{
	_EXTERNAL_INPUT             _external_data;
	_EXTERNAL_INFO             _external_info;
	_OUTPUT_ReadComReg          _output_read_comreg;
	_OUTPUT_FBLevelCtrl         _output_fblevelctrl;
	_OUTPUT_RimCtrl             _output_rimctrl;
	_OUTPUT_DH_CLOSE            _output_dh_close;

	_OUTPUT_MC_LBMC_SWITCH      _output_mc_lbmcswitch;
	_OUTPUT_FRC_PH_TABLE        _output_frc_phtable;
	_OUTPUT_FilmDetectCtrl      _output_filmDetectctrl;
	_OUTPUT_FRC_LGDet           _output_FRC_LgDet;
	_OUTPUT_FRC_SceneAnalysis   _output_frc_sceneAnalysis;

	_OUTPUT_ME_SCENE_ANALYSIS   _output_me_sceneAnalysis;

	_OUTPUT_WRT_COM_REG         _output_wrt_comreg;
	_OUTPUT_PATCH_M             _output_Patch_Manage;
	_OUTPUT_ME_VST_CTRL         _output_me_vst_ctrl;
	_OUTPUT_bRMV_rFB_CTRL       _output_bRMV_rFB_ctrl;
}_PQLCONTEXT;

typedef enum{
	CAD_N = 0,	// no consider this cadence+timing
	CAD_T,		// support
	CAD_F,		// no support
	CAD_TYPE_NUM,
}CAD_SUPPORT_TYPE;



///////////////////////////////////////////////////////////////////////
VOID PQL_MessageInit(const _PQLPARAMETER *pParam,_PQLCONTEXT *pContext);
VOID PQL_ContextInit(VOID);
VOID PQL_CRTCStatus_Init(VOID);
VOID PQL_Init_By_ChangeSize(VOID);
VOID PQL_PhaseTableInit(VOID);
BOOL PQL_ReadHDF(unsigned int uiAddr, int ibitStart, int ibitEnd,  unsigned int *puiVal);
VOID PQL_ContextProc_oneFifth_outputInterrupt(int iSliceIdx);
VOID PQL_ContextProc_outputInterrupt_Hsync(int iSliceIdx);
VOID PQL_ContextProc_inputInterrupt(int iSliceIdx);
VOID PQL_ContextProc_outputInterrupt(int iSliceIdx);
VOID PQL_ContextProc_ReadParam(_PQLPARAMETER *pParam);
CAD_SUPPORT_TYPE PQL_FRCCaseSupportJudge(unsigned char CurCadence);
VOID Debug_Msg_Print(_PQLPARAMETER* g_pParam, _PQLCONTEXT* pContext);
VOID DebugCommandCheck(VOID);
BOOL PQL_GetMEMCActSafeStatus(VOID);
VOID Debug_OnlineMeasure(VOID);
VOID RTICEDebug_OnlineMeasure(unsigned char int_type);

const _PQLCONTEXT *GetPQLContext(VOID);
_PQLCONTEXT *GetPQLContext_m(VOID);
BOOL GetStillFrameFlag(VOID);
_PQLPARAMETER *GetPQLParameter(VOID);

DEBUGSERVER_BEGIN(_PQLPARAMETER,_PQLCONTEXT)
	ADD_MODULE_NAME_DES(_EXTERNAL_PARAM,           _param_external,           _EXTERNAL_INPUT,             _external_data,           SystemInput,    "external inputs")
	ADD_MODULE_NAME_DES(_PARAM_ReadComReg,         _param_read_comreg,        _OUTPUT_ReadComReg,          _output_read_comreg,      Read-Register,  "Read Common Registers")
	ADD_MODULE_NAME_DES(_PARAM_FBLevelCtrl,        _param_fblevelctrl,        _OUTPUT_FBLevelCtrl,         _output_fblevelctrl,      Fall-back,      "fallback level control")
	ADD_MODULE_NAME_DES(_PARAM_RimCtrl,            _param_rimctrl,            _OUTPUT_RimCtrl,             _output_rimctrl,          Rim-control,    "rim ctrl")
	ADD_MODULE_NAME_DES(_PARAM_DH_CLOSE,           _param_dh_close,           _OUTPUT_DH_CLOSE,            _output_dh_close,         Dh-close,       "dh close")
	ADD_MODULE_NAME_DES(_PARAM_MC_LBMC_SWITCH,     _param_mc_lbmcswitch,      _OUTPUT_MC_LBMC_SWITCH,      _output_mc_lbmcswitch,    LBMC-switch,    "lbmc switch")
	ADD_MODULE_NAME_DES(_PARAM_FRC_PH_TABLE,       _param_frc_phtable,        _OUTPUT_FRC_PH_TABLE,        _output_frc_phtable,      Phase-Table,    "phase table")
	ADD_MODULE_NAME_DES(_PARAM_FilmDetectCtrl,     _param_filmDetectctrl,     _OUTPUT_FilmDetectCtrl,      _output_filmDetectctrl,   Film-Detect,    "film detect control")
	ADD_MODULE_NAME_DES(_PARAM_FRC_LGDet,          _param_FRC_LGDet,          _OUTPUT_FRC_LGDet,           _output_FRC_LgDet,        Logo-Detect,    "Logo Detect control")
	ADD_MODULE_NAME_DES(_PARAM_FRC_SceneAnalysis,  _param_frc_sceneAnalysis,  _OUTPUT_FRC_SceneAnalysis,   _output_frc_sceneAnalysis,Scene-Analysis, "Scene Analysis")
	ADD_MODULE_DES(_PARAM_ME_SCENE_ANALYSIS,  _param_me_sceneAnalysis,   _OUTPUT_ME_SCENE_ANALYSIS,   _output_me_sceneAnalysis,   "ME scene analysis")
	ADD_MODULE_NAME_DES(_PARAM_WRT_COM_REG,        _param_wrt_comreg,         _OUTPUT_WRT_COM_REG,         _output_wrt_comreg,       Write-Register, "Wrt common reg")
DEBUGSERVER_END

#ifdef __cplusplus
}
#endif


#endif
