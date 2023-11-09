/*=============================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2009
  * All rights reserved.
  * ============================================ */

/*================= File Description ================= */
/**
 * @file
 * 	This file is for the main function and some initial functions.
 *
 * @author 	$Author: Jennifer $
 * @date 	$Date: 2016/01/05 16:30 $
 * @version 	$Revision: 0.1 $
 * @ingroup 	VIDEO_DECODER
 */

/**
 * @addtogroup vdc
 * @{
 */

/*================ Module dependency  =============== */
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/delay.h>

//#include <rbus/rbusVDTopReg.h>
//#include <rbus/rbusVDPQReg.h>
//#include <rbus/crt_reg.h>
#include <rbus/vdtop_reg.h>//sunray
#include <rbus/vdpq_reg.h>//sunray
#include <rbus/sys_reg_reg.h>//sunray
#include <rbus/pll27x_reg_reg.h>
#include <rbus/pll_reg_reg.h>
#include <tvscalercontrol/vdc/video.h>
#include <tvscalercontrol/vdc/video_table.h>
#include <tvscalercontrol/vdc/video_input.h>
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scaler/modeState.h>
#include <tvscalercontrol/scaler/scalertimer.h>

/*==================== Definitions ================ */
#define _ENABLE 			1
#define _DISABLE			0

/*================ VIDEO_DECODER tables  =============== */
void module_vdc_TV_DCTIeh_Table_write(unsigned short *table)
{
	/* 9f30 ~ 9f40 */

	vdpq_dcti_filter_coef_b1_RBUS dcti_filter_coef_b1_reg;
	vdpq_dcti_filter_coef_b2_RBUS dcti_filter_coef_b2_reg;
	vdpq_dcti_filter_coef_b3_RBUS dcti_filter_coef_b3_reg;
	vdpq_dcti_filter_coef_a2_RBUS dcti_filter_coef_a2_reg;
	vdpq_dcti_filter_coef_a3_RBUS dcti_filter_coef_a3_reg;

	dcti_filter_coef_b1_reg.regValue = vdcf_inl(VDPQ_DCTI_FILTER_COEF_B1_reg);
	dcti_filter_coef_b1_reg.dcti_filter_coef_b1 = table[0];
	vdcf_outl(VDPQ_DCTI_FILTER_COEF_B1_reg, dcti_filter_coef_b1_reg.regValue);

	dcti_filter_coef_b2_reg.regValue = vdcf_inl(VDPQ_DCTI_FILTER_COEF_B2_reg);
	dcti_filter_coef_b2_reg.dcti_filter_coef_b2 = table[1];
	vdcf_outl(VDPQ_DCTI_FILTER_COEF_B2_reg, dcti_filter_coef_b2_reg.regValue);

	dcti_filter_coef_b3_reg.regValue = vdcf_inl(VDPQ_DCTI_FILTER_COEF_B3_reg);
	dcti_filter_coef_b3_reg.dcti_filter_coef_b3 = table[2];
	vdcf_outl(VDPQ_DCTI_FILTER_COEF_B3_reg, dcti_filter_coef_b3_reg.regValue);

	dcti_filter_coef_a2_reg.regValue = vdcf_inl(VDPQ_DCTI_FILTER_COEF_A2_reg);
	dcti_filter_coef_a2_reg.dcti_filter_coef_a2 = table[3];
	vdcf_outl(VDPQ_DCTI_FILTER_COEF_A2_reg, dcti_filter_coef_a2_reg.regValue);

	dcti_filter_coef_a3_reg.regValue = vdcf_inl(VDPQ_DCTI_FILTER_COEF_A3_reg);
	dcti_filter_coef_a3_reg.dcti_filter_coef_a3 = table[4];
	vdcf_outl(VDPQ_DCTI_FILTER_COEF_A3_reg, dcti_filter_coef_a3_reg.regValue);

#if 0
	VDC_DEBUG_MSG(VDC_MSG_DEBUG,"DCTIeh 9f30(B1) : %x\n", vdcf_inl(VDPQ_DCTI_FILTER_COEF_B1_VADDR));
	VDC_DEBUG_MSG(VDC_MSG_DEBUG,"DCTIeh 9f34(B2) : %x\n", vdcf_inl(VDPQ_DCTI_FILTER_COEF_B2_VADDR));
	VDC_DEBUG_MSG(VDC_MSG_DEBUG,"DCTIeh 9f38(B3) : %x\n", vdcf_inl(VDPQ_DCTI_FILTER_COEF_B3_VADDR));
	VDC_DEBUG_MSG(VDC_MSG_DEBUG,"DCTIeh 9f3c(A2) : %x\n", vdcf_inl(VDPQ_DCTI_FILTER_COEF_A2_VADDR));
	VDC_DEBUG_MSG(VDC_MSG_DEBUG,"DCTIeh 9f40(A3) : %x\n", vdcf_inl(VDPQ_DCTI_FILTER_COEF_A3_VADDR));
#endif
}

unsigned char VDC_DCTI_SET1[18] =
//PAL M ,PALN NTSC50,NTSC443,PAL60   disable the function
{
	0,	//[0] : 9f0c[0]
	0,	//[1] : 9f0c[1]
	1,	//[2] : 9f0c[2]
	1,	//[3] : 9f0c[5]
	0x03,	//[4] : 9f14[3:0]
	0x06,	//[5] : 9f14[7:4]
	0x06,	//[6] : 9f18[3:0]
	0x07,	//[7] : 9f18[7:4]
	0x05,	//[8] : 9f1c[3:0]
	0x06,	//[9] : 9f1c[7:6]
	0x0f,	//[10] : 9f20[3:0]
	0x0f,	//[11] : 9f20[7:6]
	0x00,	//[12] : 9f24[3:0]
	0x05,	//[13] : 9f24[7:6]
	0x09,	//[14] : 9f28[3:0]
	0x03,	//[15] : 9f28[7:6]
	0x09,	//[16] : 9f2c[3:0]
	0x08,	//[17] : 9f2c[7:6]
};

unsigned char  VDC_DCTI_SET2[18] =
//ntsc pal i  , av setting
{
	0,	//[0] : 9f0c[0]
	1,	//[1] : 9f0c[1]
	1,	//[2] : 9f0c[2]
	1,	//[3] : 9f0c[5]
	0x06,	//[4] : 9f14[3:0]	//rock_rau 20120827 0023745: [Video] Please change CTI coring level
	0x02,	//[5] : 9f14[7:4]
	0x06,	//[6] : 9f18[3:0]
	0x07,	//[7] : 9f18[7:4]
	0x05,	//[8] : 9f1c[3:0]
	0x06,	//[9] : 9f1c[7:6]
	0x0f,	//[10] : 9f20[3:0]
	0x0f,	//[11] : 9f20[7:6]
	0x00,	//[12] : 9f24[3:0]
	0x00,	//[13] : 9f24[7:6]
	0x07,	//[14] : 9f28[3:0]
	0x02,	//[15] : 9f28[7:6]
	0x03,	//[16] : 9f2c[3:0]
	0x03,	//[17] : 9f2c[7:6]
};

unsigned char VDC_DCTI_SET3[18] =
//ntsc pal i  , tv setting
{
	1,	//[0] : 9f0c[0]
	1,	//[1] : 9f0c[1]
	1,	//[2] : 9f0c[2]
	1,	//[3] : 9f0c[5]
	0x03,	//[4] : 9f14[3:0]
	0x02,	//[5] : 9f14[7:4]
	0x06,	//[6] : 9f18[3:0]
	0x07,	//[7] : 9f18[7:4]
	0x05,	//[8] : 9f1c[3:0]
	0x06,	//[9] : 9f1c[7:6]
	0x0f,	//[10] : 9f20[3:0]
	0x0f,	//[11] : 9f20[7:6]
	0x00,	//[12] : 9f24[3:0]
	0x00,	//[13] : 9f24[7:6]
	0x09,	//[14] : 9f28[3:0]
	0x03,	//[15] : 9f28[7:6]
	0x07,	//[16] : 9f2c[3:0]
	0x02,	//[17] : 9f2c[7:6]
};

unsigned char  VDC_DCTI_SET4[18] =
//secam
{
	1,	//[0] : 9f0c[0]
	1,	//[1] : 9f0c[1]
	1,	//[2] : 9f0c[2]
	1,	//[3] : 9f0c[5]
	0x03,	//[4] : 9f14[3:0]	//rock_rau 20120906 mantis: //rock_rau 20120827 mantis:0023745
	0x0b,	//[5] : 9f14[7:4]
	0x06,	//[6] : 9f18[3:0]
	0x07,	//[7] : 9f18[7:4]
	0x05,	//[8] : 9f1c[3:0]
	0x06,	//[9] : 9f1c[7:6]
	0x0f,	//[10] : 9f20[3:0]
	0x0f,	//[11] : 9f20[7:6]
	0x04,	//[12] : 9f24[3:0]
	0x00,	//[13] : 9f24[7:6]
	0x07,	//[14] : 9f28[3:0]
	0x02,	//[15] : 9f28[7:6]
	0x03,	//[16] : 9f2c[3:0]
	0x03,	//[17] : 9f2c[7:6]
};

unsigned char  VDC_DCTI_SET5[18] =
//LG TV ntsc setting
{
    /*
	1,	//[0] : 9f0c[0]
	1,	//[1] : 9f0c[1]
	0,	//[2] : 9f0c[2]
	1,	//[3] : 9f0c[5]
	0x06,	//[4] : 9f14[3:0]	//rock_rau 20120827 0023745: [Video] Please change CTI coring level
	0x02,	//[5] : 9f14[7:4]
	0x01,	//[6] : 9f18[3:0]
	0x03,	//[7] : 9f18[7:4]
	0x03,	//[8] : 9f1c[3:0]
	0x02,	//[9] : 9f1c[7:6]
	0x0f,	//[10] : 9f20[3:0]
	0x0f,	//[11] : 9f20[7:6]
	0x04,	//[12] : 9f24[3:0]
	0x05,	//[13] : 9f24[7:6]
	0x00,	//[14] : 9f28[3:0]
	0x03,	//[15] : 9f28[7:6]
	0x02,	//[16] : 9f2c[3:0]
	0x04,	//[17] : 9f2c[7:6]
	*/
	1,	//[0] : 9f0c[0]
    1,	//[1] : 9f0c[1]
    1,	//[2] : 9f0c[2]
    1,	//[3] : 9f0c[5]
    0x03,	//[4] : 9f14[3:0]
    0x04,	//[5] : 9f14[7:4]
    0x02,	//[6] : 9f18[3:0]
    0x03,	//[7] : 9f18[7:4]
    0x0A,	//[8] : 9f1c[3:0]
    0x0F,	//[9] : 9f1c[7:6]
    0x07,	//[10] : 9f20[3:0]
    0x07,	//[11] : 9f20[7:6]
    0x00,	//[12] : 9f24[3:0]
    0x0F,	//[13] : 9f24[7:6]
    0x0B,	//[14] : 9f28[3:0]
    0x0D,	//[15] : 9f28[7:6]
    0x04,	//[16] : 9f2c[3:0]
    0x02,	//[17] : 9f2c[7:6]
};


unsigned short  VDC_DCTI_Enhance_3db_1level[5] =
//// 3db ~~1 level
{
	//0x016a, 0x0e95, 0x0000, 0x0fca, 0x0000,
	0x00ec, 0x0627, 0x00ec, 0x0101, 0x005f,//for LG internal signal TV NTSC colorbar
};


unsigned short  VDC_DCTI_Enhance_3db_2level[5] =
//// 3db ~~2 level
{
	//0x00de, 0x0e44, 0x00dc, 0x0f67, 0x005f,
	//0x110, 0x6ae, 0x1a8, 0x729, 0xdb,
	0x6ae, 0x6ae, 0x1a8, 0x6c4, 0x0c2 // 1710, 1710, 424, 1732, 194
	//0x188, 0x676, 0x000, 0x00d9, 0x0c4 // 392, 1654, 0, 217, 196
};

unsigned short  VDC_DCTI_Enhance_9db_1level[5] =
//// 9db ~~1 level
{
	0x0188, 0x0e76, 0x0000, 0x00d9, 0x0000,
};

unsigned short  VDC_DCTI_Enhance_9db_2level[5] =
//// 9db ~~2 level
{
	0x00d1, 0x0e5e, 0x00d1, 0x0083, 0x005d,
};

unsigned short  VDC_DCTI_Enhance_6db_1level[5] =
//// 6db ~~1 level
{
	0x0174, 0x0e8a, 0x0000, 0x0f80, 0x0000,
};

unsigned short  VDC_DCTI_Enhance_6db_2level[5] =
//// 6db ~~2 level
{
	0x00d3, 0x0e5a, 0x00d3, 0x0000, 0x0059,
};

unsigned short  VDC_DCTI_Enhance_12db_1level[5] =
//// 12db ~~1 level
{
	0x0100, 0x0600, 0x0100, 0x02a0, 0x037d,
};

unsigned short  VDC_DCTI_Enhance_12db_2level[5] =
//// 12db ~~2 level
{
	0x00ec, 0x0e27, 0x00ec, 0x0101, 0x006e,
};

#ifdef VD_hdto_adjust
UINT32 VDC_HDTO_table[4][6] = { // mode: NTSC / PAL / PAL-M / PAL-N
	{
		// NTSC
		0x205D903C,	// +200Hz
		0x219555F0,	// +800Hz
		0x22a00000,	// +1.6KHz	0x23896554
		0x1F8295F4,	// -200Hz
		0x1E4AFA58,	// -800Hz
		0x1CB54750	// -1.6KHz
	},
	{
		// NTSC
		0x205D903C,	// +200Hz
		0x219555F0,	// +800Hz
		0x23896554,	// +1.6KHz
		0x1F8295F4,	// -200Hz
		0x1E4AFA58,	// -800Hz
		0x1CB54750	// -1.6KHz
	},
	{
		// NTSC
		0x205D903C,	// +200Hz
		0x219555F0,	// +800Hz
		0x23896554,	// +1.6KHz
		0x1F8295F4,	// -200Hz
		0x1E4AFA58,	// -800Hz
		0x1CB54750	// -1.6KHz
	},
	{
		// NTSC
		0x205D903C,	// +200Hz
		0x219555F0,	// +800Hz
		0x23896554,	// +1.6KHz
		0x1F8295F4,	// -200Hz
		0x1E4AFA58,	// -800Hz
		0x1CB54750	// -1.6KHz
	},
};
#endif

//#define VD_INPUT_FILTER_SIZE 	72

unsigned short INPUT_FILTER_55MHZ[72] = {
	0x0000,	0x0080,//00
	0x0770,	0x0320,
	0x09c0,	0x0028,
	0x1ff3,	0x1fe1,
	0x1FD1,	0x1FD1,//10
	0x1FE8,	0x0010,
	0x0039,	0x0048,
	0x0030,	0x1FF4,
	0x1FAF,	0x1F86,//20
	0x1F9C,	0x1FF1,
	0x0067,	0x00C0,
	0x00C0,	0x004D,
	0x1F86,	0x1EC4,//30
	0x1E79,	0x1F07,
	0x0086,	0x02B7,
	0x0509,	0x06CD,
	0x0776,	0x0000,//40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_60MHZ[72] = {
	0x0000,	0x0070,//00
	0x0310,	0x0780,
	0x09e0,	0x000a,
	0x003E,	0x0018,
	0x0006,	0x1FE3,//10
	0x1FCC,	0x1FD2,
	0x1FFA,	0x0031,
	0x0056,	0x0049,
	0x0008,	0x1FB0,//20
	0x1F77,	0x1F8D,
	0x1FF6,	0x0084,
	0x00DF,	0x00BC,
	0x000B,	0x1F14,//30
	0x1E65,	0x1E95,
	0x1FF4,	0x0256,
	0x0510,	0x073A,
	0x080C,	0x0000,//40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_65MHZ[72] = {
	0x0000,	0x0030,//00
	0x02c0,	0x07c0,
	0x0a90,	0x1FDB,
	0x0012,	0x0026,
	0x0031,	0x0022,//10
	0x1FFA,	0x1FCF,
	0x1FC2,	0x1FE3,
	0x0025,	0x005D,
	0x005C,	0x0014,//20
	0x1FA9,	0x1F66,
	0x1F89,	0x0012,
	0x00B4,	0x00FA,
	0x008F,	0x1F8C,//30
	0x1E82,	0x1E40,
	0x1F61,	0x01E1,
	0x0506,	0x07A2,
	0x08A5,	0x0000,//40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_70MHZ[72] = {
	0x0000,	0x0020,//00
	0x02b0,	0x07d0,
	0x0ab0,	0x1FF3,
	0x1FC2,	0x1FEF,
	0x0008,	0x002A,//10
	0x002E,	0x000B,
	0x1FD3,	0x1FB4,
	0x1FCF,	0x001D,
	0x0066,	0x0068,//20
	0x0011,	0x1F93,
	0x1F53,	0x1F9A,
	0x004F,	0x00F4,
	0x00F4,	0x0015,//30
	0x1ECC,	0x1E12,
	0x1ED8,	0x0160,
	0x04EA,	0x0802,
	0x093C,	0x0000,//40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_75MHZ[72] = {
	0x0000,	0x0010,//00
	0x0290,	0x07d0,
	0x0ad0,	0x0023,
	0x1FE8,	0x1FD4,
	0x1FD2,	0x1FF2,//10
	0x0021,	0x0039,
	0x001C,	0x1FDA,
	0x1FAA,	0x1FC2,
	0x001D,	0x0073,//20
	0x006E,	0x1FFC,
	0x1F6F,	0x1F4A,
	0x1FCF,	0x00AD,
	0x0125,	0x009D,//30
	0x1F3D,	0x1E0D,
	0x1E5E,	0x00D2,
	0x04BD,	0x085C,
	0x09D4,	0x0000,//40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_80MHZ[72] = {
	0x0000,  0x1eb1, // 00
	0x0090,  0x0930,
	0x0f00,  0x0007,
	0x0001,  0x1ff9,
	0x1ff2,  0x1ff7, // 10
	0x000a,  0x001d,
	0x0017,  0x1ff5,
	0x1fcf,  0x1fcf,
	0x0004,  0x004c, // 20
	0x005d,  0x0012,
	0x1f98,  0x1f5f,
	0x1fbd,  0x0086,
	0x010f,  0x00ad, // 30
	0x1f63,  0x1e23,
	0x1e4f,  0x00af,
	0x04ac,  0x086f,
	0x09f9,  0x0000, //40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_85MHZ[72] = {
	0x0000,  0x1eb1, // 00
	0x0090,  0x0930,
	0x0f00,  0x0000,
	0x0003,  0x0004,
	0x0000,  0x1ff9, // 10
	0x1ff5,  0x0000,
	0x0014,  0x001a,
	0x0000,  0x1fd9,
	0x1fcf,  0x0000, // 20
	0x004a,  0x0059,
	0x0000,  0x1f81,
	0x1f68,  0x0000,
	0x00dd,  0x010c, // 30
	0x0000,  0x1e66,
	0x1dee,  0x0000,
	0x0458,  0x08c9,
	0x0aaa,  0x0000, //40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_60_75MHZ[72] = {
	0x0000,  0x1eb1, // 00
	0x0090,  0x0930,
	0x0f00,  0x000a,
	0x0078,  0x001c,
	0x0000,  0x1fd7, // 10
	0x1fbf,  0x1fce,
	0x0000,  0x003f,
	0x0062,  0x004d,
	0x0000,  0x1fa3, // 20
	0x1f6e,  0x1f8e,
	0x0000,  0x0090,
	0x00e5,  0x00b7,
	0x0000,  0x1f0d, // 30
	0x1e68,  0x1ea2,
	0x0000,  0x0253,
	0x04f4,  0x0707,
	0x07d1,  0x0000, //40
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

unsigned short INPUT_FILTER_216_75MHZ[72] = {
	0x1fc1,	0x1f71,//00
	0x1f51,	0x0010,
	0x01f0,	0x0490,
	0x06f0,	0x07f0,
	0x1ff6,	0x1ffd,//10
	0x0009,	0x0012,
	0x0007,	0x1fee,
	0x1fe2,	0x1ffd,
	0x0029,	0x0032,//20
	0x1ffe,	0x1fb8,
	0x1fba,	0x0019,
	0x1FCF,	0x005f,
	0x1fbe,	0x1f3a,//30
	0x1f8c,	0x0097,
	0x0145,	0x0087,
	0x1eb3,	0x1dab,
	0x1f6e,	0x0409,//40
	0x0911,	0x0b3e,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//50
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//60
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//70
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,//80
	0x0000,	0x0000,
	0x0000,	0x0000,
	0x0000,	0x0000,
};

