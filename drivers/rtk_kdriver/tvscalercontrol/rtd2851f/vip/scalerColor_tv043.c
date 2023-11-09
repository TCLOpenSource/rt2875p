#include <linux/kernel.h>

#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/io/ioregdrv.h>

/*some include about VIP Driver*/
/*#include <tvscalercontrol/vip/scalerColor.h>*/
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/scalerColor_tv043.h>
#include <tvscalercontrol/vip/di_ma.h>
#include <tvscalercontrol/vip/peaking.h>
#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/vip/dcc.h>
#include <tvscalercontrol/vip/ultrazoom.h>
#include <tvscalercontrol/vip/color.h>
#include <tvscalercontrol/vip/xc.h>
#include <tvscalercontrol/vip/intra.h>
#include <tvscalercontrol/vip/nr.h>
#include <tvscalercontrol/vip/film.h>
#include <tvscalercontrol/vip/pq_adaptive.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/vip/localdimming.h>
#include <tvscalercontrol/vip/localcontrast.h>
#include <tvscalercontrol/vip/fcic.h>
#include <tvscalercontrol/vip/st2094.h>
/*some include about scaler*/
#include <scaler/scalerDrvCommon.h>
#include <rbus/pinmux_main_reg.h>





#undef rtd_outl
#define rtd_outl(x, y)     								IoReg_Write32(x,y)
#undef rtd_inl
#define rtd_inl(x)     									IoReg_Read32(x)
#undef rtd_maskl
#define rtd_maskl(x, y, z)     							IoReg_Mask32(x,y,z)
#undef rtd_setbits
#define rtd_setbits(offset, Mask) rtd_outl(offset, (rtd_inl(offset) | Mask))
#undef rtd_clearbits
#define rtd_clearbits(offset, Mask) rtd_outl(offset, ((rtd_inl(offset) & ~(Mask))))

static void rtd_part_outl(unsigned int reg_addr, unsigned int endBit, unsigned int startBit, unsigned int value)
{
	unsigned int X,A,result;
	X=(1<<(endBit-startBit+1))-1;
	A=rtd_inl(reg_addr);
	result = (A & (~(X<<startBit))) | (value<<startBit);
	rtd_outl(reg_addr,result);
}


void fwif_color_set_color_temp_tv043(unsigned char enable, unsigned short Red_Con, unsigned short Green_Con, unsigned short Blue_Con,
	short Red_Bri, short Green_Bri, short Blue_Bri)
{

	Red_Bri = Red_Bri - 2048;
	Green_Bri = Green_Bri - 2048;
	Blue_Bri = Blue_Bri - 2048;

	fwif_color_WaitFor_SYNC_START_UZUDTG();
	drvif_color_set_color_temp(enable, Red_Con, Green_Con, Blue_Con, Red_Bri, Green_Bri, Blue_Bri);
}

unsigned short LDDutyLimit[2] = {0,1023};
unsigned short Pre_DutyLimit[2] = {0,0};
unsigned char LDTableIdx;
unsigned short backlight_boost_lut[65] = {0, 16, 32, 48, 64, 80, 96, 112,
			128, 144, 160, 176, 192, 208, 224, 240,
			256, 272, 288, 304, 320, 336, 352, 368,
			384, 400, 416, 432, 448, 464, 480, 496,
			512, 528, 544, 560, 576, 592, 608, 624,
			640, 656, 672, 688, 704, 720, 736, 752,
			768, 784, 800, 816, 832, 848, 864, 880,
			896, 912, 928, 944, 960, 976, 992, 1008, 1023};
extern DRV_Local_Dimming_Table Local_Dimming_Table[LD_Table_NUM];


void fwif_color_set_LD_Boost_TV043(unsigned short *DutyLimit)
{

	DRV_LD_Boost data;
	int i;

	if (DutyLimit == NULL)
		return;
	
	LDDutyLimit[0] = DutyLimit[0];
	LDDutyLimit[1] = DutyLimit[1];	
	data.ld_backlightboosten= Local_Dimming_Table[LDTableIdx].LD_Boost.ld_backlightboosten;
	//if (DutyLimit == NULL) {		//Coverity CID42959, DEAD code, never NULL, NULL will return
	//	memcpy(&(data.ld_boost_gain_lut), &(Local_Dimming_Table[LDTableIdx].LD_Boost.ld_boost_gain_lut),sizeof(unsigned short)*65);
	//	memcpy(&(data.ld_boost_curve_lut), &(Local_Dimming_Table[LDTableIdx].LD_Boost.ld_boost_curve_lut),sizeof(unsigned short)*65);
	//} else {
		memcpy(&(data.ld_boost_gain_lut), &(Local_Dimming_Table[LDTableIdx].LD_Boost.ld_boost_gain_lut),sizeof(unsigned short)*65);		
		for (i=0;i<65;i++)
			data.ld_boost_curve_lut[i] = DutyLimit[0] + (Local_Dimming_Table[LDTableIdx].LD_Boost.ld_boost_curve_lut[i]* (DutyLimit[1]-DutyLimit[0]))/1023;
	//}
	drvif_color_set_LD_Boost((DRV_LD_Boost *) &data);
}

unsigned short gLD_BlockMax[2048];
unsigned short gLD_BlockAve[2048],gLD_BlockAve_Pre[2048];
unsigned short gLD_Global_Max_Ave, gLD_Global_Ave, gLD_Max_High_Low_Score;
unsigned short tLD_Light_Leakage_LUT[17] = {0,240,448,624,768,880,960,1008,1024,1008,960,880,768,624,448,240,0};
unsigned short tLD_Global_APL_LUT[17] = {1023,960,896,832,768,704,640,576,512,448,384,320,256,192,128,64,0};
unsigned short tLD_Final_Score_LUT[17] = {1023,1023,1023,1023,1000,875,726,576,427,277,150,128,128,128,128,128,128};

void fwif_color_LD_SW_1DLUT(unsigned short* input, unsigned short* output, unsigned short length, unsigned short *LUT, INPUT_DATA_RANGE input_data_range, LUT_LENGTH lut_length) 
{
	unsigned short data_range;
	unsigned short lut_interval;
	unsigned short rate;
	unsigned int i;
	int src0,src1,coef1,coef0,result;

	/*NULL protection*/
	if (input==NULL || input==NULL)
		return;

	
	if (input_data_range == INPUT_DATA_8BITS)
		data_range = 256;
	else if (input_data_range == INPUT_DATA_10BITS)
		data_range = 1024;
	else if (input_data_range == INPUT_DATA_12BITS)
		data_range = 4096;
	else /*default 12 bits*/
		data_range = 4096;

	if (lut_length == LUT_17POINTS)
		lut_interval = 16;
	else if (lut_length == LUT_33POINTS)
		lut_interval = 32;
	else if (lut_length == LUT_65POINTS)
		lut_interval = 64;
	else if (lut_length == LUT_257POINTS)
		lut_interval = 256;
	else /*defaut 257 points LUT*/
		lut_interval = 256;	

	rate = data_range/lut_interval;
	
	for(i=0; i<length; i++)
    {
        src0 = (int)LUT[input[i]/rate];
        src1 = (int)LUT[input[i]/rate+1];
        if(src0 == (data_range-1)) 
            src0=data_range;
        if(src1 == (data_range-1)) 
            src1=data_range;
        coef1 = input[i]%rate;
        coef0 = rate-coef1;
        result = (src0*coef0+src1*coef1)/rate;
        if(result>(data_range-1)) 
            result=(data_range-1);

		output[i] = (unsigned short)result;
    } 
}

unsigned char gPatternFlag;
unsigned char fwif_color_get_LD_Pattern_Flag_tv043(void) 
{
	return gPatternFlag;
}
void fwif_color_set_LD_Pattern_Flag_tv043(unsigned char pattern_flag) 
{
	gPatternFlag = pattern_flag;
}

void fwif_color_LD_SW_Temporal_Filter_tv043(unsigned short* current_value, unsigned short *target_value, unsigned short step, unsigned short cnt) 
{
	if(current_value == NULL || target_value==NULL)
		return;

	if(cnt%step == 0) {
		if (*current_value > *target_value)
			*current_value = *current_value - 1;
		else if(*current_value < *target_value)
			*current_value = *current_value + 1;
	}
}

void fwif_color_LD_Dynamic_System_tv043(void) 
{
	
	static unsigned int cnt = 0;
	unsigned short i,j=0;
	unsigned short Hnum, Vnum;
	unsigned short light_leakage;
	unsigned short global_apl;
	unsigned short final_score;
	unsigned short target_gain;
	unsigned int high_low_sum = 0;
	unsigned short leakage_high_low;
	unsigned short black_blk_cnt,black_th, white_blk_cnt,white_th,dark_bg_cnt,dark_bg_th;
	static unsigned int motion_level = 0;
	static unsigned short current_hist0 = 0;
	unsigned short target_hist0 = 0;
	static unsigned short current_spatial_gain00 = 63;
	static unsigned short current_spatial_gain01 = 0;
	unsigned short target_spatial_gain01 = 0;
	unsigned char pattern_flag = 0;
	static unsigned short current_bluspi_gain = 16;
	unsigned short target_bluspi_gain = 0;	

		
	int temp_a, temp_b = 0;
	DRV_LD_Spatial_Filter tSpatial_Filter;
	DRV_LD_Backlight_Decision tBacklight_Decision;
	DRV_LD_Global_Ctrl tLD_Global_Ctrl;
	DRV_LD_Temporal_Filter tTemporal_Filter;
	DRV_LD_Backlight_Final_Decision tBacklight_Final_Decision;
	
	/*Get Block information*/
	drvif_color_get_LD_Global_Ctrl(&tLD_Global_Ctrl);
	Hnum = tLD_Global_Ctrl.ld_blk_hnum+1;
	Vnum = tLD_Global_Ctrl.ld_blk_vnum+1;

	/*Get Block max & ave information*/
	drvif_color_get_LD_Block_Max_Info(gLD_BlockMax, 0, 1920, &gLD_Global_Max_Ave,&gLD_Max_High_Low_Score);
	drvif_color_get_LD_Block_Ave_Info(gLD_BlockAve, 0, 1920, &gLD_Global_Ave);
	light_leakage = gLD_Global_Max_Ave-gLD_Global_Ave;
	global_apl = gLD_Global_Ave;

	high_low_sum = 0;
	black_blk_cnt = 0;
	white_blk_cnt = 0;
	dark_bg_cnt = 0;
	black_th = 10;
	white_th = 980;
	dark_bg_th = 30;

	if (cnt % 5 ==0)
		motion_level = 0;

	for (i=0; i<(Hnum*Vnum+1)/2 ;i++) {
		/*Black blcok cnt*/
		if (*(gLD_BlockMax+2*i)<black_th)
			black_blk_cnt ++;
		if (*(gLD_BlockMax+2*i+1)<black_th)
			black_blk_cnt ++;
		/*White blcok cnt*/
		if (*(gLD_BlockMax+2*i)>white_th)
			white_blk_cnt ++;
		if (*(gLD_BlockMax+2*i+1)>white_th)
			white_blk_cnt ++;
		/*Black blcok cnt*/
		if (*(gLD_BlockMax+2*i)<dark_bg_th)
			dark_bg_cnt ++;
		if (*(gLD_BlockMax+2*i+1)<dark_bg_th)
			dark_bg_cnt ++;
		
		if(i%(Hnum/2)==0) {/*need to consider odd block number condition later*/
			temp_a = *(gLD_BlockMax+2*i+1) - *(gLD_BlockAve+2*i+1);
			temp_b = *(gLD_BlockMax+2*i) - *(gLD_BlockAve+2*i);
			high_low_sum = high_low_sum + ABS(temp_a,temp_b);
		} else {
			temp_a = *(gLD_BlockMax+2*i) - *(gLD_BlockAve+2*i);
			temp_b = *(gLD_BlockMax+2*i-1) - *(gLD_BlockAve+2*i-1);
			high_low_sum = high_low_sum + ABS(temp_a,temp_b);
			temp_a = *(gLD_BlockMax+2*i+1) - *(gLD_BlockAve+2*i+1);
			temp_b = *(gLD_BlockMax+2*i) - *(gLD_BlockAve+2*i); 		
			high_low_sum = high_low_sum + ABS(temp_a,temp_b);
		}
		if(i/(Hnum/2) != 0) {
			temp_a = *(gLD_BlockMax+2*i) - *(gLD_BlockAve+2*i);
			temp_b = *(gLD_BlockMax+2*i-Hnum) - *(gLD_BlockAve+2*i-Hnum);
			high_low_sum = high_low_sum + ABS(temp_a,temp_b);
			temp_a = *(gLD_BlockMax+2*i+1) - *(gLD_BlockAve+2*i+1);
			temp_b = *(gLD_BlockMax+2*i+1-Hnum) - *(gLD_BlockAve+2*i+1-Hnum);
			high_low_sum = high_low_sum + ABS(temp_a,temp_b);			
		}
		/*Motion Estimation*/
		if (cnt % 5 ==0) {
			temp_a = *(gLD_BlockAve_Pre+2*i);
			temp_b = *(gLD_BlockAve+2*i);
			motion_level = motion_level + ABS(temp_a,temp_b);
			*(gLD_BlockAve_Pre+2*i) = *(gLD_BlockAve+2*i);
			temp_a = *(gLD_BlockAve_Pre+2*i+1);
			temp_b = *(gLD_BlockAve+2*i+1);
			motion_level = motion_level + ABS(temp_a,temp_b);
			*(gLD_BlockAve_Pre+2*i+1) = *(gLD_BlockAve+2*i+1);
		}
	}
	leakage_high_low = high_low_sum/((Hnum-1)*Vnum + Hnum*(Vnum-1));
	if (cnt % 5 ==0)
		motion_level = motion_level/(white_blk_cnt+1);

	/*test pattern detection*/
	pattern_flag = 1;
	temp_a = *(gLD_BlockAve+16*Hnum+30);
	if ((temp_a < 8)||!(((dark_bg_cnt > 1650)&&(dark_bg_cnt < 1750))||((dark_bg_cnt > 1400)&&(dark_bg_cnt < 1500)))||(gLD_Global_Max_Ave-gLD_Global_Ave)>30) {/*black or leakage is high*/ 
		pattern_flag = 0;
		if (cnt%120 == 0) {
			//rtd_pr_vpq_info("1 Not Pattern, cnt=%d,temp_a=%d, con_1=%d,con_2=%d,\n",cnt,temp_a,(!((dark_bg_cnt > 1650)&&(dark_bg_cnt < 1750))),(!((dark_bg_cnt > 1500)&&(dark_bg_cnt < 1400))));
		}
	} else {
		for (j=13;j<19;j++){
			 if (pattern_flag == 0)
				break;
			 for(i=22;i<38;i++) {
				temp_b = *(gLD_BlockAve+j*Hnum+i);
				if (ABS(temp_a,temp_b) > 8) {
					pattern_flag = 0;
					if (cnt%120 == 0) {
						//rtd_pr_vpq_info("2 Not Pattern, cnt=%d, ave[16][30]=%d, ave[%d][%d]=%d, diff=%d\n",cnt,temp_a,j,i,temp_b,ABS(temp_a,temp_b));
					}
					break;
				}			
			}
		}
	}



	/*Apply setting for Backlight decision Max Gain*/
	drvif_color_get_LD_Backlight_Decision(&tBacklight_Decision);
	if (gLD_Global_Ave<=250)
		tBacklight_Decision.ld_maxgain = 0;
	else if (gLD_Global_Ave<=500)
		tBacklight_Decision.ld_maxgain = 1;
	else if (gLD_Global_Ave<=750)
		tBacklight_Decision.ld_maxgain = 2;
	else
		tBacklight_Decision.ld_maxgain = 3;

	drvif_color_get_LD_Spatial_Filter(&tSpatial_Filter);	
	drvif_color_get_LD_Temporal_Filter(&tTemporal_Filter);


	
	/*Apply setting for Hist gain or Spatial Coef00*/
#if 0	
		/*Remapping*/	
		fwif_color_LD_SW_1DLUT(&light_leakage, &light_leakage, 1, tLD_Light_Leakage_LUT, INPUT_DATA_10BITS, LUT_17POINTS);
		fwif_color_LD_SW_1DLUT(&global_apl, &global_apl, 1, tLD_Global_APL_LUT, INPUT_DATA_10BITS, LUT_17POINTS);
	
		/*Final Score*/
		final_score = global_apl*light_leakage/1023;
		fwif_color_LD_SW_1DLUT(&final_score, &target_gain, 1, tLD_Final_Score_LUT, INPUT_DATA_10BITS, LUT_17POINTS);
#else
		final_score = leakage_high_low*leakage_high_low/((global_apl+1));
		if(final_score > 1023)
			final_score = 1023;
		
		if(final_score>=500)
			target_gain = 8;
		else if (final_score>=300)
			target_gain = 15;
		else if (final_score>=100)
			target_gain = 21;
		else
			target_gain = 63;
#endif

	
	if ((IoReg_Read32(0xB802c200) & 0x3)>>1 != 1) {
		/*Apply setting for spatial filter coef00*/
		fwif_color_LD_SW_Temporal_Filter_tv043(&current_spatial_gain00,&target_gain,3 ,cnt);
		tSpatial_Filter.ld_spatialnewcoef00 = current_spatial_gain00;
		//tBacklight_Decision.ld_hist0gain = 0;
	} else {
		/*Apply hist_gain0*/
		if (final_score>=200)
			target_hist0 = 31;
		else
			target_hist0 = 0;
		tSpatial_Filter.ld_spatialnewcoef00 = 63;
		fwif_color_LD_SW_Temporal_Filter_tv043(&current_hist0,&target_hist0,5,cnt);
		tBacklight_Decision.ld_hist0gain = current_hist0;
	}
	/*Apply RTINGS protection*/
	if ((gLD_Global_Max_Ave-gLD_Global_Ave)<=20 && *gLD_BlockAve<=100 && (white_blk_cnt+black_blk_cnt)>=(94*Hnum*Vnum/100) && motion_level >30){
		target_spatial_gain01 = tSpatial_Filter.ld_spatialnewcoef00*4/10;
		tTemporal_Filter.ld_tmp_neg1thd = 0;
		tTemporal_Filter.ld_tmp_negmingain = 8;
		tTemporal_Filter.ld_tmp_negmaxgain = 8;
		tTemporal_Filter.ld_tmp_posmingain = 4;
		
	} else {
		target_spatial_gain01 = Local_Dimming_Table[3].LD_Spatial_Filter.ld_spatialnewcoef01;
		tTemporal_Filter.ld_tmp_neg1thd = Local_Dimming_Table[3].LD_Temporal_Filter.ld_tmp_neg1thd;
		tTemporal_Filter.ld_tmp_negmingain = Local_Dimming_Table[3].LD_Temporal_Filter.ld_tmp_negmingain;
		tTemporal_Filter.ld_tmp_negmaxgain = Local_Dimming_Table[3].LD_Temporal_Filter.ld_tmp_negmaxgain;
		tTemporal_Filter.ld_tmp_posmingain = Local_Dimming_Table[3].LD_Temporal_Filter.ld_tmp_posmingain;
	}
	fwif_color_LD_SW_Temporal_Filter_tv043(&current_spatial_gain01,&target_spatial_gain01,3,cnt);
		
	tSpatial_Filter.ld_spatialnewcoef01 = current_spatial_gain01;
	tSpatial_Filter.ld_spatialnewcoef10 = current_spatial_gain01;
	tSpatial_Filter.ld_spatialnewcoef11 = current_spatial_gain01;
	


	/*Apply Calman Pattern protection*/
	drvif_color_get_LD_Backlight_Final_Decision(&tBacklight_Final_Decision);
	if (pattern_flag) {
		target_bluspi_gain = 16;
		tBacklight_Decision.ld_maxgain = 3;

	} else {
		target_bluspi_gain = 16;// 5;
	}
	
	//if(pre_luttableindex != 2)//level MID is use for ld demo,don't set filter by dynamic.
	drvif_color_set_LD_Spatial_Filter(&tSpatial_Filter);
	
	drvif_color_set_LD_Temporal_Filter(&tTemporal_Filter);

	drvif_color_set_LD_Backlight_Decision(&tBacklight_Decision);

	
	fwif_color_LD_SW_Temporal_Filter_tv043(&current_bluspi_gain,&target_bluspi_gain,5,cnt);
	if (tBacklight_Final_Decision.ld_blu_usergain != current_bluspi_gain) {
		tBacklight_Final_Decision.ld_blu_usergain = current_bluspi_gain;
		drvif_color_set_LD_Backlight_Final_Decision(&tBacklight_Final_Decision);
	}
	
		
	if (cnt%1200 == 0) {
		rtd_pr_vpq_info("Leakage=%d,gAPL=%d,leakage_h_l=%d,fscore=%d,fscore2=%d,tar_gain=%d,max_gain=%d,tar_h0=%d,cur_h0=%d,mv=%d,w_cnt=%d,b_cnt=%d,d_cnt=%d,pattern=%d\n",gLD_Global_Max_Ave-gLD_Global_Ave,gLD_Global_Ave, leakage_high_low,final_score, leakage_high_low/(1+(global_apl*global_apl)/1023), target_gain/16, tBacklight_Decision.ld_maxgain,target_spatial_gain01,current_spatial_gain01,motion_level,white_blk_cnt,black_blk_cnt,dark_bg_cnt,pattern_flag);
	}
	
	cnt ++;
}

MOVE_LD_TO_LDSPI LD_To_LDSPI_Method = MOVE_LD_TO_LDSPI_BYHW;

MOVE_LD_TO_LDSPI fwif_color_get_LD_To_LDSPI_Method_tv043(void)
{
	return LD_To_LDSPI_Method;
}

void fwif_color_set_LD_To_LDSPI_Method_tv043(MOVE_LD_TO_LDSPI method)
{
	DRV_LD_Backlight_Final_Decision LD_Backlight_Final_Decision;
	ldspi_ld_ctrl_RBUS ld_ctrl;

	LD_To_LDSPI_Method = method;
	drvif_color_get_LD_Backlight_Final_Decision(&LD_Backlight_Final_Decision);
	ld_ctrl.regValue = IoReg_Read32(LDSPI_LD_CTRL_reg);
	if (method == MOVE_LD_TO_LDSPI_BYSW_120HZ) {
		LD_Backlight_Final_Decision.ld_blu_spien = 0;	
		rtd_part_outl(PINMUX_MAIN_GPIO_TLEFT_CFG_6_reg,31,28,0xD);
		ld_ctrl.send_follow_vsync = 0;
		IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);

	} else if (method == MOVE_LD_TO_LDSPI_BYSW_60HZ){
		LD_Backlight_Final_Decision.ld_blu_spien = 0;	
		rtd_part_outl(PINMUX_MAIN_GPIO_TLEFT_CFG_6_reg,31,28,0xF);
		ld_ctrl.send_follow_vsync = 0;
		IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);

	} else {
		LD_Backlight_Final_Decision.ld_blu_spien = 1;
		rtd_part_outl(PINMUX_MAIN_GPIO_TLEFT_CFG_6_reg,31,28,0x6);
		ld_ctrl.send_follow_vsync = 1;
		IoReg_Write32(LDSPI_LD_CTRL_reg, ld_ctrl.regValue);

	}
	drvif_color_set_LD_Backlight_Final_Decision(&LD_Backlight_Final_Decision);
		
}

unsigned short LD_AlgoSRAM_BLValue[LD_Max_Num];
void fwif_color_LD_Set_LD_To_LDSPI_BySW_Calculate(MOVE_LD_TO_LDSPI method)
{
	unsigned short i;
	DRV_LD_Backlight_Final_Decision LD_Backlight_Final_Decision;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	
	if (VIP_RPC_system_info_structure_table == NULL) {
		rtd_pr_vpq_emerg("fwif_color_LD_Set_LD_To_LDSPI_BySW_Calculate, VIP_RPC_system_info_structure_table NULL !!!\n");
		return;
	}
	drvif_color_get_LD_Backlight_Final_Decision(&LD_Backlight_Final_Decision);
	if (method == MOVE_LD_TO_LDSPI_BYSW_120HZ || method == MOVE_LD_TO_LDSPI_BYSW_60HZ) {
		/*get Algo SRAM data*/
		drvif_color_set_LD_AlgoSRAM_SWMode_En(1);	
		drvif_color_get_LD_AlgoSRAM((unsigned int)(LD_Backlight_Final_Decision.ld_blu_spitotal+1), LD_AlgoSRAM_BLValue);
		drvif_color_set_LD_AlgoSRAM_SWMode_En(0);
		/*Implement Boost Function*/
		fwif_color_LD_Boost_1DLUT_SWProcess_tv043(LD_AlgoSRAM_BLValue,LD_AlgoSRAM_BLValue);
		/*Implement BLU_SPI_User_Gain*/
		if(LD_Backlight_Final_Decision.ld_blu_usergain == 255)
			for (i=0;i<(LD_Backlight_Final_Decision.ld_blu_spitotal+1);i++)
				LD_AlgoSRAM_BLValue[i] = LD_AlgoSRAM_BLValue[i]*256>>6;
		else
			for (i=0;i<(LD_Backlight_Final_Decision.ld_blu_spitotal+1);i++)
				LD_AlgoSRAM_BLValue[i] = LD_AlgoSRAM_BLValue[i]*LD_Backlight_Final_Decision.ld_blu_usergain>>6;

		/*test*/
		#if 0
		for (i=0;i<(LD_Backlight_Final_Decision.ld_blu_spitotal+1);i++) {
			if (i%2==0)
				LD_AlgoSRAM_BLValue[i] = 0;
			else
				LD_AlgoSRAM_BLValue[i] = 4095;
		}
		#endif
			
		/*set data to sharememory, write to LDSPI data SRAM in VCPU1*/
		memcpy((void *)&VIP_RPC_system_info_structure_table->PQ_LD_INFO.PQ_LD_AlgoSRAM_Data[0], (void *)&LD_AlgoSRAM_BLValue[0], sizeof(unsigned short)*(LD_Backlight_Final_Decision.ld_blu_spitotal+1));
		VIP_RPC_system_info_structure_table->PQ_LD_INFO.ld_blu_spitotal = LD_Backlight_Final_Decision.ld_blu_spitotal;
		VIP_RPC_system_info_structure_table->PQ_LD_INFO.data_apply = 1;
			
	}
	
}

void fwif_color_LD_Set_LD_To_LDSPI_BySW_Apply(void)/*Function usage same as scalerVIP_Set_LD_To_LDSPI_BySW in VCPU*/
{
	static unsigned int i=0;
	DRV_LD_LDSPI_DATASRAM_TYPE tLD_LDSPI_DataSRAM;
	_RPC_system_setting_info *VIP_RPC_system_info_structure_table = (_RPC_system_setting_info *)Scaler_GetShareMemVirAddr(SCALERIOC_VIP_RPC_system_info_structure);
	
	if (VIP_RPC_system_info_structure_table == NULL) {
		rtd_pr_vpq_emerg("fwif_color_LD_Set_LD_To_LDSPI_BySW_Apply, VIP_RPC_system_info_structure_table NULL !!!\n");
		return;
	}

	#if 1 /*debug*/
	if (i%1200==0) { 
		if (VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_apply== 1) {		
			rtd_pr_vpq_emerg("[SCPU] data_apply=%d, spi_total=%d, arr=%d %d %d\n",VIP_RPC_system_info_structure_table->PQ_LD_INFO.data_apply, VIP_RPC_system_info_structure_table->PQ_LD_INFO.ld_blu_spitotal,VIP_RPC_system_info_structure_table->PQ_LD_INFO.PQ_LD_AlgoSRAM_Data[0],VIP_RPC_system_info_structure_table->PQ_LD_INFO.PQ_LD_AlgoSRAM_Data[1],VIP_RPC_system_info_structure_table->PQ_LD_INFO.PQ_LD_AlgoSRAM_Data[2]);
			rtd_pr_vpq_emerg("[SCPU] header_tail_apply=%d, header_tail_length=%d, header_tail_position=%d arr=%d %d %d\n",VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_apply, VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_length,VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_position,VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_data[0],VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_data[1],VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_data[2]);
		}
	}
	#endif
	i++;
	if (VIP_RPC_system_info_structure_table->PQ_LD_INFO.data_apply == 1) {
		tLD_LDSPI_DataSRAM.SRAM_Position = 0;
		tLD_LDSPI_DataSRAM.SRAM_Length = VIP_RPC_system_info_structure_table->PQ_LD_INFO.ld_blu_spitotal+1;
		tLD_LDSPI_DataSRAM.SRAM_Value = &VIP_RPC_system_info_structure_table->PQ_LD_INFO.PQ_LD_AlgoSRAM_Data[0];
		drvif_color_set_LDSPI_DataSRAM_Data_Continuous(&tLD_LDSPI_DataSRAM,0);
		VIP_RPC_system_info_structure_table->PQ_LD_INFO.data_apply = 0;
	}
	if (VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_apply== 1) {
		tLD_LDSPI_DataSRAM.SRAM_Position = VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_position;
		tLD_LDSPI_DataSRAM.SRAM_Length = VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_length;
		tLD_LDSPI_DataSRAM.SRAM_Value = &VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_data[0];
		drvif_color_set_LDSPI_DataSRAM_Data_Continuous(&tLD_LDSPI_DataSRAM,0);
		VIP_RPC_system_info_structure_table->PQ_LD_INFO.header_tail_apply = 0;
	}	

}


void fwif_color_LD_Boost_1DLUT_SWProcess_tv043(unsigned short* backlight_input, unsigned short* backlight_boost)
{

    //backlight boost
    unsigned int i;
    unsigned short total_block;	
    DRV_LD_Backlight_Final_Decision LD_Backlight_Final_Decision;
	int boost_remap_src0,boost_remap_src1,boost_remap_coef1,boost_remap_coef0,boost_remap_result;
	/*NULL protection*/
	if (backlight_input==NULL || backlight_boost==NULL)
		return;

	/*get total block*/	
	drvif_color_get_LD_Backlight_Final_Decision(&LD_Backlight_Final_Decision);
	total_block = LD_Backlight_Final_Decision.ld_blu_spitotal+1;

    
    for(i=0; i<total_block; i++)// remap
    {
        // Boost Curve: individual BL -> remapped BL
        boost_remap_src0 = (int)backlight_boost_lut[backlight_input[i]/16];
        boost_remap_src1 = (int)backlight_boost_lut[backlight_input[i]/16+1];
        if(boost_remap_src0 == 1023) 
            boost_remap_src0=1024;
        if(boost_remap_src1 == 1023) 
            boost_remap_src1=1024;
        boost_remap_coef1 = backlight_input[i]%16;
        boost_remap_coef0 = 16-boost_remap_coef1;
        boost_remap_result = (boost_remap_src0*boost_remap_coef0+boost_remap_src1*boost_remap_coef1)/16;
        if(boost_remap_result>1023) 
            boost_remap_result=1023;

		backlight_boost[i] = (unsigned short)boost_remap_result;
    } 
}



