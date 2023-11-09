#ifndef __SCALERThread_rtice2AP_H__
#define __SCALERThread_rtice2AP_H__

#ifdef __cplusplus
extern "C" {
#endif


#include <scaler/vipCommon.h>
#include <scaler/vipRPCCommon.h>
#include <tvscalercontrol/vip/scalerColor_tv006.h>

typedef struct {
unsigned short id;
unsigned int size;
unsigned int saddr;
unsigned int eaddr;
unsigned char mode0;
unsigned char mode1;
} ap_data_param;

typedef enum _VIP_TOOL_ACCESS_INFO_Item{

/*Basic OSD Control Item*/
	TOOLACCESS_Contrast = 0,
	TOOLACCESS_Brightness,
	TOOLACCESS_Saturation,
	TOOLACCESS_Hue,
/*Eng. menu item*/
	TOOLACCESS_Local_Contrast,
	TOOLACCESS_Dynamic_Contrast,
	TOOLACCESS_VD_Contrast,
	TOOLACCESS_VD_Brightness,
	TOOLACCESS_VD_Saturation,
	TOOLACCESS_VD_Hue,
	TOOLACCESS_Gamma,
	TOOLACCESS_DCTI,
	TOOLACCESS_DLTI,
	TOOLACCESS_MADI_HMC,
	TOOLACCESS_MADI_HME,
	TOOLACCESS_MADI_PAN,
	TOOLACCESS_DI_MA_Adjust_Table,
	TOOLACCESS_DI_DiSmd,
	TOOLACCESS_TNRXC_Ctrl,
	TOOLACCESS_TNRXC_MK2,
	TOOLACCESS_ScaleUPH,
	TOOLACCESS_ScaleUPH_8tap,
	TOOLACCESS_ScaleUPV_6tap,
	TOOLACCESS_ScaleUPV,
	TOOLACCESS_ScaleUPDir,
	TOOLACCESS_ScaleUPDir_Weighting,
	TOOLACCESS_ScaleDOWN_H_Table,
	TOOLACCESS_ScaleDOWN_V_Table,
	TOOLACCESS_ScaleDOWN_444To422,
	TOOLACCESS_D_UVDelay_en,
	TOOLACCESS_D_UVDelay,
	TOOLACCESS_Pattern_Off,
	TOOLACCESS_Pattern_Red,
	TOOLACCESS_Pattern_Green,
	TOOLACCESS_Pattern_Blue,
	TOOLACCESS_Pattern_White,
	TOOLACCESS_Pattern_Black,
	TOOLACCESS_Pattern_Color_Bar,
	TOOLACCESS_Pattern_Gray_Bar,
	TOOLACCESS_Magic_Off,
	TOOLACCESS_SetMagic_Still_DEMO,
	TOOLACCESS_SetMagic_Still_DEMO_Inverse,
	TOOLACCESS_SetMagic_Dynamic_DEMO,
	TOOLACCESS_SetMagic_Move,
	TOOLACCESS_SetMagic_Move_Inverse,
	TOOLACCESS_SetMagic_Zoom,
	TOOLACCESS_SetMagic_Optimize,
	TOOLACCESS_SetMagic_Enhance,
	TOOLACCESS_VD_SetYC_Separation,
	TOOLACCESS_VD_Set3D_Table,
	TOOLACCESS_VD_Set2D_Chroma_BW_Low,
	TOOLACCESS_VD_SetSetDCTI_Table,
	TOOLACCESS_VD_Set2D_Table,
	TOOLACCESS_VD_SetNarrow_BPF_Y,
	TOOLACCESS_VD_SetWide_BPF_Y,
	TOOLACCESS_VD_SetWide_BPF_C,
	TOOLACCESS_VD_SetNarrow_BPF_C,
	TOOLACCESS_VD_SetVflag_Remg_Thr,
	TOOLACCESS_VD_SetVflag_Mag_Thr,
	TOOLACCESS_VD_SetY_Noise_Thr,
	TOOLACCESS_VD_SetNoise_Max_Hdy,
	TOOLACCESS_VD_SetNoise_Y_Add_DC,
	TOOLACCESS_VD_SetBlend_Ratio,
	TOOLACCESS_VD_SetTDma_Enable,
	TOOLACCESS_VD_SetTDma_Mode,
	TOOLACCESS_VD_SetDeb_Print,
	TOOLACCESS_VD_Set625_Mode,
	TOOLACCESS_VD_SetMV_Check,
	TOOLACCESS_VD_SetClamp_Mode,
	TOOLACCESS_VD_SetRc_Rate_Func,
	TOOLACCESS_VD_SetRc_Rate_Value,
	TOOLACCESS_VD_SetV_Clamp,
	TOOLACCESS_VD_SetClamp_Delay,
	TOOLACCESS_VD_SetClamp_Delay_Value,
	TOOLACCESS_VD_SetSetH_State_Write,
	TOOLACCESS_VD_SetHsync_Start,
	TOOLACCESS_VD_SetV_State_Write,
	TOOLACCESS_VD_SetV_Detect_Mode,
	TOOLACCESS_VD_SetVsync_Ctrl,
	TOOLACCESS_VD_SetC_State_W,
	TOOLACCESS_VD_SetAuto_BP,
	TOOLACCESS_VD_SetCkill_Mode,
	TOOLACCESS_VD_SetCkill_Value,
	TOOLACCESS_VD_SetSet_Dgain,
	TOOLACCESS_VD_SetFix_Dgain,
	TOOLACCESS_VD_SetSet_Cgain,
	TOOLACCESS_VD_SetFix_Cgain,
	TOOLACCESS_VD_Set27M_LPF,
	TOOLACCESS_VD_Set27M_Table,
	TOOLACCESS_VD_Set108M_LPF,
	TOOLACCESS_VD_SetSecam_F_sel,
	TOOLACCESS_VD_SetSecam2,
	TOOLACCESS_VD_Set443358pk_sel,
	TOOLACCESS_VD_Set443thr,
	TOOLACCESS_VD_Set358thr,
	TOOLACCESS_VD_SetSoft_Reset,
	TOOLACCESS_VD_PQ_SetBypassPQ,
	TOOLACCESS_VD_PQ_Set1D3D_Pos,
	TOOLACCESS_VD_PQ_SetContrast,
	TOOLACCESS_VD_PQ_SetBrightness,
	TOOLACCESS_VD_PQ_SetSaturation,
	TOOLACCESS_VD_PQ_SetYC_Delay,
	TOOLACCESS_VD_PQ_SetY1delay_En,
	TOOLACCESS_VD_PQ_SetCb1delay_En,
	TOOLACCESS_VD_PQ_SetCr1delay_En,
	TOOLACCESS_VD_PQ_SetY1delay,
	TOOLACCESS_VD_PQ_SetCb1delay,
	TOOLACCESS_VD_PQ_SetCr1delay,
	TOOLACCESS_VD_PQ_SetKill_Ydelay_En,
	TOOLACCESS_VD_PQ_SetKill_Ydelay,
	TOOLACCESS_VD_PQ_SetSCART_Con,
	TOOLACCESS_VD_PQ_SetSCART_Bri,
	TOOLACCESS_VD_PQ_SetSCART_Sat_cr,
	TOOLACCESS_VD_PQ_SetSCART_Sat_cb,
	TOOLACCESS_VD_PQ_Style,
	TOOLACCESS_VD_PQ_TableControl,


	TOOLACCESS_DCC_PQ_current_mean,	/*read only*/
	TOOLACCESS_DCC_PQ_current_dev,	/*read only*/
	TOOLACCESS_DCC_PQ_Level,
	TOOLACCESS_DCC_PQ_Cubic_index,
	TOOLACCESS_DCC_PQ_Cubic_low,
	TOOLACCESS_DCC_PQ_Cubic_high,

	TOOLACCESS_VIP_QUALITY_Coef,                 	/*20*/
	TOOLACCESS_VIP_QUALITY_Extend_Coef,           	/*21*/
	TOOLACCESS_VIP_QUALITY_Extend2_Coef,         	/*22*/
	TOOLACCESS_VIP_QUALITY_Extend3_Coef,        	/*23*/
	TOOLACCESS_tICM_ini,				  			/* 24*/
	TOOLACCESS_SHPTable,			  				/* 25*/
	TOOLACCESS_Hist_Y_Mean_Value,			  		/* 26*/
	TOOLACCESS_TABLE_NTSC_3D,			  			/* 28*/
	TOOLACCESS_MANUAL_NR,			  				/* 29*/
	/*TOOLACCESS_PQA_Input_Table,			 			*/ /* 30*/
	/*TOOLACCESS_PQA_Table,			  				*/ /* 31*/
	TOOLACCESS_SmartPic_clue_Motion_Cal,			/*32*/
	TOOLACCESS_SmartPic_clue_HSI_Histogram_Cal, 	/*33*/
	TOOLACCESS_SmartPic_clue_VD_Noise_Cal, 			/*34*/
	TOOLACCESS_ISR_Print_Ctrl,
	TOOLACCESS_xvYcc_mode,
	TOOLACCESS_BP_Function_CTRL,
	TOOLACCESS_BP_Function_TBL,
	TOOLACCESS_ADV_API_CTRL,
	TOOLACCESS_ADV_API_TBL,
	TOOLACCESS_DBC_STATUS,
	TOOLACCESS_DBC_SW_REG,

	TOOLACCESS_ICM_7axis_adjust,
	TOOLACCESS_switch_DVIandHDMI,
	TOOLACCESS_AutoMA_API_CTRL,

	TOOLACCESS_Debug_Buff_8,
	TOOLACCESS_Debug_Buff_16,
	TOOLACCESS_Debug_Buff_32,
	TOOLACCESS_Debug_HDR,		/* read only*/

	TOOLACCESS_MEMC_ENVIRONMENT_CHECK,
	TOOLACCESS_MEMC_AUTOTEST_INTERNAL_PTG,
	TOOLACCESS_MEMC_AUTOTEST_REAL_PATTERN,
	TOOLACCESS_MEMC_AUTOREGRESSION,

	TOOLACCESS_MAX,
	/*==== from MacArthur3===*/
	TOOLACCESS_GetIdxTable_IDMax = 1024,
	TOOLACCESS_GetIdxTable_Type = 1025,
	TOOLACCESS_GetIdxTable_MaxNum = 1026,
	TOOLACCESS_GetIdxTable_StringName = 1027,


	/* For PQA*/
	TOOLACCESS_PQA_Table_Size = 1028,
	TOOLACCESS_PQA_Table = 1029,
	TOOLACCESS_PQA_Level_Table = 1030,
	TOOLACCESS_PQA_Level_Index_Table = 1031,
	TOOLACCESS_PQA_Input_Table = 1032,
	TOOLACCESS_PQA_FlowCtrl_Input_Type = 1033,
	TOOLACCESS_PQA_FlowCtrl_Input_Item = 1034,
	TOOLACCESS_PQA_SPM_Info_Get = 1035,

	TOOLACCESS_DCC_Curve_Control_Coef = 1100,
	TOOLACCESS_DCC_Boundary_check_Table = 1101,
	TOOLACCESS_DCC_Level_and_Blend_Coef_Table = 1102,/*(DCC_MISC)*/
	TOOLACCESS_DCC_Hist_Factor_Table = 1103,
	TOOLACCESS_DCC_AdaptCtrl_Level_Table = 1104,
	TOOLACCESS_DCC_AdaptCtrl_Param = 1105,

	TOOLACCESS_DCC_UserDef_Ctrl_TABLE = 1106,
	TOOLACCESS_DCC_UserDef_Curve_All_TABLE = 1107,
	TOOLACCESS_DCC_UserDef_Curve_Current_TABLE = 1108,
	TOOLACCESS_DCC_Database_Curve_CRTL_All_Table = 1109,
	TOOLACCESS_DCC_Database_Curve_CRTL_Curve_Current_Table = 1110,
	TOOLACCESS_DCC_Database_Curve_CRTL_Histogram_Current_Table = 1111,
	TOOLACCESS_DCC_Histogram_Curve = 1112,
	TOOLACCESS_DCC_S_Curve = 1113, 									/*read only*/
	TOOLACCESS_DCC_Histogram_Adjust = 1114,							/*read only*/
    	TOOLACCESS_DCC_APL = 1115,								/*read only*/
	TOOLACCESS_DCC_Apply_Curve = 1116,										/*read only*/
	TOOLACCESS_DCC_Curve_Boundary_Type = 1117,
	TOOLACCESS_DCC_Curve_Boundary = 1118,
	TOOLACCESS_DCC_panel_compensation_Curve = 1119,
	TOOLACCESS_DCC_DataBaseHistogrm = 1120,
	TOOLACCESS_DCC_Database_Curve_CRTL_User_Gain_Parameter_Current_Table = 1121,
	TOOLACCESS_VIP_DCC_Advance_Level_Control_Flag = 1122,
	TOOLACCESS_VIP_DCC_Advance_Level_Control_Step_DT_Table = 1123,
	TOOLACCESS_VIP_DCC_Identity_Condition_Check = 1124,

	TOOLACCESS_VIP_I_Main_Dither_Hist_Get = 1150,

	TOOLACCESS_ICM_Global_Adjust = 1200,
	TOOLACCESS_ICM_BOX_Adjust = 1201,
	TOOLACCESS_ICM_ReadWrite_Buf = 1202,
	TOOLACCESS_ICM_ReadWrite_CurrentPrevious_Buf = 1203,

	TOOLACCESS_MPEG_SW_Ctrl = 1230,

	TOOLACCESS_FIML_Result = 1240,

	TOOLACCESS_PCID_RgnTbl = 1252,
	TOOLACCESS_VALC_Tbl = 1254,

	TOOLACCESS_MEMC_CHANGE_SIZE = 1256,

	TOOLACCESS_AI_Ctrl_Tbl = 1260,
// lesley 0906
	TOOLACCESS_AI_Ctrl_Draw = 1261,
// end lesley 0906

	// lesley 1002_1
	TOOLACCESS_AI_INFO = 1262,
	// end lesley 1002_1

	// lesley 1007
	TOOLACCESS_AI_TUNE_ICM = 1263,
	TOOLACCESS_AI_TUNE_DCC = 1264,
	TOOLACCESS_AI_DCC_USER_CURVE = 1265,
	// end lesley 1007
	TOOLACCESS_AI_DB = 1266,
	TOOLACCESS_AI_SCENE_Ctrl = 1270,
	TOOLACCESS_AI_SCENE_Ctrl_test = 1271,
	TOOLACCESS_AI_SCENE_Target = 1272,

	TOOLACCESS_AI_PQMASK_Control = 1275,
	TOOLACCESS_AI_PQMASK_SemanticWeight = 1276,
	TOOLACCESS_AI_PQMASK_DepthWeight = 1277,
	TOOLACCESS_AI_PQMASK_HistInfo = 1278,
	TOOLACCESS_AI_PQMASK_HistData = 1279,

	TOOLACCESS_PQ_SR_MODE_Ctrl = 1280,
	TOOLACCESS_NNSR_Blanding_Idx = 1281,
	TOOLACCESS_NNSR_Weight_Table_Idx = 1282,
	TOOLACCESS_NNSR_Weight_Tbl = 1283,
	TOOLACCESS_NNSR_Weight_Blend = 1284,
	TOOLACCESS_NNSR_load_weight_tbl = 1286,
	TOOLACCESS_NNSR_test_set = 1287,
        TOOLACCESS_siw_PCLRC_Table = 1285,


	TOOLACCESS_BIT_TEST = 1290,

	TOOLACCESS_AI_PQMASK_SQM_TABLE = 1295,
	TOOLACCESS_NNSR_VIP_TABLE = 1296,

	TOOLACCESS_DisplayInformation_table = 1300, 						/*read only*/

	TOOLACCESS_YUV2RGB_Global_Sat_Gain = 1400,
	TOOLACCESS_YUV2RGB_Global_Hue_Gain = 1401,
	TOOLACCESS_YUV2RGB_Global_Contrast_Gain = 1402,
	TOOLACCESS_YUV2RGB_Global_Brightness_Gain = 1403,

	TOOLACCESS_Function_Coef_Dlti = 1500,
	TOOLACCESS_Function_Coef_Dcti = 1501,
	TOOLACCESS_Function_Coef_iDlti = 1502,
	TOOLACCESS_Function_Coef_iDcti = 1503,
	TOOLACCESS_Function_Coef_VDcti = 1504,
	TOOLACCESS_Function_Coef_UV_Delay_En = 1505,
	TOOLACCESS_Function_Coef_UV_Delay = 1506,
	TOOLACCESS_Function_Coef_YUV2RGB = 1507,
	TOOLACCESS_Function_Coef_Film_Mode = 1508,
	TOOLACCESS_Function_Coef_Intra = 1509,
	TOOLACCESS_Function_Coef_MA = 1510,
	TOOLACCESS_Function_Coef_TnrXC = 1511,
//	TOOLACCESS_Function_Coef_Ma_Chroma_Error = 1512,
	TOOLACCESS_Function_Coef_NM_Level = 1513,
	TOOLACCESS_Function_Coef_Madi_Hme = 1514,
	TOOLACCESS_Function_Coef_Madi_Hmc = 1515,
	TOOLACCESS_Function_Coef_Madi_Pan = 1516,
	TOOLACCESS_Function_Coef_Di_Smd = 1517,
	TOOLACCESS_Function_Coef_NEW_UVC = 1518,
	TOOLACCESS_Function_Coef_UV_DELAY_TOP = 1519,
	TOOLACCESS_Function_Coef_V_DCTi_LPF = 1520,

	TOOLACCESS_Function_Extend_CDS = 1521,
	TOOLACCESS_Function_Extend_Emfmk2 = 1522,
	TOOLACCESS_Function_Extend_Dnoise_Table = 1525,
	TOOLACCESS_Function_Extend_Adatp_Gamma = 1526,
	TOOLACCESS_Function_Extend_LD_Ebabel = 1527,
	TOOLACCESS_Function_Extend_LD_Table_Select = 1528,
	TOOLACCESS_Function_Extend_LC_Enable = 1529,
	TOOLACCESS_Function_Extend_LC_Table = 1530,
	TOOLACCESS_Function_Extend_3dLUT_Table = 1531,
	TOOLACCESS_Function_Extend_I_De_XC = 1532,
	TOOLACCESS_Function_Extend_I_De_Contour = 1533,
	TOOLACCESS_Function_Extend_SLD = 1534,
	TOOLACCESS_Function_Extend_Output_InvOutput_GAMMA = 1535,
	TOOLACCESS_Function_Extend_Tone_Mapping_Table = 1536,
	TOOLACCESS_Function_Extend_Reserve16 = 1537,
	TOOLACCESS_Function_Extend_Reserve17 = 1538,
	TOOLACCESS_Function_Extend_Reserve18 = 1539,
	TOOLACCESS_Function_Extend_Reserve19 = 1540,
	TOOLACCESS_Function_Extend_Reserve20 = 1541,

	TOOLACCESS_Function_Extend2_SU_H = 1542,
	TOOLACCESS_Function_Extend2_SU_V = 1543,
	TOOLACCESS_Function_Extend2_S_PK = 1544,
	TOOLACCESS_Function_Extend2_SUPK_Mask = 1545,
	TOOLACCESS_Function_Extend2_Unsharp_Mask = 1546,
	TOOLACCESS_Function_Extend2_Egsm_PostSHP_Table = 1547,
	TOOLACCESS_Function_Extend2_Iegsm_Table = 1548,
	TOOLACCESS_Function_Extend2_SR_Init_Table = 1549,
	TOOLACCESS_Function_Extend2_SR_Edge_Gain = 1550,
	TOOLACCESS_Function_Extend2_SR_TEX_Gain = 1551,
	TOOLACCESS_Function_Extend2_SD_H_Table = 1552,
	TOOLACCESS_Function_Extend2_SD_V_Table = 1553,
	TOOLACCESS_Function_Extend2_SD_444TO422 = 1554,
	TOOLACCESS_Function_Extend2_Color_Space_Control = 1555,
	TOOLACCESS_Function_Extend2_SU_H_8Tab = 1556,
	TOOLACCESS_Function_Extend2_SU_V_6Tab = 1557,
	TOOLACCESS_Function_Extend2_OSD_Sharpness = 1558,
	TOOLACCESS_Function_Extend2_SU_DIR = 1559,
	TOOLACCESS_Function_Extend2_SU_DIR_Weigh = 1560,
	TOOLACCESS_Function_Extend2_SU_C_H = 1561,
	TOOLACCESS_Function_Extend2_SU_C_V = 1562,

	TOOLACCESS_Function_Extend3_VD_CON_BRI_HUE_SAT = 1563,
	TOOLACCESS_Function_Extend3_ICM_Tablee = 1564,
	TOOLACCESS_Function_Extend3_Gamma = 1565,
	TOOLACCESS_Function_Extend3_S_Gamma_Index = 1566,
	TOOLACCESS_Function_Extend3_S_Gamma_Low = 1567,
	TOOLACCESS_Function_Extend3_S_Gamma_High = 1568,
	TOOLACCESS_Function_Extend3_DCC_Table = 1569,
	TOOLACCESS_Function_Extend3_DCC_Color_Indep_t = 1570,
	TOOLACCESS_Function_Extend3_DCC_Chroma_Comp_t = 1571,
	TOOLACCESS_Function_Extend3_Sharpness_Table = 1572,
	TOOLACCESS_Function_Extend3_NR_Table = 1573,
	TOOLACCESS_Function_Extend3_PQA_INPUT_TABLE = 1574,
	TOOLACCESS_Function_Extend3_MB_Peaking = 1575,
	TOOLACCESS_Function_Extend3_blue_stretch = 1576,
	TOOLACCESS_Function_Extend3_Reserve14 = 1577,
	TOOLACCESS_Function_Extend3_Reserve15 = 1578,
	TOOLACCESS_Function_Extend3_HDR = 1579,
	TOOLACCESS_Function_Extend3_AI = 1580,
	TOOLACCESS_Function_Extend3_Reserve18 = 1581,
	TOOLACCESS_Function_Extend3_Reserve19 = 1582,
	TOOLACCESS_Function_Extend3_MB_SU_Peaking = 1583,

	TOOLACCESS_Function_Extend4_Reserve00 = 1584,
	TOOLACCESS_Function_Extend4_Reserve01 = 1585,
	TOOLACCESS_Function_Extend4_Reserve02 = 1586,
	TOOLACCESS_Function_Extend4_Reserve03 = 1587,
	TOOLACCESS_Function_Extend4_Reserve04 = 1588,
	TOOLACCESS_Function_Extend4_Reserve05 = 1589,
	TOOLACCESS_Function_Extend4_Reserve06 = 1590,
	TOOLACCESS_Function_Extend4_Reserve07 = 1591,
	TOOLACCESS_Function_Extend4_Reserve08 = 1592,
	TOOLACCESS_Function_Extend4_Reserve09 = 1593,
	TOOLACCESS_Function_Extend4_Reserve10 = 1594,
	TOOLACCESS_Function_Extend4_Reserve11 = 1595,
	TOOLACCESS_Function_Extend4_Reserve12 = 1696,
	TOOLACCESS_Function_Extend4_Reserve13 = 1697,
	TOOLACCESS_Function_Extend4_Reserve14 = 1698,
	TOOLACCESS_Function_Extend4_Reserve15 = 1699,
	TOOLACCESS_Function_Extend4_Reserve16 = 1600,
	TOOLACCESS_Function_Extend4_Reserve17 = 1601,
	TOOLACCESS_Function_Extend4_Reserve18 = 1602,
	TOOLACCESS_Function_Extend4_Reserve19 = 1603,
	TOOLACCESS_Function_Extend4_Reserve20 = 1604,

	TOOLACCESS_Function_Table3D_Reserve00 = 1605,
	TOOLACCESS_Function_Table3D_Reserve01 = 1606,
	TOOLACCESS_Function_Table3D_Reserve02 = 1607,
	TOOLACCESS_Function_Table3D_Reserve03 = 1608,
	TOOLACCESS_Function_Table3D_Reserve04 = 1609,
	TOOLACCESS_Function_Table3D_Reserve05 = 1610,
	TOOLACCESS_Function_Table3D_Reserve06 = 1611,
	TOOLACCESS_Function_Table3D_Reserve07 = 1612,
	TOOLACCESS_Function_Table3D_Reserve08 = 1613,
	TOOLACCESS_Function_Table3D_Reserve09 = 1614,
	TOOLACCESS_Function_Table3D_Reserve10 = 1615,
	TOOLACCESS_Function_Table3D_Reserve11 = 1616,
	TOOLACCESS_Function_Table3D_Reserve12 = 1617,
	TOOLACCESS_Function_Table3D_Reserve13 = 1618,
	TOOLACCESS_Function_Table3D_Reserve14 = 1619,
	TOOLACCESS_Function_Table3D_Reserve15 = 1620,
	TOOLACCESS_Function_Table3D_Reserve16 = 1621,
	TOOLACCESS_Function_Table3D_Reserve17 = 1622,
	TOOLACCESS_Function_Table3D_Reserve18 = 1623,
	TOOLACCESS_Function_Table3D_Reserve19 = 1624,
	TOOLACCESS_Function_Table3D_Reserve20 = 1625,

	TOOLACCESS_Function_Coef_ALL = 1626,
	TOOLACCESS_Function_Extend_ALL = 1627,
	TOOLACCESS_Function_Extend2_ALL = 1628,
	TOOLACCESS_Function_Extend3_ALL = 1629,
	TOOLACCESS_Function_Extend4_ALL = 1630,
	TOOLACCESS_Function_TABLE3D_ALL = 1631,
	TOOLACCESS_Function_Extend3_DEBUG_LOG_ENABLE = 1632,
	TOOLACCESS_Function_Extend3_DEBUG_LOG_PRINT_DELAY = 1633,
	TOOLACCESS_LOGO_DETECT_GET = 1634,
	TOOLACCESS_GSR_BLK_APL_GET = 1635,

	TOOLACCESS_DumpDi = 1700,
	TOOLACCESS_LocalContrast_Curve = 1710,
	TOOLACCESS_SoftwareBaseLocalContrastCurve = 1712,
	TOOLACCESS_SoftwareBaseLocalContrastHistogram = 1714,
	TOOLACCESS_ColorFacTable = 1720,

	/*below for tv006 request*/
	TOOLACCESS_SRGB_33_Matrix = 1800, /*for assign sRGB 3*3 matrix*/
	TOOLACCESS_ICM_GainAdjust = 1801, /*for ICM gain adjuest*/
	TOOLACCESS_DSE_Gain = 1802, /*for DSE gain*/
	TOOLACCESS_D_3D_LUT_Index = 1803, /*for D domain 3D Lut get index*/
	TOOLACCESS_D_3D_LUT_Reset = 1804, /*for D domain 3D Lut default value reset*/
	TOOLACCESS_OD_Table_33x33 = 1805, /*for OverDrive*/
	TOOLACCESS_OD_Table_17x17 = 1806,
	TOOLACCESS_DI_PAN_detect = 1807,
	TOOLACCESS_ICM_K5_STRUCTURE = 1808,
	/*above for tv006 request*/
	/*for tv006 debug*/
	TOOLACCESS_VPQ_IOCTL_DataAccess_cmd = 1850,

	TOOLACCESS_VPQ_IOCTL_StopRun_cmd = 1860,
	TOOLACCESS_VPQ_LED_IOCTL_StopRun_cmd = 1861,
	TOOLACCESS_VPQ_MEMC_IOCTL_StopRun_cmd = 1862,
	TOOLACCESS_VPQ_HDR_IOCTL_StopRun_cmd = 1863,
	TOOLACCESS_VPQ_dolbyHDR_IOCTL_StopRun_cmd = 1864,
	TOOLACCESS_VPQ_MEMC_DynamicBypass = 1865,
	TOOLACCESS_VPQ_DIGameMode_Only = 1866,
	TOOLACCESS_VPQ_DynamicOptimizeSystem = 1867,
	TOOLACCESS_VPQ_IOCTL_StopRun = 1870,
	/*for demo*/
	TOOLACCESS_DemoOverscan = 1887,
	TOOLACCESS_DemoPQ = 1888,
	TOOLACCESS_ONEKEY_HDR = 1889,
	TOOLACCESS_ONEKEY_BBC = 1890,
	TOOLACCESS_ONEKEY_SDR2HDR = 1891,
	TOOLACCESS_TC_DEMO = 1892,
	TOOLACCESS_OSD_HDRMODE_DEMO = 1893,
	TOOLACCESS_LOCALCONTRAST_DEMO = 1894,
	TOOLACCESS_CHANGE_PQ_FLOW = 1895,

	/* HDR(DM/DM2) */
	/*HDR  3d  17*17*17 lut table,*/
	TOOLACCESS_DM_HDR_3D_Lut_TBL = 1900,
	TOOLACCESS_DM_HDR_1D_Lut_TBL = 1901,//yuan::20151027
  	TOOLACCESS_DM_HDR_1D_Lut_TBL_table1 = 1902,//yuan::20151027
	TOOLACCESS_DM2_HDR_EOTF_TBL = 1903,
	TOOLACCESS_DM2_HDR_OETF_TBL = 1904,
	TOOLACCESS_DM2_HDR_Tone_Mapping_TBL = 1905,
	TOOLACCESS_DM2_HDR_24x24x24_3D_TBL = 1906,
	TOOLACCESS_DM2_HDR_3D_TBL_FromReg = 1907,
	TOOLACCESS_DM2_HDR_Hist_RGB_max = 1908,
	TOOLACCESS_DM2_HDR_Hist_Auto_Mode = 1909,
	TOOLACCESS_DM2_HDR_Hist_Auto_Mode_TBL = 1910,
	TOOLACCESS_DM2_HDR_Sat_Hist_TBL = 1911,
	TOOLACCESS_DM2_HDR_Hue_Hist_TBL = 1912,
	TOOLACCESS_DM2_HDR_Sat_Hue_Hist_Skip = 1913,

	TOOLACCESS_DM2_EOTF_byLuminance_CTRL = 1930,
	TOOLACCESS_DM2_OETF_byLuminance_CTRL = 1931,
	TOOLACCESS_DM2_panel_luminance = 1932,
	/* D Doamin 3d  17*17*17 lut table,*/
	TOOLACCESS_D_3D_Lut_TBL = 1950,

	/*All system Info*/
	TOOLACCESS_VIP_System_Info = 2100,
	TOOLACCESS_VO_Info = 2101,
	TOOLACCESS_DRM_Info_Frame = 2102,
	TOOLACCESS_AVI_Info_Frame = 2103,

	// ICM DMA Debug
	TOOLACCESS_VIP_ICM_DMA_DEBUG = 2150,
	TOOLACCESS_VIP_ICM_RANDOM_TABLE = 2151,
	TOOLACCESS_VIP_D3DLUT_DMA_DEBUG = 2160,
	TOOLACCESS_VIP_D3DLUT_RANDOM_TABLE = 2161,

	/* PQ Power Saving mode*/
	TOOLACCESS_VIP_PQ_Power_Saving = 2200,

	/* PQ AP DIAS eng menu enable*/
	TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_10p_gamma_offset_gain = 2249,
	TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_EN = 2250,
	TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_TEST = 2251,
	TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_DEM_PANEL_INI_TBL = 2252,

	TOOLACCESS_VIP_PQ_AP_DIAS_ENG_MENU_PTG_CTRL = 2260,

	/* for gamma,inv gamma, out gamma, inv out gamma read and write speed up mode*/
	TOOLACCESS_Gamma_CTRL = 2280,
	TOOLACCESS_INV_Gamma_CTRL = 2281,
	TOOLACCESS_Out_Gamma_CTRL = 2282,
	TOOLACCESS_INV_Out_Gamma_CTRL = 2283,

	/* Demura*/
	TOOLACCESS_VIP_DeMura_TBL = 2300,
	TOOLACCESS_VIP_DeMura_INI = 2301,
	TOOLACCESS_VIP_DeMura_TBL_Mode = 2302,
	TOOLACCESS_VIP_DeMura_Adaptive = 2303,
	TOOLACCESS_VIP_DeMura_Channel_Mode = 2304,
	TOOLACCESS_VIP_DeMura_READ_INX_DATA = 2305,

	TOOLACCESS_VIP_SLD = 2350,
	TOOLACCESS_VIP_SLD_BLK_APL = 2351,
	TOOLACCESS_VIP_LEA_apply_filter = 2353,
	TOOLACCESS_VIP_LEA_debug = 2354,
	TOOLACCESS_VIP_LEA = 2355,
	TOOLACCESS_VIP_LEA_logo_flag_map_raw = 2356,
	TOOLACCESS_VIP_LEA_logo_flag_map = 2357,
	TOOLACCESS_VIP_LEA_logo_flag_map_buf = 2358,
	TOOLACCESS_VIP_LEA_logo_demura_counter = 2359,
	TOOLACCESS_VIP_LEA_demura_tbl = 2360,
	TOOLACCESS_VIP_LEA_LD_APL = 2361,
	TOOLACCESS_VIP_LEA_blk_apl_interp = 2362,
	TOOLACCESS_VIP_SLD_Hybrid_logo_Debug_Show = 2363,
	TOOLACCESS_VIP_SLD_swap_location = 2364,
	TOOLACCESS_VIP_SLD_drop_gain = 2365,
	TOOLACCESS_VIP_SLD_Global_SLD_CTRL = 2366,
	TOOLACCESS_VIP_SLD_Global_SLD_INFO = 2367,
	TOOLACCESS_VIP_SLD_Global_SLD_TBL = 2368,

	/* force vTop */
	TOOLACCESS_VIP_Force_vTop = 2400,

	/* DCR control by tool for Debug */
	TOOLACCESS_VIP_ISR_set_PWM_Enable = 2450,
	TOOLACCESS_VIP_set_DCR_Parameter = 2451,
	TOOLACCESS_RADCR_TBL = 2460,
	TOOLACCESS_RADCR_BL_MAP_TBL = 2461,

	/* for ST2094*/
	TOOLACCESS_ST2094_CTRL = 2500,
	TOOLACCESS_ST2094_METADATA = 2501,
	TOOLACCESS_ST2094_METADATA_Curve_Info = 2502,
	TOOLACCESS_ST2094_Basis_OOTF_Info = 2503,
	TOOLACCESS_ST2094_Guided_OOTF_Info = 2504,
	TOOLACCESS_ST2094_OETF_CTRL_Info = 2505,
	TOOLACCESS_ST2094_OETF_Curve = 2506,


	/* for TV006 */

	TOOLACCESS_Sharpness_Table_TV006 = 2600,
	TOOLACCESS_NR_Table_TV006 = 2601,

	TOOLACCESS_OPS_En = 2700,
	TOOLACCESS_OPS_TPC = 2701,
	TOOLACCESS_OPS_LBC = 2702,
	TOOLACCESS_OPS_CPC = 2703,
	TOOLACCESS_OPS_PLC_Curve = 2704,
	TOOLACCESS_OPS_Debug = 2705,

	/* force MEX all En*/
	TOOLACCESS_Force_MEX_En_ALL = 2800,

	/* check PQ function */
	TOOLACCESS_PQ_CHECK_ALL = 3000,
	TOOLACCESS_Set_VIP_Disable_PQ = 3001,
	TOOLACCESS_Set_VIP_Disable_IPQ = 3002,
	TOOLACCESS_Set_VIP_Disable_IPQCurve = 3003,
	TOOLACCESS_SQM_PQA_PARAM = 3010,

	/* For MEMC use */
	TOOLACCESS_Function_MEMC_Performance_Checking_Database = 4000,
	TOOLACCESS_Function_MEMC_Reserve01 = 4001,
	TOOLACCESS_Function_MEMC_Reserve02 = 4002,
	TOOLACCESS_Function_MEMC_Reserve03 = 4003,
	TOOLACCESS_Function_MEMC_Reserve04 = 4004,
	TOOLACCESS_Function_MEMC_Reserve05 = 4005,
	TOOLACCESS_Function_MEMC_Reserve06 = 4006,
	TOOLACCESS_Function_MEMC_Reserve07 = 4007,
	TOOLACCESS_Function_MEMC_Reserve08 = 4008,
	TOOLACCESS_Function_MEMC_Reserve09 = 4009,
	TOOLACCESS_Function_MEMC_Reserve10 = 4010,
	TOOLACCESS_Function_MEMC_Reserve11 = 4011,
	TOOLACCESS_Function_MEMC_Reserve12 = 4012,
	TOOLACCESS_Function_MEMC_Reserve13 = 4013,
	TOOLACCESS_Function_MEMC_Reserve14 = 4014,
	TOOLACCESS_Function_MEMC_Reserve15 = 4015,
	TOOLACCESS_Function_MEMC_Reserve16 = 4016,
	TOOLACCESS_Function_MEMC_Reserve17 = 4017,
	TOOLACCESS_Function_MEMC_Reserve18 = 4018,
	TOOLACCESS_Function_MEMC_Reserve19 = 4019,
	TOOLACCESS_Function_MEMC_Reserve20 = 4020,

	/* debug */
	TOOLACCESS_SIP_1_DEBUG = 5595,
	TOOLACCESS_SIP_2_DEBUG = 5596,

	/* for TV002*/
	TOOLACCESS_TV002_STYLE = 10000,
	TOOLACCESS_BP_CTRL_TV002 = 10001,
	TOOLACCESS_BP_CTRL_TBL_TV002 = 10002,
	TOOLACCESS_BP_STATUS_TV002 = 10003,
	TOOLACCESS_BP_TBL_TV002 = 10004,
	TOOLACCESS_DBC_STATUS_TV002 = 10005,
	TOOLACCESS_DBC_CTRL_TV002 = 10006,
	TOOLACCESS_DBC_SW_REG_TV002 = 10007,
	TOOLACCESS_ID_Detect_result_TV002 = 10008,
	TOOLACCESS_ID_Detect_Apply_flag_TV002 = 10009,
	TOOLACCESS_Zero_D_DBC_STATUS_TV002 = 10010,
	TOOLACCESS_Zero_D_DBC_CTRL_TV002 = 10011,
	TOOLACCESS_Zero_D_DBC_LUT_TV002 = 10012,
	TOOLACCESS_DBC_LUT_TV002 = 10013,
	TOOLACCESS_DBC_DCC_S_High_CMPS_LUT_TV002 = 10014,
	TOOLACCESS_DBC_DCC_S_Index_CMPS_LUT_TV002 = 10015,
	TOOLACCESS_DCC_STYLE_TV002 = 10018,
	TOOLACCESS_DCC_CTRL_TV002 = 10019,
	TOOLACCESS_DCC_Step_byAPL_TV002 = 10020,
	TOOLACCESS_Dynamic_Setting_by_StepCNT_TV002 = 10021,
	TOOLACCESS_WBBL_enable_TV002 = 10022,
	TOOLACCESS_WBBL_table_TV002 = 10023,
	TOOLACCESS_Black_Adjust_TV002 = 10024,
	TOOLACCESS_PQ_Extend_Index_TV002 = 10025,
	TOOLACCESS_PQ_DBC_Item_Check_TV002 = 10026,
	TOOLACCESS_PQ_SC_Item_Check_TV002 = 10027,
	TOOLACCESS_PQ_DCON_DBRI_TV002 = 10028,
	TOOLACCESS_PQ_DYNAMIC_SHP_TV002 =10029,
	TOOLACCESS_PQ_ZERO_DIMMING_IIR_CTRL_TV002 = 10030,
	TOOLACCESS_PQ_Extend_Enable_TV002 = 10031,
	TOOLACCESS_OSD_Alpha_Detect_TV002 = 10032,
	TOOLACCESS_OSD_Alpha_Ratio_TV002 = 10033,
	TOOLACCESS_PQ_DSLC_CTRL_TV002 = 10034,
	TOOLACCESS_PQ_FREQ_DET_TV002 = 10035,
	TOOLACCESS_PQ_VGIP_ISR_STATUS_TV002 = 10036,
	TOOLACCESS_PQ_SRS_SHP_OUT = 10037,
	
	TOOLACCESS_PQ_SoLib_ShareMem_UINT8 = 10100,
	TOOLACCESS_PQ_SoLib_ShareMem_UINT16 = 10101,
	TOOLACCESS_PQ_SoLib_ShareMem_UINT32 = 10102,
	TOOLACCESS_PQ_SoLib_ShareMem_SINT8 = 10103,
	TOOLACCESS_PQ_SoLib_ShareMem_SINT16 = 10104,
	TOOLACCESS_PQ_SoLib_ShareMem_SINT32 = 10105,
	TOOLACCESS_PQ_SoLib_ShareMem_UINT16_ALIGN = 10106,
	TOOLACCESS_PQ_SoLib_ShareMem_UINT32_ALIGN = 10107,
	TOOLACCESS_PQ_SoLib_ShareMem_SINT16_ALIGN = 10108,
	TOOLACCESS_PQ_SoLib_ShareMem_SINT32_ALIGN = 10109,

	/*JUST FOR BRING-UP VERIFY*/
	TOOLACCESS_PQMASK_BRINGUP_VERIFY = 16000,
} VIP_TOOL_ACCESS_INFO_Item;


/*int rtice_get_vip_table(unsigned int id, unsigned short size, unsigned char *num_type, unsigned char *buf);*/
int rtice_get_vip_table(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf);
/*int rtice_set_vip_table(unsigned int id, unsigned short size, unsigned char *buf);*/
int rtice_set_vip_table(ap_data_param rtice_param, unsigned char *buf);
int rtice_set_IdxTable2Buff(unsigned short mode, unsigned short size, unsigned char *num_type, unsigned char *buf);	/*return idx table size*/
int rtice_SetGet_PQA_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size, unsigned char *num_type, unsigned char *buf);	/*return idx table size*/
int rtice_SetGet_YUV2RGB_Table_Info(VIP_TOOL_ACCESS_INFO_Item mode, unsigned char isSet_Flag, unsigned short size,
	unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, SLR_VIP_TABLE *vipTable_ShareMem);	/*return idx table size*/
int rtice_Set_YUV2RGB_Sat(unsigned short satGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Hue(unsigned short hueGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Contrast(unsigned short ContrastGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
int rtice_Set_YUV2RGB_Bri(unsigned short BriGain, _system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table);
void rtice_Covert_ICM_Gain_Struct(CHIP_COLOR_ICM_GAIN_ADJUST_T *pDB, CHIP_COLOR_ICM_GAIN_ADJUST_RTICE_T *pRTICE, unsigned char flag);

int rtice_Get_VIP_System_Info(_system_setting_info *system_setting_info, _RPC_system_setting_info *RPC_system_setting_info, _RPC_clues *RPC_smartPic_clue, SLR_VIP_TABLE *gVip_Table, unsigned int* pVIPSysInfo);
int rtice_Get_VO_Info(_system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int* pVOInfo);
int rtice_Get_HDMI_AVI_Info(_system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int *pAviInfo);
int rtice_Get_HDMI_DRM_Info(_system_setting_info *system_setting_info, SLR_VIP_TABLE *gVip_Table, unsigned int *pDrmInfo);

int  rtice_ICM_ReadWrite_CurrentPrevious_Buff_save(ap_data_param rtice_param, unsigned char *num_type, unsigned int *curPre_buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);
int  rtice_ICM_ReadWrite_From_CurrentPrevious_Buff(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);

int  rtice_SetGet_HDR_DM2(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);

int  rtice_SetGet_PQ_Power_Saving(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);
int  rtice_SetGet_DeMura(ap_data_param *rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);

unsigned char  rtice_SetGet_Froce_vTop_Flag(unsigned char Force_flag, unsigned char isSet_Flag);
char  rtice_SetGet_Froce_vTop_HDMI_DRM(void *pHDMI_DRM, unsigned char isSet_Flag);
int  rtice_SetGet_Froce_vTop_TBL(ap_data_param *rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);
int  rtice_Set_D_LUT_9to17(unsigned int *array);

/* TV 002 function*/
int  rtice_SetGet_BP_TV002(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);
int  rtice_SetGet_DBC_TV002(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);
int  rtice_SetGet_Zero_D_DBC_TV002(ap_data_param rtice_param, unsigned char *num_type, unsigned char *buf, _system_setting_info *system_setting_info, unsigned char isSet_Flag);

int rtice_set_Function_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag);
int rtice_set_Function_Extend_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag);
int rtice_set_Function_Extend2_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag);
int rtice_set_Function_Extend3_Coef(ap_data_param rtice_param, unsigned char *buf, unsigned char isSet_Flag);

int rtice_Load_SIP_1_bin(void);
int rtice_Get_SIP_1_Enable(void);
void rtice_Set_SIP_1_Enable(int enable);

#ifdef __cplusplus
}
#endif


#endif /* __SCALER_LIB_H__*/


