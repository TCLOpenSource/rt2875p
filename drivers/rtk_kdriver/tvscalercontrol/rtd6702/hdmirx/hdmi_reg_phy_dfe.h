#include <rbus/dfe_p0_reg.h>

#ifndef uint32_t
typedef unsigned int __UINT32_T;
#define uint32_t __UINT32_T
#endif

#define HDMI20_PORT  HDMI_PORT2

//dfe_21
#define HDMI_DFE0_BASE           (0xB80B7C00)
#define HDMI_DFE1_BASE           (0xB80B9C00)
#define HDMI_DFE2_BASE           (0xB80BAC00)
#define HDMI_DFE3_BASE           (0xB80BCC00)


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
uint32_t CTRL_R0;	//7C00 9C00 AC00 CC00
uint32_t CTRL_R1;	//7C04 9C04 AC04 CC04
uint32_t CTRL_R2;	//7C08 9C08 AC08 CC08
uint32_t CTRL_R3;	//7C0C 9C0C AC0C CC0C
uint32_t CTRL_R4;	//7C10 9C10 AC10 CC10
uint32_t CTRL_R5;	//7C14 9C14 AC14 CC14
uint32_t CTRL_R6;	//7C18 9C18 AC18 CC18
uint32_t CTRL_R7;	//7C1C 9C1C AC1C CC1C
uint32_t CTRL_R8;	//7C20 9C20 AC20 CC20
uint32_t CTRL_R9;	//7C24 9C24 AC24 CC24
uint32_t CTRL_RA;	//7C28 9C28 AC28 CC28
uint32_t CTRL_RB;	//7C2C 9C2C AC2C CC2C
uint32_t CTRL_RC;	//7C30 9C30 AC30 CC30
uint32_t CTRL_RD;	//7C34 9C34 AC34 CC34
uint32_t rsvd0;  //7C38	
uint32_t rsvd1;  //7C3C	
 
uint32_t CTRL_G0;	//7C40 9C40 AC40 CC40
uint32_t CTRL_G1;	//7C44 9C44 AC44 CC44
uint32_t CTRL_G2;	//7C48 9C48 AC48 CC48
uint32_t CTRL_G3;	//7C4C 9C4C AC4C CC4C
uint32_t CTRL_G4;	//7C50 9C50 AC50 CC50
uint32_t CTRL_G5;	//7C54 9C54 AC54 CC54
uint32_t CTRL_G6;	//7C58 9C58 AC58 CC58
uint32_t CTRL_G7;	//7C5C 9C5C AC5C CC5C
uint32_t CTRL_G8;	//7C60 9C60 AC60 CC60
uint32_t CTRL_G9;	//7C64 9C64 AC64 CC64
uint32_t CTRL_GA;//7C68 9C68 AC68 CC68
uint32_t CTRL_GB;	//7C6C 9C6C AC6C CC6C
uint32_t CTRL_GC;	//7C70 9C70 AC70 CC70 //
uint32_t CTRL_GD;//7C74 9C74 AC74 CC74 //
uint32_t rsvd2;  //7C78	
uint32_t rsvd3;  //7C7C	
 
									   
uint32_t CTRL_B0;	//7C80 9C80 AC80 CC80
uint32_t CTRL_B1;	//7C84 9C84 AC84 CC84
uint32_t CTRL_B2;	//7C88 9C88 AC88 CC88
uint32_t CTRL_B3;	//7C8C 9C8C AC8C CC8C
uint32_t CTRL_B4;	//7C90 9C90 AC90 CC90
uint32_t CTRL_B5;	//7C94 9C94 AC94 CC94
uint32_t CTRL_B6;	//7C98 9C98 AC98 CC98
uint32_t CTRL_B7;	//7C9C 9C9C AC9C CC9C
uint32_t CTRL_B8;	//7CA0 9CA0 ACA0 CCA0
uint32_t CTRL_B9;	//7CA4 9CA4 ACA4 CCA4
uint32_t CTRL_BA;	//7CA8 9CA8 ACA8 CCA8
uint32_t CTRL_BB;	//7CAC 9CAC ACAC CCAC
uint32_t CTRL_BC;	//7CB0 9CB0 ACA0 CCA0 //
uint32_t CTRL_BD;	//7CB4 9CB4 ACA4 CCA4
uint32_t rsvd4;  //7CB8	
uint32_t rsvd5;  //7CBC	
                                       
uint32_t CTRL_CK0;//7CC0 9CC0 ACC0 CCC0
uint32_t CTRL_CK1;//7CC4 9CC4 ACC4 CCC4
uint32_t CTRL_CK2;//7CC8 9CC8 ACC8 CCC8
uint32_t CTRL_CK3;//7CCC 9CCC ACCC CCCC
uint32_t CTRL_CK4;//7CD0 9CD0 ACD0 CCD0
uint32_t CTRL_CK5;//7CD4 9CD4 ACD4 CCD4
uint32_t CTRL_CK6;//7CD8 9CD8 ACD8 CCD8
uint32_t CTRL_CK7;//7CDC 9CDC ACDC CCDC
uint32_t CTRL_CK8;//7CE0 9CE0 ACE0 CCE0
uint32_t CTRL_CK9;//7CE4 9CE4 ACE4 CCE4
uint32_t CTRL_CKA;//7CE8 9CE8 ACE8 CCE8
uint32_t CTRL_CKB;//7CEC 9CEC ACEC CCEC
uint32_t CTRL_CKC;//7CF0 9CF0 ACF0 CCF0
uint32_t CTRL_CKD;//7CF4 9CF4 ACF4 CCF4

};


extern const struct hdmi_dfe_reg_st *dfe[4];
extern const struct hdmi21_dfe_reg_st *hd21_dfe[4];  //hd20 & hd21 share

// New
#define REG_DFE_CTRL_R0_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R0)//7C00 9C00 AC00 CC00   
#define REG_DFE_CTRL_R1_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R1)//7C04 9C04 AC04 CC04   
#define REG_DFE_CTRL_R2_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R2)//7C08 9C08 AC08 CC08   
#define REG_DFE_CTRL_R3_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R3)//7C0C 9C0C AC0C CC0C   
#define REG_DFE_CTRL_R4_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R4)//7C10 9C10 AC10 CC10   
#define REG_DFE_CTRL_R5_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R5)//7C14 9C14 AC14 CC14   
#define REG_DFE_CTRL_R6_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R6)//7C18 9C18 AC18 CC18   
#define REG_DFE_CTRL_R7_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R7)//7C1C 9C1C AC1C CC1C   
#define REG_DFE_CTRL_R8_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R8)//7C20 9C20 AC20 CC20   
#define REG_DFE_CTRL_R9_reg (unsigned int)(&hd21_dfe[nport]->CTRL_R9)//7C24 9C24 AC24 CC24   
#define REG_DFE_CTRL_RA_reg (unsigned int)(&hd21_dfe[nport]->CTRL_RA)//7C28 9C28 AC28 CC28   
#define REG_DFE_CTRL_RB_reg (unsigned int)(&hd21_dfe[nport]->CTRL_RB)//7C2C 9C2C AC2C CC2C   
#define REG_DFE_CTRL_RC_reg (unsigned int)(&hd21_dfe[nport]->CTRL_RC)//7C30 9C30 AC30 CC30   
#define REG_DFE_CTRL_RD_reg (unsigned int)(&hd21_dfe[nport]->CTRL_RD)//7C34 9C34 AC34 CC34   
                                                                                             
#define REG_DFE_CTRL_G0_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G0)//7C40 9C40 AC40 CC40   
#define REG_DFE_CTRL_G1_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G1)//7C44 9C44 AC44 CC44   
#define REG_DFE_CTRL_G2_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G2)//7C48 9C48 AC48 CC48   
#define REG_DFE_CTRL_G3_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G3)//7C4C 9C4C AC4C CC4C   
#define REG_DFE_CTRL_G4_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G4)//7C50 9C50 AC50 CC50   
#define REG_DFE_CTRL_G5_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G5)//7C54 9C54 AC54 CC54   
#define REG_DFE_CTRL_G6_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G6)//7C58 9C58 AC58 CC58   
#define REG_DFE_CTRL_G7_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G7)//7C5C 9C5C AC5C CC5C   
#define REG_DFE_CTRL_G8_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G8)//7C60 9C60 AC60 CC60   
#define REG_DFE_CTRL_G9_reg (unsigned int)(&hd21_dfe[nport]->CTRL_G9)//7C64 9C64 AC64 CC64   
#define REG_DFE_CTRL_GA_reg (unsigned int)(&hd21_dfe[nport]->CTRL_GA)//7C68 9C68 AC68 CC68   
#define REG_DFE_CTRL_GB_reg (unsigned int)(&hd21_dfe[nport]->CTRL_GB)//7C6C 9C6C AC6C CC6C   
#define REG_DFE_CTRL_GC_reg (unsigned int)(&hd21_dfe[nport]->CTRL_GC)//7C70 9C70 AC70 CC70   
#define REG_DFE_CTRL_GD_reg (unsigned int)(&hd21_dfe[nport]->CTRL_GD)//7C74 9C74 AC74 CC74   
                                                                                        
#define REG_DFE_CTRL_B0_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B0)//7C80 9C80 AC80 CC80   
#define REG_DFE_CTRL_B1_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B1)//7C84 9C84 AC84 CC84   
#define REG_DFE_CTRL_B2_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B2)//7C88 9C88 AC88 CC88   
#define REG_DFE_CTRL_B3_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B3)//7C8C 9C8C AC8C CC8C   
#define REG_DFE_CTRL_B4_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B4)//7C90 9C90 AC90 CC90   
#define REG_DFE_CTRL_B5_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B5)//7C94 9C94 AC94 CC94   
#define REG_DFE_CTRL_B6_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B6)//7C98 9C98 AC98 CC98   
#define REG_DFE_CTRL_B7_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B7)//7C9C 9C9C AC9C CC9C   
#define REG_DFE_CTRL_B8_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B8)//7CA0 9CA0 ACA0 CCA0   
#define REG_DFE_CTRL_B9_reg (unsigned int)(&hd21_dfe[nport]->CTRL_B9)//7CA4 9CA4 ACA4 CCA4   
#define REG_DFE_CTRL_BA_reg (unsigned int)(&hd21_dfe[nport]->CTRL_BA)//7CA8 9CA8 ACA8 CCA8   
#define REG_DFE_CTRL_BB_reg (unsigned int)(&hd21_dfe[nport]->CTRL_BB)//7CAC 9CAC ACAC CCAC   
#define REG_DFE_CTRL_BC_reg (unsigned int)(&hd21_dfe[nport]->CTRL_BC)//7CB0 9A0 ACA0 CCA0   
#define REG_DFE_CTRL_BD_reg (unsigned int)(&hd21_dfe[nport]->CTRL_BD)//7CB4 9CA4 ACA4 CCA4   
                                                                                             
#define REG_DFE_CTRL_CK0_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK0)//7CC0 9CC0 ACC0 CCC0 
#define REG_DFE_CTRL_CK1_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK1)//7CC4 9CC4 ACC4 CCC4 
#define REG_DFE_CTRL_CK2_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK2)//7CC8 9CC8 ACC8 CCC8 
#define REG_DFE_CTRL_CK3_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK3)//7CCC 9CCC ACCC CCCC 
#define REG_DFE_CTRL_CK4_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK4)//7CD0 9CD0 ACD0 CCD0 
#define REG_DFE_CTRL_CK5_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK5)//7CD4 9CD4 ACD4 CCD4 
#define REG_DFE_CTRL_CK6_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK6)//7CD8 9CD8 ACD8 CCD8 
#define REG_DFE_CTRL_CK7_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK7)//7CDC 9CDC ACDC CCDC 
#define REG_DFE_CTRL_CK8_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK8)//7CE0 9CE0 ACE0 CCE0 
#define REG_DFE_CTRL_CK9_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CK9)//7CE4 9CE4 ACE4 CCE4 
#define REG_DFE_CTRL_CKA_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CKA)//7CE8 9CE8 ACE8 CCE8 
#define REG_DFE_CTRL_CKB_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CKB)//7CEC 9CEC ACEC CCEC 
#define REG_DFE_CTRL_CKC_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CKC)//7CF0 9CF0 ACF0 CCF0 
#define REG_DFE_CTRL_CKD_reg (unsigned int)(&hd21_dfe[nport]->CTRL_CKD)//7CF4 9CF4 ACF4 CCF4 




#define LOAD_IN_INIT_LE		(_BIT31)
//#define LOAD_IN_INIT_VTH	(_BIT30)
//#define LOAD_IN_INIT_SERVO	(_BIT29)
//#define LOAD_IN_INIT_TAP4	(_BIT28)
//#define LOAD_IN_INIT_TAP3	(_BIT27)
//#define LOAD_IN_INIT_TAP2	(_BIT26)
//#define LOAD_IN_INIT_TAP1	(_BIT25)
//#define LOAD_IN_INIT_TAP0	(_BIT24)
//#define LOAD_IN_INIT_ALL	(LOAD_IN_INIT_LE|LOAD_IN_INIT_VTH|LOAD_IN_INIT_SERVO|LOAD_IN_INIT_TAP4|LOAD_IN_INIT_TAP3|LOAD_IN_INIT_TAP2|LOAD_IN_INIT_TAP1|LOAD_IN_INIT_TAP0)

#define REG_DFE_ADAPT_EN_TAP0                                         _BIT25
#define REG_DFE_ADAPT_EN_TAP1                                         _BIT26
#define REG_DFE_ADAPT_EN_TAP2                                         _BIT27
#define REG_DFE_ADAPT_EN_TAP3                                         _BIT28
#define REG_DFE_ADAPT_EN_TAP4                                         _BIT29

#define COEF_SEL_VTHP_COEF	 7
#define COEF_SEL_VTHN_COEF	 8
#define COEF_SEL_LE	 10

#define COEF_SEL_TAP0				0
#define COEF_SEL_TAP1				1
#define COEF_SEL_TAP2				2
#define COEF_SEL_TAP3				3
#define COEF_SEL_TAP4				4

#define LIMIT_SEL_TAP0_MAX				0
#define LIMIT_SEL_TAP1_MIN				1
#define LIMIT_SEL_TAP1_MAX				2
#define LIMIT_SEL_TAP2_MIN				3
#define LIMIT_SEL_TAP2_MAX				4
#define LIMIT_SEL_TAP3_MIN				5
#define LIMIT_SEL_TAP3_MAX				6
#define LIMIT_SEL_TAP4_MIN				7
#define LIMIT_SEL_TAP4_MAX				8

#define REG_dfe_adapt_en_TAP0                                         _BIT25
#define REG_dfe_adapt_en_TAP1                                         _BIT26
#define REG_dfe_adapt_en_TAP2                                         _BIT27
#define REG_dfe_adapt_en_TAP3                                         _BIT28
#define REG_dfe_adapt_en_TAP4                                         _BIT29
#define REG_dfe_adapt_en_VTH_DFE_EN                             _BIT23
#define REG_dfe_adapt_en_VTH_EN					   _BIT22
#define REG_dfe_adapt_en_LE                                              _BIT13


#define LOAD_IN_INIT_VTH	(_BIT5)
#define LOAD_IN_INIT_TAP4	(_BIT4)
#define LOAD_IN_INIT_TAP3	(_BIT3)
#define LOAD_IN_INIT_TAP2	(_BIT2)
#define LOAD_IN_INIT_TAP1	(_BIT1)
#define LOAD_IN_INIT_TAP0	(_BIT0)


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


#define get_TAP2_coef_sign(data)                   	(0x20&(data))
#define get_TAP2_coef(data)                          	(0x1F&(data))
#define get_TAP3_coef_sign(data)                    	(0x20&(data))
#define get_TAP3_coef(data)                          	(0x1F&(data))
#define get_TAP4_coef_sign(data)                     (0x20&(data))
#define get_TAP4_coef(data)                          	(0x1F&(data))

