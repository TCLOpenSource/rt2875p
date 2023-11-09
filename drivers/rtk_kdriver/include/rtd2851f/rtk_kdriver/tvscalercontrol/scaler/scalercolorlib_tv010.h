#ifndef __SCALERCOLOR_LIB_TV010_H__
#define __SCALERCOLOR_LIB_TV010_H__

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
#include <tvscalercontrol/vip/scalerColor.h>
#include <tvscalercontrol/scaler/scalercolorlib_tv001.h>

/*******************************************************************************
* Macro
******************************************************************************/



/*******************************************************************************
* Constant
******************************************************************************/
/*#define example  100000*/ /* 100Khz */




/*******************************************************************************
 * Structure
 ******************************************************************************/
/*typedef struct*/
/*{*/
/*} MID_example_Param_t;*/




/*******************************************************************************
* enumeration
******************************************************************************/
typedef enum 
{
    TV010_COLORTEMP_NATURE = 0,    
    TV010_COLORTEMP_COOL,          
    TV010_COLORTEMP_WARM,        
    TV010_COLORTEMP_USER,        

    TV010_COLORTEMP_MAX          
}TV010_COLORTEMP_E;

typedef enum 
{
    TV010_DEMOLEVEL_OFF = 0,      
    TV010_DEMOLEVEL_ON,           
    TV010_DEMOLEVEL_DEMO,         

    TV010_DEMOLEVEL_MAX          
}TV010_DEMOLEVEL_E;
typedef enum 
{
    TV010_DEMO_DBDR = 0,     
    TV010_DEMO_NR,           
    TV010_DEMO_SHARPNESS,    
    TV010_DEMO_DCI,          
    TV010_DEMO_WCG,          
    TV010_DEMO_MEMC,         
    TV010_DEMO_COLOR,        
    TV010_DEMO_SR,           
    TV010_DEMO_ALL,          
    TV010_DEMO_HDR,          
    TV010_DEMO_SDR_TO_HDR,   

    TV010_DEMO_MAX          
}TV010_DEMO_MODE_E;
typedef enum 
{
     TV010_PQ_MODULE_FMD = 0,      
     TV010_PQ_MODULE_NR,           
     TV010_PQ_MODULE_DB,           
     TV010_PQ_MODULE_DR,           
     TV010_PQ_MODULE_HSHARPNESS,   
     TV010_PQ_MODULE_SHARPNESS,    
     TV010_PQ_MODULE_CCCL,         
     TV010_PQ_MODULE_COLOR_CORING, 
     TV010_PQ_MODULE_BLUE_STRETCH, 
     TV010_PQ_MODULE_GAMMA,        
     TV010_PQ_MODULE_DBC,          
     TV010_PQ_MODULE_DCI,          
     TV010_PQ_MODULE_COLOR,        
     TV010_PQ_MODULE_ES,           
     TV010_PQ_MODULE_SR,           
     TV010_PQ_MODULE_FRC,          
     TV010_PQ_MODULE_WCG,          
     TV010_PQ_MODULE_ALL,          

     TV010_PQ_MODULE_MAX          
}TV010_PQ_MODULE_E;


typedef enum 
{
    TV010_LDM_DEMO_RIGHT_SCREEN          = 0,    /**<right */ 
    TV010_LDM_DEMO_LEFT_SCREEN           = 1,    /**<left */  
    TV010_LDM_DEMO_TOP_SCREEN            = 2,    /**<top */   
    TV010_LDM_DEMO_BOTTOM_SCREEN         = 3,    /**<bottom */
    TV010_LDM_DEMO_WHITE_SCREEN          = 4,    /**<white */ 
    TV010_LDM_DEMO_RIGHT_WHITE_SCREEN    = 5,    /**<right white */ 
    TV010_LDM_DEMO_LEFT_WHITE_SCREEN     = 6,    /**<left white */ 
    TV010_LDM_DEMO_TOP_WHITE_SCREEN      = 7,    /**<top white */  
    TV010_LDM_DEMO_BOTTOM_WHITE_SCREEN   = 8,    /**<bottom white */
    TV010_LDM_DEMO_HORSELIGHT            = 9,    /**<LDM type is HORSELIGHT */ 
    TV010_LDM_DEMO_OFF                   = 10,   /**<LDM type is off */

    TV010_LDM_DEMO_MAX                        

}TV010_LOCALDIMMING_DEMO_MODE_E;

typedef enum 
{
    TV010_SDR_ICM_STD,
    TV010_SDR_ICM_VIVID,
    TV010_SDR_ICM_GENTLE,
    TV010_HDR_ICM_STD,       
    TV010_HDR_ICM_VIVID,  
    TV010_HDR_ICM_GENTLE,  
    TV010_SDR_ICM_RESERVED1,
    TV010_SDR_ICM_RESERVED2,
    TV010_SDR_ICM_RESERVED3,
    
    TV010_PQ_ICM_MAX,    
}TV010_ICM_TABLE_INDEXE_E;

typedef enum  
{ 
SKY_MW_LDM_DEMO_OFF = 0, /**<LDM type is off */ /**<CNcomment:??Demo模式 */ 
SKY_MW_LDM_DEMO_RIGHT_SCREEN = 1, /**<right */ /**<CNcomment:右?是?理后的?像，左?是源?像*/ 
SKY_MW_LDM_DEMO_LEFT_SCREEN = 2, /**<left */ /**<CNcomment:左?是?理后的?像，右?是源?像*/ 
SKY_MW_LDM_DEMO_TOP_SCREEN = 3, /**<top */ /**<CNcomment:上?是?理后的?像，下?是源?像*/ 
SKY_MW_LDM_DEMO_BOTTOM_SCREEN = 4, /**<bottom *//**<CNcomment:下?是?理后的?像，上?是源?像*/ 
SKY_MW_LDM_DEMO_WHITE_SCREEN = 5, /**<white */ /**<CNcomment:全屏白屏*/ 
SKY_MW_LDM_DEMO_RIGHT_WHITE_SCREEN = 6, /**<right white */ /**<CNcomment:右?是白屏的?像，左?是源?像*/ 
SKY_MW_LDM_DEMO_LEFT_WHITE_SCREEN = 7, /**<left white */ /**<CNcomment:左?是白屏的?像，右?是源?像*/ 
SKY_MW_LDM_DEMO_TOP_WHITE_SCREEN = 8, /**<top white */ /**<CNcomment:上?是白屏的?像，下?是源?像*/ 
SKY_MW_LDM_DEMO_BOTTOM_WHITE_SCREEN = 9, /**<bottom white *//**<CNcomment:下?是白屏的?像，上?是源?像*/ 
SKY_MW_LDM_DEMO_HORSELIGHT = 10, /**<LDM type is HORSELIGHT */ /**<CNcomment:跑??*/ 
SKY_MW_LDM_DEMO_HORSELIGHT_ONLYONCE = 11, /**<LDM type is HORSELIGHT */ /**<CNcomment:跑??只跑一次*/ 

SKY_MW_LDM_DEMO_BUTT /**<Invalid value*//**<CNcomment:非法?界值*/ 
}SKY_MW_LOCALDIMMING_DEMO_MODE_E;

typedef enum 
{
    RTK_MW_COLORTEMP_NATURE = 0,    /**< ?准*/
    RTK_MW_COLORTEMP_COOL,          /**< 冷色*/
    RTK_MW_COLORTEMP_WARM,          /**< ??*/
    RTK_MW_COLORTEMP_USER,          /**< 用?自定?*/

    RTK_MW_COLORTEMP_BUTT           /**< 未定??型*/
}RTK_MW_COLORTEMP_E;

/*******************************************************************************
* Variable
******************************************************************************/
/*static unsigned char gExample = 100000;*/ /* 100K */

//TODO, Merlin3 tv010 sync, wait vip to add
//extern Scaler_ICM_Block_Adj icm_block_adj;

/*******************************************************************************
* Program
******************************************************************************/


/*o---------------------------------------------------------------------------o*/
/*o-------------Scalercolor.cpp------------------------------------o*/
/*o---------------------------------------------------------------------------o*/


/*==================== Definitions ================= */



/*=========================================================*/


/*==================== Functions ================== */




/*o===========================================================o*/
/*o==================== OSD MENU Start =======================o*/
/*o===========================================================o*/


unsigned int Scaler_GetDeRinging_level(TV001_LEVEL_E *drLevel);
void Scaler_SetDeRinging_level(TV001_LEVEL_E drLevel);
unsigned int Scaler_GetSRLevel(TV010_DEMOLEVEL_E *srLevel);
void Scaler_SetSRLevel(TV010_DEMOLEVEL_E srLevel);
unsigned int Scaler_GetColorGainLevel(TV001_LEVEL_E *colorGain);
void Scaler_SetColorGainLevel(unsigned int icmIndex);
unsigned int Scaler_GetFleshToneLevel(TV001_LEVEL_E *fleshtone);
void Scaler_SetFleshToneLevel(TV001_LEVEL_E fleshtone);
void fwif_color_icm_set_block_para(TV001_LEVEL_E fleshtone);


/*o===========================================================o*/
/*o==================== OSD MENU End = =======================o*/
/*o===========================================================o*/
/*o===========================================================o*/
/*o==================== DemoMode Start =======================o*/
/*o===========================================================o*/

void Scaler_Set_TV010_DemoMode(TV010_DEMO_MODE_E demoMode,unsigned char onOff);

/*o===========================================================o*/
/*o==================== DemoMode End = =======================o*/
/*o===========================================================o*/


unsigned int Scaler_GetODLevel(unsigned int *level);
void Scaler_SetODReadEnable(unsigned char enable);
unsigned int Scaler_GetGammaNum(unsigned int *number);
void Scaler_Set_Software_Gamma(unsigned char gammaIndex);

unsigned int Scaler_Get_TV010_PQModule(TV010_PQ_MODULE_E pqModule,unsigned char * onOff);
void Scaler_Set_TV010_PQModule(TV010_PQ_MODULE_E pqModule,unsigned char onOff);



/*==================== Localdimming ================== */

unsigned int Scaler_GetCurrentPanelSupportLocaldimming(unsigned char *support);

void Scaler_SetLocalDimmingDemoMode(unsigned char mode);
unsigned int Scaler_GetLocalDimmingDemoMode(TV010_LOCALDIMMING_DEMO_MODE_E *demoMode);
void Scaler_SetLocalDimmingSpiEnable(unsigned char bEnable);


/*==================== other ================== */

void Scaler_SetDCCProcess(void);

unsigned char Scaler_Get_LD_Table(void);
void Scaler_Set_LD_Demo_Round(unsigned char Num);
void Scaler_Set_LD_Demo_Mode(SKY_MW_LOCALDIMMING_DEMO_MODE_E mode);

void Scaler_SetDCC_Table_tv010(unsigned char dccTblIdx);

void Scaler_SetPictureMode_tv010(unsigned char level);
void Scaler_SetDCCMapping_tv010(unsigned char dccTblIdx);
void Scaler_SetGAMMAMapping_tv010(unsigned char nTblIdx);
void Scaler_SetCOLORTEMPMapping_tv010(unsigned char mode);
unsigned char Scaler_GetCOLORTEMPMapping_tv010(void);
bool Scaler_GetHorseLight(void);
bool LD_round_demo_OnlyOnce(void);
void Scaler_LD_Spi_Init(void);
//void Scaler_LD_Spi_UnInit(void);
#ifdef __cplusplus
}
#endif

#endif /* __SCALER_LIB_H__*/

