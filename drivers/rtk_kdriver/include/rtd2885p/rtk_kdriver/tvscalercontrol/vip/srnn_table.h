#ifndef _SRNN_TABLE_H_
#define _SRNN_TABLE_H_

#include <scaler/vipCommon.h>

/* ----- ----- ----- ----- ----- HEADER ----- ----- ----- ----- ----- */
/* ----- ----- ----- ----- ----- DEFINITIONS ----- ----- ----- ----- ----- */
#if 0 // move to vipcommon

#define SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX (8)

#define SRNN_WEIGHT_NUM_MODE2_SU0 (37825)
#define SRNN_WEIGHT_NUM_MODE2_SU1 (38044)
#define SRNN_WEIGHT_NUM_MODE2_SU2 (38409)
#define SRNN_WEIGHT_NUM_MODE2_SU3 (38920)
#define SRNN_WEIGHT_NUM_MODE2_SU4 (38920)
#define SRNN_WEIGHT_NUM_MODE2_SU5 (38409)

#define SRNN_WEIGHT_NUM_MODE3_SU0 (19169)
#define SRNN_WEIGHT_NUM_MODE3_SU1 (19388)
#define SRNN_WEIGHT_NUM_MODE3_SU2 (19753)
#define SRNN_WEIGHT_NUM_MODE3_SU3 (20264)
#define SRNN_WEIGHT_NUM_MODE3_SU4 (20264)
#define SRNN_WEIGHT_NUM_MODE3_SU5 (19753)

#define SRNN_WEIGHT_NUM_MODE4_SU0 (9953)
#define SRNN_WEIGHT_NUM_MODE4_SU1 (10172)
#define SRNN_WEIGHT_NUM_MODE4_SU2 (10537)
#define SRNN_WEIGHT_NUM_MODE4_SU3 (11048)
#define SRNN_WEIGHT_NUM_MODE4_SU4 (11048)
#define SRNN_WEIGHT_NUM_MODE4_SU5 (10537)

#define SRNN_WEIGHT_NUM_MODE5_SU0 (5033)
#define SRNN_WEIGHT_NUM_MODE5_SU1 (5252)
#define SRNN_WEIGHT_NUM_MODE5_SU2 (5617)
#define SRNN_WEIGHT_NUM_MODE5_SU3 (6128)
#define SRNN_WEIGHT_NUM_MODE5_SU4 (6128)
#define SRNN_WEIGHT_NUM_MODE5_SU5 (5617)

#define SRNN_WEIGHT_NUM_MAX (38920)

#define SEM_SRNN_LABEL_NUM (8)
#define DEPTH_SRNN_LABEL_NUM (32)

/*******************************************************************************
 * Structure
 ******************************************************************************/

enum {
	SRNN_SCALE_MODE_1x = 0,
	SRNN_SCALE_MODE_2x = 1,
	SRNN_SCALE_MODE_3x = 2,
	SRNN_SCALE_MODE_4x = 3,
	SRNN_SCALE_MODE_4_3x = 4,
	SRNN_SCALE_MODE_3_2x = 5,
	SRNN_SCALE_MODE_MAX,
};

enum {
	SRNN_TBL_SRC_VIPSRNN = 0,
	SRNN_TBL_SRC_DEFAULT = 1,
	SRNN_TBL_SRC_MAX,
};


typedef enum _VIP_SRNN_SOURCE_TIMING {
	VIP_SRNN_60HZ_1X, 
	VIP_SRNN_60HZ_1_33X,
	VIP_SRNN_60HZ_1_5X_576,
	VIP_SRNN_60HZ_1_5X_720,
	VIP_SRNN_60HZ_2X_576,
	VIP_SRNN_60HZ_2X_720,
	VIP_SRNN_60HZ_2X_PREDOWN,
	VIP_SRNN_60HZ_3X_576,
	VIP_SRNN_60HZ_3X_720,
	VIP_SRNN_60HZ_4X, 
	VIP_SRNN_120HZ_2X_576,
	VIP_SRNN_120HZ_2X_720,
	VIP_SRNN_120HZ_3X_576,
	VIP_SRNN_120HZ_3X_720,
	VIP_SRNN_120HZ_4X,
	VIP_SRNN_HSR_1X,
	VIP_SRNN_HSR_1_33X,
	VIP_SRNN_HSR_1_5X_576,
	VIP_SRNN_HSR_1_5X_720,
	VIP_SRNN_HSR_2X_576,
	VIP_SRNN_HSR_2X_720,
	VIP_SRNN_HSR_2X_PREDOWN,
	VIP_SRNN_HSR_3X_576,
	VIP_SRNN_HSR_3X_720,
	VIP_SRNN_HSR_4X,
	VIP_SRNN_SQM_4K_4K,
	VIP_SRNN_SQM_4K_2KGOOD,
	VIP_SRNN_SQM_4K_2KNORM,
	VIP_SRNN_SQM_4K_720,
	VIP_SRNN_SQM_4K_480,
	VIP_SRNN_SQM_2K_2KGOOD,
	VIP_SRNN_SQM_2K_2KNORM,
	VIP_SRNN_SQM_2K_720,
	VIP_SRNN_SQM_2K_480,
	VIP_SRNN_TIMING_MAX,
}VIP_SRNN_SOURCE_TIMING;

typedef enum _VIP_SRNN_PANEL {
	VIP_SRNN_PANEL_1366_768,
	VIP_SRNN_PANEL_1920_1080,
	VIP_SRNN_PANEL_2560_1440,
	VIP_SRNN_PANEL_3840_2160,
	VIP_SRNN_PANEL_MAX,
}VIP_SRNN_PANEL;

enum {
	PQMASK_WEIGHT_SRNN_IN_0,		// srnn-domain srnn-i0
	PQMASK_WEIGHT_SRNN_IN_1,		// srnn-domain srnn-i1
	PQMASK_WEIGHT_SRNN_IN_2,		// srnn-domain srnn-i2
	PQMASK_WEIGHT_SRNN_OUT_0,		// srnn-domain srnn-o0
	PQMASK_WEIGHT_SRNN_MAX,
};

typedef struct VIP_SRNN_POST_CTRL {
	unsigned char Coring;
	unsigned char GainPos;
	unsigned char GainNeg;
	unsigned short ClampPos;
	unsigned short ClampNeg;
} VIP_SRNN_POST_CTRL;

typedef struct VIP_SRNN_LPF_CTRL {
	unsigned char Mode;
	unsigned char Coeff0[3];
} VIP_SRNN_LPF_CTRL;

typedef struct VIP_SRNN_POST_TABLE{
	VIP_SRNN_POST_CTRL PSNR_Post;
	VIP_SRNN_LPF_CTRL LPF;
} VIP_SRNN_POST_TABLE;

typedef struct VIP_SRNN_PQMASK_CTRL{
	unsigned char Gain;
	unsigned char Offset;
} VIP_SRNN_PQMASK_CTRL;

typedef struct VIP_SRNN_PQMASK_TABLE {
	VIP_SRNN_PQMASK_CTRL In[3];
	VIP_SRNN_PQMASK_CTRL Out;
} VIP_SRNN_PQMASK_TABLE;

typedef struct VIP_SRNN_Hx2_TABLE {
	unsigned short Coeff0[4];
	unsigned short Coeff1[4];
} VIP_SRNN_Hx2_TABLE;

typedef struct VIP_SRNN_BLOCK_CTRL {
	unsigned char BlockDisable[8];
	unsigned char FeatReduce[8];
} VIP_SRNN_BLOCK_CTRL;

typedef struct DRV_srnn_table {
	VIP_SRNN_POST_TABLE PostProc;
	VIP_SRNN_PQMASK_TABLE PQmask;
	VIP_SRNN_BLOCK_CTRL BlockCtrl;
	VIP_SRNN_Hx2_TABLE Hx2;
} DRV_srnn_table;

typedef struct {
	VIP_SRNN_POST_TABLE PostProc;
	VIP_SRNN_PQMASK_TABLE PQmask;
} DRV_SRNN_CTRL_TABLE;

typedef struct VIP_SRNN_DEFAULT_TABLE_INFO {
	unsigned char TableSrc;
	unsigned char TableIdx;
	unsigned char TimingIdx;
} VIP_SRNN_TABLE_INFO;

typedef struct {
	const char Version[32];
	const char CustomerName[32];
	const char ProjectName[32];
	const char ModelName[32];
	const char PCBVersion[32];
	const char ReservedString[32];
	VIP_SRNN_PANEL PanelSize; 
}DRV_SRNN_TABLE_LABEL;

typedef struct {
	unsigned short Param_SRNN_60HZ_1X[SRNN_WEIGHT_NUM_MODE4_SU0];        
	unsigned short Param_SRNN_60HZ_1_33X[SRNN_WEIGHT_NUM_MODE4_SU4];     
	unsigned short Param_SRNN_60HZ_1_5X_576[SRNN_WEIGHT_NUM_MODE4_SU5];  
	unsigned short Param_SRNN_60HZ_1_5X_720[SRNN_WEIGHT_NUM_MODE4_SU5];  
	unsigned short Param_SRNN_60HZ_2X_576[SRNN_WEIGHT_NUM_MODE4_SU1];    
	unsigned short Param_SRNN_60HZ_2X_720[SRNN_WEIGHT_NUM_MODE4_SU1];                  
	unsigned short Param_SRNN_60HZ_2X_PREDOWN[SRNN_WEIGHT_NUM_MODE4_SU1];              
	unsigned short Param_SRNN_60HZ_3X_576[SRNN_WEIGHT_NUM_MODE3_SU2]; 
	unsigned short Param_SRNN_60HZ_3X_720[SRNN_WEIGHT_NUM_MODE3_SU2]; 
	unsigned short Param_SRNN_60HZ_4X[SRNN_WEIGHT_NUM_MODE2_SU3];                         
	unsigned short Param_SRNN_120HZ_2X_576[SRNN_WEIGHT_NUM_MODE5_SU1];                    
	unsigned short Param_SRNN_120HZ_2X_720[SRNN_WEIGHT_NUM_MODE5_SU1];
	unsigned short Param_SRNN_120HZ_3X_576[SRNN_WEIGHT_NUM_MODE4_SU2];                                     
	unsigned short Param_SRNN_120HZ_3X_720[SRNN_WEIGHT_NUM_MODE4_SU2];                
	unsigned short Param_SRNN_120HZ_4X[SRNN_WEIGHT_NUM_MODE3_SU3];    
	unsigned short Param_SRNN_HSR_1X[SRNN_WEIGHT_NUM_MODE4_SU0];     
	unsigned short Param_SRNN_HSR_1_33X[SRNN_WEIGHT_NUM_MODE4_SU4];   
	unsigned short Param_SRNN_HSR_1_5X_576[SRNN_WEIGHT_NUM_MODE4_SU5];
	unsigned short Param_SRNN_HSR_1_5X_720[SRNN_WEIGHT_NUM_MODE4_SU5];
	unsigned short Param_SRNN_HSR_2X_576[SRNN_WEIGHT_NUM_MODE4_SU1];   
	unsigned short Param_SRNN_HSR_2X_720[SRNN_WEIGHT_NUM_MODE4_SU1];    
	unsigned short Param_SRNN_HSR_2X_PREDOWN[SRNN_WEIGHT_NUM_MODE4_SU1];
	unsigned short Param_SRNN_HSR_3X_576[SRNN_WEIGHT_NUM_MODE3_SU2];  
	unsigned short Param_SRNN_HSR_3X_720[SRNN_WEIGHT_NUM_MODE3_SU2];   
	unsigned short Param_SRNN_HSR_4X[SRNN_WEIGHT_NUM_MODE2_SU3];      	
	unsigned short Param_SRNN_SQM_4K_4K[SRNN_WEIGHT_NUM_MODE4_SU1];    	
	unsigned short Param_SRNN_SQM_4K_2KGOOD[SRNN_WEIGHT_NUM_MODE4_SU1];	
	unsigned short Param_SRNN_SQM_4K_2KNORM[SRNN_WEIGHT_NUM_MODE4_SU1];
	unsigned short Param_SRNN_SQM_4K_720[SRNN_WEIGHT_NUM_MODE4_SU1];  
	unsigned short Param_SRNN_SQM_4K_480[SRNN_WEIGHT_NUM_MODE4_SU1];   
	unsigned short Param_SRNN_SQM_2K_2KGOOD[SRNN_WEIGHT_NUM_MODE4_SU1];
	unsigned short Param_SRNN_SQM_2K_2KNORM[SRNN_WEIGHT_NUM_MODE4_SU1];
	unsigned short Param_SRNN_SQM_2K_720[SRNN_WEIGHT_NUM_MODE4_SU1];
	unsigned short Param_SRNN_SQM_2K_480[SRNN_WEIGHT_NUM_MODE4_SU1];
}DRV_SRNN_PARAM;

typedef struct {
	DRV_SRNN_TABLE_LABEL Label; 
	unsigned char PQMask_SRNN_WeightTable[VIP_SRNN_TIMING_MAX][PQMASK_WEIGHT_SRNN_MAX][SEM_SRNN_LABEL_NUM];
	unsigned char Depth_SRNN_WeightTable[VIP_SRNN_TIMING_MAX][PQMASK_WEIGHT_SRNN_MAX][DEPTH_SRNN_LABEL_NUM];
	DRV_SRNN_CTRL_TABLE SRNN_Ctrl_Tbl[VIP_SRNN_TIMING_MAX];
	DRV_SRNN_PARAM SRNN_Param;
}SRNN_VIP_TABLE;
#endif
/* ----- ----- ----- ----- ----- GLOBAL PARMETER ----- ----- ----- ----- ----- */
extern VIP_SRNN_TABLE_INFO gSrnnTableInfo;
extern const unsigned int gSrnnWeightNum[4][SRNN_SCALE_MODE_MAX];
extern unsigned short srnn_current_table[SRNN_WEIGHT_NUM_MAX];

// Empty table for verify
extern unsigned short mode2su0_default_table[SRNN_WEIGHT_NUM_MODE2_SU0];
extern unsigned short mode2su1_default_table[SRNN_WEIGHT_NUM_MODE2_SU1];
extern unsigned short mode2su2_default_table[SRNN_WEIGHT_NUM_MODE2_SU2];
extern unsigned short mode2su4_default_table[SRNN_WEIGHT_NUM_MODE2_SU4];
extern unsigned short mode2su5_default_table[SRNN_WEIGHT_NUM_MODE2_SU5];

extern unsigned short mode3su0_default_table[SRNN_WEIGHT_NUM_MODE3_SU0];
extern unsigned short mode3su1_default_table[SRNN_WEIGHT_NUM_MODE3_SU1];
extern unsigned short mode3su4_default_table[SRNN_WEIGHT_NUM_MODE3_SU4];
extern unsigned short mode3su5_default_table[SRNN_WEIGHT_NUM_MODE3_SU5];

extern unsigned short mode4su3_default_table[SRNN_WEIGHT_NUM_MODE4_SU3];

extern unsigned short mode5su2_default_table[SRNN_WEIGHT_NUM_MODE5_SU2];
extern unsigned short mode5su3_default_table[SRNN_WEIGHT_NUM_MODE5_SU3];

// Default table for init
// x1
extern unsigned short mode4su0_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE4_SU0];	// 60Hz
extern unsigned short mode5su0_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE5_SU0];	// 120Hz
// x1.33
extern unsigned short mode4su4_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE4_SU4];	// 60Hz
extern unsigned short mode5su4_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE5_SU4];	// 120Hz
// x1.5
extern unsigned short mode4su5_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE4_SU5];	// 60Hz
extern unsigned short mode5su5_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE5_SU5];	// 120Hz
// x2
extern unsigned short mode4su1_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE4_SU1];	// 60Hz
extern unsigned short mode5su1_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE5_SU1];	// 120Hz
// x3
extern unsigned short mode3su2_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE3_SU2];	// 60Hz
extern unsigned short mode4su2_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE4_SU2];	// 120Hz
// x4
extern unsigned short mode2su3_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE2_SU3];	// 60Hz
extern unsigned short mode3su3_default_table[SRNN_DEFAULT_WEIGHT_TABLE_NUM_MAX][SRNN_WEIGHT_NUM_MODE3_SU3];	// 120Hz


extern SRNN_VIP_TABLE gVipSRNNTbl;
/* ----- ----- ----- ----- ----- FUNCTIONS ----- ----- ----- ----- ----- */

#endif /* _SRNN_TABLE_H_ */
