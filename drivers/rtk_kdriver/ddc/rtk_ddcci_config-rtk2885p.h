#ifndef __RTK_DDCCI_CONFIG_RTK2885P_H__
#define __RTK_DDCCI_CONFIG_RTK2885P_H__

#include <rbus/iso_misc_ddc1_reg.h>
#include <rbus/iso_misc_ddc2_reg.h>
#include <rbus/iso_misc_ddc3_reg.h>
#include <rbus/iso_misc_ddc4_reg.h>

#include "rtk_ddcci_priv.h"

#define RTK_DDCCI_NAME                	"rtk_ddcci"

#define DDC_DDC_SFCR_DOFFPT_RESET_MASK 						ISO_MISC_DDC1_DDC_SFCR_doffpt_reset_mask
#define DDC_DDC_SFCR_DOFFPT_RESET(value)					ISO_MISC_DDC1_DDC_SFCR_doffpt_reset(value)

#define DDC_DDC_SFCR_DIFFPT_RESET_MASK 						ISO_MISC_DDC1_DDC_SFCR_diffpt_reset_mask
#define DDC_DDC_SFCR_DIFFPT_RESET(value)					ISO_MISC_DDC1_DDC_SFCR_diffpt_reset(value)

#define DDC_DDC_SFCR_CIID_EN_MASK 							ISO_MISC_DDC1_DDC_SFCR_ciid_en_mask
#define DDC_DDC_SFCR_CIID_EN(value)							ISO_MISC_DDC1_DDC_SFCR_ciid_en(value)

#define DDC_DDC_SFSAR_CIID_ADDRESS_MASK 					ISO_MISC_DDC1_DDC_SFSAR_ciid_address_mask
#define DDC_DDC_SFSAR_CIID_ADDRESS(value)					ISO_MISC_DDC1_DDC_SFSAR_ciid_address(value)

#define DDC_DDC_SFSAR_GET_CIRWS_CURRENT(value)				ISO_MISC_DDC1_DDC_SFSAR_get_cirws_current(value)


#define DDC_DDC_SFDR_CIIDDP(value)							ISO_MISC_DDC1_DDC_SFDR_ciiddp(value)
#define DDC_DDC_SFDR_GET_CIIDDP(value)						ISO_MISC_DDC1_DDC_SFDR_get_ciiddp(value)


#define DDC_DDC_SFSR_GET_FFDINE_ST(value)					ISO_MISC_DDC1_DDC_SFSR_get_ffdine_st(value)
#define DDC_DDC_SFSR_FFDINE_ST_MASK 						ISO_MISC_DDC1_DDC_SFSR_ffdine_st_mask
#define DDC_DDC_SFSR_FFDINE_ST(value)						ISO_MISC_DDC1_DDC_SFSR_ffdine_st(value)

#define DDC_DDC_SFSR_GET_FFDOET_ST(value)					ISO_MISC_DDC1_DDC_SFSR_get_ffdoet_st(value)
#define DDC_DDC_SFSR_FFDOET_ST_MASK 						ISO_MISC_DDC1_DDC_SFSR_ffdoet_st_mask
#define DDC_DDC_SFSR_FFDOET_ST(value)						ISO_MISC_DDC1_DDC_SFSR_ffdoet_st(value)


#define DDC_DDC_SFIBLR_GET_FFDI_DLEN(value)					ISO_MISC_DDC1_DDC_SFIBLR_get_ffdi_dlen(value)
#define DDC_DDC_SFOBSR_GET_FFDO_DLEN(value)					ISO_MISC_DDC1_DDC_SFOBSR_get_ffdo_dlen(value)

#define DDC_DDC_SFBSR_GET_CIRWS_1ST(value)					ISO_MISC_DDC1_DDC_SFBSR_get_cirws_1st(value)
#define DDC_DDC_SFBSR_GET_CIPPT_1ST(value)					ISO_MISC_DDC1_DDC_SFBSR_get_cippt_1st(value)
#define DDC_DDC_SFBSR_GET_CISPT_1ST(value)					ISO_MISC_DDC1_DDC_SFBSR_get_cispt_1st(value)


#define DDC_DDC_SFBSR_GET_CIRWS_2ND(value)					ISO_MISC_DDC1_DDC_SFBSR_get_cirws_2nd(value)
#define DDC_DDC_SFBSR_GET_CIPPT_2ND(value)					ISO_MISC_DDC1_DDC_SFBSR_get_cippt_2nd(value)
#define DDC_DDC_SFBSR_GET_CISPT_2ND(value)					ISO_MISC_DDC1_DDC_SFBSR_get_cispt_2nd(value)

#define DDC_DDC_SFBSR_CISPT_1ST_MASK 						ISO_MISC_DDC1_DDC_SFBSR_cispt_1st_mask
#define DDC_DDC_SFBSR_CIPPT_1ST_MASK 						ISO_MISC_DDC1_DDC_SFBSR_cippt_1st_mask
#define DDC_DDC_SFBSR_CISPT_2ND_MASK 						ISO_MISC_DDC1_DDC_SFBSR_cispt_2nd_mask
#define DDC_DDC_SFBSR_CIPPT_2ND_MASK 						ISO_MISC_DDC1_DDC_SFBSR_cippt_2nd_mask
#define DDC_DDC_SFBSR_CISPT_1ST(value)						ISO_MISC_DDC1_DDC_SFBSR_cispt_1st(value)
#define DDC_DDC_SFBSR_CIPPT_1ST(value) 						ISO_MISC_DDC1_DDC_SFBSR_cippt_1st(value)
#define DDC_DDC_SFBSR_CISPT_2ND(value) 						ISO_MISC_DDC1_DDC_SFBSR_cispt_2nd(value)
#define DDC_DDC_SFBSR_CIPPT_2ND(value) 						ISO_MISC_DDC1_DDC_SFBSR_cippt_2nd(value)

#define DDC_DDC_SFIR_CIPPT_IE_MASK                                            ISO_MISC_DDC1_DDC_SFIR_cippt_ie_mask
#define DDC_DDC_SFIR_CIPPT_IE(value)                                            ISO_MISC_DDC1_DDC_SFIR_cippt_ie(value)

const static srtk_ddcci_reg_map srtk_ddcci1_reg =
{
	.DDC_DDC_SFCR						= ISO_MISC_DDC1_DDC_SFCR_reg,
	.DDC_DDC_SFSAR						= ISO_MISC_DDC1_DDC_SFSAR_reg,
	.DDC_DDC_SFDR						= ISO_MISC_DDC1_DDC_SFDR_reg,
	.DDC_DDC_SFBSR						= ISO_MISC_DDC1_DDC_SFBSR_reg,
	.DDC_DDC_SFIBLR						= ISO_MISC_DDC1_DDC_SFIBLR_reg,
	.DDC_DDC_SFOBSR						= ISO_MISC_DDC1_DDC_SFOBSR_reg,
	.DDC_DDC_SFIR 						= ISO_MISC_DDC1_DDC_SFIR_reg,
	.DDC_DDC_SFSR						= ISO_MISC_DDC1_DDC_SFSR_reg,
};

const static srtk_ddcci_reg_map srtk_ddcci2_reg =
{
	.DDC_DDC_SFCR 						= ISO_MISC_DDC2_DDC_SFCR_reg,
	.DDC_DDC_SFSAR						= ISO_MISC_DDC2_DDC_SFSAR_reg,
	.DDC_DDC_SFDR						= ISO_MISC_DDC2_DDC_SFDR_reg,
	.DDC_DDC_SFBSR						= ISO_MISC_DDC2_DDC_SFBSR_reg,
	.DDC_DDC_SFIBLR						= ISO_MISC_DDC2_DDC_SFIBLR_reg,
	.DDC_DDC_SFOBSR						= ISO_MISC_DDC2_DDC_SFOBSR_reg,
	.DDC_DDC_SFIR 						= ISO_MISC_DDC2_DDC_SFIR_reg,
	.DDC_DDC_SFSR						= ISO_MISC_DDC2_DDC_SFSR_reg,
};


const static srtk_ddcci_reg_map srtk_ddcci3_reg =
{
	.DDC_DDC_SFCR 						= ISO_MISC_DDC3_DDC_SFCR_reg,
	.DDC_DDC_SFSAR						= ISO_MISC_DDC3_DDC_SFSAR_reg,
	.DDC_DDC_SFDR						= ISO_MISC_DDC3_DDC_SFDR_reg,
	.DDC_DDC_SFBSR						= ISO_MISC_DDC3_DDC_SFBSR_reg,
	.DDC_DDC_SFIBLR						= ISO_MISC_DDC3_DDC_SFIBLR_reg,
	.DDC_DDC_SFOBSR						= ISO_MISC_DDC3_DDC_SFOBSR_reg,
	.DDC_DDC_SFIR 						= ISO_MISC_DDC3_DDC_SFIR_reg,
	.DDC_DDC_SFSR						= ISO_MISC_DDC3_DDC_SFSR_reg,
};

const static srtk_ddcci_reg_map srtk_ddcci4_reg =
{
	.DDC_DDC_SFCR 						= ISO_MISC_DDC4_DDC_SFCR_reg,
	.DDC_DDC_SFSAR						= ISO_MISC_DDC4_DDC_SFSAR_reg,
	.DDC_DDC_SFDR						= ISO_MISC_DDC4_DDC_SFDR_reg,
	.DDC_DDC_SFBSR						= ISO_MISC_DDC4_DDC_SFBSR_reg,
	.DDC_DDC_SFIBLR						= ISO_MISC_DDC4_DDC_SFIBLR_reg,
	.DDC_DDC_SFOBSR						= ISO_MISC_DDC4_DDC_SFOBSR_reg,
	.DDC_DDC_SFIR 						= ISO_MISC_DDC4_DDC_SFIR_reg,
	.DDC_DDC_SFSR						= ISO_MISC_DDC4_DDC_SFSR_reg,
};

static srtk_ddcci_chip rtk_ddcci_chip[] =
{

	{
		.id					= 0,
		.ddcci_type			= DDCCI_VGA,
		.reg_remap 			= NULL,
	},

	{
		.id					= 1,
		.ddcci_type			= DDCCI_HDMI,
		.reg_remap 			= &srtk_ddcci1_reg,
	},

	{
		.id					= 2,
		.ddcci_type			= DDCCI_HDMI,
		.reg_remap 			= &srtk_ddcci2_reg,
	},

	{
		.id					= 3,
		.ddcci_type			= DDCCI_HDMI,
		.reg_remap 			= &srtk_ddcci3_reg,
	},
	{
		.id					= 4,
		.ddcci_type			= DDCCI_HDMI,
		.reg_remap 			= &srtk_ddcci4_reg,
	},
};
#endif //__RTK_DDCCI_CONFIG_RTK2885P_H__
