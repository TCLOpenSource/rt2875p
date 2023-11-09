#ifndef __SCALERCOLOR_ENGMENU_H__
#define __SCALERCOLOR_ENGMENU_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Header include
******************************************************************************/


#include <tvscalercontrol/vip/icm.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scaler/vipinclude.h>
#include <tvscalercontrol/vip/color.h>


/*******************************************************************************
* Macro
******************************************************************************/


/*******************************************************************************
* Constant
******************************************************************************/


/*******************************************************************************
* enumeration
******************************************************************************/
typedef enum _HAL_VPQ_ENG_ITEM_NUM
{
	HAL_VPQ_ENG_VIP_QUALITY_Coef = 0,
	HAL_VPQ_ENG_VIP_QUALITY_Extend_Coef,
	HAL_VPQ_ENG_OTHER,
	HAL_VPQ_ENG_PQ_Function,
	HAL_VPQ_ENG_HDR,
	HAL_VPQ_ENG_ICM_7_COLOR,
	HAL_VPQ_ENG_ITEM_MAX_NUM,
}HAL_VPQ_ENG_ITEM_NUM;

typedef enum _HAL_VPQ_Coef_ID_NUM
{
	HAL_VPQ_ENG_DLTi = 0,
	HAL_VPQ_ENG_DCTi,
	HAL_VPQ_Coef_ID_MAX_NUM,
}HAL_VPQ_Coef_ID_NUM;

typedef enum _HAL_VPQ_Extend_Coef_ID_NUM
{
	HAL_VPQ_ENG_CDS = 0,
	HAL_VPQ_ENG_EMFMK2i,
	HAL_VPQ_ENG_LC_DEMO_Split_Screen,
	HAL_VPQ_ENG_LC_DEMO_Step,
	HAL_VPQ_ENG_LC_DEMO_Debug,
	HAL_VPQ_ENG_OSDHDR_DEMO_MODE,
	HAL_VPQ_Extend_Coef_ID_MAX_NUM,
}HAL_VPQ_Extend_Coef_ID_NUM;

typedef enum _HAL_VPQ_OTHER_ID_NUM
{
	HAL_VPQ_ENG_OD_EN_TEST = 0,
	HAL_VPQ_ENG_OD_BIT_TEST,
	HAL_VPQ_ENG_DEMURA_EN,
	HAL_VPQ_ENG_PQ_TABLE_BIN_EN,
	HAL_VPQ_ENG_I_De_Contour,
	HAL_VPQ_ENG_VDPQ_PRJ_ID,
	HAL_VPQ_OTHER_ID_MAX_NUM,
}HAL_VPQ_OTHER_ID_NUM;

typedef enum _HAL_VPQ_FUNCTION_ID_NUM
{
	HAL_VPQ_FUNCTION_PCID = 0,
	HAL_VPQ_FUNCTION_OD,
	HAL_VPQ_FUNCTION_Inv_Output_Gamma,
	HAL_VPQ_FUNCTION_Output_Gamma,
	HAL_VPQ_FUNCTION_RGBW,
	HAL_VPQ_FUNCTION_Demura,
	HAL_VPQ_FUNCTION_ID_MAX_NUM,
}HAL_VPQ_FUNCTION_ID_NUM;

typedef enum _HAL_VPQ_HDR_ID_NUM
{
	HAL_VPQ_ENG_ST2094_EN = 0,
	HAL_VPQ_ENG_ST2094_EOTF_Gain,
	HAL_VPQ_ENG_ST2094_EOTF_Linear_W,

	HAL_VPQ_HDR_ID_MAX_NUM,

}HAL_VPQ_HDR_ID_NUM;

typedef enum _HAL_VPQ_ENG_ICM_7_COLOR_NUM
{
	HAL_VPQ_ENG_R_START = 0,
	HAL_VPQ_ENG_R_END,
	HAL_VPQ_ENG_R_HUE,
	HAL_VPQ_ENG_R_SAT,
	HAL_VPQ_ENG_R_ITN,

	HAL_VPQ_ENG_SKIN_START,
	HAL_VPQ_ENG_SKIN_END,
	HAL_VPQ_ENG_SKIN_HUE,
	HAL_VPQ_ENG_SKIN_SAT,
	HAL_VPQ_ENG_SKIN_ITN,

	HAL_VPQ_ENG_Y_START,
	HAL_VPQ_ENG_Y_END,
	HAL_VPQ_ENG_Y_HUE,
	HAL_VPQ_ENG_Y_SAT,
	HAL_VPQ_ENG_Y_ITN,

	HAL_VPQ_ENG_G_START,
	HAL_VPQ_ENG_G_END,
	HAL_VPQ_ENG_G_HUE,
	HAL_VPQ_ENG_G_SAT,
	HAL_VPQ_ENG_G_ITN,

	HAL_VPQ_ENG_C_START,
	HAL_VPQ_ENG_C_END,
	HAL_VPQ_ENG_C_HUE,
	HAL_VPQ_ENG_C_SAT,
	HAL_VPQ_ENG_C_ITN,

	HAL_VPQ_ENG_B_START,
	HAL_VPQ_ENG_B_END,
	HAL_VPQ_ENG_B_HUE,
	HAL_VPQ_ENG_B_SAT,
	HAL_VPQ_ENG_B_ITN,

	HAL_VPQ_ENG_M_START,
	HAL_VPQ_ENG_M_END,
	HAL_VPQ_ENG_M_HUE,
	HAL_VPQ_ENG_M_SAT,
	HAL_VPQ_ENG_M_ITN,

	HAL_VPQ_ICM_7_COLOR_MAX_NUM,
}HAL_VPQ_ENG_ICM_7_COLOR_NUM;

typedef enum _HAL_VPQ_ENG_ID_TYPE_NUM
{
	HAL_VPQ_ENG_checkbox = 0,
	HAL_VPQ_ENG_combobox,
	HAL_VPQ_ENG_scrollbox,
	HAL_VPQ_ENG_display,

	HAL_VPQ_ENG_ID_MAX_TYPE_NUM,
}HAL_VPQ_ENG_ID_TYPE_NUM;



/*******************************************************************************
 * Structure
 ******************************************************************************/
#define HAL_VPQ_ENG_ID_MAX_NUM (HAL_VPQ_Coef_ID_MAX_NUM+HAL_VPQ_Extend_Coef_ID_MAX_NUM+HAL_VPQ_OTHER_ID_MAX_NUM+HAL_VPQ_FUNCTION_ID_MAX_NUM+HAL_VPQ_HDR_ID_MAX_NUM+HAL_VPQ_ICM_7_COLOR_MAX_NUM)

typedef struct _HAL_VPQ_ENG_ITEM_STRUCT {
	unsigned int item_ID;
	char item_srt[20];
} HAL_VPQ_ENG_ITEM_STRUCT;

typedef struct _HAL_VPQ_ENG_ITEM_TWOLAYER_STRUCT {
	unsigned int item_ID;
	char item_srt[20];
	unsigned int SubItem_num;
} HAL_VPQ_ENG_ITEM_TWOLAYER_STRUCT;

#if 0
typedef struct _HAL_VPQ_ENG_ITEM_STRUCT {
	unsigned int size;
	HAL_VPQ_ENG_ITEM item[HAL_VPQ_ENG_ITEM_MAX_NUM];
} HAL_VPQ_ENG_ITEM_STRUCT;
#endif

typedef struct _HAL_VPQ_ENG_ID_TYPE_NAME {
	char ID_combobox_srt[20];
} HAL_VPQ_ENG_ID_TYPE_NAME;

typedef struct _HAL_VPQ_ENG_ID_STRUCT {
	unsigned int item_ID;
	unsigned int ID;
	char ID_srt[20];
	unsigned char ID_type;
	int range_min;
	int range_max;
	HAL_VPQ_ENG_ID_TYPE_NAME combobox_srt[5];
} HAL_VPQ_ENG_ID_STRUCT;

#if 0
typedef struct _HAL_VPQ_ENG_ID_STRUCT {
	unsigned int size;
	HAL_VPQ_ENG_ID ID[HAL_VPQ_ENG_ID_MAX_NUM];
} HAL_VPQ_ENG_ID_STRUCT;
#endif

typedef struct _HAL_VPQ_ENG_STRUCT {
	HAL_VPQ_ENG_ITEM_STRUCT	item[HAL_VPQ_ENG_ITEM_MAX_NUM];
	HAL_VPQ_ENG_ID_STRUCT 	ID[HAL_VPQ_ENG_ID_MAX_NUM];
} HAL_VPQ_ENG_STRUCT;

typedef struct _HAL_VPQ_ENG_TWOLAYER_STRUCT {
	HAL_VPQ_ENG_ITEM_TWOLAYER_STRUCT	item[HAL_VPQ_ENG_ITEM_MAX_NUM];
	HAL_VPQ_ENG_ID_STRUCT 			ID[HAL_VPQ_ENG_ID_MAX_NUM];
} HAL_VPQ_ENG_TWOLAYER_STRUCT;

typedef struct _HAL_VPQ_ENG_SIZE {
	unsigned int	item_num;
	unsigned int 	ID_num;
} HAL_VPQ_ENG_SIZE;

typedef struct _HAL_VPQ_ENG_ID {
	unsigned int	ID;
	int 	ID_value;
} HAL_VPQ_ENG_ID;

typedef struct _HAL_VPQ_ENG_TWOLAYER_ID {
	unsigned int	ITEM;
	unsigned int	ID;
	int 	ID_value;
} HAL_VPQ_ENG_TWOLAYER_ID;


/*******************************************************************************
* Variable
******************************************************************************/
/*static unsigned char gExample = 100000;*/ /* 100K */



/*******************************************************************************
* Program
******************************************************************************/
HAL_VPQ_ENG_STRUCT* Scaler_Get_ENGMENU(void);
HAL_VPQ_ENG_SIZE* Scaler_Get_ENGMENU_size(void);
unsigned int Scaler_Get_ENGMENU_ID(unsigned int ID);
unsigned int Scaler_Set_ENGMENU_ID(unsigned int ID, int value);
HAL_VPQ_ENG_TWOLAYER_STRUCT* Scaler_Get_ENG_TWOLAYER_MENU(void);
unsigned int Scaler_Get_VIP_QUALITY_Coef_ID(unsigned int ID);
unsigned int Scaler_Get_VIP_QUALITY_Extend_Coef_ID(unsigned int ID);
unsigned int Scaler_Get_ENG_OTHER_ID(unsigned int ID);
unsigned int Scaler_Get_ENG_HDR_ID(unsigned int ID);
unsigned int Scaler_Get_ENG_TWOLAYER_MENU_ID(unsigned int ITEM, unsigned int ID);
unsigned int Scaler_Set_VIP_QUALITY_Coef_ID(unsigned int ID, int value);
unsigned int Scaler_Set_VIP_QUALITY_Extend_Coef_ID(unsigned int ID, int value);
unsigned int Scaler_Set_ENG_OTHER_ID(unsigned int ID, int value);
unsigned int Scaler_Set_ENG_HDR_ID(unsigned int ID, int value);
unsigned int Scaler_Set_ENG_TWOLAYER_MENU_ID(unsigned int ITEM, unsigned int ID, int value);


#ifdef __cplusplus
}
#endif

#endif /* __SCALER_ENGMENU_H__*/

