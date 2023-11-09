#ifndef __PANEL_API_EXTERN_TV010_H__
#define __PANEL_API_EXTERN_TV010_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <generated/autoconf.h>

#ifdef CONFIG_CUSTOMER_TV010
#include <tvscalercontrol/panel/panelapi.h>

unsigned char Get_PANEL_LD_Enable(void);
unsigned char Get_PANEL_LD_SPIDecoderMcuInPower_Enable(void);
unsigned char Get_PANEL_LD_Alogorithm_Soc(void);
unsigned char Get_PANEL_LD_Partition_Amount(void);
void Panel_ResetScreenShiftPos(void);
void Panel_ScreenShift(void);

  typedef enum {
	  P_VBY1_2K1K = P_PANEL_TYPE_MAX+1,
  }PANEL_TYPE_EX;

  typedef enum {
        VBY_ONE_PANEL_LG_OLED_2K1K_120HZ_1S_4L = P_PANEL_CUSTOM_INDEX_MAX+1,
        P_M4_Default_2k1k_2lane,
        P_EPI_LG_55_4K2K,
        P_EPI_LG_LC490DGG_UHD60_12LANE,
        P_EPI_LG_LC550DGG_UHD60_12LANE,
        P_EPI_LG_LC550V16_UHD60_12LANE,
  }PANEL_CUSTOM_INDEX_EX;

#endif



#ifdef __cplusplus
}
#endif

#endif // __PANEL_API_H__
