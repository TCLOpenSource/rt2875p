
#include <linux/slab.h>
#include <rtk_kdriver/io.h>

#include "demod_rtk_common.h"
#include "atsc3_demod_rtk_r.h"
#include "atsc3_demod_rtk_r_base.h"
//#include "rbus/atb_demod_time_reg.h"

stRtkaAtsc3Config gstRTK_R_Atsc3Config;
static unsigned char u8sATSC3Bandwidth = 6;


unsigned char Atsc3LdpcAlphaFixModeStatus = 0;
static unsigned int L1DataDump[256] = {0};

//#define ATSC3GETPLPWMOREINFO


/***************************default register value group for dvbt and dvbt2 *********************************************/
const unsigned short Atsc3BandWidthFilter[3][81] = {
	{0x000, 0x000, 0x000, 0x000, 0xfff, 0xfff, 0x000, 0x001, 0x000, 0xfff, 0x000, 0x002, 0x001, 0xffe, 0xffe, 0x001, 0x003, 0x000, 0xffd, 0xfff, 0x004, 0x002, 0xffc, 0xffc, 0x003, 0x006, 0xfff, 0xff9, 0xffe, 0x007, 0x005, 0xff9, 0xff8, 0x005, 0x00b, 0xfff, 0xff3, 0xffd, 0x00d, 0x009, 0xff4, 0xff2, 0x008, 0x013, 0xffe, 0xfea, 0xffa, 0x017, 0x00f, 0xfec, 0xfe7, 0x00d, 0x021, 0xffd, 0xfda, 0xff5, 0x026, 0x01b, 0xfdf, 0xfd5, 0x016, 0x03a, 0xffd, 0xfbc, 0xfea, 0x046, 0x035, 0xfc1, 0xfa9, 0x02a, 0x07b, 0xffd, 0xf63, 0xfc5, 0x0bb, 0x0a3, 0xf2d, 0xe8b, 0x0e2, 0x503, 0x719}, /*6MHz 24*/
	{0x000, 0x000, 0x001, 0x000, 0xfff, 0x000, 0x001, 0x001, 0xfff, 0xffe, 0x001, 0x002, 0x000, 0xffd, 0xfff, 0x003, 0x002, 0xffd, 0xffc, 0x002, 0x005, 0x000, 0xffa, 0xffe, 0x006, 0x005, 0xffb, 0xff9, 0x002, 0x009, 0x001, 0xff6, 0xffb, 0x00a, 0x00a, 0xff9, 0xff2, 0x003, 0x010, 0x003, 0xfef, 0xff5, 0x00f, 0x012, 0xff6, 0xfe9, 0x002, 0x01b, 0x009, 0xfe6, 0xfec, 0x016, 0x01f, 0xff4, 0xfd9, 0xffe, 0x02b, 0x013, 0xfd7, 0xfda, 0x020, 0x038, 0xff2, 0xfbb, 0xff4, 0x04c, 0x02c, 0xfb9, 0xfaf, 0x035, 0x079, 0xff1, 0xf61, 0xfd0, 0x0c1, 0x09a, 0xf24, 0xe91, 0x0ed, 0x501, 0x70d}, /*7MHz 24*/
	{0x000, 0x001, 0x001, 0xfff, 0xfff, 0x001, 0x002, 0x000, 0xffe, 0xfff, 0x003, 0x002, 0xffe, 0xffd, 0x001, 0x004, 0x001, 0xffb, 0xffd, 0x004, 0x005, 0xffd, 0xff9, 0x001, 0x008, 0x003, 0xff8, 0xffa, 0x006, 0x00a, 0xffd, 0xff4, 0xfff, 0x00d, 0x007, 0xff4, 0xff4, 0x008, 0x011, 0xffe, 0xfec, 0xffa, 0x013, 0x00e, 0xff0, 0xfea, 0x008, 0x01c, 0x002, 0xfe2, 0xff2, 0x01c, 0x01b, 0xfec, 0xfd9, 0x006, 0x02e, 0x00c, 0xfd2, 0xfe0, 0x027, 0x034, 0xfe9, 0xfbb, 0xffd, 0x04f, 0x024, 0xfb2, 0xfb5, 0x03d, 0x076, 0xfe7, 0xf60, 0xfda, 0x0c6, 0x092, 0xf1d, 0xe96, 0x0f7, 0x4ff, 0x703}, /*8MHz 24*/
};

const unsigned short Atsc3PreamleStruct[160][3] = {
	{  8,  192, 16 }, //00
	{  8,  192, 16 }, //01
	{  8,  192, 16 }, //02
	{  8,  192, 16 }, //03
	{  8,  192, 16 }, //04
	{  8,  384,  8 }, //05
	{  8,  384,  8 }, //06
	{  8,  384,  8 }, //07
	{  8,  384,  8 }, //08
	{  8,  384,  8 }, //09
	{  8,  512,  6 }, //10
	{  8,  512,  6 }, //11
	{  8,  512,  6 }, //12
	{  8,  512,  6 }, //13
	{  8,  512,  6 }, //14
	{  8,  768,  4 }, //15
	{  8,  768,  4 }, //16
	{  8,  768,  4 }, //17
	{  8,  768,  4 }, //18
	{  8,  768,  4 }, //19
	{  8, 1024,  3 }, //20
	{  8, 1024,  3 }, //21
	{  8, 1024,  3 }, //22
	{  8, 1024,  3 }, //23
	{  8, 1024,  3 }, //24
	{  8, 1536,  4 }, //25
	{  8, 1536,  4 }, //26
	{  8, 1536,  4 }, //27
	{  8, 1536,  4 }, //28
	{  8, 1536,  4 }, //29
	{  8, 2048,  3 }, //30
	{  8, 2048,  3 }, //31
	{  8, 2048,  3 }, //32
	{  8, 2048,  3 }, //33
	{  8, 2048,  3 }, //34
	{ 16,  192, 32 }, //35
	{ 16,  192, 32 }, //36
	{ 16,  192, 32 }, //37
	{ 16,  192, 32 }, //38
	{ 16,  192, 32 }, //39
	{ 16,  384, 16 }, //40
	{ 16,  384, 16 }, //41
	{ 16,  384, 16 }, //42
	{ 16,  384, 16 }, //43
	{ 16,  384, 16 }, //44
	{ 16,  512, 12 }, //45
	{ 16,  512, 12 }, //46
	{ 16,  512, 12 }, //47
	{ 16,  512, 12 }, //48
	{ 16,  512, 12 }, //49
	{ 16,  768,  8 }, //50
	{ 16,  768,  8 }, //51
	{ 16,  768,  8 }, //52
	{ 16,  768,  8 }, //53
	{ 16,  768,  8 }, //54
	{ 16, 1024,  6 }, //55
	{ 16, 1024,  6 }, //56
	{ 16, 1024,  6 }, //57
	{ 16, 1024,  6 }, //58
	{ 16, 1024,  6 }, //59
	{ 16, 1536,  4 }, //60
	{ 16, 1536,  4 }, //61
	{ 16, 1536,  4 }, //62
	{ 16, 1536,  4 }, //63
	{ 16, 1536,  4 }, //64
	{ 16, 2048,  3 }, //65
	{ 16, 2048,  3 }, //66
	{ 16, 2048,  3 }, //67
	{ 16, 2048,  3 }, //68
	{ 16, 2048,  3 }, //69
	{ 16, 2432,  3 }, //70
	{ 16, 2432,  3 }, //71
	{ 16, 2432,  3 }, //72
	{ 16, 2432,  3 }, //73
	{ 16, 2432,  3 }, //74
	{ 16, 3072,  4 }, //75
	{ 16, 3072,  4 }, //76
	{ 16, 3072,  4 }, //77
	{ 16, 3072,  4 }, //78
	{ 16, 3072,  4 }, //79
	{ 16, 3648,  4 }, //80
	{ 16, 3648,  4 }, //81
	{ 16, 3648,  4 }, //82
	{ 16, 3648,  4 }, //83
	{ 16, 3648,  4 }, //84
	{ 16, 4096,  3 }, //85
	{ 16, 4096,  3 }, //86
	{ 16, 4096,  3 }, //87
	{ 16, 4096,  3 }, //88
	{ 16, 4096,  3 }, //89
	{ 32,  192, 32 }, //90
	{ 32,  192, 32 }, //91
	{ 32,  192, 32 }, //92
	{ 32,  192, 32 }, //93
	{ 32,  192, 32 }, //94
	{ 32,  384, 32 }, //95
	{ 32,  384, 32 }, //96
	{ 32,  384, 32 }, //97
	{ 32,  384, 32 }, //98
	{ 32,  384, 32 }, //99
	{ 32,  512, 24 }, //100
	{ 32,  512, 24 }, //101
	{ 32,  512, 24 }, //102
	{ 32,  512, 24 }, //103
	{ 32,  512, 24 }, //104
	{ 32,  768, 16 }, //105
	{ 32,  768, 16 }, //106
	{ 32,  768, 16 }, //107
	{ 32,  768, 16 }, //108
	{ 32,  768, 16 }, //109
	{ 32, 1024, 12 }, //110
	{ 32, 1024, 12 }, //111
	{ 32, 1024, 12 }, //112
	{ 32, 1024, 12 }, //113
	{ 32, 1024, 12 }, //114
	{ 32, 1536,  8 }, //115
	{ 32, 1536,  8 }, //116
	{ 32, 1536,  8 }, //117
	{ 32, 1536,  8 }, //118
	{ 32, 1536,  8 }, //119
	{ 32, 2048,  6 }, //120
	{ 32, 2048,  6 }, //121
	{ 32, 2048,  6 }, //122
	{ 32, 2048,  6 }, //123
	{ 32, 2048,  6 }, //124
	{ 32, 2432,  6 }, //125
	{ 32, 2432,  6 }, //126
	{ 32, 2432,  6 }, //127
	{ 32, 2432,  6 }, //128
	{ 32, 2432,  6 }, //129
	{ 32, 3072,  8 }, //130
	{ 32, 3072,  8 }, //131
	{ 32, 3072,  8 }, //132
	{ 32, 3072,  8 }, //133
	{ 32, 3072,  8 }, //134
	{ 32, 3072,  3 }, //135
	{ 32, 3072,  3 }, //136
	{ 32, 3072,  3 }, //137
	{ 32, 3072,  3 }, //138
	{ 32, 3072,  3 }, //139
	{ 32, 3648,  8 }, //140
	{ 32, 3648,  8 }, //141
	{ 32, 3648,  8 }, //142
	{ 32, 3648,  8 }, //143
	{ 32, 3648,  8 }, //144
	{ 32, 3648,  3 },  //145
	{ 32, 3648,  3 },  //146
	{ 32, 3648,  3 },  //147
	{ 32, 3648,  3 },  //148
	{ 32, 3648,  3 },  //149
	{ 32, 4096,  3 },  //150
	{ 32, 4096,  3 },  //151
	{ 32, 4096,  3 },  //152
	{ 32, 4096,  3 },  //153
	{ 32, 4096,  3 },  //154
	{ 32, 4864,  3 },  //155
	{ 32, 4864,  3 },  //156
	{ 32, 4864,  3 },  //157
	{ 32, 4864,  3 },  //158
	{ 32, 4864,  3 },  //159
};

const unsigned short NCP_table[32][8] = {
	{8,  192,  16, 6432,  6342,  6253,  6164,  6075 },
	{8,  384,  8,  6000,  5916,  5833,  5750,  5667 },
	{8,  512,  6,  5712,  5632,  5553,  5474,  5395 },
	{8,  768,  4,  5136,  5064,  4993,  4922,  4851 },
	{8,  1024, 3,  4560,  4496,  4433,  4370,  4307 },
	{8,  1536, 4,  5136,  5064,  4993,  4922,  4851 },
	{8,  2048, 3,  4560,  4496,  4433,  4370,  4307 },
	{16, 192,  32, 13296, 13110, 12927, 12742, 12558},
	{16, 384,  16, 12864, 12684, 12507, 12328, 12150},
	{16, 512,  12, 12576, 12400, 12227, 12052, 11878},
	{16, 768,  8,  12000, 11832, 11667, 11500, 11334},
	{16, 1024, 6,  11424, 11264, 11107, 10948, 10790},
	{16, 1536, 4,  10272, 10128, 9987,  9844,  9702 },
	{16, 2048, 3,  9120,  8992,  8867,  8740,  8614 },
	{16, 2432, 3,  9120,  8992,  8867,  8740,  8614 },
	{16, 3072, 4,  10272, 10128, 9987,  9844,  9702 },
	{16, 3648, 4,  10272, 10128, 9987,  9844,  9702 },
	{16, 4096, 3,  9120,  8992,  8867,  8740,  8614 },
	{32, 192,  32, 26592, 26220, 25854, 25484, 25116},
	{32, 384,  32, 26592, 26220, 25854, 25484, 25116},
	{32, 512,  24, 26304, 25936, 25574, 25208, 24844},
	{32, 768,  16, 25728, 25368, 25014, 24656, 24300},
	{32, 1024, 12, 25152, 24800, 24454, 24104, 23756},
	{32, 1536, 8,  24000, 23664, 23334, 23000, 22668},
	{32, 2048, 6,  22848, 22528, 22214, 21896, 21580},
	{32, 2432, 6,  22848, 22528, 22214, 21896, 21580},
	{32, 3072, 8,  24000, 23664, 23334, 23000, 22668},
	{32, 3072, 3,  18240, 17984, 17734, 17480, 17228},
	{32, 3648, 8,  24000, 23664, 23334, 23000, 22668},
	{32, 3648, 3,  18240, 17984, 17734, 17480, 17228},
	{32, 4096, 3,  18240, 17984, 17734, 17480, 17228},
	{32, 4864, 3,  18240, 17984, 17734, 17480, 17228}
};

const unsigned short NCB_table[15][19] = {
	{8, 0, 6913, 4560, 4560, 5136, 5136, 5712, 5712, 6000, 6000, 6288, 6288, 6432, 6432, 6576, 6576, 6648, 6648 },
	{8, 1, 6817, 4496, 4496, 5064, 5064, 5632, 5632, 5916, 5916, 6200, 6200, 6342, 6342, 6484, 6484, 6555, 6555 },
	{8, 2, 6721, 4433, 4433, 4993, 4993, 5553, 5553, 5833, 5833, 6113, 6113, 6253, 6253, 6393, 6393, 6463, 6463 },
	{8, 3, 6625, 4370, 4370, 4922, 4922, 5474, 5474, 5750, 5750, 6026, 6026, 6164, 6164, 6302, 6302, 6371, 6371 },
	{8, 4, 6529, 4307, 4307, 4851, 4851, 5395, 5395, 5667, 5667, 5939, 5939, 6075, 6075, 6211, 6211, 6279, 6279 },
	{16, 0, 13825, 9120, 9120, 10272, 10272, 11424, 11424, 12000, 12000, 12576, 12576, 12864, 12864, 13152, 13152, 13296, 13296},
	{16, 1, 13633, 8992, 8992, 10128, 10128, 11264, 11264, 11832, 11832, 12400, 12400, 12684, 12684, 12968, 12968, 13110, 13110},
	{16, 2, 13441, 8867, 8867, 9987, 9987, 11107, 11107, 11667, 11667, 12227, 12227, 12507, 12507, 12787, 12787, 12927, 12927},
	{16, 3, 13249, 8740, 8740, 9844, 9844, 10948, 10948, 11500, 11500, 12052, 12052, 12328, 12328, 12604, 12604, 12742, 12742},
	{16, 4, 13057, 8614, 8614, 9702, 9702, 10790, 10790, 11334, 11334, 11878, 11878, 12150, 12150, 12422, 12422, 12558, 12558},
	{32, 0, 27649, 18240, 0,   0,    0,    22848, 0,    24000, 0,    25152, 0,    25728, 0,    26304, 0,    26592, 0    },
	{32, 1, 27265, 17984, 0,   0,    0,    22528, 0,    23664, 0,    24800, 0,    25368, 0,    25936, 0,    26220, 0    },
	{32, 2, 26881, 17734, 0,   0,    0,    22214, 0,    23334, 0,    24454, 0,    25014, 0,    25574, 0,    25854, 0    },
	{32, 3, 26497, 17480, 0,   0,    0,    21896, 0,    23000, 0,    24104, 0,    24656, 0,    25208, 0,    25484, 0    },
	{32, 4, 26113, 17228, 0,   0,    0,    21580, 0,    22668, 0,    23756, 0,    24300, 0,    24844, 0,    25116, 0    }
};

const unsigned short  NCD_table[15][19] = {
	{8, 0, 6913, 5711, 6285, 5999, 6429, 6287, 6573, 6431, 6645, 6575, 6717, 6647, 6753, 6719, 6789, 6755, 6807 },
	{8, 1, 6817, 5631, 6197, 5915, 6339, 6199, 6481, 6341, 6552, 6483, 6623, 6554, 6660, 6625, 6694, 6661, 6714 },
	{8, 2, 6721, 5552, 6110, 5832, 6250, 6112, 6390, 6252, 6460, 6392, 6530, 6462, 6565, 6532, 6600, 6567, 6619 },
	{8, 3, 6625, 5473, 6023, 5749, 6161, 6025, 6299, 6163, 6368, 6301, 6437, 6370, 6473, 6439, 6506, 6474, 6524 },
	{8, 4, 6529, 5394, 5936, 5666, 6072, 5938, 6208, 6074, 6276, 6210, 6344, 6278, 6378, 6346, 6412, 6380, 6429 },
	{16, 0, 13825, 11423, 12573, 11999, 12861, 12575, 13149, 12863, 13293, 13151, 13437, 13295, 13509, 13439, 13581, 13511, 13617},
	{16, 1, 13633, 11263, 12397, 11831, 12681, 12399, 12965, 12683, 13107, 12967, 13249, 13109, 13320, 13251, 13391, 13322, 13428},
	{16, 2, 13441, 11106, 12224, 11666, 12504, 12226, 12784, 12506, 12924, 12786, 13064, 12926, 13134, 13066, 13204, 13136, 13239},
	{16, 3, 13249, 10947, 12049, 11499, 12325, 12051, 12601, 12327, 12739, 12603, 12877, 12741, 12946, 12879, 13015, 12948, 13051},
	{16, 4, 13057, 10789, 11875, 11333, 12147, 11877, 12419, 12149, 12555, 12421, 12691, 12557, 12759, 12693, 12827, 12761, 12861},
	{32, 0, 27649, 22847, 0,    0,    0,    25151, 0,    25727, 0,    26303, 0,    26591, 0,    26879, 0,    27023, 0    },
	{32, 1, 27265, 22527, 0,    0,    0,    24799, 0,    25367, 0,    25935, 0,    26219, 0,    26503, 0,    26645, 0    },
	{32, 2, 26881, 22213, 0,    0,    0,    24453, 0,    25013, 0,    25573, 0,    25853, 0,    26133, 0,    26273, 0    },
	{32, 3, 26497, 21895, 0,    0,    0,    24103, 0,    24655, 0,    25207, 0,    25483, 0,    25759, 0,    25897, 0    },
	{32, 4, 26113, 21579, 0,    0,    0,    23755, 0,    24299, 0,    24843, 0,    25115, 0,    25387, 0,    25523, 0    }
};

const unsigned int Atsc3GITable[16] = {0, 192, 384, 512, 768, 1024, 1536, 2048, 2432, 3072, 3648, 4096, 4864, 0, 0, 0};

void
BuildRtkrAtsc3Module(
	ATSC3_DEMOD_MODULE **ppDemod,
	ATSC3_DEMOD_MODULE *pDvbtDemodModuleMemory,
	BASE_INTERFACE_MODULE *pBaseInterfaceModuleMemory,
	I2C_BRIDGE_MODULE *pI2cBridgeModuleMemory,
	unsigned char DeviceAddr,
	unsigned int CrystalFreqHz
)
{
	ATSC3_DEMOD_MODULE *pDemod;
	RTK_R_ATSC3_EXTRA_MODULE *pExtra;


	// Set demod module pointer,
	*ppDemod = pDvbtDemodModuleMemory;

	// Get demod module.
	pDemod = *ppDemod;

	// Get demod extra module.
	pExtra = &(pDemod->Extra.RtkaAtsc3);


	pDemod->pBaseInterface = pBaseInterfaceModuleMemory;
	pDemod->pI2cBridge     = pI2cBridgeModuleMemory;


	// Set demod type.
	pDemod->DemodType = ATSC3_DEMOD_TYPE_REALTEK_R;

	// Set demod I2C device address.
	pDemod->DeviceAddr = DeviceAddr;

	// Set demod crystal frequency in Hz.
	pDemod->CrystalFreqHz = CrystalFreqHz;

	// Set demod TS interface mode.
	pDemod->TsInterfaceMode = NO_USE;


	// Initialize demod parameter setting status
	pDemod->IsIfFreqHzSet      = NO;
	pDemod->IsSpectrumModeSet  = NO;


	pDemod->m_ldpcIterMode = 0;
	pDemod->m_ldpcIterNum = 50;


	// Build I2C birdge module.


	pDemod->GetCrystalFreqHz        = atsc3_demod_default_GetCrystalFreqHz;
	// Set demod module manipulating function pointers with default functions.
	//pDemod->GetBandwidthMode        = atsc3_demod_default_GetBandwidthMode;



	// Set demod module manipulating function pointers with default functions.
	pDemod->GetDemodType     = atsc3_demod_default_GetDemodType;
	pDemod->GetDeviceAddr    = atsc3_demod_default_GetDeviceAddr;
	pDemod->GetCrystalFreqHz = atsc3_demod_default_GetCrystalFreqHz;

	pDemod->GetIfFreqHz      = atsc3_demod_default_GetIfFreqHz;
	pDemod->GetSpectrumMode  = atsc3_demod_default_GetSpectrumMode;


	// Set demod module manipulating function pointers with particular functions.
	pDemod->IsConnectedToI2c  = realtek_r_atsc3_IsConnectedToI2c;
	pDemod->SoftwareReset     = realtek_r_atsc3_SoftwareReset;



	// Set demod module manipulating function pointers with particular functions.
	pDemod->Initialize              = realtek_r_atsc3_Initialize;
	pDemod->SetBandwidthMode        = realtek_r_atsc3_SetBandwidthMode;

	pDemod->IsSignalLocked          = realtek_r_atsc3_IsSignalLocked;
	pDemod->IsSignalSyncLocked  = realtek_r_atsc3_IsSignalSyncLock;
	pDemod->IsL1Locked  = realtek_r_atsc3_IsL1Lock;

	pDemod->GetSignalStrength       = realtek_r_atsc3_GetSignalStrength;
	pDemod->GetLayerSignalQuality   = realtek_r_atsc3_GetLayerSignalQuality;

	pDemod->GetBer             = realtek_r_atsc3_GetBer;
	pDemod->GetPer             = realtek_r_atsc3_GetPer;
	pDemod->GetSnrDb                = realtek_r_atsc3_GetSnrDb;

	pDemod->SetAgcPol                = realtek_r_atsc3_SetAgcPol;
	pDemod->GetIfAgc                = realtek_r_atsc3_GetIfAgc;

	pDemod->GetTrOffsetPpm          = realtek_r_atsc3_GetTrOffsetPpm;
	pDemod->GetCrOffsetHz           = realtek_r_atsc3_GetCrOffsetHz;

	pDemod->GetCrOffsetHz           = realtek_r_atsc3_GetCrOffsetHz;

	pDemod->SetTsInterfaceMode      = realtek_r_atsc3_SetTsInterfaceMode;
	pDemod->SetIfFreqHz             = realtek_r_atsc3_SetIfFreqHz;
	pDemod->SetSpectrumMode         = realtek_r_atsc3_SetSpectrumMode;
	pDemod->SetPlpID            = realtek_r_atsc3_SetPlpID;
	pDemod->GetPlpID            = realtek_r_atsc3_GetPlpID;
	pDemod->GetSignalParams              = realtek_r_atsc3_GetSignalParams;
	pDemod->GetTsThroughput              = realtek_r_atsc3_TSBitAccu;


	return;
}


/**

@see   DVBT_DEMOD_FP_GET_CRYSTAL_FREQ_HZ

*/
/*
void
realtek_r_atsc3_GetCrystalFreqHz(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pCrystalFreqHz
)
{
	// Get demod crystal frequency in Hz from demod module.
	*pCrystalFreqHz = pDemod->CrystalFreqHz;
	return;
}

*/

void
realtek_r_atsc3_IsConnectedToI2c(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{

	/*
	unsigned long ReadingValue;



	// Set reading value to zero, and get SYS_VERSION value.
	// Note: Use GetRegBits() to get register bits with page setting.
	ReadingValue = 0;

	if (pDemod->RegAccess.Addr16Bit.GetRegBits(pDemod, ATSC_SYS_VERSION, &ReadingValue) != FUNCTION_SUCCESS)
		goto error_status_get_demod_registers;


	// Compare SYS_VERSION value with RTD299S_ATSC_SYS_VERSION_VALUE.
	if (ReadingValue == RTD299S_ATSC_SYS_VERSION_VALUE)
		*pAnswer = YES;
	else
		*pAnswer = NO;


	return;


	error_status_get_demod_registers:

	*pAnswer = NO;
	*/
	return;
}




/**

@see   DVBT_DEMOD_FP_INITIALIZE

*/
int
realtek_r_atsc3_Initialize(
	ATSC3_DEMOD_MODULE *pDemod
)
{
	int ret = FUNCTION_SUCCESS;


	/*Demodulator TS interface  setting	*/
	gstRTK_R_Atsc3Config.stMPEGTSMode.ui8TSTransferType           = RTK_R_ATSC3_TS_PARALLEL_MODE;
	gstRTK_R_Atsc3Config.stMPEGTSMode.ui8OutputEdge               = RTK_R_ATSC3_TS_OUTPUT_FALLING_EDGE;
	gstRTK_R_Atsc3Config.stMPEGTSMode.ui8SPIClockConstantOutput   = RTK_R_ATSC3_TS_CLOCK_CONST_OUTPUT;
	gstRTK_R_Atsc3Config.stMPEGTSMode.ui8TSSPIMSBSelection        = RTK_R_ATSC3_TS_SPI_MSB_ON_DATA_BIT7;
	gstRTK_R_Atsc3Config.stMPEGTSMode.ui8TSSSIOutputSelection     = RTK_R_ATSC3_TS_SSI_OUTPUT_ON_DATA_BIT0;
	gstRTK_R_Atsc3Config.stMPEGTSMode.ui8TSClockVariableMode = RTK_R_ATSC3_TS_CLOCK_VARIABLE;             /*TS clock frequency fixed or variable with real TS rate*/
	/* 	//Demodulator DVB-T/T2 setting*/
	gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8IQSwapMode = RTK_R_ATSC3_INPUT_IQ_NORMAL;
	gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8TxIFAgcPolarity = RTK_R_ATSC3_IF_AGC_POLARITY_NORMAL;
	gstRTK_R_Atsc3Config.Atsc3TunerInput.u32IFFrequencyKHz = 5000; /*5MHz, Here the unit is KHz,  Should be equal to the input of Tuner IF frequency.*/
	gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8BandwidthType = RTK_R_ATSC3_BANDWIDTH_8MHZ;


	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816643c, 0xFFFFFE00, 0x000000D8);  //bsd_sync_th_weight
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816646C, 0xFFFF0000, 0x000001e0);   //BSD thd, free_ctrl_v4[15:0]=480
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160060, 0x00000002); //atsc3_bsd_rst_en  reset dcorr only

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816ef40, 0x00000040); // PLPA VLD=1 / ID=0
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816e810, 0xFFFFFEFF, 0x00000100); //Plpa_force_mode=1, enable from 0416_r5395
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816ef2c, 0x010c2040); //re-assign plp_id by Plp_dump_all, bit31:disable OIF filter, bit24:VV550 patch
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0xFFFFFFF5, 0x0000000a);
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0xFF01FFFF, 0x00FE0000);
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0x40FFFFFF, 0x81000000); //set debug mode for 2nd frame flag
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e80c, 0xFFFFFFFF); // plp_suben_lsb
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8163634, 0xFFFFFFEF, 0x00000000);  //param_h_t_remove_qerr_flag_a3
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816e45c, 0xFFFFE000, 0x00000020); //atsc3_plp0_conti_err_rst_thd

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb81614f0, 0x00020000);

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816c990, 0xFFFFFFFE, 0x00000001);  //params_average_cp_en
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8163448, 0xFFFFFF00, 0x00000020);  //params_cfo_dpll_c1_converge_16k=0x20
	ret |= rtd_4bytes_i2c_mask(pDemod, 0x18163450, 0xFFFFFF00, 0x00000020);  //params_cfo_dpll_c1_converge_32k=0x20
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816344c, 0xFFFFFF00, 0x00000040);  //params_cfo_dpll_c2_converge_16k=0x40
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8163454, 0xFFFFFF00, 0x00000040);  //params_cfo_dpll_c2_converge_32k=0x40

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816c9b4, 0xffffff00, 0x00000008); //params_feq_sfi_det_th_1
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8166464, 0xfffffeff, 0x00000000); //free_ctrl_v2[8], bsd fcfo bypass peak check

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000004b); // ldpc_cfg_addr 0x4B
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x00000048); // ldpc_cfg_dat
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000004c); // ldpc_cfg_addr 0x4C
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x0000000d); // ldpc_cfg_dat
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000004a); // ldpc_cfg_addr 0x4A
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x00000001); // ldpc_cfg_dat
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en

	//set LDPC alpha manual mode to 0xfff for all mode; disable manual mode for default.
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000028); // ldpc_cfg_addr 0x28
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x000000ff); // ldpc_cfg_dat
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000029); // ldpc_cfg_addr 0x29
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x0000000f); // ldpc_cfg_dat
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000027); // ldpc_cfg_addr 0x29
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x00000001); // ldpc_cfg_dat: turn off
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
	Atsc3LdpcAlphaFixModeStatus = 0;


	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8161c84, 0xffffffef, 0x00000010); //Oif_error_mode

	//for field stream 20170602_8MHz_-70.0dBm_atsc3.0_768.00M_131541_0000.dat
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816C8CC, 0xFFFFFF00, 0x00000014); //params_noise_change_var_last_factor_fo = 0x14
	ret |= rtd_4bytes_i2c_mask(pDemod,  0xB8166510, 0xFFFFFFFE, 0x00000001); //dagc_impulse_detection_bypass

	ret |= rtd_4bytes_i2c_mask(pDemod,  0xb816db74, 0xFFFFFFFE, 0x00000000); //params_t2_dcci_modify_flag

	//sync CE setting from matlab
	ret |= rtd_4bytes_i2c_mask(pDemod,  0xB816C8F4, 0xFFFFFFFC, 0x00000001); //params_always_c_pilots_cfo_ppm_est
	ret |= rtd_4bytes_i2c_mask(pDemod,  0xB8163860, 0x0000FF00, 0x00400037); //params_chan_st_dy_state_ofdm_num_a3, params_noise_change_var_min_32k_a3, params_noise_change_var_min_8k_a3
	ret |= rtd_4bytes_i2c_mask(pDemod,  0xB816D9B0, 0xFFFFFFFC, 0x00000001); //params_cp_average_det_mode
	ret |= rtd_4bytes_i2c_mask(pDemod,  0xB8163764, 0x00FFFFFF, 0x25000000); //params_cp_average_ofdm_num_dpll_a3

	ret |= rtd_4bytes_i2c_wr(pDemod,  0xB816E414, 0x07270E00); //atsc3_ber_per_window 1 sec



	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160010, 0xFFFFFFFE, 0x00000001);  //softcfg_done_r
	//reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP

	return ret;

}



/**

@see   DVBT_DEMOD_FP_SET_TS_INTERFACE_MODE

*/
int
realtek_r_atsc3_SetTsInterfaceMode(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_MPEGTSMode stTSMode
)
{

	unsigned int DataTmp = 0;
	int ret = FUNCTION_SUCCESS;

	//ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8161c84, &DataTmp);
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8161c84, &DataTmp);

	if (stTSMode.ui8SPIClockConstantOutput == RTK_R_ATSC3_TS_CLOCK_CONST_OUTPUT) {
		DataTmp = DataTmp & (0xFFFFFFDF);
	} else {
		DataTmp = DataTmp | (0x00000020);
	}


	if (stTSMode.ui8TSTransferType == RTK_R_ATSC3_TS_SERIAL_MODE) {
		DataTmp = (DataTmp & 0xFFFFFFFC) | 0x3;
	} else {
		DataTmp = (DataTmp & 0xFFFFFFFC);
	}


	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161c84, DataTmp);


	return ret;
}


/**

@see   DVBT_DEMOD_FP_SET_IF_FREQ_HZ

*/
int
realtek_r_atsc3_SetIfFreqHz(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int IfFreqHz
)
{
	char  ui8FreqLifHigh = 0;
	unsigned int ui32FreqLifLow = 0;
	RTK_R_ATSC3_64Data  i64Data, i64Data1, i64Rst;
	unsigned int ui32TunerIFKHz = 5000;
	unsigned int ui32ParamsIFFreq = 0;
	unsigned int CrystalFreqHz = 0;
	int ret = FUNCTION_SUCCESS;



	ui32TunerIFKHz = (unsigned int)(IfFreqHz / 1000);
	// Get demod crystal frequency in Hz.
	pDemod->GetCrystalFreqHz(pDemod, &CrystalFreqHz);
	gstRTK_R_Atsc3Config.ui32DemodClkKHz = (unsigned int)(CrystalFreqHz / 1000);


	/* 	ui32TunerIFKHz = 5000;*/
	/*freq Calculate*/
	if (ui32TunerIFKHz < gstRTK_R_Atsc3Config.ui32DemodClkKHz) { /*use equation dbTunerIF/dbDemodClk*/
		RTK_R_ATSC3_64Mult(&i64Data, RTK_R_ATSC3_FixPow(2, 30), ui32TunerIFKHz * 64);
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = gstRTK_R_Atsc3Config.ui32DemodClkKHz;
		i64Rst  = RTK_R_ATSC3_64DivReturn64(i64Data, i64Data1);
	} else { /*use equation (dbTunerIF-dbDemodClk)/dbDemodClk*/
		RTK_R_ATSC3_64Mult(&i64Data, RTK_R_ATSC3_FixPow(2, 30), (ui32TunerIFKHz - gstRTK_R_Atsc3Config.ui32DemodClkKHz) * 64);
		i64Data1.ui32High = 0;
		i64Data1.ui32Low = gstRTK_R_Atsc3Config.ui32DemodClkKHz;
		i64Rst  = RTK_R_ATSC3_64DivReturn64(i64Data, i64Data1);
	}
	ui32FreqLifLow = i64Rst.ui32Low;/*64bit system */
	ui8FreqLifHigh = i64Rst.ui32High;

	/*start write register*/

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162800, 0xffffff00, (ui32FreqLifLow & 0xff));
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162804, 0xffffff00, ((ui32FreqLifLow >> 8) & 0xff));
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162808, 0xffffff00, ((ui32FreqLifLow >> 16) & 0xff));
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816280c, 0xffffff00, ((ui32FreqLifLow >> 24) & 0xff));
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162810, 0xfffffff0, (ui8FreqLifHigh & 0x0f));


	ui32ParamsIFFreq = (ui32TunerIFKHz * 32768 + 250) / 500; /*round(IF_MHz*Pow(2,16)) //Add 2014.3.24*/

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162af0, 0xffffff00, (ui32ParamsIFFreq & 0xff));
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162af4, 0xffffff00, ((ui32ParamsIFFreq >> 8) & 0xff));
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8162af8, 0xffffffc0, ((ui32ParamsIFFreq >> 16) & 0x3f));

	//reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP

	return ret;
}


/**

@see   DVBT_DEMOD_FP_SET_SPECTRUM_MODE

*/
int
realtek_r_atsc3_SetSpectrumMode(
	ATSC3_DEMOD_MODULE *pDemod,
	int SpectrumMode
)
{
	int ret = FUNCTION_SUCCESS;


	gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8IQSwapMode = (SpectrumMode == SPECTRUM_NORMAL) ? RTK_R_ATSC3_INPUT_IQ_NORMAL : RTK_R_ATSC3_INPUT_IQ_SWAP;

	if (gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8IQSwapMode) {
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160c08, 0xfffffffe, 0x1);
	} else {
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160c08, 0xfffffffe, 0x0);
	}


	//reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP

	return ret;

}





/**

@see   DVBT_DEMOD_FP_SET_BANDWIDTH_MODE

*/
int
realtek_r_atsc3_SetBandwidthMode(
	ATSC3_DEMOD_MODULE *pDemod,
	int BandwidthMode
)
{

	unsigned int DataTmp = 0;
	unsigned char ui8Tmp = 0;
	unsigned char ui8Iter = 0;
	unsigned int RateRatio = 0;
	unsigned int Atsc3BWmode = 0;
	int ret = FUNCTION_SUCCESS;


	if (BandwidthMode == RTK_R_ATSC3_BANDWIDTH_6MHZ) {
		ui8Iter = 0;
		RateRatio = 0x1BC71B72;
		Atsc3BWmode = 0;
	} else if (BandwidthMode == RTK_R_ATSC3_BANDWIDTH_7MHZ) {
		ui8Iter = 1;
		RateRatio = 0x17CF3CF4;
		Atsc3BWmode = 0x1 << 28;
	} else if (BandwidthMode == RTK_R_ATSC3_BANDWIDTH_8MHZ) {
		ui8Iter = 2;
		RateRatio = 0x14D55555;
		Atsc3BWmode = 0x2 << 28;
	} else {
		ui8Iter = 0;
		RateRatio = 0x1BC71B72;
		Atsc3BWmode = 0;
	}


	for (ui8Tmp = 0; ui8Tmp < 81; ui8Tmp++) {

		DataTmp = ((ui8Tmp & 0xff) << 12) | (Atsc3BandWidthFilter[ui8Iter][ui8Tmp] & 0xfff);
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816642c, DataTmp);

		DataTmp = (0x1 << 20) | DataTmp;
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816642c, DataTmp);

	}

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8166524, RateRatio);

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816643C, 0xcfffffff, Atsc3BWmode);

	//reset DSP
	//ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
	//ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP

	return ret;


}


/**

@see   DVBT_DEMOD_FP_IS_SIGNAL_LOCKED

*/
int
realtek_r_atsc3_IsSignalLocked(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{

	unsigned int ReadingData = 0;
	unsigned char lockcnt = 0;
	int i32Cnt = 0;
	int ret = FUNCTION_SUCCESS;

	for (i32Cnt = 0; i32Cnt < 3; i32Cnt++) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E500, &ReadingData);

		lockcnt = (((ReadingData >> 1) & 0x1) == 1) ? lockcnt + 1 : lockcnt;

		if ((i32Cnt == 1) && ((lockcnt == 0) || (lockcnt == 2)))
			break;

		tv_osal_usleep(10);
	}
	*pAnswer = (lockcnt >= 2) ? 1 : 0;
	return ret;

}


int
realtek_r_atsc3_IsSignalSyncLock(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
)
{
	unsigned int bsd_fsm = 0;
	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8166450, &ReadingData);

	bsd_fsm = ReadingData & 0x7;
	*pAnswer  = (bsd_fsm >= 6) ? 1 : 0;

	return ret;
}


int
realtek_r_atsc3_IsL1Lock(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pAnswer
)

{

	unsigned int  L1Done = 0, L1CrcErr = 0;
	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E810, &ReadingData);
	L1Done =  ReadingData & 0x3;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816EE30, &ReadingData);
	L1CrcErr = (ReadingData >> 30) & 0x3;

	*pAnswer  = ((L1Done == 0x3) && (L1CrcErr == 0)) ? 1 : 0;

	return ret;
}



int
realtek_r_atsc3_SoftwareReset(
	ATSC3_DEMOD_MODULE *pDemod
)

{

	int ret = FUNCTION_SUCCESS;

	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP

	return ret;
}


/**

@see   DVBT_DEMOD_FP_GET_SIGNAL_STRENGTH

*/
int
realtek_r_atsc3_GetSignalStrength(
	ATSC3_DEMOD_MODULE *pDemod,
	int TxRSSIdBm,
	unsigned int *pSignalStrength
)

{
	unsigned char u8CodeRate = 0, u8PlpMod = 0, u8SSI = 0;
	int i32Prel = 0, i32Prec = 0, i32Pref = 0, i32SnrPref = 0;
	RTK_R_ATSC3_PARAMS Atsc3Params;

	// 32K FFT, 64K LDPC
	static int i32A327SnrX10[6][12] = {
		/* 2/15, 3/15, 4/15, 5/15, 6/15, 7/15, 8/15, 9/15, 10/15, 11/15, 12/15, 13/15 */
		{-26, -7, 7, 19, 31, 39, 48, 56, 64, 72, 81, 92},         	/* QPSK */
		{9, 34, 51, 64, 78, 88, 99, 110, 120, 132, 142, 155},     	/* 16-QAM */
		{34, 59, 78, 96, 113, 126, 140, 152, 166, 180, 194, 209}, 	/* 64-QAM */
		{52, 79, 102, 122, 143, 158, 177, 193, 210, 228, 247, 268},	/* 256-QAM */
		{68, 98, 124, 147, 171, 190, 212, 232, 252, 274, 298, 323},	/* 1024-QAM */
		{82, 115, 144, 171, 197, 219, 245, 270, 298, 330, 363, 423},/* 4096-QAM */
	};

	i32Prel = -100;
	i32Prec = TxRSSIdBm;

	if (realtek_r_atsc3_GetSignalParams(pDemod, &Atsc3Params) != FUNCTION_SUCCESS) {
		return FUNCTION_ERROR;
	}

	u8CodeRate = (unsigned char) Atsc3Params.CodeRate[0];
	u8PlpMod = (unsigned char) Atsc3Params.Constellation[0];

	if ((u8CodeRate < 12) && (u8PlpMod < 6)) {
		i32SnrPref = i32A327SnrX10[u8PlpMod][u8CodeRate];
		i32Pref = -1003 + i32SnrPref;
		i32Prel = (i32Prec * 10 - i32Pref) / 10;
	} else {
		*pSignalStrength = 0;
		return FUNCTION_ERROR;
	}

	/*
	SSI = 0, if Prel < -15dB
	SSI = (2/3) * (Prel + 15), if -15dB <= Prel < 0dB
	SSI = 4 * Prel + 10, if 0dB <= Prel < 20 dB
	SSI = (2/3) * (Prel - 20) + 90, if 20dB <= Prel < 35 dB
	SSI = 100, if Prel >= 35 dB
	
	Prel = Prec - Pref
	*/

	if (i32Prel >= 35) {
		u8SSI = 100;
	} else if ((i32Prel >= 20) && (i32Prel < 35)) {
		u8SSI = (unsigned char)(((i32Prel - 20) * 2) / 3 + 90);
	} else if ((i32Prel >= 0) && (i32Prel < 20)) {
		u8SSI  = (unsigned char)(4 * i32Prel + 10);
	} else if ((i32Prel >= -15) && (i32Prel < 0)) {
		u8SSI = (unsigned char)(((i32Prel + 15) * 2) / 3);
	} else {
		u8SSI = 0;
	}
	
	if (i32Prec < -90) {
		u8SSI = 0;
	}
	
	*pSignalStrength = u8SSI;

	RTK_DEMOD_ATSC3_DBG("u8CodeRate=%u, u8PlpMod=%u, i32SnrPref=%d, i32Pref=%d, i32Prel=%d, i32Prec=%d, u8SSI=%u, pSignalStrength=%u", 
		u8CodeRate, u8PlpMod, i32SnrPref, i32Pref, i32Prel, i32Prec, u8SSI, *pSignalStrength);

	return FUNCTION_SUCCESS;
}


/**

@see   DVBT_DEMOD_FP_GET_LAYER_SIGNAL_QUALITY

*/
int
realtek_r_atsc3_GetLayerSignalQuality(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pSignalQuality
)

{

	unsigned char ui8CodeRate = 0, ui8PlpModu = 0;
	unsigned int ui32SqiValue = 0;
	unsigned int ui32Ddvbt2BER = 0;
	unsigned int ui32BerSqi = 1;
	unsigned int ui32SnrX10 = 0, ui32SnrRel = 0;
	unsigned int ui32BERNum = 0;
	unsigned int ui32BERDen = 0;
	RTK_R_ATSC3_64Data i64Data, i64Data1;
	TV_ATSC3_ErrRate BitErr;
	RTK_R_ATSC3_PARAMS Atsc3Params;



	static  int ui32PSnrNordigX10[6][12] = {
		/*   2/15, 3/15, 4/15, 5/15, 6/15, 7/15, 8/15, 9/15, 10/15, 11/15, 12/15, 13/15 */
		{-26, -7, 7, 19, 31, 39, 48,  56, 64, 72, 81, 92},         /* QPSK   */
		{9, 34, 51, 64, 78, 88, 99,  110, 120, 132, 142, 155},     /* 16-QAM */
		{34, 59, 78, 96, 113, 126, 140,  152, 166, 180, 194, 209}, /* 64-QAM */
		{52, 79, 102, 122, 143, 158, 177,  193, 210, 228, 247, 268},  /* 256-QAM */
		{68, 98, 124, 147, 171, 190, 212,  232, 252, 274, 298, 323}, /* 1024-QAM */
		{82, 115, 144, 171, 197, 219, 245,  270, 298, 330, 363, 423},  /* 4096-QAM */
	};

	*pSignalQuality = 0;


	if (realtek_r_atsc3_GetSignalParams(pDemod, &Atsc3Params) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	if (realtek_r_atsc3_GetSnrDb(pDemod, &ui32SnrX10) != FUNCTION_SUCCESS) /*snr已经*10*/
		return FUNCTION_ERROR;

	if (realtek_r_atsc3_GetBer(pDemod, &BitErr) != FUNCTION_SUCCESS)
		return FUNCTION_ERROR;

	ui32BERNum = BitErr.ErrNum[0];
	ui32BERDen = BitErr.ErrDen[0];


	if (ui32BERDen != 0) {
		RTK_R_ATSC3_64Mult(&i64Data, ui32BERNum, 1000000000);
		RTK_R_ATSC3_64Mult(&i64Data1, ui32BERDen, 1);
		ui32Ddvbt2BER = RTK_R_ATSC3_64Div(i64Data, i64Data1);

	} else
		ui32Ddvbt2BER = 1000000000;


	ui8CodeRate = (unsigned char) Atsc3Params.CodeRate[0];
	ui8PlpModu = (unsigned char) Atsc3Params.Constellation[0];

	if ((ui8CodeRate < 12) && (ui8PlpModu < 6)) {
		ui32SnrRel = ui32PSnrNordigX10[ui8PlpModu][ui8CodeRate];
	} else {
		*pSignalQuality = 0;
		return FUNCTION_ERROR; /*error*/
	}


	if (ui32SnrX10 > ui32SnrRel + 30) { /*SNR >3dB + C/Nrel, SQI = 100*/
		ui32SqiValue = 100;
	} else if ((ui32SnrX10  <= ui32SnrRel + 30) && (ui32SnrX10  >= ui32SnrRel - 30)) {
		/*
		BER_SQI = 0             if BER>1.0E-4
		BER_SQI = (100/15)      if 1.0E-4 <= BER <= 1.0E-7
		BER_SQI = (100/6)       if BER<1.0E-7
		*/
		//ui8Status |= RTK_A_DVBTX_GetDVBT2BER(&ui32Ddvbt2BER);


		/* 		 if( ui32Ddvbt2BER >0.000101)*/
		if (ui32Ddvbt2BER >= 101000) {
			ui32BerSqi = 0;
			ui32SqiValue = ui32BerSqi;
			*pSignalQuality = ui32SqiValue;
			return FUNCTION_SUCCESS;
		}
		/*       else if((ui32Ddvbt2BER < 0.0001)&&(ui32Ddvbt2BER >0.0000001))*/
		else if ((ui32Ddvbt2BER < 101000) && (ui32Ddvbt2BER > 100))

		{
			/* 			 ui32BerSqi = 100.0/15;*/
			ui32BerSqi = 15;
		} else {
			/* 			 ui32BerSqi = 100.0/6;*/
			ui32BerSqi = 6;
		}
		/*SQI = (CR_received - CR_nordig + 3) * BER_SQI*/
		/* 		 ui32SqiValue = ((ui32SnrX10 - ui32SnrRel + 30.0)/10) * ui32BerSqi;*/
		if (ui32BerSqi != 0) {
			ui32SqiValue = (((ui32SnrX10 + 30) - ui32SnrRel) * 10) / ui32BerSqi;
		}
	} else {
		ui32SqiValue = 0;
	}
	if ((ui32SqiValue >= 5) && (ui32SqiValue < 10)) {
		ui32SqiValue = ui32SqiValue - 2;
	}
	if (ui32SqiValue < 5) {
		ui32SqiValue = 0;
	}

	if ((ui32SqiValue < 30) && (ui32SqiValue >= 10) && (ui8CodeRate == 3)) {
		ui32SqiValue = ui32SqiValue - ((80 - ui32SqiValue) / 10);
	}

	*pSignalQuality = (unsigned char)(ui32SqiValue);

	return FUNCTION_SUCCESS;
}


/**

@see   DVBT_DEMOD_FP_GET_LAYER_BER

*/

int
realtek_r_atsc3_GetBer(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_ErrRate *BitErr
)
{

	unsigned int plpId = 0, ExtractPLp = 0;
	unsigned int BER[4] = {0};
	unsigned char PlpDumpAllFlag = 0;
	int counterTmp = 0;
	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned int  BCHLenTable[2][12] = {
		{1992,  3072,  4152, 5232, 6312, 7392, 8472, 9552, 10632, 11712, 12792, 13872}, /* Short FEC */
		{8448,  12768,  17088, 21408, 25728, 30048, 34368, 38688, 43008, 47328, 51648, 55968}  /* Normal FEC, the last two is not used */
	};
	unsigned int  BCHLen[4] = {0};
	TV_ATSC3_PLP_ID Atsc3Plp = {0};
	unsigned int i = 0, plpIdIndex = 0;
	unsigned int L1D_plp_fec_type = 0, L1D_plpA_cod = 0;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E810, &ReadingData);
	PlpDumpAllFlag = (ReadingData >> 31) & 0x1;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816EF40, &ReadingData);
	ExtractPLp = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E420, &ReadingData);
	BER[0] = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E424, &ReadingData);
	BER[1] = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E428, &ReadingData);
	BER[2] = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E42c, &ReadingData);
	BER[3] = ReadingData;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ED08, &ReadingData);
	L1D_plp_fec_type = (ReadingData >> 8) & 0xf;
	L1D_plpA_cod = ReadingData & 0xf;
	if ((L1D_plp_fec_type < 2) && (L1D_plpA_cod < 12))
		BCHLen[0] = BCHLenTable[L1D_plp_fec_type][L1D_plpA_cod];
	else
		BCHLen[0] = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ED50, &ReadingData);
	L1D_plp_fec_type = (ReadingData >> 8) & 0xf;
	L1D_plpA_cod = ReadingData & 0xf;
	if ((L1D_plp_fec_type < 2) && (L1D_plpA_cod < 12))
		BCHLen[1] = BCHLenTable[L1D_plp_fec_type][L1D_plpA_cod];
	else
		BCHLen[1] = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ED98, &ReadingData);
	L1D_plp_fec_type = (ReadingData >> 8) & 0xf;
	L1D_plpA_cod = ReadingData & 0xf;
	if ((L1D_plp_fec_type < 2) && (L1D_plpA_cod < 12))
		BCHLen[2] = BCHLenTable[L1D_plp_fec_type][L1D_plpA_cod];
	else
		BCHLen[2] = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816EDE0, &ReadingData);
	L1D_plp_fec_type = (ReadingData >> 8) & 0xf;
	L1D_plpA_cod = ReadingData & 0xf;
	if ((L1D_plp_fec_type < 2) && (L1D_plpA_cod < 12))
		BCHLen[3] = BCHLenTable[L1D_plp_fec_type][L1D_plpA_cod];
	else
		BCHLen[3] = 0;


	if (PlpDumpAllFlag == 1) {
		Atsc3Plp.PLPCnt = 0;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E828, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1) {
				Atsc3Plp.PLPId[Atsc3Plp.PLPCnt] = i;
				Atsc3Plp.PLPCnt++;
			}
			ReadingData = ReadingData >> 1;
		}

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E824, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1) {
				Atsc3Plp.PLPId[Atsc3Plp.PLPCnt] = i + 32;
				Atsc3Plp.PLPCnt++;
			}
			ReadingData = ReadingData >> 1;
		}

	}



	for (counterTmp = 0; counterTmp < 4; counterTmp++) {

		if ((ExtractPLp >> 31) & 0x1) {
			BitErr->PLPvalid[counterTmp] = counterTmp ? 0 : 1;
			plpId = 0;
		} else {
			BitErr->PLPvalid[counterTmp] = ((ExtractPLp >> ((8 * counterTmp) + 6)) & 0x1);
			plpId = (ExtractPLp >> (8 * counterTmp)) & 0x3f;
		}

		if (PlpDumpAllFlag == 0) {
			BitErr->ErrNum[counterTmp] = (BitErr->PLPvalid[counterTmp]) ? (BER[counterTmp] >> 16) & 0xffff : 0;
			BitErr->ErrDen[counterTmp] = (BitErr->PLPvalid[counterTmp]) ? (BER[counterTmp] & 0xffff) * BCHLen[counterTmp] : 0;
		} else {
			for (plpIdIndex = 0; plpIdIndex < Atsc3Plp.PLPCnt; plpIdIndex++) {
				if (plpId == Atsc3Plp.PLPId[plpIdIndex])
					break;
			}
			plpIdIndex = (plpIdIndex > 3) ? 3 : plpIdIndex;
			BitErr->ErrNum[counterTmp] = (BitErr->PLPvalid[counterTmp]) ? (BER[plpIdIndex] >> 16) & 0xffff : 0;
			BitErr->ErrDen[counterTmp] = (BitErr->PLPvalid[counterTmp]) ? (BER[plpIdIndex] & 0xffff) * BCHLen[plpIdIndex] : 0;

		}
	}

	return ret;
}


/**

@see   DVBT_DEMOD_FP_GET_LAYER_PER

*/

/****************************************************************************
Function:	RTK_R_ATSC3_GetDVBT2PostBCHBlockER
Parameters: ui32T2BlockER: return dvbt2  Post BCH block error rate by this pointer.
Return: 	Operate  Status:RTK_R_ATSC3_SUCCESSFUL is Normal and other is abnormal
Description:
          Call this function to get DVBT2 BlockErrorRate value, can be used to evaluate TS output error rate.
          the *ui32T2BlockER value is the error rate value * 1e9.so ,if you want to get the correct
          BlockER value,you should use the value/1e9.
	      Block error rate can be used to indicate demodulator's output error rate.
******************************************************************************/


int
realtek_r_atsc3_GetPer(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_ErrRate *PacketErr
)
{

	unsigned int plpId = 0, ExtractPLp = 0;
	unsigned int PER[4] = {0};
	unsigned char PlpDumpAllFlag = 0;
	int counterTmp = 0;

	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	TV_ATSC3_PLP_ID Atsc3Plp = {0};
	unsigned int i = 0, plpIdIndex = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E810, &ReadingData);
	PlpDumpAllFlag = (ReadingData >> 31) & 0x1;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816EF40, &ReadingData);
	ExtractPLp = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E438, &ReadingData);
	PER[0] = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E43c, &ReadingData);
	PER[1] = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E440, &ReadingData);
	PER[2] = ReadingData;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E444, &ReadingData);
	PER[3] = ReadingData;


	//RTK_DEMOD_ATSC3_INFO("PlpDumpAllFlag = %u, PERA=0x%x, PERB=0x%x, PERC=0x%x, PERD=0x%x \n", PlpDumpAllFlag, PER[0], PER[1], PER[2], PER[3]);


	if (PlpDumpAllFlag == 1) {
		Atsc3Plp.PLPCnt = 0;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E828, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1) {
				Atsc3Plp.PLPId[Atsc3Plp.PLPCnt] = i;
				Atsc3Plp.PLPCnt++;
			}
			ReadingData = ReadingData >> 1;
		}

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E824, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1) {
				Atsc3Plp.PLPId[Atsc3Plp.PLPCnt] = i + 32;
				Atsc3Plp.PLPCnt++;
			}
			ReadingData = ReadingData >> 1;
		}

	}



	for (counterTmp = 0; counterTmp < 4; counterTmp++) {

		if ((ExtractPLp >> 31) & 0x1) {
			PacketErr->PLPvalid[counterTmp] = counterTmp ? 0 : 1;
			plpId = 0;
		} else {
			PacketErr->PLPvalid[counterTmp] = ((ExtractPLp >> ((8 * counterTmp) + 6)) & 0x1);
			plpId = (ExtractPLp >> (8 * counterTmp)) & 0x3f;
		}

		if (PlpDumpAllFlag == 0) {
			PacketErr->ErrNum[counterTmp] = (PER[counterTmp] >> 16) & 0xffff;
			PacketErr->ErrDen[counterTmp] = (PER[counterTmp]) & 0xffff;
		} else {
			for (plpIdIndex = 0; plpIdIndex < Atsc3Plp.PLPCnt; plpIdIndex++) {

				if (plpId == Atsc3Plp.PLPId[plpIdIndex])
					break;
			}

			plpIdIndex = (plpIdIndex > 3) ? 3 : plpIdIndex;
			PacketErr->ErrNum[counterTmp] = (PacketErr->PLPvalid[counterTmp]) ? (PER[plpIdIndex] >> 16) & 0xffff : 0;
			PacketErr->ErrDen[counterTmp] = (PacketErr->PLPvalid[counterTmp]) ? (PER[plpIdIndex]) & 0xffff : 0;

		}
	}

	return ret;
}


/**

@see   DVBT_DEMOD_FP_GET_SNR_DB

*/
int
realtek_r_atsc3_GetSnrDb(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *ui32SNRValuex10
)
{

	unsigned int ui32RoSignalVar = 0, ui32RoNoiseVar = 0, u32tmp = 0;
	RTK_R_ATSC3_64Data i64Data, i64Data1;
	unsigned int ReadingData3 = 0, ReadingData2 = 0, ReadingData1 = 0;
	int ret = FUNCTION_SUCCESS;


	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160434, 0xfffffffe, 0x1); //ro_latch ON

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8164570, &ReadingData3);
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816456c, &ReadingData2);
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8164568, &ReadingData1);
	ui32RoSignalVar = ((ReadingData3 & 0xff) << 16) + ((ReadingData2 & 0xff) << 8) + (ReadingData1 & 0xff);
	ui32RoSignalVar = ui32RoSignalVar * 10;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8164588, &ReadingData3);
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8164584, &ReadingData2);
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8164580, &ReadingData1);
	ui32RoNoiseVar = ((ReadingData3 & 0xff) << 16) + ((ReadingData2 & 0xff) << 8) + (ReadingData1 & 0xff);

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160434, 0xfffffffe, 0x0); //ro_latch OFF

	if (ui32RoNoiseVar == 0 || ui32RoSignalVar == 0) {
		*ui32SNRValuex10 = 0;
		return ret;
	}
	i64Data1.ui32High = 0;
	i64Data1.ui32Low = ui32RoNoiseVar;
	RTK_R_ATSC3_64Mult(&i64Data, ui32RoSignalVar, 4);
	u32tmp = RTK_R_ATSC3_Log10ConvertdBx10(RTK_R_ATSC3_64Div(i64Data, i64Data1));
	if (u32tmp > 100) {
		u32tmp =  u32tmp - 100;
	} else {
		u32tmp = 0;
	}
	*ui32SNRValuex10 = u32tmp;


	return ret;
}



/**

@see   DVBT_DEMOD_FP_GET_RF_AGC

*/
int
realtek_r_atsc3_SetAgcPol(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned char agcpola
)
{
// agc gain, 0: postive, 1:negative
	int ret = FUNCTION_SUCCESS;

	gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8TxIFAgcPolarity =  agcpola ? RTK_R_ATSC3_IF_AGC_POLARITY_INVERTED : RTK_R_ATSC3_IF_AGC_POLARITY_NORMAL;


	if (gstRTK_R_Atsc3Config.Atsc3TunerInput.ui8TxIFAgcPolarity == RTK_R_ATSC3_IF_AGC_POLARITY_INVERTED) {

		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160080, 0xfffffffd, 0x2);
	} else {

		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160080, 0xfffffffd, 0x0);
	}

	return ret;

}





/**

@see   DVBT_DEMOD_FP_GET_IF_AGC

*/
int
realtek_r_atsc3_GetIfAgc(
	ATSC3_DEMOD_MODULE *pDemod,
	unsigned int *pIfAgc
)
{
	unsigned char u8AGCPolarity;
	unsigned int u32AGCValue = 0;
	unsigned int ReadingData2 = 0, ReadingData1 = 0;
	int ret = FUNCTION_SUCCESS;

	*pIfAgc = 0;

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160434, 0xfffffffe, 0x1); //ro_latch ON
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8160080, &ReadingData1);
	u8AGCPolarity = (unsigned char)((ReadingData1 >> 1) & 0x1);

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb81640a4, &ReadingData2);
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb81640a0, &ReadingData1);
	u32AGCValue = ((ReadingData2 & 0x3) << 8) | (ReadingData1 & 0xff);
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8160434, 0xfffffffe, 0x0); //ro_latch OFF

	if (u8AGCPolarity == 0x1) { //AGV inverted
		*pIfAgc = ((1024 - u32AGCValue) * 100) / 1024;
	} else {
		*pIfAgc  = (u32AGCValue * 100) / 1024;
	}

	return ret;

}


/**

@see   DVBT_DEMOD_FP_GET_TR_OFFSET_PPM

*/
int
realtek_r_atsc3_GetTrOffsetPpm(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pTrOffsetPpm
)
{

	return FUNCTION_SUCCESS;

}





/**

@see   DVBT_DEMOD_FP_GET_CR_OFFSET_HZ

*/
int
realtek_r_atsc3_GetCrOffsetHz(
	ATSC3_DEMOD_MODULE *pDemod,
	int *pCrOffsetHz
)
{

	unsigned int bsd_fcfo = 0, bsd_icfo = 0, fineCfo_fcfo = 0, fineCfo_icfo = 0;
	int bsd_fcfo_sign = 1, bsd_icfo_sign = 1, fineCfo_fcfo_sign = 1, fineCfo_icfo_sign = 1;
	unsigned int BsdPreambleFFTSize = 0;
	unsigned int DataTmp = 0;
	unsigned int BBRate_modified = 18;
	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8166450, &ReadingData);
	bsd_fcfo = (ReadingData >> 16) & 0x3fff;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8166454, &ReadingData);
	bsd_icfo = ReadingData & 0xfff;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb8166458, &DataTmp);
	fineCfo_fcfo = DataTmp & 0x3fff;
	fineCfo_icfo = (DataTmp >> 16) & 0xf;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816645c, &ReadingData);
	BsdPreambleFFTSize = (ReadingData & 0xffff);

	if (BsdPreambleFFTSize == 0) {
		RTK_DEMOD_ATSC3_WARNING("Error - BsdPreambleFFTSize = 0  !!!\n");
		*pCrOffsetHz = 0;
		return FUNCTION_ERROR;
	}

	if (bsd_fcfo & 0x2000) {
		bsd_fcfo = bsd_fcfo - 16384; /*bsd_fcfo - 2^14*/
		bsd_fcfo_sign = -1;
		bsd_fcfo = 0 - bsd_fcfo;/*first convert it to Positive data*/
	}

	if (bsd_icfo & 0x800) {
		bsd_icfo = bsd_icfo - 4096; /*bsd_icfo - 2^12*/
		bsd_icfo_sign = -1;
		bsd_icfo = 0 - bsd_icfo;/*first convert it to Positive data*/
	}

	if (fineCfo_fcfo & 0x2000) {
		fineCfo_fcfo = fineCfo_fcfo - 16384; /*fineCfo_fcfo - 2^14*/
		fineCfo_fcfo_sign = -1;
		fineCfo_fcfo = 0 - fineCfo_fcfo;/*first convert it to Positive data*/
	}

	if (fineCfo_icfo & 0x8) {
		fineCfo_icfo = fineCfo_icfo - 16; /*fineCfo_icfo - 2^4*/
		fineCfo_icfo_sign = -1;
		fineCfo_icfo = 0 - fineCfo_icfo;/*first convert it to Positive data*/
	}


	if (u8sATSC3Bandwidth == RTK_R_ATSC3_BANDWIDTH_6MHZ) { /*6MHz*/
		BBRate_modified = 18;
	} else if (u8sATSC3Bandwidth == RTK_R_ATSC3_BANDWIDTH_7MHZ) { /*7MHz*/
		BBRate_modified = 21;
	} else if (u8sATSC3Bandwidth == RTK_R_ATSC3_BANDWIDTH_8MHZ) { /*8MHz*/
		BBRate_modified = 24;
	}


	/*
	bsd_fcfo:         s(14,11)
	bsd_icfo:          s(12,1)
	bsd_cfo (Hz) = (bsd_icfo*1024 + bsd_fcfo / 2/ pi) / 2048 * 6.144e6 / 2048

	fine_cfo_fcfo:  s(14,11)
	fine_cfo_icfo:  s(4,0)
	fine_cfo (Hz) = (fine_cfo_icfo * 2048 + fine_cfo_fcfo  / 2 / pi) / 2048 * bb_rate / preamble_FFT_size
	*/


	bsd_icfo = bsd_icfo * 1500;
	bsd_fcfo = (bsd_fcfo * 9375)  / (314 * 128);


	fineCfo_icfo = (fineCfo_icfo * BBRate_modified * 48)  / (BsdPreambleFFTSize / 8000);
	fineCfo_fcfo = (fineCfo_fcfo *  BBRate_modified * 1875) / 314 / (BsdPreambleFFTSize / 5);


	*pCrOffsetHz = ((int) bsd_icfo * bsd_icfo_sign) + ((int) bsd_fcfo * bsd_fcfo_sign) + ((int) fineCfo_icfo * fineCfo_icfo_sign) + ((int) fineCfo_fcfo * fineCfo_fcfo_sign);
	return ret;


}


int
realtek_r_atsc3_SetPlpID(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_PLP_ID Atsc3Plp
)
{

	unsigned int DataTmp = 0, ReadingData = 0;
	int i = 0;
	int ret = FUNCTION_SUCCESS;
	int CounterTmp = 0, CounterTmp2 = 0;
	unsigned char u8dataTmp = 0;
	unsigned char Atsc3PlpSort[4];


	if ((Atsc3Plp.PLPCnt > 4) || (Atsc3Plp.PLPCnt == 0)) {
		DataTmp = 0x80000000;   //set plp auto mode
		//RTK_DEMOD_ATSC3_INFO("realtek_r_atsc3_SetPlpID PLPCnt =0 or >4 ! SET to PLP auto mode!\n");
	} else {


		for (CounterTmp = 0; CounterTmp < Atsc3Plp.PLPCnt; CounterTmp++)
			Atsc3PlpSort[CounterTmp] = Atsc3Plp.PLPId[CounterTmp] & 0x3f;


		for (CounterTmp = 0; CounterTmp < Atsc3Plp.PLPCnt; CounterTmp++)
			for (CounterTmp2 = 0; CounterTmp2 < Atsc3Plp.PLPCnt - CounterTmp - 1; CounterTmp2++)
				if (Atsc3PlpSort[CounterTmp2] > Atsc3PlpSort[CounterTmp2 + 1]) {
					u8dataTmp = Atsc3PlpSort[CounterTmp2];
					Atsc3PlpSort[CounterTmp2] = Atsc3PlpSort[CounterTmp2 + 1];
					Atsc3PlpSort[CounterTmp2 + 1] = u8dataTmp;
				}


		for (i = 0; i < Atsc3Plp.PLPCnt; i++) {
			DataTmp =  DataTmp | (Atsc3PlpSort[i] << (8 * i)  | (0x1 << (6 + (8 * i))));
		}

	}


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);


	if (ReadingData != DataTmp) {
		RTK_DEMOD_ATSC3_INFO("[SetPlpID] 0xb816ef40 = 0x%x\n", DataTmp);
		//ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816EF3C, 0xFFFFFFFE, 0x1); //hold CE data output
		//ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8160018, 0xFFFFFFFE, 0x1); //RESET plp BLOCK
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816ef40, DataTmp);  //change plp
		//ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8160018, 0xFFFFFFFE, 0x0); //RESET plp BLOCK release
		//ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816EF3C, 0xFFFFFFFE, 0x0); //hold CE data output release


		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP
	}
	return ret;
}



int
realtek_r_atsc3_GetPlpID(
	ATSC3_DEMOD_MODULE *pDemod,
	TV_ATSC3_PLP_ID* Atsc3Plp,
	ATSC3_MPLP_ID_MODE_SEL Atsc3MPlpMode
)
{

	unsigned int DataTmp = 0;
	int i = 0, i32Cnt2 = 0;
	int ret = FUNCTION_SUCCESS;
	TV_ATSC3_PLP_ID Atsc3PlpDataTmp = {0};

#ifdef ATSC3GETPLPWMOREINFO
	unsigned int ReadingData = 0;
	int i32CntTmp = 0;
	unsigned char plpSelectedCnt = 0;
	unsigned char selectedPlpID[4] = {0};

	int j = 0;
	unsigned char plpLayerInfo = 0, plpTiMode = 0;
	unsigned char PlpDumpAllFlag = 0;
#endif

	Atsc3Plp->PLPCnt = 0;

	if (Atsc3MPlpMode == NONE_PLP_ID)
		return FUNCTION_SUCCESS;

	if (Atsc3MPlpMode == FULL_PLP_ID)
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E828, &DataTmp);
	else if (Atsc3MPlpMode == LLS_ONLY_PLP_ID)
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E830, &DataTmp);
	else
		return FUNCTION_ERROR;

	for (i = 0; i < 32; i++) {
		if (DataTmp & 0x1) {
			Atsc3Plp->PLPId[Atsc3Plp->PLPCnt] = i;
			Atsc3Plp->PLPCnt++;
		}
		DataTmp = DataTmp >> 1;
	}


	if (Atsc3MPlpMode == FULL_PLP_ID)
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E824, &DataTmp);
	else if (Atsc3MPlpMode == LLS_ONLY_PLP_ID)
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E82C, &DataTmp);
	else
		return FUNCTION_ERROR;

	for (i = 0; i < 32; i++) {
		if (DataTmp & 0x1) {
			Atsc3Plp->PLPId[Atsc3Plp->PLPCnt] = i + 32;
			Atsc3Plp->PLPCnt++;
		}
		DataTmp = DataTmp >> 1;
	}

	realtek_r_atsc3_L1DataDump(pDemod, &Atsc3PlpDataTmp);

	for (i = 0; i < Atsc3Plp->PLPCnt; i++) {
		for (i32Cnt2 = 0; i32Cnt2 < Atsc3PlpDataTmp.PLPCnt; i32Cnt2++) {
			if (Atsc3Plp->PLPId[i] == (Atsc3PlpDataTmp.PLPId[i32Cnt2] & 0x3f)) {
				Atsc3Plp->PLPId[i] = Atsc3PlpDataTmp.PLPId[i32Cnt2];
				break;
			}
		}
	}

//parsing TI and layer Info for Selected PLP
#ifdef ATSC3GETPLPWMOREINFO


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E810, &ReadingData);
	PlpDumpAllFlag = (ReadingData >> 31) & 0x1;

	if (PlpDumpAllFlag == 1) {
		plpSelectedCnt = Atsc3Plp->PLPCnt;
		selectedPlpID[0] = 0;
		selectedPlpID[1] = 1;
		selectedPlpID[2] = 2;
		selectedPlpID[3] = 3;

	} else {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);

		for (i32CntTmp = 0; i32CntTmp < 4; i32CntTmp++) {
			if (((ReadingData >> (6 + (i32CntTmp * 8))) & 0x1)  == 0)
				break;

			selectedPlpID[i32CntTmp] = (ReadingData >> (i32CntTmp * 8)) & 0x3f;
		}

		plpSelectedCnt = i32CntTmp;

	}


	if (plpSelectedCnt  >= 1) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &DataTmp);
		plpLayerInfo = (DataTmp >> 16) & 0x3;
		plpLayerInfo = (plpLayerInfo > 0) ? 1 : 0;

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED18, &DataTmp);
		plpTiMode = (DataTmp >> 16) & 0x3;
		plpTiMode = (plpTiMode == 1) ? 1 : 0;

		selectedPlpID[0] = (plpLayerInfo << 7) | (plpTiMode << 6) | (selectedPlpID[0] & 0x3f);

	}

	if (plpSelectedCnt  >= 2) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &DataTmp);
		plpLayerInfo = (DataTmp >> 16) & 0x3;
		plpLayerInfo = (plpLayerInfo > 0) ? 1 : 0;

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED60, &DataTmp);
		plpTiMode = (DataTmp >> 16) & 0x3;
		plpTiMode = (plpTiMode == 1) ? 1 : 0;

		selectedPlpID[1] = (plpLayerInfo << 7) | (plpTiMode << 6) | (selectedPlpID[1] & 0x3f);

	}

	if (plpSelectedCnt  >= 3) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED98, &DataTmp);
		plpLayerInfo = (DataTmp >> 16) & 0x3;
		plpLayerInfo = (plpLayerInfo > 0) ? 1 : 0;

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816EDA8, &DataTmp);
		plpTiMode = (DataTmp >> 16) & 0x3;
		plpTiMode = (plpTiMode == 1) ? 1 : 0;

		selectedPlpID[2] = (plpLayerInfo << 7) | (plpTiMode << 6) | (selectedPlpID[2] & 0x3f);

	}

	if (plpSelectedCnt  >= 4) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816EDE0, &DataTmp);
		plpLayerInfo = (DataTmp >> 16) & 0x3;
		plpLayerInfo = (plpLayerInfo > 0) ? 1 : 0;

		ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDF0, &DataTmp);
		plpTiMode = (DataTmp >> 16) & 0x3;
		plpTiMode = (plpTiMode == 1) ? 1 : 0;

		selectedPlpID[3] = (plpLayerInfo << 7) | (plpTiMode << 6) | (selectedPlpID[3] & 0x3f);

	}


	for (j = 0; j < plpSelectedCnt; j++) {
		for (i = 0; i < Atsc3Plp->PLPCnt; i++) {
			if (Atsc3Plp->PLPId[i] == (selectedPlpID[j] & 0x3f)) {
				Atsc3Plp->PLPId[i] = selectedPlpID[j];
				break;
			}
		}
	}



#endif

	return ret;
}


/********************************************************************************
function    :RTK_R_ATSC3_GetSignalParams
parameter   :pDvbTParams:pointer to DVB-T2 signal parameters.
return      :Operate  Status:RTK_R_ATSC3_SUCCESSFUL is Normal and other is abnormal
Description :Call this function to Get DVB-T2 signal parameters.
*********************************************************************************/

int
realtek_r_atsc3_GetSignalParams(
	ATSC3_DEMOD_MODULE *pDemod,
	RTK_R_ATSC3_PARAMS *pAtsc3Params
)
{
	unsigned int PLPCnt = 0, plpId = 0;
	unsigned int DataTmp = 0;
	unsigned char PlpDumpAllFlag = 0;
	int i = 0;
	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E810, &ReadingData);
	PlpDumpAllFlag = (ReadingData >> 31) & 0x1;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0x18166454, &ReadingData);
	pAtsc3Params ->bSpectrumInv = (ReadingData >> 15) & 0x1;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816EF40, &DataTmp);

	if (DataTmp == 0x80000000) {   //PLP auto mode
		pAtsc3Params ->PLPSelectedCnt = 1;
		pAtsc3Params ->PLP[0] = 0;

	} else { //PLP manual mode

		PLPCnt = 0;

		for (i = 0; i < 4; i++) {
			if ((DataTmp >> ((8 * i) + 6)) & 0x1) {
				PLPCnt++;
				pAtsc3Params ->PLP[i] = (DataTmp >> (8 * i)) & 0x3f;
			} else {
				break;
			}
		}
		pAtsc3Params ->PLPSelectedCnt = PLPCnt;
	}

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E904, &DataTmp);
	for (i = 0; i < pAtsc3Params ->PLPSelectedCnt; i++) {
		pAtsc3Params ->FFTMode[i] = (ATSC3_FFT_MODE_TYPE)((DataTmp >> 8) & 0x3);
		pAtsc3Params ->GuardInterval[i] = (ATSC3_GUARD_INTERVAL_TYPE)(DataTmp & 0xF);
	}

	if ((pAtsc3Params ->PLPSelectedCnt >= 1) || (PlpDumpAllFlag == 1)) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &DataTmp);
		pAtsc3Params ->CodeRate[0] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[0] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);
	}

	if ((pAtsc3Params ->PLPSelectedCnt >= 2) || (PlpDumpAllFlag == 1)) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &DataTmp);
		pAtsc3Params ->CodeRate[0] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[0] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &DataTmp);
		pAtsc3Params ->CodeRate[1] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[1] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);
	}

	if ((pAtsc3Params ->PLPSelectedCnt >= 3) || (PlpDumpAllFlag == 1)) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &DataTmp);
		pAtsc3Params ->CodeRate[0] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[0] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &DataTmp);
		pAtsc3Params ->CodeRate[1] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[1] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED98, &DataTmp);
		pAtsc3Params ->CodeRate[2] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[2] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);
	}

	if ((pAtsc3Params ->PLPSelectedCnt >= 4) || (PlpDumpAllFlag == 1)) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &DataTmp);
		pAtsc3Params ->CodeRate[0] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[0] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &DataTmp);
		pAtsc3Params ->CodeRate[1] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[1] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED98, &DataTmp);
		pAtsc3Params ->CodeRate[2] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[2] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDE0, &DataTmp);
		pAtsc3Params ->CodeRate[3] = (ATSC3_CODE_RATE_TYPE)(DataTmp & 0xF);
		pAtsc3Params ->Constellation[3] = (ATSC3_CONSTELLATION_TYPE)((DataTmp >> 4) & 0xF);
	}

	if (PlpDumpAllFlag == 1) {
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816EF40, &DataTmp);

		for (i = 0; i < pAtsc3Params ->PLPSelectedCnt; i++) {
			plpId = pAtsc3Params ->PLP[i];
			pAtsc3Params ->CodeRate[i] = pAtsc3Params ->CodeRate[plpId];
			pAtsc3Params ->Constellation[i] = pAtsc3Params ->Constellation[plpId];
		}
	}

	return ret;
}


/*Get counted TS bit rate*/
int  realtek_r_atsc3_TSBitAccu(ATSC3_DEMOD_MODULE *pDemod, unsigned int *TSThroughputBps)
{

	return FUNCTION_SUCCESS;
}


int  realtek_r_atsc3_vv526_detect_flow(ATSC3_DEMOD_MODULE *pDemod, unsigned char* flag)
{

	unsigned char L1ParsingDone = 0, L1D_subf1_num_plp = 0, Plp_dump_all_flag = 0;
	unsigned char PLPA_Layer = 0, PLPB_Layer = 0, PLPC_Layer = 0;
	unsigned char PLPA_TI = 0, plpA_CTI_depth = 0;
	unsigned int plpA_CTI_start_row = 0;
	unsigned int plp_start[3] = {0}, plp_CTI_fec_block_start[3] = {0};
	unsigned int u32DataTmp = 0, u32DataTmp1 = 0, u32CntTmp = 0;
	unsigned int Nrow = 0;
	unsigned int u32C[3] = {0}, u32C0[3] = {0}, u32C1[3] = {0};
	unsigned int plpA_offset = 0, plpB_offset = 0, plpC_offset = 0;
	unsigned char plpB_FecType = 0, plpB_Mod = 0;
	unsigned int plpB_FecType_Size = 0, plpB_Mod_Bits = 0;
	unsigned char ldm_part1_vld = 0;
	unsigned char frame2Flag = 0;
	unsigned char vv526PreDetectedFlag = 0;

	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;





	vv526PreDetectedFlag = *flag;
	*flag = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
	L1ParsingDone = ReadingData & 0x3;

	if (L1ParsingDone == 0x3) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E908, &ReadingData);
		L1D_subf1_num_plp = (ReadingData >> 16) & 0x3f;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
		Plp_dump_all_flag = (ReadingData >> 31) & 0x1;

		if ((L1D_subf1_num_plp == 0x2) && (Plp_dump_all_flag == 1)) {

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &ReadingData);
			PLPA_Layer = (ReadingData >> 16) & 0x3;
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &ReadingData);
			PLPB_Layer = (ReadingData >> 16) & 0x3;
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED98, &ReadingData);
			PLPC_Layer = (ReadingData >> 16) & 0x3;
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED18, &ReadingData);
			PLPA_TI = (ReadingData >> 16) & 0x3;
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E808, &ReadingData);
			frame2Flag = (ReadingData >> 30) & 0x1;

			if ((PLPA_Layer == 0) && (PLPB_Layer > 0) && (PLPC_Layer > 0) && (PLPA_TI == 1) && (frame2Flag == 1)) {

				RTK_DEMOD_ATSC3_INFO("VV526 signal 2nd frame has been detected!!\n");


				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED20, &u32DataTmp);
				plpA_CTI_depth = (u32DataTmp >> 16) & 0x7;
				plpA_CTI_start_row = u32DataTmp & 0x7FF;


				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED00, &ReadingData);
				plp_start[0] =  ReadingData & 0xFFFFFF;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED48, &ReadingData);
				plp_start[1] =  ReadingData & 0xFFFFFF;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED90, &ReadingData);
				plp_start[2] =  ReadingData & 0xFFFFFF;

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED1C, &ReadingData);
				plp_CTI_fec_block_start[0] = ReadingData & 0x3FFFFF;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED64, &ReadingData);
				plp_CTI_fec_block_start[1] = ReadingData & 0x3FFFFF;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816EDAC, &ReadingData);
				plp_CTI_fec_block_start[2] = ReadingData & 0x3FFFFF;

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &u32DataTmp);

				plpB_FecType = (u32DataTmp >> 8) & 0xF;
				plpB_Mod = (u32DataTmp >> 4) & 0xF;


				switch (plpA_CTI_depth) {
				case 0:
					Nrow = 512;
					break;
				case 1:
					Nrow = 724;
					break;
				case 2:
					Nrow = 827;
					break;
				case 3:
				default:
					Nrow = 1024;
					break;
				}


				switch (plpB_FecType) {
				case 0:
				case 2:
				case 4:
					plpB_FecType_Size = 16200;
					break;
				case 1:
				case 3:
				case 5:
				default:
					plpB_FecType_Size = 64800;
					break;
				}


				switch (plpB_Mod) {
				case 0:  //QPSK
					plpB_Mod_Bits = 2;
					break;
				case 1:  //16QAM
					plpB_Mod_Bits = 4;
					break;
				case 2:  //64QAM
					plpB_Mod_Bits = 8;
					break;
				case 3:  //256QAM
					plpB_Mod_Bits = 16;
					break;
				case 4:  //1024QAM
					plpB_Mod_Bits = 32;
					break;
				case 5:  //4096QAM
				default:
					plpB_Mod_Bits = 64;
					break;
				}



				for (u32CntTmp = 0; u32CntTmp < 3; u32CntTmp++) {

					u32DataTmp = plp_CTI_fec_block_start[u32CntTmp] + plp_start [u32CntTmp];
					u32C0[u32CntTmp] = u32DataTmp - ((u32DataTmp / Nrow) * Nrow);

					u32DataTmp1 = plpA_CTI_start_row + u32C0[u32CntTmp];
					u32C1[u32CntTmp] = (u32DataTmp / Nrow) - (u32DataTmp1 - ((u32DataTmp1 / Nrow) * Nrow));

					u32C[u32CntTmp] = (u32C1[u32CntTmp] * Nrow) + u32C0[u32CntTmp] - plp_start[u32CntTmp];

				}


				plpA_offset = u32C[0];
				plpB_offset = u32C[1];
				plpC_offset = u32C[2];


				while (plpB_offset < plpA_offset)
					plpB_offset = plpB_offset + (plpB_FecType_Size / plpB_Mod_Bits);


				plpB_offset = plpB_offset - plpA_offset;


				for (u32CntTmp = 0; u32CntTmp < 50; u32CntTmp++) {

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8161D00, &ReadingData);
					ldm_part1_vld = ReadingData >> 31;

					if (ldm_part1_vld)
						break;

					tv_osal_msleep(1);
				}


				if (ldm_part1_vld) {

					if (vv526PreDetectedFlag == 0) {
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161C0C, 0x8000FFFF, (plpA_offset & 0x7FFF) << 16);
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161D64, 0xC000FFFF, (plpB_offset & 0x3FFF) << 16);
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161D64, 0xFFFF8000, (plpC_offset & 0x7FFF));
						*flag = 1;
					} else if (vv526PreDetectedFlag > 0) {
						*flag = 2;
					}

				}

			}

		}

	}


	if ((*flag == 0) && (vv526PreDetectedFlag != 0)) {
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161C0C, 0x8000FFFF, 0x0);
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161D64, 0xC000FFFF, 0x0);
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161D64, 0xFFFF8000, 0x0);
	}

	return ret;
}

int  realtek_r_atsc3_kvv_fdm_check_flow(ATSC3_DEMOD_MODULE *pDemod, unsigned char* flag)
{

	//for KVV2804/2808/5504/5508
	unsigned int L1ParsingDone = 0;
	unsigned char PlpSelectNum = 0;
	unsigned int preamble_structure = 0;
	unsigned int PreambleFFTSize = 0, PreambleGILength = 0, PreambleDx = 0, CredCoeff = 4;
	unsigned int L1BPreamNumSymb = 0, L1BPreamReduceCarr = 0;
	unsigned int L1B1stSubFftSize = 0, L1B1stSubReduceCarr = 0, L1B1stSubScatPilotPatt = 0, L1B1stSubSbsFirst = 0;
	unsigned int L1BL1DetailTotCell = 0, L1DSbsf1NullCell = 0;
	unsigned int totalNcp = 0, ncb = 0, norPream = 0, norSbs = 0, nocPream = 0, nocSbs = 0, nocData = 0;
	unsigned int plpAStartSymbolNoc = 0, plpBStartSymbolNoc = 0;
	unsigned int u32DataTmp = 0;
	unsigned int u32TmpCnt = 0, u32TmpCnt2 = 0;
	unsigned int L1BPaprReduc = 0;

	unsigned int L1DPlpAType = 0, L1DPlpASubsliceInterval = 0, L1DPlpAStart = 0, L1DPlpASize = 0, L1DPlpANumSubslice = 0;

	unsigned int L1DPlpBStart = 0, L1DPlpBSize = 0, L1DPlpBNumSubslice = 0;

	unsigned int plpASubsliceSize = 0, plpASubsliceCrossSymbol = 0, plpBSubsliceSize = 0, plpBSubsliceCrossSymbol = 0;


	unsigned int eco_fdm_cross_en = 0, subslice_interval_larger_symb = 0;
	unsigned int dhti0_start = 0, dhti1_start = 0, plp_in_symb_start = 0, dhti0_insymb_size = 0, dhti1_insymb_size = 0;
	unsigned int dhti0_cross_symb = 0, dhti1_cross_symb = 0;
	unsigned int cross_symb_offset = 0, cross_symb_offset_below = 0;
	unsigned int dhti0_insymb_size_add_cross_offset = 0, dhti1_insymb_size_add_cross_offset = 0;
	unsigned int symb_offset_add_sram_a_offset = 0, symb_offset_add_sram_b_offset = 0, symb_offset_add_sram_c_offset = 0, symb_offset_add_sram_d_offset = 0;
	unsigned int symb_offset_add_sram_a_offset_minus1 = 0,  symb_offset_add_sram_b_offset_minus1 = 0,  symb_offset_add_sram_c_offset_minus1 = 0, symb_offset_add_sram_d_offset_minus1 = 0;
	unsigned int size_remain_up = 0;
	unsigned int dhti0_sram_beg_add = 0, dhti1_sram_beg_add = 0;
	unsigned int dhti0_beg_add_sram_a_offset = 0, dhti0_beg_add_sram_b_offset = 0, dhti0_beg_add_sram_c_offset = 0, dhti0_beg_add_sram_d_offset = 0;
	unsigned int dhti1_beg_add_sram_a_offset = 0, dhti1_beg_add_sram_b_offset = 0, dhti1_beg_add_sram_c_offset = 0, dhti1_beg_add_sram_d_offset = 0;
	unsigned int  ReadingData = 0;

	unsigned int L1DSubf1NumPlp = 0;
	unsigned int swExtractPlp = 0;

	int ret = FUNCTION_SUCCESS;

	*flag = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
	L1ParsingDone = ReadingData & 0x3;

	if (L1ParsingDone == 0x3) {


		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E908, &ReadingData);
		L1DSubf1NumPlp = (ReadingData >> 16) & 0x3f;

		if ((L1DSubf1NumPlp == 1) || (L1DSubf1NumPlp == 2)) { //subFrame1 have 2 or 3 PLP

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED10, &ReadingData);
			L1DPlpAType = (ReadingData >> 16) & 0x1;


			if (L1DPlpAType == 1) {
				RTK_DEMOD_ATSC3_INFO("dispered PLP has been detected!!\n");
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8166404, &ReadingData);
				preamble_structure = (ReadingData >> 16) & 0xff;  //bsd_info_graycode3
				PreambleFFTSize = Atsc3PreamleStruct[preamble_structure][0];
				PreambleGILength = Atsc3PreamleStruct[preamble_structure][1];
				PreambleDx = Atsc3PreamleStruct[preamble_structure][2];

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E814, &u32DataTmp);
				L1BPreamNumSymb = (u32DataTmp >> 28) & 0x7;
				L1BPreamNumSymb = L1BPreamNumSymb + 1;
				L1BPreamReduceCarr = (u32DataTmp >> 24) & 0x7;
				L1BPreamReduceCarr = (L1BPreamReduceCarr <= 4) ? L1BPreamReduceCarr : 4; //remove reserved

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E834, &ReadingData);
				L1BPaprReduc = (ReadingData >> 4) & 0x3;


				for (u32TmpCnt2 = 0; u32TmpCnt2 < 32; u32TmpCnt2++) {

					if ((NCP_table[u32TmpCnt2][0] == PreambleFFTSize) && (NCP_table[u32TmpCnt2][1] == PreambleGILength) && (NCP_table[u32TmpCnt2][2] == PreambleDx)) {
						for (u32TmpCnt = 0; u32TmpCnt < L1BPreamNumSymb ; u32TmpCnt++) {
							CredCoeff = u32TmpCnt ? L1BPreamReduceCarr : 4;
							totalNcp  =  totalNcp + NCP_table[u32TmpCnt2][CredCoeff + 3];
						}
						break;
					}
				}


				if ((L1BPaprReduc == 1) || (L1BPaprReduc == 3)) {

					if (PreambleFFTSize == 8)
						norPream = 72;
					else if (PreambleFFTSize == 16)
						norPream = 144;
					else
						norPream = 288;

				} else {
					norPream = 0;
				}

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E900, &ReadingData);
				L1B1stSubScatPilotPatt = (ReadingData >> 24) & 0x1f;
				L1B1stSubScatPilotPatt = L1B1stSubScatPilotPatt & 0xf; //remove reserved Pilot pattern
				L1B1stSubSbsFirst = (ReadingData >> 16) & 0x1;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E904, &u32DataTmp);
				L1B1stSubFftSize = (u32DataTmp >> 8) & 0x3;
				L1B1stSubReduceCarr = (u32DataTmp >> 4) & 0x7;

				switch (L1B1stSubFftSize) {
				case 0:  //8K
					L1B1stSubFftSize = 8;
					break;
				case 1:  //16K
					L1B1stSubFftSize = 16;
					break;
				case 2:  //32K
				default:
					L1B1stSubFftSize = 32;
					break;
				}



				for (u32TmpCnt = 0; u32TmpCnt < 15; u32TmpCnt++) {
					if ((NCB_table[u32TmpCnt][0] == L1B1stSubFftSize) && (NCB_table[u32TmpCnt][1] == L1B1stSubReduceCarr)) {
						ncb  = NCB_table[u32TmpCnt][L1B1stSubScatPilotPatt + 3];
						break;
					}
				}


				for (u32TmpCnt = 0; u32TmpCnt < 15; u32TmpCnt++) {
					if ((NCD_table[u32TmpCnt][0] == L1B1stSubFftSize) && (NCD_table[u32TmpCnt][1] == L1B1stSubReduceCarr)) {
						nocData  = NCD_table[u32TmpCnt][L1B1stSubScatPilotPatt + 3];
						break;
					}
				}


				if (nocData == 0) {
					RTK_DEMOD_ATSC3_INFO("invalid parameter (nocData = 0) !!\n");
					return FUNCTION_SUCCESS;
				}

				if ((L1BPaprReduc == 1) || (L1BPaprReduc == 3)) {

					if (L1B1stSubFftSize == 8)
						norSbs = 72;
					else if (L1B1stSubFftSize == 16)
						norSbs = 144;
					else
						norSbs = 288;

				} else {
					norSbs = 0;
				}

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E818, &ReadingData);
				L1BL1DetailTotCell = ReadingData & 0x7ffff;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E908, &ReadingData);
				L1DSbsf1NullCell = ReadingData & 0x1fff;

				nocPream =  totalNcp - (norPream * L1BPreamNumSymb) - 3820 - L1BL1DetailTotCell;
				nocSbs = ncb - norSbs - L1DSbsf1NullCell;


				ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED14, &ReadingData);
				L1DPlpASubsliceInterval = ReadingData & 0xffffff;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED00, &ReadingData);
				L1DPlpAStart = ReadingData & 0xffffff;

				if (L1DPlpAStart >= nocPream) {
					L1DPlpAStart = L1DPlpAStart - nocPream;
					if (L1B1stSubSbsFirst == 1)
						plpAStartSymbolNoc = nocSbs;
					else
						plpAStartSymbolNoc = nocData;

				} else {
					plpAStartSymbolNoc = nocPream;
				}


				if (L1B1stSubSbsFirst == 1) {
					if (L1DPlpAStart >= nocSbs) {
						L1DPlpAStart = L1DPlpAStart - nocSbs;
						plpAStartSymbolNoc = nocData;
					}
				}


				if (L1DPlpAStart >= nocData) {
					L1DPlpAStart = L1DPlpAStart - ((L1DPlpAStart / nocData) * nocData);
				}



				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED04, &ReadingData);
				L1DPlpASize = ReadingData & 0xffffff;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED10, &ReadingData);
				L1DPlpANumSubslice = (ReadingData & 0x3fff) + 1;
				plpASubsliceSize = ((L1DPlpASize / L1DPlpANumSubslice) * L1DPlpANumSubslice == L1DPlpASize) ? (L1DPlpASize / L1DPlpANumSubslice) : (L1DPlpASize / L1DPlpANumSubslice) + 1;
				plpASubsliceCrossSymbol = ((L1DPlpAStart + plpASubsliceSize) > plpAStartSymbolNoc) ? 1 : 0;


				ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);
				swExtractPlp = ReadingData;

				if ((swExtractPlp == 0x4140) || (swExtractPlp == 0x4241)) {

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED48, &ReadingData);
					L1DPlpBStart = ReadingData & 0xffffff;

					if (L1DPlpBStart >= nocPream) {
						L1DPlpBStart = L1DPlpBStart - nocPream;
						if (L1B1stSubSbsFirst == 1)
							plpBStartSymbolNoc = nocSbs;
						else
							plpBStartSymbolNoc = nocData;
					} else {
						plpBStartSymbolNoc = nocPream;
					}

					if (L1B1stSubSbsFirst == 1) {
						if (L1DPlpBStart >= nocSbs) {
							L1DPlpBStart = L1DPlpBStart - nocSbs;
							plpBStartSymbolNoc = nocData;
						}
					}

					if (L1DPlpBStart >= nocData) {
						L1DPlpBStart = L1DPlpBStart - ((L1DPlpBStart / nocData) * nocData);
					}

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED4C, &ReadingData);
					L1DPlpBSize = ReadingData & 0xffffff;
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED58, &ReadingData);
					L1DPlpBNumSubslice = (ReadingData & 0x3fff) + 1;
					plpBSubsliceSize = ((L1DPlpBSize / L1DPlpBNumSubslice) * L1DPlpBNumSubslice == L1DPlpBSize) ? (L1DPlpBSize / L1DPlpBNumSubslice) : (L1DPlpBSize / L1DPlpBNumSubslice) + 1;
					plpBSubsliceCrossSymbol = ((L1DPlpBStart + plpBSubsliceSize) > plpBStartSymbolNoc) ? 1 : 0;
				}


				eco_fdm_cross_en = 0;

				if ((swExtractPlp == 0x42) || (swExtractPlp == 0x41) || (swExtractPlp == 0x40)) {
					if (plpASubsliceCrossSymbol == 1) {
						//PLP0: KVV5501, KVV5506
						//PLP1: KVV2801, KVV2804, KVV5504, KVV2808, KVV5508
						//PLP2: LG_VV508

						eco_fdm_cross_en = 1;
						subslice_interval_larger_symb = 1;

						plp_in_symb_start = plpAStartSymbolNoc;
						dhti0_start = L1DPlpAStart;
						dhti0_insymb_size = plpASubsliceSize;
						dhti0_cross_symb =	plpASubsliceCrossSymbol;
					} else {


						if (swExtractPlp == 0x40) {
							if (L1DPlpASubsliceInterval > plpAStartSymbolNoc) {
								//PLP0: KVV2801, KVV2804, KVV5504, KVV2808, KVV5508

								eco_fdm_cross_en = 1;
								subslice_interval_larger_symb = 1;

								plp_in_symb_start = plpAStartSymbolNoc;
								dhti0_start = L1DPlpAStart;
								dhti0_insymb_size = plpASubsliceSize;
								dhti0_cross_symb =	plpASubsliceCrossSymbol;
							}
						}


					}
				} else if ((swExtractPlp == 0x4140) || (swExtractPlp == 0x4241)) {
					if (plpASubsliceCrossSymbol + plpBSubsliceCrossSymbol == 1) {
						//PLP0 & PLP1: KVV2808, KVV5506, KVV5508
						//PLP1 & PLP2: LG_VV508

						eco_fdm_cross_en = 1;
						subslice_interval_larger_symb = 1;

						plp_in_symb_start = plpAStartSymbolNoc;
						dhti0_start = L1DPlpAStart;
						dhti1_start = L1DPlpBStart;
						dhti0_insymb_size = plpASubsliceSize;
						dhti1_insymb_size = plpBSubsliceSize;
						dhti0_cross_symb =	plpASubsliceCrossSymbol;
						dhti1_cross_symb =	plpBSubsliceCrossSymbol;
					}
				}


				if (eco_fdm_cross_en == 1) {
					RTK_DEMOD_ATSC3_INFO("KVV cross symbol case has been detected !!\n");
					//RTK_DEMOD_ATSC3_INFO("nocSbs = %u ,nocPream = %u , nocData = %u, L1B1stSubSbsFirst = %u!!\n", nocSbs, nocPream, nocData, L1B1stSubSbsFirst);
					//RTK_DEMOD_ATSC3_INFO("ncb = %u ,norSbs = %u , L1DSbsf1NullCell = %u!!\n", ncb, norSbs, L1DSbsf1NullCell);

					//RTK_DEMOD_ATSC3_INFO("plp_in_symb_start = %u ,dhti_start = %u , %u, dhti_insymb_size = %u , %u, dhti_cross_symb = %u , %u !!\n", plp_in_symb_start, dhti0_start, dhti1_start, dhti0_insymb_size, dhti1_insymb_size, dhti0_cross_symb, dhti1_cross_symb);

					PlpSelectNum = 0;
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816EF40, &u32DataTmp);
					for (u32TmpCnt = 0; u32TmpCnt < 4; u32TmpCnt++) {
						if (((u32DataTmp >> (6 + (u32TmpCnt * 8))) & 0x1) == 1) {
							PlpSelectNum++;
						} else {
							break;
						}
					}


					if (dhti0_cross_symb == 1) {
						cross_symb_offset = dhti0_insymb_size - (plp_in_symb_start - L1DPlpAStart);
						dhti0_insymb_size_add_cross_offset =  dhti0_insymb_size - cross_symb_offset;
						dhti1_insymb_size_add_cross_offset = dhti0_insymb_size_add_cross_offset;

						if ((cross_symb_offset - ((cross_symb_offset / 4) * 4)) == 0)
							size_remain_up = 4;
						else
							size_remain_up = cross_symb_offset - ((cross_symb_offset / 4) * 4);

					}

					cross_symb_offset_below = L1DPlpASubsliceInterval - plp_in_symb_start;

					symb_offset_add_sram_a_offset = 0;
					symb_offset_add_sram_b_offset = 0;
					symb_offset_add_sram_c_offset = 0;
					symb_offset_add_sram_d_offset = 0;


					if (dhti0_cross_symb == 1) {
						symb_offset_add_sram_a_offset_minus1 = (dhti0_insymb_size_add_cross_offset + size_remain_up) - (((dhti0_insymb_size_add_cross_offset + size_remain_up) / 8192) * 8192);
						symb_offset_add_sram_b_offset_minus1 = (dhti0_insymb_size_add_cross_offset + size_remain_up) >> 13 ;
						symb_offset_add_sram_c_offset_minus1 = 0;
						symb_offset_add_sram_d_offset_minus1 = 0;

					} else {
						symb_offset_add_sram_a_offset_minus1 = 0;
						symb_offset_add_sram_b_offset_minus1 =  0;
						symb_offset_add_sram_c_offset_minus1 = 0;
						symb_offset_add_sram_d_offset_minus1 = 0;
					}


					dhti0_sram_beg_add = dhti0_start + cross_symb_offset_below;

					dhti0_beg_add_sram_a_offset = (dhti0_sram_beg_add / 4) + 0 - 1 + dhti0_cross_symb;
					dhti0_beg_add_sram_b_offset = (dhti0_sram_beg_add / 4) + 1712 - 1 + dhti0_cross_symb;
					dhti0_beg_add_sram_c_offset = (dhti0_sram_beg_add / 4) + 3424 - 1 + dhti0_cross_symb;
					dhti0_beg_add_sram_d_offset = (dhti0_sram_beg_add / 4) + 5136 - 1 + dhti0_cross_symb;


					if (PlpSelectNum == 2) {

						if ((dhti0_cross_symb == 0) && (dhti1_cross_symb == 1)) {
							cross_symb_offset = dhti1_insymb_size - (plp_in_symb_start - L1DPlpBStart);

							if ((cross_symb_offset - ((cross_symb_offset / 4) * 4)) == 0)
								size_remain_up = 4;
							else
								size_remain_up = cross_symb_offset - ((cross_symb_offset / 4) * 4);


							dhti0_insymb_size_add_cross_offset = dhti1_insymb_size - cross_symb_offset;
							dhti1_insymb_size_add_cross_offset = dhti0_insymb_size_add_cross_offset;
						}

						if (dhti1_cross_symb == 1) {

							symb_offset_add_sram_c_offset_minus1 = (dhti0_insymb_size_add_cross_offset + size_remain_up) - (((dhti0_insymb_size_add_cross_offset + size_remain_up) / 8192) * 8192);
							symb_offset_add_sram_d_offset_minus1 = (dhti0_insymb_size_add_cross_offset + size_remain_up) >> 13 ;

						} else {
							symb_offset_add_sram_c_offset_minus1 = 0;
							symb_offset_add_sram_d_offset_minus1 = 0;
						}

						dhti1_sram_beg_add = dhti1_start + cross_symb_offset_below;


						dhti1_beg_add_sram_a_offset = (dhti1_sram_beg_add / 4) + 0 - 1 + dhti1_cross_symb;
						dhti1_beg_add_sram_b_offset = (dhti1_sram_beg_add / 4) + 1712 - 1 + dhti1_cross_symb;
						dhti1_beg_add_sram_c_offset = (dhti1_sram_beg_add / 4) + 3424 - 1 + dhti1_cross_symb;
						dhti1_beg_add_sram_d_offset = (dhti1_sram_beg_add / 4) + 5136 - 1 + dhti1_cross_symb;

					}


					u32DataTmp = ((symb_offset_add_sram_a_offset & 0x1fff) << 16) | (symb_offset_add_sram_b_offset & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb81614F0, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("1. 0xb81614F0 = 0x%x\n", u32DataTmp);
					u32DataTmp = ((symb_offset_add_sram_c_offset & 0x1fff) << 16) | (symb_offset_add_sram_d_offset & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb81614F4, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("2. 0xb81614F4 = 0x%x\n", u32DataTmp);
					u32DataTmp = ((symb_offset_add_sram_a_offset_minus1 & 0x1fff) << 16) | (symb_offset_add_sram_b_offset_minus1 & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb81614F8, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("3. 0xb81614F8 = 0x%x\n", u32DataTmp);
					u32DataTmp = ((symb_offset_add_sram_c_offset_minus1 & 0x1fff) << 16) | (symb_offset_add_sram_d_offset_minus1 & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb81614FC, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("4. 0xb81614FC = 0x%x\n", u32DataTmp);
					u32DataTmp = ((dhti0_cross_symb & 0x1) << 4) | ((dhti1_cross_symb & 0x1) << 3) | ((subslice_interval_larger_symb & 0x1) << 2) | ((eco_fdm_cross_en & 0x1) << 1);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161C04, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("5. 0xb8161C04 = 0x%x\n", u32DataTmp);
					u32DataTmp = ((dhti0_beg_add_sram_a_offset & 0x1fff) << 16) | (dhti0_beg_add_sram_b_offset & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161C00, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("6. 0xb8161C00 = 0x%x\n", u32DataTmp);

					u32DataTmp = ((dhti0_beg_add_sram_c_offset & 0x1fff) << 16) | (dhti0_beg_add_sram_d_offset & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161C14, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("7. 0xb8161C14 = 0x%x\n", u32DataTmp);
					u32DataTmp = ((dhti1_beg_add_sram_a_offset & 0x1fff) << 16) | (dhti1_beg_add_sram_b_offset & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161C18, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("8. 0xb8161C18 = 0x%x\n", u32DataTmp);
					u32DataTmp = ((dhti1_beg_add_sram_c_offset & 0x1fff) << 16) | (dhti1_beg_add_sram_d_offset & 0x1fff);
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D5C, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("9. 0xb8161D5C = 0x%x\n", u32DataTmp);
					u32DataTmp = cross_symb_offset & 0x7fff;
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D6C, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("10. 0xb8161D6C = 0x%x\n", u32DataTmp);
					u32DataTmp = dhti0_insymb_size_add_cross_offset & 0x7fff;
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D70, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("11. 0xb8161D70 = 0x%x\n", u32DataTmp);
					u32DataTmp = dhti1_insymb_size_add_cross_offset & 0x7fff;
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D78, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("12. 0xb8161D78 = 0x%x\n", u32DataTmp);
					u32DataTmp = dhti0_sram_beg_add & 0x7fff;
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D7C, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("13. 0xb8161D7C = 0x%x\n", u32DataTmp);
					u32DataTmp = dhti1_sram_beg_add & 0x7fff;
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D80, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("14. 0xb8161D80 = 0x%x\n", u32DataTmp);
					u32DataTmp = cross_symb_offset_below & 0x7fff;
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161D84, u32DataTmp);
					//RTK_DEMOD_ATSC3_INFO("15. 0xb8161D84 = 0x%x\n", u32DataTmp);
					*flag = 1;

				}

			}

		}

	}

	return ret;
}


int  realtek_r_atsc3_multisubframe_fft8K_ce_fix(ATSC3_DEMOD_MODULE * pDemod, unsigned char* flag)
{

	unsigned int  ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned int L1BNumSubframes = 0, L1ParsingDone = 0;

	unsigned int plpA_mod = 0, plpA_fec_type = 0;
	unsigned int plpA_cod = 0;
	unsigned int preamble_structure = 0;
	unsigned int PreambleGILength = 0;
	unsigned int L1B_1stsub_GI = 0;


	*flag = 0xf;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);

	//only 1 PLP ID selected
	if (((ReadingData & 0xc0404040) == 0x40) || ((ReadingData & 0xc0404040) == 0x80000000)) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
		L1ParsingDone = ReadingData & 0x3;

		//RTK_DEMOD_ATSC3_DBG("L1ParsingDone = 0x%x\n", L1ParsingDone);

		if (L1ParsingDone == 0x3) {

			*flag = 0x0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e834, &L1BNumSubframes);

			L1BNumSubframes = (L1BNumSubframes >> 16) & 0xff;

			//RTK_DEMOD_ATSC3_DBG("L1BNumSubframes = 0x%x\n", L1BNumSubframes);

			//multi-subframe
			if (L1BNumSubframes == 0) {

				//single subframe  for  213 Field Stream
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &ReadingData);
				plpA_cod =  ReadingData & 0xf;
				plpA_mod = (ReadingData >> 4) & 0xf;
				plpA_fec_type = (ReadingData >> 8) & 0xf;


				//256QAM BCH 64K 10/15
				if ((plpA_mod == 3) && (plpA_fec_type == 1) && (plpA_cod == 8)) {

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8166404, &ReadingData);
					preamble_structure = (ReadingData >> 16) & 0xff;  //bsd_info_graycode3
					PreambleGILength = Atsc3PreamleStruct[preamble_structure][1];

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E904, &ReadingData);
					L1B_1stsub_GI =  Atsc3GITable[ReadingData & 0xf];

					if (PreambleGILength != L1B_1stsub_GI) {
						RTK_DEMOD_ATSC3_INFO("PreambleGILength != L1B_1stsub_GI !! force CE state = 0x4\n");
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816c804, 0xFFFFFFF8, 0x3);  //force_chan_state - 0x4
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816c800, 0xFFFFFFFE, 0x1);  //force_chan_state_en
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816C990, 0xFFFFFFFE, 0x0);  //params_average_cp_en
						ret |= rtd_4bytes_i2c_mask(pDemod, 0xb81634bc, 0xFFFFFFFE, 0x1);  //white
						*flag = 1;

					}

				}


			}


		}

	}


	if (*flag == 0)
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816c800, 0xFFFFFFFE, 0x0);  //realtek_r_atsc3_multisubframe_fft8K_ce_fix; disable  force_chan_state_en

	return ret;
}


int  realtek_r_atsc3_mplp_cti_detect(ATSC3_DEMOD_MODULE * pDemod, unsigned char* flag)
{

	//flag 0xf: unknow  0: No mplp cti selected  1: mplp cti selected

	unsigned int  ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned int L1ParsingDone = 0;
	unsigned char L1PlpATiMode = 0, L1PlpBTiMode = 0, L1PlpCTiMode = 0, L1PlpDTiMode = 0;
	unsigned char plpSelectedCnt = 0;
	int  i32CntTmp = 0;


	*flag = 0xf;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);

	if (((ReadingData >> 31) & 0x1) == 1) {   //PLP auto mode
		plpSelectedCnt = 1;

	} else {

		plpSelectedCnt = 0;

		for (i32CntTmp = 0; i32CntTmp < 4 ; i32CntTmp++) {
			if (((ReadingData >> (6 + (i32CntTmp * 8))) & 0x1)  == 1)
				plpSelectedCnt++;
			else
				break;

		}

	}

	if (plpSelectedCnt > 1) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
		L1ParsingDone = ReadingData & 0x3;

		//RTK_DEMOD_ATSC3_DBG("L1ParsingDone = 0x%x\n", L1ParsingDone);

		if (L1ParsingDone == 0x3) {

			*flag = 0x0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED18, &ReadingData);

			L1PlpATiMode = (ReadingData >> 16) & 0x3;

			//RTK_DEMOD_ATSC3_DBG("L1BNumSubframes = 0x%x\n", L1BNumSubframes);

			//PLPA is CTI
			if (L1PlpATiMode == 0x1) {
				*flag = 1;
			} else {

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED60, &ReadingData);
				L1PlpBTiMode = (ReadingData >> 16) & 0x3;
				if (L1PlpBTiMode == 0x1)
					*flag = 1;

				if ((*flag == 0) && (plpSelectedCnt > 2)) {
					ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDA8, &ReadingData);
					L1PlpCTiMode = (ReadingData >> 16) & 0x3;
					if (L1PlpCTiMode == 0x1)
						*flag = 1;
				}

				if ((*flag == 0) && (plpSelectedCnt > 3)) {
					ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDF0, &ReadingData);
					L1PlpDTiMode = (ReadingData >> 16) & 0x3;
					if (L1PlpDTiMode == 0x1)
						*flag = 1;
				}


			}


		}

	} else {
		*flag = 0;
	}

	return ret;


}

int  realtek_r_atsc3_config1PlpElOnly_multisubframe(ATSC3_DEMOD_MODULE * pDemod, unsigned char* flag)
{

	//flag 0xf: unknow  0: not  config1PlpElOnly  1: config1PlpElOnly case
	//for VV350 and 351

	unsigned int  ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned int L1BNumSubframes = 0, L1ParsingDone = 0;
	unsigned int L1DPlpALayer = 0;
	unsigned char only1PlpSelectedFlag = 0;


	*flag = 0xf;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);
	only1PlpSelectedFlag = (ReadingData == 0x42) ? 1 : 0;



	if (only1PlpSelectedFlag == 1) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
		L1ParsingDone = ReadingData & 0x3;

		//RTK_DEMOD_ATSC3_DBG("L1ParsingDone = 0x%x\n", L1ParsingDone);

		if (L1ParsingDone == 0x3) {

			*flag = 0x0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e834, &L1BNumSubframes);

			L1BNumSubframes = (L1BNumSubframes >> 16) & 0xff;

			//RTK_DEMOD_ATSC3_DBG("L1BNumSubframes = 0x%x\n", L1BNumSubframes);

			//multi-subframe
			if (L1BNumSubframes != 0) {
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &ReadingData);
				L1DPlpALayer = (ReadingData >> 16) & 0x3;

				if (L1DPlpALayer > 0) {
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816ef40, 0x00004241); //for realtek_r_atsc3_kvv_fdm_check_flow
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
					ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP
					*flag = 1;
					RTK_DEMOD_ATSC3_INFO("config1PlpElOnly_multisubframe case !!! switch to PLP ID 1&2 \n");

				}
			}
		}

	} else {
		*flag = 0;
	}

	return ret;


}


int  realtek_r_atsc3_2plp_hti_TiExtInter_detect(ATSC3_DEMOD_MODULE * pDemod, unsigned char* flag, TV_ATSC3_PLP_ID* pAtsc3PlpAll, TV_ATSC3_PLP_ID* pAtsc3PlpLlsOnly)
{

	//flag 0xf: unknow  0: No mplp cti selected  1: mplp cti selected

	unsigned int  ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned int demodlock = 0;
	unsigned char plpSelectedCnt = 0;
	unsigned char PlpATiMode = 0, PlpBTiMode = 0, PlpATiExtendedInt = 0, PlpBTiExtendedInt = 0;
	unsigned char PlpAMod = 0, PlpBMod = 0, PlpAFecType = 0, PlpBFecType = 0, PlpALayer = 0, PlpBLayer = 0, PlpACod = 0, PlpBCod = 0;
	unsigned char PlpAType = 0, PlpBType = 0;
	unsigned char SwPlp0SubframeId = 0, SwPlp1SubframeId = 1;
	unsigned int PlpASize = 0, PlpBSize = 0;
	unsigned int PlpANumSubslices = 0, PlpBNumSubslices = 0, PlpASubsliceInterval = 0, PlpBSubsliceInterval = 0;
	unsigned char PlpAScramblerType = 0, PlpBScramblerType = 0;
	unsigned int PlpAStart = 0, PlpBStart = 0;
	unsigned char PlpAHtiInterSubframe = 0, PlpBHtiInterSubframe = 0, PlpAHtiNumTiBlocks = 0, PlpBHtiNumTiBlocks = 0;
	unsigned int PlpAHtiFecBlocksMax = 0, PlpBHtiFecBlocksMax = 0, PlpAHtiFecBlocks = 0, PlpBHtiFecBlocks = 0;
	unsigned char PlpAHtiCellInterleaver = 0, PlpBHtiCellInterleaver = 0;
	unsigned int L1bL1DetailTotalCells = 0, L1bPaprReduction = 0, L1b1stSubScatteredPilotPattern = 0;
	unsigned char L1bNumSubframes = 0;
	unsigned int L1b1stSubNumOfdmSymbols = 0;
	unsigned char L1b1stSubReducedCarriers = 0, L1b1stSubFftSize = 0;
	unsigned int L1dSubf1SbsNullCells = 0;
	unsigned char L1b1stSubSbsLast = 0, L1b1stSubSbsFirst = 0;
	unsigned char L1bPreambleReducedCarriers = 0, L1bPreambleNumSymbols = 0, L1bL1DetailFecType = 0;
	unsigned int L1bL1DetailSizeBytes = 0;
	unsigned char L1dSubf2ScatteredPilotPattern = 0;
	unsigned int L1dSubf2NumOfdmSymbols = 0;
	unsigned char L1dSubf2ReducedCarriers = 0, L1dSubf2FftSize = 0;
	unsigned char L1dSubf2SbsNullCells = 0;
	unsigned char L1dSubf2SbsLast = 0, L1dSubf2SbsFirst = 0;
	unsigned char PreambleStructure = 0;

	int  i32CntTmp = 0;


	*flag = 0xf;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816ef40, &ReadingData);

	if (((ReadingData >> 31) & 0x1) == 1) {   //PLP auto mode
		plpSelectedCnt = 1;

	} else {
		plpSelectedCnt = 0;

		for (i32CntTmp = 0; i32CntTmp < 4 ; i32CntTmp++) {
			if (((ReadingData >> (6 + (i32CntTmp * 8))) & 0x1)  == 1)
				plpSelectedCnt++;
			else
				break;
		}
	}



	if (plpSelectedCnt == 2) {

		//ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
		//L1ParsingDone = ReadingData & 0x3;

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E500, &ReadingData);

		demodlock = (ReadingData >> 1) & 0x1;

		//RTK_DEMOD_ATSC3_INFO("L1ParsingDone = 0x%x\n", L1ParsingDone);

		if (demodlock == 0x1) {

			*flag = 0x0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED18, &ReadingData);
			PlpATiMode = (ReadingData >> 16) & 0x3;
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED60, &ReadingData);
			PlpBTiMode = (ReadingData >> 16) & 0x3;

			//PLPA&PLPB are HTI
			if ((PlpATiMode == 0x2) && (PlpBTiMode == 0x2)) {

				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED14, &ReadingData);
				PlpATiExtendedInt = (ReadingData >> 24) & 0x1;
				PlpASubsliceInterval = ReadingData & 0xffffff;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED5C, &ReadingData);
				PlpBTiExtendedInt = (ReadingData >> 24) & 0x1;
				PlpBSubsliceInterval = ReadingData & 0xffffff;


				if (PlpATiExtendedInt != PlpBTiExtendedInt) {
					*flag = 0x1;


					ret |= realtek_r_atsc3_GetPlpID(pDemod, pAtsc3PlpAll,	FULL_PLP_ID);
					ret |= realtek_r_atsc3_GetPlpID(pDemod, pAtsc3PlpLlsOnly,	LLS_ONLY_PLP_ID);

					//Read L1 info
					//PLPA
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &ReadingData);
					PlpAMod = (ReadingData >> 4) & 0xf;
					PlpAFecType = (ReadingData >> 8) & 0xf;
					PlpALayer = (ReadingData >> 16) & 0x3;
					PlpACod = ReadingData & 0xf;
					PlpAScramblerType = (ReadingData >> 12) & 0x3;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED10, &ReadingData);
					PlpAType = (ReadingData >> 16) & 0x1;
					PlpANumSubslices = ReadingData & 0x3fff;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED04, &ReadingData);
					PlpASize = ReadingData & 0xFFFFFF;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED00, &ReadingData);
					PlpAStart = ReadingData & 0xFFFFFF;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED24, &ReadingData);
					PlpAHtiInterSubframe = (ReadingData >> 20) & 0x1;
					PlpAHtiNumTiBlocks = ReadingData & 0xf;
					PlpAHtiFecBlocksMax = (ReadingData >> 4) & 0xfff;
					PlpAHtiCellInterleaver = (ReadingData >> 16) & 0x1;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED28, &ReadingData);
					PlpAHtiFecBlocks = ReadingData & 0xfff;


					//PLPB
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED50, &ReadingData);
					PlpBMod = (ReadingData >> 4) & 0xf;
					PlpBFecType = (ReadingData >> 8) & 0xf;
					PlpBLayer = (ReadingData >> 16) & 0x3;
					PlpBCod = ReadingData & 0xf;
					PlpBScramblerType = (ReadingData >> 12) & 0x3;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED58, &ReadingData);
					PlpBType = (ReadingData >> 16) & 0x1;
					PlpBNumSubslices = ReadingData & 0x3fff;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED4C, &ReadingData);
					PlpBSize = ReadingData & 0xFFFFFF;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED48, &ReadingData);
					PlpBStart = ReadingData & 0xFFFFFF;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED6C, &ReadingData);
					PlpBHtiInterSubframe = (ReadingData >> 20) & 0x1;
					PlpBHtiNumTiBlocks = ReadingData & 0xf;
					PlpBHtiFecBlocksMax = (ReadingData >> 4) & 0xfff;
					PlpBHtiCellInterleaver = (ReadingData >> 16) & 0x1;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED70, &ReadingData);
					PlpBHtiFecBlocks = ReadingData & 0xfff;


					//L1B
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E818, &ReadingData);
					L1bL1DetailTotalCells = ReadingData & 0x7ffff;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E834, &ReadingData);
					L1bNumSubframes = (ReadingData >> 16) & 0xff;
					L1bPaprReduction = (ReadingData >> 4) & 0x3;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E900, &ReadingData);
					L1b1stSubScatteredPilotPattern = (ReadingData >> 24) & 0x1f;
					L1b1stSubNumOfdmSymbols = ReadingData & 0x7ff;
					L1b1stSubSbsLast = (ReadingData >> 12) & 0x1;
					L1b1stSubSbsFirst = (ReadingData >> 16) & 0x1;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E904, &ReadingData);
					L1b1stSubReducedCarriers = (ReadingData >> 4) & 0x7;
					L1b1stSubFftSize = (ReadingData >> 8) & 0x3;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E908, &ReadingData);
					L1dSubf1SbsNullCells = ReadingData & 0x1fff;


					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E814, &ReadingData);
					L1bPreambleReducedCarriers = (ReadingData >> 24) & 0x7;
					L1bPreambleNumSymbols = (ReadingData >> 28) & 0x7;
					L1bL1DetailFecType = (ReadingData >> 20) & 0x7;
					L1bL1DetailSizeBytes = ReadingData & 0x1fff;

					//L1D
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E90C, &ReadingData);
					L1dSubf2ScatteredPilotPattern = (ReadingData >> 24) & 0x1f;
					L1dSubf2NumOfdmSymbols = ReadingData & 0x7ff;
					L1dSubf2SbsLast = (ReadingData >> 12) & 0x1;
					L1dSubf2SbsFirst = (ReadingData >> 16) & 0x1;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E910, &ReadingData);
					L1dSubf2ReducedCarriers = (ReadingData >> 4) & 0x7;
					L1dSubf2FftSize = (ReadingData >> 8) & 0x3;

					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E914, &ReadingData);
					L1dSubf2SbsNullCells = ReadingData & 0x1fff;


					ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8166404, &ReadingData);
					PreambleStructure = (ReadingData >> 16) & 0xff;

					//set TiExtendedInt to 0
					PlpATiExtendedInt = 0;
					PlpBTiExtendedInt = 0;

					//Write L1 info for SW mode
					rtd_4bytes_i2c_mask(pDemod, 0xB816EE38, 0xFFFF0000, (PlpBMod << 12) | (PlpBFecType << 8) | (PlpAMod << 4) | (PlpAFecType));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EE34, 0xFFFFE1E1, (PlpBHtiCellInterleaver << 12) | (PlpBHtiInterSubframe << 11) | (PlpBTiMode << 9) | (PlpAHtiCellInterleaver << 4) | (PlpAHtiInterSubframe << 3) | (PlpATiMode << 1));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EEC4, 0xFFFF2020, (PlpBLayer << 14) | (PlpBType << 12) | (PlpBCod << 8) | (PlpALayer << 6) | (PlpAType << 4) | (PlpACod));
					rtd_4bytes_i2c_wr(pDemod, 0xB816EEC8, (SwPlp0SubframeId << 24) | (PlpASize));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EED8, 0x1F000000, (PlpAScramblerType << 30) | (PlpATiExtendedInt << 29) | (PlpAStart));
					rtd_4bytes_i2c_wr(pDemod, 0xB816EE3C, (PlpBHtiFecBlocksMax << 20) | (PlpBHtiNumTiBlocks << 16) | (PlpAHtiFecBlocksMax << 4) | (PlpAHtiNumTiBlocks));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EE44, 0xFFFFF000, PlpAHtiFecBlocks);
					rtd_4bytes_i2c_wr(pDemod, 0xB816EECC, (SwPlp1SubframeId << 24) | (PlpBSize));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EEDC, 0x1F000000, (PlpBScramblerType << 30) | (PlpBTiExtendedInt << 29) | (PlpBStart));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EE64, 0xFFFFF000, PlpBHtiFecBlocks);

					//subframe 0
					rtd_4bytes_i2c_mask(pDemod, 0xB816EF00, 0x8000000C, (L1bL1DetailTotalCells << 12) | (L1bNumSubframes << 4) | (L1bPaprReduction));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EF04, 0xFE08008C, (L1b1stSubScatteredPilotPattern << 20) | (L1b1stSubNumOfdmSymbols << 8) | (L1b1stSubReducedCarriers << 4) | (L1b1stSubFftSize));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EF08, 0xFFE000EE, (L1dSubf1SbsNullCells << 8) | (L1b1stSubSbsLast << 4) | (L1b1stSubSbsFirst));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EF10, 0xF8E000EE, (L1bPreambleReducedCarriers << 24) | (L1dSubf2SbsNullCells << 8) | (L1dSubf2SbsLast << 4) | (L1dSubf2SbsFirst));
					rtd_4bytes_i2c_mask(pDemod, 0xB816EF14, 0x8008E000, (L1bPreambleNumSymbols << 28) | (PreambleStructure << 20) | (L1bL1DetailFecType << 16) | (L1bL1DetailSizeBytes));

					//subframe 1
					rtd_4bytes_i2c_mask(pDemod, 0xB816EF0C, 0xFE08008C, (L1dSubf2ScatteredPilotPattern << 20) | (L1dSubf2NumOfdmSymbols << 8) | (L1dSubf2ReducedCarriers << 4) | (L1dSubf2FftSize));

					//Load setting
					rtd_4bytes_i2c_mask(pDemod, 0xB816EE34, 0x7FFFFFFF, 0x80000000);
					rtd_4bytes_i2c_mask(pDemod, 0xB8161C04, 0xFFFFFFEF, 0x00000010);

					RTK_DEMOD_ATSC3_INFO(" 2plp_hti_TiExtInter case was detected!! eable L1 SW mode~~~\n");
				}
			}
		}
	} else {
		*flag = 0;
	}
	return ret;
}


int  realtek_r_atsc3_restore_default_setting(ATSC3_DEMOD_MODULE *pDemod)
{

	int ret = FUNCTION_SUCCESS;
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8161C04, 0x0); //for realtek_r_atsc3_kvv_fdm_check_flow
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161C0C, 0x8000FFFF, 0x0);  //realtek_r_atsc3_vv526_detect_flow
	ret |= rtd_4bytes_i2c_mask(pDemod, 0x18161D64, 0xC000FFFF, 0x0);  //realtek_r_atsc3_vv526_detect_flow
	ret |= rtd_4bytes_i2c_mask(pDemod, 0x18161D64, 0xFFFF8000, 0x0);  //realtek_r_atsc3_vv526_detect_flow
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816c800, 0xFFFFFFFE, 0x0);  //realtek_r_atsc3_multisubframe_fft8K_ce_fix; disable  force_chan_state_en
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb81634bc, 0xFFFFFFFE, 0x0);  //white  //for Field Stream 213
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816C990, 0xFFFFFFFE, 0x1);  //params_average_cp_en //for Field Stream 213

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0xFF01FFFF, 0x00FE0000);
	ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e80c, 0xFFFFFFFF); // plp_suben_lsb

	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161614, 0xFFFF0000, 0x7E90);  //realtek_r_atsc3_L1BD_latency_modify


	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB816EE34, 0x7FFFFFFF, 0x00000000);  //restore default vaule for realtek_r_atsc3_2plp_hti_TiExtInter_detect  (dhti_mode)
	ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161C04, 0xFFFFFFEF, 0x00000000);  //restore default vaule for realtek_r_atsc3_2plp_hti_TiExtInter_detect (Bicm_plp_table_SW_mode)
	return ret;

}

int  realtek_r_atsc3_L1CRCErr_check(ATSC3_DEMOD_MODULE *pDemod, unsigned char* status)
{

	unsigned int  L1BPerNum = 0, L1BPerDen = 0, L1DPerNum = 0, L1DPerDen = 0;
	unsigned int l1bParsingErr = 0, l1bCrcErr = 0, l1dParsingErr = 0, l1dCrcErr = 0;
	unsigned int PlpAFecTyp = 0;
	//unsigned int dataErrCnt = 0;
	int i32CntTmp = 0;
	//unsigned int  Atsc3CedagcScale = 0;
	unsigned int ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	TV_ATSC3_ErrRate PacketErr;


	//*status: 0x1 L1DError, 0x2 L1B error, 0x4 Data Error, 0x8 DAGC error

	*status = 0x0;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816EE30, &ReadingData);

	l1bCrcErr = (ReadingData >> 31) & 0x1;
	l1bParsingErr = (ReadingData >> 25) & 0x1;
	l1dCrcErr = (ReadingData >> 30) & 0x1;
	l1dParsingErr = (ReadingData >> 24) & 0x1;

	if ((l1bParsingErr == 1) || (l1bCrcErr == 1) || (l1dParsingErr == 1) || (l1dCrcErr == 1)) {
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816EE30, 0x3CFFFFFF, 0xC3000000);
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816EE30, 0x3CFFFFFF, 0x00000000);
		RTK_DEMOD_ATSC3_INFO(" l1bParsingErr=%u,  l1dParsingErr=%u, l1bCrcErr=%u, l1dCrcErr=%u  !!!\n", l1bParsingErr, l1dParsingErr, l1bCrcErr, l1dCrcErr);
	}

	if ((l1bParsingErr == 1) || (l1bCrcErr == 1)) {
		*status |= 0x2;
	}

	if ((l1dParsingErr == 1) || (l1dCrcErr == 1)) {
		*status |= 0x1;
	}



	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E430, &ReadingData);
	//RTK_DEMOD_ATSC3_INFO("0xb816E430=0x%x \n", ReadingData);
	L1BPerNum = (ReadingData >> 16) & 0xffff;
	L1BPerDen = ReadingData & 0xffff;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E434, &ReadingData);
	//RTK_DEMOD_ATSC3_INFO("0xb816E434=0x%x \n", ReadingData);
	L1DPerNum = (ReadingData >> 16) & 0xffff;
	L1DPerDen = ReadingData & 0xffff;

	if ((L1BPerDen != 0) && (L1DPerDen != 0)) {

		if (L1BPerNum * 2 > L1BPerDen)
			*status |= 0x2;

		if (L1DPerNum * 2 > L1DPerDen)
			*status |= 0x1;

	}


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &ReadingData);
	PlpAFecTyp  = (ReadingData >> 8) & 0xf;


	realtek_r_atsc3_GetPer(pDemod, &PacketErr);


	for (i32CntTmp = 0; i32CntTmp < 4; i32CntTmp++) {

		if (PacketErr.PLPvalid[i32CntTmp] == 0)
			break;

		if (PacketErr.ErrNum[i32CntTmp] * 3 > PacketErr.ErrDen[i32CntTmp]) {
			*status |= 0x4;
		} else  {
			*status &= 0xFB;
			break;
		}

	}


	/*
		if ((PlpAFecTyp & 0x4) == 0x4)  { //LDPC only

			for (i32CntTmp = 0; i32CntTmp < 5; i32CntTmp++) {
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8165840, &ReadingData);
				ReadingData  = ReadingData & 0x7f;
				dataErrCnt = (ReadingData >= 48) ? dataErrCnt + 1 : dataErrCnt;
				tv_osal_msleep(1);
			}

			if (dataErrCnt >= 5)
				*status |= 0x4;

		}

	*/

	/*
		//DAGC check
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8166530, &ReadingData);
		//RTK_DEMOD_ATSC3_INFO("0xB8166530=0x%x \n", ReadingData);
		Atsc3CedagcScale  = (ReadingData >> 4) & 0xffff;

		if (Atsc3CedagcScale  ==  0x8000)  //max value of dagc
			*status |= 0x8;
	*/

	return ret;
}


int  realtek_r_atsc3_SetLdpcIterNum(ATSC3_DEMOD_MODULE * pDemod, unsigned char mode, unsigned char num)
{

//mode 0:1X(orignal)  1: 1.5X

	unsigned int DataTmp = 0;
	int ret = FUNCTION_SUCCESS;


	if ((pDemod->m_ldpcIterNum != num) || (pDemod -> m_ldpcIterMode != mode))
		RTK_DEMOD_ATSC3_INFO("[LdpcParam]Set ldpcIterMode = %u, ldpcIterNum = %u \n", mode, num);

	if (pDemod->m_ldpcIterNum != num) {

		DataTmp = (unsigned int) num & 0xff;

		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, DataTmp);                  // ldpc_cdg_dat: max iteration: 0x32 -> 0x3f
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);              // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000000);              // LONG 2/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);              // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000001);              // LONG 3/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000002);             // LONG 4/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000003);              // LONG 5/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000004);              // LONG 6/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000005);              // LONG 7/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000006);              // LONG 8/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000007);              // LONG 9/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000008);              // LONG 10/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000009);              // LONG 11/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000000A);             // LONG 12/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000000B);             // LONG 13/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000000C);             // SHORT 2/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000000D);             // SHORT 3/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000000E);              // SHORT 4/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x0000000F);              // SHORT 5/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000010);              // SHORT 6/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000011);              // SHORT 7/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000012);              // SHORT 8/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000013);              // SHORT 9/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000014);              // SHORT 10/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000015);              // SHORT 11/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000016);              // SHORT 12/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000);             // ldpc_cdg_en = 0
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000017);              // SHORT 13/15
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001);             // ldpc_cdg_en = 1
		if (ret == FUNCTION_SUCCESS)
			pDemod->m_ldpcIterNum = num;
	}

	if (pDemod -> m_ldpcIterMode != mode) {
		DataTmp = (unsigned int) mode & 0x1;
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb81659a4, DataTmp);             // opt_ldpc_max_iter
		if (ret == FUNCTION_SUCCESS)
			pDemod->m_ldpcIterMode = mode;
	}

	return ret;
}


int  realtek_r_atsc3_SetLdpcLatency(ATSC3_DEMOD_MODULE * pDemod, unsigned char ldpcLatencyCtrlEn, unsigned char ldpcLatencyVal)
{

	unsigned int u32DataTmp = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned char PreLdpcLatencyCtrlEn = 0, PreLdpcLatencyVal = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8165998, &u32DataTmp);
	PreLdpcLatencyCtrlEn = u32DataTmp & 0x1;
	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816599c, &u32DataTmp);
	PreLdpcLatencyVal = u32DataTmp & 0xff;

	if ((PreLdpcLatencyCtrlEn != ldpcLatencyCtrlEn) || (PreLdpcLatencyVal != ldpcLatencyVal)) {
		u32DataTmp = (unsigned int) ldpcLatencyCtrlEn & 0x1;
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb8165998, 0xFFFFFFFE, u32DataTmp);
		u32DataTmp = (unsigned int) ldpcLatencyVal & 0xff;
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xb816599c, 0xFFFFFF00, u32DataTmp);
		RTK_DEMOD_ATSC3_INFO("[LdpcParam]Set ldpcLatencyCtrlEn = %u, ldpcLatencyVal = %u \n", ldpcLatencyCtrlEn, ldpcLatencyVal);
	}

	return ret;
}


int  realtek_r_atsc3_LdpcParamCheck(ATSC3_DEMOD_MODULE * pDemod)
{

	unsigned int u32DataTmp = 0;
	unsigned char L1ParsingDone = 0, plpCnt = 0;
	unsigned char ldpcItrMode = 0,  ldpcIterNum = 50, ldpcLatencyCtrlEn = 0, ldpcLatencyVal = 0;
	//unsigned char PreLdpcLatencyCtrlEn = 0, PreLdpcLatencyVal = 0;
	unsigned char plpA_FecType = 0, plpA_cod = 0;
	int i = 0;

	int ret = FUNCTION_SUCCESS;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &u32DataTmp);
	L1ParsingDone = u32DataTmp & 0x3;

	if (L1ParsingDone == 0x3) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E828, &u32DataTmp);

		for (i = 0; i < 32; i++) {
			if (u32DataTmp & 0x1)
				plpCnt++;
			u32DataTmp = u32DataTmp >> 1;
		}

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E824, &u32DataTmp);

		for (i = 0; i < 32; i++) {
			if (u32DataTmp & 0x1)
				plpCnt++;
			u32DataTmp = u32DataTmp >> 1;
		}

		if (plpCnt == 1) {

			ldpcLatencyCtrlEn = 0;
			ldpcLatencyVal = 0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &u32DataTmp);
			plpA_FecType = (u32DataTmp >> 8) & 0xf;
			plpA_cod = u32DataTmp & 0xf;

			if ((plpA_FecType == 0) || (plpA_FecType == 2) || (plpA_FecType == 4)) { //short mode

				switch (plpA_cod) {
				case 3:   //5_15
					ldpcLatencyCtrlEn = 1;
					ldpcLatencyVal = 16;
					break;
				case 4:  //6_15
					ldpcLatencyCtrlEn = 1;
					ldpcLatencyVal = 12;
					break;
				case 6:  //8_15
					ldpcLatencyCtrlEn = 1;
					ldpcLatencyVal = 14;
					break;
				case 8:   //10_15
					ldpcLatencyCtrlEn = 1;
					ldpcLatencyVal = 14;
					break;
				case 10:  //12_15
					ldpcLatencyCtrlEn = 1;
					ldpcLatencyVal = 17;
					break;
				case 11:  //13_15
					ldpcLatencyCtrlEn = 1;
					ldpcLatencyVal = 18;
					break;
				default:
					ldpcLatencyCtrlEn = 0;
					ldpcLatencyVal = 0;
					break;
				}

			}


			if (ldpcLatencyCtrlEn == 0) {
				ldpcItrMode = 1;
				ldpcIterNum = 50;
			} else {
				ldpcItrMode = 0;
				ldpcIterNum = 50;
			}

		} else {
			ldpcLatencyCtrlEn = 0;
			ldpcLatencyVal = 0;
			ldpcItrMode = 0;
			ldpcIterNum = 50;
		}

	} else {
		ldpcLatencyCtrlEn = 0;
		ldpcLatencyVal = 0;
		ldpcItrMode = 0;
		ldpcIterNum = 50;
	}


	//ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8165998, &u32DataTmp);
	//PreLdpcLatencyCtrlEn = u32DataTmp & 0x1;
	//ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816599c, &u32DataTmp);
	//PreLdpcLatencyVal = u32DataTmp & 0xff;


	if ((pDemod->m_ldpcIterMode != ldpcItrMode) || (pDemod->m_ldpcIterNum != ldpcIterNum))
		realtek_r_atsc3_SetLdpcIterNum(pDemod, ldpcItrMode, ldpcIterNum);


	//if ((PreLdpcLatencyCtrlEn != ldpcLatencyCtrlEn) || (PreLdpcLatencyVal != ldpcLatencyVal))
	realtek_r_atsc3_SetLdpcLatency(pDemod, ldpcLatencyCtrlEn, ldpcLatencyVal);



	return ret;
}


int  realtek_r_atsc3_L1DUpdateItemCheck(ATSC3_DEMOD_MODULE * pDemod)
{

	unsigned int ReadingData = 0;
	unsigned char demodLock = 0, plpCnt = 0;
	unsigned int PlpUpdateSubEnMsb = 0;
	int i = 0;

	int ret = FUNCTION_SUCCESS;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E500, &ReadingData);
	demodLock = (ReadingData >> 1) & 0x1;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e808, &ReadingData);
	PlpUpdateSubEnMsb = (ReadingData >> 17) & 0x7f;

	if (demodLock == 1) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E828, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1)
				plpCnt++;
			ReadingData = ReadingData >> 1;
		}

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E824, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1)
				plpCnt++;
			ReadingData = ReadingData >> 1;
		}

		if (plpCnt == 1) {
			if (PlpUpdateSubEnMsb != 0) {
				rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0xFF01FFFF, 0x00000000);
				rtd_4bytes_i2c_wr(pDemod, 0xb816e80c, 0x00020c02); // plp_suben_lsb
				RTK_DEMOD_ATSC3_INFO("Update L1D neccessary Info Only!!\n");
			}

		} else {
			if (PlpUpdateSubEnMsb == 0) {
				rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0xFF01FFFF, 0x00FE0000);
				rtd_4bytes_i2c_wr(pDemod, 0xb816e80c, 0xFFFFFFFF); // plp_suben_lsb
				RTK_DEMOD_ATSC3_INFO("Update L1D all Info !!\n");
			}
		}

	} else {
		if (PlpUpdateSubEnMsb == 0) {
			rtd_4bytes_i2c_mask(pDemod, 0xb816e808, 0xFF01FFFF, 0x00FE0000);
			rtd_4bytes_i2c_wr(pDemod, 0xb816e80c, 0xFFFFFFFF); // plp_suben_lsb
			RTK_DEMOD_ATSC3_INFO("Update L1D all Info !!\n");
		}
	}


	return ret;
}

int  realtek_r_atsc3_1PLPLdpcOnlyDetect(ATSC3_DEMOD_MODULE * pDemod)
{
	unsigned int  ReadingData = 0;
	int ret = FUNCTION_SUCCESS;
	unsigned int L1ParsingDone = 0;
	unsigned char plpCnt = 0;
	int  i = 0;
	unsigned char plpA_mod = 0, plpA_fec_type = 0, plpA_cod = 0;
	unsigned char u8DetectFlag = 0;


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &ReadingData);
	L1ParsingDone = ReadingData & 0x3;

	//RTK_DEMOD_ATSC3_DBG("L1ParsingDone = 0x%x\n", L1ParsingDone);

	if (L1ParsingDone == 0x3) {

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E828, &ReadingData);
		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1)
				plpCnt++;
			ReadingData = ReadingData >> 1;
		}

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E824, &ReadingData);

		for (i = 0; i < 32; i++) {
			if (ReadingData & 0x1)
				plpCnt++;
			ReadingData = ReadingData >> 1;
		}

		if (plpCnt == 1) {
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &ReadingData);
			plpA_cod =  ReadingData & 0xf;
			plpA_mod = (ReadingData >> 4) & 0xf;
			plpA_fec_type = (ReadingData >> 8) & 0xf;

			if ((plpA_fec_type == 0x4) || (plpA_fec_type == 0x5)) { //LDPC only
				//VV122 and VV134
				if (((plpA_mod == 0x4) || (plpA_mod == 0x5)) && ((plpA_cod == 0x4) || (plpA_cod == 0x7)))
					u8DetectFlag = 1;
			}
		}
	}
	realtek_r_atsc3_LdpcAlphaFixModeEn(pDemod, u8DetectFlag);
	return ret;
}

int  realtek_r_atsc3_LdpcAlphaFixModeEn(ATSC3_DEMOD_MODULE * pDemod, unsigned char enable)
{
	int ret = FUNCTION_SUCCESS;

	if (enable == 1) {
		if (Atsc3LdpcAlphaFixModeStatus != 1) {
			// turn on LDPC alpha fixed mode for all modes
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000027); // ldpc_cfg_addr 0x29
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x00000000); // ldpc_cfg_dat: turn on
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
			RTK_DEMOD_ATSC3_INFO("Enable LDPC alpha fixed mode !!\n");
			Atsc3LdpcAlphaFixModeStatus = 1;
		}
	} else {

		if (Atsc3LdpcAlphaFixModeStatus != 0) {
			// turn off LDPC alpha fixed mode for all modes
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000000); // ldpc_cfg_en
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165940, 0x00000027); // ldpc_cfg_addr 0x29
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165944, 0x00000001); // ldpc_cfg_dat: turn off
			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8165948, 0x00000001); // ldpc_cfg_en
			RTK_DEMOD_ATSC3_INFO("Disable LDPC alpha fixed mode !!\n");
			Atsc3LdpcAlphaFixModeStatus = 0;
		}
	}

	return ret;
}


int  realtek_r_atsc3_L1DataDump(ATSC3_DEMOD_MODULE * pDemod, TV_ATSC3_PLP_ID* Atsc3Plp)
{

	int ret = FUNCTION_SUCCESS;
	unsigned int  ReadingData = 0;
	int i32CntTmp = 0, i32CntTmp2 = 0;
	//unsigned int L1SramWrDoneAdrr = 0;
	unsigned int u32DataDumpCnt = 0;
	//unsigned int L1DataDump[256];
	unsigned int L1BTimeInfoFlag = 0, L1BNumSubframes = 0, L1B1stSubMimo = 0, L1B1stSubSbsFirst = 0, L1B1stSubSbsLast = 0;
	unsigned int BitsIdx = 0;
	unsigned int L1DNumRf = 0, L1DNumPlp = 0, L1DMimo = 0, L1DSbsFirst = 0, L1DSbsLast = 0;
	unsigned int L1DPlpId = 0, L1DPlpLayer = 0, L1DPlpFecType = 0, L1DPlpMod = 0, L1DPlpTiMode = 0, L1DPlpNumChBonded = 0, L1DPlpType = 0, L1DPlpHtiInterSubframe = 0, L1DPlpHtiNumTiBlocks = 0;
	int DataIdx = 0, DataPos = 0, DataBits = 0;
	unsigned int PLPIdx = 0;
	//unsigned int L1SramWrRealDone = 0;
	unsigned char L1ZeroDataCnt  = 0;
	unsigned int L1SramWrFrmCnt = 0;


	Atsc3Plp->PLPCnt = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e84c, &ReadingData); // wait bit[9:4] > 1
	L1SramWrFrmCnt = (ReadingData >> 4) & 0x3f;

	if (L1SramWrFrmCnt == 0) {


		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x82000000); // sram clr auto

		for (i32CntTmp = 0; i32CntTmp < 50; i32CntTmp++) {
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e84c, &ReadingData); //wait bit31 to 0
			ReadingData = (ReadingData >> 31) & 0x1;
			if (ReadingData == 0)
				break;
			tv_osal_msleep(1);
		}

		if (i32CntTmp >= 50)
			RTK_DEMOD_ATSC3_WARNING("\033[1;32;32m" "ATSC3L1DataDump TIMEOUT - sram clr !!!!! \n" "\033[m");

		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000001); // clear L1_sram_wr_real_done
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x42000000); // sram wr enable

		for (i32CntTmp = 0; i32CntTmp < 600; i32CntTmp++) {
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e84c, &ReadingData); // wait bit[9:4] > 1
			L1SramWrFrmCnt = (ReadingData >> 4) & 0x3f;
			if (L1SramWrFrmCnt >= 1)
				break;
			tv_osal_msleep(5);
		}

		RTK_DEMOD_ATSC3_INFO("L1_sram_wr_frm_cnt = %u, i32CntTmp = %d \n", L1SramWrFrmCnt, i32CntTmp);

		if (i32CntTmp >= 600)
			RTK_DEMOD_ATSC3_WARNING("\033[1;32;32m" "ATSC3L1DataDump TIMEOUT - sram wr !!!!! \n" "\033[m");


		/*
				ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x62000000); //L1_Sram_wr_stop


				for (i32CntTmp = 0; i32CntTmp < 600; i32CntTmp++) {
					ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e84c, &ReadingData); // wait L1_sram_wr_real_done bit[0] == 1
					ReadingData = ReadingData & 0x1;
					if (ReadingData == 1)
						break;
					tv_osal_msleep(5);
				}

				RTK_DEMOD_ATSC3_INFO("L1_sram_wr_real_done = 0x%x, i32CntTmp = %d \n", ReadingData, i32CntTmp);

		ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e850, &L1SramWrDoneAdrr); // confirm wr sram last addr
		RTK_DEMOD_ATSC3_INFO("L1_sram_wr_done_adr = %u \n", L1SramWrDoneAdrr);

				ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x42000000); //clear L1_Sram_wr_stop

		*/

		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // wr sram disable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x42000000); // sram wr enable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // wr sram disable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x42000000); // sram wr enable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // wr sram disable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x42000000); // sram wr enable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // wr sram disable


		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x12000000); // sram read en


		/*
				u32DataDumpCnt = L1SramWrDoneAdrr / 4;

				if ((L1SramWrDoneAdrr - ((L1SramWrDoneAdrr / 4) * 4)) > 0)
					u32DataDumpCnt++;

		*/

		u32DataDumpCnt = sizeof(L1DataDump) / sizeof(*L1DataDump);
		u32DataDumpCnt /= 4;
		//RTK_DEMOD_ATSC3_INFO("u32DataDumpCnt = %u \n", u32DataDumpCnt);


		for (i32CntTmp2 = 0; i32CntTmp2 < u32DataDumpCnt; i32CntTmp2++) {

			ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x1a000000); // sram read sub en, 一次读取4×32 byte


			for (i32CntTmp = 0; i32CntTmp < 100; i32CntTmp++) {
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e84c, &ReadingData); // confirm bit27 1->0
				ReadingData = (ReadingData >> 27) & 0x1;
				if (ReadingData == 0)
					break;
				tv_osal_msleep(1);
			}

			if (i32CntTmp >= 100)
				RTK_DEMOD_ATSC3_WARNING("\033[1;32;32m" "ATSC3L1DataDump TIMEOUT - sram sub rd !!!!! \n" "\033[m");


			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e854, &ReadingData);
			L1DataDump[i32CntTmp2 * 4] = ReadingData;
			if (ReadingData == 0)
				L1ZeroDataCnt++;
			else
				L1ZeroDataCnt = 0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e858, &ReadingData);
			L1DataDump[(i32CntTmp2 * 4) + 1] = ReadingData;
			if (ReadingData == 0)
				L1ZeroDataCnt++;
			else
				L1ZeroDataCnt = 0;

			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e85c, &ReadingData);
			L1DataDump[(i32CntTmp2 * 4) + 2] = ReadingData;
			if (ReadingData == 0)
				L1ZeroDataCnt++;
			else
				L1ZeroDataCnt = 0;


			ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816e860, &ReadingData);
			L1DataDump[(i32CntTmp2 * 4) + 3] = ReadingData;
			if (ReadingData == 0)
				L1ZeroDataCnt++;
			else
				L1ZeroDataCnt = 0;

			//RTK_DEMOD_ATSC3_INFO("L1DataDump[%d] = 0x%08x 0x%08x 0x%08x 0x%08x \n", i32CntTmp2 * 4, L1DataDump[(i32CntTmp2 * 4)], L1DataDump[(i32CntTmp2 * 4) + 1], L1DataDump[(i32CntTmp2 * 4) + 2], L1DataDump[(i32CntTmp2 * 4) + 3]);

			if (L1ZeroDataCnt > 3)
				break;

		}


		//RTK_DEMOD_ATSC3_INFO("u32DataDumpCnt = %u, i32CntTmp2 = %d, L1ZeroDataCnt = %u \n", u32DataDumpCnt, i32CntTmp2, L1ZeroDataCnt);



		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // sram read disable

		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x12000000); // sram read en
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // sram read disable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x12000000); // sram read en
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // sram read disable
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x12000000); // sram read en
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb816e84c, 0x02000000); // sram read disable

	}


	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E834, &ReadingData);
	L1BTimeInfoFlag =  ReadingData & 0x3;
	L1BNumSubframes = (ReadingData >> 16) & 0xff;
	//RTK_DEMOD_ATSC3_INFO("L1BTimeInfoFlag = %d\n", L1BTimeInfoFlag);
	//RTK_DEMOD_ATSC3_INFO("L1BNumSubframes = %d\n", L1BNumSubframes);

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E904, &ReadingData);
	L1B1stSubMimo = (ReadingData >> 20) & 0x1;
	//RTK_DEMOD_ATSC3_INFO("L1B1stSubMimo = %d\n", L1B1stSubMimo);

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E900, &ReadingData);
	L1B1stSubSbsFirst = (ReadingData >> 16) & 0x1;
	L1B1stSubSbsLast = (ReadingData >> 12) & 0x1;
	//RTK_DEMOD_ATSC3_INFO("L1B1stSubSbsFirst = %d\n", L1B1stSubSbsFirst);
	//RTK_DEMOD_ATSC3_INFO("L1B1stSubSbsLast = %d\n", L1B1stSubSbsLast);

	BitsIdx = 200 + 4;
	DataIdx = BitsIdx / 32;
	DataPos =  31 - (BitsIdx - DataIdx * 32);
	DataBits = 3;
	DataPos =  DataPos - DataBits + 1;

	if (DataPos < 0)
		ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
	else
		ReadingData = (L1DataDump[DataIdx] >> DataPos);


	L1DNumRf = ReadingData & 0x7;
	//RTK_DEMOD_ATSC3_INFO("L1DNumRf = %d @ BitsIdx = %d\n", L1DNumRf, BitsIdx);

	BitsIdx = BitsIdx + 3 + (19 * L1DNumRf);

	if (L1BTimeInfoFlag != 0) {
		BitsIdx = BitsIdx + 42;
		if (L1BTimeInfoFlag != 1) {
			BitsIdx = BitsIdx + 10;
			if (L1BTimeInfoFlag != 2) {
				BitsIdx = BitsIdx + 10;
			}
		}
	}

	for (i32CntTmp = 0; i32CntTmp <= L1BNumSubframes; i32CntTmp++) {

		if (i32CntTmp > 0) {
			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 1;
			DataPos =  DataPos - DataBits + 1;
			ReadingData = (L1DataDump[DataIdx] >> DataPos);
			L1DMimo = ReadingData & 0x1;

			//RTK_DEMOD_ATSC3_INFO("L1DMimo = %d @ BitsIdx = %d\n", L1DMimo, BitsIdx);

			BitsIdx = BitsIdx + 1 + 2 + 2 + 3 + 4 + 11 + 5 + 3;


			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 1;
			DataPos =  DataPos - DataBits + 1;
			ReadingData = (L1DataDump[DataIdx] >> DataPos);
			L1DSbsFirst = ReadingData & 0x1;
			//RTK_DEMOD_ATSC3_INFO("L1DSbsFirst = %d @ BitsIdx = %d\n", L1DSbsFirst, BitsIdx);

			BitsIdx = BitsIdx + 1;


			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 1;
			DataPos =  DataPos - DataBits + 1;
			ReadingData = (L1DataDump[DataIdx] >> DataPos);
			L1DSbsLast = ReadingData & 0x1;
			//RTK_DEMOD_ATSC3_INFO("L1DSbsLast = %d @ BitsIdx = %d\n", L1DSbsLast, BitsIdx);

			BitsIdx = BitsIdx + 1;
		}


		if (L1BNumSubframes > 0)
			BitsIdx = BitsIdx + 1;

		BitsIdx = BitsIdx + 1;

		if (((i32CntTmp == 0) && ((L1B1stSubSbsFirst == 1) || (L1B1stSubSbsLast == 1))) || ((i32CntTmp > 0) && ((L1DSbsFirst == 1) || (L1DSbsLast == 1))))
			BitsIdx = BitsIdx + 13;


		DataIdx = BitsIdx / 32;
		DataPos =  31 - (BitsIdx - DataIdx * 32);
		DataBits = 6;
		DataPos =  DataPos - DataBits + 1;

		if (DataPos < 0)
			ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
		else
			ReadingData = (L1DataDump[DataIdx] >> DataPos);

		L1DNumPlp = ReadingData & 0x3f;

		//RTK_DEMOD_ATSC3_INFO("L1DNumPlp = %d @ BitsIdx = %d\n", L1DNumPlp, BitsIdx);
		Atsc3Plp->PLPCnt += L1DNumPlp + 1;

		BitsIdx = BitsIdx + 6;

		for (i32CntTmp2 = 0; i32CntTmp2 <= L1DNumPlp; i32CntTmp2++) {

			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 6;
			DataPos =  DataPos - DataBits + 1;

			if (DataPos < 0)
				ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
			else
				ReadingData = (L1DataDump[DataIdx] >> DataPos);


			L1DPlpId = ReadingData & 0x3f;

			//RTK_DEMOD_ATSC3_INFO("L1DPlpId = %d @ BitsIdx = %d\n", L1DPlpId, BitsIdx);
			Atsc3Plp->PLPId[PLPIdx] = L1DPlpId;

			BitsIdx = BitsIdx + 6 + 1;

			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 2;
			DataPos =  DataPos - DataBits + 1;

			if (DataPos < 0)
				ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
			else
				ReadingData = (L1DataDump[DataIdx] >> DataPos);


			L1DPlpLayer = ReadingData & 0x3;
			//RTK_DEMOD_ATSC3_INFO("L1DPlpLayer = %d @ BitsIdx = %d\n", L1DPlpLayer, BitsIdx);

			if (L1DPlpLayer > 0)
				Atsc3Plp->PLPId[PLPIdx] |= 0x80;

			BitsIdx = BitsIdx + 2 + 24 + 24 + 2;


			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 4;
			DataPos =  DataPos - DataBits + 1;


			if (DataPos < 0)
				ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
			else
				ReadingData = (L1DataDump[DataIdx] >> DataPos);


			L1DPlpFecType = ReadingData & 0xf;
			//RTK_DEMOD_ATSC3_INFO("L1DPlpFecType = %d @ BitsIdx = %d\n", L1DPlpFecType, BitsIdx);

			BitsIdx = BitsIdx + 4;

			if (L1DPlpFecType <= 5) {

				DataIdx = BitsIdx / 32;
				DataPos =  31 - (BitsIdx - DataIdx * 32);
				DataBits = 4;
				DataPos =  DataPos - DataBits + 1;

				if (DataPos < 0)
					ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
				else
					ReadingData = (L1DataDump[DataIdx] >> DataPos);

				L1DPlpMod = ReadingData & 0xf;

				//RTK_DEMOD_ATSC3_INFO("L1DPlpMod = %d @ BitsIdx = %d\n", L1DPlpMod, BitsIdx);

				BitsIdx = BitsIdx + 4 + 4;
			}

			DataIdx = BitsIdx / 32;
			DataPos =  31 - (BitsIdx - DataIdx * 32);
			DataBits = 2;
			DataPos =  DataPos - DataBits + 1;

			if (DataPos < 0)
				ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
			else
				ReadingData = (L1DataDump[DataIdx] >> DataPos);


			L1DPlpTiMode = ReadingData & 0x3;

			//RTK_DEMOD_ATSC3_INFO("L1DPlpTiMode = %d @ BitsIdx = %d\n", L1DPlpTiMode, BitsIdx);

			if (L1DPlpTiMode == 1)
				Atsc3Plp->PLPId[PLPIdx] |= 0x40;

			BitsIdx = BitsIdx + 2;

			if (L1DPlpTiMode == 0)
				BitsIdx = BitsIdx + 15;
			else if (L1DPlpTiMode == 1)
				BitsIdx = BitsIdx + 22;


			if (L1DNumRf > 0) {

				DataIdx = BitsIdx / 32;
				DataPos =  31 - (BitsIdx - DataIdx * 32);
				DataBits = 3;
				DataPos =  DataPos - DataBits + 1;

				if (DataPos < 0)
					ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
				else
					ReadingData = (L1DataDump[DataIdx] >> DataPos);

				L1DPlpNumChBonded = ReadingData & 0x7;

				//RTK_DEMOD_ATSC3_INFO("L1DPlpNumChBonded = %d @ BitsIdx = %d\n", L1DPlpNumChBonded, BitsIdx);

				BitsIdx = BitsIdx + 3;

				if (L1DPlpNumChBonded > 0)
					BitsIdx = BitsIdx + 2 + 3 * (L1DPlpNumChBonded + 1);
			}

			if (((i32CntTmp == 0) && (L1B1stSubMimo == 1)) || ((i32CntTmp > 1) && (L1DMimo == 1)))
				BitsIdx = BitsIdx + 3;

			if (L1DPlpLayer == 0) {

				DataIdx = BitsIdx / 32;
				DataPos =  31 - (BitsIdx - DataIdx * 32);
				DataBits = 1;
				DataPos =  DataPos - DataBits + 1;

				ReadingData = (L1DataDump[DataIdx] >> DataPos);

				L1DPlpType = ReadingData & 0x1;

				//RTK_DEMOD_ATSC3_INFO("L1DPlpType = %d @ BitsIdx = %d\n", L1DPlpType, BitsIdx);
				BitsIdx = BitsIdx + 1;

				if (L1DPlpType == 1)
					BitsIdx = BitsIdx + 14 + 24;

				if (((L1DPlpTiMode == 1) || (L1DPlpTiMode == 2)) && (L1DPlpMod == 0))
					BitsIdx = BitsIdx + 1;

				if (L1DPlpTiMode == 1) {
					BitsIdx = BitsIdx + 3 + 11;
				} else if (L1DPlpTiMode == 2) {


					DataIdx = BitsIdx / 32;
					DataPos =  31 - (BitsIdx - DataIdx * 32);
					DataBits = 1;
					DataPos =  DataPos - DataBits + 1;
					ReadingData = (L1DataDump[DataIdx] >> DataPos);
					L1DPlpHtiInterSubframe = ReadingData & 0x1;
					//RTK_DEMOD_ATSC3_INFO("L1DPlpHtiInterSubframe = %d @ BitsIdx = %d\n", L1DPlpHtiInterSubframe, BitsIdx);
					BitsIdx = BitsIdx + 1;

					DataIdx = BitsIdx / 32;
					DataPos =  31 - (BitsIdx - DataIdx * 32);
					DataBits = 4;
					DataPos =  DataPos - DataBits + 1;
					if (DataPos < 0)
						ReadingData = (L1DataDump[DataIdx] << (0 - DataPos)) + (L1DataDump[DataIdx + 1] >> (32 + DataPos));
					else
						ReadingData = (L1DataDump[DataIdx] >> DataPos);

					L1DPlpHtiNumTiBlocks = ReadingData & 0xf;

					//RTK_DEMOD_ATSC3_INFO("L1DPlpHtiNumTiBlocks = %d @ BitsIdx = %d\n", L1DPlpHtiNumTiBlocks, BitsIdx);
					BitsIdx = BitsIdx + 4;

					BitsIdx = BitsIdx + 12;

					if (L1DPlpHtiInterSubframe == 0)
						BitsIdx = BitsIdx + 12;
					else
						BitsIdx = BitsIdx + 12 * (L1DPlpHtiNumTiBlocks + 1);

					BitsIdx = BitsIdx + 1;

				}

			} else {
				BitsIdx = BitsIdx + 5;
			}

			PLPIdx++;

		}

	}


	return ret;

}


int  realtek_r_atsc3_GetChipId(ATSC3_DEMOD_MODULE * pDemod, unsigned char* pChipId)
{
	int ret = FUNCTION_SUCCESS;
	unsigned int ReadingData = 0;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB80004BC, &ReadingData);
	*pChipId = (ReadingData >> 28) & 0xF;
	return ret;

}

int  realtek_r_atsc3_L1BD_latency_modify(ATSC3_DEMOD_MODULE * pDemod, unsigned char* flag)
{
//for US field stream
//2019JUN_KR-17_-41.0dBm_8MHz_20190626_091839_0000.dat
//2019JUN_KR-17_-42.0dBm_8MHz_20190626_062710_0000.dat
//2019JUN_NPG_DAY3_KR-17_-40.0dBm_8MHz_20190628_011909_0000.dat

	int ret = FUNCTION_SUCCESS;
	unsigned int l1bSwParsingTime = 0x7E90, PreL1bSwParsingTime = 0;
	unsigned int u32DataTmp = 0;
	unsigned int L1ParsingDone = 0;
	unsigned int L1BNumSubframes = 0, L1BPreamNumSymb = 0;
	unsigned int L1B1stSubFftSize = 0, L1B1stSubGI = 0, L1DSubf1NumPlp = 0;
	unsigned int PlpALayer = 0, PlpAMod = 0, PlpATiMode = 0, PlpAHtiInterSubframe = 0, PlpAHtiFecBlocksMax = 0, PlpAHtiNumTiBlocks = 0, PlpAHtiFecBlocks = 0;
	unsigned int PlpBLayer = 0, PlpBMod = 0, PlpBTiMode = 0, PlpBHtiInterSubframe = 0, PlpBHtiFecBlocksMax = 0, PlpBHtiNumTiBlocks = 0, PlpBHtiFecBlocks = 0;
	unsigned int PlpCLayer = 0, PlpCMod = 0, PlpCTiMode = 0, PlpCHtiInterSubframe = 0, PlpCHtiFecBlocksMax = 0, PlpCHtiNumTiBlocks = 0, PlpCHtiFecBlocks = 0;

	*flag = 0xf;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB8161614, &u32DataTmp);
	PreL1bSwParsingTime = u32DataTmp & 0xffff;

	ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E810, &u32DataTmp);
	L1ParsingDone = u32DataTmp & 0x3;

	if (L1ParsingDone == 0x3) {

		*flag = 0x0;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xb816E834, &u32DataTmp);
		L1BNumSubframes = (u32DataTmp >> 16) & 0xff;
		ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E814, &u32DataTmp);
		L1BPreamNumSymb = (u32DataTmp >> 28) & 0x7;

		if ((L1BNumSubframes == 0) && (L1BPreamNumSymb == 0))  {
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E904, &u32DataTmp);
			L1B1stSubFftSize = (u32DataTmp >> 8) & 0x3;
			L1B1stSubGI = u32DataTmp  & 0xF;
			ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816E908, &u32DataTmp);
			L1DSubf1NumPlp = (u32DataTmp >> 16) & 0x3f;

			if ((L1B1stSubFftSize == 1) && (L1B1stSubGI == 6) && (L1DSubf1NumPlp == 2)) {
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED08, &u32DataTmp);
				PlpALayer = (u32DataTmp >> 16) & 0x3;
				PlpAMod = (u32DataTmp >> 4) & 0xf;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED18, &u32DataTmp);
				PlpATiMode = (u32DataTmp >> 16) & 0x3;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED24, &u32DataTmp);
				PlpAHtiInterSubframe = (u32DataTmp >> 20) & 0x1;
				PlpAHtiFecBlocksMax = (u32DataTmp >> 4) & 0xfff;
				PlpAHtiNumTiBlocks = u32DataTmp & 0xf;
				ret |= rtd_4bytes_i2c_rd(pDemod, 0xB816ED28, &u32DataTmp);
				PlpAHtiFecBlocks = u32DataTmp & 0xfff;

				if ((PlpALayer == 0) && (PlpAMod == 3) && (PlpATiMode == 2) && (PlpAHtiInterSubframe == 1) && (PlpAHtiFecBlocksMax == 0x14) && (PlpAHtiNumTiBlocks == 0) && (PlpAHtiFecBlocks == 0x14)) {
					ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED50, &u32DataTmp);
					PlpBLayer = (u32DataTmp >> 16) & 0x3;
					PlpBMod = (u32DataTmp >> 4) & 0xf;
					ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED60, &u32DataTmp);
					PlpBTiMode = (u32DataTmp >> 16) & 0x3;
					ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED6C, &u32DataTmp);
					PlpBHtiInterSubframe = (u32DataTmp >> 20) & 0x1;
					PlpBHtiFecBlocksMax = (u32DataTmp >> 4) & 0xfff;
					PlpBHtiNumTiBlocks = u32DataTmp & 0xf;
					ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED70, &u32DataTmp);
					PlpBHtiFecBlocks = u32DataTmp & 0xfff;

					if ((PlpBLayer == 0) && (PlpBMod == 3) && (PlpBTiMode == 2) && (PlpBHtiInterSubframe == 1) && (PlpBHtiFecBlocksMax == 0x14) && (PlpBHtiNumTiBlocks == 0) && (PlpBHtiFecBlocks == 0x14)) {
						ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816ED98, &u32DataTmp);
						PlpCLayer = (u32DataTmp >> 16) & 0x3;
						PlpCMod = (u32DataTmp >> 4) & 0xf;
						ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDA8, &u32DataTmp);
						PlpCTiMode = (u32DataTmp >> 16) & 0x3;
						ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDB4, &u32DataTmp);
						PlpCHtiInterSubframe = (u32DataTmp >> 20) & 0x1;
						PlpCHtiFecBlocksMax = (u32DataTmp >> 4) & 0xfff;
						PlpCHtiNumTiBlocks = u32DataTmp & 0xf;
						ret |= rtd_4bytes_i2c_rd(pDemod, 0x1816EDB8, &u32DataTmp);
						PlpCHtiFecBlocks = u32DataTmp & 0xfff;

						if ((PlpCLayer == 0) && (PlpCMod == 3) && (PlpCTiMode == 2) && (PlpCHtiInterSubframe == 1) && (PlpCHtiFecBlocksMax == 0x14) && (PlpCHtiNumTiBlocks == 0) && (PlpCHtiFecBlocks == 0x14)) {
							*flag = 0x1;
							l1bSwParsingTime = 0xFD20;

						}
					}
				}
			}
		}
	}


	if (PreL1bSwParsingTime != l1bSwParsingTime) {
		ret |= rtd_4bytes_i2c_mask(pDemod, 0xB8161614, 0xFFFF0000, l1bSwParsingTime);
		//reset DSP
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000001);//0x0005 = 0x01//Reset DSP
		ret |= rtd_4bytes_i2c_wr(pDemod, 0xb8160014, 0x00000000);//0x0005 = 0x00//Start DSP
		RTK_DEMOD_ATSC3_INFO("PreL1bSwParsingTime change to = 0x%x (default: 0x7E90) \n", l1bSwParsingTime);
	}

	return ret;

}


