/* hdr10+ header */
#ifndef  __SIP_H__
#define  __SIP_H__

#ifdef __cplusplus
extern "C" {
#endif


/* some include about scaler*/
#include <scaler/vipCommon.h>

#define VIP_SIP_1_Size_default 0x18000
#define VIP_SIP_2_Size_default 0x6F00
typedef struct _VIP_SIP_DMA_CTRL {
	unsigned int size;
	unsigned int phy_addr_align;
	unsigned int *pVir_addr_align;

} VIP_SIP_DMA_CTRL;

void drvif_color_VPQ_SIP_TOP_CLK(void);
void drvif_color_VPQ_SIP_1_init(unsigned short h_sta, unsigned short v_sta, unsigned short h_size, unsigned short v_size, unsigned int addr, unsigned int tbl_len);
void drvif_color_VPQ_SIP_2_init(unsigned short h_sta, unsigned short v_sta, unsigned short h_size, unsigned short v_size, unsigned int addr, unsigned int tbl_len);
char drvif_color_Set_SIP_REG(unsigned int  *pTBL, unsigned int sta_addr, unsigned int size);
char drvif_color_Get_SIP_REG(unsigned int  *pTBL, unsigned int sta_addr, unsigned int size);
char drvif_color_reset_SIP1_DMA(void);
char drvif_color_set_SIP1_DMA(unsigned char LUT_Ctrl, unsigned int *pArray, unsigned int addr, unsigned int* vir_addr, unsigned int flush_size);
char drvif_color_SIP1_ERR_Check_DMA(unsigned char showMSG);
char drvif_color_SIP1_apply_done_Check_DMA(unsigned char showMSG);

char drvif_color_reset_SIP2_DMA(void);
char drvif_color_set_SIP2_DMA(unsigned char LUT_Ctrl, unsigned int *pArray, unsigned int addr, unsigned int* vir_addr, unsigned int flush_size);
char drvif_color_SIP2_ERR_Check_DMA(unsigned char showMSG);
char drvif_color_SIP2_apply_done_Check_DMA(unsigned char showMSG);

#ifdef __cplusplus
}
#endif

#endif // __SIP_H__


