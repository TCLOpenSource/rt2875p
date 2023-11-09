#include <rbus/hdmirx_2p1_phy_reg.h>
#include <rbus/efuse_reg.h>

#define HDMI_APHY0_BASE	    (0xb800DA20)   //DA20 ~ DAEC (P0)   
#define HDMI_APHY1_BASE	    (0xb800DB20)  //DB20 ~ DBEC(P1)

#define HDMI_DPHY0_BASE	    (0xb800DC00)   //DC00 ~ DCD8 (P0) 
#define HDMI_DPHY1_BASE	    (0xb800DD00)   // DD00 ~ DDD8(P1) 


struct hdmi21_aphy_reg_st {
	uint32_t top_out_0;  //DA20
	uint32_t top_out_1; //DA24
	uint32_t koffset;  //DA28
	uint32_t ck_cmu;  //DA2C
	uint32_t ck0; //DA30
	uint32_t ck1; //DA34
	uint32_t ck2; //DA38
	uint32_t ck3; //DA3C
	uint32_t ck4; //DA40
	uint32_t acdr_ck0;  //DA44
	uint32_t acdr_ck1;  //DA48
	uint32_t acdr_ck2;  //DA4C
	uint32_t b0;  //DA50
	uint32_t b1;  //DA54
	uint32_t b2;  //DA58
	uint32_t b3;  //DA5C
	uint32_t b4;  //DA60
	uint32_t acdr_b0;  //DA64
	uint32_t acdr_b1;  //DA68
	uint32_t acdr_b2;  //DA6C
	uint32_t g0;  //DA70
	uint32_t g1;  //DA74
	uint32_t g2;  //DA78
	uint32_t g3;  //DA7C
	uint32_t g4;  //DA80
	uint32_t acdr_g0;  //DA84
	uint32_t acdr_g1;  //DA88
	uint32_t acdr_g2;  //DA8C
	uint32_t r0;  //DA90
	uint32_t r1;  //DA94
	uint32_t r2;  //DA98
	uint32_t r3;  //DA9C
	uint32_t r4;  //DAA0
	uint32_t acdr_r0;  //DAA4
	uint32_t acdr_r1;  //DAA8
	uint32_t acdr_r2;  //DAAC
	uint32_t hd21_y1_rst;  //DAB0

	uint32_t rsvd1;  //DAB4;
	uint32_t rsvd2;  //DAB8
	uint32_t rsvd3;  //DABC
	
	
	uint32_t acdr_r_pll_cfg;  //DAC0
	uint32_t acdr_g_pll_cfg;  //DAC4
	uint32_t acdr_b_pll_cfg;  //DAC8
	uint32_t acdr_ck_pll_cfg;  //DACC
	uint32_t acdr_r_cdr_cfg;  //DAD0
	uint32_t acdr_g_cdr_cfg;  //DAD4
	uint32_t acdr_b_cdr_cfg;  //DAD8
	uint32_t acdr_ck_cdr_cfg;  //DADC
	uint32_t acdr_r_manual_cfg;  //DAE0
	uint32_t acdr_g_manual_cfg;  //DAE4
	uint32_t acdr_b_manual_cfg;  //DAE8
	uint32_t acdr_ck_manual_cfg;  //DAEC
};


extern const struct hdmi21_aphy_reg_st *hd21_aphy[2];

#define  HDMIRX_PHY_TOP_OUT_0_reg		(unsigned long)(&hd21_aphy[nport]->top_out_0)
#define  HDMIRX_PHY_TOP_OUT_1_reg		(unsigned long)(&hd21_aphy[nport]->top_out_1)
#define  HDMIRX_PHY_KOFFSET_reg		(unsigned long)(&hd21_aphy[nport]->koffset)
#define  HDMIRX_PHY_CK_CMU_reg			(unsigned long)(&hd21_aphy[nport]->ck_cmu)

#define HDMIRX_PHY_CK0_reg				  (unsigned long)(&hd21_aphy[nport]->ck0)
#define HDMIRX_PHY_CK1_reg				  (unsigned long)(&hd21_aphy[nport]->ck1)
#define HDMIRX_PHY_CK2_reg				  (unsigned long)(&hd21_aphy[nport]->ck2)
#define HDMIRX_PHY_CK3_reg				  (unsigned long)(&hd21_aphy[nport]->ck3)
#define HDMIRX_PHY_CK4_reg				  (unsigned long)(&hd21_aphy[nport]->ck4)
#define HDMIRX_PHY_ACDR_CK0_reg             (unsigned long)(&hd21_aphy[nport]->acdr_ck0)
#define HDMIRX_PHY_ACDR_CK1_reg             (unsigned long)(&hd21_aphy[nport]->acdr_ck1)
#define HDMIRX_PHY_ACDR_CK2_reg             (unsigned long)(&hd21_aphy[nport]->acdr_ck2)


#define HDMIRX_PHY_B0_reg				  (unsigned long)(&hd21_aphy[nport]->b0)
#define HDMIRX_PHY_B1_reg				  (unsigned long)(&hd21_aphy[nport]->b1)
#define HDMIRX_PHY_B2_reg				  (unsigned long)(&hd21_aphy[nport]->b2)
#define HDMIRX_PHY_B3_reg				  (unsigned long)(&hd21_aphy[nport]->b3)
#define HDMIRX_PHY_B4_reg				  (unsigned long)(&hd21_aphy[nport]->b4)
#define HDMIRX_PHY_ACDR_B0_reg               (unsigned long)(&hd21_aphy[nport]->acdr_b0)
#define HDMIRX_PHY_ACDR_B1_reg               (unsigned long)(&hd21_aphy[nport]->acdr_b1)
#define HDMIRX_PHY_ACDR_B2_reg               (unsigned long)(&hd21_aphy[nport]->acdr_b2)


#define HDMIRX_PHY_G0_reg				  (unsigned long)(&hd21_aphy[nport]->g0)
#define HDMIRX_PHY_G1_reg				  (unsigned long)(&hd21_aphy[nport]->g1)
#define HDMIRX_PHY_G2_reg				  (unsigned long)(&hd21_aphy[nport]->g2)
#define HDMIRX_PHY_G3_reg				  (unsigned long)(&hd21_aphy[nport]->g3)
#define HDMIRX_PHY_G4_reg				  (unsigned long)(&hd21_aphy[nport]->g4)
#define HDMIRX_PHY_ACDR_G0_reg               (unsigned long)(&hd21_aphy[nport]->acdr_g0)
#define HDMIRX_PHY_ACDR_G1_reg               (unsigned long)(&hd21_aphy[nport]->acdr_g1)
#define HDMIRX_PHY_ACDR_G2_reg               (unsigned long)(&hd21_aphy[nport]->acdr_g2)

#define HDMIRX_PHY_R0_reg				  (unsigned long)(&hd21_aphy[nport]->r0)
#define HDMIRX_PHY_R1_reg				  (unsigned long)(&hd21_aphy[nport]->r1)
#define HDMIRX_PHY_R2_reg				  (unsigned long)(&hd21_aphy[nport]->r2)
#define HDMIRX_PHY_R3_reg				  (unsigned long)(&hd21_aphy[nport]->r3)
#define HDMIRX_PHY_R4_reg				  (unsigned long)(&hd21_aphy[nport]->r4)
#define HDMIRX_PHY_ACDR_R0_reg               (unsigned long)(&hd21_aphy[nport]->acdr_r0)
#define HDMIRX_PHY_ACDR_R1_reg               (unsigned long)(&hd21_aphy[nport]->acdr_r1)
#define HDMIRX_PHY_ACDR_R2_reg               (unsigned long)(&hd21_aphy[nport]->acdr_r2)

#define HDMIRX_PHY_Y1_RST_reg			  		(unsigned long)(&hd21_aphy[nport]->hd21_y1_rst)

#define HDMIRX_PHY_ACDR_R_PLL_CONFIG_reg		(unsigned long)(&hd21_aphy[nport]->acdr_r_pll_cfg)
#define HDMIRX_PHY_ACDR_G_PLL_CONFIG_reg		(unsigned long)(&hd21_aphy[nport]->acdr_g_pll_cfg)
#define HDMIRX_PHY_ACDR_B_PLL_CONFIG_reg		(unsigned long)(&hd21_aphy[nport]->acdr_b_pll_cfg)
#define HDMIRX_PHY_ACDR_CK_PLL_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_ck_pll_cfg)


#define HDMIRX_PHY_ACDR_R_CDR_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_r_cdr_cfg)
#define HDMIRX_PHY_ACDR_G_CDR_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_g_cdr_cfg)
#define HDMIRX_PHY_ACDR_B_CDR_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_b_cdr_cfg)
#define HDMIRX_PHY_ACDR_CK_CDR_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_ck_cdr_cfg)


#define HDMIRX_PHY_ACDR_R_MANUAL_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_r_manual_cfg)
#define HDMIRX_PHY_ACDR_G_MANUAL_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_g_manual_cfg)
#define HDMIRX_PHY_ACDR_B_MANUAL_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_b_manual_cfg)
#define HDMIRX_PHY_ACDR_CK_MANUAL_CONFIG_reg	(unsigned long)(&hd21_aphy[nport]->acdr_ck_manual_cfg)


struct hdmi21_dphy_reg_st {
	uint32_t regd00;  //DC00
	uint32_t regd01;  //DC04
	uint32_t regd02;  //DC08
	uint32_t regd03;  //DC0C
	uint32_t regd04;  //DC10
	uint32_t regd05;  //DC14
	uint32_t regd06;  //DC18
	uint32_t regd07;  //DC1C
	uint32_t regd08;  //DC20
	uint32_t regd09;  //DC24
	uint32_t fld_r_regd00;  //DC28
	uint32_t fld_r_regd01;  //DC2C
	uint32_t fld_r_regd02;  //DC30
	uint32_t fld_r_regd03;  //DC34
	uint32_t fld_r_regd04;  //DC38
	uint32_t fld_r_regd05;  //DC3C
	uint32_t fld_r_regd06;  //DC40

	uint32_t fld_g_regd00;  //DC44
	uint32_t fld_g_regd01;  //DC48
	uint32_t fld_g_regd02;  //DC4C
	uint32_t fld_g_regd03;  //DC50
	uint32_t fld_g_regd04;  //DC54
	uint32_t fld_g_regd05;  //DC58
	uint32_t fld_g_regd06;  //DC5C

	uint32_t fld_b_regd00;  //DC60
	uint32_t fld_b_regd01;  //DC64
	uint32_t fld_b_regd02;  //DC68
	uint32_t fld_b_regd03;  //DC6C
	uint32_t fld_b_regd04;  //DC70
	uint32_t fld_b_regd05;  //DC74
	uint32_t fld_b_regd06;  //DC78

	uint32_t fld_ck_regd00; //DC7C
	uint32_t fld_ck_regd01; //DC80
	uint32_t fld_ck_regd02; //DC84
	uint32_t fld_ck_regd03; //DC88
	uint32_t fld_ck_regd04; //DC8C
	uint32_t fld_ck_regd05; //DC90
	uint32_t fld_ck_regd06; //DC94
	
	uint32_t misc_0; //DC98
	
	uint32_t regd10;  //DC9C
	uint32_t regd11;  //DCA0
	uint32_t regd12;  //DCA4
	uint32_t regd13;  //DCA8
	uint32_t regd14;  //DCAC
	uint32_t regd15;  //DCB0
	uint32_t regd16;  //DCB4
	uint32_t regd17;  //DCB8
	uint32_t regd18;  //DCBC
	uint32_t regd19;  //DCC0
	uint32_t regd20;  //DCC4
	uint32_t regd21;  //DCC8
	uint32_t regd22;  //DCCC
	uint32_t regd23;  //DCD0
	uint32_t regd24;  //DCD4
	uint32_t regd25;  //DCD8
	
};


extern const struct hdmi21_dphy_reg_st *hd21_dphy[2];

#define HDMIRX_PHY_REGD00_reg	(unsigned long)(&hd21_dphy[nport]->regd00)
#define HDMIRX_PHY_REGD01_reg	(unsigned long)(&hd21_dphy[nport]->regd01)
#define HDMIRX_PHY_REGD02_reg	(unsigned long)(&hd21_dphy[nport]->regd02)
#define HDMIRX_PHY_REGD03_reg	(unsigned long)(&hd21_dphy[nport]->regd03)
#define HDMIRX_PHY_REGD04_reg	(unsigned long)(&hd21_dphy[nport]->regd04)
#define HDMIRX_PHY_REGD05_reg	(unsigned long)(&hd21_dphy[nport]->regd05)
#define HDMIRX_PHY_REGD06_reg	(unsigned long)(&hd21_dphy[nport]->regd06)
#define HDMIRX_PHY_REGD07_reg	(unsigned long)(&hd21_dphy[nport]->regd07)
#define HDMIRX_PHY_REGD08_reg	(unsigned long)(&hd21_dphy[nport]->regd08)
#define HDMIRX_PHY_REGD09_reg	(unsigned long)(&hd21_dphy[nport]->regd09)


#define HDMIRX_PHY_FLD_R_REGD00_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd00)
#define HDMIRX_PHY_FLD_R_REGD01_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd01)
#define HDMIRX_PHY_FLD_R_REGD02_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd02)
#define HDMIRX_PHY_FLD_R_REGD03_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd03)
#define HDMIRX_PHY_FLD_R_REGD04_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd04)
#define HDMIRX_PHY_FLD_R_REGD05_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd05)
#define HDMIRX_PHY_FLD_R_REGD06_reg	(unsigned long)(&hd21_dphy[nport]->fld_r_regd06)


#define HDMIRX_PHY_FLD_G_REGD00_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd00)
#define HDMIRX_PHY_FLD_G_REGD01_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd01)
#define HDMIRX_PHY_FLD_G_REGD02_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd02)
#define HDMIRX_PHY_FLD_G_REGD03_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd03)
#define HDMIRX_PHY_FLD_G_REGD04_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd04)
#define HDMIRX_PHY_FLD_G_REGD05_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd05)
#define HDMIRX_PHY_FLD_G_REGD06_reg	(unsigned long)(&hd21_dphy[nport]->fld_g_regd06)

#define HDMIRX_PHY_FLD_B_REGD00_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd00)
#define HDMIRX_PHY_FLD_B_REGD01_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd01)
#define HDMIRX_PHY_FLD_B_REGD02_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd02)
#define HDMIRX_PHY_FLD_B_REGD03_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd03)
#define HDMIRX_PHY_FLD_B_REGD04_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd04)
#define HDMIRX_PHY_FLD_B_REGD05_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd05)
#define HDMIRX_PHY_FLD_B_REGD06_reg	(unsigned long)(&hd21_dphy[nport]->fld_b_regd06)

#define HDMIRX_PHY_FLD_CK_REGD00_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd00)
#define HDMIRX_PHY_FLD_CK_REGD01_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd01)
#define HDMIRX_PHY_FLD_CK_REGD02_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd02)
#define HDMIRX_PHY_FLD_CK_REGD03_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd03)
#define HDMIRX_PHY_FLD_CK_REGD04_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd04)
#define HDMIRX_PHY_FLD_CK_REGD05_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd05)
#define HDMIRX_PHY_FLD_CK_REGD06_reg	(unsigned long)(&hd21_dphy[nport]->fld_ck_regd06)


#define HDMIRX_PHY_MISC_0_reg  		(unsigned long)(&hd21_dphy[nport]->misc_0)


#define HDMIRX_PHY_REGD10_reg	(unsigned long)(&hd21_dphy[nport]->regd10)
#define HDMIRX_PHY_REGD11_reg	(unsigned long)(&hd21_dphy[nport]->regd11)
#define HDMIRX_PHY_REGD12_reg	(unsigned long)(&hd21_dphy[nport]->regd12)
#define HDMIRX_PHY_REGD13_reg	(unsigned long)(&hd21_dphy[nport]->regd13)
#define HDMIRX_PHY_REGD14_reg	(unsigned long)(&hd21_dphy[nport]->regd14)
#define HDMIRX_PHY_REGD15_reg	(unsigned long)(&hd21_dphy[nport]->regd15)
#define HDMIRX_PHY_REGD16_reg	(unsigned long)(&hd21_dphy[nport]->regd16)
#define HDMIRX_PHY_REGD17_reg	(unsigned long)(&hd21_dphy[nport]->regd17)
#define HDMIRX_PHY_REGD18_reg	(unsigned long)(&hd21_dphy[nport]->regd18)
#define HDMIRX_PHY_REGD19_reg	(unsigned long)(&hd21_dphy[nport]->regd19)
#define HDMIRX_PHY_REGD20_reg	(unsigned long)(&hd21_dphy[nport]->regd20)
#define HDMIRX_PHY_REGD21_reg	(unsigned long)(&hd21_dphy[nport]->regd21)
#define HDMIRX_PHY_REGD22_reg	(unsigned long)(&hd21_dphy[nport]->regd22)
#define HDMIRX_PHY_REGD23_reg	(unsigned long)(&hd21_dphy[nport]->regd23)
#define HDMIRX_PHY_REGD24_reg	(unsigned long)(&hd21_dphy[nport]->regd24)
#define HDMIRX_PHY_REGD25_reg	(unsigned long)(&hd21_dphy[nport]->regd25)







#define IB_LEQ_ADJ_L					(_BIT13|_BIT12|_BIT11)
#define IB_LEQ_ADJ_L_001				(_BIT11)
#define IPTAT_LEQ_ADJ_L					(_BIT10|_BIT9|_BIT8)
#define IPTAT_LEQ_ADJ_L_101				(_BIT10|_BIT8)

#define CK_CMP							(_BIT6)  //h5    //standby clock
#define Z0_P_OFF						(_BIT1)  //h5
#define Z0_N_OFF						(_BIT0)   //h5

//#define TOP_IN_REG_IBHN_TUNE		(_BIT21|_BIT20)


#define HD21_TOP_IN_CK_TX_1			(_BIT12)
#define HD21_TOP_IN_CK_TX_0			(_BIT4)

#define HD21_TOP_IN_Z0_FIX_SELECT_0 (_BIT3|_BIT2)
#define HD21_TOP_IN_Z0_FIX_SELECT_1 (_BIT5|_BIT4)
#define HD21_TOP_IN_Z0_FIX_SELECT_10 (_BIT3)


#define TOP_IN_7_P1_FS_SIGNAL_DLY_TIME	(_BIT20|_BIT19|_BIT18)
#define TOP_IN_7_P1_FS_SIGNAL_DLY_TIME_100	(_BIT20)
#define TOP_IN_7_P1_FS_SIGNAL_DLY	(_BIT17)
#define TOP_IN_7_P1_FS_SIGNAL_SEL	(_BIT16)

#define TOP_IN_8_P0_FS_SIGNAL_DLY_TIME	(_BIT28|_BIT27|_BIT26)
#define TOP_IN_8_P0_FS_SIGNAL_DLY_TIME_100	(_BIT28)
#define TOP_IN_8_P0_FS_SIGNAL_DLY	(_BIT25)
#define TOP_IN_8_P0_FS_SIGNAL_SEL	(_BIT24)


#define BG_ENVBGUP					(_BIT17)      //h5
#define BG_RBG2					(_BIT15|_BIT14)   //h5
#define BG_RBG2_DEFAULT			(_BIT15)       //h5
#define BG_RBG					(_BIT13|_BIT12|_BIT11)    //h5
#define BG_RBG_DEFAULT			(_BIT13)   //h5
#define BG_RBGLOOP2				(_BIT10|_BIT9|_BIT8)  //h5
#define BG_RBGLOOP2_DEFAULT		(_BIT10)    //h5
#define Port1_Hysteresis_Window	(_BIT27|_BIT26)   //h5
#define Port0_Hysteresis_Window	(_BIT25|_BIT24)   //h5
#define CK_detect_DIV8_enable	(_BIT6)   //h5
#define CK_LATCH					(_BIT1)   //h5
#define BIAS_EN					(_BIT0)   //h5

//#define TOP_IN_BIAS_POW (_BIT16)



#define  REG_ACDR_CPCVM_EN                     (_BIT26)
#define  REG_ACDR_VCO_TUNER                   (_BIT23|_BIT22)
#define  REG_ACDR_VCOGAIN                       (_BIT21|_BIT20)
#define  REG_ACDR_LFRS_SEL                      (_BIT15|_BIT14|_BIT13)
#define  REG_ACDR_CKFLD_EN                     (_BIT5)
#define  REG_ACDR_HR_PD	                   (_BIT4)
#define  REG_ACDR_EN                                 (_BIT1)
#define  REG_ACDR_TOP_EN                         (_BIT0)



//////////////////////////////////////// B lane //////////////////////////////////////////////////////////

#define  b_4_FORE_KOFF_RANGE                                                                    (_BIT30|_BIT29) //h5
#define  b_4_FORE_KOFF_AUTOK                                                                    (_BIT28)  //h5
#define  b_4_BBPD_RSTB												(_BIT25)  //h5
#define  b_4_FORE_POW_FORE_KOFF                                                                  (_BIT24) //h5
#define  b_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)  //h5

#define  b_2_RXVCM_SEL							(_BIT12|_BIT11)		//h5
#define b_2_RXVCM_10							(_BIT12)   //h5

#define  b_1_OFFCAL_SEL							(_BIT7|_BIT6|_BIT5)	//h5
#define  b_1_OFFCAL_DA_E						(0)					//h5
#define  b_1_OFFCAL_DA_O						(_BIT5)				//h5
#define  b_1_OFFCAL_EG_E						(_BIT6)				//h5
#define  b_1_OFFCAL_EG_O						(_BIT6|_BIT5)		//h5
#define  b_1_OFFCAL_OP_E						(_BIT7)				//h5
#define  b_1_OFFCAL_OP_O						(_BIT7|_BIT5)		//h5
#define  b_1_OFFCAL_ON_E						(_BIT7|_BIT6)		//h5
#define  b_1_OFFCAL_ON_O						(_BIT7|_BIT6|_BIT5)	//h5


#define  b_1_inputoff_p				       		(_BIT2)  //h5
#define  b_1_inputoff_n				       		(_BIT1)  //h5
#define  b_1_inputoff                                                                      		(_BIT0)  //h5

#define  b_12_FR_CK_SEL							(_BIT31) //h5	
#define  b_12_PIX_RATE_SEL						(_BIT30|_BIT29)   //h5

#define  b_12_PI_CSEL							(_BIT25|_BIT24)   //h5


#define  b_10_DCVS_EN												     (_BIT15) //h5
#define  b_10_DFE_SUPAMP_DCGAIN_MAX									     (_BIT14) //h5
#define  b_10_DFE_SUMAMP_ISEL											 (_BIT13|_BIT12|_BIT11)  //h5
#define  b_10_DFE_SUMAMP_ISEL_101										 (_BIT13|_BIT11)	//h5
#define  b_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)    //h5
#define  b_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28)  //h5
#define  b_8_POW_PR                                                                                            (_BIT25)  //h5
#define  b_8_BY_PASS_PR                                                                                      (_BIT24)
#define  b_7_PI_ISEL                                                                                              (_BIT24|_BIT16|_BIT17)  //M7
#define  b_6_EQ_POW															(_BIT9)			//h5
#define  b_6_RS_CAL_EN														(_BIT8)			//h5
//#define  b_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)  //h5
//#define  b_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)  //h5
//#define  b_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)  //h5
//#define  b_6_TAP0_HBR                                                                              	        (_BIT13)  //h5
//#define  b_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
//#define  b_6_TAP0_LE_ISO							(_BIT8)

#define  b_5_TRANS_CNT_EN													 (_BIT1)    //h5
#define  b_5_AC_COPULE_POW                                                                                            (_BIT0)    //h5


#define b_9_DFE_ADAPTION_POW_EN 								              (_BIT7)
#define b_9_DFE_TAP_EN 								                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define b_9_DFE_TAP1_EN 								                           (_BIT3)
#define HD21_RX_PHY_P0_DFE_ADAPTION_EN(data)							(0x00000080&((data)<<7))
#define HD21_RX_PHY_P0_b_tap_en(data)								(0x00000078&((data)<<3))


#define acdr_b_8_FS_SIGNAL_DLY_EN				(_BIT26)  //merlin5
#define acdr_b_8_FS_EN							(_BIT25)  //merlin5
#define acdr_b_8_DELAY_CELL_SEL				(_BIT24)  //h5

#define  acdr_b_7_CMU_PREDIVN                                                                          (_BIT21|_BIT20|_BIT19|_BIT18)  //h5
#define  acdr_b_7_CMU_N_code                                                                          (_BIT17|_BIT16)  //h5

#define acdr_b_6_CK_SRC_SEL					(_BIT14)  //h5
#define acdr_b_6_LDO2V							(_BIT13|_BIT12|_BIT11)  //h5
#define acdr_b_5_VC_TEST							(_BIT7)  //h5
#define acdr_b_5_KVCO_SEL						(_BIT5)  //h5
#define acdr_b_5_VC_INIT						(_BIT3|_BIT2|_BIT1|_BIT0)  //h5

#define acdr_b_4_CP_OP							(_BIT25) //h5
#define acdr_b_4_CP_CAP							(_BIT24) //h5
#define acdr_b_3_BAND_SEL						(_BIT23|_BIT22)  //h5
#define acdr_b_3_VC_TUNE						(_BIT21|_BIT20)  //h5
#define acdr_b_3_VC_DEFAULT						(_BIT20)  //h5
#define acdr_b_3_ICP_TUNE						(_BIT17|_BIT16)  //h5
#define acdr_b_3_ICP_DEFAULT					(_BIT16)  //h5


#define acdr_b_2_CKFB							(_BIT13)  //h5
#define acdr_b_2_LPF_CP							(_BIT12)  //h5
#define acdr_b_2_PFD_RSTB						(_BIT11)  //h5
#define acdr_b_2_POST_M							(_BIT10)  //h5
#define acdr_b_2_PRE_M							(_BIT9)  //h5
#define acdr_b_2_CK_ENABLE						(_BIT8)  //y5


#define acdr_b_1_FLD_CK_EN					(_BIT5)  //h5
#define acdr_b_1_HALF_RATE_SEL				(_BIT4)
#define acdr_b_1_CCO_POW					(_BIT1) //h5
#define acdr_b_1_LDO_EN						(_BIT0) //h5


#define acdr_b_manual_RDY_FROM		(_BIT5)  //h5
#define acdr_b_manual_RDY_FROM_REG		(_BIT5)  //h5
#define acdr_b_manual_RDY_FROM_DPHY		(0x0)  //h5
#define acdr_b_manual_CMU_SEL				(_BIT4)  //h5


//////////////////////////////////////// CK lane //////////////////////////////////////////////////////////

#define LDO10V_CMU_EN					(_BIT7)

#define cmu_ck_1_HYS_AMP_EN								(_BIT6)   //CKAFE POWER

#define cmu_ck_1_HYS_WIN_SEL									(_BIT5|_BIT2)
#define cmu_ck_1_ENHANCE_BIAS_11								(_BIT5|_BIT2)
#define cmu_ck_1_ENHANCE_BIAS_10								(_BIT5)
#define cmu_ck_1_ENHANCE_BIAS_01								(_BIT2)
#define cmu_ck_1_ENHANCE_BIAS_00								(0)

#define cmu_ck_1_NON_HYS_CLKDIV									(_BIT4)
#define cmu_ck_1_MD_DET_SRC										(_BIT3)
#define cmu_ck_1_CKIN_SEL											(_BIT1)
#define cmu_ck_1_NON_HYS_AMP_EN									(_BIT0)  //CKD2S






//#define  ck_3_CMU_SEL_D4										 (_BIT16)
//#define  ck_2_CMU_CKIN_SEL                                                                   (_BIT15)
//#define  ck_2_ACDR_CBUS_REF										(_BIT13|_BIT14)
//#define  ck_2_CK_MD_DET_SEL										(_BIT12)
//#define  ck_2_LDO_EN                                                                             (_BIT11)
//#define  CMU_CKAFE_ENHANCE_BIAS								 (_BIT10)
#define  ck_2_CK_MD_REF_SEL                                                                   (_BIT9)
//#define  CK_2_CKD2S_EN0											   (_BIT8)




//#define  ck_1_port_bias                                                                              (_BIT2)
//#define  ck_1_CKAFE_POW                                                                          (_BIT1)



#define  HD21_RX_PHY_P0_get_CMU_SEL_D4(data)                        ((0x00010000&(data))>>16)


//#define ck_8_LDO_EN                                                                                  (_BIT25)
//#define  ck_8_VSEL_LDO_A_mask                                                                   (_BIT29|_BIT30|_BIT31)
//#define  ck_8_CCO_BAND_SEL_mask                                                                   (_BIT26|_BIT27|_BIT28)
//#define  ck_8_LDO_EN_mask                                                                          (_BIT25)
//#define  ck_8_CMU_BPPSR_mask                                                                          (_BIT24)
//#define ck_8_CMU_SEL_CP_mask										   (_BIT20|_BIT21)
//#define  ck_7_CMU_BYPASS_PI_mask                                                              (_BIT19)
//--#define  ck_7_CMU_PI_I_SEL_mask                                                                 (_BIT16|_BIT17|_BIT18)
//#define  ck_6_CMU_BIG_KVCO_mask                                                                 (_BIT13)
//#define  ck_6_CMU_EN_CAP_mask                                                                     (_BIT12)
//#define  ck_6_CMU_SEL_CS_mask                                                                     (_BIT8|_BIT9|_BIT10)
//#define  ck_5_CMU_SEL_PUMP_I_mask                                                               (_BIT4|_BIT5|_BIT6|_BIT7)
//#define  ck_5_CMU_SEL_PUMP_DB_mask				(_BIT3)
//#define  ck_5_CMU_SEL_R1_mask                                                                       (_BIT0|_BIT1|_BIT2)





//h5 band sel
#define  HD21_RX_PHY_CCO_BAND_SEL(data)                            (0x00C00000&((data)<<22))


//h5 vsel
#define  HD21_RX_PHY_P0_VSEL_LDO(data)					   (0x00003800&((data)<<11))

//#define  DFE_HDMI_RX_PHY_P0_CMU_BPPSR(data)                            (0x01000000&((data)<<24))
//#define  DFE_HDMI_RX_PHY_P0_CMU_SEL_CS(data)                            (0x00000700&((data)<<8))
//#define  DFE_HDMI_RX_PHY_P0_CMU_SEL_PUMP_I(data)                            (0x000000F0&((data)<<4))
//#define  DFE_HDMI_RX_PHY_P0_CMU_SEL_R1(data)                            (0x00000007&((data)<<0))
//#define  DFE_HDMI_RX_PHY_P0_get_CMU_BPPSR(data)                        ((0x01000000&(data))>>24)

#define  EQ1_2PTAT_pow						(_BIT7)  //
#define  EQ1_1_and_EQ1_2PTAT_pow			(_BIT6)  //

#define  ck_12_FR_CK_SEL							(_BIT31)  //h5
#define  ck_12_PIX_RATE_SEL						(_BIT30|_BIT29)   //h5
#define  ck_12_PI_CSEL							(_BIT25|_BIT24)   //h5
#define  ck_10_DCVS_EN												     (_BIT15) //h5
#define  ck_10_DFE_SUPAMP_DCGAIN_MAX									     (_BIT14) //h5
#define  ck_10_DFE_SUMAMP_ISEL											 (_BIT13|_BIT12|_BIT11)  //h5
#define  ck_10_DFE_SUMAMP_ISEL_101										 (_BIT13|_BIT11)  //h5

#define  ck_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)    //h5
#define  ck_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28)  //h5
#define  ck_8_POW_PR                                                                                            (_BIT25)  //h5
#define  ck_7_PI_ISEL                                                                                              (_BIT24|_BIT16|_BIT17)  //M7

#define  ck_6_EQ_POW															(_BIT9)			//h5

#define  ck_5_TRANS_CNT_EN													 (_BIT1)  //h5
#define  ck_5_AC_COPULE_POW                                                                                            (_BIT0)    //h5


#define  ck_4_FORE_KOFF_RANGE                                                                    (_BIT30|_BIT29)  //h5
#define  ck_4_FORE_KOFF_AUTOK                                                                    (_BIT28)  //h5
#define  ck_4_BBPD_RSTB												(_BIT25)  //h5
#define  ck_4_FORE_POW_FORE_KOFF                                                                  (_BIT24)  //h5
#define  ck_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)  //h5

#define  ck_2_RXVCM_SEL							(_BIT12|_BIT11)		//h5
#define  ck_2_RXVCM_10							(_BIT12)		//h5					

#define  ck_1_inputoff_p				       		(_BIT2)   //h5
#define  ck_1_inputoff_n				       		(_BIT1)   //h5
#define  ck_1_inputoff                                                                      		(_BIT0)   //h5

#define acdr_ck_8_FS_SIGNAL_DLY_EN				(_BIT26)  //merlin5
#define acdr_ck_8_FS_EN							(_BIT25)  //merlin5
#define acdr_ck_8_DELAY_CELL_SEL				(_BIT24)  //h5

#define  acdr_ck_7_CMU_PREDIVN                                                                          (_BIT21|_BIT20|_BIT19|_BIT18)  //h5
#define  acdr_ck_7_CMU_N_code                                                                          (_BIT17|_BIT16)  //h5


#define acdr_ck_6_CK_SRC_SEL					(_BIT14)  //h5
#define acdr_ck_6_LDO2V							(_BIT13|_BIT12|_BIT11)  //h5
#define acdr_ck_5_VC_TEST							(_BIT7)  //h5
#define acdr_ck_5_KVCO_SEL						(_BIT5)  //h5
#define acdr_ck_5_VC_INIT						(_BIT3|_BIT2|_BIT1|_BIT0)  //h5


#define acdr_ck_4_CP_OP							(_BIT25) //h5
#define acdr_ck_4_CP_CAP							(_BIT24) //h5
#define acdr_ck_3_BAND_SEL						(_BIT23|_BIT22)  //h5
#define acdr_ck_3_VC_TUNE							(_BIT21|_BIT20)  //h5
#define acdr_ck_3_VC_DEFAULT						(_BIT20)  //h5
#define acdr_ck_3_ICP_TUNE						(_BIT17|_BIT16)  //h5
#define acdr_ck_3_ICP_DEFAULT					(_BIT16)  //h5


#define acdr_ck_2_CKFB							(_BIT13)  //h5
#define acdr_ck_2_LPF_CP							(_BIT12) //h5
#define acdr_ck_2_PFD_RSTB						(_BIT11) //h5
#define acdr_ck_2_POST_M						(_BIT10) //h5
#define acdr_ck_2_PRE_M							(_BIT9)  //h5
#define acdr_ck_2_CK_ENABLE						(_BIT8)  //h5




#define acdr_ck_1_FLD_CK_EN					(_BIT5)  //h5
#define acdr_ck_1_HALF_RATE_SEL				(_BIT4)  //h5
#define acdr_ck_1_CCO_POW						(_BIT1) //h5
#define acdr_ck_1_LDO_EN						(_BIT0)  //h5


#define acdr_ck_manual_RDY_FROM		(_BIT5)
#define acdr_ck_manual_RDY_FROM_REG		(_BIT5)
#define acdr_ck_manual_RDY_FROM_DPHY		(0x0)
#define acdr_ck_manual_CMU_SEL				(_BIT4)


#if 0
#define  ck_9_CMU_PFD_RSTB                                                                          (_BIT5)
#define  ck_9_CMU_WDRST                                                                          (_BIT2)
#endif







//////////////////////////////////////// G lane //////////////////////////////////////////////////////////

#define  g_4_FORE_KOFF_RANGE                                                                    (_BIT30|_BIT29)
#define  g_4_FORE_KOFF_AUTOK                                                                    (_BIT28)
#define  g_4_BBPD_RSTB												(_BIT25)  //h5
#define  g_4_FORE_POW_FORE_KOFF                                                                  (_BIT24)
#define  g_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)

#define  g_1_OFFCAL_SEL							(_BIT7|_BIT6|_BIT5)
#define  g_1_inputoff_p				       		(_BIT2)
#define  g_1_inputoff_n				       		(_BIT1)
#define  g_1_inputoff                                                                      		(_BIT0)

#define  g_12_FR_CK_SEL							(_BIT31)  //h5
#define  g_12_PIX_RATE_SEL						(_BIT30|_BIT29)   //h5
#define  g_12_PI_CSEL							(_BIT25|_BIT24)   //h5
#define  g_10_DCVS_EN												     (_BIT15) //h5
#define  g_10_DFE_SUPAMP_DCGAIN_MAX									     (_BIT14) //h5
#define  g_10_DFE_SUMAMP_ISEL											 (_BIT13|_BIT12|_BIT11)  //h5
#define  g_10_DFE_SUMAMP_ISEL_101										 (_BIT13|_BIT11)  //h5

#define  g_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)  //h5
#define  g_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28) //h5
#define  g_8_POW_PR                                                                                            (_BIT25)
#define  g_8_BY_PASS_PR                                                                                      (_BIT24)
#define  g_7_PI_ISEL                                                                                              (_BIT24|_BIT16|_BIT17)  //M7
#define  g_6_EQ_POW															  (_BIT9)
#define  g_6_RS_CAL_EN														(_BIT8)			//h5
//#define  g_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)
//#define  g_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)
//#define  g_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)
//#define  g_6_TAP0_HBR                                                                              	        (_BIT13)
//#define  g_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
#define  g_5_TRANS_CNT_EN													 (_BIT1)
#define  g_5_AC_COPULE_POW                                                                                            (_BIT0)    //h5

#define  g_2_RXVCM_SEL							(_BIT12|_BIT11)		//h5
#define  g_2_RXVCM_10							(_BIT12)     //h5





#define g_9_DFE_ADAPTION_POW_EN 						              (_BIT7)
#define g_9_DFE_TAP_EN 						                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define g_9_DFE_TAP1_EN 						                           (_BIT3)
#define  HD21_RX_PHY_P0_g_tap_en(data)                                                           (0x00000078&((data)<<3))


#define acdr_g_8_FS_SIGNAL_DLY_EN				(_BIT26)  //merlin5
#define acdr_g_8_FS_EN							(_BIT25)  //merlin5
#define acdr_g_8_DELAY_CELL_SEL				(_BIT24)  //h5

#define  acdr_g_7_CMU_PREDIVN                                                                          (_BIT21|_BIT20|_BIT19|_BIT18)  //h5
#define  acdr_g_7_CMU_N_code                                                                          (_BIT17|_BIT16)  //h5

#define acdr_g_6_CK_SRC_SEL					(_BIT14)  //h5
#define acdr_g_6_LDO2V							(_BIT13|_BIT12|_BIT11)  //h5
#define acdr_g_5_VC_TEST							(_BIT7)  //h5
#define acdr_g_5_KVCO_SEL						(_BIT5)  //h5
#define acdr_g_5_VC_INIT						(_BIT3|_BIT2|_BIT1|_BIT0)  //h5

#define acdr_g_4_CP_OP							(_BIT25) //h5
#define acdr_g_4_CP_CAP							(_BIT24) //h5
#define acdr_g_3_BAND_SEL						(_BIT23|_BIT22)  //h5
#define acdr_g_3_VC_TUNE						(_BIT21|_BIT20)  //h5
#define acdr_g_3_VC_DEFAULT						(_BIT20)  //h5
#define acdr_g_3_ICP_TUNE						(_BIT17|_BIT16)  //h5
#define acdr_g_3_ICP_DEFAULT					(_BIT16)  //h5


#define acdr_g_2_CKFB							(_BIT13)  //h5
#define acdr_g_2_LPF_CP							(_BIT12) //h5
#define acdr_g_2_PFD_RSTB						(_BIT11)  //h5
#define acdr_g_2_POST_M							(_BIT10)  //h5
#define acdr_g_2_PRE_M							(_BIT9)  //h5
#define acdr_g_2_CK_ENABLE						(_BIT8)  //h5

#define acdr_g_1_FLD_CK_EN					(_BIT5)  //h5
#define acdr_g_1_HALF_RATE_SEL				(_BIT4)  //h5
#define acdr_g_1_CCO_POW					(_BIT1) //h5
#define acdr_g_1_LDO_EN						(_BIT0)  //h5


#define acdr_g_manual_RDY_FROM		(_BIT5)
#define acdr_g_manual_RDY_FROM_REG		(_BIT5)
#define acdr_g_manual_RDY_FROM_DPHY		(0x0)
#define acdr_g_manual_CMU_SEL				(_BIT4)

//////////////////////////////////////// R lane //////////////////////////////////////////////////////////

#define  r_4_FORE_KOFF_RANGE                                                                    (_BIT30|_BIT29)
#define  r_4_FORE_KOFF_AUTOK                                                                    (_BIT28)
#define  r_4_BBPD_RSTB												(_BIT25)  //h5
#define  r_4_FORE_POW_FORE_KOFF                                                                  (_BIT24)
#define  r_3_FORE_EN_FORE_KOFF                                                                    (_BIT23)

#define  r_2_RXVCM_SEL							(_BIT12|_BIT11)		//h5
#define  r_2_RXVCM_10							(_BIT12)     //h5

#define  r_1_OFFCAL_SEL							(_BIT7|_BIT6|_BIT5)
#define  r_1_inputoff                                                                      		(_BIT0)
#define  r_1_inputoff_n				       		(_BIT1)
#define  r_1_inputoff_p				       		(_BIT2)

#define  r_12_FR_CK_SEL							(_BIT31) //h5
#define  r_12_PIX_RATE_SEL						(_BIT30|_BIT29)   //h5

#define  r_12_PI_CSEL							(_BIT25|_BIT24)   //h5
#define  r_10_DCVS_EN												     (_BIT15) //h5
#define  r_10_DFE_SUPAMP_DCGAIN_MAX									     (_BIT14) //h5
#define  r_10_DFE_SUMAMP_ISEL											 (_BIT13|_BIT12|_BIT11)  //h5
#define  r_10_DFE_SUMAMP_ISEL_101										 (_BIT13|_BIT11)  //h5
#define  r_8_DEMUX                                                                                             (_BIT29|_BIT30|_BIT31)  //h5
#define  r_8_PR_RATE_SEL                                                                                    (_BIT26|_BIT27|_BIT28)
#define  r_8_POW_PR                                                                                            (_BIT25)
#define  r_8_BY_PASS_PR                                                                                      (_BIT24)
#define  r_7_PI_ISEL                                                                                              (_BIT24|_BIT16|_BIT17)  //M7
#define  r_6_EQ_POW														(_BIT9)
#define  r_6_RS_CAL_EN														(_BIT8)			//h5
//#define  r_6_LEQ_BIT_RATE                                                                                    (_BIT14|_BIT15)
//#define  r_6_LEQ_BIT_RATE_HBR                                                                            (_BIT15)
//#define  r_6_LEQ_BIT_RATE_MBR                                                                            (_BIT14)
//#define  r_6_TAP0_HBR                                                                              	        (_BIT13)
//#define  r_6_LEQ_PASSIVE_CORNER                                                                      (_BIT12|_BIT11)
#define  r_5_TRANS_CNT_EN													 (_BIT1)
#define  r_5_AC_COPULE_POW                                                                                            (_BIT0)    //h5



#define r_9_DFE_ADAPTION_POW_EN 								              (_BIT7)
#define r_9_DFE_TAP_EN 								                           (_BIT3|_BIT4|_BIT5|_BIT6)
#define r_9_DFE_TAP1_EN 								                           (_BIT3)
#define  HD21_RX_PHY_P0_r_tap_en(data)                                                           (0x00000078&((data)<<3))
#define  r_11_ACDR_RATE_SEL 												(_BIT24|_BIT25)

#define acdr_r_8_FS_SIGNAL_DLY_EN				(_BIT26)  //merlin5
#define acdr_r_8_FS_EN							(_BIT25)  //merlin5
#define acdr_r_8_DELAY_CELL_SEL				(_BIT24)  //h5

#define  acdr_r_7_CMU_PREDIVN                                                                          (_BIT21|_BIT20|_BIT19|_BIT18)  //h5
#define  acdr_r_7_CMU_N_code                                                                          (_BIT17|_BIT16)  //h5

#define acdr_r_6_CK_SRC_SEL						(_BIT14)  //h5
#define acdr_r_6_LDO2V							(_BIT13|_BIT12|_BIT11)  //h5
#define acdr_r_5_VC_TEST							(_BIT7)  //h5
#define acdr_r_5_KVCO_SEL						(_BIT5)  //h5
#define acdr_r_5_VC_INIT							(_BIT3|_BIT2|_BIT1|_BIT0)  //h5

#define acdr_r_4_CP_OP							(_BIT25) //h5
#define acdr_r_4_CP_CAP							(_BIT24) //h5
#define acdr_r_3_BAND_SEL						(_BIT23|_BIT22)  //h5
#define acdr_r_3_VC_TUNE						(_BIT21|_BIT20)  //h5
#define acdr_r_3_VC_DEFAULT						(_BIT20)  //h5
#define acdr_r_3_ICP_TUNE						(_BIT17|_BIT16)  //h5
#define acdr_r_3_ICP_DEFAULT					(_BIT16)  //h5


#define acdr_r_2_CKFB							(_BIT13)  //h5
#define acdr_r_2_LPF_CP							(_BIT12)  //h5
#define acdr_r_2_PFD_RSTB						(_BIT11)  //h5
#define acdr_r_2_POST_M							(_BIT10) //h5
#define acdr_r_2_PRE_M							(_BIT9)  //h5
#define acdr_r_2_CK_ENABLE						(_BIT8)  //h5


#define acdr_r_1_FLD_CK_EN					(_BIT5)  //h5
#define acdr_r_1_HALF_RATE_SEL				(_BIT4)  //h5
#define acdr_r_1_CCO_POW					(_BIT1) //h5
#define acdr_r_1_LDO_EN						(_BIT0)  //h5

#define acdr_r_manual_RDY_FROM		(_BIT5)
#define acdr_r_manual_RDY_FROM_REG		(_BIT5)
#define acdr_r_manual_RDY_FROM_DPHY		(0x0)
#define acdr_r_manual_CMU_SEL				(_BIT4)



//EQ part (h5)
//////////////////////////////////////// Global//////////////////////////////////////////////////////////
#define TOP_IN_2_IPTAT_LEQ_ADJ				(_BIT10|_BIT9|_BIT8)
#define TOP_IN_2_IB_LEQ_ADJ				(_BIT13|_BIT12|_BIT11)

//////////////////////////////////////// B lane //////////////////////////////////////////////////////////
#define b_6_RLSEL_LEQ					(_BIT15|_BIT14|_BIT13|_BIT12)
#define b_14_RLSEL_LEQ1_NC1				(_BIT15|_BIT14|_BIT13)
#define b_2_RLSEL_LEQ1_NC2				(_BIT10|_BIT9|_BIT8)
#define b_6_RLSEL_TAP0					(_BIT11)
#define b_16_RSSEL_LEQ2					(_BIT31|_BIT30|_BIT29|_BIT23)  //bit23 is high bit
#define b_11_RSSEL_LEQ1_1				(_BIT19|_BIT18|_BIT17|_BIT16)
#define b_11_RSSEL_LEQ1_2				(_BIT23|_BIT22|_BIT21|_BIT20)
#define b_6_RSSEL_TAP0					(_BIT10)
#define b_5_LEQ_CURRENT_ADJ			(_BIT4|_BIT3)
#define b_7_LEQ_ISEL						(_BIT20|_BIT19|_BIT18)
#define b_2_LE_PTAT_ISEL					(_BIT15|_BIT14|_BIT13)
#define b_7_TAP0_ISEL					(_BIT23|_BIT22|_BIT21)
#define b_17_POW_PTAT					(_BIT6)
#define b_17_POW_PTAT_STAGE1			(_BIT7)

//BIAS
#define b_12_PI_CURRENT_ADJ			(_BIT28)			//h5
#define b_5_TAP0_CURRENT_ADJ			(_BIT2)     //h5
#define b_3_PTAT_CURRENT_ADJ			(_BIT22|_BIT21)

//DFE
#define b_1_DFE_BLEED				(_BIT3)  //h5


//////////////////////////////////////// CK lane //////////////////////////////////////////////////////////
#define ck_6_RLSEL_LEQ					(_BIT15|_BIT14|_BIT13|_BIT12)
#define ck_14_RLSEL_LEQ1_NC1				(_BIT15|_BIT14|_BIT13)
#define ck_2_RLSEL_LEQ1_NC2				(_BIT10|_BIT9|_BIT8)
#define ck_6_RLSEL_TAP0					(_BIT11)
#define ck_16_RSSEL_LEQ2					(_BIT31|_BIT30|_BIT29|_BIT23)
#define ck_11_RSSEL_LEQ1_1				(_BIT19|_BIT18|_BIT17|_BIT16)
#define ck_11_RSSEL_LEQ1_2				(_BIT23|_BIT22|_BIT21|_BIT20)
#define ck_6_RSSEL_TAP0					(_BIT10)
#define ck_5_LEQ_CURRENT_ADJ			(_BIT4|_BIT3)
#define ck_7_LEQ_ISEL						(_BIT20|_BIT19|_BIT18)
#define ck_2_LE_PTAT_ISEL					(_BIT15|_BIT14|_BIT13)
#define ck_7_TAP0_ISEL					(_BIT23|_BIT22|_BIT21)
#define ck_17_POW_PTAT					(_BIT6)
#define ck_17_POW_PTAT_STAGE1			(_BIT7)

//BIAS
#define ck_12_PI_CURRENT_ADJ			(_BIT28)			//h5
#define ck_5_TAP0_CURRENT_ADJ			(_BIT2)     //h5
#define ck_3_PTAT_CURRENT_ADJ			(_BIT22|_BIT21)

//DFE
#define ck_1_DFE_BLEED				(_BIT3)  //h5


//////////////////////////////////////// G lane //////////////////////////////////////////////////////////
#define g_6_RLSEL_LEQ					(_BIT15|_BIT14|_BIT13|_BIT12)
#define g_14_RLSEL_LEQ1_NC1				(_BIT15|_BIT14|_BIT13)
#define g_2_RLSEL_LEQ1_NC2				(_BIT10|_BIT9|_BIT8)
#define g_6_RLSEL_TAP0					(_BIT11)
#define g_16_RSSEL_LEQ2					(_BIT31|_BIT30|_BIT29|_BIT23)
#define g_11_RSSEL_LEQ1_1				(_BIT19|_BIT18|_BIT17|_BIT16)
#define g_11_RSSEL_LEQ1_2				(_BIT23|_BIT22|_BIT21|_BIT20)
#define g_6_RSSEL_TAP0					(_BIT10)
#define g_5_LEQ_CURRENT_ADJ			(_BIT4|_BIT3)
#define g_7_LEQ_ISEL						(_BIT20|_BIT19|_BIT18)
#define g_2_LE_PTAT_ISEL					(_BIT15|_BIT14|_BIT13)
#define g_7_TAP0_ISEL					(_BIT23|_BIT22|_BIT21)
#define g_17_POW_PTAT					(_BIT6)
#define g_17_POW_PTAT_STAGE1			(_BIT7)

//BIAS
#define g_12_PI_CURRENT_ADJ			(_BIT28)			//h5
#define g_5_TAP0_CURRENT_ADJ			(_BIT2)     //h5
#define g_3_PTAT_CURRENT_ADJ			(_BIT22|_BIT21)

//DFE
#define g_1_DFE_BLEED				(_BIT3)  //h5

//////////////////////////////////////// R lane //////////////////////////////////////////////////////////
#define r_6_RLSEL_LEQ					(_BIT15|_BIT14|_BIT13|_BIT12)
#define r_14_RLSEL_LEQ1_NC1				(_BIT15|_BIT14|_BIT13)
#define r_2_RLSEL_LEQ1_NC2				(_BIT10|_BIT9|_BIT8)
#define r_6_RLSEL_TAP0					(_BIT11)
#define r_16_RSSEL_LEQ2					(_BIT31|_BIT30|_BIT29|_BIT23)
#define r_11_RSSEL_LEQ1_1				(_BIT19|_BIT18|_BIT17|_BIT16)
#define r_11_RSSEL_LEQ1_2				(_BIT23|_BIT22|_BIT21|_BIT20)
#define r_6_RSSEL_TAP0					(_BIT10)
#define r_5_LEQ_CURRENT_ADJ			(_BIT4|_BIT3)
#define r_7_LEQ_ISEL						(_BIT20|_BIT19|_BIT18)
#define r_2_LE_PTAT_ISEL					(_BIT15|_BIT14|_BIT13)
#define r_7_TAP0_ISEL					(_BIT23|_BIT22|_BIT21)
#define r_17_POW_PTAT					(_BIT6)
#define r_17_POW_PTAT_STAGE1			(_BIT7)

//BIAS
#define r_12_PI_CURRENT_ADJ			(_BIT28)  //h5
#define r_5_TAP0_CURRENT_ADJ			(_BIT2)     //h5
#define r_3_PTAT_CURRENT_ADJ			(_BIT22|_BIT21)

//DFE
#define P0_r_1_DFE_BLEED				(_BIT3)  //h5



#define  b_3_FORE_KOFF_ADJR			(_BIT16|_BIT17|_BIT18|_BIT19|_BIT20)
#define  g_3_FORE_KOFF_ADJR			(_BIT16|_BIT17|_BIT18|_BIT19|_BIT20)
#define  r_3_FORE_KOFF_ADJR			(_BIT16|_BIT17|_BIT18|_BIT19|_BIT20)



#define  b_8_BY_PASS_PR                                                                                      (_BIT24)
#define  g_8_BY_PASS_PR                                                                                      (_BIT24)
#define  r_8_BY_PASS_PR                                                                                      (_BIT24)
#define  b_8_BY_PASS_PR                                                                                      (_BIT24)
#define  g_8_BY_PASS_PR                                                                                      (_BIT24)
#define  r_8_BY_PASS_PR                                                                                      (_BIT24)


//VTH DAC
#define VTH_MANUAL					_BIT31
#define KOFF_VTH_MANUAL_0			_BIT29
#define KOFF_VTH_MANUAL_1			_BIT21
#define KOFF_VTH_MANUAL_2			_BIT13
#define KOFF_VTH_MANUAL_3			_BIT5


//ICP (aphy->dphy)
#define ICP_PLL_MODE_mask	(_BIT31|_BIT30|_BIT29|_BIT28)
#define ICP_ACDR_MODE_STEP1_mask  (_BIT27|_BIT26|_BIT25|_BIT24)
#define ICP_ACDR_MODE_STEP2_mask  (_BIT23|_BIT22|_BIT21|_BIT20)
#define ICP_ACDR_MODE_STEP3_mask  (_BIT19|_BIT18|_BIT17|_BIT16)


