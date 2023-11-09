#include <rbus/dfe_p0_regNew.h>
#include <rbus/dfe_p1_regNew.h>

#ifdef UT_flag
typedef unsigned int uint32_t;
#endif
#ifdef BUILD_QUICK_SHOW
#include <sysdefs.h>
#include <sed_types.h>
#endif

#define HDMI_DFE0_BASE           (0xB800DE00)
#define HDMI_DFE1_BASE           (0xB803BE00)
#define HDMI_DFE2_BASE           (0xB803CE00)

#define HDMI21_PORT  HDMI_PORT1

struct hdmi_dfe_reg_st
{
    uint32_t en_l0;//DE00
    uint32_t init0_l0;//DE04
    uint32_t init1_l0;//DE08
    uint32_t dummy1;//DE0C
    uint32_t en_l1;//DE10
    uint32_t init0_l1;//DE14
    uint32_t init1_l1;//DE18
    uint32_t dummy2;//DE1C
    uint32_t en_l2;//DE20
    uint32_t init0_l2;//DE24
    uint32_t init1_l2;//DE28
    uint32_t dummy3;//DE2C
    uint32_t mode;//DE30
    uint32_t gain;//DE34
    uint32_t limit0;//DE38
    uint32_t limit1;//DE3C
    uint32_t limit2;//DE40
    uint32_t readback;//DE44
    uint32_t flag;//DE48
    uint32_t debug;//DE4C
};

struct hdmi21_dfe_reg_st {
uint32_t CTRL_R0;	//BE00 CE00 
uint32_t CTRL_R1;	//BE04 CE04 
uint32_t CTRL_R2;	//BE08 CE08
uint32_t CTRL_R3;	//BE0C CE0C
uint32_t CTRL_R4;	//BE10 CE10
uint32_t CTRL_R5;	//BE14 CE14
uint32_t CTRL_R6;	//BE18 CE18
uint32_t CTRL_R7;	//BE1C CE1C
uint32_t CTRL_R8;	//BE20 CE20
uint32_t CTRL_R9;	//BE24 CE24
uint32_t CTRL_RA;	//BE28 CE28 
uint32_t CTRL_RB;	//BE2C CE2C
uint32_t CTRL_RC;	//BE30 CE30
uint32_t CTRL_RD;	//BE34 CE34
uint32_t rsvd0;  //BE38	CE38
uint32_t rsvd1;  //BE3C	 CE3C
 
uint32_t CTRL_G0;	//BE40 CE40 
uint32_t CTRL_G1;	//BE44 CE44 
uint32_t CTRL_G2;	//BE48 CE48 
uint32_t CTRL_G3;	//BE4C CE4C
uint32_t CTRL_G4;	//BE50 CE50 
uint32_t CTRL_G5;	//BE54 CE54 
uint32_t CTRL_G6;	//BE58 CE58 
uint32_t CTRL_G7;	//BE5C CE5C
uint32_t CTRL_G8;	//BE60 CE60
uint32_t CTRL_G9;	//BE64 CE64 
uint32_t CTRL_GA;//BE68 CE68
uint32_t CTRL_GB;	//BE6C CE6C 
uint32_t CTRL_GC;	//BE70 CE70
uint32_t CTRL_GD;//BE74 CE74 
uint32_t rsvd2;  //BE78	CE78
uint32_t rsvd3;  //BE7C	 CE7C
 
									   
uint32_t CTRL_B0;	//BE80 CE80 
uint32_t CTRL_B1;	//BE84 CE84 
uint32_t CTRL_B2;	//BE88 CE88 
uint32_t CTRL_B3;	//BE8C CE8C 
uint32_t CTRL_B4;	//BE90 CE90 
uint32_t CTRL_B5;	//BE94 CE94 
uint32_t CTRL_B6;	//BE98 CE98 
uint32_t CTRL_B7;	//BE9C CE9C 
uint32_t CTRL_B8;	//BEA0 CEA0 
uint32_t CTRL_B9;	//BEA4 CEA4 
uint32_t CTRL_BA;	//BEA8 CEA8 
uint32_t CTRL_BB;	//BEAC CEAC
uint32_t CTRL_BC;	//BEB0 CEB0 
uint32_t CTRL_BD;	//BEB4 CEB4 
uint32_t rsvd4;  //BEB8	 CEB8
uint32_t rsvd5;  //BEBC	 CEBC
                                       
uint32_t CTRL_CK0;//BEC0 CEC0
uint32_t CTRL_CK1;//BEC4 CEC4
uint32_t CTRL_CK2;//BEC8 CEC8 
uint32_t CTRL_CK3;//BECC CECC 
uint32_t CTRL_CK4;//BED0 CED0
uint32_t CTRL_CK5;//BED4 CED4
uint32_t CTRL_CK6;//BED8 CED8 
uint32_t CTRL_CK7;//BEDC CEDC
uint32_t CTRL_CK8;//BEE0 CEE0
uint32_t CTRL_CK9;//BEE4 CEE4
uint32_t CTRL_CKA;//BEE8 CEE8
uint32_t CTRL_CKB;//BEEC CEEC
uint32_t CTRL_CKC;//BEF0 CEF0 
uint32_t CTRL_CKD;//BEF4 CEF4 

};




extern const struct hdmi_dfe_reg_st *dfe[1];
extern const struct hdmi21_dfe_reg_st *hd21_dfe[2];  //hd20 & hd21 share

#ifdef CONFIG_ARM64
#define DFE_REG_DFE_EN_L0_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->en_l0))          // 0xB800DE00 DE50 DEA0
#define DFE_REG_DFE_INIT0_L0_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init0_l0))       // 0xB800DE04 DE54 DEA4
#define DFE_REG_DFE_INIT1_L0_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init1_l0))       // 0xB800DE08 DE58 DEA8
#define DFE_REG_DFE_EN_L1_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->en_l1))          // 0xB800DE10 DE60 DEB0
#define DFE_REG_DFE_INIT0_L1_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init0_l1))       // 0xB800DE14 DE64 DEA4
#define DFE_REG_DFE_INIT1_L1_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init1_l1))       // 0xB800DE18 DE68 DEA8
#define DFE_REG_DFE_EN_L2_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->en_l2 ))         // 0xB800DE20 DE70 DEB0
#define DFE_REG_DFE_INIT0_L2_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init0_l2))       // 0xB800DE24 DE74 DEC4
#define DFE_REG_DFE_INIT1_L2_reg               ((unsigned int)(unsigned long)(&dfe[nport]->init1_l2))       // 0xB800DE28 DE78 DEC8

#define DFE_REG_DFE_MODE_reg                   ((unsigned int)(unsigned long)(&dfe[nport]->mode))           // 0xB800DE30 DE80 DED0
#define DFE_REG_DFE_GAIN_reg                   ((unsigned int)(unsigned long)(&dfe[nport]->gain))           // 0xB800DE34 DE84 DED4
#define DFE_REG_DFE_LIMIT0_reg                 ((unsigned int)(unsigned long)(&dfe[nport]->limit0))         // 0xB800DE38 DE88 DED8
#define DFE_REG_DFE_LIMIT1_reg                 ((unsigned int)(unsigned long)(&dfe[nport]->limit1))         // 0xB800DE3C DE8C DEDC
#define DFE_REG_DFE_LIMIT2_reg                 ((unsigned int)(unsigned long)(&dfe[nport]->limit2))         // 0xB800DE40 DE90 DEE0
#define DFE_REG_DFE_READBACK_reg               ((unsigned int)(unsigned long)(&dfe[nport]->readback))       // 0xB800DE44 DE94 DEE4
#define DFE_REG_DFE_FLAG_reg                   ((unsigned int)(unsigned long)(&dfe[nport]->flag))           // 0xB800DE48 DE98 DEE8
#define DFE_REG_DFE_DEBUG_reg                  ((unsigned int)(unsigned long)(&dfe[nport]->debug))          // 0xB800DE4C DE9C DEEC

// new
#define REG_DFE_CTRL_R0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R0)//7C00 9C00 AC00 CC00   
#define REG_DFE_CTRL_R1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R1)//7C04 9C04 AC04 CC04   
#define REG_DFE_CTRL_R2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R2)//7C08 9C08 AC08 CC08   
#define REG_DFE_CTRL_R3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R3)//7C0C 9C0C AC0C CC0C   
#define REG_DFE_CTRL_R4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R4)//7C10 9C10 AC10 CC10   
#define REG_DFE_CTRL_R5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R5)//7C14 9C14 AC14 CC14   
#define REG_DFE_CTRL_R6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R6)//7C18 9C18 AC18 CC18   
#define REG_DFE_CTRL_R7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R7)//7C1C 9C1C AC1C CC1C   
#define REG_DFE_CTRL_R8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R8)//7C20 9C20 AC20 CC20   
#define REG_DFE_CTRL_R9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R9)//7C24 9C24 AC24 CC24   
#define REG_DFE_CTRL_RA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RA)//7C28 9C28 AC28 CC28   
#define REG_DFE_CTRL_RB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RB)//7C2C 9C2C AC2C CC2C   
#define REG_DFE_CTRL_RC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RC)//7C30 9C30 AC30 CC30   
#define REG_DFE_CTRL_RD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RD)//7C34 9C34 AC34 CC34   
                                                                                             
#define REG_DFE_CTRL_G0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G0)//7C40 9C40 AC40 CC40   
#define REG_DFE_CTRL_G1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G1)//7C44 9C44 AC44 CC44   
#define REG_DFE_CTRL_G2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G2)//7C48 9C48 AC48 CC48   
#define REG_DFE_CTRL_G3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G3)//7C4C 9C4C AC4C CC4C   
#define REG_DFE_CTRL_G4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G4)//7C50 9C50 AC50 CC50   
#define REG_DFE_CTRL_G5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G5)//7C54 9C54 AC54 CC54   
#define REG_DFE_CTRL_G6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G6)//7C58 9C58 AC58 CC58   
#define REG_DFE_CTRL_G7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G7)//7C5C 9C5C AC5C CC5C   
#define REG_DFE_CTRL_G8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G8)//7C60 9C60 AC60 CC60   
#define REG_DFE_CTRL_G9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G9)//7C64 9C64 AC64 CC64   
#define REG_DFE_CTRL_GA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GA)//7C68 9C68 AC68 CC68   
#define REG_DFE_CTRL_GB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GB)//7C6C 9C6C AC6C CC6C   
#define REG_DFE_CTRL_GC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GC)//7C70 9C70 AC70 CC70   
#define REG_DFE_CTRL_GD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GD)//7C74 9C74 AC74 CC74   
                                                                                        
#define REG_DFE_CTRL_B0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B0)//7C80 9C80 AC80 CC80   
#define REG_DFE_CTRL_B1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B1)//7C84 9C84 AC84 CC84   
#define REG_DFE_CTRL_B2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B2)//7C88 9C88 AC88 CC88   
#define REG_DFE_CTRL_B3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B3)//7C8C 9C8C AC8C CC8C   
#define REG_DFE_CTRL_B4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B4)//7C90 9C90 AC90 CC90   
#define REG_DFE_CTRL_B5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B5)//7C94 9C94 AC94 CC94   
#define REG_DFE_CTRL_B6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B6)//7C98 9C98 AC98 CC98   
#define REG_DFE_CTRL_B7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B7)//7C9C 9C9C AC9C CC9C   
#define REG_DFE_CTRL_B8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B8)//7CA0 9CA0 ACA0 CCA0   
#define REG_DFE_CTRL_B9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B9)//7CA4 9CA4 ACA4 CCA4   
#define REG_DFE_CTRL_BA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BA)//7CA8 9CA8 ACA8 CCA8   
#define REG_DFE_CTRL_BB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BB)//7CAC 9CAC ACAC CCAC   
#define REG_DFE_CTRL_BC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BC)//7CB0 9A0 ACA0 CCA0   
#define REG_DFE_CTRL_BD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BD)//7CB4 9CA4 ACA4 CCA4   
                                                                                             
#define REG_DFE_CTRL_CK0_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK0)//7CC0 9CC0 ACC0 CCC0 
#define REG_DFE_CTRL_CK1_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK1)//7CC4 9CC4 ACC4 CCC4 
#define REG_DFE_CTRL_CK2_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK2)//7CC8 9CC8 ACC8 CCC8 
#define REG_DFE_CTRL_CK3_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK3)//7CCC 9CCC ACCC CCCC 
#define REG_DFE_CTRL_CK4_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK4)//7CD0 9CD0 ACD0 CCD0 
#define REG_DFE_CTRL_CK5_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK5)//7CD4 9CD4 ACD4 CCD4 
#define REG_DFE_CTRL_CK6_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK6)//7CD8 9CD8 ACD8 CCD8 
#define REG_DFE_CTRL_CK7_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK7)//7CDC 9CDC ACDC CCDC 
#define REG_DFE_CTRL_CK8_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK8)//7CE0 9CE0 ACE0 CCE0 
#define REG_DFE_CTRL_CK9_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK9)//7CE4 9CE4 ACE4 CCE4 
#define REG_DFE_CTRL_CKA_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKA)//7CE8 9CE8 ACE8 CCE8 
#define REG_DFE_CTRL_CKB_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKB)//7CEC 9CEC ACEC CCEC 
#define REG_DFE_CTRL_CKC_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKC)//7CF0 9CF0 ACF0 CCF0 
#define REG_DFE_CTRL_CKD_reg (unsigned int)(unsigned long)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKD)//7CF4 9CF4 ACF4 CCF4 

#else
#define DFE_REG_DFE_EN_L0_reg                  ((unsigned int)(&dfe[nport]->en_l0))             //  0xB800DE00 DE50 DEA0
#define DFE_REG_DFE_INIT0_L0_reg               ((unsigned int)(&dfe[nport]->init0_l0))          // 0xB800DE04 DE54 DEA4
#define DFE_REG_DFE_INIT1_L0_reg               ((unsigned int)(&dfe[nport]->init1_l0))          // 0xB800DE08 DE58 DEA8
#define DFE_REG_DFE_EN_L1_reg                  ((unsigned int)(&dfe[nport]->en_l1))             //  0xB800DE10 DE60 DEB0
#define DFE_REG_DFE_INIT0_L1_reg               ((unsigned int)(&dfe[nport]->init0_l1))          //  0xB800DE14 DE64 DEA4
#define DFE_REG_DFE_INIT1_L1_reg               ((unsigned int)(&dfe[nport]->init1_l1))          // 0xB800DE18 DE68 DEA8
#define DFE_REG_DFE_EN_L2_reg                  ((unsigned int)(&dfe[nport]->en_l2 ))            // 0xB800DE20 DE70 DEB0
#define DFE_REG_DFE_INIT0_L2_reg               ((unsigned int)(&dfe[nport]->init0_l2))          // 0 0xB800DE24 DE74 DEC4
#define DFE_REG_DFE_INIT1_L2_reg               ((unsigned int)(&dfe[nport]->init1_l2))          //  0xB800DE24 DE74 DEC4

#define DFE_REG_DFE_MODE_reg                   ((unsigned int)(&dfe[nport]->mode))              // 0xB800DE30 DE80 DEC0
#define DFE_REG_DFE_GAIN_reg                   ((unsigned int)(&dfe[nport]->gain))              // 0xB800DE34 DE84 DEC4
#define DFE_REG_DFE_LIMIT0_reg                 ((unsigned int)(&dfe[nport]->limit0))            // 0xB800DE38 DE88 DEC8
#define DFE_REG_DFE_LIMIT1_reg                 ((unsigned int)(&dfe[nport]->limit1))            // 0xB800DE3C DE8C DEDC
#define DFE_REG_DFE_LIMIT2_reg                 ((unsigned int)(&dfe[nport]->limit2))            // 0xB800DE40 DE90 DEE0
#define DFE_REG_DFE_READBACK_reg               ((unsigned int)(&dfe[nport]->readback))          // 0xB800DE44 DE94 DEE4
#define DFE_REG_DFE_FLAG_reg                   ((unsigned int)(&dfe[nport]->flag))              // 0xB800DE48 DE98 DEE8
#define DFE_REG_DFE_DEBUG_reg                  ((unsigned int)(&dfe[nport]->debug))             // 0xB800DE4C DE9C DEEC
// New
#define REG_DFE_CTRL_R0_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R0)//7C00 9C00 AC00 CC00   
#define REG_DFE_CTRL_R1_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R1)//7C04 9C04 AC04 CC04   
#define REG_DFE_CTRL_R2_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R2)//7C08 9C08 AC08 CC08   
#define REG_DFE_CTRL_R3_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R3)//7C0C 9C0C AC0C CC0C   
#define REG_DFE_CTRL_R4_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R4)//7C10 9C10 AC10 CC10   
#define REG_DFE_CTRL_R5_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R5)//7C14 9C14 AC14 CC14   
#define REG_DFE_CTRL_R6_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R6)//7C18 9C18 AC18 CC18   
#define REG_DFE_CTRL_R7_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R7)//7C1C 9C1C AC1C CC1C   
#define REG_DFE_CTRL_R8_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R8)//7C20 9C20 AC20 CC20   
#define REG_DFE_CTRL_R9_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_R9)//7C24 9C24 AC24 CC24   
#define REG_DFE_CTRL_RA_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RA)//7C28 9C28 AC28 CC28   
#define REG_DFE_CTRL_RB_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RB)//7C2C 9C2C AC2C CC2C   
#define REG_DFE_CTRL_RC_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RC)//7C30 9C30 AC30 CC30   
#define REG_DFE_CTRL_RD_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_RD)//7C34 9C34 AC34 CC34   
                                                                                             
#define REG_DFE_CTRL_G0_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G0)//7C40 9C40 AC40 CC40   
#define REG_DFE_CTRL_G1_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G1)//7C44 9C44 AC44 CC44   
#define REG_DFE_CTRL_G2_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G2)//7C48 9C48 AC48 CC48   
#define REG_DFE_CTRL_G3_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G3)//7C4C 9C4C AC4C CC4C   
#define REG_DFE_CTRL_G4_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G4)//7C50 9C50 AC50 CC50   
#define REG_DFE_CTRL_G5_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G5)//7C54 9C54 AC54 CC54   
#define REG_DFE_CTRL_G6_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G6)//7C58 9C58 AC58 CC58   
#define REG_DFE_CTRL_G7_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G7)//7C5C 9C5C AC5C CC5C   
#define REG_DFE_CTRL_G8_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G8)//7C60 9C60 AC60 CC60   
#define REG_DFE_CTRL_G9_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_G9)//7C64 9C64 AC64 CC64   
#define REG_DFE_CTRL_GA_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GA)//7C68 9C68 AC68 CC68   
#define REG_DFE_CTRL_GB_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GB)//7C6C 9C6C AC6C CC6C   
#define REG_DFE_CTRL_GC_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GC)//7C70 9C70 AC70 CC70   
#define REG_DFE_CTRL_GD_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_GD)//7C74 9C74 AC74 CC74   
                                                                                        
#define REG_DFE_CTRL_B0_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B0)//7C80 9C80 AC80 CC80   
#define REG_DFE_CTRL_B1_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B1)//7C84 9C84 AC84 CC84   
#define REG_DFE_CTRL_B2_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B2)//7C88 9C88 AC88 CC88   
#define REG_DFE_CTRL_B3_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B3)//7C8C 9C8C AC8C CC8C   
#define REG_DFE_CTRL_B4_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B4)//7C90 9C90 AC90 CC90   
#define REG_DFE_CTRL_B5_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B5)//7C94 9C94 AC94 CC94   
#define REG_DFE_CTRL_B6_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B6)//7C98 9C98 AC98 CC98   
#define REG_DFE_CTRL_B7_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B7)//7C9C 9C9C AC9C CC9C   
#define REG_DFE_CTRL_B8_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B8)//7CA0 9CA0 ACA0 CCA0   
#define REG_DFE_CTRL_B9_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_B9)//7CA4 9CA4 ACA4 CCA4   
#define REG_DFE_CTRL_BA_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BA)//7CA8 9CA8 ACA8 CCA8   
#define REG_DFE_CTRL_BB_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BB)//7CAC 9CAC ACAC CCAC   
#define REG_DFE_CTRL_BC_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BC)//7CB0 9A0 ACA0 CCA0   
#define REG_DFE_CTRL_BD_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_BD)//7CB4 9CA4 ACA4 CCA4   
                                                                                             
#define REG_DFE_CTRL_CK0_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK0)//7CC0 9CC0 ACC0 CCC0 
#define REG_DFE_CTRL_CK1_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK1)//7CC4 9CC4 ACC4 CCC4 
#define REG_DFE_CTRL_CK2_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK2)//7CC8 9CC8 ACC8 CCC8 
#define REG_DFE_CTRL_CK3_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK3)//7CCC 9CCC ACCC CCCC 
#define REG_DFE_CTRL_CK4_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK4)//7CD0 9CD0 ACD0 CCD0 
#define REG_DFE_CTRL_CK5_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK5)//7CD4 9CD4 ACD4 CCD4 
#define REG_DFE_CTRL_CK6_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK6)//7CD8 9CD8 ACD8 CCD8 
#define REG_DFE_CTRL_CK7_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK7)//7CDC 9CDC ACDC CCDC 
#define REG_DFE_CTRL_CK8_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK8)//7CE0 9CE0 ACE0 CCE0 
#define REG_DFE_CTRL_CK9_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CK9)//7CE4 9CE4 ACE4 CCE4 
#define REG_DFE_CTRL_CKA_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKA)//7CE8 9CE8 ACE8 CCE8 
#define REG_DFE_CTRL_CKB_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKB)//7CEC 9CEC ACEC CCEC 
#define REG_DFE_CTRL_CKC_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKC)//7CF0 9CF0 ACF0 CCF0 
#define REG_DFE_CTRL_CKD_reg (unsigned int)(&hd21_dfe[nport-HDMI_PORT1]->CTRL_CKD)//7CF4 9CF4 ACF4 CCF4 
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

#define REG_dfe_adapt_en_lane0_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane0_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane0_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane0_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane0_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane0_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane0_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane0_LE                                          _BIT23
#define REG_dfe_adapt_en_lane1_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane1_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane1_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane1_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane1_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane1_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane1_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane1_LE                                          _BIT23
#define REG_dfe_adapt_en_lane2_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane2_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane2_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane2_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane2_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane2_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane2_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane2_LE                                          _BIT23
#define REG_dfe_adapt_en_lane3_TAP0                                        _BIT16
#define REG_dfe_adapt_en_lane3_TAP1                                        _BIT17
#define REG_dfe_adapt_en_lane3_TAP2                                        _BIT18
#define REG_dfe_adapt_en_lane3_TAP3                                        _BIT19
#define REG_dfe_adapt_en_lane3_TAP4                                        _BIT20
#define REG_dfe_adapt_en_lane3_SERVO                                       _BIT21
#define REG_dfe_adapt_en_lane3_Vth                                         _BIT22
#define REG_dfe_adapt_en_lane3_LE                                          _BIT23


#define LIMIT_TAP0_MAX          0
#define LIMIT_TAP0_MIN          1
#define LIMIT_TAP1_MAX          2
#define LIMIT_TAP1_MIN          3
#define LIMIT_LE_MAX            4
#define LIMIT_LE_MIN            5

#define COEF_VTH                0
#define COEF_TAP0               1
#define COEF_TAP1               2
#define COEF_TAP2               3
#define COEF_TAP3               4
#define COEF_TAP4               5
#define COEF_SERVO              6
#define COEF_LE                 7


#define get_TAP2_coef_sign(data)                    (0x20&(data))
#define get_TAP2_coef(data)                         (0x1F&(data))
#define get_TAP3_coef_sign(data)                    (0x20&(data))
#define get_TAP3_coef(data)                         (0x1F&(data))
#define get_TAP4_coef_sign(data)                    (0x20&(data))
#define get_TAP4_coef(data)                         (0x1F&(data))

