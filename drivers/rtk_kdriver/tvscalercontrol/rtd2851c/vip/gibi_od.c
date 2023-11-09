/*==========================================================================
  * Copyright (c)      Realtek Semiconductor Corporation, 2009
  * All rights reserved.
  * ========================================================================*/

/*================= File Description =======================================*/
/**
 * @file
 * 	This file is for color related functions.
 *
 * @author 	$Author$
 * @date 	$Date$
 * @version 	$Revision$
 */

/**
 * @addtogroup color
 * @{
 */

/*============================ Module dependency  ===========================*/
#ifndef BUILD_QUICK_SHOW

#include <linux/mtd/mtd.h>
#include <linux/uaccess.h>
#include <asm/cacheflush.h>
#include <linux/pageremap.h>
#include <uapi/linux/const.h>
#include <linux/mm.h>
#include <linux/auth.h>
#include <linux/semaphore.h>//for semaphore
#endif
#include <tvscalercontrol/io/ioregdrv.h>
#include <tvscalercontrol/scaler/scalerstruct.h>
#include <tvscalercontrol/scalerdrv/mode.h>
#include <tvscalercontrol/scalerdrv/scalermemory.h>
#include <tvscalercontrol/scaler/source.h>
#include <tvscalercontrol/scalerdrv/scalerdrv.h>
#include <tvscalercontrol/vip/vip_reg_def.h>
#include <tvscalercontrol/vip/gibi_od.h>
#include <tvscalercontrol/scaler/scalercolorlib.h>
#include <tvscalercontrol/scalerdrv/scalerdisplay.h>
#include <rtd_log/rtd_module_log.h>
#include <rbus/pcid_reg.h>
#include <rbus/od_dma_reg.h>
#include "rtk_vip_logger.h"


#define OD_TABLE_LEN	17

/*#ifdef OPEN_CELL_PANEL*/
extern unsigned int *ODtable_Coeff;    /*young vippanel*/
/*#endif*/

/*===================================  Types ================================*/

/*================================== Variables ==============================*/
unsigned char bODMemCheck = 0;

/*================================ Definitions ==============================*/
#undef VIPprintf
#define VIPprintf(fmt, args...)	VIPprintfPrivate(VIP_LOGLEVEL_DEBUG,VIP_LOGMODULE_VIP_GIBI_OD_DEBUG,fmt,##args)

/*================================== Functions ===============================*/
void drvif_color_od_for_demo()
{
	//unsigned int memory_address;

	VIPprintf("[OD] Gibi_od.cpp drvif_color_od_for_demo() calling...\n");

	/* Od table setting*/
	IoReg_Write32(0xb802ca08, 0x00053000); /* write all channel, output value*/

	IoReg_Write32(0xb802ca0c, 0x00000909);
	IoReg_Write32(0xb802ca0c, 0x12111C1B);
	IoReg_Write32(0xb802ca0c, 0x2524312F);
	IoReg_Write32(0xb802ca0c, 0x3C3A4745);
	IoReg_Write32(0xb802ca0c, 0x52505D5B);
	IoReg_Write32(0xb802ca0c, 0x67657270);
	IoReg_Write32(0xb802ca0c, 0x7D7B8886);
	IoReg_Write32(0xb802ca0c, 0x93919E9C);
	IoReg_Write32(0xb802ca0c, 0xA8A6B1AF);
	IoReg_Write32(0xb802ca0c, 0xB9B7C2C0);
	IoReg_Write32(0xb802ca0c, 0xCBC9D0CE);
	IoReg_Write32(0xb802ca0c, 0xD4D3DBDA);
	IoReg_Write32(0xb802ca0c, 0xE1E0E6E5);
	IoReg_Write32(0xb802ca0c, 0xEAE9EEEE);
	IoReg_Write32(0xb802ca0c, 0xF2F2F7F7);
	IoReg_Write32(0xb802ca0c, 0xFCFCFEFE);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000807);
	IoReg_Write32(0xb802ca0c, 0x100E1A18);
	IoReg_Write32(0xb802ca0c, 0x23222E2D);
	IoReg_Write32(0xb802ca0c, 0x38374342);
	IoReg_Write32(0xb802ca0c, 0x4E4C5957);
	IoReg_Write32(0xb802ca0c, 0x63616E6C);
	IoReg_Write32(0xb802ca0c, 0x79778482);
	IoReg_Write32(0xb802ca0c, 0x8F8D9A98);
	IoReg_Write32(0xb802ca0c, 0xA4A2ACAA);
	IoReg_Write32(0xb802ca0c, 0xB4B2BEBC);
	IoReg_Write32(0xb802ca0c, 0xC7C5CCCB);
	IoReg_Write32(0xb802ca0c, 0xD1D0D8D7);
	IoReg_Write32(0xb802ca0c, 0xDFDEE4E3);
	IoReg_Write32(0xb802ca0c, 0xE8E8EDEC);
	IoReg_Write32(0xb802ca0c, 0xF1F0F6F6);
	IoReg_Write32(0xb802ca0c, 0xFBFBFDFD);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000605);
	IoReg_Write32(0xb802ca0c, 0x0B091614);
	IoReg_Write32(0xb802ca0c, 0x201E2B29);
	IoReg_Write32(0xb802ca0c, 0x3533403E);
	IoReg_Write32(0xb802ca0c, 0x4A485452);
	IoReg_Write32(0xb802ca0c, 0x5E5C6967);
	IoReg_Write32(0xb802ca0c, 0x74727F7D);
	IoReg_Write32(0xb802ca0c, 0x8A889593);
	IoReg_Write32(0xb802ca0c, 0x9F9DA8A6);
	IoReg_Write32(0xb802ca0c, 0xB0AEBAB8);
	IoReg_Write32(0xb802ca0c, 0xC3C1C9C7);
	IoReg_Write32(0xb802ca0c, 0xCECDD6D5);
	IoReg_Write32(0xb802ca0c, 0xDDDCE2E1);
	IoReg_Write32(0xb802ca0c, 0xE7E6EBEB);
	IoReg_Write32(0xb802ca0c, 0xEFEFF5F5);
	IoReg_Write32(0xb802ca0c, 0xFAFAFDFD);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000403);
	IoReg_Write32(0xb802ca0c, 0x0705110F);
	IoReg_Write32(0xb802ca0c, 0x1B192624);
	IoReg_Write32(0xb802ca0c, 0x302E3B39);
	IoReg_Write32(0xb802ca0c, 0x4543504E);
	IoReg_Write32(0xb802ca0c, 0x5A586563);
	IoReg_Write32(0xb802ca0c, 0x6F6D7A78);
	IoReg_Write32(0xb802ca0c, 0x8583908E);
	IoReg_Write32(0xb802ca0c, 0x9A98A3A1);
	IoReg_Write32(0xb802ca0c, 0xACAAB6B4);
	IoReg_Write32(0xb802ca0c, 0xBFBDC5C4);
	IoReg_Write32(0xb802ca0c, 0xCBCAD3D2);
	IoReg_Write32(0xb802ca0c, 0xDBDAE0DF);
	IoReg_Write32(0xb802ca0c, 0xE5E4EAE9);
	IoReg_Write32(0xb802ca0c, 0xEFEEF4F4);
	IoReg_Write32(0xb802ca0c, 0xF9F9FCFC);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000201);
	IoReg_Write32(0xb802ca0c, 0x03020D0B);
	IoReg_Write32(0xb802ca0c, 0x1614211E);
	IoReg_Write32(0xb802ca0c, 0x2B283633);
	IoReg_Write32(0xb802ca0c, 0x403E4B49);
	IoReg_Write32(0xb802ca0c, 0x5653615E);
	IoReg_Write32(0xb802ca0c, 0x6B697674);
	IoReg_Write32(0xb802ca0c, 0x817F8C8A);
	IoReg_Write32(0xb802ca0c, 0x96949F9D);
	IoReg_Write32(0xb802ca0c, 0xA8A6B2B0);
	IoReg_Write32(0xb802ca0c, 0xBBB9C2C0);
	IoReg_Write32(0xb802ca0c, 0xC8C7D0CF);
	IoReg_Write32(0xb802ca0c, 0xD8D7DEDD);
	IoReg_Write32(0xb802ca0c, 0xE3E2E8E8);
	IoReg_Write32(0xb802ca0c, 0xEDEDF3F3);
	IoReg_Write32(0xb802ca0c, 0xF9F9FCFC);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000908);
	IoReg_Write32(0xb802ca0c, 0x110F1B19);
	IoReg_Write32(0xb802ca0c, 0x2523302E);
	IoReg_Write32(0xb802ca0c, 0x3B384643);
	IoReg_Write32(0xb802ca0c, 0x504E5B59);
	IoReg_Write32(0xb802ca0c, 0x6663716E);
	IoReg_Write32(0xb802ca0c, 0x7C798784);
	IoReg_Write32(0xb802ca0c, 0x918F9A98);
	IoReg_Write32(0xb802ca0c, 0xA3A1ADAB);
	IoReg_Write32(0xb802ca0c, 0xB7B5BEBD);
	IoReg_Write32(0xb802ca0c, 0xC5C4CECD);
	IoReg_Write32(0xb802ca0c, 0xD6D5DCDB);
	IoReg_Write32(0xb802ca0c, 0xE1E0E7E6);
	IoReg_Write32(0xb802ca0c, 0xECECF2F2);
	IoReg_Write32(0xb802ca0c, 0xF8F8FCFC);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000605);
	IoReg_Write32(0xb802ca0c, 0x0C0A1614);
	IoReg_Write32(0xb802ca0c, 0x201E2B29);
	IoReg_Write32(0xb802ca0c, 0x3533403E);
	IoReg_Write32(0xb802ca0c, 0x4B485653);
	IoReg_Write32(0xb802ca0c, 0x605D6B68);
	IoReg_Write32(0xb802ca0c, 0x7673817E);
	IoReg_Write32(0xb802ca0c, 0x8C899693);
	IoReg_Write32(0xb802ca0c, 0x9F9DA9A7);
	IoReg_Write32(0xb802ca0c, 0xB2B0BBB9);
	IoReg_Write32(0xb802ca0c, 0xC3C1CBCA);
	IoReg_Write32(0xb802ca0c, 0xD3D2D9D9);
	IoReg_Write32(0xb802ca0c, 0xDFDFE5E5);
	IoReg_Write32(0xb802ca0c, 0xEBEBF1F1);
	IoReg_Write32(0xb802ca0c, 0xF7F7FBFB);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000403);
	IoReg_Write32(0xb802ca0c, 0x0705110F);
	IoReg_Write32(0xb802ca0c, 0x1B192624);
	IoReg_Write32(0xb802ca0c, 0x302E3B39);
	IoReg_Write32(0xb802ca0c, 0x4543504E);
	IoReg_Write32(0xb802ca0c, 0x5A586563);
	IoReg_Write32(0xb802ca0c, 0x706D7B78);
	IoReg_Write32(0xb802ca0c, 0x8683908E);
	IoReg_Write32(0xb802ca0c, 0x9A98A4A2);
	IoReg_Write32(0xb802ca0c, 0xAEACB7B5);
	IoReg_Write32(0xb802ca0c, 0xBFBEC8C7);
	IoReg_Write32(0xb802ca0c, 0xD0CFD7D6);
	IoReg_Write32(0xb802ca0c, 0xDEDDE4E4);
	IoReg_Write32(0xb802ca0c, 0xEAEAF1F1);
	IoReg_Write32(0xb802ca0c, 0xF7F7FBFB);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000101);
	IoReg_Write32(0xb802ca0c, 0x02010C0B);
	IoReg_Write32(0xb802ca0c, 0x1614211F);
	IoReg_Write32(0xb802ca0c, 0x2B293634);
	IoReg_Write32(0xb802ca0c, 0x403E4B48);
	IoReg_Write32(0xb802ca0c, 0x55525F5C);
	IoReg_Write32(0xb802ca0c, 0x69667572);
	IoReg_Write32(0xb802ca0c, 0x807D8B88);
	IoReg_Write32(0xb802ca0c, 0x9593A09E);
	IoReg_Write32(0xb802ca0c, 0xAAA8B3B1);
	IoReg_Write32(0xb802ca0c, 0xBCBAC5C4);
	IoReg_Write32(0xb802ca0c, 0xCECDD5D4);
	IoReg_Write32(0xb802ca0c, 0xDCDBE3E2);
	IoReg_Write32(0xb802ca0c, 0xE9E8F0EF);
	IoReg_Write32(0xb802ca0c, 0xF6F6FBFB);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000908);
	IoReg_Write32(0xb802ca0c, 0x12101C1A);
	IoReg_Write32(0xb802ca0c, 0x2624312F);
	IoReg_Write32(0xb802ca0c, 0x3B394543);
	IoReg_Write32(0xb802ca0c, 0x4F4D5957);
	IoReg_Write32(0xb802ca0c, 0x63606F6C);
	IoReg_Write32(0xb802ca0c, 0x7A788583);
	IoReg_Write32(0xb802ca0c, 0x908D9B98);
	IoReg_Write32(0xb802ca0c, 0xA5A3AFAD);
	IoReg_Write32(0xb802ca0c, 0xB8B7C2C1);
	IoReg_Write32(0xb802ca0c, 0xCBCAD3D2);
	IoReg_Write32(0xb802ca0c, 0xDAD9E1E0);
	IoReg_Write32(0xb802ca0c, 0xE7E7EEEE);
	IoReg_Write32(0xb802ca0c, 0xF5F5FAFA);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000706);
	IoReg_Write32(0xb802ca0c, 0x0D0B1715);
	IoReg_Write32(0xb802ca0c, 0x211F2C2A);
	IoReg_Write32(0xb802ca0c, 0x3634403E);
	IoReg_Write32(0xb802ca0c, 0x4A485452);
	IoReg_Write32(0xb802ca0c, 0x5D5B6967);
	IoReg_Write32(0xb802ca0c, 0x75737F7D);
	IoReg_Write32(0xb802ca0c, 0x89879592);
	IoReg_Write32(0xb802ca0c, 0xA09DABA8);
	IoReg_Write32(0xb802ca0c, 0xB5B3BFBD);
	IoReg_Write32(0xb802ca0c, 0xC8C7D0CF);
	IoReg_Write32(0xb802ca0c, 0xD8D7DFDF);
	IoReg_Write32(0xb802ca0c, 0xE6E6EEEE);
	IoReg_Write32(0xb802ca0c, 0xF5F5FAFA);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000403);
	IoReg_Write32(0xb802ca0c, 0x08061210);
	IoReg_Write32(0xb802ca0c, 0x1C1A2725);
	IoReg_Write32(0xb802ca0c, 0x312F3C3A);
	IoReg_Write32(0xb802ca0c, 0x46444F4D);
	IoReg_Write32(0xb802ca0c, 0x58566462);
	IoReg_Write32(0xb802ca0c, 0x706E7A78);
	IoReg_Write32(0xb802ca0c, 0x84818F8C);
	IoReg_Write32(0xb802ca0c, 0x9996A5A2);
	IoReg_Write32(0xb802ca0c, 0xB0ADBBB8);
	IoReg_Write32(0xb802ca0c, 0xC5C3CECC);
	IoReg_Write32(0xb802ca0c, 0xD6D5DEDD);
	IoReg_Write32(0xb802ca0c, 0xE5E4EDEC);
	IoReg_Write32(0xb802ca0c, 0xF4F4FAFA);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000201);
	IoReg_Write32(0xb802ca0c, 0x03020D0C);
	IoReg_Write32(0xb802ca0c, 0x17152220);
	IoReg_Write32(0xb802ca0c, 0x2C2A3735);
	IoReg_Write32(0xb802ca0c, 0x413F4A48);
	IoReg_Write32(0xb802ca0c, 0x53505F5D);
	IoReg_Write32(0xb802ca0c, 0x6B697573);
	IoReg_Write32(0xb802ca0c, 0x7E7C8986);
	IoReg_Write32(0xb802ca0c, 0x93909E9B);
	IoReg_Write32(0xb802ca0c, 0xA9A6B5B2);
	IoReg_Write32(0xb802ca0c, 0xC0BDCAC8);
	IoReg_Write32(0xb802ca0c, 0xD3D2DBDB);
	IoReg_Write32(0xb802ca0c, 0xE3E3EBEB);
	IoReg_Write32(0xb802ca0c, 0xF3F3F9F9);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000A09);
	IoReg_Write32(0xb802ca0c, 0x13111E1C);
	IoReg_Write32(0xb802ca0c, 0x28263230);
	IoReg_Write32(0xb802ca0c, 0x3C3A4543);
	IoReg_Write32(0xb802ca0c, 0x4D4B5A58);
	IoReg_Write32(0xb802ca0c, 0x6664706E);
	IoReg_Write32(0xb802ca0c, 0x79778381);
	IoReg_Write32(0xb802ca0c, 0x8D8B9896);
	IoReg_Write32(0xb802ca0c, 0xA3A1AFAC);
	IoReg_Write32(0xb802ca0c, 0xBAB7C5C2);
	IoReg_Write32(0xb802ca0c, 0xD0CDD9D7);
	IoReg_Write32(0xb802ca0c, 0xE2E1EAEA);
	IoReg_Write32(0xb802ca0c, 0xF2F2F9F9);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000706);
	IoReg_Write32(0xb802ca0c, 0x0E0C1917);
	IoReg_Write32(0xb802ca0c, 0x23212D2B);
	IoReg_Write32(0xb802ca0c, 0x3735403E);
	IoReg_Write32(0xb802ca0c, 0x49465553);
	IoReg_Write32(0xb802ca0c, 0x615F6B69);
	IoReg_Write32(0xb802ca0c, 0x74727E7C);
	IoReg_Write32(0xb802ca0c, 0x88859390);
	IoReg_Write32(0xb802ca0c, 0x9E9BA9A6);
	IoReg_Write32(0xb802ca0c, 0xB4B1BFBC);
	IoReg_Write32(0xb802ca0c, 0xCAC7D5D2);
	IoReg_Write32(0xb802ca0c, 0xE0DDE9E7);
	IoReg_Write32(0xb802ca0c, 0xF1F1F8F8);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000504);
	IoReg_Write32(0xb802ca0c, 0x09071412);
	IoReg_Write32(0xb802ca0c, 0x1E1C2927);
	IoReg_Write32(0xb802ca0c, 0x33313B39);
	IoReg_Write32(0xb802ca0c, 0x4341504E);
	IoReg_Write32(0xb802ca0c, 0x5D5B6664);
	IoReg_Write32(0xb802ca0c, 0x6F6D7976);
	IoReg_Write32(0xb802ca0c, 0x827F8D8A);
	IoReg_Write32(0xb802ca0c, 0x9895A3A0);
	IoReg_Write32(0xb802ca0c, 0xAEABB9B6);
	IoReg_Write32(0xb802ca0c, 0xC4C1CFCC);
	IoReg_Write32(0xb802ca0c, 0xDAD7E5E3);
	IoReg_Write32(0xb802ca0c, 0xF0EFF8F7);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000000);
	IoReg_Write32(0xb802ca0c, 0x00000202);
	IoReg_Write32(0xb802ca0c, 0x04040F0F);
	IoReg_Write32(0xb802ca0c, 0x19192424);
	IoReg_Write32(0xb802ca0c, 0x2E2E3737);
	IoReg_Write32(0xb802ca0c, 0x3F3F4C4C);
	IoReg_Write32(0xb802ca0c, 0x58586161);
	IoReg_Write32(0xb802ca0c, 0x6A6A7373);
	IoReg_Write32(0xb802ca0c, 0x7C7C8787);
	IoReg_Write32(0xb802ca0c, 0x92929D9D);
	IoReg_Write32(0xb802ca0c, 0xA8A8B3B3);
	IoReg_Write32(0xb802ca0c, 0xBEBEC9C9);
	IoReg_Write32(0xb802ca0c, 0xD4D4E1E1);
	IoReg_Write32(0xb802ca0c, 0xEDEDF6F6);
	IoReg_Write32(0xb802ca0c, 0xFFFFFFFF);

	IoReg_Write32(0xb802ca08, 0x00000000);  /* disable table access*/
}

void drvif_color_od_load_table(UINT32 *input_array)
{
	/*	VIPprintf("[OD] load table testing\n", input_array[0]);*/
	/*	VIPprintf("[OD] load table testing\n", input_array[1]);*/
	/*	VIPprintf("[OD] load table testing\n", input_array[2]);*/
	/*	VIPprintf("[OD] load table testing\n", input_array[3]);*/
}

/* default y mode */
#ifndef BUILD_QUICK_SHOW

void drvif_color_set_od_Ymode(void)
{
	// 170524 henry Merlin3
	od_ymode_ctrl_RBUS od_ymode_ctrl_reg;
	od_ymode_ctrl_reg.regValue = IoReg_Read32(OD_YMODE_CTRL_reg);
	od_ymode_ctrl_reg.od_ymode_en = 1;
	od_ymode_ctrl_reg.od_ymode_sat_en = 0;
	od_ymode_ctrl_reg.r_coeff = 0x4D;
	od_ymode_ctrl_reg.g_coeff = 0x96;
	od_ymode_ctrl_reg.b_coeff = 0x1D;
	IoReg_Write32(OD_YMODE_CTRL_reg, od_ymode_ctrl_reg.regValue);
}
#endif
unsigned char drvif_color_od_calc_max_bit(unsigned short width, unsigned short height, unsigned char dma_mode, unsigned int mem_size/*byte*/)
{
	unsigned char max_bit;

	if (width == 0 || height == 0 || mem_size < 4)
		return 0;
	
	if (dma_mode == 0) { //line_mode
		max_bit = ((((mem_size<<3)-0x80)/(height+2))-256)/width;
	} else { //frame mode
		max_bit = ((mem_size<<2)-0x40)/(width*height);
	}

	return max_bit;
}
// 160615 added by henry
// 160805 modified by henry
// 160810 modified by henry - add the static memory address
// 170524 modified by henry - Merlin3 driver
//
// === Argument Explanation ===
//
// width & height: 4K - 3840x2160, 2K - 1920x1080
//
// bit_sel: dma input bit depth = 0 : 6 bits
//   						 	= 1 : 8 bits
//   						 	= 2 : 10 bits
//
// dma_mode = 1: frame mode
//            0: line mode
//
// FrameLimitBit: compression ratio
//
// dataColor = 0: RGB Compression
//			 = 1: YUV Compression
//
// dataFormat = 0: 444
//			  = 1: 422
//			  = 2: 420 --> not support RGB compression
//			  = 3: 400 --> not support RGB compression
//
// === Example of using OD PQ compression function ===
//
// Video Resolution   : 4K
// bit_sel            : 6
// PQC Mode           : line mode
// FrameLimitBit      : 8
// Compression Color  : RGB
// Compression Format : 422
//
// drvif_color_od_pqc(2160, 3840, 0, 0, 8, 0, 1)
//
#ifndef BUILD_QUICK_SHOW

bool drvif_color_od_pqc(UINT16 height, UINT16 width, UINT8 bit_sel, UINT8 dma_mode, UINT8 FrameLimitBit, int dataColor, int dataFormat)
{

	unsigned long od_size = 0;
	unsigned long od_addr = 0;
	unsigned int burst_length = 0x40;  //0x20;
	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_od_dma_ctrl_2_RBUS od_dma_od_dma_ctrl_2_reg;
	od_dma_oddma_pq_cmp_allocate_RBUS od_dma_oddma_pq_cmp_allocate_reg;
	od_dma_oddma_pq_cmp_blk0_add0_RBUS od_dma_oddma_pq_cmp_blk0_add0_reg;
	od_dma_oddma_pq_cmp_blk0_add1_RBUS od_dma_oddma_pq_cmp_blk0_add1_reg;
	od_dma_oddma_pq_cmp_RBUS oddma_pq_cmp_reg;
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	od_dma_oddma_pq_decmp_RBUS oddma_pq_decmp_reg;
	od_dma_oddma_pq_decmp_sat_en_RBUS oddma_pq_decmp_sat_en_reg;
	od_dma_oddma_wr_num_bl_wrap_ctl_RBUS od_dma_oddma_wr_num_bl_wrap_ctl_reg;
	od_dma_oddma_rd_num_bl_wrap_ctl_RBUS od_dma_oddma_rd_num_bl_wrap_ctl_reg;
	od_dma_oddma_wr_num_bl_wrap_rolling_RBUS od_dma_oddma_wr_num_bl_wrap_rolling_reg;
	od_dma_oddma_rd_num_bl_wrap_rolling_RBUS od_dma_oddma_rd_num_bl_wrap_rolling_reg;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS od_dma_oddma_wr_num_bl_wrap_word_reg;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS od_dma_oddma_rd_num_bl_wrap_word_reg;
	od_dma_oddma_pq_cmp_st_RBUS od_dma_oddma_pq_cmp_st_reg;
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS od_dma_oddma_cap_linestep_reg;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS od_dma_oddma_rd_num_bl_wrap_line_step_reg;
	od_dma_oddma_pq_cmp_enable_RBUS od_dma_oddma_pq_cmp_enable_reg;
	od_dma_oddma_pq_cmp_pair_RBUS oddma_pq_cmp_pair_reg;
	od_dma_oddma_pq_decmp_pair_RBUS oddma_pq_decmp_pair_reg;
	od_dma_oddma_pqc_pqdc_compensation_RBUS od_dma_oddma_pqc_pqdc_compensation_reg;
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;
	od_dma_oddma_pqc_pqdc_compensation_RBUS od_dma_oddma_pqc_decmp_compensation_reg;


	VIPprintf("[OD] Gibi_od.cpp::drvif_color_od_pqc() calling...\n");

#ifdef CONFIG_OPTEE_SECURE_SVP_PROTECTION
	od_size = carvedout_buf_query_secure(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#else
	od_size = carvedout_buf_query(CARVEDOUT_SCALER_OD,(void*) &od_addr);
#endif
	if (od_addr == 0 || od_size == 0) {
		VIPprintf("[%s %d] ERR : %lx %lx\n", __func__, __LINE__, od_addr, od_size);
		return false;
	}

	// cmp, decmp pair setting
	//
	// dataFormat = 0: 444
	//			  = 1: 422
	//			  = 2: 420 --> not support RGB compression
	//			  = 3: 400 --> not support RGB compression
	//
	// dataColor = 0: RGB Compression
	//			 = 1: YUV Compression
	//
	if (dataColor == 0 && (dataFormat == 2 || dataFormat == 3))
	{
		VIPprintf("[OD] Gibi_od.cpp::input value conflict...\n");
		VIPprintf("420 or 400 doesn't support RGB compression\n");
		return false;
	}

	/* Merlin3 default on, henry
	// comp decomp clk enable
	OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg od_dma_oddma_pqc_pqdc_compensation_reg;
	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.comp_decomp_clk_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);
	*/

	// b802cdc0 dma ctrl
	od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
	od_dma_od_dma_ctrl_reg.bit_sel = bit_sel;
	od_dma_od_dma_ctrl_reg.oddma_mode_sel = dma_mode;
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg, od_dma_od_dma_ctrl_reg.regValue);

	// b802cdb8 vact and hact
	od_dma_od_dma_ctrl_2_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_2_reg);
	od_dma_od_dma_ctrl_2_reg.hact = width;
	od_dma_od_dma_ctrl_2_reg.vact = height;
	IoReg_Write32(OD_DMA_OD_DMA_CTRL_2_reg, od_dma_od_dma_ctrl_2_reg.regValue);

	// b802cd14 PQC constraint
	od_dma_oddma_pq_cmp_allocate_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg);
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
	od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 12;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg, od_dma_oddma_pq_cmp_allocate_reg.regValue);

	// b802cd60 PQC constraint
	od_dma_oddma_pq_cmp_blk0_add0_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BLK0_ADD0_reg);
	od_dma_oddma_pq_cmp_blk0_add1_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BLK0_ADD1_reg);
	od_dma_oddma_pq_cmp_blk0_add0_reg.blk0_add_value32 = 1;
	od_dma_oddma_pq_cmp_blk0_add0_reg.blk0_add_value16 = 1;
	od_dma_oddma_pq_cmp_blk0_add1_reg.blk0_add_value8 = 1;
	od_dma_oddma_pq_cmp_blk0_add1_reg.blk0_add_value4 = 1;
	od_dma_oddma_pq_cmp_blk0_add1_reg.blk0_add_value2 = 1;
	od_dma_oddma_pq_cmp_blk0_add1_reg.blk0_add_value1 = 0;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BLK0_ADD0_reg, od_dma_oddma_pq_cmp_blk0_add0_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BLK0_ADD1_reg, od_dma_oddma_pq_cmp_blk0_add1_reg.regValue);

	// b802CD00 PQ_CMP enable
	oddma_pq_cmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_reg);
	oddma_pq_cmp_reg.cmp_en = 1;
	oddma_pq_cmp_reg.cmp_width_div32 = width >> 5;
	oddma_pq_cmp_reg.cmp_height = height;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_reg, oddma_pq_cmp_reg.regValue);

	// b802cd08 Compression ratio
	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
	oddma_pq_cmp_bit_reg.frame_limit_bit = FrameLimitBit;
    oddma_pq_cmp_bit_reg.block_limit_bit = 0x3F;

	// DMA buffer line limit clamp
	if(dma_mode == 0)
		oddma_pq_cmp_bit_reg.line_limit_bit = oddma_pq_cmp_bit_reg.frame_limit_bit;
	else if(dma_mode == 1)
		oddma_pq_cmp_bit_reg.line_limit_bit = FrameLimitBit + 6;
	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BIT_reg, oddma_pq_cmp_bit_reg.regValue);

	// b802cd80 PQ_DECMP enable
	oddma_pq_decmp_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_reg);
	oddma_pq_decmp_reg.decmp_en = 1;
	oddma_pq_decmp_reg.decmp_width_div32 = width >> 5;
	oddma_pq_decmp_reg.decmp_height = height;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_reg, oddma_pq_decmp_reg.regValue);

	oddma_pq_cmp_pair_reg.regValue =  IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
	oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);

	oddma_pq_cmp_pair_reg.cmp_data_format = dataFormat;
	oddma_pq_decmp_pair_reg.decmp_data_format = dataFormat;
	oddma_pq_cmp_pair_reg.cmp_data_color = dataColor;
	oddma_pq_decmp_pair_reg.decmp_data_color = dataColor;
	oddma_pq_cmp_pair_reg.cmp_pair_para = 1;
	oddma_pq_decmp_pair_reg.decmp_pair_para = 1;

	if(FrameLimitBit <= 6) {
		oddma_pq_cmp_pair_reg.cmp_data_bit_width = 0;
		oddma_pq_decmp_pair_reg.decmp_data_bit_width = 0;
	} else {
		oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1;
		oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1;
	}

	if(dma_mode == 0) {
		oddma_pq_cmp_pair_reg.cmp_line_sum_bit = (width * oddma_pq_cmp_bit_reg.line_limit_bit + 256) / 128;
		oddma_pq_decmp_pair_reg.decmp_line_sum_bit = oddma_pq_cmp_pair_reg.cmp_line_sum_bit;
	}

	IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);

	// b802cd88 Saturation enable
	oddma_pq_decmp_sat_en_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg);
	oddma_pq_decmp_sat_en_reg.saturation_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_SAT_EN_reg, oddma_pq_decmp_sat_en_reg.regValue);

	// b802ce3c b802cebc cap disp line_num, burst_length
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg);
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_burst_len = burst_length;
	od_dma_oddma_wr_num_bl_wrap_ctl_reg.cap_line_num = height;
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_ctl_reg, od_dma_oddma_wr_num_bl_wrap_ctl_reg.regValue);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg);
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_burst_len = burst_length;
	od_dma_oddma_rd_num_bl_wrap_ctl_reg.disp_line_num = height;
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_ctl_reg, od_dma_oddma_rd_num_bl_wrap_ctl_reg.regValue);

	od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg);
	od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg);
	od_dma_oddma_wr_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
	od_dma_oddma_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);

	if(dma_mode == 0) { // LINE MODE
		// b802cd0c Merlin3 first_line_more_en = 0 when line mode
		od_dma_oddma_pq_cmp_enable_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg);
		od_dma_oddma_pq_cmp_enable_reg.fisrt_line_more_en = 0;
		//od_dma_oddma_pq_cmp_enable_reg.force_last2blk_pe_mode0 = 0; //mac7p pq compile fix
		od_dma_oddma_pq_cmp_enable_reg.guarantee_max_qp_en = 0;
		od_dma_oddma_pq_cmp_enable_reg.g_ratio = 14;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ENABLE_reg, od_dma_oddma_pq_cmp_enable_reg.regValue);

		// b802cd14 setting g_ratio_max g_ratio_min
		od_dma_oddma_pq_cmp_allocate_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg);
		od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_max = 16;
		od_dma_oddma_pq_cmp_allocate_reg.dynamic_allocate_ratio_min = 13;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ALLOCATE_reg, od_dma_oddma_pq_cmp_allocate_reg.regValue);

		// b802cd2c 10 bits 400 SW aggressive table
		od_dma_oddma_pq_cmp_st_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_ST_reg);
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_ST_reg, od_dma_oddma_pq_cmp_st_reg.regValue);

		// b802ce38 Cap LineStep
		od_dma_oddma_cap_linestep_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
		od_dma_oddma_cap_linestep_reg.cap_line_step = oddma_pq_cmp_pair_reg.cmp_line_sum_bit;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_cap_linestep_reg.regValue);

		// b802ceac Disp LineStep
		od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
		od_dma_oddma_rd_num_bl_wrap_line_step_reg.disp_line_step = oddma_pq_decmp_pair_reg.decmp_line_sum_bit;
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue);

		// b802ce44 cap_line_128_num
		od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = oddma_pq_cmp_pair_reg.cmp_line_sum_bit;

		// b802cec4 disp_line_128_num
		od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = oddma_pq_decmp_pair_reg.decmp_line_sum_bit;

		// b802ce40 b802cec0 cap disp line rolling space
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_space = height + 2;
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_space = height + 2;
		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);

		// b802ce40 b802cec0 cap disp line rolling enable
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = 1;
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = 1;

		// b802cd04 b802cd84 cmp decmp line mode en
		oddma_pq_cmp_pair_reg.regValue =  IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
		oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);
		oddma_pq_cmp_pair_reg.cmp_line_mode = 1;
		oddma_pq_decmp_pair_reg.decmp_line_mode = 1;
		oddma_pq_cmp_pair_reg.two_line_prediction_en = 1;
		oddma_pq_decmp_pair_reg.decmp_two_line_prediction_en = 1;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);
	}
	else if(dma_mode == 1) { // FRAME MODE
		// b802ce40 cap line rolling space and enable
		od_dma_oddma_wr_num_bl_wrap_rolling_reg.cap_rolling_enable = 0;

		// b802cec0 disp line rolling space and enable
		od_dma_oddma_rd_num_bl_wrap_rolling_reg.disp_rolling_enable = 0;

		// b802ce44 cap_line_128_num = 0xffff
		od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = 0xffff;

		// b802cec4 disp_line_128_num = 0xffff
		od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = 0xffff;
	}

	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_rolling_reg, od_dma_oddma_wr_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_rolling_reg, od_dma_oddma_rd_num_bl_wrap_rolling_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word_reg.regValue);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word_reg.regValue);

	// b802ce30 b802ce34 b802ced4 b802ced8 start address
	IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_addr_0_reg, ((unsigned int)od_addr) + burst_length);
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_up_reg, ((unsigned int)od_addr) + od_size - burst_length);
	IoReg_Write32(OD_DMA_ODDMA_WR_Rule_check_low_reg, (unsigned int)od_addr);
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_0_reg, ((unsigned int)od_addr) + burst_length);
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_up_reg, ((unsigned int)od_addr) + od_size - burst_length);
	IoReg_Write32(OD_DMA_ODDMA_RD_Rule_check_low_reg, (unsigned int)od_addr);

	// pqc pqdc bypass disable
	od_dma_oddma_pqc_pqdc_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_pqdc_compensation_reg.comp_de_comp_bypass = 0;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_pqdc_compensation_reg.regValue);

	// b802ce08 cap enable
	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_wr_ctrl_reg.cap_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl_reg.regValue);

	// b802ce88 disp enable
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.disp_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl_reg.regValue);

	od_dma_oddma_pqc_decmp_compensation_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg);
	od_dma_oddma_pqc_decmp_compensation_reg.tr_en = 1;
	IoReg_Write32(OD_DMA_ODDMA_PQC_PQDC_COMPENSATION_reg, od_dma_oddma_pqc_decmp_compensation_reg.regValue);
	// end henry

	VIPprintf("[OD] Gibi_od.cpp::drvif_color_od_pqc() finished...\n");

	return true;
}
#endif

#if 0
// RGBW OD
void drvif_color_rgbw_od(bool rgbw_en)
{
	od_od_rgbw_ctrl_RBUS od_od_rgbw_ctrl_reg;
	od_od_noise_reduction_ctrl_RBUS noise_reduction_ctrl_reg;
	rgbw_d_rgbw_spr_order_RBUS rgbw_d_rgbw_spr_order_reg;
	if(rgbw_en == 0)
		return;
	else
	{
		// input format == 1
		od_od_rgbw_ctrl_reg.regValue = IoReg_Read32(OD_OD_RGBW_CTRL_reg);
		rgbw_d_rgbw_spr_order_reg.regValue = IoReg_Read32(RGBW_D_RGBW_SPR_ORDER_reg);

		od_od_rgbw_ctrl_reg.od_input_format = 1;
		od_od_rgbw_ctrl_reg.even_ch_b = rgbw_d_rgbw_spr_order_reg.even_ch_b;
		od_od_rgbw_ctrl_reg.even_ch_g = rgbw_d_rgbw_spr_order_reg.even_ch_g;
		od_od_rgbw_ctrl_reg.even_ch_r = rgbw_d_rgbw_spr_order_reg.even_ch_r;
		od_od_rgbw_ctrl_reg.even_ch_w = rgbw_d_rgbw_spr_order_reg.even_ch_w;
		od_od_rgbw_ctrl_reg.odd_ch_b = rgbw_d_rgbw_spr_order_reg.odd_ch_b;
		od_od_rgbw_ctrl_reg.odd_ch_g = rgbw_d_rgbw_spr_order_reg.odd_ch_g;
		od_od_rgbw_ctrl_reg.odd_ch_r = rgbw_d_rgbw_spr_order_reg.odd_ch_r;
		od_od_rgbw_ctrl_reg.odd_ch_w = rgbw_d_rgbw_spr_order_reg.odd_ch_w;
		IoReg_Write32(OD_OD_RGBW_CTRL_reg, od_od_rgbw_ctrl_reg.regValue);

		// nr old mode only
		noise_reduction_ctrl_reg.regValue = IoReg_Read32(OD_od_Noise_reduction_ctrl_reg);
		noise_reduction_ctrl_reg.od_nr_mode = 0;
		IoReg_Write32(OD_od_Noise_reduction_ctrl_reg, noise_reduction_ctrl_reg.regValue);
	}
}
#endif

// OD IP enable
void drvif_color_od(unsigned char bOD)
{
	od_od_ctrl_RBUS od_ctrl_reg;
	od_od_ctrl2_RBUS od_ctrl2_reg;
	od_od_noise_reduction_ctrl_RBUS noise_reduction_ctrl_reg;


	od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	od_ctrl2_reg.od_delta_gain_b = 64;
	od_ctrl2_reg.od_delta_gain_g = 64;
	od_ctrl2_reg.od_delta_gain_r = 64;
	IoReg_Write32(OD_OD_CTRL2_reg, od_ctrl2_reg.regValue);


	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	if (bOD == 0) {
		//od_ctrl_reg.od_fun_sel = 0; //mac6p removed
		od_ctrl_reg.od_en = 0;
	} else {
		//od_ctrl_reg.od_fun_sel = 1; //mac6p removed
		od_ctrl_reg.od_en = 1;
	}
	IoReg_Write32(OD_OD_CTRL_reg, od_ctrl_reg.regValue);

	/* default nr*/
	noise_reduction_ctrl_reg.regValue = IoReg_Read32(OD_od_Noise_reduction_ctrl_reg);
	noise_reduction_ctrl_reg.od_nr_mode = 0;
	noise_reduction_ctrl_reg.od_b_nr_en = 1;
	noise_reduction_ctrl_reg.od_g_nr_en = 1;
	noise_reduction_ctrl_reg.od_r_nr_en = 1;
	noise_reduction_ctrl_reg.od_b_nr_thd = 3;
	noise_reduction_ctrl_reg.od_g_nr_thd = 3;
	noise_reduction_ctrl_reg.od_r_nr_thd = 3;
	IoReg_Write32(OD_od_Noise_reduction_ctrl_reg, noise_reduction_ctrl_reg.regValue);

}

// henry 170524
#if 0
#ifndef BUILD_QUICK_SHOW

void drvif_color_od_table_seperate(unsigned int *pODtable, unsigned char tableType)
{
	int i;//, count = 30;
	od_od_lut_addr_RBUS lut_addr_reg;

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.table_type = tableType;
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; //col(cur) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	for (i = 0; i < OD_table_length; i++) {
		IoReg_Write32(OD_OD_LUT_DATA_reg, *(pODtable+i));
	}
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, _BIT16);
}
#endif
#endif

unsigned int od_table_transformed[OD_table_length];
#ifndef BUILD_QUICK_SHOW
void drvif_color_od_table_write(unsigned int *pODtable, unsigned char tableType, char channel)
{
	int i;//, count = 30;
	od_od_lut_addr_RBUS lut_addr_reg;
	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.table_type = tableType;
	lut_addr_reg.od_lut_sel = channel;
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; //col(cur) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);
	//rtd_pr_vpq_emerg("drvif_color_od_table_write, channel = %d\n", channel);
	for (i = 0; i < OD_table_length; i++)
		od_table_transformed[i] = *(pODtable+i);

	for (i = 0; i < OD_table_length; i++) {
		IoReg_Write32(OD_OD_LUT_DATA_reg, od_table_transformed[i]);
		
		//rtd_pr_vpq_emerg("drvif_color_od_table_write, od_do_resume = %d\n", *(pODtable+i));
	}
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, _BIT16);
}
#endif

void drvif_color_od_table_read(unsigned char *pODtable, unsigned char channel, unsigned char DbusMode)
{
	od_od_lut_addr_RBUS lut_addr_reg;
	int i;
	unsigned int ODtable_tmp[OD_table_length];
	char ODtable_sign[OD_table_length];
	unsigned char remain;

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.od_lut_dma_read = DbusMode; //0: rbus, 1:dbus
	lut_addr_reg.od_lut_row_addr = 0;
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_sel = channel;
	lut_addr_reg.od_lut_ax_en = 1; // auto rolling index
	lut_addr_reg.od_lut_adr_mode = 0; //cur index first
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	// target table
	if(lut_addr_reg.table_type)
	{
		for (i = 0; i < OD_table_length; i++){
			RBus_UInt32 regValue_tmp = IoReg_Read32(OD_OD_LUT_DATA_reg);
			// 33 to 17 table
			//*(pODtable+i) = ( regValue_tmp >> 24);

			/* henry merlin3 OD table only 17x17 */
			*(pODtable+i) = regValue_tmp;
		}
	}
	// delta table
	else
	{

		for (i = 0; i < OD_table_length; i++) {
			ODtable_tmp[i] = IoReg_Read32(OD_OD_LUT_DATA_reg);

			remain = i%17;

			// 33 to 17 table
			//ODtable_sign[i] = (char)(ODtable_tmp[i] >> 24);

			/* henry merlin3 OD table only 17x17 */
			ODtable_sign[i] = (char)(ODtable_tmp[i]);

			// add cur pixel value (to target table)
			if(remain != 16)
				*(pODtable+i) = ODtable_sign[i] + 16 * remain;
			else
				*(pODtable+i) = ODtable_sign[i] + 255;

		}

	}
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, _BIT16);
}
// end henry

void drvif_color_od_table_set_dbus(unsigned int lutaddress, unsigned char tableType)
{
	//unsigned char tmp;
	od_od_lut_addr_RBUS lut_addr_reg;

	//tmp = drvif_color_od_enable_get_verify();
	//drvif_color_od_enable_set_verify(0);

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.od_lut_dma_read = 1; //0: rbus, 1:dbus
	lut_addr_reg.table_type = tableType; // 0: delta, 1: target
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; //col(cur) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);	
	
	// set dbus lut address
	IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_addr_table_reg, lutaddress);	

	/* disable table access*/
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, _BIT16);

}
#if 0
void drvif_color_od_table_seperate_read(unsigned char *pODtable, unsigned char channel)
{
	od_od_lut_addr_RBUS lut_addr_reg;
	int i;
	unsigned int ODtable_tmp[OD_table_length];
	char ODtable_sign[OD_table_length];
	unsigned char remain;

	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.od_lut_row_addr = 0;
	lut_addr_reg.table_type = 0;
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_sel = channel;
	lut_addr_reg.od_lut_ax_en = 1; // auto rolling index
	lut_addr_reg.od_lut_adr_mode = 0; //cur index first
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	// target table
	if(lut_addr_reg.table_type)
	{
		for (i = 0; i < OD_table_length; i++){
			RBus_UInt32 regValue_tmp = IoReg_Read32(OD_OD_LUT_DATA_reg);
			// 33 to 17 table
			//*(pODtable+i) = ( regValue_tmp >> 24);

			/* henry merlin3 OD table only 17x17 */
			*(pODtable+i) = regValue_tmp;
		}
	}
	// delta table
	else
	{

		for (i = 0; i < OD_table_length; i++) {
			ODtable_tmp[i] = IoReg_Read32(OD_OD_LUT_DATA_reg);

			remain = i%17;

			// 33 to 17 table
			//ODtable_sign[i] = (char)(ODtable_tmp[i] >> 24);

			/* henry merlin3 OD table only 17x17 */
			ODtable_sign[i] = (char)(ODtable_tmp[i]);

			// add cur pixel value (to target table)
			if(remain != 16)
				*(pODtable+i) = ODtable_sign[i] + 16 * remain;
			else
				*(pODtable+i) = ODtable_sign[i] + 255;

		}

	}
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, _BIT16);
}
#endif


void drvif_color_od_table(unsigned int *pODtable, unsigned char targetmode)
{
	int i;
	od_od_lut_addr_RBUS lut_addr_reg;

	/*
	int count = 30;
	ppoverlay_display_timing_ctrl2_RBUS display_timing_ctrl2_reg;
	struct timespec timeout;
	timeout = ns_to_timespec(1000 * 1000);

	display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	while((display_timing_ctrl2_reg.pcid2_en || display_timing_ctrl2_reg.pcid_en) && count-- > 0)
	{
		hrtimer_nanosleep(&timeout, NULL, HRTIMER_MODE_REL, CLOCK_MONOTONIC);
		display_timing_ctrl2_reg.regValue = IoReg_Read32(PPOVERLAY_Display_Timing_CTRL2_reg);
	}
	*/
	lut_addr_reg.regValue = IoReg_Read32(OD_OD_LUT_ADDR_reg);
	lut_addr_reg.table_type = targetmode;
	lut_addr_reg.od_lut_sel = 3;
	lut_addr_reg.od_lut_ax_en = 1;
	lut_addr_reg.od_lut_ax_mode = 0;
	lut_addr_reg.od_lut_adr_mode = 0; //col(cur) indexfirst
	//lut_addr_reg.od_lut_adr_mode = 1; //row(pre) indexfirst
	lut_addr_reg.od_lut_column_addr = 0;
	lut_addr_reg.od_lut_row_addr = 0;
	IoReg_Write32(OD_OD_LUT_ADDR_reg, lut_addr_reg.regValue);

	for (i = 0; i < OD_table_length; i++) {
		IoReg_Write32(OD_OD_LUT_DATA_reg, *(pODtable+i));
		VIPprintf("[VPQ][OD]Table[%d]=0x%x\n",i,*(pODtable+i));
	}

	/* disable table access*/
	IoReg_ClearBits(OD_OD_LUT_ADDR_reg, _BIT16);
}

void drvif_color_set_od_gain(unsigned char ucGain)
{
	od_od_ctrl2_RBUS od_ctrl2_reg;

	if (ucGain > 127)
		ucGain = 127;

	od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	od_ctrl2_reg.od_delta_gain_r = ucGain;
	od_ctrl2_reg.od_delta_gain_g = ucGain;
	od_ctrl2_reg.od_delta_gain_b = ucGain;

	IoReg_Write32(OD_OD_CTRL2_reg, od_ctrl2_reg.regValue);
}

void drvif_color_set_od_gain_RGB(unsigned char rGain, unsigned char gGain, unsigned char bGain)
{
	od_od_ctrl2_RBUS od_ctrl2_reg;

	if (rGain > 127)
		rGain = 64;
	if (gGain > 127)
		gGain = 64;
	if (bGain > 127)
		bGain = 64;

	od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	od_ctrl2_reg.od_delta_gain_r = rGain;
	od_ctrl2_reg.od_delta_gain_g = gGain;
	od_ctrl2_reg.od_delta_gain_b = bGain;

	IoReg_Write32(OD_OD_CTRL2_reg, od_ctrl2_reg.regValue);
}

unsigned char drvif_color_get_od_gain(unsigned char channel)
{
	od_od_ctrl2_RBUS od_ctrl2_reg;
	od_ctrl2_reg.regValue = IoReg_Read32(OD_OD_CTRL2_reg);
	switch(channel)
	{
		case 0:
		default:
			return od_ctrl2_reg.od_delta_gain_r;
		case 1:
			return od_ctrl2_reg.od_delta_gain_g;
		case 2:
			return od_ctrl2_reg.od_delta_gain_b;
	}
}

void drvif_color_safe_od_enable(unsigned char bEnable, unsigned char bOD_OnOff_Switch)
{
	// henry Merlin3 has fixed hw bug
	return;
#if 0

	UINT32 timeoutcnt = 0x032500;
	od_od_ctrl_RBUS od_ctrl_reg;

	// OD function db on
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	// OD function db read sel: reg
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25

	if (!bEnable) { // OD Disable
		if (bOD_OnOff_Switch) {
			// OD off
			IoReg_ClearBits(OD_OD_CTRL_reg, OD_OD_CTRL_od_en_mask|OD_OD_CTRL_od_fun_sel_mask); //_BIT0|_BIT1

			// OD function db apply
			IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

			// wait vsync
			od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			while((od_ctrl_reg.regValue & 0x3) != 0 && timeoutcnt-- != 0) {
				od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			}
		}

		IoReg_Write32(OD_DMA_ODDMA_Cap_BoundaryAddr1_reg, IoReg_Read32(OD_DMA_ODDMA_Cap_BoundaryAddr2_reg));
	} else { // OD Enable
		od_od_ctrl1_RBUS od_ctrl1_reg;

		IoReg_Write32(OD_DMA_ODDMA_Cap_BoundaryAddr1_reg, IoReg_Read32(OD_DMA_ODDMA_Cap_L2_Start_reg)+0x800000-0x20);

		IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26
		od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);
		while(od_ctrl1_reg.db_apply == 0x1 && timeoutcnt-- != 0) {
			od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);
		}

		if (bOD_OnOff_Switch) {
			// OD on
			IoReg_SetBits(OD_OD_CTRL_reg, OD_OD_CTRL_od_en_mask|OD_OD_CTRL_od_fun_sel_mask); //_BIT0|_BIT1

			// OD function db apply
			IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

			// wait vsync
			timeoutcnt = 0x032500;
			od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			while((od_ctrl_reg.regValue & 0x3) != 0x3 && timeoutcnt-- != 0) {
				od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			}
		}
	}

	// OD function db off
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	// OD function db read sel: temp
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
#endif
}

void drvif_color_set_od_bits(unsigned char FrameLimitBit, unsigned char input_bit_sel)
{
	od_od_ctrl_RBUS od_ctrl_reg;
	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_od_dma_ctrl_2_RBUS od_dma_od_dma_ctrl_2_reg;
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	od_dma_oddma_wr_num_bl_wrap_word_RBUS od_dma_oddma_wr_num_bl_wrap_word_reg;
	od_dma_oddma_rd_num_bl_wrap_word_RBUS od_dma_oddma_rd_num_bl_wrap_word_reg;
	od_dma_oddma_wr_num_bl_wrap_line_step_RBUS od_dma_oddma_cap_linestep_reg;
	od_dma_oddma_rd_num_bl_wrap_line_step_RBUS od_dma_oddma_rd_num_bl_wrap_line_step_reg;
	od_dma_oddma_pq_cmp_pair_RBUS oddma_pq_cmp_pair_reg;
	od_dma_oddma_pq_decmp_pair_RBUS oddma_pq_decmp_pair_reg;

	UINT32 timeoutcnt = 0x032500;
	unsigned char dma_mode;
	unsigned char od_enable_status = 0;
	unsigned short width;

	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	od_enable_status = (od_ctrl_reg.regValue & 0x3)?1:0;

	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
	if (oddma_pq_cmp_bit_reg.frame_limit_bit == FrameLimitBit && (od_enable_status == (FrameLimitBit?1:0)))
		return;

	// disable OD
	if (od_enable_status) {
		// OD function db on
		IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
		// OD function db read sel: reg
		IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25

		//IoReg_ClearBits(OD_OD_CTRL_reg, OD_OD_CTRL_od_en_mask|OD_OD_CTRL_od_fun_sel_mask); //_BIT0|_BIT1 ////mac6p removed
		IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26
		od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		while((od_ctrl_reg.regValue & 0x3) != 0 && timeoutcnt-- != 0) {
			od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
		}
	}

	if (FrameLimitBit > 0) {
		// b802cdc0 dma ctrl
		od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);
		dma_mode = od_dma_od_dma_ctrl_reg.oddma_mode_sel;
		od_dma_od_dma_ctrl_reg.bit_sel = input_bit_sel;
		IoReg_Write32(OD_DMA_OD_DMA_CTRL_reg, od_dma_od_dma_ctrl_reg.regValue);

		od_dma_od_dma_ctrl_2_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_2_reg);
		width = od_dma_od_dma_ctrl_2_reg.hact;

		// b802cd08 Compression ratio
		//oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);
		oddma_pq_cmp_bit_reg.frame_limit_bit = FrameLimitBit;

		// DMA buffer line limit clamp
		if(dma_mode == 0)
			oddma_pq_cmp_bit_reg.line_limit_bit = oddma_pq_cmp_bit_reg.frame_limit_bit;
		else if(dma_mode == 1)
			oddma_pq_cmp_bit_reg.line_limit_bit = FrameLimitBit + 6;
		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_BIT_reg, oddma_pq_cmp_bit_reg.regValue);

		oddma_pq_cmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg);
		oddma_pq_decmp_pair_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg);

		if(FrameLimitBit <= 6) {
			oddma_pq_cmp_pair_reg.cmp_data_bit_width = 0;
			oddma_pq_decmp_pair_reg.decmp_data_bit_width = 0;
		}
		else {
			oddma_pq_cmp_pair_reg.cmp_data_bit_width = 1;
			oddma_pq_decmp_pair_reg.decmp_data_bit_width = 1;
		}

		if(dma_mode == 0) {
			oddma_pq_cmp_pair_reg.cmp_line_sum_bit = (width * oddma_pq_cmp_bit_reg.line_limit_bit + 256) / 128;
			oddma_pq_decmp_pair_reg.decmp_line_sum_bit = oddma_pq_cmp_pair_reg.cmp_line_sum_bit;
		}

		IoReg_Write32(OD_DMA_ODDMA_PQ_CMP_PAIR_reg, oddma_pq_cmp_pair_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_PQ_DECMP_PAIR_reg, oddma_pq_decmp_pair_reg.regValue);

		od_dma_oddma_wr_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg);
		od_dma_oddma_rd_num_bl_wrap_word_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg);

		if(dma_mode == 0) { // LINE MODE
			// b802ce38 Cap LineStep
			od_dma_oddma_cap_linestep_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg);
			od_dma_oddma_cap_linestep_reg.cap_line_step = oddma_pq_cmp_pair_reg.cmp_line_sum_bit;
			IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_line_step_reg, od_dma_oddma_cap_linestep_reg.regValue);

			// b802ceac Disp LineStep
			od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg);
			od_dma_oddma_rd_num_bl_wrap_line_step_reg.disp_line_step = oddma_pq_decmp_pair_reg.decmp_line_sum_bit;
			IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_line_step_reg, od_dma_oddma_rd_num_bl_wrap_line_step_reg.regValue);

			// b802ce44 cap_line_128_num
			od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = oddma_pq_cmp_pair_reg.cmp_line_sum_bit;

			// b802cec4 disp_line_128_num
			od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = oddma_pq_decmp_pair_reg.decmp_line_sum_bit;
		} else if(dma_mode == 1) { // FRAME MODE
			// b802ce44 cap_line_128_num = 0xffff
			od_dma_oddma_wr_num_bl_wrap_word_reg.cap_line_128_num = 0xffff;

			// b802cec4 disp_line_128_num = 0xffff
			od_dma_oddma_rd_num_bl_wrap_word_reg.disp_line_128_num = 0xffff;
		}

		IoReg_Write32(OD_DMA_ODDMA_WR_num_bl_wrap_word_reg, od_dma_oddma_wr_num_bl_wrap_word_reg.regValue);
		IoReg_Write32(OD_DMA_ODDMA_RD_num_bl_wrap_word_reg, od_dma_oddma_rd_num_bl_wrap_word_reg.regValue);

		drvif_color_set_od_dma_enable(1);

		//if (od_enable_status) {
		if (1) {
			od_od_ctrl1_RBUS od_ctrl1_reg;
			IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26
			od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);
			while(od_ctrl1_reg.db_apply == 0x1 && timeoutcnt-- != 0) {
				od_ctrl1_reg.regValue = IoReg_Read32(OD_OD_CTRL1_reg);
			}

			// OD on
			//IoReg_SetBits(OD_OD_CTRL_reg, OD_OD_CTRL_od_en_mask|OD_OD_CTRL_od_fun_sel_mask); //_BIT0|_BIT1
			drvif_color_od(1);

			// OD function db apply
			IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_apply_mask); //_BIT26

			// wait vsync
			timeoutcnt = 0x032500;
			od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			while((od_ctrl_reg.regValue & 0x3) != 0x3 && timeoutcnt-- != 0) {
				od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
			}
		}
	} else {
		drvif_color_set_od_dma_enable(0);
	}

	// OD function db off
	IoReg_ClearBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_od_en_dbf_en_mask); //_BIT24
	// OD function db read sel: temp
	IoReg_SetBits(OD_OD_CTRL1_reg, OD_OD_CTRL1_db_read_level_mask); //_BIT25
}

unsigned char drvif_color_get_od_bits(void)
{
	od_dma_oddma_pq_cmp_bit_RBUS oddma_pq_cmp_bit_reg;
	oddma_pq_cmp_bit_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_PQ_CMP_BIT_reg);

	return oddma_pq_cmp_bit_reg.frame_limit_bit;
}

unsigned char drvif_color_get_od_en(void)
{
	od_od_ctrl_RBUS od_ctrl_reg;
	od_ctrl_reg.regValue = IoReg_Read32(OD_OD_CTRL_reg);
	return od_ctrl_reg.od_en;
}

unsigned char drvif_color_get_od_dma_mode(void)
{
	od_dma_od_dma_ctrl_RBUS od_dma_od_dma_ctrl_reg;
	od_dma_od_dma_ctrl_reg.regValue = IoReg_Read32(OD_DMA_OD_DMA_CTRL_reg);

	return od_dma_od_dma_ctrl_reg.oddma_mode_sel;
}

unsigned char drvif_color_set_od_dma_enable(unsigned char enable)
{
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;

	if (!enable && drvif_color_get_od_en())
		drvif_color_od(0);

	// b802ce08 cap enable
	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_wr_ctrl_reg.cap_en = enable;
	IoReg_Write32(OD_DMA_ODDMA_WR_Ctrl_reg, od_dma_oddma_wr_ctrl_reg.regValue);

	// b802ce88 disp enable
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.disp_en = enable;
	IoReg_Write32(OD_DMA_ODDMA_RD_Ctrl_reg, od_dma_oddma_rd_ctrl_reg.regValue);

	return TRUE;
}

unsigned char drvif_color_get_od_dma_enable(void)
{
	od_dma_oddma_wr_ctrl_RBUS od_dma_oddma_wr_ctrl_reg;
	od_dma_oddma_rd_ctrl_RBUS od_dma_oddma_rd_ctrl_reg;

	od_dma_oddma_wr_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_WR_Ctrl_reg);
	od_dma_oddma_rd_ctrl_reg.regValue = IoReg_Read32(OD_DMA_ODDMA_RD_Ctrl_reg);

	if (od_dma_oddma_wr_ctrl_reg.cap_en && od_dma_oddma_rd_ctrl_reg.disp_en)
		return TRUE;
	else
		return FALSE;
}

void drvif_color_od_set_vergain(DRV_OD_VerGain od_vergain)
{
	od_od_vertical_gain_setting_RBUS od_vergain_reg;
	od_od_vertical_gain_debug_mode_RBUS od_vergain_debug_mode_reg;
	od_od_vertical_gain_table0_RBUS od_vergain_table0_reg;
	od_od_vertical_gain_table1_RBUS od_vergain_table1_reg;
	od_od_vertical_gain_table2_RBUS od_vergain_table2_reg;
	od_od_vertical_gain_table3_RBUS od_vergain_table3_reg;
	od_od_vertical_gain_table4_RBUS od_vergain_table4_reg;
	od_od_vertical_gain_table5_RBUS od_vergain_table5_reg;
	od_od_vertical_gain_table6_RBUS od_vergain_table6_reg;
	od_od_vertical_gain_table7_RBUS od_vergain_table7_reg;
	
	od_vergain_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_SETTING_reg);
	od_vergain_debug_mode_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_DEBUG_MODE_reg);
	od_vergain_table0_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table0_reg);
	od_vergain_table1_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table1_reg);
	od_vergain_table2_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table2_reg);
	od_vergain_table3_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table3_reg);
	od_vergain_table4_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table4_reg);
	od_vergain_table5_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table5_reg);
	od_vergain_table6_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table6_reg);
	od_vergain_table7_reg.regValue = IoReg_Read32(OD_OD_VERTICAL_GAIN_table7_reg);

	od_vergain_reg.vergain_en = od_vergain.enable;
	od_vergain_reg.vergain_factor = od_vergain.factor;
	od_vergain_debug_mode_reg.vergain_offset = od_vergain.offset;
	od_vergain_table0_reg.vergain_table0 = od_vergain.vergain[0];
	od_vergain_table0_reg.vergain_table1 = od_vergain.vergain[1];
	od_vergain_table0_reg.vergain_table2 = od_vergain.vergain[2];
	od_vergain_table0_reg.vergain_table3 = od_vergain.vergain[3];
	od_vergain_table1_reg.vergain_table4 = od_vergain.vergain[4];
	od_vergain_table1_reg.vergain_table5 = od_vergain.vergain[5];
	od_vergain_table1_reg.vergain_table6 = od_vergain.vergain[6];
	od_vergain_table1_reg.vergain_table7 = od_vergain.vergain[7];
	od_vergain_table2_reg.vergain_table8 = od_vergain.vergain[8];
	od_vergain_table2_reg.vergain_table9 = od_vergain.vergain[9];
	od_vergain_table2_reg.vergain_table10 = od_vergain.vergain[10];
	od_vergain_table2_reg.vergain_table11 = od_vergain.vergain[11];
	od_vergain_table3_reg.vergain_table12 = od_vergain.vergain[12];
	od_vergain_table3_reg.vergain_table13 = od_vergain.vergain[13];
	od_vergain_table3_reg.vergain_table14 = od_vergain.vergain[14];
	od_vergain_table3_reg.vergain_table15 = od_vergain.vergain[15];
	od_vergain_table4_reg.vergain_table16 = od_vergain.vergain[16];
	od_vergain_table4_reg.vergain_table17 = od_vergain.vergain[17];
	od_vergain_table4_reg.vergain_table18 = od_vergain.vergain[18];
	od_vergain_table4_reg.vergain_table19 = od_vergain.vergain[19];
	od_vergain_table5_reg.vergain_table20 = od_vergain.vergain[20];
	od_vergain_table5_reg.vergain_table21 = od_vergain.vergain[21];
	od_vergain_table5_reg.vergain_table22 = od_vergain.vergain[22];
	od_vergain_table5_reg.vergain_table23 = od_vergain.vergain[23];
	od_vergain_table6_reg.vergain_table24 = od_vergain.vergain[24];
	od_vergain_table6_reg.vergain_table25 = od_vergain.vergain[25];
	od_vergain_table6_reg.vergain_table26 = od_vergain.vergain[26];
	od_vergain_table6_reg.vergain_table27 = od_vergain.vergain[27];
	od_vergain_table7_reg.vergain_table28 = od_vergain.vergain[28];
	od_vergain_table7_reg.vergain_table29 = od_vergain.vergain[29];
	od_vergain_table7_reg.vergain_table30 = od_vergain.vergain[30];
	od_vergain_table7_reg.vergain_table31 = od_vergain.vergain[31];

	IoReg_Write32(OD_OD_VERTICAL_GAIN_SETTING_reg, od_vergain_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_DEBUG_MODE_reg, od_vergain_debug_mode_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table0_reg, od_vergain_table0_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table1_reg, od_vergain_table1_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table2_reg, od_vergain_table2_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table3_reg, od_vergain_table3_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table4_reg, od_vergain_table4_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table5_reg, od_vergain_table5_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table6_reg, od_vergain_table6_reg.regValue);
	IoReg_Write32(OD_OD_VERTICAL_GAIN_table7_reg, od_vergain_table7_reg.regValue);
}
