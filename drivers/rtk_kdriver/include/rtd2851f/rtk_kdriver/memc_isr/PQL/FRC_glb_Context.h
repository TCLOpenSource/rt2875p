#ifndef _FRC_GLB_CONTEXT_DEF_H
#define _FRC_GLB_CONTEXT_DEF_H

#define IDC_Total_Frm_cnt			26
#define IDC_Total_Frm_cnt_LP		12
#define IDC_Total_Frm_cnt_HP		14
#define _CAD_CHANGE_QUE_BUF_FRM		19

typedef struct  
{
	unsigned char  state[_CAD_CHANGE_QUE_BUF_FRM];
	unsigned int  type[_CAD_CHANGE_QUE_BUF_FRM];
	unsigned char  mc_wrt_index[_CAD_CHANGE_QUE_BUF_FRM];
}CAD_CHANGE_QUE_BUFFER;

typedef struct  
{
	unsigned char  me_wrt_oft;
	unsigned char  me1_rd_i_oft;
	unsigned char  me1_rd_p_oft;
	unsigned char  me2_rd_i_oft;
	unsigned char  me2_rd_p_oft;
	unsigned char  mc_wrt_oft;
	unsigned char  mc_rd_i_oft;
	unsigned char  mc_rd_p_oft;
	unsigned char  me_phase;
	unsigned char  mc_phase;
}IDX_DYNAMIC_CTRL_OFFSET_TABLE;

#if CONFIG_MC_8_BUFFER
typedef enum  
{
	_CASE_NULL,			// 0
	_CASE_11_TO_2_NEW,		// 1
	_CASE_11_TO_22_NEW,		// 2
	_CASE_11_TO_12_NEW,		// 3
	_CASE_11_TO_13_NEW,		// 4
	_CASE_11_TO_232_NEW,		// 5
	_CASE_11_TO_323_NEW,		// 6
	_CASE_11_TO_322_NEW,	// 7
	_CASE_11_TO_321_NEW,	// 7
	_CASE_11_TO_222_NEW,		// 8
	_CASE_11_TO_223_NEW,		// 9
	_CASE_11_TO_221_NEW,		// 10
	_CASE_11_TO_212_NEW,		// 11
	_CASE_11_TO_122_NEW,		// 12
	_CASE_11_TO_31_NEW,		// 10
	_CASE_11_TO_4_NEW,		// 11
	_CASE_11_TO_422_NEW,		// 12
	_CASE_22_TO_11_NEW,		// 13
	_CASE_22_TO_111_NEW,		// 14
	_CASE_22_TO_3_NEW,		// 15
	_CASE_22_TO_32_NEW,		// 16
	_CASE_22_TO_322_NEW,		// 17
	_CASE_32_TO_11_NEW,		// 18
	_CASE_32_TO_111_NEW,		// 19
	_CASE_32_TO_11_SC_NEW,	// 20
	_CASE_32_TO_122_SC_NEW,	// 21
	_CASE_32_TO_1122_SC_NEW,	// 22
	_CASE_32_TO_11112_NEW,	// 23
	_CASE_32_TO_2_NEW,		// 24
	_CASE_32_TO_23_NEW,		// 25
	_CASE_23_TO_32_NEW,		// 26
	_CASE_32_TO_21_NEW,		// 27
	_CASE_32_TO_211_NEW,		// 28
	_CASE_32_TO_212_NEW,		// 29
	_CASE_32_TO_22_NEW,		// 30
	_CASE_32_TO_223_NEW,		// 31
	_CASE_32_TO_324_NEW,		// 32
	_CASE_32_TO_3214_NEW,	// 33
	_CASE_32_TO_4_NEW,		// 34
	_CASE_32_TO_423_NEW,		// 35
	_CASE_32_TO_31_NEW,		// 36
	_CASE_32_TO_311_NEW,		// 37
	_CASE_32_TO_313_NEW,		// 38
	_CASE_33_TO_11_NEW,		// 39
	_CASE_32322_TO_11_NEW,	// 40
	_CASE_32322_TO_111_NEW,	// 41
	_CASE_32322_TO_22_NEW,	// 42
	_CASE_32322_TO_4_NEW,	// 43
	_CASE_2224_TO_11_NEW,	// 44
	_CASE_2224_TO_111_NEW,	// 45
	_CASE_2224_TO_423_NEW,	// 46
	_CASE_2224_TO_422_NEW,	// 46
	_CASE_2224_TO_3_NEW,		// 47
	_CASE_2224_TO_32_NEW,	// 48
	_CASE_2224_TO_322_NEW,	// 49  // same as _CASE_2224_TO_4232_NEW,
	_CASE_2224_TO_4242_NEW,	// 49
	_CASE_2224_TO_41222_NEW,	// 49
	_CASE_22224_TO_111_NEW,	// 50

	_CASE_11_TO_32_CHG,		// 51
	_CASE_11_TO_32322_CHG,	// 52
	_CASE_11_TO_2224_CHG,		// 53
	_CASE_11_TO_22224_CHG,	// 54
	_CASE_11_TO_11112_CHG,	// 55
	_CASE_22_TO_11_CHG,		// 56
	_CASE_22_TO_32_CHG,		// 57
	_CASE_22_TO_2224_CHG,		// 58
	_CASE_32_TO_11_CHG,		// 59
	_CASE_32_TO_22_CHG,		// 60
	_CASE_32_TO_2224_CHG,		// 61
	_CASE_32_TO_32322_CHG,	// 62
	_CASE_32_TO_22224_CHG,	// 63
	_CASE_32322_TO_11_CHG,	// 64
	_CASE_32322_TO_22_CHG,	// 65
	_CASE_32322_TO_32_CHG,	// 66
	_CASE_32322_TO_2224_CHG,	// 67
	_CASE_32322_TO_22224_CHG,	// 68
	_CASE_2224_TO_11_CHG,		// 69
	_CASE_2224_TO_22_CHG,		// 70
	_CASE_2224_TO_32_CHG,		// 71
	_CASE_2224_TO_32322_CHG,	// 72
	_CASE_22224_TO_11_CHG,	// 73
	_CASE_22224_TO_22_CHG,	// 74
	_CASE_22224_TO_32_CHG,	// 75
	_CASE_22224_TO_32322_CHG,	// 76
	_CASE_11112_TO_11_CHG,	// 77

	_CASE_22_TO_MIX_NEW,		// 78
	_CASE_32_TO_MIX_NEW,		// 79
	_CASE_2224_TO_MIX_NEW,	// 80
	_CASE_11112_TO_MIX_NEW,	// 81

	_CASE_FILM_TO_11_CHG,		// 82
	_CASE_FILM_TO_MIX_CHG,		// 82

	_CASE_91_NEW,
	_CASE_91_TO_91_NEW,
	_CASE_91_TO_93_NEW,
	_CASE_92_NEW,
	_CASE_92_TO_91_NEW,
	_CASE_92_TO_93_NEW,
	_CASE_93_NEW,
	_CASE_93_TO_91_NEW,
	_CASE_93_TO_92_NEW,
	_CASE_94_NEW,
	_CASE_94_TO_91_NEW,
	_CASE_94_TO_93_NEW,
	_CASE_95_NEW,
	_CASE_95_TO_91_NEW,
	_CASE_95_TO_93_NEW,
	_CASE_96_NEW,

	_CASE_NUM_NEW,
}IDX_DYNAMIC_CTRL_CASE_ID_NEW;
#else
typedef enum  
{
	_CASE_NULL, // 0
	_CASE_22_TO_32,
	_CASE_32_TO_22,
	_CASE_11_TO_22_TYPE1,
	_CASE_11_TO_22_TYPE2,
	_CASE_22_TO_11, // 5
	_CASE_11_TO_32_TYPE1,
	_CASE_11_TO_32_TYPE5,
	_CASE_32_TO_11_TYPE1,
	_CASE_32_TO_11_TYPE2,
	_CASE_32_TO_11_TYPE4, // 10
	_CASE_11_TO_2224_TYPE1,
	_CASE_11_TO_2224_TYPE2,
	_CASE_11_TO_2224_TYPE3,
	_CASE_11_TO_2224_TYPE4,
	_CASE_11_TO_2224_TYPE5, // 15
	_CASE_2224_TO_11_TYPE1,
	_CASE_2224_TO_11_TYPE3,
	_CASE_2224_TO_11_TYPE5,
	_CASE_2224_TO_11_TYPE7,
	_CASE_2224_TO_11_TYPE8, // 20
	_CASE_2224_TO_11_TYPE9,
	_CASE_22_TYPE1_TO_2224_TYPE6,
	_CASE_2224_TYPE1_TO_22_TYPE0,
	_CASE_32_TYPE4_TO_2224_TYPE1,
	_CASE_2224_TYPE8_TO_32_TYPE3,
	_CASE_22_TO_MIX,
	_CASE_32_TO_MIX,

	_CASE_NUM,
}IDX_DYNAMIC_CTRL_CASE_ID;
#endif

typedef enum{
	_CAD_VIDEO, 	// 0
	_CAD_22,      	// 1
	_CAD_32,		// 2
	_CAD_32322,	// 3
	_CAD_334,		// 4
	_CAD_22224,	//5
	_CAD_2224,		//6
	_CAD_3223,		//7
	_CAD_55,		//8
	_CAD_66,		//9
	_CAD_44,		//0xA
	_CAD_1112,		//0xB
	_CAD_11112,	//0xC
	_CAD_122,		//0xD
	_CAD_11i23,		//0xE
	_CAD_321,		//0xF
	_CAD_1225_T1,	//0x10
	_CAD_1225_T2,	//0x11
	_CAD_12214,		//0x12
	_FRC_CADENCE_NUM_,
}FRC_CADENCE_ID;

typedef struct
{
	unsigned int cad_seq;
	unsigned int cad_usableFlag;
	unsigned char  cad_inLen;
	unsigned char  cad_outLen;
	unsigned char  cad_isLRin;

	unsigned int cad_check;
	unsigned char  cad_len;
	unsigned char  cad_film_resync_phId;

	unsigned char  cad_id;
	unsigned char  u1_isLowFrameRate;
	
} FRC_CADENCE;

extern FRC_CADENCE  frc_cadTable[_FRC_CADENCE_NUM_];

// void  cadenceTable_int();

void  cadence_copy_change(FRC_CADENCE_ID cpy_cadId, unsigned char isLR, FRC_CADENCE *pOut_cadInfo);
unsigned char cadence_seqFlag(FRC_CADENCE cadInfo, unsigned char filmPh);
unsigned char cadence_usableFlag(FRC_CADENCE cadInfo, unsigned char filmPh);
unsigned char cadence_prePhase(FRC_CADENCE cadInfo, unsigned char filmPh, unsigned char k);
unsigned char cadence_nxtPhase(FRC_CADENCE cadInfo, unsigned char filmPh, unsigned char k);

#endif

