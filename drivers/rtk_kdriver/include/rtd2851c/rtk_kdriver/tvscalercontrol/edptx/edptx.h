#ifndef __EDPTX_H__
#define __EDPTX_H__
/********************************************************************************/
/*   The  Software  is  proprietary,  confidential,  and  valuable to Realtek   */
/*   Semiconductor  Corporation  ("Realtek").  All  rights, including but not   */
/*   limited  to  copyrights,  patents,  trademarks, trade secrets, mask work   */
/*   rights, and other similar rights and interests, are reserved to Realtek.   */
/*   Without  prior  written  consent  from  Realtek,  copying, reproduction,   */
/*   modification,  distribution,  or  otherwise  is strictly prohibited. The   */
/*   Software  shall  be  kept  strictly  in  confidence,  and  shall  not be   */
/*   disclosed to or otherwise accessed by any third party.                     */
/*   c<2003> - <2012>                                                           */
/*   The Software is provided "AS IS" without any warranty of any kind,         */
/*   express, implied, statutory or otherwise.                                  */
/********************************************************************************/
#include <mach/rtk_log.h>

//****************************************************************************
// STRUCT / TYPE / ENUM DEFINITTIONS
//****************************************************************************

//****************************************************************************
// DEFINITIONS / MACROS
//****************************************************************************
#define TAG_NAME "EDPTX"

#define ErrorMessageEDPTx(format, args...) rtd_printk(KERN_EMERG, TAG_NAME, format, ## args)
#define NoteMessageEDPTx(format, args...) rtd_printk(KERN_NOTICE, TAG_NAME, format, ## args)
#define InfoMessageEDPTx(format, args...) rtd_printk(KERN_INFO, TAG_NAME, format, ## args)
#define DebugMessageEDPTx(format, args...)  rtd_printk(KERN_DEBUG, TAG_NAME, format, ## args)

//****************************************************************************
// FUNCTION EXTERN
//****************************************************************************
int EDPTX_state_handler(void *data);
int EDPTX_state_set_targetPanelPwrSeq(EnumPanelPowerSeq state);

#endif // #ifndef __EDPTX_H__

