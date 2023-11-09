#include "memc_isr/PQL/PQLPlatformDefs.h"
#include "memc_isr/PQL/PQLContext.h"
#include "memc_isr/PQL/PQLGlobalDefs.h"
#include <tvscalercontrol/panel/panelapi.h>
#include "memc_reg_def.h"
///////////////////////////////////////////////////////////////////////////////////////////

unsigned char u1_SuperAction_RimCompflag=0;
unsigned short u16_RightRim_16px = 0;


_Param_By_Resolution_RimCtrl Param_By_Resolution[_PQL_OUT_RESOLUTION_MAX] = {
	//                      height width    	bound(T,B,R,L)         divide	slowin_min_shift   RimDiffTh_cof	apl_size	scale(H,V)
	/*_PQL_OUT_1920x1080*/	{1920, 1080,	{180,  900, 1600,  320},	2, 1,		12, 12,				1,			64800,		2, 2},
	/*_PQL_OUT_1920x540*/	{1920,  540,	{ 90,  450,  160, 3200},	2, 0, 		12,  6, 			1, 			64800,		2, 1},
	/*_PQL_OUT_3840x2160*/	{3840, 2160,	{360, 1800, 3200,  640},	3, 2, 		24, 24, 			1, 			64800,		4, 4},
	/*_PQL_OUT_3840x1080*/	{3840, 1080,	{180,  900, 3200,  640},	3, 2, 		24, 12, 			1, 			64800,		4, 2},
	/*_PQL_OUT_3840x540*/	{3840,  540,	{ 90,  450, 3200,  640},	3, 0, 		24,  6, 			1, 			64800,		4, 1},
//	/*_PQL_OUT_768x480*/	{ 768,  480,	{ 80,  400,  640,  128},	0, 0,		 5,  5, 			1, 			64800,		1, 1},
//	/*_PQL_OUT_1280x720*/	{1280,  720,	{120,  600, 1066,  214},	1, 0, 		 8,  8, 			1, 			64800,		1, 1},
//	/*_PQL_OUT_7680x4320*/	{7680, 4320,	{720, 3600, 6400, 1280},	4, 3, 		48, 48, 			2, 			64800,		8, 8},
};

const _Param_By_Resolution_RimCtrl *GetRimParam(PQL_OUTPUT_RESOLUTION resolution_id)
{
	return (const _Param_By_Resolution_RimCtrl *)&Param_By_Resolution[resolution_id];
}

VOID RimCtrl_OutResolution_Parse(_OUTPUT_RimCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext   = GetPQLContext();
	_PQLCONTEXT *s_pContext_w   = GetPQLContext_m();
	PQL_OUTPUT_RESOLUTION outResolution_cur = s_pContext->_external_data._output_resolution;
	unsigned int u1_force960x540, log_en;
	ReadRegister(SOFTWARE3_SOFTWARE3_12_reg, 0, 0, &u1_force960x540);
	ReadRegister(SOFTWARE3_SOFTWARE3_47_reg,31,31, &log_en);
	if(u1_force960x540){//d930[0]
		s_pContext_w->_external_data._output_resolution = _PQL_OUT_960x540;
	}
	if(log_en){//d930[31]
		rtd_pr_memc_emerg("[%d] =========\n",outResolution_cur);
	}

	if (outResolution_cur != (PQL_OUTPUT_RESOLUTION)pOutput->u4_outResolution_pre)
	{
		rtd_pr_memc_emerg("[%s][%d][Bringup 4k1k]>>[cur,%d,pre,%d]\n",__FUNCTION__, __LINE__, outResolution_cur, pOutput->u4_outResolution_pre);
		pOutput->u4_outResolution_pre = outResolution_cur;

		switch (outResolution_cur)
		{
		case _PQL_OUT_1920x1080 :
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 1919;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 1079;
			break;
		case _PQL_OUT_1920x540:
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 1919;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 539;
			break;
		case _PQL_OUT_3840x2160:
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 3839;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 2159;
			break;
		case _PQL_OUT_3840x1080:
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 3839;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 1079;
			break;
		case _PQL_OUT_3840x540:
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 3839;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 539;
			break;
		case _PQL_OUT_960x540:
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 959;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 539;
		default: //_PQL_OUT_3840x2160
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = 3839;
			pOutput->u12_out_resolution[_RIM_TOP] = 0;
			pOutput->u12_out_resolution[_RIM_BOT] = 2159;
			break;
		}
		rtd_pr_memc_emerg(  "[%s][%d][Bringup 4k1k]>>[%d,%d,%d,%d]\n",__FUNCTION__, __LINE__, 
			pOutput->u12_out_resolution[_RIM_LFT], pOutput->u12_out_resolution[_RIM_RHT], pOutput->u12_out_resolution[_RIM_TOP], pOutput->u12_out_resolution[_RIM_BOT]);
	}
}

unsigned int SetSearchLine(unsigned int H_line_search_position, unsigned short MinVal, unsigned short MaxVal)
{ 
	H_line_search_position=_CLIP_(H_line_search_position,MinVal,MaxVal);
	WriteRegister(KMC_METER_TOP_KMC_METER_TOP_00_reg, 2,14,H_line_search_position);
	WriteRegister(KMC_METER_TOP_KMC_METER_TOP_00_reg, 16,28,H_line_search_position);
	return H_line_search_position;
}

VOID RimDet_by_KMC_YUV_Meter(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	unsigned int u32_rb_val = 0, sum_apl = 0, avg_pos = 0, log_en = 0, u1_Algo_en = 0;
	unsigned int right_sketchy_line = 0, right_precise_line = 0, right_sketchy_line_vaild = 0, right_precise_line_vaild = 0;
	unsigned int rdbk_histo00_cnt = 0,sum_rdbk_histo01to31_cnt = 0; //rdbk_histo00_cnt+sum_rdbk_histo01to31_cnt=2xheight (4k:2x2160)
	unsigned char u32_i, debugID = 0;
	unsigned short case_bright, case_dark, h_line_total = 3840;
	bool chg2RoughSearch = false;
	static unsigned short status_machine = 0, finetune_pos = 0;
	static unsigned int H_line_search_position = 0,H_line_search_position_pre = 0;
	static unsigned int apl_pre[32] ={0}, apl[32] ={0};
	static unsigned short line_pos[32]={0};
	static bool finetune = false;
	static char hold_cnt = 0;

	//kmc_meter_top_kmc_meter_top_04_RBUS kmc_meter_top_kmc_meter_top_04;
	ReadRegister(HARDWARE_HARDWARE_43_reg, 15, 15, &u1_Algo_en);

	if(!u1_Algo_en) { //d5ac[15]
		u16_RightRim_16px = 0;
		line_pos[0]=0;
		return;
	}

	//decide if need rim detction
	ReadRegister(BBD_BBD_OUTPUT_RIGHT_ACTIVE_LINE_reg,0,12,&right_sketchy_line);
	ReadRegister(BBD_BBD_OUTPUT_RIGHT_ACTIVE_LINE_reg,16,28,&right_precise_line);
	ReadRegister(BBD_BBD_OUTPUT_RIGHT_ACTIVE_LINE_reg,13,13,&right_sketchy_line_vaild);
	ReadRegister(BBD_BBD_OUTPUT_RIGHT_ACTIVE_LINE_reg,29,29,&right_precise_line_vaild);
	ReadRegister(HARDWARE_HARDWARE_43_reg,7,7, &log_en);

	//only search for the right rim(16px)
	right_sketchy_line = right_sketchy_line<<4;
	if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){// 2k
		if(right_precise_line_vaild){
			if(right_precise_line<1903) {
				u16_RightRim_16px = 0;
				line_pos[0]=0;
				return;
			}
		}
		else if(right_sketchy_line_vaild) {
			if(right_sketchy_line<1888) {
				u16_RightRim_16px = 0;
				line_pos[0]=0;
				return;
			}
		}
	}else{//4k
		if(right_precise_line_vaild){
			if(right_precise_line<3823) {
				u16_RightRim_16px = 0;
				line_pos[0]=0;
				return;
			}
		}
		else if(right_sketchy_line_vaild) {
			if(right_sketchy_line<3808) {
				u16_RightRim_16px = 0;
				line_pos[0]=0;
				return;
			}
		}
	}


	//set Y mode 
	WriteRegister(KMC_METER_TOP_KMC_METER_TOP_00_reg, 0,1,0x0);
	//read blckth
	for (u32_i = 0; u32_i < 32; u32_i ++)
	{
		//rdbk_histo00 ( cnt of Y_histo )			
		ReadRegister(KMC_METER_TOP_KMC_METER_TOP_80_reg + 4 * u32_i, 0, 24, &u32_rb_val);
		{
			if(u32_i<1){
				rdbk_histo00_cnt = rdbk_histo00_cnt+u32_rb_val; //black pixel number
			}else{
				sum_rdbk_histo01to31_cnt = u32_rb_val + sum_rdbk_histo01to31_cnt; //non black pixel number
			}
		}
	}
	for(u32_i = 0; u32_i < 32; u32_i ++)
	{
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_80_reg + 4 * u32_i, 0, 19, &u32_rb_val);
		apl_pre[u32_i] =  u32_rb_val;
		ReadRegister(KME_ME1_TOP9_KME_ME1_TOP9_00_reg + 4 * u32_i, 0, 19, &u32_rb_val);
		apl[u32_i] =  u32_rb_val;
		sum_apl = _ABS_DIFF_(apl_pre[u32_i],apl[u32_i])+sum_apl;
	}

	case_bright = (sum_rdbk_histo01to31_cnt>(((rdbk_histo00_cnt+sum_rdbk_histo01to31_cnt)*51)>>10));//51>>10 -> 5%
	case_dark = rdbk_histo00_cnt>(((rdbk_histo00_cnt+sum_rdbk_histo01to31_cnt)*1004)>>10);//1004>>10 -> 98%//(rdbk_histo00_cnt>((rdbk_histo00_cnt+sum_rdbk_histo01to31_cnt)>>1));

	//set V range
	h_line_total = pOutput->u12_out_resolution[_RIM_RHT] + 1;
	WriteRegister(KMC_METER_TOP_KMC_METER_TOP_04_reg,  0, 12, 0x0);
	WriteRegister(KMC_METER_TOP_KMC_METER_TOP_04_reg, 16, 28, pOutput->u12_out_resolution[_RIM_BOT]);


	//H left Rim check
	if(status_machine == 0){//init
		H_line_search_position = h_line_total-1;
		SetSearchLine(H_line_search_position, 0, 0x1FFF);
		status_machine = 1;
	}
	else if(hold_cnt>0){
		hold_cnt--;
		debugID = 88;
	}
	else if (status_machine == 1){
		ReadRegister(KMC_METER_TOP_KMC_METER_TOP_00_reg,2,14,&H_line_search_position_pre);
		if(sum_apl == 0){
			status_machine = 1;
			debugID = 1;	
		}else{
			if(H_line_search_position!=H_line_search_position_pre){
				status_machine = 1;
				debugID = 2;
				H_line_search_position = SetSearchLine(H_line_search_position, h_line_total-17, h_line_total-1);			
			}else{
				if(case_bright){ 
					status_machine = 2;
					debugID = 3;
					H_line_search_position = SetSearchLine(H_line_search_position+1, h_line_total-17, h_line_total-1);
					hold_cnt=1;
 				}else if(case_dark){ 
	 				status_machine = 3;
					debugID = 4;
					H_line_search_position = SetSearchLine(H_line_search_position-1, h_line_total-17, h_line_total-1);
					hold_cnt=1;
				}else{
					status_machine = 4;
					debugID = 10;
					H_line_search_position = SetSearchLine(H_line_search_position, h_line_total-17, h_line_total-1);
					finetune_pos = H_line_search_position;
					//hold_cnt=1;
				}
			}
		}
	}else if(status_machine == 2){ 
		status_machine = 1;
		debugID = 5;
		H_line_search_position = SetSearchLine(H_line_search_position, h_line_total-17, h_line_total-1);
		hold_cnt=1;
 	}else if(status_machine == 3){ 
 		status_machine = 1;
		debugID = 6;
		H_line_search_position = SetSearchLine(H_line_search_position, h_line_total-17, h_line_total-1);
		hold_cnt=1;
	}
	else if(status_machine == 4){ 
		status_machine = (H_line_search_position==h_line_total-1) ? 6 : 5;
		H_line_search_position = (H_line_search_position==h_line_total-1) ? 
									SetSearchLine(finetune_pos-1, h_line_total-17, h_line_total-1) : 
									SetSearchLine(finetune_pos+1, h_line_total-17, h_line_total-1);
		hold_cnt=3;
		debugID = 77;
	}
	else if (status_machine == 5){ 
		if(case_bright){ 
			status_machine = 2;
			chg2RoughSearch=true;
			H_line_search_position = SetSearchLine(finetune_pos+1, h_line_total-17, h_line_total-1);
			hold_cnt=1;
			debugID = 51;
		}
		else if(H_line_search_position==h_line_total-17){
			status_machine = 4;
			debugID = 63;
			H_line_search_position = SetSearchLine(finetune_pos, h_line_total-17, h_line_total-1);	
		}else{
			status_machine = 6;
			H_line_search_position = SetSearchLine(finetune_pos-1, h_line_total-17, h_line_total-1);
			hold_cnt=3;
			debugID = 52;
		}
	}
	else if (status_machine == 6){
		if(case_dark){ //dark
			status_machine = 3;
			chg2RoughSearch=true;
			H_line_search_position = SetSearchLine(finetune_pos-1, h_line_total-17, h_line_total-1);
			hold_cnt=1;
			debugID = 61;
		}
		else {
			status_machine = 4;
			debugID = 62;
			H_line_search_position = SetSearchLine(finetune_pos, h_line_total-17, h_line_total-1);
		}
	}
	else {
		status_machine = 0;
		debugID = 99;
	}
	
	u16_RightRim_16px = (status_machine>=4&&status_machine<=6) ? finetune_pos : H_line_search_position;
	finetune = (status_machine>=4&&status_machine<=6) ? true : false;

	//statis histogram is invaild at rightest two lines
	u16_RightRim_16px = (u16_RightRim_16px>=h_line_total-3) ? h_line_total-1 : u16_RightRim_16px;

#if 1
	if(chg2RoughSearch){
		for(u32_i=0;u32_i<32;u32_i++){
			line_pos[u32_i] = 0;
		}
	}
	else if(finetune==false) {
		for(u32_i=31;u32_i>0;u32_i--){
			line_pos[u32_i] = line_pos[u32_i-1];
			avg_pos += line_pos[u32_i];
		}
		line_pos[0]=u16_RightRim_16px;
		avg_pos = (avg_pos+line_pos[0])>>5;

		finetune=true;
		for(u32_i=0;u32_i<32;u32_i++){
			if(_ABS_DIFF_(line_pos[u32_i], avg_pos)>2){
				finetune = false;
				break;
			}
		}
		if(finetune==true){
			status_machine = 4;
			finetune_pos = H_line_search_position;
		}
	}
#endif
	if(log_en) { //d5ac[7]
		rtd_pr_memc_err("[%s][%d] Result>>[%d] status>>[%d][%d] hist cnt>>[%u,%u] search>>[%u,%d] case>>[%d,%d] apl>>[%u] finetune>>[%d]\n",__FUNCTION__, __LINE__,
			u16_RightRim_16px,
			status_machine, debugID, 
			rdbk_histo00_cnt, sum_rdbk_histo01to31_cnt,
			H_line_search_position,finetune_pos,
			case_bright, case_dark,
			sum_apl, finetune );
	}
}








#if 0 // YE Test  it's K6 only

unsigned int  RimDet_Check_Right(unsigned int H_line_search_position,unsigned int v_line_total)
{
	
	unsigned int cursor_color_mc_y0 = 0,cursor_color_mc_y1 = 0,cursor_color_ippre_r = 0, cursor_color_kpost_r = 0;
	//unsigned int right_blk = H_line_search_position/64;
	//unsigned int score=-1;
	static unsigned int H_line_search_position_R = 0;	
	//unsigned int i//,j;
	//unsigned int sum_apl_hist00,blk_status = 0;
	//unsigned short *APL=NULL;
	static unsigned int status_machine = 0;//,move_status =3;
	unsigned int kpost_cursor_en =0,mc_cursor_en =0,ippre_cursor_en =0;
	unsigned int kpost_cursor_x = 0,mc_cursor_x1=0,mc_cursor_x0=0,ippre_cursor_x=0;
	static unsigned int H_line_search_position_max = 0;
	static unsigned int count =0;
	bool sync_flag = 1;

	#if 0 //H-Rim
	static unsigned int Test_value=0;
		
	ReadRegister(FRC_TOP__software1__reg_software_22_ADDR, 8, 15, &Test_value);	
	#endif

#if	0
	unsigned int u32_rb_val_flag = 0;
	ReadRegister(KIWI_REG(FRC_TOP__PQL_0__pql_dummy_ac), &u32_rb_val_flag);/*0xB809D5AC*/		

	rtd_pr_memc_err("===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
	rtd_pr_memc_err("[RimDet_Check_Right_init_done]\n");
#endif 
		ReadRegister(FRC_TOP__MC__regr_mc_rdbk_value0_ADDR,0,9, &cursor_color_mc_y0);
		ReadRegister(FRC_TOP__MC__regr_mc_rdbk_value1_ADDR,0,9, &cursor_color_mc_y1);
		ReadRegister(FRC_TOP__IPPRE__regr_inp_cursor_bd_r_ADDR,0,9,&cursor_color_ippre_r);
		ReadRegister(FRC_TOP__KPOST_TOP__regr_post_cursor_fd_r_ADDR,0,9,&cursor_color_kpost_r);
	if((H_line_search_position_R!=H_line_search_position))
		sync_flag = 0;

	//if(_ABS_(H_line_search_position-H_line_search_position_R)>Test_value)
	if(_ABS_(H_line_search_position-H_line_search_position_R)>60)
		sync_flag = 1;

	if(H_line_search_position>H_line_search_position_R)
		sync_flag = 1;

	
	if(sync_flag)
	{
		H_line_search_position_R = H_line_search_position;
		H_line_search_position_max = H_line_search_position;
	}

	#if 0
	APL = fwif_color_Get_APL();
	for( i = 50; i<60;i++)
	{
		for( j = 0;j<34;j++)
		{
			
			if(APL[j*60+i]<64)
				sum_apl_hist00++;
			//rtd_pr_memc_err("[APL[j*60+i]]>>[%d,%d]\n",APL[j*60+i],j*60+i);
			if(sum_apl_hist00>30)
			{
				right_blk = i;
				blk_status =1;
				//rtd_pr_memc_err("[sum_apl_hist00,i,j]>>[%d,%d,%d]\n",sum_apl_hist00,i,j);
				break;				
			}
			
		}
		sum_apl_hist00 =0;
	}
	if(blk_status)
	{
		if(APL[(right_blk-1)+16*60]!=0)
		{
			H_line_search_position_R = right_blk*64;
			status_machine = 0;
		}
		else
		{
			blk_status =0;
		}
	}
	i=0;
	j=0;
	#endif
	//for(i = 0;i<8;i++)
	{	
	if(status_machine == 0){//init
	
		WriteRegister(FRC_TOP__IPPRE__inp_cursor_en_ADDR,15,15,1);
		WriteRegister(FRC_TOP__KPOST_TOP__post_cursor_en_ADDR,15,15,1);
		WriteRegister(FRC_TOP__MC__mc_cursor_en_ADDR,FRC_TOP__MC__mc_cursor_en_BITSTART,FRC_TOP__MC__mc_cursor_en_BITEND, 0x3);
		WriteRegister(FRC_TOP__IPPRE__inp_cursor_v_ADDR,FRC_TOP__IPPRE__inp_cursor_v_BITSTART,FRC_TOP__IPPRE__inp_cursor_v_BITEND,v_line_total/2);
		WriteRegister(FRC_TOP__MC__mc_cursor_y0_ADDR,FRC_TOP__MC__mc_cursor_y0_BITSTART,FRC_TOP__MC__mc_cursor_y0_BITEND,v_line_total/2);
		WriteRegister(FRC_TOP__MC__mc_cursor_y1_ADDR,FRC_TOP__MC__mc_cursor_y1_BITSTART,FRC_TOP__MC__mc_cursor_y1_BITEND,v_line_total/2);
		WriteRegister(FRC_TOP__KPOST_TOP__post_cursor_v_ADDR,FRC_TOP__KPOST_TOP__post_cursor_v_BITSTART,FRC_TOP__KPOST_TOP__post_cursor_v_BITEND,v_line_total/2);
		WriteRegister(FRC_TOP__IPPRE__inp_cursor_h_ADDR,FRC_TOP__IPPRE__inp_cursor_h_BITSTART,FRC_TOP__IPPRE__inp_cursor_h_BITEND,H_line_search_position_R-2);
		WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,H_line_search_position_R-1);
		WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,H_line_search_position_R+1);
		WriteRegister(FRC_TOP__KPOST_TOP__post_cursor_h_ADDR,FRC_TOP__KPOST_TOP__post_cursor_h_BITSTART,FRC_TOP__KPOST_TOP__post_cursor_h_BITEND,H_line_search_position_R+2);
		status_machine = 1;
		//H_line_search_position = 3840;

	}
	else if(status_machine ==1)
	{

		ReadRegister(FRC_TOP__KPOST_TOP__post_cursor_en_ADDR,15,15,&kpost_cursor_en);
		ReadRegister(FRC_TOP__IPPRE__inp_cursor_en_ADDR,15,15,&ippre_cursor_en);
		ReadRegister(FRC_TOP__MC__mc_cursor_en_ADDR,FRC_TOP__MC__mc_cursor_en_BITSTART,FRC_TOP__MC__mc_cursor_en_BITEND, &mc_cursor_en);
		ReadRegister(FRC_TOP__IPPRE__inp_cursor_h_ADDR,FRC_TOP__IPPRE__inp_cursor_h_BITSTART,FRC_TOP__IPPRE__inp_cursor_h_BITEND,&ippre_cursor_x);
		ReadRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,&mc_cursor_x0);
		ReadRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,&mc_cursor_x1);
		ReadRegister(FRC_TOP__KPOST_TOP__post_cursor_h_ADDR,FRC_TOP__KPOST_TOP__post_cursor_h_BITSTART,FRC_TOP__KPOST_TOP__post_cursor_h_BITEND,&kpost_cursor_x);
		if((kpost_cursor_en ==0)&&(ippre_cursor_en == 0)&&(mc_cursor_en==0))
		{
			status_machine = 0;
		}
		else if((ippre_cursor_x !=H_line_search_position_R-2)&&(mc_cursor_x0 !=H_line_search_position_R-1)&&(mc_cursor_x1 !=H_line_search_position_R+1)&&(kpost_cursor_x !=H_line_search_position_R+2))
		{
			status_machine = 0;
		}
		else{
		ReadRegister(FRC_TOP__MC__regr_mc_rdbk_value0_ADDR,0,9, &cursor_color_mc_y0);
		ReadRegister(FRC_TOP__MC__regr_mc_rdbk_value1_ADDR,0,9, &cursor_color_mc_y1);
		ReadRegister(FRC_TOP__IPPRE__regr_inp_cursor_bd_r_ADDR,0,9,&cursor_color_ippre_r);
		ReadRegister(FRC_TOP__KPOST_TOP__regr_post_cursor_fd_r_ADDR,0,9,&cursor_color_kpost_r);
		if(cursor_color_mc_y1>=64)
		{
			if(cursor_color_mc_y0>=64)
			{
				sync_flag = 0;
				{
				H_line_search_position_R = H_line_search_position_R+7;
				H_line_search_position = H_line_search_position_R;//debug ,remove later
				//move_status = 2;//right move
				if(count >0)
					count=0;
				}
			}
			else
			{
				H_line_search_position = H_line_search_position_R;
			}
		}
		else if(cursor_color_mc_y0<=64)
		{
			count++;
			H_line_search_position = H_line_search_position_R;
			if(count >5)
				H_line_search_position = H_line_search_position_R-2;
		}
		else
		{
				H_line_search_position = H_line_search_position_R;
		}
		/*
		 if(cursor_color_kpost_r>=64)
		{
			sync_flag = 0;
			if(move_status ==1)
			{
				move_status = 3;
				H_line_search_position_R = H_line_search_position_R-3;
				H_line_search_position = H_line_search_position_R;
				
			}
			else
			{
				H_line_search_position_R = H_line_search_position_R+5;	
				H_line_search_position = H_line_search_position_R;//debug ,remove later
				move_status = 2;//right move
			}
		}
		else if(cursor_color_ippre_r<=64)
		{
			if((cursor_color_mc_y0<=64)&&(cursor_color_mc_y1<=64))
			{
				if(move_status ==2)
				{
					move_status = 3;
					H_line_search_position_R = H_line_search_position_R-3;
					H_line_search_position = H_line_search_position_R;
				}
				else
				{
					H_line_search_position_R = H_line_search_position_R-5;
					move_status =1;//left
				}
			}
			
			else if(cursor_color_mc_y1>64)
			{
				H_line_search_position_R = H_line_search_position_R-1;
				H_line_search_position = H_line_search_position_R;					
			}
			else if(cursor_color_mc_y0>64)
			{
				H_line_search_position_R = H_line_search_position_R-2;
				H_line_search_position = H_line_search_position_R;
			}
			
		}
		else if(cursor_color_mc_y1>64)
			{
				H_line_search_position_R = H_line_search_position_R+2;
				H_line_search_position = H_line_search_position_R;					
			}
			else if(cursor_color_mc_y0>64)
			{
				H_line_search_position_R = H_line_search_position_R+1;
				H_line_search_position = H_line_search_position_R;
			}
			else
			{
				H_line_search_position_R = H_line_search_position_R-1;
				H_line_search_position = H_line_search_position_R;
			}
			*/
		H_line_search_position_R = _CLIP_(H_line_search_position_R,3200,3839);
		WriteRegister(FRC_TOP__IPPRE__inp_cursor_h_ADDR,FRC_TOP__IPPRE__inp_cursor_h_BITSTART,FRC_TOP__IPPRE__inp_cursor_h_BITEND,H_line_search_position_R-2);
		WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,H_line_search_position_R-1);
		WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,H_line_search_position_R+1);
		WriteRegister(FRC_TOP__KPOST_TOP__post_cursor_h_ADDR,FRC_TOP__KPOST_TOP__post_cursor_h_BITSTART,FRC_TOP__KPOST_TOP__post_cursor_h_BITEND,H_line_search_position_R+2);
			}
			}
	}
	
	if(H_line_search_position_R>=H_line_search_position_max)
	{
		H_line_search_position_max = H_line_search_position_R;
	}
	
	//H_line_search_position = H_line_search_position_max;
	
	/*
	rtd_pr_memc_err("[H_line_search_position]>>[%d]\n",H_line_search_position);
	rtd_pr_memc_err("[H_line_search_position_R]>>[%d]\n",H_line_search_position_R);
	rtd_pr_memc_err("[cursor_color_kpost_r]>>[%d]\n",cursor_color_kpost_r);
	*/
	return H_line_search_position;	
}


#endif


#if 0 // YE Test  it's K6 only
VOID RimDet_by_KMC_YUV_Meter(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	unsigned int u32_rb_val = 0;
	unsigned int rdbk_histo00_cnt = 0,sum_rdbk_histo01to31_cnt = 0;
	//unsigned char cnt_shift = 1;
	unsigned int u32_i;
	unsigned int v_line_total = 2160,h_line_total = 3840;//4
	static unsigned int H_boundary_search_range = 640;//4
	static unsigned int H_line_search_start = 0, H_line_search_end_check1= 0, H_line_search_end_check2 = 0;
	static unsigned int H_line_search_position = 0,H_line_search_position_pre = 0;
	unsigned int H_line_search_position_R = 0;
	static unsigned int status_machine = 0;

	static unsigned int H_line_search_position_max = 0;
	//unsigned int cursor_y0 =0,cursor_y1=0,cursor_x0 = 0,cursor_x1 = 0;
	unsigned int cursor_color_y0 = 0,cursor_color_y1 = 0;
	unsigned int u8_black_th;
	unsigned int sum_apl = 0;
	//static unsigned int sum_apl_pre =0;
	static unsigned int apl_pre[32] ={0};
	static unsigned int apl[32] ={0};
	//static unsigned char status_machine_offset = 3;
	//static unsigned int rdbk_histo00_cnt_max=0;
	//static unsigned int sum_rdbk_histo01to31_cnt_min=64;

	#if 0 //H-Rim
	static unsigned int Test_bit_6=0,Test_bit_7=0;
	ReadRegister(FRC_TOP__software1__reg_software_22_ADDR, 6, 6, &Test_bit_6);		
	ReadRegister(FRC_TOP__software1__reg_software_22_ADDR, 7, 7, &Test_bit_7);		

	#endif
	
 #if	0
	unsigned int u32_rb_val_flag = 0;
	ReadRegister(KIWI_REG(FRC_TOP__PQL_0__pql_dummy_ac), &u32_rb_val_flag);/*0xB809B03C*/		
#endif 
	
	//set Y mode 
	WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_yuv_sel_ADDR, 0,1,0x0);
	//WriteRegister(FRC_TOP__MC__mc_cursor_en_ADDR,FRC_TOP__MC__mc_cursor_en_BITSTART,FRC_TOP__MC__mc_cursor_en_BITEND, 0x3);
	//read blckth
	ReadRegister(KIWI_REG(FRC_TOP__BBD__BBD_black_th), &u8_black_th);
	for (u32_i = 0; u32_i < 32; u32_i ++)
	{
		//rdbk_histo00 ( cnt of Y_histo )			
		ReadRegister(FRC_TOP__KMC_METER_TOP__regr_kmc_meter_rdbk_histo00_ADDR + 4 * u32_i, 0, 22, &u32_rb_val);
		/*
		if(u8_black_th == 0x27)
		{
		if(u32_i==0){
			rdbk_histo00_cnt = u32_rb_val;
		}else{
			sum_rdbk_histo01to31_cnt = u32_rb_val + sum_rdbk_histo01to31_cnt;
		}
		}
		else
		*/
		{
			if(u32_i<2){
				rdbk_histo00_cnt = rdbk_histo00_cnt+u32_rb_val;
			}else{
				sum_rdbk_histo01to31_cnt = u32_rb_val + sum_rdbk_histo01to31_cnt;
			}
		}
	}
	for(u32_i = 0; u32_i < 32; u32_i ++)
	{
			ReadRegister(FRC_TOP__KME_ME1_TOP9__regr_me1_statis_rgn00_aplp_ADDR + 4 * u32_i, 0, 19, &u32_rb_val);
			//pOutput->u20_me_rAPLi_rb[u32_i] = u32_rb_val;
			//sum_apl = _ABS_(apl_pre[u32_i]-u32_rb_val)+sum_apl;
			apl_pre[u32_i] =  u32_rb_val;
			ReadRegister(FRC_TOP__KME_ME1_TOP9__regr_me1_statis_rgn00_apli_ADDR + 4 * u32_i, 0, 19, &u32_rb_val);
			apl[u32_i] =  u32_rb_val;
			sum_apl = _ABS_(apl_pre[u32_i]-apl[u32_i])+sum_apl;
	}
	
	ReadRegister(FRC_TOP__MC__regr_mc_rdbk_value0_ADDR,0,9, &cursor_color_y0);
	ReadRegister(FRC_TOP__MC__regr_mc_rdbk_value1_ADDR,0,9, &cursor_color_y1);
#if	0
	//if(u32_rb_val_flag&0x4)
	if(Test_bit_6==1) //H-Rim
	{
		rtd_pr_memc_err("[YE RIM]===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
		rtd_pr_memc_err("[YE RIM][sum_rdbk_histo01to31_cnt]>>[%d]\n",sum_rdbk_histo01to31_cnt);
		rtd_pr_memc_err("[YE RIM][rdbk_histo00_cnt]>>[%d]\n",rdbk_histo00_cnt);
	}
#endif			

	//fix V range	
	if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){// 2k
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_start_y_yuv_ADDR, 0,11,0x0);
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_y_yuv_ADDR, 12,23,0x437);
		WriteRegister(FRC_TOP__MC__mc_cursor_y0_ADDR,FRC_TOP__MC__mc_cursor_y0_BITSTART,FRC_TOP__MC__mc_cursor_y0_BITEND,0x168);
		WriteRegister(FRC_TOP__MC__mc_cursor_y1_ADDR,FRC_TOP__MC__mc_cursor_y1_BITSTART,FRC_TOP__MC__mc_cursor_y1_BITEND,0x2d0);
		v_line_total = 1080;
		h_line_total = 1920;
		//H_boundary_search_range = 320;
	}else{//4//4k
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_start_y_yuv_ADDR, 0,11,0x0);
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_y_yuv_ADDR, 12,23,0x86f);
		WriteRegister(FRC_TOP__MC__mc_cursor_y0_ADDR,FRC_TOP__MC__mc_cursor_y0_BITSTART,FRC_TOP__MC__mc_cursor_y0_BITEND,0x2d0);
		WriteRegister(FRC_TOP__MC__mc_cursor_y1_ADDR,FRC_TOP__MC__mc_cursor_y1_BITSTART,FRC_TOP__MC__mc_cursor_y1_BITEND,0x5a0);
		v_line_total = 2160;
		h_line_total = 3840;
		//H_boundary_search_range = 640;
	}

	//H left Rim check
	if(status_machine == 0){//init
	
		H_line_search_start = 0;
		H_line_search_end_check1 = 0;
		H_line_search_end_check2 = H_boundary_search_range;
		H_line_search_position = H_boundary_search_range>>1;
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_start_x_yuv_ADDR, 2,13,H_line_search_start);
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_x_yuv_ADDR, 14,25,H_line_search_position);
		status_machine = 1;

	}
	else if (status_machine == 1){// cnt of rdbk_histo00 check	

		ReadRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_x_yuv_ADDR, 14,25,&H_line_search_position_pre);
		/*
		if(sum_apl == 0)
		{
			status_machine = 1;
			pOutput->u12_out_resolution[_RIM_LFT] = 0;
			pOutput->u12_out_resolution[_RIM_RHT] = h_line_total-1;
			
		}
		else
		*/
		{
			//sum_apl_pre = sum_apl;
		if(H_line_search_position!=H_line_search_position_pre){
		//sum_rdbk_histo01to31_cnt = (sum_rdbk_histo01to31_cnt>>cnt_shift);
			WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_start_x_yuv_ADDR, 2,13,H_line_search_start);
			WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_x_yuv_ADDR, 14,25,H_line_search_position);
			if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
				WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,1920-H_line_search_position/2);
				WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,1920-H_line_search_position/2);
			}
			else{
				WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,3840-H_line_search_position/2);
				WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,3840-H_line_search_position/2);
			}
			status_machine = 1;
		}
		else{
			
			//if( sum_rdbk_histo01to31_cnt >(v_line_total<<1)){
			//if( sum_rdbk_histo01to31_cnt >200){   //YE Test it's ok
			if( sum_rdbk_histo01to31_cnt >(v_line_total>>2)){	
				status_machine = 2;

				
			}else{
					/*
						H_line_search_position=_CLIP_(H_line_search_position,0,640);
						pOutput->u12_out_resolution[_RIM_LFT] = H_line_search_position;
						if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
							pOutput->u12_out_resolution[_RIM_RHT] = 1920-H_line_search_position;
						}
						else{
							pOutput->u12_out_resolution[_RIM_RHT] = 3840-H_line_search_position;
						}
						*/
					if((sum_rdbk_histo01to31_cnt>0)&&(sum_rdbk_histo01to31_cnt<(v_line_total>>2))){
						status_machine = 4;
						H_line_search_position_max = H_line_search_position;
					}
					else
					{
						H_line_search_position=_CLIP_(H_line_search_position,0,640);
						if((H_line_search_end_check2-H_line_search_end_check1)<2){
							H_line_search_end_check1 = H_line_search_position;
							H_line_search_end_check2 = H_line_search_position+32;
						}
						status_machine = 3;
					}
				}
			}
			}

	}		
	else if(status_machine==2)
	{
		H_line_search_start = 0;
		H_line_search_end_check2 = H_line_search_position; 	
		H_line_search_position = H_line_search_end_check2>>1; //YE Test for caculate back position ORI

		#if 0
		if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
		H_line_search_position = H_line_search_end_check2-100; //YE Test for caculate back position
			}
		else
		H_line_search_position = H_line_search_end_check2-200; //YE Test for caculate back position	
		#endif
		H_line_search_position=_CLIP_(H_line_search_position,0,640);
		H_line_search_end_check1 = H_line_search_position;


		if(sum_rdbk_histo01to31_cnt<(10*v_line_total)){
			if(H_line_search_position>4){
				H_line_search_position=H_line_search_position<<1;
				H_line_search_position = H_line_search_position-8;
				H_line_search_end_check1 = H_line_search_position; 
			}
		}

		

		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_start_x_yuv_ADDR, 2,13,H_line_search_start);
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_x_yuv_ADDR, 14,25,H_line_search_position);
		#if 0
		if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
			WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,1920-H_line_search_position/2);
			WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,1920-H_line_search_position/2);
		}
		else{
			WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,3840-H_line_search_position/2);
			WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,3840-H_line_search_position/2);
		}
		#endif
		status_machine=1;


			
		pOutput->u12_out_resolution[_RIM_LFT] = H_line_search_position/2;
		//if((cursor_color_y0<=64)&&(cursor_color_y1<=64)){
			if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
				pOutput->u12_out_resolution[_RIM_RHT] = 1920-H_line_search_position/2;
			}
			else{
				pOutput->u12_out_resolution[_RIM_RHT] = 3840-H_line_search_position/2;
			}
			H_line_search_position_R = h_line_total -H_line_search_position/2; 
			//for(u32_i=0;u32_i<32;u32_i++)
			H_line_search_position_R = RimDet_Check_Right(H_line_search_position_R,v_line_total);
			pOutput->u12_out_resolution[_RIM_RHT] = H_line_search_position_R;
		//}
		H_line_search_position_max=H_line_search_position;

	}
	else if(status_machine==3)
	{
		H_line_search_start =0 ;
		H_line_search_end_check1 = H_line_search_position;
		H_line_search_end_check2 = H_line_search_position+16;
		H_line_search_position=(H_line_search_end_check1+H_line_search_end_check2)>>1; 
		H_line_search_position=_CLIP_(H_line_search_position,0,640);
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_start_x_yuv_ADDR, 2,13,H_line_search_start);
		WriteRegister(FRC_TOP__KMC_METER_TOP__kmc_meter_end_x_yuv_ADDR, 14,25,H_line_search_position);
		#if 0
		if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
			WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,1920-H_line_search_position/2);
			WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,1920-H_line_search_position/2);
		}
		else{
			WriteRegister(FRC_TOP__MC__mc_cursor_x0_ADDR,FRC_TOP__MC__mc_cursor_x0_BITSTART,FRC_TOP__MC__mc_cursor_x0_BITEND,3840-H_line_search_position/2);
			WriteRegister(FRC_TOP__MC__mc_cursor_x1_ADDR,FRC_TOP__MC__mc_cursor_x1_BITSTART,FRC_TOP__MC__mc_cursor_x1_BITEND,3840-H_line_search_position/2);
		}
		#endif
		
		if(H_line_search_position>H_line_search_position_max){
			H_line_search_position_max=H_line_search_position;
		}
		else{
			pOutput->u12_out_resolution[_RIM_LFT] =H_line_search_position_max/2;
			//if((cursor_color_y0<=64)&&(cursor_color_y1<=64))
				{
				if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
					pOutput->u12_out_resolution[_RIM_RHT] = 1920-H_line_search_position_max/2;
				}
				else{
					pOutput->u12_out_resolution[_RIM_RHT] = 3840- H_line_search_position_max/2;
				}
				H_line_search_position_R = h_line_total -H_line_search_position/2; 
				//for(u32_i=0;u32_i<32;u32_i++)
				H_line_search_position_R = RimDet_Check_Right(H_line_search_position_R,v_line_total);
				pOutput->u12_out_resolution[_RIM_RHT] = H_line_search_position_R;
			}
		}


		status_machine=1;
		//if(H_line_search_position ==640)
			//status_machine = 5;
	}
	else if(status_machine==4)
	{
			//if( sum_rdbk_histo01to31_cnt > (v_line_total>>1)){
			//if( sum_rdbk_histo01to31_cnt > 200){
			if( sum_rdbk_histo01to31_cnt >(v_line_total>>2)){	
				status_machine = 2;
			}else{
				
					if((sum_rdbk_histo01to31_cnt>0)&&(sum_rdbk_histo01to31_cnt<(v_line_total>>1))){
						status_machine = 4;
						H_line_search_position=_CLIP_(H_line_search_position,0,640);
						pOutput->u12_out_resolution[_RIM_LFT] = H_line_search_position/2;
						//if((cursor_color_y0<=64)&&(cursor_color_y1<=64))
							{
							if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
								pOutput->u12_out_resolution[_RIM_RHT] = 1920-H_line_search_position/2;
							}
							else{
								pOutput->u12_out_resolution[_RIM_RHT] = 3840-H_line_search_position/2;
							}
							H_line_search_position_R = h_line_total -H_line_search_position/2; 
							//for(u32_i=0;u32_i<32;u32_i++)
							H_line_search_position_R =RimDet_Check_Right(H_line_search_position_R,v_line_total);
							pOutput->u12_out_resolution[_RIM_RHT] = H_line_search_position_R;
						}
					}
					else
					{
						status_machine = 3;
					}
			}
			if(status_machine !=4)
			{
				if(status_machine == 2){
					H_line_search_end_check1 = 0;
					H_line_search_position=_CLIP_(H_line_search_position,0,640);
					H_line_search_end_check2 = H_line_search_position;

					if(sum_rdbk_histo01to31_cnt<(5*v_line_total)){
						if(H_line_search_position>8){
							pOutput->u12_out_resolution[_RIM_LFT] =( H_line_search_position-8)/2;
							//if((cursor_color_y0<=64)&&(cursor_color_y1<=64))
								{
								if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
									pOutput->u12_out_resolution[_RIM_RHT] = 1920-( H_line_search_position-8)/2;
								}
								else{
									pOutput->u12_out_resolution[_RIM_RHT] = 3840-( H_line_search_position-8)/2;
								}
								H_line_search_position_R = h_line_total -( H_line_search_position-8)/2; 
								//for(u32_i=0;u32_i<32;u32_i++)
								H_line_search_position_R =RimDet_Check_Right(H_line_search_position_R,v_line_total);
								pOutput->u12_out_resolution[_RIM_RHT] = H_line_search_position_R;
							}
						}
					}
					else{
						pOutput->u12_out_resolution[_RIM_LFT] = H_line_search_position/4;
						//if((cursor_color_y0<=64)&&(cursor_color_y1<=64))
							{
							if(pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080){
								pOutput->u12_out_resolution[_RIM_RHT] = 1920-H_line_search_position/4;
							}
							else{
								pOutput->u12_out_resolution[_RIM_RHT] = 3840-H_line_search_position/4;
							}
								H_line_search_position_R = h_line_total -H_line_search_position/4; 
								//for(u32_i=0;u32_i<32;u32_i++)
								H_line_search_position_R = RimDet_Check_Right(H_line_search_position_R,v_line_total);
								pOutput->u12_out_resolution[_RIM_RHT] = H_line_search_position_R;							
						}
					}
				}
				else {
					H_line_search_position=_CLIP_(H_line_search_position,0,640);
					H_line_search_end_check1 = H_line_search_position;
					H_line_search_end_check2 = H_line_search_position+16;
				}
				status_machine = 1;
			}
			
	}

#if 0 
	  if(u32_rb_val_flag&0x4)
		//if(Test_bit_7==1)
		{// bit 2
		rtd_pr_memc_err("[YE RIM]===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
		rtd_pr_memc_err("[YE RIM][status_machine]>>[%d]\n",status_machine);
		rtd_pr_memc_err("[YE RIM][H_line_search_position]>>[%d]\n",H_line_search_position);
		rtd_pr_memc_err("[YE RIM][H_line_search_end_check1]>>[%d]\n",H_line_search_end_check1);
		rtd_pr_memc_err("[YE RIM][H_line_search_end_check2]>>[%d]\n",H_line_search_end_check2);
		rtd_pr_memc_err("[YE RIM][v_line_total]>>[%d]\n",v_line_total);
		rtd_pr_memc_err("[YE RIM][out_resolution][T,B,L,R]>>[%d,%d,%d,%d]\n",pOutput->u12_out_resolution[_RIM_TOP],pOutput->u12_out_resolution[_RIM_BOT],pOutput->u12_out_resolution[_RIM_LFT],pOutput->u12_out_resolution[_RIM_RHT]);
		rtd_pr_memc_err("[YE RIM][cursor_color_y0,cursor_color_y1]>>[%d,%d]\n",cursor_color_y0,cursor_color_y1);
		rtd_pr_memc_err("[YE RIM][sum_apl,sum_apl_pre]>>[%d,%d]\n",sum_apl,sum_apl_pre);
		
		}
#endif

#if 0 
	//if(u32_rb_val_flag&0x4)
		if(Test_bit_7==1)
		{// bit 2
		rtd_pr_memc_err("[YE RIM]===== ,%s, ,%d,outRes_pre=%d, histo01to31_cnt=%d,v_line_total=%d, status_machine =%d , H_position=%d, H_check1=%d,H_check2=%d, H_max=%d,v_line_total=%d ,T=%d,B=%d,L=%d,R=%d, cursor_color_y0=%d,cursor_color_y1=%d,sum_apl=%d, sum_apl_pre=%d,\n"
			  ,__FUNCTION__, __LINE__,pOutput->u4_outResolution_pre,sum_rdbk_histo01to31_cnt,v_line_total,status_machine,H_line_search_position,H_line_search_end_check1,H_line_search_end_check2,H_line_search_position_max,v_line_total
			  ,pOutput->u12_out_resolution[_RIM_TOP],pOutput->u12_out_resolution[_RIM_BOT],pOutput->u12_out_resolution[_RIM_LFT],pOutput->u12_out_resolution[_RIM_RHT]
			  ,cursor_color_y0,cursor_color_y1,sum_apl,sum_apl_pre);
			
		}
#endif


}

#endif

VOID RimCtrl_HW_extract(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	unsigned short u12_BBD_roughRim[_RIM_NUM], u12_BBD_fineRim[_RIM_NUM], u12_det_Rim[_RIM_NUM], u12_rim_bound[_RIM_NUM];
	unsigned char  u1_BBD_roughValid[_RIM_NUM], u1_BBD_fineValid[_RIM_NUM];
	unsigned char  u8_k;
	static unsigned char u8_Rim_Comp_cnt = 0;
	unsigned char  apl_avg = 0;
	static bool Unbalance_last_time = 0;
	static unsigned char u8_Rim_Balance_cnt = 0;
#if 0 // LR RIM
	signed short u11_gmv_mvx;
	signed short u10_gmv_mvy;
	signed short speed = 10;
	kme_me1_top4_kme_me1_top4_00_RBUS kme_me1_top4_kme_me1_top4_00;
	kme_me1_top4_kme_me1_top4_04_RBUS kme_me1_top4_kme_me1_top4_04;

	kme_me1_top4_kme_me1_top4_38_RBUS kme_me1_top4_kme_me1_top4_38;
	kme_me1_top4_kme_me1_top4_3c_RBUS kme_me1_top4_kme_me1_top4_3c;

	kme_me1_top4_kme_me1_top4_40_RBUS kme_me1_top4_kme_me1_top4_40;
	kme_me1_top4_kme_me1_top4_44_RBUS kme_me1_top4_kme_me1_top4_44;

	kme_me1_top4_kme_me1_top4_78_RBUS kme_me1_top4_kme_me1_top4_78;
	kme_me1_top4_kme_me1_top4_7c_RBUS kme_me1_top4_kme_me1_top4_7c;


	kme_me1_top4_kme_me1_top4_80_RBUS kme_me1_top4_kme_me1_top4_80;
	kme_me1_top4_kme_me1_top4_84_RBUS kme_me1_top4_kme_me1_top4_84;

	kme_me1_top4_kme_me1_top4_b8_RBUS kme_me1_top4_kme_me1_top4_b8;
	kme_me1_top4_kme_me1_top4_bc_RBUS kme_me1_top4_kme_me1_top4_bc;

	kme_me1_top4_kme_me1_top4_c0_RBUS kme_me1_top4_kme_me1_top4_c0;
	kme_me1_top4_kme_me1_top4_c4_RBUS kme_me1_top4_kme_me1_top4_c4;

	kme_me1_top4_kme_me1_top4_f8_RBUS kme_me1_top4_kme_me1_top4_f8;
	kme_me1_top4_kme_me1_top4_fc_RBUS kme_me1_top4_kme_me1_top4_fc;
	#endif
#if	1
	unsigned int u32_rb_val = 0;
	ReadRegister(SOFTWARE3_SOFTWARE3_47_reg,0,31, &u32_rb_val);
#endif
#if 0 // LR RIM
	u11_gmv_mvx = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
	u10_gmv_mvy = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;

	kme_me1_top4_kme_me1_top4_00.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_00_reg);
	kme_me1_top4_kme_me1_top4_04.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_04_reg);
	kme_me1_top4_kme_me1_top4_38.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_38_reg);
	kme_me1_top4_kme_me1_top4_3c.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_3C_reg);
	kme_me1_top4_kme_me1_top4_40.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_40_reg);
	kme_me1_top4_kme_me1_top4_44.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_44_reg);
	kme_me1_top4_kme_me1_top4_78.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_78_reg);
	kme_me1_top4_kme_me1_top4_7c.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_7C_reg);
	kme_me1_top4_kme_me1_top4_80.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_80_reg);
	kme_me1_top4_kme_me1_top4_84.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_84_reg);
	kme_me1_top4_kme_me1_top4_b8.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_B8_reg);
	kme_me1_top4_kme_me1_top4_bc.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_BC_reg);
	kme_me1_top4_kme_me1_top4_c0.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_C0_reg);
	kme_me1_top4_kme_me1_top4_c4.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_C4_reg);
	kme_me1_top4_kme_me1_top4_f8.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_F8_reg);
	kme_me1_top4_kme_me1_top4_fc.regValue = rtd_inl(KME_ME1_TOP4_KME_ME1_TOP4_FC_reg);
#endif
	// copy data.
	for (u8_k = 0; u8_k < _RIM_NUM; u8_k ++)
	{
		u12_BBD_roughRim[u8_k]  = s_pContext->_output_read_comreg.u12_BBD_roughRim_rb[u8_k];
		u1_BBD_roughValid[u8_k] = s_pContext->_output_read_comreg.u1_BBD_roughValid_rb[u8_k];
		u12_BBD_fineRim[u8_k]   = s_pContext->_output_read_comreg.u12_BBD_fineRim_rb[u8_k];
		u1_BBD_fineValid[u8_k]  = s_pContext->_output_read_comreg.u1_BBD_fineValid_rb[u8_k];
	}

	u12_rim_bound[_RIM_LFT] = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 320 :  
							  (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ?  160 : 640; // else, Hactive = 3840.
	u12_rim_bound[_RIM_RHT] = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1600 :
							  (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ?  800 : 3200;

	u12_rim_bound[_RIM_TOP] = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 180 : 
		                      (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160)?  360 : 90; // else, Vactive = 540.
	u12_rim_bound[_RIM_BOT] = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 900 : 
		                      (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160)? 1800 : 450;

	// rim source select.
	for (u8_k = 0; u8_k < _RIM_NUM; u8_k ++)
	{
//		unsigned short u12_roughRim = (u8_k == _RIM_TOP || u8_k == _RIM_BOT)? u12_BBD_roughRim[u8_k] :
//			                  (u8_k == _RIM_LFT? (u12_BBD_roughRim[u8_k] * _Rim_Hor_Bin_) : (u12_BBD_roughRim[u8_k] * _Rim_Hor_Bin_ - 1));

		if(u8_k==_RIM_RHT && u16_RightRim_16px != 0){
			u12_det_Rim[u8_k] = u16_RightRim_16px;
		}
		else if(u8_k==_RIM_LFT||u8_k==_RIM_RHT){
			u12_det_Rim[u8_k] = u1_BBD_fineValid[u8_k] == 1? u12_BBD_fineRim[u8_k] :
				               (u1_BBD_roughValid[u8_k] == 1? 16*u12_BBD_roughRim[u8_k] : pOutput->u12_out_resolution[u8_k]);
		}
		else{
			u12_det_Rim[u8_k] = u1_BBD_fineValid[u8_k] == 1? u12_BBD_fineRim[u8_k] :
				               (u1_BBD_roughValid[u8_k] == 1? u12_BBD_roughRim[u8_k] : pOutput->u12_out_resolution[u8_k]);
		}

		//if(u8_k == _RIM_RHT){
		//	if(u12_det_Rim[_RIM_RHT]>=3823)
		//		u12_det_Rim[_RIM_RHT] =  pOutput->u12_out_resolution[_RIM_RHT];
		//}

	}

#if 0 // LR RIM
	rtd_pr_memc_err("mvx:%d mvy:%d sf:%d aDTL:%d unconf:%d\n",u11_gmv_mvx, u10_gmv_mvy, 
		s_pContext->_output_frc_sceneAnalysis.u1_still_frame, s_pContext->_output_read_comreg.u25_me_aDTL_rb, 
		s_pContext->_output_read_comreg.u12_me_GMV_1st_unconf_rb );

	if(u32_rb_val&0x800000){//bit23
		if((kme_me1_top4_kme_me1_top4_38.regr_me1_rgn07_grp0_mvx>1024&&kme_me1_top4_kme_me1_top4_38.regr_me1_rgn07_grp0_mvx<2048-speed) || 
			) {
			u12_det_Rim[_RIM_RHT] = pOutput->u12_out_resolution[_RIM_RHT];
		}
		else if(u11_gmv_mvx>10) {
			u12_det_Rim[_RIM_LFT] = pOutput->u12_out_resolution[_RIM_LFT];
		}
	}
#endif
	if(u32_rb_val&0x4000){// bit 14
		rtd_pr_memc_err("[RimCtrl_HW_extract]%d %d [%d-%d] [%d-%d]\n", u12_det_Rim[_RIM_RHT], u16_RightRim_16px,
											u1_BBD_fineValid[_RIM_RHT], u12_BBD_fineRim[_RIM_RHT], 
											u1_BBD_roughValid[_RIM_RHT], u12_BBD_roughRim[_RIM_RHT]);
	}
#if 1
	if(u32_rb_val&0x4){// bit 2
		rtd_pr_memc_err("===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
		rtd_pr_memc_err("[u12_det_Rim][T,B,L,R]>>[%d,%d,%d,%d]\n",u12_det_Rim[_RIM_TOP],u12_det_Rim[_RIM_BOT],u12_det_Rim[_RIM_LFT],u12_det_Rim[_RIM_RHT]);
		rtd_pr_memc_err("[u12_BBD_fineRim][T,B,L,R]>>[%d,%d,%d,%d]\n",u12_BBD_fineRim[_RIM_TOP],u12_BBD_fineRim[_RIM_BOT],u12_BBD_fineRim[_RIM_LFT],u12_BBD_fineRim[_RIM_RHT]);
		rtd_pr_memc_err("[u1_BBD_roughValid][T,B,L,R]>>[%d,%d,%d,%d]\n",u1_BBD_roughValid[_RIM_TOP],u1_BBD_roughValid[_RIM_BOT],u1_BBD_roughValid[_RIM_LFT],u1_BBD_roughValid[_RIM_RHT]);
		rtd_pr_memc_err("[u12_rim_bound][T,B,L,R]>>[%d,%d,%d,%d]\n",u12_rim_bound[_RIM_TOP],u12_rim_bound[_RIM_BOT],u12_rim_bound[_RIM_LFT],u12_rim_bound[_RIM_RHT]);
		rtd_pr_memc_err("[out_resolution][T,B,L,R]>>[%d,%d,%d,%d]\n",pOutput->u12_out_resolution[_RIM_TOP],pOutput->u12_out_resolution[_RIM_BOT],pOutput->u12_out_resolution[_RIM_LFT],pOutput->u12_out_resolution[_RIM_RHT]);
		rtd_pr_memc_err("u12_BBD_roughRim[u8_k]>>[%d]\n",u12_BBD_roughRim[_RIM_LFT]);
		rtd_pr_memc_err("u12_BBD_fineRim_rb[u8_k]>>[%d]\n",u12_BBD_fineRim[_RIM_LFT]);
		rtd_pr_memc_err("u1_BBD_fineValid[u8_k]>>[%d]\n",u1_BBD_fineValid[_RIM_LFT]);
		rtd_pr_memc_err("u1_BBD_roughValid[u8_k]>>[%d]\n",u1_BBD_roughValid[_RIM_LFT]);
		
	}
#endif


	// Rim qualified for Two-side balancing.
	// Vertical
	if ( (pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT]) > (u12_det_Rim[_RIM_TOP] +  pParam->u12_RimDiffTh) && u8_Rim_Comp_cnt>=8)
	{
		u12_det_Rim[_RIM_BOT] = pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_TOP];
		pOutput->u1_RimTwoSide_balancing=2;//bot > top  //player subtitle or logo
		u8_Rim_Balance_cnt = 12;
	}
	else if (u12_det_Rim[_RIM_TOP] > (pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT] +  pParam->u12_RimDiffTh) && u8_Rim_Comp_cnt>=8)
	{
		u12_det_Rim[_RIM_TOP] = pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT];
		pOutput->u1_RimTwoSide_balancing=0; //top > bot  //player subtitle or logo
		u8_Rim_Balance_cnt = 12;
	}
	else if ((pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT]) > (u12_det_Rim[_RIM_TOP] +  pParam->u12_RimDiffTh)) //moment bot > top
	{
		u12_det_Rim[_RIM_BOT] = pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_TOP];
		if ( Unbalance_last_time == 1 ) {
			pOutput->u1_RimTwoSide_balancing=2;
			u8_Rim_Balance_cnt = 12;
		}
		else {
			pOutput->u1_RimTwoSide_balancing=1;
			u8_Rim_Comp_cnt++;
			u8_Rim_Balance_cnt = u8_Rim_Balance_cnt > 0 ? u8_Rim_Balance_cnt - 1 : 0;
		}
	}
	else if (u12_det_Rim[_RIM_TOP] > (pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT] +  pParam->u12_RimDiffTh)) // moment  top > bot
	{
		u12_det_Rim[_RIM_TOP] = pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT];
		if ( Unbalance_last_time == 1 ) {
			pOutput->u1_RimTwoSide_balancing=0;
			u8_Rim_Balance_cnt = 12;
		}
		else {
			pOutput->u1_RimTwoSide_balancing=1; 
			u8_Rim_Comp_cnt++;
			u8_Rim_Balance_cnt = u8_Rim_Balance_cnt > 0 ? u8_Rim_Balance_cnt - 1 : 0;
		}
	}
	else  //top = bot
	{
		pOutput->u1_RimTwoSide_balancing=1;
		u8_Rim_Comp_cnt=0;
		u8_Rim_Balance_cnt = u8_Rim_Balance_cnt > 0 ? u8_Rim_Balance_cnt - 1 : 0;
	}

	// Hortizontal
	/*
	if ((pOutput->u12_out_resolution[_RIM_RHT] - u12_det_Rim[_RIM_RHT]) > (u12_det_Rim[_RIM_LFT] + pParam->u12_RimDiffTh))
	{
		u12_det_Rim[_RIM_RHT] = (pOutput->u12_out_resolution[_RIM_RHT] - u12_det_Rim[_RIM_LFT]);
	}
	else if (u12_det_Rim[_RIM_LFT] > (pOutput->u12_out_resolution[_RIM_RHT] - u12_det_Rim[_RIM_RHT] + pParam->u12_RimDiffTh))
	{
		u12_det_Rim[_RIM_LFT] = (pOutput->u12_out_resolution[_RIM_RHT] - u12_det_Rim[_RIM_RHT]);
	}
	*/
	// bound protect.
	if( !(u32_rb_val & 0x8)){// bit 3
		pOutput->u12_Rim_Cur[_RIM_LFT] = (u12_det_Rim[_RIM_LFT] >= u12_rim_bound[_RIM_LFT])? pOutput->u12_out_resolution[_RIM_LFT] : u12_det_Rim[_RIM_LFT];
		pOutput->u12_Rim_Cur[_RIM_RHT] = (u12_det_Rim[_RIM_RHT] <= u12_rim_bound[_RIM_RHT])? pOutput->u12_out_resolution[_RIM_RHT] : u12_det_Rim[_RIM_RHT];
		pOutput->u12_Rim_Cur[_RIM_TOP] = (u12_det_Rim[_RIM_TOP] >= u12_rim_bound[_RIM_TOP])? pOutput->u12_out_resolution[_RIM_TOP] : u12_det_Rim[_RIM_TOP];
		pOutput->u12_Rim_Cur[_RIM_BOT] = (u12_det_Rim[_RIM_BOT] <= u12_rim_bound[_RIM_BOT])? pOutput->u12_out_resolution[_RIM_BOT] : u12_det_Rim[_RIM_BOT];	
	

		//-----sync  from merlin2-------------------
		if ((_ABS_DIFF_(pOutput->u12_Rim_Cur[_RIM_RHT] , pOutput->u12_Rim_Cur[_RIM_LFT]) < _ABS_DIFF_(pOutput->u12_Rim_Cur[_RIM_BOT] , pOutput->u12_Rim_Cur[_RIM_TOP])*4/5) )
		{
				pOutput->u12_Rim_Cur[_RIM_LFT] =  pOutput->u12_out_resolution[_RIM_LFT];
				pOutput->u12_Rim_Cur[_RIM_RHT] =  pOutput->u12_out_resolution[_RIM_RHT];
				pOutput->u12_Rim_Cur[_RIM_TOP] =  pOutput->u12_out_resolution[_RIM_TOP];
				pOutput->u12_Rim_Cur[_RIM_BOT] =  pOutput->u12_out_resolution[_RIM_BOT];
		}
		//need change to any blocknum case.
		apl_avg = (unsigned int)(s_pContext->_output_read_comreg.u26_me_aAPLi_rb + s_pContext->_output_read_comreg.u26_me_aAPLp_rb)/(240*135*2);
		if (apl_avg < 0x10)
		{
			pOutput->u12_Rim_Cur[_RIM_LFT] =  pOutput->u12_out_resolution[_RIM_LFT];
			pOutput->u12_Rim_Cur[_RIM_RHT] =  pOutput->u12_out_resolution[_RIM_RHT];
			pOutput->u12_Rim_Cur[_RIM_TOP] =  pOutput->u12_out_resolution[_RIM_TOP];
			if(apl_avg<0xc) //syn from : patch 56683 for #005 bot rim detection issue
				pOutput->u12_Rim_Cur[_RIM_BOT] =  pOutput->u12_out_resolution[_RIM_BOT];
		}

	}
	else{
		pOutput->u12_Rim_Cur[_RIM_LFT] =  u12_det_Rim[_RIM_LFT];
		pOutput->u12_Rim_Cur[_RIM_RHT] =  u12_det_Rim[_RIM_RHT];
		pOutput->u12_Rim_Cur[_RIM_TOP] =  u12_det_Rim[_RIM_TOP];
		pOutput->u12_Rim_Cur[_RIM_BOT] =  u12_det_Rim[_RIM_BOT];
	}
#if 1
	if(u32_rb_val&0x4){// bit 2
		rtd_pr_memc_err("===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
		rtd_pr_memc_err("===== apl_avg detection=========\n");	
		rtd_pr_memc_err("apl_avg >> %d, %d \n",apl_avg, pOutput->u1_RimTwoSide_balancing);
		rtd_pr_memc_err("===== u12_det_Rim =========\n");	
		rtd_pr_memc_err("res_BOT_size >> %d,%d,%d\n",u12_det_Rim[_RIM_BOT],pOutput->u12_out_resolution[_RIM_BOT],(pOutput->u12_out_resolution[_RIM_BOT] - u12_det_Rim[_RIM_BOT]));
		rtd_pr_memc_err("res_TOP_size >> %d,%d,%d\n",u12_det_Rim[_RIM_TOP],pOutput->u12_out_resolution[_RIM_TOP],(u12_det_Rim[_RIM_TOP] - pOutput->u12_out_resolution[_RIM_TOP]));
		rtd_pr_memc_err("res_LFT_size >> %d,%d,%d\n",u12_det_Rim[_RIM_LFT],pOutput->u12_out_resolution[_RIM_LFT],(u12_det_Rim[_RIM_LFT] - pOutput->u12_out_resolution[_RIM_LFT]));
		rtd_pr_memc_err("res_RHT_size >> %d,%d,%d\n",u12_det_Rim[_RIM_RHT],pOutput->u12_out_resolution[_RIM_RHT],(pOutput->u12_out_resolution[_RIM_RHT] - u12_det_Rim[_RIM_RHT]));
		rtd_pr_memc_err("===== rim_cur =========\n");
		rtd_pr_memc_err("cur_BOT_size >> %d\n",pOutput->u12_Rim_Cur[_RIM_BOT]);
		rtd_pr_memc_err("cur_TOP_size >> %d\n",pOutput->u12_Rim_Cur[_RIM_TOP]);
		rtd_pr_memc_err("cur_LFT_size >> %d\n",pOutput->u12_Rim_Cur[_RIM_LFT]);
		rtd_pr_memc_err("cur_RHT_size >> %d\n",pOutput->u12_Rim_Cur[_RIM_RHT]);
	}
#endif
	
	for (u8_k = 0; u8_k < _RIM_NUM; u8_k++)
	{
		pOutput->u12_Rim_Target[u8_k] = pOutput->u12_Rim_Cur[u8_k];
		pOutput->u12_Rim_Result[u8_k] = pOutput->u12_Rim_Pre[u8_k];
	}
	Unbalance_last_time = pOutput->u1_RimTwoSide_balancing == 1 && u8_Rim_Balance_cnt == 0 ? 0 : 1;
}

VOID RimCtrl_FastOut(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
/*	const _PQLCONTEXT *s_pContext = GetPQLContext();*/

	int i = 0;
	for (i = 0; i < _RIM_NUM; i ++)
	{
		if (i == _RIM_LFT || i == _RIM_TOP)
		{
			if (pOutput->u12_Rim_Target[i] < pOutput->u12_Rim_Pre[i])
			{
				pOutput->s8_BiggerCnt[i] =  pOutput->s8_BiggerCnt[i]+1;
			}
			else
			{
				pOutput->s8_BiggerCnt[i] =  _MAX_((pOutput->s8_BiggerCnt[i]-2), 0);
			}
			
		}
		else // if (i == _RIM_RHT || i == _RIM_BOT)
		{
			if (pOutput->u12_Rim_Target[i] > pOutput->u12_Rim_Pre[i])
			{
				pOutput->s8_BiggerCnt[i] = pOutput->s8_BiggerCnt[i]+1;
			}
			else
			{
				pOutput->s8_BiggerCnt[i] = _MAX_((pOutput->s8_BiggerCnt[i]-2),0);
			}
		}	
		// if change rim?
		if (pOutput->s8_BiggerCnt[i] >= pParam->u8_RimBiggerTh)
		{
			pOutput->u12_Rim_Result[i] = pOutput->u12_Rim_Target[i];
			pOutput->u1_RimChange      = 1;
			pOutput->s8_BiggerCnt[i]   = 0;
		}
	}
}

VOID RimCtrl_SlowIn(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	int i = 0;
	unsigned char  u8_thr_coef = 1;
	unsigned char  u8_dyn_thr;
	unsigned char  u8_rim_iir = 0;
	static unsigned char u8_sc_cnt = 0;
	unsigned char u1_Hist_SC_flg;

	for (i = 0; i < _RIM_NUM; i ++)
	{		
		if (i == _RIM_LFT || i == _RIM_TOP)
		{
			if (pOutput->u12_Rim_Target[i] > pOutput->u12_Rim_Pre[i])
			{
				pOutput->s8_SmallerCnt[i] = _CLIP_(pOutput->s8_SmallerCnt[i]+1,-128,126);
			}
			else
			{
				pOutput->s8_SmallerCnt[i] = _MAX_((pOutput->s8_SmallerCnt[i]-2),0);
			}

		}
		else // if (i == _RIM_RHT || i == _RIM_BOT)
		{
			if (pOutput->u12_Rim_Target[i] < pOutput->u12_Rim_Pre[i])
			{
				pOutput->s8_SmallerCnt[i] = _CLIP_(pOutput->s8_SmallerCnt[i]+1,-128,126);
			}
			else
			{
				pOutput->s8_SmallerCnt[i] = _MAX_((pOutput->s8_SmallerCnt[i]-2),0);
			}
		}	
		
		if (i < 2)
		{
			u8_thr_coef = 1;   //default u_thr_coef need consider again.
		}
		else  // right/left
		{
			u8_thr_coef =  (1<<pParam->u5_RimLRSmallShift);
		}

		// if change rim
		u1_Hist_SC_flg = (s_pContext->_external_info.u1_Hist_SC_flg == 1) ? 1 :0;		

//		if (s_pContext->_output_read_comreg.u1_sc_status_rb == 1)
		if (u1_Hist_SC_flg == 1)
		{
			u8_dyn_thr = pParam->u8_RimSmallerSCTh * u8_thr_coef;
			u8_sc_cnt = 10;
		}
		else
		{
			u8_dyn_thr = pParam->u8_RimSmallerTh * u8_thr_coef;
			if (u8_sc_cnt > 0)
			{
				u8_sc_cnt--;
			}
		}

		if (pOutput->s8_SmallerCnt[i] >= u8_dyn_thr /*pParam->u8_RimSmallerTh*/)
		{
			u8_rim_iir = pParam->u8_RimIIRAlpha;
			if (u8_sc_cnt > 0)
			{
				if ((i == _RIM_TOP) || (i == _RIM_BOT))
				{
					u8_rim_iir = 255;
				}
			}
			pOutput->u12_Rim_Result[i] = _2Val_RndBlending_(pOutput->u12_Rim_Target[i], pOutput->u12_Rim_Pre[i], u8_rim_iir, 8);
			if((i == _RIM_LFT) || (i == _RIM_RHT)){
				pOutput->u12_Rim_Result[i] = pOutput->u12_Rim_Target[i];
			}
			pOutput->u1_RimChange      = 1;
			pOutput->s8_SmallerCnt[i]  = 0;			
		}		
	}



	
	
}


VOID RimCtrl_ME1_Rim_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	unsigned short me1_blkV_Resolution = 0, me1_pixV_Resolution = 0;	
//	unsigned short me1_pix_rim_oft[_RIM_NUM];

//	me1_pix_rim_oft[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] == pOutput->u12_out_resolution[_RIM_LFT]? 0 : pParam->u8_me_pixRim_shrink;
//	me1_pix_rim_oft[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] == pOutput->u12_out_resolution[_RIM_RHT]? 0 : pParam->u8_me_pixRim_shrink;
//	me1_pix_rim_oft[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] == pOutput->u12_out_resolution[_RIM_TOP]? 0 : pParam->u8_me_pixRim_shrink;
//	me1_pix_rim_oft[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] == pOutput->u12_out_resolution[_RIM_BOT]? 0 : pParam->u8_me_pixRim_shrink;

	pOutput->u12_me1_lbme_rim[_RIM_LFT] = (pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me1_pixH_sft_bit);// + pParam->u8_lbme_pixRim_shrink;
	pOutput->u12_me1_lbme_rim[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me1_pixH_sft_bit);// - pParam->u8_lbme_pixRim_shrink;
	pOutput->u12_me1_lbme_rim[_RIM_TOP] = (pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me1_pixV_sft_bit) + pParam->u8_lbme_pixRim_shrink;
	pOutput->u12_me1_lbme_rim[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me1_pixV_sft_bit) - pParam->u8_lbme_pixRim_shrink;

	pOutput->u12_me1_vbuf_pix_rim[_RIM_LFT] = (pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me1_pixH_sft_bit);// + pParam->u8_me_pixRim_shrink;
	pOutput->u12_me1_vbuf_pix_rim[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me1_pixH_sft_bit);// - pParam->u8_me_pixRim_shrink;
	pOutput->u12_me1_vbuf_pix_rim[_RIM_TOP] = (pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me1_pixV_sft_bit) + pParam->u8_me_pixRim_shrink;
	pOutput->u12_me1_vbuf_pix_rim[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me1_pixV_sft_bit) - pParam->u8_me_pixRim_shrink;

	if(pOutput->u1_RimCompflag){
		if(pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] > pParam->u8_lbme_pixRim_comp){
			pOutput->u12_me1_lbme_rim[_RIM_TOP] = pOutput->u12_me1_lbme_rim[_RIM_TOP] - pParam->u8_lbme_pixRim_comp;
			pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] = pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] - pParam->u8_lbme_pixRim_comp;
		}

		if(pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] > (539-pParam->u8_lbme_pixRim_comp)){
			pOutput->u12_me1_lbme_rim[_RIM_BOT] = pOutput->u12_me1_lbme_rim[_RIM_BOT] - pParam->u8_lbme_pixRim_comp;
			pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] = pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] + pParam->u8_lbme_pixRim_comp;
		}
	}

	pOutput->u12_me1_vbuf_blk_rim[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me1_blkH_sft_bit;
	pOutput->u12_me1_vbuf_blk_rim[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me1_blkH_sft_bit;
	pOutput->u12_me1_vbuf_blk_rim[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me1_blkV_sft_bit;
	pOutput->u12_me1_vbuf_blk_rim[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me1_blkV_sft_bit;

	// meander top/bottom rim.
	me1_blkV_Resolution = pOutput->u12_out_resolution[_RIM_BOT] >> pOutput->u8_me1_blkV_sft_bit;
	pOutput->u12_me1_vbuf_meander_blk_rim[_RIM_TOP] = me1_blkV_Resolution - pOutput->u12_me1_vbuf_blk_rim[_RIM_BOT];
	pOutput->u12_me1_vbuf_meander_blk_rim[_RIM_BOT] = me1_blkV_Resolution - pOutput->u12_me1_vbuf_blk_rim[_RIM_TOP];

	me1_pixV_Resolution = pOutput->u12_out_resolution[_RIM_BOT] >> pOutput->u8_me1_pixV_sft_bit;
	pOutput->u12_me1_vbuf_meander_pix_rim[_RIM_TOP] = me1_pixV_Resolution - pOutput->u12_me1_vbuf_pix_rim[_RIM_BOT];
	pOutput->u12_me1_vbuf_meander_pix_rim[_RIM_BOT] = me1_pixV_Resolution - pOutput->u12_me1_vbuf_pix_rim[_RIM_TOP];
}

VOID RimCtrl_ME2_Rim_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	//unsigned short me2_pix_rim_oft[_RIM_NUM];
	//me2_pix_rim_oft[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] == pOutput->u12_out_resolution[_RIM_LFT]? 0 : pParam->u8_me_pixRim_shrink;
	//me2_pix_rim_oft[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] == pOutput->u12_out_resolution[_RIM_RHT]? 0 : pParam->u8_me_pixRim_shrink;
	//me2_pix_rim_oft[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] == pOutput->u12_out_resolution[_RIM_TOP]? 0 : pParam->u8_me_pixRim_shrink;
	//me2_pix_rim_oft[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] == pOutput->u12_out_resolution[_RIM_BOT]? 0 : pParam->u8_me_pixRim_shrink;

	pOutput->u12_me2_lbme_rim[_RIM_LFT] = (pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me2_pixH_sft_bit);// + pParam->u8_lbme_pixRim_shrink;
	pOutput->u12_me2_lbme_rim[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me2_pixH_sft_bit);// - pParam->u8_lbme_pixRim_shrink;
	pOutput->u12_me2_lbme_rim[_RIM_TOP] = (pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me2_pixV_sft_bit) + pParam->u8_lbme_pixRim_shrink;
	pOutput->u12_me2_lbme_rim[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me2_pixV_sft_bit) - pParam->u8_lbme_pixRim_shrink;

	
	pOutput->u12_me2_vbuf_pix_rim[_RIM_LFT] = (pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me2_pixH_sft_bit);// + pParam->u8_me_pixRim_shrink;
	pOutput->u12_me2_vbuf_pix_rim[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me2_pixH_sft_bit);// - pParam->u8_me_pixRim_shrink;
	pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] = (pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me2_pixV_sft_bit) + pParam->u8_me_pixRim_shrink;
	pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me2_pixV_sft_bit) - pParam->u8_me_pixRim_shrink;

	if(pOutput->u1_RimCompflag){
		if(pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] > pParam->u8_lbme_pixRim_comp){
			pOutput->u12_me2_lbme_rim[_RIM_TOP] = pOutput->u12_me2_lbme_rim[_RIM_TOP] - pParam->u8_lbme_pixRim_comp;
			pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] = pOutput->u12_me2_vbuf_pix_rim[_RIM_TOP] - pParam->u8_lbme_pixRim_comp;
		}

		if(pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] < (539-pParam->u8_lbme_pixRim_comp)){
			pOutput->u12_me2_lbme_rim[_RIM_BOT] = pOutput->u12_me2_lbme_rim[_RIM_BOT] + pParam->u8_lbme_pixRim_comp;
			pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] = pOutput->u12_me2_vbuf_pix_rim[_RIM_BOT] + pParam->u8_lbme_pixRim_comp;
		}
	}

	pOutput->u12_me2_vbuf_ph_blk_rim[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me2_BlkH_sft_bit;
	pOutput->u12_me2_vbuf_ph_blk_rim[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me2_BlkH_sft_bit;
	pOutput->u12_me2_vbuf_ph_blk_rim[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me2_BlkV_sft_bit;
	pOutput->u12_me2_vbuf_ph_blk_rim[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me2_BlkV_sft_bit;

	pOutput->u12_me2_vbuf_pfv_blk_rim[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_me2_BlkH_sft_bit;
	pOutput->u12_me2_vbuf_pfv_blk_rim[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_me2_BlkH_sft_bit;
	pOutput->u12_me2_vbuf_pfv_blk_rim[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_me2_BlkV_sft_bit;
	pOutput->u12_me2_vbuf_pfv_blk_rim[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_me2_BlkV_sft_bit;


	#if 0
	printk("===== RimCtrl_ME2_Rim_Convert =========\n");	
	printk("u1_RimCompflag >> [%d,%d]\n",pOutput->u8_me2_BlkH_sft_bit,pOutput->u8_me2_BlkV_sft_bit);
	printk("Pxl[_RIM_BOT] >> [%d,%d,%d,%d]\n",pOutput->u12_Rim_Result[_RIM_LFT],pOutput->u12_Rim_Result[_RIM_RHT] ,pOutput->u12_Rim_Result[_RIM_TOP],pOutput->u12_Rim_Result[_RIM_BOT]);
	printk("================\n");
	#endif


	
}


VOID RimCtrl_MC_Rim_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
/*	const _PQLCONTEXT *s_pContext = GetPQLContext();*/
	pOutput->u12_mc_rim0[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] + pParam->u8_mc_ptRim_shrink;
	pOutput->u12_mc_rim0[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] - pParam->u8_mc_ptRim_shrink;
	pOutput->u12_mc_rim0[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] + pParam->u8_mc_ptRim_shrink;
	pOutput->u12_mc_rim0[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] - pParam->u8_mc_ptRim_shrink;

	if(Get_DISP_ACT_END_VPOS()<=1088){//4k1k		
		pOutput->u12_mc_rim1[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT];
		pOutput->u12_mc_rim1[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT];
		pOutput->u12_mc_rim1[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP]+2;//pOutput->u12_mc_rim1[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP]+2;
		
		if(pOutput->u12_Rim_Result[_RIM_BOT]>0){
			pOutput->u12_mc_rim1[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT]-2;//pOutput->u12_mc_rim1[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT]-2;
		}

		if(pOutput->u12_mc_rim0[_RIM_TOP]==pOutput->u12_mc_rim1[_RIM_TOP])
			pOutput->u12_mc_rim0[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] + pParam->u8_mc_ptRim_shrink;
		if(pOutput->u12_Rim_Result[_RIM_BOT]>0 && (pOutput->u12_mc_rim0[_RIM_BOT]==pOutput->u12_mc_rim1[_RIM_BOT]))
			pOutput->u12_mc_rim0[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] - pParam->u8_mc_ptRim_shrink;
	}
	else{
		pOutput->u12_mc_rim1[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT];
		pOutput->u12_mc_rim1[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT];
		pOutput->u12_mc_rim1[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP];
		pOutput->u12_mc_rim1[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT];
	}
}

VOID RimCtrl_Logo_Rim_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
/*	const _PQLCONTEXT *s_pContext = GetPQLContext();*/

	unsigned char u8_logoBlk_rim_oft[_RIM_NUM], u8_logoPix_rim_oft[_RIM_NUM];

	//blk rim
	u8_logoBlk_rim_oft[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] == pOutput->u12_out_resolution[_RIM_LFT]? 0 : pParam->u8_LogoBlkRimHOffset;
	u8_logoBlk_rim_oft[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] == pOutput->u12_out_resolution[_RIM_RHT]? 0 : pParam->u8_LogoBlkRimHOffset;
	u8_logoBlk_rim_oft[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] == pOutput->u12_out_resolution[_RIM_TOP]? 0 : pParam->u8_LogoBlkRimVOffset;
	u8_logoBlk_rim_oft[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] == pOutput->u12_out_resolution[_RIM_BOT]? 0 : pParam->u8_LogoBlkRimVOffset;

	pOutput->u8_LogoRim_Blk[_RIM_LFT] = (pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_logo_blkH_sft_bit) + u8_logoBlk_rim_oft[_RIM_LFT];
	pOutput->u8_LogoRim_Blk[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_logo_blkH_sft_bit) - u8_logoBlk_rim_oft[_RIM_RHT];
	pOutput->u8_LogoRim_Blk[_RIM_TOP] = (pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_logo_blkV_sft_bit) + u8_logoBlk_rim_oft[_RIM_TOP];
	//pOutput->u8_LogoRim_Blk[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_blkV_sft_bit) - u8_logoBlk_rim_oft[_RIM_BOT];
	if(pOutput->u1_RimTwoSide_balancing != 1){ // non-balancing case
		pOutput->u8_LogoRim_Blk[_RIM_BOT] = ( pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_blkV_sft_bit) + u8_logoBlk_rim_oft[_RIM_BOT];
	}
	else if(pOutput->u1_RimCompflag){
		pOutput->u8_LogoRim_Blk[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_blkV_sft_bit);
	}else{
		pOutput->u8_LogoRim_Blk[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_blkV_sft_bit)- u8_logoBlk_rim_oft[_RIM_BOT];
	}
	pOutput->u8_LogoRim_Blk[_RIM_BOT] = _CLIP_(pOutput->u8_LogoRim_Blk[_RIM_BOT], 0, 132);
	

	//pixel rim
	u8_logoPix_rim_oft[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] == pOutput->u12_out_resolution[_RIM_LFT]? 0 : pParam->u8_LogoPixRimHOffset;
	u8_logoPix_rim_oft[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] == pOutput->u12_out_resolution[_RIM_RHT]? 0 : pParam->u8_LogoPixRimHOffset;
	u8_logoPix_rim_oft[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] == pOutput->u12_out_resolution[_RIM_TOP]? 0 : pParam->u8_LogoPixRimVOffset;
	u8_logoPix_rim_oft[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] == pOutput->u12_out_resolution[_RIM_BOT]? 0 : pParam->u8_LogoPixRimVOffset;

	pOutput->u12_LogoRim_Pxl[_RIM_LFT] = (pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_logo_pixH_sft_bit) + u8_logoPix_rim_oft[_RIM_LFT];
	pOutput->u12_LogoRim_Pxl[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_logo_pixH_sft_bit) - u8_logoPix_rim_oft[_RIM_RHT];
	pOutput->u12_LogoRim_Pxl[_RIM_TOP] = (pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_logo_pixV_sft_bit) + u8_logoPix_rim_oft[_RIM_TOP];
	//pOutput->u12_LogoRim_Pxl[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_pixV_sft_bit) - u8_logoPix_rim_oft[_RIM_BOT];
	if(pOutput->u1_RimTwoSide_balancing != 1){ // non-balancing case
		pOutput->u12_LogoRim_Pxl[_RIM_BOT] = ( pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_pixV_sft_bit) + u8_logoPix_rim_oft[_RIM_BOT];
	}
	else if(pOutput->u1_RimCompflag){
		pOutput->u12_LogoRim_Pxl[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_pixV_sft_bit);
	}else{
		pOutput->u12_LogoRim_Pxl[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_logo_pixV_sft_bit)- u8_logoPix_rim_oft[_RIM_BOT];
	}
	pOutput->u12_LogoRim_Pxl[_RIM_BOT] = _CLIP_(pOutput->u12_LogoRim_Pxl[_RIM_BOT], 0, 531);

	#if 0
	printk("===== RimCtrl_Logo_Rim_Convert =========\n");	
	printk("u1_RimCompflag >> [%d]\n",pOutput->u1_RimCompflag);
	printk("Pxl[_RIM_BOT] >> [%d,%d],[%d],[%d]\n",pOutput->u8_LogoRim_Pxl[_RIM_BOT],pOutput->u12_Rim_Result[_RIM_BOT] ,pOutput->u8_logo_pixV_sft_bit,u8_logoPix_rim_oft[_RIM_BOT]);
	printk("Blk[_RIM_BOT] >> [%d,%d],[%d],[%d]\n",pOutput->u12_LogoRim_Blk[_RIM_BOT],pOutput->u12_Rim_Result[_RIM_BOT],pOutput->u8_logo_blkV_sft_bit,u8_logoBlk_rim_oft[_RIM_BOT]);
	printk("===== rim cur =========\n");
	#endif
	
}

VOID RimCtrl_Film5RgnPos_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
/*		const _PQLCONTEXT *s_pContext = GetPQLContext();*/
	unsigned char  u4_Res_H_Divide = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 2 : 
		                     			(pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x540) ? 3 :
										(pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 1 : 0;
    unsigned char  u4_Res_V_Divide = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 ) ? 1 : 
	                         			(pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 2 : 0;

	unsigned short u12_filmResolutionB = pOutput->u12_out_resolution[_RIM_BOT]>>u4_Res_V_Divide;
	unsigned short u12_filmResolutionR = pOutput->u12_out_resolution[_RIM_RHT]>>u4_Res_H_Divide;

	unsigned short u10_film5Region1_v0 = 0;
	unsigned short u10_film5Region1_v1 = 0;
	unsigned short u10_film5Region2_v0 = 0;
	unsigned short u10_film5Region2_v1 = 0;
	unsigned short u10_film5Region3_h0 = 0;
	unsigned short u10_film5Region3_h1 = 0;
	unsigned short u10_film5Region4_h0 = 0;
	unsigned short u10_film5Region4_h1 = 0;

	//unsigned int u32_rb_val = 0;
	
	// K2:PQL_ReadHDF
	// Need to confirm t/b/l/r order
	//ReadRegister((KME_IPME_KME_IPME_40_reg), 0, 31, &u32_rb_val);//top
	u10_film5Region1_v0 = 0x4;//(u32_rb_val) & 0x3ff;
	u10_film5Region1_v1 = 0x58;//(u32_rb_val>>10) & 0x3ff;
	//ReadRegister((KME_IPME_KME_IPME_44_reg), 0, 31, &u32_rb_val);//bottom
	u10_film5Region2_v0 = 0x1bc;//(u32_rb_val) & 0x3ff;
	u10_film5Region2_v1 = 0x20d;//(u32_rb_val>>10) & 0x3ff;
	//ReadRegister((KME_IPME_KME_IPME_48_reg), 0, 31, &u32_rb_val);//left
	u10_film5Region3_h0 = 0x2;//(u32_rb_val) & 0x3ff;
	u10_film5Region3_h1 = 0x60;//(u32_rb_val>>10) & 0x3ff;
	//ReadRegister((KME_IPME_KME_IPME_4C_reg), 0, 31, &u32_rb_val);//bottom
	u10_film5Region4_h0 = 0x140;//(u32_rb_val) & 0x3ff;
	u10_film5Region4_h1 = 0x1d4;//(u32_rb_val>>10) & 0x3ff;


	//top
	{	
		unsigned short u10_rgnHWshift = ((pOutput->u12_ipme_rim[_RIM_TOP]>u10_film5Region1_v0) ? (pOutput->u12_ipme_rim[_RIM_TOP]-u10_film5Region1_v0) : 0);
		u10_film5Region1_v0 += u10_rgnHWshift + 13;
		u10_film5Region1_v1 += u10_rgnHWshift;
		u10_film5Region1_v1 = (u10_film5Region1_v1<u10_film5Region1_v0) ? u10_film5Region1_v0 : u10_film5Region1_v1;
	}

	//bot
	{
		unsigned short u10_rgnHWshift = ((pOutput->u12_ipme_rim[_RIM_BOT]<u10_film5Region2_v1) ? (u10_film5Region2_v1-pOutput->u12_ipme_rim[_RIM_BOT]) : 0);
		u10_film5Region2_v0 -= u10_rgnHWshift;
		u10_film5Region2_v1 -= u10_rgnHWshift;//(u10_rgnHWshift + 13);
		u10_film5Region2_v0 = (u10_film5Region2_v0>u10_film5Region2_v1) ? u10_film5Region2_v1 : u10_film5Region2_v0;
	}

	//left
	{
		unsigned short u10_rgnHWshift = ((pOutput->u12_ipme_rim[_RIM_LFT]>u10_film5Region3_h0) ? (pOutput->u12_ipme_rim[_RIM_LFT]-u10_film5Region3_h0) : 0);
		u10_film5Region3_h0 += u10_rgnHWshift + 13;
		u10_film5Region3_h1 += u10_rgnHWshift;
		u10_film5Region3_h1 = (u10_film5Region3_h1<u10_film5Region3_h0) ? u10_film5Region3_h0 : u10_film5Region3_h1;
	}
	
	//right
	{
		unsigned short u10_rgnHWshift = ((pOutput->u12_ipme_rim[_RIM_RHT]<u10_film5Region4_h1) ? (u10_film5Region4_h1-pOutput->u12_ipme_rim[_RIM_RHT]) : 0);
		u10_film5Region4_h0 -= u10_rgnHWshift;
		u10_film5Region4_h1 -= (u10_rgnHWshift + 4);
		u10_film5Region4_h0 = (u10_film5Region4_h0>u10_film5Region4_h1) ? u10_film5Region4_h1 : u10_film5Region4_h0;
	}


	u10_film5Region1_v0 = _CLIP_UBOUND_(u10_film5Region1_v0, u12_filmResolutionB-1);
	u10_film5Region1_v1 = _CLIP_UBOUND_(u10_film5Region1_v1, u12_filmResolutionB-1);
	u10_film5Region2_v0 = _CLIP_UBOUND_(u10_film5Region2_v0, u12_filmResolutionB-1);
	u10_film5Region2_v1 = _CLIP_UBOUND_(u10_film5Region2_v1, u12_filmResolutionB-1);
	u10_film5Region3_h0 = _CLIP_UBOUND_(u10_film5Region3_h0, u12_filmResolutionR-1);
	u10_film5Region3_h1 = _CLIP_UBOUND_(u10_film5Region3_h1, u12_filmResolutionR-1);
	u10_film5Region4_h0 = _CLIP_UBOUND_(u10_film5Region4_h0, u12_filmResolutionR-1);
	u10_film5Region4_h1 = _CLIP_UBOUND_(u10_film5Region4_h1, u12_filmResolutionR-1);


 	pOutput->u12_5Rgn1_v0 = u10_film5Region1_v0;
	pOutput->u12_5Rgn1_v1 = u10_film5Region1_v1;
	pOutput->u12_5Rgn2_v0 = u10_film5Region2_v0;
	pOutput->u12_5Rgn2_v1 = u10_film5Region2_v1;
	pOutput->u12_5Rgn3_h0 = u10_film5Region3_h0;
	pOutput->u12_5Rgn3_h1 = u10_film5Region3_h1;
	pOutput->u12_5Rgn4_h0 = u10_film5Region4_h0;
	pOutput->u12_5Rgn4_h1 = u10_film5Region4_h1;

	// update 12rgn rim
     	pOutput->u12_12Rgn_v0 = u10_film5Region1_v0;
	pOutput->u12_12Rgn_v1 = u10_film5Region1_v1;
	pOutput->u12_12Rgn_v2 = u10_film5Region2_v0;
	pOutput->u12_12Rgn_v3 = u10_film5Region2_v1;
	pOutput->u12_12Rgn_h0 = u10_film5Region3_h0;
	pOutput->u12_12Rgn_h1 = u10_film5Region3_h1;
	pOutput->u12_12Rgn_h2 = (u10_film5Region3_h1+u10_film5Region4_h0)/2;
	pOutput->u12_12Rgn_h3 = u10_film5Region4_h0;
	pOutput->u12_12Rgn_h4 = u10_film5Region4_h1;	
 
     
}

VOID RimCtrl_Film_Rim_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{

	unsigned char  u4_Res_H_Divide = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 2 : 
		                     			(pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080 ||pOutput->u4_outResolution_pre == _PQL_OUT_3840x540) ? 3 :
										(pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 1 : 0;
    unsigned char  u4_Res_V_Divide = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 ) ? 1 : 
	                         			(pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 2 : 0;

	//pOutput->u12_ipme_rim[_RIM_LFT] = pOutput->u12_me1_vbuf_pix_rim[_RIM_LFT]/2+2; //two pixel
	//pOutput->u12_ipme_rim[_RIM_RHT] = pOutput->u12_me1_vbuf_pix_rim[_RIM_RHT]/2-2; //two pixel
	//pOutput->u12_ipme_rim[_RIM_TOP] = pOutput->u12_me1_vbuf_pix_rim[_RIM_TOP]+2;
	//pOutput->u12_ipme_rim[_RIM_BOT] = pOutput->u12_me1_vbuf_pix_rim[_RIM_BOT]-2;

	pOutput->u12_ipme_rim[_RIM_LFT] = ((pOutput->u12_Rim_Result[_RIM_LFT] + (1<<u4_Res_H_Divide)/2) >> u4_Res_H_Divide)+2; 
	pOutput->u12_ipme_rim[_RIM_RHT] = (pOutput->u12_Rim_Result[_RIM_RHT] >> u4_Res_H_Divide)-2; 
	pOutput->u12_ipme_rim[_RIM_TOP] = ((pOutput->u12_Rim_Result[_RIM_TOP] + (1<<u4_Res_V_Divide)/2) >> u4_Res_V_Divide)+2;
	pOutput->u12_ipme_rim[_RIM_BOT] = (pOutput->u12_Rim_Result[_RIM_BOT] >> u4_Res_V_Divide)-2;



	RimCtrl_Film5RgnPos_Convert(pParam, pOutput);
}

VOID RimCtrl_Dh_Rim_Convert(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	pOutput->u9_dh_blk_oftrim[_RIM_LFT] = pOutput->u12_me1_vbuf_blk_rim[_RIM_LFT];
	pOutput->u9_dh_blk_oftrim[_RIM_RHT] = (pOutput->u12_out_resolution[_RIM_RHT] >> pOutput->u8_me1_blkH_sft_bit) - pOutput->u12_me1_vbuf_blk_rim[_RIM_RHT];
	pOutput->u9_dh_blk_oftrim[_RIM_TOP] = pOutput->u12_me1_vbuf_blk_rim[_RIM_TOP];
	pOutput->u9_dh_blk_oftrim[_RIM_BOT] = (pOutput->u12_out_resolution[_RIM_BOT] >> pOutput->u8_me1_blkV_sft_bit) - pOutput->u12_me1_vbuf_blk_rim[_RIM_BOT];

	pOutput->u9_dh_blk_rim[_RIM_LFT] = pOutput->u12_Rim_Result[_RIM_LFT] >> pOutput->u8_dh_BlkH_sft_bit;
	pOutput->u9_dh_blk_rim[_RIM_RHT] = pOutput->u12_Rim_Result[_RIM_RHT] >> pOutput->u8_dh_BlkH_sft_bit;
	pOutput->u9_dh_blk_rim[_RIM_TOP] = pOutput->u12_Rim_Result[_RIM_TOP] >> pOutput->u8_dh_BlkV_sft_bit;
	pOutput->u9_dh_blk_rim[_RIM_BOT] = pOutput->u12_Rim_Result[_RIM_BOT] >> pOutput->u8_dh_BlkV_sft_bit;
}

VOID RimCtrl_Output(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();

	if((pOutput->u12_mc_rim1[_RIM_LFT] != pOutput->u12_Rim_Result[_RIM_LFT])||
		(pOutput->u12_mc_rim1[_RIM_RHT] != pOutput->u12_Rim_Result[_RIM_RHT])||
		(pOutput->u12_mc_rim1[_RIM_TOP] != pOutput->u12_Rim_Result[_RIM_TOP])||
		(pOutput->u12_mc_rim1[_RIM_BOT] != pOutput->u12_Rim_Result[_RIM_BOT]))
	{
		pOutput->u1_RimChange = 1;
	}
		
	
	if (pOutput->u1_RimChange == 1)
	{
#if 0
		//Vetical.
		if (pOutput->u12_out_resolution[_RIM_BOT] == 2159)
		{
			pOutput->u8_me1_pixV_sft_bit    = 2; // 540
			pOutput->u8_me1_blkV_sft_bit    = 3; // 270

			pOutput->u8_me2_pixV_sft_bit    = 3; // 270
			pOutput->u8_me2_BlkV_sft_bit    = 4; // 135
			
			pOutput->u8_dh_BlkV_sft_bit 	= 3; // 270

			pOutput->u8_logo_pixV_sft_bit   = 2; // 540
			pOutput->u8_logo_blkV_sft_bit   = 4; // 135
		}
		else if (pOutput->u12_out_resolution[_RIM_BOT] == 1079)
		{
			pOutput->u8_me1_pixV_sft_bit    = 1; // 540
			pOutput->u8_me1_blkV_sft_bit    = 3; // 135

			pOutput->u8_me2_pixV_sft_bit    = 2; // 270
			pOutput->u8_me2_BlkV_sft_bit    = 3; // 135

			pOutput->u8_dh_BlkV_sft_bit 	= 3; // 135

			pOutput->u8_logo_pixV_sft_bit   = 1; // 540
			pOutput->u8_logo_blkV_sft_bit   = 3; // 135
		}
		else // if (pOutput->u12_out_resolution[_RIM_BOT] == 539)
		{
			pOutput->u8_me1_pixV_sft_bit    = 1; // 270
			pOutput->u8_me1_blkV_sft_bit    = 2; // 135

			pOutput->u8_me2_pixV_sft_bit    = 2; // 135
			pOutput->u8_me2_BlkV_sft_bit    = 3; // 68

			pOutput->u8_dh_BlkV_sft_bit 	= 2; // 135

			pOutput->u8_logo_pixV_sft_bit   = 0; // 540
			pOutput->u8_logo_blkV_sft_bit   = 2; // 135
		}

		// Horizontal.
		if (pOutput->u12_out_resolution[_RIM_RHT] == 1919)
		{
			pOutput->u8_me1_pixH_sft_bit    = 1; // 960
			pOutput->u8_me1_blkH_sft_bit    = 3; // 240

			pOutput->u8_me2_pixH_sft_bit    = 2; // 480
			pOutput->u8_me2_BlkH_sft_bit    = 3; // 240

			pOutput->u8_dh_BlkH_sft_bit 	= 3; // 240

			pOutput->u8_logo_pixH_sft_bit   = 1; // 960
			pOutput->u8_logo_blkH_sft_bit   = 3; // 240
		}
		else // Hactive = 3839
		{
			pOutput->u8_me1_pixH_sft_bit    = 2; // 960
			pOutput->u8_me1_blkH_sft_bit    = 3; // 480

			pOutput->u8_me2_pixH_sft_bit    = 3; // 480
			pOutput->u8_me2_BlkH_sft_bit	= 4; // 240

			pOutput->u8_dh_BlkH_sft_bit 	= 3; // 480

			pOutput->u8_logo_pixH_sft_bit   = 2; // 960
			pOutput->u8_logo_blkH_sft_bit   = 4; // 240
		}
#else
		pOutput->u8_me1_pixH_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me1_pix_hsft;
		pOutput->u8_me1_pixV_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me1_pix_vsft;
		pOutput->u8_me1_blkH_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me1_blk_hsft; 
		pOutput->u8_me1_blkV_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me1_blk_vsft; 

		pOutput->u8_me2_pixH_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me2_pix_hsft;
		pOutput->u8_me2_pixV_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me2_pix_vsft;
		pOutput->u8_me2_BlkH_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me2_blk_hsft; 
		pOutput->u8_me2_BlkV_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_me2_blk_vsft; 

		pOutput->u8_dh_BlkH_sft_bit 	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_dh_blk_hsft;	
		pOutput->u8_dh_BlkV_sft_bit 	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_dh_blk_vsft;

		pOutput->u8_logo_pixH_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_logo_pix_hsft;
		pOutput->u8_logo_pixV_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_logo_pix_vsft;
		pOutput->u8_logo_blkH_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_logo_blk_hsft;	
		pOutput->u8_logo_blkV_sft_bit	= s_pContext->_external_data._stc_memc_module_ratio.u8_in_logo_blk_vsft;

		//LogPrintf(DBG_MSG,"u8_me1_pixH_sft_bit = %d, u8_me1_pixV_sft_bit = %d\r\n",pOutput->u8_me1_pixH_sft_bit,pOutput->u8_me1_pixV_sft_bit);
		//LogPrintf(DBG_MSG,"u8_me1_blkH_sft_bit = %d, u8_me1_blkV_sft_bit = %d\r\n",pOutput->u8_me1_blkH_sft_bit,pOutput->u8_me1_blkV_sft_bit);
		//LogPrintf(DBG_MSG,"u8_me2_pixH_sft_bit = %d, u8_me2_pixV_sft_bit = %d\r\n",pOutput->u8_me2_pixH_sft_bit,pOutput->u8_me2_pixV_sft_bit);
		//LogPrintf(DBG_MSG,"u8_me2_BlkH_sft_bit = %d, u8_me2_BlkV_sft_bit = %d\r\n",pOutput->u8_me2_BlkH_sft_bit,pOutput->u8_me2_BlkV_sft_bit);
		//LogPrintf(DBG_MSG,"u8_dh_BlkH_sft_bit = %d, u8_dh_BlkV_sft_bit = %d\r\n",pOutput->u8_dh_BlkH_sft_bit,pOutput->u8_dh_BlkV_sft_bit);
		//LogPrintf(DBG_MSG,"u8_logo_pixH_sft_bit = %d, u8_logo_pixV_sft_bit = %d\r\n",pOutput->u8_logo_pixH_sft_bit,pOutput->u8_logo_pixV_sft_bit);
		//LogPrintf(DBG_MSG,"u8_logo_blkH_sft_bit = %d, u8_logo_blkV_sft_bit = %d\r\n",pOutput->u8_logo_blkH_sft_bit,pOutput->u8_logo_blkV_sft_bit);
#endif
		RimCtrl_ME1_Rim_Convert(pParam, pOutput);
		RimCtrl_ME2_Rim_Convert(pParam, pOutput);	
		RimCtrl_Logo_Rim_Convert(pParam, pOutput);
		RimCtrl_MC_Rim_Convert(pParam, pOutput);

		RimCtrl_Film_Rim_Convert(pParam, pOutput);
		RimCtrl_Dh_Rim_Convert(pParam, pOutput);
	}
}

VOID Rim_check_protect_ME1(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	int i=0;
	unsigned int Res_H = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1920 :
						 (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 960 : 3840;
    unsigned int Res_V = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 1080 : 
	                     (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160) ? 2160 : 540;		
	unsigned int reg_val_cur[4] = {0};
	unsigned int Res_cur_me1_pix[4] = { (Res_V>>pOutput->u8_me1_pixV_sft_bit)*1/2, (Res_V>>pOutput->u8_me1_pixV_sft_bit)*1/2,(Res_H>>pOutput->u8_me1_pixH_sft_bit)*1/2,(Res_H>>pOutput->u8_me1_pixH_sft_bit)*1/2};
	unsigned int Res_cur_me1_blk[4] = { (Res_V>>pOutput->u8_me1_blkV_sft_bit)*1/2, (Res_V>>pOutput->u8_me1_blkV_sft_bit)*1/2,(Res_H>>pOutput->u8_me1_blkH_sft_bit)*1/2,(Res_H>>pOutput->u8_me1_blkH_sft_bit)*1/2};
	
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_00_reg,0,10, &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_00_reg,11,21, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_04_reg,0,10, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_04_reg,11,21, &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me1_lbme_rim[i] - reg_val_cur[i])>Res_cur_me1_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}

	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_1C_reg,0,10, &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_1C_reg,11,21, &reg_val_cur[_RIM_RHT]);  //rht
	for (i=2;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me1_lbme_rim[i] - reg_val_cur[i])>Res_cur_me1_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,0,9,  &reg_val_cur[_RIM_TOP]); //top
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_44_reg,10,19,  &reg_val_cur[_RIM_BOT]);//bot
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,0,9, &reg_val_cur[_RIM_LFT]); //lft
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_48_reg,10,19, &reg_val_cur[_RIM_RHT]); //rht
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me1_vbuf_pix_rim[i] - reg_val_cur[i])>Res_cur_me1_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_3C_reg,16,24,  &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,18,26,   &reg_val_cur[_RIM_BOT]);  //bot
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,0,8,  &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_40_reg,9,17, &reg_val_cur[_RIM_RHT]);  //rht
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me1_vbuf_blk_rim[i] - reg_val_cur[i])>Res_cur_me1_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,0,8, &reg_val_cur[_RIM_TOP]); //top
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,9,17, &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<2;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me1_vbuf_meander_blk_rim[i] - reg_val_cur[i])>Res_cur_me1_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_20_reg,0,10,  &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_LBME_TOP_KME_LBME_TOP_20_reg,11,21,   &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<2;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me1_vbuf_meander_pix_rim[i] - reg_val_cur[i])>Res_cur_me1_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}
	ReadRegister(KME_VBUF_TOP_KME_VBUF_TOP_84_reg,18,27, &reg_val_cur[_RIM_TOP]);  //top
	if(_ABS_(pOutput->u12_me1_vbuf_meander_pix_rim[_RIM_TOP] - reg_val_cur[_RIM_TOP])>((Res_V>>pOutput->u8_me1_pixV_sft_bit)*1/2))
		pOutput->u1_RimChange = 1;

}
VOID Rim_check_protect_ME2(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	int i=0;
	unsigned int Res_H = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1920 :
						 (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 960 : 3840;
    unsigned int Res_V = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 1080 : 
	                     (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160) ? 2160 : 540;
	unsigned int reg_val_cur[4] = {0};
	unsigned int Res_cur_me2_pix[4] = { (Res_V>>pOutput->u8_me2_pixV_sft_bit)*1/2, (Res_V>>pOutput->u8_me2_pixV_sft_bit)*1/2,(Res_H>>pOutput->u8_me2_pixH_sft_bit)*1/2,(Res_H>>pOutput->u8_me2_pixH_sft_bit)*1/2};
	unsigned int Res_cur_me2_blk[4] = { (Res_V>>pOutput->u8_me2_BlkV_sft_bit)*1/2, (Res_V>>pOutput->u8_me2_BlkV_sft_bit)*1/2,(Res_H>>pOutput->u8_me2_BlkH_sft_bit)*1/2,(Res_H>>pOutput->u8_me2_BlkH_sft_bit)*1/2};
	
	ReadRegister(KME_LBME2_TOP_KME_LBME2_TOP_00_reg,0,10, &reg_val_cur[_RIM_LFT]); //lft
	ReadRegister(KME_LBME2_TOP_KME_LBME2_TOP_00_reg,11,21,   &reg_val_cur[_RIM_RHT]); //rht
	ReadRegister(KME_LBME2_TOP_KME_LBME2_TOP_04_reg,0,10, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_LBME2_TOP_KME_LBME2_TOP_04_reg,11,21,   &reg_val_cur[_RIM_BOT]); //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me2_lbme_rim[i] - reg_val_cur[i])>Res_cur_me2_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
			
	}
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg,12,21,  &reg_val_cur[_RIM_LFT]); // lft
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D4_reg,22,31, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,0,9,   &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_D8_reg,10,19,   &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me2_vbuf_pix_rim[i] - reg_val_cur[i])>Res_cur_me2_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
	}


	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,18,26,  &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,0,8, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,9,17, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_28_reg,18,26,  &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me2_vbuf_ph_blk_rim[i] - reg_val_cur[i])>Res_cur_me2_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
	}

	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,9,17,  &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_20_reg,18,26, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,0,8,  &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_24_reg,9,17,   &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_me2_vbuf_pfv_blk_rim[i] - reg_val_cur[i])>Res_cur_me2_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
	}

}
VOID Rim_check_protect_Logo(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	int i=0;
	unsigned int Res_H = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1920 :
						 (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 960 : 3840;
    unsigned int Res_V = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 1080 : 
	                     (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160) ? 2160 : 540;	
	unsigned int reg_val_cur[4] = {0};
	unsigned int Res_cur_logo_pix[4] = { (Res_V>>pOutput->u8_logo_pixV_sft_bit)*1/2, (Res_V>>pOutput->u8_logo_pixV_sft_bit)*1/2,(Res_H>>pOutput->u8_logo_pixH_sft_bit)*1/2,(Res_H>>pOutput->u8_logo_pixH_sft_bit)*1/2};
	unsigned int Res_cur_logo_blk[4] = { (Res_V>>pOutput->u8_logo_blkV_sft_bit)*1/2, (Res_V>>pOutput->u8_logo_blkV_sft_bit)*1/2,(Res_H>>pOutput->u8_logo_blkH_sft_bit)*1/2,(Res_H>>pOutput->u8_logo_blkH_sft_bit)*1/2};
	
	ReadRegister(KME_LOGO1_KME_LOGO1_04_reg,0,7,&reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_LOGO1_KME_LOGO1_04_reg,8,15,&reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_LOGO1_KME_LOGO1_04_reg,16,23, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_LOGO1_KME_LOGO1_04_reg,24,31, &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u8_LogoRim_Blk[i] - reg_val_cur[i])>Res_cur_logo_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
	}

	ReadRegister(MC2_MC2_9C_reg,0,9, &reg_val_cur[_RIM_LFT]);  //lft 
	ReadRegister(MC2_MC2_9C_reg,10,19, &reg_val_cur[_RIM_RHT]); //rht
	ReadRegister(MC2_MC2_9C_reg,20,29, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(MC2_MC2_A0_reg,0,9, &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u8_LogoRim_Blk[i] - reg_val_cur[i])>Res_cur_logo_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
	}
	ReadRegister(KME_LOGO1_KME_LOGO1_D8_reg,0,9, &reg_val_cur[_RIM_LFT]);   //lft
	ReadRegister(KME_LOGO1_KME_LOGO1_D8_reg,10,19, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_LOGO1_KME_LOGO1_DC_reg,0,9, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_LOGO1_KME_LOGO1_DC_reg,10,19, &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_LogoRim_Pxl[i] - reg_val_cur[i])>Res_cur_logo_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}
	}


	ReadRegister(MC2_MC2_A4_reg,0,9, &reg_val_cur[_RIM_LFT]); //lft
	ReadRegister(MC2_MC2_A4_reg,10,19, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(MC2_MC2_A4_reg,20,29, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(MC2_MC2_A8_reg,0,9,&reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_LogoRim_Pxl[i] - reg_val_cur[i])>Res_cur_logo_pix[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
		
}
VOID Rim_check_protect_MC(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	int i=0;
	unsigned int Res_H = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1920 :
						 (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 960 : 3840;
    unsigned int Res_V = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 1080 : 
	                     (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160) ? 2160 : 540;
	unsigned int reg_val_cur[4] = {0};
	unsigned int Res_cur[4] = { Res_V*1/2,  Res_V*1/2, Res_H*1/2, Res_H*1/2};
	
	ReadRegister(MC_MC_50_reg,0,11, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(MC_MC_50_reg,16,27,&reg_val_cur[_RIM_BOT]);  //bot
	ReadRegister(MC_MC_54_reg,0,11, &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(MC_MC_54_reg,16,27, &reg_val_cur[_RIM_RHT]);  //rht
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_mc_rim0[i] - reg_val_cur[i])>Res_cur[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
	
	ReadRegister(MC_MC_58_reg,0,11, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(MC_MC_58_reg,16,27, &reg_val_cur[_RIM_BOT]);  //bot
	ReadRegister(MC_MC_5C_reg,0,11,&reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(MC_MC_5C_reg,16,27, &reg_val_cur[_RIM_RHT]); //rht
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_mc_rim1[i] - reg_val_cur[i])>Res_cur[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
}
VOID Rim_check_protect_Film(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	int i=0;
	unsigned int Res_H = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1920 :
						 (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 960 : 3840;
    unsigned int Res_V = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 1080 : 
	                     (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160) ? 2160 : 540;
	unsigned int reg_val_cur[4] = {0};
	unsigned char  Res_H_Divide = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 2 : 
		                     			(pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080 ||pOutput->u4_outResolution_pre == _PQL_OUT_3840x540) ? 3 :
										(pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 1 : 0;
    unsigned char  Res_V_Divide = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080) ? 1 : 
	                         			(pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 2 : 0;  // Fot RimCtrl_Film_Rim_Convert
	 
	unsigned int Res_cur_film[4] = { (Res_V>>Res_V_Divide)*1/2, (Res_V>>Res_V_Divide)*1/2,(Res_H>>Res_H_Divide)*1/2,(Res_H>>Res_H_Divide)*1/2};
	ReadRegister(KME_IPME_KME_IPME_A0_reg,0,9, &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_IPME_KME_IPME_A0_reg,10,19, &reg_val_cur[_RIM_BOT]);  //bot
	ReadRegister(KME_IPME_KME_IPME_9C_reg,0,9, &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_IPME_KME_IPME_9C_reg,10,19, &reg_val_cur[_RIM_RHT]);  //rht
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u12_ipme_rim[i] - reg_val_cur[i])>Res_cur_film[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
	
}
VOID Rim_check_protect_Dh(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	int i=0;
	unsigned int Res_H = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_1920x540) ? 1920 :
						 (pOutput->u4_outResolution_pre == _PQL_OUT_960x540) ? 960 : 3840;
    unsigned int Res_V = (pOutput->u4_outResolution_pre == _PQL_OUT_1920x1080 || pOutput->u4_outResolution_pre == _PQL_OUT_3840x1080) ? 1080 : 
	                     (pOutput->u4_outResolution_pre == _PQL_OUT_3840x2160) ? 2160 : 540;
	unsigned int reg_val_cur[4] = {0};
	unsigned int Res_cur_df_oftrim_blk[4] = 	{ Res_V*1/2, (Res_V>>pOutput->u8_me1_blkV_sft_bit)*1/2,(Res_H>>pOutput->u8_me1_blkH_sft_bit)*1/2,Res_H*1/2};
	unsigned int Res_cur_df_blk[4] = { (Res_V>>pOutput->u8_dh_BlkV_sft_bit)*1/2, (Res_V>>pOutput->u8_dh_BlkV_sft_bit)*1/2,(Res_H>>pOutput->u8_dh_BlkH_sft_bit)*1/2,(Res_H>>pOutput->u8_dh_BlkH_sft_bit)*1/2};
	ReadRegister(KME_DEHALO_KME_DEHALO_18_reg,0,8,  &reg_val_cur[_RIM_LFT]);   //lft
	ReadRegister(KME_DEHALO_KME_DEHALO_18_reg,9,17, &reg_val_cur[_RIM_RHT]);   //rht
	ReadRegister(KME_DEHALO_KME_DEHALO_18_reg,18,26, & reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_DEHALO_KME_DEHALO_1C_reg,0,8,   &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u9_dh_blk_oftrim[i] - reg_val_cur[i])>Res_cur_df_oftrim_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
	ReadRegister(KME_DEHALO_KME_DEHALO_1C_reg,9,17,  &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_DEHALO_KME_DEHALO_1C_reg,18,26, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_DEHALO_KME_DEHALO_20_reg,0,8,   &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_DEHALO_KME_DEHALO_20_reg,9,17,   &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u9_dh_blk_oftrim[i] - reg_val_cur[i])>Res_cur_df_oftrim_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
	ReadRegister(KME_DEHALO_KME_DEHALO_20_reg,18,26,&reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_DEHALO_KME_DEHALO_24_reg,0,8, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_DEHALO_KME_DEHALO_24_reg,9,17,   &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_DEHALO_KME_DEHALO_24_reg,18,26, &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u9_dh_blk_oftrim[i] - reg_val_cur[i])>Res_cur_df_oftrim_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}


	ReadRegister(KME_DEHALO_KME_DEHALO_E0_reg,0,8,  &reg_val_cur[_RIM_LFT]);  //lft
	ReadRegister(KME_DEHALO_KME_DEHALO_E0_reg,9,17, &reg_val_cur[_RIM_RHT]);  //rht
	ReadRegister(KME_DEHALO_KME_DEHALO_E4_reg,0,8,   &reg_val_cur[_RIM_TOP]);  //top
	ReadRegister(KME_DEHALO_KME_DEHALO_E4_reg,9,17,  &reg_val_cur[_RIM_BOT]);  //bot
	for (i=0;i<4;i++)
	{
		if(pOutput->u1_RimChange==0 && _ABS_(pOutput->u9_dh_blk_rim[i] - reg_val_cur[i])>Res_cur_df_blk[i])
		{
			pOutput->u1_RimChange = 1;
			break;
		}

	}
}
VOID Rim_check_protect(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
		
	//printk("[Rim_check_protect] u1_RimChange  = %d\n",pOutput->u1_RimChange);
	Rim_check_protect_ME1(pParam,pOutput);
	Rim_check_protect_ME2(pParam,pOutput);
	Rim_check_protect_Logo(pParam,pOutput);
	Rim_check_protect_MC(pParam,pOutput);
	Rim_check_protect_Film(pParam,pOutput);
	Rim_check_protect_Dh(pParam,pOutput);
}

VOID RimCtrl_Stable(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
#if 0
    int i = 0; 
    for (i = 0; i < _RIM_NUM; i ++)
    {
         pOutput->s8_StableCnt[i] = (_ABS_(pOutput->u12_Rim_Cur[i]-pOutput->u12_Rim_Pre_Det[i]) < 3) ? (pOutput->s8_StableCnt[i] + 1) :\
		 	                        ((_ABS_(pOutput->u12_Rim_Cur[i]-pOutput->u12_Rim_Pre_Det[i]) < 8) ? pOutput->s8_StableCnt[i] :\
		 	                        ((_ABS_(pOutput->u12_Rim_Cur[i]-pOutput->u12_Rim_Pre_Det[i]) < 16) ? (pOutput->s8_StableCnt[i] - 1) : 0));
		 pOutput->s8_StableCnt[i] = (pOutput->s8_StableCnt[i] > 32) ? 32 : ((pOutput->s8_StableCnt[i] < 0) ? 0 : pOutput->s8_StableCnt[i]);
    }
#endif
}

VOID RimCtrl_Panning_Detect(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	//const _PQLCONTEXT *s_pContext = GetPQLContext();
	//unsigned short u16_rb_smv_posCnt, u16_rb_Lmv_posCnt, u16_rb_smv_negCnt, u16_rb_Lmv_negCnt;
	//unsigned short u16_cnt_totalNum = (s_pContext->_output_read_comreg.u8_me1_Vbuf_Hsize_rb * s_pContext->_output_read_comreg.u8_me1_Vbuf_Vsize_rb);
	//unsigned short u16_cnt_th = u16_cnt_totalNum * pParam->u8_PanningCntTH / 255;
	//int i;
	//unsigned char u8_panning_good_rgn_num = 0;

	//u16_rb_smv_posCnt		 = s_pContext->_output_read_comreg.u17_me_posCnt_0_rb;
	//u16_rb_Lmv_posCnt		 = s_pContext->_output_read_comreg.u17_me_posCnt_1_rb;
	//u16_rb_smv_negCnt		 = s_pContext->_output_read_comreg.u17_me_negCnt_0_rb;
	//u16_rb_Lmv_negCnt		 = s_pContext->_output_read_comreg.u17_me_negCnt_1_rb;

	//for(i=0;i<32;i++)
	//{
	//	if((_ABS_(s_pContext->_output_read_comreg.s11_me_rMV_1st_vx_rb[i]) < pParam->u10_PanningRMVXTH)&&
	//		(_ABS_(s_pContext->_output_read_comreg.s10_me_rMV_1st_vy_rb[i]) > pParam->u10_PanningRMVYTH)&&
	//		(s_pContext->_output_read_comreg.u12_me_rMV_1st_cnt_rb[i] > pParam->u12_PanningGMVRngCnt)&& 
	//		(s_pContext->_output_read_comreg.u12_me_rMV_1st_unconf_rb[i] < pParam->u12_PanningGMVUnconf))
	//	{
	//		u8_panning_good_rgn_num++;
	//	}
	//}

	//if(((u16_rb_smv_posCnt > u16_cnt_th) || (u16_rb_Lmv_posCnt > u16_cnt_th)) && 
	//	(u8_panning_good_rgn_num >= pParam->u8_PanningGoodRngCntTH))
	//{
	//	pOutput->u8_PanningMode = _PANNING_DOWN;	
	//	//ROSPrintf("[MEMC]::_PANNING_DOWN \r\n");
	//}
	//else if(((u16_rb_smv_negCnt > u16_cnt_th) || (u16_rb_Lmv_negCnt > u16_cnt_th)) && 
	//	    (u8_panning_good_rgn_num >= pParam->u8_PanningGoodRngCntTH))
	//{
	//	pOutput->u8_PanningMode = _PANNING_UP;
	//	//ROSPrintf("[MEMC]::_PANNING_UP \r\n");
	//}
	//else
	//{
	//	pOutput->u8_PanningMode = _PANNING_NORMAL;
	//	//ROSPrintf("[MEMC]::_PANNING_NORMAL \r\n");
	//}
}

//////////////////////////////////////////////////////////////////////////


VOID RimCtrl_Init(_OUTPUT_RimCtrl *pOutput)
{
	unsigned char u8_i = 0;
	unsigned short u16_Rim_hsize, u16_Rim_vsize;
	bbd_bbd_window_end_point_RBUS bbd_bbd_window_end_point_reg;

	if(Get_DISP_ACT_END_VPOS()<=1088){//4k1k
		u16_Rim_hsize = 3839;
		u16_Rim_vsize = 1079;
	}
	else{//4k2k
		u16_Rim_hsize = 3839;
		u16_Rim_vsize = 2159;
	}

	pOutput->u4_outResolution_pre         = _PQL_OUT_1920x1080;
	pOutput->u12_out_resolution[_RIM_LFT] = 0;
	pOutput->u12_out_resolution[_RIM_RHT] = 1919;
	pOutput->u12_out_resolution[_RIM_TOP] = 0;
	pOutput->u12_out_resolution[_RIM_BOT] = 1079;
	pOutput->u1_RimControl_pre            = 1;
	pOutput->u1_Resolution_change         = 0;
	//pOutput->u1_satelite				  = 0;

	for(u8_i=0; u8_i<_RIM_NUM;u8_i++)
	{
		pOutput->u12_Rim_Cur[u8_i] = (u8_i==_RIM_TOP ||u8_i==_RIM_LFT)?0:((u8_i==_RIM_BOT)? u16_Rim_vsize : u16_Rim_hsize);	       // current rim  0:top 1:bottom 2:right 3:left 
		pOutput->u12_Rim_Pre[u8_i] = (u8_i==_RIM_TOP ||u8_i==_RIM_LFT)?0:((u8_i==_RIM_BOT)? u16_Rim_vsize : u16_Rim_hsize);          // previous rim
		pOutput->u12_Rim_Result[u8_i] = (u8_i==_RIM_TOP ||u8_i==_RIM_LFT)?0:((u8_i==_RIM_BOT)? u16_Rim_vsize : u16_Rim_hsize);       // rim result
		pOutput->u12_Rim_Pre_Det[u8_i] = (u8_i==_RIM_TOP ||u8_i==_RIM_LFT)?0:((u8_i==_RIM_BOT)? u16_Rim_vsize : u16_Rim_hsize);          // rim result
		pOutput->u12_Rim_Target[u8_i] = (u8_i==_RIM_TOP ||u8_i==_RIM_LFT)?0:((u8_i==_RIM_BOT)? u16_Rim_vsize : u16_Rim_hsize);		// rim target of fast-in and slow-out

		pOutput->s8_BiggerCnt[u8_i] = 0;
		pOutput->s8_SmallerCnt[u8_i] = 0;
	}

	pOutput->u12_5Rgn1_v0 = 0x4;
	pOutput->u12_5Rgn1_v1 = 0x58;
	pOutput->u12_5Rgn2_v0 = 0x1bc;
	pOutput->u12_5Rgn2_v1 = 0x20d;
	pOutput->u12_5Rgn3_h0 = 0x2;
	pOutput->u12_5Rgn3_h1 = 0x60;
	pOutput->u12_5Rgn4_h0 = 0x140;
	pOutput->u12_5Rgn4_h1 = 0x1d0;

	pOutput->u12_12Rgn_v0 = 0x4;
	pOutput->u12_12Rgn_v1 = 0x58;
	pOutput->u12_12Rgn_v2 = 0x1bc;
	pOutput->u12_12Rgn_v3 = 0x20d;
	pOutput->u12_12Rgn_h0 = 0x2;
	pOutput->u12_12Rgn_h1 = 0x60;
	pOutput->u12_12Rgn_h2 = 0xd0;
	pOutput->u12_12Rgn_h3 = 0x140;
	pOutput->u12_12Rgn_h4 = 0x1d0;	

	bbd_bbd_window_end_point_reg.regValue = rtd_inl(BBD_BBD_WINDOW_END_POINT_reg);
	//window direction defined by PQ and DIC is different
	//window_x_end->height, window_y_end->width
	bbd_bbd_window_end_point_reg.bbd_window_x_end = u16_Rim_vsize;
	bbd_bbd_window_end_point_reg.bbd_window_y_end = u16_Rim_hsize;
	rtd_outl(BBD_BBD_WINDOW_END_POINT_reg, bbd_bbd_window_end_point_reg.regValue);
}

VOID RimCtrl_DynamicBlackTh(VOID)
{
#if 1
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	
	unsigned char u8_black_th;	
	unsigned char u8_darklevel = (s_pContext->_external_info.u8_DarkLevel/*+u4_black_offset*/);
	#if 1
	unsigned int u32_rb_val = 0;
	unsigned char u1_dynamic_disable;
	static unsigned int cnt =0;
	cnt++;
	ReadRegister(SOFTWARE3_SOFTWARE3_47_reg,0,31, &u32_rb_val);
	u1_dynamic_disable = ((u32_rb_val)&0x1);//bit 0
	#endif	
	if (!u1_dynamic_disable)
	{
		if (u8_darklevel < 0x8)
		{
			u8_black_th = 0x8;
		}
		else if (u8_darklevel > 0x40)
		{
			u8_black_th = 0x40;
		}
		else
		{
			u8_black_th = u8_darklevel;
		}
        //u8_black_th = 0xA; // fixed for real black
		WriteRegister(BBD_BBD_INPUT_THRESHOLD_CTRL_reg,0,7, u8_black_th);
	}
#if 1	
	if((u32_rb_val)&0x2){ //bit 1
		if( cnt%10 == 0 ){
			rtd_pr_memc_err("===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
			rtd_pr_memc_err("===== u8_darklevel [%d]=========\n",u8_darklevel);	
		}
	}
#endif

#endif
}


VOID RimCtrl_Rim_Comp(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	unsigned int BOT_ratio = 0, center_V_ratio = 0, TOP_ratio = 0;
	static unsigned int pre_BOT_ratio = 0, pre_center_V_ratio = 0, pre_TOP_ratio = 0;
	static unsigned char Comp_flag_cnt = 0;
	//unsigned char Comp_flag = 0;
	int ratio_diff = 0;

	unsigned char BOT_Comp_value = 0, TOP_Comp_value = 0;

#if 1
	unsigned int u32_rb_val = 0;

      unsigned int logo_idx7_cnt=0,logo_idx24_cnt=0;
	//unsigned char u1_dynamic_en;
	//unsigned char u4_black_offset;

	static unsigned char SuperAction_flag_cnt = 0,Comp_flag_zero_cnt = 0; //YE Test
      static unsigned int u32_ratio_temp_min =20,u32_ratio_temp_max=84;
	
      ReadRegister(KME_LOGO2_KME_LOGO2_9C_reg,20,29,&logo_idx7_cnt); //To count SuperAction Logo
	ReadRegister(KME_LOGO2_KME_LOGO2_E0_reg,20,29,&logo_idx24_cnt);

	
	ReadRegister(SOFTWARE3_SOFTWARE3_47_reg,0,31, &u32_rb_val);
	//u1_dynamic_en = (u32_rb_val & 0x01);
	//u4_black_offset = ((u32_rb_val>>1) & 0x07);	
#endif	


	TOP_ratio = ((_ABS_DIFF_( pOutput->u12_Rim_Cur[_RIM_TOP] , pOutput->u12_out_resolution[_RIM_TOP]))<<10)>>11;
	center_V_ratio= ((_ABS_DIFF_( pOutput->u12_Rim_Cur[_RIM_BOT] , pOutput->u12_Rim_Cur[_RIM_TOP]))<<10)>>11;
	BOT_ratio = ((_ABS_DIFF_(pOutput->u12_out_resolution[_RIM_BOT] , pOutput->u12_Rim_Cur[_RIM_BOT]))<<10)>>11;

	ratio_diff = TOP_ratio-BOT_ratio;
					
					

	if ((((_ABS_(ratio_diff)<20) && (BOT_ratio<=u32_ratio_temp_max && BOT_ratio>=u32_ratio_temp_min)) 
		&& (TOP_ratio <=u32_ratio_temp_max &&TOP_ratio >=u32_ratio_temp_min )  
		&& pOutput->u1_RimTwoSide_balancing == 1)
		&& ((logo_idx7_cnt>= 80) && (logo_idx24_cnt>=200)))
	      {
			
		SuperAction_flag_cnt++;
		if(Comp_flag_zero_cnt>1)
		Comp_flag_zero_cnt--;
		
		}
	else if(((logo_idx7_cnt<80)&&(logo_idx7_cnt>=0))||((logo_idx24_cnt<200)&&(logo_idx24_cnt>=0))){

		if(SuperAction_flag_cnt>1)
		SuperAction_flag_cnt--;
				
		Comp_flag_zero_cnt++;

		}
	else if ((TOP_ratio !=0)&&(BOT_ratio !=0)){
		if(SuperAction_flag_cnt>1)
		SuperAction_flag_cnt--;
		
		Comp_flag_zero_cnt++;

		}
	else{

		}

	if(((BOT_ratio==0)&&(TOP_ratio==0))&&(Comp_flag_zero_cnt>=5)){
	SuperAction_flag_cnt=0;
	Comp_flag_zero_cnt=0;

      	}
	 
	

	if(SuperAction_flag_cnt >=5)
		{
		pOutput->u1_RimCompflag=3;
		u1_SuperAction_RimCompflag=3;
		if(SuperAction_flag_cnt >=10)		
			SuperAction_flag_cnt=5;
		
		}
	else{
		u1_SuperAction_RimCompflag=0;
		//if((_ABS_(ratio_diff)>4 && (TOP_ratio>BOT_ratio)&&(TOP_ratio>30&&BOT_ratio>30))|| pOutput->u1_RimTwoSide_balancing == 0){
		if((_ABS_(ratio_diff)>5 && (TOP_ratio>BOT_ratio)&&(TOP_ratio>30&&BOT_ratio>30))|| pOutput->u1_RimTwoSide_balancing == 0){
			pOutput->u1_RimCompflag = 1;
			Comp_flag_cnt = 5;
		}else if(pOutput->u1_RimTwoSide_balancing == 2){
			pOutput->u1_RimCompflag = 2;
			Comp_flag_cnt = 5;
		//}else if(((_ABS_(ratio_diff)>4 && _ABS_(ratio_diff)<10) && ((TOP_ratio<60 && TOP_ratio>50) && (BOT_ratio<65 && BOT_ratio>55)) && (BOT_ratio>TOP_ratio)) && pOutput->u1_RimTwoSide_balancing == 1){
		}else if(((_ABS_(ratio_diff)>5 && _ABS_(ratio_diff)<10) && ((TOP_ratio<60 && TOP_ratio>50) && (BOT_ratio<65 && BOT_ratio>55)) && (BOT_ratio>TOP_ratio)) && pOutput->u1_RimTwoSide_balancing == 1){
			pOutput->u1_RimCompflag = 1;
			Comp_flag_cnt = 5;
		}else if(((_ABS_(ratio_diff)>5 && _ABS_(ratio_diff)<10) && ((TOP_ratio<145 && TOP_ratio>135) && (BOT_ratio<140 && BOT_ratio>130)) && (TOP_ratio>BOT_ratio)) && pOutput->u1_RimTwoSide_balancing == 1){
		//}else if(((_ABS_(ratio_diff)>3 && _ABS_(ratio_diff)<10) && ((TOP_ratio<145 && TOP_ratio>135) && (BOT_ratio<140 && BOT_ratio>130)) && (TOP_ratio>BOT_ratio)) && pOutput->u1_RimTwoSide_balancing == 1){
			pOutput->u1_RimCompflag = 1;
			Comp_flag_cnt = 5;
		}else{
			Comp_flag_cnt = Comp_flag_cnt>0?(Comp_flag_cnt-1):0;
			if(Comp_flag_cnt == 0){
				pOutput->u1_RimCompflag = 0;
				Comp_flag_cnt = 0;
			}	
		 }

	}

#if 1

	if(u32_rb_val&0x20){//bit 5
		pOutput->u1_RimCompflag = 0;
	}

	if(pOutput->u1_RimCompflag == 1){
		BOT_Comp_value = 25;
		TOP_Comp_value =0;
	}else if(pOutput->u1_RimCompflag == 2){		
		BOT_Comp_value = 0;
		TOP_Comp_value = 25;
	}else if(pOutput->u1_RimCompflag == 3){	

		#if 1 //Boundary +85 solution
	      BOT_Comp_value = 85;
		TOP_Comp_value = 0;	
		#else // ZMV Solution	
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_10_reg, 16, 16, 0x1);
		WriteRegister(KME_VBUF_TOP_KME_VBUF_TOP_14_reg, 15, 15, 0x1);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_28_reg, 16, 28, 0x20);
		WriteRegister(KME_ME1_TOP0_KME_ME1_TOP0_68_reg, 16, 28, 0x20);
		#endif	
	}else{
		BOT_Comp_value = 0;//org setting
		TOP_Comp_value = 0;
	}

	
	if(u32_rb_val&0x10){ //bit 4
		rtd_pr_memc_err("######## [%s], [%d] ########\n",__FUNCTION__, __LINE__);
		rtd_pr_memc_err("===== rim resolution =========\n");	
		rtd_pr_memc_err("res_V_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_out_resolution[_RIM_BOT] , pOutput->u12_out_resolution[_RIM_TOP]),pOutput->u12_out_resolution[_RIM_BOT],pOutput->u12_out_resolution[_RIM_TOP]);
		rtd_pr_memc_err("res_H_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_out_resolution[_RIM_RHT] , pOutput->u12_out_resolution[_RIM_LFT]),pOutput->u12_out_resolution[_RIM_RHT],pOutput->u12_out_resolution[_RIM_LFT]);
		rtd_pr_memc_err("===== rim cur =========\n");
		rtd_pr_memc_err("cur_V_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_Rim_Cur[_RIM_BOT] , pOutput->u12_Rim_Cur[_RIM_TOP]),pOutput->u12_Rim_Cur[_RIM_BOT],pOutput->u12_Rim_Cur[_RIM_TOP]);
		rtd_pr_memc_err("cur_H_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_Rim_Cur[_RIM_RHT] , pOutput->u12_Rim_Cur[_RIM_LFT]),pOutput->u12_Rim_Cur[_RIM_RHT],pOutput->u12_Rim_Cur[_RIM_LFT]);
		rtd_pr_memc_err("===== ratio check =========\n");
		rtd_pr_memc_err("cur_Rim_ratio >> %d,%d,%d\n", TOP_ratio, center_V_ratio, BOT_ratio);
		rtd_pr_memc_err("pre_Rim_ratio >> %d,%d,%d\n", pre_TOP_ratio, pre_center_V_ratio, pre_BOT_ratio);
		rtd_pr_memc_err("===== rim result(FW) =========\n");	
		rtd_pr_memc_err("result_V_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_Rim_Result[_RIM_BOT] , pOutput->u12_Rim_Result[_RIM_TOP]),pOutput->u12_Rim_Result[_RIM_BOT],pOutput->u12_Rim_Result[_RIM_TOP]);
		rtd_pr_memc_err("result_H_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_Rim_Result[_RIM_RHT] , pOutput->u12_Rim_Result[_RIM_LFT]),pOutput->u12_Rim_Result[_RIM_RHT],pOutput->u12_Rim_Result[_RIM_LFT]);
		rtd_pr_memc_err("===== final check =========\n");		
		rtd_pr_memc_err("final_comp_rim >> %d,%d,%d,%d, %d\n", pOutput->u1_RimCompflag, Comp_flag_cnt, BOT_Comp_value, pParam->u8_lbme_pixRim_comp,pOutput->u1_RimTwoSide_balancing);
		rtd_pr_memc_err("===== value write for register =========\n");
		rtd_pr_memc_err("mc_rim1_V_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_mc_rim1[_RIM_BOT] , pOutput->u12_mc_rim1[_RIM_TOP]),pOutput->u12_mc_rim1[_RIM_BOT],pOutput->u12_mc_rim1[_RIM_TOP]);
		rtd_pr_memc_err("mc_rim1_H_size >> %d,%d,%d\n",_ABS_DIFF_(pOutput->u12_mc_rim1[_RIM_RHT] , pOutput->u12_mc_rim1[_RIM_LFT]),pOutput->u12_mc_rim1[_RIM_RHT],pOutput->u12_mc_rim1[_RIM_LFT]);
	}
#endif



#if 0
	if(u1_dynamic_en){
		if(pOutput->u1_RimCompflag!=0){
			BOT_Comp_value = u4_black_offset;//org setting
			TOP_Comp_value = 0;
		}
	}
#endif

	pOutput->u12_Rim_Target[_RIM_TOP] = _MAX_(pOutput->u12_Rim_Cur[_RIM_TOP] - TOP_Comp_value, pOutput->u12_out_resolution[_RIM_TOP]);
	pOutput->u12_Rim_Target[_RIM_BOT] = _MIN_( pOutput->u12_Rim_Cur[_RIM_BOT] + BOT_Comp_value, pOutput->u12_out_resolution[_RIM_BOT]);
	
	pre_BOT_ratio = BOT_ratio;
	pre_center_V_ratio = center_V_ratio;
	pre_TOP_ratio = TOP_ratio;	

}


//Input signal and parameters are connected locally
VOID RimCtrl_Proc(const _PARAM_RimCtrl *pParam, _OUTPUT_RimCtrl *pOutput)
{
	const _PQLCONTEXT *s_pContext = GetPQLContext();
	RimCtrl_OutResolution_Parse(pOutput);
	//RimCtrl_Panning_Detect(pParam, pOutput);
	if (pParam->u1_RimCtrl_en == 1)
	{
		int i = 0;
		unsigned int rimSize = 0;
		int horizatal_speed = 0;
		int vertical_speed = 0;
		unsigned int u32_rb_val = 0;

		ReadRegister(SOFTWARE3_SOFTWARE3_47_reg,0,31, &u32_rb_val);
		RimCtrl_DynamicBlackTh();
		pOutput->u1_RimChange = 0;
		pOutput->u1_RimCompflag = 0;
		RimDet_by_KMC_YUV_Meter(pParam, pOutput);
	//	if((u32_rb_val)&0x4) return;
		RimCtrl_HW_extract(pParam, pOutput);
		RimCtrl_Rim_Comp(pParam, pOutput);//add from K5lp for boundary compensation! Terrence
	//	if((u32_rb_val)&0x8) return;
		RimCtrl_FastOut(pParam, pOutput);
		RimCtrl_SlowIn(pParam, pOutput);
	//	if((u32_rb_val)&0x10) return;
		RimCtrl_Output(pParam, pOutput); // adjust features base on result rim
	//	if((u32_rb_val)&0x20) return;
		Rim_check_protect(pParam,pOutput); // protect function modify u1_RimChange which be used in RimCtrl_wrtAction
	//	if((u32_rb_val)&0x40) return;
		//RimCtrl_Stable(pParam, pOutput);

		for (i = 0; i < _RIM_NUM; i ++)
		{
			pOutput->u12_Rim_Pre[i] = pOutput->u12_Rim_Result[i];
			pOutput->u12_Rim_Pre_Det[i] = pOutput->u12_Rim_Cur[i];
		}

		//calculate rim ratio
		rimSize = (pOutput->u12_Rim_Result[_RIM_RHT]-pOutput->u12_Rim_Result[_RIM_LFT] + 1)*
		          (pOutput->u12_Rim_Result[_RIM_BOT]-pOutput->u12_Rim_Result[_RIM_TOP] + 1);
				
		pOutput->u32_rimRatio = (rimSize == 0) ? 128 :(rimSize<<7) / 
								((pOutput->u12_out_resolution[_RIM_RHT]+1)*(pOutput->u12_out_resolution[_RIM_BOT]+1));

		if((u32_rb_val)&0x80) return;

		// --------vertical and horizatal motion detect-----
		horizatal_speed = s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb;
		vertical_speed  = s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb;
		#if 0
		rtd_printk(KERN_NOTICE, TAG_NAME_MEMC, "[RimCtrl_Proc]horizatal_speed = %d,%d\r\n",s_pContext->_output_read_comreg.s11_me_GMV_1st_vx_rb);
		rtd_printk(KERN_NOTICE, TAG_NAME_MEMC, "[RimCtrl_Proc]vertical_speed = %d,%d\r\n",s_pContext->_output_read_comreg.s10_me_GMV_1st_vy_rb);		
		#endif
		#if 0 //don't need this condiction, because new algo. of H-near rim after Merlin7
		if(( horizatal_speed <(-90))&&( horizatal_speed >(-220))&&( vertical_speed == 0))
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg, 30,30,0x0);
		else
				WriteRegister(KME_ME2_VBUF_TOP_KME_ME2_VBUF_TOP_DC_reg, 30,30,0x1);
		#endif
		
		if((u32_rb_val)&0x100) return;
	} 
	else
	{
		pOutput->u1_RimChange = (pOutput->u1_RimControl_pre !=  pParam->u1_RimCtrl_en) ? 1 : 0;
		pOutput->u12_Rim_Result[_RIM_LFT] = pOutput->u12_out_resolution[_RIM_LFT];
		pOutput->u12_Rim_Result[_RIM_RHT] = pOutput->u12_out_resolution[_RIM_RHT];
		pOutput->u12_Rim_Result[_RIM_TOP] = pOutput->u12_out_resolution[_RIM_TOP];
		pOutput->u12_Rim_Result[_RIM_BOT] = pOutput->u12_out_resolution[_RIM_BOT];

		RimCtrl_Output(pParam, pOutput);
	}
#if 1
	
		//rtd_pr_memc_err("===== [%s], [%d] =========\n",__FUNCTION__, __LINE__);
		//rtd_pr_memc_err("[u12_Rim_Result][T,B,L,R]>>[%d,%d,%d,%d]\n",pOutput->u12_Rim_Result[_RIM_TOP],pOutput->u12_Rim_Result[_RIM_BOT],pOutput->u12_Rim_Result[_RIM_LFT],pOutput->u12_Rim_Result[_RIM_RHT]);
	
#endif
	pOutput->u1_RimControl_pre = pParam->u1_RimCtrl_en;
}

unsigned char scalerVIP_Get_MEMCRimFlag()
{
	return u1_SuperAction_RimCompflag;
}



