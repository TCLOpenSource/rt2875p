/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version $Revision$
 */

/**
 * @addtogroup color
 * @{
 */

#ifndef _HCIC_H
#define _HCIC_H
/*============================ Module dependency  ===========================*/

/*===================================  Types ================================*/



/*================================== Variables ==============================*/
/*================================ Definitions ==============================*/

/*================================== Function ===============================*/
/*============================================================================*/
void drvif_color_set_fcic_enable(unsigned char bCtrl);
void drvif_color_set_fcic_table(unsigned int *pFCICtbl, unsigned int uFCICtbl_length);
void drvif_color_set_fcic_ctrl(unsigned char *u8pControlTbl, unsigned int u32Ctrlsize);
int drvif_color_check_fcic_table(unsigned int *pFCICtbl, unsigned int uFCICtbl_length);
void drvif_color_enable_FCIC_CLK(void);
void drvif_color_disable_FCIC_CLK(void);


/*======================== End of File =======================================*/

#endif
