#include <rbus/dfe_p0_reg.h>
#include <rbus/hdmirx_2p0_phy_reg.h>
#include <rbus/efuse_reg.h>




#if 0


#define HDMI20_PORT  HDMI_PORT2

//dfe_21
#define HDMI_DFE0_BASE           (0xb800DE00)
#define HDMI_DFE1_BASE           (0xb800DE50)


//dfe_20
#define HDMI_DFE2_BASE           (0xb803CD00)
#define HDMI_DFE3_BASE           (0xb803CD50)


struct hdmi_dfe_reg_st {
uint32_t en_l0;	//cd00
uint32_t init0_l0;  //cd04
uint32_t init1_l0;  //cd08
uint32_t dummy1;  //cd0c
uint32_t en_l1;    //cd10
uint32_t init0_l1;  //cd14
uint32_t init1_l1;  //cd18
uint32_t dummy2;  //cd1c
uint32_t en_l2;    //cd20
uint32_t init0_l2;  //cd24
uint32_t init1_l2;  //cd28
uint32_t dummy3;  //cd2c
uint32_t mode;  //cd30
uint32_t gain;  //cd34
uint32_t limit0;  //cd38
uint32_t limit1;  //cd3c
uint32_t limit2;  //cd40
uint32_t readback;  //cd44
uint32_t flag;  //cd48
uint32_t debug;  //cd4c
};



struct hdmi21_dfe_reg_st {
	uint32_t en_l0;    // de00
	uint32_t init0_l0; // de04
	uint32_t init1_l0; // de08
	uint32_t en_l1;   // de0c
	uint32_t init0_l1;  // de10
	uint32_t init1_l1;  // de14
	uint32_t en_l2;    // de18
	uint32_t init0_l2;  // de1c
	uint32_t init1_l2;  // de20
	uint32_t en_l3;     // de24
	uint32_t init0_l3;  // de28
	uint32_t init1_l3;  // de2c 
	uint32_t mode;    // de30
	uint32_t gain;     // de34
	uint32_t limit0;   // de38
	uint32_t limit1;   // de3c
	uint32_t limit2;   // de40
	uint32_t readback;  // de44
	uint32_t flag;         // de48
	uint32_t debug;    // de4c
};


extern const struct hdmi_dfe_reg_st *dfe[2];
extern const struct hdmi21_dfe_reg_st *hd21_dfe[2];  //hd20 & hd21 share


#if 0 //def CONFIG_ARM64
#define DFE_REG_DFE_EN_L0_reg                    ((nport<HDMI20_PORT)?(unsigned int)(unsigned long)(&hd21_dfe[0]->en_l0):(unsigned int)(unsigned long)(&dfe[nport]->en_l0))              // 0xB800DE00 DE50 CD00 CD50      
#define DFE_REG_DFE_INIT0_L0_reg               ((nport<HDMI20_PORT)?(unsigned int)(unsigned long)(&hd21_dfe[0]->init0_l0):(unsigned int)(unsigned long)(&dfe[nport]->init0_l0))              // 0xB800DE04 DE54 CD04 CD54  
#define DFE_REG_DFE_INIT1_L0_reg               ((nport<HDMI20_PORT)?(unsigned int)(unsigned long)(&hd21_dfe[0]->init1_l0):(unsigned int)(unsigned long)(&dfe[nport]->init1_l0))              // 0xB800DE08 DE58 CD08 CD58  
#define DFE_REG_DFE_EN_L1_reg                    ((nport<HDMI20_PORT)?(unsigned int)(unsigned long)(&hd21_dfe[0]->en_l1):(unsigned int)(unsigned long)(&dfe[nport]->en_l1))              // 0xB800DE0C DE5C CD10 CD60      
#define DFE_REG_DFE_INIT0_L1_reg               ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->init0_l1):(unsigned int)(unsigned long)(&dfe[nport]->init0_l1))              // 0xB800DE10 DE60  CD14 CD64
#define DFE_REG_DFE_INIT1_L1_reg               ((nport<HDMI20_PORT)?(unsigned int)(unsigned long)(&hd21_dfe[0]->init1_l1):(unsigned int)(unsigned long)(&dfe[nport]->init1_l1))              // 0xB800DE14 DE64 CD18 CD68  
#define DFE_REG_DFE_EN_L2_reg                    ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->en_l2 ): (unsigned int)(unsigned long)(&dfe[nport]->en_l2 ))              // 0xB800DE18 DE68 CD20 CD70  
#define DFE_REG_DFE_INIT0_L2_reg               ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->init0_l2):(unsigned int)(unsigned long)(&dfe[nport]->init0_l2))              // 0xB800DE1C DE6C CD24 CD74 
#define DFE_REG_DFE_INIT1_L2_reg               ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->init1_l2):(unsigned int)(unsigned long)(&dfe[nport]->init1_l2))              // 0xB800DE20 DE70 CD28 CD78 

#define DFE_REG_DFE_EN_L3_reg          (unsigned int)(unsigned long)(&hd21_dfe[nport]->en_l3)               // 0xB800DE24 DE74  
#define DFE_REG_DFE_INIT0_L3_reg     (unsigned int)(unsigned long)(&hd21_dfe[nport]->init0_l3)               // 0xB800DE28 DE78 
#define DFE_REG_DFE_INIT1_L3_reg     (unsigned int)(unsigned long)(&hd21_dfe[nport]->init1_l3)               // 0xB800DE2C DE7C 


#define DFE_REG_DFE_MODE_reg                     ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->mode):(unsigned int)(unsigned long)(&dfe[nport]->mode))              // 0xB800DE30 DE80 CD30 CD80         
#define DFE_REG_DFE_GAIN_reg                      ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->gain):(unsigned int)(unsigned long)(&dfe[nport]->gain    ))              // 0xB800DE34 DE84  CD34 CD84   
#define DFE_REG_DFE_LIMIT0_reg                   ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->limit0):(unsigned int)(unsigned long)(&dfe[nport]->limit0  ))              // 0xB800DE38 DE88  CD38 CD88  
#define DFE_REG_DFE_LIMIT1_reg                   ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->limit1) :(unsigned int)(unsigned long)(&dfe[nport]->limit1  ))              // 0xB800DE3C DE8C  CD3C CD8C 
#define DFE_REG_DFE_LIMIT2_reg                   ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->limit2  ):(unsigned int)(unsigned long)(&dfe[nport]->limit2  ))              // 0xB800DE40 DE90  CD40 CD90
#define DFE_REG_DFE_READBACK_reg              ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->readback): (unsigned int)(unsigned long)(&dfe[nport]->readback))              // 0xB800DE44 DE94 CD44 CD94   
#define DFE_REG_DFE_FLAG_reg                       ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->flag) :(unsigned int)(unsigned long)(&dfe[nport]->flag    ))              // 0xB800DE48 DE98 CD48 CD98  
#define DFE_REG_DFE_DEBUG_reg                    ((nport<HDMI20_PORT)? (unsigned int)(unsigned long)(&hd21_dfe[0]->debug) :(unsigned int)(unsigned long)(&dfe[nport]->debug   ))              // 0xB800DE4C DE9C CD4C CD9C   

#else
#define DFE_REG_DFE_EN_L0_reg                   ((nport<HDMI20_PORT)?(unsigned int)(&hd21_dfe[nport]->en_l0):(unsigned int)(&dfe[nport-HDMI20_PORT]->en_l0))            // 0xB800DE00 DE50 CD00 CD50
#define DFE_REG_DFE_INIT0_L0_reg               ((nport<HDMI20_PORT)?(unsigned int)(&hd21_dfe[nport]->init0_l0):(unsigned int)(&dfe[nport-HDMI20_PORT]->init0_l0))              // 0xB800DE04 DE54 CD04 CD54
#define DFE_REG_DFE_INIT1_L0_reg               ((nport<HDMI20_PORT)?(unsigned int)(&hd21_dfe[nport]->init1_l0):(unsigned int)(&dfe[nport-HDMI20_PORT]->init1_l0))              // 0xB800DE08 DE58 CD08 CD58
#define DFE_REG_DFE_EN_L1_reg                    ((nport<HDMI20_PORT)?(unsigned int)(&hd21_dfe[nport]->en_l1):(unsigned int)(&dfe[nport-HDMI20_PORT]->en_l1))              // 0xB800DE0C DE5C CD10 CD60
#define DFE_REG_DFE_INIT0_L1_reg               ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->init0_l1):(unsigned int)(&dfe[nport-HDMI20_PORT]->init0_l1))              // 0xB800DE10 DE60  CD14 CD64
#define DFE_REG_DFE_INIT1_L1_reg               ((nport<HDMI20_PORT)?(unsigned int)(&hd21_dfe[nport]->init1_l1):(unsigned int)(&dfe[nport-HDMI20_PORT]->init1_l1))              // 0xB800DE14 DE64 CD18 CD68
#define DFE_REG_DFE_EN_L2_reg                    ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->en_l2 ): (unsigned int)(&dfe[nport-HDMI20_PORT]->en_l2 ))              // 0xB800DE18 DE68 CD20 CD70
#define DFE_REG_DFE_INIT0_L2_reg               ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->init0_l2):(unsigned int)(&dfe[nport-HDMI20_PORT]->init0_l2))              // 0xB800DE1C DE6C CD24 CD74
#define DFE_REG_DFE_INIT1_L2_reg               ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->init1_l2):(unsigned int)(&dfe[nport-HDMI20_PORT]->init1_l2))              // 0xB800DE20 DE70 CD28 CD78

#define DFE_REG_DFE_EN_L3_reg          (unsigned int)(&hd21_dfe[nport]->en_l3)              // 0xB800DE24 DE74
#define DFE_REG_DFE_INIT0_L3_reg     (unsigned int)(&hd21_dfe[nport]->init0_l3)              // 0xB800DE28 DE78
#define DFE_REG_DFE_INIT1_L3_reg     (unsigned int)(&hd21_dfe[nport]->init1_l3)              // 0xB800DE2C DE7C


#define DFE_REG_DFE_MODE_reg                     ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->mode):(unsigned int)(&dfe[nport-HDMI20_PORT]->mode))              // 0xB800DE30 DE80 CD30 CD80
#define DFE_REG_DFE_GAIN_reg                      ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->gain):(unsigned int)(&dfe[nport-HDMI20_PORT]->gain    ))              // 0xB800DE34 DE84  CD34 CD84
#define DFE_REG_DFE_LIMIT0_reg                   ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->limit0):(unsigned int)(&dfe[nport-HDMI20_PORT]->limit0  ))              // 0xB800DE38 DE88  CD38 CD88
#define DFE_REG_DFE_LIMIT1_reg                   ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->limit1) :(unsigned int)(&dfe[nport-HDMI20_PORT]->limit1  ))              // 0xB800DE3C DE8C  CD3C CD8C
#define DFE_REG_DFE_LIMIT2_reg                   ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->limit2  ):(unsigned int)(&dfe[nport-HDMI20_PORT]->limit2  ))              // 0xB800DE40 DE90  CD40 CD90
#define DFE_REG_DFE_READBACK_reg              ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->readback): (unsigned int)(&dfe[nport-HDMI20_PORT]->readback))              // 0xB800DE44 DE94 CD44 CD94
#define DFE_REG_DFE_FLAG_reg                       ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->flag) :(unsigned int)(&dfe[nport-HDMI20_PORT]->flag    ))              // 0xB800DE48 DE98 CD48 CD98
#define DFE_REG_DFE_DEBUG_reg                    ((nport<HDMI20_PORT)? (unsigned int)(&hd21_dfe[nport]->debug) :(unsigned int)(&dfe[nport-HDMI20_PORT]->debug   ))              // 0xB800DE4C DE9C CD4C CD9C
#endif




#define HDMI_APLLCR1_dpll_CS_MASK                                        ((1<<18)|(1<<17))
#define HDMI_APLLCR1_dpll_CS_20P                                          (0)
#define HDMI_APLLCR1_dpll_CS_25P                                          (1<<17)
#define HDMI_APLLCR1_dpll_CS_30P                                          (2<<17)
#define HDMI_APLLCR1_dpll_CS_35P                                          (3<<17)


#define LOAD_IN_INIT_LE		(_BIT31)
#define LOAD_IN_INIT_VTH	(_BIT30)
#define LOAD_IN_INIT_SERVO	(_BIT29)
#define LOAD_IN_INIT_TAP4	(_BIT28)
#define LOAD_IN_INIT_TAP3	(_BIT27)
#define LOAD_IN_INIT_TAP2	(_BIT26)
#define LOAD_IN_INIT_TAP1	(_BIT25)
#define LOAD_IN_INIT_TAP0	(_BIT24)
#define LOAD_IN_INIT_ALL	(LOAD_IN_INIT_LE|LOAD_IN_INIT_VTH|LOAD_IN_INIT_SERVO|LOAD_IN_INIT_TAP4|LOAD_IN_INIT_TAP3|LOAD_IN_INIT_TAP2|LOAD_IN_INIT_TAP1|LOAD_IN_INIT_TAP0)


#define REG_dfe_adapt_en_lane0_TAP0                                         _BIT16
#define REG_dfe_adapt_en_lane0_TAP1                                         _BIT17
#define REG_dfe_adapt_en_lane0_TAP2                                         _BIT18
#define REG_dfe_adapt_en_lane0_TAP3                                         _BIT19
#define REG_dfe_adapt_en_lane0_TAP4                                         _BIT20
#define REG_dfe_adapt_en_lane0_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane0_Vth                                            _BIT22
#define REG_dfe_adapt_en_lane0_LE                                              _BIT23
#define REG_dfe_adapt_en_lane1_TAP0                                         _BIT16
#define REG_dfe_adapt_en_lane1_TAP1                                         _BIT17
#define REG_dfe_adapt_en_lane1_TAP2                                         _BIT18
#define REG_dfe_adapt_en_lane1_TAP3                                         _BIT19
#define REG_dfe_adapt_en_lane1_TAP4                                         _BIT20
#define REG_dfe_adapt_en_lane1_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane1_Vth                                            _BIT22
#define REG_dfe_adapt_en_lane1_LE                                              _BIT23
#define REG_dfe_adapt_en_lane2_TAP0                                         _BIT16
#define REG_dfe_adapt_en_lane2_TAP1                                         _BIT17
#define REG_dfe_adapt_en_lane2_TAP2                                         _BIT18
#define REG_dfe_adapt_en_lane2_TAP3                                         _BIT19
#define REG_dfe_adapt_en_lane2_TAP4                                         _BIT20
#define REG_dfe_adapt_en_lane2_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane2_Vth                                            _BIT22
#define REG_dfe_adapt_en_lane2_LE                                              _BIT23
#define REG_dfe_adapt_en_lane3_TAP0                                         _BIT16
#define REG_dfe_adapt_en_lane3_TAP1                                         _BIT17
#define REG_dfe_adapt_en_lane3_TAP2                                         _BIT18
#define REG_dfe_adapt_en_lane3_TAP3                                         _BIT19
#define REG_dfe_adapt_en_lane3_TAP4                                         _BIT20
#define REG_dfe_adapt_en_lane3_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane3_Vth                                            _BIT22
#define REG_dfe_adapt_en_lane3_LE                                              _BIT23


#define LIMIT_TAP0_MAX				0
#define LIMIT_TAP0_MIN				1
#define LIMIT_TAP1_MAX				2
#define LIMIT_TAP1_MIN				3
#define LIMIT_LE_MAX				4
#define LIMIT_LE_MIN				5

#define COEF_VTH				0
#define COEF_TAP0				1
#define COEF_TAP1				2
#define COEF_TAP2				3
#define COEF_TAP3				4
#define COEF_TAP4				5
#define COEF_SERVO				6
#define COEF_LE					7


//#define limit_set_lane_mask					(_BIT30|_BIT29)
//#define limit_set_lane_shift					(29)
//#define limit_set_mask						(_BIT28|_BIT27|_BIT26|_BIT25|_BIT24)
//#define limit_sel(data)  						(limit_set_mask&((data)<<24))
//#define get_Tap0_max(data)                          	((0x7F0000&(data))>>16)
//#define get_Tap0_min(data)                          	((0x7F0000&(data))>>16)
//#define get_Tap1_max(data)                          	((0x3F0000&(data))>>16)
//#define get_Tap1_min(data)                          	((0x3F0000&(data))>>16)
//#define get_LEQ_max(data)                          	((0x1F0000&(data))>>16)
//#define get_LEQ_min(data)                          	((0x1F0000&(data))>>16)
//#define coef_set_lane_mask					(_BIT15|_BIT14)
//#define coef_set_lane_shift					(14)
//#define coef_set_mask						(_BIT13|_BIT12|_BIT11|_BIT10|_BIT9|_BIT8)
//#define coef_sel(data)                               		(coef_set_mask&((data)<<8))
//#define get_VTH_coef(data)                          	(0xF&(data))
//#define get_TAP0_coef(data)                          	(0x1F&(data))
//#define get_TAP1_coef(data)                          	(0x3F&(data))
#define get_TAP2_coef_sign(data)                   	(0x20&(data))
#define get_TAP2_coef(data)                          	(0x1F&(data))
//#define get_TAP3_coef_sign(data)                    	(0x20&(data))
//#define get_TAP3_coef(data)                          	(0x1F&(data))
//#define get_TAP4_coef_sign(data)                     (0x20&(data))
//#define get_TAP4_coef(data)                          	(0x1F&(data))
//#define get_SERVO_coef(data)                          	(0x1F&(data))
//#define get_LE1_coef(data)                          	(0x1F&(data))
//#define get_CurrentStep_coef(data) 			((0x30&(data))>>4)
//#define get_BufferMode_coef(data) 			((0xc0&(data))>>6)
#endif

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
#define  p0_ck_3_CMU_SEL_D4										 (_BIT16)
#define  p0_ck_2_CMU_CKIN_SEL                                                                   (_BIT15)
#define  p0_ck_2_ACDR_CBUS_REF										(_BIT13|_BIT14)
#define  p0_ck_2_CK_MD_DET_SEL										(_BIT12)

//the same as hdmi2.1 
#define  p0_ck_2_HYS_WIN_SEL								 (_BIT11|_BIT10)
#define  p0_ck_2_ENHANCE_BIAS_11							 (_BIT11|_BIT10)
#define  p0_ck_2_ENHANCE_BIAS_10							 (_BIT11)
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





