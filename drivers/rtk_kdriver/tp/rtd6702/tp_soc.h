#ifndef __TP_SOC_H__
#define __TP_SOC_H__

#define TP_SPEED_MHZ                              1000   /* defined by CRT spec */
#define TP_VDEC_SPEED_MHZ                         900    /* Vdec set pll bush clock */

#define TPO_CLK_INIT_PRE_DIV                      0x36
#define TPO_CLK_INIT_POST_DIV                     0x37

#define TPO_CLK_CIP_PRE_DIV                       40
#define TPO_CLK_CIP_POST_DIV                      40

#define REG_TPO_BYPASS_CLK_EXIST

#define SUPPORT_TP_PLL_DISPB
#endif
