/******************************************************************************
*
*   Copyright(c) 2017 Realtek Semiconductor Corp. All rights reserved.
*
*   @author baker.cheng@realtek.com
*
******************************************************************************/
#ifndef _SE_REG_MAP_H_
#define _SE_REG_MAP_H_

typedef union _SEREG_CMDMEM
{
	uint32_t  Value;
	struct
	{
		uint32_t  Reserved0: 4;
		uint32_t  base: 28;
	} Fields;
} SEREG_CMDMEM;
typedef union _SEREG_SRWORDCNT
{
	uint32_t  Value;
	struct
	{
		uint32_t  srwordcnt: 7;
		uint32_t  Reserved: 25;
	} Fields;
} SEREG_SRWORDCNT;
typedef union _SEREG_Q_PRIORITY
{
	uint32_t  Value;
	struct
	{
		uint32_t  pri1: 2;
		uint32_t  pri2: 2;
		uint32_t  pri3: 2;
		uint32_t  pri4: 2;
		uint32_t  disable: 4;
		uint32_t  Reserved: 20 ;
	} Fields;
} SEREG_Q_PRIORITY;
typedef union _SEREG_CLR_FMT
{
	uint32_t  Value;
	struct
	{
		uint32_t  Format: 4;
		uint32_t  write_enable1: 1;
		uint32_t  alpha_loc: 1;
		uint32_t  write_enable2: 1;
		uint32_t  alpha_loc_rslt: 1;
		uint32_t  write_enable3: 1;
		uint32_t  alpha_loc3: 1;
		uint32_t  write_enable4: 1;
		uint32_t  alpha_loc4: 1;
		uint32_t  write_enable9: 1;
		uint32_t  alpha_loc2: 1;
		uint32_t  write_enable5: 1;
		uint32_t  big_endian_i1: 1;
		uint32_t  write_enable6: 1;
		uint32_t  big_endian_i2: 1;
		uint32_t  write_enable7: 1;
		uint32_t  big_endian_o: 1;
		uint32_t  write_enable8: 1;
		uint32_t  rounding_en: 1;
		uint32_t  write_enable10: 1;
		uint32_t  big_endian_i3: 1;
		uint32_t  write_enable11: 1;
		uint32_t  big_endian_i4: 1;
		uint32_t  write_enable12: 1;
		uint32_t  Reserved2: 5;
	} Fields;
} SEREG_CLR_FMT;
typedef union _SEREG_COLOR_KEY
{
	uint32_t  Value;
	struct
	{
		uint32_t  color_key: 24;
		uint32_t  Reserved: 8;
	} Fields;
} SEREG_COLOR_KEY;
typedef union _SEREG_COLOR
{
	uint32_t  Value;
	struct
	{
		uint32_t  src_color: 32;
	} Fields;
} SEREG_COLOR;
typedef union _SEREG_ALPHA
{
	uint32_t  Value;
	struct
	{
		uint32_t  src_alpha: 8;
		uint32_t  Reserved: 24;
	} Fields;
} SEREG_ALPHA;
typedef union _SEREG_BADDR
{
	uint32_t  Value;
	struct
	{
		uint32_t  baddr: 32;
	} Fields;
} SEREG_BADDR;
typedef union _SEREG_PITCH
{
	uint32_t  Value;
	struct
	{
		uint32_t  pitch: 16;
		uint32_t  interleave: 1;
		uint32_t  Reserved: 15;
	} Fields;
} SEREG_PITCH;
typedef struct _SEREG_DMAINFO
{
	SEREG_BADDR		SeRegBaseAddress[4];
	SEREG_PITCH		SeRegPitch[4];
} SEREG_DMAINFO;
typedef union _SE20REG_BADDR
{
	SEREG_BADDR	Se20Baddr[4];
} SE20REG_BADDR;
typedef union _SE20REG_PITCH
{
	SEREG_PITCH	Se20Pitch[4];
} SE20REG_PITCH;
typedef struct _SE20REG_DMAINFO
{
	SE20REG_BADDR		Se20BaddrSet[16];
	SE20REG_PITCH		Se20PitchSet[16];
} SE20REG_DMAINFO;

typedef union _SEREG_CLUT_LOCK_ST
{
	uint32_t  Value;
	struct
	{
		uint32_t clut_lock_st: 3;
		uint32_t Reserved:29;
	} Fields;
} SEREG_CLUT_LOCK_ST;
typedef union _SEREG_CONVSCALING
{
	uint32_t  Value;
	struct
	{
		uint32_t  vup: 1;
		uint32_t  hup: 1;
		uint32_t  vdown: 1;
		uint32_t  hdown: 1;
		uint32_t  Reserved: 28;
	} Fields;
} SEREG_CONVSCALING;
typedef union _SEREG_CONVCOEF
{
	uint32_t  Value;
	struct
	{
		uint32_t  m0: 13;
		uint32_t  m1: 13;
		uint32_t  Reserved: 6;
	} Fields;
} SEREG_CONVCOEF;
typedef union _SEREG_CONVCOEF6
{
	uint32_t  Value;
	struct
	{
		uint32_t  a0: 10;
		uint32_t  a1: 10;
		uint32_t  a2: 10;
		uint32_t  Reserved: 2;
	} Fields;
} SEREG_CONVCOEF6;
typedef union _SEREG_SRC_CLR
{
	uint32_t  Value;
	struct
	{
		uint32_t  src_clr_mul_red: 9;
		uint32_t  Reserved: 7;
		uint32_t  src_clr_add_red: 9;
		uint32_t  Reserved1: 7;
	} Fields;
} SEREG_SRC_CLR;
typedef union _SEREG_SYNC_VO_MARGIN
{
	uint32_t  Value;
	struct
	{
		uint32_t  margin_VOtoSE: 12;
		uint32_t  Reserved: 14;
		uint32_t  vo_osd_plane: 1;
		uint32_t  vbi: 1;
		uint32_t  after_vo_cur: 1;
		uint32_t  before_vo_cur: 1;
		uint32_t  vo_plane: 1;
		uint32_t  auto_dir_en: 1;
	} Fields;
} SEREG_SYNC_VO_MARGIN;
typedef union _SEREG_SYNC_VO_LOCATION
{
	uint32_t  Value;
	struct
	{
		uint32_t  VO_current_y: 14;
		uint32_t  Reserved: 18;
	} Fields;
} SEREG_SYNC_VO_LOCATION;
typedef union _SEREG_SYNC_VO_Y
{
	uint32_t  Value;
	struct
	{
		uint32_t  VO_total_y: 13;
		uint32_t  Reserved0: 3;
		uint32_t  gdma_dir: 1;
		uint32_t  Reserved: 15;
	} Fields;
} SEREG_SYNC_VO_Y;
typedef union _SEREG_STRETCH
{
	uint32_t  Value;
	struct
	{
		uint32_t  vodd: 1;
		uint32_t  hodd: 1;
		uint32_t  vtype: 1;
		uint32_t  htype: 2;
		uint32_t  ref_alpha: 1;
		uint32_t  repl_color: 1;
		uint32_t  repl_alpha: 1;
		uint32_t  Reserved: 24;
	} Fields;
} SEREG_STRETCH;
typedef union _SEREG_HDHS
{
	uint32_t  Value;
	struct
	{
		uint32_t  H_scaling_dph_lsb: 14;
		uint32_t  H_scaling_dph_msb: 4;
		uint32_t  H_scaling_iniph: 7;
		uint32_t  Reserved: 7;
	} Fields;
} SEREG_HDHS;
typedef union _SEREG_VDHS
{
	uint32_t  Value;
	struct
	{
		uint32_t  V_scaling_dph_lsb: 14;
		uint32_t  V_scaling_dph_msb: 4;
		uint32_t  V_scaling_iniph: 7;
		uint32_t  Reserved: 7;
	} Fields;
} SEREG_VDHS;

typedef struct _SEREG_SEREG_STCH_SET
{
	SEREG_STRETCH			SeStretch[4];
	SEREG_HDHS			SeHdhs[4];
	SEREG_VDHS			SeVdhs[4];
} SEREG_STCH_SET;
typedef union _SEREG_HCOEF
{
	uint32_t  Value;
	struct
	{
		uint32_t  hcoef: 14;
		uint32_t  Reserved: 18;
	} Fields;
} SEREG_HCOEF;
typedef union _SEREG_VCOEF
{
	uint32_t  Value;
	struct
	{
		uint32_t  vcoef: 14;
		uint32_t  Reserved: 18;
	} Fields;
} SEREG_VCOEF;
typedef struct _SEREG_COEFINFO
{
	SEREG_HCOEF		SeRegHCoef[32];
	SEREG_HCOEF		SeRegHCoef32[32];
	SEREG_VCOEF		SeRegVCoef[16];
	SEREG_VCOEF		SeRegVCoef16[16];
} SEREG_COEFINFO;

typedef union _SE20REG_HCOEF
{
	SEREG_HCOEF	Se20HCoef[4];
} SE20REG_HCOEF;

typedef union _SE20REG_VCOEF
{
	SEREG_VCOEF	Se20VCoef[4];
} SE20REG_VCOEF;

typedef struct _SE20REG_COEFINFO
{
	SE20REG_HCOEF		SeRegHCoef[32];
	SE20REG_VCOEF		SeRegVCoef[32];
} SE20REG_COEFINFO;

typedef struct _SEREG_COEFINFO2
{
	SEREG_HCOEF		SeRegHCoef[16];
	SEREG_HCOEF		SeRegHCoef32[16];
	SEREG_VCOEF		SeRegVCoef[16];
	SEREG_VCOEF		SeRegVCoef16[16];
} SEREG_COEFINFO2;
typedef union _SEREG_CTRL
{
	uint32_t  Value;
	struct
	{
		uint32_t  write_data: 1;
		uint32_t  go: 1;
		uint32_t  endiswap: 1;
		uint32_t  Reserved: 29;
	} Fields;
} SEREG_CTRL;
typedef union _SEREG_IDLE
{
	uint32_t  Value;
	struct
	{
		uint32_t  idle: 1;
		uint32_t  Reserved: 31;
	} Fields;
} SEREG_IDLE;
typedef union _SEREG_INTSEL
{
	uint32_t  Value;
	struct
	{
		uint32_t  Sel1: 3;
		uint32_t  Reserved0: 1;
		uint32_t  Sel2: 3;
		uint32_t  Reserved1: 25;
	} Fields;
} SEREG_INTSEL;
typedef union _SEREG_INTS
{
	uint32_t  Value;
	struct
	{
		uint32_t  write_data: 1;
		uint32_t  sync: 1;
		uint32_t  com_err: 1;
		uint32_t  com_empty: 1;
		uint32_t  Reserved1: 12;
		uint32_t  fmt_err: 1;
		uint32_t  vo_sync_err: 1;
		uint32_t  Reserved2: 14;
	} Fields;
} SEREG_INTS;
typedef union _SEREG_INTE
{
	uint32_t  Value;
	struct
	{
		uint32_t  write_data: 1;
		uint32_t  sync: 1;
		uint32_t  com_err: 1;
		uint32_t  com_empty: 1;
		uint32_t  Reserved1: 12;
		uint32_t  fmt_err: 1;
		uint32_t  vo_sync_err: 1;
		uint32_t  Reserved: 14;
	} Fields;
} SEREG_INTE;
typedef union _SEREG_INSTCNT_L
{
	uint32_t  Value;
	struct
	{
		uint32_t instcnt_l;
	} Fields;
} SEREG_INSTCNT_L;
typedef union _SEREG_INSTCNT_H
{
	uint32_t  Value;
	struct
	{
		uint32_t instcnt_h;
	} Fields;
} SEREG_INSTCNT_H;
typedef union _SEREG_CLUT_RADR
{
	uint32_t  Value;
	struct
	{
		uint32_t  radr: 8;
		uint32_t  Reserved: 24;
	} Fields;
} SEREG_CLUT_RADR;
typedef union _SEREG_CLUT_VALUE
{
	uint32_t  Value;
	struct
	{
		uint32_t  value;
	} Fields;
} SEREG_CLUT_VALUE;
typedef union _SEREG_DBG
{
	uint32_t  Value;
	struct
	{
		uint32_t  dbg_sel0: 5;
		uint32_t  Reserved1: 3;
		uint32_t  dbg_sel1: 5;
		uint32_t  Reserved2: 3;
		uint32_t  dbg_module_sel0:4;
		uint32_t  dbg_module_sel1:4;
		uint32_t  dbg_en: 1;
		uint32_t  dbg_change: 1;
		uint32_t  Reserved3: 6;
	} Fields;
} SEREG_DBG;
typedef union _SEREG_SCTCH
{
	uint32_t  Value;
	struct
	{
		uint32_t  Reg0: 16;
		uint32_t  Reg1: 16;
	} Fields;
} SEREG_SCTCH;
typedef union _SEREG_DCU_CTRL
{
	uint32_t  Value;
	struct
	{
		uint32_t  dcu_access_mode: 2;
		uint32_t  Reserved: 30;
	} Fields;
} SEREG_DCU_CTRL;
typedef union _SEREG_READ_MARGIN
{
	uint32_t  Value;
	struct
	{
		uint32_t  rme: 1;
		uint32_t  rm: 4;
		uint32_t  ls: 1;
		uint32_t  Reserved: 26;
	} Fields;
} SEREG_READ_MARGIN;
typedef union _SEREG_BIST
{
	uint32_t  Value;
	struct
	{
		uint32_t  se_bist_0: 1;
		uint32_t  se_bist_1: 1;
		uint32_t  Reserved: 30;
	} Fields;
} SEREG_BIST;
typedef union _SEREG_SE_BIST_0_FAIL
{
	uint32_t  Value;
	struct
	{
		uint32_t  se_bist_0_fail_0: 1;
		uint32_t  se_bist_0_fail_1: 1;
		uint32_t  se_bist_0_fail_2: 1;
		uint32_t  se_bist_0_fail_3: 1;
		uint32_t  se_bist_0_fail_4: 1;
		uint32_t  se_bist_0_fail_5: 1;
		uint32_t  Reserved: 26;
	} Fields;
} SEREG_BIST_0;
typedef union _SEREG_SE_DRF_0_FAIL
{
	uint32_t  Value;
	struct
	{
		uint32_t  se_drf_0_fail_0: 1;
		uint32_t  se_drf_0_fail_1: 1;
		uint32_t  se_drf_0_fail_2: 1;
		uint32_t  se_drf_0_fail_3: 1;
		uint32_t  se_drf_0_fail_4: 1;
		uint32_t  se_drf_0_fail_5: 1;
		uint32_t  se_drf_0_fail_6: 1;
		uint32_t  se_drf_0_fail_7: 1;
		uint32_t  se_drf_0_fail_8: 1;
		uint32_t  Reserved: 23;
	} Fields;
} SEREG_SE_DRF_0_FAIL;
typedef union _SEREG_SE_SWAP
{
	uint32_t  Value;
	struct
	{
		uint32_t  w_1b_en: 1;
		uint32_t  w_2b_en: 1;
		uint32_t  w_4b_en: 1;
		uint32_t  w_8b_en: 1;
		uint32_t  r_1b_en: 1;
		uint32_t  r_2b_en: 1;
		uint32_t  r_4b_en: 1;
		uint32_t  r_8b_en: 1;
		uint32_t  Reserved: 24;
	} Fields;
} SEREG_SWAP;
typedef union _SEREG_DFB_CTRL
{
	uint32_t  Value;
	struct
	{
		uint32_t  src_round: 1;
		uint32_t  dst_round: 1;
		uint32_t  Reserved: 30;
	} Fields;
} SEREG_DFB_CTRL;

#if SE_SE20_EXT
typedef struct _SEREG_INFO
{
	SEREG_CMDMEM			SeCmdBase[4];
	SEREG_CMDMEM			SeCmdLimit[4];
	SEREG_CMDMEM			SeCmdReadPtr[4];
	SEREG_CMDMEM			SeCmdWritePtr[4];
	SEREG_SRWORDCNT			SeSramWordCount[4];
	SEREG_Q_PRIORITY		SeQueuePriority;
	uint32_t				ReservedQueuePriority[3];
	SEREG_CLR_FMT			SeColorFormat[4];
	SEREG_COLOR_KEY			SeColorKey[4];
	SEREG_COLOR_KEY			SeDestColorKey[4];/*80*/
	uint32_t				ReservedDestColorKey[16];/*90~CC*/
	SEREG_COLOR			SeSrcColor[4];/*D0*/
	SEREG_ALPHA			SeSrcAlpha[4];
	SEREG_ALPHA			SeDestAlpha[4];
	SEREG_ALPHA			SeResultAlpha[4];/*100*/
	SEREG_COLOR			SeResultColor[4];/*110*/
	uint32_t				ReservedResultAlpha[24];/*120~17C*/
	SEREG_DMAINFO			SeDmaInfo1[2];/*180~1B0*/
	SEREG_CLUT_LOCK_ST		SeClutLockSt[4];/*1C0, dummy*3*/
	SEREG_CONVSCALING		SeConvScaling[4]; /*1d0*/
	SEREG_CONVCOEF			SeConvCoef1[4];
	SEREG_CONVCOEF			SeConvCoef2[4];
	SEREG_CONVCOEF			SeConvCoef3[4];
	SEREG_CONVCOEF			SeConvCoef4[4];
	SEREG_CONVCOEF			SeConvCoef5[4];
	SEREG_CONVCOEF6			SeConvCoef6[4];
	SEREG_COLOR_KEY			SeFcvBitblitColorKey[4]; /*24*/
	SEREG_SRC_CLR			SeSrcClrRed[4];
	SEREG_SRC_CLR			SeSrcClrGreen[4];
	SEREG_SRC_CLR			SeSrcClrBlue[4];
	SEREG_SRC_CLR			SeSrcClrAlpha[4];/*28*/
	uint32_t				ReservedSrcClrAlpha[12];/*29~2bc*/
	SEREG_ALPHA			SeFBColorAlpha1[4];/*2C*/
	uint32_t				ReservedFBColorAlpha1[8];/*2D~2E*/
	SEREG_STCH_SET			SeStchSet[2];/*2F~34*/
/*
	SEREG_STRETCH			SeStretch[4];
	SEREG_HDHS			SeHdhs[4];
	SEREG_VDHS			SeVdhs[4];
	SEREG_STRETCH			Se20Stretch[4];
	SEREG_HDHS			Se20Hdhs[4];
	SEREG_VDHS			Se20Vdhs[4];
*/
	uint32_t				ReservedStchSet[12];/*35~37*/
	/*[38, 40, 48, 4C], [50, 58, 60, 64]*/
	SEREG_COEFINFO			SeCoefInfo[2];
	/*SE20 [68, 88]*/
	SE20REG_COEFINFO		Se20CoefInfo[1];
	SEREG_COLOR_KEY			SeScBitblitColorKey[4]; /*a8*/
	uint32_t				SeLinearG[20];/*a9~ad*/
	uint32_t				ReservedLinearG[40];/*ae~b7*/
	SE20REG_DMAINFO			Se20DmaInfo;/*b8~D7*/
	SEREG_SWAP			SeSWAP[4];/*D8*/
	SEREG_DFB_CTRL			SeDfbCtrl[4];/*D9*/
	uint32_t				ReservedDfbCtrl[24];/*DADF*/
	uint32_t				SeCacheReq[4];/*e0*/
	uint32_t				SeCacheMiss[4];
	uint32_t				Se20Dummy[4];
	uint32_t				Se20DbgIdle;
	uint32_t				Se20Dbgwdma;
	uint32_t				Se20DbgIntf;
	uint32_t				ReservedDbgIntf;
	uint32_t				Se20DbgPerf;/*e4*/
	uint32_t				Se20Option;
	uint32_t				Se20IdleMask;
	uint32_t				ReservedIdleMask[13];
	SEREG_CTRL			SeCtrl[4]; /*e8*/
	SEREG_IDLE			SeIdle[4];
	SEREG_INTSEL			SeIntSel;
	uint32_t				ReservedIntSel[3];
	SEREG_INTS			SeInts[4];
	SEREG_INTE			SeInte[4];
	SEREG_INSTCNT_L			SeInstCntL[4];
	SEREG_INSTCNT_H			SeInstCntH[4];
	SEREG_CLUT_RADR			SeClutReadAddress;/*ef*/
	SEREG_CLUT_VALUE		SeClutValue;
	SEREG_DBG			SeDebug;
	SEREG_SCTCH			SeSctch;
	uint32_t				SeSFT;/*f0*/
} SEREG_INFO;
#else
/*SE_OLD*/
typedef struct _SEREG_INFO
{
	SEREG_CMDMEM			SeCmdBase[4];
	SEREG_CMDMEM			SeCmdLimit[4];
	SEREG_CMDMEM			SeCmdReadPtr[4];
	SEREG_CMDMEM			SeCmdWritePtr[4];
	SEREG_SRWORDCNT			SeSramWordCount[4];
	SEREG_Q_PRIORITY		SeQueuePriority;
	uint32_t				Reserved0[3];
	SEREG_CLR_FMT			SeColorFormat[4];
	SEREG_COLOR_KEY			SeColorKey[4];
	SEREG_COLOR_KEY			SeDestColorKey[4];
	SEREG_COLOR			SeSrcColor[4];
	SEREG_ALPHA			SeSrcAlpha[4];/*a0*/
	SEREG_ALPHA			SeDestAlpha[4];
	SEREG_ALPHA			SeResultAlpha[4];
	SEREG_DMAINFO			SeDmaInfo1[2];/*d0e0 f0100*/
	uint32_t				Reserved1[4];/*110*/
	SEREG_ALPHA			SeSrcAlpha4[4];/*120*/
	SEREG_ALPHA			SeDestAlpha3[4];/*130*/
	SEREG_COLOR			SeColorKey2[4];/*140*/
	SEREG_COLOR			SeDestColorKey2[4];/*150*/
	SEREG_COLOR			SeColorKey3[4];/*160*/
	SEREG_COLOR			SeDestColorKey3[4];/*170*/
	SEREG_COLOR			SeResultColor[4];/*180*/
	SEREG_COLOR			SeSrcColor3[4];/*190*/
	SEREG_ALPHA			SeSrcAlpha3[4];/*1a0*/
	SEREG_ALPHA			SeDestAlpha2[4];/*1b0*/
	SEREG_COLOR			SeSrcColor4[4];/*1c0*/
	SEREG_CLUT_LOCK_ST		SeClutLockSt[4];/*1d0, dummy*3*/
	SEREG_CONVSCALING		SeConvScaling[4]; /*1e0*/
	SEREG_CONVCOEF			SeConvCoef1[4];
	SEREG_CONVCOEF			SeConvCoef2[4];
	SEREG_CONVCOEF			SeConvCoef3[4];
	SEREG_CONVCOEF			SeConvCoef4[4];
	SEREG_CONVCOEF			SeConvCoef5[4];/*dummy, m1*/
	SEREG_CONVCOEF6			SeConvCoef6[4];
	SEREG_COLOR_KEY			SeFcvBitblitColorKey[4]; /*25*/
	SEREG_SRC_CLR			SeSrcClrRed[4];
	SEREG_SRC_CLR			SeSrcClrGreen[4];
	SEREG_SRC_CLR			SeSrcClrBlue[4];
	SEREG_SRC_CLR			SeSrcClrAlpha[4];
	SEREG_SYNC_VO_MARGIN		SeSyncVoMargin[4]; /*2A*/
	SEREG_SYNC_VO_LOCATION		SeSyncVoLocation[4];
	SEREG_STRETCH			SeStretch[4];
	SEREG_HDHS			SeHdhs[4];
	SEREG_VDHS			SeVdhs[4];
	uint32_t				Reserved2[4];
	SEREG_COEFINFO			SeCoefInfo[2];
	/*[30, 38, 40, 44], [48, 50, 58, 5c]*/
	SEREG_COEFINFO2			SeCoefInfo2[3];
	/*[60, 64, 68, 6c], [70, 74, 78, 7c] dummy[80, 84, 88, 8c]*/
	SEREG_COLOR_KEY			SeScBitblitColorKey[4]; /*90*/
	uint32_t				Reserved3[20];/*91,92,93,94,95*/
	SEREG_CTRL			SeCtrl[4]; /*96*/
	SEREG_IDLE			SeIdle[4];
	SEREG_INTSEL			SeIntSel;
	uint32_t				Reserved4[3];
	SEREG_INTS			SeInts[4];
	SEREG_INTE			SeInte[4];
	SEREG_INSTCNT_L			SeInstCntL[4];
	SEREG_INSTCNT_H			SeInstCntH[4];
	SEREG_CLUT_RADR			SeClutReadAddress;/*9d*/
	SEREG_CLUT_VALUE		SeClutValue;
	SEREG_DBG			SeDebug;
	SEREG_SCTCH			SeSctch;
	uint32_t				Reserved5[8];/*9e,9f*/
#if SE_SE20
	SEREG_READ_MARGIN		SeReadMargin;/*a0*/
	SEREG_BIST			SeBistMode;
	SEREG_BIST			SeBistDone;
	SEREG_BIST			SeBistFailGroup;
	SEREG_BIST_0			SeBist0Fail;/*a1*/
	SEREG_BIST			SeBist1Fail;
	SEREG_BIST			SeDrfMode;
	SEREG_BIST			SeDrfResume;
	SEREG_BIST			SeDrfDone;/*a2*/
	SEREG_BIST			SeDrfPause;
	SEREG_BIST			SeDrfFailGroup;
	SEREG_SE_DRF_0_FAIL		SeDrf0Fail;
	SEREG_BIST			SeDrf1Fail[4];/*a3, dummy*3*/
	uint32_t				Reserved6[68];/*a4~a68, */
	SEREG_ALPHA			SeFBColorAlpha1[4];/*b5*/
	SEREG_SYNC_VO_Y			SeSyncVoY[4];/*b6*/
	SEREG_ALPHA			SeFBColorAlpha2[4];
	SEREG_ALPHA			SeFBColorAlpha3[4];
	uint32_t				Reserved7[36];/*b9~c1*/
	uint32_t				SeLinearG[20];/*c2~c6*/
	uint32_t				Reserved8[36];/*c7~cf*/
	SEREG_DMAINFO			SeDmaInfo2[6];/*d0~db*/
	SEREG_SWAP			SeSWAP[4];/*dc*/
	SEREG_DFB_CTRL			SeDfbCtrl[4];/*dd*/
	uint32_t				Reserved9[8];/*dedf*/
	uint32_t				SeCacheReq[4];/*e0*/
	uint32_t				SeCacheMiss[4];
	uint32_t				SeDummy[4];
	uint32_t				SeDbg[4];
#endif
} SEREG_INFO;

#endif
#define  SE_CONVCOEF1_0_m01(data)		(0x03FFE000&((data)<<13))
#define  SE_CONVCOEF1_0_m00(data)		(0x00001FFF&(data))
#define  SE_CONVCOEF2_0_m10(data)		(0x03FFE000&((data)<<13))
#define  SE_CONVCOEF2_0_m02(data)		(0x00001FFF&(data))
#define  SE_CONVCOEF3_0_m12(data)		(0x03FFE000&((data)<<13))
#define  SE_CONVCOEF3_0_m11(data)		(0x00001FFF&(data))
#define  SE_CONVCOEF4_0_m21(data)		(0x03FFE000&((data)<<13))
#define  SE_CONVCOEF4_0_m20(data)		(0x00001FFF&(data))
#define  SE_CONVCOEF5_0_m22(data)		(0x00001FFF&(data))

#define  SE_CONVCOEF6_0_a2(data)		(0x3FF00000&((data)<<20))
#define  SE_CONVCOEF6_0_a1(data)		(0x000FFC00&((data)<<10))
#define  SE_CONVCOEF6_0_a0(data)		(0x000003FF&(data))

#endif
