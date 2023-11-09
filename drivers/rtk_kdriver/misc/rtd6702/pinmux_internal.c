/* Auto Created File */
#include <rtk_kdriver/io.h>
#include <rbus/pinmux_reg.h>

#define PINMUX_MAIN_1_BEGIN PINMUX_GPIO_TLEFT_CFG_0_reg
#define PINMUX_MAIN_1_END (PINMUX_GPIO_TLEFT_CFG_12_reg+ 0x4)

#define PINMUX_MAIN_2_BEGIN PINMUX_TLEFT_IEV18_EA_0_reg
#define PINMUX_MAIN_2_END (PINMUX_GPIO_EMMCGPIO_CFG_0_reg+ 0x4)

#define PINMUX_MAIN_3_BEGIN PINMUX_GPIO_EMMCCLK_CFG_0_reg
#define PINMUX_MAIN_3_END (PINMUX_GPIO_EMMC_CFG_0_reg+ 0x4)

#define PINMUX_MAIN_4_BEGIN PINMUX_Pin_Mux_Ctrl0_reg
#define PINMUX_MAIN_4_END (PINMUX_Pin_Mux_Ctrl6_reg+ 0x4)

#define PINMUX_MAIN_5_BEGIN PINMUX_Pin_Mux_DBG0_reg
#define PINMUX_MAIN_5_END (PINMUX_Pin_Mux_DBG2_reg+ 0x4)

#define PINMUX_MAIN_6_BEGIN PINMUX_PIF_3_reg
#define PINMUX_MAIN_6_END (PINMUX_Pin_Mux_LVDS_reg+ 0x4)

#define PINMUX_MAIN_7_BEGIN PINMUX_PIF_A_reg
#define PINMUX_MAIN_7_END (PINMUX_PIF_A_reg+ 0x4)

#define PINMUX_MAIN_8_BEGIN PINMUX_PIF_B_reg
#define PINMUX_MAIN_8_END (PINMUX_PIF_B_reg+ 0x4)

#define PINMUX_MAIN_9_BEGIN PINMUX_EMMCCLK_CFG_0_reg
#define PINMUX_MAIN_9_END (PINMUX_EMMC_EXTRACFG_NP4E_2_reg+ 0x4)

#define PINMUX_MAIN_10_BEGIN PINMUX_emmc_33v_sel_reg
#define PINMUX_MAIN_10_END (PINMUX_emmc_33v_sel_reg+ 0x4)

#define PINMUX_MAIN_11_BEGIN PINMUX_PIF_C_reg
#define PINMUX_MAIN_11_END (PINMUX_PIF_F_reg+ 0x4)

#define PINMUX_MAIN_12_BEGIN PINMUX_ST_GPIO_ST_CFG_0_reg
#define PINMUX_MAIN_12_END (PINMUX_ST_ST_SMT_0_reg+ 0x4)

#define PINMUX_MAIN_13_BEGIN PINMUX_ST_STB_0_reg
#define PINMUX_MAIN_13_END (PINMUX_ST_Pin_Mux_Ctrl1_reg+ 0x4)

#define PINMUX_MAIN_14_BEGIN PINMUX_ST_Pin_Mux_DBG_reg
#define PINMUX_MAIN_14_END (PINMUX_ST_Pin_Mux_DBG_reg+ 0x4)

#define PINMUX_MAIN_15_BEGIN PINMUX_ST_HPD_R1K_Ctrl0_reg
#define PINMUX_MAIN_15_END (PINMUX_ST_HPD_R1K_Ctrl3_reg+ 0x4)

unsigned int pinmux_buffer[((PINMUX_MAIN_1_END - PINMUX_MAIN_1_BEGIN) + 
	(PINMUX_MAIN_2_END - PINMUX_MAIN_2_BEGIN) + 
	(PINMUX_MAIN_3_END - PINMUX_MAIN_3_BEGIN) + 
	(PINMUX_MAIN_4_END - PINMUX_MAIN_4_BEGIN) + 
	(PINMUX_MAIN_5_END - PINMUX_MAIN_5_BEGIN) + 
	(PINMUX_MAIN_6_END - PINMUX_MAIN_6_BEGIN) + 
	(PINMUX_MAIN_7_END - PINMUX_MAIN_7_BEGIN) + 
	(PINMUX_MAIN_8_END - PINMUX_MAIN_8_BEGIN) + 
	(PINMUX_MAIN_9_END - PINMUX_MAIN_9_BEGIN) + 
	(PINMUX_MAIN_10_END - PINMUX_MAIN_10_BEGIN) + 
	(PINMUX_MAIN_11_END - PINMUX_MAIN_11_BEGIN) + 
	(PINMUX_MAIN_12_END - PINMUX_MAIN_12_BEGIN) + 
	(PINMUX_MAIN_13_END - PINMUX_MAIN_13_BEGIN) + 
	(PINMUX_MAIN_14_END - PINMUX_MAIN_14_BEGIN) + 
	(PINMUX_MAIN_15_END - PINMUX_MAIN_15_BEGIN))/4];

static int pinmux_suspend_store(unsigned int *idx, unsigned int register_begin, unsigned int register_end)
{
	while(register_begin < register_end){
		pinmux_buffer[(*idx)] = rtd_inl(register_begin);
		register_begin += 4;
		(*idx)++;
	}

	return 0;
}
static int pinmux_resume_reload(unsigned int *idx, unsigned int register_begin, unsigned int register_end)
{
	while(register_begin < register_end){
		rtd_outl(register_begin, pinmux_buffer[(*idx)]);
		register_begin += 4;
		(*idx)++;
	}

	return 0;
}


void pinmux_suspend_store_range(void)
{
	unsigned int idx = 0;
	pinmux_suspend_store(&idx, PINMUX_MAIN_1_BEGIN, PINMUX_MAIN_1_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_2_BEGIN, PINMUX_MAIN_2_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_3_BEGIN, PINMUX_MAIN_3_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_4_BEGIN, PINMUX_MAIN_4_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_5_BEGIN, PINMUX_MAIN_5_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_6_BEGIN, PINMUX_MAIN_6_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_7_BEGIN, PINMUX_MAIN_7_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_8_BEGIN, PINMUX_MAIN_8_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_9_BEGIN, PINMUX_MAIN_9_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_10_BEGIN, PINMUX_MAIN_10_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_11_BEGIN, PINMUX_MAIN_11_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_12_BEGIN, PINMUX_MAIN_12_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_13_BEGIN, PINMUX_MAIN_13_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_14_BEGIN, PINMUX_MAIN_14_END);
	pinmux_suspend_store(&idx, PINMUX_MAIN_15_BEGIN, PINMUX_MAIN_15_END);
}

void pinmux_resume_reload_range(void)
{
	unsigned int idx = 0;
	pinmux_resume_reload(&idx, PINMUX_MAIN_1_BEGIN, PINMUX_MAIN_1_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_2_BEGIN, PINMUX_MAIN_2_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_3_BEGIN, PINMUX_MAIN_3_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_4_BEGIN, PINMUX_MAIN_4_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_5_BEGIN, PINMUX_MAIN_5_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_6_BEGIN, PINMUX_MAIN_6_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_7_BEGIN, PINMUX_MAIN_7_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_8_BEGIN, PINMUX_MAIN_8_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_9_BEGIN, PINMUX_MAIN_9_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_10_BEGIN, PINMUX_MAIN_10_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_11_BEGIN, PINMUX_MAIN_11_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_12_BEGIN, PINMUX_MAIN_12_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_13_BEGIN, PINMUX_MAIN_13_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_14_BEGIN, PINMUX_MAIN_14_END);
	pinmux_resume_reload(&idx, PINMUX_MAIN_15_BEGIN, PINMUX_MAIN_15_END);
}
