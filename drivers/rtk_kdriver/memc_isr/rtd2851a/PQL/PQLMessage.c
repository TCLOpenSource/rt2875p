
#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/include/PQLAPI.h"
#include "memc_isr/PQL/PQLContext.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
//message slice processing
#define _CMDQUEUE_NUM  20
#define _INFOQUEUE_NUM  6
static unsigned char  s_CmdQueue[_CMDQUEUE_NUM];
static unsigned int s_CurCmdIdx;
static unsigned char  s_InfoQueue[_INFOQUEUE_NUM];
static unsigned int s_CurInfoIdx;

static const _PQLPARAMETER *s_pParam;
static _PQLCONTEXT *s_pContext;

VOID PQL_MessageInit(const _PQLPARAMETER *pParam,_PQLCONTEXT *pContext)
{
	int i;
	s_CurCmdIdx = _CMDQUEUE_NUM-1;

	for (i = 0; i < _CMDQUEUE_NUM; i++)
	{
		s_CmdQueue[i] = PQL_CMD_VOID;
	}

	s_pParam = pParam;
	s_pContext = pContext;

	//pContext->_external_data.u8_deblur_lvl             = 64;
	//pContext->_external_data.u8_dejudder_lvl           = 128;
	//pContext->_external_data.u8_flbk_lvl               = 64;
	pContext->_external_data.u8_InputVSXral_FrameRepeatMode = 0;
	pContext->_external_data.u1_powerSaving = 0;

	pContext->_external_info.s16_speed = 256;
	pContext->_external_info.u1_mVOSeek = 0;
	pContext->_external_info.u1_mMiraCast = 0;
	pContext->_external_info.u1_402_plane = 0;
}

VOID PQL_PostMessage(const unsigned char CmdID)
{
	//Avoid sequential identical commands
	if (CmdID != s_CmdQueue[s_CurCmdIdx])
	{
		if (s_CurCmdIdx == 0)
		{
			s_CurCmdIdx = _CMDQUEUE_NUM-1;
		}else
		{
			s_CurCmdIdx--;
		}

		s_CmdQueue[s_CurCmdIdx] = CmdID;
	}
}

VOID PQL_INFO_PostMessage(const unsigned char InfoID)
{
	//Avoid sequential identical commands
	if (InfoID != s_InfoQueue[s_CurInfoIdx])
	{
		if (s_CurInfoIdx == 0)
		{
			s_CurInfoIdx = _INFOQUEUE_NUM-1;
		}else
		{
			s_CurInfoIdx--;
		}

		s_InfoQueue[s_CurInfoIdx] = InfoID;
	}
}

VOID PQL_MessageProc(VOID)
{
	while(s_CmdQueue[s_CurCmdIdx] != PQL_CMD_VOID)
	{
		switch(s_CmdQueue[s_CurCmdIdx])
		{
		case  PQL_CMD_MEMC_MODE_DEJUDDER:  //
			break;

		case PQL_CMD_MEMC_MODE_DEBLUR:   //
			break;

		case PQL_CMD_MEMC_MODE_FB:   //
			break;

		case PQL_CMD_INPUT_RESOLUTION:   //
			break;

		case PQL_CMD_INPUT_FRAME_RATE:   //
			break;

		case PQL_CMD_INPUT_FORMAT:   //
			//
			break;
		case PQL_CMD_OUTPUT_RESOLUTION:   //
			break;

		case PQL_CMD_OUTPUT_FRAME_RATE:   //
			break;

		case PQL_CMD_OUTPUT_FORMAT:   //
			break;

		case PQL_CMD_OSD_FORCE_MODE:   //
			break;

		case PQL_CMD_PULLDOWNMODE:   //
			break;

		case PQL_CMD_INPUTVSXRAL_FRAMEREPEAT_MODE:   //
			break;

		case PQL_CMD_LOWDELAYMODE:   //
			break;
		}

		s_CmdQueue[s_CurCmdIdx] = PQL_CMD_VOID;
		s_CurCmdIdx             = (s_CurCmdIdx+1)%_CMDQUEUE_NUM;
	}
}

VOID PQL_INFO_MessageProc(VOID)
{
	while(s_InfoQueue[s_CurInfoIdx] != PQL_INFO_VOID)
	{
		switch(s_InfoQueue[s_CurInfoIdx])
		{
		case  PQL_INFO_SRC_VO:  //
			break;
		}

		s_InfoQueue[s_CurInfoIdx] = PQL_INFO_VOID;
		s_CurInfoIdx             = (s_CurInfoIdx+1)%_INFOQUEUE_NUM;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
//external commands
VOID PQL_CMD_MEMC_Module_ratio(const _PQL_MEMC_MODULE_RATIO *pCommand)
{
	if(pCommand)
	{
		_EXTERNAL_INPUT  *pExtData       = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_stc_memc_module_ratio = *pCommand;
	}
}

VOID PQL_CMD_MEMC_Mode_Dejudder(const unsigned char  *pCommand)
{
	//LogPrintf(DBG_MSG, "dejudder level = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u8_dejudder_lvl = (unsigned char)*pCommand;
	}
}
VOID PQL_CMD_MEMC_Mode_Deblur(const unsigned char  *pCommand)
{
	//LogPrintf(DBG_MSG, "deblur level = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u8_deblur_lvl   = (unsigned char)*pCommand;
	}
}
VOID PQL_CMD_MEMC_Mode_FB(const unsigned char  *pCommand)
{
	//LogPrintf(DBG_MSG, "fallback level = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u8_flbk_lvl     = (unsigned char)*pCommand;
	}
}
VOID PQL_CMD_InResolution(const PQL_INPUT_RESOLUTION *pCommand)
{
#if 0
	switch(*pCommand)
	{
	case _PQL_IN_1920x540:
		LogPrintf(DBG_MSG, "Input: 1920x540\r\n");
		break;
	case _PQL_OUT_1920x1080:
		LogPrintf(DBG_MSG, "Input: 1920x1080\r\n");
		break;
	case _PQL_OUT_3840x1080:
		LogPrintf(DBG_MSG, "Input: 3840x1080\r\n");
		break;
	case _PQL_IN_3840x2160:
		LogPrintf(DBG_MSG, "Input: 3840x2160\r\n");
		break;
	case _PQL_IN_3840x540:
		LogPrintf(DBG_MSG, "Input: 3840x540\r\n");
		break;
	case _PQL_IN_1920x2160:
		LogPrintf(DBG_MSG, "Input: 1920x2160\r\n");
		break;
	}
#endif
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_input_resolution = (PQL_INPUT_RESOLUTION)*pCommand;
	}
}
VOID PQL_CMD_InFrameRate( const PQL_INPUT_FRAME_RATE *pCommand)
{
	//LogPrintf(DBG_MSG, "input frame rate = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_input_frameRate = (PQL_INPUT_FRAME_RATE)*pCommand;
	}
}
VOID PQL_CMD_In3DFormat(const PQL_INPUT_3D_FORMAT *pCommand)
{
	//LogPrintf(DBG_MSG, "input 3d format = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_input_3dFormat = (PQL_INPUT_3D_FORMAT)*pCommand;
	}
}

VOID PQL_CMD_OutResolution(const PQL_OUTPUT_RESOLUTION *pCommand)
{
	//LogPrintf(DBG_MSG, "output resolution = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_output_resolution = (PQL_OUTPUT_RESOLUTION)*pCommand;
	}
}
VOID PQL_CMD_OutFrameRate( const PQL_OUTPUT_FRAME_RATE *pCommand)
{
	//LogPrintf(DBG_MSG, "output frame rate = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_output_frameRate = (PQL_OUTPUT_FRAME_RATE)*pCommand;
	}
}
VOID PQL_CMD_Out3DFormat(const PQL_OUTPUT_3D_FORMAT *pCommand)
{
	//LogPrintf(DBG_MSG, "output 3d format = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_output_3dFormat = (PQL_OUTPUT_3D_FORMAT)*pCommand;
	}
}
VOID PQL_CMD_OutMode(const PQL_OUT_MODE_ *pCommand)
{
	//LogPrintf(DBG_MSG, "output mode = %d\r\n", *pCommand);
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->_output_mode    = (PQL_OUT_MODE_)*pCommand;
	}
}
VOID PQL_CMD_LR_Swap(const         unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u1_LR_swap      = (unsigned char)*pCommand;
	}
}
VOID PQL_CMD_InsertBlack(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u1_InsertBlack  = (unsigned char)*pCommand;
	}
}

VOID PQL_CMD_crtc_N(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u8_crtc_n= (unsigned char)*pCommand;

		rtd_pr_memc_debug("CMD:: crtc_N = %d\r\n", pExtData->u8_crtc_n);
	}
}
VOID PQL_CMD_crtc_M(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u8_crtc_m= (unsigned char)*pCommand;

		rtd_pr_memc_debug("CMD:: crtc_M = %d\r\n", pExtData->u8_crtc_m);
	}
}

VOID PQL_CMD_OsdForceMode(const     BOOL *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u1_osd_force_mode= (BOOL)*pCommand;

		rtd_pr_memc_debug("CMD:: u1_osd_force_mode = %d\r\n", pExtData->u1_osd_force_mode);
	}
}

VOID PQL_CMD_PullDownMode(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData=NULL;
		if(s_pContext == NULL)
			return;
		pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		pExtData->u8_pulldown_mode = (unsigned char)*pCommand;

		rtd_pr_memc_debug("CMD:: PULLDOWNMODE = %d\r\n", pExtData->u8_pulldown_mode);
	}
}

VOID PQL_CMD_MEMC_Mode(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u3_memcMode= (unsigned char)*pCommand;

		rtd_pr_memc_debug("CMD:: MEMCMODE = %d\r\n", pExtData->u3_memcMode);
	}
}

VOID PQL_CMD_InputVSXral_FrameRepeatMode(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u8_InputVSXral_FrameRepeatMode = (unsigned char)*pCommand;

		rtd_pr_memc_debug("CMD:: InputVSXral_FrameRepeatMode = %d\r\n", pExtData->u8_InputVSXral_FrameRepeatMode);
	}
}

VOID PQL_CMD_LowDelayMode(const     unsigned char *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u1_lowDelayMode = (unsigned char)*pCommand;

		rtd_pr_memc_debug("CMD:: LowDelayMode = %d\r\n", pExtData->u1_lowDelayMode);
	}
}

VOID PQL_CMD_PowerSaving(const     BOOL *pCommand)
{
	if (pCommand)
	{
		_EXTERNAL_INPUT *pExtData = (_EXTERNAL_INPUT *)&(s_pContext->_external_data);
		if(pExtData == NULL || s_pContext == NULL){
			return;
		}
		pExtData->u1_powerSaving= (BOOL)*pCommand;

		rtd_pr_memc_debug("CMD:: u1_osd_force_mode = %d\r\n", pExtData->u1_powerSaving);
	}
}

////////////////////////////////////////////////////////////////////////////
VOID PQL_REQ_cadenceID(unsigned char *pRequest)
{
	if (pRequest)
	{
#if 0/*need resolve:compiler error*/

		const _PQLCONTEXT *s_pContext = GetPQLContext();

		(*pRequest) = (s_pContext->_output_filmDetectctrl.u8_CurCadence == 0)? 1 : 0;
#endif
	}
}

VOID PQL_INFO_SRCVO(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext();
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u1_SRC_VO =  (unsigned char)*pInfo;
		rtd_pr_memc_debug("INFO:: SRC_VO = %d\r\n", pExtInfo->u1_SRC_VO);
	}
}

VOID PQL_INFO_HDR_Chg(const unsigned char  *pInfo)
{
	if (pInfo)
	{
        const _PQLCONTEXT *s_pContext_temp = GetPQLContext();
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u1_HDR_Chg=  (unsigned char)*pInfo;
		rtd_pr_memc_debug("INFO:: HDR_CHG = %d\r\n", pExtInfo->u1_HDR_Chg);
	}
}

VOID PQL_INFO_Brightness(const unsigned short  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext();//fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u16_Brightness=  (unsigned short)*pInfo;
		rtd_pr_memc_debug("INFO:: Brightness = s%d, m%d\r\n", (pExtInfo->u16_Brightness)>>8, (pExtInfo->u16_Brightness & 0xff));
	}
}

VOID PQL_INFO_Contrast(const unsigned short  *pInfo)
{
	if (pInfo)
	{
        const _PQLCONTEXT *s_pContext_temp = GetPQLContext();//fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u16_Contrast=  (unsigned short)*pInfo;
		rtd_pr_memc_debug("INFO:: Brightness = s%d, m%d\r\n", (pExtInfo->u16_Contrast)>>8, (pExtInfo->u16_Contrast & 0xff));
	}
}

VOID PQL_INFO_Speed(const signed short  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext(); //fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);//fix me !!
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->s16_speed =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: Speed = s%d, m%d\r\n", (pExtInfo->s16_speed)>>8, (pExtInfo->s16_speed & 0xff));
	}
}

VOID PQL_INFO_Zoom(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext();//fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);//fix me !!
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u1_ZoomAction = (unsigned char)*pInfo;
		rtd_pr_memc_debug("INFO:: ZoomAction = %d\r\n", pExtInfo->u1_ZoomAction);
	}
}

VOID PQL_INFO_Shift(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext();//fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);//fix me !!
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u1_ShiftAction = (unsigned char)*pInfo;
		rtd_pr_memc_debug("INFO:: OnlyXYShitfAction = %d\r\n", pExtInfo->u1_ShiftAction);
	}
}

VOID PQL_INFO_SourceType(const unsigned short  *pInfo)
{
	if(pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext();//fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);//fix me !!
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u16_source_type = (unsigned short)*pInfo;
		rtd_pr_memc_debug("INFO:: source_type = %d\r\n", pExtInfo->u16_source_type);
	}
}

VOID PQL_INFO_VOSeek(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext(); //fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);//fix me !!
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u1_mVOSeek =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: VOSeek = %d\r\n", pExtInfo->u1_mVOSeek);
	}

}
VOID PQL_INFO_MEMCMode(const unsigned short  *pInfo)
{
	if (pInfo)
	{
		const _PQLCONTEXT *s_pContext_temp = GetPQLContext(); //fix me !!
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext_temp->_external_info);//fix me !!
		if(pExtInfo == NULL || s_pContext_temp == NULL){
			return;
		}
		pExtInfo->u16_MEMCMode =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: MEMCMode = %d\r\n", pExtInfo->u16_MEMCMode);
	}	
}


VOID PQL_INFO_DarkLevel(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u8_DarkLevel =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: DarkLevel = %d\r\n", pExtInfo->u8_DarkLevel);
	}
}

VOID PQL_INFO_Hist_SC(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u1_Hist_SC_flg =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: Hist_SC_flg = %d\r\n", pExtInfo->u1_Hist_SC_flg);
	}
}

VOID PQL_INFO_SoccerPatch(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u8_Soccer_holdfrm =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: Soccer = %d\r\n", pExtInfo->u8_Soccer_holdfrm);
	}
}

VOID PQL_INFO_triballPatch(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		pExtInfo->u8_triball_holdfrm =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: triball = %d\r\n", pExtInfo->u8_triball_holdfrm);
	}
}

VOID PQL_INFO_PictureMode_Chg(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u1_PictureMode_Chg =  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: PictureMode_Chg = %d\r\n", pExtInfo->u1_PictureMode_Chg);
	}
}

VOID PQL_INFO_VIP_Still(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u1_VIP_Still=  (unsigned char)*pInfo;
		rtd_pr_memc_debug("INFO:: VIP_Still = %d\r\n", pExtInfo->u1_VIP_Still);
	}
}

VOID PQL_INFO_MiraCast(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u1_mMiraCast=  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: MiraCast = %d\r\n", pExtInfo->u1_mMiraCast);
	}
}

VOID PQL_INFO_Forbes(const unsigned short  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u1_Forbes_flg=  (signed short)*pInfo;
		rtd_pr_memc_debug("INFO:: Forbes = %d\r\n", pExtInfo->u1_Forbes_flg);
	}
}

VOID PQL_INFO_DTL_Step_Hist_sum_ratio(const unsigned int  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}

		pExtInfo->DTL_Step_Hist_sum_ratio[0] =  (unsigned int)pInfo[0];
		pExtInfo->DTL_Step_Hist_sum_ratio[1] =  (unsigned int)pInfo[1];
		pExtInfo->DTL_Step_Hist_sum_ratio[2] =  (unsigned int)pInfo[2];
		pExtInfo->DTL_Step_Hist_sum_ratio[3] =  (unsigned int)pInfo[3];
		pExtInfo->DTL_Step_Hist_sum_ratio[4] =  (unsigned int)pInfo[4];
		
		//rtd_pr_memc_debug("INFO:: DTL_Step_Hist_sum_ratio[0] = %d,[1] = %d,[2] = %d,[3] = %d,[4] = %d\r\n", pExtInfo->DTL_Step_Hist_sum_ratio[0]
		//			, pExtInfo->DTL_Step_Hist_sum_ratio[1], pExtInfo->DTL_Step_Hist_sum_ratio[2], pExtInfo->DTL_Step_Hist_sum_ratio[3], pExtInfo->DTL_Step_Hist_sum_ratio[4]);
	}
}


VOID PQL_INFO_VIP_project_ID(const unsigned char  *pInfo)
{
	if (pInfo)
	{
		_EXTERNAL_INFO*pExtInfo = (_EXTERNAL_INFO*)&(s_pContext->_external_info);
		if(pExtInfo == NULL || s_pContext == NULL){
			return;
		}
		pExtInfo->u8_VIP_Project_ID=  (unsigned char)*pInfo;
		rtd_pr_memc_debug("INFO:: u8_VIP_Project_ID = %d\r\n", pExtInfo->u8_VIP_Project_ID);
	}
}



