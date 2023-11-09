#include <rbus/iso_misc_reg.h>
#include <rbus/iso_misc_ddc1_reg.h>
#include <rbus/iso_misc_ddc2_reg.h>
#include <rbus/iso_misc_ddc3_reg.h>
#include <rbus/sys_reg_reg.h>
#include <rbus/stb_reg.h>
#include "rtk_ddc_priv.h"

#define RTK_DDC_NAME                	"rtk_ddc"

#define STB_ST_CTRL0_REG 				STB_ST_CTRL0_reg
#define SYS_REG_SYS_SRST0_REG 			SYS_REG_SYS_SRST0_reg

#define DDC_FSM_STATUS_IDEL             (1<<0)  // IDEL
#define DDC_FSM_STATUS_ID_DETECT        (1<<1)
#define DDC_FSM_STATUS_WR_BIT_7_0       (1<<2)
#define DDC_FSM_STATUS_WR_ACK           (1<<4)
#define DDC_FSM_STATUS_RD_BIT_7_0       (1<<5)
#define DDC_FSM_STATUS_RD_ACK           (1<<7)
#define DDC_DATA_WRITE_DEBOUNCE 		2
#define DDC_DATA_READ_DELAY				1

#define DDC_SCR_DATA_OUTPUT_DELAY_SEL_MASK 					ISO_MISC_DDC1_DDC_SCR_dataoutputdelay_sel_mask
#define DDC_SCR_DATA_OUTPUT_DELAY_SEL(value) 				ISO_MISC_DDC1_DDC_SCR_dataoutputdelay_sel(value)
#define DDC_DDC_SCR_GET_DATAOUTPUTDELAY_SEL(value) 			ISO_MISC_DDC1_DDC_SCR_get_dataoutputdelay_sel(value)

#define DDC_I2C_CR_DEBOUNCE_MODE_MASK 						ISO_MISC_DDC1_I2C_CR_debounce_mode_mask
#define DDC_I2C_CR_DEBOUNCE_MODE(value) 					ISO_MISC_DDC1_I2C_CR_debounce_mode(value) 
#define DDC_I2C_CR_GET_DEBOUNCE_MODE(value) 				ISO_MISC_DDC1_I2C_CR_get_debounce_mode(value) 

#define DDC_EDID_CR_EDID_EN_MASK 							ISO_MISC_DDC1_EDID_CR_edid_en_mask
#define DDC_EDID_CR_EDID_EN(value)							ISO_MISC_DDC1_EDID_CR_edid_en(value)

#define DDC_DDC_SCR_SGID_EN_MASK							ISO_MISC_DDC1_DDC_SCR_sgid_en_mask
#define DDC_DDC_SCR_SGID_EN(value) 							ISO_MISC_DDC1_DDC_SCR_sgid_en(value)
#define DDC_DDC_SCR_GET_SGID_EN(value) 						ISO_MISC_DDC1_DDC_SCR_get_sgid_en(value)

#define DDC_DDC_SCR_I2C_M_RD_ST_STS_MASK					ISO_MISC_DDC1_DDC_SCR_i2c_m_rd_sp_sts_mask
#define DDC_DDC_SCR_I2C_M_RD_ST_STS(value) 					ISO_MISC_DDC1_DDC_SCR_i2c_m_rd_st_sts(value)
#define DDC_DDC_SCR_GET_I2C_M_RD_ST_STS(value) 				ISO_MISC_DDC1_DDC_SCR_get_i2c_m_rd_st_sts(value)

#define DDC_DDC_SCR_I2C_M_RD_SP_STS_MASK					ISO_MISC_DDC1_DDC_SCR_i2c_m_rd_st_sts_mask
#define DDC_DDC_SCR_I2C_M_RD_SP_STS(value) 					ISO_MISC_DDC1_DDC_SCR_i2c_m_rd_sp_sts(value)
#define DDC_DDC_SCR_GET_I2C_M_RD_SP_STS(value) 				ISO_MISC_DDC1_DDC_SCR_get_i2c_m_rd_sp_sts(value)

#define DDC_DDC_SPR_SEG_CLR_EN_MASK 						ISO_MISC_DDC1_DDC_SPR_seg_clr_en_mask
#define DDC_DDC_SPR_SEG_CLR_EN(value) 					ISO_MISC_DDC1_DDC_SPR_seg_clr_en(value)
#define DDC_DDC_SPR_SEGPT_LATCHED_MASK 						ISO_MISC_DDC1_DDC_SPR_segpt_latched_mask
#define DDC_DDC_SPR_SEGPT_LATCHED(value) 					ISO_MISC_DDC1_DDC_SPR_segpt_latched(value)
#define DDC_DDC_SPR_GET_SEGPT_LATCHED(value) 				ISO_MISC_DDC1_DDC_SPR_get_segpt_latched(value)
#define DDC_DDC_SPR_GET_SEG_PT(value)						ISO_MISC_DDC1_DDC_SPR_get_seg_pt(value)

#define DDC_EDID_IR_I2C_M_RD_SP_IE_MASK 					ISO_MISC_DDC1_EDID_IR_i2c_m_rd_sp_ie_mask
#define DDC_EDID_IR_I2C_M_RD_SP_IE(value) 					ISO_MISC_DDC1_EDID_IR_i2c_m_rd_sp_ie(value)

#define DDC_EDID_CR_GET_EDID_EN(value) 						ISO_MISC_DDC1_EDID_CR_get_edid_en(value)

#define DDC_DDC_EDID_CR_EDID_ADDRESS_MASK 					ISO_MISC_DDC1_EDID_CR_edid_address_mask
#define DDC_DDC_EDID_CR_EDID_ADDRESS(value) 				ISO_MISC_DDC1_EDID_CR_edid_address(value)

#define INT_CTRL_REG 										ISO_MISC_INT_CTRL_reg
#define INT_CTRL_DDC1_INT_TO_SCPU_EN_MASK 					ISO_MISC_INT_CTRL_ddc1_int_to_scpu_en_mask
#define INT_CTRL_DDC2_INT_TO_SCPU_EN_MASK 					ISO_MISC_INT_CTRL_ddc2_int_to_scpu_en_mask
#define INT_CTRL_DDC3_INT_TO_SCPU_EN_MASK 					ISO_MISC_INT_CTRL_ddc3_int_to_scpu_en_mask


#define INT_CTRL_DDC1_INT_TO_SCPU_EN(value) 				ISO_MISC_INT_CTRL_ddc1_int_to_scpu_en(value)
#define INT_CTRL_DDC2_INT_TO_SCPU_EN(value) 				ISO_MISC_INT_CTRL_ddc2_int_to_scpu_en(value)
#define INT_CTRL_DDC3_INT_TO_SCPU_EN(value) 				ISO_MISC_INT_CTRL_ddc3_int_to_scpu_en(value)


#define INT_CTRL_DDC1_INT_TO_EMCU_EN_MASK 					ISO_MISC_INT_CTRL_ddc1_int_to_emcu_en_mask
#define INT_CTRL_DDC2_INT_TO_EMCU_EN_MASK 					ISO_MISC_INT_CTRL_ddc2_int_to_emcu_en_mask
#define INT_CTRL_DDC3_INT_TO_EMCU_EN_MASK 					ISO_MISC_INT_CTRL_ddc3_int_to_emcu_en_mask


#define INT_CTRL_DDC1_INT_TO_EMCU_EN(value) 				ISO_MISC_INT_CTRL_ddc1_int_to_emcu_en(value)
#define INT_CTRL_DDC2_INT_TO_EMCU_EN(value) 				ISO_MISC_INT_CTRL_ddc2_int_to_emcu_en(value)
#define INT_CTRL_DDC3_INT_TO_EMCU_EN(value) 				ISO_MISC_INT_CTRL_ddc3_int_to_emcu_en(value)


#define INT_CTRL_DDC_INT_TO_SCPU_EN_ALL 					(INT_CTRL_DDC1_INT_TO_SCPU_EN_MASK | \
															INT_CTRL_DDC2_INT_TO_SCPU_EN_MASK | \
															INT_CTRL_DDC3_INT_TO_SCPU_EN_MASK)


#define STB_ST_CLKEN1_REG									STB_ST_CLKEN1_reg
#define STB_ST_CLKEN1_WRITE_DATA_MASK						STB_ST_CLKEN1_write_data_mask
#define STB_ST_CLKEN1_CLKEN_DDC_ALL    						(STB_ST_CLKEN1_clken_ddc_mask | \
															STB_ST_CLKEN1_clken_ddc1_mask | \
															STB_ST_CLKEN1_clken_ddc2_mask | \
															STB_ST_CLKEN1_clken_ddc3_mask)

#define STB_ST_CLKEN1_WRITE_DATA(value) 					STB_ST_CLKEN1_write_data(value)
#define STB_ST_CLKEN1_CLK_OFF 								(STB_ST_CLKEN1_CLKEN_DDC_ALL | STB_ST_CLKEN1_WRITE_DATA(0))
#define STB_ST_CLKEN1_CLK_ON 								(STB_ST_CLKEN1_CLKEN_DDC_ALL | STB_ST_CLKEN1_WRITE_DATA(1))


#define STB_ST_SRST1_REG									STB_ST_SRST1_reg
#define STB_ST_SRST1_WRITE_DATA_MASK   						STB_ST_SRST1_write_data_mask
#define STB_ST_SRST1_RSTN_DDC_ALL      						(STB_ST_SRST1_rstn_ddc_mask | \
															STB_ST_SRST1_rstn_ddc1_mask | \
															STB_ST_SRST1_rstn_ddc2_mask | \
															STB_ST_SRST1_rstn_ddc3_mask)
#define STB_ST_SRST1_WRITE_DATA(value) 						STB_ST_SRST1_write_data(value)
#define STB_ST_SRST1_RESET_OFF 								(STB_ST_SRST1_RSTN_DDC_ALL | STB_ST_SRST1_WRITE_DATA(1))
#define STB_ST_SRST1_RESET_ON 								(STB_ST_SRST1_RSTN_DDC_ALL | STB_ST_SRST1_WRITE_DATA(0))

#define DDC_CTRL_REG 										ISO_MISC_DDC_CTRL_reg
#define DDC_CTRL_AUXRX_DDC_EN_MASK 							ISO_MISC_DDC_CTRL_auxrx_ddc_en_mask
#define DDC_CTRL_AUXRX_DDC_EN(value) 						ISO_MISC_DDC_CTRL_auxrx_ddc_en(value)

// for HDMI PHY0
static srtk_ddc_reg_map srtk_ddc1_reg =
{
	.DDC_I2C_CR  						= ISO_MISC_DDC1_I2C_CR_reg,
	.DDC_EDID_CR 						= ISO_MISC_DDC1_EDID_CR_reg,
	.DDC_EDID_IR 						= ISO_MISC_DDC1_EDID_IR_reg,
	.DDC_DDC_SIR 						= ISO_MISC_DDC1_DDC_SIR_reg,
	.DDC_DDC_SAP 						= ISO_MISC_DDC1_DDC_SAP_reg,
	.DDC_DDC_SCR 						= ISO_MISC_DDC1_DDC_SCR_reg,
	.DDC_DDC_FSMS 						= ISO_MISC_DDC1_DDC_FSMS_reg,
	.DDC_DDC_SPR 						= ISO_MISC_DDC1_DDC_SPR_reg,
	.SRST1_RSTN_DDC_RESET_MASK			= STB_ST_SRST1_rstn_ddc1_mask,
	.CLKEN1_CLKEN_DDC_ENABLE_MSAK 		= STB_ST_CLKEN1_clken_ddc1_mask,
	.MISC_ISR 							= ISO_MISC_ISR_reg,
	.ISR_DDC_INT_MASK 					= ISO_MISC_ISR_ddc1_int_mask,
	.DDC_DDC_SSAR						= ISO_MISC_DDC1_DDC_SSAR_reg,
};

	// for HDMI PHY1
static srtk_ddc_reg_map srtk_ddc2_reg =
{
	.DDC_I2C_CR  						= ISO_MISC_DDC2_I2C_CR_reg,
	.DDC_EDID_CR 						= ISO_MISC_DDC2_EDID_CR_reg,
	.DDC_EDID_IR 						= ISO_MISC_DDC2_EDID_IR_reg,
	.DDC_DDC_SIR 						= ISO_MISC_DDC2_DDC_SIR_reg,
	.DDC_DDC_SAP 						= ISO_MISC_DDC2_DDC_SAP_reg,
	.DDC_DDC_SCR 						= ISO_MISC_DDC2_DDC_SCR_reg,
	.DDC_DDC_FSMS 						= ISO_MISC_DDC2_DDC_FSMS_reg,
	.DDC_DDC_SPR 						= ISO_MISC_DDC2_DDC_SPR_reg,
	.SRST1_RSTN_DDC_RESET_MASK			= STB_ST_SRST1_rstn_ddc2_mask,
	.CLKEN1_CLKEN_DDC_ENABLE_MSAK 		= STB_ST_CLKEN1_clken_ddc2_mask,
	.MISC_ISR 							= ISO_MISC_ISR_reg,
	.ISR_DDC_INT_MASK 					= ISO_MISC_ISR_ddc2_int_mask,
	.DDC_DDC_SSAR						= ISO_MISC_DDC2_DDC_SSAR_reg,
};


// for HDMI PHY2
static srtk_ddc_reg_map srtk_ddc3_reg =
{
	.DDC_I2C_CR  						= ISO_MISC_DDC3_I2C_CR_reg,
	.DDC_EDID_CR 						= ISO_MISC_DDC3_EDID_CR_reg,
	.DDC_EDID_IR 						= ISO_MISC_DDC3_EDID_IR_reg,
	.DDC_DDC_SIR 						= ISO_MISC_DDC3_DDC_SIR_reg,
	.DDC_DDC_SAP 						= ISO_MISC_DDC3_DDC_SAP_reg,
	.DDC_DDC_SCR 						= ISO_MISC_DDC3_DDC_SCR_reg,
	.DDC_DDC_FSMS 						= ISO_MISC_DDC3_DDC_FSMS_reg,
	.DDC_DDC_SPR 						= ISO_MISC_DDC3_DDC_SPR_reg,
	.SRST1_RSTN_DDC_RESET_MASK			= STB_ST_SRST1_rstn_ddc3_mask,
	.CLKEN1_CLKEN_DDC_ENABLE_MSAK 		= STB_ST_CLKEN1_clken_ddc3_mask,
	.MISC_ISR 							= ISO_MISC_ISR_reg,
	.ISR_DDC_INT_MASK 					= ISO_MISC_ISR_ddc3_int_mask,
	.DDC_DDC_SSAR						= ISO_MISC_DDC3_DDC_SSAR_reg,
};
/*
// for HDMI PHY3
static srtk_ddc_reg_map srtk_ddc4_reg =
{
	.DDC_I2C_CR  						= ISO_MISC_DDC4_I2C_CR_reg,
	.DDC_EDID_CR 						= ISO_MISC_DDC4_EDID_CR_reg,
	.DDC_EDID_IR 						= ISO_MISC_DDC4_EDID_IR_reg,
	.DDC_DDC_SIR 						= ISO_MISC_DDC4_DDC_SIR_reg,
	.DDC_DDC_SAP 						= ISO_MISC_DDC4_DDC_SAP_reg,
	.DDC_DDC_SCR 						= ISO_MISC_DDC4_DDC_SCR_reg,
	.DDC_DDC_FSMS 						= ISO_MISC_DDC4_DDC_FSMS_reg,
	.DDC_DDC_SPR 						= ISO_MISC_DDC4_DDC_SPR_reg,
	.SRST1_RSTN_DDC_RESET_MASK			= STB_ST_SRST1_rstn_ddc4_mask,
	.CLKEN1_CLKEN_DDC_ENABLE_MSAK 		= STB_ST_CLKEN1_clken_ddc4_mask,
	.MISC_ISR 							= ISO_MISC_ISR_reg,
	.ISR_DDC_INT_MASK 					= ISO_MISC_ISR_ddc4_int_mask,
};*/

static srtk_ddc_chip rtk_ddc_chip[] = 
{

	{
		.id 				= 0,
		.ddc_type   		= DDC_VGA,
		.reg_remap 			= NULL,
	},

	{
		.id 				= 1,
		.ddc_type   		= DDC_HDMI,
		.reg_remap 			= &srtk_ddc1_reg,
	},
	
	{
		.id 				= 2,
		.ddc_type   		= DDC_HDMI,
		.reg_remap 			= &srtk_ddc2_reg,
	},
	
	{
		.id 				= 3,
		.ddc_type   		= DDC_HDMI,
		.reg_remap 			= &srtk_ddc3_reg,
	},

	{
		.id 				= 4,
		.ddc_type   		= DDC_HDMI,
		.reg_remap 			= NULL,
	},
};
