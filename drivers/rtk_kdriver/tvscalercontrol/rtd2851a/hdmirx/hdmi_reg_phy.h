#include <rbus/dfe_p0_reg.h>
#include <rbus/hdmirx_2p0_phy_reg.h>
#include <rbus/efuse_reg.h>

#define REG_CK_LATCH						(_BIT5)

#define TOP_IN_Z0_N						(_BIT23)
#define TOP_IN_Z0_P						(_BIT22)
#define TOP_IN_REG_IBHN_TUNE		(_BIT21|_BIT20)
//#define TOP_IN_CK_TX_3			(_BIT3)
#define TOP_IN_CK_TX_2			(_BIT2)
#define TOP_IN_CK_TX_1			(_BIT1)
#define TOP_IN_CK_TX_0			(_BIT0)

#define TOP_IN_Z0_FIX_SELECT_0 (_BIT9|_BIT8)
#define TOP_IN_Z0_FIX_SELECT_1 (_BIT11|_BIT10)
#define TOP_IN_Z0_FIX_SELECT_2 (_BIT13|_BIT12)
#define TOP_IN_Z0_FIX_SELECT_3 (_BIT15|_BIT14)
#define TOP_IN_Z0_FIX_SELECT_10 (_BIT9)

#define TOP_IN_BIAS_POW (_BIT16)



#define  p0_REG_ACDR_CPCVM_EN                     (_BIT26)
#define  p0_REG_ACDR_VCO_TUNER                   (_BIT23|_BIT22)
#define  p0_REG_ACDR_VCOGAIN                       (_BIT21|_BIT20)
#define  p0_REG_ACDR_LFRS_SEL                      (_BIT15|_BIT14|_BIT13)
#define  p0_REG_ACDR_CKFLD_EN                     (_BIT5)
#define  p0_REG_ACDR_HR_PD	                   (_BIT4)
#define  p0_REG_ACDR_EN                                 (_BIT1)
#define  p0_REG_ACDR_TOP_EN                         (_BIT0)



//////////////////////////////////////// B lane //////////////////////////////////////////////////////////


#define  P0_b_4_FORE_KOFF_AUTOK                                                                    (_BIT28)
#define  P0_b_4_FORE_POW_FORE_KOFF                                                                  (_BIT24)
#define  P0_b_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)
#define  P0_b_1_FORE_KOFF_RANGE                                                                    (_BIT22|_BIT21)
#define  P0_b_1_inputoff_p				       		(_BIT2)
#define  P0_b_1_inputoff_n				       		(_BIT1)
#define  P0_b_1_inputoff                                                                      		(_BIT0)

#define  P0_b_11_transition_cnt_en										     (_BIT20)
#define  P0_b_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)
#define  P0_b_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28)
#define  P0_b_8_POW_PR                                                                                            (_BIT25)
#define  P0_b_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P0_b_7_TAP0_ISEL                                                                                         (_BIT21|_BIT22|_BIT23)
#define  P0_b_7_TAP0_ISEL_400uA                                                                              (_BIT22|_BIT23)
#define  P0_b_7_LEQ_ISEL                                                                                           (_BIT18|_BIT19|_BIT20)
#define  P0_b_7_LEQ_ISEL_400uA                                                                                (_BIT19|_BIT20)
#define  P0_b_7_LEQ_ISEL_200uA                                                                                (_BIT20)
#define  P0_b_7_PI_ISEL                                                                                              (_BIT24|_BIT17|_BIT16)
#define  P0_b_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)
#define  P0_b_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)
#define  P0_b_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)
#define  P0_b_6_TAP0_HBR                                                                              	        (_BIT13)
#define  P0_b_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
#define  P0_b_6_TAP0_LE_ISO							(_BIT8)
#define  P0_b_5_LEQ_CURRENT_ADJ                                                                            (_BIT3|_BIT4)
#define  P0_b_5_LEQ_CURRENT_ADJ_1X                                                                            (_BIT3)
#define  P0_b_5_LEQ_CURRENT_ADJ_1p25X                                                                       (_BIT4)
#define  P0_b_5_EQ_POW                                                                                             (_BIT0)


//#define P0_B_DFE_TAPEN4321                                                                              (_BIT6|_BIT5|_BIT4|_BIT3)
#define P0_b_12_PI_CURRENT														(_BIT28)
#define P0_b_12_PI_CSEL 														 (_BIT24|_BIT25)
#define P0_b_9_DFE_ADAPTION_POW_EN 								              (_BIT7)
#define P0_b_9_DFE_TAP_EN 								                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define P0_b_9_DFE_TAP1_EN 								                           (_BIT3)
#define DFE_HDMI_RX_PHY_P0_DFE_ADAPTION_EN(data)							(0x00000080&((data)<<7))
#define DFE_HDMI_RX_PHY_P0_b_tap_en(data)								(0x00000078&((data)<<3))
#define P0_b_11_ACDR_RATE_SEL_HALF_RATE 								 (_BIT16)
#define P0_b_10_DFE_SUMAMP_LP_MANUALOFF								 (_BIT15)
#define P0_b_10_DFE_SUMAMP_ISEL_mask 								              (_BIT11|_BIT12|_BIT13)
#define P0_b_10_DFE_SUMAMP_ISEL_400uA 								              (_BIT13)
#define P0_b_10_DFE_PREAMP_ISEL_mask 								                    (_BIT8|_BIT9|_BIT10)
#define P0_b_10_DFE_PREAMP_ISEL_400uA 								                    (_BIT10)


//////////////////////////////////////// CK lane //////////////////////////////////////////////////////////

#define P0_LDO10V_CMU_EN					(_BIT7)

#define  p0_ck_4_SEL_SOFT_ON_OFF_TIMER 	(_BIT25|_BIT24)
#define  p0_ck_4_SEL_SOFT_ON_OFF 			(_BIT26)
#define  p0_ck_4_POW_SOFT_ON_OFF			(_BIT27)


#define  p0_ck_3_CMU_PREDIVN                                                                          (_BIT20|_BIT21|_BIT22|_BIT23)
#define  p0_ck_3_CMU_N_code                                                                          (_BIT18|_BIT19)
#define  p0_ck_3_CMU_N_code_SEL                                                                          (_BIT18)
#define  p0_ck_3_CMU_SEL_D4										 (_BIT16)
#define  p0_ck_2_CMU_CKIN_SEL                                                                   (_BIT15)
#define  p0_ck_2_ACDR_CBUS_REF										(_BIT13|_BIT14)
#define  p0_ck_2_CK_MD_DET_SEL										(_BIT12)

//the same as hdmi2.1 
#define  p0_ck_2_HYS_WIN_SEL								 (_BIT11|_BIT10)
#define  p0_ck_2_ENHANCE_BIAS_11							 (_BIT11|_BIT10)
#define  p0_ck_2_ENHANCE_BIAS_10							 (_BIT11)
#define  p0_ck_2_LDO_EN										 (_BIT11) //For Ma7p
#define  p0_ck_2_ENHANCE_BIAS_01							 (_BIT10)
#define  p0_ck_2_ENHANCE_BIAS_00							 (0)

#define  p0_ck_2_CK_MD_REF_SEL                                                                   (_BIT9)
#define  p0_ck_2_NON_HYS_AMP_EN									   (_BIT8)  //CKD2S
#define  p0_ck_1_port_bias                                                                              (_BIT2)
#define  p0_ck_1_HYS_AMP_EN										    (_BIT1)

#define  DFE_HDMI_RX_PHY_P0_get_CMU_SEL_D4(data)                        ((0x00010000&(data))>>16)

#define P0_ck_11_transition_cnt_en									   (_BIT20)
#define P0_ck_8_LDO_EN                                                                                  (_BIT25)
//#define  P0_ck_8_VSEL_LDO_A_mask                                                                   (_BIT29|_BIT30|_BIT31)  merlin5 remove
//#define  P0_ck_8_CCO_BAND_SEL_mask                                                                   (_BIT26|_BIT27|_BIT28)
#define P0_ck_8_ENC_mask												(_BIT31|_BIT30|_BIT29|_BIT28)  //cco band sel
//#define  P0_ck_8_LDO_EN_mask                                                                          (_BIT25)  //merlin5 remove
#define  P0_ck_8_CMU_BPPSR_mask                                                                          (_BIT24)
#define P0_ck_8_CMU_SEL_CP_mask										   (_BIT20|_BIT21)
#define  P0_ck_7_CMU_BYPASS_PI_mask                                                              (_BIT19)
#define  P0_ck_7_CMU_PI_I_SEL_mask                                                                 (_BIT16|_BIT17|_BIT18)
//#define  P0_ck_6_CMU_BIG_KVCO_mask                                                                 (_BIT13)  //merlin5 remove
#define  P0_ck_6_CMU_EN_CAP_mask                                                                     (_BIT12)
#define  P0_ck_6_CMU_SEL_CS_mask                                                                     (_BIT8|_BIT9|_BIT10)
#define  P0_ck_5_CMU_SEL_PUMP_I_mask                                                               (_BIT4|_BIT5|_BIT6|_BIT7)
#define  P0_ck_5_CMU_SEL_PUMP_DB_mask				(_BIT3)
#define  P0_ck_5_CMU_SEL_R1_mask                                                                       (_BIT0|_BIT1|_BIT2)
//#define  DFE_HDMI_RX_PHY_P0_VSEL_LDO(data)					   (0xE0000000&((data)<<29))
#define  DFE_HDMI_RX_PHY_P0_ENC(data)                            		    (0xF0000000&((data)<<28))
#define  DFE_HDMI_RX_PHY_P0_CMU_BPPSR(data)                            (0x01000000&((data)<<24))
#define  DFE_HDMI_RX_PHY_P0_CMU_SEL_CS(data)                            (0x00000700&((data)<<8))
#define  DFE_HDMI_RX_PHY_P0_CMU_SEL_PUMP_I(data)                            (0x000000F0&((data)<<4))
#define  DFE_HDMI_RX_PHY_P0_CMU_SEL_R1(data)                            (0x00000007&((data)<<0))
#define  DFE_HDMI_RX_PHY_P0_get_CMU_BPPSR(data)                        ((0x01000000&(data))>>24)


#define  P0_ck_9_CMU_PFD_RSTB                                                                          (_BIT5)
#define  P0_ck_9_CMU_WDRST                                                                          (_BIT2)




//////////////////////////////////////// G lane //////////////////////////////////////////////////////////

#define  P0_g_4_FORE_KOFF_AUTOK                                                                    (_BIT28)
#define  P0_g_4_FORE_POW_FORE_KOFF                                                                  (_BIT24)
#define  P0_g_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)
#define  P0_g_1_FORE_KOFF_RANGE                                                                    (_BIT22|_BIT21)
#define  P0_g_1_inputoff_p				       		(_BIT2)
#define  P0_g_1_inputoff_n				       		(_BIT1)
#define  P0_g_1_inputoff                                                                      		(_BIT0)



#define  P0_g_11_transition_cnt_en										     (_BIT20)
#define  P0_g_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)
#define  P0_g_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28)
#define  P0_g_8_POW_PR                                                                                            (_BIT25)
#define  P0_g_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P0_g_7_TAP0_ISEL                                                                                         (_BIT21|_BIT22|_BIT23)
#define  P0_g_7_TAP0_ISEL_400uA                                                                              (_BIT22|_BIT23)
#define  P0_g_7_LEQ_ISEL                                                                                           (_BIT18|_BIT19|_BIT20)
#define  P0_g_7_LEQ_ISEL_400uA                                                                                (_BIT19|_BIT20)
#define  P0_g_7_LEQ_ISEL_200uA                                                                                (_BIT20)
#define  P0_g_7_PI_ISEL                                                                                              (_BIT24|_BIT17|_BIT16)
#define  P0_g_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)
#define  P0_g_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)
#define  P0_g_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)
#define  P0_g_6_TAP0_HBR                                                                              	        (_BIT13)
#define  P0_g_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
#define  P0_g_5_LEQ_CURRENT_ADJ                                                                            (_BIT3|_BIT4)
#define  P0_g_5_LEQ_CURRENT_ADJ_1X                                                                            (_BIT3)
#define  P0_g_5_LEQ_CURRENT_ADJ_1p25X                                                                       (_BIT4)
#define  P0_g_5_EQ_POW                                                                                             (_BIT0)


#define P0_G_DFE_TAPEN4321 													(_BIT6|_BIT5|_BIT4|_BIT3)
#define P0_g_12_PI_CURRENT														(_BIT28)
#define P0_g_12_PI_CSEL 														 (_BIT24|_BIT25)
#define P0_g_9_DFE_ADAPTION_POW_EN 						              (_BIT7)
#define P0_g_9_DFE_TAP_EN 						                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define P0_g_9_DFE_TAP1_EN 						                           (_BIT3)
#define  DFE_HDMI_RX_PHY_P0_g_tap_en(data)                                                           (0x00000078&((data)<<3))
#define P0_g_11_ACDR_RATE_SEL_HALF_RATE 								 (_BIT16)
#define P0_g_10_DFE_SUMAMP_LP_MANUALOFF								 (_BIT15)
#define P0_g_10_DFE_SUMAMP_ISEL_mask 								              (_BIT11|_BIT12|_BIT13)
#define P0_g_10_DFE_SUMAMP_ISEL_400uA 								              (_BIT13)
#define P0_g_10_DFE_PREAMP_ISEL_mask 								                    (_BIT8|_BIT9|_BIT10)
#define P0_g_10_DFE_PREAMP_ISEL_400uA 								                    (_BIT10)

//////////////////////////////////////// R lane //////////////////////////////////////////////////////////

#define  P0_r_4_FORE_KOFF_AUTOK                                                                    (_BIT28)
#define  P0_r_4_FORE_POW_FORE_KOFF                                                                  (_BIT24)
#define  P0_r_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)
#define  P0_r_1_FORE_KOFF_RANGE                                                                    (_BIT22|_BIT21)
#define  P0_r_1_inputoff                                                                      		(_BIT0)
#define  P0_r_1_inputoff_n				       		(_BIT1)
#define  P0_r_1_inputoff_p				       		(_BIT2)


#define  P0_r_11_transition_cnt_en											 (_BIT20)
#define  P0_r_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)
#define  P0_r_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28)
#define  P0_r_8_POW_PR                                                                                            (_BIT25)
#define  P0_r_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P0_r_7_TAP0_ISEL                                                                                         (_BIT21|_BIT22|_BIT23)
#define  P0_r_7_TAP0_ISEL_400uA                                                                              (_BIT22|_BIT23)
#define  P0_r_7_LEQ_ISEL                                                                                           (_BIT18|_BIT19|_BIT20)
#define  P0_r_7_LEQ_ISEL_400uA                                                                                (_BIT19|_BIT20)
#define  P0_r_7_LEQ_ISEL_200uA                                                                                (_BIT20)
#define  P0_r_7_PI_ISEL                                                                                              (_BIT24|_BIT17|_BIT16)
#define  P0_r_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)
#define  P0_r_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)
#define  P0_r_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)
#define  P0_r_6_TAP0_HBR                                                                              	        (_BIT13)
#define  P0_r_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
#define  P0_r_5_LEQ_CURRENT_ADJ                                                                            (_BIT3|_BIT4)
#define  P0_r_5_LEQ_CURRENT_ADJ_1X                                                                            (_BIT3)
#define  P0_r_5_LEQ_CURRENT_ADJ_1p25X                                                                       (_BIT4)
#define  P0_r_5_EQ_POW                                                                                             (_BIT0)



#define  P0_R_DFE_TAPEN4321                                                                             (_BIT6|_BIT5|_BIT4|_BIT3)
#define P0_r_12_PI_CURRENT														(_BIT28)
#define  P0_r_12_PI_CSEL 														      (_BIT24|_BIT25)
#define P0_r_9_DFE_ADAPTION_POW_EN 								              (_BIT7)
#define P0_r_9_DFE_TAP_EN 								                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define P0_r_9_DFE_TAP1_EN 								                           (_BIT3)
#define  DFE_HDMI_RX_PHY_P0_r_tap_en(data)                                                           (0x00000078&((data)<<3))
#define  P0_r_11_ACDR_RATE_SEL 												(_BIT24|_BIT25)
#define P0_r_11_ACDR_RATE_SEL_HALF_RATE 								             (_BIT16)
#define P0_r_10_DFE_SUMAMP_LP_MANUALOFF								 (_BIT15)
#define P0_r_10_DFE_SUMAMP_ISEL_mask 								              (_BIT11|_BIT12|_BIT13)
#define P0_r_10_DFE_SUMAMP_ISEL_400uA 								              (_BIT13)
#define P0_r_10_DFE_PREAMP_ISEL_mask 								                    (_BIT8|_BIT9|_BIT10)
#define P0_r_10_DFE_PREAMP_ISEL_400uA 								                    (_BIT10)




#define  P0_b_1_FORE_KOFF_ADJR			(_BIT16|_BIT17|_BIT18|_BIT19|_BIT20)
#define  P0_b_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P0_b_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P0_b_2_LEQ_BIT_RATE_HBR			(_BIT9)
#define  P1_b_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P1_b_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P1_b_2_LEQ_BIT_RATE_HBR			(_BIT9)
#define  P2_b_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P2_b_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P2_b_2_LEQ_BIT_RATE_HBR			(_BIT9)


#define  P0_g_1_FORE_KOFF_ADJR			(_BIT16|_BIT17|_BIT18|_BIT19|_BIT20)
#define  P0_g_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P0_g_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P0_g_2_LEQ_BIT_RATE_HBR			(_BIT9)
#define  P1_g_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P1_g_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P1_g_2_LEQ_BIT_RATE_HBR			(_BIT9)
#define  P2_g_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P2_g_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P2_g_2_LEQ_BIT_RATE_HBR			(_BIT9)


#define  P0_r_1_FORE_KOFF_ADJR			(_BIT16|_BIT17|_BIT18|_BIT19|_BIT20)
#define  P0_r_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P0_r_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P0_r_2_LEQ_BIT_RATE_HBR			(_BIT9)
#define  P1_r_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P1_r_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P1_r_2_LEQ_BIT_RATE_HBR			(_BIT9)
#define  P2_r_2_LEQ_BIT_RATE_mask			(_BIT8|_BIT9)
#define  P2_r_2_LEQ_BIT_RATE_MBR			(_BIT8)
#define  P2_r_2_LEQ_BIT_RATE_HBR			(_BIT9)


#define PIX2_RATE_SEL  (_BIT26)
#define PIX2_RATE_DIV5  (_BIT26)
#define PIX2_RATE_DIV10  (0)





#define  P1_b_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P1_g_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P1_r_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P2_b_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P2_g_8_BY_PASS_PR                                                                                      (_BIT24)
#define  P2_r_8_BY_PASS_PR                                                                                      (_BIT24)





