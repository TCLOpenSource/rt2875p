#ifndef __RKT_THERMAL_REG_H__
#define __RKT_THERMAL_REG_H__
#include <rbus/pll_reg_reg.h>

#define THERMAL_SENSOR_1           PLL_REG_TEMP_SENSOR_1_reg
#define THERMAL_SENSOR_2           PLL_REG_TEMP_SENSOR_2_reg
#define THERMAL_SENSOR_3           PLL_REG_TEMP_SENSOR_3_reg
#define THERMAL_SENSOR_4           PLL_REG_TEMP_SENSOR_4_reg
#define THERMAL_SENSOR_5           PLL_REG_TEMP_SENSOR_5_reg
#define THERMAL_SENSOR_6           PLL_REG_TEMP_SENSOR_6_reg
#define THERMAL_SENSOR_7           PLL_REG_TEMP_SENSOR_7_reg
#define THERMAL_SENSOR_8           PLL_REG_TEMP_SENSOR_8_reg
#define THERMAL_RESULT_1            PLL_REG_THERMAL_RESULT_1_reg
#define THERMAL_RESULT_2                PLL_REG_THERMAL_RESULT_2_reg
#define THERMAL_IRQ_Threshold_1      PLL_REG_THERMAL_IRQ_Threshold_1_reg
#define THERMAL_IRQ_Threshold_2      PLL_REG_THERMAL_IRQ_Threshold_2_reg


#define THERMAL_TM_A(x)   (((x) << PLL_REG_TEMP_SENSOR_1_tm_a_shift) & PLL_REG_TEMP_SENSOR_1_tm_a_mask)                                                  
#define THERMAL_TM_B(x)   (((x) << PLL_REG_TEMP_SENSOR_2_tm_b_shift) & PLL_REG_TEMP_SENSOR_2_tm_b_mask)               
#define THERMAL_TM_ADCCKSEL(x)  (((x) << PLL_REG_TEMP_SENSOR_2_tm_adccksel_shift) & PLL_REG_TEMP_SENSOR_2_tm_adccksel_mask)
#define THERMAL_TM_CHOPEN(x) (((x) << PLL_REG_TEMP_SENSOR_2_tm_chopen_shift) & PLL_REG_TEMP_SENSOR_2_tm_chopen_mask)
#define THERMAL_TM_OSR(x)   (((x) << PLL_REG_TEMP_SENSOR_2_tm_osr_shift) & PLL_REG_TEMP_SENSOR_2_tm_osr_mask)

#define THERMAL_TM_HOLD_EN(x)  (((x) << PLL_REG_TEMP_SENSOR_3_tm_hold_en_shift) & PLL_REG_TEMP_SENSOR_3_tm_hold_en_mask)
#define THERMAL_TM_POW(x)  (((x) << PLL_REG_TEMP_SENSOR_3_tm_pow_shift) & PLL_REG_TEMP_SENSOR_3_tm_pow_mask)

#define THERMAL_TM_HOLD_dly(x)  (((x) << PLL_REG_TEMP_SENSOR_4_tm_hold_dly_shift) & PLL_REG_TEMP_SENSOR_4_tm_hold_dly_mask)

#define THERMAL_TM_RSTB(x)  (((x) << PLL_REG_TEMP_SENSOR_5_tm_rstb_shift) & PLL_REG_TEMP_SENSOR_5_tm_rstb_mask)
#define THERMAL_TM_CHOPFREQSEL(x)  (((x) << PLL_REG_TEMP_SENSOR_5_tm_chopfreqsel_shift) & PLL_REG_TEMP_SENSOR_5_tm_chopfreqsel_mask)

#define THERMAL_TM_BIASDEM(x)  (((x) << PLL_REG_TEMP_SENSOR_7_tm_biasdem_enshift) & PLL_REG_TEMP_SENSOR_7_tm_biasdem_mask)
#define THERMAL_TD_WREQ_EDGE(x)  (((x) << PLL_REG_TEMP_SENSOR_7_td_wrep_edge_sel_shift) & PLL_REG_TEMP_SENSOR_7_td_wrep_edge_sel_mask)



#define SET_THERMAL_SENSOR_1(x)             rtd_outl(THERMAL_SENSOR_1, x)
#define SET_THERMAL_SENSOR_2(x)             rtd_outl(THERMAL_SENSOR_2, x)
#define SET_THERMAL_SENSOR_3(x)             rtd_outl(THERMAL_SENSOR_3, x)
#define SET_THERMAL_SENSOR_4(x)             rtd_outl(THERMAL_SENSOR_4, x)
#define SET_THERMAL_SENSOR_5(x)             rtd_outl(THERMAL_SENSOR_5, x)
#define SET_THERMAL_SENSOR_6(x)             rtd_outl(THERMAL_SENSOR_6, x)
#define SET_THERMAL_SENSOR_7(x)             rtd_outl(THERMAL_SENSOR_7, x)
#define SET_THERMAL_SENSOR_8(x)             rtd_outl(THERMAL_SENSOR_8, x)
#define SET_THERMAL_RESULT_1(x)             rtd_outl(THERMAL_RESULT_1, x)
#define SET_THERMAL_RESULT_2(x)             rtd_outl(THERMAL_RESULT_2, x)
#define SET_THERMAL_IRQ_Threshold_1(x)             rtd_outl(THERMAL_IRQ_Threshold_1, x)
#define SET_THERMAL_IRQ_Threshold_2(x)             rtd_outl(THERMAL_IRQ_Threshold_2, x)

#define GET_THERMAL_SENSOR_1()             rtd_inl(THERMAL_SENSOR_1)
#define GET_THERMAL_SENSOR_2()             rtd_inl(THERMAL_SENSOR_2)
#define GET_THERMAL_SENSOR_3()             rtd_inl(THERMAL_SENSOR_3)
#define GET_THERMAL_SENSOR_4()             rtd_inl(THERMAL_SENSOR_4)
#define GET_THERMAL_SENSOR_5()             rtd_inl(THERMAL_SENSOR_5)
#define GET_THERMAL_SENSOR_6()             rtd_inl(THERMAL_SENSOR_6)
#define GET_THERMAL_SENSOR_7()             rtd_inl(THERMAL_SENSOR_7)
#define GET_THERMAL_SENSOR_8()             rtd_inl(THERMAL_SENSOR_8)
#define GET_THERMAL_RESULT_1()             rtd_inl(THERMAL_RESULT_1)
#define GET_THERMAL_RESULT_2()             rtd_inl(THERMAL_RESULT_2)
#define GET_THERMAL_IRQ_Threshold_1()             rtd_inl(THERMAL_IRQ_Threshold_1)
#define GET_THERMAL_IRQ_Threshold_2()             rtd_inl(THERMAL_IRQ_Threshold_2)

#endif
