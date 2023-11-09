//#include <rbus/dfe_p0_reg.h>

#ifndef uint32_t
typedef unsigned int __UINT32_T;
#define uint32_t __UINT32_T
#endif

#define HDMI20_PORT  HDMI_PORT2

//dfe_21
#define HDMI_DFE0_BASE           (0xb800DE00)
#define HDMI_DFE1_BASE           (0xb803BE00)
#define HDMI_DFE2_BASE           (0xb803CE00)
#define HDMI_DFE3_BASE           (0xb80B2E00)


struct hdmi21_dfe_reg_st {
uint32_t CTRL_R0;	//DE00 BE00 CE00 2E00
uint32_t CTRL_R1;	//DE04 BE04 CE04 2E04
uint32_t CTRL_R2;	//DE08 BE08 CE08 2E08
uint32_t CTRL_R3;	//DE0C BE0C CE0C 2E0C
uint32_t CTRL_R4;	//DE10 BE10 CE10 2E10
uint32_t CTRL_R5;	//DE14 BE14 CE14 2E14
uint32_t CTRL_R6;	//DE18 BE18 CE18 2E18
uint32_t CTRL_R7;	//DE1C BE1C CE1C 2E1C
uint32_t CTRL_R8;	//DE20 BE20 CE20 2E20
uint32_t CTRL_R9;	//DE24 BE24 CE24 2E24
uint32_t CTRL_RA;	//DE28 BE28 CE28 2E28
uint32_t CTRL_RB;	//DE2C BE2C CE2C 2E2C
uint32_t CTRL_RC;	//DE30 BE30 CE30 2E30
uint32_t CTRL_RD;	//DE34 BE34 CE34 2E34
uint32_t rsvd0;   //DE38 BE38 CE38 2E38
uint32_t rsvd1;   //DE3C BE3C CE3C 2E3C
                                       
uint32_t CTRL_G0;	//DE40 BE40 CE40 2E40
uint32_t CTRL_G1;	//DE44 BE44 CE44 2E44
uint32_t CTRL_G2;	//DE48 BE48 CE48 2E48
uint32_t CTRL_G3;	//DE4C BE4C CE4C 2E4C
uint32_t CTRL_G4;	//DE50 BE50 CE50 2E50
uint32_t CTRL_G5;	//DE54 BE54 CE54 2E54
uint32_t CTRL_G6;	//DE58 BE58 CE58 2E58
uint32_t CTRL_G7;	//DE5C BE5C CE5C 2E5C
uint32_t CTRL_G8;	//DE60 BE60 CE60 2E60
uint32_t CTRL_G9;	//DE64 BE64 CE64 2E64
uint32_t CTRL_GA; //DE68 BE68 CE68 2E68
uint32_t CTRL_GB;	//DE6C BE6C CE6C 2E6C
uint32_t CTRL_GC;	//DE70 BE70 CE70 2E70
uint32_t CTRL_GD; //DE74 BE74 CE74 2E74
uint32_t rsvd2;   //DE78 BE78 CE78 2E78
uint32_t rsvd3;   //DE7C BE7C CE7C 2E7C
                                       
									                     
uint32_t CTRL_B0;	//DE80 BE80 CE80 2E80
uint32_t CTRL_B1;	//DE84 BE84 CE84 2E84
uint32_t CTRL_B2;	//DE88 BE88 CE88 2E88
uint32_t CTRL_B3;	//DE8C BE8C CE8C 2E8C
uint32_t CTRL_B4;	//DE90 BE90 CE90 2E90
uint32_t CTRL_B5;	//DE94 BE94 CE94 2E94
uint32_t CTRL_B6;	//DE98 BE98 CE98 2E98
uint32_t CTRL_B7;	//DE9C BE9C CE9C 2E9C
uint32_t CTRL_B8;	//DEA0 BEA0 CEA0 2EA0
uint32_t CTRL_B9;	//DEA4 BEA4 CEA4 2EA4
uint32_t CTRL_BA;	//DEA8 BEA8 CEA8 2EA8
uint32_t CTRL_BB;	//DEAC BEAC CEAC 2EAC
uint32_t CTRL_BC;	//DEB0 BEB0 CEB0 2EB0
uint32_t CTRL_BD;	//DEB4 BEB4 CEB4 2EB4
uint32_t rsvd4;   //DEB8 BEB8 CEB8 2EB8
uint32_t rsvd5;   //DEBC BEBC CEBC 2EBC
                                                
uint32_t CTRL_CK0;//DEC0 BEC0 CEC0 2EC0
uint32_t CTRL_CK1;//DEC4 BEC4 CEC4 2EC4
uint32_t CTRL_CK2;//DEC8 BEC8 CEC8 2EC8
uint32_t CTRL_CK3;//DECC BECC CECC 2ECC
uint32_t CTRL_CK4;//DED0 BED0 CED0 2ED0
uint32_t CTRL_CK5;//DED4 BED4 CED4 2ED4
uint32_t CTRL_CK6;//DED8 BED8 CED8 2ED8
uint32_t CTRL_CK7;//DEDC BEDC CEDC 2EDC
uint32_t CTRL_CK8;//DEE0 BEE0 CEE0 2EE0
uint32_t CTRL_CK9;//DEE4 BEE4 CEE4 2EE4
uint32_t CTRL_CKA;//DEE8 BEE8 CEE8 2EE8
uint32_t CTRL_CKB;//DEEC BEEC CEEC 2EEC
uint32_t CTRL_CKC;//DEF0 BEF0 CEF0 2EF0
uint32_t CTRL_CKD;//DEF4 BEF4 CEF4 2EF4

};




//extern const struct hdmi_dfe_reg_st *dfe[4];
extern const struct hdmi21_dfe_reg_st *hd21_dfe[4];  //hd20 & hd21 share

#ifdef CONFIG_ARM64
// new
#define REG_DFE_CTRL_R0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R0)//DE00 BE00 CE00 2E00   
#define REG_DFE_CTRL_R1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R1)//DE04 BE04 CE04 2E04   
#define REG_DFE_CTRL_R2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R2)//DE08 BE08 CE08 2E08   
#define REG_DFE_CTRL_R3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R3)//DE0C BE0C CE0C 2E0C   
#define REG_DFE_CTRL_R4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R4)//DE10 BE10 CE10 2E10   
#define REG_DFE_CTRL_R5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R5)//DE14 BE14 CE14 2E14   
#define REG_DFE_CTRL_R6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R6)//DE18 BE18 CE18 2E18   
#define REG_DFE_CTRL_R7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R7)//DE1C BE1C CE1C 2E1C   
#define REG_DFE_CTRL_R8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R8)//DE20 BE20 CE20 2E20   
#define REG_DFE_CTRL_R9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_R9)//DE24 BE24 CE24 2E24   
#define REG_DFE_CTRL_RA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_RA)//DE28 BE28 CE28 2E28   
#define REG_DFE_CTRL_RB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_RB)//DE2C BE2C CE2C 2E2C   
#define REG_DFE_CTRL_RC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_RC)//DE30 BE30 CE30 2E30   
#define REG_DFE_CTRL_RD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_RD)//DE34 BE34 CE34 2E34   
                                                                                             
#define REG_DFE_CTRL_G0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G0)//DE40 BE40 CE40 2E40   
#define REG_DFE_CTRL_G1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G1)//DE44 BE44 CE44 2E44   
#define REG_DFE_CTRL_G2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G2)//DE48 BE48 CE48 2E48   
#define REG_DFE_CTRL_G3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G3)//DE4C BE4C CE4C 2E4C   
#define REG_DFE_CTRL_G4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G4)//DE50 BE50 CE50 2E50   
#define REG_DFE_CTRL_G5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G5)//DE54 BE54 CE54 2E54   
#define REG_DFE_CTRL_G6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G6)//DE58 BE58 CE58 2E58   
#define REG_DFE_CTRL_G7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G7)//DE5C BE5C CE5C 2E5C   
#define REG_DFE_CTRL_G8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G8)//DE60 BE60 CE60 2E60   
#define REG_DFE_CTRL_G9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_G9)//DE64 BE64 CE64 2E64   
#define REG_DFE_CTRL_GA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_GA)//DE68 BE68 CE68 2E68   
#define REG_DFE_CTRL_GB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_GB)//DE6C BE6C CE6C 2E6C   
#define REG_DFE_CTRL_GC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_GC)//DE70 BE70 CE70 2E70   
#define REG_DFE_CTRL_GD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_GD)//DE74 BE74 CE74 2E74   
                                                                                        
#define REG_DFE_CTRL_B0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B0)//DE80 BE80 CE80 2E80   
#define REG_DFE_CTRL_B1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B1)//DE84 BE84 CE84 2E84   
#define REG_DFE_CTRL_B2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B2)//DE88 BE88 CE88 2E88   
#define REG_DFE_CTRL_B3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B3)//DE8C BE8C CE8C 2E8C   
#define REG_DFE_CTRL_B4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B4)//DE90 BE90 CE90 2E90   
#define REG_DFE_CTRL_B5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B5)//DE94 BE94 CE94 2E94   
#define REG_DFE_CTRL_B6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B6)//DE98 BE98 CE98 2E98   
#define REG_DFE_CTRL_B7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B7)//DE9C BE9C CE9C 2E9C   
#define REG_DFE_CTRL_B8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B8)//DEA0 BEA0 CEA0 2EA0   
#define REG_DFE_CTRL_B9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_B9)//DEA4 BEA4 CEA4 2EA4   
#define REG_DFE_CTRL_BA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_BA)//DEA8 BEA8 CEA8 2EA8   
#define REG_DFE_CTRL_BB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_BB)//DEAC BEAC CEAC 2EAC   
#define REG_DFE_CTRL_BC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_BC)//DEB0 BEB0 CEB0 2EB0  
#define REG_DFE_CTRL_BD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_BD)//DEB4 BEB4 CEB4 2EB4   
                                                                                             
#define REG_DFE_CTRL_CK0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK0)//DEC0 BEC0 CEC0 2EC0 
#define REG_DFE_CTRL_CK1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK1)//DEC4 BEC4 CEC4 2EC4 
#define REG_DFE_CTRL_CK2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK2)//DEC8 BEC8 CEC8 2EC8 
#define REG_DFE_CTRL_CK3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK3)//DECC BECC CECC 2ECC 
#define REG_DFE_CTRL_CK4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK4)//DED0 BED0 CED0 2ED0 
#define REG_DFE_CTRL_CK5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK5)//DED4 BED4 CED4 2ED4 
#define REG_DFE_CTRL_CK6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK6)//DED8 BED8 CED8 2ED8 
#define REG_DFE_CTRL_CK7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK7)//DEDC BEDC CEDC 2EDC 
#define REG_DFE_CTRL_CK8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK8)//DEE0 BEE0 CEE0 2EE0 
#define REG_DFE_CTRL_CK9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CK9)//DEE4 BEE4 CEE4 2EE4 
#define REG_DFE_CTRL_CKA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CKA)//DEE8 BEE8 CEE8 2EE8 
#define REG_DFE_CTRL_CKB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CKB)//DEEC BEEC CEEC 2EEC 
#define REG_DFE_CTRL_CKC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CKC)//DEF0 BEF0 CEF0 2EF0 
#define REG_DFE_CTRL_CKD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport]->CTRL_CKD)//DEF4 BEF4 CEF4 2EF4 

#else

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
#endif



//P0

#define LOAD_IN_INIT_LE     (_BIT31)
#define LOAD_IN_INIT_VTH    (_BIT30)
#define LOAD_IN_INIT_SERVO  (_BIT29)
#define LOAD_IN_INIT_TAP4   (_BIT28)
#define LOAD_IN_INIT_TAP3   (_BIT27)
#define LOAD_IN_INIT_TAP2   (_BIT26)
#define LOAD_IN_INIT_TAP1   (_BIT25)
#define LOAD_IN_INIT_TAP0   (_BIT24)
#define LOAD_IN_INIT_ALL    (LOAD_IN_INIT_LE|LOAD_IN_INIT_VTH|LOAD_IN_INIT_SERVO|LOAD_IN_INIT_TAP4|LOAD_IN_INIT_TAP3|LOAD_IN_INIT_TAP2|LOAD_IN_INIT_TAP1|LOAD_IN_INIT_TAP0)

//From Mark2
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

#define LOAD_IN_INIT_LE_P1		(_BIT31)
#define LOAD_IN_INIT_VTH_P1	(_BIT5)
#define LOAD_IN_INIT_TAP4_P1	(_BIT4)
#define LOAD_IN_INIT_TAP3_P1	(_BIT3)
#define LOAD_IN_INIT_TAP2_P1	(_BIT2)
#define LOAD_IN_INIT_TAP1_P1	(_BIT1)
#define LOAD_IN_INIT_TAP0_P1	(_BIT0)

/////End from Mark2 

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

