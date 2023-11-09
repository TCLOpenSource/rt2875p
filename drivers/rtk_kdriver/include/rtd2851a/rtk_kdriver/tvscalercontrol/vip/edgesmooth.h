#ifndef __VIP_EDGESMOOTH_H__
#define __VIP_EDGESMOOTH_H__

#include <scaler/vipCommon.h>	/* in common*/
#include <scaler/vipRPCCommon.h>	/* in common*/

/* I Edge smooth*/
void drvif_color_iEdgeSmooth_en(unsigned char enable);
void drvif_color_iEdgeSmooth_init(void);
void drvif_color_iEdgeSmooth_chaos_set(unsigned char level);
void drvif_color_iEdgeSmooth_t2df_set(unsigned char level);
void drvif_color_iEdgeSmooth_edge_set(unsigned char level);
void drvif_color_iEdgeSmooth_crct_set(unsigned char level);
void drvif_color_iEdgeSmooth_extension_set(unsigned char level);
void drvif_color_Mixer_ctrl_iedge(unsigned char channel, int mixer_weight);
void drvif_color_DRV_IESM_Basic(DRV_IESM_Basic *ptr);
void drvif_color_DRV_IESM_Additional_setting(DRV_IESM_Additional_setting *ptr);

#endif // end of #ifndef __VIP_NR_H__
